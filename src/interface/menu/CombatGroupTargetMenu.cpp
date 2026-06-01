// EN: CombatGroupTargetMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroupTargetMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Lets the player choose targets through CombatGroup slots during the gradual migration to real group fights.

#include "interface/menu/CombatGroupTargetMenu.hpp"

#include "combat/CombatActions.hpp"
#include "combat/group/CombatGroup.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatSide.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/group/CombatUnitSlot.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"

#include <string>
#include <vector>

namespace
{
    MenuOptionItemData buildTargetItemData(const CombatUnitSlot& slot)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = slot.getKind() == CombatUnitKind::Summon ? "summon" : "entity";
        itemData.section = "Cibles";
        itemData.actionType = "target";
        itemData.name = slot.getDisplayName();
        itemData.detail = slot.getKind() == CombatUnitKind::Summon ? "Invocation" : "Cible principale";

        if (const Entity* entity = slot.getEntity())
        {
            itemData.status = "PV : " + std::to_string(entity->getHp()) + "/" + std::to_string(entity->getMaxHp());
            itemData.important = entity->getMaxHp() > 0 && entity->getHp() * 100 <= entity->getMaxHp() * 35;
        }
        else if (const Summon* summon = slot.getSummon())
        {
            itemData.status = "PV : " + std::to_string(summon->getHp()) + "/" + std::to_string(summon->getMaxHp());
            itemData.progress = "Durée : " + std::to_string(summon->getDurationTurns()) + " tour(s)";
            itemData.owner = summon->getOwnerName();
        }

        return itemData;
    }
}

bool CombatGroupTargetMenu::openSingleEnemyAttack(
    Entity& attacker,
    Entity& mainDefender,
    std::vector<Summon>& defenderSummons,
    Random& random
)
{
    CombatGroup targetGroup = CombatGroupBuilder::buildSideFromEntityAndSummons(
        mainDefender,
        defenderSummons,
        CombatSide::EnemySide,
        CombatUnitKind::Enemy
    );

    std::vector<CombatUnitSlot*> targetableSlots = targetGroup.getTargetableSlots();

    if (targetableSlots.empty())
    {
        MessageScreen::show(
            "AUCUNE CIBLE",
            "combat_group.target_select.empty",
            {
                "Aucune cible n'est disponible.",
                "L'action ne part pas dans le vide."
            }
        );
        return false;
    }

    if (targetableSlots.size() == 1 && targetableSlots[0]->getEntity() != nullptr)
    {
        CombatActions::executeAttack(attacker, mainDefender, random);
        return true;
    }

    constexpr std::size_t itemsPerPage = 6;
    std::size_t pageIndex = 0;
    int selectedIndex = -1;

    while (selectedIndex < 0)
    {
        const std::size_t totalPages = PagedMenu::pageCount(targetableSlots.size(), itemsPerPage);
        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(targetableSlots.size(), pageIndex, itemsPerPage);

        MenuScreen screen("CHOIX DE CIBLE", "combat_group.target_select");
        screen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, targetableSlots.size()));
        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);

        for (std::size_t i = first; i < last; ++i)
        {
            CombatUnitSlot* slot = targetableSlots[i];
            std::string hint = slot->getKind() == CombatUnitKind::Summon
                ? "invocation"
                : "cible principale";

            screen.addOption(
                static_cast<int>(i - first + 1),
                slot->getDisplayName(),
                hint,
                true,
                "combat_group.target." + std::to_string(i),
                buildTargetItemData(*slot)
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Cible invalide. Choisis une cible affichée, 98/99 pour naviguer, ou 0 pour revenir."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice >= 1 && choice <= visibleCount)
        {
            selectedIndex = static_cast<int>(first) + choice - 1;
        }
    }

    CombatUnitSlot* selectedSlot = targetableSlots[selectedIndex];

    if (selectedSlot->getEntity() != nullptr)
    {
        CombatActions::executeAttack(
            attacker,
            *selectedSlot->getEntity(),
            random
        );

        return true;
    }

    if (selectedSlot->getSummon() != nullptr)
    {
        SummonCombatSystem::entityAttacksSummon(
            attacker,
            *selectedSlot->getSummon(),
            random
        );

        SummonCombatSystem::removeInactiveSummons(defenderSummons);
        return true;
    }

    MessageScreen::show(
        "CIBLE INTROUVABLE",
        "combat_group.target_select.missing",
        {
            "Cette cible n'est plus disponible.",
            "Le combat continue sans gaspiller l'action."
        }
    );

    return false;
}
