#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"
#include "story/StoryPrologueMemory.hpp"

#include <cassert>
#include <string>
#include <vector>

int main()
{
    Player source("MémoireTest", ClassCatalog::createClassByName("Guerrier"));
    source.getInventory().clearAll();
    source.setLoadedProgress(1, 0, source.getMaxHp());

    const int sourceLevel = source.getLevel();
    const int sourceWeapons = source.getInventory().getWeaponCount();
    const int sourceArmors = source.getInventory().getArmorCount();

    Player memory = StoryPrologueMemory::createTemporaryPlayer(source, DifficultyMode::Normal);
    assert(source.getLevel() == sourceLevel);
    assert(source.getInventory().getWeaponCount() == sourceWeapons);
    assert(source.getInventory().getArmorCount() == sourceArmors);

    assert(memory.getName() == source.getName());
    assert(memory.getType() == source.getType());
    assert(memory.getLevel() == StoryPrologueMemory::MEMORY_LEVEL);
    assert(memory.getMaxHp() >= 900);
    assert(memory.getInventory().getWeaponCount() >= 1);
    assert(memory.getInventory().getArmorCount() >= 1);
    assert(memory.getInventory().getConsumableCount() >= 4);
    assert(memory.hasEquippedWeapon());
    assert(memory.hasEquippedArmor());
    assert(!memory.getUnlockedActiveSkills().empty() || !memory.getUnlockedPassiveSkills().empty());

    auto memoryForClass = [](const std::string& className) {
        Player candidate("ClasseMemoire", ClassCatalog::createClassByName(className));
        candidate.getInventory().clearAll();
        return StoryPrologueMemory::createTemporaryPlayer(candidate, DifficultyMode::Normal);
    };

    Player arbaletrier = memoryForClass("Arbalétrier");
    assert(arbaletrier.getEquippedWeapon().getName().find("Arbalète") != std::string::npos);
    assert(arbaletrier.getEquippedWeapon().getType() == WeaponType::Bow);

    Player archer = memoryForClass("Archer");
    assert(archer.getEquippedWeapon().getType() == WeaponType::Bow);

    Player assassin = memoryForClass("Assassin");
    assert(assassin.getEquippedWeapon().getType() == WeaponType::Dagger);

    Player lancier = memoryForClass("Lancier");
    assert(lancier.getEquippedWeapon().getType() == WeaponType::Spear);

    Player clerc = memoryForClass("Clerc");
    assert(clerc.getEquippedWeapon().getType() == WeaponType::Staff);

    Player invocateur = memoryForClass("Invocateur");
    assert(invocateur.getEquippedWeapon().getType() == WeaponType::Staff);

    Player colosse = memoryForClass("Colosse");
    assert(colosse.getEquippedWeapon().getType() == WeaponType::Hammer);

    const std::vector<Monster> pack = StoryPrologueMemory::createPackHunt();
    assert(pack.size() == 3);
    assert(pack[0].getName().find("Chef de meute") != std::string::npos);
    assert(pack[0].getLevel() >= 40);
    assert(pack[1].getLevel() >= 38);
    assert(pack[2].getLevel() >= 38);

    const std::vector<Monster> easyPack = StoryPrologueMemory::createPackHunt(DifficultyMode::Easy);
    const std::vector<Monster> hardPack = StoryPrologueMemory::createPackHunt(DifficultyMode::Hard);
    const std::vector<Monster> nightmarePack = StoryPrologueMemory::createPackHunt(DifficultyMode::Nightmare);
    const std::vector<Monster> lethalPack = StoryPrologueMemory::createPackHunt(DifficultyMode::Lethal);
    assert(easyPack[0].getMaxHp() < pack[0].getMaxHp());
    assert(easyPack[0].getMaxDamage() < pack[0].getMaxDamage());
    assert(hardPack[0].getMaxHp() > pack[0].getMaxHp());
    assert(hardPack[0].getMaxDamage() > pack[0].getMaxDamage());
    assert(nightmarePack[0].getMaxHp() > hardPack[0].getMaxHp());
    assert(lethalPack[0].getMaxHp() > nightmarePack[0].getMaxHp());
    assert(lethalPack[0].getMaxDamage() > nightmarePack[0].getMaxDamage());

    for (int phase = 0; phase < 8; ++phase)
    {
        const std::string first = StoryPrologueMemory::obscuredCompanionLabel(true, phase);
        const std::string second = StoryPrologueMemory::obscuredCompanionLabel(false, phase);
        assert(first != "Scarlett");
        assert(second != "Lorenzo");
        assert(first.find("Scarlett") == std::string::npos);
        assert(second.find("Lorenzo") == std::string::npos);
    }

    const std::vector<std::string> mission = StoryPrologueMemory::buildMissionFragmentLines(memory);
    assert(!mission.empty());
    bool hasZone = false;
    bool hasMemoryContract = false;
    for (const std::string& line : mission)
    {
        hasZone = hasZone || line.find("Glacier des Serments froids") != std::string::npos;
        hasMemoryContract = hasMemoryContract || line.find("fragment mémoriel") != std::string::npos;
        assert(line.find("Scarlett") == std::string::npos);
        assert(line.find("Lorenzo") == std::string::npos);
    }
    assert(hasZone);
    assert(hasMemoryContract);

    StoryPrologueCombatResult victory;
    victory.outcome = StoryPrologueOutcome::Victory;
    const std::vector<std::string> transition = StoryPrologueMemory::buildFogTransitionLines(source, victory);
    assert(!transition.empty());
    for (const std::string& line : transition)
    {
        assert(line.find("Scarlett") == std::string::npos);
        assert(line.find("Lorenzo") == std::string::npos);
    }

    StoryPrologueCombatResult retreat;
    retreat.outcome = StoryPrologueOutcome::Retreat;
    const std::vector<std::string> retreatTransition = StoryPrologueMemory::buildFogTransitionLines(source, retreat);
    assert(!retreatTransition.empty());
    assert(retreatTransition.front() != transition.front());

    StoryPrologueCombatResult defeat;
    defeat.outcome = StoryPrologueOutcome::Defeat;
    const std::vector<std::string> defeatTransition = StoryPrologueMemory::buildFogTransitionLines(source, defeat);
    assert(!defeatTransition.empty());
    assert(defeatTransition.front() != transition.front());
    assert(defeatTransition.front() != retreatTransition.front());

    return 0;
}
