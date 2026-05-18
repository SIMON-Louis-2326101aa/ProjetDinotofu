// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pve/MonsterPveMode.hpp"

#include "combat/EnemyCombatQueue.hpp"
#include "combat/system/WaveCombatSystem.hpp"

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include <iostream>

void MonsterPveMode::run(Player& player, Random& random)
{
    WaveCombatSystem::displayWaveIntroduction();

    EnemyCombatQueue wave = WaveCombatSystem::createDemoWave();

    WaveCombatSystem::displayFrontLineArrival(wave);

    bool escapeSucceeded = false;

    while (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        bool playerTurnFinished = false;

        while (!playerTurnFinished
            && !player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            playerTurnFinished = PlayerWaveCombatTurn::jouer(
                player,
                wave,
                random,
                escapeSucceeded
            );

            if (!playerTurnFinished && !escapeSucceeded)
            {
                std::cout << "Ton tour n'est pas encore consommé." << std::endl;
                std::cout << std::endl;
            }
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            MonsterWaveCombatTurn::jouerToursMonstres(
                player,
                wave,
                random
            );
        }
    }

    if (escapeSucceeded)
    {
        std::cout << "Tu as quitté le combat." << std::endl;
        std::cout << "Tu ne récupéreras qu'une partie des récompenses liées à ce qui s'est réellement passé." << std::endl;
        std::cout << std::endl;
        return;
    }

    if (player.isDead())
    {
        std::cout << player.getName() << " tombe face à la vague ennemie." << std::endl;
        std::cout << "L'arène se referme dans un silence brutal." << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "Tous les monstres de la vague ont été vaincus." << std::endl;
    std::cout << player.getName() << " reste debout au milieu des corps et de la poussière." << std::endl;
    std::cout << std::endl;
}