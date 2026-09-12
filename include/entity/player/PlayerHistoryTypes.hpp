#ifndef INCLUDE_ENTITY_PLAYER_PLAYER_HISTORY_TYPES_HPP
#define INCLUDE_ENTITY_PLAYER_PLAYER_HISTORY_TYPES_HPP

#include <string>

struct PlayerHistoricalEvent
{
    std::string id;
    std::string category;
    std::string subjectId;
    std::string label;
    std::string locationId;
    int day = 0;
    bool resolved = false;
};

struct PlayerRivalRecord
{
    std::string rivalId;
    std::string enemyName;
    std::string enemyFamily;
    std::string originLocationId;
    std::string lastKnownLocationId;
    std::string rivalryReason;
    int baseLevel = 1;
    int currentLevel = 1;
    int baseMaxHp = 1;
    int baseAttack = 1;
    int encounters = 1;
    int escapes = 0;
    int returns = 0;
    int wounds = 0;
    int firstSeenDay = 0;
    int lastSeenDay = 0;
    bool alive = true;
};

#endif
