#ifndef INCLUDE_WORLD_NPC_NPCRELATIONSHIPSYSTEM_HPP
#define INCLUDE_WORLD_NPC_NPCRELATIONSHIPSYSTEM_HPP

#include <string>

struct NpcRelationship
{
    bool known = false;
    std::string type;
    std::string description;
    int relayModifier = 0;
};

class NpcRelationshipSystem
{
public:
    static NpcRelationship between(const std::string& firstNpc, const std::string& secondNpc);
    static int relayModifier(const std::string& firstNpc, const std::string& secondNpc);
};

#endif
