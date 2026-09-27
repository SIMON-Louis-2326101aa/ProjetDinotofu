#include "world/npc/NpcRelationshipSystem.hpp"
#include <cassert>
int main()
{
    const NpcRelationship trusted = NpcRelationshipSystem::between("Mira", "Orren");
    assert(trusted.known);
    assert(trusted.relayModifier > 0);
    assert(NpcRelationshipSystem::relayModifier("Orren", "Mira") == trusted.relayModifier);
    const NpcRelationship cautious = NpcRelationshipSystem::between("Prunigil", "Meron");
    assert(cautious.known);
    assert(cautious.relayModifier < 0);
    assert(!NpcRelationshipSystem::between("Inconnu A", "Inconnu B").known);
    return 0;
}
