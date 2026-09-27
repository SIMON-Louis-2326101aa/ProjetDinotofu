#ifndef INCLUDE_QUEST_LANGUAGE_QUESTLANGUAGESYSTEM_HPP
#define INCLUDE_QUEST_LANGUAGE_QUESTLANGUAGESYSTEM_HPP

#include <string>

struct Quest;
class Player;

class QuestLanguageSystem
{
public:
    static void assignOptionalForeignLanguage(Quest& quest);
    static bool canRead(const Player& player, const Quest& quest);
    static std::string requirementLine(const Player& player, const Quest& quest);
    static std::string readableObjective(const Player& player, const Quest& quest);
};

#endif
