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
#include "interface/model/MenuScreen.hpp"

#include <iostream>
#include <vector>

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
        std::cout << "Aucune cible n'est disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (targetableSlots.size() == 1 && targetableSlots[0]->getEntity() != nullptr)
    {
        CombatActions::executeAttack(attacker, mainDefender, random);
        return true;
    }

    MenuScreen screen("CHOIX DE CIBLE", "combat_group.target_select");
    screen.addBackOption();

    for (int i = 0; i < static_cast<int>(targetableSlots.size()); ++i)
    {
        CombatUnitSlot* slot = targetableSlots[i];
        std::string hint = slot->getKind() == CombatUnitKind::Summon
            ? "invocation"
            : "cible principale";

        screen.addOption(
            i + 1,
            slot->getDisplayName(),
            hint,
            true,
            "combat_group.target"
        );
    }

    TerminalInterface::renderMenuScreen(screen);

    int choice = Console::askNumberBetween(
        0,
        static_cast<int>(targetableSlots.size()),
        "Cible invalide. Choisis une cible affichée, ou 0 pour revenir."
    );

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    CombatUnitSlot* selectedSlot = targetableSlots[choice - 1];

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

    std::cout << "Cette cible n'est plus disponible." << std::endl;
    std::cout << std::endl;

    return false;
}
