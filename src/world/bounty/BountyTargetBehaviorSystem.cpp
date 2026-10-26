#include "world/bounty/BountyTargetBehaviorSystem.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    int currentLevel(const BountyPursuitPreparation& p, BountyTargetCountermeasure action)
    {
        switch (action)
        {
            case BountyTargetCountermeasure::ChangeRoute: return p.routeVariation;
            case BountyTargetCountermeasure::UseDecoy: return p.decoyQuality;
            case BountyTargetCountermeasure::CounterSurveillance: return p.counterSurveillance;
            case BountyTargetCountermeasure::HideOrChangeRefuge: return p.refugeConcealment;
        }
        return 0;
    }
}

BountyTargetBehaviorDecision BountyTargetBehaviorSystem::chooseCountermeasures(
    const BountyTargetBehaviorContext& context,
    const BountyPursuitPreparation& existing)
{
    BountyTargetBehaviorDecision result;
    result.preparation = existing;

    const int awareness = std::clamp(context.awareness, 0, 100);
    const int resources = std::clamp(context.resources, 0, 100);
    const int support = std::clamp(context.localSupport, 0, 100);
    const int mobility = std::clamp(context.mobility, 0, 100);
    const int window = std::clamp(context.preparationWindow, 0, 4);
    const int previousEscapes = std::clamp(context.previousEscapeAttempts, 0, 12);
    const std::string mood = lower(context.temperament);

    if (window <= 0)
    {
        result.summary = "La cible ne dispose d'aucune fenêtre crédible pour préparer sa fuite.";
        return result;
    }

    // A wanted person does not magically know that a hunter is currently following them.
    // Without a concrete suspicion, their normal caution is not converted into anti-hunt bonuses.
    if (!context.knowsItIsHunted && !context.suspectsSurveillance)
    {
        result.summary = "La cible ignore qu'une traque est en cours : elle suit sa routine et ne prépare aucune contre-mesure spécifique.";
        return result;
    }

    if (awareness < 20)
    {
        result.summary = "La cible a un doute trop faible pour organiser un vrai contre-pistage.";
        return result;
    }

    int actionBudget = window;
    if (resources < 25) actionBudget = std::min(actionBudget, 1);
    else if (resources < 55) actionBudget = std::min(actionBudget, 2);
    else if (resources < 80) actionBudget = std::min(actionBudget, 3);
    result.availableActions = actionBudget;

    struct Candidate { BountyTargetCountermeasure action; int score; };
    std::array<Candidate, 4> candidates = {{
        {BountyTargetCountermeasure::ChangeRoute,
            28 + mobility / 2 + (context.knowsTerrain ? 18 : 0) + previousEscapes * 2
                + ((mood.find("prudent") != std::string::npos || mood.find("surviv") != std::string::npos) ? 12 : 0)},
        {BountyTargetCountermeasure::UseDecoy,
            18 + resources / 2 + support / 4
                + ((mood.find("rus") != std::string::npos || mood.find("menteur") != std::string::npos || mood.find("opportun") != std::string::npos) ? 18 : 0)},
        {BountyTargetCountermeasure::CounterSurveillance,
            24 + awareness / 2 + previousEscapes * 2
                + ((mood.find("méthod") != std::string::npos || mood.find("method") != std::string::npos || mood.find("méfi") != std::string::npos) ? 16 : 0)},
        {BountyTargetCountermeasure::HideOrChangeRefuge,
            20 + support / 2 + (context.injured ? 24 : 0)
                + ((mood.find("prudent") != std::string::npos || mood.find("craint") != std::string::npos || mood.find("discret") != std::string::npos) ? 14 : 0)}
    }};

    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return a.score > b.score;
    });

    // A target that merely suspects surveillance reacts cautiously instead of somehow knowing
    // it needs a full escape package. Confirmation unlocks decoys/refuge changes.
    if (!context.knowsItIsHunted && context.suspectsSurveillance)
    {
        actionBudget = std::min(actionBudget, 1);
        result.availableActions = actionBudget;
    }

    for (const Candidate& candidate : candidates)
    {
        if (static_cast<int>(result.selectedActions.size()) >= actionBudget) break;
        if (candidate.score < 42) continue;
        if (!context.knowsItIsHunted && context.suspectsSurveillance
            && candidate.action != BountyTargetCountermeasure::CounterSurveillance
            && candidate.action != BountyTargetCountermeasure::ChangeRoute)
        {
            continue;
        }
        if (currentLevel(result.preparation, candidate.action) >= 2) continue;
        BountyPursuitSystem::applyTargetCountermeasure(result.preparation, candidate.action);
        result.selectedActions.push_back(candidate.action);
    }

    result.counterplayScore = std::clamp(
        result.preparation.routeVariation * 14
        + result.preparation.decoyQuality * 15
        + result.preparation.counterSurveillance * 13
        + result.preparation.refugeConcealment * 11,
        0, 100);

    if (result.selectedActions.empty())
        result.summary = "La cible sait qu'elle est menacée mais ne dispose pas d'un contre-jeu crédible supplémentaire.";
    else if (result.counterplayScore >= 55)
        result.summary = "La cible combine plusieurs contre-mesures cohérentes et peut réellement rouvrir des routes de fuite.";
    else
        result.summary = "La cible prépare une réponse limitée à la traque, sans connaître magiquement le plan du chasseur.";

    return result;
}

std::string BountyTargetBehaviorSystem::actionLabel(BountyTargetCountermeasure action)
{
    switch (action)
    {
        case BountyTargetCountermeasure::ChangeRoute: return "changer de route";
        case BountyTargetCountermeasure::UseDecoy: return "utiliser un leurre";
        case BountyTargetCountermeasure::CounterSurveillance: return "contre-surveillance";
        case BountyTargetCountermeasure::HideOrChangeRefuge: return "se cacher ou changer de refuge";
    }
    return "contre-mesure inconnue";
}
