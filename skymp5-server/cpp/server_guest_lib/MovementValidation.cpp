#include "MovementValidation.h"
#include "FormDesc.h"
#include "NetworkingInterface.h"
#include "NiPoint3.h"
#include <nlohmann/json.hpp>
#include <string>

bool MovementValidation::Validate(const NiPoint3& currentPos,
                                  const NiPoint3& currentRot,
                                  const FormDesc& currentCellOrWorld,
                                  const NiPoint3& newPos,
                                  const FormDesc& newCellOrWorld,
                                  IMessageOutput& tgt,
                                  const std::vector<std::string>& espmFiles)
{
  float maxDistance = 4096;
  if (currentCellOrWorld != newCellOrWorld ||
      (currentPos - newPos).Length() >= maxDistance) {
    std::string s;
    s += Networking::MinPacketId;
    s += nlohmann::json{
      { "type", "teleport" },
      { "pos", { currentPos[0], currentPos[1], currentPos[2] } },
      { "rot", { currentRot[0], currentRot[1], currentRot[2] } },
      { "worldOrCell", currentCellOrWorld.ToFormId(espmFiles) }
    }.dump();
    tgt.Send(reinterpret_cast<uint8_t*>(s.data()), s.length(), true);
    return false;
  }
  return true;
}
