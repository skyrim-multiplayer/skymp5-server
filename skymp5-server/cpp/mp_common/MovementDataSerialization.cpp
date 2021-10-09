#include "MovementDataSerialization.h"

#include <nlohmann/json.hpp>
#include <slikenet/BitStream.h>

#include "MsgType.h"
#include "SerializationUtil/BitStreamUtil.h"

// namespace serialization {

void WriteToBitStream(SLNet::BitStream& stream, const MovementData& movData)
{
  using SerializationUtil::WriteToBitStream;

  WriteToBitStream(stream, movData.idx);
  WriteToBitStream(stream, movData.worldOrCell);
  WriteToBitStream(stream, movData.pos);
  WriteToBitStream(stream, movData.rot);
  WriteToBitStream(stream, movData.direction);
  WriteToBitStream(stream, movData.healthPercentage);

  WriteToBitStream(stream, static_cast<bool>(static_cast<uint8_t>(movData.runMode) & 2));
  WriteToBitStream(stream, static_cast<bool>(static_cast<uint8_t>(movData.runMode) & 1));

  WriteToBitStream(stream, movData.isInJumpState);
  WriteToBitStream(stream, movData.isSneaking);
  WriteToBitStream(stream, movData.isBlocking);
  WriteToBitStream(stream, movData.isWeapDrawn);
  WriteToBitStream(stream, movData.lookAt);
}

void ReadFromBitStream(SLNet::BitStream& stream, MovementData& movData)
{
  using SerializationUtil::ReadFromBitStream;

  ReadFromBitStream(stream, movData.idx);
  ReadFromBitStream(stream, movData.worldOrCell);
  ReadFromBitStream(stream, movData.pos);
  ReadFromBitStream(stream, movData.rot);
  ReadFromBitStream(stream, movData.direction);
  ReadFromBitStream(stream, movData.healthPercentage);

  uint8_t runMode = 0;
  runMode |= static_cast<uint8_t>(ReadFromBitStream<bool>(stream));
  runMode <<= 1;
  runMode |= static_cast<uint8_t>(ReadFromBitStream<bool>(stream));
  movData.runMode = static_cast<RunMode>(runMode);

  ReadFromBitStream(stream, movData.isInJumpState);
  ReadFromBitStream(stream, movData.isSneaking);
  ReadFromBitStream(stream, movData.isBlocking);
  ReadFromBitStream(stream, movData.isWeapDrawn);
  ReadFromBitStream(stream, movData.lookAt);
}

MovementData MovementDataFromJson(const nlohmann::json& json)
{
  MovementData result;
  result.idx = json.at("idx").get<uint32_t>();
  result.worldOrCell = json.at("data").at("worldOrCell").get<uint32_t>();
  result.pos = json.at("data").at("pos").get<std::array<float, 3>>();
  result.rot = json.at("data").at("rot").get<std::array<float, 3>>();
  result.direction = json.at("data").at("direction").get<float>();
  result.healthPercentage = json.at("data").at("healthPercentage").get<float>();
  result.runMode =
    RunModeFromString(json.at("data").at("runMode").get<std::string_view>());
  result.isInJumpState = json.at("data").at("isInJumpState").get<bool>();
  result.isSneaking = json.at("data").at("isSneaking").get<bool>();
  result.isBlocking = json.at("data").at("isBlocking").get<bool>();
  result.isWeapDrawn = json.at("data").at("isWeapDrawn").get<bool>();
  const auto& data = json.at("data");
  const auto lookAtIt = data.find("lookAt");
  if (lookAtIt != data.end()) {
    result.lookAt = lookAtIt->get<std::array<float, 3>>();
  }
  return result;
}

nlohmann::json MovementDataToJson(const MovementData& movData)
{
  auto result = nlohmann::json{
    { "t", MsgType::UpdateMovement },
    { "idx", movData.idx },
    {
      "data",
      {
        { "worldOrCell", movData.worldOrCell },
        { "pos", movData.pos },
        { "rot", movData.rot },
        { "runMode", ToString(movData.runMode) },
        { "direction", movData.direction },
        { "healthPercentage", movData.healthPercentage },
        { "isInJumpState", movData.isInJumpState },
        { "isSneaking", movData.isSneaking },
        { "isBlocking", movData.isBlocking },
        { "isWeapDrawn", movData.isWeapDrawn },
      },
    },
  };
  if (movData.lookAt) {
    result["data"]["lookAt"] = *movData.lookAt;
  }
  return result;
}

// }
