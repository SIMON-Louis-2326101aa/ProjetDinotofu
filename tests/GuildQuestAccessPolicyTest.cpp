#include "quest/GuildQuestAccessPolicy.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"

#include <cassert>
#include <vector>

namespace
{
    void addActiveRankFRecruit(Player& player, const std::string& name)
    {
        player.recordCanonicalEvent(
            "pnj_recrutables_retenus",
            name,
            name + " | race=Humain | profil=Éclaireur | trait=fiable | niveau_recrutement=1 | rang_origine_index=0 | part_base=12 | salaire=30 | profil_seulement=1",
            1
        );
        player.recordCanonicalEvent("recrues_engagees", name, name + " rejoint réellement le groupe", 1);
        player.recordCanonicalEvent("recrues_equipees_toggle", name, name + " équipé", 1);
    }
}

int main()
{
    Player player;
    Quest quest;
    quest.guildQuest = true;

    // Solo: never above the player's own guild rank.
    quest.rank = "F";
    assert(GuildQuestAccessPolicy::canAccept(player, quest, "F", "verte"));
    quest.rank = "E";
    assert(!GuildQuestAccessPolicy::canAccept(player, quest, "F", "verte"));

    // Real group: exactly +1 maximum.
    addActiveRankFRecruit(player, "Aline");
    assert(GuildQuestAccessPolicy::hasActiveGroup(player));
    assert(GuildQuestAccessPolicy::canAccept(player, quest, "F", "verte"));
    quest.rank = "D";
    assert(!GuildQuestAccessPolicy::canAccept(player, quest, "F", "verte"));

    // Reliability sanctions remain stricter than group privileges. Use a clean player
    // so a low-rank active recruit does not intentionally lower this separate test.
    Player reliabilityPlayer;
    quest.rank = "A";
    assert(!GuildQuestAccessPolicy::canAccept(reliabilityPlayer, quest, "A", "orange"));
    quest.rank = "B";
    assert(GuildQuestAccessPolicy::canAccept(reliabilityPlayer, quest, "A", "orange"));

    // Parties of 2-4: the weakest member remains limited to +1 rank.
    assert(GuildQuestAccessPolicy::groupMaximumQuestRankIndex(0, {0}) == 1);
    assert(GuildQuestAccessPolicy::groupMembersCanTakeRank(0, {0}, 1)); // F + F -> E allowed.
    assert(!GuildQuestAccessPolicy::groupMembersCanTakeRank(0, {0}, 2)); // F + F -> D forbidden.
    assert(!GuildQuestAccessPolicy::groupMembersCanTakeRank(4, {0}, 2)); // A leader cannot drag an F recruit into D in a small party.
    assert(GuildQuestAccessPolicy::groupMembersCanTakeRank(2, {2, 1}, 2)); // D/D/E -> D allowed.
    assert(GuildQuestAccessPolicy::groupMembersCanTakeRank(2, {2, 2}, 3)); // D/D/D -> C allowed (+1).

    // From five members onward, the maximum is based on the rounded group average.
    // Five D members: average D, close to the strongest D -> C allowed.
    assert(GuildQuestAccessPolicy::groupMaximumQuestRankIndex(2, {2, 2, 2, 2}) == 3);
    assert(GuildQuestAccessPolicy::groupMembersCanTakeRank(2, {2, 2, 2, 2}, 3));

    // Four A + one F => average B, one rank below the strongest A -> A allowed, but not S.
    assert(GuildQuestAccessPolicy::groupMaximumQuestRankIndex(5, {5, 5, 5, 0}) == 5);
    assert(GuildQuestAccessPolicy::groupMembersCanTakeRank(5, {5, 5, 5, 0}, 5));
    assert(!GuildQuestAccessPolicy::groupMembersCanTakeRank(5, {5, 5, 5, 0}, 6));

    // A top-heavy pair cannot pull three weak members too high: average is too far from the maximum.
    assert(GuildQuestAccessPolicy::groupMaximumQuestRankIndex(5, {5, 0, 0, 0}) == 2);
    assert(!GuildQuestAccessPolicy::groupMembersCanTakeRank(5, {5, 0, 0, 0}, 3));

    return 0;
}
