// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/WaveCombatSystem.hpp"

#include "combat/wave/WaveGenerator.hpp"

#include "core/Console.hpp"

#include <iostream>

EnemyCombatQueue WaveCombatSystem::createWaveForPlayer(
    const Player& player,
    Random& random
)
{
    return WaveGenerator::createWaveForPlayer(player, random);
}

void WaveCombatSystem::displayWaveIntroduction()
{
    Console::clear();

    std::cout << "Une nouvelle vague de monstres approche." << std::endl;
    std::cout << "La composition change selon ton niveau, mais ils ne viennent pas tous en même temps..." << std::endl;
    std::cout << "Tant que la file n'est pas vide, le combat continue." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);
}

void WaveCombatSystem::displayFrontLineArrival(const EnemyCombatQueue& wave)
{
    std::cout << "La première ligne ennemie entre dans l'arène." << std::endl;
    std::cout << std::endl;

    wave.displayActiveEnemies();
    wave.displayQueueSummary();
}
