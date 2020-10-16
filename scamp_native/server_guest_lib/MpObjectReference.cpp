#include "MpObjectReference.h"
#include "ChangeFormGuard.h"
#include "LeveledListUtils.h"
#include "MpActor.h"
#include "MpChangeForms.h"
#include "Reader.h"
#include "ScriptStorage.h"
#include "VirtualMachine.h"
#include "WorldState.h"
#include <MsgType.h>

class OccupantDestroyEventSink : public MpActor::DestroyEventSink
{
public:
  OccupantDestroyEventSink(WorldState& wst_,
                           MpObjectReference* untrustedRefPtr_)
    : wst(wst_)
    , untrustedRefPtr(untrustedRefPtr_)
    , refId(untrustedRefPtr_->GetFormId())
  {
  }

  void BeforeDestroy(MpActor& actor) override
  {
    if (!RefStillValid())
      return;
    if (untrustedRefPtr->occupant == &actor) {
      untrustedRefPtr->SetOpen(false);
      untrustedRefPtr->occupant = nullptr;
    }
  }

private:
  bool RefStillValid() const
  {
    return untrustedRefPtr == wst.LookupFormById(refId).get();
  }

  WorldState& wst;
  MpObjectReference* const untrustedRefPtr;
  const uint32_t refId;
};

namespace {
std::pair<int16_t, int16_t> GetGridPos(const NiPoint3& pos) noexcept
{
  return { int16_t(pos.x / 4096), int16_t(pos.y / 4096) };
}
}

struct MpObjectReference::Impl
  : public ChangeFormGuard<MpChangeFormREFR>
  , public std::enable_shared_from_this<Impl>
  , public IGameObject
{
public:
  Impl(MpChangeFormREFR changeForm_, MpObjectReference* self_)
    : ChangeFormGuard(changeForm_, self_)
  {
    stringId = "[ObjectReference " + ChangeForm().formDesc.ToString() + "]";
  }

  std::shared_ptr<IGameObject> GetGameObject() { return shared_from_this(); }

  const char* GetStringID() override { return stringId.data(); };

  std::unique_ptr<VirtualMachine> vm;
  std::string stringId;
};

MpObjectReference::MpObjectReference(const LocationalData& locationalData_,
                                     const FormCallbacks& callbacks_,
                                     uint32_t baseId_, const char* baseType_)
  : callbacks(new FormCallbacks(callbacks_))
  , baseId(baseId_)
  , baseType(baseType_)
{
  MpChangeFormREFR changeForm;
  changeForm.position = locationalData_.pos;
  changeForm.angle = locationalData_.rot;
  changeForm.worldOrCell = locationalData_.cellOrWorld;
  pImpl.reset(new Impl{ changeForm, this });

  if (!strcmp(baseType_, "FLOR") || !strcmp(baseType_, "TREE")) {
    relootTime = std::chrono::hours(1);
  } else if (!strcmp(baseType_, "DOOR")) {
    relootTime = std::chrono::seconds(3);
  } else if (espm::IsItem(baseType_)) {
    relootTime = std::chrono::hours(1);
  } else if (!strcmp(baseType_, "CONT")) {
    relootTime = std::chrono::hours(1);
  }
}

const NiPoint3& MpObjectReference::GetPos() const
{
  return pImpl->ChangeForm().position;
}

const NiPoint3& MpObjectReference::GetAngle() const
{
  return pImpl->ChangeForm().angle;
}

const uint32_t& MpObjectReference::GetCellOrWorld() const
{
  return pImpl->ChangeForm().worldOrCell;
}

const uint32_t& MpObjectReference::GetBaseId() const
{
  return baseId;
}

const Inventory& MpObjectReference::GetInventory() const
{
  return pImpl->ChangeForm().inv;
}

const bool& MpObjectReference::IsHarvested() const
{
  return pImpl->ChangeForm().isHarvested;
}

const bool& MpObjectReference::IsOpen() const
{
  return pImpl->ChangeForm().isOpen;
}

const std::chrono::milliseconds& MpObjectReference::GetRelootTime() const
{
  return relootTime;
}

void MpObjectReference::VisitProperties(const PropertiesVisitor& visitor)
{
  if (IsHarvested())
    visitor("isHarvested", "true");
  if (IsOpen())
    visitor("isOpen", "true");
}

void MpObjectReference::SetPos(const NiPoint3& newPos)
{
  auto& grid = GetParent()->grids[GetCellOrWorld()];

  auto oldGridPos = GetGridPos(pImpl->ChangeForm().position);
  auto newGridPos = GetGridPos(newPos);

  pImpl->EditChangeForm(
    [&newPos](MpChangeFormREFR& changeForm) { changeForm.position = newPos; });

  if (oldGridPos != newGridPos || !everSubscribedOrListened) {
    everSubscribedOrListened = true;

    InitListenersAndEmitters();

    MoveOnGrid(grid);

    auto& was = *this->listeners;
    auto& now = grid.GetNeighboursAndMe(this);

    std::vector<MpObjectReference*> toRemove;
    std::set_difference(was.begin(), was.end(), now.begin(), now.end(),
                        std::inserter(toRemove, toRemove.begin()));
    for (auto listener : toRemove) {
      Unsubscribe(this, listener);
      if (this != listener)
        Unsubscribe(listener, this);
    }

    std::vector<MpObjectReference*> toAdd;
    std::set_difference(now.begin(), now.end(), was.begin(), was.end(),
                        std::inserter(toAdd, toAdd.begin()));
    for (auto listener : toAdd) {
      Subscribe(this, listener);
      if (this != listener)
        Subscribe(listener, this);
    }
  }
}

void MpObjectReference::SetAngle(const NiPoint3& newAngle)
{
  pImpl->EditChangeForm(
    [&](MpChangeFormREFR& changeForm) { changeForm.angle = newAngle; });
}

void MpObjectReference::SetHarvested(bool harvested)
{
  if (harvested != pImpl->ChangeForm().isHarvested) {
    pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
      changeForm.isHarvested = harvested;
    });
    SendPropertyToListeners("isHarvested", harvested);
  }
}

void MpObjectReference::SetOpen(bool open)
{
  if (open != pImpl->ChangeForm().isOpen) {
    pImpl->EditChangeForm(
      [&](MpChangeFormREFR& changeForm) { changeForm.isOpen = open; });
    SendPropertyToListeners("isOpen", open);
  }
}

void MpObjectReference::Activate(MpActor& activationSource)
{
  std::cout << "Activate " << this->GetFormId() << " by "
            << activationSource.GetFormId() << std::endl;

  auto& loader = GetParent()->GetEspm();
  auto& compressedFieldsCache = GetParent()->GetEspmCache();

  CheckInteractionAbility(activationSource);

  auto base = loader.GetBrowser().LookupById(GetBaseId());
  if (!base.rec || !GetBaseId()) {
    std::stringstream ss;
    ss << std::hex << GetFormId() << " doesn't have base form";
    throw std::runtime_error(ss.str());
  }

  auto t = base.rec->GetType();

  if (t == espm::TREE::type || t == espm::FLOR::type || espm::IsItem(t)) {
    if (!IsHarvested()) {
      auto mapping = loader.GetBrowser().GetMapping(base.fileIdx);
      uint32_t resultItem = 0;
      if (t == espm::TREE::type) {
        espm::FLOR::Data data;
        data = espm::Convert<espm::TREE>(base.rec)->GetData();
        resultItem = espm::GetMappedId(data.resultItem, *mapping);
      } else if (t == espm::FLOR::type) {
        espm::FLOR::Data data;
        data = espm::Convert<espm::FLOR>(base.rec)->GetData();
        resultItem = espm::GetMappedId(data.resultItem, *mapping);
      } else {
        resultItem = espm::GetMappedId(base.rec->GetId(), *mapping);
      }

      activationSource.AddItem(resultItem, 1);
      SetHarvested(true);
      RequestReloot();
    }
  } else if (t == espm::DOOR::type) {

    auto refrRecord = espm::Convert<espm::REFR>(
      loader.GetBrowser().LookupById(GetFormId()).rec);
    auto teleport = refrRecord->GetData().teleport;
    if (teleport) {
      if (!IsOpen()) {
        SetOpen(true);
        RequestReloot();
      }

      auto destinationRecord = espm::Convert<espm::REFR>(
        loader.GetBrowser().LookupById(teleport->destinationDoor).rec);
      if (!destinationRecord)
        throw std::runtime_error(
          "No destination found for this teleport door");

      auto teleportWorldOrCell = espm::GetWorldOrCell(destinationRecord);

      static const auto g_pi = std::acos(-1.f);
      std::string msg;
      msg += Networking::MinPacketId;
      msg += nlohmann::json{
        { "pos", { teleport->pos[0], teleport->pos[1], teleport->pos[2] } },
        { "rot",
          { teleport->rotRadians[0] / g_pi * 180,
            teleport->rotRadians[1] / g_pi * 180,
            teleport->rotRadians[2] / g_pi * 180 } },
        { "worldOrCell", teleportWorldOrCell },
        { "type", "teleport" }
      }.dump();
      activationSource.SendToUser(msg.data(), msg.size(), true);

      activationSource.SetCellOrWorld(teleportWorldOrCell);

    } else {
      SetOpen(!IsOpen());
    }
  } else if (t == espm::CONT::type) {
    EnsureBaseContainerAdded(loader);
    if (!this->occupant) {
      SetOpen(true);
      SendPropertyTo("inventory", GetInventory().ToJson(), activationSource);
      activationSource.SendOpenContainer(GetFormId());

      this->occupant = &activationSource;

      this->occupantDestroySink.reset(
        new OccupantDestroyEventSink(*GetParent(), this));
      this->occupant->AddEventSink(occupantDestroySink);
    } else if (this->occupant == &activationSource) {
      SetOpen(false);
      this->occupant->RemoveEventSink(this->occupantDestroySink);
      this->occupant = nullptr;
    }
  }

  if (pImpl->vm) {
    /*std::vector<VarValue> arguments;
    pImpl->vm->SendEvent(pImpl->GetGameObject(), "OnReset", arguments);*/

    std::vector<VarValue> activateArguments{ VarValue::None() };
    pImpl->vm->SendEvent(pImpl->GetGameObject(), "OnActivate",
                         activateArguments);
  }
}

void MpObjectReference::PutItem(MpActor& ac, const Inventory::Entry& e)
{
  std::cout << "PutItem into " << this->GetFormId() << " by " << ac.GetFormId()
            << std::endl;

  CheckInteractionAbility(ac);
  if (this->occupant != &ac) {
    std::stringstream err;
    err << std::hex << "Actor 0x" << ac.GetFormId() << " doesn't occupy ref 0x"
        << GetFormId();
    throw std::runtime_error(err.str());
  }
  ac.RemoveItems({ e }, this);
}

void MpObjectReference::TakeItem(MpActor& ac, const Inventory::Entry& e)
{
  std::cout << "TakeItem from " << this->GetFormId() << " by "
            << ac.GetFormId() << std::endl;

  CheckInteractionAbility(ac);
  if (this->occupant != &ac) {
    std::stringstream err;
    err << std::hex << "Actor 0x" << ac.GetFormId() << " doesn't occupy ref 0x"
        << GetFormId();
    throw std::runtime_error(err.str());
  }
  RemoveItems({ e }, &ac);

  if (GetInventory().IsEmpty()) {
    RequestReloot();
  }
}

void MpObjectReference::SetRelootTime(std::chrono::milliseconds newRelootTime)
{
  relootTime = newRelootTime;
}

void MpObjectReference::SetCellOrWorld(uint32_t newWorldOrCell)
{
  everSubscribedOrListened = false;
  auto& grid = GetParent()->grids[pImpl->ChangeForm().worldOrCell];
  grid.Forget(this);

  pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
    changeForm.worldOrCell = newWorldOrCell;
  });
}

void MpObjectReference::SetChanceNoneOverride(uint8_t newChanceNone)
{
  chanceNoneOverride.reset(new uint8_t(newChanceNone));
}

void MpObjectReference::AddItem(uint32_t baseId, uint32_t count)
{
  pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
    changeForm.baseContainerAdded = true;
    changeForm.inv.AddItem(baseId, count);
  });
  SendInventoryUpdate();
}

void MpObjectReference::AddItems(const std::vector<Inventory::Entry>& entries)
{
  if (entries.size() > 0) {
    pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
      changeForm.baseContainerAdded = true;
      changeForm.inv.AddItems(entries);
    });
    SendInventoryUpdate();
  }
}

void MpObjectReference::RemoveItems(
  const std::vector<Inventory::Entry>& entries, MpObjectReference* target)
{
  pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
    changeForm.inv.RemoveItems(entries);
  });

  if (target)
    target->AddItems(entries);

  SendInventoryUpdate();
}

void MpObjectReference::RelootContainer()
{
  pImpl->EditChangeForm(
    [&](MpChangeFormREFR& changeForm) {
      changeForm.baseContainerAdded = false;
    },
    Impl::Mode::NoRequestSave);
  EnsureBaseContainerAdded(*GetParent()->espm);
}

void MpObjectReference::Subscribe(MpObjectReference* emitter,
                                  MpObjectReference* listener)
{
  bool bothNonActors =
    !dynamic_cast<MpActor*>(emitter) && !dynamic_cast<MpActor*>(listener);
  if (bothNonActors)
    return;

  emitter->InitListenersAndEmitters();
  listener->InitListenersAndEmitters();
  emitter->listeners->insert(listener);
  listener->emitters->insert(emitter);
  emitter->callbacks->subscribe(emitter, listener);
}

void MpObjectReference::Unsubscribe(MpObjectReference* emitter,
                                    MpObjectReference* listener)
{
  bool bothNonActors =
    !dynamic_cast<MpActor*>(emitter) && !dynamic_cast<MpActor*>(listener);
  if (bothNonActors)
    return;

  emitter->callbacks->unsubscribe(emitter, listener);
  emitter->listeners->erase(listener);
  listener->emitters->erase(emitter);
}

const std::set<MpObjectReference*>& MpObjectReference::GetListeners() const
{
  static const std::set<MpObjectReference*> g_emptyListeners;
  return listeners ? *listeners : g_emptyListeners;
}

const std::set<MpObjectReference*>& MpObjectReference::GetEmitters() const
{
  static const std::set<MpObjectReference*> g_emptyEmitters;
  return emitters ? *emitters : g_emptyEmitters;
}

void MpObjectReference::RequestReloot()
{
  if (!pImpl->ChangeForm().nextRelootDatetime) {
    pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
      changeForm.nextRelootDatetime = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + GetRelootTime());
    });

    GetParent()->RequestReloot(*this);
  }
}

void MpObjectReference::DoReloot()
{
  if (pImpl->ChangeForm().nextRelootDatetime) {
    pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
      changeForm.nextRelootDatetime = 0;
    });
    SetOpen(false);
    SetHarvested(false);
    RelootContainer();
  }
}

std::shared_ptr<std::chrono::time_point<std::chrono::system_clock>>
MpObjectReference::GetNextRelootMoment() const
{
  std::shared_ptr<std::chrono::time_point<std::chrono::system_clock>> res;
  if (pImpl->ChangeForm().nextRelootDatetime)
    res.reset(new std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::system_clock::from_time_t(
        pImpl->ChangeForm().nextRelootDatetime)));
  return res;
}

MpChangeForm MpObjectReference::GetChangeForm() const
{
  MpChangeForm res;
  static_cast<MpChangeFormREFR&>(res) = pImpl->ChangeForm();

  if (!GetParent()->espmFiles.empty()) {
    res.formDesc = FormDesc::FromFormId(GetFormId(), GetParent()->espmFiles);
    res.baseDesc = FormDesc::FromFormId(GetBaseId(), GetParent()->espmFiles);
  } else
    res.formDesc = res.baseDesc = FormDesc(GetFormId(), "");

  return res;
}

void MpObjectReference::ApplyChangeForm(const MpChangeForm& changeForm)
{
  const auto currentBaseId = GetBaseId();
  const auto newBaseId = changeForm.baseDesc.ToFormId(GetParent()->espmFiles);
  if (currentBaseId != newBaseId) {
    std::stringstream ss;
    ss << "Anomally, baseId should never change (";
    ss << std::hex << currentBaseId << " => " << newBaseId << ")";
    throw std::runtime_error(ss.str());
  }

  if (pImpl->ChangeForm().formDesc != changeForm.formDesc) {
    throw std::runtime_error("Expected formDesc to be " +
                             pImpl->ChangeForm().formDesc.ToString() +
                             ", but found " + changeForm.formDesc.ToString());
  }

  // Perform all required grid operations
  SetCellOrWorld(changeForm.worldOrCell);
  SetPos(changeForm.position);

  pImpl->EditChangeForm(
    [&](MpChangeFormREFR& f) {
      f = static_cast<const MpChangeFormREFR&>(changeForm);
    },
    Impl::Mode::NoRequestSave);

  if (changeForm.nextRelootDatetime) {
    const auto tp =
      std::chrono::system_clock::from_time_t(changeForm.nextRelootDatetime);

    if (tp < std::chrono::system_clock::now()) {
      const auto prevRelootTime = GetRelootTime();
      SetRelootTime(std::chrono::duration_cast<std::chrono::milliseconds>(
        tp - std::chrono::system_clock::now()));
      RequestReloot();
      SetRelootTime(prevRelootTime);
    }
  }
}

void MpObjectReference::Init(WorldState* parent, uint32_t formId)
{
  MpForm::Init(parent, formId);

  auto& grid = GetParent()->grids[pImpl->ChangeForm().worldOrCell];
  MoveOnGrid(grid);

  // We should queue created form for saving as soon as it is initialized
  const auto mode =
    formId >= 0xff000000 ? Impl::Mode::RequestSave : Impl::Mode::NoRequestSave;

  pImpl->EditChangeForm(
    [&](MpChangeFormREFR& changeForm) {
      changeForm.formDesc =
        FormDesc::FromFormId(formId, GetParent()->espmFiles);
    },
    mode);

  InitScripts();
}

namespace {
/*VarValue CastProperty(const espm::Property& prop)
{
  switch (prop.propertyType) {
    case espm::PropertyType::Object:
      break;
    case espm::PropertyType::String:
      break;
    case espm::PropertyType::Int:
      break;
    case espm::PropertyType::Float:
      break;
    case espm::PropertyType::Bool:
      break;
  }
}*/

VarForBuildActivePex BuildScriptProperties(const espm::ScriptData& scriptData)
{
  VarForBuildActivePex res;
  for (auto& entry : scriptData.scripts) {
    auto& resultProps = res[entry.scriptName];
    for (auto& prop : entry.properties) {
      // resultProps.push_back;
    }
  }
  return res;
}
}

class MpFormGameObject : public IGameObject
{
public:
  MpFormGameObject(MpForm* form_)
    : form(form_)
    , parent(form_ ? form_->GetParent() : nullptr)
    , formId(form_ ? form_->GetFormId() : 0)
  {
  }

  MpForm* GetFormPtr() const noexcept
  {
    bool formStillValid = parent->LookupFormById(formId).get() == form;
    if (!formStillValid)
      return nullptr;
    return form;
  }

private:
  WorldState* const parent;
  MpForm* const form;
  const uint32_t formId;
};

class EspmGameObject : public IGameObject
{
public:
  EspmGameObject(const espm::LookupResult& record_)
    : record(record_)
  {
  }

  const espm::LookupResult record;
};

template <class T>
T* GetFormPtr(const VarValue& papyrusObject)
{
  if (papyrusObject.GetType() != VarValue::kType_Object)
    return nullptr;
  auto gameObject = static_cast<IGameObject*>(papyrusObject);
  auto mpFormGameObject = dynamic_cast<MpFormGameObject*>(gameObject);
  if (!mpFormGameObject)
    return nullptr;
  return dynamic_cast<T*>(mpFormGameObject->GetFormPtr());
}

const espm::LookupResult& GetRecordPtr(VarValue papyrusObject)
{
  static const espm::LookupResult emptyResult;

  if (papyrusObject.GetType() != VarValue::kType_Object)
    return emptyResult;
  auto gameObject = static_cast<IGameObject*>(papyrusObject);
  auto espmGameObject = dynamic_cast<EspmGameObject*>(gameObject);
  if (!espmGameObject)
    return emptyResult;
  return espmGameObject->record;
}

namespace PapyrusObjectReference {
VarValue IsDisabled(VarValue self, const std::vector<VarValue>& arguments)
{
  return VarValue(false);
}

VarValue GetScale(VarValue self, const std::vector<VarValue>& arguments)
{
  return VarValue(1.f);
}

VarValue SetScale(VarValue self, const std::vector<VarValue>& arguments)
{
  return VarValue::None();
}

VarValue EnableNoWait(VarValue self, const std::vector<VarValue>& arguments)
{
  return VarValue::None();
}

VarValue AddItem(VarValue self, const std::vector<VarValue>& arguments)
{
  if (arguments.size() < 2)
    return VarValue::None();
  auto item = GetRecordPtr(arguments[0]);
  auto count = static_cast<int>(arguments[1]);
  auto selfRefr = GetFormPtr<MpObjectReference>(self);
  if (!selfRefr || !item.rec || count <= 0)
    return VarValue::None();

  auto itemId = item.ToGlobalId(item.rec->GetId());
  selfRefr->AddItem(itemId, count);

  return VarValue::None();
}

void Register(VirtualMachine& vm)
{
  vm.RegisterFunction("objectreference", "IsDisabled", FunctionType::Method,
                      IsDisabled);
  vm.RegisterFunction("objectreference", "GetScale", FunctionType::Method,
                      GetScale);
  vm.RegisterFunction("objectreference", "SetScale", FunctionType::Method,
                      SetScale);
  vm.RegisterFunction("objectreference", "EnableNoWait", FunctionType::Method,
                      SetScale);
  vm.RegisterFunction("objectreference", "AddItem", FunctionType::Method,
                      SetScale);
}
}

void MpObjectReference::InitScripts()
{
  auto baseId = GetBaseId();
  if (!baseId || !GetParent()->espm)
    return;

  auto& br = GetParent()->espm->GetBrowser();
  auto base = br.LookupById(baseId);
  if (!base.rec)
    return;

  auto scriptStorage = GetParent()->GetScriptStorage();
  if (!scriptStorage)
    return;

  espm::ScriptData scriptData;
  base.rec->GetScriptData(&scriptData);

  std::vector<std::shared_ptr<PexScript>> pexStructures;
  std::vector<std::string> scriptNames;

  for (auto& script : scriptData.scripts) {
    scriptNames.push_back(script.scriptName);
    auto pex = scriptStorage->GetScriptPex(script.scriptName.data());
    if (!pex.empty()) {

      std::vector<std::vector<uint8_t>> pexVec = { pex };

      for (auto required : { "game", "form", "objectreference" }) {
        auto requiredPex = scriptStorage->GetScriptPex(required);
        if (requiredPex.empty())
          throw std::runtime_error("'" + std::string(required) +
                                   "' is a strictly required script");
        pexVec.push_back(requiredPex);
      }

      auto pexStructure = Reader(pexVec).GetSourceStructures();
      for (auto& v : pexStructure)
        pexStructures.push_back(v);
    }
  }

  if (!pexStructures.empty()) {
    pImpl->vm.reset(new VirtualMachine(pexStructures));

    PapyrusObjectReference::Register(*pImpl->vm);

    pImpl->vm->AddObject(pImpl->GetGameObject(), scriptNames,
                         BuildScriptProperties(scriptData));
  }
}

void MpObjectReference::MoveOnGrid(GridImpl<MpObjectReference*>& grid)
{
  auto newGridPos = GetGridPos(GetPos());
  grid.Move(this, newGridPos.first, newGridPos.second);
}

void MpObjectReference::InitListenersAndEmitters()
{
  if (!listeners) {
    listeners.reset(new std::set<MpObjectReference*>);
    emitters.reset(new std::set<MpObjectReference*>);
  }
}

void MpObjectReference::SendInventoryUpdate()
{
  auto actor = dynamic_cast<MpActor*>(this);
  if (actor) {
    std::string msg;
    msg += Networking::MinPacketId;
    msg += nlohmann::json{
      { "inventory", actor->GetInventory().ToJson() },
      { "type", "setInventory" }
    }.dump();
    actor->SendToUser(msg.data(), msg.size(), true);
  }
}

void MpObjectReference::SendOpenContainer(uint32_t targetId)
{
  auto actor = dynamic_cast<MpActor*>(this);
  if (actor) {
    std::string msg;
    msg += Networking::MinPacketId;
    msg += nlohmann::json{
      { "target", targetId }, { "type", "openContainer" }
    }.dump();
    actor->SendToUser(msg.data(), msg.size(), true);
  }
}

void MpObjectReference::EnsureBaseContainerAdded(espm::Loader& espm)
{
  if (pImpl->ChangeForm().baseContainerAdded)
    return;

  constexpr uint32_t pcLevel = 1;

  std::map<uint32_t, uint32_t> itemsToAdd;

  auto lookupRes = espm.GetBrowser().LookupById(GetBaseId());
  auto baseContainer = espm::Convert<espm::CONT>(lookupRes.rec);
  if (baseContainer) {
    auto data = baseContainer->GetData();
    for (auto& entry : data.objects) {
      auto formLookupRes = espm.GetBrowser().LookupById(entry.formId);
      auto leveledItem = espm::Convert<espm::LVLI>(formLookupRes.rec);
      if (leveledItem) {
        auto map = LeveledListUtils::EvaluateListRecurse(
          espm.GetBrowser(), formLookupRes, 1, pcLevel,
          chanceNoneOverride.get());
        for (auto& p : map)
          itemsToAdd[p.first] += p.second;
      } else
        itemsToAdd[entry.formId] += entry.count;
    }
  }

  std::vector<Inventory::Entry> entries;
  for (auto& p : itemsToAdd)
    entries.push_back({ p.first, p.second });
  AddItems(entries);

  if (!pImpl->ChangeForm().baseContainerAdded) {
    pImpl->EditChangeForm([&](MpChangeFormREFR& changeForm) {
      changeForm.baseContainerAdded = true;
    });
  }
}

void MpObjectReference::CheckInteractionAbility(MpActor& ac)
{
  auto& loader = GetParent()->GetEspm();
  auto& compressedFieldsCache = GetParent()->GetEspmCache();

  auto casterWorld = loader.GetBrowser().LookupById(ac.GetCellOrWorld()).rec;
  auto targetWorld = loader.GetBrowser().LookupById(GetCellOrWorld()).rec;

  if (targetWorld != casterWorld) {
    const char* casterWorldName =
      casterWorld ? casterWorld->GetEditorId(&compressedFieldsCache) : "";

    const char* targetWorldName =
      targetWorld ? targetWorld->GetEditorId(&compressedFieldsCache) : "";
    std::stringstream ss;
    ss << "WorldSpace doesn't match: caster is in " << casterWorldName
       << ", target is in " << targetWorldName;
    throw std::runtime_error(ss.str());
  }
}

namespace {
std::string CreatePropertyMessage(MpObjectReference* self, const char* name,
                                  const nlohmann::json& value)
{
  nlohmann::json j{ { "idx", self->GetIdx() },
                    { "t", MsgType::UpdateProperty },
                    { "propName", name },
                    { "data", value } };
  std::string str;
  str += Networking::MinPacketId;
  str += j.dump();
  return str;
}
}

void MpObjectReference::SendPropertyToListeners(const char* name,
                                                const nlohmann::json& value)
{
  auto str = CreatePropertyMessage(this, name, value);
  for (auto listener : GetListeners()) {
    auto listenerAsActor = dynamic_cast<MpActor*>(listener);
    if (listenerAsActor)
      listenerAsActor->SendToUser(str.data(), str.size(), true);
  }
}

void MpObjectReference::SendPropertyTo(const char* name,
                                       const nlohmann::json& value,
                                       MpActor& target)
{
  auto str = CreatePropertyMessage(this, name, value);
  target.SendToUser(str.data(), str.size(), true);
}

void MpObjectReference::BeforeDestroy()
{
  if (this->occupant && this->occupantDestroySink)
    this->occupant->RemoveEventSink(this->occupantDestroySink);

  MpForm::BeforeDestroy();

  GetParent()->grids[GetCellOrWorld()].Forget(this);

  auto listenersCopy = GetListeners();
  for (auto listener : listenersCopy)
    if (this != listener)
      Unsubscribe(this, listener);
}