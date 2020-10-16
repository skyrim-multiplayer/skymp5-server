#include "VirtualMachine.h"
#include <algorithm>
#include <stdexcept>

VirtualMachine::VirtualMachine(
  std::vector<std::shared_ptr<PexScript>> loadedScripts)
{
  this->allLoadedScripts = loadedScripts;
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
  auto loadedScript =
    std::find_if(this->allLoadedScripts.begin(), this->allLoadedScripts.end(),
                 [&className](const std::shared_ptr<PexScript>& pex) {
                   return !stricmp(pex->source.data(), className.data());
                 });
  if (loadedScript == this->allLoadedScripts.end())
    throw std::runtime_error(
      "Unable to register function in unexisting script");

  const std::string& classNameInNeededCase = (*loadedScript)->source;

  switch (type) {
    case FunctionType::GlobalFunction:

      nativeStaticFunctions[classNameInNeededCase][ToLower(functionName)] = fn;
      break;
    case FunctionType::Method:
      nativeFunctions[classNameInNeededCase][ToLower(functionName)] = fn;
      break;
  }
}

void VirtualMachine::AddObject(std::shared_ptr<IGameObject> self,
                               std::vector<std::string> scripts,
                               VarForBuildActivePex vars)
{
  std::vector<ActivePexInstance> scriptsForObject;

  for (auto& baseScript : allLoadedScripts) {
    for (auto& nameNeedScript : scripts) {
      if (baseScript->source == nameNeedScript) {

        ActivePexInstance scriptInstance(
          baseScript, vars, this, VarValue((IGameObject*)self.get()), "");
        scriptsForObject.push_back(scriptInstance);
      }
    }
  }

  gameObjects[self] = scriptsForObject;
}

void VirtualMachine::SendEvent(std::shared_ptr<IGameObject> self,
                               const char* eventName,
                               std::vector<VarValue>& arguments)
{
  for (auto& object : gameObjects) {
    if (object.first == self) {
      for (auto& scriptInstance : object.second) {
        auto name = scriptInstance.GetActiveStateName();

        auto fn = scriptInstance.GetFunctionByName(
          eventName, scriptInstance.GetActiveStateName());
        if (fn.valid) {
          scriptInstance.StartFunction(fn, arguments);
        }
      }
    }
  }
}

void VirtualMachine::SendEvent(ActivePexInstance* instance,
                               const char* eventName,
                               std::vector<VarValue>& arguments)
{

  auto fn =
    instance->GetFunctionByName(eventName, instance->GetActiveStateName());
  if (fn.valid) {
    instance->StartFunction(fn, arguments);
  }
}

VarValue VirtualMachine::CallMethod(ActivePexInstance* instance,
                                    IGameObject* self, const char* methodName,
                                    std::vector<VarValue>& arguments)
{
  NativeFunction f;
  auto it = instance;
  while (it && !f) {
    std::string className = it->sourcePex->source;
    f = nativeFunctions[className][ToLower(methodName)];
    if (!f) {
      it = it->parentInstance.get();
    }
  }
  if (f)
    return f(VarValue(self), arguments);

  FunctionInfo function;

  std::string nameGoToState = "GotoState";
  std::string nameGetState = "GetState";

  if (methodName == nameGoToState || methodName == nameGetState) {
    function = instance->GetFunctionByName(methodName, "");
  } else
    function =
      instance->GetFunctionByName(methodName, instance->GetActiveStateName());

  if (function.valid) {
    return instance->StartFunction(function, arguments);
  }
  throw std::runtime_error("Method not found - '" + std::string(methodName) +
                           "'");
}

VarValue VirtualMachine::CallStatic(std::string className,
                                    std::string functionName,
                                    std::vector<VarValue>& arguments)
{
  VarValue result = VarValue::None();
  FunctionInfo function;

  auto functionNameLower = ToLower(functionName);
  auto f = nativeStaticFunctions[className][functionNameLower]
    ? nativeStaticFunctions[className][functionNameLower]
    : nativeStaticFunctions[""][functionNameLower];

  if (f) {
    NativeFunction func = f;
    result = func(VarValue::None(), arguments);
    return result;
  }

  auto it =
    std::find_if(this->allLoadedScripts.begin(), this->allLoadedScripts.end(),
                 [&](std::shared_ptr<PexScript> a) -> bool {
                   return !stricmp(a->source.data(), className.data());
                 });

  if (it == this->allLoadedScripts.end())
    throw std::runtime_error("script not found - '" + className + "'");

  ActivePexInstance instance = ActivePexInstance(*it, VarForBuildActivePex({}),
                                                 this, VarValue::None(), "");

  function = instance.GetFunctionByName(functionName.c_str(),
                                        instance.GetActiveStateName());

  if (function.valid) {
    if (function.IsNative())
      throw std::runtime_error("Function not found - '" +
                               std::string(functionName) + "'");

    result = instance.StartFunction(function, arguments);
  }
  if (!function.valid)
    throw std::runtime_error("function is not valid");

  return result;
}

void VirtualMachine::RemoveObject(std::shared_ptr<IGameObject> self)
{
}