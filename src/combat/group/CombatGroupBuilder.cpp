// EN: CombatGroupBuilder.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroupBuilder.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Builds and displays combat groups from existing entities, waves, and summons during the gradual slot-system migration.

#include "combat/group/CombatGroupBuilder.hpp"

#include "entity/Monster.hpp"

#include <iostream>

CombatGroup CombatGroupBuilder::buildSideFromEntityAndSummons(
    Entity& mainEntity,
    std::vector<Summon>& summons,
    CombatSide side,
    CombatUnitKind mainKind
)
{
    CombatGroup group(3, 2);

    group.addSlot(
        CombatUnitSlot::createEntitySlot(
            0,
            side,
            mainKind,
            mainEntity
        )
    );

    int slotIndex = 1;

    for (Summon& summon : summons)
    {
        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        group.addSlot(
            CombatUnitSlot::createSummonSlot(
                slotIndex,
                side,
                summon
            )
        );

        slotIndex++;
    }

    return group;
}

CombatGroup CombatGroupBuilder::buildSideFromWave(
    EnemyCombatQueue& wave,
    CombatSide side
)
{
    CombatGroup group(3, 2);

    if (!wave.hasActiveEnemies())
    {
        wave.initializeFrontLine();
    }

    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        if (!wave.isActiveIndexValid(i))
        {
            continue;
        }

        Monster& monster = wave.getActiveEnemy(i);

        if (monster.isDead())
        {
            continue;
        }

        group.addSlot(
            CombatUnitSlot::createEntitySlot(
                i,
                side,
                CombatUnitKind::Enemy,
                monster
            )
        );
    }

    return group;
}

void CombatGroupBuilder::displayGroup(
    const CombatGroup& group,
    const std::string& title
)
{
    std::cout << "========== " << title << " ==========" << std::endl;

    if (group.getSlotCount() <= 0)
    {
        std::cout << "Aucune unité visible." << std::endl;
    }

    for (const CombatUnitSlot& slot : group.getSlots())
    {
        if (slot.isEmpty())
        {
            continue;
        }

        std::cout << "[" << slot.getSlotIndex() << "] "
                  << slot.getDisplayName();

        if (!slot.isAlive())
        {
            std::cout << " - hors combat";
        }

        if (slot.getKind() == CombatUnitKind::Summon)
        {
            std::cout << " - invocation";
        }
        else if (slot.getKind() == CombatUnitKind::Boss)
        {
            std::cout << " - boss";
        }
        else if (slot.getKind() == CombatUnitKind::Ally)
        {
            std::cout << " - allié";
        }
        else if (slot.getKind() == CombatUnitKind::Enemy)
        {
            std::cout << " - ennemi";
        }
        else if (slot.getKind() == CombatUnitKind::MainFighter)
        {
            std::cout << " - joueur";
        }

        std::cout << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}
