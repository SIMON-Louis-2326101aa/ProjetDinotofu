// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_COMBATCLASSSYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_COMBATCLASSSYSTEM_HPP

#include "entity/Entity.hpp"

#include <string>

class CombatClassSystem
{
public:
    static int getBaseEscapeChance(const Entity& entity);
    static int getBaseDamageReductionPercentage(const Entity& entity);

private:
    static std::string normalizeClassText(const std::string& classText);
};

#endif
