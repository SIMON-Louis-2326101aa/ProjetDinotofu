// EN: Central progression thresholds for player combat techniques on the 1-255 level curve.
// FR: Seuils centraux de progression des techniques de combat du joueur sur la courbe 1-255.
#ifndef INCLUDE_PROGRESSION_TECHNIQUEPROGRESSIONRULES_HPP
#define INCLUDE_PROGRESSION_TECHNIQUEPROGRESSIONRULES_HPP

#include <string>

namespace TechniqueProgressionRules
{
    int genericTechniqueUnlockLevel(const std::string& techniqueId);
    int chainUnlockLevel(bool magicProfile);
    int tripleChainUnlockLevel();
    int quadrupleChainUnlockLevel();
    int genericMasteryUnlockLevel(const std::string& masteryId);
    int weaponPracticeUnlockLevel(const std::string& skillId);
    int classTechniqueUnlockLevel(const std::string& techniqueId);
    int classIdentityUnlockLevel(const std::string& className);
    int magicStudyUnlockLevel(const std::string& learnedSkillId);
    int tacticalActionUnlockLevel(const std::string& actionKey);
}

#endif
