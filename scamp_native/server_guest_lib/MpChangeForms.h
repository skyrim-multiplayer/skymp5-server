#pragma once
#include "Equipment.h"
#include "FormDesc.h"
#include "Inventory.h"
#include "Look.h"
#include <cstdint>
#include <optional>
#include <string>

class MpObjectReference;
class WorldState;

class MpChangeFormREFR
{
public:
  FormDesc formDesc;
  float pos[3] = { 0, 0, 0 };
  float rot[3] = { 0, 0, 0 };
  uint32_t worldOrCell = 0;
  Inventory inv;
  bool isHarvested = false;
  bool isOpen = false;
  uint64_t nextRelootDatetime = 0;
};

class MpChangeFormACHR
{
public:
  bool isRaceMenuOpen = false;
  std::optional<Look> look;

  // "unexisting" equipment and equipment with zero entries are different
  // values in skymp due to poor design
  std::optional<Equipment> equipment;
};

class MpChangeForm
  : public MpChangeFormREFR
  , public MpChangeFormACHR
{
public:
  static void Load(MpChangeForm source, WorldState* parentWorldState);

  static MpChangeForm Save(MpObjectReference* refr,
                           WorldState* parentWorldState);

  std::string GetInventory() const;
  void SetInventory(const std::string& inventoryDump);

  std::string GetEquipment() const;
  void SetEquipment(const std::string& equipmentDump);

  std::string GetFormDesc() const;
  void SetFormDesc(const std::string& formDesc);

  std::string GetLook() const;
  void SetLook(const std::string& lookDump);
};