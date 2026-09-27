#pragma once

class Player;
class EnemyCombatQueue;

namespace PveWaveNarrativeSupport
{
    void recordWaveEncountersInBestiary(const EnemyCombatQueue& wave);
    void recordWaveEncountersInJournal(Player& player, const EnemyCombatQueue& wave);
    void recordWaveKillsInBestiary(const EnemyCombatQueue& wave);
    void recordWaveKillsInJournal(Player& player, const EnemyCombatQueue& wave);
    void displaySpecialDefeatDialogues(const EnemyCombatQueue& wave);
    void displaySpecialVictoryDialogues(const EnemyCombatQueue& wave);
}
