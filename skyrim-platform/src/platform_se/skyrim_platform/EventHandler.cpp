#include "EventHandler.h"
#include "EventManager.h"
#include "EventsApi.h"
#include "JsUtils.h"
#include "SkyrimPlatform.h"

namespace {
inline void SendEvent(const char* name)
{
  EventsApi::SendEvent(name, { JsValue::Undefined() });
}

inline void SendEvent(const char* name, JsValue obj)
{

  EventsApi::SendEvent(name, { JsValue::Undefined(), obj });
}
}

void EventHandler::HandleSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
  switch (msg->type) {
    // case SKSE::MessagingInterface::kDataLoaded: {
    //   EventManager::Init();
    //   SendEvent("dataLoaded");
    // } break;
    // case SKSE::MessagingInterface::kInputLoaded:
    //   SendEvent("inputLoaded");
    //   break;
    // case SKSE::MessagingInterface::kPostLoad:
    //   SendEvent("postLoad");
    //   break;
    // case SKSE::MessagingInterface::kPostPostLoad:
    //   SendEvent("postPostLoad");
    //   break;
    // case SKSE::MessagingInterface::kNewGame:
    //   SendEvent("newGame");
    //   break;
    // case SKSE::MessagingInterface::kPreLoadGame:
    //   SendEvent("preLoadGame");
    //   break;
    // case SKSE::MessagingInterface::kPostLoadGame:
    //   SendEvent("postLoadGame");
    //   break;
    // case SKSE::MessagingInterface::kSaveGame:
    //   SendEvent("saveGame");
    //   break;
    // case SKSE::MessagingInterface::kDeleteGame:
    //   SendEvent("deleteGame");
    //   break;
  }
}

EventResult EventHandler::ProcessEvent(
  const RE::TESActivateEvent* event, RE::BSTEventSource<RE::TESActivateEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "target", e->objectActivated.get(),
                   "ObjectReference");
    AddObjProperty(&obj, "caster", e->actionRef.get(), "ObjectReference");
    AddObjProperty(&obj, "isCrimeToActivate",
                   e->objectActivated.get()->IsCrimeToActivate());

    SendEvent("activate", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESActiveEffectApplyRemoveEvent* event,
  RE::BSTEventSource<RE::TESActiveEffectApplyRemoveEvent>*)
{
  auto caster = event->caster.get() ? event->caster.get() : nullptr;
  auto target = event->target.get() ? event->target.get() : nullptr;

  auto casterId = caster ? caster->formID : 0;
  auto targetId = target ? target->formID : 0;

  if ((!caster || caster->formID != casterId) ||
      (!target || target->formID != targetId) ||
      (target->formType.get() != RE::FormType::ActorCharacter)) {
    return EventResult::kContinue;
  }

  auto activeEffectUniqueID = event ? event->activeEffectUniqueID : 0;

  RE::ActiveEffect* activeEffect = nullptr;
  for (const auto& effect : *target->As<RE::Actor>()->GetActiveEffectList()) {
    if (effect->usUniqueID == event->activeEffectUniqueID) {
      activeEffect = effect;
      break;
    }
  }

  if (!activeEffect)
    return EventResult::kContinue;

  SkyrimPlatform::GetSingleton().AddUpdateTask([=] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "effect", activeEffect->GetBaseObject(),
                   "MagicEffect");
    AddObjProperty(&obj, "activeEffect", activeEffect, "ActiveMagicEffect");
    AddObjProperty(&obj, "caster", caster, "ObjectReference");
    AddObjProperty(&obj, "target", target, "ObjectReference");

    if (event->isApplied)
      EventsApi::SendEvent("effectStart", { JsValue::Undefined(), obj });
    else
      EventsApi::SendEvent("effectFinish", { JsValue::Undefined(), obj });
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESActorLocationChangeEvent* event,
  RE::BSTEventSource<RE::TESActorLocationChangeEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "actor", e->actor.get(), "Actor");
    AddObjProperty(&obj, "oldLoc", e->oldLoc, "Location");
    AddObjProperty(&obj, "newLoc", e->newLoc, "Location");

    SendEvent("locationChanged", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESBookReadEvent* event, RE::BSTEventSource<RE::TESBookReadEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "book", e->book.get(), "ObjectReference");

    SendEvent("bookRead", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESCellAttachDetachEvent* event,
  RE::BSTEventSource<RE::TESCellAttachDetachEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "refr", e->reference.get(), "ObjectReference");

    if (e->action == 1) {
      SendEvent("cellAttach", obj);
    } else if (e->action == 0) {
      SendEvent("cellDetach", obj);
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESCellFullyLoadedEvent* event,
  RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "cell", e->cell, "Cell");

    SendEvent("cellFullyLoaded", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const RE::TESCombatEvent* event,
                                       RE::BSTEventSource<RE::TESCombatEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "target", e->targetActor.get(), "ObjectReference");
    AddObjProperty(&obj, "actor", e->actor.get(), "ObjectReference");
    AddObjProperty(&obj, "isCombat",
                   e->newState.any(RE::ACTOR_COMBAT_STATE::kCombat));
    AddObjProperty(&obj, "isSearching",
                   e->newState.any(RE::ACTOR_COMBAT_STATE::kSearching));

    SendEvent("combatState", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESContainerChangedEvent* event,
  RE::BSTEventSource<RE::TESContainerChangedEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto contFormOld = RE::TESForm::LookupByID(e->oldContainer);
    auto contFormNew = RE::TESForm::LookupByID(e->newContainer);
    auto baseObjForm = RE::TESForm::LookupByID(e->baseObj);

    AddObjProperty(&obj, "oldContainer", contFormOld, "ObjectReference");
    AddObjProperty(&obj, "newContainer", contFormNew, "ObjectReference");
    AddObjProperty(&obj, "baseObj", baseObjForm, "Form");
    AddObjProperty(&obj, "numItems", e->itemCount);
    AddObjProperty(&obj, "uniqueID", e->uniqueID);
    AddObjProperty(&obj, "reference", e->reference.get().get(),
                   "ObjectReference");

    SendEvent("containerChanged", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const RE::TESDeathEvent* event,
                                       RE::BSTEventSource<RE::TESDeathEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "actorDying", e->actorDying.get(), "ObjectReference");
    AddObjProperty(&obj, "actorKiller", e->actorKiller.get(),
                   "ObjectReference");

    e->dead ? SendEvent("deathEnd", obj) : SendEvent("deathStart", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESDestructionStageChangedEvent* event,
  RE::BSTEventSource<RE::TESDestructionStageChangedEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");
    AddObjProperty(&obj, "oldStage", e->oldStage);
    AddObjProperty(&obj, "newStage", e->newStage);

    SendEvent("destructionStageChanged", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESEnterBleedoutEvent* event,
  RE::BSTEventSource<RE::TESEnterBleedoutEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "actor", e->actor.get(), "ObjectReference");

    SendEvent("enterBleedout", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const RE::TESEquipEvent* event,
                                       RE::BSTEventSource<RE::TESEquipEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto baseObjForm = RE::TESForm::LookupByID(e->baseObject);
    auto originalRefrForm = RE::TESForm::LookupByID(e->originalRefr);

    AddObjProperty(&obj, "actor", e->actor.get(), "ObjectReference");
    AddObjProperty(&obj, "baseObj", baseObjForm, "Form");
    AddObjProperty(&obj, "originalRefr", originalRefrForm, "ObjectReference");
    AddObjProperty(&obj, "uniqueId", e->uniqueID);

    e->equipped ? SendEvent("equip", obj) : SendEvent("unequip", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESFastTravelEndEvent* event,
  RE::BSTEventSource<RE::TESFastTravelEndEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "travelTimeGameHours", e->travelTimeGameHours);

    SendEvent("fastTravelEnd", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESFurnitureEvent* event,
  RE::BSTEventSource<RE::TESFurnitureEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "actor", e->actor.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->targetFurniture.get(),
                   "ObjectReference");

    if (e->type == RE::TESFurnitureEvent::FurnitureEventType::kExit) {
      SendEvent("furnitureExit", obj);
    } else if (e->type == RE::TESFurnitureEvent::FurnitureEventType::kEnter) {
      SendEvent("furnitureEnter", obj);
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESGrabReleaseEvent* event,
  RE::BSTEventSource<RE::TESGrabReleaseEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "refr", e->ref.get(), "ObjectReference");
    AddObjProperty(&obj, "isGrabbed", e->grabbed);

    SendEvent("grabRelease", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const RE::TESHitEvent* event,
                                       RE::BSTEventSource<RE::TESHitEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto sourceForm = RE::TESForm::LookupByID(e->source);
    auto projectileForm = RE::TESForm::LookupByID(e->projectile);

    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");
    AddObjProperty(&obj, "aggressor", e->cause.get(), "ObjectReference");
    AddObjProperty(&obj, "source", sourceForm, "Form");
    AddObjProperty(&obj, "projectile", projectileForm, "Form");
    AddObjProperty(&obj, "isPowerAttack",
                   e->flags.any(RE::TESHitEvent::Flag::kPowerAttack));
    AddObjProperty(&obj, "isSneakAttack",
                   e->flags.any(RE::TESHitEvent::Flag::kSneakAttack));
    AddObjProperty(&obj, "isBashAttack",
                   e->flags.any(RE::TESHitEvent::Flag::kBashAttack));
    AddObjProperty(&obj, "isHitBlocked",
                   e->flags.any(RE::TESHitEvent::Flag::kHitBlocked));

    SendEvent("hit", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESInitScriptEvent* event,
  RE::BSTEventSource<RE::TESInitScriptEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "initializedObject", e->objectInitialized.get(),
                   "ObjectReference");

    SendEvent("scriptInit", obj);
  });

  return EventResult::kContinue;
}

// TODO: Look into LoadGame event
EventResult EventHandler::ProcessEvent(
  const RE::TESLoadGameEvent* event, RE::BSTEventSource<RE::TESLoadGameEvent>*)
{
  SkyrimPlatform::GetSingleton().AddUpdateTask([] { SendEvent("loadGame"); });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESLockChangedEvent* event,
  RE::BSTEventSource<RE::TESLockChangedEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "lockedObject", e->lockedObject.get(),
                   "ObjectReference");

    SendEvent("lockChanged", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESMagicEffectApplyEvent* event,
  RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto effect = RE::TESForm::LookupByID(e->magicEffect);

    AddObjProperty(&obj, "effect", effect, "MagicEffect");
    AddObjProperty(&obj, "caster", e->caster.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");

    SendEvent("magicEffectApply", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESMagicWardHitEvent* event,
  RE::BSTEventSource<RE::TESMagicWardHitEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto spell = RE::TESForm::LookupByID(e->spell);
    auto status = to_underlying(e->status);

    AddObjProperty(&obj, "caster", e->caster.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");
    AddObjProperty(&obj, "spell", spell, "Spell");
    AddObjProperty(&obj, "status", status);

    SendEvent("wardHit", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESMoveAttachDetachEvent* event,
  RE::BSTEventSource<RE::TESMoveAttachDetachEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "movedRef", e->movedRef.get(), "ObjectReference");
    AddObjProperty(&obj, "isCellAttached", e->isCellAttached);

    SendEvent("moveAttachDetach", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESObjectLoadedEvent* event,
  RE::BSTEventSource<RE::TESObjectLoadedEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto object = RE::TESForm::LookupByID(e->formID);

    AddObjProperty(&obj, "object", object, "Form");
    AddObjProperty(&obj, "isLoaded", e->loaded);

    SendEvent("objectLoaded", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESObjectREFRTranslationEvent* event,
  RE::BSTEventSource<RE::TESObjectREFRTranslationEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "reference", e->refr.get(), "ObjectReference");

    switch (e->type) {
      case RE::TESObjectREFRTranslationEvent::EventType::kFailed: {
        SendEvent("translationFailed", obj);
        break;
      }
      case RE::TESObjectREFRTranslationEvent::EventType::kAlmostCompleted: {
        SendEvent("translationAlmostCompleted", obj);
        break;
      }
      case RE::TESObjectREFRTranslationEvent::EventType::kCompleted: {
        SendEvent("translationCompleted", obj);
        break;
      }
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESOpenCloseEvent* event,
  RE::BSTEventSource<RE::TESOpenCloseEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "caster", e->activeRef.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->ref.get(), "ObjectReference");

    if (e->opened) {
      SendEvent("open", obj);
    } else {
      SendEvent("close", obj);
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESPackageEvent* event, RE::BSTEventSource<RE::TESPackageEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto package = RE::TESForm::LookupByID(e->package);

    AddObjProperty(&obj, "actor", e->actor.get(), "ObjectReference");
    AddObjProperty(&obj, "package", package, "Package");

    switch (e->type) {
      case RE::TESPackageEvent::EventType::kStart: {
        SendEvent("packageStart", obj);
        break;
      }
      case RE::TESPackageEvent::EventType::kChange: {
        SendEvent("packageChange", obj);
        break;
      }
      case RE::TESPackageEvent::EventType::kEnd: {
        SendEvent("packageEnd", obj);
        break;
      }
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESPerkEntryRunEvent* event,
  RE::BSTEventSource<RE::TESPerkEntryRunEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto perk = RE::TESForm::LookupByID(e->perkId);

    AddObjProperty(&obj, "cause", e->cause.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");
    AddObjProperty(&obj, "perk", perk, "Perk");
    AddObjProperty(&obj, "flag", e->flag);

    SendEvent("perkEntryRun", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESPlayerBowShotEvent* event,
  RE::BSTEventSource<RE::TESPlayerBowShotEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto weapon = RE::TESForm::LookupByID(e->weaponId);
    auto ammo = RE::TESForm::LookupByID(e->ammoId);

    AddObjProperty(&obj, "weapon", weapon, "Weapon");
    AddObjProperty(&obj, "ammo", ammo, "Ammo");
    AddObjProperty(&obj, "power", e->power);
    AddObjProperty(&obj, "target", e->isSunGazing);

    SendEvent("playerBowShot", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESQuestInitEvent* event,
  RE::BSTEventSource<RE::TESQuestInitEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto quest = RE::TESForm::LookupByID(e->questId);

    AddObjProperty(&obj, "quest", quest, "Quest");

    SendEvent("questInit", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESQuestStageEvent* event,
  RE::BSTEventSource<RE::TESQuestStageEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto quest = RE::TESForm::LookupByID(e->questId);

    AddObjProperty(&obj, "quest", quest, "Quest");
    AddObjProperty(&obj, "stage", e->stage);

    SendEvent("questStage", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESQuestStartStopEvent* event,
  RE::BSTEventSource<RE::TESQuestStartStopEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto quest = RE::TESForm::LookupByID(e->questId);

    AddObjProperty(&obj, "quest", quest, "Quest");

    if (e->isStarted) {
      SendEvent("questStart", obj);
    } else {
      SendEvent("questStop", obj);
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const RE::TESResetEvent* event,
                                       RE::BSTEventSource<RE::TESResetEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "object", e->object.get(), "ObjectReference");

    SendEvent("reset", obj);
  });

  return EventResult::kStop;
}

EventResult EventHandler::ProcessEvent(const RE::TESSellEvent* event,
                                       RE::BSTEventSource<RE::TESSellEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "seller", e->seller.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");

    SendEvent("sell", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESSleepStartEvent* event,
  RE::BSTEventSource<RE::TESSleepStartEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "startTime", e->sleepStartTime);
    AddObjProperty(&obj, "desiredStopTime", e->desiredSleepEndTime);

    SendEvent("sleepStart", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESSleepStopEvent* event,
  RE::BSTEventSource<RE::TESSleepStopEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "isInterrupted", e->isInterrupted);

    SendEvent("sleepStop", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESSpellCastEvent* event,
  RE::BSTEventSource<RE::TESSpellCastEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto spell = RE::TESForm::LookupByID(e->spell);

    AddObjProperty(&obj, "caster", e->caster.get(), "ObjectReference");
    AddObjProperty(&obj, "spell", spell, "Spell");

    SendEvent("spellCast", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESSwitchRaceCompleteEvent* event,
  RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "subject", e->subject.get(), "ObjectReference");

    SendEvent("switchRaceComplete", obj);
  });
  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESTrackedStatsEvent* event,
  RE::BSTEventSource<RE::TESTrackedStatsEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "statName", e->stat.data());
    AddObjProperty(&obj, "newValue", e->value);

    SendEvent("trackedStats", obj);
  });
  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESTriggerEnterEvent* event,
  RE::BSTEventSource<RE::TESTriggerEnterEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "caster", e->caster.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");

    SendEvent("triggerEnter", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESTriggerEvent* event, RE::BSTEventSource<RE::TESTriggerEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "caster", e->caster.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");

    SendEvent("trigger", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESTriggerLeaveEvent* event,
  RE::BSTEventSource<RE::TESTriggerLeaveEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([=] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "caster", e->caster.get(), "ObjectReference");
    AddObjProperty(&obj, "target", e->target.get(), "ObjectReference");

    SendEvent("triggerLeave", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESUniqueIDChangeEvent* event,
  RE::BSTEventSource<RE::TESUniqueIDChangeEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "oldBaseID", e->oldBaseID);
    AddObjProperty(&obj, "newBaseID", e->newBaseID);
    AddObjProperty(&obj, "oldUniqueID", e->oldUniqueID);
    AddObjProperty(&obj, "newUniqueID", e->newUniqueID);

    SendEvent("uniqueIdChange", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESWaitStartEvent* event,
  RE::BSTEventSource<RE::TESWaitStartEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "startTime", e->waitStartTime);
    AddObjProperty(&obj, "desiredStopTime", e->desiredWaitEndTime);

    SendEvent("waitStart", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::TESWaitStopEvent* event, RE::BSTEventSource<RE::TESWaitStopEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "isInterrupted", e->interrupted);

    SendEvent("waitStop", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const SKSE::ActionEvent* event,
                                       RE::BSTEventSource<SKSE::ActionEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto slot = to_underlying(e->slot.get());

    AddObjProperty(&obj, "actor", e->actor, "Actor");
    AddObjProperty(&obj, "source", e->sourceForm, "Form");
    AddObjProperty(&obj, "slot", slot);

    switch (e->type.get()) {
      case SKSE::ActionEvent::Type::kWeaponSwing: {
        SendEvent("actionWeaponSwing", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kBeginDraw: {
        SendEvent("actionBeginDraw", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kEndDraw: {
        SendEvent("actionEndDraw", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kBowDraw: {
        SendEvent("actionBowDraw", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kBowRelease: {
        SendEvent("actionBowRelease", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kBeginSheathe: {
        SendEvent("actionBeginSheathe", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kEndSheathe: {
        SendEvent("actionEndSheathe", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kSpellCast: {
        SendEvent("actionSpellCast", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kSpellFire: {
        SendEvent("actionSpellFire", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kVoiceCast: {
        SendEvent("actionVoiceCast", obj);
        break;
      }
      case SKSE::ActionEvent::Type::kVoiceFire: {
        SendEvent("actionVoiceFire", obj);
        break;
      }
    }
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(const SKSE::CameraEvent* event,
                                       RE::BSTEventSource<SKSE::CameraEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto oldStateId = to_underlying(e->oldState->id);
    auto newStateId = to_underlying(e->newState->id);

    AddObjProperty(&obj, "oldStateId", oldStateId);
    AddObjProperty(&obj, "newStateId", newStateId);

    SendEvent("cameraStateChanged", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const SKSE::CrosshairRefEvent* event,
  RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "reference", e->crosshairRef.get(),
                   "ObjectReference");

    SendEvent("crosshairRefChanged", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const SKSE::NiNodeUpdateEvent* event,
  RE::BSTEventSource<SKSE::NiNodeUpdateEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "reference", e->reference, "ObjectReference");

    SendEvent("niNodeUpdate", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const SKSE::ModCallbackEvent* event,
  RE::BSTEventSource<SKSE::ModCallbackEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([=] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "sender", event->sender, "Form");
    AddObjProperty(&obj, "eventName", event->eventName);
    AddObjProperty(&obj, "strArg", event->strArg);
    AddObjProperty(&obj, "numArg", event->numArg);

    SendEvent("modEvent", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::MenuOpenCloseEvent* event,
  RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "name", e->menuName);

    if (e->opening) {
      SendEvent("menuOpen", obj);
    } else {
      SendEvent("menuClose", obj);
    }
  });

  return EventResult::kContinue;
};

EventResult EventHandler::ProcessEvent(RE::InputEvent* const* event,
                                       RE::BSTEventSource<RE::InputEvent*>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    for (auto eventItem = *e; eventItem; eventItem = eventItem->next) {
      if (!eventItem) {
        return;
      }

      auto device = to_underlying(eventItem->device.get());

      switch (eventItem->eventType.get()) {
        case RE::INPUT_EVENT_TYPE::kButton: {
          auto buttonEvent = static_cast<RE::ButtonEvent*>(eventItem);
          auto obj = JsValue::Object();

          AddObjProperty(&obj, "device", device);
          AddObjProperty(&obj, "code", buttonEvent->idCode);
          AddObjProperty(&obj, "userEventName", buttonEvent->userEvent);
          AddObjProperty(&obj, "value", buttonEvent->value);
          AddObjProperty(&obj, "heldDuration", buttonEvent->heldDownSecs);
          AddObjProperty(&obj, "isPressed", buttonEvent->IsPressed());
          AddObjProperty(&obj, "isUp", buttonEvent->IsUp());
          AddObjProperty(&obj, "isDown", buttonEvent->IsDown());
          AddObjProperty(&obj, "isHeld", buttonEvent->IsHeld());
          AddObjProperty(&obj, "isRepeating", buttonEvent->IsRepeating());

          SendEvent("buttonEvent", obj);
          break;
        }
        case RE::INPUT_EVENT_TYPE::kMouseMove: {
          auto mouseEvent = static_cast<RE::MouseMoveEvent*>(eventItem);
          auto obj = JsValue::Object();

          AddObjProperty(&obj, "device", device);
          AddObjProperty(&obj, "code", mouseEvent->idCode);
          AddObjProperty(&obj, "userEventName", mouseEvent->userEvent);
          AddObjProperty(&obj, "inputX", mouseEvent->mouseInputX);
          AddObjProperty(&obj, "inputY", mouseEvent->mouseInputY);

          SendEvent("mouseMove", obj);
          break;
        }
        case RE::INPUT_EVENT_TYPE::kDeviceConnect: {
          auto deviceConnectEvent =
            static_cast<RE::DeviceConnectEvent*>(eventItem);
          auto obj = JsValue::Object();

          AddObjProperty(&obj, "device", device);
          AddObjProperty(&obj, "isConnected", deviceConnectEvent->connected);

          SendEvent("deviceConnect", obj);
          break;
        }
        case RE::INPUT_EVENT_TYPE::kThumbstick: {
          auto thumbstickEvent = static_cast<RE::ThumbstickEvent*>(eventItem);
          auto obj = JsValue::Object();

          AddObjProperty(&obj, "device", device);
          AddObjProperty(&obj, "code", thumbstickEvent->idCode);
          AddObjProperty(&obj, "userEventName", thumbstickEvent->userEvent);
          AddObjProperty(&obj, "inputX", thumbstickEvent->xValue);
          AddObjProperty(&obj, "inputY", thumbstickEvent->yValue);
          AddObjProperty(&obj, "isLeft", thumbstickEvent->IsLeft());
          AddObjProperty(&obj, "isRight", thumbstickEvent->IsRight());

          SendEvent("thumbstickEvent", obj);
          break;
        }
        case RE::INPUT_EVENT_TYPE::kKinect: {
          auto kinectEvent = static_cast<RE::KinectEvent*>(eventItem);
          auto obj = JsValue::Object();

          AddObjProperty(&obj, "device", device);
          AddObjProperty(&obj, "code", kinectEvent->idCode);
          AddObjProperty(&obj, "userEventName", kinectEvent->userEvent);
          AddObjProperty(&obj, "heard", kinectEvent->heard);

          SendEvent("kinectEvent", obj);
          break;
        }
      }
    }
  });

  return EventResult::kContinue;
};

EventResult EventHandler::ProcessEvent(
  const RE::BGSFootstepEvent* event, RE::BSTEventSource<RE::BGSFootstepEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    AddObjProperty(&obj, "tag", e->tag);

    SendEvent("footstep", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::PositionPlayerEvent* event,
  RE::BSTEventSource<RE::PositionPlayerEvent>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  auto e = CopyEventPtr(event);

  SkyrimPlatform::GetSingleton().AddUpdateTask([e] {
    auto obj = JsValue::Object();

    auto type = to_underlying(e->type.get());

    AddObjProperty(&obj, "eventType", type);

    SendEvent("positionPlayer", obj);
  });

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::ActorKill::Event* event, RE::BSTEventSource<RE::ActorKill::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::BooksRead::Event* event, RE::BSTEventSource<RE::BooksRead::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::CriticalHit::Event* event,
  RE::BSTEventSource<RE::CriticalHit::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::DisarmedEvent::Event* event,
  RE::BSTEventSource<RE::DisarmedEvent::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::DragonSoulsGained::Event* event,
  RE::BSTEventSource<RE::DragonSoulsGained::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::ItemHarvested::Event* event,
  RE::BSTEventSource<RE::ItemHarvested::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::LevelIncrease::Event* event,
  RE::BSTEventSource<RE::LevelIncrease::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::LocationDiscovery::Event* event,
  RE::BSTEventSource<RE::LocationDiscovery::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::ShoutAttack::Event* event,
  RE::BSTEventSource<RE::ShoutAttack::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::SkillIncrease::Event* event,
  RE::BSTEventSource<RE::SkillIncrease::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::SoulsTrapped::Event* event,
  RE::BSTEventSource<RE::SoulsTrapped::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}

EventResult EventHandler::ProcessEvent(
  const RE::SpellsLearned::Event* event,
  RE::BSTEventSource<RE::SpellsLearned::Event>*)
{
  if (!event) {
    return EventResult::kContinue;
  }

  return EventResult::kContinue;
}
