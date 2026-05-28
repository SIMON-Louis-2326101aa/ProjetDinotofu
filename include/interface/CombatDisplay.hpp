// EN: CombatDisplay.hpp centralizes combat presentation for terminal and future GUI.
// FR: CombatDisplay.hpp centralise la présentation du combat pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_COMBATDISPLAY_HPP
#define INCLUDE_INTERFACE_COMBATDISPLAY_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "combat/group/CombatGroup.hpp"
#include "entity/Entity.hpp"
#include "combat/summon/Summon.hpp"
#include "interface/model/CombatStateSnapshot.hpp"

#include <vector>

class CombatDisplay
{
public:
    static void displayCombatResult(
        const Entity& combattant1,
        const Entity& combattant2
    );

    static GuiCombatStateSnapshot buildDuelSnapshot(
        const Entity& playerSideEntity,
        const Entity& enemySideEntity,
        const std::string& title,
        const std::string& phase,
        int turnNumber = 0
    );

    static GuiCombatStateSnapshot buildGroupSnapshot(
        const CombatGroup& playerGroup,
        const CombatGroup& enemyGroup,
        const std::string& title,
        const std::string& phase,
        int turnNumber = 0
    );

    static GuiCombatStateSnapshot buildWaveSnapshot(
        const Entity& playerSideEntity,
        const EnemyCombatQueue& wave,
        const std::vector<Summon>& playerSummons,
        const std::string& title,
        const std::string& phase,
        int turnNumber = 0
    );

    static GuiCombatStateSnapshot buildWavePartySnapshot(
        const std::vector<Entity*>& playerSideEntities,
        const EnemyCombatQueue& wave,
        const std::vector<Summon>& playerSummons,
        const std::string& title,
        const std::string& phase,
        int turnNumber = 0
    );

    static void displayCombatState(const GuiCombatStateSnapshot& snapshot, bool waitAndClear = false);
};

#endif
