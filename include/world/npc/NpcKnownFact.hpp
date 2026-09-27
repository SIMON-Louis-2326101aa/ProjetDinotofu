#ifndef INCLUDE_WORLD_NPC_NPCKNOWNFACT_HPP
#define INCLUDE_WORLD_NPC_NPCKNOWNFACT_HPP

#include <string>

// A fact is stored from the NPC point of view. It is not global truth: the
// source, confidence and evidence strength explain why this NPC believes it.
struct NpcKnownFact
{
    std::string npcId;
    std::string factType;
    std::string subjectId;
    std::string label;
    std::string sourceType;
    std::string sourceId;
    std::string locationId;
    // Distinct claims about the same subject may coexist without one overwriting another.
    std::string claimVariant = "default";
    // Social path used for the latest relay: guard desk, market, inn, guild, etc.
    std::string relayChannel;
    int firstLearnedDay = 0;
    int lastReinforcedDay = 0;
    int confidence = 0;      // 0..100, belief rather than omniscient truth.
    int evidenceLevel = 0;   // 0 rumor, 1 testimony, 2 trace/document, 3 direct/proven.
    int timesHeard = 1;
};

#endif
