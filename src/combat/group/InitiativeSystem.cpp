// EN: InitiativeSystem.cpp exposes initiative helpers for group fights.
// FR: InitiativeSystem.cpp expose les aides d'initiative pour les combats de groupe.

#include "combat/group/InitiativeSystem.hpp"

#include "combat/EnemyCombatQueue.hpp"
#include "combat/initiative/InitiativeRules.hpp"
#include "combat/summon/Summon.hpp"
#include "entity/Boss.hpp"
#include "entity/Player.hpp"

#include <algorithm>

InitiativeQueue InitiativeSystem::buildWaveQueue(
    const std::vector<Player*>& party,
    const EnemyCombatQueue& wave,
    const std::vector<std::vector<Summon>>& partySummons,
    Random& random
)
{
    InitiativeQueue queue;
    for (std::size_t index = 0; index < party.size(); ++index)
    {
        Player* player = party[index];
        if (player == nullptr || player->isDead()) continue;
        queue.add({"player:" + std::to_string(index), player->getName(), InitiativeSide::Players, static_cast<int>(index), InitiativeRules::playerBaseScore(*player)});

        if (index < partySummons.size())
        {
            bool hasActiveSummon = false;
            int summonScore = 0;
            for (const Summon& summon : partySummons[index])
            {
                if (summon.isDead() || summon.isExpired()) continue;
                hasActiveSummon = true;
                summonScore = std::max(summonScore, InitiativeRules::summonBaseScore(*player, summon));
            }
            if (hasActiveSummon)
            {
                queue.add({"summons:" + std::to_string(index), "Invocations de " + player->getName(), InitiativeSide::Players, static_cast<int>(index), summonScore});
            }
        }
    }

    for (int enemyIndex = 0; enemyIndex < wave.getActiveEnemyCount(); ++enemyIndex)
    {
        const Monster& monster = wave.getActiveEnemy(enemyIndex);
        if (monster.isDead()) continue;
        queue.add({"enemy:" + std::to_string(enemyIndex), monster.getName(), InitiativeSide::Enemies, enemyIndex, InitiativeRules::monsterBaseScore(monster)});
    }

    queue.rollAndSort(random);
    return queue;
}

InitiativeQueue InitiativeSystem::buildBossQueue(
    const std::vector<Player*>& party,
    const Boss& boss,
    const std::vector<std::vector<Summon>>& partySummons,
    Random& random
)
{
    InitiativeQueue queue;
    for (std::size_t index = 0; index < party.size(); ++index)
    {
        Player* player = party[index];
        if (player == nullptr || player->isDead()) continue;
        queue.add({"player:" + std::to_string(index), player->getName(), InitiativeSide::Players, static_cast<int>(index), InitiativeRules::playerBaseScore(*player)});

        if (index < partySummons.size())
        {
            bool hasActiveSummon = false;
            int summonScore = 0;
            for (const Summon& summon : partySummons[index])
            {
                if (summon.isDead() || summon.isExpired()) continue;
                hasActiveSummon = true;
                summonScore = std::max(summonScore, InitiativeRules::summonBaseScore(*player, summon));
            }
            if (hasActiveSummon)
            {
                queue.add({"summons:" + std::to_string(index), "Invocations de " + player->getName(), InitiativeSide::Players, static_cast<int>(index), summonScore});
            }
        }
    }

    if (!boss.isDead())
    {
        queue.add({"boss", boss.getName(), InitiativeSide::Enemies, 0, InitiativeRules::bossBaseScore(boss)});
    }

    queue.rollAndSort(random);
    return queue;
}

std::vector<std::string> InitiativeSystem::buildDisplayLines(const InitiativeQueue& queue)
{
    std::vector<std::string> lines;
    int position = 1;
    for (const InitiativeRoll& entry : queue.getEntries())
    {
        lines.push_back(
            std::to_string(position++) + ". " + entry.label
            + " | base " + std::to_string(entry.baseScore)
            + " + d20 " + std::to_string(entry.dieRoll)
            + " = " + std::to_string(entry.totalScore)
        );
    }
    if (lines.empty()) lines.push_back("Aucune unité capable d'agir.");
    return lines;
}
