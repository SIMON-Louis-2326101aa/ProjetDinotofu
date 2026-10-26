#include "quest/QuestSecondPhaseSystem.hpp"

#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cstdint>

namespace
{
    std::uint32_t stableHash(const std::string& text)
    {
        std::uint32_t hash = 2166136261u;
        for (unsigned char c : text)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }


    std::string nonMonetaryRewardLabel(const std::string& kind)
    {
        if (kind == "reputation") return "réputation locale";
        if (kind == "relationship") return "relation avec le commanditaire";
        if (kind == "information") return "information vérifiée";
        if (kind == "opportunity") return "opportunité future";
        return "argent contractuel";
    }

    void configureRewardProfile(Quest& quest, std::uint32_t h)
    {
        quest.phaseTwoRewardKind = "money";
        quest.phaseTwoRewardDetail.clear();
        quest.phaseTwoRewardExperience = std::max(
            4,
            quest.rewardExperience > 0
                ? quest.rewardExperience * (quest.phaseTwoMandatory ? 28 : 20) / 100
                : 4
        );
        quest.phaseTwoRewardGold = std::max(0, quest.rewardGold * (quest.phaseTwoMandatory ? 25 : 18) / 100);

        // Mandatory or inter-city work stays contractually paid. The non-monetary variants are
        // reserved for optional local follow-ups so accepting them feels like a role-play choice,
        // not an economic trap or a free travel pass.
        if (quest.phaseTwoMandatory || quest.phaseTwoTravelRequired)
        {
            return;
        }

        const int selector = static_cast<int>((h / 100000u) % 5u);
        switch (selector)
        {
            case 0:
                quest.phaseTwoRewardKind = "reputation";
                quest.phaseTwoRewardDetail = "Le client et le quartier retiennent surtout que tu as pris le temps d'aller au bout du service.";
                break;
            case 1:
                quest.phaseTwoRewardKind = "relationship";
                quest.phaseTwoRewardDetail = "Le bénéfice principal est la confiance gagnée auprès du commanditaire, pas une deuxième enveloppe.";
                break;
            case 2:
                quest.phaseTwoRewardKind = "information";
                quest.phaseTwoRewardDetail = "La suite permet de repartir avec une information recoupée et utile plutôt qu'avec davantage d'argent.";
                break;
            case 3:
                quest.phaseTwoRewardKind = "opportunity";
                quest.phaseTwoRewardDetail = "Le service ouvre une possibilité future mémorisée, sans créer immédiatement une nouvelle quête.";
                break;
            default:
                return;
        }
        quest.phaseTwoRewardGold = 0;
    }

    bool isEligible(const Quest& quest)
    {
        if (!quest.accepted || quest.failed || quest.turnedIn) return false;
        if (quest.phaseTwoChecked || quest.phaseTwoActive || quest.phaseTwoCompleted) return false;
        if (quest.bountyMission || quest.guildChallenge || quest.tradeIncidentMission) return false;
        if (quest.origin == "Quête principale" || quest.id.rfind("story_ch", 0) == 0) return false;
        // Employer orders are contractual service tasks with their own lifecycle/history.
        // A generic surprise follow-up must never block their synchronization or payroll flow.
        if (quest.id.rfind("team_job_", 0) == 0 || quest.origin.rfind("Équipe employeuse", 0) == 0) return false;
        return !quest.id.empty();
    }

    std::string primarySummary(const Quest& quest)
    {
        std::string result = "Phase 1 terminée : " + quest.title + " — " + quest.objective;
        if (quest.target > 0)
        {
            result += " (" + std::to_string(std::max(0, quest.target)) + "/" + std::to_string(std::max(1, quest.target)) + ").";
        }
        else
        {
            result += ".";
        }
        return result;
    }

    bool objectiveContains(const Quest& quest, const std::string& needle)
    {
        return quest.objectiveType.find(needle) != std::string::npos;
    }

    bool isServiceLike(const Quest& quest)
    {
        return quest.objectiveType == "service"
            || quest.objectiveType == "livraison"
            || quest.objectiveType == "intercity_information"
            || quest.objectiveType == "intercity_material"
            || objectiveContains(quest, "logistique");
    }

    bool isInvestigationLike(const Quest& quest)
    {
        return objectiveContains(quest, "enquête") || objectiveContains(quest, "verification")
            || objectiveContains(quest, "vérification");
    }

    bool isGatherLike(const Quest& quest)
    {
        return quest.objectiveType == "material" || objectiveContains(quest, "récolte");
    }

    bool isHuntLike(const Quest& quest)
    {
        return quest.objectiveType == "combat" || quest.objectiveType == "bestiaire";
    }

    bool isExplorationLike(const Quest& quest)
    {
        return quest.objectiveType == "exploration" || objectiveContains(quest, "exploration");
    }

    std::string secondPhaseTypeLabel(const Quest& quest)
    {
        if (isServiceLike(quest)) return "service / livraison";
        if (isInvestigationLike(quest)) return "enquête / recoupement";
        if (isGatherLike(quest)) return "récolte / vérification de ressource";
        if (isHuntLike(quest)) return "chasse / sécurisation";
        if (isExplorationLike(quest)) return "exploration / reconnaissance";
        return "suite de terrain";
    }

    std::string secondPhaseDifficultyLabel(const Quest& quest)
    {
        int score = 0;
        if (quest.phaseTwoMandatory) score += 1;
        if (quest.phaseTwoTravelRequired) score += 2;
        if (quest.phaseTwoTarget > 1) score += 1;
        if (isHuntLike(quest) || isInvestigationLike(quest)) score += 1;

        if (score <= 1) return "faible à modérée";
        if (score <= 3) return "modérée";
        return "soutenue";
    }

    std::string secondPhaseKnownRisk(const Quest& quest)
    {
        if (quest.phaseTwoTravelRequired)
        {
            return "déplacement réel, retard possible, relais indisponible ou changement de contact ; les détours personnels restent à ta charge";
        }
        if (isInvestigationLike(quest))
        {
            return "témoignage incomplet, piste trompeuse ou temps perdu ; aucune information non recoupée n'est traitée comme une preuve";
        }
        if (isGatherLike(quest))
        {
            return "échantillon inutilisable, zone déjà exploitée ou vérification supplémentaire ; aucun danger précis n'est garanti";
        }
        if (isHuntLike(quest))
        {
            return "nouveau contact hostile ou zone encore instable ; la présence exacte d'un ennemi n'est pas révélée à l'avance";
        }
        if (isExplorationLike(quest))
        {
            return "détour, mauvaise lecture du terrain ou découverte incomplète ; les dangers inconnus restent inconnus";
        }
        if (isServiceLike(quest))
        {
            return "retard, mauvais interlocuteur ou étape administrative supplémentaire ; pas de catastrophe artificielle obligatoire";
        }
        return "temps supplémentaire et résultat incertain ; le journal n'annonce que les risques déjà connus du personnage";
    }

    bool forceOptionalTitle(const std::string& title)
    {
        return title == "Avant de partir..."
            || title == "Un dernier service pour la personne aidée"
            || title == "Le témoin du combat"
            || title == "Un dernier prélèvement"
            || title == "La piste secondaire"
            || title == "Le repère oublié";
    }

    bool forceMandatoryTitle(const std::string& title)
    {
        return title == "Le mauvais guichet"
            || title == "Signature obtenue, remise ailleurs"
            || title == "Confirmer que la zone est sûre"
            || title == "Échantillon à vérifier"
            || title == "Recouper le témoignage";
    }

    void buildTemplate(Quest& quest, std::uint32_t h)
    {
        const std::string combined = quest.title + " " + quest.objective + " " + quest.targetFamily;
        const bool paperworkLike = combined.find("papier") != std::string::npos
            || combined.find("formulaire") != std::string::npos
            || combined.find("reçu") != std::string::npos
            || combined.find("registre") != std::string::npos
            || combined.find("signature") != std::string::npos
            || combined.find("dossier") != std::string::npos;
        const bool rescueLike = combined.find("sauv") != std::string::npos
            || combined.find("secour") != std::string::npos
            || combined.find("disparu") != std::string::npos
            || combined.find("retrouver") != std::string::npos;

        if (isServiceLike(quest))
        {
            const int serviceSelector = static_cast<int>((h / 100u) % 5u);
            if (paperworkLike && serviceSelector <= 1)
            {
                quest.phaseTwoTitle = serviceSelector == 0 ? "Le mauvais guichet" : "Signature obtenue, remise ailleurs";
                quest.phaseTwoReason = serviceSelector == 0
                    ? "Le document est bien rempli et signé, mais le guichet confirme qu'il devait finalement être validé auprès d'un autre service."
                    : "La signature règle la première demande, puis le contact précise que le document signé doit encore être remis à son destinataire final.";
                quest.phaseTwoObjective = serviceSelector == 0
                    ? "Faire corriger l'acheminement administratif et obtenir le bon tampon sans recommencer inutilement tout le dossier."
                    : "Acheminer le document signé au bon contact et obtenir l'accusé de réception final.";
                return;
            }

            if (serviceSelector == 2)
            {
                quest.phaseTwoTitle = "Avant de partir...";
                quest.phaseTwoReason = "Le service demandé est bien terminé, mais le client profite de ta présence pour demander un dernier coup de main lié à la même affaire.";
                quest.phaseTwoObjective = "Choisir si tu rends ce service complémentaire avant de clôturer définitivement l'affaire.";
                return;
            }
            if (serviceSelector == 3)
            {
                quest.phaseTwoTitle = "Transmission complémentaire";
                quest.phaseTwoReason = "La première remise est validée, mais l'information obtenue doit maintenant être transmise proprement à un autre contact avant de devenir vraiment utile.";
                quest.phaseTwoObjective = "Faire parvenir l'information au bon relais sans la transformer en rumeur imprécise.";
                return;
            }

            quest.phaseTwoTitle = "Le dernier paquet";
            quest.phaseTwoReason = "La livraison principale est terminée, mais un petit lot lié au même dossier n'a pas suivi le bon chargement.";
            quest.phaseTwoObjective = "Remettre le lot complémentaire au contact prévu, contre rémunération supplémentaire.";
            return;
        }

        if (rescueLike)
        {
            const int rescueSelector = static_cast<int>((h / 100u) % 3u);
            if (rescueSelector == 0)
            {
                quest.phaseTwoTitle = "Un dernier service pour la personne aidée";
                quest.phaseTwoReason = "La personne est maintenant hors de danger, mais elle se souvient d'un proche, d'un objet ou d'un message resté derrière elle.";
                quest.phaseTwoObjective = "Décider s'il faut revenir sur place ou transmettre ce dernier service avant de classer l'affaire.";
                return;
            }
            if (rescueSelector == 1)
            {
                quest.phaseTwoTitle = "Vérifier le chemin du retour";
                quest.phaseTwoReason = "Le sauvetage est réussi, mais l'itinéraire utilisé reste assez mauvais pour mettre en danger la prochaine personne qui l'empruntera.";
                quest.phaseTwoObjective = "Repasser sur une courte portion du trajet et signaler les dangers réellement observés.";
                return;
            }

            quest.phaseTwoTitle = "Le message après le sauvetage";
            quest.phaseTwoReason = "La personne aidée veut prévenir quelqu'un de confiance avant que les rumeurs locales ne déforment ce qui s'est réellement passé.";
            quest.phaseTwoObjective = "Transmettre un compte rendu bref et fidèle au contact indiqué.";
            return;
        }

        if (isInvestigationLike(quest))
        {
            const int selector = static_cast<int>((h / 100u) % 4u);
            if (selector == 0)
            {
                quest.phaseTwoTitle = "Recouper le témoignage";
                quest.phaseTwoReason = "La première piste tient, mais une affirmation importante ne repose encore que sur une seule source.";
                quest.phaseTwoObjective = "Trouver un second élément indépendant avant de considérer l'information comme fiable.";
            }
            else if (selector == 1)
            {
                quest.phaseTwoTitle = "Deux versions, un lieu";
                quest.phaseTwoReason = "Deux témoins décrivent le même endroit différemment sans qu'il soit encore possible de savoir lequel se trompe.";
                quest.phaseTwoObjective = "Retourner vérifier uniquement les détails observables et séparer faits, erreurs et suppositions.";
            }
            else if (selector == 2)
            {
                quest.phaseTwoTitle = "La piste secondaire";
                quest.phaseTwoReason = "La demande principale est résolue, mais une piste moins urgente reste exploitable si tu veux comprendre davantage l'affaire.";
                quest.phaseTwoObjective = "Suivre cette piste facultative sans retarder artificiellement la clôture du contrat principal.";
            }
            else
            {
                quest.phaseTwoTitle = "La pièce manquante";
                quest.phaseTwoReason = "Le dossier est presque cohérent, sauf un détail matériel ou chronologique qui ne correspond pas encore au reste.";
                quest.phaseTwoObjective = "Vérifier ce détail précis et consigner seulement ce qui peut être confirmé.";
            }
            return;
        }

        if (isGatherLike(quest))
        {
            const int selector = static_cast<int>((h / 100u) % 4u);
            if (selector == 0)
            {
                quest.phaseTwoTitle = "Échantillon à vérifier";
                quest.phaseTwoReason = "La quantité demandée est réunie, mais un des lots présente une qualité ou une origine douteuse.";
                quest.phaseTwoObjective = "Vérifier l'échantillon avant remise pour éviter de livrer une ressource inutilisable.";
            }
            else if (selector == 1)
            {
                quest.phaseTwoTitle = "Un dernier prélèvement";
                quest.phaseTwoReason = "Le commanditaire a déjà ce qu'il demandait, mais un petit prélèvement supplémentaire permettrait de comparer deux zones.";
                quest.phaseTwoObjective = "Choisir si tu réalises ce prélèvement facultatif avant de rentrer.";
            }
            else if (selector == 2)
            {
                quest.phaseTwoTitle = "La bonne provenance";
                quest.phaseTwoReason = "Les ressources sont suffisantes, mais leur provenance exacte compte pour éviter de mélanger deux qualités différentes.";
                quest.phaseTwoObjective = "Confirmer l'origine du lot et écarter ce qui ne correspond pas à la demande.";
            }
            else
            {
                quest.phaseTwoTitle = "Zone déjà exploitée";
                quest.phaseTwoReason = "La collecte est terminée, mais les traces montrent que la zone conseillée a été fortement exploitée récemment.";
                quest.phaseTwoObjective = "Signaler l'état réel de la zone et vérifier s'il faut conseiller un autre secteur pour les prochaines demandes.";
            }
            return;
        }

        if (isHuntLike(quest))
        {
            const int selector = static_cast<int>((h / 100u) % 4u);
            if (selector == 0)
            {
                quest.phaseTwoTitle = "Confirmer que la zone est sûre";
                quest.phaseTwoReason = "La cible demandée est vaincue, mais des traces fraîches empêchent encore la guilde de déclarer le secteur totalement sûr.";
                quest.phaseTwoObjective = "Effectuer une vérification courte et confirmer si la menace immédiate est réellement terminée.";
            }
            else if (selector == 1)
            {
                quest.phaseTwoTitle = "Le témoin du combat";
                quest.phaseTwoReason = "Un témoin arrive après l'affrontement et pense avoir vu la même créature ou le même groupe plus loin.";
                quest.phaseTwoObjective = "Choisir si tu vérifies ce témoignage facultatif sans traiter sa parole comme une certitude.";
            }
            else if (selector == 2)
            {
                quest.phaseTwoTitle = "Traces après l'affrontement";
                quest.phaseTwoReason = "Une fois le combat terminé, des traces indiquent qu'une partie du danger a peut-être quitté la zone avant ton arrivée.";
                quest.phaseTwoObjective = "Lire les traces restantes et déterminer si une action immédiate est encore nécessaire.";
            }
            else
            {
                quest.phaseTwoTitle = "Sécuriser le passage";
                quest.phaseTwoReason = "Le contrat de chasse est réussi, mais le passage utilisé par les voyageurs reste encombré ou mal signalé après le combat.";
                quest.phaseTwoObjective = "Rendre le passage praticable et transmettre un état fiable à la guilde.";
            }
            return;
        }

        if (isExplorationLike(quest))
        {
            const int selector = static_cast<int>((h / 100u) % 4u);
            if (selector == 0)
            {
                quest.phaseTwoTitle = "Cartographier la sortie";
                quest.phaseTwoReason = "La zone demandée est explorée, mais le chemin de sortie le plus sûr n'a pas encore été noté correctement.";
                quest.phaseTwoObjective = "Compléter une courte reconnaissance et laisser un repère exploitable pour les prochains passages.";
            }
            else if (selector == 1)
            {
                quest.phaseTwoTitle = "Le repère oublié";
                quest.phaseTwoReason = "La mission est réussie, mais un ancien repère mentionné dans les notes reste à proximité et n'est pas nécessaire à la validation principale.";
                quest.phaseTwoObjective = "Choisir si tu vérifies ce repère facultatif avant de quitter la zone.";
            }
            else if (selector == 2)
            {
                quest.phaseTwoTitle = "Comparer les traces";
                quest.phaseTwoReason = "Deux traces découvertes pendant l'exploration semblent similaires sans être assez nettes pour conclure.";
                quest.phaseTwoObjective = "Comparer les indices de terrain et enregistrer seulement ce qui peut être réellement distingué.";
            }
            else
            {
                quest.phaseTwoTitle = "Dernier point de reconnaissance";
                quest.phaseTwoReason = "L'objectif principal est rempli, mais un angle mort proche limite encore la qualité du compte rendu.";
                quest.phaseTwoObjective = "Observer ce dernier secteur sans transformer la mission en exploration infinie.";
            }
            return;
        }

        const int selector = static_cast<int>((h / 100u) % 5u);
        switch (selector)
        {
            case 0:
                quest.phaseTwoTitle = "Conséquence imprévue";
                quest.phaseTwoReason = "En terminant l'objectif, tu découvres que le problème s'est déplacé au lieu de disparaître complètement.";
                quest.phaseTwoObjective = "Vérifier la conséquence immédiate et sécuriser ce qui peut encore l'être.";
                break;
            case 1:
                quest.phaseTwoTitle = "Un témoin tardif";
                quest.phaseTwoReason = "Un témoin se manifeste seulement après la résolution de la première demande et relie l'affaire à un détail passé inaperçu.";
                quest.phaseTwoObjective = "Retrouver le témoin, vérifier son récit et décider quoi faire de la nouvelle piste.";
                break;
            case 2:
                quest.phaseTwoTitle = "La piste derrière la piste";
                quest.phaseTwoReason = "Un objet, une trace ou un comportement observé pendant la phase 1 révèle un lien que le client ne connaissait pas.";
                quest.phaseTwoObjective = "Suivre ce nouveau lien sans considérer la première réussite comme annulée.";
                break;
            case 3:
                quest.phaseTwoTitle = "Retour de situation";
                quest.phaseTwoReason = "La résolution provoque une réaction rapide : quelqu'un tente de récupérer, cacher ou déplacer ce qui vient d'être découvert.";
                quest.phaseTwoObjective = "Intervenir avant que la nouvelle situation ne se stabilise hors de portée.";
                break;
            default:
                quest.phaseTwoTitle = "Dernière vérification";
                quest.phaseTwoReason = "La phase 1 est bien réussie, mais une incohérence constatée au dernier moment mérite une vérification supplémentaire.";
                quest.phaseTwoObjective = "Faire une vérification de terrain et confirmer si l'affaire peut vraiment être classée.";
                break;
        }
    }

}

bool QuestSecondPhaseSystem::revealAfterPrimaryCompletion(Quest& quest)
{
    if (!isEligible(quest)) return false;

    quest.phaseTwoChecked = true;
    const std::uint32_t h = stableHash(quest.id + "|" + quest.title + "|phase2");
    int chance = quest.guildQuest ? 20 : 16;
    if (quest.objectiveType == "exploration" || quest.objectiveType == "service") chance += 4;
    if (static_cast<int>(h % 100u) >= chance) return false;

    quest.phaseTwoPreviousSummary = primarySummary(quest);
    buildTemplate(quest, h);
    quest.phaseTwoMandatory = ((h / 10u) % 100u) < 46u;
    // Ordinary favors are usually optional; contractual corrections and safety checks can remain mandatory.
    if (forceOptionalTitle(quest.phaseTwoTitle))
    {
        quest.phaseTwoMandatory = false;
    }
    else if (forceMandatoryTitle(quest.phaseTwoTitle))
    {
        quest.phaseTwoMandatory = true;
    }
    quest.phaseTwoTarget = 1 + static_cast<int>((h / 1000u) % 2u);
    quest.phaseTwoProgress = 0;
    // Rank F stays local by design: even a paperwork/service follow-up is resolved inside the
    // current town instead of unexpectedly turning a beginner errand into inter-city travel.
    quest.phaseTwoTravelRequired = quest.rank != "F"
        && (quest.phaseTwoTitle == "Signature obtenue, remise ailleurs"
            || quest.phaseTwoTitle == "Transmission complémentaire"
            || quest.phaseTwoTitle == "Le dernier paquet");
    configureRewardProfile(quest, h);
    quest.phaseTwoTravelSourceCityId.clear();
    quest.phaseTwoTravelDestinationCityId.clear();
    quest.phaseTwoTravelDistanceKm = 0;
    quest.phaseTwoOffered = !quest.phaseTwoMandatory;
    quest.phaseTwoActive = quest.phaseTwoMandatory;
    quest.phaseTwoDeclined = false;
    quest.phaseTwoCompleted = false;

    // Mandatory complications are part of the same contract and must be solved before hand-in.
    if (quest.phaseTwoMandatory)
    {
        quest.completed = false;
        if (quest.expiresAtDay >= 0) quest.expiresAtDay += 2;
    }
    else if (quest.expiresAtDay >= 0)
    {
        // Optional follow-ups get a small grace period only if the player elects to pursue them.
        // The extra day is already visible in the journal and never stacks on repeated menu opens.
        quest.expiresAtDay += 1;
    }

    return true;
}

bool QuestSecondPhaseSystem::canOfferOptional(const Quest& quest)
{
    return quest.phaseTwoOffered && !quest.phaseTwoMandatory && !quest.phaseTwoActive
        && !quest.phaseTwoCompleted && !quest.phaseTwoDeclined && quest.completed && !quest.turnedIn && !quest.failed;
}

bool QuestSecondPhaseSystem::activateOptional(Quest& quest)
{
    if (!canOfferOptional(quest)) return false;
    quest.phaseTwoOffered = false;
    quest.phaseTwoActive = true;
    quest.completed = false;
    return true;
}

bool QuestSecondPhaseSystem::declineOptional(Quest& quest)
{
    if (!canOfferOptional(quest)) return false;
    quest.phaseTwoOffered = false;
    quest.phaseTwoDeclined = true;
    quest.completed = true;
    return true;
}

QuestSecondPhaseResult QuestSecondPhaseSystem::resolve(Player& player, Quest& quest, bool cautiousApproach)
{
    QuestSecondPhaseResult result;
    if (!quest.phaseTwoActive || quest.phaseTwoCompleted || quest.failed || quest.turnedIn)
    {
        result.lines.push_back("Aucune phase 2 active ne peut être résolue sur cette quête.");
        return result;
    }
    if (quest.phaseTwoTravelRequired)
    {
        result.lines.push_back("Cette phase 2 demande un vrai déplacement inter-ville : utilise le voyage vers la destination indiquée au journal.");
        return result;
    }

    result.attempted = true;
    result.timeUnits = cautiousApproach ? 2 : 1;
    player.advanceWorldDayUnits(result.timeUnits);
    ++quest.phaseTwoAttempts;

    int chance = 67 + std::min(14, player.getLevel() / 8);
    chance += cautiousApproach ? 14 : -5;
    chance -= std::max(0, quest.phaseTwoTarget - 1) * 4;
    result.chancePercent = std::clamp(chance, 38, 92);

    const std::uint32_t h = stableHash(
        quest.id + "|phase2_attempt=" + std::to_string(quest.phaseTwoAttempts)
        + "|day=" + std::to_string(player.getWorldDaysElapsed())
        + (cautiousApproach ? "|careful" : "|quick"));
    result.success = static_cast<int>(h % 100u) < result.chancePercent;

    result.lines.push_back(cautiousApproach
        ? "Approche prudente : tu prends le temps de vérifier la nouvelle piste avant d'agir."
        : "Approche rapide : tu essaies de régler la complication avant qu'elle ne grossisse, avec moins de vérifications.");
    result.lines.push_back("Chance de réussite : " + std::to_string(result.chancePercent) + "%.");

    if (!result.success)
    {
        result.lines.push_back("La tentative n'aboutit pas. Le temps est réellement consommé et la phase 2 reste active.");
        return result;
    }

    quest.phaseTwoProgress = std::min(quest.phaseTwoTarget, quest.phaseTwoProgress + 1);
    result.lines.push_back("Progression de la phase 2 : " + std::to_string(quest.phaseTwoProgress)
        + "/" + std::to_string(quest.phaseTwoTarget) + ".");

    if (quest.phaseTwoProgress >= quest.phaseTwoTarget)
    {
        quest.phaseTwoActive = false;
        quest.phaseTwoCompleted = true;
        quest.completed = true;
        result.completed = true;
        player.recordCanonicalEvent(
            "suites_quete_terminees",
            quest.phaseTwoMandatory ? "mandatory" : "optional",
            quest.phaseTwoTitle.empty() ? quest.title : quest.phaseTwoTitle,
            1
        );
        result.lines.push_back("Phase 2 terminée : la quête peut maintenant être finalisée avec sa récompense augmentée.");
    }
    return result;
}

bool QuestSecondPhaseSystem::applyNonMonetaryReward(Player& player, const Quest& quest, std::vector<std::string>* lines)
{
    if (!quest.phaseTwoCompleted || quest.phaseTwoRewardKind.empty() || quest.phaseTwoRewardKind == "money")
    {
        return false;
    }

    const std::string rewardKey = quest.id + ":" + quest.phaseTwoRewardKind;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == "phase2_non_monetary_rewards" && record.key == rewardKey)
        {
            return false;
        }
    }

    player.recordCanonicalEvent(
        "phase2_non_monetary_rewards",
        rewardKey,
        "Récompense de phase 2 : " + nonMonetaryRewardLabel(quest.phaseTwoRewardKind),
        1
    );

    if (quest.phaseTwoRewardKind == "reputation")
    {
        player.recordCanonicalEvent("reputation_locale_positive", "phase2:" + quest.id, quest.phaseTwoTitle, 1);
        player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
        if (lines) lines->push_back("Suite facultative : la récompense principale est de la réputation locale. Note de réputation locale x1.");
    }
    else if (quest.phaseTwoRewardKind == "relationship")
    {
        const std::string client = quest.client.empty() ? std::string("Contact du contrat") : quest.client;
        player.rememberNpcFact(
            client,
            "phase2_extra_help",
            quest.id,
            "Le joueur a accepté et terminé un service complémentaire facultatif : " + quest.phaseTwoTitle,
            "interaction_directe",
            player.getName(),
            95,
            3
        );
        player.recordCanonicalEvent("relations_pnj_positives", client + ":" + quest.id, quest.phaseTwoTitle, 1);
        if (lines) lines->push_back("Suite facultative : le bénéfice principal est la confiance gagnée auprès du commanditaire.");
    }
    else if (quest.phaseTwoRewardKind == "information")
    {
        player.recordCanonicalEvent("informations_contractuelles", quest.id, quest.phaseTwoTitle + " — information recoupée", 1);
        player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
        if (lines) lines->push_back("Suite facultative : information recoupée enregistrée + Recommandation de client x1.");
    }
    else if (quest.phaseTwoRewardKind == "opportunity")
    {
        player.recordCanonicalEvent("opportunites_debloquees", quest.id, quest.phaseTwoTitle, 1);
        if (lines) lines->push_back("Suite facultative : une opportunité future est mémorisée. Elle ne crée pas automatiquement une nouvelle quête.");
    }
    else
    {
        return false;
    }
    return true;
}

bool QuestSecondPhaseSystem::hasVisibleSecondPhase(const Quest& quest)
{
    return quest.phaseTwoActive || quest.phaseTwoOffered || quest.phaseTwoCompleted || quest.phaseTwoDeclined;
}

std::vector<std::string> QuestSecondPhaseSystem::journalLines(const Quest& quest)
{
    std::vector<std::string> lines;
    if (!hasVisibleSecondPhase(quest)) return lines;

    lines.push_back("--- Phase 2 ---");
    if (!quest.phaseTwoPreviousSummary.empty()) lines.push_back(quest.phaseTwoPreviousSummary);
    lines.push_back("Type de suite : " + secondPhaseTypeLabel(quest) + ".");
    lines.push_back("Difficulté estimée : " + secondPhaseDifficultyLabel(quest) + ".");
    lines.push_back("Risques connus : " + secondPhaseKnownRisk(quest) + ".");
    if (!quest.phaseTwoReason.empty()) lines.push_back("Déclencheur : " + quest.phaseTwoReason);
    if (!quest.phaseTwoObjective.empty()) lines.push_back("Nouvel objectif : " + quest.phaseTwoObjective);
    if (quest.phaseTwoTravelRequired)
    {
        if (!quest.phaseTwoTravelDestinationCityId.empty())
        {
            lines.push_back("Déplacement requis : " + quest.phaseTwoTravelSourceCityId + " → " + quest.phaseTwoTravelDestinationCityId
                + " (~" + std::to_string(std::max(0, quest.phaseTwoTravelDistanceKm)) + " km)." );
        }
        else if (!quest.phaseTwoCompleted)
        {
            lines.push_back("Déplacement requis : destination inter-ville à organiser depuis ta ville actuelle.");
        }
    }

    if (quest.phaseTwoCompleted)
    {
        lines.push_back("État : phase 2 terminée ; récompense supplémentaire acquise.");
    }
    else if (quest.phaseTwoDeclined)
    {
        lines.push_back("État : suite facultative laissée de côté ; la phase 1 reste valablement terminée.");
    }
    else if (quest.phaseTwoActive)
    {
        lines.push_back(std::string("Nature : ") + (quest.phaseTwoMandatory ? "obligatoire pour finaliser la quête." : "facultative, choisie par le joueur."));
        lines.push_back("Progression phase 2 : " + std::to_string(quest.phaseTwoProgress) + "/" + std::to_string(std::max(1, quest.phaseTwoTarget)) + ".");
    }
    else if (quest.phaseTwoOffered)
    {
        lines.push_back("Nature : facultative. Tu peux rendre la quête maintenant ou poursuivre cette suite pour une meilleure récompense.");
    }

    if (quest.phaseTwoRewardExperience > 0 || quest.phaseTwoRewardGold > 0 || (!quest.phaseTwoRewardKind.empty() && quest.phaseTwoRewardKind != "money"))
    {
        std::string rewardLine = "Bonus si accomplie : XP +" + std::to_string(quest.phaseTwoRewardExperience);
        if (quest.phaseTwoRewardGold > 0)
        {
            rewardLine += " | argent contractuel +" + std::to_string(quest.phaseTwoRewardGold) + " unité(s)";
        }
        else if (!quest.phaseTwoRewardKind.empty() && quest.phaseTwoRewardKind != "money")
        {
            rewardLine += " | priorité : " + nonMonetaryRewardLabel(quest.phaseTwoRewardKind);
        }
        rewardLine += ".";
        lines.push_back(rewardLine);
        if (!quest.phaseTwoRewardDetail.empty()) lines.push_back("Récompense particulière : " + quest.phaseTwoRewardDetail);
    }
    return lines;
}
