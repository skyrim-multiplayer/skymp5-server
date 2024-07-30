#pragma once
#include "IDatabase.h"
#include <memory>

class MongoDatabase : public IDatabase
{
public:
  MongoDatabase(std::string uri_, std::string name_);
  size_t Upsert(
    std::vector<std::optional<MpChangeForm>>&& changeForms) override;
  void Iterate(const IterateCallback& iterateCallback) override;

private:
  int GetDocumentCount();
  void CheckErrorList(const std::vector<std::string>& errorList);

  struct Impl;
  std::shared_ptr<Impl> pImpl;
};
