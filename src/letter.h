#pragma once

#include <string>

class Letter
{
public:
    Letter();
    Letter(std::string symbol);
    ~Letter();

    std::string origin;
    std::string technic;
    std::string printable;
    bool isConsonant;
    bool isVolve;
    int syll;
    int pwr;
    int fw_pos;
    int number;
    int word;
    int pEnd;
    bool accent;

    int w_pos;

    std::string getLetter();
    std::string getLetterRepr();

    bool operator ==(const Letter& letter) const;
    bool operator <(const Letter& letter) const;
};

namespace std {
template<> struct hash<Letter> {
    size_t operator()(const Letter& l) const {
        size_t h1 = hash<string>()(l.origin);
        size_t h2 = hash<string>()(l.technic);
        size_t h3 = hash<string>()(l.printable);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
}
