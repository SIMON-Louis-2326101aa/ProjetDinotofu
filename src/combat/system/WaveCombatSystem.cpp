// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/WaveCombatSystem.hpp"

#include "core/Console.hpp"
#include "entity/MonsterCatalog.hpp"

#include <iostream>

EnemyCombatQueue WaveCombatSystem::createDemoWave()
{
    EnemyCombatQueue wave;

    wave.addWaitingEnemy(MonsterCatalog::creerGobelin());
    wave.addWaitingEnemy(MonsterCatalog::creerGobelinBrutal());
    wave.addWaitingEnemy(MonsterCatalog::creerLoupAffame());
    wave.addWaitingEnemy(MonsterCatalog::creerSquelette());
    wave.addWaitingEnemy(MonsterCatalog::creerOrcMineur());

    wave.initializeFrontLine();

    return wave;
}

void WaveCombatSystem::displayWaveIntroduction()
{
    Console::clear();

    std::cout << "Une vague de monstres approche." << std::endl;
    std::cout << "Ils ne viennent pas tous en même temps..." << std::endl;
    std::cout << "Mais tant que la file n'est pas vide, le combat continue." << std::endl;
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