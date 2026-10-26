#include "world/bounty/BountyQuestSystem.hpp"
#include "world/bounty/BountyTargetBehaviorSystem.hpp"
#include "world/bounty/BountyIdentityCatalog.hpp"
#include "world/bounty/BountyRelocationSystem.hpp"

#include <algorithm>
#include <array>
#include <functional>

namespace
{
    int clamp100(int v) { return std::clamp(v, 0, 100); }

    int deterministicBucket(const std::string& seed, int minValue, int maxValue)
    {
        if (maxValue <= minValue) return minValue;
        const std::size_t h = std::hash<std::string>{}(seed);
        return minValue + static_cast<int>(h % static_cast<std::size_t>(maxValue - minValue + 1));
    }

    void updateAwareness(Quest& quest, int hunterStealth, BountyQuestStepResult& result)
    {
        BountyHuntAwarenessContext ctx;
        ctx.targetAwareness = quest.bountyTargetAwareness;
        ctx.hunterStealth = hunterStealth;
        ctx.investigationNoise = quest.bountyInvestigationNoise;
        ctx.informantExposure = quest.bountyInformantExposure;
        ctx.previousFailedPursuits = quest.bountyTargetEscapeAttempts;
        ctx.directWarning = quest.bountyTargetKnowsHunt;
        auto awareness = BountyHuntAwarenessSystem::evaluate(ctx);
        result.awarenessState = awareness.state;

        if (awareness.state == BountyTargetAwarenessState::ConfirmedHunt)
        {
            quest.bountyTargetKnowsHunt = true;
            quest.bountyTargetSuspectsHunt = true;
        }
        else if (awareness.state == BountyTargetAwarenessState::Suspicious)
        {
            quest.bountyTargetSuspectsHunt = true;
        }

        result.lines.push_back(awareness.summary);
    }

    BountyTargetBehaviorContext buildTargetContext(const Quest& quest)
    {
        BountyTargetBehaviorContext ctx;
        ctx.temperament = quest.bountyTargetTemperament;
        ctx.resources = quest.bountyTargetResources;
        ctx.awareness = quest.bountyTargetAwareness;
        ctx.preparationWindow = std::clamp(1 + quest.bountyTargetEscapeAttempts, 1, 4);
        ctx.localSupport = quest.bountyTargetLocalSupport;
        ctx.mobility = quest.bountyTargetMobility;
        ctx.previousEscapeAttempts = quest.bountyTargetEscapeAttempts;
        ctx.knowsTerrain = quest.bountyTargetKnowsTerrain;
        ctx.injured = quest.bountyTargetInjured;
        ctx.knowsItIsHunted = quest.bountyTargetKnowsHunt;
        ctx.suspectsSurveillance = quest.bountyTargetSuspectsHunt;
        return ctx;
    }
}

bool BountyQuestSystem::isActiveBounty(const Quest& quest)
{
    return quest.bountyMission && quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed;
}

void BountyQuestSystem::initializeBounty(Quest& quest, int playerLevel)
{
    // A bounty target is generated exactly once. Investigation/pursuit steps may call
    // this helper defensively, but must never reroll an existing target's traits.
    const bool alreadyInitialized = !quest.bountyTargetName.empty();
    quest.bountyMission = true;
    if (quest.bountyStage.empty()) quest.bountyStage = "investigation";
    if (quest.target <= 0) quest.target = 1;

    const int safeLevel = std::max(1, playerLevel);
    const std::string seed = quest.id + "|" + quest.title + "|" + std::to_string(safeLevel);

    const BountyIdentityDefinition* identity = nullptr;
    if (!quest.bountyTargetId.empty()) identity = BountyIdentityCatalog::findById(quest.bountyTargetId);
    if (identity == nullptr && alreadyInitialized) identity = BountyIdentityCatalog::findByName(quest.bountyTargetName);
    if (identity == nullptr && !alreadyInitialized)
        identity = BountyIdentityCatalog::pickFor(seed + "identity", safeLevel, quest.bountyCurrentCityId);

    if (identity != nullptr)
    {
        quest.bountyTargetId = identity->id;
        if (quest.bountyTargetName.empty()) quest.bountyTargetName = identity->name;
        if (quest.bountyTargetTemperament.empty()) quest.bountyTargetTemperament = identity->temperament;
        if (quest.bountyTargetHomeCityId.empty()) quest.bountyTargetHomeCityId = identity->homeCityId;
        if (quest.bountyTargetNetworkId.empty()) quest.bountyTargetNetworkId = identity->networkId;
        if (quest.bountyIssuerId.empty()) quest.bountyIssuerId = identity->issuerId;
        if (quest.bountyIssuerName.empty()) quest.bountyIssuerName = identity->issuerName;
        if (quest.bountyIssuerOrganizationId.empty()) quest.bountyIssuerOrganizationId = identity->issuerOrganizationId;
        if (quest.bountyIssuerOrganizationName.empty()) quest.bountyIssuerOrganizationName = identity->issuerOrganizationName;
        quest.bountyIssuerPrefersLiveCapture = identity->issuerPrefersLiveCapture;
        if (quest.bountyCloseRelationId.empty()) quest.bountyCloseRelationId = identity->closeRelationId;
        if (quest.bountyCloseRelationName.empty()) quest.bountyCloseRelationName = identity->closeRelationName;
        if (quest.bountyCloseRelationType.empty()) quest.bountyCloseRelationType = identity->closeRelationType;
    }

    if (!alreadyInitialized)
    {
        if (identity == nullptr)
        {
            static const std::array<const char*, 4> fallbackNames = {
                "Orven Trois-Ponts", "Bram le Tardif", "Vela la Passeuse", "Ilyne des Fossés"
            };
            static const std::array<const char*, 6> moods = {
                "prudent", "rusé opportuniste", "méfiant méthodique", "survivant discret", "impulsif", "calculateur"
            };
            quest.bountyTargetName = fallbackNames[deterministicBucket(seed + "name", 0, static_cast<int>(fallbackNames.size()) - 1)];
            quest.bountyTargetTemperament = moods[deterministicBucket(seed + "mood", 0, static_cast<int>(moods.size()) - 1)];
        }

        const int awarenessBonus = identity == nullptr ? 0 : identity->awarenessBonus;
        const int resourcesBonus = identity == nullptr ? 0 : identity->resourcesBonus;
        const int supportBonus = identity == nullptr ? 0 : identity->supportBonus;
        const int mobilityBonus = identity == nullptr ? 0 : identity->mobilityBonus;
        quest.bountyTargetAwareness = clamp100(deterministicBucket(seed + "aware", 30, 78) + awarenessBonus);
        quest.bountyTargetResources = clamp100(deterministicBucket(seed + "resources", 25, 80) + resourcesBonus);
        quest.bountyTargetLocalSupport = clamp100(deterministicBucket(seed + "support", 5, 65) + supportBonus);
        quest.bountyTargetMobility = clamp100(deterministicBucket(seed + "mobility", 35, 85) + mobilityBonus);
        quest.bountyTargetKnowsTerrain = deterministicBucket(seed + "terrain", 0, 99) < 55;
        quest.bountyTargetInjured = false;
        quest.bountyTargetSuspectsHunt = false;
        quest.bountyTargetKnowsHunt = false;
    }

    // Old saves may already contain a generated target but not the V3.51.73 valuation fields.
    // Value is applied once from the persisted target traits; reloads therefore never inflate a bounty twice.
    if (!quest.bountyRewardProfileApplied)
    {
        quest.bountyBaseRewardGold = std::max(1, quest.rewardGold);

        const int traitPressure = (quest.bountyTargetAwareness + quest.bountyTargetResources + quest.bountyTargetMobility) / 3;
        const int levelDanger = safeLevel >= 25 ? 3 : (safeLevel >= 12 ? 2 : 1);
        const int traitDanger = traitPressure >= 70 ? 2 : (traitPressure >= 50 ? 1 : 0);
        quest.bountyDangerRating = std::clamp(levelDanger + traitDanger, 1, 5);

        const int notorietyBase = 1 + safeLevel / 10;
        quest.bountyNotorietyRating = std::clamp(
            notorietyBase + deterministicBucket(seed + "notoriety", 0, 2),
            1,
            5);

        const int minDays = 2 + quest.bountyNotorietyRating * 3;
        const int maxDays = 20 + safeLevel * 3 + quest.bountyNotorietyRating * 24;
        quest.bountyDaysAtLarge = deterministicBucket(seed + "days_at_large", minDays, std::max(minDays, maxDays));

        const int dangerBonus = (quest.bountyDangerRating - 1) * 11;
        const int notorietyBonus = (quest.bountyNotorietyRating - 1) * 9;
        const int fugitiveBonus = std::min(38, (quest.bountyDaysAtLarge / 14) * 3);
        const int totalBonusPercent = std::min(90, dangerBonus + notorietyBonus + fugitiveBonus);
        quest.rewardGold = std::max(
            quest.bountyBaseRewardGold,
            quest.bountyBaseRewardGold * (100 + totalBonusPercent) / 100);
        quest.bountyRewardProfileApplied = true;
    }
}

BountyPursuitPreparation BountyQuestSystem::readPreparation(const Quest& quest)
{
    BountyPursuitPreparation p;
    p.exitRecon = quest.bountyExitRecon;
    p.informantNetwork = quest.bountyInformantNetwork;
    p.ambushPreparation = quest.bountyAmbushPreparation;
    p.shadowingPreparation = quest.bountyShadowingPreparation;
    p.routeVariation = quest.bountyRouteVariation;
    p.decoyQuality = quest.bountyDecoyQuality;
    p.counterSurveillance = quest.bountyCounterSurveillance;
    p.refugeConcealment = quest.bountyRefugeConcealment;
    return p;
}

void BountyQuestSystem::writePreparation(Quest& quest, const BountyPursuitPreparation& p)
{
    quest.bountyExitRecon = p.exitRecon;
    quest.bountyInformantNetwork = p.informantNetwork;
    quest.bountyAmbushPreparation = p.ambushPreparation;
    quest.bountyShadowingPreparation = p.shadowingPreparation;
    quest.bountyRouteVariation = p.routeVariation;
    quest.bountyDecoyQuality = p.decoyQuality;
    quest.bountyCounterSurveillance = p.counterSurveillance;
    quest.bountyRefugeConcealment = p.refugeConcealment;
}

BountyQuestStepResult BountyQuestSystem::investigate(
    Quest& quest,
    BountyInvestigationAction action,
    int hunterStealth,
    int roll1To100)
{
    BountyQuestStepResult result;
    if (!isActiveBounty(quest)) return result;
    initializeBounty(quest, 1);

    BountyPursuitPreparation preparation = readPreparation(quest);
    const int roll = std::clamp(roll1To100, 1, 100);
    switch (action)
    {
        case BountyInvestigationAction::ObserveQuietly:
            BountyPursuitSystem::applyHunterPreparation(preparation, BountyHunterPreparationAction::ShadowTarget);
            result.cluesGained = roll <= 82 ? 1 : 0;
            quest.bountyInvestigationNoise = clamp100(quest.bountyInvestigationNoise + 2);
            result.lines.push_back("Tu observes les habitudes, les relais et les heures de passage sans aller poser de questions partout.");
            break;
        case BountyInvestigationAction::AskAround:
            BountyPursuitSystem::applyHunterPreparation(preparation, BountyHunterPreparationAction::UseInformant);
            result.cluesGained = roll <= 78 ? 2 : 1;
            quest.bountyInvestigationNoise = clamp100(quest.bountyInvestigationNoise + 18);
            quest.bountyInformantExposure = clamp100(quest.bountyInformantExposure + 12);
            result.lines.push_back("Tu poses des questions dans les relais locaux : la piste avance plus vite, mais ton intérêt peut finir par se remarquer.");
            break;
        case BountyInvestigationAction::CheckExits:
            BountyPursuitSystem::applyHunterPreparation(preparation, BountyHunterPreparationAction::ScoutExits);
            result.cluesGained = roll <= 65 ? 1 : 0;
            quest.bountyInvestigationNoise = clamp100(quest.bountyInvestigationNoise + 4);
            result.lines.push_back("Tu vérifies les routes, passages secondaires et points de sortie probables au lieu de chercher directement la cible.");
            break;
        case BountyInvestigationAction::PrepareInterception:
            BountyPursuitSystem::applyHunterPreparation(preparation, BountyHunterPreparationAction::PrepareAmbush);
            result.cluesGained = 0;
            quest.bountyInvestigationNoise = clamp100(quest.bountyInvestigationNoise + 6);
            result.lines.push_back("Tu prépares un point d'interception sur une route déjà crédible. Sans piste suffisante, ce serait seulement attendre au hasard.");
            break;
    }

    if (quest.bountyTransitTrailAvailable)
    {
        int trailBias = 0;
        if (action == BountyInvestigationAction::AskAround) trailBias = 14;
        else if (action == BountyInvestigationAction::CheckExits) trailBias = 8;
        else if (action == BountyInvestigationAction::ObserveQuietly) trailBias = 5;
        else trailBias = -8;

        const BountyTransitTrailUseResult trail = BountyRelocationSystem::useTransitTrail(quest, trailBias, roll);
        if (trail.evaluated)
        {
            result.transitTrailEvaluated = true;
            result.transitTrailUseful = trail.cluesGained > 0;
            result.cluesGained += trail.cluesGained;
            quest.bountyInvestigationNoise = clamp100(quest.bountyInvestigationNoise + trail.noiseAdded);
            result.lines.insert(result.lines.end(), trail.lines.begin(), trail.lines.end());
        }
    }

    quest.bountyClues = std::clamp(quest.bountyClues + result.cluesGained, 0, 8);
    writePreparation(quest, preparation);
    result.changed = true;
    if (result.cluesGained > 0)
        result.lines.push_back("Piste utile : +" + std::to_string(result.cluesGained) + " indice(s), total " + std::to_string(quest.bountyClues) + "/3 minimum pour tenter une interception.");
    else
        result.lines.push_back("Aucun nouvel indice solide cette fois, mais la préparation effectuée reste utile.");

    updateAwareness(quest, hunterStealth, result);
    result.pursuitAvailable = quest.bountyClues >= 3;
    quest.bountyStage = result.pursuitAvailable ? "preparation" : "investigation";
    return result;
}

BountyQuestStepResult BountyQuestSystem::attemptPursuit(
    Quest& quest,
    const BountyPursuitHunterProfile& hunter,
    int escapeRoll1To100)
{
    BountyQuestStepResult result;
    if (!isActiveBounty(quest) || quest.bountyClues < 3) return result;
    initializeBounty(quest, 1);

    BountyPursuitPreparation preparation = readPreparation(quest);
    auto targetDecision = BountyTargetBehaviorSystem::chooseCountermeasures(buildTargetContext(quest), preparation);
    preparation = targetDecision.preparation;
    writePreparation(quest, preparation);

    BountyPursuitTargetProfile target;
    target.mobility = quest.bountyTargetMobility;
    target.knownEscapeAttempts = quest.bountyTargetEscapeAttempts;
    target.knowsItIsHunted = quest.bountyTargetKnowsHunt;
    target.terrainKnownToHunter = quest.bountyExitRecon > 0;
    target.hasPreparedEscapeRoute = preparation.routeVariation > 0 || preparation.refugeConcealment > 0;

    result.pursuitPlan = BountyPursuitSystem::plan(hunter, target, preparation);
    result.escapeChancePercent = BountyPursuitSystem::adjustedEscapeChance(62, result.pursuitPlan);
    result.pursuitAttempted = true;
    result.changed = true;
    result.lines.push_back(result.pursuitPlan.summary);
    result.lines.push_back(targetDecision.summary);

    const int roll = std::clamp(escapeRoll1To100, 1, 100);
    if (roll <= result.escapeChancePercent)
    {
        result.escaped = true;
        quest.bountyTargetEscapeAttempts++;
        quest.bountyTargetKnowsHunt = true;
        quest.bountyTargetSuspectsHunt = true;
        quest.bountyClues = std::max(1, quest.bountyClues - 2);
        quest.bountyStage = "investigation";
        // The hunter keeps broad knowledge, but exact ambush/shadow positions expire after a failed interception.
        quest.bountyAmbushPreparation = std::max(0, quest.bountyAmbushPreparation - 1);
        quest.bountyShadowingPreparation = std::max(0, quest.bountyShadowingPreparation - 1);
        result.lines.push_back("La cible trouve une ouverture et s'échappe. Cette fois, elle sait qu'une chasse est réellement en cours.");
    }
    else
    {
        result.intercepted = true;
        quest.bountyStage = "confrontation";
        result.lines.push_back("Interception réussie : la cible n'a plus de route propre pour quitter immédiatement la zone.");
    }

    return result;
}

std::string BountyQuestSystem::stageLabel(const Quest& quest)
{
    if (!quest.bountyMission) return "pas une prime ciblée";
    if (quest.completed) return "cible neutralisée / contrat prêt à rendre";
    if (quest.bountyStage == "confrontation") return "confrontation";
    if (quest.bountyStage == "preparation") return "piste suffisante — interception possible";
    return "enquête et localisation";
}

std::string BountyQuestSystem::awarenessLabel(const Quest& quest)
{
    if (quest.bountyTargetKnowsHunt) return "la cible sait qu'elle est chassée";
    if (quest.bountyTargetSuspectsHunt) return "la cible soupçonne une surveillance";
    return "rien n'indique que la cible sache qu'elle est chassée";
}

std::string BountyQuestSystem::investigationActionLabel(BountyInvestigationAction action)
{
    switch (action)
    {
        case BountyInvestigationAction::ObserveQuietly: return "Observer discrètement";
        case BountyInvestigationAction::AskAround: return "Poser des questions dans le secteur";
        case BountyInvestigationAction::CheckExits: return "Repérer les sorties et routes secondaires";
        case BountyInvestigationAction::PrepareInterception: return "Préparer un point d'interception";
    }
    return "Action de traque";
}


std::string BountyQuestSystem::dangerLabel(const Quest& quest)
{
    switch (std::clamp(quest.bountyDangerRating, 1, 5))
    {
        case 1: return "faible";
        case 2: return "modérée";
        case 3: return "sérieuse";
        case 4: return "très élevée";
        case 5: return "extrême";
    }
    return "inconnue";
}

std::string BountyQuestSystem::notorietyLabel(const Quest& quest)
{
    switch (std::clamp(quest.bountyNotorietyRating, 1, 5))
    {
        case 1: return "locale";
        case 2: return "connue";
        case 3: return "importante";
        case 4: return "majeure";
        case 5: return "prioritaire";
    }
    return "inconnue";
}

std::string BountyQuestSystem::contractValueSummary(const Quest& quest)
{
    return "Dangerosité : " + dangerLabel(quest) + " (" + std::to_string(std::clamp(quest.bountyDangerRating, 1, 5)) + "/5)"
        + " | Notoriété : " + notorietyLabel(quest) + " (" + std::to_string(std::clamp(quest.bountyNotorietyRating, 1, 5)) + "/5)"
        + " | En fuite depuis environ " + std::to_string(std::max(0, quest.bountyDaysAtLarge)) + " jour(s).";
}
