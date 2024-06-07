#pragma once
#include "IDatabase.h"
#include <spdlog/spdlog.h>

class FileDatabase : public IDatabase
{
public:
  FileDatabase(std::string directory_,
               std::shared_ptr<spdlog::logger> logger_);

  UpsertResult Upsert(
    std::vector<std::optional<MpChangeForm>>&& changeForms) override;
  void Iterate(const IterateCallback& iterateCallback) override;

private:
  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
