#include "Timer.h"
#include <MakeID.h-1.0.2>
#include <algorithm>
#include <chrono>
#include <deque>
#include <limits>
#include <memory>
#include <utility>

namespace Viet {

namespace {
struct TimerEntry
{
  uint32_t id;
  Promise<Void> promise;
  std::chrono::system_clock::time_point finish;
};
}

struct Timer::Impl
{
  std::deque<TimerEntry> timers;
  static const std::unique_ptr<MakeID> s_idGenerator;

  void DestroyID(const TimerEntry& entry);
};

const std::unique_ptr<MakeID> Timer::Impl::s_idGenerator =
  std::make_unique<MakeID>(std::numeric_limits<uint32_t>::max());

void Timer::Impl::DestroyID(const TimerEntry& entry)
{
  s_idGenerator->DestroyID(entry.id);
}

Timer::Timer()
{
  pImpl = std::make_shared<Impl>();
}

void Timer::TickTimers()
{
  auto now = std::chrono::system_clock::now();

  auto& timers = pImpl->timers;
  while (!timers.empty() && now >= timers.front().finish) {
    auto front = std::move(timers.front());
    timers.pop_front();
    front.promise.Resolve(Void());
    pImpl->DestroyID(front);
  }
}

bool Timer::RemoveTimer(const uint32_t timerId)
{
  auto& timers = pImpl->timers;
  for (auto it = timers.begin(); it != timers.end(); ++it) {
    uint32_t id = (*it).id;
    if (id == timerId) {
      timers.erase(it);
      return true;
    }
  }
  return false;
}

Promise<Void> Timer::Set(const std::chrono::system_clock::time_point& endTime,
                         uint32_t* outId)
{
  Promise<Void> promise;
  bool sortRequired =
    !pImpl->timers.empty() && endTime > pImpl->timers.front().finish;

  uint32_t timerId;
  Impl::s_idGenerator->CreateID(timerId);
  pImpl->timers.push_front({ timerId, promise, endTime });
  if (outId) {
    *outId = timerId;
  }

  if (sortRequired) {
    std::sort(pImpl->timers.begin(), pImpl->timers.end(),
              [](const TimerEntry& lhs, const TimerEntry& rhs) {
                return lhs.finish < rhs.finish;
              });
  }
  return promise;
}

}
