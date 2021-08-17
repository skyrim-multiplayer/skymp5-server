#pragma once

#include <filesystem>

#include "../Loader.h"
#include "IBuffer.h"

namespace espm::impl {

class AllocatedBuffer : public IBuffer
{
public:
  AllocatedBuffer(const std::filesystem::path& path);

  const char* GetData() const override;

  size_t GetLength() const override;

private:
  std::vector<char> data;
};

} // namespace espm::impl
