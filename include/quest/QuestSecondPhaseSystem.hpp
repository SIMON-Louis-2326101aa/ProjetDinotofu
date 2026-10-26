#pragma once

#include <string>
#include <vector>

class Player;
struct Quest;

struct QuestSecondPhaseResult
{
    bool attempted = false;
    bool success = false;
    bool completed = false;
    int chancePercent = 0;
    int timeUnits = 0;
    std::vector<std::string> lines;
};

namespace QuestSecondPhaseSystem
{
    // Called when the original objective reaches completion. The decision is deterministic
    // from the quest identity: reopening menus can never reroll a follow-up.
    bool revealAfterPrimaryCompletion(Quest& quest);

    bool canOfferOptional(const Quest& quest);
    bool activateOptional(Quest& quest);
    bool declineOptional(Quest& quest);

    // Dedicated follow-up action. It deliberately uses real world time and can fail.
    QuestSecondPhaseResult resolve(Player& player, Quest& quest, bool cautiousApproach);

    // Applied once during normal quest turn-in. Returns true when a non-monetary
    // phase-2 reward was actually granted/recorded.
    bool applyNonMonetaryReward(Player& player, const Quest& quest, std::vector<std::string>* lines = nullptr);

    bool hasVisibleSecondPhase(const Quest& quest);
    std::vector<std::string> journalLines(const Quest& quest);
}
