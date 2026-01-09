#include "proccessing.h"
#include <climits>
#include <QDebug>
#include <map>
#include <iostream>
#include <unordered_set>

Proccessing::Proccessing(Phonotext pt, std::string lng, double min_pwr, double max_pwr)
    : pt(pt), CONFIG(lng), min_pwr(min_pwr), max_pwr(max_pwr)
{
    qDebug() << "proccess start";
    this->proccess();
}

void Proccessing::proccess()
{
    qDebug() << "modify";
    modifyProccessor();
    qDebug() << "same";
    sameProccessor();
    qDebug() << "join";
    joinProccessor();
    qDebug() << "number";
    numberProccessor();
    qDebug() << "finder";
    finderVolve();
    qDebug() << "SP";
    SPmaxProccessor();
    qDebug() << "combinations";
    combinationsProccessor();
    qDebug() << "repeat";
    repeatProccessor();
    qDebug() << "repeat ended";
    qDebug() << "proccessing end";
}

void Proccessing::modifyProccessor()
{
    std::string tmp_a;
    std::string tmp_b;
    std::string tmp_c;
    auto it = pt.basetext.begin();
    auto itPreviosLetter = pt.basetext.before_begin();
    bool needChange = false;

    const auto& modifications = CONFIG.getModifications();

    while (it != pt.basetext.end())
    {
        if (needChange)
        {
            needChange = false;
        }

        std::string origin = it->origin;
        if (it == pt.basetext.begin())
        {
            tmp_b = origin;
        }
        else
        {
            tmp_a = tmp_b;
            tmp_b = origin;

            auto modFirstKey = modifications.find(tmp_a);
            if (modFirstKey != modifications.end())
            {
                const auto& innerMap = modFirstKey->second;
                auto modSecondKey = innerMap.find(tmp_b);
                if (modSecondKey != innerMap.end())
                {
                    tmp_c = modSecondKey->second;
                    int i,l;
                    int firstCharLen = 1;
                    if (!tmp_c.empty()) {
                        for (i = 0; tmp_c[0] & (0x80 >> i); ++i); i = (i) ? i : 1;
                        for (l = 0; tmp_c[i] & (0x80 >> l); ++l); l = (l) ? l : 1;
                    }
                    itPreviosLetter->origin = tmp_c.substr(0, i);
                    it->origin = tmp_c.substr(i, l);
                    pt.basetext.emplace_after(it, Letter((tmp_c.substr(i + l))));
                    auto b = it;
                    (++b)->printable = it->printable;
                    it->printable = "`";
                    needChange = true;
                }
            }
        }
        itPreviosLetter = it;
        ++it;
    }
}

void Proccessing::sameProccessor()
{
    const auto& asSame = CONFIG.getAsSame();

    for (auto& symb : pt.basetext)
    {
        auto sameKey = asSame.find(symb.printable);
        if (sameKey != asSame.end())
            symb.technic = sameKey->second;
        else if (symb.technic == "+")
            symb.technic = "&";

        if (symb.origin == " ")
            symb.technic = "|";

        if (symb.technic == "&")
            symb.technic = symb.origin;
    }
}

void Proccessing::joinProccessor()
{
    std::string tmp_a;
    std::string tmp_b;

    const auto& asOne = CONFIG.getAsOne();

    auto it = pt.basetext.begin();
    auto itPreviosLetter = pt.basetext.begin();

    while (it != pt.basetext.end())
    {
        if (it == pt.basetext.begin())
            tmp_a = it->origin;
        else
        {
            tmp_a = itPreviosLetter->origin;
            tmp_b = it->origin;

            auto oneKey = asOne.find(tmp_a + tmp_b);
            if (oneKey != asOne.end())
            {
                itPreviosLetter->origin = oneKey->second;
                itPreviosLetter->printable = oneKey->second;
                itPreviosLetter->technic = oneKey->second;

                if (std::next(it) == pt.basetext.end())
                {
                    pt.basetext.erase_after(itPreviosLetter);
                    break;
                }

                it = itPreviosLetter;
                pt.basetext.erase_after(itPreviosLetter);
            }
            if (tmp_b == "\u0301")
            {
                itPreviosLetter->printable = itPreviosLetter->printable + tmp_b;
                itPreviosLetter->accent = true;
                it = itPreviosLetter;
                pt.basetext.erase_after(itPreviosLetter);
            }
            if (tmp_b == "\u0484")
            {
                it = itPreviosLetter;
                pt.basetext.erase_after(itPreviosLetter);
            }
            itPreviosLetter = it;
        }
        ++it;
    }
}

void Proccessing::numberProccessor()
{
    int i = 0;
    int j = 1;
    int k = 0;
    int l = 0;
    int word_len = 0;
    bool space = false;
    int space_pos = 0;
    std::forward_list<Letter>::iterator beginWord = pt.basetext.begin();
    int l_number = 0;

    const auto& volvesSet = CONFIG.getVolvesSet();
    const auto& consonantsSet = CONFIG.getConsonantsSet();

    for (auto it = pt.basetext.begin(); it != pt.basetext.end(); ++it, ++l_number)
    {
        it->number = i;
        if (it->origin == " " || it->technic == "|" || it->technic == "\n")
        {
            space_pos = 0;
            k++;
            l = 0;
            space = true;
        }
        else
        {
            space_pos++;
            bool flag = (volvesSet.find(it->technic) != volvesSet.end() ||
                         consonantsSet.find(it->technic) != consonantsSet.end());
            space = (space && flag);
        }

        if (it->technic == "\n")
        {
            auto lastWord = beginWord;
            while (beginWord != it)
            {
                beginWord->pEnd = 1;
                ++beginWord;
            }
        }

        if (volvesSet.find(it->technic) != volvesSet.end())
        {
            it->isVolve = true;
            j++;
        }

        if (consonantsSet.find(it->technic) != consonantsSet.end())
        {
            it->isConsonant = true;
        }

        if(it->isConsonant || it->isVolve) {
            it->word = k;
            word_len++;
        }
        else it->word = k - 1;

        it->syll = j;
        i++;

        if (l == 1)
        {
            beginWord = it;
        }
        it->fw_pos = l;

        if (it->origin == " " || it->origin == "\n")
        {
            auto tmpIt = beginWord;
            if (word_len <= 2)
            {
                while (tmpIt != it)
                {
                    tmpIt->fw_pos = 0;
                    ++tmpIt;
                }
            }
            word_len = 0;
        }
        l++;
        it->w_pos = space_pos;
    }
}

void Proccessing::finderVolve()
{
    for (auto it = pt.basetext.begin(); it != pt.basetext.end(); ++it)
        if (it->isVolve)
            volveIterators.push_back(it);
}

void Proccessing::SPmaxProccessor()
{
    std::vector<std::pair<std::forward_list<Letter>::iterator, std::forward_list<Letter>::iterator>> dividedVolveIterators;

    if (volveIterators.size() > 1)
    {
        auto startVolveIt = pt.basetext.begin();
        ++startVolveIt;
        auto middleVolveIt = volveIterators[0];

        for (size_t i = 1; i < volveIterators.size(); ++i)
        {
            auto endVolveIt = volveIterators[i];
            dividedVolveIterators.emplace_back(startVolveIt, endVolveIt);
            ++middleVolveIt;
            startVolveIt = middleVolveIt;
            middleVolveIt = endVolveIt;
        }

        ++startVolveIt;
        if (middleVolveIt->origin != "\n")
            dividedVolveIterators.emplace_back(startVolveIt, pt.basetext.end());
    }
    else
        dividedVolveIterators.emplace_back(pt.basetext.begin(), pt.basetext.end());

    pt.SP = std::move(dividedVolveIterators);
}

std::pair<int, std::vector<int>> Proccessing::findLocalWordsInds(std::pair<std::forward_list<Letter>::iterator, std::forward_list<Letter>::iterator> localSP)
{
    int indVolve = 0;
    std::vector<int> indCons;

    int i = 0;
    for (auto it = localSP.first; it != localSP.second; ++it, ++i)
    {
        if (it->isVolve)
            indVolve = i;
        else
            indCons.push_back(i);
    }

    return std::make_pair(indVolve, std::move(indCons));
}

bool isCorrectComb(std::forward_list<Letter>::iterator it1, std::forward_list<Letter>::iterator it2, std::forward_list<Letter>::iterator it3)
{
    static const std::unordered_set<std::string> invalidSymbols = {
        "\n", "-", "—", "–", ",", ".", "’", "€", "!", "?", "j", " ", ":"
    };

    if (invalidSymbols.find(it1->origin) != invalidSymbols.end() ||
        invalidSymbols.find(it2->origin) != invalidSymbols.end() ||
        invalidSymbols.find(it3->origin) != invalidSymbols.end())
        return false;

    char c1 = it1->origin[0];
    char c2 = it2->origin[0];
    char c3 = it3->origin[0];

    if ((c1 >= 'a' && c1 <= 'z') ||
        (c2 >= 'a' && c2 <= 'z') ||
        (c3 >= 'a' && c3 <= 'z'))
        return false;

    return true;
}

void Proccessing::combinationsProccessor(int N)
{
    N++;
    pt.syllableCombinations.reserve(pt.SP.size());

    for (size_t i = 0; i < pt.SP.size(); ++i)
    {
        auto localInds = findLocalWordsInds(pt.SP[i]);
        int posVolve = localInds.first;
        const auto& posCons = localInds.second;

        if (posCons.size() < 2) continue;

        std::vector<std::pair<int, int>> consCombs;
        size_t expectedSize = (posCons.size() * (posCons.size() - 1)) / 2;
        consCombs.reserve(expectedSize);

        for (size_t j = 0; j < posCons.size(); ++j)
            for (size_t k = j + 1; k < posCons.size(); ++k)
                consCombs.emplace_back(posCons[j], posCons[k]);

        std::vector<std::vector<std::forward_list<Letter>::iterator>> itCombs;
        itCombs.reserve(consCombs.size());

        for (const auto& comb : consCombs)
        {
            std::vector<int> combs = {comb.first, comb.second, posVolve};
            std::sort(combs.begin(), combs.end());

            auto it1 = pt.SP[i].first;
            auto it2 = pt.SP[i].first;
            auto it3 = pt.SP[i].first;

            std::advance(it1, combs[0]);
            std::advance(it2, combs[1]);
            std::advance(it3, combs[2]);

            if (isCorrectComb(it1, it2, it3))
            {
                itCombs.emplace_back(std::vector<std::forward_list<Letter>::iterator>{it1, it2, it3});
            }
        }

        if (!itCombs.empty())
            pt.syllableCombinations.emplace_back(std::move(itCombs));
    }
}

void Proccessing::repeatProccessor()
{
    const auto& words = CONFIG.getWords();

    for (size_t n_syll = 0; n_syll < pt.syllableCombinations.size(); ++n_syll)
    {
        const auto& syllable = pt.syllableCombinations[n_syll];
        for (const auto& comb : syllable)
        {
            std::vector<std::string> a;
            a.reserve(comb.size());
            for (const auto& i : comb)
            {
                if (i->isConsonant)
                    a.push_back(i->technic);
            }

            if (a.size() < 2) continue; // Нужно минимум 2 согласных

            std::set<std::string> tmpWords(a.begin(), a.end());
            std::string setToStr;
            setToStr.reserve(tmpWords.size() * 2);
            for (const auto& i : tmpWords)
                setToStr += i;

            auto filter = rusFilterComb(comb, words);
            if (filter.first)
            {
                auto it = pt.repeats.find(setToStr);
                if (it == pt.repeats.end())
                {
                    Repeat tmpRepeat;
                    tmpRepeat._words = std::move(tmpWords);
                    tmpRepeat.count = 1;
                    tmpRepeat.power = filter.second;

                    std::unordered_set<int> uniqueNumbers;
                    uniqueNumbers.reserve(comb.size());
                    for (const auto& itL : comb)
                        uniqueNumbers.insert(itL->number);

                    tmpRepeat.letters.clear();
                    tmpRepeat.letters.reserve(uniqueNumbers.size());
                    for (const auto& itL : comb)
                    {
                        if (uniqueNumbers.erase(itL->number))
                            tmpRepeat.letters.push_back(*itL);
                    }
                    tmpRepeat.combs.push_back(comb);

                    if (tmpRepeat._words.size() != 1)
                        pt.repeats.emplace(setToStr, std::move(tmpRepeat));
                }
                else
                {
                    it->second._words.insert(a.begin(), a.end());
                    it->second.count += 1;
                    it->second.power = filter.second;

                    std::unordered_set<int> uniqueNumbers;
                    uniqueNumbers.reserve(it->second.letters.size() + comb.size());
                    for (const auto& ltr : it->second.letters)
                        uniqueNumbers.insert(ltr.number);
                    for (const auto& itL : comb)
                        uniqueNumbers.insert(itL->number);

                    std::vector<Letter> merged;
                    merged.reserve(uniqueNumbers.size());
                    for (const auto& ltr : it->second.letters)
                    {
                        if (uniqueNumbers.erase(ltr.number))
                            merged.push_back(ltr);
                    }
                    for (const auto& itL : comb)
                    {
                        if (uniqueNumbers.erase(itL->number))
                            merged.push_back(*itL);
                    }
                    it->second.letters = std::move(merged);
                    it->second.combs.push_back(comb);
                }
            }
        }
    }

    auto tmp = handlePower(pt.repeats);
    if (!pt.repeats.empty()) {
        pt.repeats.erase(pt.repeats.begin());
    }
}

std::pair<bool, double> Proccessing::rusFilterComb(std::vector<std::forward_list<Letter>::iterator> comb, const std::vector<std::string>& words)
{
    std::string tmptxt;
    tmptxt.reserve(3);
    for (auto it = comb[0]; it != comb[2]; ++it)
        tmptxt += it->technic;

    std::string txt;
    txt.reserve(tmptxt.size());
    bool seen[256] = {false};

    for (char c : tmptxt)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        if (!seen[uc])
        {
            seen[uc] = true;
            txt += c;
        }
    }

    if (txt.size() < 3)
        return std::make_pair(false, 0);

    double pwr;
    if (comb[0]->isVolve)
        pwr = 2;
    else if (comb[2]->isVolve)
        pwr = 1;
    else
        pwr = 3;

    int count = 0;
    for (char i : txt)
        if (i == '|')
            count++;

    pwr += (comb[2]->number - comb[0]->number - count == 2 ? 5 : 0);
    pwr += (count == 0 ? 2 : 0);

    count = 0;
    std::string й_str = "й";
    for (size_t i = (txt.size() > 3 ? txt.size() - 3 : 0); i < txt.size(); ++i) {
        if (i + 1 < txt.size() && txt.substr(i, 2) == й_str) {
            count++;
            i++;
        }
    }

    pwr += (count == 0 ? 4 : 0);
    pwr += (!comb[0]->w_pos || !comb[1]->w_pos || !comb[2]->w_pos ? 1 : 0);
    pwr /= 15;

    return std::make_pair((min_pwr <= pwr) && (pwr <= max_pwr), pwr);
}

#include <QFile>
void Proccessing::print(QString filename)
{
    QFile out(filename);
    if(!out.open(QIODevice::ReadWrite | QIODevice::Text))
        return;
    QTextStream stream (&out);
    stream << "===========\n";

    stream << "-----------\n";
    stream << "origin      : ";
    for (auto& i : pt.basetext)
    {
        stream << QString::fromStdString(i.origin);
    }
    stream << Qt::endl;

    stream << "-----------\n";
    stream << "technic     : ";
    for (auto& i : pt.basetext)
    {
        stream << QString::fromStdString(i.technic);
    }
    stream << Qt::endl;

    stream << "-----------\n";
    stream << "printable   : ";
    for (auto& i : pt.basetext)
    {
        stream << QString::fromStdString(i.printable);
    }
    stream << Qt::endl;

    stream << "-----------\n";
    stream << "isWord      : ";
    for (auto& i : pt.basetext)
    {
        if (i.isVolve)
            stream << "v";
        else if (i.isConsonant)
            stream << "c";
        else
            stream << "n";
    }
    stream << Qt::endl;

    stream << "-----------\n";
    stream << "w_pos      : ";
    for (auto& i : pt.basetext)
    {
        stream << i.w_pos;
    }
    stream << Qt::endl;

    stream << "-----------\n";
    stream << "SP          :\n";
    for (size_t i = 0; i < pt.SP.size(); ++i)
    {
        stream << i << ": \"";
        for (auto it = pt.SP[i].first; it != pt.SP[i].second; ++it)
        {
            stream << QString::fromStdString(it->technic);
        }
        stream << "\"" << Qt::endl;
    }
    stream << Qt::endl;
    for (auto& i : pt.basetext)
    {
        stream << QString::fromStdString(i.origin) << QString::fromStdString(i.technic) << QString::fromStdString(i.printable) << i.isVolve << i.isConsonant << i.number << '|';
    }

    stream << "-----------\n";

    stream << "combinations:\n";
    for (size_t i = 0; i < pt.syllableCombinations.size(); ++i)
    {
        stream << i << ":\n";
        for (size_t j = 0; j < pt.syllableCombinations[i].size(); ++j)
        {
            for (int k = 0; k < 3; ++k)
                stream << QString::fromStdString(pt.syllableCombinations[i][j][k]->origin);
            stream << Qt::endl;
        }
        stream << Qt::endl;
    }
    stream << Qt::endl;

    std::string printable = "";
    for (auto it = pt.basetext.begin(); it != pt.basetext.end(); ++it)
        printable += it->printable;
    stream << "-----------\n";
    stream << "repeats:\n";
    for (auto& i : pt.repeats)
    {
        std::string letters = "";
        for (size_t j = 0; j < i.second.letters.size(); ++j)
            letters += i.second.letters[j].origin;
        std::string swords = "";
        for (auto& j : i.second._words)
        {
            swords += "<" + j + ">";
        }
        std::vector<std::string> combs;

        stream << "key : " << QString::fromStdString(i.first) << Qt::endl;
        stream << "Repeat.power : " << i.second.power << Qt::endl;
        stream << "Repeat.count : " << i.second.count << Qt::endl;
        stream << "Repeat.letters : " << QString::fromStdString(letters) << Qt::endl;
        stream << "Repeat._words : " << QString::fromStdString(swords) << Qt::endl;
        stream << "Repeat.combs : ";
        for (size_t j = 0; j < i.second.combs.size(); ++j)
        {
            std::string tmpComb = "";
            for (size_t k = 0; k < i.second.combs[j].size(); ++k)
            {
                stream << QString::fromStdString(i.second.combs[j][k]->origin);
                tmpComb += i.second.combs[j][k]->origin;
            }
            combs.push_back(tmpComb);
            stream << " ";
        }
        stream << Qt::endl << Qt::endl;
    }
    stream << Qt::endl;
    stream << "-----------\n";
    out.close();
}

void Proccessing::createJson(std::string filename)
{
    nlohmann::json outJson;
    int counter = 0;
    for (auto& j : pt.basetext)
    {
        nlohmann::json tmpOutTextJson = {
            {"origin", j.origin},
            {"technic", j.technic},
            {"printable", j.printable},
            {"is_consonant", j.isConsonant},
            {"is_accent", j.accent},
            {"is_volve", j.isVolve},
            {"word", j.word},
            {"positions:", {
                               {"text", j.number},
                               {"syllab", j.syll},
                               {"word_start", j.fw_pos},
                               {"word_end", 0},
                               {"last_word_in_line", j.pEnd}
                           }}
        };
        outJson["text"][counter] = tmpOutTextJson;
        ++counter;
    }
    counter = 0;
    for (auto& i : pt.repeats)
    {
        std::string key = i.first;
        std::string power = std::to_string(i.second.power);
        std::string count = std::to_string(i.second.count);
        std::vector<int> letters_array;
        letters_array.reserve(i.second.letters.size());
        std::string words;
        std::vector<std::string> combs;
        std::vector<std::vector<int>> indCombs;
        indCombs.reserve(i.second.combs.size());

        for (size_t j = 0; j < i.second.letters.size(); ++j)
            letters_array.push_back(i.second.letters[j].number);

        for (auto& j : i.second._words)
            words += j;

        for (size_t j = 0; j < i.second.combs.size(); ++j)
        {
            std::string tCombs = "";
            std::vector<int> tmpArr;
            tmpArr.reserve(i.second.combs[j].size());
            for (size_t k = 0; k < i.second.combs[j].size(); ++k){
                tCombs += i.second.combs[j][k]->origin;
                tmpArr.push_back(i.second.combs[j][k]->number);
            }
            indCombs.emplace_back(std::move(tmpArr));
            combs.emplace_back(std::move(tCombs));
        }

        nlohmann::json tmpOutRepeatJson = {
            {"key", key},
            {"count", count},
            {"power", power},
            {"letters", letters_array},
            {"combs", combs},
            {"indCombs", indCombs}
        };

        outJson["repeats:"][counter] = tmpOutRepeatJson;
        ++counter;
    }

    std::ofstream fout;
    fout.open(filename);
    if (!fout.is_open()){
        std::cout << "File cannot be opened!" << std::endl;
        return;
    };
    fout << std::setw(4) << outJson;
    fout.close();
}

double Proccessing::get_pwr(const std::forward_list<Letter>::iterator &a, const std::forward_list<Letter>::iterator &b)
{
    if (a->technic != b->technic)
        return 0;

    int dist = b->syll - a->syll;
    if (dist < 1)
        return 0;

    double mul = 1;
    int dist_w = b->word - a->word;
    double pwr = 1.0 / dist + 1.0 / (dist_w + 2);

    if ((a->origin == b->origin) && a->isConsonant)
        mul += 1;

    mul *= 1.0 / (1 + a->fw_pos + b->fw_pos);
    return pwr * mul;
}

double Proccessing::get_pwr_combs(const std::vector<std::forward_list<Letter>::iterator>& combA, const std::vector<std::forward_list<Letter>::iterator>& combB)
{
    double pwr = 0;

    for (size_t i = 0; i < combA.size(); ++i)
        for (size_t j = 0; j < combB.size(); ++j)
            pwr += get_pwr(combA[i], combB[j]);

    double mul_1 = 1;
    double mul_2 = 1;

    for (size_t i = 0; i < combA.size() - 1; ++i)
        mul_1 *= combA[i + 1]->number - combA[i]->number;

    for (size_t i = 0; i < combB.size() - 1; ++i)
        mul_2 *= combB[i + 1]->number - combB[i]->number;

    double mul = 10 * rusFilterComb(combA, CONFIG.getWords()).second
                 * rusFilterComb(combB, CONFIG.getWords()).second
                 * (1 + combA[combA.size()-1]->pEnd + combB[combB.size()-1]->pEnd);

    pwr *= 1.0 / (mul_1 + 1) + 1.0 / (mul_2 + 1);
    return pwr * mul;
}

double Proccessing::handlePower(std::map<std::string, Repeat>& repeats)
{
    double repeats_power = 0;

    for (auto &rep : repeats)
    {
        double pwr = 0;
        const auto& combs = rep.second.combs;

        for (size_t i = 0; i < combs.size(); ++i)
        {
            for (size_t j = i + 1; j < combs.size(); ++j)
            {
                pwr += get_pwr_combs(combs[i], combs[j]);
                if (combs[j][0]->number - combs[i][0]->number > 50)
                    break;
            }
        }

        rep.second.power = pwr;
        repeats_power += pwr;
    }

    return repeats_power;
}
