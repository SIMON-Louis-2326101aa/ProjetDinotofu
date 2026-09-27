#ifndef INCLUDE_WORLD_NPC_NPCINFORMATIONPROPAGATIONSYSTEM_HPP
#define INCLUDE_WORLD_NPC_NPCINFORMATIONPROPAGATIONSYSTEM_HPP

#include <string>
#include <vector>

class Player;

struct NpcPropagationResult
{
    bool transferred = false;
    std::string sourceNpc;
    std::string factType;
    std::string subjectId;
    int confidence = 0;
    std::string relayChannel;
    std::string informationNetwork;
    std::string claimVariant = "default";
    std::vector<std::string> lines;
};

// Local information circulation without omniscience. A recipient can only
// receive a fact that another already-known NPC actually holds in the same
// local area, and the transmission weakens certainty instead of copying truth.
class NpcInformationPropagationSystem
{
public:
    static NpcPropagationResult propagateOneLocalFact(Player& player, const std::string& recipientNpc);
    static NpcPropagationResult propagateOneIntercityFact(Player& player, const std::string& recipientNpc);
    static bool professionWouldRelay(const std::string& profession, const std::string& factType);
    static std::string relayChannelForProfession(const std::string& profession);
    static std::string informationNetworkForProfession(const std::string& profession);
    static int relayDelayDaysForNetwork(const std::string& networkId);
    static int intercityTravelDelayDays(int distanceKm, const std::string& networkId);
    static std::string intercityCarrierForNetwork(const std::string& networkId);
};

#endif
