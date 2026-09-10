// EN: TrainingGroundMenu implements the rentable practice stand requested by beta feedback.
// FR: TrainingGroundMenu implémente le stand d'entraînement louable demandé par les retours bêta.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/training/TrainingGroundMenu.hpp"

#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/Inventory.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/weapon/WeaponType.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    struct TrainingUnlock
    {
        bool active;
        std::string id;
        std::string name;
        std::string explanation;
    };

    std::string weaponTrainingLabel(WeaponType type)
    {
        switch (type)
        {
            case WeaponType::Dagger: return "dague / ouverture courte";
            case WeaponType::Bow: return "arme à distance / lecture de visée";
            case WeaponType::Staff: return "bâton / catalyseur";
            case WeaponType::Sword: return "épée / ligne propre";
            case WeaponType::Axe: return "hache / brise-ouverture";
            case WeaponType::Hammer: return "marteau / garde fissurée";
            case WeaponType::Spear: return "lance / contrôle d'allonge";
            case WeaponType::BareHands: return "mains nues / réflexe";
            default: return "arme improvisée";
        }
    }

    TrainingUnlock unlockForCurrentWeapon(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return {false, "combat_observation", "Observation tactique", "Sans arme équipée, le maître d'armes travaille surtout la lecture du rythme adverse."};
        }

        const WeaponType type = player.getEquippedWeapon().getType();
        switch (type)
        {
            case WeaponType::Dagger:
                return {true, "chain_execution", "Enchaînement", "La dague apprend à prolonger une ouverture déjà créée."};
            case WeaponType::Bow:
                return {false, "ranger_eye", "Œil de rôdeur", "La séance apprend à lire les trajectoires, les distances et les respirations."};
            case WeaponType::Staff:
                return {true, "cautious_channeling", "Canalisation prudente", "Le bâton sert de catalyseur stable plutôt que de simple bâton de combat."};
            case WeaponType::Sword:
                return {false, "blade_discipline", "Discipline de lame", "La lame revient dans l'axe après chaque coup au lieu de flotter au hasard."};
            case WeaponType::Axe:
                return {true, "splitting_blow", "Frappe fendue", "La hache travaille les trajectoires larges qui ouvrent les défenses."};
            case WeaponType::Hammer:
                return {true, "armor_crack", "Fracasse-garde", "Le marteau cherche les points de rupture de la garde adverse."};
            case WeaponType::Spear:
                return {false, "reach_control", "Contrôle d'allonge", "La lance impose une distance lisible au lieu de seulement piquer plus fort."};
            case WeaponType::BareHands:
                return {true, "reflex_counter", "Contre réflexe", "Le corps apprend à répondre quand l'ennemi rate son propre geste."};
            default:
                return {false, "combat_observation", "Observation tactique", "La séance transforme une arme improvisée en lecture plus propre du combat."};
        }
    }

    bool alreadyKnows(const Player& player, const TrainingUnlock& unlock)
    {
        return unlock.active ? player.hasActiveSkill(unlock.id) : player.hasPassiveSkill(unlock.id);
    }

    bool unlockSkill(Player& player, const TrainingUnlock& unlock)
    {
        return unlock.active
            ? player.unlockActiveSkill(unlock.id, unlock.name)
            : player.unlockPassiveSkill(unlock.id, unlock.name);
    }

    bool payTraining(Player& player, int cost, const std::string& screenId)
    {
        if (player.getInventory().getGold() < cost)
        {
            MessageScreen::show(
                "ENTRAÎNEMENT IMPOSSIBLE",
                screenId + ".not_enough_gold",
                {
                    "Coût demandé : " + std::to_string(cost) + " pièces d'or.",
                    "Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces d'or.",
                    "Le stand reste louable plus tard : pas besoin de forcer maintenant."
                }
            );
            return false;
        }

        player.getInventory().spendGold(cost);
        return true;
    }

    MenuOptionItemData makeTrainingData(
        const Player& player,
        const std::string& name,
        const std::string& detail,
        const std::string& status,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "training";
        itemData.section = "Stand d'entraînement";
        itemData.actionType = "train";
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = player.getName();
        itemData.progress = "Niveau " + std::to_string(player.getLevel());
        itemData.important = important;
        return itemData;
    }

    void showTrainingResult(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines, true);
    }
}

bool TrainingGroundMenu::open(Player& player)
{
    bool changed = false;

    while (true)
    {
        const int weaponCost = 35 + std::min(65, player.getLevel() * 3);
        const int observationCost = 42 + std::min(58, player.getLevel() * 2);
        const int environmentCost = 48 + std::min(72, player.getLevel() * 2);
        const int sparringCost = 55 + std::min(75, player.getLevel() * 3);
        const TrainingUnlock weaponUnlock = unlockForCurrentWeapon(player);
        const bool hasWeaponUnlock = alreadyKnows(player, weaponUnlock);
        const bool knowsObservation = player.hasActiveSkill("combat_observation");
        const bool knowsTemperatureDrill = player.hasPassiveSkill("temperature_drill");
        const bool knowsTerrainDrill = player.hasPassiveSkill("terrain_drill");
        const bool knowsFootworkDrill = player.hasPassiveSkill("footwork_drill");

        MenuScreen screen("STAND D'ENTRAÎNEMENT", "training.ground.main");
        screen.addSubtitle("Louable entre deux combats");
        screen.addLine("Or disponible : " + player.getInventory().getWalletTotalLine());
        screen.addLine("Le stand sert à apprendre plus vite, pas à remplacer toute la progression naturelle.");
        screen.addLine("Certaines séances débloquent des passifs qui se combinent plus tard avec d'autres passifs de température, terrain ou résistance.");
        if (player.hasEquippedWeapon())
        {
            screen.addLine("Arme équipée : " + player.getEquippedWeapon().getName() + " — axe d'entraînement : " + weaponTrainingLabel(player.getEquippedWeapon().getType()) + ".");
        }
        else
        {
            screen.addLine("Aucune arme équipée : la séance d'arme bascule vers de l'observation générale.");
        }
        screen.addBackOption("Retour", "training.ground.back");

        screen.addOption(
            1,
            "Séance technique d'arme",
            "Coût " + std::to_string(weaponCost) + " PO. Débloque ou consolide : " + weaponUnlock.name + ".",
            !hasWeaponUnlock,
            "training.ground.weapon",
            makeTrainingData(player, "Séance technique d'arme", weaponUnlock.explanation, hasWeaponUnlock ? "Déjà maîtrisé" : std::to_string(weaponCost) + " PO", !hasWeaponUnlock)
        );
        screen.addOption(
            2,
            "Observation tactique",
            "Coût " + std::to_string(observationCost) + " PO. Compétence de combat : lire la cible, créer une faille courte.",
            !knowsObservation,
            "training.ground.observation",
            makeTrainingData(player, "Observation tactique", "Débloque une compétence générale utilisable via Compétence de classe.", knowsObservation ? "Déjà appris" : std::to_string(observationCost) + " PO", !knowsObservation)
        );
        screen.addOption(
            3,
            "Habitude chaleur / froid",
            "Coût " + std::to_string(environmentCost) + " PO. Ajoute un passif compatible Résilience environnementale.",
            !knowsTemperatureDrill,
            "training.ground.temperature",
            makeTrainingData(player, "Habitude chaleur / froid", "Travaille les transitions de température sans donner une immunité gratuite.", knowsTemperatureDrill ? "Déjà appris" : std::to_string(environmentCost) + " PO")
        );
        screen.addOption(
            4,
            "Habitude terrain difficile",
            "Coût " + std::to_string(environmentCost) + " PO. Ajoute un passif compatible Résilience environnementale.",
            !knowsTerrainDrill,
            "training.ground.terrain",
            makeTrainingData(player, "Habitude terrain difficile", "Travaille boue, pente, cailloux, souffle et appuis.", knowsTerrainDrill ? "Déjà appris" : std::to_string(environmentCost) + " PO")
        );
        screen.addOption(
            5,
            "Appuis entraînés",
            "Coût " + std::to_string(sparringCost) + " PO. Améliore aussi l'action tactique Coup de pied / repousser.",
            !knowsFootworkDrill,
            "training.ground.footwork",
            makeTrainingData(player, "Appuis entraînés", "Apprend à replacer les pieds, attaquer moins mécaniquement et mieux repousser une cible.", knowsFootworkDrill ? "Déjà appris" : std::to_string(sparringCost) + " PO")
        );
        screen.addOption(
            6,
            "Voir mes compétences",
            "Relire ce que le personnage connaît déjà avant de payer.",
            true,
            "training.ground.skills",
            makeTrainingData(player, "Voir mes compétences", "Consultation gratuite.", "Gratuit")
        );

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une séance affichée.");

        if (choice == 0)
        {
            return changed;
        }

        if (choice == 6)
        {
            player.displaySkillProgress();
            continue;
        }

        if (choice == 1)
        {
            if (hasWeaponUnlock)
            {
                continue;
            }
            if (!payTraining(player, weaponCost, "training.ground.weapon"))
            {
                continue;
            }
            unlockSkill(player, weaponUnlock);
            showTrainingResult(
                "SÉANCE TERMINÉE",
                "training.ground.weapon.done",
                {
                    "Séance : technique d'arme.",
                    "Déblocage : " + weaponUnlock.name + ".",
                    weaponUnlock.explanation,
                    "Le combat devrait avoir plus d'options que juste attaque normale / gros coup."
                }
            );
            changed = true;
        }
        else if (choice == 2)
        {
            if (knowsObservation)
            {
                continue;
            }
            if (!payTraining(player, observationCost, "training.ground.observation"))
            {
                continue;
            }
            player.unlockActiveSkill("combat_observation", "Observation tactique");
            showTrainingResult(
                "OBSERVATION APPRISE",
                "training.ground.observation.done",
                {
                    "Nouvelle compétence générale : Observation tactique.",
                    "En combat, ouvre Compétence de classe puis choisis Observation tactique.",
                    "Elle lit l'état de la cible et pose une faille/faiblesse courte au lieu de seulement faire plus de dégâts."
                }
            );
            changed = true;
        }
        else if (choice == 3)
        {
            if (knowsTemperatureDrill)
            {
                continue;
            }
            if (!payTraining(player, environmentCost, "training.ground.temperature"))
            {
                continue;
            }
            player.unlockPassiveSkill("temperature_drill", "Habitude chaleur / froid");
            player.refreshCareerSkillProgress();
            showTrainingResult(
                "PARCOURS VALIDÉ",
                "training.ground.temperature.done",
                {
                    "Passif appris : Habitude chaleur / froid.",
                    "Ce n'est pas une immunité, mais il compte dans les synergies d'environnement.",
                    "Avec assez de passifs compatibles, Résilience environnementale peut se déclencher."
                }
            );
            changed = true;
        }
        else if (choice == 4)
        {
            if (knowsTerrainDrill)
            {
                continue;
            }
            if (!payTraining(player, environmentCost, "training.ground.terrain"))
            {
                continue;
            }
            player.unlockPassiveSkill("terrain_drill", "Habitude terrain difficile");
            player.refreshCareerSkillProgress();
            showTrainingResult(
                "PARCOURS VALIDÉ",
                "training.ground.terrain.done",
                {
                    "Passif appris : Habitude terrain difficile.",
                    "Il représente l'habitude des appuis compliqués, pas un bonus magique gratuit.",
                    "Il peut compléter d'autres passifs de résistance, température ou route."
                }
            );
            changed = true;
        }
        else if (choice == 5)
        {
            if (knowsFootworkDrill)
            {
                continue;
            }
            if (!payTraining(player, sparringCost, "training.ground.footwork"))
            {
                continue;
            }
            player.unlockPassiveSkill("footwork_drill", "Appuis entraînés");
            showTrainingResult(
                "DUEL TERMINÉ",
                "training.ground.footwork.done",
                {
                    "Passif appris : Appuis entraînés.",
                    "Les attaques rapides gagnent un peu plus d'intérêt et une petite fenêtre défensive.",
                    "Le Coup de pied / repousser des actions tactiques devient aussi plus propre.",
                    "Ce passif ne remplace pas une classe agile, il rend seulement le rythme moins plat."
                }
            );
            changed = true;
        }
    }
}
