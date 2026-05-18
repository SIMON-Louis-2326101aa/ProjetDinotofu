// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossDecryption.hpp"

#include "core/Console.hpp"

#include <iostream>

void BossDecryption::checkBossDecryption(Boss& boss)
{
    if (!boss.mustDecryptStats())
    {
        return;
    }

    std::cout << std::endl;

    if (boss.getBossId() == 1)
    {
        std::cout << "Très bien, humain..." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Je vois que je t'ai sous-estimé." << std::endl;
        std::cout << "La lumière qui me protège commence à se fissurer." << std::endl;
        std::cout << "Alors regarde bien... mais ne crois pas que cela suffira à me vaincre." << std::endl;
    }
    else if (boss.getBossId() == 2)
    {
        std::cout << "Hahaha..." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Tu as réussi à m'arracher assez de sang pour voir derrière le voile." << std::endl;
        std::cout << "Mais plus tu comprends ce que je suis, plus tu t'approches de la peur." << std::endl;
        std::cout << "Allez, humain. Lis mes statistiques... et désespère." << std::endl;
    }
    else if (boss.getBossId() == 3)
    {
        std::cout << "Analyse défensive compromise." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Le protocole de dissimulation perd en stabilité." << std::endl;
        std::cout << "Tu as atteint un seuil que peu d'êtres auraient dû franchir." << std::endl;
        std::cout << "Mes données deviennent visibles... mais mon armure, elle, tient encore." << std::endl;
    }
    else
    {
        std::cout << "Le voile autour de " << boss.getName() << " se déchire." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Ses statistiques deviennent enfin lisibles." << std::endl;
    }

    std::cout << std::endl;

    Console::pauseSeconds(2);

    std::cout << "Décryptage en cours..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);

    boss.decryptStats();

    std::cout << "Décryptage terminé." << std::endl;
    std::cout << std::endl;

    boss.displayStats();

    std::cout << "Le combat vient de changer de rythme." << std::endl;
    std::cout << std::endl;
}