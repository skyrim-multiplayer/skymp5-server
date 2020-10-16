#include "TestUtils.hpp"

#include "MpChangeForms.h"
#include "SqliteSaveStorage.h"
#include <filesystem>

std::shared_ptr<ISaveStorage> MakeSaveStorage()
{
  auto fileName = "unit.sqlite";
  if (std::filesystem::exists(fileName))
    std::filesystem::remove(fileName);
  return std::make_shared<SqliteSaveStorage>(fileName);
}

MpChangeForm CreateChangeForm(const char* descStr)
{
  MpChangeForm res;
  res.formDesc = FormDesc::FromString(descStr);
  return res;
}

void UpsertSync(ISaveStorage& st, std::vector<MpChangeForm> changeForms)
{
  bool finished = false;
  st.Upsert(changeForms, [&] { finished = true; });

  int i = 0;
  while (!finished) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    st.Tick();
    ++i;
    if (i > 2000)
      throw std::runtime_error("Timeout exceeded");
  }
}

void WaitForNextUpsert(ISaveStorage& st, WorldState& wst)
{
  uint32_t n = st.GetNumFinishedUpserts();

  int i = 0;
  while (n == st.GetNumFinishedUpserts()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    wst.TickTimers(); // should include st.Tick()

    ++i;
    if (i > 2000)
      throw std::runtime_error("Timeout exceeded");
  }
}

TEST_CASE("ChangeForm is saved correctly", "[save]")
{
  auto st = MakeSaveStorage();

  MpChangeForm f1, f2;
  f1.formDesc = { 1, "" };
  f1.position = { 1, 2, 3 };
  f1.lookDump = "{}";
  f1.inv.AddItem(0xf, 1000);
  f1.equipmentDump = "[]";
  f2.formDesc = { 2, "" };
  f2.position = { 2, 4, 6 };
  UpsertSync(*st, { f1, f2 });

  auto res = ISaveStorageUtils::FindAllSync(*st);
  REQUIRE(res.size() == 2);
  REQUIRE(res[{ 1, "" }].position == NiPoint3(1, 2, 3));
  REQUIRE(res[{ 1, "" }].lookDump == "{}");
  REQUIRE(res[{ 1, "" }].equipmentDump == "[]");
  REQUIRE(res[{ 1, "" }].inv == Inventory().AddItem(0xf, 1000));
  REQUIRE(res[{ 2, "" }].position == NiPoint3(2, 4, 6));
}

TEST_CASE("Upsert affects the number of change forms in the database in the "
          "correct way",
          "[save]")
{
  auto st = MakeSaveStorage();

  REQUIRE(ISaveStorageUtils::CountSync(*st) == 0);

  UpsertSync(
    *st,
    { CreateChangeForm("0"), CreateChangeForm("1"), CreateChangeForm("2") });

  REQUIRE(ISaveStorageUtils::CountSync(*st) == 3);

  UpsertSync(*st,
             { CreateChangeForm("0"), CreateChangeForm("1"),
               CreateChangeForm("2"), CreateChangeForm("3") });

  REQUIRE(ISaveStorageUtils::CountSync(*st) == 4);
}

TEST_CASE("AttachSaveStorage forces loading", "[save]")
{
  FakeSendTarget tgt;
  PartOne p;
  p.worldState.espmFiles = { "AaAaAa.esm" };
  p.worldState.AddForm(
    std::unique_ptr<MpObjectReference>(new MpObjectReference(
      LocationalData(), FormCallbacks::DoNothing(), 0xaaaa, "STAT")),
    0xee);

  auto& refr = p.worldState.GetFormAt<MpObjectReference>(0xee);
  REQUIRE(refr.GetPos() == NiPoint3(0, 0, 0));

  auto st = MakeSaveStorage();
  auto f = CreateChangeForm("ee:AaAaAa.esm");
  f.position = { 1, 1, 1 };
  f.baseDesc = FormDesc::FromString("aaaa:AaAaAa.esm");
  UpsertSync(*st, { f });
  p.AttachSaveStorage(st, &tgt);

  REQUIRE(refr.GetPos() == NiPoint3(1, 1, 1));
}

TEST_CASE("Changes are transferred to SaveStorage", "[save]")
{
  FakeSendTarget tgt;
  PartOne p;
  auto st = MakeSaveStorage();
  p.AttachSaveStorage(st, &tgt);

  REQUIRE(ISaveStorageUtils::CountSync(*st) == 0);
  p.CreateActor(0xffaaaeee, { 1, 1, 1 }, 1, 0x3c, &tgt);

  WaitForNextUpsert(*st, p.worldState);
  REQUIRE(ISaveStorageUtils::CountSync(*st) == 1);
}