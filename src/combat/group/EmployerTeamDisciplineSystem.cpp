#include "combat/group/EmployerTeamDisciplineSystem.hpp"

#include "combat/group/TeamEmploymentSystem.hpp"
#include "entity/Player.hpp"

#include <algorithm>

namespace
{
    constexpr const char* Category = "team_employment_discipline";

    const PlayerJournalRecord* findRecord(const Player& player, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == Category && record.key == key)
            {
                return &record;
            }
        }
        return nullptr;
    }

    std::string labelFor(const Player& player, const std::string& key, const std::string& fallback)
    {
        const PlayerJournalRecord* record = findRecord(player, key);
        return record == nullptr || record->label.empty() ? fallback : record->label;
    }

    int intFor(const Player& player, const std::string& key, int fallback)
    {
        try
        {
            return std::stoi(labelFor(player, key, std::to_string(fallback)));
        }
        catch (...)
        {
            return fallback;
        }
    }

    void setState(Player& player, const std::string& key, int value)
    {
        player.recordCanonicalEvent(Category, key, std::to_string(value), 1);
    }

    void setText(Player& player, const std::string& key, const std::string& value)
    {
        player.recordCanonicalEvent(Category, key, value.empty() ? "-" : value, 1);
    }

    void updateReserveFromStanding(Player& player, int trust, int warnings)
    {
        const int current = std::max(0, intFor(player, "reserve_battles", 0));
        int wanted = current;
        if (warnings >= 3 || trust <= 25)
        {
            wanted = std::max(wanted, 2);
        }
        else if (warnings >= 2 || trust <= EmployerTeamDisciplineSystem::ReserveTrustThreshold)
        {
            wanted = std::max(wanted, 1);
        }
        if (wanted != current)
        {
            setState(player, "reserve_battles", wanted);
            player.recordCanonicalEvent(
                "team_employment_discipline_history",
                "reserve_assignment_day_" + std::to_string(player.getWorldDaysElapsed()),
                "Le chef place temporairement le joueur en liste d'attente pour " + std::to_string(wanted) + " combat(s) de service.",
                1
            );
        }
    }

    void applyTrustChange(Player& player, int delta, const std::string& reason)
    {
        const int before = std::clamp(intFor(player, "trust", EmployerTeamDisciplineSystem::InitialProfessionalTrust), 0, 100);
        const int after = std::clamp(before + delta, 0, 100);
        setState(player, "trust", after);
        setText(player, "last_reason", reason);
        updateReserveFromStanding(player, after, std::max(0, intFor(player, "warnings", 0)));
    }

    void addWarning(Player& player, const std::string& reason)
    {
        const int warnings = std::max(0, intFor(player, "warnings", 0)) + 1;
        setState(player, "warnings", warnings);
        setText(player, "last_reason", reason);
        player.recordCanonicalEvent(
            "team_employment_discipline_history",
            "warning_" + std::to_string(warnings),
            "Avertissement professionnel n°" + std::to_string(warnings) + " : " + reason,
            1
        );
        updateReserveFromStanding(player, std::clamp(intFor(player, "trust", EmployerTeamDisciplineSystem::InitialProfessionalTrust), 0, 100), warnings);
    }
}

void EmployerTeamDisciplineSystem::initializeForContract(Player& player)
{
    if (!TeamEmploymentSystem::hasActiveContract(player)) return;
    setState(player, "trust", InitialProfessionalTrust);
    setState(player, "warnings", 0);
    setState(player, "missions_completed", 0);
    setState(player, "missions_failed", 0);
    setState(player, "orders_obeyed", 0);
    setState(player, "orders_disobeyed", 0);
    setState(player, "orders_justified", 0);
    setState(player, "reserve_battles", 0);
    setText(player, "last_reason", "prise_de_poste");
}

EmployerTeamDisciplineState EmployerTeamDisciplineSystem::stateFor(const Player& player)
{
    EmployerTeamDisciplineState state;
    state.professionalTrust = std::clamp(intFor(player, "trust", InitialProfessionalTrust), 0, 100);
    state.warnings = std::max(0, intFor(player, "warnings", 0));
    state.completedMissions = std::max(0, intFor(player, "missions_completed", 0));
    state.failedMissions = std::max(0, intFor(player, "missions_failed", 0));
    state.obeyedOrders = std::max(0, intFor(player, "orders_obeyed", 0));
    state.unjustifiedDisobedience = std::max(0, intFor(player, "orders_disobeyed", 0));
    state.justifiedDisobedience = std::max(0, intFor(player, "orders_justified", 0));
    state.reserveBattlesRemaining = std::max(0, intFor(player, "reserve_battles", 0));
    state.lastReason = labelFor(player, "last_reason", "-");
    return state;
}

void EmployerTeamDisciplineSystem::recordMissionCompleted(Player& player, const std::string& missionId)
{
    if (!TeamEmploymentSystem::hasActiveContract(player)) return;
    const int completed = std::max(0, intFor(player, "missions_completed", 0)) + 1;
    setState(player, "missions_completed", completed);
    applyTrustChange(player, +9, "mission_accomplie:" + missionId);
    player.recordCanonicalEvent(
        "team_employment_discipline_history",
        "mission_success_" + missionId,
        "Mission employeur accomplie : confiance professionnelle renforcée.",
        1
    );
}

void EmployerTeamDisciplineSystem::recordMissionFailed(Player& player, const std::string& missionId, const std::string& reason)
{
    if (!TeamEmploymentSystem::hasActiveContract(player)) return;
    const int failed = std::max(0, intFor(player, "missions_failed", 0)) + 1;
    setState(player, "missions_failed", failed);
    applyTrustChange(player, -17, reason + ":" + missionId);
    addWarning(player, reason.empty() ? "mission employeur non accomplie" : reason);
}

void EmployerTeamDisciplineSystem::recordOrderOutcome(Player& player, const EmployerTeamOrderOutcome& outcome)
{
    if (!TeamEmploymentSystem::hasActiveContract(player)) return;

    if (outcome.obeyed)
    {
        setState(player, "orders_obeyed", std::max(0, intFor(player, "orders_obeyed", 0)) + 1);
        applyTrustChange(player, +2, outcome.reason.empty() ? "ordre_execute" : outcome.reason);
        return;
    }

    if (outcome.justifiedDisobedience)
    {
        setState(player, "orders_justified", std::max(0, intFor(player, "orders_justified", 0)) + 1);
        applyTrustChange(player, outcome.preventedSeriousHarm ? +3 : 0,
            outcome.reason.empty() ? "desobeissance_justifiee" : outcome.reason);
        return;
    }

    const int disobedience = std::max(0, intFor(player, "orders_disobeyed", 0)) + 1;
    setState(player, "orders_disobeyed", disobedience);
    applyTrustChange(player, -11, outcome.reason.empty() ? "ordre_ignore" : outcome.reason);
    if ((disobedience % 2) == 0)
    {
        addWarning(player, outcome.reason.empty() ? "ordres répétés ignorés sans justification" : outcome.reason);
    }
}


std::string EmployerTeamDisciplineSystem::orderObservationVerdictLabel(EmployerOrderObservationVerdict verdict)
{
    switch (verdict)
    {
        case EmployerOrderObservationVerdict::Unobserved: return "non observé";
        case EmployerOrderObservationVerdict::Inconclusive: return "inconclusif";
        case EmployerOrderObservationVerdict::Obeyed: return "obéi";
        case EmployerOrderObservationVerdict::JustifiedDisobedience: return "désobéissance justifiée";
        case EmployerOrderObservationVerdict::UnjustifiedDisobedience: return "désobéissance injustifiée";
        case EmployerOrderObservationVerdict::NotApplicable:
        default: return "non applicable";
    }
}

EmployerObservedOrderActionAssessment EmployerTeamDisciplineSystem::assessObservedOrderAction(
    const EmployerObservedOrderActionContext& context)
{
    EmployerObservedOrderActionAssessment assessment;
    assessment.directlyObservedByLeader = context.leaderPresent;
    assessment.credibleWitnessCount = std::max(0, context.witnessCount);

    if (context.orderKind == EmployerTacticalOrderKind::None || context.orderCode.empty())
    {
        assessment.reason = "aucun ordre tactique précis n'était actif";
        return assessment;
    }

    if (!context.leaderPresent && context.witnessCount <= 0)
    {
        assessment.verdict = EmployerOrderObservationVerdict::Unobserved;
        assessment.reason = "aucun chef ni collègue crédible n'a pu constater l'action";
        return assessment;
    }

    const int hpPercent = std::clamp(context.playerHpPercent, 0, 100);
    const bool survivalEmergency = context.immediateSurvivalThreat || hpPercent <= 20;
    const std::string& action = context.actionKind;

    if (action == "forced_skip")
    {
        assessment.verdict = EmployerOrderObservationVerdict::Inconclusive;
        assessment.reason = "le tour a été perdu par un état subi et non par un choix volontaire";
        return assessment;
    }

    if (action == "escape")
    {
        if (survivalEmergency)
        {
            assessment.verdict = EmployerOrderObservationVerdict::JustifiedDisobedience;
            assessment.preventedSeriousHarm = context.escapeSucceeded;
            assessment.reason = context.escapeSucceeded
                ? "retrait hors consigne, mais urgence de survie crédible et extraction réussie"
                : "tentative de retrait hors consigne dans une urgence de survie crédible";
        }
        else
        {
            assessment.verdict = EmployerOrderObservationVerdict::UnjustifiedDisobedience;
            assessment.reason = "tentative de quitter le combat malgré un ordre tactique observable et sans urgence vitale manifeste";
        }
        return assessment;
    }

    if (context.orderKind == EmployerTacticalOrderKind::FocusFire
        && (action == "basic_attack" || action == "skill" || action == "tactical_action"))
    {
        if (context.orderedTargetId.empty() || !context.orderedTargetStillActive)
        {
            assessment.verdict = EmployerOrderObservationVerdict::NotApplicable;
            assessment.reason = "la cible de concentration n'est plus active : l'ordre de focus a expiré";
            return assessment;
        }
        if (context.actionTargetId.empty())
        {
            assessment.verdict = EmployerOrderObservationVerdict::Inconclusive;
            assessment.reason = "l'offensive est visible, mais cette action ne conserve pas une cible unique vérifiable";
            return assessment;
        }
        if (context.actionTargetId == context.orderedTargetId)
        {
            assessment.verdict = EmployerOrderObservationVerdict::Obeyed;
            assessment.reason = "la cible attaquée correspond exactement à la cible de concentration : " + context.orderedTargetName;
            return assessment;
        }
        if (context.targetForcedByCombatRules)
        {
            assessment.verdict = EmployerOrderObservationVerdict::JustifiedDisobedience;
            assessment.reason = "la cible diffère du focus, mais une règle de combat imposait réellement " + context.actionTargetName;
            return assessment;
        }
        assessment.verdict = EmployerOrderObservationVerdict::UnjustifiedDisobedience;
        assessment.reason = "la cible de focus " + context.orderedTargetName + " était encore active, mais l'action a visé " + context.actionTargetName;
        return assessment;
    }

    if (context.orderKind == EmployerTacticalOrderKind::HoldLine
        && (action == "defense" || action == "wait"))
    {
        assessment.verdict = EmployerOrderObservationVerdict::Obeyed;
        assessment.reason = "la position tenue correspond directement à l'ordre de conserver la ligne";
        return assessment;
    }

    if (context.orderKind == EmployerTacticalOrderKind::ControlledBreakthrough
        && (action == "basic_attack" || action == "skill"))
    {
        assessment.verdict = EmployerOrderObservationVerdict::Obeyed;
        assessment.reason = "l'action offensive contribue clairement à la percée coordonnée demandée";
        return assessment;
    }

    if (context.orderKind == EmployerTacticalOrderKind::ExtractWounded
        && action == "team_order")
    {
        assessment.verdict = EmployerOrderObservationVerdict::Obeyed;
        assessment.reason = "le joueur utilise son tour pour coordonner les alliés pendant l'extraction demandée";
        return assessment;
    }

    if (context.orderKind == EmployerTacticalOrderKind::ExtractWounded
        && action == "consumable" && hpPercent <= 30)
    {
        assessment.verdict = EmployerOrderObservationVerdict::Obeyed;
        assessment.reason = "le joueur se stabilise alors qu'il est lui-même assez blessé pour compromettre l'extraction";
        return assessment;
    }

    assessment.verdict = EmployerOrderObservationVerdict::Inconclusive;
    if (context.orderKind == EmployerTacticalOrderKind::ProtectSupport)
    {
        assessment.reason = "l'action est visible, mais le suivi ne prouve pas qu'elle protégeait réellement le soutien désigné";
    }
    else
    {
        assessment.reason = "l'action observée n'est ni une contradiction claire ni une exécution assez certaine de l'ordre";
    }
    return assessment;
}

EmployerObservedOrderActionAssessment EmployerTeamDisciplineSystem::assessUnauthorizedIndividualSwitch(
    const EmployerUnauthorizedSwitchContext& context)
{
    EmployerObservedOrderActionAssessment assessment;
    assessment.directlyObservedByLeader = context.leaderPresent;
    assessment.credibleWitnessCount = std::max(0, context.witnessCount);

    if (!context.leaderPresent && context.witnessCount <= 0)
    {
        assessment.verdict = EmployerOrderObservationVerdict::Unobserved;
        assessment.reason = "aucun témoin crédible n'a vu le switch autonome";
        return assessment;
    }

    const int hp = std::clamp(context.outgoingHpPercent, 0, 100);
    const int incomingHp = std::clamp(context.incomingHpPercent, 0, 100);
    const int fear = std::clamp(context.fearPercent, 0, 100);

    if (context.survivalUrgent || hp <= 15)
    {
        assessment.verdict = EmployerOrderObservationVerdict::JustifiedDisobedience;
        assessment.preventedSeriousHarm = context.switchSucceeded;
        assessment.reason = "switch sans autorisation, mais urgence de survie immédiate crédible";
        return assessment;
    }

    if (context.tacticalOpportunity && incomingHp >= hp + 20)
    {
        assessment.verdict = EmployerOrderObservationVerdict::JustifiedDisobedience;
        assessment.reason = "switch autonome tactiquement défendable : un remplaçant nettement plus frais pouvait stabiliser la ligne";
        return assessment;
    }

    if (fear >= 70 && hp <= 30)
    {
        assessment.verdict = EmployerOrderObservationVerdict::Inconclusive;
        assessment.reason = "peur élevée et blessures sérieuses : l'écart doit être revu après combat, pas sanctionné automatiquement";
        return assessment;
    }

    if (hp >= 60 && fear < 50 && !context.tacticalOpportunity)
    {
        assessment.verdict = EmployerOrderObservationVerdict::UnjustifiedDisobedience;
        assessment.reason = "switch sans autorisation alors que l'état du membre et la situation ne montrent ni urgence ni avantage tactique clair";
        return assessment;
    }

    assessment.verdict = EmployerOrderObservationVerdict::Inconclusive;
    assessment.reason = "switch autonome observable, mais les faits disponibles ne suffisent pas pour conclure à une faute ou à une justification";
    return assessment;
}

void EmployerTeamDisciplineSystem::appendObservedOrderAction(
    Player& player,
    EmployerOrderCombatReview& review,
    const EmployerObservedOrderActionContext& context,
    const std::string& combatContextId,
    int turnNumber)
{
    const EmployerObservedOrderActionAssessment assessment = assessObservedOrderAction(context);
    if (assessment.verdict == EmployerOrderObservationVerdict::NotApplicable) return;

    if (assessment.verdict == EmployerOrderObservationVerdict::Unobserved)
    {
        ++review.unobservedActions;
    }
    else
    {
        ++review.observedActions;
        if (assessment.directlyObservedByLeader) ++review.directLeaderObservations;
        else ++review.reportedObservations;
    }

    switch (assessment.verdict)
    {
        case EmployerOrderObservationVerdict::Obeyed:
            ++review.obeyedActions;
            review.lastObeyedReason = assessment.reason;
            break;
        case EmployerOrderObservationVerdict::JustifiedDisobedience:
            ++review.justifiedDisobedienceActions;
            review.preventedSeriousHarm = review.preventedSeriousHarm || assessment.preventedSeriousHarm;
            review.lastJustifiedDisobedienceReason = assessment.reason;
            break;
        case EmployerOrderObservationVerdict::UnjustifiedDisobedience:
            ++review.unjustifiedDisobedienceActions;
            review.lastUnjustifiedDisobedienceReason = assessment.reason;
            break;
        case EmployerOrderObservationVerdict::Inconclusive:
            ++review.inconclusiveActions;
            review.lastInconclusiveReason = assessment.reason;
            break;
        case EmployerOrderObservationVerdict::Unobserved:
        case EmployerOrderObservationVerdict::NotApplicable:
        default:
            break;
    }

    review.lastObservedReason = assessment.reason;

    std::string observationSource;
    if (assessment.verdict == EmployerOrderObservationVerdict::Unobserved)
    {
        observationSource = "aucun_temoin";
    }
    else if (assessment.directlyObservedByLeader)
    {
        observationSource = "chef_direct";
    }
    else
    {
        observationSource = "temoins_equipe=" + std::to_string(assessment.credibleWitnessCount);
    }

    player.recordCanonicalEvent(
        "team_employer_order_observations",
        combatContextId + ":tour_" + std::to_string(std::max(1, turnNumber)) + ":" + context.actionKind,
        "ordre=" + context.orderCode
            + " | action=" + context.actionKind
            + " | cible_action=" + (context.actionTargetName.empty() ? "-" : context.actionTargetName)
            + " | cible_action_id=" + (context.actionTargetId.empty() ? "-" : context.actionTargetId)
            + " | cible_ordre=" + (context.orderedTargetName.empty() ? "-" : context.orderedTargetName)
            + " | cible_ordre_id=" + (context.orderedTargetId.empty() ? "-" : context.orderedTargetId)
            + " | cible_forcee=" + std::string(context.targetForcedByCombatRules ? "oui" : "non")
            + " | verdict=" + orderObservationVerdictLabel(assessment.verdict)
            + " | source=" + observationSource
            + " | raison=" + assessment.reason,
        1
    );
}

EmployerOrderReviewOutcome EmployerTeamDisciplineSystem::finalizeOrderReview(
    Player& player,
    const EmployerOrderCombatReview& review,
    const std::string& orderCode,
    const std::string& combatContextId)
{
    EmployerOrderReviewOutcome result;
    if (orderCode.empty()) return result;

    if (review.observedActions <= 0)
    {
        result.verdict = EmployerOrderObservationVerdict::Unobserved;
        result.reason = "aucune action du joueur n'a été observée de façon crédible";
        result.reaction = "Aucun rapport disciplinaire fiable n'est établi : personne de crédible n'a réellement vu assez de choses pour juger ton comportement.";
    }
    else if (review.unjustifiedDisobedienceActions > 0)
    {
        result.verdict = EmployerOrderObservationVerdict::UnjustifiedDisobedience;
        result.reason = review.lastUnjustifiedDisobedienceReason.empty()
            ? "désobéissance observable sans justification suffisante"
            : review.lastUnjustifiedDisobedienceReason;
        EmployerTeamOrderOutcome outcome;
        outcome.obeyed = false;
        outcome.justifiedDisobedience = false;
        outcome.reason = "ordre_tactique_observe:" + orderCode + ":" + combatContextId;
        recordOrderOutcome(player, outcome);
        result.disciplineRecorded = true;
        result.reaction = "Le chef retient une désobéissance observable. Ce n'est pas le résultat du combat qui te condamne : c'est un choix précis, vu et jugé sans justification suffisante.";
    }
    else if (review.justifiedDisobedienceActions > 0)
    {
        result.verdict = EmployerOrderObservationVerdict::JustifiedDisobedience;
        result.reason = review.lastJustifiedDisobedienceReason.empty()
            ? "écart à l'ordre justifié par la situation"
            : review.lastJustifiedDisobedienceReason;
        EmployerTeamOrderOutcome outcome;
        outcome.obeyed = false;
        outcome.justifiedDisobedience = true;
        outcome.preventedSeriousHarm = review.preventedSeriousHarm;
        outcome.reason = "ordre_tactique_ecart_justifie:" + orderCode + ":" + combatContextId;
        recordOrderOutcome(player, outcome);
        result.disciplineRecorded = true;
        result.reaction = review.preventedSeriousHarm
            ? "Le chef reconnaît que tu as quitté la consigne pour éviter un désastre immédiat. L'écart est noté, mais il est considéré comme justifié."
            : "Le chef relève l'écart à la consigne, puis admet que le contexte le rendait défendable. Aucune faute disciplinaire n'est retenue.";
    }
    else if (review.obeyedActions > 0)
    {
        result.verdict = EmployerOrderObservationVerdict::Obeyed;
        result.reason = review.lastObeyedReason.empty()
            ? "ordre tactique suivi de façon observable"
            : review.lastObeyedReason;
        EmployerTeamOrderOutcome outcome;
        outcome.obeyed = true;
        outcome.reason = "ordre_tactique_execute:" + orderCode + ":" + combatContextId;
        recordOrderOutcome(player, outcome);
        result.disciplineRecorded = true;
        result.reaction = "Le chef a suffisamment vu ton comportement pour considérer la consigne comme respectée. La confiance progresse légèrement, sans récompense artificielle liée au simple résultat du combat.";
    }
    else
    {
        result.verdict = EmployerOrderObservationVerdict::Inconclusive;
        result.reason = review.lastInconclusiveReason.empty()
            ? "actions observées mais ambiguës"
            : review.lastInconclusiveReason;
        result.reaction = "Le chef a vu le combat, mais rien ne permet de transformer tes actions en faute ou en mérite certain. Aucun changement disciplinaire n'est appliqué.";
    }

    player.recordCanonicalEvent(
        "team_employer_order_reviews",
        combatContextId + ":" + orderCode + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
        "verdict=" + orderObservationVerdictLabel(result.verdict)
            + " | observes=" + std::to_string(review.observedActions)
            + " | chef_direct=" + std::to_string(review.directLeaderObservations)
            + " | rapports=" + std::to_string(review.reportedObservations)
            + " | inconclusifs=" + std::to_string(review.inconclusiveActions)
            + " | non_observes=" + std::to_string(review.unobservedActions)
            + " | raison=" + result.reason,
        1
    );

    return result;
}

bool EmployerTeamDisciplineSystem::playerStartsInReserve(const Player& player)
{
    return TeamEmploymentSystem::hasActiveContract(player) && reserveBattlesRemaining(player) > 0;
}

int EmployerTeamDisciplineSystem::reserveBattlesRemaining(const Player& player)
{
    return std::max(0, intFor(player, "reserve_battles", 0));
}

bool EmployerTeamDisciplineSystem::consumeReserveBattle(Player& player)
{
    const int current = reserveBattlesRemaining(player);
    if (current <= 0) return false;
    setState(player, "reserve_battles", current - 1);
    player.recordCanonicalEvent(
        "team_employment_discipline_history",
        "reserve_served_day_" + std::to_string(player.getWorldDaysElapsed()),
        "Une affectation en liste d'attente a été purgée après un combat de service observé.",
        1
    );
    return true;
}

bool EmployerTeamDisciplineSystem::dismissalRecommended(const Player& player)
{
    const EmployerTeamDisciplineState state = stateFor(player);
    return TeamEmploymentSystem::hasActiveContract(player)
        && (state.professionalTrust <= DismissalTrustThreshold
            || state.warnings >= 4
            || state.failedMissions >= 4);
}

int EmployerTeamDisciplineSystem::recommendedSalaryRetentionPercent(const Player& player)
{
    if (!dismissalRecommended(player)) return 0;
    const EmployerTeamDisciplineState state = stateFor(player);
    int retention = 50;
    if (state.professionalTrust <= 8 || state.warnings >= 5 || state.failedMissions >= 5)
    {
        retention = MaxDisciplinarySalaryRetentionPercent;
    }
    else if (state.professionalTrust <= DismissalTrustThreshold || state.warnings >= 4)
    {
        retention = 65;
    }
    return std::clamp(retention, 0, MaxDisciplinarySalaryRetentionPercent);
}

std::string EmployerTeamDisciplineSystem::standingLabel(const Player& player)
{
    const EmployerTeamDisciplineState state = stateFor(player);
    if (dismissalRecommended(player)) return "licenciement probable";
    if (state.reserveBattlesRemaining > 0) return "placé en liste d'attente";
    if (state.professionalTrust >= 80) return "très fiable";
    if (state.professionalTrust >= 60) return "fiable";
    if (state.professionalTrust >= 41) return "sous surveillance légère";
    return "confiance fragile";
}
