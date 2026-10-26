#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "quest/QuestSecondPhaseSystem.hpp"
#include "quest/IntercityQuestSystem.hpp"

#include <cassert>
#include <iostream>
#include <string>

namespace
{
    Quest baseQuest(const std::string& id)
    {
        Quest q;
        q.id = id;
        q.rank = "E";
        q.title = "Livraison ordinaire";
        q.origin = "Guilde";
        q.client = "Maître de guilde";
        q.location = "Route commerciale";
        q.objective = "Apporter les fournitures demandées au relais.";
        q.objectiveType = "service";
        q.rewardExperience = 80;
        q.rewardGold = 60;
        q.target = 2;
        q.progress = 2;
        q.guildQuest = true;
        q.accepted = true;
        q.completed = true;
        q.expiresAtDay = 10;
        return q;
    }
}

int main()
{
    Quest mandatory;
    Quest optional;
    bool foundMandatory = false;
    bool foundOptional = false;

    for (int i = 0; i < 2000 && (!foundMandatory || !foundOptional); ++i)
    {
        Quest q = baseQuest("phase2_test_" + std::to_string(i));
        q.objectiveType = "combat";
        q.title = "Nettoyage de route";
        q.objective = "Sécuriser la zone demandée.";
        if (!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(q)) continue;
        assert(q.phaseTwoChecked);
        assert(!q.phaseTwoPreviousSummary.empty());
        assert(q.phaseTwoPreviousSummary.find("Phase 1 terminée") != std::string::npos);
        assert(!q.phaseTwoReason.empty());
        assert(!q.phaseTwoObjective.empty());
        assert(q.phaseTwoRewardExperience > 0);
        if (q.phaseTwoMandatory && !q.phaseTwoTravelRequired && !foundMandatory)
        {
            mandatory = q;
            foundMandatory = true;
        }
        if (!q.phaseTwoMandatory && !q.phaseTwoTravelRequired && !foundOptional)
        {
            optional = q;
            foundOptional = true;
        }
    }

    assert(foundMandatory);
    assert(foundOptional);

    // Service/paperwork follow-ups can stay mundane instead of always becoming a hidden conspiracy.
    bool foundPaperworkFollowUp = false;
    bool foundSmallFavor = false;
    for (int i = 0; i < 5000 && (!foundPaperworkFollowUp || !foundSmallFavor); ++i)
    {
        Quest paperwork = baseQuest("phase2_paperwork_" + std::to_string(i));
        paperwork.title = "Faire signer un formulaire";
        paperwork.objective = "Obtenir la signature du bon guichet.";
        paperwork.targetFamily = "Guilde / papier / registre";
        if (!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(paperwork)) continue;
        if (paperwork.phaseTwoTitle == "Le mauvais guichet" || paperwork.phaseTwoTitle == "Signature obtenue, remise ailleurs")
        {
            foundPaperworkFollowUp = true;
            assert(paperwork.phaseTwoMandatory);
            assert(paperwork.phaseTwoReason.find("document") != std::string::npos || paperwork.phaseTwoReason.find("signature") != std::string::npos);
        }
        if (paperwork.phaseTwoTitle == "Avant de partir...")
        {
            foundSmallFavor = true;
            assert(!paperwork.phaseTwoMandatory);
        }
    }
    assert(foundPaperworkFollowUp);
    assert(foundSmallFavor);

    // Phase-2 templates now reflect the real quest family instead of falling back to the same generic surprise.
    auto findTypedFollowUp = [](const std::string& type, const std::string& title, const std::string& objective, const std::string& family) {
        Quest found;
        for (int i = 0; i < 10000; ++i)
        {
            Quest candidate = baseQuest("phase2_typed_" + type + "_" + std::to_string(i));
            candidate.objectiveType = type;
            candidate.title = title;
            candidate.objective = objective;
            candidate.targetFamily = family;
            if (!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(candidate)) continue;
            return candidate;
        }
        assert(false && "No deterministic phase-2 candidate found for quest type");
        return found;
    };

    const Quest huntFollowUp = findTypedFollowUp("combat", "Chasse de route", "Éliminer les créatures signalées.", "loups / chasse");
    assert(
        huntFollowUp.phaseTwoTitle == "Confirmer que la zone est sûre"
        || huntFollowUp.phaseTwoTitle == "Le témoin du combat"
        || huntFollowUp.phaseTwoTitle == "Traces après l'affrontement"
        || huntFollowUp.phaseTwoTitle == "Sécuriser le passage"
    );

    const Quest gatherFollowUp = findTypedFollowUp("material", "Récolte locale", "Réunir les matériaux demandés.", "plantes / ressources");
    assert(
        gatherFollowUp.phaseTwoTitle == "Échantillon à vérifier"
        || gatherFollowUp.phaseTwoTitle == "Un dernier prélèvement"
        || gatherFollowUp.phaseTwoTitle == "La bonne provenance"
        || gatherFollowUp.phaseTwoTitle == "Zone déjà exploitée"
    );

    const Quest investigationFollowUp = findTypedFollowUp("enquête / exploration", "Vérification de témoignages", "Recouper les informations du quartier.", "témoins / dossier");
    assert(
        investigationFollowUp.phaseTwoTitle == "Recouper le témoignage"
        || investigationFollowUp.phaseTwoTitle == "Deux versions, un lieu"
        || investigationFollowUp.phaseTwoTitle == "La piste secondaire"
        || investigationFollowUp.phaseTwoTitle == "La pièce manquante"
    );

    const Quest explorationFollowUp = findTypedFollowUp("exploration", "Reconnaissance locale", "Explorer la zone indiquée.", "terrain / repères");
    assert(
        explorationFollowUp.phaseTwoTitle == "Cartographier la sortie"
        || explorationFollowUp.phaseTwoTitle == "Le repère oublié"
        || explorationFollowUp.phaseTwoTitle == "Comparer les traces"
        || explorationFollowUp.phaseTwoTitle == "Dernier point de reconnaissance"
    );

    const Quest rescueFollowUp = findTypedFollowUp("exploration", "Sauvetage d'un voyageur", "Retrouver et secourir la personne disparue.", "personne disparue / secours");
    assert(
        rescueFollowUp.phaseTwoTitle == "Un dernier service pour la personne aidée"
        || rescueFollowUp.phaseTwoTitle == "Vérifier le chemin du retour"
        || rescueFollowUp.phaseTwoTitle == "Le message après le sauvetage"
    );

    const auto structuredJournal = QuestSecondPhaseSystem::journalLines(investigationFollowUp);
    bool hasType = false;
    bool hasDifficulty = false;
    bool hasKnownRisk = false;
    for (const std::string& line : structuredJournal)
    {
        hasType = hasType || line.find("Type de suite :") != std::string::npos;
        hasDifficulty = hasDifficulty || line.find("Difficulté estimée :") != std::string::npos;
        hasKnownRisk = hasKnownRisk || line.find("Risques connus :") != std::string::npos;
    }
    assert(hasType);
    assert(hasDifficulty);
    assert(hasKnownRisk);

    // Rank F remains local even when the same paperwork template asks for another hand-off.
    bool checkedRankFLocal = false;
    for (int i = 0; i < 6000 && !checkedRankFLocal; ++i)
    {
        Quest beginner = baseQuest("phase2_rank_f_" + std::to_string(i));
        beginner.rank = "F";
        beginner.title = "Faire signer un formulaire";
        beginner.objective = "Faire signer le registre du comptoir.";
        beginner.targetFamily = "Guilde / papier / registre";
        if (!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(beginner)) continue;
        if (beginner.phaseTwoTitle != "Signature obtenue, remise ailleurs"
            && beginner.phaseTwoTitle != "Transmission complémentaire"
            && beginner.phaseTwoTitle != "Le dernier paquet") continue;
        assert(!beginner.phaseTwoTravelRequired);
        checkedRankFLocal = true;
    }
    assert(checkedRankFLocal);

    // A document that must be handed elsewhere becomes a real city-to-city follow-up, not a fake dice roll.
    Quest travelFollowUp;
    bool foundTravelFollowUp = false;
    for (int i = 0; i < 6000 && !foundTravelFollowUp; ++i)
    {
        Quest paperwork = baseQuest("phase2_travel_paper_" + std::to_string(i));
        paperwork.title = "Faire signer un formulaire";
        paperwork.objective = "Faire signer le registre du comptoir.";
        paperwork.targetFamily = "Guilde / papier / registre";
        if (!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(paperwork)) continue;
        if (!paperwork.phaseTwoTravelRequired) continue;
        travelFollowUp = paperwork;
        foundTravelFollowUp = true;
    }
    assert(foundTravelFollowUp);
    Player courier;
    courier.setLoadedProgress(10, 0, 100);
    courier.grantTitle("Aventurier");
    courier.setCurrentCityId("valebrume");
    if (QuestSecondPhaseSystem::canOfferOptional(travelFollowUp))
    {
        assert(QuestSecondPhaseSystem::activateOptional(travelFollowUp));
    }
    assert(travelFollowUp.phaseTwoActive);
    std::vector<std::string> travelPrep;
    assert(IntercityQuestSystem::prepareSecondPhaseTravel(courier, travelFollowUp, &travelPrep));
    assert(!travelFollowUp.phaseTwoTravelDestinationCityId.empty());
    const std::string destination = travelFollowUp.phaseTwoTravelDestinationCityId;
    courier.getQuestLog().getQuests().push_back(travelFollowUp);
    const auto travelResult = IntercityQuestSystem::onCityArrival(courier, "valebrume", destination);
    assert(!travelResult.empty());
    const Quest& completedTravelFollowUp = courier.getQuestLog().getQuests().back();
    assert(completedTravelFollowUp.phaseTwoCompleted);
    assert(completedTravelFollowUp.completed);

    // Mandatory phase blocks hand-in until resolved and extends the deadline once.
    assert(mandatory.phaseTwoActive);
    assert(!mandatory.completed);
    assert(!mandatory.phaseTwoOffered);
    assert(mandatory.expiresAtDay == 12);
    const int deadlineAfterReveal = mandatory.expiresAtDay;
    assert(!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(mandatory));
    assert(mandatory.expiresAtDay == deadlineAfterReveal);

    Player player;
    player.setLoadedProgress(30, 0, 0);
    const int dayBefore = player.getWorldDaysElapsed();
    for (int i = 0; i < 12 && !mandatory.phaseTwoCompleted; ++i)
    {
        const auto result = QuestSecondPhaseSystem::resolve(player, mandatory, true);
        assert(result.attempted);
        assert(result.timeUnits == 2);
    }
    assert(mandatory.phaseTwoCompleted);
    assert(mandatory.completed);
    assert(!mandatory.phaseTwoActive);
    assert(player.getWorldDaysElapsed() >= dayBefore);

    // Optional phase leaves the original quest hand-in ready until the player explicitly accepts it.
    assert(optional.completed);
    assert(optional.phaseTwoOffered);
    assert(QuestSecondPhaseSystem::canOfferOptional(optional));

    Quest declined = optional;
    assert(QuestSecondPhaseSystem::declineOptional(declined));
    assert(declined.phaseTwoDeclined);
    assert(declined.completed);
    assert(!declined.phaseTwoActive);

    assert(QuestSecondPhaseSystem::activateOptional(optional));
    assert(optional.phaseTwoActive);
    assert(!optional.completed);
    for (int i = 0; i < 12 && !optional.phaseTwoCompleted; ++i)
    {
        QuestSecondPhaseSystem::resolve(player, optional, true);
    }
    assert(optional.phaseTwoCompleted);
    assert(optional.completed);


    // Optional local follow-ups can deliberately pay in social/information value rather than cash.
    Quest nonMoney;
    bool foundNonMoney = false;
    for (int i = 0; i < 8000 && !foundNonMoney; ++i)
    {
        Quest candidate = baseQuest("phase2_non_money_" + std::to_string(i));
        candidate.title = "Petit service de quartier";
        candidate.objective = "Aider le client à terminer une démarche locale.";
        candidate.targetFamily = "Habitants / service local";
        if (!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(candidate)) continue;
        if (candidate.phaseTwoMandatory || candidate.phaseTwoTravelRequired) continue;
        if (candidate.phaseTwoRewardKind.empty() || candidate.phaseTwoRewardKind == "money") continue;
        assert(candidate.phaseTwoRewardGold == 0);
        assert(!candidate.phaseTwoRewardDetail.empty());
        nonMoney = candidate;
        foundNonMoney = true;
    }
    assert(foundNonMoney);
    Player socialPlayer;
    socialPlayer.setCurrentCityId("valebrume");
    nonMoney.phaseTwoCompleted = true;
    nonMoney.phaseTwoActive = false;
    nonMoney.completed = true;
    std::vector<std::string> nonMoneyLines;
    assert(QuestSecondPhaseSystem::applyNonMonetaryReward(socialPlayer, nonMoney, &nonMoneyLines));
    assert(!nonMoneyLines.empty());
    assert(!QuestSecondPhaseSystem::applyNonMonetaryReward(socialPlayer, nonMoney, &nonMoneyLines));

    // Story, bounty and special incident missions do not unexpectedly sprout a generic second phase.
    Quest story = baseQuest("story_ch_test");
    story.origin = "Quête principale";
    assert(!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(story));
    Quest bounty = baseQuest("bounty_test");
    bounty.bountyMission = true;
    assert(!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(bounty));
    Quest incident = baseQuest("incident_test");
    incident.tradeIncidentMission = true;
    assert(!QuestSecondPhaseSystem::revealAfterPrimaryCompletion(incident));

    std::cout << "QuestSecondPhaseSystemTest OK\n";
    return 0;
}
