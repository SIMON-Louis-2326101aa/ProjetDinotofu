#include "combat/boss/BossDecryptage.hpp"

#include "core/Console.hpp"

#include <iostream>

void BossDecryptage::verifierDecryptageBoss(Boss& boss)
{
    if (!boss.doitDecrypterStats())
    {
        return;
    }

    std::cout << std::endl;

    if (boss.getIdBoss() == 1)
    {
        std::cout << "Très bien, humain..." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Je vois que je t'ai sous-estimé." << std::endl;
        std::cout << "La lumière qui me protège commence à se fissurer." << std::endl;
        std::cout << "Alors regarde bien... mais ne crois pas que cela suffira à me vaincre." << std::endl;
    }
    else if (boss.getIdBoss() == 2)
    {
        std::cout << "Hahaha..." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Tu as réussi à m'arracher assez de sang pour voir derrière le voile." << std::endl;
        std::cout << "Mais plus tu comprends ce que je suis, plus tu t'approches de la peur." << std::endl;
        std::cout << "Allez, humain. Lis mes statistiques... et désespère." << std::endl;
    }
    else if (boss.getIdBoss() == 3)
    {
        std::cout << "Analyse défensive compromise." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Le protocole de dissimulation perd en stabilité." << std::endl;
        std::cout << "Tu as atteint un seuil que peu d'êtres auraient dû franchir." << std::endl;
        std::cout << "Mes données deviennent visibles... mais mon armure, elle, tient encore." << std::endl;
    }
    else
    {
        std::cout << "Le voile autour de " << boss.getNom() << " se déchire." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Ses statistiques deviennent enfin lisibles." << std::endl;
    }

    std::cout << std::endl;

    Console::pauseSecondes(2);

    std::cout << "Décryptage en cours..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    boss.decrypterStats();

    std::cout << "Décryptage terminé." << std::endl;
    std::cout << std::endl;

    boss.afficherStats();

    std::cout << "Le combat vient de changer de rythme." << std::endl;
    std::cout << std::endl;
}