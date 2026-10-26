#pragma once

#include <string>
#include <vector>

class Player;
class Random;

struct ReaperWorldEncounterResult
{
    bool occurred = false;
    bool respondedToProtocol13 = false;
    bool firstCredibleKnowledge = false;
    bool recruitmentOffered = false;
    std::string encounterId;
    std::vector<std::string> lines;
};

namespace ReaperWorldSystem
{
    // REAPERS-13 is a major, optional faction that exists independently from the secret protocol.
    // Natural encounters stay rare and must never become a main-story dependency or free recruitment.
    // A valid Protocol 13 signal may later receive one contextual field response, also without automatic membership.
    bool hasCredibleKnowledge(const Player& player);
    bool hasPendingProtocol13Signal(const Player& player);
    bool hasReceivedProtocol13Response(const Player& player);
    int observationScore(const Player& player);

    ReaperWorldEncounterResult tryExplorationEncounter(
        Player& player,
        Random& random,
        const std::string& biomeName
    );
}
