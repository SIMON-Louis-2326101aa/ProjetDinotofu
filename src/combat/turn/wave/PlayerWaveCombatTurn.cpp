// EN: PlayerWaveCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerWaveCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"

#include "combat/system/EscapeSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"

#include "core/Console.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/CombatTargetMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"
#include "interface/menu/CombatRoleMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"

#include <iostream>

bool PlayerWaveCombatTurn::play(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool& escapeSucceeded,
    DifficultyMode difficulty
)
{
    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(
            wave.getActiveEnemy(i),
            random
        );
    }

    const MenuScreen turnScreen = CombatMenu::buildTurnScreen(player);
    int choice = TerminalInterface::askMenuChoiceFromOptions(
        turnScreen,
        "Choix invalide. Entre un chiffre entre 0 et 8."
    );

    Console::clear();

    if (choice == 0)
    {
        return openWaveInterface(player, wave, difficulty);
    }

    if (choice == 1)
    {
        const bool used = CombatTargetMenu::openForAttack(
            player,
            wave,
            random
        );
        if (used)
        {
            player.recordChallengeCombatAction("basic_attack");
        }
        return used;
    }

    if (choice == 2)
    {
        const bool used = CombatPotionMenu::openQuickHealing(player);
        if (used)
        {
            player.recordChallengeCombatAction("consumable");
        }
        return used;
    }

    if (choice == 3)
    {
        const bool used = CombatPotionMenu::openAgainstWave(
            player,
            wave,
            random,
            PVE_POTION_DAMAGE_BONUS
        );
        if (used)
        {
            player.recordChallengeCombatAction("consumable");
        }
        return used;
    }

    if (choice == 4)
    {
        EquipmentMenu::open(player);
        return false;
    }

    if (choice == 5)
    {
        return InventoryMenu::open(player);
    }

    if (choice == 6)
    {
        DefensePostureSystem::enterDefensePosture(player);
        player.recordChallengeCombatAction("defense");
        return true;
    }

    if (choice == 7)
    {
        MessageScreen::show(
            "TOUR PASSÉ",
            "wave.combat.wait",
            {
                player.getName() + " choisit de ne rien faire ce tour-ci.",
                "Parfois, survivre commence par attendre le bon moment."
            },
            false
        );

        player.recordChallengeCombatAction("wait");
        return true;
    }

    if (choice == 8)
    {
        escapeSucceeded = EscapeSystem::playerAttemptsEscape(
            player,
            random,
            difficulty,
            wave.getTotalRemainingEnemyCount()
        );
        player.recordChallengeCombatAction("escape");
        return true;
    }

    return false;
}

bool PlayerWaveCombatTurn::openWaveInterface(
    Player& player,
    EnemyCombatQueue& wave,
    DifficultyMode difficulty
)
{
    MenuScreen screen("INTERFACE DE VAGUE", "wave.combat.interface");
    screen.addSubtitle(player.getName() + " face à " + std::to_string(wave.getTotalRemainingEnemyCount()) + " adversaire(s) restant(s)");
    screen.addBackOption("Retour", "wave.interface.back");
    screen.addOption(1, "Voir l'état du combat", "Adversaires actifs et résumé de la file.", true, "wave.interface.state");
    screen.addOption(2, "Voir mes statistiques", "Ouvre les statistiques du personnage.", true, "wave.interface.stats");
    screen.addOption(3, "Résumé équipement", "Affichage simple de l'équipement.", true, "wave.interface.equipment");
    screen.addOption(4, "Compétences de rôle", "Actions et rappels liés au rôle.", true, "wave.interface.role");
    screen.addOption(5, "Observer / analyser les adversaires", "Relit la vague active.", true, "wave.interface.observe");
    screen.addOption(6, "Voir un adversaire dans le bestiaire", "Choisir une entrée parmi les ennemis actifs.", true, "wave.interface.bestiary");
    screen.addOption(7, "Ordres aux alliés", "Indisponible sans allié stable.", true, "wave.interface.allies");
    screen.addOption(8, "Contrôle des invocations", "Rappel des ordres actuels.", true, "wave.interface.summons");

    int interfaceChoice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");

    Console::clear();

    if (interfaceChoice == 0)
    {
        return false;
    }

    if (interfaceChoice == 1)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 2)
    {
        StatisticsMenu::open(player, difficulty);
        return false;
    }

    if (interfaceChoice == 3)
    {
        player.displaySimpleEquipment();
        return false;
    }

    if (interfaceChoice == 4)
    {
        return CombatRoleMenu::open(player);
    }

    if (interfaceChoice == 5)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 6)
    {
        if (!wave.hasActiveEnemies())
        {
            MessageScreen::show(
                "BESTIAIRE",
                "wave.interface.bestiary_empty",
                {"Aucun adversaire actif à consulter dans le bestiaire."},
                false
            );
            return false;
        }

        MenuScreen targetScreen("BESTIAIRE DE COMBAT", "wave.interface.bestiary_target");
        targetScreen.addLine("Choisis l'adversaire à rechercher dans le bestiaire.");
        targetScreen.addBackOption("Retour", "wave.interface.bestiary.back");

        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            targetScreen.addOption(
                index + 1,
                wave.getActiveEnemy(index).getName(),
                "Consulter ce que tu sais déjà sur cette créature.",
                true,
                "wave.interface.bestiary.target"
            );
        }

        int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choix invalide.");

        Console::clear();

        if (targetChoice == 0)
        {
            return false;
        }

        BestiaryMenu::displayObjectEntry(
            wave.getActiveEnemy(targetChoice - 1).getName()
        );

        return false;
    }

    if (interfaceChoice == 7)
    {
        MessageScreen::show(
            "ORDRES AUX ALLIÉS",
            "wave.interface.allies_unavailable",
            {"Aucun allié stable n'attend d'ordre sur ce champ de bataille."},
            false
        );
        return false;
    }

    if (interfaceChoice == 8)
    {
        MessageScreen::show(
            "CONTRÔLE DES INVOCATIONS",
            "wave.interface.summons_order",
            {
                "Tes invocations suivent l'ordre donné au début du combat.",
                "Changer cet ordre au milieu du chaos demande une ouverture que tu n'as pas encore."
            },
            false
        );
        return false;
    }

    return false;
}
