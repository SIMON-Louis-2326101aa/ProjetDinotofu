#include "combat/flavor/MonsterFlavorCatalog.hpp"
#include "entity/Monster.hpp"
#include <cassert>
#include <string>

namespace
{
Monster make(const std::string& name, const std::string& type, Race race)
{
    return Monster(name, type, race, 8, 180, 14, 30, 42, 0, 0, false, false, false, false);
}
}

int main()
{
    const Monster goblin = make("Gobelin pillard", "Voleur opportuniste", Race::Gobelin);
    const Monster construct = make("Automate de péage rouillé", "Construction appliquant un ordre disparu", Race::Construction);
    const Monster undead = make("Squelette fissuré", "Mort-vivant fragile", Race::MortVivant);
    const Monster dragon = make("Draconide à écailles grises", "Draconide jeune", Race::Draconide);

    const std::string goblinFlee = MonsterFlavorCatalog::buildFleeLine(goblin);
    const std::string goblinSurrender = MonsterFlavorCatalog::buildSurrenderLine(goblin);
    const std::string constructDeath = MonsterFlavorCatalog::buildDeathLine(construct);
    const std::string undeadDeath = MonsterFlavorCatalog::buildDeathLine(undead);
    const std::string dragonFlee = MonsterFlavorCatalog::buildFleeLine(dragon);

    assert(!goblinFlee.empty());
    assert(!goblinSurrender.empty());
    assert(!constructDeath.empty());
    assert(!undeadDeath.empty());
    assert(!dragonFlee.empty());
    assert(constructDeath != undeadDeath);
    assert(goblinFlee != dragonFlee);
    return 0;
}
