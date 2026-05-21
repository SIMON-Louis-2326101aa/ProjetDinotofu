// EN: CombatGroupBuilder.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroupBuilder.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Builds and displays combat groups from existing entities, waves, and summons during the gradual slot-system migration.

#ifndef INCLUDE_COMBAT_GROUP_COMBATGROUPBUILDER_HPP
#define INCLUDE_COMBAT_GROUP_COMBATGROUPBUILDER_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "combat/group/CombatGroup.hpp"
#include "combat/group/CombatSide.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/summon/Summon.hpp"
#include "entity/Entity.hpp"

#include <string>
#include <vector>

class CombatGroupBuilder
{
public:
    static CombatGroup buildSideFromEntityAndSummons(
        Entity& mainEntity,
        std::vector<Summon>& summons,
        CombatSide side,
        CombatUnitKind mainKind
    );

    static CombatGroup buildSideFromWave(
        EnemyCombatQueue& wave,
        CombatSide side
    );

    static void displayGroup(
        const CombatGroup& group,
        const std::string& title
    );
};

#endif
