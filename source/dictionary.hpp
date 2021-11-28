#pragma once
#include <utility>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

enum struct EntryFormat
{
    NONE,
    PLAINTEXT,
    HTML
};

class IDictionary
{
public:
    IDictionary() : format(EntryFormat::NONE), loaded(false) {};
    IDictionary(EntryFormat format) : format(format), loaded(false) {};

    // Get dictionary entries for the given string
    virtual std::vector<std::string> get_entries(std::string string) = 0;
    // Search the string for possible entries starting at the given position
    virtual std::vector<std::string> search_string(std::string line, int position) = 0;
    EntryFormat getEntryFormat() { return format; };
    bool isLoaded() { return loaded; };
protected:
    bool loaded;
private:
    EntryFormat format;
};

struct CEDICTEntry
{
    std::string simplified;
    std::string traditional;
    std::string pinyin;
    std::string definition;
};

class CEDICTDictionary : public IDictionary
{
public:
    explicit CEDICTDictionary(std::string path);
    std::vector<std::string> get_entries(std::string string) override;
    std::vector<std::string> search_string(std::string line, int position) override;
private:
    void parse_files(std::string path);
    std::string format_entry(std::string entry);
    std::string format_hanzi(std::string simplified, std::string traditional);
    std::string format_pinyin(std::string pinyin);
    std::string format_definition(std::string definition);

    std::unordered_map<std::string, std::vector<uint32_t>> simplifiedIndex;
    std::unordered_map<std::string, std::vector<uint32_t>> traditionalIndex;
    std::string dictionary;
    int maxSearchLength;
};