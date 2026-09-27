// EN: PlayerWaveCombatTurn.cpp implements the orchestration of a player's turn during enemy waves.
// FR: PlayerWaveCombatTurn.cpp orchestre le tour du joueur pendant les vagues ennemies.

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/PlayerWaveTacticalActionMenu.hpp"

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
#include "diagnostic/RuntimeLog.hpp"

#include <algorithm>
#include <string>
#include <vector>

bool PlayerWaveCombatTurn::play(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool& escapeSucceeded,
    DifficultyMode difficulty,
    bool teamOrdersAvailable,
    const std::function<bool()>& openTeamOrders
)
{
    if (player.hasEntanglement() && player.hasPassiveSkill("church_oath_roots"))
    {
        const int rootChance = std::clamp(30 + player.getLevel() / 6 + player.getActiveSkillMasteryLevel("retrait_controle") * 3, 30, 58);
        if (random.between(1, 100) <= rootChance && player.cureEntanglement())
        {
            MessageScreen::show(
                "SERMENT DES RACINES",
                "wave.combat.player.entangled_oath_roots",
                {
                    player.getName() + " sent les fils ou racines avant qu'ils ne ferment complètement le tour.",
                    "L'entrave est brisée, mais ce n'est pas gratuit : le serment donne un vrai contre-jeu, pas une immunité."
                },
                false
            );
            player.recordCanonicalEvent("serments_eglise", "racines_contre_entrave", "Serment des Racines a brisé une entrave", 1);
        }
    }

    if (player.consumeEntanglementTurn())
    {
        MessageScreen::show(
            "ENTRAVÉ",
            "wave.combat.player.entangled_skip",
            {
                player.getName() + " perd son tour : des fils, racines ou liens verrouillent le mouvement.",
                "Un ordre clair à un allié peut parfois compenser, mais le corps principal ne peut pas agir cette fois."
            },
            false
        );
        return true;
    }

    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(
            wave.getActiveEnemy(i),
            random
        );
    }

    const MenuScreen turnScreen = CombatMenu::buildTurnScreen(player, teamOrdersAvailable);
    int choice = TerminalInterface::askMenuChoiceFromOptions(
        turnScreen,
        "Choix invalide. Entre un chiffre entre 0 et 10."
    );

    Console::clear();

    if (choice == 0)
    {
        return openWaveInterface(player, wave, difficulty, teamOrdersAvailable, openTeamOrders);
    }

    if (choice == 1)
    {
        MenuScreen attackScreen("ACTION OFFENSIVE", "wave.combat.attack_selector");
        attackScreen.addSubtitle("Tour de " + player.getName());
        attackScreen.addLine("Choisis entre une attaque ciblée classique ou une compétence active avec cooldown.");
        attackScreen.addBackOption("Retour", "wave.combat.attack.back");
        attackScreen.addOption(1, "Attaque ciblée", "Choisir un ennemi actif puis attaquer normalement.", true, "wave.combat.attack.targeted");
        attackScreen.addOption(2, "Compétence active", "Multi-coups, multi-cibles ou cible aléatoire selon la technique choisie.", true, "wave.combat.attack.class_skill");

        int attackChoice = TerminalInterface::askMenuChoiceFromOptions(
            attackScreen,
            "Choix invalide."
        );

        Console::clear();

        if (attackChoice == 0)
        {
            return false;
        }

        bool used = false;
        if (attackChoice == 1)
        {
            used = CombatTargetMenu::openForAttack(
                player,
                wave,
                random
            );
            if (used)
            {
                player.recordChallengeCombatAction("basic_attack");
            }
        }
        else if (attackChoice == 2)
        {
            used = CombatTargetMenu::openForClassSkill(
                player,
                wave,
                random
            );
            if (used)
            {
                player.recordChallengeCombatAction("skill");
            }
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

    if (choice == 9)
    {
        if (teamOrdersAvailable && openTeamOrders)
        {
            openTeamOrders();
            return false;
        }

        MessageScreen::show(
            "CONSIGNES D'ÉQUIPE",
            "wave.combat.team_orders.unavailable",
            {"Aucune recrue ou IA alliée stable n'attend de consigne dans ce combat."},
            false
        );
        return false;
    }

    if (choice == 10)
    {
        const bool used = PlayerWaveTacticalActionMenu::open(player, wave, random);
        if (used)
        {
            player.recordChallengeCombatAction("tactical_action");
        }
        return used;
    }

    return false;
}

bool PlayerWaveCombatTurn::openWaveInterface(
    Player& player,
    EnemyCombatQueue& wave,
    DifficultyMode difficulty,
    bool teamOrdersAvailable,
    const std::function<bool()>& openTeamOrders
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
    screen.addOption(7, "Consignes aux alliés", teamOrdersAvailable ? "Consignes ciblées ou de groupe, sans consommer le tour." : "Indisponible sans allié stable.", teamOrdersAvailable, teamOrdersAvailable ? "wave.interface.team_directives" : "wave.interface.team_directives_unavailable");
    screen.addOption(8, "Contrôle des invocations", "Rappel des ordres actuels.", true, "wave.interface.summons");
    screen.addOption(9, "Journal bêta local", "Affiche le fichier de logs combat/exploration à envoyer au dev.", true, "wave.interface.beta_log");

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
        StatisticsMenu::open(player, difficulty, false);
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
        if (teamOrdersAvailable && openTeamOrders)
        {
            openTeamOrders();
            return false;
        }

        MessageScreen::show(
            "CONSIGNES AUX ALLIÉS",
            "wave.interface.team_directives_unavailable",
            {"Aucun allié stable n'attend de consigne sur ce champ de bataille."},
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

    if (interfaceChoice == 9)
    {
        MessageScreen::show(
            "JOURNAL BÊTA LOCAL",
            "wave.interface.beta_log",
            {
                "Le journal local note les écrans importants, les combats, l'exploration et les menus traversés.",
                "Fichier à envoyer au dev en cas de retour précis : " + RuntimeLog::currentLogPath(),
                "Il sert surtout à comprendre ce qui s'est passé sans demander au testeur de tout raconter à la main."
            },
            false
        );
        return false;
    }

    return false;
}
