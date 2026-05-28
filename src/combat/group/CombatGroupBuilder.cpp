// EN: CombatGroupBuilder.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroupBuilder.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Builds and displays combat groups from existing entities, waves, and summons during the gradual slot-system migration.

#include "combat/group/CombatGroupBuilder.hpp"

#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <vector>

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


CombatGroup CombatGroupBuilder::buildSideFromPlayersAndSummons(
    std::vector<Player*>& players,
    std::vector<Summon>& summons,
    CombatSide side
)
{
    CombatGroup group(3, 2);

    // Placement allié validé : centre = joueur principal, deux côtés = joueurs/personnages,
    // extrémités = invocations uniquement. Les joueurs restent prioritaires sur les alliés IA.
    const int playerSlots[3] = {0, 1, 2};
    int placedPlayers = 0;

    for (Player* player : players)
    {
        if (player == nullptr)
        {
            continue;
        }

        if (placedPlayers >= 3)
        {
            MessageScreen::show(
                "FORMATION COMPLÈTE",
                "combat.group.player.overflow",
                {
                    player->getName() + " ne peut pas entrer dans la ligne active.",
                    "Les trois emplacements de personnage sont déjà pris."
                },
                false
            );
            continue;
        }

        CombatUnitKind kind = placedPlayers == 0
            ? CombatUnitKind::MainFighter
            : CombatUnitKind::Ally;

        group.addSlot(
            CombatUnitSlot::createEntitySlot(
                playerSlots[placedPlayers],
                side,
                kind,
                *player
            )
        );

        placedPlayers++;
    }

    const int summonSlots[2] = {3, 4};
    int placedSummons = 0;

    for (Summon& summon : summons)
    {
        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        if (placedSummons >= 2)
        {
            MessageScreen::show(
                "INVOCATION EN RETRAIT",
                "combat.group.summon.overflow",
                {
                    summon.getName() + " reste en retrait.",
                    "Les extrémités réservées aux invocations sont déjà occupées."
                },
                false
            );
            continue;
        }

        group.addSlot(
            CombatUnitSlot::createSummonSlot(
                summonSlots[placedSummons],
                side,
                summon
            )
        );

        placedSummons++;
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
    std::vector<std::string> lines;

    if (group.getSlotCount() <= 0)
    {
        lines.push_back("Aucune unité visible.");
    }

    for (const CombatUnitSlot& slot : group.getSlots())
    {
        if (slot.isEmpty())
        {
            continue;
        }

        std::string line = "[" + std::to_string(slot.getSlotIndex()) + " - "
            + getFormationSlotLabel(slot.getSlotIndex()) + "] " + slot.getDisplayName();

        if (!slot.isAlive())
        {
            line += " - hors combat";
        }

        if (slot.getKind() == CombatUnitKind::Summon)
        {
            line += " - invocation";
        }
        else if (slot.getKind() == CombatUnitKind::Boss)
        {
            line += " - boss";
        }
        else if (slot.getKind() == CombatUnitKind::Ally)
        {
            line += " - allié";
        }
        else if (slot.getKind() == CombatUnitKind::Enemy)
        {
            line += " - ennemi";
        }
        else if (slot.getKind() == CombatUnitKind::MainFighter)
        {
            line += " - joueur";
        }

        lines.push_back(line);
    }

    MessageScreen::show(title, "combat.group.display", lines, false);
}


std::string CombatGroupBuilder::getFormationSlotLabel(int slotIndex)
{
    switch (slotIndex)
    {
        case 0: return "centre";
        case 1: return "côté gauche";
        case 2: return "côté droit";
        case 3: return "extrémité gauche / invocation";
        case 4: return "extrémité droite / invocation";
        default: return "hors formation";
    }
}

void CombatGroupBuilder::displayFormationRules()
{
    MessageScreen::show(
        "RÈGLES DE FORMATION",
        "combat.group.formation.rules",
        {
            "Formation alliée :",
            "- centre : joueur principal ;",
            "- côtés : joueurs secondaires ou personnages alliés ;",
            "- extrémités : invocations uniquement ;",
            "- priorité : joueurs > personnages alliés > invocations."
        },
        false
    );
}
