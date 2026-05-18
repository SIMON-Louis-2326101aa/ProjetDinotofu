// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/boss/BossPveMode.hpp"

#include "combat/TurnManager.hpp"

#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "boss/BossCatalog.hpp"
#include "interface/CombatDisplay.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltyResult.hpp"
#include "progression/death/DeathPenaltySystem.hpp"

#include <iostream>

void BossPveMode::run(
    Player& player1,
    Random& random,
    DifficultyMode difficulty
)
{
    std::cout << "Vous sentez une aura maléfique autour de vous." << std::endl;
    Console::pauseSeconds(2);

    std::cout << "Par chance, votre âme est encore pure." << std::endl;
    std::cout << "À mesure que vous avancez, votre force change, comme si l'arène refusait de vous laisser mourir trop vite." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);

    PlayerClass evolvedClass = ClassCatalog::createEvolvedClassFromClass(player1.getType());
    player1.applyClass(evolvedClass);

    std::cout << player1.getName() << ", ta classe évolue en : " << player1.getType() << "." << std::endl;
    std::cout << "Tes PV et tes objets ont été renforcés pour ce combat." << std::endl;
    std::cout << "Tes dégâts, eux, restent bloqués : même l'arène semble avoir ses limites." << std::endl;
    std::cout << std::endl;

    player1.displayStats();

    Console::pauseSeconds(3);

    std::cout << player1.getName() << ", choisis le type d'apparition du boss :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Boss aléatoire" << std::endl;
    std::cout << "2 : Choisir le boss toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int bossChoiceType = Console::askNumberBetween(
        1,
        2,
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    int bossChoice;

    if (bossChoiceType == 1)
    {
        bossChoice = random.between(1, 3);
    }
    else
    {
        Console::clear();

        std::cout << "Sélectionne l'entité que tu veux exterminer :" << std::endl;
        std::cout << std::endl;

        BossCatalog::displayAvailableBosses();

        std::cout << "Leurs statistiques resteront inconnues pour le moment." << std::endl;
        std::cout << "Tu auras besoin d'un bon instinct... ou d'une chance insolente." << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        bossChoice = Console::askNumberBetween(
            1,
            3,
            "Veuillez entrer un chiffre valide : 1, 2 ou 3."
        );
    }

    Boss boss = BossCatalog::createBoss(bossChoice);

    Console::clear();

    std::cout << "Préparation du boss..." << std::endl;
    Console::pauseSeconds(2);

    std::cout << boss.getName() << " est entré dans l'arène." << std::endl;
    std::cout << boss.getName() << " est de type : " << boss.getType() << "." << std::endl;
    std::cout << "Fais attention : ce combat pourrait vraiment te coûter cher." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(3);

    int turn = random.chooseFirstTurn();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSeconds(2);
    std::cout << "Le combat contre " << boss.getName() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!player1.isDead() && !boss.isDead())
    {
        bool turnFinished = false;

        if (turn == 1)
        {
            turnFinished = TurnManager::playHumanTurn(
                player1,
                boss,
                random,
                BOSS_POTION_HEAL_AMOUNT,
                BOSS_POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                TurnManager::checkBossDecryption(boss);
                boss.reduceUltimateCooldown();
                turn = 2;
            }
        }
        else
        {
            turnFinished = TurnManager::playBossTurn(
                boss,
                player1,
                random
            );

            if (turnFinished)
            {
                turn = 1;
            }
        }
    }

    CombatDisplay::displayCombatResult(player1, boss);

    if (player1.isDead())
    {
        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player1,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);

        player1.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        std::cout << player1.getName()
                  << " revient à lui avec "
                  << player1.getHp()
                  << "/"
                  << player1.getMaxHp()
                  << " PV."
                  << std::endl;
        std::cout << "Même vaincu, tu n'es pas encore sorti du registre des vivants." << std::endl;
        std::cout << std::endl;
    }
}