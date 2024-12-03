#include "Equipment.h"
#include "archives/SimdJsonInputArchive.h"
#include "archives/JsonOutputArchive.h"

bool Equipment::IsSpellEquipped(const uint32_t spellFormId) const
{
  return spellFormId == leftSpell || spellFormId == rightSpell ||
    spellFormId == voiceSpell || spellFormId == instantSpell;
}

nlohmann::json Equipment::ToJson() const
{
  JsonOutputArchive ar;
  const_cast<Equipment*>(this)->Serialize(ar);
  return std::move(ar.Release());
}

Equipment Equipment::FromJson(const simdjson::dom::element& element)
{
  // std::string minifiedDump = simdjson::minify(element);
  // nlohmann::json j = nlohmann::json::parse(minifiedDump);

  SimdJsonInputArchive ar(element);
  Equipment res;
  res.Serialize(ar);
  return res;
}
