#ifndef INCLUDE_ADVENTURE_CONTENT_BIOMENONCOMBATINTERACTIONSYSTEM_HPP
#define INCLUDE_ADVENTURE_CONTENT_BIOMENONCOMBATINTERACTIONSYSTEM_HPP

#include <string>
#include <vector>

struct BiomeNonCombatChoice
{
    int id = 0;
    std::string label;
    std::string description;
    std::string resultLine;
    int explorationRollShift = 0;
    int questProgress = 0;
};

struct BiomeNonCombatInteraction
{
    bool active = false;
    std::string id;
    std::string title;
    std::string prompt;
    std::vector<std::string> contextLines;
    std::vector<BiomeNonCombatChoice> choices;
};

struct BiomeNonCombatInteractionResult
{
    bool resolved = false;
    std::string interactionId;
    std::string choiceLabel;
    std::vector<std::string> lines;
    int explorationRollShift = 0;
    int questProgress = 0;
    bool notableForLongTermHistory = false;
};

class BiomeNonCombatInteractionSystem
{
public:
    static BiomeNonCombatInteraction buildCurrentInteraction(const std::string& biomeName, int worldDay, int dayProgressUnit);
    static BiomeNonCombatInteractionResult resolve(const BiomeNonCombatInteraction& interaction, int choiceId);
    static std::string journalKey(const std::string& biomeName, int worldDay, const std::string& interactionId);
};

#endif
