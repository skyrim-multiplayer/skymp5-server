#include "InventoryApi.h"
#include "NullPointerException.h"

namespace {

JsValue ToJsValue(RE::ExtraHealth& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("health", extra.health);
  res.SetProperty("type", "Health");
  return res;
}

JsValue ToJsValue(RE::ExtraCount& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("count", static_cast<int>(extra.count));
  res.SetProperty("type", "Count");
  return res;
}

JsValue ToJsValue(RE::ExtraEnchantment& extra)
{
  auto res = JsValue::Object();
  res.SetProperty(
    "enchantmentId",
    static_cast<double>(extra.enchantment ? extra.enchantment->formID : 0));
  res.SetProperty("maxCharge", extra.charge);
  res.SetProperty("type", "Enchantment");
  return res;
}

JsValue ToJsValue(RE::ExtraCharge& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("charge", extra.charge);
  res.SetProperty("type", "Charge");
  return res;
}

JsValue ToJsValue(RE::ExtraTextDisplayData& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("name", extra.displayName.c_str());
  res.SetProperty("type", "TextDisplayData");
  return res;
}

JsValue ToJsValue(RE::ExtraSoul& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("soul", static_cast<int>(extra.GetType()));
  res.SetProperty("type", "Soul");
  return res;
}

JsValue ToJsValue(RE::ExtraPoison& extra)
{
  auto res = JsValue::Object();
  res.SetProperty(
    "poisonId", static_cast<double>(extra.poison ? extra.poison->formID : 0));
  res.SetProperty(
    "count",
    static_cast<double>(reinterpret_cast<RE::ExtraPoison&>(extra).count));
  res.SetProperty("type", "Poison");
  return res;
}

JsValue ToJsValue(RE::ExtraWorn& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("type", "Worn");
  return res;
}

JsValue ToJsValue(RE::ExtraWornLeft& extra)
{
  auto res = JsValue::Object();
  res.SetProperty("type", "WornLeft");
  return res;
}

JsValue ToJsValue(RE::BSExtraData* extraData)
{
  /**
   * ExtraDataList has GetByType<T> which can be used to avoid casts *
   */
  if (extraData) {
    switch (extraData->GetType()) {
      case RE::ExtraDataType::kHealth:
        return ToJsValue(*reinterpret_cast<RE::ExtraHealth*>(extraData));
      case RE::ExtraDataType::kCount:
        return ToJsValue(*reinterpret_cast<RE::ExtraCount*>(extraData));
      case RE::ExtraDataType::kEnchantment:
        return ToJsValue(*reinterpret_cast<RE::ExtraEnchantment*>(extraData));
      case RE::ExtraDataType::kCharge:
        return ToJsValue(*reinterpret_cast<RE::ExtraCharge*>(extraData));
      case RE::ExtraDataType::kTextDisplayData:
        return ToJsValue(
          *reinterpret_cast<RE::ExtraTextDisplayData*>(extraData));
      case RE::ExtraDataType::kSoul:
        return ToJsValue(*reinterpret_cast<RE::ExtraSoul*>(extraData));
      case RE::ExtraDataType::kPoison:
        return ToJsValue(*reinterpret_cast<RE::ExtraPoison*>(extraData));
      case RE::ExtraDataType::kWorn:
        return ToJsValue(*reinterpret_cast<RE::ExtraWorn*>(extraData));
      case RE::ExtraDataType::kWornLeft:
        return ToJsValue(*reinterpret_cast<RE::ExtraWornLeft*>(extraData));
    }
  }
  return JsValue::Undefined();
}

JsValue ToJsValue(RE::ExtraDataList* extraList)
{
  if (!extraList) {
    return JsValue::Null();
  }

  std::vector<JsValue> jData;

  RE::BSReadLockGuard lock(extraList->_lock);

  for (auto it = extraList->begin(); it != extraList->end(); ++it) {
    auto extra = ToJsValue(&(*it));
    if (extra.GetType() != JsValue::Type::Undefined) {
      jData.push_back(extra);
    }
  }

  return jData;
}

JsValue ToJsValue(RE::BSSimpleList<RE::ExtraDataList*>* extendDataList)
{
  if (!extendDataList)
    return JsValue::Null();

  auto first = extendDataList->begin();
  auto last = extendDataList->end();

  std::vector<JsValue> arr;
  for (auto it = first; it != last; ++it) {
    arr.push_back(ToJsValue(*it));
  }

  return arr;
}
}

JsValue InventoryApi::GetExtraContainerChanges(const JsFunctionArguments& args)
{
  auto objectReferenceId = static_cast<double>(args[1]);

  auto refr = RE::TESForm::LookupByID<RE::TESObjectREFR>(objectReferenceId);
  if (!refr ||
      (refr->formType != RE::FormType::ActorCharacter &&
       refr->formType != RE::FormType::Reference)) {
    return JsValue::Null();
  }

  auto cntChanges = refr->extraList.GetByType<RE::ExtraContainerChanges>();

  if (!cntChanges || !cntChanges->changes || !cntChanges->changes->entryList)
    return JsValue::Null();

  auto first = cntChanges->changes->entryList->begin();
  auto last = cntChanges->changes->entryList->end();

  std::vector<JsValue> res;

  for (auto it = first; it != last; ++it) {
    const auto baseID = (*it)->object ? (*it)->object->formID : 0;

    auto jEntry = JsValue::Object();
    jEntry.SetProperty("countDelta", (*it)->countDelta);
    jEntry.SetProperty("baseId", JsValue::Double(baseID));
    jEntry.SetProperty("extendDataList", ToJsValue((*it)->extraLists));

    res.push_back(jEntry);
  }

  return res;
}

JsValue InventoryApi::GetContainer(const JsFunctionArguments& args)
{
  auto formID = static_cast<double>(args[1]);
  // why not cast to Container straight away?
  auto form = RE::TESForm::LookupByID<RE::TESObjectREFR>(formID);

  if (!form)
    return JsValue::Array(0);

  auto pContainer = form->As<RE::TESContainer>();

  if (!pContainer)
    return JsValue::Array(0);

  auto res = JsValue::Array(pContainer->numContainerObjects);

  for (uint32_t i = 0; i < pContainer->numContainerObjects; ++i) {
    auto e = pContainer->containerObjects[i];
    auto jEntry = JsValue::Object();
    jEntry.SetProperty("count", JsValue::Double(e->count));
    jEntry.SetProperty("baseId", JsValue::Double(e->obj ? e->obj->formID : 0));
    res.SetProperty(JsValue::Int(i), jEntry);
  }

  return res;
}

JsValue SetInventory(const JsFunctionArguments& args)
{
  double formId = static_cast<double>(args[1]);
  RE::Actor* pActor = RE::TESForm::LookupByID<RE::Actor>(formId);
  if (!pActor) {
    throw NullPointerException("pActor");
  }
  for (auto& [pBoundObject, countAndEntryData] : pActor->GetInventory()) {
    pActor->RemoveItem(pBoundObject, countAndEntryData.first,
                       RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
  };
  const JsValue& entries = args[2].GetProperty("entries");
  for (uint32_t i = 0; i < entries.GetArrayBufferLength(); ++i) {
    const JsValue& entry = entries.GetProperty(JsValue::Int(i));
    const double baseId = static_cast<double>(entry.GetProperty("baseId"));
    RE::TESBoundObject* pBoundObject =
      RE::TESForm::LookupByID<RE::TESBoundObject>(baseId);
    if (!pBoundObject) {
      throw NullPointerException("pBoundObject");
    }
    const uint32_t count = static_cast<int>(entry.GetProperty("count"));
    const bool worn =
      entry.GetProperty("worn").GetType() != JsValue::Type::Undefined
      ? static_cast<bool>(entry.GetProperty("worn"))
      : false;
    pActor->AddObjectToContainer(pBoundObject, nullptr, count, nullptr);
    if (worn) {
      RE::ActorEquipManager* equipManager =
        RE::ActorEquipManager::GetSingleton();
      equipManager->EquipObject(pActor, pBoundObject, nullptr, 1, nullptr,
                                false, true);
    }
  }
  return JsValue::Undefined();
}

void Register(JsValue& exports)
{
  exports.SetProperty("getExtraContainerChanges",
                      JsValue::Function(GetExtraContainerChanges));
  exports.SetProperty("getContainer", JsValue::Function(GetContainer));
  exports.SetProperty("setInventory", JsValue::Function(SetInventory));
}
