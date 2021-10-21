#include "SkyrimPlatform.h"

#include "ThreadPoolWrapper.h"
#include <SKSE/API.h>
#include <SKSE/Interfaces.h>
#include <SKSE/Stubs.h>
#include <skse64/PluginAPI.h>

#include "BrowserApi.h"    // BrowserApi::State
#include "CallNativeApi.h" // CallNativeApi::NativeCallRequirements

// HelloTickListener
#include <RE/ConsoleLog.h>

// CommonExecutionListener
#include "ConsoleApi.h"
#include "DirectoryMonitor.h"
#include "EventsApi.h"
#include "ExceptionPrinter.h"
#include "HttpClient.h"
#include "ReadFile.h"
#include "SkyrimPlatformProxy.h"

// APIs for register in CommonExecutionListener
#include "BrowserApi.h"
#include "CallNativeApi.h"
#include "CameraApi.h"
#include "ConsoleApi.h"
#include "DevApi.h"
#include "EncodingApi.h"
#include "EventsApi.h"
#include "HttpClientApi.h"
#include "InventoryApi.h"
#include "LoadGameApi.h"
#include "MpClientPluginApi.h"

CallNativeApi::NativeCallRequirements g_nativeCallRequirements;
ThreadPoolWrapper g_pool;

namespace {
const char* RemoveMultiplePrefixes(const char* str, const char* prefix)
{
  size_t prefixLen = strlen(prefix);
  while (strlen(str) >= prefixLen && !memcmp(str, prefix, prefixLen)) {
    str += prefixLen;
  }
  return str;
}

void PrintExceptionToGameConsole(const std::exception& e)
{
  if (auto console = RE::ConsoleLog::GetSingleton()) {
    auto what = RemoveMultiplePrefixes(e.what(), "Error: ");
    ExceptionPrinter(ConsoleApi::GetExceptionPrefix()).PrintException(what);
  }
}

bool EndsWith(const std::wstring& value, const std::wstring& ending)
{
  return ending.size() <= value.size() &&
    std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
}

namespace {
class TickListener
{
public:
  virtual ~TickListener() = default;
  virtual void Tick() = 0;
  virtual void Update() = 0;
};

class HelloTickListener : public TickListener
{
public:
  void Tick() override
  {
    if (auto console = RE::ConsoleLog::GetSingleton()) {
      if (!helloSaid) {
        helloSaid = true;
        console->Print("Hello SE");
      }
    }
  }

  void Update() override {}

private:
  bool helloSaid = false;
};

class CommonExecutionListener : public TickListener
{
public:
  CommonExecutionListener(std::shared_ptr<BrowserApi::State> browserApiState_)
    : nativeCallRequirements(g_nativeCallRequirements)
    , browserApiState(browserApiState_)
  {
  }

  void Tick() override
  {
    try {
      auto fileDir = GetFileDir();

      if (!monitor) {
        monitor = std::make_shared<DirectoryMonitor>(fileDir);
      }

      ++tickId;

      bool pluginsDirectoryUpdated = monitor->Updated();
      monitor->ThrowOnceIfHasError();
      if (tickId == 1 || pluginsDirectoryUpdated) {
        ClearState();
        LoadFiles(GetPathsToLoad(fileDir));
      }

      HttpClientApi::GetHttpClient().ExecuteQueuedCallbacks();

      EventsApi::SendEvent("tick", {});

    } catch (const std::exception& e) {
      PrintExceptionToGameConsole(e);
    }
  }

  void Update() override
  {
    try {
      taskQueue.Update();
      nativeCallRequirements.jsThrQ->Update();
      jsPromiseTaskQueue.Update();
      EventsApi::SendEvent("update", {});
    } catch (const std::exception& e) {
      PrintExceptionToGameConsole(e);
    }
  }

private:
  const char* GetFileDir() const { return "Data/Platform/Plugins"; }

  void LoadFiles(const std::vector<std::filesystem::path>& pathsToLoad)
  {
    auto& engine = GetJsEngine();

    for (auto& path : pathsToLoad) {
      if (EndsWith(path.wstring(), L"-settings.txt")) {
        LoadSettingsFile(path);
        continue;
      }
      if (EndsWith(path.wstring(), L"-logs.txt")) {
        continue;
      }
      LoadPluginFile(path);
    }
  }

  void LoadSettingsFile(const std::filesystem::path& path)
  {
    auto s = path.filename().wstring();
    s.resize(s.size() - strlen("-settings.txt"));

    auto pluginName = std::filesystem::path(s).string();

    // Why do we treat it as an exception actually?
    std::string what =
      "Found settings file: " + path.string() + " for plugin " + pluginName;
    ExceptionPrinter(ConsoleApi::GetExceptionPrefix())
      .PrintException(what.data());

    settingsByPluginName[pluginName] = ReadFile(path);
  }

  void LoadPluginFile(const std::filesystem::path& path)
  {
    auto scriptSrc = ReadFile(path);

    getSettings = [this](const JsFunctionArguments&) {
      auto result = JsValue::Object();
      auto standardJson = JsValue::GlobalObject().GetProperty("JSON");
      auto parse = standardJson.GetProperty("parse");
      for (const auto& [pluginName, settings] : settingsByPluginName) {
        result.SetProperty(pluginName, parse.Call({ standardJson, settings }));
      }
      return result;
    };

    // We will be able to use require()
    JsValue devApi = JsValue::Object();
    DevApi::Register(devApi, &engine,
                     { { "skyrimPlatform",
                         [this](JsValue e) {
                           EncodingApi::Register(e);
                           LoadGameApi::Register(e);
                           CameraApi::Register(e);
                           MpClientPluginApi::Register(e);
                           HttpClientApi::Register(e);
                           ConsoleApi::Register(e);
                           DevApi::Register(e, &engine, {}, GetFileDir());
                           EventsApi::Register(e);
                           BrowserApi::Register(e, browserApiState);
                           InventoryApi::Register(e);
                           CallNativeApi::Register(
                             e, [this] { return nativeCallRequirements; });
                           e.SetProperty("settings", getSettings, nullptr);

                           return SkyrimPlatformProxy::Attach(e);
                         } } },
                     GetFileDir());

    JsValue consoleApi = JsValue::Object();
    ConsoleApi::Register(consoleApi);
    for (auto f : { "require", "addNativeExports" }) {
      JsValue::GlobalObject().SetProperty(f, devApi.GetProperty(f));
    }
    JsValue::GlobalObject().SetProperty(
      "log", consoleApi.GetProperty("printConsole"));

    engine->RunScript(
      ReadFile(std::filesystem::path("Data/Platform/Distribution") /
               "___systemPolyfill.js"),
      "___systemPolyfill.js");
    engine->RunScript(
      "skyrimPlatform = addNativeExports('skyrimPlatform', {})", "");
    engine->RunScript(scriptSrc, path.filename().string()).ToString();
  }

  void ClearState()
  {
    ConsoleApi::Clear();
    EventsApi::Clear();
    taskQueue.Clear();
    jsPromiseTaskQueue.Clear();
    nativeCallRequirements.jsThrQ->Clear();
    settingsByPluginName.clear();
  }

  JsEngine& GetJsEngine()
  {
    if (!engine) {
      engine = std::make_shared<JsEngine>();
      engine->ResetContext(jsPromiseTaskQueue);
    }
    return *engine;
  }

  std::vector<std::filesystem::path> GetPathsToLoad(
    const std::filesystem::path& directory)
  {
    std::vector<std::filesystem::path> paths;
    for (auto& it : std::filesystem::directory_iterator(directory)) {
      std::filesystem::path p = it.is_directory() ? it / "index.js" : it;
      paths.push_back(p);
    }
    return paths;
  }

  std::shared_ptr<JsEngine> engine;
  std::shared_ptr<DirectoryMonitor> monitor;
  uint32_t tickId = 0;
  TaskQueue taskQueue;
  TaskQueue jsPromiseTaskQueue;
  CallNativeApi::NativeCallRequirements& nativeCallRequirements;
  std::unordered_map<std::string, std::string> settingsByPluginName;
  std::shared_ptr<BrowserApi::State> browserApiState;
  std::function<JsValue(const JsFunctionArguments&)> getSettings;
};
}

struct SkyrimPlatform::Impl
{
  std::shared_ptr<BrowserApi::State> browserApiState;
  std::vector<std::shared_ptr<TickListener>> tickListeners;
  TaskQueue tickTasks, updateTasks;
};

SkyrimPlatform::SkyrimPlatform()
{
  pImpl = std::make_shared<Impl>();
  pImpl->browserApiState = std::make_shared<BrowserApi::State>();

  pImpl->tickListeners.push_back(std::make_shared<HelloTickListener>());
  pImpl->tickListeners.push_back(
    std::make_shared<CommonExecutionListener>(pImpl->browserApiState));
}

SkyrimPlatform& SkyrimPlatform::GetSingleton()
{
  static SkyrimPlatform g_skyrimPlatform;
  return g_skyrimPlatform;
}

void SkyrimPlatform::JsTick(bool gameFunctionsAvailable)
{
  for (auto& listener : pImpl->tickListeners) {
    gameFunctionsAvailable ? listener->Update() : listener->Tick();
  }

  try {
    (gameFunctionsAvailable ? pImpl->updateTasks : pImpl->tickTasks).Update();
  } catch (const std::exception& e) {
    PrintExceptionToGameConsole(e);
  }
}

void SkyrimPlatform::SetOverlayService(
  std::shared_ptr<OverlayService> overlayService)
{
  pImpl->browserApiState->overlayService = overlayService;
}

void SkyrimPlatform::AddTickTask(const std::function<void()>& f)
{
  pImpl->tickTasks.AddTask(f);
}

void SkyrimPlatform::AddUpdateTask(const std::function<void()>& f)
{
  pImpl->updateTasks.AddTask(f);
}
