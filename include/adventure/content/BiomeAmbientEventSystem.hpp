#ifndef INCLUDE_ADVENTURE_CONTENT_BIOMEAMBIENTEVENTSYSTEM_HPP
#define INCLUDE_ADVENTURE_CONTENT_BIOMEAMBIENTEVENTSYSTEM_HPP

#include <string>
#include <vector>

struct BiomeAmbientEvent
{
    bool active = false;
    std::string id;
    std::string title;
    std::vector<std::string> lines;
    int explorationRollShift = 0;
};

// Produces current, observable micro-events from structured biome content.
// The event describes what exists now; it never predicts the next encounter.
class BiomeAmbientEventSystem
{
public:
    static BiomeAmbientEvent buildCurrentEvent(const std::string& biomeName, int worldDay, int dayProgressUnit);
    static std::string journalKey(const std::string& biomeName, int worldDay, const std::string& eventId);
};

#endif
