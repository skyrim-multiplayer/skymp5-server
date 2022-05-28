#include <filesystem>
#include <map>
#include <string>
#include <vector>

class DirectoryEntry
{
public:
  uint32_t stringId;
  uint32_t offset;
  uint32_t length;
  std::string str;

  DirectoryEntry();
  DirectoryEntry(uint32_t stringId, uint32_t stringOffset);
};

class LocalizationProvider
{
  std::vector<DirectoryEntry*> ParseDirectoryEntries(std::vector<char> buffer);

  std::map<uint32_t, std::string> ParseStrings(
    std::vector<char> buffer, std::vector<DirectoryEntry*> entries);

  std::map<uint32_t, std::string> ParseILDLStrings(
    std::vector<char> buffer, std::vector<DirectoryEntry*> entries);

  std::map<uint32_t, std::string> Parse(
    const std::filesystem::directory_entry& file);

public:
  LocalizationProvider(const std::string& language);
  std::map<std::string, std::map<uint32_t, std::string>>
    localization; // localization[filename][stringId]
  std::string Get(const std::string& file, uint32_t stringId);
};
