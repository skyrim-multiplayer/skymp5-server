#pragma once

#include <filesystem>

#ifdef WIN32
#  include <Windows.h>
#endif

#include "../Loader.h"
#include "IBuffer.h"

namespace espm::impl {

class MappedBuffer : public impl::IBuffer
{
public:
  MappedBuffer(const fs::path& path);

  ~MappedBuffer();

  const char* GetData() const override;

  size_t GetLength() const override;

private:
#ifdef WIN32
  HANDLE fileHandle_ = nullptr;
  HANDLE mapHandle_ = nullptr;
  LPVOID viewPtr_ = nullptr;
#else
  int fd_ = -1;
#endif
  char* data_ = nullptr;
  size_t size_;
};

} // namespace espm::impl
