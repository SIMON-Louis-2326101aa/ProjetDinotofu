#include "progression/Level.hpp"
#include "progression/TechniqueProgressionRules.hpp"

#include <cassert>

int main()
{
    assert(Level::getMaxLevel() == 255);

    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("safe_rally") == 3);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("guard_break") == 5);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("stop_hit") == 12);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("line_sweep") == 24);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("line_break") == 40);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("material_sacrifice") == 60);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("random_ricochet") == 65);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("opportunist_execute") == 80);
    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("unstable_mix") == 100);

    assert(TechniqueProgressionRules::chainUnlockLevel(false) == 8);
    assert(TechniqueProgressionRules::chainUnlockLevel(true) == 15);
    assert(TechniqueProgressionRules::tripleChainUnlockLevel() == 35);
    assert(TechniqueProgressionRules::quadrupleChainUnlockLevel() == 120);
    assert(TechniqueProgressionRules::genericMasteryUnlockLevel("chain_mastery") == 120);
    assert(TechniqueProgressionRules::genericMasteryUnlockLevel("battlefield_mastery") == 150);
    assert(TechniqueProgressionRules::genericMasteryUnlockLevel("veteran_rally") == 180);
    assert(TechniqueProgressionRules::genericMasteryUnlockLevel("master_execution") == 210);
    assert(TechniqueProgressionRules::genericMasteryUnlockLevel("perfect_guard_break") == 240);
    assert(TechniqueProgressionRules::genericMasteryUnlockLevel("unknown_mastery") == 255);

    assert(TechniqueProgressionRules::weaponPracticeUnlockLevel("ranger_eye") == 6);
    assert(TechniqueProgressionRules::weaponPracticeUnlockLevel("chain_execution") == 10);
    assert(TechniqueProgressionRules::weaponPracticeUnlockLevel("armor_crack") == 12);

    assert(TechniqueProgressionRules::classIdentityUnlockLevel("Chevalier") == 8);
    assert(TechniqueProgressionRules::classIdentityUnlockLevel("Assassin") == 10);
    assert(TechniqueProgressionRules::classIdentityUnlockLevel("Mage") == 10);
    assert(TechniqueProgressionRules::classTechniqueUnlockLevel("arcane_mark") == 10);
    assert(TechniqueProgressionRules::classTechniqueUnlockLevel("elemental_ward") == 35);
    assert(TechniqueProgressionRules::classTechniqueUnlockLevel("controlled_overload") == 55);
    assert(TechniqueProgressionRules::classTechniqueUnlockLevel("resistance_rift") == 75);
    assert(TechniqueProgressionRules::classTechniqueUnlockLevel("pocket_strike") == 40);

    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_arcane_mark") == 8);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_arcane_binding") == 15);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_elemental_ward") == 25);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_frost_needle") == 20);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_mana_suture") == 30);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_occult_bramble") == 25);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("learned_resistance_rift") == 55);
    assert(TechniqueProgressionRules::magicStudyUnlockLevel("unknown_study") == 255);

    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("coup_de_pied") == 1);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("brise_garde") == 5);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("pas_ombre") == 10);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("coup_arc") == 24);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("perce_rempart") == 35);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("rupture_formation") == 40);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("lien_invocation") == 45);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("danse_lame") == 50);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("chaine_etats") == 55);
    assert(TechniqueProgressionRules::tacticalActionUnlockLevel("unknown_tactical_action") == 255);

    assert(TechniqueProgressionRules::genericTechniqueUnlockLevel("unstable_mix") < Level::getMaxLevel());
    assert(TechniqueProgressionRules::classTechniqueUnlockLevel("resistance_rift") < Level::getMaxLevel());
    return 0;
}
