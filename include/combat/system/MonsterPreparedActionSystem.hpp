#ifndef INCLUDE_COMBAT_SYSTEM_MONSTER_PREPARED_ACTION_SYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_MONSTER_PREPARED_ACTION_SYSTEM_HPP

#include <string>

class Monster;
class Player;

struct MonsterPreparedActionResolution
{
    bool hadPreparation = false;
    bool interrupted = false;
    bool resolved = false;
    bool forcedReposition = false;
    int damage = 0;
    int tier = 0;
    std::string label;
    std::string interruptReason;
    std::string family;
    std::string effectLine;
    int damageInterruptPercent = 10;
};

class MonsterPreparedActionSystem
{
public:
    static MonsterPreparedActionResolution resolve(Monster& monster, Player& player);
    static std::string familyForLabel(const std::string& label);
    static std::string telegraphLineForLabel(const std::string& label);
    static std::string interruptHintForLabel(const std::string& label);
};

#endif
