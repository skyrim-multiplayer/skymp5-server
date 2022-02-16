#pragma once

namespace RE {

struct LevelIncrease
{
public:
  struct Event
  {
  public:
    // members
    RE::PlayerCharacter* player; // 00
    std::uint16_t newLevel;      // 08
    std::uint16_t pad0A;         // 0A
    std::uint32_t pad0C;         // 0C
  };
  static_assert(sizeof(Event) == 0x10);

  static RE::BSTEventSource<LevelIncrease::Event>* GetEventSource()
  {
    using func_t = decltype(&LevelIncrease::GetEventSource);
#ifdef SKYRIMSE
    REL::Relocation<func_t> func{ REL::ID(39247) };
#else
    REL::Relocation<func_t> func{ REL::ID(40319) };
#endif
    return func();
  }
};

}
