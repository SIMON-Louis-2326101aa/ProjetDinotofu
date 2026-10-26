// EN: TutorialSystem.cpp implements the short, replayable Dinotofu onboarding route.
// FR: TutorialSystem.cpp implémente le tutoriel court et rejouable de Dinotofu.

#include "tutorial/TutorialSystem.hpp"

#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "save/SaveManager.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    const std::string DummyName = "Jean-Michel Test";
    const std::string StartedTitle = "Rencontre avec notre cobaye";
    const std::string CompletedTitle = "J'ai lu le manuel";

    bool containsTitle(const std::vector<std::string>& titles, const std::string& title)
    {
        return std::find(titles.begin(), titles.end(), title) != titles.end();
    }

    bool grantAccountTutorialTitle(const std::string& accountName, const std::string& title)
    {
        const std::vector<std::string> before = SaveManager::getAccountUnlockedTitles(accountName);
        if (containsTitle(before, title))
        {
            return false;
        }

        if (!SaveManager::mergeAccountUnlockedTitles(accountName, {title}, DummyName))
        {
            return false;
        }

        MessageScreen::show(
            "TITRE DE COMPTE OBTENU",
            "tutorial.title.unlocked",
            {
                "Titre obtenu : " + title + ".",
                "Débloqué avec : " + DummyName + ".",
                "Ce trophée appartient au compte ; le cobaye, lui, appartient surtout au service qualité."
            },
            false
        );
        return true;
    }

    bool askContinueOrLeave(const std::string& stepLabel)
    {
        MenuScreen screen("TUTORIEL — " + stepLabel, "tutorial.continue_or_leave");
        screen.addSubtitle("Parcours isolé — aucune progression réelle n'est modifiée");
        screen.addOption(1, "Continuer", "Poursuivre avec Jean-Michel Test.", true, "tutorial.continue");
        screen.addOption(0, "Quitter le tutoriel", "Retourner à la sélection des personnages.", true, "tutorial.leave");
        return TerminalInterface::askMenuChoice(screen, 0, 1, "Choisis 1 pour continuer ou 0 pour quitter.") == 1;
    }

    bool runCityLesson()
    {
        MessageScreen::show(
            "TUTORIEL — LA VILLE",
            "tutorial.city.intro",
            {
                "Instructeur : Une ville n'est pas un fond d'écran. Ici, tu peux chercher des services, parler aux PNJ, consulter la guilde et préparer tes sorties.",
                "Jean-Michel Test : Je peux entrer partout ?",
                "Instructeur : Non. C'est une ville, pas ta cuisine.",
                "Les lieux importants changent selon la ville : guilde, commerces, auberge, artisans, stockage et événements locaux."
            },
            false
        );

        bool visitedSomething = false;
        while (true)
        {
            MenuScreen city("PLACE D'ENTRAÎNEMENT", "tutorial.city.menu");
            city.addSubtitle("Choisis un lieu pour comprendre son rôle");
            city.addOption(1, "Guilde", "Contrats, rangs, informations et services officiels.", true, "tutorial.city.guild");
            city.addOption(2, "Boutique", "Achats, ventes, prix et stocks locaux.", true, "tutorial.city.shop");
            city.addOption(3, "Auberge", "Repos, informations et services selon la ville.", true, "tutorial.city.inn");
            city.addOption(4, "Continuer", visitedSomething ? "Passer à la mission d'entraînement." : "Visite d'abord au moins un lieu.", visitedSomething, "tutorial.city.next");
            city.addOption(0, "Quitter le tutoriel", "Retour à la sélection des personnages.", true, "tutorial.city.leave");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(city, "Choisis un lieu affiché.");
            if (choice == 0) return false;
            if (choice == 4 && visitedSomething) return true;

            visitedSomething = true;
            if (choice == 1)
            {
                MessageScreen::show(
                    "LA GUILDE",
                    "tutorial.city.guild.info",
                    {
                        "Instructeur : Ici, tu inspectes les contrats avant de les accepter.",
                        "Un contrat peut indiquer une difficulté, un lieu, un délai, un risque et parfois une suite imprévue.",
                        "Jean-Michel Test : Et si je ne lis rien ?",
                        "Instructeur : Alors le registre aura une très belle anecdote à raconter."
                    },
                    false
                );
            }
            else if (choice == 2)
            {
                MessageScreen::show(
                    "LA BOUTIQUE",
                    "tutorial.city.shop.info",
                    {
                        "Instructeur : Les prix et les stocks peuvent varier selon la ville et son économie.",
                        "Achète pour une raison, pas parce qu'un objet brille dans une liste.",
                        "Jean-Michel Test : J'allais acheter neuf cailloux.",
                        "Instructeur : Je sais."
                    },
                    false
                );
            }
            else if (choice == 3)
            {
                MessageScreen::show(
                    "L'AUBERGE",
                    "tutorial.city.inn.info",
                    {
                        "Instructeur : Les auberges servent surtout de point de repos, de passage et parfois d'information.",
                        "Tous les services ne sont pas identiques partout : le monde n'a pas signé de contrat de franchise."
                    },
                    false
                );
            }
        }
    }

    bool runQuestLesson()
    {
        MessageScreen::show(
            "TUTORIEL — UN CONTRAT",
            "tutorial.quest.intro",
            {
                "Contrat d'entraînement : récupérer une herbe bleue près du sentier-école.",
                "Rang : entraînement | Risque : faible | Lieu : accessible.",
                "Instructeur : Dans le vrai jeu, vérifie toujours l'état du lieu.",
                "Accessible : tu peux partir normalement.",
                "Chance réduite de trouvaille : la cible peut exister ailleurs, mais tu pars avec un avertissement.",
                "Inaccessible / inconnu : une mission exigeant précisément ce lieu peut être bloquée.",
                "Jean-Michel Test : Donc je lis avant d'accepter ?",
                "Instructeur : Je commence à croire en toi. Très légèrement."
            },
            false
        );

        return askContinueOrLeave("CONTRAT ACCEPTÉ");
    }

    bool runExplorationLesson()
    {
        bool inspectedTrail = false;
        while (true)
        {
            MenuScreen exploration("TUTORIEL — EXPLORATION", "tutorial.exploration.menu");
            exploration.addSubtitle("Sentier-école — zone temporaire du tutoriel");
            exploration.addLine("Objectif : retrouver l'herbe bleue sans modifier le monde réel.");
            exploration.addOption(1, "Observer le sentier", "Chercher des indices avant d'avancer.", true, "tutorial.exploration.observe");
            exploration.addOption(2, "Fouiller près des pierres", inspectedTrail ? "L'indice observé augmente tes chances de chercher au bon endroit." : "Tu peux tenter directement, mais observer aide souvent.", true, "tutorial.exploration.search");
            exploration.addOption(0, "Quitter le tutoriel", "Retour à la sélection des personnages.", true, "tutorial.exploration.leave");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(exploration, "Choisis une action d'exploration.");
            if (choice == 0) return false;
            if (choice == 1)
            {
                inspectedTrail = true;
                MessageScreen::show(
                    "INDICE TROUVÉ",
                    "tutorial.exploration.clue",
                    {
                        "Une trace de terre humide mène vers trois pierres plates.",
                        "Instructeur : Explorer, ce n'est pas seulement avancer. Observer peut changer ce que tu trouves et les risques que tu prends.",
                        "Jean-Michel Test : J'ai regardé le sol et il s'est passé quelque chose.",
                        "Instructeur : Notez la date."
                    },
                    false
                );
                continue;
            }

            MessageScreen::show(
                "OBJET TROUVÉ",
                "tutorial.exploration.found",
                {
                    inspectedTrail ? "Grâce à l'indice, Jean-Michel trouve rapidement l'herbe bleue." : "Après une recherche un peu moins élégante, Jean-Michel trouve quand même l'herbe bleue.",
                    "Inventaire temporaire du tutoriel : Herbe bleue x1.",
                    "Rien de cette étape ne sera ajouté à ton vrai personnage."
                },
                false
            );
            return true;
        }
    }

    bool runCombatLesson()
    {
        Player dummy(DummyName, ClassCatalog::createClassByName("Guerrier"));
        Monster trainingSlime(
            "Gloubi d'entraînement",
            "Créature d'entraînement",
            Race::Slime,
            1,
            26,
            2,
            4,
            5,
            0,
            0,
            false,
            false,
            false,
            false
        );
        Random random;
        bool defendedLastTurn = false;

        MessageScreen::show(
            "TUTORIEL — COMBAT",
            "tutorial.combat.intro",
            {
                "Une petite créature d'entraînement bloque le retour.",
                "Jean-Michel Test : Vous pourriez m'aider ?",
                "Instructeur : Non.",
                "Jean-Michel Test : Pourquoi ?",
                "Instructeur : Tutoriel.",
                "Le cobaye utilise de vraies bases de PV et de dégâts, mais cette rencontre reste isolée et sans conséquence."
            },
            false
        );

        while (!trainingSlime.isDead())
        {
            MenuScreen combat("COMBAT D'ENTRAÎNEMENT", "tutorial.combat.turn");
            combat.addSubtitle(DummyName + " contre " + trainingSlime.getName());
            combat.addLine(DummyName + " : " + std::to_string(dummy.getHp()) + "/" + std::to_string(dummy.getMaxHp()) + " PV");
            combat.addLine(trainingSlime.getName() + " : " + std::to_string(trainingSlime.getHp()) + "/" + std::to_string(trainingSlime.getMaxHp()) + " PV");
            combat.addOption(1, "Attaque simple", "Inflige des dégâts avec les statistiques du cobaye.", true, "tutorial.combat.attack");
            combat.addOption(2, "Défendre", "Réduit fortement les dégâts de la prochaine riposte.", true, "tutorial.combat.defend");
            combat.addOption(3, "Tenter de fuir", "Montre que la fuite existe, mais le tutoriel te fera finir ce combat.", true, "tutorial.combat.escape");
            combat.addOption(0, "Quitter le tutoriel", "Abandonne seulement la session d'entraînement.", true, "tutorial.combat.leave");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(combat, "Choisis une action de combat.");
            if (choice == 0) return false;

            if (choice == 3)
            {
                MessageScreen::show(
                    "TENTATIVE DE FUITE",
                    "tutorial.combat.escape.info",
                    {
                        "Instructeur : Oui, la fuite existe dans Dinotofu et certaines situations demandent même de préparer une ouverture.",
                        "Instructeur : Ici, non. Le Gloubi est littéralement payé pour rester devant la porte.",
                        "Jean-Michel Test : Il est payé ?",
                        "Instructeur : Plus que toi."
                    },
                    false
                );
                continue;
            }

            defendedLastTurn = choice == 2;
            if (choice == 1)
            {
                bool dodged = false;
                bool critical = false;
                const int damage = dummy.attack(random, dodged, critical);
                trainingSlime.takeDamage(damage);
                MessageScreen::show(
                    "ATTAQUE",
                    "tutorial.combat.attack.result",
                    {
                        DummyName + " inflige " + std::to_string(damage) + " dégâts" + (critical ? " critiques." : "."),
                        "Instructeur : Les dégâts réels dépendront ensuite de ta classe, de ton équipement, de tes effets et de la situation."
                    },
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "DÉFENSE",
                    "tutorial.combat.defend.result",
                    {
                        DummyName + " se met en garde.",
                        "Instructeur : Défendre sacrifie de la pression immédiate pour mieux encaisser ou préparer la suite selon le système de combat."
                    },
                    false
                );
            }

            if (trainingSlime.isDead()) break;

            bool slimeDodged = false;
            bool slimeCritical = false;
            int incoming = trainingSlime.attack(random, slimeDodged, slimeCritical);
            if (defendedLastTurn)
            {
                incoming = std::max(1, incoming / 2);
            }
            dummy.takeDamage(incoming);

            if (dummy.getHp() <= 5)
            {
                dummy.heal(12);
                MessageScreen::show(
                    "GARDE-FOU DU TUTORIEL",
                    "tutorial.combat.safety",
                    {
                        "Le terrain d'entraînement neutralise automatiquement une blessure trop sérieuse.",
                        "Instructeur : Dans une vraie partie, personne ne promet ce filet de sécurité.",
                        "Jean-Michel Test : Je proteste contre le mot « sérieuse »."
                    },
                    false
                );
            }
        }

        MessageScreen::show(
            "VICTOIRE D'ENTRAÎNEMENT",
            "tutorial.combat.victory",
            {
                "Le Gloubi d'entraînement est vaincu.",
                "Instructeur : Attaquer, défendre et fuir sont les bases. Les vrais combats ajoutent compétences, objets, états, positionnement, groupes, boss et conséquences.",
                "Jean-Michel Test : Donc ça devient pire ?",
                "Instructeur : Beaucoup plus intéressant."
            },
            false
        );
        return true;
    }

    void showFinalLesson()
    {
        MessageScreen::show(
            "TUTORIEL — RETOUR À LA GUILDE",
            "tutorial.final.quest_return",
            {
                "Herbe bleue remise. Récompense d'entraînement : 999 999 pièces imaginaires.",
                "Jean-Michel Test : Elles sont où ?",
                "Instructeur : Imaginaires.",
                "Dans une vraie mission, les récompenses peuvent être de l'argent, de la réputation, une relation, une information, une opportunité ou une suite de contrat.",
                "Le monde peut aussi réagir à ce que tu fais : villes, rumeurs, économie, groupes, blessures et conséquences ne vivent pas uniquement dans un menu."
            },
            false
        );

        MessageScreen::show(
            "TUTORIEL TERMINÉ",
            "tutorial.final.complete",
            {
                "Tu connais maintenant le minimum pour ne pas envoyer ton premier vrai personnage dans un mur avec conviction.",
                "Le tutoriel reste disponible depuis la sélection des personnages et peut être refait à tout moment.",
                "Aucune progression, monnaie, blessure, quête ou relation de Jean-Michel Test n'a été sauvegardée.",
                "Instructeur : Merci, Jean-Michel.",
                "Jean-Michel Test : Est-ce que je suis libre ?",
                "Instructeur : Nous ajouterons probablement d'autres mécaniques plus tard.",
                "Jean-Michel Test : ..."
            },
            false
        );
    }
}

const std::string& TutorialSystem::dummyName()
{
    return DummyName;
}

const std::string& TutorialSystem::startedTitle()
{
    return StartedTitle;
}

const std::string& TutorialSystem::completedTitle()
{
    return CompletedTitle;
}

bool TutorialSystem::run(const std::string& accountName)
{
    Console::clear();
    grantAccountTutorialTitle(accountName, StartedTitle);

    MessageScreen::show(
        "BIENVENUE DANS LE TUTORIEL",
        "tutorial.introduction",
        {
            "Instructeur : Bienvenue dans Dinotofu.",
            "Instructeur : Avant de risquer la vie d'un véritable aventurier, nous allons utiliser... ça.",
            DummyName + " : Bonjour.",
            "Instructeur : " + DummyName + ". Sa progression n'a aucune importance et ne sera jamais sauvegardée.",
            DummyName + " : Pardon ?",
            "Ce parcours présente rapidement la ville, les contrats, l'exploration et le combat. Durée visée : moins de dix minutes."
        },
        false
    );

    if (!askContinueOrLeave("PRÉSENTATION")) return false;
    if (!runCityLesson()) return false;
    if (!runQuestLesson()) return false;
    if (!runExplorationLesson()) return false;
    if (!runCombatLesson()) return false;

    showFinalLesson();
    grantAccountTutorialTitle(accountName, CompletedTitle);
    return true;
}
