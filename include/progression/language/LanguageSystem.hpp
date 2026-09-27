#ifndef INCLUDE_PROGRESSION_LANGUAGE_LANGUAGESYSTEM_HPP
#define INCLUDE_PROGRESSION_LANGUAGE_LANGUAGESYSTEM_HPP

#include "character/CharacterRace.hpp"
#include "entity/Race.hpp"
#include <string>
#include <vector>

class Player;
class Random;

struct LanguageDefinition
{
    std::string id;
    std::string name;
    std::string familyDescription;
    bool learnableAtLibrary = true;
    bool unstable = false;
};

class LanguageSystem
{
public:
    static const std::vector<LanguageDefinition>& getCatalog();
    static const LanguageDefinition* find(const std::string& languageId);
    static std::string displayName(const std::string& languageId);
    static std::string languageForMonsterRace(Race race);
    static std::string nativeLanguageForPlayerRace(CharacterRace race);
    static void ensureStarterLanguages(Player& player);
    static int getKnowledgeLevel(const Player& player, const std::string& languageId);
    static bool understands(const Player& player, const std::string& languageId, int requiredLevel = 2);
    static std::string knowledgeLabel(int level);
    static std::string studyHint(const std::string& languageId, int level);
    static bool applyStudyItem(Player& player, const std::string& itemId, std::vector<std::string>* notes = nullptr);
    static bool canPracticeTowardFluency(const Player& player, const std::string& languageId);
    static bool applyGuidedPractice(Player& player, const std::string& languageId, int practiceQuality = 1, std::vector<std::string>* notes = nullptr);
    static std::string languageIdForStudyItem(const std::string& itemId);
    static int studyTargetLevelForItem(const std::string& itemId);
    static std::string renderSpeechForKnowledge(
        const Player& player,
        const std::string& languageId,
        const std::string& foreignText,
        const std::string& translatedText
    );
};

#endif
