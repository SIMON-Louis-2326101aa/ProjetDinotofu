#ifndef INCLUDE_WORLD_NPC_NPCKNOWLEDGESYSTEM_HPP
#define INCLUDE_WORLD_NPC_NPCKNOWLEDGESYSTEM_HPP

#include "world/npc/LivingNpcProfile.hpp"
#include "world/npc/NpcKnownFact.hpp"
#include <string>
#include <vector>

class Player;

class NpcKnowledgeSystem
{
public:
    static LivingNpcProfile profileForNamedNpc(const std::string& npcName);
    static std::vector<std::string> conversationMemoryLines(const Player& player, const std::string& npcName, int limit = 3);
    static std::vector<std::string> spontaneousIntroLines(const Player& player, const std::string& npcName);
    static bool hasShareableRecentFact(const Player& player);
    static std::vector<std::string> shareMostRecentFact(Player& player, const std::string& npcName);
    static int effectiveConfidence(const NpcKnownFact& fact, int currentWorldDay);
    static std::string confidenceLabel(int confidence, int evidenceLevel);
    static std::string sourceLabel(const std::string& sourceType);
    static bool hasContradictoryClaims(const Player& player, const std::string& npcName, const std::string& factType, const std::string& subjectId);
};

#endif
