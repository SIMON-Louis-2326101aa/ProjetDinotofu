#include "combat/ally/RecruitedAllyAftermathSystem.hpp"

#include "core/Random.hpp"
#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>

namespace
{
    int journalCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return std::max(0, record.count);
            }
        }
        return 0;
    }

    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& value, std::initializer_list<const char*> terms)
    {
        const std::string text = lower(value);
        for (const char* term : terms)
        {
            if (text.find(term) != std::string::npos) return true;
        }
        return false;
    }

    const EmployerTeamMember* employerMemberFor(const Player& player, const RecruitedAllyCombatSupport& ally, std::vector<EmployerTeamMember>& storage)
    {
        if (ally.teamOrigin != AllyTeamOrigin::EmployerTeam || ally.persistentMemberId.empty()) return nullptr;
        storage = EmployerTeamRosterSystem::rosterFor(player);
        auto it = std::find_if(storage.begin(), storage.end(), [&](const EmployerTeamMember& member) {
            return member.memberId == ally.persistentMemberId;
        });
        return it == storage.end() ? nullptr : &(*it);
    }

    bool riskyForTheft(const RecruitedAllyCombatSupport& ally)
    {
        return containsAny(
            ally.job + " " + ally.trait + " " + ally.motivation,
            {"voleur", "roublard", "brigand", "pillard", "opportun", "cupide", "egoiste", "égoïste", "argent", "profit"}
        );
    }

    int reporterPriority(const Player& player, const RecruitedAllyCombatSupport& ally)
    {
        if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
        {
            std::vector<EmployerTeamMember> storage;
            const EmployerTeamMember* member = employerMemberFor(player, ally, storage);
            const int loyalty = member == nullptr ? 50 : member->loyalty;
            return RecruitedAllyCombatRoster::maturityScore(ally)
                + RecruitedAllyAftermathSystem::moodScore(player, ally)
                + loyalty
                + (ally.teamLeader ? 25 : 0);
        }
        return RecruitedAllyCombatRoster::relationshipScore(player, ally.name) * 2
            + RecruitedAllyCombatRoster::maturityScore(ally)
            + RecruitedAllyAftermathSystem::moodScore(player, ally);
    }
}

int RecruitedAllyAftermathSystem::moodScore(const Player& player, const RecruitedAllyCombatSupport& ally)
{
    if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
    {
        std::vector<EmployerTeamMember> storage;
        const EmployerTeamMember* member = employerMemberFor(player, ally, storage);
        int score = member == nullptr ? 55 : member->mood;
        if (ally.maxHp > 0)
        {
            const int hpPercent = ally.currentHp * 100 / ally.maxHp;
            if (hpPercent <= 20) score -= 14;
            else if (hpPercent <= 45) score -= 7;
        }
        if (containsAny(ally.trait, {"loyal", "calme", "protect", "solidaire"})) score += 5;
        if (containsAny(ally.trait, {"coler", "rancun", "instable", "egoiste", "égoïste"})) score -= 8;
        return std::clamp(score, 0, 100);
    }

    int score = 55;
    const int relation = RecruitedAllyCombatRoster::relationshipScore(player, ally.name);
    score += (relation - 35) / 5;

    const int unpaid = journalCount(player, "salaires_recrues_impayes", ally.name);
    score -= std::min(36, unpaid * 14);

    if (journalCount(player, "quetes_personnelles_recrues_terminees", ally.name) > 0) score += 8;
    if (journalCount(player, "sauvetages_relation_recrues", ally.name) > 0) score += 6;
    if (journalCount(player, "soins_relation_recrues", ally.name) > 0) score += 4;

    if (ally.maxHp > 0)
    {
        const int hpPercent = ally.currentHp * 100 / ally.maxHp;
        if (hpPercent <= 20) score -= 14;
        else if (hpPercent <= 45) score -= 7;
    }

    if (containsAny(ally.trait, {"loyal", "contrat", "calme", "protect"})) score += 6;
    if (containsAny(ally.trait, {"coler", "rancun", "instable", "egoiste", "égoïste"})) score -= 8;
    score -= std::clamp(ally.fatiguePercent / 8, 0, 12);
    score += std::clamp((ally.readinessPercent - 50) / 15, -3, 3);

    return std::clamp(score, 0, 100);
}

std::string RecruitedAllyAftermathSystem::moodLabel(int score)
{
    if (score >= 85) return "très bonne humeur";
    if (score >= 68) return "bonne humeur";
    if (score >= 48) return "stable";
    if (score >= 30) return "tendu";
    if (score >= 15) return "mauvaise passe";
    return "hostile / à bout";
}

int RecruitedAllyAftermathSystem::reportChancePercent(const Player& player, const RecruitedAllyCombatSupport& ally)
{
    if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
    {
        std::vector<EmployerTeamMember> storage;
        const EmployerTeamMember* member = employerMemberFor(player, ally, storage);
        const int mood = moodScore(player, ally);
        const int loyalty = member == nullptr ? 50 : member->loyalty;
        int chance = 85 + (mood - 50) / 8 + (loyalty - 50) / 12 + (ally.teamLeader ? 4 : 0);
        if (containsAny(ally.trait, {"silenc", "taciturne", "froid", "secret"})) chance -= 10;
        if (containsAny(ally.trait, {"bavard", "franc", "loyal", "protect", "solidaire"})) chance += 5;
        return std::clamp(chance, 45, 98);
    }

    const int relation = RecruitedAllyCombatRoster::relationshipScore(player, ally.name);
    const int mood = moodScore(player, ally);

    // ~85% is the normal RP baseline requested by design. Relationship and current mood can
    // move it, but a companion never becomes a deterministic exposition machine.
    int chance = 85 + (relation - 35) / 10 + (mood - 50) / 10;
    if (containsAny(ally.trait, {"silenc", "taciturne", "froid", "secret"})) chance -= 10;
    if (containsAny(ally.trait, {"bavard", "franc", "loyal", "protect"})) chance += 5;
    return std::clamp(chance, 45, 98);
}

int RecruitedAllyAftermathSystem::theftChancePercent(const Player& player, const RecruitedAllyCombatSupport& ally)
{
    if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
    {
        if (!riskyForTheft(ally)) return 0;
        std::vector<EmployerTeamMember> storage;
        const EmployerTeamMember* member = employerMemberFor(player, ally, storage);
        const int loyalty = member == nullptr ? 50 : member->loyalty;
        const int mood = moodScore(player, ally);
        if (loyalty >= 70 && mood >= 45) return 0;
        int chance = 2;
        if (loyalty < 40) chance += 5;
        if (loyalty < 25) chance += 5;
        if (mood < 35) chance += 4;
        if (mood < 20) chance += 3;
        return std::clamp(chance, 0, 18);
    }

    const int relation = RecruitedAllyCombatRoster::relationshipScore(player, ally.name);
    const int mood = moodScore(player, ally);
    const int unpaid = journalCount(player, "salaires_recrues_impayes", ally.name);
    const bool risky = riskyForTheft(ally);

    // No arbitrary betrayal: it needs an actually dubious profile or an already damaged contract.
    if (!risky && unpaid <= 0) return 0;
    if (relation >= 55 && unpaid <= 0) return 0;

    int chance = risky ? 3 : 0;
    if (relation < 30) chance += 4;
    if (relation < 15) chance += 4;
    if (mood < 35) chance += 5;
    if (mood < 20) chance += 4;
    chance += std::min(8, unpaid * 4);
    if (journalCount(player, "quetes_personnelles_recrues_terminees", ally.name) > 0) chance -= 7;
    if (journalCount(player, "sauvetages_relation_recrues", ally.name) > 0) chance -= 5;
    return std::clamp(chance, 0, 24);
}

int RecruitedAllyAftermathSystem::carryingCapacityUnits(const RecruitedAllyCombatSupport& ally)
{
    if (ally.currentHp <= 0 || ally.maxHp <= 0) return 0;

    // Abstract cargo units intentionally stay small and readable. They represent what remains
    // available for salvage after the ally carries their own equipment and survives the fight.
    int units = 3;
    const int hpPercent = ally.currentHp * 100 / ally.maxHp;
    if (hpPercent <= 20) units -= 2;
    else if (hpPercent <= 45) units -= 1;

    const std::string bodyProfile = ally.race + " " + ally.job + " " + ally.trait;
    if (containsAny(bodyProfile, {
            "orc", "nain", "dwarf", "semi-dragon", "dragon", "golem", "massif", "massive",
            "brute", "guerrier", "garde", "soldat", "paladin", "forgeron", "tank", "porteur"}))
    {
        ++units;
    }
    if (containsAny(bodyProfile, {
            "fée", "fee", "gnome", "halfelin", "petit", "petite", "minuscule", "frêle", "frele"}))
    {
        --units;
    }

    return std::clamp(units, 1, 5);
}

RecruitedAllyLootTransportPlan RecruitedAllyAftermathSystem::buildLootTransportPlan(
    const std::vector<RecruitedAllyCombatSupport>& survivingAllies,
    int defeatedEnemyBundles,
    bool transportingPlayer)
{
    RecruitedAllyLootTransportPlan plan;
    defeatedEnemyBundles = std::max(0, defeatedEnemyBundles);

    for (const RecruitedAllyCombatSupport& ally : survivingAllies)
    {
        if (ally.currentHp <= 0) continue;
        ++plan.capableSurvivors;
        const int units = carryingCapacityUnits(ally);
        plan.totalCarryUnits += units;
        if (units >= 4) ++plan.strongCarriers;
        if (ally.maxHp > 0 && ally.currentHp * 100 / ally.maxHp <= 35) ++plan.injuredSurvivors;
    }

    if (plan.capableSurvivors <= 0 || defeatedEnemyBundles <= 0) return plan;

    plan.playerTransportConsumesCapacity = transportingPlayer;
    if (transportingPlayer)
    {
        // Extracting an unconscious person ties up hands, balance and attention. One average
        // survivor still keeps room for a small pouch; larger groups gain cargo room but not for free.
        plan.playerTransportUnitCost = std::min(
            plan.totalCarryUnits,
            2 + std::max(0, plan.capableSurvivors - 1));
    }

    plan.availableLootCarryUnits = std::max(0, plan.totalCarryUnits - plan.playerTransportUnitCost);

    // One compact enemy salvage bundle consumes one unit. Equipment is resolved later from the
    // same shared pool using its actual light/medium/heavy weight class, so this is only an upper
    // bound on how many defeated enemies can be searched and carried home.
    plan.portableBundleCapacity = plan.availableLootCarryUnits;
    plan.recoverableEnemyBundles = std::min(defeatedEnemyBundles, plan.portableBundleCapacity);
    plan.abandonedEnemyBundles = std::max(0, defeatedEnemyBundles - plan.recoverableEnemyBundles);
    plan.heavyItemCapacity = plan.availableLootCarryUnits / 2;
    return plan;
}

PostKoRecoverySelection RecruitedAllyAftermathSystem::selectPostKoRecoveryCandidates(
    const std::vector<PostKoRecoveryCandidate>& candidates,
    int availableCarryUnits)
{
    PostKoRecoverySelection result;
    result.remainingCarryUnits = std::max(0, availableCarryUnits);

    std::vector<PostKoRecoveryCandidate> ordered = candidates;
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        if (a.protectedFromAutomaticAbandon != b.protectedFromAutomaticAbandon)
            return a.protectedFromAutomaticAbandon > b.protectedFromAutomaticAbandon;
        if (a.priorityScore != b.priorityScore)
            return a.priorityScore > b.priorityScore;
        if (a.transportCost != b.transportCost)
            return a.transportCost < b.transportCost;
        return a.id < b.id;
    });

    for (const PostKoRecoveryCandidate& candidate : ordered)
    {
        const int cost = std::max(1, candidate.transportCost);
        if (result.remainingCarryUnits >= cost)
        {
            result.remainingCarryUnits -= cost;
            result.usedCarryUnits += cost;
            result.recoveredIds.push_back(candidate.id);
        }
        else
        {
            result.deferredIds.push_back(candidate.id);
        }
    }
    return result;
}

RecruitedAllyAftermathDecision RecruitedAllyAftermathSystem::decide(
    Player& player,
    const std::vector<RecruitedAllyCombatSupport>& survivingAllies,
    Random& random,
    const std::string& contextId
)
{
    RecruitedAllyAftermathDecision decision;
    if (survivingAllies.empty()) return decision;

    const RecruitedAllyCombatSupport* reporter = nullptr;
    int bestPriority = -1;
    for (const RecruitedAllyCombatSupport& ally : survivingAllies)
    {
        if (ally.currentHp <= 0 || !ally.postKoAvailableForDebrief) continue;
        const int priority = reporterPriority(player, ally);
        if (reporter == nullptr || priority > bestPriority)
        {
            reporter = &ally;
            bestPriority = priority;
        }
    }
    if (reporter == nullptr) return decision;

    decision.memberName = reporter->name;
    decision.teamOrigin = reporter->teamOrigin;
    decision.persistentMemberId = reporter->persistentMemberId;
    if (reporter->teamOrigin == AllyTeamOrigin::EmployerTeam)
    {
        std::vector<EmployerTeamMember> storage;
        const EmployerTeamMember* member = employerMemberFor(player, *reporter, storage);
        decision.relationshipScore = member == nullptr ? 50 : member->loyalty;
    }
    else
    {
        decision.relationshipScore = RecruitedAllyCombatRoster::relationshipScore(player, reporter->name);
    }
    decision.moodScore = moodScore(player, *reporter);
    decision.moodLabel = moodLabel(decision.moodScore);
    decision.reportChancePercent = reportChancePercent(player, *reporter);
    decision.theftChancePercent = theftChancePercent(player, *reporter);

    decision.givesDetailedReport = random.between(1, 100) <= decision.reportChancePercent;
    decision.refusesDetailedReport = !decision.givesDetailedReport;
    if (decision.refusesDetailedReport)
    {
        if (decision.moodScore < 30)
            decision.refusalReason = "Il/elle est dans une mauvaise passe et ne veut pas revenir sur le combat.";
        else if (decision.relationshipScore < 20)
            decision.refusalReason = "La relation est trop froide pour un débrief détaillé.";
        else
            decision.refusalReason = "Il/elle estime qu'un compte rendu détaillé n'est pas nécessaire.";
    }

    decision.stealsPlayerShare = decision.theftChancePercent > 0
        && random.between(1, 100) <= decision.theftChancePercent;

    const bool employer = reporter->teamOrigin == AllyTeamOrigin::EmployerTeam;
    player.recordCanonicalEvent(
        employer ? "team_employer_member_mood" : "humeur_recrues",
        employer && !reporter->persistentMemberId.empty() ? reporter->persistentMemberId : reporter->name,
        reporter->name + " | score=" + std::to_string(decision.moodScore)
            + " | etat=" + decision.moodLabel
            + " | contexte=" + contextId,
        1
    );

    if (decision.refusesDetailedReport)
    {
        player.recordCanonicalEvent(
            employer ? "team_employer_after_action_reports" : "rapports_combat_recrues_refuses",
            contextId + ":" + reporter->name,
            reporter->name + " refuse ou abrège le compte rendu | humeur=" + decision.moodLabel,
            1
        );
    }
    return decision;
}

RecruitedAllyAftermathKnowledgeReport RecruitedAllyAftermathSystem::buildKnowledgeReport(
    const RecruitedAllyAftermathDecision& decision,
    const std::vector<RecruitedAllyCombatSupport>& participants,
    int simulatedRounds,
    bool teamFinishedFight,
    int defeatedWhileUnconscious,
    int enemiesRemaining)
{
    RecruitedAllyAftermathKnowledgeReport report;
    (void)defeatedWhileUnconscious;
    if (decision.memberName.empty() || participants.empty() || simulatedRounds <= 0) return report;

    const RecruitedAllyCombatSupport* reporter = nullptr;
    for (const RecruitedAllyCombatSupport& ally : participants)
    {
        const bool samePersistentMember = !decision.persistentMemberId.empty()
            && ally.persistentMemberId == decision.persistentMemberId;
        if (samePersistentMember || ally.name == decision.memberName)
        {
            reporter = &ally;
            break;
        }
    }
    if (reporter == nullptr) return report;

    report.directFirstRound = reporter->postKoFirstObservedRound;
    report.directLastRound = reporter->postKoLastObservedRound;
    report.directlyObservedDefeats = std::max(0, reporter->postKoObservedDefeats);
    report.directReserveObservationRounds = std::max(0, reporter->postKoObservedRoundsFromReserve);
    report.observedFromReserve = reporter->postKoObservedFromReserve;
    report.reporterNeverEnteredCombat = reporter->turnsTaken <= 0 && report.observedFromReserve;
    report.knowsFinalOutcomeDirectly = reporter->postKoObservedFinalOutcome;

    if (report.reporterNeverEnteredCombat)
    {
        report.lines.push_back(
            decision.memberName + " n'est jamais entré(e) sur le terrain, mais a assisté directement au combat depuis la file d'attente / le bord de l'arène."
        );
    }
    else if (report.observedFromReserve && report.directReserveObservationRounds > 0)
    {
        report.lines.push_back(
            decision.memberName + " a observé " + std::to_string(report.directReserveObservationRounds)
            + " round(s) depuis la réserve avant ou entre ses phases actives : ces souvenirs restent directs, même sans action de combat."
        );
    }

    if (report.directFirstRound > 0 && report.directLastRound >= report.directFirstRound)
    {
        const std::string roundRange = report.directFirstRound == report.directLastRound
            ? "round " + std::to_string(report.directFirstRound)
            : "rounds " + std::to_string(report.directFirstRound) + " à " + std::to_string(report.directLastRound);
        report.lines.push_back(
            decision.memberName + " confirme directement ce qu'il/elle a vu pendant les " + roundRange
            + " : " + std::to_string(report.directlyObservedDefeats)
            + " ennemi(s) vu(s) tomber pendant sa présence."
        );
    }
    else
    {
        report.lines.push_back(
            decision.memberName + " ne peut confirmer aucun round précis de la phase où tu étais inconscient."
        );
    }

    const int missingBeforeRound = report.directFirstRound > 1 ? report.directFirstRound - 1 : 0;
    if (missingBeforeRound > 0)
    {
        const RecruitedAllyCombatSupport* source = nullptr;
        for (const RecruitedAllyCombatSupport& candidate : participants)
        {
            if (!candidate.postKoAvailableForDebrief || candidate.name == reporter->name) continue;
            if (candidate.postKoFirstObservedRound != 1) continue;
            if (candidate.postKoLastObservedRound < missingBeforeRound) continue;
            if (source == nullptr
                || candidate.postKoFirstObservedRound < source->postKoFirstObservedRound
                || (candidate.postKoFirstObservedRound == source->postKoFirstObservedRound
                    && candidate.postKoLastObservedRound > source->postKoLastObservedRound))
            {
                source = &candidate;
            }
        }

        if (source != nullptr)
        {
            report.usesSecondHandSource = true;
            report.secondHandSourceName = source->name;
            report.lines.push_back(
                "Pour les rounds précédant son entrée, " + decision.memberName
                + " précise que l'information vient de " + source->name
                + " : c'est un témoignage de seconde main, pas un souvenir direct."
            );
        }
        else
        {
            report.hasUnknownGap = true;
            report.lines.push_back(
                "Les rounds précédant l'entrée de " + decision.memberName
                + " restent inconnus dans ce témoignage : aucun survivant crédible ne les lui a transmis."
            );
        }
    }

    if (report.knowsFinalOutcomeDirectly)
    {
        report.lines.push_back(
            decision.memberName + " confirme directement le résultat final : "
            + std::string(teamFinishedFight ? "victoire de l'équipe." : "extraction sans fin complète du combat.")
        );
    }
    else
    {
        const RecruitedAllyCombatSupport* finalWitness = nullptr;
        for (const RecruitedAllyCombatSupport& candidate : participants)
        {
            if (!candidate.postKoAvailableForDebrief || candidate.name == reporter->name) continue;
            if (!candidate.postKoObservedFinalOutcome) continue;
            finalWitness = &candidate;
            break;
        }
        if (finalWitness != nullptr)
        {
            report.usesSecondHandSource = true;
            if (report.secondHandSourceName.empty()) report.secondHandSourceName = finalWitness->name;
            report.lines.push_back(
                "Le résultat final lui est rapporté par " + finalWitness->name + " : "
                + std::string(teamFinishedFight ? "victoire de l'équipe." : "extraction sans fin complète du combat.")
            );
        }
        else
        {
            report.hasUnknownGap = true;
            report.lines.push_back("Le résultat exact de la fin du combat n'est pas confirmé par son propre témoignage.");
        }
    }

    const bool reporterSawEverything = report.directFirstRound == 1
        && report.directLastRound >= simulatedRounds
        && report.knowsFinalOutcomeDirectly;
    if (reporterSawEverything)
    {
        report.lines.push_back(
            "Son témoignage couvre toute la phase hors de ta vue : "
            + std::to_string(report.directlyObservedDefeats) + " ennemi(s) vaincu(s) au total"
            + (enemiesRemaining >= 0 ? ", " + std::to_string(std::max(0, enemiesRemaining)) + " restant(s)." : ".")
        );
    }
    else
    {
        report.lines.push_back(
            "Le moteur connaît le bilan complet, mais il n'est pas injecté dans ce dialogue tant que "
            "le reporter ne l'a pas réellement vu ou reçu d'une source identifiable."
        );
    }

    return report;
}

int RecruitedAllyAftermathSystem::deliverPlayerEconomyShare(
    Player& player,
    int owedEconomyUnits,
    const RecruitedAllyAftermathDecision& decision,
    const std::string& contextId
)
{
    owedEconomyUnits = std::max(0, owedEconomyUnits);
    if (owedEconomyUnits <= 0) return 0;

    if (decision.stealsPlayerShare && !decision.memberName.empty())
    {
        if (decision.teamOrigin == AllyTeamOrigin::EmployerTeam)
        {
            player.recordCanonicalEvent(
                "team_employer_share_thefts",
                contextId + ":" + decision.memberName,
                decision.memberName + " disparaît avec la part qui devait revenir au joueur après sa chute",
                owedEconomyUnits
            );
            player.recordCanonicalEvent(
                "team_employer_roster_history",
                decision.persistentMemberId + ":trahison",
                decision.memberName + " quitte l'escouade après avoir détourné la part du joueur",
                1
            );
            EmployerTeamRosterSystem::setMemberVitals(
                player, decision.persistentMemberId, 1, false, "depart_apres_detournement"
            );
            return 0;
        }
        player.recordCanonicalEvent(
            "parts_joueur_detournees_recrues",
            contextId + ":" + decision.memberName,
            decision.memberName + " quitte le groupe avec la part du joueur après sa chute",
            owedEconomyUnits
        );
        player.recordCanonicalEvent(
            "recrues_trahisons",
            decision.memberName,
            decision.memberName + " profite de l'inconscience du chef pour détourner sa part",
            1
        );
        player.recordCanonicalEvent(
            "recrues_departies",
            decision.memberName,
            decision.memberName + " a quitté le groupe après avoir détourné une part de récompense",
            1
        );
        return 0;
    }

    player.getInventory().earnEconomyUnits(owedEconomyUnits);
    player.refreshCurrencyTitles();
    player.recordCanonicalEvent(
        decision.teamOrigin == AllyTeamOrigin::EmployerTeam ? "team_employer_share_deliveries" : "parts_joueur_remises_recrues",
        contextId + ":" + (decision.memberName.empty() ? std::string("equipe") : decision.memberName),
        (decision.memberName.empty() ? std::string("L'équipe") : decision.memberName)
            + " remet au joueur la part qui lui revient après sa chute",
        owedEconomyUnits
    );
    return owedEconomyUnits;
}
