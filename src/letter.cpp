#include "letter.h"

Letter::Letter()
{
    origin = "";
    technic = "";
    printable = "";
    isConsonant = false;
    isVolve = false;
    syll = 0;
    pwr = 0;
    number = 0;
    word = 0;
    pEnd = 0;
    fw_pos = 0;
    accent = false;
    w_pos = 0;
}

Letter::Letter(std::string symbol)
{
    origin = symbol;
    technic = "+";
    printable = symbol;
    isConsonant = false;
    isVolve = false;
    syll = 0;
    pwr = 0;
    number = 0;
    word = 0;
    pEnd = 0;
    fw_pos = 0;
    accent = false;
    w_pos = 0;
}

Letter::~Letter()
{
}

std::string Letter::getLetter()
{
    std::string outLetter = "";
    outLetter += origin;
    outLetter += "(";
    outLetter += technic;
    outLetter += printable;
    outLetter += ")";
    return outLetter;
}

std::string Letter::getLetterRepr()
{
    std::string outLetter = "";
    outLetter += origin;
    outLetter += "'";
    outLetter += technic;
    outLetter += "'";
    outLetter += printable;
    outLetter += "'";
    outLetter += (isConsonant ? "C" : "-");
    outLetter += (isVolve ? "V" : "-");
    outLetter += std::to_string(number);
    outLetter += "'";
    outLetter += std::to_string(syll);
    outLetter += "'";
    outLetter += std::to_string(word);
    return outLetter;
}

bool Letter::operator==(const Letter& letter) const
{
    return origin == letter.origin &&
           technic == letter.technic &&
           printable == letter.printable &&
           isConsonant == letter.isConsonant &&
           isVolve == letter.isVolve &&
           syll == letter.syll &&
           pwr == letter.pwr &&
           number == letter.number &&
           word == letter.word &&
           pEnd == letter.pEnd &&
           accent == letter.accent &&
           w_pos == letter.w_pos;
}

bool Letter::operator<(const Letter& letter) const
{
    if (origin != letter.origin) return origin < letter.origin;
    if (technic != letter.technic) return technic < letter.technic;
    if (printable != letter.printable) return printable < letter.printable;
    if (number != letter.number) return number < letter.number;
    return false;
}
