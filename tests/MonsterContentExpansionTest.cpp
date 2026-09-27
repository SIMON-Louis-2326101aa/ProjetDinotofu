#include "entity/MonsterCatalog.hpp"
#include <cassert>
#include <set>
#include <string>

int main()
{
    std::set<std::string> names;
    for (const Monster& monster : MonsterCatalog::createAllPreviewMonsters()) names.insert(monster.getName());
    assert(names.count("Arbalétrier de relais") == 1);
    assert(names.count("Tisseuse de racines anciennes") == 1);
    assert(names.count("Chaman de vase au souffle long") == 1);
    assert(names.count("Copiste de sceau noyé") == 1);
    assert(names.count("Drake gris plongeur") == 1);
    assert(names.count("Sonneur de grand serment") == 1);
    assert(names.count("Huissier des trois versions") == 1);
    assert(names.count("Maître-carillonneur du pacte") == 1);
    return 0;
}
