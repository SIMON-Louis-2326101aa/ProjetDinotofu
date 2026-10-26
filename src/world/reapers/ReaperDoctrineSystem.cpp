#include "world/reapers/ReaperDoctrineSystem.hpp"

#include "entity/Player.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>

namespace
{
    const std::vector<ReaperLegendDefinition> kLegends = {
        {"reaper13_origins_nobody", "Il n'était personne",
         "Avant le nom, le grade et la réputation, Reaper-13 aurait surtout été quelqu'un qui survivait, transportait et évitait les combats qu'il ne pouvait pas gagner.",
         ReaperLegendReliability::Plausible, 48},
        {"reaper13_thirteenth_signal", "Le treizième signal",
         "Certains chasseurs jurent que trois avertissements précèdent une condamnation : identifier, avertir, puis seulement engager.",
         ReaperLegendReliability::Folklore, 32},
        {"reaper13_name_superstition", "Ne prononce pas son nom",
         "Des réseaux criminels prétendent que prononcer Reaper-13 pendant une chasse attire son attention.",
         ReaperLegendReliability::Folklore, 14},
        {"reaper13_unpaid_bounty", "La prime qui n'a jamais été payée",
         "Une prime placée sur Reaper-13 aurait attiré plusieurs chasseurs avant de devenir elle-même un avertissement à ne pas poursuivre.",
         ReaperLegendReliability::Folklore, 28},
        {"reapers_protect_innocents", "Les Reapers ne sauvent personne",
         "Une version populaire les décrit comme froids et intéressés par les contrats; d'autres témoignages parlent pourtant d'innocents escortés et de menaces neutralisées sans massacre inutile.",
         ReaperLegendReliability::Contradictory, 44},
        {"reaper13_recruitment_why", "Le recrutement",
         "Une vieille histoire prétend que savoir se battre ne suffit pas : il faudrait surtout savoir pourquoi on décide de frapper.",
         ReaperLegendReliability::Plausible, 52},
        {"reaper13_not_every_bounty", "Le Reaper qui ne prend pas toutes les primes",
         "Selon certains chasseurs indépendants, Reaper-13 refuserait ou déclasserait certaines primes lorsque la cible n'est pas une menace réelle mais seulement quelqu'un ayant commis une erreur.",
         ReaperLegendReliability::Plausible, 58},
        {"reapers_one_two_three_hunt", "La règle des Reapers",
         "Une cible pourrait fuir une fois; à la deuxième fuite, cela deviendrait une chasse; à la troisième, une affaire personnelle. Personne ne sait si cette règle est réellement écrite.",
         ReaperLegendReliability::Folklore, 25},
        {"reaper13_fallen_agent_or_clone", "Origines contradictoires",
         "Certains disent agent d'élite déchu, d'autres survivant d'un projet oublié, clone échappé ou simple chasseur parti de rien. Ces versions ne peuvent pas toutes être prises comme des faits.",
         ReaperLegendReliability::Contradictory, 20},
        {"reapers_doctrine_responsibility", "Code, responsabilité et nécessité",
         "Les Reapers sont décrits comme autonomes, disciplinés, peu intéressés par le prestige politique et attachés à la responsabilité de celui qui décide de traquer une cible.",
         ReaperLegendReliability::Doctrine, 82},
        {"reapers_shadow_bounty_hunters", "Chasseurs de l'ombre",
         "Les Reapers sont avant tout des chasseurs de primes exécuteurs : ils observent, ferment les routes de fuite et terminent les contrats qu'ils ont jugés légitimes, sans confondre vitesse et omniscience.",
         ReaperLegendReliability::Doctrine, 90}
    };

    WorldRumorSourceType sourceTypeFrom(const std::string& source)
    {
        if (source == "tavern") return WorldRumorSourceType::Tavern;
        if (source == "guild") return WorldRumorSourceType::Guild;
        if (source == "guard") return WorldRumorSourceType::Guard;
        if (source == "archive") return WorldRumorSourceType::Archive;
        if (source == "witness") return WorldRumorSourceType::Witness;
        return WorldRumorSourceType::Traveler;
    }
}

const std::vector<ReaperLegendDefinition>& ReaperDoctrineSystem::legendCatalog()
{
    return kLegends;
}

const ReaperLegendDefinition* ReaperDoctrineSystem::findLegend(const std::string& id)
{
    for (const auto& legend : kLegends)
        if (legend.id == id) return &legend;
    return nullptr;
}

void ReaperDoctrineSystem::hearLegend(Player& player, const std::string& legendId, const std::string& sourceKind)
{
    const ReaperLegendDefinition* legend = findLegend(legendId);
    if (legend == nullptr) return;
    WorldRumorSystem::recordRumor(
        player,
        legend->id,
        "reapers13",
        legend->text,
        sourceTypeFrom(sourceKind),
        legend->baseCredibility,
        legend->reliability == ReaperLegendReliability::Doctrine ? 2 : 0);
    player.recordCanonicalEvent("reapers13_legends_heard", legend->id, legend->title, 1);
}

BountyPursuitPlan ReaperDoctrineSystem::planPursuit(
    int targetMobility,
    int knownEscapeAttempts,
    bool targetKnowsItIsHunted,
    bool terrainKnownToReapers,
    bool targetPreparedEscapeRoute)
{
    BountyPursuitHunterProfile hunter;
    hunter.trackingSkill = 92;
    hunter.routeControl = 90;
    hunter.stealthSkill = 88;
    hunter.coordination = 86;
    hunter.shadowHunter = true;
    hunter.capturePreferred = false;

    BountyPursuitTargetProfile target;
    target.mobility = std::clamp(targetMobility, 0, 100);
    target.knownEscapeAttempts = std::max(0, knownEscapeAttempts);
    target.knowsItIsHunted = targetKnowsItIsHunted;
    target.terrainKnownToHunter = terrainKnownToReapers;
    target.hasPreparedEscapeRoute = targetPreparedEscapeRoute;
    return BountyPursuitSystem::plan(hunter, target);
}

ReaperHuntAssessment ReaperDoctrineSystem::assessHunt(
    int evidencePercent,
    int dangerPercent,
    int repeatOffenses,
    bool deliberateHarmToInnocents,
    bool activeThreat,
    bool surrenderingOrCorrectingMistake,
    bool provenIntentionalKilling)
{
    ReaperHuntAssessment result;
    const int evidence = std::clamp(evidencePercent, 0, 100);
    const int danger = std::clamp(dangerPercent, 0, 100);
    const int repeats = std::clamp(repeatOffenses, 0, 20);

    result.threatScore = danger
        + repeats * 7
        + (deliberateHarmToInnocents ? 28 : 0)
        + (activeThreat ? 22 : 0)
        + (provenIntentionalKilling ? 35 : 0)
        - ((!provenIntentionalKilling && surrenderingOrCorrectingMistake) ? 24 : 0);
    result.threatScore = std::clamp(result.threatScore, 0, 100);

    if (evidence < 45)
    {
        result.reason = "Preuves trop faibles : observer, vérifier et recouper avant toute chasse.";
        return result;
    }
    if (!provenIntentionalKilling && !deliberateHarmToInnocents && repeats == 0 && !activeThreat && surrenderingOrCorrectingMistake)
    {
        result.reason = "Erreur isolée ou correction volontaire : aucune nécessité crédible de transformer cela en chasse.";
        return result;
    }

    // Une vie pour une vie n'est appliqué qu'après preuve d'un homicide volontaire injustifié.
    // Un soldat, une légitime défense ou une exécution légitime ne passent pas par ce booléen.
    if (provenIntentionalKilling && evidence >= 70)
    {
        result.accepted = true;
        result.executionExpected = true;
        result.lethalForceJustified = true;
        result.priorityTarget = result.threatScore >= 78 || repeats > 0 || activeThreat;
        result.reason = "Homicide volontaire établi : contrat d'exécution accepté. La reddition peut faciliter la fin de la chasse, pas annuler la dette de sang.";
        return result;
    }

    result.accepted = result.threatScore >= 48 || (activeThreat && evidence >= 65);
    result.priorityTarget = result.accepted && result.threatScore >= 78 && evidence >= 70;
    result.executionExpected = result.accepted && evidence >= 70
        && (deliberateHarmToInnocents || repeats >= 2 || danger >= 85);
    result.lethalForceJustified = result.executionExpected
        || (result.priorityTarget && activeThreat && danger >= 80);

    if (!result.accepted)
        result.reason = "Le contrat ne démontre pas une menace suffisante : prime refusée ou déclassée.";
    else if (result.executionExpected)
        result.reason = "Le dossier est assez grave pour une exécution Reaper : la chasse vise à empêcher la fuite puis terminer le contrat.";
    else
        result.reason = "Dossier accepté mais jugement incomplet : surveillance ou neutralisation temporaire possible avant décision finale.";
    return result;
}

ReaperExecutionRecordAssessment ReaperDoctrineSystem::assessExecutionRecord(
    int verifiedLegitimateBountyExecutions,
    int unjustifiedKills)
{
    ReaperExecutionRecordAssessment result;
    const int legitimate = std::max(0, verifiedLegitimateBountyExecutions);
    const int unjustified = std::max(0, unjustifiedKills);
    result.meritScore = std::clamp(legitimate * 8 - unjustified * 35, -100, 100);
    result.respectedRecord = legitimate > 0 && unjustified == 0 && result.meritScore >= 8;
    if (unjustified > 0)
        result.reason = "Des morts injustifiées détruisent la valeur du dossier, même si certaines primes étaient légitimes.";
    else if (legitimate > 0)
        result.reason = "Des exécutions de primes légitimement condamnées prouvent que le chasseur sait aller jusqu'au bout d'un contrat.";
    else
        result.reason = "Aucun historique vérifié ne permet encore d'évaluer le chasseur.";
    return result;
}
