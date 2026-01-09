#pragma once

#include <forward_list>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <array>

#include <iostream>

#include "letter.h"

struct RepeatComb
{
    std::array<std::forward_list<Letter>::iterator, 3> comb;
    int firstNumber;
    double score;
};

struct Repeat
{
    int count = 0;
    double power = 0;
    std::vector<Letter> letters;
    std::vector<RepeatComb> combs;
    std::set<std::string> _words;
    std::unordered_set<int> _letterNums;
};

class Phonotext
{
public:
    Phonotext();
    Phonotext(std::string text);
    ~Phonotext();

    std::forward_list<Letter> basetext;
    std::vector<std::pair<std::forward_list<Letter>::iterator, std::forward_list<Letter>::iterator>> SP;
    std::vector<std::vector<std::vector<std::forward_list<Letter>::iterator>>> syllableCombinations;
    std::unordered_map<std::string, Repeat> repeats;

    std::pair<int, int> countLetters();
    std::string getOrigin();
    std::string getTechnic();
    std::string getPrintable();
    std::string getPhonotextRepr();
    int length();
};
