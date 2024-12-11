#include "ActionListener.h"
#include "AnimationSystem.h"
#include "ConsoleCommands.h"
#include "CropRegeneration.h"
#include "DummyMessageOutput.h"
#include "Exceptions.h"
#include "FindRecipe.h"
#include "GetBaseActorValues.h"
#include "HitData.h"
#include "MathUtils.h"
#include "MovementValidation.h"
#include "MpObjectReference.h"
#include "MsgType.h"
#include "UserMessageOutput.h"
#include "WorldState.h"
#include "gamemode_events/CraftEvent.h"
#include "gamemode_events/CustomEvent.h"
#include "gamemode_events/EatItemEvent.h"
#include "gamemode_events/UpdateAppearanceAttemptEvent.h"
#include "script_objects/EspmGameObject.h"
#include "viet/Overloaded.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <unordered_set>

#include "UpdateEquipmentMessage.h"

MpActor* ActionListener::SendToNeighbours(
  uint32_t idx, const simdjson::dom::element& jMessage,
  Networking::UserId userId, Networking::PacketData data, size_t length,
  bool reliable)
{
  MpActor* myActor = partOne.serverState.ActorByUser(userId);
  // The old behavior is doing nothing in that case. This is covered by tests
  if (!myActor) {
    spdlog::warn("SendToNeighbours - No actor assigned to user");
    return nullptr;
  }

  MpForm* form = partOne.worldState.LookupFormByIdx(idx);
  MpActor* actor = form ? form->AsActor() : nullptr;
  if (!actor) {
    spdlog::error("SendToNeighbours - Target actor doesn't exist");
    return nullptr;
  }

  if (idx != myActor->GetIdx()) {
    // Possible fix for "players link to each other" bug
    // See also PartOne::SetUserActor
    Networking::UserId actorsOwningUserId =
      partOne.serverState.UserByActor(actor);
    if (actorsOwningUserId != Networking::InvalidUserId) {
      spdlog::error("SendToNeighbours - No permission to update actor {:x} "
                    "(already owned by user {})",
                    actor->GetFormId(), actorsOwningUserId);
      partOne.SendHostStop(userId, *actor);

      partOne.worldState.hosters.erase(actor->GetFormId());
      return nullptr;
    }

    auto it = partOne.worldState.hosters.find(actor->GetFormId());
    if (it == partOne.worldState.hosters.end() ||
        it->second != myActor->GetFormId()) {
      if (idx == 0) {
        spdlog::warn("SendToNeighbours - idx=0, <Message>::ReadJson or "
                     "similar is probably incorrect");
      }
      spdlog::error(
        "SendToNeighbours - No permission to update actor {:x} (not a hoster)",
        actor->GetFormId());
      partOne.SendHostStop(userId, *actor);
      return nullptr;
    }
  }

  for (auto listener : actor->GetActorListeners()) {
    auto targetuserId = partOne.serverState.UserByActor(listener);
    if (targetuserId != Networking::InvalidUserId) {
      partOne.GetSendTarget().Send(targetuserId, data, length, reliable);
    }
  }

  return actor;
}

MpActor* ActionListener::SendToNeighbours(uint32_t idx,
                                          const RawMessageData& rawMsgData,
                                          bool reliable)
{
  return SendToNeighbours(idx, rawMsgData.parsed, rawMsgData.userId,
                          rawMsgData.unparsed, rawMsgData.unparsedLength,
                          reliable);
}

void ActionListener::OnCustomPacket(const RawMessageData& rawMsgData,
                                    simdjson::dom::element& content)
{
  for (auto& listener : partOne.GetListeners())
    listener->OnCustomPacket(rawMsgData.userId, content);
}

void ActionListener::OnUpdateMovement(const RawMessageData& rawMsgData,
                                      uint32_t idx, const NiPoint3& pos,
                                      const NiPoint3& rot, bool isInJumpState,
                                      bool isWeapDrawn, bool isBlocking,
                                      uint32_t worldOrCell,
                                      const std::string& runMode)
{
  auto actor = SendToNeighbours(idx, rawMsgData);
  if (actor) {
    DummyMessageOutput msgOutputDummy;
    UserMessageOutput msgOutput(partOne.GetSendTarget(), rawMsgData.userId);

    bool isMe = partOne.serverState.ActorByUser(rawMsgData.userId) == actor;

    bool teleportFlag = actor->GetTeleportFlag();
    actor->SetTeleportFlag(false);

    static const NiPoint3 reallyWrongPos = {
      std::numeric_limits<float>::infinity(),
      std::numeric_limits<float>::infinity(),
      std::numeric_limits<float>::infinity()
    };

    auto& espmFiles = actor->GetParent()->espmFiles;

    const auto& currentPos = actor->GetPos();
    const auto& currentRot = actor->GetAngle();
    const auto& currentCellOrWorld = actor->GetCellOrWorld();

    if (!MovementValidation::Validate(
          currentPos, currentRot, currentCellOrWorld,
          teleportFlag ? reallyWrongPos : pos,
          FormDesc::FromFormId(worldOrCell, espmFiles),
          isMe ? static_cast<IMessageOutput&>(msgOutput)
               : static_cast<IMessageOutput&>(msgOutputDummy),
          espmFiles)) {
      return;
    }

    if (!isBlocking) {
      actor->IncreaseBlockCount();
    } else {
      actor->ResetBlockCount();
    }

    actor->SetPos(pos, SetPosMode::CalledByUpdateMovement);
    actor->SetAngle(rot, SetAngleMode::CalledByUpdateMovement);
    actor->SetAnimationVariableBool(
      AnimationVariableBool::kVariable_bInJumpState, isInJumpState);
    actor->SetAnimationVariableBool(
      AnimationVariableBool::kVariable__skymp_isWeapDrawn, isWeapDrawn);
    actor->SetAnimationVariableBool(
      AnimationVariableBool::kVariable_IsBlocking, isBlocking);

    if (actor->GetBlockCount() == 5) {
      actor->SetIsBlockActive(false);
      actor->ResetBlockCount();
    }

    if (runMode != "Standing") {
      // otherwise, people will slide in anims after quitting furniture
      actor->SetLastAnimEvent(std::nullopt);
    }

    if (partOne.worldState.lastMovUpdateByIdx.size() <= idx) {
      auto newSize = static_cast<size_t>(idx) + 1;
      partOne.worldState.lastMovUpdateByIdx.resize(newSize);
    }
    partOne.worldState.lastMovUpdateByIdx[idx] =
      std::chrono::system_clock::now();
  }
}

void ActionListener::OnUpdateAnimation(const RawMessageData& rawMsgData,
                                       uint32_t idx,
                                       const AnimationData& animationData)
{
  MpActor* myActor = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!myActor) {
    return;
  }

  auto targetActor = SendToNeighbours(idx, rawMsgData);

  if (!targetActor) {
    return;
  }

  // Only process animation system and set last anim event for player's actor
  if (targetActor != myActor) {
    return;
  }

  partOne.animationSystem.Process(targetActor, animationData);
  targetActor->SetLastAnimEvent(animationData);
}

void ActionListener::OnUpdateAppearance(const RawMessageData& rawMsgData,
                                        uint32_t idx,
                                        const Appearance& appearance)
{ // TODO: validate

  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!actor) {
    return;
  }

  const bool isAllowed = actor->IsRaceMenuOpen();

  if (isAllowed) {
    actor->SetRaceMenuOpen(false);
    actor->SetAppearance(&appearance);
    SendToNeighbours(idx, rawMsgData, true);
  }

  UpdateAppearanceAttemptEvent updateAppearanceAttemptEvent(actor, appearance,
                                                            isAllowed);
  updateAppearanceAttemptEvent.Fire(actor->GetParent());
}

void ActionListener::OnUpdateEquipment(
  const RawMessageData& rawMsgData, const uint32_t idx, const Equipment& data,
  const Inventory& equipmentInv, const uint32_t leftSpell,
  const uint32_t rightSpell, const uint32_t voiceSpell,
  const uint32_t instantSpell)
{
  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);

  if (!actor) {
    return;
  }

  if (leftSpell > 0 && !actor->IsSpellLearned(leftSpell)) {
    spdlog::debug(
      "OnUpdateEquipment result false. Spell with id ({}) not learned",
      leftSpell);
    return;
  }

  if (rightSpell > 0 && !actor->IsSpellLearned(rightSpell)) {
    spdlog::debug(
      "OnUpdateEquipment result false. Spell with id ({}) not learned",
      leftSpell);
    return;
  }

  if (voiceSpell > 0 && !actor->IsSpellLearned(voiceSpell)) {
    spdlog::debug(
      "OnUpdateEquipment result false. Spell with id ({}) not learned",
      leftSpell);
    return;
  }

  if (instantSpell > 0 && !actor->IsSpellLearned(instantSpell)) {
    spdlog::debug(
      "OnUpdateEquipment result false. Spell with id ({}) not learned",
      leftSpell);
    return;
  }

  const auto& inventory = actor->GetInventory();

  for (auto& entry : equipmentInv.entries) {
    if (!inventory.HasItem(entry.baseId)) {
      spdlog::debug(
        "OnUpdateEquipment result false. The inventory does not contain item "
        "with id {:x}",
        entry.baseId);
      return;
    }
  }

  SendToNeighbours(idx, rawMsgData, true);
  actor->SetEquipment(data.ToJson().dump());
}

void ActionListener::OnActivate(const RawMessageData& rawMsgData,
                                uint32_t caster, uint32_t target,
                                bool isSecondActivation)
{
  if (!partOne.HasEspm())
    throw std::runtime_error("No loaded esm or esp files are found");

  const auto ac = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!ac)
    throw std::runtime_error("Can't do this without Actor attached");

  auto it = partOne.worldState.hosters.find(caster);
  auto hosterId = it == partOne.worldState.hosters.end() ? 0 : it->second;

  if (caster != 0x14) {
    if (hosterId != ac->GetFormId()) {
      std::stringstream ss;
      ss << std::hex << "Bad hoster is attached to caster 0x" << caster
         << ", expected 0x" << ac->GetFormId() << ", but found 0x" << hosterId;
      throw std::runtime_error(ss.str());
    }
  }

  auto targetPtr = std::dynamic_pointer_cast<MpObjectReference>(
    partOne.worldState.LookupFormById(target));
  if (!targetPtr)
    return;

  constexpr bool kDefaultProcessingOnlyFalse = false;
  targetPtr->Activate(
    caster == 0x14 ? *ac
                   : partOne.worldState.GetFormAt<MpObjectReference>(caster),
    kDefaultProcessingOnlyFalse, isSecondActivation);
  if (hosterId) {
    auto actor = std::dynamic_pointer_cast<MpActor>(
      partOne.worldState.LookupFormById(caster));
    if (actor) {
      actor->EquipBestWeapon();
    }
  }
}

void ActionListener::OnPutItem(const RawMessageData& rawMsgData,
                               uint32_t target, const Inventory::Entry& entry)
{
  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);
  auto& ref = partOne.worldState.GetFormAt<MpObjectReference>(target);

  if (!actor)
    return;

  auto worldState = actor->GetParent();
  if (!worldState) {
    return spdlog::error("No WorldState attached");
  }

  if (worldState->HasKeyword(entry.baseId, "SweetCantDrop")) {
    return spdlog::error("Attempt to put SweetCantDrop item {:x}",
                         actor->GetFormId());
  }

  ref.PutItem(*actor, entry);
}

void ActionListener::OnTakeItem(const RawMessageData& rawMsgData,
                                uint32_t target, const Inventory::Entry& entry)
{
  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);
  auto& ref = partOne.worldState.GetFormAt<MpObjectReference>(target);

  if (!actor)
    return;

  auto worldState = actor->GetParent();
  if (!worldState) {
    return spdlog::error("No WorldState attached");
  }

  if (worldState->HasKeyword(entry.baseId, "SweetCantDrop")) {
    return spdlog::error("Attempt to take SweetCantDrop item {:x}",
                         actor->GetFormId());
  }

  ref.TakeItem(*actor, entry);
}

void ActionListener::OnDropItem(const RawMessageData& rawMsgData,
                                uint32_t baseId, const Inventory::Entry& entry)
{
  MpActor* ac = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!ac) {
    return spdlog::error("Unable to drop an item from user with id: {}.",
                         rawMsgData.userId);
  }

  auto worldState = ac->GetParent();
  if (!worldState) {
    return spdlog::error("No WorldState attached");
  }

  if (worldState->HasKeyword(entry.baseId, "SweetCantDrop")) {
    return spdlog::error("Attempt to drop SweetCantDrop item {:x}",
                         ac->GetFormId());
  }

  ac->DropItem(baseId, entry);
}

void ActionListener::OnPlayerBowShot(const RawMessageData& rawMsgData,
                                     uint32_t weaponId, uint32_t ammoId,
                                     float power, bool isSunGazing)
{
  MpActor* ac = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!ac) {
    return spdlog::error("Unable to shot from user with id: {}.",
                         rawMsgData.userId);
  }

  auto worldState = ac->GetParent();

  if (!worldState) {
    return;
  }

  auto ammoLookupRes = worldState->GetEspm().GetBrowser().LookupById(ammoId);

  if (!ammoLookupRes.rec) {
    return spdlog::error("ActionListener::OnPlayerBowShot {:x} - unable to "
                         "find espm record for {:x}",
                         ac->GetFormId(), ammoId);
  }

  if (ammoLookupRes.rec->GetType().ToString() != "AMMO") {
    return spdlog::error(
      "ActionListener::OnPlayerBowShot {:x} - unable to shot not an ammo {:x}",
      ac->GetFormId(), ammoId);
  }

  ac->RemoveItem(ammoId, 1, nullptr);
}

namespace {

VarValue VarValueFromSpSnippetReturnValue(
  const std::optional<std::variant<bool, double, std::string>>& returnValue)
{
  static const auto kKey = JsonPointer("returnValue");

  if (!returnValue) {
    return VarValue::None();
  }

  return std::visit(
    Viet::Overloaded{
      [&](bool v) { return VarValue(v); },
      [&](double v) {
        // TODO: consider removing std::floor and logs after careful test
        // because Papyrus VM should support mixed arithmetics, so we can
        // always pass double
        auto rounded = static_cast<int32_t>(std::floor(v));
        if (std::abs(std::floor(v) - v) >
            std::numeric_limits<double>::epsilon()) {
          spdlog::error(
            "VarValueFromSpSnippetReturnValue - Floating point values are not "
            "yet supported, rounding down ({} -> {})",
            v, rounded);
        }
        return VarValue(rounded);
      },
      [&](const std::string& v) { return VarValue(v); } } *
    returnValue);
}

}
void ActionListener::OnFinishSpSnippet(
  const RawMessageData& rawMsgData, uint32_t snippetIdx,
  const std::optional<std::variant<bool, double, std::string>>& returnValue)
{
  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!actor)
    throw std::runtime_error(
      "Unable to finish SpSnippet: No Actor found for user " +
      std::to_string(rawMsgData.userId));

  actor->ResolveSnippet(snippetIdx,
                        VarValueFromSpSnippetReturnValue(returnValue));
}

void ActionListener::OnEquip(const RawMessageData& rawMsgData, uint32_t baseId)
{
  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!actor) {
    throw std::runtime_error(
      "Unable to finish SpSnippet: No Actor found for user " +
      std::to_string(rawMsgData.userId));
  }

  std::ignore = actor->OnEquip(baseId);
}

void ActionListener::OnConsoleCommand(
  const RawMessageData& rawMsgData, const std::string& consoleCommandName,
  const std::vector<ConsoleCommands::Argument>& args)
{
  MpActor* me = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (me)
    ConsoleCommands::Execute(*me, consoleCommandName, args);
}

void UseCraftRecipe(MpActor* me, const espm::COBJ* recipeUsed,
                    espm::CompressedFieldsCache& cache,
                    const espm::CombineBrowser& br, int espmIdx)
{
  auto recipeData = recipeUsed->GetData(cache);
  auto mapping = br.GetCombMapping(espmIdx);

  spdlog::info("Using craft recipe with EDID {} from espm file with index {}",
               recipeUsed->GetEditorId(cache), espmIdx);

  for (auto& condition : recipeData.conditions) {
    // impl race, item, perk? checks
  }

  std::vector<Inventory::Entry> entries;
  for (auto& entry : recipeData.inputObjects) {
    auto formId = espm::utils::GetMappedId(entry.formId, *mapping);
    entries.push_back({ formId, entry.count });
  }

  auto outputFormId =
    espm::utils::GetMappedId(recipeData.outputObjectFormId, *mapping);

  if (spdlog::should_log(spdlog::level::info)) {
    std::string s = fmt::format("User formId={:#x} crafted", me->GetFormId());
    for (const auto& entry : entries) {
      s += fmt::format(" -{:#x} x{}", entry.baseId, entry.count);
    }
    s += fmt::format(" +{:#x} x{}", outputFormId, recipeData.outputCount);
    spdlog::info("{}", s);
  }

  auto recipeId = espm::utils::GetMappedId(recipeUsed->GetId(), *mapping);

  CraftEvent craftEvent(me, outputFormId, recipeData.outputCount, recipeId,
                        entries);

  craftEvent.Fire(me->GetParent());
}

void ActionListener::OnCraftItem(const RawMessageData& rawMsgData,
                                 const Inventory& inputObjects,
                                 uint32_t workbenchId, uint32_t resultObjectId)
{
  auto& workbench =
    partOne.worldState.GetFormAt<MpObjectReference>(workbenchId);

  auto& br = partOne.worldState.GetEspm().GetBrowser();
  auto& cache = partOne.worldState.GetEspmCache();
  auto base = br.LookupById(workbench.GetBaseId());

  spdlog::info("User {} tries to craft {:#x} on workbench {:#x}",
               rawMsgData.userId, resultObjectId, workbenchId);

  bool isFurnitureOrActivator =
    base.rec->GetType() == "FURN" || base.rec->GetType() == "ACTI";
  if (!isFurnitureOrActivator) {
    return spdlog::error("Unable to use {} as workbench",
                         base.rec->GetType().ToString());
  }

  int espmIdx = 0;
  auto recipeUsed = FindRecipe(br, inputObjects, resultObjectId, &espmIdx);

  if (!recipeUsed) {
    return spdlog::error(
      "Recipe not found: inputObjects={}, workbenchId={:#x}, "
      "resultObjectId={:#x}",
      inputObjects.ToJson().dump(), workbenchId, resultObjectId);
  }

  MpActor* me = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!me) {
    return spdlog::error("Unable to craft without Actor attached");
  }

  UseCraftRecipe(me, recipeUsed, cache, br, espmIdx);
}

void ActionListener::OnHostAttempt(const RawMessageData& rawMsgData,
                                   uint32_t remoteId)
{
  MpActor* me = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!me) {
    throw std::runtime_error("Unable to host without actor attached");
  }

  auto& remote = partOne.worldState.GetFormAt<MpObjectReference>(remoteId);

  auto user = partOne.serverState.UserByActor(remote.AsActor());
  if (user != Networking::InvalidUserId) {
    return;
  }

  auto& hoster = partOne.worldState.hosters[remoteId];
  const uint32_t prevHoster = hoster;

  auto remoteIdx = remote.GetIdx();

  std::optional<std::chrono::system_clock::time_point> lastRemoteUpdate;
  if (partOne.worldState.lastMovUpdateByIdx.size() > remoteIdx) {
    lastRemoteUpdate = partOne.worldState.lastMovUpdateByIdx[remoteIdx];
  }

  const auto hostResetTimeout = std::chrono::seconds(2);

  if (hoster == 0 || !lastRemoteUpdate ||
      std::chrono::system_clock::now() - *lastRemoteUpdate >
        hostResetTimeout) {
    partOne.GetLogger().info("Hoster changed from {0:x} to {0:x}", prevHoster,
                             me->GetFormId());
    hoster = me->GetFormId();
    remote.UpdateHoster(hoster);

    // Prevents too fast host switch
    partOne.worldState.lastMovUpdateByIdx[remoteIdx] =
      std::chrono::system_clock::now();

    auto remoteAsActor = remote.AsActor();

    if (remoteAsActor) {
      remoteAsActor->EquipBestWeapon();
    }

    uint64_t longFormId = remote.GetFormId();
    if (remoteAsActor && longFormId < 0xff000000) {
      longFormId += 0x100000000;
    }

    Networking::SendFormatted(&partOne.GetSendTarget(), rawMsgData.userId,
                              R"({ "type": "hostStart", "target": %llu })",
                              longFormId);

    // Otherwise, health percentage would remain unsynced until someone hits
    // npc
    auto formId = remote.GetFormId();
    partOne.worldState.SetTimer(std::chrono::seconds(1))
      .Then([this, formId](Viet::Void) {
        // Check if form is still here
        auto& remote = partOne.worldState.GetFormAt<MpActor>(formId);

        auto changeForm = remote.GetChangeForm();

        ChangeValuesMessage msg;
        msg.idx = remote.GetIdx();
        msg.data.health = changeForm.actorValues.healthPercentage;
        msg.data.magicka = changeForm.actorValues.magickaPercentage;
        msg.data.stamina = changeForm.actorValues.staminaPercentage;
        remote.SendToUser(msg, true); // in fact sends to hoster
      });

    auto& prevHosterForm = partOne.worldState.LookupFormById(prevHoster);
    if (MpActor* prevHosterActor =
          prevHosterForm ? prevHosterForm->AsActor() : nullptr) {
      auto prevHosterUser = partOne.serverState.UserByActor(prevHosterActor);
      if (prevHosterUser != Networking::InvalidUserId &&
          prevHosterUser != rawMsgData.userId) {
        Networking::SendFormatted(&partOne.GetSendTarget(), prevHosterUser,
                                  R"({ "type": "hostStop", "target": %llu })",
                                  longFormId);
      }
    }
  }
}

void ActionListener::OnCustomEvent(
  const RawMessageData& rawMsgData, const char* eventName,
  const std::vector<std::string>& argsJsonDumps)
{
  auto ac = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!ac) {
    return;
  }

  if (eventName[0] != '_') {
    return;
  }

  nlohmann::json jsonArray = nlohmann::json::array();

  for (auto& arg : argsJsonDumps) {
    jsonArray.push_back(nlohmann::json::parse(arg));
  }

  const std::string jsonArrayDump = jsonArray.dump();

  for (auto& listener : partOne.GetListeners()) {
    CustomEvent customEvent(ac->GetFormId(), eventName, jsonArrayDump);
    listener->OnMpApiEvent(customEvent);
  }
}

void ActionListener::OnChangeValues(const RawMessageData& rawMsgData,
                                    const ActorValues& newActorValues)
{
  MpActor* actor = partOne.serverState.ActorByUser(rawMsgData.userId);
  if (!actor) {
    throw std::runtime_error("Unable to change values without Actor attached");
  }

  if (actor->ShouldSkipRestoration()) {
    return;
  }

  auto now = std::chrono::steady_clock::now();

  float timeAfterRegeneration = CropPeriodAfterLastRegen(
    actor->GetDurationOfAttributesPercentagesUpdate(now).count());

  ActorValues currentActorValues = actor->GetActorValues();
  float health = newActorValues.healthPercentage;
  float magicka = newActorValues.magickaPercentage;
  float stamina = newActorValues.staminaPercentage;

  if (newActorValues.healthPercentage != currentActorValues.healthPercentage) {
    currentActorValues.healthPercentage =
      CropHealthRegeneration(health, timeAfterRegeneration, actor);
  }
  if (newActorValues.magickaPercentage !=
      currentActorValues.magickaPercentage) {
    currentActorValues.magickaPercentage =
      CropMagickaRegeneration(magicka, timeAfterRegeneration, actor);
  }
  if (newActorValues.staminaPercentage !=
      currentActorValues.staminaPercentage) {
    currentActorValues.staminaPercentage =
      CropStaminaRegeneration(stamina, timeAfterRegeneration, actor);
  }

  if (!MathUtils::IsNearlyEqual(currentActorValues.healthPercentage,
                                newActorValues.healthPercentage) ||
      !MathUtils::IsNearlyEqual(currentActorValues.magickaPercentage,
                                newActorValues.magickaPercentage) ||
      !MathUtils::IsNearlyEqual(currentActorValues.staminaPercentage,
                                newActorValues.staminaPercentage)) {
    actor->NetSendChangeValues(currentActorValues);
  }
  actor->SetPercentages(currentActorValues);
}

namespace {

bool IsUnarmedAttack(const uint32_t sourceFormId)
{
  return sourceFormId == 0x1f4;
}

float CalculateCurrentHealthPercentage(const MpActor& actor, float damage,
                                       float healthPercentage,
                                       float* outBaseHealth)
{
  const uint32_t baseId = actor.GetBaseId();
  const uint32_t raceId = actor.GetRaceId();
  WorldState* espmProvider = actor.GetParent();

  const float baseHealth =
    GetBaseActorValues(espmProvider, baseId, raceId, actor.GetTemplateChain())
      .health;

  if (outBaseHealth) {
    *outBaseHealth = baseHealth;
  }

  const float damagePercentage = damage / baseHealth;
  const float currentHealthPercentage = healthPercentage - damagePercentage;

  /// TODO add check for nan and inf!
  return currentHealthPercentage <= 0.f ? 0.f : currentHealthPercentage;
}

float GetReach(const MpActor& actor, const uint32_t source,
               float reachHotfixMult)
{
  auto espmProvider = actor.GetParent();
  if (IsUnarmedAttack(source)) {
    uint32_t raceId = actor.GetRaceId();
    return reachHotfixMult *
      espm::GetData<espm::RACE>(raceId, espmProvider).unarmedReach;
  }
  auto weapDNAM = espm::GetData<espm::WEAP>(source, espmProvider).weapDNAM;
  float fCombatDistance =
    espm::GetData<espm::GMST>(espm::GMST::kFCombatDistance, espmProvider)
      .value;
  float weaponReach = weapDNAM ? weapDNAM->reach : 0;
  return reachHotfixMult * weaponReach * fCombatDistance;
}

NiPoint3 RotateZ(const NiPoint3& point, float angle)
{
  static const float kPi = std::acos(-1.f);
  static const float kAngleToRadians = kPi / 180.f;
  float cos = std::cos(angle * kAngleToRadians);
  float sin = std::sin(angle * kAngleToRadians);

  return { point.x * cos - point.y * sin, point.x * sin + point.y * cos,
           point.z };
}

float GetSqrDistanceToBounds(const MpActor& actor, const MpActor& target)
{
  // TODO(#491): Figure out where to take the missing reach component
  constexpr float kPatch = 15.f;

  auto bounds = actor.GetBounds();
  auto targetBounds = target.GetBounds();

  // "Y" is "face" of character
  const float angleZ = 90.f - target.GetAngle().z;
  float direction = actor.GetAngle().z;

  // vector from target to the actor
  NiPoint3 position = actor.GetPos() - target.GetPos();
  position += RotateZ(
    NiPoint3(kPatch + bounds.pos2[1], 0.f, 0.f + bounds.pos2[2]), direction);

  NiPoint3 pos = RotateZ(position, angleZ);

  bool isProjectionInside[3] = {
    (targetBounds.pos1[0] <= pos.x && pos.x <= targetBounds.pos2[0]),
    (targetBounds.pos1[1] <= pos.y && pos.y <= targetBounds.pos2[1]),
    (targetBounds.pos1[2] <= pos.z && pos.z <= targetBounds.pos2[2])
  };

  NiPoint3 nearestCorner = {
    pos[0] > 0 ? 0.f + targetBounds.pos2[0] : 0.f + targetBounds.pos1[0],
    pos[1] > 0 ? 0.f + targetBounds.pos2[1] : 0.f + targetBounds.pos1[1],
    pos[2] > 0 ? 0.f + targetBounds.pos2[2] : 0.f + targetBounds.pos1[2]
  };

  return NiPoint3(isProjectionInside[0] ? 0.f : pos.x - nearestCorner.x,
                  isProjectionInside[1] ? 0.f : pos.y - nearestCorner.y,
                  isProjectionInside[2] ? 0.f : pos.z - nearestCorner.z)
    .SqrLength();
}

bool IsDistanceValid(const MpActor& actor, const MpActor& targetActor,
                     const HitData& hitData)
{
  float sqrDistance = GetSqrDistanceToBounds(actor, targetActor);

  // TODO: fix bounding boxes for creatures such as chicken, mudcrab, etc
  float reachPveHotfixMult =
    (actor.GetBaseId() <= 0x7 && targetActor.GetBaseId() <= 0x7)
    ? 1.f
    : std::numeric_limits<float>::infinity();

  float reach = GetReach(actor, hitData.source, reachPveHotfixMult);

  // For bow/crossbow shots we don't want to check melee radius
  if (!hitData.isBashAttack) {
    constexpr float kExteriorCellWidthUnits = 4096.f;
    if (auto worldState = actor.GetParent()) {
      if (worldState->HasEspm()) {
        auto weapDNAM =
          espm::GetData<espm::WEAP>(hitData.source, worldState).weapDNAM;
        if (weapDNAM->animType == espm::WEAP::AnimType::Bow) {
          reach = kExteriorCellWidthUnits * 2;
        } else if (weapDNAM->animType == espm::WEAP::AnimType::Crossbow) {
          reach = kExteriorCellWidthUnits * 2;
        }
      }
    }
  }

  return reach * reach > sqrDistance;
}

bool CanHit(const MpActor& actor, const HitData& hitData,
            const std::chrono::duration<float>& timePassed)
{
  WorldState* espmProvider = actor.GetParent();
  auto weapDNAM =
    espm::GetData<espm::WEAP>(hitData.source, espmProvider).weapDNAM;

  if (weapDNAM) {
    float speedMult = weapDNAM->speed;
    return timePassed.count() >= (1.1 * (1 / speedMult)) -
      (1.1 * (1 / speedMult) * (speedMult <= 0.75 ? 0.45 : 0.3));
  }

  throw std::runtime_error(
    fmt::format("Cannot get weapon speed from source: {0:x}", hitData.source));
}

bool ShouldBeBlocked(const MpActor& aggressor, const MpActor& target)
{
  NiPoint3 targetViewDirection = target.GetViewDirection();
  NiPoint3 aggressorDirection = aggressor.GetPos() - target.GetPos();
  if (targetViewDirection * aggressorDirection <= 0) {
    return false;
  }
  float angle =
    std::acos((targetViewDirection * aggressorDirection) /
              (targetViewDirection.Length() * aggressorDirection.Length()));
  return angle < 1;
}
}

void ActionListener::OnHit(const RawMessageData& rawMsgData_,
                           const HitData& hitData_)
{
  auto currentHitTime = std::chrono::steady_clock::now();
  MpActor* myActor = partOne.serverState.ActorByUser(rawMsgData_.userId);
  if (!myActor) {
    throw std::runtime_error("Unable to change values without Actor attached");
  }

  MpActor* aggressor = nullptr;

  HitData hitData = hitData_;
  if (hitData.aggressor == 0x14) {
    aggressor = myActor;
    hitData.aggressor = aggressor->GetFormId();
  } else {
    aggressor = &partOne.worldState.GetFormAt<MpActor>(hitData.aggressor);
    auto it = partOne.worldState.hosters.find(hitData.aggressor);
    if (it == partOne.worldState.hosters.end() ||
        it->second != myActor->GetFormId()) {
      spdlog::error("SendToNeighbours - No permission to send OnHit with "
                    "aggressor actor {:x}",
                    aggressor->GetFormId());
      return;
    }
  }

  if (hitData.target == 0x14) {
    hitData.target = myActor->GetFormId();
  }

  if (aggressor->IsDead()) {
    spdlog::debug(fmt::format("{:x} actor is dead and can't attack. "
                              "requesting respawn in order to fix death state",
                              aggressor->GetFormId()));
    aggressor->RespawnWithDelay(true);
    return;
  }

  if (aggressor->GetEquipment().inv.HasItem(hitData.source) == false &&
      IsUnarmedAttack(hitData.source) == false) {

    if (aggressor->GetInventory().HasItem(hitData.source) == false) {
      spdlog::debug("{:x} actor has no {:x} weapon and can't attack",
                    hitData.aggressor, hitData.source);
    }
    spdlog::debug(
      "{:x} weapon is not equipped by {:x} actor and cannot be used",
      hitData.source, hitData.aggressor);
    return;
  }

  auto refr = std::dynamic_pointer_cast<MpObjectReference>(
    partOne.worldState.LookupFormById(hitData.target));
  if (!refr) {
    spdlog::error("ActionListener::OnHit - MpObjectReference not found for "
                  "hitData.target {:x}",
                  hitData.target);
    return;
  }

  auto& browser = partOne.worldState.GetEspm().GetBrowser();
  std::array<VarValue, 7> args;
  args[0] = VarValue(aggressor->ToGameObject()); // akAgressor
  args[1] = VarValue(std::make_shared<EspmGameObject>(
    browser.LookupById(hitData.source)));    // akSource
  args[2] = VarValue::None();                // akProjectile
  args[3] = VarValue(hitData.isPowerAttack); // abPowerAttack
  args[4] = VarValue(hitData.isSneakAttack); // abSneakAttack
  args[5] = VarValue(hitData.isBashAttack);  // abBashAttack
  args[6] = VarValue(hitData.isHitBlocked);  // abHitBlocked
  refr->SendPapyrusEvent("OnHit", args.data(), args.size());

  auto targetActorPtr = refr->AsActor();
  if (!targetActorPtr) {
    return; // Not an actor, damage calculation is not needed
  }

  auto& targetActor = *targetActorPtr;

  auto lastHitTime = aggressor->GetLastHitTime();
  std::chrono::duration<float> timePassed = currentHitTime - lastHitTime;

  if (!CanHit(*aggressor, hitData, timePassed)) {
    WorldState* espmProvider = targetActor.GetParent();
    auto weapDNAM =
      espm::GetData<espm::WEAP>(hitData.source, espmProvider).weapDNAM;
    float expectedAttackTime = (1.1 * (1 / weapDNAM->speed)) -
      (1.1 * (1 / weapDNAM->speed) * (weapDNAM->speed <= 0.75 ? 0.45 : 0.3));
    spdlog::debug(
      "Target {0:x} is not available for attack due to fast "
      "attack speed. Weapon: {1:x}. Elapsed time: {2}. Expected attack time: "
      "{3}",
      hitData.target, hitData.source, timePassed.count(), expectedAttackTime);
    return;
  }

  // if (IsDistanceValid(*aggressor, targetActor, hitData) == false) {
  //   float distance =
  //     std::sqrt(GetSqrDistanceToBounds(*aggressor, targetActor));

  //   // TODO: fix bounding boxes for creatures such as chicken, mudcrab, etc
  //   float reachPveHotfixMult =
  //     (aggressor->GetBaseId() <= 0x7 && targetActor.GetBaseId() <= 0x7)
  //     ? 1.f
  //     : std::numeric_limits<float>::infinity();

  //   float reach = GetReach(*aggressor, hitData.source, reachPveHotfixMult);
  //   uint32_t aggressorId = aggressor->GetFormId();
  //   uint32_t targetId = targetActor.GetFormId();
  //   spdlog::debug(
  //     fmt::format("{:x} actor can't reach {:x} target because distance {} is
  //     "
  //                 "greater then first actor attack radius {}",
  //                 aggressorId, targetId, distance, reach));
  //   return;
  // }

  ActorValues currentActorValues = targetActor.GetChangeForm().actorValues;

  float healthPercentage = currentActorValues.healthPercentage;

  if (targetActor.IsBlockActive()) {
    if (ShouldBeBlocked(*aggressor, targetActor)) {
      bool isRemoteBowAttack = false;

      auto sourceLookupResult =
        targetActor.GetParent()->GetEspm().GetBrowser().LookupById(
          hitData.source);
      if (sourceLookupResult.rec &&
          sourceLookupResult.rec->GetType() == espm::WEAP::kType) {
        auto weapData =
          espm::GetData<espm::WEAP>(hitData.source, targetActor.GetParent());
        if (weapData.weapDNAM) {
          if (weapData.weapDNAM->animType == espm::WEAP::AnimType::Bow ||
              weapData.weapDNAM->animType == espm::WEAP::AnimType::Crossbow) {
            if (!hitData.isBashAttack) {
              isRemoteBowAttack = true;
            }
          }
        }
      }

      bool isBlockingByShield = false;

      auto targetActorEquipmentEntries =
        targetActor.GetEquipment().inv.entries;
      for (auto& entry : targetActorEquipmentEntries) {
        if (entry.GetWorn() != Inventory::Worn::None) {
          auto res =
            targetActor.GetParent()->GetEspm().GetBrowser().LookupById(
              entry.baseId);
          if (res.rec && res.rec->GetType() == espm::ARMO::kType) {
            auto data =
              espm::GetData<espm::ARMO>(entry.baseId, targetActor.GetParent());
            bool isShield = data.equipSlotId > 0;
            if (isShield) {
              isBlockingByShield = isShield;
            }
          }
        }
      }

      if (!isRemoteBowAttack || isBlockingByShield) {
        hitData.isHitBlocked = true;
      }
    }
  }

  float damage = partOne.CalculateDamage(*aggressor, targetActor, hitData);
  damage = damage < 0.f ? 0.f : damage;
  float outBaseHealth = 0.f;
  currentActorValues.healthPercentage = CalculateCurrentHealthPercentage(
    targetActor, damage, healthPercentage, &outBaseHealth);

  currentActorValues.healthPercentage =
    currentActorValues.healthPercentage < 0.f
    ? 0.f
    : currentActorValues.healthPercentage;

  targetActor.NetSetPercentages(currentActorValues, aggressor);
  aggressor->SetLastHitTime();

  spdlog::debug("Target {0:x} is hitted by {1} damage. Percentage was: {3}, "
                "percentage now: {2}, base health: {4})",
                hitData.target, damage, currentActorValues.healthPercentage,
                healthPercentage, outBaseHealth);
}

void ActionListener::OnUpdateAnimVariables(const RawMessageData& rawMsgData)
{
  const MpActor* myActor = partOne.serverState.ActorByUser(rawMsgData.userId);

  if (!myActor) {
    throw std::runtime_error("Unable to change values without Actor attached");
  }

  SendToNeighbours(myActor->idx, rawMsgData);
}

void ActionListener::OnSpellCast(const RawMessageData& rawMsgData,
                                 const SpellCastData& spellCastData_)
{
  MpActor* myActor = partOne.serverState.ActorByUser(rawMsgData.userId);

  if (!myActor) {
    throw std::runtime_error("Unable to change values without Actor attached");
  }

  MpActor* caster = nullptr;

  SpellCastData spellCastData = spellCastData_;

  if (spellCastData.caster == 0x14 ||
      spellCastData.caster == myActor->GetFormId()) {
    caster = myActor;
    spellCastData.caster = caster->GetFormId();
  } else {
    caster = &partOne.worldState.GetFormAt<MpActor>(spellCastData.caster);
    const auto it = partOne.worldState.hosters.find(spellCastData.caster);

    if (it == partOne.worldState.hosters.end() ||
        it->second != myActor->GetFormId()) {
      spdlog::error(
        "SendToNeighbours - No permission to send OnSpellCast with "
        "caster actor {:x}",
        caster->GetFormId());
      return;
    }
  }

  if (spellCastData.target == 0x14 ||
      spellCastData.target == myActor->GetFormId()) {
    spellCastData.target = myActor->GetFormId();
  }

  if (caster->IsDead()) {
    spdlog::info(fmt::format("{:x} actor is dead and can't spell cast. "
                             "requesting respawn in order to fix death state",
                             caster->GetFormId()));
    caster->RespawnWithDelay(true);
    return;
  }

  const auto equipment = caster->GetEquipment();

  bool isEquippedSpellValid = false;

  switch (spellCastData.castingSource) {
    case SpellType::Left:
      isEquippedSpellValid = spellCastData.spell == equipment.leftSpell;
      break;
    case SpellType::Right:
      isEquippedSpellValid = spellCastData.spell == equipment.rightSpell;
      break;
    case SpellType::Voise:
      isEquippedSpellValid = spellCastData.spell == equipment.voiceSpell;
      break;
    case SpellType::Instant:
      isEquippedSpellValid = spellCastData.spell == equipment.instantSpell;
      break;
  }

  if (isEquippedSpellValid == false) {
    spdlog::info("ActionListener::OnSpellCast - spell {0:x} not "
                 "found in equipment",
                 spellCastData.spell);
    return;
  }

  SendToNeighbours(myActor->idx, rawMsgData);

  if (spellCastData.isInterruptCast) {
    return;
  }

  auto& browser = partOne.worldState.GetEspm().GetBrowser();

  const std::array<VarValue, 1> args{ VarValue(
    std::make_shared<EspmGameObject>(
      browser.LookupById(spellCastData.spell))) };

  caster->SendPapyrusEvent("OnSpellCast", args.data(), args.size());

  const auto targetRef = std::dynamic_pointer_cast<MpObjectReference>(
    partOne.worldState.LookupFormById(spellCastData.target));

  if (!targetRef) {
    spdlog::info(
      "ActionListener::OnSpellCast - MpObjectReference not found for "
      "spellCastData.target {:x}",
      spellCastData.target);
    return;
  }

  const auto targetActorPtr = targetRef->AsActor();

  if (!targetActorPtr) {
    return; // Not an actor, damage calculation is not needed
  }

  auto targetActorValues = targetActorPtr->GetChangeForm().actorValues;

  float damage =
    partOne.CalculateDamage(*caster, *targetActorPtr, spellCastData);
  damage = damage <= 0.f ? 0.f : damage;

  targetActorValues.healthPercentage = CalculateCurrentHealthPercentage(
    *targetActorPtr, damage, targetActorValues.healthPercentage, nullptr);

  targetActorPtr->NetSetPercentages(targetActorValues, caster);

  spdlog::info(
    "Target {0:x} is hitted by {1:x} spell on {2} damage. By caster: {3:x}, "
    "from castingSource : {4})",
    spellCastData.target, spellCastData.spell, damage, spellCastData.caster,
    static_cast<uint32_t>(spellCastData.castingSource));
}

void ActionListener::OnUnknown(const RawMessageData& rawMsgData)
{
  spdlog::error("Got unhandled message: {}",
                simdjson::minify(rawMsgData.parsed));
}
