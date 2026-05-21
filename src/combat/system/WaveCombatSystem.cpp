// EN: WaveCombatSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: WaveCombatSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    return WaveGenerator::createWaveForPlayer(player, random, DifficultyMode::Normal);
}

EnemyCombatQueue WaveCombatSystem::createWaveForPlayer(
    const Player& player,
    Random& random,
    DifficultyMode difficulty
)
{
    return WaveGenerator::createWaveForPlayer(player, random, difficulty);
}

// EN: displayWaveIntroduction declares or implements a focused behavior used by this module.
// FR: displayWaveIntroduction déclare ou implémente un comportement précis utilisé par ce module.
void WaveCombatSystem::displayWaveIntroduction()
{
    Console::clear();

    std::cout << "Une nouvelle vague de monstres approche." << std::endl;
    std::cout << "La composition change selon ton niveau, mais ils ne viennent pas tous en même temps..." << std::endl;
    std::cout << "Tant que la file n'est pas vide, le combat continue." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);
}

// EN: displayFrontLineArrival declares or implements a focused behavior used by this module.
// FR: displayFrontLineArrival déclare ou implémente un comportement précis utilisé par ce module.
void WaveCombatSystem::displayFrontLineArrival(const EnemyCombatQueue& wave)
{
    std::cout << "La première ligne ennemie entre dans l'arène." << std::endl;
    std::cout << std::endl;

    wave.displayActiveEnemies();
    wave.displayQueueSummary();
}
