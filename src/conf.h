#pragma once

#include <fstream>
#include "../include/nlohmann/json/json.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>

class Conf
{
public:
    Conf();
    Conf(std::string lng);
    ~Conf();

    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& getModifications() const { return modifications; }
    const std::unordered_map<std::string, std::string>& getAsSame() const { return asSame; }
    const std::unordered_map<std::string, std::string>& getAsOne() const { return asOne; }
    const std::vector<std::string>& getWords() const { return words; }
    const std::unordered_set<std::string>& getVolvesSet() const { return volvesSet; }
    const std::unordered_set<std::string>& getConsonantsSet() const { return consonantsSet; }
    const std::vector<std::string>& getVolves() const { return volves; }
    const std::vector<std::string>& getConsonants() const { return consonants; }

    Conf& operator=(const Conf& CONFIG) = default;
    void makeConfig(std::string lngPath);

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> modifications;
    std::unordered_map<std::string, std::string> asOne;
    std::vector<std::string> consonants;
    std::unordered_set<std::string> consonantsSet;
    std::vector<std::string> volves;
    std::unordered_set<std::string> volvesSet;
    std::vector<std::string> words;
    std::unordered_map<std::string, std::string> asSame;

    void makeAsOneConfig(const std::vector<std::string>& jAsOne);
    void makeAsSameConfig(const std::vector<std::vector<std::string>>& jAsSame, const std::string& jAlphabet);
    void makeModificationsConfig(const std::unordered_map<std::string, std::string>& jDictionary);
};
