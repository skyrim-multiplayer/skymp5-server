#pragma once
#include "IPapyrusClass.h"

#include "SpSnippetFunctionGen.h"

class PapyrusActor : public IPapyrusClass<PapyrusActor>
{
public:
  const char* GetName() override { return "actor"; }

  DEFINE_METHOD_SPSNIPPET(DrawWeapon);
  DEFINE_METHOD_SPSNIPPET(PlayIdle);

  VarValue IsWeaponDrawn(VarValue self,
                         const std::vector<VarValue>& arguments);

  void Register(VirtualMachine& vm,
                std::shared_ptr<IPapyrusCompatibilityPolicy> policy) override
  {
    compatibilityPolicy = policy;

    AddMethod(vm, "IsWeaponDrawn", &PapyrusActor::IsWeaponDrawn);
    AddMethod(vm, "DrawWeapon", &PapyrusActor::DrawWeapon);
    AddMethod(vm, "PlayIdle", &PapyrusActor::PlayIdle);
  }

  std::shared_ptr<IPapyrusCompatibilityPolicy> compatibilityPolicy;
};
