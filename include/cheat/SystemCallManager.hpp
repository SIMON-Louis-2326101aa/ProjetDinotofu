// EN: Hidden system-call console. It intentionally reveals no command list or autocomplete.
// FR: Console system-call cachée. Elle ne révèle volontairement aucune liste ni autocomplétion.
#ifndef INCLUDE_CHEAT_SYSTEMCALLMANAGER_HPP
#define INCLUDE_CHEAT_SYSTEMCALLMANAGER_HPP

class Player;
enum class DifficultyMode;
enum class DeathRuleMode;

class SystemCallManager
{
public:
    static void open(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule);
};

#endif
