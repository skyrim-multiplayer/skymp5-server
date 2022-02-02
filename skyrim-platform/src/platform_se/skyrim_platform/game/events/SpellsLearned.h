#pragma once

namespace RE {

struct SpellsLearned
{
public:
  struct Event
  {
  public:
    // members
    RE::SpellItem* spell; // 00
  };
  static_assert(sizeof(Event) == 0x8);

  static RE::BSTEventSource<SpellsLearned::Event>* GetEventSource()
  {
    using func_t = decltype(&SpellsLearned::GetEventSource);
    REL::Relocation<func_t> func{ REL::ID(38874) };
    return func();
  }
  static void SendEvent(SpellItem* spell)
  {
    Event e = { spell };
    auto source = GetEventSource();
    if (source) {
      source->SendEvent(std::addressof(e));
    }
  }
};

}
