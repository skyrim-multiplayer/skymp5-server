#include "SqliteSaveStorage.h"
#include "SqliteChangeForm.h"
#include <atomic>
#include <list>
#include <mutex>
#include <sqlite_orm/sqlite_orm.h>
#include <thread>

using namespace sqlite_orm;

#define MAKE_STORAGE(name)                                                    \
  auto storage = make_storage(                                                \
    name,                                                                     \
    make_table<SqliteChangeForm>(                                             \
      "SqliteChangeForm",                                                     \
      make_column("primary", &SqliteChangeForm::primary, autoincrement(),     \
                  primary_key()),                                             \
      make_column("record_type", &SqliteChangeForm::recType),                 \
      make_column("base_desc", &SqliteChangeForm::GetBaseFormDesc,            \
                  &SqliteChangeForm::SetBaseFormDesc),                        \
      make_column("form_desc", &SqliteChangeForm::GetFormDesc,                \
                  &SqliteChangeForm::SetFormDesc),                            \
      make_column("x", &SqliteChangeForm::GetX, &SqliteChangeForm::SetX),     \
      make_column("y", &SqliteChangeForm::GetY, &SqliteChangeForm::SetY),     \
      make_column("z", &SqliteChangeForm::GetZ, &SqliteChangeForm::SetZ),     \
      make_column("angle_x", &SqliteChangeForm::GetAngleX,                    \
                  &SqliteChangeForm::SetAngleX),                              \
      make_column("angle_y", &SqliteChangeForm::GetAngleY,                    \
                  &SqliteChangeForm::SetAngleY),                              \
      make_column("angle_z", &SqliteChangeForm::GetAngleZ,                    \
                  &SqliteChangeForm::SetAngleZ),                              \
      make_column("inventory_dump", &SqliteChangeForm::GetInventory,          \
                  &SqliteChangeForm::SetInventory),                           \
      make_column("is_harvested", &SqliteChangeForm::isHarvested),            \
      make_column("is_open", &SqliteChangeForm::isOpen),                      \
      make_column("next_reloot_datetime",                                     \
                  &SqliteChangeForm::nextRelootDatetime),                     \
      make_column("world_or_cell", &SqliteChangeForm::worldOrCell),           \
      make_column("is_race_menu_open", &SqliteChangeForm::isRaceMenuOpen),    \
      make_column("look_dump", &SqliteChangeForm::GetLook,                    \
                  &SqliteChangeForm::SetLook),                                \
      make_column("equipment_dump", &SqliteChangeForm::GetEquipment,          \
                  &SqliteChangeForm::SetEquipment),                           \
      make_column("base_container_added",                                     \
                  &SqliteChangeForm::baseContainerAdded)));

struct UpsertTask
{
  std::vector<MpChangeForm> changeForms;
  std::function<void()> callback;
};

struct SqliteSaveStorage::Impl
{
  struct
  {
    std::string storageName;
    std::mutex m;
  } share;

  struct
  {
    std::list<std::exception_ptr> exceptions;
    std::mutex m;
  } share2;

  struct
  {
    std::list<UpsertTask> upsertTasks;
    std::mutex m;
  } share3;

  struct
  {
    std::vector<std::function<void()>> upsertCallbacksToFire;
    std::mutex m;
  } share4;

  std::unique_ptr<std::thread> thr;
  std::atomic<bool> destroyed = false;
  uint32_t numFinishedUpserts = 0;
};

SqliteSaveStorage::SqliteSaveStorage(const char* filename)
  : pImpl(new Impl, [](Impl* p) { delete p; })
{
  MAKE_STORAGE(filename);

  auto res = storage.sync_schema_simulate(true);

  std::vector<std::string> destructiveActions;

  for (auto [str, result] : res) {
    const char* action = "";
    switch (result) {
      case sync_schema_result::dropped_and_recreated:
        action = "dropped_and_recreated";
        break;
      case sync_schema_result::new_columns_added_and_old_columns_removed:
        action = "new_columns_added_and_old_columns_removed";
        break;
      case sync_schema_result::old_columns_removed:
        action = "old_columns_removed";
        break;
    }
    if (action[0])
      destructiveActions.push_back(action + (" (target is " + str + ")"));
  }
  if (destructiveActions.size()) {
    std::stringstream ss;
    ss << "Sqlite is going to take some destructive actions: ";
    for (auto v : destructiveActions)
      ss << v << "; ";
    throw std::runtime_error(ss.str());
  }
  storage.sync_schema(true);

  pImpl->share.storageName = filename;
  auto p = this->pImpl.get();
  pImpl->thr.reset(new std::thread([p] { SaverThreadMain(p); }));
}

SqliteSaveStorage::~SqliteSaveStorage()
{
  pImpl->destroyed = true;
  pImpl->thr->join();
}

void SqliteSaveStorage::SaverThreadMain(Impl* pImpl)
{
  while (!pImpl->destroyed) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    try {

      decltype(pImpl->share3.upsertTasks) tasks;
      {
        std::lock_guard l(pImpl->share3.m);
        tasks = std::move(pImpl->share3.upsertTasks);
        pImpl->share3.upsertTasks.clear();
      }

      std::vector<std::function<void()>> callbacksToFire;

      {
        std::lock_guard l(pImpl->share.m);
        MAKE_STORAGE(pImpl->share.storageName.data());
        auto g = storage.transaction_guard();
        int numChangeForms = 0;
        auto was = clock();
        for (auto& t : tasks) {
          std::map<FormDesc, int> existingFormDescs;
          for (auto changeForm : storage.iterate<SqliteChangeForm>()) {
            existingFormDescs.insert(
              { changeForm.formDesc, changeForm.primary });
          }

          std::vector<SqliteChangeForm> toInsert, toUpdate;

          for (auto& changeForm : t.changeForms) {
            SqliteChangeForm f;
            std::vector<SqliteChangeForm>* target;

            if (auto it = existingFormDescs.find(changeForm.formDesc);
                it != existingFormDescs.end()) {
              f.primary = it->second;
              target = &toUpdate;
            } else {
              f.primary = -1;
              target = &toInsert;
            }
            numChangeForms++;

            static_cast<MpChangeForm&>(f) = std::move(changeForm);
            target->push_back(f);
          }

          storage.insert_range(toInsert.data(),
                               toInsert.data() + toInsert.size());
          for (auto& v : toUpdate)
            storage.update(v);
          callbacksToFire.push_back(t.callback);
        }
        g.commit();
        if (numChangeForms > 0)
          printf("Saved %d ChangeForms in %d ticks\n", numChangeForms,
                 clock() - was);
      }

      {
        std::lock_guard l(pImpl->share4.m);
        for (auto& cb : callbacksToFire)
          pImpl->share4.upsertCallbacksToFire.push_back(cb);
      }
    } catch (...) {
      std::lock_guard l(pImpl->share2.m);
      auto exceptionPtr = std::current_exception();
      pImpl->share2.exceptions.push_back(exceptionPtr);
    }
  }
}

void SqliteSaveStorage::IterateSync(const IterateSyncCallback& cb)
{
  std::lock_guard l(pImpl->share.m);
  MAKE_STORAGE(pImpl->share.storageName.data());
  for (auto v : storage.iterate<SqliteChangeForm>()) {
    cb(v);
  }
}

void SqliteSaveStorage::Upsert(const std::vector<MpChangeForm>& changeForms,
                               const UpsertCallback& cb)
{
  std::lock_guard l(pImpl->share3.m);
  pImpl->share3.upsertTasks.push_back({ changeForms, cb });
}

uint32_t SqliteSaveStorage::GetNumFinishedUpserts() const
{
  return pImpl->numFinishedUpserts;
}

void SqliteSaveStorage::Tick()
{
  {
    std::lock_guard l(pImpl->share2.m);
    if (!pImpl->share2.exceptions.empty()) {
      auto exceptionPtr = std::move(pImpl->share2.exceptions.front());
      pImpl->share2.exceptions.pop_front();
      std::rethrow_exception(exceptionPtr);
    }
  }

  decltype(pImpl->share4.upsertCallbacksToFire) upsertCallbacksToFire;
  {
    std::lock_guard l(pImpl->share4.m);
    upsertCallbacksToFire = std::move(pImpl->share4.upsertCallbacksToFire);
    pImpl->share4.upsertCallbacksToFire.clear();
  }
  for (auto& cb : upsertCallbacksToFire) {
    pImpl->numFinishedUpserts++;
    cb();
  }
}