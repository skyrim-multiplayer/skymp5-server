#include "SpSnippetFunctionGen.h"

#include "EspmGameObject.h"
#include "MpFormGameObject.h"
#include "SpSnippet.h"
#include "WorldState.h"
#include <nlohmann/json.hpp>
#include <sstream>

uint32_t SpSnippetFunctionGen::GetFormId(VarValue varValue)
{
  if (auto form = GetFormPtr<MpForm>(varValue))
    return form->GetFormId();
  if (auto record = GetRecordPtr(varValue); record.rec)
    return record.ToGlobalId(record.rec->GetId());
  std::stringstream ss;
  ss << varValue << " is not a valid Papyrus object";
  throw std::runtime_error(ss.str());
}

std::string SpSnippetFunctionGen::SerializeArguments(
  const std::vector<VarValue>& arguments)
{
  std::stringstream ss;
  ss << '[';
  for (auto& arg : arguments) {
    if (&arg != &arguments[0])
      ss << ", ";

    switch (arg.GetType()) {
      case VarValue::kType_String:
        ss << nlohmann::json(static_cast<const char*>(arg)).dump();
        break;
      case VarValue::kType_Bool:
        ss << (static_cast<bool>(arg) ? "true" : "false");
        break;
      case VarValue::kType_Integer:
        ss << static_cast<int>(arg);
        break;
      case VarValue::kType_Float:
        ss << static_cast<float>(arg);
        break;
      default: {
        std::stringstream err;
        err << "Unable to serialize VarValue " << arg
            << " due to unsupported type (" << static_cast<int>(arg.GetType())
            << ")";
        throw std::runtime_error(err.str());
      }
    }
  }
  ss << ']';
  return ss.str();
}