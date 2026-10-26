#include "entity/Player.hpp"
#include "quest/IntercityQuestSystem.hpp"
#include "quest/Quest.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::vector<std::string> splitPipe(const std::string& value)
    {
        std::vector<std::string> out;
        std::stringstream stream(value);
        std::string part;
        while (std::getline(stream, part, '|')) if (!part.empty()) out.push_back(part);
        return out;
    }
}

int main()
{
    Player player;
    player.setLoadedProgress(10, 0, 100);
    player.grantTitle("Aventurier");
    player.setCurrentCityId("valebrume");

    Quest info;
    info.id = "intercity_info_test";
    info.rank = "C";
    info.title = "Relais d'informations entre villes";
    info.objectiveType = "intercity_information";
    info.targetFamily = "Inter-ville / information / relais";
    info.target = 2;
    info.rewardGold = 20;
    info.rewardExperience = 30;
    info.accepted = true;
    info.expiresAtDay = 12;

    std::vector<std::string> prepLines;
    assert(IntercityQuestSystem::initializeAtAcceptance(player, info, &prepLines));
    assert(info.intercityMission);
    assert(info.intercityKind == "information");
    assert(info.intercitySourceCityId == "valebrume");
    assert(!info.intercityRouteCityIds.empty());
    assert(!info.intercityPayloadName.empty());
    assert(info.intercityTotalDistanceKm > 0);
    assert(info.intercityTravelRewardGold > 0);
    assert(info.intercityTravelRewardGold <= 18);
    assert(info.intercityTravelRewardExperience > 0);
    assert(info.rewardGold > 20);
    assert(info.rewardExperience > 30);
    assert(info.target == 2);
    assert(info.progress == 0);
    assert(prepLines.size() >= 3);

    const std::vector<std::string> route = splitPipe(info.intercityRouteCityIds);
    assert(route.size() == 2);
    player.getQuestLog().getQuests().push_back(info);

    std::string from = "valebrume";
    auto firstLines = IntercityQuestSystem::onCityArrival(player, from, route[0]);
    assert(!firstLines.empty());
    Quest& live = player.getQuestLog().getQuests().back();
    assert(live.progress == 1);
    assert(!live.completed);
    assert(live.intercityStopIndex == 1);
    assert(!IntercityQuestSystem::nextDestinationName(live).empty());

    from = route[0];
    auto secondLines = IntercityQuestSystem::onCityArrival(player, from, route[1]);
    assert(!secondLines.empty());
    assert(live.progress == 2);
    assert(live.intercityStopIndex == 2);
    assert(live.completed || live.phaseTwoActive || live.phaseTwoOffered);

    Quest materials;
    materials.id = "intercity_material_test";
    materials.rank = "D";
    materials.title = "Transport de matériaux entre comptoirs";
    materials.objectiveType = "intercity_material";
    materials.target = 1;
    materials.rewardGold = 10;
    materials.rewardExperience = 20;
    materials.accepted = true;
    player.setCurrentCityId("rocheveille");
    assert(IntercityQuestSystem::initializeAtAcceptance(player, materials));
    assert(materials.intercityKind == "materials");
    assert(materials.target == 1);
    assert(materials.intercityTravelRewardGold > 0);
    assert(materials.intercityTravelRewardGold <= 22);

    Player lowLevel;
    lowLevel.setLoadedProgress(1, 0, 0);
    lowLevel.setCurrentCityId("valebrume");
    Quest impossible = materials;
    impossible.intercityMission = false;
    impossible.intercityRouteCityIds.clear();
    impossible.objectiveType = "intercity_information";
    impossible.target = 1;
    assert(!IntercityQuestSystem::initializeAtAcceptance(lowLevel, impossible));

    std::cout << "IntercityQuestSystemTest OK\n";
    return 0;
}
