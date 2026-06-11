// EN: InitiativeRules.hpp defines Dexterity-based initiative rules.
// FR: InitiativeRules.hpp définit les règles d'initiative basées sur la Dextérité.

#ifndef INCLUDE_COMBAT_INITIATIVE_INITIATIVERULES_HPP
#define INCLUDE_COMBAT_INITIATIVE_INITIATIVERULES_HPP

class Player;
class Monster;
class Boss;
class Summon;

class InitiativeRules
{
public:
    static int playerBaseScore(const Player& player);
    static int monsterBaseScore(const Monster& monster);
    static int bossBaseScore(const Boss& boss);
    static int summonBaseScore(const Player& owner, const Summon& summon);
};

#endif
