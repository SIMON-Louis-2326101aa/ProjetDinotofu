// EN: TurnOrder.cpp provides small helpers around initiative identifiers.
// FR: TurnOrder.cpp fournit des aides autour des identifiants d'initiative.

#include "combat/group/TurnOrder.hpp"

bool TurnOrder::isPlayer(const std::string& id) { return id.rfind("player:", 0) == 0; }
bool TurnOrder::isSummonGroup(const std::string& id) { return id.rfind("summons:", 0) == 0; }
bool TurnOrder::isEnemy(const std::string& id) { return id.rfind("enemy:", 0) == 0; }
bool TurnOrder::isBoss(const std::string& id) { return id == "boss"; }
