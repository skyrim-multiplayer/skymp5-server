#pragma once
#include <cstdint>

class ScriptHeaderView
{
public:
  ScriptHeaderView(const uint8_t* data_, size_t dataSize_,
                   const StringTable* stringTable_) noexcept;

  enum
  {
    kSignature = 0xFA57C0DE,
    kVerMajor = 0x03,
    kVerMinor = 0x01,
    kGameID = 0x0001,
  };

  uint32_t GetSignature() const noexcept;
  uint8_t GetVerMajor() const noexcept;
  uint8_t GetVerMinor() const noexcept;
  uint16_t GetGameID() const noexcept;
  uint64_t GetBuildTime() const noexcept;

private:
  const uint8_t* data = nullptr;
  size_t dataSize = 0;
  const StringTable* stringTable = nullptr;
};

struct ScriptHeader
{
  enum
  {
    kSignature = 0xFA57C0DE,
    kVerMajor = 0x03,
    kVerMinor = 0x01,
    kGameID = 0x0001,
  };

  uint32_t Signature = 0; // 00	FA57C0DE
  uint8_t VerMajor = 0;   // 04	03
  uint8_t VerMinor = 0;   // 05	01
  uint16_t GameID = 0;    // 06	0001
  uint64_t BuildTime = 0; // 08	uint64_t
};
