// EN: CombatTargetMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatTargetMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatTargetMenu.hpp"

#include "combat/TurnManager.hpp"
#include "combat/CombatActions.hpp"
#include "combat/action/CombatAttack.hpp"
#include "combat/system/ObservationSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"

#include <string>
#include <vector>

namespace
{
    // EN: findForcedTargetIndex declares or implements a focused behavior used by this module.
    // FR: findForcedTargetIndex déclare ou implémente un comportement précis utilisé par ce module.
    int findForcedTargetIndex(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& enemy = wave.getActiveEnemy(i);

            if (enemy.isProvoking() || enemy.hasHealingThreat())
            {
                return i;
            }
        }

        return -1;
    }
}

bool CombatTargetMenu::openForAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random
)
{
    return openTargetMenu(
        player,
        wave,
        random,
        false,
        0
    );
}

bool CombatTargetMenu::openForDamagePotion(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int potionDamageBonus
)
{
    return openForBoostedAttack(
        player,
        wave,
        random,
        potionDamageBonus
    );
}

bool CombatTargetMenu::openForBoostedAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int damageBonus
)
{
    return openTargetMenu(
        player,
        wave,
        random,
        true,
        damageBonus
    );
}

// EN: chooseTarget declares or implements a focused behavior used by this module.
// FR: chooseTarget déclare ou implémente un comportement précis utilisé par ce module.
int CombatTargetMenu::chooseTarget(const EnemyCombatQueue& wave)
{
    MenuScreen screen("CHOIX DE CIBLE", "combat.target_select");
    screen.addLine("Choisis une cible active.");
    screen.addLine("Les ennemis en attente ne peuvent pas encore être ciblés.");

    for (const std::string& summaryLine : wave.getQueueSummaryLines())
    {
        screen.addFooterLine(summaryLine);
    }

    screen.addBackOption("Retour au menu principal", "combat.target.back");

    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        const Monster& enemy = wave.getActiveEnemy(i);
        std::string hint = std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()) + " PV";

        if (enemy.isInvocation())
        {
            hint += " | invocation";
        }
        else if (enemy.isEvolved())
        {
            hint += " | variation évoluée";
        }
        else if (enemy.isElite())
        {
            hint += " | élite";
        }

        if (enemy.isProvoking())
        {
            hint += " | provocation";
        }

        if (enemy.hasHealingThreat())
        {
            hint += " | soigneur marqué";
        }

        screen.addOption(
            i + 1,
            enemy.getName(),
            hint,
            !enemy.isDead(),
            "combat.target.select." + std::to_string(i)
        );
    }

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Cible invalide. Choisis une cible affichée, ou 0 pour revenir."
    );

    if (choice == 0)
    {
        return -1;
    }

    return choice - 1;
}

bool CombatTargetMenu::openTargetMenu(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool boostedAttack,
    int damageBonus
)
{
    while (wave.hasEnemiesLeft())
    {
        int targetIndex = chooseTarget(wave);

        Console::clear();

        if (targetIndex == -1)
        {
            return false;
        }

        if (!wave.isActiveIndexValid(targetIndex))
        {
            MessageScreen::show(
                "CIBLE INDISPONIBLE",
                "combat.target.unavailable",
                {"Cette cible n'est plus disponible."},
                false
            );
            return false;
        }

        int forcedTargetIndex = findForcedTargetIndex(wave);

        if (forcedTargetIndex >= 0 && forcedTargetIndex != targetIndex)
        {
            const Monster& forcedTarget = wave.getActiveEnemy(forcedTargetIndex);

            std::vector<std::string> lines;

            if (forcedTarget.isProvoking())
            {
                lines.push_back(forcedTarget.getName() + " bloque la ligne.");
                lines.push_back("Sa provocation t'empêche d'ignorer sa présence.");
            }
            else
            {
                lines.push_back(forcedTarget.getName() + " vient de soigner un allié.");
                lines.push_back("Ton attention se fixe sur le soigneur.");
            }

            MessageScreen::show(
                "CIBLE FORCÉE",
                "combat.target.forced",
                lines,
                false
            );
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);

        bool stayOnThisTarget = true;

        while (stayOnThisTarget && !target.isDead())
        {
            MenuScreen targetScreen("CIBLE SÉLECTIONNÉE", "combat.target_actions");
            targetScreen.addLine("Cible : " + target.getName());
            targetScreen.addLine("Race : " + target.getRaceText());

            if (target.isInvocation())
            {
                targetScreen.addLine("Statut : Invocation");
            }
            else if (target.isElite())
            {
                targetScreen.addLine("Statut : Élite");
            }
            else
            {
                targetScreen.addLine("Statut : Ennemi standard");
            }

            targetScreen.addOption(0, "Retour au menu principal", "", true, "combat.target.back");

            if (boostedAttack)
            {
                targetScreen.addOption(1, "Utiliser la potion de rage sur cette cible", "", true, "combat.target.use_rage_potion");
            }
            else
            {
                targetScreen.addOption(1, "Attaquer cette cible", "", true, "combat.target.attack");
            }

            targetScreen.addOption(2, "Inspecter cette cible", "Voir les informations de combat connues.", true, "combat.target.inspect");
            targetScreen.addOption(3, "Choisir une autre cible", "", true, "combat.target.change");
            TerminalInterface::renderMenuScreen(targetScreen);

            int choice = Console::askNumberBetween(
                0,
                3,
                "Choix invalide. Entre un chiffre entre 0 et 3."
            );

            Console::clear();

            if (choice == 0)
            {
                return false;
            }

            if (choice == 1)
            {
                if (boostedAttack)
                {
                    CombatAttack::executeBoostedAttack(
                        player,
                        target,
                        random,
                        damageBonus
                    );

                    ThreatSystem::consumeForcedTargetIfNeeded(target);
                    wave.removeDeadAndReplace();

                    return true;
                }

                TurnManager::executeAttack(player, target, random);
                ThreatSystem::consumeForcedTargetIfNeeded(target);

                wave.removeDeadAndReplace();

                return true;
            }

            if (choice == 2)
            {
                ObservationSystem::displayTerminalStats(target);
            }

            if (choice == 3)
            {
                stayOnThisTarget = false;
            }
        }
    }

    return false;
}
