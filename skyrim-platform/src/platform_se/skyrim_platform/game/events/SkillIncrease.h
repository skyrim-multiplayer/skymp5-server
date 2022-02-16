#pragma once

namespace RE {

struct SkillIncrease
{
public:
  struct Event
  {
  public:
    // members
    RE::PlayerCharacter* player; // 00
    RE::ActorValue actorValue;   // 08
    std::uint32_t pad0C;         // 0C
  };
  static_assert(sizeof(Event) == 0x10);

  static RE::BSTEventSource<SkillIncrease::Event>* GetEventSource()
  {
    using func_t = decltype(&SkillIncrease::GetEventSource);
#ifdef SKYRIMSE
    REL::Relocation<func_t> func{ REL::ID(39248) };
#else
    REL::Relocation<func_t> func{ REL::ID(40320) };
#endif
    return func();
  }
};

}
