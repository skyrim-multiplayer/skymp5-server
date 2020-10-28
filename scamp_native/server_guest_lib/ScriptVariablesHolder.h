#pragma once
#include "CIString.h"
#include "Loader.h"
#include "VirtualMachine.h"

class EspmGameObject;

class ScriptVariablesHolder : public IVariablesHolder
{
public:
  ScriptVariablesHolder(const std::string& myScriptName,
                        espm::RecordHeader* recordWithScripts,
                        const espm::CombineBrowser* browser);

  VarValue* GetVariableByName(const char* name, const PexScript& pex) override;

private:
  void FillProperties(const espm::Script& script);
  void FillNormalVariables(const PexScript& pex);
  void FillState(const PexScript& pex);
  espm::Script GetScript();

  using VarsMap = CIMap<VarValue>;
  using EspmObjectsHolder =
    std::map<uint32_t, std::shared_ptr<EspmGameObject>>;
  using PropStringValues = std::map<std::string, std::shared_ptr<std::string>>;
  struct ScriptsCache
  {
    EspmObjectsHolder espmObjectsHolder;
    PropStringValues propStringValues;
  };

  static VarValue CastPrimitivePropertyValue(
    const espm::CombineBrowser& br, ScriptsCache& st,
    const espm::Property::Value& propValue, espm::PropertyType type);

  static void CastProperty(const espm::CombineBrowser& br,
                           const espm::Property& prop, VarValue* out,
                           ScriptsCache* scriptsCache);
  static espm::PropertyType ScriptVariablesHolder::GetElementType(
    espm::PropertyType arrayType);

  espm::RecordHeader* const recordWithScripts;
  const std::string myScriptName;
  const espm::CombineBrowser* const browser;
  std::unique_ptr<VarsMap> vars;
  std::unique_ptr<ScriptsCache> scriptsCache;
};