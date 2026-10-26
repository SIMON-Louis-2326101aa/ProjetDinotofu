#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "progression/magic/MagicStudyRules.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace
{
    bool containsText(const std::vector<std::string>& lines, const std::string& needle)
    {
        for (const std::string& line : lines)
        {
            if (line.find(needle) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
}

int main()
{
    const MagicStudyRequirement* basic = MagicStudyRules::findStudyRequirement("basic_magic_manual");
    assert(basic != nullptr);
    assert(basic->minimumLevel == 8);
    assert(basic->minimumMaxMana == 45);
    assert(basic->minimumKnowledge == 11);

    const MagicStudyRequirement* frost = MagicStudyRules::findStudyRequirement("frost_needle_grimoire");
    assert(frost != nullptr);
    assert(frost->languageId == "nain");
    assert(frost->minimumLanguageLevel == 2);

    const MagicStudyRequirement* rift = MagicStudyRules::findStudyRequirement("resistance_rift_grimoire");
    assert(rift != nullptr);
    assert(rift->minimumLevel == 55);
    assert(rift->minimumMaxMana == 120);
    assert(rift->minimumKnowledge == 20);
    assert(rift->languageId == "anormal");

    Player mage("Lecteur", ClassCatalog::createClassByName("Mage"));
    std::vector<std::string> notes;
    const auto basicRequirementLines = MagicStudyRules::studyRequirementLines(mage, "basic_magic_manual");
    assert(containsText(basicRequirementLines, "niveau 8"));
    assert(containsText(basicRequirementLines, "mana maximal 45"));
    assert(containsText(basicRequirementLines, "connaissances magiques 11"));
    assert(containsText(basicRequirementLines, "[À ATTEINDRE]"));
    assert(MagicStudyRules::attemptStudy(mage, "basic_magic_manual", notes) == MagicStudyOutcome::Blocked);
    assert(containsText(notes, "niveau 8 requis"));

    mage.setLoadedProgress(8, 0, mage.getHp());
    notes.clear();
    assert(MagicStudyRules::attemptStudy(mage, "basic_magic_manual", notes) == MagicStudyOutcome::Learned);
    assert(mage.isActiveSkillUnlocked("learned_arcane_mark"));

    mage.setLoadedProgress(20, 0, mage.getHp());
    notes.clear();
    assert(MagicStudyRules::attemptStudy(mage, "frost_needle_grimoire", notes) == MagicStudyOutcome::Blocked);
    assert(containsText(notes, "Nain"));
    mage.setLanguageKnowledgeLevel("nain", 1, false);
    notes.clear();
    assert(MagicStudyRules::attemptStudy(mage, "frost_needle_grimoire", notes) == MagicStudyOutcome::Blocked);
    assert(containsText(notes, "conversation"));
    mage.setLanguageKnowledgeLevel("nain", 2, false);
    const auto frostRequirementLines = MagicStudyRules::studyRequirementLines(mage, "frost_needle_grimoire");
    assert(containsText(frostRequirementLines, "Nain"));
    assert(containsText(frostRequirementLines, "[OK]"));
    notes.clear();
    assert(MagicStudyRules::attemptStudy(mage, "frost_needle_grimoire", notes) == MagicStudyOutcome::Learned);
    assert(mage.isActiveSkillUnlocked("learned_frost_needle"));

    Player undereducated("Apprenti", ClassCatalog::createClassByName("Mage"));
    undereducated.setLoadedProgress(30, 0, undereducated.getHp());
    undereducated.setLanguageKnowledgeLevel("celeste", 2, false);
    notes.clear();
    assert(MagicStudyRules::attemptStudy(undereducated, "mana_suture_grimoire", notes) == MagicStudyOutcome::Blocked);
    assert(containsText(notes, "connaissances magiques"));

    Player warrior("Bran", ClassCatalog::createClassByName("Guerrier"));
    warrior.setLoadedProgress(20, 0, warrior.getHp());
    notes.clear();
    assert(MagicStudyRules::attemptStudy(warrior, "basic_magic_manual", notes) == MagicStudyOutcome::Blocked);
    assert(containsText(notes, "voie magique insuffisante"));

    Player scrollUser("Papier", ClassCatalog::createClassByName("Guerrier"));
    assert(MagicStudyRules::canUseScroll(scrollUser, "Parchemin d'étincelle arcanique"));
    scrollUser.setLoadedProgress(8, 0, scrollUser.getHp());
    auto emberBlockers = MagicStudyRules::scrollUseBlockers(scrollUser, "Parchemin de braise errante");
    assert(containsText(emberBlockers, "Kitsune"));
    scrollUser.setLanguageKnowledgeLevel("kitsune", 1, false);
    assert(MagicStudyRules::canUseScroll(scrollUser, "Parchemin de braise errante"));

    Player riftScrollUser("Rift", ClassCatalog::createClassByName("Mage"));
    riftScrollUser.setLoadedProgress(25, 0, riftScrollUser.getHp());
    auto riftScrollBlockers = MagicStudyRules::scrollUseBlockers(riftScrollUser, "Parchemin de faille fragile");
    assert(containsText(riftScrollBlockers, "Notation anormale"));

    return 0;
}
