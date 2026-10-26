#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "entity/Player.hpp"

int main()
{
    Player player("Enquêteur", ClassCatalog::createClassByName("Guerrier"));
    PlayerCrimeContactState& contact = player.getOrCreateCrimeContactState("test_receiver", "Mira la receleuse");
    contact.knownRole = "receleur";
    contact.pressure = 70;
    contact.cooperation = 5;
    contact.reportedToGuard = true;
    contact.provisionalDetentions = 1;
    contact.detainedUntilDay = player.getWorldDaysElapsed() + 2;

    player.advanceWorldDays(2);
    PlayerCrimeContactState& released = player.getOrCreateCrimeContactState("test_receiver", "Mira la receleuse");
    assert(released.releases == 1);
    assert(released.lastReleaseDay == player.getWorldDaysElapsed());
    assert(released.pressure < 70);
    assert(released.cooperation > 5);
    assert(!released.arrested);

    const int pressureAfterRelease = released.pressure;
    // Refreshing again on the same already-expired detention must not count another release.
    PlayerCrimeContactState& noDuplicateRelease = player.getOrCreateCrimeContactState("test_receiver", "Mira la receleuse");
    assert(noDuplicateRelease.releases == 1);
    assert(noDuplicateRelease.pressure == pressureAfterRelease);

    released.fledUntilDay = player.getWorldDaysElapsed() + 3;
    player.advanceWorldDays(3);
    PlayerCrimeContactState& returned = player.getOrCreateCrimeContactState("test_receiver", "Mira la receleuse");
    assert(returned.returnsAfterFlight == 1);
    assert(returned.lastReturnDay == player.getWorldDaysElapsed());
    assert(returned.knownRole == "receleur");

    return 0;
}
