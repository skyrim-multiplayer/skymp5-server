#pragma once
#include "RecordHeader.h"
#include "Effects.h"

#pragma pack(push, 1)

namespace espm {

class INGR : public RecordHeader
{
public:
  static constexpr auto kType = "INGR";

  struct Data
  {
    std::vector<Effects::Effect> effects;
  };

  Data GetData(CompressedFieldsCache& compressedFieldsCache) const noexcept;
};

static_assert(sizeof(INGR) == sizeof(RecordHeader));

}

#pragma pack(pop)
