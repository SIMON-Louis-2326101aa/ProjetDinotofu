#ifndef INCLUDE_ADVENTURE_FLAVOR_EXPLORATIONLANGUAGETRACE_HPP
#define INCLUDE_ADVENTURE_FLAVOR_EXPLORATIONLANGUAGETRACE_HPP

#include <string>

class Player;

struct ExplorationLanguageTrace
{
    std::string languageId;
    std::string sourceText;
    std::string translatedMeaning;
    std::string contextHint;
    int requiredLevel = 1;
};

class ExplorationLanguageTraceCatalog
{
public:
    static ExplorationLanguageTrace forBiome(const std::string& biomeName);
    static bool hasTrace(const std::string& biomeName);
    static std::string renderForPlayer(const Player& player, const ExplorationLanguageTrace& trace);
};

#endif
