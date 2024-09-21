#pragma once
#include "CallNative.h" // CallNative::State

#include "NapiHelper.h"

namespace CallNativeApi {

struct NativeCallRequirements
{
  NativeCallRequirements()
  {
    gameThrQ = std::make_shared<Viet::TaskQueue>();
    jsThrQ = std::make_shared<Viet::TaskQueue>();
  }

  IVM* vm = nullptr;
  StackID stackId = std::numeric_limits<StackID>::max();

  std::shared_ptr<Viet::TaskQueue> gameThrQ, jsThrQ;
};

Napi::Value CallNative(
  const Napi::CallbackInfo& info,
  const std::function<NativeCallRequirements()>& getNativeCallRequirements);

Napi::Value DynamicCast(
  const Napi::CallbackInfo& info,
  const std::function<NativeCallRequirements()>& getNativeCallRequirements);

inline void Register(
  Napi::Env env,
  Napi::Object& exports,
  std::function<NativeCallRequirements()> getNativeCallRequirements)
{
  exports.Set("callNative", Napi::Function::New(env, NapiHelper::WrapCppExceptions([=](auto& args) {
                        return CallNative(args, getNativeCallRequirements);
                      })));
  exports.Set("dynamicCast",  Napi::Function::New(env, NapiHelper::WrapCppExceptions([=](auto& args) {
                        return DynamicCast(args, getNativeCallRequirements);
                      })));
}
}
