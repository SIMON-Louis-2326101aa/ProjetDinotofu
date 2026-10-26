#ifndef INCLUDE_QUEST_QUESTLOCATIONACCESSSYSTEM_HPP
#define INCLUDE_QUEST_QUESTLOCATIONACCESSSYSTEM_HPP

#include <string>
#include <vector>

class Player;
struct Quest;

enum class QuestLocationAccessState
{
    Accessible,
    ReducedChance,
    Blocked
};

struct QuestLocationAccessReport
{
    QuestLocationAccessState state = QuestLocationAccessState::Accessible;
    std::string announcedLocation;
    std::string reason;
    std::vector<std::string> lines;
    bool locationKnown = true;
    bool exactLocationRequired = false;

    bool blocksAcceptance() const { return state == QuestLocationAccessState::Blocked; }
    bool needsWarningConfirmation() const { return state == QuestLocationAccessState::ReducedChance; }
};

namespace QuestLocationAccessSystem
{
    QuestLocationAccessReport evaluate(const Player& player, const Quest& quest);
    std::string statusLabel(const QuestLocationAccessReport& report);
    std::string decorateLocation(const std::string& locationText, const QuestLocationAccessReport& report);
}

#endif
