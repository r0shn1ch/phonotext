#include "conf.h"

static std::vector<std::string> splitUtf8(const std::string& s)
{
    std::vector<std::string> out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size();)
    {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = 1;
        if ((c & 0x80u) == 0)
            len = 1;
        else if ((c & 0xE0u) == 0xC0u)
            len = 2;
        else if ((c & 0xF0u) == 0xE0u)
            len = 3;
        else if ((c & 0xF8u) == 0xF0u)
            len = 4;

        if (i + len > s.size())
            len = 1;

        out.emplace_back(s.substr(i, len));
        i += len;
    }
    return out;
}

Conf::Conf()
{
}

Conf::Conf(std::string lng)
{
    if (lng == "rus")
        makeConfig("res/lng_conf/russian.json");
    else if (lng == "eng")
        makeConfig("res/lng_conf/english.json");
    else if (lng == "lat")
        makeConfig("res/lng_conf/latin.json");
}

Conf::~Conf()
{
}

void Conf::makeConfig(std::string lngPath)
{
    std::ifstream fin(lngPath);
    if (!fin.is_open())
        throw std::runtime_error("Cannot open config file: " + lngPath);

    fin.peek();
    if (fin.eof())
        throw std::runtime_error("Config file is empty: " + lngPath);

    nlohmann::json data = nlohmann::json::parse(fin);

    consonants = data["consonants"];
    consonantsSet.insert(consonants.begin(), consonants.end());

    volves = data["volves"];
    volvesSet.insert(volves.begin(), volves.end());

    for (auto& i : consonants)
        words.push_back(i);
    for (auto& i : volves)
        words.push_back(i);

    std::vector<std::string> jAsOne;
    for (auto& i : data["as_one"])
        jAsOne.push_back(i);
    makeAsOneConfig(jAsOne);

    makeAsSameConfig(data["as_same"], data["alphabet"]);

    std::unordered_map<std::string, std::string> jDictionary;
    for (nlohmann::json::iterator it = data["modifications"].begin(); it != data["modifications"].end(); ++it)
        jDictionary.emplace(it.key(), it.value());
    makeModificationsConfig(jDictionary);
}

void Conf::makeAsOneConfig(const std::vector<std::string>& jAsOne)
{
    for (auto& i : jAsOne)
    {
        asOne.emplace(i, i);
    }
}

void Conf::makeAsSameConfig(const std::vector<std::vector<std::string>>& jAsSame, const std::string& jAlphabet)
{
    for (const auto& sym : splitUtf8(jAlphabet))
        asSame[sym] = sym;

    for (const auto& group : jAsSame)
    {
        const std::string& target = group[0];
        for (const auto& symbol : group)
        {
            asSame[symbol] = target;
        }
    }
}

void Conf::makeModificationsConfig(const std::unordered_map<std::string, std::string>& jDictionary)
{
    for (const auto& pair : jDictionary)
    {
        const std::string& key = pair.first;
        size_t firstCharLen = 0;
        for (; key[0] & (0x80 >> firstCharLen); ++firstCharLen);
        firstCharLen = (firstCharLen) ? firstCharLen : 1;

        std::string firstChar = key.substr(0, firstCharLen);
        std::string secondChar = key.substr(firstCharLen);

        auto it = modifications.find(firstChar);
        if (it == modifications.end())
        {
            std::unordered_map<std::string, std::string> tmp;
            tmp.emplace(secondChar, pair.second);
            modifications.emplace(firstChar, std::move(tmp));
        }
        else
        {
            it->second.emplace(secondChar, pair.second);
        }
    }
}
