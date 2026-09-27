#ifndef INCLUDE_COMBAT_FLAVOR_MONSTERFLAVORCATALOG_HPP
#define INCLUDE_COMBAT_FLAVOR_MONSTERFLAVORCATALOG_HPP

#include <string>
class Monster;

class MonsterFlavorCatalog
{
public:
    static std::string buildAppearanceLine(const Monster& monster);
    static std::string buildIdleLine(const Monster& monster);
    static std::string buildAttackMotionLine(const Monster& monster);
    static std::string buildImpactTextureLine(const Monster& monster, bool critical, bool boosted);
    static std::string buildDeathLine(const Monster& monster);
    static std::string buildFleeLine(const Monster& monster);
    static std::string buildSurrenderLine(const Monster& monster);
    static std::string buildBestiaryFlavor(const Monster& monster);
};

#endif
