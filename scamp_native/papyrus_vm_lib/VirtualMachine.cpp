#include "VirtualMachine.h"
#include "Utils.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

VirtualMachine::VirtualMachine(std::vector<PexScript::Lazy> loadedScripts)
{
  for (auto& script : loadedScripts) {
    allLoadedScripts[CIString{ script.source.begin(), script.source.end() }] =
      script;
  }
}

VirtualMachine::VirtualMachine(std::vector<PexScript::Ptr> loadedScripts)
{
  for (auto& script : loadedScripts) {
    allLoadedScripts[CIString{ script->source.begin(),
                               script->source.end() }] = {
      script->source, [script] { return script; }
    };
  }
}

void VirtualMachine::SetExceptionHandler(const ExceptionHandler& handler)
{
  this->handler = handler;
}

std::string ToLower(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(), tolower);
  return s;
}

void VirtualMachine::RegisterFunction(std::string className,
                                      std::string functionName,
                                      FunctionType type, NativeFunction fn)
{
  switch (type) {
    case FunctionType::GlobalFunction:

      nativeStaticFunctions[ToLower(className)][ToLower(functionName)] = fn;
      break;
    case FunctionType::Method:
      nativeFunctions[ToLower(className)][ToLower(functionName)] = fn;
      break;
  }
}

void VirtualMachine::AddObject(IGameObject::Ptr self,
                               const std::vector<ScriptInfo>& scripts)
{
  std::vector<ActivePexInstance::Ptr> scriptsForObject;

  for (auto& s : scripts) {
    CIString ciNameNeedScript{ s.name.begin(), s.name.end() };
    auto it = allLoadedScripts.find(ciNameNeedScript);
    if (it != allLoadedScripts.end()) {
      auto scriptInstance = std::make_shared<ActivePexInstance>(
        it->second, s.vars, this, VarValue((IGameObject*)self.get()), "");
      scriptsForObject.push_back(scriptInstance);
    }
  }

  self->activePexInstances = scriptsForObject;
  gameObjectsHolder.insert(self);
}

void VirtualMachine::SendEvent(IGameObject::Ptr self, const char* eventName,
                               const std::vector<VarValue>& arguments)
{
  for (auto& scriptInstance : self->activePexInstances) {
    auto name = scriptInstance->GetActiveStateName();

    auto fn = scriptInstance->GetFunctionByName(
      eventName, scriptInstance->GetActiveStateName());
    if (fn.valid) {
      scriptInstance->StartFunction(
        fn, const_cast<std::vector<VarValue>&>(arguments));
    }
  }
}

void VirtualMachine::SendEvent(ActivePexInstance* instance,
                               const char* eventName,
                               const std::vector<VarValue>& arguments)
{

  auto fn =
    instance->GetFunctionByName(eventName, instance->GetActiveStateName());
  if (fn.valid) {
    instance->StartFunction(fn, const_cast<std::vector<VarValue>&>(arguments));
  }
}

VarValue VirtualMachine::CallMethod(IGameObject* self, const char* methodName,
                                    std::vector<VarValue>& arguments)
{
  if (!self) {
    std::stringstream ss;
    ss << '\'' << methodName
       << "' requires self argument to be a valid object";
    throw std::runtime_error(ss.str());
  }

  const char* nativeClass = self->GetParentNativeScript();
  const char* base = nativeClass;
  while (1) {
    if (auto f = nativeFunctions[ToLower(base)][ToLower(methodName)]) {
      return f(VarValue(self), arguments);
    }
    auto it = allLoadedScripts.find(base);
    if (it == allLoadedScripts.end())
      break;
    base = it->second.fn()->objectTable.m_data[0].parentClassName.data();
    if (!base[0])
      break;
  }

  const char* classToPrint = "";

  for (auto& activeScript : self->activePexInstances) {
    FunctionInfo functionInfo;

    if (!Utils::stricmp(methodName, "GotoState") ||
        !Utils::stricmp(methodName, "GetState")) {
      functionInfo = activeScript->GetFunctionByName(methodName, "");
    } else {
      functionInfo = activeScript->GetFunctionByName(
        methodName, activeScript->GetActiveStateName());
      if (!functionInfo.valid)
        functionInfo = activeScript->GetFunctionByName(methodName, "");
    }

    if (functionInfo.valid) {
      return activeScript->StartFunction(functionInfo, arguments);
    }
  }

  std::string e = "Method not found - '";
  e += classToPrint;
  e += (classToPrint[0] ? "." : "") + std::string(methodName) + "'";
  throw std::runtime_error(e);
}

VarValue VirtualMachine::CallStatic(std::string className,
                                    std::string functionName,
                                    std::vector<VarValue>& arguments)
{
  VarValue result = VarValue::None();
  FunctionInfo function;

  auto functionNameLower = ToLower(functionName);
  auto f = nativeStaticFunctions[ToLower(className)][functionNameLower]
    ? nativeStaticFunctions[ToLower(className)][functionNameLower]
    : nativeStaticFunctions[""][functionNameLower];

  if (f) {
    return f(VarValue::None(), arguments);
  }

  auto it =
    allLoadedScripts.find(CIString{ className.begin(), className.end() });
  if (it == allLoadedScripts.end())
    throw std::runtime_error("script not found - '" + className + "'");

  auto& instance = instancesForStaticCalls[className];
  if (!instance) {
    instance = std::make_shared<ActivePexInstance>(it->second, nullptr, this,
                                                   VarValue::None(), "");
  }

  function = instance->GetFunctionByName(functionName.c_str(), "");

  if (function.valid) {
    if (function.IsNative())
      throw std::runtime_error("Function not found - '" +
                               std::string(functionName) + "'");

    result = instance->StartFunction(function, arguments);
  }
  if (!function.valid)
    throw std::runtime_error("function is not valid");

  return result;
}

PexScript::Lazy VirtualMachine::GetPexByName(const std::string& name)
{
  auto it = allLoadedScripts.find(CIString{ name.begin(), name.end() });
  if (it != allLoadedScripts.end())
    return it->second;
  return PexScript::Lazy();
}

ActivePexInstance::Ptr VirtualMachine::CreateActivePexInstance(
  const std::string& pexScriptName, VarValue activeInstanceOwner,
  const std::shared_ptr<IVariablesHolder>& mapForFillPropertys,
  std::string childrenName)
{

  auto it = allLoadedScripts.find(
    CIString{ pexScriptName.begin(), pexScriptName.end() });
  if (it != allLoadedScripts.end()) {
    ActivePexInstance scriptInstance(it->second, mapForFillPropertys, this,
                                     activeInstanceOwner, childrenName);
    return std::make_shared<ActivePexInstance>(scriptInstance);
  }

  static const ActivePexInstance::Ptr notValidInstance =
    std::make_shared<ActivePexInstance>();

  if (pexScriptName != "")
    throw std::runtime_error("Unable to find script '" + pexScriptName + "'");

  return notValidInstance;
}

bool VirtualMachine::IsNativeFunctionByNameExisted(
  const std::string& name) const
{
  for (auto& staticFunction : nativeStaticFunctions) {
    if (staticFunction.first == name)
      return true;
  }

  for (auto& metod : nativeFunctions) {
    for (auto& func : metod.second)
      if (func.first == name)
        return true;
  }

  return false;
}

VirtualMachine::ExceptionHandler VirtualMachine::GetExceptionHandler() const
{
  return handler;
}

void VirtualMachine::RemoveObject(IGameObject::Ptr self)
{
}