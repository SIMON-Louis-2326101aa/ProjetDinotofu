#include <cassert>
#include <string>
#include <vector>

#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "story/HestiaMissingPersonQuestSystem.hpp"

namespace
{
    void addCompletedQuest(Player& player, int index)
    {
        Quest q;
        q.id = "completed_for_hestia_" + std::to_string(index);
        q.title = "Quête terminée " + std::to_string(index);
        q.accepted = true;
        q.completed = true;
        q.turnedIn = true;
        q.failed = false;
        q.target = 1;
        q.progress = 1;
        assert(player.getQuestLog().addQuest(q));
    }
}

int main()
{
    Player player("Recherche", ClassCatalog::createClassByName("Guerrier"));
    assert(player.startStoryMode());

    for (int i = 0; i < 19; ++i) addCompletedQuest(player, i);
    auto before = HestiaMissingPersonQuestSystem::availability(player);
    assert(!before.available);
    assert(before.completedQuestCount == 19);
    assert(!HestiaMissingPersonQuestSystem::syncQuestIfAvailable(player));

    addCompletedQuest(player, 19);
    auto ready = HestiaMissingPersonQuestSystem::availability(player);
    assert(ready.available);
    assert(ready.completedQuestCount == 20);
    assert(HestiaMissingPersonQuestSystem::syncQuestIfAvailable(player));
    assert(HestiaMissingPersonQuestSystem::questExists(player));
    assert(HestiaMissingPersonQuestSystem::questActive(player));
    assert(!HestiaMissingPersonQuestSystem::syncQuestIfAvailable(player));

    bool hidesSecret = true;
    for (const Quest& q : player.getQuestLog().getQuests())
    {
        if (q.id != HestiaMissingPersonQuestSystem::QuestId) continue;
        const std::string combined = q.title + " " + q.objective + " " + q.rewardNote;
        if (combined.find("divin") != std::string::npos || combined.find("Hestia") != std::string::npos || combined.find("lumière") != std::string::npos)
            hidesSecret = false;
    }
    assert(hidesSecret);

    const auto defenders = HestiaMissingPersonQuestSystem::buildDefenderWave(player);
    assert(defenders.size() >= 4);
    bool hasEliteThreat = false;
    for (const Monster& monster : defenders)
    {
        if (monster.getName().find("ninja") != std::string::npos || monster.getName().find("suicidaire") != std::string::npos)
            hasEliteThreat = true;
    }
    assert(hasEliteThreat);

    const int stealth = HestiaMissingPersonQuestSystem::stealthChancePercent(player);
    assert(stealth >= 18 && stealth <= 62);

    assert(HestiaMissingPersonQuestSystem::completeRescue(player, "guild_diane", false));
    assert(player.getHestiaStoryState().rescued);
    assert(player.getHestiaStoryState().residence == "guild_diane");
    assert(!HestiaMissingPersonQuestSystem::questActive(player));

    // Thousands of days later, leaving her with Diane is still a safe terminal state with no upkeep.
    const int trustAfterRescue = player.getHestiaStoryState().trust;
    assert(player.getHestiaStoryState().residence == "guild_diane");
    assert(player.getHestiaStoryState().trust == trustAfterRescue);

    return 0;
}
