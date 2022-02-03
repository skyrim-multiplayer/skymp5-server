#pragma once

#include "EventHandlerBase.h"

using EventResult = RE::BSEventNotifyControl;

class EventHandlerStory final
  : EventHandlerBase
  , public RE::BSTEventSink<RE::ActorKill::Event>
  , public RE::BSTEventSink<RE::BooksRead::Event>
  , public RE::BSTEventSink<RE::CriticalHit::Event>
  , public RE::BSTEventSink<RE::DisarmedEvent::Event>
  , public RE::BSTEventSink<RE::DragonSoulsGained::Event>
  , public RE::BSTEventSink<RE::ItemHarvested::Event>
  , public RE::BSTEventSink<RE::LevelIncrease::Event>
  , public RE::BSTEventSink<RE::LocationDiscovery::Event>
  , public RE::BSTEventSink<RE::ShoutAttack::Event>
  , public RE::BSTEventSink<RE::SkillIncrease::Event>
  , public RE::BSTEventSink<RE::SoulsTrapped::Event>
  , public RE::BSTEventSink<RE::SpellsLearned::Event>
{
public:
  [[nodiscard]] static EventHandlerStory* GetSingleton()
  {
    static EventHandlerStory singleton;
    return &singleton;
  }

  EventResult ProcessEvent(const RE::ActorKill::Event* event,
                           RE::BSTEventSource<RE::ActorKill::Event>*) override;

  EventResult ProcessEvent(const RE::BooksRead::Event* event,
                           RE::BSTEventSource<RE::BooksRead::Event>*) override;

  EventResult ProcessEvent(
    const RE::CriticalHit::Event* event,
    RE::BSTEventSource<RE::CriticalHit::Event>*) override;

  EventResult ProcessEvent(
    const RE::DisarmedEvent::Event* event,
    RE::BSTEventSource<RE::DisarmedEvent::Event>*) override;

  EventResult ProcessEvent(
    const RE::DragonSoulsGained::Event* event,
    RE::BSTEventSource<RE::DragonSoulsGained::Event>*) override;

  EventResult ProcessEvent(
    const RE::ItemHarvested::Event* event,
    RE::BSTEventSource<RE::ItemHarvested::Event>*) override;

  EventResult ProcessEvent(
    const RE::LevelIncrease::Event* event,
    RE::BSTEventSource<RE::LevelIncrease::Event>*) override;

  EventResult ProcessEvent(
    const RE::LocationDiscovery::Event* event,
    RE::BSTEventSource<RE::LocationDiscovery::Event>*) override;

  EventResult ProcessEvent(
    const RE::ShoutAttack::Event* event,
    RE::BSTEventSource<RE::ShoutAttack::Event>*) override;

  EventResult ProcessEvent(
    const RE::SkillIncrease::Event* event,
    RE::BSTEventSource<RE::SkillIncrease::Event>*) override;

  EventResult ProcessEvent(
    const RE::SoulsTrapped::Event* event,
    RE::BSTEventSource<RE::SoulsTrapped::Event>*) override;

  EventResult ProcessEvent(
    const RE::SpellsLearned::Event* event,
    RE::BSTEventSource<RE::SpellsLearned::Event>*) override;

private:
  EventHandlerStory()
  {
    // TODO: implement these on JS side
    AppendSink<RE::ActorKill>(&std::vector{ "actorkill" });
    AppendSink<RE::BooksRead>(&std::vector{ "booksread" });
    AppendSink<RE::CriticalHit>(&std::vector{ "criticalhit" });
    AppendSink<RE::DisarmedEvent>(&std::vector{ "disarmedevent" });
    AppendSink<RE::DragonSoulsGained>(&std::vector{ "dragonsoulsgained" });
    AppendSink<RE::ItemHarvested>(&std::vector{ "itemharvested" });
    AppendSink<RE::LevelIncrease>(&std::vector{ "levelincrease" });
    AppendSink<RE::LocationDiscovery>(&std::vector{ "locationdiscovery" });
    AppendSink<RE::ShoutAttack>(&std::vector{ "shoutattack" });
    AppendSink<RE::SkillIncrease>(&std::vector{ "skillincrease" });
    AppendSink<RE::SoulsTrapped>(&std::vector{ "soulstrapped" });
    AppendSink<RE::SpellsLearned>(&std::vector{ "spellslearned" });
  };
  EventHandlerStory(const EventHandlerStory&) = delete;
  EventHandlerStory(EventHandlerStory&&) = delete;

  ~EventHandlerStory() = default;
};
