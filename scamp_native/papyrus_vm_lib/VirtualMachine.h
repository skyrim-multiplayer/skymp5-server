#pragma once
#include "CIString.h"
#include "Structures.h"
#include <functional>
#include <map>
#include <set>

class VirtualMachine
{
public:
  using ExceptionHandler = std::function<void(std::string)>;

  struct ScriptInfo
  {
    std::string name;
    std::shared_ptr<IVariablesHolder> vars;
  };

  VirtualMachine(std::vector<PexScript::Lazy> loadedScripts);
  VirtualMachine(std::vector<PexScript::Ptr> loadedScripts);

  void SetExceptionHandler(const ExceptionHandler& handler);

  void AddObject(IGameObject::Ptr self,
                 const std::vector<ScriptInfo>& scripts);

  void RemoveObject(IGameObject::Ptr self); // ?

  void RegisterFunction(std::string className, std::string functionName,
                        FunctionType type, NativeFunction fn);

  void SendEvent(IGameObject::Ptr self, const char* eventName,
                 const std::vector<VarValue>& arguments);

  void SendEvent(ActivePexInstance* instance, const char* eventName,
                 const std::vector<VarValue>& arguments);

  VarValue CallMethod(IGameObject* self, const char* methodName,
                      std::vector<VarValue>& arguments);

  VarValue CallStatic(std::string className, std::string functionName,
                      std::vector<VarValue>& arguments);

  PexScript::Lazy GetPexByName(const std::string& name);

  ActivePexInstance::Ptr CreateActivePexInstance(
    const std::string& pexScriptName, VarValue activeInstanceOwner,
    const std::shared_ptr<IVariablesHolder>& mapForFillPropertys,
    std::string childrenName);

  bool IsNativeFunctionByNameExisted(const std::string& name) const;

  ExceptionHandler GetExceptionHandler() const;

private:
  using RegisteredGameOgject =
    std::pair<const IGameObject::Ptr, std::vector<ActivePexInstance>>;

  CIMap<PexScript::Lazy> allLoadedScripts;

  std::map<std::string, std::map<std::string, NativeFunction>> nativeFunctions,
    nativeStaticFunctions;

  std::map<std::string, ActivePexInstance::Ptr> instancesForStaticCalls;

  std::set<IGameObject::Ptr> gameObjectsHolder;

  ExceptionHandler handler;
};
