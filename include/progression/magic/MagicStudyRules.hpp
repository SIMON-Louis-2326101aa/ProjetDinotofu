// EN: Central prerequisites for studying grimoires and safely releasing spell scrolls.
// FR: Prérequis centraux pour étudier les grimoires et déclencher les parchemins magiques.
#ifndef INCLUDE_PROGRESSION_MAGIC_MAGICSTUDYRULES_HPP
#define INCLUDE_PROGRESSION_MAGIC_MAGICSTUDYRULES_HPP

#include <string>
#include <vector>

class Player;

enum class MagicStudyOutcome
{
    NotApplicable,
    Blocked,
    AlreadyKnown,
    Learned
};

struct MagicStudyRequirement
{
    std::string itemId;
    std::string skillId;
    std::string skillName;
    int minimumLevel = 1;
    int minimumMaxMana = 0;
    int minimumKnowledge = 0;
    std::string languageId;
    int minimumLanguageLevel = 0;
};

struct MagicScrollRequirement
{
    std::string nameFragment;
    int minimumLevel = 1;
    int minimumMaxMana = 0;
    int minimumKnowledge = 0;
    std::string languageId;
    int minimumLanguageLevel = 0;
};

namespace MagicStudyRules
{
    const MagicStudyRequirement* findStudyRequirement(const std::string& itemId);
    bool isMagicStudyItem(const std::string& itemId);
    int magicKnowledgeScore(const Player& player);
    std::string magicKnowledgeLabel(int score);
    std::vector<std::string> studyBlockers(const Player& player, const MagicStudyRequirement& requirement);
    std::vector<std::string> studyRequirementLines(const Player& player, const std::string& itemId);
    MagicStudyOutcome attemptStudy(Player& player, const std::string& itemId, std::vector<std::string>& notes);

    const MagicScrollRequirement* findScrollRequirement(const std::string& scrollName);
    std::vector<std::string> scrollUseBlockers(const Player& player, const std::string& scrollName);
    bool canUseScroll(const Player& player, const std::string& scrollName);
}

#endif
