#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "quest/QuestLocationAccessSystem.hpp"

#include <cassert>
#include <iostream>

int main()
{
    Player beginner;
    beginner.setLoadedProgress(1, 0, 0);
    beginner.setCurrentCityId("valebrume");

    Quest exactRemote;
    exactRemote.id = "exact_remote";
    exactRemote.rank = "D";
    exactRemote.location = "Rocheveille";
    exactRemote.objectiveType = "service";
    exactRemote.objective = "Faire signer le registre à Rocheveille.";
    auto blocked = QuestLocationAccessSystem::evaluate(beginner, exactRemote);
    assert(blocked.state == QuestLocationAccessState::Blocked);
    assert(blocked.blocksAcceptance());
    assert(QuestLocationAccessSystem::statusLabel(blocked) == "Actuellement inaccessible");

    Quest flexibleHunt;
    flexibleHunt.id = "flexible_hunt";
    flexibleHunt.rank = "D";
    flexibleHunt.location = "Falaises des drakes gris";
    flexibleHunt.objectiveType = "combat";
    flexibleHunt.objective = "Éliminer quelques drakes gris devenus agressifs.";
    flexibleHunt.targetFamily = "Drakes";
    auto reduced = QuestLocationAccessSystem::evaluate(beginner, flexibleHunt);
    assert(reduced.state == QuestLocationAccessState::ReducedChance);
    assert(reduced.needsWarningConfirmation());
    assert(!reduced.blocksAcceptance());

    Quest exactHunt = flexibleHunt;
    exactHunt.objective = "Éliminer les drakes précisément dans les Falaises des drakes gris.";
    auto exactBlocked = QuestLocationAccessSystem::evaluate(beginner, exactHunt);
    assert(exactBlocked.state == QuestLocationAccessState::Blocked);

    Quest local;
    local.id = "local";
    local.rank = "F";
    local.location = "Route commerciale";
    local.objectiveType = "exploration";
    local.objective = "Retrouver une caisse perdue sur la route.";
    auto accessible = QuestLocationAccessSystem::evaluate(beginner, local);
    assert(accessible.state == QuestLocationAccessState::Accessible);

    std::cout << "QuestLocationAccessSystemTest OK\n";
    return 0;
}
