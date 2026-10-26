#include <cassert>
#include <string>
#include <vector>

#include "class_system/ClassCatalog.hpp"
#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "story/HestiaArcSystem.hpp"
#include "story/HestiaCompanionSystem.hpp"

int main()
{
    Player player("Compagnon", ClassCatalog::createClassByName("Guerrier"));
    assert(HestiaArcSystem::completeMainStoryRescue(player, "active_party", false));

    HestiaCompanionProfile initial = HestiaCompanionSystem::profile(player);
    assert(initial.available);
    assert(initial.activeInParty);
    assert(initial.generalAptitude == -4);
    assert(initial.magicAptitude == -4);

    std::string chosenClass;
    for (const std::string& candidate : HestiaCompanionSystem::classChoices())
    {
        if (candidate.find("Mage") != std::string::npos || candidate.find("mage") != std::string::npos)
        {
            chosenClass = candidate;
            break;
        }
    }
    if (chosenClass.empty()) chosenClass = HestiaCompanionSystem::classChoices().front();
    assert(HestiaCompanionSystem::chooseClass(player, chosenClass));
    assert(HestiaCompanionSystem::chooseWeaponStyle(player, "Bâton / catalyseur"));
    assert(HestiaCompanionSystem::chooseArmorStyle(player, "Tenue légère"));

    for (int i = 0; i < 12; ++i) assert(HestiaCompanionSystem::practice(player));
    for (int i = 0; i < 6; ++i) HestiaCompanionSystem::recordCombatParticipation(player);
    player.getMutableHestiaStoryState().trust = 30;

    const HestiaCompanionProfile trained = HestiaCompanionSystem::profile(player);
    assert(trained.practiceSessions >= 12);
    assert(trained.combatParticipations >= 6);
    assert(trained.magicAptitude > trained.generalAptitude);
    assert(trained.mishapChancePercent < initial.mishapChancePercent);
    assert(HestiaCompanionSystem::looksMagicOriented(trained));

    const auto status = HestiaCompanionSystem::statusLines(player);
    bool leakedRawMagicScore = false;
    for (const std::string& line : status)
        if (line.find("magie :") != std::string::npos) leakedRawMagicScore = true;
    assert(!leakedRawMagicScore);

    const auto active = RecruitedAllyCombatRoster::collectActive(player);
    assert(!active.empty());
    assert(active.size() <= static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithPlayer));
    assert(active.front().persistentMemberId == "special:hestia");
    assert(active.front().temporaryNoReward);

    Random random(1313);
    bool awakened = false;
    for (int attempt = 0; attempt < 500 && !awakened; ++attempt)
    {
        const auto result = HestiaCompanionSystem::tryAwakening(player, random, true);
        awakened = result.triggered;
    }
    assert(awakened);
    assert(player.getHestiaStoryState().awakeningStage == 1);
    assert(HestiaArcSystem::canSacrificeForPlayer(player, true));

    return 0;
}
