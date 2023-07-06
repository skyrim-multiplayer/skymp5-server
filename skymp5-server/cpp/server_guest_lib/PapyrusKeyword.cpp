#include "PapyrusUtility.h"

#include "EspmGameObject.h"
#include "PapyrusKeyword.h"
#include "WorldState.h"

VarValue PapyrusKeyword::GetKeyword(VarValue self,
                                    const std::vector<VarValue>& arguments)
{
  static espm::CompressedFieldsCache g_dummyCache;

  if (arguments.empty()) {
    spdlog::error("Form.HasKeyword - at least one argument expected");
    return VarValue();
  }

  std::string keywordName = arguments[0].ToString();

  return VarValue();
}