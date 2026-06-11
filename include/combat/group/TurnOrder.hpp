// EN: TurnOrder.hpp provides small helpers around initiative identifiers.
// FR: TurnOrder.hpp fournit des aides autour des identifiants d'initiative.

#ifndef INCLUDE_COMBAT_GROUP_TURNORDER_HPP
#define INCLUDE_COMBAT_GROUP_TURNORDER_HPP

#include <string>

class TurnOrder
{
public:
    static bool isPlayer(const std::string& id);
    static bool isSummonGroup(const std::string& id);
    static bool isEnemy(const std::string& id);
    static bool isBoss(const std::string& id);
};

#endif
