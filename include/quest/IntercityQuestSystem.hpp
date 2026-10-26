#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;

namespace IntercityQuestSystem
{
    bool isIntercityTemplate(const Quest& quest);
    bool initializeAtAcceptance(const Player& player, Quest& quest, std::vector<std::string>* lines = nullptr);
    bool prepareSecondPhaseTravel(const Player& player, Quest& quest, std::vector<std::string>* lines = nullptr);
    std::vector<std::string> onCityArrival(Player& player, const std::string& fromCityId, const std::string& toCityId);
    std::vector<std::string> journalLines(const Quest& quest);
    std::string nextDestinationName(const Quest& quest);
}
