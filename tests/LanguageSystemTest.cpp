#include "progression/language/LanguageSystem.hpp"

#include <cassert>

int main()
{
    assert(LanguageSystem::displayName("commun") == "Commun");
    assert(LanguageSystem::displayName("infernal") == "Infernal");
    assert(LanguageSystem::languageForMonsterRace(Race::Gobelin) == "gobelin");
    assert(LanguageSystem::languageForMonsterRace(Race::Demon) == "infernal");
    assert(LanguageSystem::languageForMonsterRace(Race::Draconide) == "draconique");
    assert(LanguageSystem::languageForMonsterRace(Race::Fee) == "feerique");
    assert(LanguageSystem::languageForMonsterRace(Race::AnomalieArcanique) == "anormal");
    assert(LanguageSystem::nativeLanguageForPlayerRace(CharacterRace::Demon) == "infernal");
    assert(LanguageSystem::nativeLanguageForPlayerRace(CharacterRace::HalfDragon) == "draconique");
    assert(LanguageSystem::studyTargetLevelForItem("language_goblin_primer") == 1);
    assert(LanguageSystem::studyTargetLevelForItem("language_goblin_course") == 2);
    assert(LanguageSystem::studyTargetLevelForItem("language_anomaly_notation") == 1);
    return 0;
}
