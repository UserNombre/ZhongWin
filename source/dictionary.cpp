#include <fstream>
#include <regex>
#include <cstdint>

#include "utfcpp/utf8.h"

#include "dictionary.hpp"

CEDICTDictionary::CEDICTDictionary(std::string path) :
    IDictionary(EntryFormat::PLAINTEXT),
    maxSearchLength(10)
{
    parse_files(path);
}

void
CEDICTDictionary::parse_files(std::string path)
{
    std::ifstream dict(path + "cedict.dict", std::ios_base::binary);
    std::ifstream idx_s(path + "cedict_s.idx", std::ios_base::binary);
    std::ifstream idx_t(path + "cedict_t.idx", std::ios_base::binary);
    if(!dict.is_open() || !idx_s.is_open() || !idx_t.is_open())
        return;

    dictionary.assign((std::istreambuf_iterator<char>(dict)),
                      (std::istreambuf_iterator<char>()));

    std::string line;
    while(std::getline(idx_s, line))
    {
        const char *cstr = line.c_str();
        std::string word(cstr);
        std::vector<uint32_t> offsets;
        
        for(uint32_t* poffset = (uint32_t*)(cstr + word.size()+1);
            (void *)poffset < cstr+line.size(); poffset++)
        {
            offsets.push_back(*poffset);
        }
        simplifiedIndex[word] = offsets;
    }
    while(std::getline(idx_t, line))
    {
        const char* cstr = line.c_str();
        std::string word(cstr);
        std::vector<uint32_t> offsets;

        for(uint32_t* poffset = (uint32_t*)(cstr + word.size()+1);
            (void*)poffset < cstr + line.size(); poffset++)
        {
            offsets.push_back(*poffset);
        }
        simplifiedIndex[word] = offsets;
    }

    loaded = true;
}

std::string
CEDICTDictionary::format_hanzi(std::string simplified, std::string traditional)
{
    if(simplified != traditional)
        return simplified + ' ' + traditional;

    return simplified;
}

std::string
CEDICTDictionary::format_pinyin(std::string pinyin)
{
    return pinyin;
}

std::string
CEDICTDictionary::format_definition(std::string definition)
{
    std::string output;
    std::regex definitionRegex(R"(([^/]+))");

    std::regex_token_iterator<std::string::iterator> endIt;
    for(std::regex_token_iterator it(definition.begin(), definition.end(), definitionRegex);
        it != endIt; it++)
    {
        output += *it;
        output += "; ";
    }

    return output;
}

std::string
CEDICTDictionary::format_entry(std::string entry)
{
    std::smatch matches;
    std::regex entryRegex(R"(^([^\s]+) ([^\s]+) \[(.+?)\] /(.+)/$)");
    std::regex_search(entry, matches, entryRegex);
    if(matches.size() != 5)
        return "Regex error";

    return format_hanzi(matches[1], matches[2]) + '\n' +
           format_pinyin(matches[3])            + '\n' +
           format_definition(matches[4]);
}

std::vector<std::string>
CEDICTDictionary::get_entries(std::string string)
{
    std::vector<std::string> entries;
    auto it = simplifiedIndex.find(string);
    if(simplifiedIndex.end() == it)
        return entries;

    std::vector<uint32_t> offsets = it->second;
    for(size_t offset : offsets)
    {
        auto beginIt = dictionary.begin() + offset;
        auto endIt = dictionary.begin() + dictionary.find('\n', offset);
        std::string entry = dictionary.substr(offset, endIt - beginIt);
        entries.push_back(format_entry(entry));
    }

    return entries;
}

std::vector<std::string>
CEDICTDictionary::search_string(std::string string, int position)
{
    ptrdiff_t utf8len = utf8::distance(string.begin(), string.end());
    ptrdiff_t utf8pos = utf8::distance(string.begin(), string.begin()+position);
    ptrdiff_t left = utf8len-utf8pos;

    std::vector<std::string> entries;
    int max = (left < maxSearchLength) ? left : maxSearchLength;
    for(int i = max; i > 0; i--)
    {
        auto beginIt = string.begin() + position;
        auto endIt = beginIt;
        utf8::advance(endIt, i, string.end());
        std::string substring = string.substr(position, endIt - beginIt);

        std::vector<std::string> subentries = get_entries(substring);
        if(subentries.size() > 0)
        {
            entries.insert(entries.end(), subentries.begin(), subentries.end());
        }
    }

    return entries;
}