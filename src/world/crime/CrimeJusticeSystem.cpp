#include "world/crime/CrimeJusticeSystem.hpp"

#include "entity/Player.hpp"
#include "world/crime/CrimeNetworkCompetitionSystem.hpp"
#include "world/crime/CrimeLocalImpactSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include <algorithm>

CrimeJusticeAssessment CrimeJusticeSystem::assessVaultTheftContact(
    const Player& player,
    const PlayerVaultTheftCase& theftCase,
    const std::string& contactId)
{
    CrimeJusticeAssessment result;
    const PlayerCrimeContactState* state = player.findCrimeContactState(contactId);
    if (state == nullptr || contactId.empty()) return result;

    result.knownContact = true;
    const bool humanHolder = theftCase.holderType == "receiver" || theftCase.holderType == "accomplice";
    const bool sameContact = theftCase.holderWorldContactId == contactId;
    const bool documented = theftCase.holderReportedToGuard && state->reportedToGuard;

    result.evidenceScore = 0;
    result.evidenceScore += std::clamp(theftCase.clues, 0, 6) * 10;
    result.evidenceScore += std::min(20, theftCase.holderObservationCount * 4);
    result.evidenceScore += std::min(20, state->guardConfrontations * 7);
    result.evidenceScore += std::min(15, state->provisionalDetentions * 8);
    if (theftCase.culpritIdentified) result.evidenceScore += 10;
    if (documented) result.evidenceScore += 15;
    if (theftCase.holderCooperation >= 50) result.evidenceScore += 5;

    // A durable arrest is deliberately difficult. It concerns documented receiving/complicity on
    // the current case; it never rewrites the contact as the original thief without explicit proof.
    result.enoughEvidenceForDurableArrest = humanHolder
        && sameContact
        && documented
        && theftCase.clues >= 6
        && theftCase.holderObservationCount >= 2
        && state->guardConfrontations >= 2
        && state->provisionalDetentions >= 1
        && result.evidenceScore >= 120;

    if (!humanHolder || !sameContact)
    {
        result.legalBasis = "La piste actuelle ne désigne pas cette personne comme détenteur humain du dossier.";
        result.lines.push_back(result.legalBasis);
        return result;
    }

    if (!documented)
        result.lines.push_back("La garde n'a pas encore un dossier officiellement documenté sur ce contact.");
    if (theftCase.clues < 6)
        result.lines.push_back("Les indices restent insuffisants pour une mesure durable : le dossier doit atteindre un faisceau complet de 6 indices.");
    if (theftCase.holderObservationCount < 2)
        result.lines.push_back("Il manque encore des observations indépendantes du contact et de ses habitudes.");
    if (state->guardConfrontations < 2)
        result.lines.push_back("Une seule confrontation ne suffit pas à transformer un soupçon en condamnation durable.");
    if (state->provisionalDetentions < 1)
        result.lines.push_back("Aucune détention provisoire documentée n'a encore permis de recouper les éléments du dossier.");

    if (result.enoughEvidenceForDurableArrest)
    {
        result.legalBasis = theftCase.holderType == "receiver"
            ? "recel documenté et persistant"
            : "complicité documentée dans la circulation des biens volés";
        result.lines.push_back("Le dossier franchit enfin le seuil d'une mesure durable pour " + result.legalBasis + ".");
        result.lines.push_back("Cette conclusion ne désigne toujours pas automatiquement le contact comme auteur initial du cambriolage.");
    }
    else if (result.legalBasis.empty())
    {
        result.legalBasis = "preuves encore insuffisantes pour une arrestation durable";
    }
    return result;
}

CrimeJusticeResolutionResult CrimeJusticeSystem::tryApplyDurableArrest(
    Player& player,
    const PlayerVaultTheftCase& theftCase,
    const std::string& contactId)
{
    CrimeJusticeResolutionResult result;
    PlayerCrimeContactState* state = nullptr;
    for (PlayerCrimeContactState& candidate : player.getCrimeContactStates())
    {
        if (candidate.contactId == contactId)
        {
            state = &candidate;
            break;
        }
    }
    if (state == nullptr) return result;
    if (state->arrested)
    {
        result.lines.push_back(state->displayName + " est déjà sous le coup d'une arrestation durable.");
        return result;
    }

    const CrimeJusticeAssessment assessment = assessVaultTheftContact(player, theftCase, contactId);
    if (!assessment.enoughEvidenceForDurableArrest)
    {
        result.lines.push_back("La garde maintient le dossier au stade provisoire : les preuves ne suffisent pas encore pour une arrestation durable.");
        return result;
    }

    state->arrested = true;
    state->detainedUntilDay = -1;
    state->fledUntilDay = -1;
    state->ruptureUntilDay = -1;
    state->pressure = 100;
    state->cooperation = std::min(state->cooperation, 0);
    state->lastInteractionDay = player.getWorldDaysElapsed();

    player.recordCanonicalEvent(
        "crime_contact_durable_arrests",
        contactId,
        state->displayName + " arrêté durablement pour " + assessment.legalBasis,
        1);
    player.recordHistoricalEvent(
        "crime_contact_conviction",
        contactId,
        state->displayName + " fait l'objet d'une mesure durable pour " + assessment.legalBasis
            + ". Le dossier ne le désigne pas automatiquement comme auteur initial du vol.",
        true);

    const CrimeNetworkDisruptionResult competition = CrimeNetworkCompetitionSystem::applyKnownDisruption(
        player, contactId, 5, "Une arrestation durable affaiblit un réseau criminel connu");
    result.lines.insert(result.lines.end(), competition.lines.begin(), competition.lines.end());

    std::string impactCityId = state->homeCityId;
    if (impactCityId.empty())
    {
        const VaultTheftNetworkContact* contact = VaultTheftNetworkCatalog::findById(contactId);
        if (contact != nullptr) impactCityId = contact->cityId;
    }
    const CrimeLocalImpactRegistration localImpact = CrimeLocalImpactSystem::registerDisruption(
        player, impactCityId, contactId, 5, "Une arrestation durable force les relais criminels connus à se réorganiser");
    result.lines.insert(result.lines.end(), localImpact.lines.begin(), localImpact.lines.end());

    result.changed = true;
    result.durableArrestApplied = true;
    result.lines.push_back("La garde transforme la mesure provisoire en arrestation durable pour " + assessment.legalBasis + ".");
    result.lines.push_back("La qualification reste limitée aux faits réellement établis : aucune culpabilité supplémentaire n'est inventée.");
    return result;
}
