#include "entity/Player.hpp"
#include "world/crime/CrimeJusticeSystem.hpp"

#include <cassert>

int main()
{
    Player player;
    player.setCurrentCityId("valebrume");
    PlayerCrimeContactState& contact = player.getOrCreateCrimeContactState(
        "valebrume_receleur_brocanteur", "Serrin le brocanteur");
    contact.reportedToGuard = true;
    contact.guardConfrontations = 1;
    contact.provisionalDetentions = 1;
    contact.cooperation = 60;

    PlayerVaultTheftCase theftCase;
    theftCase.caseId = "vault_case:justice";
    theftCase.cityId = "valebrume";
    theftCase.holderType = "receiver";
    theftCase.holderLabel = "Serrin le brocanteur";
    theftCase.holderWorldContactId = contact.contactId;
    theftCase.holderReportedToGuard = true;
    theftCase.holderObservationCount = 5;
    theftCase.holderCooperation = 60;
    theftCase.clues = 6;

    auto insufficient = CrimeJusticeSystem::assessVaultTheftContact(player, theftCase, contact.contactId);
    assert(insufficient.knownContact);
    assert(!insufficient.enoughEvidenceForDurableArrest);
    assert(!CrimeJusticeSystem::tryApplyDurableArrest(player, theftCase, contact.contactId).durableArrestApplied);
    assert(!player.findCrimeContactState(contact.contactId)->arrested);

    player.getCrimeContactStates().front().guardConfrontations = 2;
    auto sufficient = CrimeJusticeSystem::assessVaultTheftContact(player, theftCase, contact.contactId);
    assert(sufficient.enoughEvidenceForDurableArrest);
    assert(sufficient.evidenceScore >= 120);

    auto applied = CrimeJusticeSystem::tryApplyDurableArrest(player, theftCase, contact.contactId);
    assert(applied.durableArrestApplied);
    const PlayerCrimeContactState* arrested = player.findCrimeContactState(contact.contactId);
    assert(arrested != nullptr);
    assert(arrested->arrested);
    assert(arrested->detainedUntilDay == -1);
    assert(player.getCanonicalJournalCategoryTotal("crime_contact_durable_arrests") == 1);

    // Reapplying the same consequence is idempotent.
    auto duplicate = CrimeJusticeSystem::tryApplyDurableArrest(player, theftCase, contact.contactId);
    assert(!duplicate.durableArrestApplied);
    assert(player.getCanonicalJournalCategoryTotal("crime_contact_durable_arrests") == 1);
    return 0;
}
