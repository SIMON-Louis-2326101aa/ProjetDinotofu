// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/wave/WaveGenerator.hpp"

#include "combat/wave/WaveRules.hpp"
#include "entity/MonsterCatalog.hpp"

EnemyCombatQueue WaveGenerator::createWaveForPlayer(
    const Player& player,
    Random& random
)
{
    EnemyCombatQueue wave;

    int waveSize = WaveRules::getWaveSizeForPlayerLevel(player.getLevel());

    for (int i = 0; i < waveSize; ++i)
    {
        int levelVariation = random.between(-1, 1);

        int monsterLevel = WaveRules::getMonsterLevelForPlayerLevel(
            player.getLevel(),
            levelVariation
        );

        wave.addWaitingEnemy(
            MonsterCatalog::createRandomMonsterForLevel(monsterLevel, random)
        );
    }

    wave.initializeFrontLine();

    return wave;
}
