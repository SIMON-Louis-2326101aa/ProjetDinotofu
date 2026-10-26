// EN: CombatGroupBuilder.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroupBuilder.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Builds and displays combat groups from existing entities, waves, and summons during the gradual slot-system migration.

#include "combat/group/CombatGroupBuilder.hpp"

#include "entity/Monster.hpp"
#include "combat/unit/CombatSlotRules.hpp"
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
    CombatGroup group(CombatSlotRules::SharedActiveSlots, CombatSlotRules::SummonOnlyBonusSlots);

    group.addSlot(
        CombatUnitSlot::createEntitySlot(
            0,
            side,
            mainKind,
            mainEntity
        )
    );

    // One fighter occupies the centre. Summons first fill the two remaining shared places,
    // then the two summon-only bonus places.
    int nextSharedSlot = 1;
    int nextBonusSlot = CombatSlotRules::SharedActiveSlots;

    for (Summon& summon : summons)
    {
        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        int slotIndex = -1;
        if (nextSharedSlot < CombatSlotRules::SharedActiveSlots)
        {
            slotIndex = nextSharedSlot++;
        }
        else if (nextBonusSlot < CombatSlotRules::MaxVisibleActiveSlots)
        {
            slotIndex = nextBonusSlot++;
        }
        else
        {
            break;
        }

        group.addSlot(CombatUnitSlot::createSummonSlot(slotIndex, side, summon));
    }

    return group;
}


CombatGroup CombatGroupBuilder::buildSideFromPlayersAndSummons(
    std::vector<Player*>& players,
    std::vector<Summon>& summons,
    CombatSide side
)
{
    CombatGroup group(CombatSlotRules::SharedActiveSlots, CombatSlotRules::SummonOnlyBonusSlots);

    // Priority is strict: players first. Any shared place left after them may be occupied by
    // allied AI later or, for this builder, by a summon. Two extra positions remain summon-only.
    int placedPlayers = 0;
    for (Player* player : players)
    {
        if (player == nullptr) continue;

        if (placedPlayers >= CombatSlotRules::SharedActiveSlots)
        {
            MessageScreen::show(
                "FORMATION COMPLÈTE",
                "combat.group.player.overflow",
                {
                    player->getName() + " ne peut pas entrer dans la ligne active.",
                    "Les trois places partagées sont déjà prises par des joueurs prioritaires."
                },
                false
            );
            continue;
        }

        const CombatUnitKind kind = placedPlayers == 0
            ? CombatUnitKind::MainFighter
            : CombatUnitKind::Ally;
        group.addSlot(CombatUnitSlot::createEntitySlot(placedPlayers, side, kind, *player));
        ++placedPlayers;
    }

    int nextSharedSlot = placedPlayers;
    int nextBonusSlot = CombatSlotRules::SharedActiveSlots;
    for (Summon& summon : summons)
    {
        if (summon.isDead() || summon.isExpired()) continue;

        int slotIndex = -1;
        if (nextSharedSlot < CombatSlotRules::SharedActiveSlots)
        {
            slotIndex = nextSharedSlot++;
        }
        else if (nextBonusSlot < CombatSlotRules::MaxVisibleActiveSlots)
        {
            slotIndex = nextBonusSlot++;
        }
        else
        {
            MessageScreen::show(
                "INVOCATION EN ATTENTE",
                "combat.group.summon.overflow",
                {
                    summon.getName() + " reste dans la file d'attente.",
                    "Les trois places partagées et les deux places bonus d'invocation sont occupées."
                },
                false
            );
            continue;
        }

        group.addSlot(CombatUnitSlot::createSummonSlot(slotIndex, side, summon));
    }

    return group;
}


CombatGroup CombatGroupBuilder::buildSideFromWave(
    EnemyCombatQueue& wave,
    CombatSide side
)
{
    CombatGroup group(CombatSlotRules::SharedActiveSlots, CombatSlotRules::SummonOnlyBonusSlots);

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
            "- 3 places partagées : joueurs > personnages alliés > invocations ;",
            "- 2 places supplémentaires : invocations uniquement ;",
            "- si une place partagée reste libre, une invocation peut l'occuper ;",
            "- au-delà : file d'attente/réserve, jamais une sixième unité visible."
        },
        false
    );
}
