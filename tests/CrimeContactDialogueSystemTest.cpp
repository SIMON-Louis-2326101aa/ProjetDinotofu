#include "entity/Player.hpp"
#include "world/crime/CrimeContactDialogueSystem.hpp"

#include <cassert>
#include <string>

namespace
{
    bool contains(const std::vector<std::string>& lines, const std::string& needle)
    {
        for (const std::string& line : lines)
            if (line.find(needle) != std::string::npos) return true;
        return false;
    }
}

int main()
{
    Player player;
    PlayerCrimeContactState& state = player.getOrCreateCrimeContactState(
        "valebrume_receleur_brocanteur", "Serrin le brocanteur");
    state.reliability = 75;
    state.cooperation = 45;
    state.favorBalance = -40;
    state.pressure = 80;
    state.reportedToGuard = true;

    auto dialogue = CrimeContactDialogueSystem::build(player, state.contactId);
    assert(dialogue.known);
    assert(dialogue.available);
    assert(contains(dialogue.lines, "dette"));
    assert(contains(dialogue.lines, "pression"));
    assert(contains(dialogue.lines, "garde"));

    state.rivalry = 80;
    dialogue = CrimeContactDialogueSystem::build(player, state.contactId);
    assert(contains(dialogue.lines, "hostilité"));

    state.ruptureUntilDay = player.getWorldDaysElapsed() + 5;
    dialogue = CrimeContactDialogueSystem::build(player, state.contactId);
    assert(dialogue.ruptured);
    assert(!dialogue.available);
    assert(contains(dialogue.lines, "refuse encore"));

    state.ruptureUntilDay = -1;
    state.arrested = true;
    dialogue = CrimeContactDialogueSystem::build(player, state.contactId);
    assert(!dialogue.available);
    assert(contains(dialogue.lines, "arrestation durable"));
    return 0;
}
