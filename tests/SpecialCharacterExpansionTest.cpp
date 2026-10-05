#include "character/SpecialCharacterCatalog.hpp"
#include "character/SpecialCharacterDateRule.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/relationship/SpecialCharacterGroupCatalog.hpp"
#include "combat/action/SpecialCombatEffects.hpp"
#include "core/Random.hpp"
#include "entity/Entity.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace
{
    SpecialCharacter requireCharacter(const std::string& name)
    {
        SpecialCharacter character;
        assert(SpecialCharacterCatalog::findByName(name, character));
        return character;
    }
}

int main()
{
    const SpecialCharacter willow = requireCharacter("Willow");
    assert(willow.getRace() == CharacterRace::Human);
    assert(willow.getNativeClass() == "Archer");
    assert(willow.getAge() == 20);
    assert(willow.getGender() == "Femme");
    assert(willow.canBePlayedWithSpecialDate());
    assert(SpecialCharacterDateRule::validate(willow, "15/12/2025"));
    assert(!SpecialCharacterDateRule::validate(willow, "15/12/25"));
    assert(SpecialCharacterDialogueCatalog::hasDialogueFor("Willow"));

    const SpecialCharacter dwarf = requireCharacter("Dwarf");
    assert(dwarf.getRace() == CharacterRace::Dwarf);
    assert(dwarf.getNativeClass() == "Guerrier");
    assert(dwarf.getAge() == 24);
    assert(dwarf.getGender() == "Homme");
    assert(SpecialCharacterDateRule::validate(dwarf, "15/12/2025"));
    assert(SpecialCharacterDialogueCatalog::hasDialogueFor("Dwarf"));

    const SpecialCharacter badr = requireCharacter("Badr");
    assert(badr.getRace() == CharacterRace::SemiHuman);
    assert(badr.getNativeClass() == "Clerc");
    assert(badr.getAge() == 48);
    assert(badr.getGender() == "Homme");
    assert(SpecialCharacterDateRule::validate(badr, "15/12/2025"));
    assert(badr.getDescription().find("Second") != std::string::npos);
    assert(SpecialCharacterDialogueCatalog::hasDialogueFor("Badr"));

    const std::vector<std::string> groupLines = SpecialCharacterGroupCatalog::getRoadmapLines();
    bool trioDocumented = false;
    for (const std::string& line : groupLines)
    {
        if (line.find("Willow / Dwarf / Badr") != std::string::npos)
        {
            trioDocumented = true;
            break;
        }
    }
    assert(trioDocumented);

    // Their group already has modest combat synergy, while Second's poison skill remains intentionally deferred.
    Entity willowEntity("Willow", "Archer", 100, 8, 12, 18, 0, 0);
    Random random;
    int rawDamage = 100;
    bool critical = false;
    SpecialCombatEffects::registerSpecialGroupContext({"Willow", "Dwarf", "Badr"});
    SpecialCombatEffects::applySpecialCharacterAttackBonus(willowEntity, random, rawDamage, critical);
    assert(rawDamage == 107);
    assert(!critical);

    return 0;
}
