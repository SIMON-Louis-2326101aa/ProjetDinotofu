// EN: ElementalAffinitySystem centralizes light elemental status affinity rules.
// FR: ElementalAffinitySystem centralise les règles légères d'affinités élémentaires des statuts.

#ifndef INCLUDE_COMBAT_SYSTEM_ELEMENTALAFFINITYSYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_ELEMENTALAFFINITYSYSTEM_HPP

#include "entity/Entity.hpp"

#include <string>

class ElementalAffinitySystem
{
public:
    static void applyBurning(Entity& target, int turns, int damage);
    static void applyPoison(Entity& target, int turns, int damage);
    static void applyFrost(Entity& target, int turns);
    static void applyShock(Entity& target, int turns);
    static void applyBleeding(Entity& target, int turns, int damage);

    static int getElementalModifierPercent(const Entity& target, const std::string& elementId);
    static std::string getAffinitySummary(const Entity& target, const std::string& elementId);

private:
    static void applyDamageStatus(Entity& target, const std::string& elementId, int turns, int damage);
    static void applyTurnStatus(Entity& target, const std::string& elementId, int turns);
};

#endif
