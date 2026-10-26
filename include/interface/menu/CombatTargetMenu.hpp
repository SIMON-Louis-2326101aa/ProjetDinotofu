// EN: CombatTargetMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatTargetMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMBATTARGETMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATTARGETMENU_HPP

#include "entity/Player.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"

#include <string>

class CombatTargetMenu
{
public:
    static bool openForAttack(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        std::string* resolvedTargetName = nullptr,
        std::string* resolvedTargetId = nullptr,
        bool* targetForcedByCombatRules = nullptr
    );

    static bool openForDamagePotion(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        int potionDamageBonus
    );

    static bool openForBoostedAttack(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        int damageBonus
    );

    static bool openForClassSkill(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        std::string* resolvedTargetName = nullptr,
        std::string* resolvedTargetId = nullptr
    );

private:
    // EN: chooseTarget declares or implements a focused behavior used by this module.
    // FR: chooseTarget déclare ou implémente un comportement précis utilisé par ce module.
    static int chooseTarget(const EnemyCombatQueue& wave);

    static bool openTargetMenu(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        bool boostedAttack,
        int damageBonus,
        std::string* resolvedTargetName = nullptr,
        std::string* resolvedTargetId = nullptr,
        bool* targetForcedByCombatRules = nullptr
    );
};

#endif
