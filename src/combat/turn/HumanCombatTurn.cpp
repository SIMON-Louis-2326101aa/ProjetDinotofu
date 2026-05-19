// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/HumanCombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/system/EscapeSystem.hpp"
#include "combat/system/ObservationSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"

#include "core/Console.hpp"

#include "entity/Player.hpp"
#include "entity/Boss.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"
#include "interface/menu/CombatRoleMenu.hpp"
#include "interface/menu/CombatGroupTargetMenu.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"

#include <iostream>

bool HumanCombatTurn::play(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    (void)potionHealAmount;

    CombatMenu::displayTurnMenu(attacker);

    int option = Console::askNumberBetween(
        0,
        7,
        "Choix invalide. Entre un chiffre entre 0 et 7."
    );

    Console::clear();

    if (option == 0)
    {
        return openObservationInterface(attacker, defender);
    }

    if (option == 1)
    {
        CombatActions::executeAttack(attacker, defender, random);
        return true;
    }

    if (option == 2)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return CombatPotionMenu::openQuickHealing(*player);
    }

    if (option == 3)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return CombatPotionMenu::openAgainstSingleTarget(
            *player,
            defender,
            random,
            potionDamageBonus
        );
    }

    if (option == 4)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        EquipmentMenu::open(*player);
        return false;
    }

    if (option == 5)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return InventoryMenu::open(*player);
    }

    if (option == 6)
    {
        std::cout << attacker.getName() << " baisse sa garde et passe son tour." << std::endl;
        std::cout << "Parfois, attendre le bon moment est déjà une décision." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (option == 7)
    {
        return handleEscape(attacker, defender, random);
    }

    return false;
}


bool HumanCombatTurn::playWithEnemySummons(
    Entity& attacker,
    Entity& defender,
    std::vector<Summon>& enemySummons,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    (void)potionHealAmount;

    CombatMenu::displayTurnMenu(attacker);

    if (SummonCombatSystem::hasTargetableSummons(enemySummons))
    {
        std::cout << "Note : l'adversaire possède une ou plusieurs invocations ciblables." << std::endl;
        std::cout << "Si tu attaques, tu pourras choisir entre l'invocateur et ses renforts." << std::endl;
        std::cout << std::endl;
    }

    int option = Console::askNumberBetween(
        0,
        7,
        "Choix invalide. Entre un chiffre entre 0 et 7."
    );

    Console::clear();

    if (option == 0)
    {
        return inspectCombatTarget(attacker, defender, enemySummons);
    }

    if (option == 1)
    {
        return chooseAndExecuteAttack(attacker, defender, enemySummons, random);
    }

    if (option == 2)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return CombatPotionMenu::openQuickHealing(*player);
    }

    if (option == 3)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        std::cout << "Pour l'instant, les potions offensives ciblent encore seulement l'adversaire principal." << std::endl;
        std::cout << "Les invocations pourront recevoir des effets spéciaux plus tard." << std::endl;
        std::cout << std::endl;

        return CombatPotionMenu::openAgainstSingleTarget(
            *player,
            defender,
            random,
            potionDamageBonus
        );
    }

    if (option == 4)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        EquipmentMenu::open(*player);
        return false;
    }

    if (option == 5)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return InventoryMenu::open(*player);
    }

    if (option == 6)
    {
        std::cout << attacker.getName() << " baisse sa garde et passe son tour." << std::endl;
        std::cout << "Parfois, attendre le bon moment est déjà une décision." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (option == 7)
    {
        return handleEscape(attacker, defender, random);
    }

    return false;
}

bool HumanCombatTurn::openObservationInterface(
    Entity& interfacePlayer,
    Entity& target
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir l'état du combat" << std::endl;
    std::cout << "2 : Voir mes statistiques" << std::endl;
    std::cout << "3 : Résumé équipement" << std::endl;
    std::cout << "4 : Compétences de rôle" << std::endl;
    std::cout << "5 : Observer / analyser l'adversaire" << std::endl;
    std::cout << "6 : Voir l'adversaire dans le bestiaire" << std::endl;
    std::cout << "7 : Ordres aux alliés" << std::endl;
    std::cout << "8 : Contrôle des invocations" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        8,
        "Choix invalide. Entre un chiffre entre 0 et 8."
    );

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    if (choice == 1)
    {
        std::cout << "========== ÉTAT DU COMBAT ==========" << std::endl;
        std::cout << interfacePlayer.getName() << " : "
                  << interfacePlayer.getHp() << "/"
                  << interfacePlayer.getMaxHp() << " PV" << std::endl;
        std::cout << target.getName() << " : "
                  << target.getHp() << "/"
                  << target.getMaxHp() << " PV" << std::endl;
        std::cout << "====================================" << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (choice == 2)
    {
        interfacePlayer.displayStats();
        return false;
    }

    if (choice == 3)
    {
        Player* player = dynamic_cast<Player*>(&interfacePlayer);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        player->displaySimpleEquipment();
        return false;
    }

    if (choice == 4)
    {
        return CombatRoleMenu::open(interfacePlayer);
    }

    if (choice == 5)
    {
        ObservationSystem::displayTerminalStats(target);
        return false;
    }

    if (choice == 6)
    {
        BestiaryMenu::displayObjectEntry(target.getName());
        return false;
    }

    if (choice == 7)
    {
        std::cout << "Les ordres aux alliés seront disponibles quand les alliés permanents seront branchés." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (choice == 8)
    {
        std::cout << "Le contrôle des invocations se choisit déjà au début du combat si tu possèdes des invocations." << std::endl;
        std::cout << "Plus tard, cette option permettra de changer les ordres pendant le combat." << std::endl;
        std::cout << std::endl;
        return false;
    }

    return false;
}


bool HumanCombatTurn::chooseAndExecuteAttack(
    Entity& attacker,
    Entity& defender,
    std::vector<Summon>& enemySummons,
    Random& random
)
{
    return CombatGroupTargetMenu::openSingleEnemyAttack(
        attacker,
        defender,
        enemySummons,
        random
    );
}

bool HumanCombatTurn::inspectCombatTarget(
    Entity& interfacePlayer,
    Entity& target,
    const std::vector<Summon>& enemySummons
)
{
    std::cout << "========== INTERFACE ==========" << std::endl;
    std::cout << "0 : Retour" << std::endl;
    std::cout << "1 : Voir l'état du combat" << std::endl;
    std::cout << "2 : Voir mes statistiques" << std::endl;
    std::cout << "3 : Résumé équipement" << std::endl;
    std::cout << "4 : Compétences de rôle" << std::endl;
    std::cout << "5 : Observer / analyser l'adversaire principal" << std::endl;
    std::cout << "6 : Voir l'adversaire dans le bestiaire" << std::endl;
    std::cout << "7 : Voir les invocations adverses" << std::endl;
    std::cout << "8 : Ordres aux alliés" << std::endl;
    std::cout << "9 : Contrôle des invocations" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        9,
        "Choix invalide. Entre un chiffre entre 0 et 9."
    );

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    if (choice == 1)
    {
        std::cout << "========== ÉTAT DU COMBAT ==========" << std::endl;
        std::cout << interfacePlayer.getName() << " : "
                  << interfacePlayer.getHp() << "/"
                  << interfacePlayer.getMaxHp() << " PV" << std::endl;
        std::cout << target.getName() << " : "
                  << target.getHp() << "/"
                  << target.getMaxHp() << " PV" << std::endl;

        if (SummonCombatSystem::hasTargetableSummons(enemySummons))
        {
            std::cout << std::endl;
            SummonCombatSystem::displayTargetableSummons(enemySummons);
        }

        std::cout << "====================================" << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (choice == 2)
    {
        interfacePlayer.displayStats();
        return false;
    }

    if (choice == 3)
    {
        Player* player = dynamic_cast<Player*>(&interfacePlayer);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        player->displaySimpleEquipment();
        return false;
    }

    if (choice == 4)
    {
        return CombatRoleMenu::open(interfacePlayer);
    }

    if (choice == 5)
    {
        ObservationSystem::displayTerminalStats(target);
        return false;
    }

    if (choice == 6)
    {
        BestiaryMenu::displayObjectEntry(target.getName());
        return false;
    }

    if (choice == 7)
    {
        if (SummonCombatSystem::hasTargetableSummons(enemySummons))
        {
            SummonCombatSystem::displayTargetableSummons(enemySummons);
        }
        else
        {
            std::cout << "Aucune invocation adverse ciblable pour le moment." << std::endl;
            std::cout << std::endl;
        }

        return false;
    }

    if (choice == 8)
    {
        std::cout << "Les ordres aux alliés seront disponibles quand les alliés permanents seront branchés." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (choice == 9)
    {
        std::cout << "Le contrôle des invocations se choisit déjà au début du combat si tu possèdes des invocations." << std::endl;
        std::cout << "Plus tard, cette option permettra de changer les ordres pendant le combat." << std::endl;
        std::cout << std::endl;
        return false;
    }

    return false;
}

bool HumanCombatTurn::handleEscape(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    Boss* targetBoss = dynamic_cast<Boss*>(&defender);

    if (targetBoss != nullptr)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player != nullptr)
        {
            EscapeSystem::playerAttemptsBossEscape(*player, *targetBoss);
        }
        else
        {
            std::cout << "[la fuite est impossible durant ce combat]" << std::endl;
            std::cout << std::endl;
        }

        return true;
    }

    Player* player = dynamic_cast<Player*>(&attacker);

    if (player == nullptr)
    {
        CombatMenu::displayUnavailableOption();
        return false;
    }

    EscapeSystem::playerAttemptsDuelEscape(*player, defender, random);

    return true;
}