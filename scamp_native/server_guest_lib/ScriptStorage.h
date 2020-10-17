#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <vector>

class IScriptStorage
{
public:
  virtual ~IScriptStorage() = default;

  virtual std::vector<uint8_t> GetScriptPex(const char* scriptName) = 0;

  virtual const std::set<std::string>& ListScripts() = 0;
};

class DirectoryScriptStorage : public IScriptStorage
{
public:
  DirectoryScriptStorage(std::filesystem::path pexDir_)
    : pexDir(pexDir_)
  {
    for (auto& p : std::filesystem::directory_iterator(pexDir)) {
      if (p.is_directory())
        continue;

      std::string s = p.path().string();
      while (s.find('/') != s.npos || s.find('\\') != s.npos) {
        while (s.find('/') != s.npos)
          s = { s.begin() + s.find('/') + 1, s.end() };
        while (s.find('\\') != s.npos)
          s = { s.begin() + s.find('\\') + 1, s.end() };
      }

      const std::regex e(".*\\.pex");
      if (std::regex_match(s, e)) {
        s = { s.begin(), s.end() - strlen(".pex") };
        scripts.insert(s);
      }
    }
  }

  std::vector<uint8_t> GetScriptPex(const char* scriptName) override
  {
    const auto path = pexDir / (scriptName + std::string(".pex"));

    if (!std::filesystem::exists(path))
      return {};

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
      throw std::runtime_error(path.string() + " is failed to open");
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(f), {});
    return buffer;
  }

  const std::set<std::string>& ListScripts() override { return scripts; }

  const std::filesystem::path pexDir;
  std::set<std::string> scripts;
};