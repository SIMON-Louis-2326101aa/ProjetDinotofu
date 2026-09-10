// EN: MonsterBehaviorProfile.hpp defines readable enemy behavior profiles for Dinotofu combat.
// FR: MonsterBehaviorProfile.hpp définit des profils de comportement ennemis lisibles pour les combats de Dinotofu.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_PROFILE_MONSTER_BEHAVIOR_PROFILE_HPP
#define INCLUDE_COMBAT_PROFILE_MONSTER_BEHAVIOR_PROFILE_HPP

#include "entity/Entity.hpp"
#include "entity/Monster.hpp"
#include "core/Random.hpp"

#include <ostream>
#include <string>
#include <vector>

struct MonsterBehaviorProfile
{
    std::string archetype;
    std::string behaviorLine;
    std::string attackDescription;
    std::string signatureMove;
    std::string counterplayLine;
    std::vector<std::string> strengths;
    std::vector<std::string> weaknesses;
    std::vector<std::string> reactions;
    int incomingAccuracyModifier = 0;
    std::string accuracyLine;
    int physicalDamageModifierPercent = 0;
    int magicalDamageModifierPercent = 0;
    std::string durabilityLine;
};

namespace MonsterBehaviorProfileCatalog
{
    MonsterBehaviorProfile build(const Monster& monster);
    std::vector<std::string> buildObservationLines(const Monster& monster, bool detailed);
    std::string buildBestiarySentence(const Monster& monster);
    std::vector<std::string> buildTurnFlavorLines(const Monster& monster, const Entity& defender, bool detailed = false);
    std::string buildAttackImpactLine(const Monster& monster, int rawDamage, bool critical, bool boosted);
    int getIncomingAccuracyModifier(const Monster& monster);
    std::string getAccuracyLine(const Monster& monster);
    int getPhysicalDamageModifierPercent(const Monster& monster);
    int getMagicalDamageModifierPercent(const Monster& monster);
    std::string getDurabilityLine(const Monster& monster);
    bool applyEvasiveMissReaction(Monster& monster, Entity& attacker, Random& random, std::ostream& output);
    bool applyIncomingHitReaction(Monster& monster, Entity& attacker, Random& random, int receivedDamage, std::ostream& output);
    int applySignaturePreImpact(Monster& monster, Entity& defender, Random& random, int currentRawDamage, std::ostream& output);
}

#endif
