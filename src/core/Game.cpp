// EN: Game.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Game.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "core/VersionInfo.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/progression/AttributeMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/PostCombatMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "cheat/CheatManager.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/DeathRuleRules.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"
#include "story/StoryCampaign.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <set>
#include <array>


namespace
{
    constexpr int UtilityChoiceOutOfCombatMenu = 89;
    constexpr int UtilityChoiceGuardian = 90;
    constexpr int UtilityChoiceInventory = 91;
    constexpr int UtilityChoiceQuickSave = 92;
    constexpr int UtilityChoiceSaveQuit = 93;
    constexpr int UtilityChoiceAlteredData = 94;
    constexpr int UtilityChoiceSettings = 95;


    std::string classCategoryBriefExample(ClassCategory category)
    {
        switch (category)
        {
            case ClassCategory::Melee:
                return "Contact direct, pression et armes proches. Exemple : Chevalier.";
            case ClassCategory::Distance:
                return "Tirs, munitions et placement. Exemple : Archer.";
            case ClassCategory::Magic:
                return "Sorts, catalyseurs et statuts. Exemple : Mage.";
            case ClassCategory::Invocation:
                return "Alliés appelés, sacrifices et tempo. Exemple : Invocateur.";
            case ClassCategory::Support:
                return "Protection, soin et contrôle défensif. Exemple : Clerc.";
            case ClassCategory::Hybrid:
                return "Mélange arme, magie ou rôle spécial. Exemple : Mage-lame.";
            case ClassCategory::Craft:
                return "Objets, économie, kits et expériences. Exemple : Forgeron.";
            case ClassCategory::Special:
            default:
                return "Profil spécial ou rare, plutôt lié à une histoire précise.";
        }
    }

    std::vector<std::string> classFuturePotentialLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        const std::string category = info.categoryName;
        const std::string name = info.name;

        if (category.find("Corps") != std::string::npos)
        {
            lines.push_back("Plus tard : styles d'arme, posture, riposte, provocation ou brise-garde selon la classe.");
        }
        else if (category == "Distance")
        {
            lines.push_back("Plus tard : munitions spéciales, pistage, tirs préparés, pièges et lecture de terrain.");
        }
        else if (category == "Magie")
        {
            lines.push_back("Plus tard : grimoires, sorts apprenables, catalyseurs, risques de canalisation et affinités élémentaires.");
        }
        else if (category == "Invocation")
        {
            lines.push_back("Plus tard : invocations maintenues, contrôle d'alliés, coût en mana et pertes possibles si le rythme casse.");
        }
        else if (category == "Soutien")
        {
            lines.push_back("Plus tard : soin, protection, stabilisation, lecture du danger et réactions aux alliés en difficulté.");
        }
        else if (category == "Hybride")
        {
            lines.push_back("Plus tard : mélange d'arme, magie et utilitaire, avec des bonus plus contextuels qu'une classe pure.");
        }
        else if (category.find("Artisanat") != std::string::npos)
        {
            lines.push_back("Plus tard : recettes, réparation, récolte propre, économie de ressources et services hors combat.");
        }
        else
        {
            lines.push_back("Plus tard : identité spéciale, effets plus liés au lore, à l'histoire ou à une règle unique.");
        }

        if (name.find("Rôdeur") != std::string::npos || name.find("Pisteur") != std::string::npos || name.find("Cartographe") != std::string::npos || name.find("Éclaireur") != std::string::npos)
        {
            lines.push_back("Évolution probable : meilleur repérage, aide aux explorations longues, distances mieux préparées et risques nocturnes mieux lus.");
        }
        if (name.find("Médecin") != std::string::npos || name.find("Clerc") != std::string::npos || name.find("Lumomancien") != std::string::npos)
        {
            lines.push_back("Évolution probable : soins plus propres, stabilisation, potions mieux utilisées et récupération hors combat améliorée.");
        }
        if (name.find("Forgeron") != std::string::npos || name.find("Récupérateur") != std::string::npos || name.find("Herboriste") != std::string::npos || name.find("Cuisinier") != std::string::npos)
        {
            lines.push_back("Évolution probable : meilleures recettes, rendement de matériaux, réparations et bonus de préparation avant sortie.");
        }
        if (name.find("Assassin") != std::string::npos || name.find("Umbromancien") != std::string::npos || name.find("Occultiste") != std::string::npos)
        {
            lines.push_back("Évolution probable : furtivité, ombre, critique ou approches nocturnes plus dangereuses mais plus rentables.");
        }

        lines.push_back("Note : ces évolutions prévues n'activent pas tout de suite un pouvoir gratuit ; elles servent à choisir une direction claire.");
        return lines;
    }


    std::vector<std::string> classWeaponGuidanceLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        const std::string name = info.name;
        const std::string category = info.categoryName;

        if (name.find("Archer") != std::string::npos || name.find("Rôdeur") != std::string::npos || name.find("Tireur") != std::string::npos || name.find("Arbal") != std::string::npos || name.find("Chasseur") != std::string::npos || name.find("Guetteur") != std::string::npos || name.find("Trappeur") != std::string::npos || category == "Distance")
        {
            lines.push_back("Recommandé : arc, arbalète/armes de trait, javelot ou dague de secours selon la classe.");
            lines.push_back("Maladroit : marteau/hache lourde, sauf classe explicitement prévue pour ça.");
        }
        else if (name.find("Assassin") != std::string::npos || name.find("Ombrelame") != std::string::npos || name.find("Lanceur de dagues") != std::string::npos || name.find("Danseur lunaire") != std::string::npos)
        {
            lines.push_back("Recommandé : dague, lame courte, sabre léger ou arme discrète.");
            lines.push_back("Maladroit : marteau, hache lourde ou bâton de canalisation trop voyant.");
        }
        else if (name.find("Mage") != std::string::npos || name.find("mancien") != std::string::npos || name.find("Sorcier") != std::string::npos || name.find("Arcaniste") != std::string::npos || name.find("Invoc") != std::string::npos || name.find("Nécro") != std::string::npos || category == "Magie" || category == "Invocation")
        {
            lines.push_back("Recommandé : bâton, sceptre, catalyseur ou dague légère de secours.");
            lines.push_back("Maladroit : armes très lourdes et arcs non canalisateurs, sauf classe hybride prévue pour ça.");
        }
        else if (name.find("Moine") != std::string::npos || name.find("Pugiliste") != std::string::npos || name.find("Cogneur") != std::string::npos)
        {
            lines.push_back("Recommandé : mains nues, gants, armes très courtes ou équipement léger.");
            lines.push_back("Maladroit : arme lourde qui casse le rythme du corps-à-corps.");
        }
        else if (name.find("Lancier") != std::string::npos || name.find("Hallebardier") != std::string::npos || name.find("Javelinier") != std::string::npos)
        {
            lines.push_back("Recommandé : lance, hallebarde, javelot ou arme d'allonge.");
            lines.push_back("Maladroit : arme trop courte si la classe mise tout sur la portée.");
        }
        else if (name.find("Barbare") != std::string::npos || name.find("Berserker") != std::string::npos || name.find("Briseur") != std::string::npos || name.find("Martelier") != std::string::npos || name.find("Colosse") != std::string::npos)
        {
            lines.push_back("Recommandé : hache, marteau, grande arme ou lame lourde.");
            lines.push_back("Maladroit : arc fin, bâton fragile ou arme trop subtile.");
        }
        else if (name.find("Chevalier") != std::string::npos || name.find("Guerrier") != std::string::npos || name.find("Paladin") != std::string::npos || name.find("Templier") != std::string::npos || name.find("Gladiateur") != std::string::npos)
        {
            lines.push_back("Recommandé : épée, lance, marteau de guerre ou arme martiale stable.");
            lines.push_back("Maladroit : bâton de mage ou arc si la classe n'a pas appris ce rythme.");
        }
        else
        {
            lines.push_back("Recommandé : arme cohérente avec le rôle affiché ; les hybrides tolèrent plus de choix.");
            lines.push_back("Maladroit : arme qui contredit totalement le style de classe.");
        }

        lines.push_back("Règle : l'arme influence maintenant à la fois la précision et les dégâts, avec bonus léger ou malus visible en combat.");
        return lines;
    }

    std::vector<std::string> buildClassInspectionLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        lines.push_back("Classe : " + info.name + ".");
        lines.push_back("Famille : " + info.categoryName + ".");
        lines.push_back("Rôle de base : " + info.role + ".");
        lines.push_back("Stats de départ : PV " + std::to_string(info.maxHp)
            + " | dégâts " + std::to_string(info.minDamage) + "-" + std::to_string(info.maxDamage)
            + " | critique " + std::to_string(info.criticalDamage) + ".");
        lines.push_back("Ressources de départ : potions soin " + std::to_string(info.healingPotionCount)
            + " | potions dégâts " + std::to_string(info.damagePotionCount) + ".");
        lines.push_back("");
        lines.push_back("Affinités d'armes :");
        const std::vector<std::string> weaponLines = classWeaponGuidanceLines(info);
        for (const std::string& line : weaponLines)
        {
            lines.push_back("- " + line);
        }
        lines.push_back("");
        lines.push_back("Ce qu'elle a de base :");
        lines.push_back("- Une identité de combat, des statistiques et un kit initial adaptés à la difficulté.");
        lines.push_back("- Des armes/potions de départ qui seront ensuite complétées par l'inventaire, les boutiques et les quêtes.");
        lines.push_back("");
        lines.push_back("Ce qu'elle pourrait avoir plus tard :");
        const std::vector<std::string> futureLines = classFuturePotentialLines(info);
        for (const std::string& line : futureLines)
        {
            lines.push_back("- " + line);
        }
        return lines;
    }

    MenuOptionItemData makeUtilityItemData(
        const Player& player,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = ""
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "utility";
        itemData.section = "Sous-menu hors combat";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = player.getName();
        itemData.progress = "Niveau " + std::to_string(player.getLevel());
        itemData.important = actionType == "save" || actionType == "guardian" || status == "Altéré";
        return itemData;
    }


    std::string lowerActivityText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool activityQuestIsActive(const Quest& quest)
    {
        return quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed;
    }

    bool activityQuestIsReady(const Quest& quest)
    {
        return quest.accepted && quest.completed && !quest.turnedIn && !quest.failed;
    }

    bool hasLikelyCombatQuest(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (activityQuestIsActive(quest) && quest.objectiveType == "combat")
            {
                return true;
            }
        }
        return false;
    }

    bool hasLikelyBossQuest(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!activityQuestIsActive(quest))
            {
                continue;
            }

            const std::string text = lowerActivityText(quest.title + " " + quest.objective + " " + quest.targetFamily);
            if (quest.objectiveType == "combat" && (text.find("boss") != std::string::npos || text.find("élite") != std::string::npos || text.find("elite") != std::string::npos || text.find("menace") != std::string::npos))
            {
                return true;
            }
        }
        return false;
    }

    bool hasLikelyExplorationQuest(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!activityQuestIsActive(quest))
            {
                continue;
            }

            if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
            {
                return true;
            }

            if (quest.objectiveType == "livraison")
            {
                return true;
            }
        }
        return false;
    }

    bool hasLikelyQuestHubObjective(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (activityQuestIsReady(quest))
            {
                return true;
            }

            if (activityQuestIsActive(quest) && quest.guildQuest && quest.objectiveType == "service")
            {
                return true;
            }
        }
        return false;
    }

    bool hasLikelyLocationOrNpcQuest(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.guildQuest && !quest.turnedIn && !quest.failed && quest.accepted)
            {
                return true;
            }
        }
        return false;
    }

    bool questInState(const Player& player, const std::string& questId, const std::string& state)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId || quest.failed)
            {
                continue;
            }

            if (state == "active") return quest.accepted && !quest.completed && !quest.turnedIn;
            if (state == "completed") return quest.completed && !quest.turnedIn;
            if (state == "turned_in") return quest.turnedIn;
        }
        return false;
    }

    bool storyQuestTurnedIn(const Player& player, const std::string& questId)
    {
        return questInState(player, questId, "turned_in");
    }

    bool storyQuestCompleted(const Player& player, const std::string& questId)
    {
        return questInState(player, questId, "completed");
    }

    bool progressStoryQuestById(Player& player, const std::string& questId, int amount)
    {
        if (amount <= 0)
        {
            return false;
        }

        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed)
            {
                quest.progress += amount;
                if (quest.progress >= quest.target)
                {
                    quest.progress = quest.target;
                    quest.completed = true;
                }
                return true;
            }
        }
        return false;
    }

    std::string storyQuestProgressText(const Player& player, const std::string& questId)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId)
            {
                continue;
            }
            if (quest.failed) return "bloquée";
            if (quest.turnedIn) return "validée";
            if (quest.completed) return "prête à rendre";
            if (quest.accepted) return "en cours " + std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            return "connue";
        }
        return "à débloquer";
    }

    std::string questActivityTag(bool likely)
    {
        return likely ? " [Objectif de quête probable]" : "";
    }

    MenuOptionItemData makeActivityItemData(
        const std::string& section,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "activity";
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress;
        itemData.owner = "Ville / hors combat";
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeCreationItemData(
        const std::string& kind,
        const std::string& section,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        const std::string& reward = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress;
        itemData.reward = reward;
        itemData.owner = "Création de personnage";
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeSessionItemData(
        const Player& player,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "session";
        itemData.section = "Session";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress.empty() ? "Joueur 1 : " + player.getName() : progress;
        itemData.owner = player.getName();
        itemData.important = important;
        return itemData;
    }

    std::string difficultyModeLabel(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Facile";
            case DifficultyMode::Hard:
                return "Difficile";
            case DifficultyMode::Nightmare:
                return "Cauchemar";
            case DifficultyMode::Lethal:
                return "Léthal";
            case DifficultyMode::Normal:
            default:
                return "Normal";
        }
    }

    std::string difficultyRespawnSummary(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Respawn non définitif : environ 75% PV.";
            case DifficultyMode::Hard:
                return "Respawn non définitif : environ 30% PV.";
            case DifficultyMode::Nightmare:
                return "Respawn non définitif : environ 10% PV.";
            case DifficultyMode::Lethal:
                return "Respawn : mort définitive possible, sauf exception narrative rarissime.";
            case DifficultyMode::Normal:
            default:
                return "Respawn non définitif : règles standards.";
        }
    }

    std::string difficultyRiskSummary(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Risque : plus permissif, pertes limitées.";
            case DifficultyMode::Hard:
                return "Risque : pertes et durabilité plus tendues.";
            case DifficultyMode::Nightmare:
                return "Risque : très punitif, équipement rarement intouchable.";
            case DifficultyMode::Lethal:
                return "Risque : le registre peut perdre ton personnage.";
            case DifficultyMode::Normal:
            default:
                return "Risque : équilibre standard.";
        }
    }

    std::string difficultyRewardSummary(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Récompenses : départ plus confortable.";
            case DifficultyMode::Hard:
                return "Récompenses : gains à surveiller face aux prix.";
            case DifficultyMode::Nightmare:
                return "Récompenses : chaque gain compte davantage.";
            case DifficultyMode::Lethal:
                return "Récompenses : progression risquée, trace de mort spéciale.";
            case DifficultyMode::Normal:
            default:
                return "Récompenses : valeurs de référence.";
        }
    }

    MenuOptionItemData makeExchangeItemData(
        const Player& owner,
        const std::string& kind,
        const std::string& section,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        const std::string& reward = "",
        int price = 0,
        int stock = 0,
        int quantity = 0,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress;
        itemData.reward = reward;
        itemData.price = price;
        itemData.stock = stock;
        itemData.quantity = quantity;
        itemData.owner = owner.getName();
        itemData.important = important || status.find("équip") != std::string::npos || status.find("port") != std::string::npos;
        return itemData;
    }

    MenuOptionItemData makeExchangeAccountItemData(
        const std::string& section,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "exchange";
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress;
        itemData.owner = "Registre local";
        itemData.important = important;
        return itemData;
    }

    std::string guardianAnswerFor(const std::string& rawText)
    {
        std::string normalized;
        normalized.reserve(rawText.size());

        for (unsigned char character : rawText)
        {
            normalized += static_cast<char>(std::tolower(character));
        }

        if (normalized.empty())
        {
            return "Le gardien attend quelques mots avant de répondre.";
        }

        if (normalized.find("bug") != std::string::npos ||
            normalized.find("bloque") != std::string::npos ||
            normalized.find("erreur") != std::string::npos)
        {
            return "Le gardien sent une fissure. Reviens à l'écran précédent, sauvegarde si possible, puis force l'arrêt seulement si le monde ne répond vraiment plus.";
        }

        if (normalized.find("aide") != std::string::npos ||
            normalized.find("perdu") != std::string::npos ||
            normalized.find("quoi faire") != std::string::npos)
        {
            return "Regarde les choix affichés à l'instant présent. Les routes disponibles sont celles que le monde accepte maintenant, pas celles qui appartiennent à un autre écran.";
        }

        if (normalized.find("mort") != std::string::npos ||
            normalized.find("boss") != std::string::npos ||
            normalized.find("combat") != std::string::npos)
        {
            return "Un combat se gagne avant le premier coup : équipement, potions, lecture des indices, puis décision nette quand le tour arrive.";
        }

        return "Le gardien t'entend. Ces mots ne modifient pas encore le monde, mais ils restent au bord de la faille.";
    }

    std::string normalizeValueName(std::string value)
    {
        std::string out;
        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                out += static_cast<char>(std::tolower(character));
            }
        }
        return out;
    }

    // EN: valueNameContainsAny declares or implements a focused behavior used by this module.
    // FR: valueNameContainsAny déclare ou implémente un comportement précis utilisé par ce module.
    bool valueNameContainsAny(const std::string& value, const std::vector<std::string>& words)
    {
        std::string normalized = normalizeValueName(value);
        for (const std::string& word : words)
        {
            if (normalized.find(normalizeValueName(word)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    // EN: rarityEstimateMultiplier declares or implements a focused behavior used by this module.
    // FR: rarityEstimateMultiplier déclare ou implémente un comportement précis utilisé par ce module.
    int rarityEstimateMultiplier(const std::string& name)
    {
        if (valueNameContainsAny(name, {"Relique", "Unique", "Divin", "God"})) return 5;
        if (valueNameContainsAny(name, {"Héroïque", "Heroique", "Légendaire", "Legendaire"})) return 3;
        if (valueNameContainsAny(name, {"Rare", "Mystique"})) return 2;
        return 1;
    }

    // EN: estimateWeaponTradeValue declares or implements a focused behavior used by this module.
    // FR: estimateWeaponTradeValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateWeaponTradeValue(const Weapon& weapon)
    {
        int value = weapon.getValue();
        value += weapon.getMinDamageBonus() * 8;
        value += weapon.getMaxDamageBonus() * 10;
        value += weapon.getCriticalBonus() * 6;
        value *= rarityEstimateMultiplier(weapon.getName());

        if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0)
        {
            value = value * std::max(1, weapon.getDurability()) / weapon.getMaxDurability();
        }

        return std::max(1, value);
    }

    // EN: estimateArmorTradeValue declares or implements a focused behavior used by this module.
    // FR: estimateArmorTradeValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateArmorTradeValue(const Armor& armor)
    {
        int value = armor.getValue();
        value += armor.getMaxHpBonus() * 8;
        value *= rarityEstimateMultiplier(armor.getName());

        if (!armor.isIndestructible() && armor.getMaxDurability() > 0)
        {
            value = value * std::max(1, armor.getDurability()) / armor.getMaxDurability();
        }

        return std::max(1, value);
    }

    // EN: estimatePlayerTradeValue declares or implements a focused behavior used by this module.
    // FR: estimatePlayerTradeValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimatePlayerTradeValue(const Player& player)
    {
        int total = player.getInventory().getGold();

        for (const Weapon& weapon : player.getInventory().getWeapons())
        {
            total += estimateWeaponTradeValue(weapon);
        }

        for (const Armor& armor : player.getInventory().getArmors())
        {
            total += estimateArmorTradeValue(armor);
        }

        for (const Consumable& consumable : player.getInventory().getConsumables())
        {
            total += consumable.getValue();
        }

        for (const Material& material : player.getInventory().getMaterials())
        {
            total += std::max(1, material.getValue() * material.getQualityPricePercent() / 100) * material.getQuantity();
        }

        return total;
    }

    // EN: displayExchangeValueEstimation declares or implements a focused behavior used by this module.
    // FR: displayExchangeValueEstimation déclare ou implémente un comportement précis utilisé par ce module.
    void displayExchangeValueEstimation(const Player& first, const Player& second)
    {
        MessageScreen::show(
            "ESTIMATION D'ÉCHANGE",
            "exchange.value_estimation",
            {
                "Valeur estimée de " + first.getName() + " : " + std::to_string(estimatePlayerTradeValue(first)) + " pièces.",
                "Valeur estimée de " + second.getName() + " : " + std::to_string(estimatePlayerTradeValue(second)) + " pièces."
            },
            false
        );
    }


    int askExchangeAccountIndex(
        const std::vector<AccountSaveSummary>& accounts,
        const std::string& currentAccountName
    )
    {
        if (accounts.empty())
        {
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(accounts.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(accounts.size(), page, itemsPerPage);

            MenuScreen screen("ÉCHANGE / DON", "exchange.account.select");
            screen.addSubtitle("Choisis le compte cible.");
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, accounts.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const AccountSaveSummary& account = accounts[i];
                const bool currentAccount = account.accountName == currentAccountName;
                screen.addOption(
                    static_cast<int>(i - first + 1),
                    account.accountName,
                    currentAccount ? "Ton compte actuel : seuls les autres personnages peuvent être ciblés." : "Compte local disponible.",
                    true,
                    "exchange.account.select",
                    makeExchangeAccountItemData(
                        "Comptes disponibles",
                        "select_account",
                        account.accountName,
                        currentAccount ? "Compte actuel." : "Compte local disponible pour l'échange.",
                        currentAccount ? "Compte actuel" : "Disponible",
                        PagedMenu::rangeText(first, last, accounts.size()),
                        currentAccount
                    )
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Veuillez choisir un compte affiché."
            );
            Console::clear();

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeCharacterIndex(
        const std::vector<CharacterSaveSummary>& characters,
        const std::string& targetAccount,
        const std::string& currentAccountName,
        const std::string& currentCharacterName
    )
    {
        if (characters.empty())
        {
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(characters.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(characters.size(), page, itemsPerPage);

            MenuScreen screen("PERSONNAGE CIBLE", "exchange.character.select");
            screen.addSubtitle("Compte cible : " + targetAccount);
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, characters.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const CharacterSaveSummary& character = characters[i];
                const bool sameCharacter = targetAccount == currentAccountName && character.characterName == currentCharacterName;
                const std::string label = character.characterName
                    + " | " + character.raceName
                    + " / " + character.className
                    + " | Niveau " + std::to_string(character.level);

                std::string status = sameCharacter ? "Personnage actuel" : "Disponible";
                if (character.clone)
                {
                    status += " | clone";
                }

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    label,
                    sameCharacter ? "C'est ton personnage actuel : échange impossible avec soi-même." : "Maître : " + character.currentOwnerAccountName,
                    !sameCharacter,
                    "exchange.character.select",
                    makeExchangeAccountItemData(
                        "Personnages disponibles",
                        "select_character",
                        character.characterName,
                        character.raceName + " / " + character.className,
                        status,
                        "Niveau " + std::to_string(character.level) + " | Version " + character.gameVersion,
                        sameCharacter || character.clone
                    )
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Veuillez choisir un personnage affiché."
            );
            Console::clear();

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeWeaponIndex(const Player& giver)
    {
        const std::vector<Weapon>& weapons = giver.getInventory().getWeapons();

        if (weapons.empty())
        {
            MenuScreen screen("ARME À TRANSFÉRER", "exchange.weapon.empty");
            screen.addLine(giver.getName() + " n'a aucune arme transférable dans son sac.");
            screen.setDisplayOnlyInput("Aucune arme transférable : retour automatique au choix précédent.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(weapons.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(weapons.size(), page, itemsPerPage);

            MenuScreen screen("ARME À TRANSFÉRER", "exchange.weapon.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, weapons.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Weapon& weapon = weapons[i];
                const bool equipped = static_cast<int>(i) == giver.getEquippedWeaponIndex();
                std::ostringstream hint;
                hint << "Dégâts +" << weapon.getMinDamageBonus() << "/+" << weapon.getMaxDamageBonus()
                     << " | Critique +" << weapon.getCriticalBonus();

                if (weapon.isIndestructible())
                {
                    hint << " | Durabilité : indestructible";
                }
                else
                {
                    hint << " | Durabilité " << weapon.getDurability() << "/" << weapon.getMaxDurability();
                }

                if (equipped)
                {
                    hint << " | équipée";
                }

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    weapon.getName(),
                    hint.str(),
                    !equipped,
                    "exchange.weapon.select",
                    makeExchangeItemData(
                        giver,
                        "weapon",
                        "Armes transférables",
                        "select_weapon",
                        weapon.getName(),
                        hint.str(),
                        equipped ? "Équipée - non transférable" : "Transférable",
                        "Valeur estimée " + std::to_string(estimateWeaponTradeValue(weapon)),
                        "",
                        estimateWeaponTradeValue(weapon),
                        1,
                        1,
                        equipped
                    )
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une arme affichée.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeArmorIndex(const Player& giver)
    {
        const std::vector<Armor>& armors = giver.getInventory().getArmors();

        if (armors.empty())
        {
            MenuScreen screen("ARMURE À TRANSFÉRER", "exchange.armor.empty");
            screen.addLine(giver.getName() + " n'a aucune armure transférable dans son sac.");
            screen.setDisplayOnlyInput("Aucune armure transférable : retour automatique au choix précédent.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(armors.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(armors.size(), page, itemsPerPage);

            MenuScreen screen("ARMURE À TRANSFÉRER", "exchange.armor.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, armors.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Armor& armor = armors[i];
                const bool equipped = static_cast<int>(i) == giver.getEquippedArmorIndex();
                std::ostringstream hint;
                hint << "PV +" << armor.getMaxHpBonus() << " | Réduction " << armor.getDamageReduction();

                if (armor.isIndestructible())
                {
                    hint << " | Durabilité : indestructible";
                }
                else
                {
                    hint << " | Durabilité " << armor.getDurability() << "/" << armor.getMaxDurability();
                }

                if (equipped)
                {
                    hint << " | portée";
                }

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    armor.getName(),
                    hint.str(),
                    !equipped,
                    "exchange.armor.select",
                    makeExchangeItemData(
                        giver,
                        "armor",
                        "Armures transférables",
                        "select_armor",
                        armor.getName(),
                        hint.str(),
                        equipped ? "Portée - non transférable" : "Transférable",
                        "Valeur estimée " + std::to_string(estimateArmorTradeValue(armor)),
                        "",
                        estimateArmorTradeValue(armor),
                        1,
                        1,
                        equipped
                    )
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une armure affichée.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeConsumableIndex(const Player& giver)
    {
        const std::vector<Consumable>& consumables = giver.getInventory().getConsumables();

        if (consumables.empty())
        {
            MenuScreen screen("CONSOMMABLE À TRANSFÉRER", "exchange.consumable.empty");
            screen.addLine(giver.getName() + " n'a aucun consommable dans son sac.");
            screen.setDisplayOnlyInput("Aucun consommable transférable : retour automatique au choix précédent.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(consumables.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(consumables.size(), page, itemsPerPage);

            MenuScreen screen("CONSOMMABLE À TRANSFÉRER", "exchange.consumable.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, consumables.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Consumable& consumable = consumables[i];
                screen.addOption(
                    static_cast<int>(i - first + 1),
                    consumable.getName(),
                    "Puissance " + std::to_string(consumable.getPower()) + " | Valeur " + std::to_string(consumable.getValue()),
                    true,
                    "exchange.consumable.select",
                    makeExchangeItemData(
                        giver,
                        "consumable",
                        "Consommables transférables",
                        "select_consumable",
                        consumable.getName(),
                        "Puissance " + std::to_string(consumable.getPower()),
                        "Transférable",
                        "Valeur " + std::to_string(consumable.getValue()),
                        "",
                        consumable.getValue(),
                        1,
                        1
                    )
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un consommable affiché.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }

    int askExchangeMaterialIndex(const Player& giver)
    {
        const std::vector<Material>& materials = giver.getInventory().getMaterials();

        if (materials.empty())
        {
            MenuScreen screen("MATÉRIAU À TRANSFÉRER", "exchange.material.empty");
            screen.addLine(giver.getName() + " n'a aucun matériau dans son sac.");
            screen.setDisplayOnlyInput("Aucun matériau transférable : retour automatique au choix précédent.");
            TerminalInterface::renderMenuScreen(screen, false);
            return -1;
        }

        constexpr std::size_t itemsPerPage = 8;
        std::size_t page = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(materials.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(page, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(materials.size(), page, itemsPerPage);

            MenuScreen screen("MATÉRIAU À TRANSFÉRER", "exchange.material.select");
            screen.addSubtitle("Source : " + giver.getName());
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, materials.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const Material& material = materials[i];
                screen.addOption(
                    static_cast<int>(i - first + 1),
                    material.getName() + " x" + std::to_string(material.getQuantity()),
                    material.getCategory() + " | Qualité " + material.getQualityLabel() + " | Valeur " + std::to_string(material.getValue()),
                    true,
                    "exchange.material.select",
                    makeExchangeItemData(
                        giver,
                        "material",
                        "Matériaux transférables",
                        "select_material",
                        material.getName(),
                        material.getCategory() + " | Qualité " + material.getQualityLabel(),
                        "Transférable",
                        "Valeur unitaire " + std::to_string(material.getValue()),
                        "",
                        material.getValue(),
                        material.getQuantity(),
                        material.getQuantity(),
                        material.getQualityPricePercent() > 120
                    )
                );
            }

            PagedMenu::addNavigationOptions(screen, page, totalPages);

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un matériau affiché.");

            if (choice == 0)
            {
                return -1;
            }

            if (choice == 98 && page > 0)
            {
                --page;
                Console::clear();
                continue;
            }

            if (choice == 99 && page + 1 < totalPages)
            {
                ++page;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice >= 1 && choice <= visibleCount)
            {
                return static_cast<int>(first + static_cast<std::size_t>(choice - 1));
            }
        }
    }
}

// EN: Game declares or implements a focused behavior used by this module.
// FR: Game déclare ou implémente un comportement précis utilisé par ce module.
Game::Game()
{
    accountName = "local";
    playerName = "";
    selectedMode = GameMode::AIPvp;
    selectedDifficulty = DifficultyMode::Normal;
    selectedDeathRule = DeathRuleRules::defaultForDifficulty(selectedDifficulty);
    selectedRace = CharacterRace::Human;
    characterLoaded = false;
    specialIdentityValidated = false;
    ephemeralSandboxSession = false;
}

// EN: run declares or implements a focused behavior used by this module.
// FR: run déclare ou implémente un comportement précis utilisé par ce module.
void Game::run()
{
    Console::clear();

    displayIntroduction();
    askAccountName();

    const int deletedEphemeralClones = SaveManager::deleteEphemeralStoryCloneSaves();
    if (deletedEphemeralClones > 0)
    {
        MessageScreen::show(
            "CLONES ÉPHÉMÈRES",
            "story.ephemeral.cleanup",
            {
                "Le registre a trouvé " + std::to_string(deletedEphemeralClones) + " clone(s) éphémère(s) de bac à sable.",
                "Ils ont été supprimés avant le chargement normal pour ne pas perturber la route histoire."
            },
            false
        );
    }

    askPlayerName();

    if (!characterLoaded)
    {
        chooseDifficulty();
        chooseDeathRule();

        if (!specialIdentityValidated)
        {
            choosePlayerRace();
        }

        choosePlayerClass();
    }

    const std::vector<std::string> titlesBeforeLaunch = mainPlayer.getTitles();
    mainPlayer.grantTitle("Bienvenue dans Dinotofu");
    mainPlayer.grantTitle("Voix du terminal");

    std::vector<std::string> newLaunchTitles;
    for (const std::string& title : mainPlayer.getTitles())
    {
        if (std::find(titlesBeforeLaunch.begin(), titlesBeforeLaunch.end(), title) == titlesBeforeLaunch.end())
        {
            newLaunchTitles.push_back(title);
        }
    }

    if (!newLaunchTitles.empty())
    {
        std::vector<std::string> lines;
        lines.push_back("Le registre a reconnu ce lancement de session.");
        for (const std::string& title : newLaunchTitles)
        {
            lines.push_back("Titre obtenu : " + title + ".");
        }
        lines.push_back("Rappel : les titres équipés restent surtout lore/dialogues/réputation, avec bonus très faibles.");
        MessageScreen::show("TITRES DE SESSION", "titles.session.launch", lines, false);
        saveCurrentProgress("Titres de lancement");
    }

    configurePartyMode();
    chooseGameMode();
    displaySelectedMode();
    launchSelectedMode();
}

// EN: displayIntroduction declares or implements a focused behavior used by this module.
// FR: displayIntroduction déclare ou implémente un comportement précis utilisé par ce module.
void Game::displayIntroduction()
{
    MessageScreen::show(
        "DINOTOFU V" + VersionInfo::currentVersion(),
        "game.introduction",
        {
            "Bonjour voyageur, et bienvenue dans Dinotofu.",
            "Un monde de fantaisie, d'arènes et de baston,",
            "où chaque choix peut transformer un simple combattant en légende."
        }
    );
}


// EN: askAccountName declares or implements a focused behavior used by this module.
// FR: askAccountName déclare ou implémente un comportement précis utilisé par ce module.
void Game::askAccountName()
{
    accountName = AccountMenu::open();
}

// EN: askPlayerName declares or implements a focused behavior used by this module.
// FR: askPlayerName déclare ou implémente un comportement précis utilisé par ce module.
void Game::askPlayerName()
{
    CharacterMenuResult result = CharacterMenu::open(accountName, mainPlayer);

    characterLoaded = result.characterLoaded;
    specialIdentityValidated = result.specialIdentityValidated;
    playerName = result.playerName;

    if (result.specialIdentityValidated)
    {
        selectedRace = result.forcedRace;
    }

    if (characterLoaded)
    {
        selectedDifficulty = result.difficulty;
        selectedDeathRule = result.deathRule;
        selectedRace = mainPlayer.getRace();
    }
}

// EN: chooseDifficulty declares or implements a focused behavior used by this module.
// FR: chooseDifficulty déclare ou implémente un comportement précis utilisé par ce module.
void Game::chooseDifficulty()
{
    MenuScreen screen("DIFFICULTÉ", "character.creation.difficulty");
    screen.addSubtitle("Ce choix influence le kit de départ, les récompenses, la mort et le respawn.");
    screen.addOption(
        1,
        "Facile",
        "Plus d'or, plus de sécurité, retour à 75% PV après une mort non définitive.",
        true,
        "difficulty.easy",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Facile", "Départ plus permissif avec davantage de sécurité.", "Sécurisé", "Respawn non définitif : 75% PV", "Ressources de départ améliorées")
    );
    screen.addOption(
        2,
        "Normal",
        "L'expérience Dinotofu standard.",
        true,
        "difficulty.normal",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Normal", "Équilibre prévu pour la majorité des premières parties.", "Standard", "Règles normales", "Kit de départ standard")
    );
    screen.addOption(
        3,
        "Difficile",
        "Moins de ressources, pénalités plus dures, retour à 30% PV.",
        true,
        "difficulty.hard",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Difficile", "Moins de ressources et pénalités de mort plus fortes.", "Punitif", "Respawn non définitif : 30% PV", "Récompenses et pertes plus tendues", true)
    );
    screen.addOption(
        4,
        "Cauchemar",
        "Très punitif, retour à 10% PV, et la mort commence vraiment à avoir des dents.",
        true,
        "difficulty.nightmare",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Cauchemar", "Mode très dur placé juste avant le Léthal.", "Très dangereux", "Respawn non définitif : 10% PV", "Pertes sévères", true)
    );
    screen.addOption(
        5,
        "Léthal",
        "Le registre ne pardonne pas : une vraie chute peut effacer ton nom.",
        true,
        "difficulty.lethal",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Léthal", "Mort définitive possible : le personnage peut quitter le registre des vivants.", "Permadeath", "Historique des morts définitives", "Statistiques corrompues possibles", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Veuillez entrer un chiffre correspondant à une difficulté affichée."
    );

    switch (choice)
    {
        case 1:
            selectedDifficulty = DifficultyMode::Easy;
            break;

        case 3:
            selectedDifficulty = DifficultyMode::Hard;
            break;

        case 4:
            selectedDifficulty = DifficultyMode::Nightmare;
            break;

        case 5:
            selectedDifficulty = DifficultyMode::Lethal;
            break;

        case 2:
        default:
            selectedDifficulty = DifficultyMode::Normal;
            break;
    }

    Console::clear();

    MenuScreen confirmation("DIFFICULTÉ VALIDÉE", "character.creation.difficulty.confirmation");
    confirmation.setContinueInput("Valide pour continuer vers la suite de création.");
    confirmation.addSubtitle("Transition de création");
    confirmation.addLine("Difficulté sélectionnée : " + getDifficultyName() + ".");
    confirmation.addLine(difficultyRiskSummary(selectedDifficulty));
    confirmation.addLine(difficultyRespawnSummary(selectedDifficulty));
    confirmation.addLine(difficultyRewardSummary(selectedDifficulty));
    confirmation.addLine("Prochaine étape : règle de mort du personnage.");
    confirmation.addLine("Ton départ sera ajusté en conséquence.");

    if (specialIdentityValidated)
    {
        confirmation.addLine("Identité spéciale reconnue : le choix de race est verrouillé par son histoire.");
        confirmation.addLine("Race imposée : " + characterRaceToText(selectedRace) + ".");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}


void Game::chooseDeathRule()
{
    selectedDeathRule = DeathRuleRules::normalizeForDifficulty(selectedDifficulty, selectedDeathRule);

    if (DeathRuleRules::isChoiceForced(selectedDifficulty))
    {
        MenuScreen forcedScreen("RÈGLE DE MORT", "character.creation.death_rule.forced");
        forcedScreen.addSubtitle("Règle imposée par la difficulté");
        forcedScreen.addLine("Difficulté : " + getDifficultyName() + ".");
        forcedScreen.addLine("Règle appliquée : " + DeathRuleRules::displayName(selectedDeathRule) + ".");
        forcedScreen.addLine(DeathRuleRules::shortSummary(selectedDifficulty, selectedDeathRule));
        forcedScreen.addLine("Prochaine étape : " + std::string(specialIdentityValidated ? "classe" : "race"));
        forcedScreen.setContinueInput("Valide pour continuer.");
        TerminalInterface::renderMenuScreen(forcedScreen, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen screen("RÈGLE DE MORT", "character.creation.death_rule");
    screen.addSubtitle("Ce choix est séparé de la difficulté.");
    screen.addLine("Difficulté choisie : " + getDifficultyName() + ".");
    screen.addLine("Facile bloque toujours la mort définitive ; Léthal la force toujours.");
    screen.addOption(
        1,
        "Mort non définitive",
        "Tu peux subir des pertes et pénalités, mais le personnage reste jouable après respawn.",
        true,
        "death_rule.non_definitive",
        makeCreationItemData("death_rule", "Création - règle de mort", "select", "Mort non définitive", "Respawn conservé malgré les pénalités.", "Sécurisé", "Pas d'effacement jouable", "Compatible hors Facile/Léthal")
    );
    screen.addOption(
        2,
        "Mort définitive",
        "Challenge supplémentaire : une vraie chute peut déplacer le personnage au registre des morts.",
        true,
        "death_rule.definitive",
        makeCreationItemData("death_rule", "Création - règle de mort", "select", "Mort définitive", "Le risque de registre mort s'ajoute à la difficulté choisie.", "Permadeath", "Personnage supprimé des jouables si mort", "Même garde-fous JcJ que Léthal", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Veuillez choisir une règle de mort affichée."
    );

    selectedDeathRule = DeathRuleRules::normalizeForDifficulty(
        selectedDifficulty,
        choice == 2 ? DeathRuleMode::Definitive : DeathRuleMode::NonDefinitive
    );

    Console::clear();

    MenuScreen confirmation("RÈGLE DE MORT VALIDÉE", "character.creation.death_rule.confirmation");
    confirmation.setContinueInput("Valide pour continuer vers la suite de création.");
    confirmation.addSubtitle("Transition de création");
    confirmation.addLine("Difficulté : " + getDifficultyName() + ".");
    confirmation.addLine("Règle de mort : " + getDeathRuleName() + ".");
    confirmation.addLine(DeathRuleRules::shortSummary(selectedDifficulty, selectedDeathRule));
    confirmation.addLine("Prochaine étape : " + std::string(specialIdentityValidated ? "classe" : "race"));
    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: choosePlayerRace declares or implements a focused behavior used by this module.
// FR: choosePlayerRace déclare ou implémente un comportement précis utilisé par ce module.
void Game::choosePlayerRace()
{
    struct RaceCreationGroup
    {
        std::string title;
        std::string hint;
        std::vector<CharacterRace> races;
    };

    const std::vector<RaceCreationGroup> groups = {
        {
            "Races classiques",
            "Humain, elfes, nain, gnome, halfelin et orc.",
            {
                CharacterRace::Human,
                CharacterRace::Elf,
                CharacterRace::DarkElf,
                CharacterRace::Dwarf,
                CharacterRace::Gnome,
                CharacterRace::Halfling,
                CharacterRace::Orc
            }
        },
        {
            "Races mystiques",
            "Origines marquées par la magie, la lumière, l'ombre ou les pactes.",
            {
                CharacterRace::Tiefling,
                CharacterRace::Aasimar,
                CharacterRace::Kitsune,
                CharacterRace::Fairy,
                CharacterRace::Vampire,
                CharacterRace::Demon
            }
        },
        {
            "Semi-humains",
            "Catégorie dédiée aux peuples semi-humains et à leurs sous-types animaux.",
            {
                CharacterRace::SemiHuman,
                CharacterRace::SemiWolf,
                CharacterRace::SemiFox,
                CharacterRace::SemiDog,
                CharacterRace::SemiCat,
                CharacterRace::SemiLizard,
                CharacterRace::SemiBird
            }
        },
        {
            "Hybrides rares",
            "Lignées moins communes, proches des semi-humains mais traitées à part.",
            {
                CharacterRace::HalfDragon
            }
        }
    };

    while (true)
    {
        MenuScreen categoryScreen("RACE", "character.creation.race.category");
        categoryScreen.addSubtitle("Choisis d'abord une catégorie");
        categoryScreen.addLine("La liste est séparée pour éviter les pages trop longues.");
        categoryScreen.addLine("Les semi-humains et leurs sous-types sont rangés ensemble.");

        for (std::size_t i = 0; i < groups.size(); ++i)
        {
            categoryScreen.addOption(
                static_cast<int>(i + 1),
                groups[i].title,
                groups[i].hint,
                true,
                "character.race.category",
                makeCreationItemData(
                    "race_category",
                    "Création - catégorie de race",
                    "select",
                    groups[i].title,
                    groups[i].hint,
                    "Catégorie",
                    std::to_string(groups[i].races.size()) + " choix",
                    "Page courte"
                )
            );
        }

        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Veuillez entrer un chiffre correspondant à une catégorie affichée."
        );
        Console::clear();

        const RaceCreationGroup& group = groups[static_cast<std::size_t>(categoryChoice - 1)];

        MenuScreen raceScreen("RACE — " + group.title, "character.creation.race");
        raceScreen.addSubtitle(group.hint);
        raceScreen.addLine("Retour permet de changer de catégorie avant de valider la race.");
        raceScreen.addBackOption("Changer de catégorie", "character.race.category.back");

        for (std::size_t i = 0; i < group.races.size(); ++i)
        {
            CharacterRace race = group.races[i];
            RaceStartingBonus bonus = RaceCatalog::getStartingBonus(race);

            std::ostringstream hint;
            hint << RaceCatalog::getGameplayIdentity(race)
                 << " | PV " << bonus.maxHpBonus
                 << " | Dégâts " << bonus.minDamageBonus << "/" << bonus.maxDamageBonus
                 << " | Critique " << bonus.criticalDamageBonus;

            if (RaceCatalog::hasInnateNightVision(race))
            {
                hint << " | Vision nocturne";
            }

            if (race == CharacterRace::Demon)
            {
                hint << " | Commerce tendu";
            }

            raceScreen.addOption(
                static_cast<int>(i + 1),
                characterRaceToText(race),
                hint.str(),
                true,
                "character.race.select",
                makeCreationItemData(
                    "race",
                    "Création - race",
                    "select",
                    characterRaceToText(race),
                    RaceCatalog::getShortDescription(race),
                    RaceCatalog::getGameplayIdentity(race),
                    "PV " + std::to_string(bonus.maxHpBonus)
                        + " | Dégâts " + std::to_string(bonus.minDamageBonus)
                        + "/" + std::to_string(bonus.maxDamageBonus)
                        + " | Critique " + std::to_string(bonus.criticalDamageBonus),
                    race == CharacterRace::Demon ? "Commerce plus tendu" : group.title,
                    race == CharacterRace::Demon
                )
            );
        }

        const int raceChoice = TerminalInterface::askMenuChoiceFromOptions(
            raceScreen,
            "Veuillez entrer un chiffre correspondant à une race affichée."
        );
        Console::clear();

        if (raceChoice == 0)
        {
            continue;
        }

        selectedRace = group.races[static_cast<std::size_t>(raceChoice - 1)];
        break;
    }

    MenuScreen confirmation("RACE VALIDÉE", "character.creation.race.confirmation");
    confirmation.setContinueInput("Valide pour continuer vers le choix de classe.");
    confirmation.addLine("Race sélectionnée : " + characterRaceToText(selectedRace) + ".");
    confirmation.addLine(RaceCatalog::getShortDescription(selectedRace));
    confirmation.addLine(RaceCatalog::getInnatePassiveLine(selectedRace));

    if (selectedRace == CharacterRace::Demon)
    {
        confirmation.addLine("Note commerce : certains marchands risquent de serrer les dents en te voyant arriver.");
        confirmation.addLine("Les prix pourront être plus élevés que la norme, surtout dans les villes peu habituées aux démons.");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: choosePlayerClass declares or implements a focused behavior used by this module.
// FR: choosePlayerClass déclare ou implémente un comportement précis utilisé par ce module.
void Game::choosePlayerClass()
{
    while (true)
    {
        MenuScreen categoryScreen("FAMILLE DE CLASSE", "character.creation.class.category");
        categoryScreen.addSubtitle("L'arène range les classes par style, sans te noyer dans une liste complète.");
        categoryScreen.addLine("Choisis une famille pour entrer dedans. Depuis la liste des classes, 0 permet de revenir ici.");

        const std::vector<ClassCategory> categories = ClassCatalog::getClassCategories();

        for (std::size_t i = 0; i < categories.size(); ++i)
        {
            ClassCategory category = categories[i];
            const int categoryChoice = static_cast<int>(i + 1);
            const std::string categoryName = classCategoryToText(category);
            const std::string categoryCount = std::to_string(ClassCatalog::getPlayableClassCountByCategory(category)) + " classes disponibles";
            const std::string categoryBrief = classCategoryBriefExample(category);

            categoryScreen.addOption(
                categoryChoice,
                categoryName,
                categoryCount + " | " + categoryBrief,
                true,
                "character.class.category.select",
                makeCreationItemData(
                    "class_category",
                    "Création - famille de classe",
                    "select",
                    categoryName,
                    categoryBrief,
                    categoryCount,
                    "Entre dans la famille puis reviens avec 0 si besoin"
                )
            );
        }

        int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Veuillez entrer un chiffre correspondant à une famille affichée."
        );

        Console::clear();

        const std::vector<ClassOptionInfo> classOptions = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);

        while (true)
        {
            MenuScreen classScreen("CLASSE", "character.creation.class.select");
            classScreen.addSubtitle("Famille sélectionnée : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
            classScreen.addLine("Tu peux revenir aux familles avec 0 si le groupe ne te plaît pas.");

            classScreen.addOption(
                0,
                "Retour aux familles de classe",
                "Revenir au choix du groupe sans valider de classe.",
                true,
                "character.class.category.back",
                makeCreationItemData(
                    "class_navigation",
                    "Création - classe",
                    "back",
                    "Retour aux familles",
                    "Changer de groupe de classe avant de valider le personnage.",
                    "Navigation",
                    "Aucune classe validée"
                )
            );

            for (std::size_t i = 0; i < classOptions.size(); ++i)
            {
                const ClassOptionInfo& info = classOptions[i];
                std::ostringstream hint;
                hint << info.role
                     << " | PV " << info.maxHp
                     << " | Dégâts " << info.minDamage << "-" << info.maxDamage
                     << " | Critique " << info.criticalDamage
                     << " | Potions " << info.healingPotionCount << "/" << info.damagePotionCount;

                classScreen.addOption(
                    static_cast<int>(i + 1),
                    info.name,
                    hint.str(),
                    true,
                    "character.class.select",
                    makeCreationItemData(
                        "class",
                        "Création - classe",
                        "select",
                        info.name,
                        info.role,
                        "Classe jouable",
                        "PV " + std::to_string(info.maxHp)
                            + " | Dégâts " + std::to_string(info.minDamage)
                            + "-" + std::to_string(info.maxDamage)
                            + " | Critique " + std::to_string(info.criticalDamage),
                        "Potions " + std::to_string(info.healingPotionCount) + "/" + std::to_string(info.damagePotionCount)
                    )
                );
            }

            int classChoice = TerminalInterface::askMenuChoiceFromOptions(
                classScreen,
                "Veuillez entrer un chiffre correspondant à une classe affichée, ou 0 pour revenir."
            );

            Console::clear();

            if (classChoice == 0)
            {
                break;
            }

            if (classChoice < 1 || static_cast<std::size_t>(classChoice) > classOptions.size())
            {
                continue;
            }

            const ClassOptionInfo& selectedInfo = classOptions[static_cast<std::size_t>(classChoice - 1)];
            bool stayOnClassAction = true;
            while (stayOnClassAction)
            {
                MenuScreen classActionScreen("CLASSE — " + selectedInfo.name, "character.creation.class.action");
                classActionScreen.addSubtitle("Inspecter ne valide rien. Sélectionner grave vraiment la classe.");
                const std::vector<std::string> previewLines = buildClassInspectionLines(selectedInfo);
                for (const std::string& line : previewLines)
                {
                    classActionScreen.addLine(line);
                }
                classActionScreen.addOption(0, "Retour à la liste", "Ne valide pas cette classe.", true, "character.class.action.back");
                classActionScreen.addOption(1, "Inspecter", "Voir clairement ce que la classe possède de base et ce qu'elle pourrait gagner plus tard.", true, "character.class.inspect");
                classActionScreen.addOption(2, "Sélectionner", "Valider cette classe pour le personnage.", true, "character.class.confirm");

                const int actionChoice = TerminalInterface::askMenuChoiceFromOptions(
                    classActionScreen,
                    "Choisis Inspecter, Sélectionner ou Retour."
                );
                Console::clear();

                if (actionChoice == 0)
                {
                    stayOnClassAction = false;
                    continue;
                }

                if (actionChoice == 1)
                {
                    MessageScreen::show(
                        "INSPECTION — " + selectedInfo.name,
                        "character.creation.class.inspect",
                        buildClassInspectionLines(selectedInfo),
                        false
                    );
                    Console::waitForEnter();
                    Console::clear();
                    continue;
                }

                if (actionChoice != 2)
                {
                    continue;
                }

                PlayerClass chosenClass = ClassCatalog::createClassByCategoryChoice(
                    categoryChoice,
                    classChoice
                );

                mainPlayer = Player(playerName, chosenClass);
                mainPlayer.setRace(selectedRace);

                bool nativeBonusApplied = SpecialCharacterNativeBonus::applyIfNativeMatch(mainPlayer);

                mainPlayer.initializeStarterInventory(selectedDifficulty);

                Console::clear();

                MenuScreen confirmation("PERSONNAGE GRAVÉ", "character.creation.summary");
                confirmation.setContinueInput("Valide pour entrer dans le jeu avec ce personnage.");
                confirmation.addLine(playerName + ", tu as choisi : " + characterRaceToText(selectedRace) + " / " + chosenClass.getName() + ".");
                confirmation.addLine("Famille : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
                confirmation.addLine(RaceCatalog::getInnatePassiveLine(selectedRace));
                confirmation.addLine("Difficulté : " + getDifficultyName() + ".");
                confirmation.addLine("Règle de mort : " + getDeathRuleName() + ".");
                confirmation.addLine("Tes statistiques ont été gravées dans l'arène avec succès.");
                confirmation.addLine("Ton équipement et tes ressources de départ ont été adaptés à la difficulté.");
                confirmation.addLine("Créé le " + mainPlayer.getCreatedAtText() + " V" + mainPlayer.getCreatedForVersion());
                confirmation.addLine("Dernière adaptation faite pour la V" + mainPlayer.getLastAdaptedVersion());

                if (nativeBonusApplied)
                {
                    confirmation.addLine("Bonus natif : actif.");
                }

                TerminalInterface::renderMenuScreen(confirmation, false);
                mainPlayer.displayStats();
                mainPlayer.displaySimpleEquipment();

                saveCurrentProgress("Création du personnage");

                Console::waitForEnter();
                Console::clear();
                return;
            }
        }
    }
}


bool Game::isMultiplayerSession() const
{
    return partyPlayers.size() > 1;
}

std::vector<Player*> Game::getActivePartyPointers()
{
    std::vector<Player*> party;
    party.push_back(&mainPlayer);
    for (Player& player : partyPlayers)
    {
        party.push_back(&player);
    }
    return party;
}

void Game::savePartyProgress(const std::string& reason) const
{
    if (ephemeralSandboxSession)
    {
        MessageScreen::show(
            "CLONE ÉPHÉMÈRE",
            "save.party.ephemeral_skipped",
            {
                "Session de bac à sable éphémère : aucune progression réelle n’est sauvegardée.",
                "Le personnage histoire original restera intact."
            },
            false
        );
        return;
    }

    if (mainPlayer.isDead() && DifficultyRules::isPermanentDeath(selectedDifficulty, selectedDeathRule))
    {
        SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty, selectedDeathRule);
        if (SaveManager::movePlayableCharacterToDead(accountName, mainPlayer.getName()))
        {
            MessageScreen::show(
                "REGISTRE LÉTHAL",
                "save.party.lethal.main_removed",
                {"Le registre Léthal retire " + mainPlayer.getName() + " des personnages jouables de " + accountName + "."},
                false
            );
        }
        return;
    }

    saveCurrentProgress(reason);

    for (std::size_t i = 0; i < partyPlayers.size(); ++i)
    {
        if (i >= partyAccountNames.size() || i >= partyDifficulties.size())
        {
            continue;
        }

        const Player& partyPlayer = partyPlayers[i];
        const std::string& ownerAccount = partyAccountNames[i];
        DifficultyMode playerDifficulty = partyDifficulties[i];
        DeathRuleMode playerDeathRule = i < partyDeathRules.size()
            ? partyDeathRules[i]
            : DeathRuleRules::defaultForDifficulty(playerDifficulty);

        if (partyPlayer.isDead() && DifficultyRules::isPermanentDeath(playerDifficulty, playerDeathRule))
        {
            SaveManager::savePlayerSnapshot(partyPlayer, ownerAccount, playerDifficulty, playerDeathRule);
            if (SaveManager::movePlayableCharacterToDead(ownerAccount, partyPlayer.getName()))
            {
                MessageScreen::show(
                    "REGISTRE LÉTHAL",
                    "save.party.lethal.member_removed",
                    {"Le registre Léthal retire " + partyPlayer.getName() + " des personnages jouables de " + ownerAccount + "."},
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "save.party.lethal.member_refused",
                    {"Le registre des morts refuse d'emporter " + partyPlayer.getName() + " dans le registre des morts."},
                    false
                );
            }
            continue;
        }

        SaveManager::savePlayerSnapshot(partyPlayer, ownerAccount, playerDifficulty, playerDeathRule);
    }
}

bool Game::addSecondaryPlayerToParty(int playerNumber)
{
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();
    std::vector<AccountSaveSummary> availableAccounts;

    for (const AccountSaveSummary& account : accounts)
    {
        if (account.accountName == accountName)
        {
            continue;
        }

        bool alreadyUsed = false;
        for (const std::string& usedAccount : partyAccountNames)
        {
            if (usedAccount == account.accountName)
            {
                alreadyUsed = true;
                break;
            }
        }

        if (!alreadyUsed)
        {
            availableAccounts.push_back(account);
        }
    }

    if (availableAccounts.empty())
    {
        MenuScreen emptyScreen("JOUEUR " + std::to_string(playerNumber), "session.party.secondary.no_account");
        emptyScreen.addLine("Aucun autre compte local disponible pour le joueur " + std::to_string(playerNumber) + ".");
        emptyScreen.addLine("La coop nécessite des comptes différents, et donc des personnages différents.");
        TerminalInterface::renderMenuScreen(emptyScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    constexpr std::size_t accountsPerPage = 10;
    std::size_t accountPage = 0;
    std::string secondaryAccount;

    while (secondaryAccount.empty())
    {
        const std::size_t totalItems = availableAccounts.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, accountsPerPage);
        const std::size_t first = PagedMenu::firstIndex(accountPage, accountsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, accountPage, accountsPerPage);

        MenuScreen accountScreen("JOUEUR " + std::to_string(playerNumber), "session.party.secondary.account");
        accountScreen.addSubtitle("Compte du joueur " + std::to_string(playerNumber));
        accountScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            accountScreen.addOption(
                static_cast<int>(i - first + 1),
                availableAccounts[i].accountName,
                "Compte local disponible pour cette session.",
                true,
                "session.party.account.select"
            );
        }

        PagedMenu::addNavigationOptions(accountScreen, accountPage, totalPages);

        int accountChoice = TerminalInterface::askMenuChoiceFromOptions(
            accountScreen,
            "Choisis un compte affiché."
        );
        Console::clear();

        if (accountChoice == 0)
        {
            return false;
        }

        if (accountChoice == 98 && accountPage > 0)
        {
            --accountPage;
            continue;
        }

        if (accountChoice == 99 && accountPage + 1 < totalPages)
        {
            ++accountPage;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (accountChoice >= 1 && accountChoice <= visibleCount)
        {
            secondaryAccount = availableAccounts[first + static_cast<std::size_t>(accountChoice - 1)].accountName;
        }
    }

    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(secondaryAccount);

    if (characters.empty())
    {
        MenuScreen emptyCharacterScreen("PERSONNAGE JOUEUR " + std::to_string(playerNumber), "session.party.secondary.no_character");
        emptyCharacterScreen.addLine("Ce compte n'a aucun personnage jouable.");
        emptyCharacterScreen.addLine("Compte choisi : " + secondaryAccount + ".");
        TerminalInterface::renderMenuScreen(emptyCharacterScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    constexpr std::size_t charactersPerPage = 8;
    std::size_t characterPage = 0;
    CharacterSaveSummary summary;
    bool characterSelected = false;

    while (!characterSelected)
    {
        const std::size_t totalItems = characters.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, charactersPerPage);
        const std::size_t first = PagedMenu::firstIndex(characterPage, charactersPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, characterPage, charactersPerPage);

        MenuScreen characterScreen("PERSONNAGE JOUEUR " + std::to_string(playerNumber), "session.party.secondary.character");
        characterScreen.addSubtitle("Compte : " + secondaryAccount);
        characterScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            const CharacterSaveSummary& character = characters[i];
            std::string label = character.characterName
                + " | " + character.raceName
                + " / " + character.className
                + " | Niveau " + std::to_string(character.level);

            characterScreen.addOption(
                static_cast<int>(i - first + 1),
                label,
                "Maître : " + character.currentOwnerAccountName,
                true,
                "session.party.character.select"
            );
        }

        PagedMenu::addNavigationOptions(characterScreen, characterPage, totalPages);

        int characterChoice = TerminalInterface::askMenuChoiceFromOptions(
            characterScreen,
            "Choisis un personnage affiché."
        );
        Console::clear();

        if (characterChoice == 0)
        {
            return false;
        }

        if (characterChoice == 98 && characterPage > 0)
        {
            --characterPage;
            continue;
        }

        if (characterChoice == 99 && characterPage + 1 < totalPages)
        {
            ++characterPage;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (characterChoice >= 1 && characterChoice <= visibleCount)
        {
            summary = characters[first + static_cast<std::size_t>(characterChoice - 1)];
            characterSelected = true;
        }
    }

    if (summary.currentOwnerAccountName != secondaryAccount || summary.accountName != secondaryAccount)
    {
        MenuScreen refusedScreen("MAÎTRISE REFUSÉE", "session.party.secondary.owner_refused");
        refusedScreen.addLine("Le fil de maîtrise refuse ce chargement.");
        refusedScreen.addLine("Un personnage n'a qu'un seul maître.");
        refusedScreen.addLine("Maître inscrit : " + summary.currentOwnerAccountName);
        refusedScreen.addLine("Compte choisi : " + secondaryAccount);
        TerminalInterface::renderMenuScreen(refusedScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    Player secondaryPlayer;
    DifficultyMode secondaryDifficulty = DifficultyMode::Normal;
    DeathRuleMode secondaryDeathRule = DeathRuleRules::defaultForDifficulty(secondaryDifficulty);

    if (!SaveManager::loadPlayerSnapshot(summary, secondaryPlayer, secondaryDifficulty, secondaryDeathRule))
    {
        MenuScreen errorScreen("CHARGEMENT IMPOSSIBLE", "session.party.secondary.load_failed");
        errorScreen.addLine("Impossible de charger ce personnage.");
        TerminalInterface::renderMenuScreen(errorScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    partyAccountNames.push_back(secondaryAccount);
    partyDifficulties.push_back(secondaryDifficulty);
    partyDeathRules.push_back(secondaryDeathRule);
    partyPlayers.push_back(secondaryPlayer);

    MenuScreen successScreen("JOUEUR AJOUTÉ", "session.party.secondary.added");
    successScreen.addSubtitle("Résumé du joueur secondaire");
    successScreen.addLine("Joueur " + std::to_string(playerNumber) + " ajouté : " + secondaryPlayer.getName() + " (" + secondaryAccount + ").");
    successScreen.addLine("Race / classe : " + secondaryPlayer.getRaceText() + " / " + secondaryPlayer.getType());
    successScreen.addLine("Difficulté personnelle : " + difficultyModeLabel(secondaryDifficulty));
    successScreen.addLine("Règle de mort : " + DeathRuleRules::displayName(secondaryDeathRule));
    successScreen.addLine("PV : " + std::to_string(secondaryPlayer.getHp()) + "/" + std::to_string(secondaryPlayer.getMaxHp()));
    TerminalInterface::renderMenuScreen(successScreen, false);
    Console::waitForEnter();
    Console::clear();
    return true;
}

void Game::configurePartyMode()
{
    partyPlayers.clear();
    partyAccountNames.clear();
    partyDifficulties.clear();
    partyDeathRules.clear();

    MenuScreen screen("SESSION", "session.party.mode");
    screen.addSubtitle("Le joueur 1 reste le point d'ancrage de la partie.");
    screen.addOption(
        1,
        "Solo",
        "Un seul personnage actif.",
        true,
        "session.solo",
        makeSessionItemData(mainPlayer, "session", "Solo", "Un seul personnage actif.", "Classique", "Difficulté : " + getDifficultyName())
    );
    screen.addOption(
        2,
        "Multi local - 2 joueurs",
        "Un allié joueur intervient surtout en combat et récompenses individuelles.",
        true,
        "session.coop.2",
        makeSessionItemData(mainPlayer, "session", "Multi local - 2 joueurs", "Un allié joueur intervient surtout en combat.", "Coop locale", "Récompenses individuelles", true)
    );
    screen.addOption(
        3,
        "Multi local - 3 joueurs",
        "Deux alliés joueurs avec inventaires et récompenses séparés.",
        true,
        "session.coop.3",
        makeSessionItemData(mainPlayer, "session", "Multi local - 3 joueurs", "Deux alliés joueurs rejoignent surtout les combats.", "Coop locale", "Récompenses individuelles", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une session affichée.");
    Console::clear();

    if (choice == 1)
    {
        MenuScreen confirmation("SESSION SOLO", "session.party.confirmation.solo");
        confirmation.setContinueInput("Valide pour ouvrir les activités disponibles.");
        confirmation.addSubtitle("Joueur actif");
        confirmation.addLine("Session solo sélectionnée.");
        confirmation.addLine("Personnage : " + mainPlayer.getName());
        confirmation.addLine("Race / classe : " + mainPlayer.getRaceText() + " / " + mainPlayer.getType());
        confirmation.addLine("Difficulté : " + getDifficultyName());
        confirmation.addLine("Règle de mort : " + getDeathRuleName());
        confirmation.addLine("PV : " + std::to_string(mainPlayer.getHp()) + "/" + std::to_string(mainPlayer.getMaxHp()));
        confirmation.addLine("Or : " + std::to_string(mainPlayer.getInventory().getGold()) + " pièces");
        confirmation.addLine("Prochaine étape : activités disponibles.");
        TerminalInterface::renderMenuScreen(confirmation, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen coopIntro("SESSION COOP", "session.party.confirmation.coop");
    coopIntro.setContinueInput("Valide pour choisir les autres joueurs.");
    coopIntro.addSubtitle("Règle de session");
    coopIntro.addLine("Le joueur 1 reste le point d'ancrage : voyage, boss, niveau de session, événements et monstres.");
    coopIntro.addLine("Joueur 1 : " + mainPlayer.getName() + " | " + mainPlayer.getRaceText() + " / " + mainPlayer.getType() + " | " + getDifficultyName() + " | " + getDeathRuleName());
    coopIntro.addLine("Les autres joueurs interviennent surtout en combat, avec leur inventaire et leurs récompenses individuelles.");
    coopIntro.addLine("Boss coop : tous les joueurs doivent avoir l'accès requis.");
    TerminalInterface::renderMenuScreen(coopIntro, false);
    Console::waitForEnter();
    Console::clear();

    for (int playerNumber = 2; playerNumber <= choice; ++playerNumber)
    {
        if (!addSecondaryPlayerToParty(playerNumber))
        {
            MessageScreen::show(
                "SESSION COOP",
                "session.party.partial",
                {"La session repasse sur les joueurs déjà validés."}
            );
            break;
        }
    }

    MenuScreen result("GROUPE", "session.party.result");
    result.setContinueInput("Valide pour ouvrir les activités disponibles.");

    if (partyPlayers.empty())
    {
        result.addSubtitle("Retour solo");
        result.addLine("Aucun joueur secondaire validé. Session solo conservée.");
        result.addLine("Personnage : " + mainPlayer.getName());
        result.addLine("Difficulté : " + getDifficultyName());
        result.addLine("Règle de mort J1 : " + getDeathRuleName());
    }
    else
    {
        result.addSubtitle("Groupe validé");
        result.addLine("Groupe actif : " + std::to_string(partyPlayers.size() + 1) + " joueurs.");
        result.addLine("- J1 " + mainPlayer.getName() + " | " + mainPlayer.getRaceText() + " / " + mainPlayer.getType() + " | " + getDifficultyName() + " | " + getDeathRuleName());
        for (std::size_t i = 0; i < partyPlayers.size(); ++i)
        {
            result.addLine(
                "- J" + std::to_string(i + 2) + " " + partyPlayers[i].getName()
                + " | " + partyPlayers[i].getRaceText()
                + " / " + partyPlayers[i].getType()
                + " | " + difficultyModeLabel(partyDifficulties[i])
                + " | " + (i < partyDeathRules.size() ? DeathRuleRules::displayName(partyDeathRules[i]) : DeathRuleRules::displayName(DeathRuleRules::defaultForDifficulty(partyDifficulties[i])))
            );
        }
    }

    TerminalInterface::renderMenuScreen(result, false);
    Console::waitForEnter();
    Console::clear();
}

// EN: chooseGameMode declares or implements a focused behavior used by this module.
// FR: chooseGameMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::chooseGameMode()
{
    while (true)
    {
        const bool combatQuestLikely = hasLikelyCombatQuest(mainPlayer);
        const bool explorationQuestLikely = hasLikelyExplorationQuest(mainPlayer);
        const bool questHubLikely = hasLikelyQuestHubObjective(mainPlayer);
        const bool locationNpcQuestLikely = hasLikelyLocationOrNpcQuest(mainPlayer);

        MenuScreen screen("ACTIVITÉS", "activity.main");
        screen.addSubtitle("Choisis une catégorie, puis une action précise.");
        screen.addLine("Date : " + mainPlayer.formatWorldDateLine() + " | Moment : " + mainPlayer.formatWorldDayPartLine());
        screen.addLine("Astuce : les quêtes d'exploration passent par Exploration. Les lieux visitables servent surtout à la ville, aux boutiques et aux PNJ.");
        screen.addOption(
            1,
            "Histoire",
            "Route principale guidée : prologue, ville, quêtes principales et chapitres.",
            true,
            "activity.story",
            makeActivityItemData("Route principale", "story", "Histoire", "Mode guidé séparé du bac à sable : peu de boutiques/quêtes au départ, puis déblocages par chapitres.", mainPlayer.getStoryProgressLabel(), "Chapitre 1/2", true)
        );
        screen.addOption(
            2,
            "Terrain : Combats / Exploration" + questActivityTag(combatQuestLikely || explorationQuestLikely),
            "Combats PvE/PvP, boss, biomes, plantes, matériaux, coffres et rencontres." + questActivityTag(combatQuestLikely || explorationQuestLikely),
            true,
            "activity.terrain",
            makeActivityItemData("Terrain", "travel", "Combats / Exploration", "Catégorie terrain : combats volontaires et sorties d'exploration hors ville.", (combatQuestLikely || explorationQuestLikely) ? "Quête probable" : "Disponible", "Combat ou exploration", true)
        );
        screen.addOption(
            3,
            "Ville : Quêtes / PNJ / Lieux" + questActivityTag(questHubLikely || locationNpcQuestLikely),
            "Journal, guilde, PNJ notables, forge, herboristerie, bibliothèque et boutiques." + questActivityTag(questHubLikely || locationNpcQuestLikely),
            true,
            "activity.city",
            makeActivityItemData("Ville", "quest", "Quêtes / PNJ / Lieux", "Catégorie ville : journal, demandes, personnages, boutiques et services.", (questHubLikely || locationNpcQuestLikely) ? "Quête probable" : "Disponible", "Ville / progression", questHubLikely || locationNpcQuestLikely)
        );
        screen.addOption(
            4,
            "Gestion",
            "Récap après-combat, statistiques, inventaire via accès rapide, sauvegarde et paramètres.",
            true,
            "activity.management",
            makeActivityItemData("Gestion", "inspect", "Gestion", "Actions de confort : récap, inventaire, sauvegarde, paramètres et état du personnage.", "Disponible", "Hub de gestion", true)
        );
        screen.addOption(
            5,
            "Infos utiles / aide",
            "Journées, argent, quêtes, exploration et catégories sans noyer la page principale.",
            true,
            "activity.info",
            makeActivityItemData("Aide", "inspect", "Infos utiles", "Guide court des routes jouables, temps, argent, quêtes et lieux.", "Aide", "Lecture")
        );
        addOutOfCombatUtilityOptions(screen, true, true);

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir une catégorie affichée."
        );

        Console::clear();

        if (handleOutOfCombatUtilityChoice(choice, true))
        {
            continue;
        }

        if (choice == 1)
        {
            selectedMode = GameMode::Story;
            return;
        }

        if (choice == 2)
        {
            bool terrainOpen = true;
            while (terrainOpen)
            {
                const bool monsterQuestLikely = hasLikelyCombatQuest(mainPlayer);
                const bool bossQuestLikely = hasLikelyBossQuest(mainPlayer);
                MenuScreen terrainScreen("TERRAIN", "activity.terrain.menu");
                terrainScreen.addSubtitle("Combattre ou sortir explorer");
                terrainScreen.addLine("Les quêtes d'exploration utilisent cette catégorie, même quand elles ont été données par un PNJ.");
                terrainScreen.addBackOption();
                terrainScreen.addOption(
                    1,
                    "Combats" + questActivityTag(monsterQuestLikely || bossQuestLikely),
                    "PvP IA, JcJ local, PvE monstres et boss." + questActivityTag(monsterQuestLikely || bossQuestLikely),
                    true,
                    "activity.terrain.combat",
                    makeActivityItemData("Terrain", "combat", "Combats", "Duel IA, JcJ local, monstres, groupes et boss.", (monsterQuestLikely || bossQuestLikely) ? "Quête probable" : "Disponible", "Combat volontaire", true)
                );
                terrainScreen.addOption(
                    2,
                    "Exploration" + questActivityTag(explorationQuestLikely),
                    "Biomes, plantes, matériaux, coffres, pièges, mimics et rencontres imprévues." + questActivityTag(explorationQuestLikely),
                    true,
                    "activity.terrain.exploration",
                    makeActivityItemData("Terrain", "travel", "Exploration", "Sorties par biome avec risques, ressources, coffres, pièges, mimics et rencontres imprévues.", explorationQuestLikely ? "Quête probable" : "Disponible", "Sortie terrain", true)
                );

                const int terrainChoice = TerminalInterface::askMenuChoiceFromOptions(
                    terrainScreen,
                    "Veuillez choisir une action de terrain affichée."
                );
                Console::clear();

                if (terrainChoice == 0)
                {
                    terrainOpen = false;
                    continue;
                }

                if (terrainChoice == 2)
                {
                    selectedMode = GameMode::Exploration;
                    return;
                }

                if (terrainChoice == 1)
                {
                    bool combatOpen = true;
                    while (combatOpen)
                    {
                        const bool currentMonsterQuestLikely = hasLikelyCombatQuest(mainPlayer);
                        const bool currentBossQuestLikely = hasLikelyBossQuest(mainPlayer);
                        MenuScreen combatScreen("COMBATS", "activity.combat.menu");
                        combatScreen.addSubtitle("Choisis le type de combat");
                        combatScreen.addBackOption();
                        combatScreen.addOption(
                            1,
                            "PvP IA",
                            "Duel contre une IA, avec personnages spéciaux possibles selon le mode.",
                            true,
                            "combat.ai_pvp",
                            makeActivityItemData("Combats", "combat", "PvP IA", "Duel contre une IA, avec personnages spéciaux possibles selon le mode.", "Disponible", "Combat volontaire")
                        );
                        combatScreen.addOption(
                            2,
                            "PvP 2 joueurs / JcJ",
                            "Duel local amical ou mortel selon les comptes, clones, altérations et difficultés.",
                            true,
                            "combat.local_pvp",
                            makeActivityItemData("Combats", "combat", "PvP 2 joueurs / JcJ", "Duel local amical ou mortel selon les comptes, clones, altérations et difficultés.", "Disponible", "Combat volontaire")
                        );
                        combatScreen.addOption(
                            3,
                            "PvE monstres" + questActivityTag(currentMonsterQuestLikely),
                            "Vagues de monstres, loots, matériaux, qualité de récupération et progression." + questActivityTag(currentMonsterQuestLikely),
                            true,
                            "combat.monster_pve",
                            makeActivityItemData("Combats", "combat", "PvE monstres", "Vagues de monstres, loots, matériaux, qualité de récupération et progression.", currentMonsterQuestLikely ? "Quête probable" : "Disponible", "Farm / progression", true)
                        );
                        combatScreen.addOption(
                            4,
                            "PvE Boss" + questActivityTag(currentBossQuestLikely),
                            "Combat contre un boss avec identité, décryptage et fragments spéciaux." + questActivityTag(currentBossQuestLikely),
                            true,
                            "combat.boss_pve",
                            makeActivityItemData("Combats", "combat", "PvE Boss", "Combat contre un boss avec identité, décryptage et fragments spéciaux. La fuite de boss reste impossible.", currentBossQuestLikely ? "Quête probable" : "Dangereux", "Boss / fragments", true)
                        );

                        const int combatChoice = TerminalInterface::askMenuChoiceFromOptions(
                            combatScreen,
                            "Veuillez choisir un combat affiché."
                        );
                        Console::clear();

                        if (combatChoice == 0)
                        {
                            combatOpen = false;
                            continue;
                        }

                        switch (combatChoice)
                        {
                            case 1:
                                selectedMode = GameMode::AIPvp;
                                return;
                            case 2:
                                selectedMode = GameMode::TwoPlayerPvp;
                                return;
                            case 3:
                                selectedMode = GameMode::MonsterPve;
                                return;
                            case 4:
                                selectedMode = GameMode::BossPve;
                                return;
                            default:
                                break;
                        }
                    }
                }
            }
            continue;
        }

        if (choice == 3)
        {
            MenuScreen cityScreen("VILLE", "activity.city.menu");
            cityScreen.addSubtitle("Quêtes, PNJ et services de ville");
            cityScreen.addLine("Les lieux visitables ne remplacent pas Exploration : ils servent surtout aux services, boutiques et dialogues.");
            cityScreen.addBackOption();
            cityScreen.addOption(
                1,
                "Quêtes" + questActivityTag(questHubLikely),
                "Quête principale, journal, guilde, demandes de PNJ et validations." + questActivityTag(questHubLikely),
                true,
                "activity.city.quests",
                makeActivityItemData("Ville", "quest", "Quêtes", "Accès au journal, à la guilde, aux demandes de PNJ et aux validations d'objectifs.", questHubLikely ? "Quête probable" : "Disponible", "Ville / progression", questHubLikely)
            );
            cityScreen.addOption(
                2,
                "PNJ notables" + questActivityTag(locationNpcQuestLikely),
                "Parler aux clients et personnages disponibles sans passer par une boutique." + questActivityTag(locationNpcQuestLikely),
                true,
                "activity.city.npcs",
                makeActivityItemData("Ville", "talk", "PNJ notables", "Discussion directe avec les clients ou personnages disponibles, sans forcer une boutique.", locationNpcQuestLikely ? "Quête probable" : "Disponible", "Ville / dialogues", locationNpcQuestLikely)
            );
            cityScreen.addOption(
                3,
                "Boutiques / lieux visitables" + questActivityTag(locationNpcQuestLikely),
                "Forge, herboristerie, bibliothèque, vendeurs et lieux sociaux." + questActivityTag(locationNpcQuestLikely),
                true,
                "activity.city.locations",
                makeActivityItemData("Ville", "travel", "Boutiques / lieux visitables", "Forge, herboristerie, bibliothèque, vendeurs et lieux sociaux centralisés hors combat.", locationNpcQuestLikely ? "Quête probable" : "Disponible", "Ville / économie", locationNpcQuestLikely)
            );
            cityScreen.addOption(
                4,
                "Échange / don",
                "Transférer des ressources entre personnages compatibles.",
                true,
                "activity.city.exchange",
                makeActivityItemData("Ville", "barter", "Échange / don", "Transfert protégé d'objets, matériaux ou or entre personnages compatibles.", "Disponible", "Gestion de compte")
            );

            const int cityChoice = TerminalInterface::askMenuChoiceFromOptions(
                cityScreen,
                "Veuillez choisir une action de ville affichée."
            );
            Console::clear();

            if (cityChoice == 0)
            {
                continue;
            }
            if (cityChoice == 1)
            {
                selectedMode = GameMode::Challenges;
                return;
            }
            if (cityChoice == 2)
            {
                selectedMode = GameMode::NotableNpcs;
                return;
            }
            if (cityChoice == 3)
            {
                selectedMode = GameMode::Locations;
                return;
            }
            if (cityChoice == 4)
            {
                selectedMode = GameMode::Exchange;
                return;
            }
            continue;
        }

        if (choice == 4)
        {
            MenuScreen managementScreen("GESTION", "activity.management.menu");
            managementScreen.addSubtitle("Actions entre deux sorties");
            managementScreen.addBackOption();
            managementScreen.addOption(
                1,
                "Gestion après-combat",
                "Récap, statistiques, équipement, potions, progression et actions entre deux combats.",
                true,
                "activity.management.post_combat",
                makeActivityItemData("Gestion", "inspect", "Gestion après-combat", "Récap, statistiques, inventaire, équipement, lieux, quêtes et actions entre deux combats.", "Disponible", "Hub de gestion", true)
            );
            managementScreen.addOption(
                2,
                "Échange / don",
                "Transférer des ressources entre personnages compatibles.",
                true,
                "activity.management.exchange",
                makeActivityItemData("Gestion", "barter", "Échange / don", "Transfert protégé d'objets, matériaux ou or entre personnages compatibles.", "Disponible", "Gestion de compte")
            );

            const int managementChoice = TerminalInterface::askMenuChoiceFromOptions(
                managementScreen,
                "Veuillez choisir une action de gestion affichée."
            );
            Console::clear();

            if (managementChoice == 0)
            {
                continue;
            }
            if (managementChoice == 1)
            {
                const bool keepPlaying = openPostCombatMenu();
                if (!keepPlaying)
                {
                    std::exit(0);
                }
                continue;
            }
            if (managementChoice == 2)
            {
                selectedMode = GameMode::Exchange;
                return;
            }
            continue;
        }

        if (choice == 5)
        {
            displayActivityInformation();
            continue;
        }
    }
}

// EN: displaySelectedMode declares or implements a focused behavior used by this module.
// FR: displaySelectedMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::displaySelectedMode()
{
    Console::clear();

    MenuScreen screen("ACTIVITÉ SÉLECTIONNÉE", "activity.selected");
    screen.setContinueInput("Valide pour lancer cette activité.");
    screen.addLine("Activité : " + getSelectedModeName());
    screen.addLine("Date actuelle : " + mainPlayer.formatWorldDateLine());
    screen.addLine("Moment actuel : " + mainPlayer.formatWorldDayPartLine());
    screen.addLine("Difficulté : " + getDifficultyName());
    screen.addLine("Règle de mort : " + getDeathRuleName());

    if (isMultiplayerSession())
    {
        screen.addLine("Groupe actif : " + std::to_string(partyPlayers.size() + 1) + " joueurs.");
    }

    TerminalInterface::renderMenuScreen(screen, false);
    Console::waitForEnter();
    Console::clear();
}


std::string Game::getSelectedModeName() const
{
    switch (selectedMode)
    {
        case GameMode::Story:
            return "Histoire";
        case GameMode::AIPvp:
            return "Combat - PvP IA";
        case GameMode::TwoPlayerPvp:
            return "Combat - PvP 2 joueurs / JcJ";
        case GameMode::MonsterPve:
            return "Combat - PvE monstres";
        case GameMode::BossPve:
            return "Combat - PvE Boss";
        case GameMode::Challenges:
            return "Quêtes";
        case GameMode::Exploration:
            return "Exploration";
        case GameMode::Locations:
            return "Boutiques / lieux visitables";
        case GameMode::NotableNpcs:
            return "PNJ notables";
        case GameMode::Exchange:
            return "Échange / don";
    }

    return "Activité inconnue";
}

Game::CombatRecapSnapshot Game::captureCombatRecapSnapshot() const
{
    CombatRecapSnapshot snapshot;
    snapshot.level = mainPlayer.getLevel();
    snapshot.experience = mainPlayer.getExperience();
    snapshot.hp = mainPlayer.getHp();
    snapshot.maxHp = mainPlayer.getMaxHp();
    snapshot.gold = mainPlayer.getInventory().getGold();
    snapshot.victories = mainPlayer.getVictories();
    snapshot.defeats = mainPlayer.getDefeats();
    snapshot.escapes = mainPlayer.getEscapes();
    snapshot.enemiesKilled = mainPlayer.getEnemiesKilled();
    snapshot.bossesKilled = mainPlayer.getBossesKilled();
    return snapshot;
}

void Game::updateLastCombatRecap(const CombatRecapSnapshot& beforeSnapshot)
{
    lastCombatRecap.available = true;
    lastCombatRecap.modeName = getSelectedModeName();
    lastCombatRecap.difficultyName = getDifficultyName();
    lastCombatRecap.before = beforeSnapshot;
    lastCombatRecap.after = captureCombatRecapSnapshot();
}

void Game::displayLastCombatRecap() const
{
    if (!lastCombatRecap.available)
    {
        MessageScreen::show(
            "DERNIER RÉCAP",
            "post_combat.last_recap.empty",
            {
                "Aucun combat récent enregistré dans cette session.",
                "Lance un combat pour que le registre compare l'avant et l'après."
            }
        );
        return;
    }

    const CombatRecapSnapshot& before = lastCombatRecap.before;
    const CombatRecapSnapshot& after = lastCombatRecap.after;

    MessageScreen::show(
        "DERNIER RÉCAP DE COMBAT",
        "post_combat.last_recap.detail",
        {
            "Activité : " + lastCombatRecap.modeName,
            "Difficulté : " + lastCombatRecap.difficultyName,
            "",
            "Avant : niveau " + std::to_string(before.level)
                + " | XP " + std::to_string(before.experience)
                + " | PV " + std::to_string(before.hp) + "/" + std::to_string(before.maxHp)
                + " | Or " + std::to_string(before.gold),
            "Après : niveau " + std::to_string(after.level)
                + " | XP " + std::to_string(after.experience)
                + " | PV " + std::to_string(after.hp) + "/" + std::to_string(after.maxHp)
                + " | Or " + std::to_string(after.gold),
            "",
            "Variations :",
            "- Niveau : " + std::to_string(after.level - before.level),
            "- Expérience : " + std::to_string(after.experience - before.experience),
            "- PV actuels : " + std::to_string(after.hp - before.hp),
            "- PV max : " + std::to_string(after.maxHp - before.maxHp),
            "- Or : " + std::to_string(after.gold - before.gold),
            "- Victoires : " + std::to_string(after.victories - before.victories),
            "- Défaites : " + std::to_string(after.defeats - before.defeats),
            "- Fuites : " + std::to_string(after.escapes - before.escapes),
            "- Ennemis vaincus : " + std::to_string(after.enemiesKilled - before.enemiesKilled),
            "- Boss vaincus : " + std::to_string(after.bossesKilled - before.bossesKilled)
        }
    );
}

// EN: displayActivityInformation declares or implements a focused behavior used by this module.
// FR: displayActivityInformation déclare ou implémente un comportement précis utilisé par ce module.
void Game::displayActivityInformation() const
{
    while (true)
    {
        MenuScreen screen("INFOS UTILES", "activity.info");
        screen.addSubtitle("Choisis un sujet court au lieu de tout lire d'un coup.");
        screen.addLine("Date : " + mainPlayer.formatWorldDateLine() + " | Moment : " + mainPlayer.formatWorldDayPartLine());
        screen.addLine("Or actuel : " + std::to_string(mainPlayer.getInventory().getGold()) + " pièce(s).");
        screen.addBackOption("Retour", "activity.info.back");
        screen.addOption(1, "Où aller ?", "Rappel des grandes catégories du menu principal.", true, "activity.info.where");
        screen.addOption(2, "Journées / temps", "Comprendre les jours, moments et conséquences des activités.", true, "activity.info.time");
        screen.addOption(3, "Argent / économie", "Or, boutiques, stocks, prix, revente et récompenses.", true, "activity.info.money");
        screen.addOption(4, "Combat / exploration", "Différence entre combat volontaire, boss et sortie de terrain.", true, "activity.info.terrain");
        screen.addOption(5, "Quêtes / rendre objectifs", "Où voir les quêtes prêtes, principales, secondaires et rendues.", true, "activity.info.quests");
        screen.addOption(6, "Ville / PNJ / lieux", "PNJ notables, lieux visitables, boutiques et services.", true, "activity.info.city");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide. Choisis un sujet affiché.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            MessageScreen::show(
                "OÙ ALLER ?",
                "activity.info.where.detail",
                {
                    "Histoire : route principale guidée et quêtes non refusables.",
                    "Terrain : combats volontaires et exploration par biome.",
                    "Ville : quêtes, PNJ notables, lieux visitables, boutiques et services.",
                    "Gestion : état du personnage, inventaire, sauvegarde, récap et options de confort.",
                    "Astuce : si une quête parle de terrain, de traces, de plantes ou de route, essaie Exploration plutôt que Lieux visitables."
                }
            );
            continue;
        }

        if (choice == 2)
        {
            MessageScreen::show(
                "JOURNÉES / TEMPS",
                "activity.info.time.detail",
                {
                    "Le monde suit des jours écoulés et des moments de journée.",
                    "Les combats, boss, explorations et certaines activités peuvent faire avancer le temps.",
                    "Certaines quêtes peuvent avoir une date limite ou demander d'attendre pendant des travaux.",
                    "Les rapports de fin de journée apparaissent quand une activité fait vraiment passer le temps."
                }
            );
            continue;
        }

        if (choice == 3)
        {
            MessageScreen::show(
                "ARGENT / ÉCONOMIE",
                "activity.info.money.detail",
                {
                    "Or actuel : " + std::to_string(mainPlayer.getInventory().getGold()) + " pièce(s).",
                    "L'or vient surtout des combats, quêtes, explorations, reventes et événements.",
                    "Les boutiques peuvent changer leurs stocks après les combats ou selon l'état de la ville.",
                    "Certains marchés accepteront plus tard du troc ou des objets précis, pas seulement de l'or.",
                    "L'économie est volontairement surveillée pour éviter que les événements chanceux détruisent les prix."
                }
            );
            continue;
        }

        if (choice == 4)
        {
            MessageScreen::show(
                "COMBAT / EXPLORATION",
                "activity.info.terrain.detail",
                {
                    "Combat : affrontement volontaire contre monstres, IA, joueurs ou boss selon le mode choisi.",
                    "Exploration : sortie de terrain par biome avec plantes, coffres, pièges, traces, ressources et combats inattendus.",
                    "Une quête d'exploration ne se fait pas forcément dans Lieux visitables : elle passe souvent par Terrain > Exploration.",
                    "Les boss représentent une vraie expédition : la fuite y est impossible."
                }
            );
            continue;
        }

        if (choice == 5)
        {
            MessageScreen::show(
                "QUÊTES / OBJECTIFS",
                "activity.info.quests.detail",
                {
                    "Quête principale : objectifs d'histoire non refusables.",
                    "Quêtes à rendre / terminées : section prioritaire pour valider ce qui est prêt.",
                    "Journal complet : filtres actifs, prêtes, guilde, demandes PNJ, combat, exploration, livraison et rendues.",
                    "Une quête peut demander de s'occuper pendant des réparations : combats, services, exploration ou contrats secondaires peuvent alors servir à progresser."
                }
            );
            continue;
        }

        if (choice == 6)
        {
            MessageScreen::show(
                "VILLE / PNJ / LIEUX",
                "activity.info.city.detail",
                {
                    "PNJ notables : parler aux personnages importants, clients, référents et survivants.",
                    "Lieux visitables : forge, herboristerie, bibliothèque, boutiques et services de ville.",
                    "Les PNJ peuvent donner ou faire avancer des quêtes, mais les sorties de terrain restent dans Exploration.",
                    "La ville peut débloquer de meilleurs services selon les personnes sauvées, les preuves ramenées et les travaux terminés."
                }
            );
            continue;
        }
    }
}

// EN: launchSelectedMode declares or implements a focused behavior used by this module.
// FR: launchSelectedMode déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchSelectedMode()
{
    Combat combat;

    if (selectedMode == GameMode::Story)
    {
        launchStoryModePlaceholder();
        saveCurrentProgress("Passage dans le mode histoire");
    }
    else if (selectedMode == GameMode::Challenges)
    {
        launchChallengeBoard();
        saveCurrentProgress("Quêtes");
    }
    else if (selectedMode == GameMode::Exploration)
    {
        QuestMenu::openExploration(mainPlayer, selectedDifficulty, selectedDeathRule);
        saveCurrentProgress("Exploration");
    }
    else if (selectedMode == GameMode::Locations)
    {
        QuestMenu::openLocations(mainPlayer);
        saveCurrentProgress("Lieux visitables");
    }
    else if (selectedMode == GameMode::NotableNpcs)
    {
        QuestMenu::openNotableNpcMenu(mainPlayer);
        saveCurrentProgress("PNJ notables");
    }
    else if (selectedMode == GameMode::Exchange)
    {
        openExchangeMenu();
        saveCurrentProgress("Échange entre personnages");
    }
    else
    {
        const CombatRecapSnapshot beforeCombatSnapshot = captureCombatRecapSnapshot();

        mainPlayer.recordCombatStarted();
        if (selectedMode == GameMode::BossPve)
        {
            // Un boss représente une vraie expédition : le combat de base compte déjà 1 jour,
            // on ajoute donc 2 jours pour atteindre 3 jours pleins au total.
            mainPlayer.advanceWorldDays(2);
        }
        mainPlayer.getQuestLog().expireOverdueQuests(mainPlayer.getWorldDaysElapsed());
        ShopTransactionSystem::clearBuybackAfterCombat();
        Console::useCombatTheme();

        switch (selectedMode)
        {
            case GameMode::AIPvp:
            {
                combat.launchAIPvp(mainPlayer);
                break;
            }

            case GameMode::TwoPlayerPvp:
            {
                combat.launchTwoPlayerPvp(mainPlayer, accountName, selectedDifficulty, selectedDeathRule);
                break;
            }

            case GameMode::MonsterPve:
            {
                if (isMultiplayerSession())
                {
                    std::vector<Player*> party = getActivePartyPointers();
                    combat.launchMonsterPveTeam(party, selectedDifficulty, selectedDeathRule);
                }
                else
                {
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                }
                break;
            }

            case GameMode::BossPve:
            {
                if (isMultiplayerSession())
                {
                    std::vector<Player*> party = getActivePartyPointers();
                    combat.launchBossPveTeam(party, selectedDifficulty, selectedDeathRule);
                }
                else
                {
                    combat.launchBossPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                }
                break;
            }

            case GameMode::Story:
            case GameMode::Challenges:
            case GameMode::Exploration:
            case GameMode::Locations:
            case GameMode::NotableNpcs:
            case GameMode::Exchange:
                break;
        }

        Console::useNormalTheme();

        updateLastCombatRecap(beforeCombatSnapshot);
        ShopRotationSystem::markShopsDirtyAfterCombat();

        std::vector<std::string> timeReportLines = mainPlayer.consumeWorldTimeReportLines();
        if (!timeReportLines.empty())
        {
            MessageScreen::show("FIN DE JOURNÉE", "combat.time_report", timeReportLines);
        }

        if (mainPlayer.isDead() && DifficultyRules::isPermanentDeath(selectedDifficulty, selectedDeathRule))
        {
            saveCurrentProgress("Mort définitive");

            if (SaveManager::movePlayableCharacterToDead(accountName, mainPlayer.getName()))
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "combat.lethal.main_moved",
                    {
                        "Le personnage a été déplacé dans le registre des morts.",
                        "Il ne sera plus disponible dans les personnages jouables."
                    },
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "combat.lethal.main_move_failed",
                    {
                        "Le registre des morts refuse de se fermer correctement autour de ce personnage.",
                        "La sauvegarde de mort a tout de même été tentée."
                    },
                    false
                );
            }

            DeathPenaltySystem::displayLethalDeathCorruption();
            Console::waitForEnter();
            return;
        }

        savePartyProgress("Fin de combat");
        QuestMenu::maybeOfferRandomInterception(mainPlayer, selectedDifficulty, selectedDeathRule);
        savePartyProgress("Événement de quête éventuel");
    }

    bool continuePlaying = openPostCombatMenu();

    if (continuePlaying)
    {
        chooseGameMode();
        displaySelectedMode();
        launchSelectedMode();
        return;
    }

    savePartyProgress("Fin de session");
}

// EN: launchStoryModePlaceholder declares or implements a focused behavior used by this module.
// FR: launchStoryModePlaceholder déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchStoryModePlaceholder()
{
    bool menuOpen = true;
    while (menuOpen)
    {
        const bool storyStarted = mainPlayer.hasStoryModeStarted();
        const int maxUnlockedChapter = StoryCampaign::maxUnlockedChapter(mainPlayer);

        MenuScreen screen("MODE HISTOIRE", "story.entry.menu");
        screen.addSubtitle("Route principale — page courte");
        screen.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        screen.addLine("Chapitre maximum débloqué : " + std::to_string(maxUnlockedChapter) + ".");
        screen.addBackOption();
        screen.addOption(
            1,
            "Continuer",
            storyStarted
                ? "Lancer automatiquement la prochaine étape disponible de l'histoire."
                : "[◘ aucune histoire commencée pour ce personnage]",
            storyStarted,
            "story.continue"
        );
        screen.addOption(
            2,
            "Nouvelle histoire",
            "Repartir au vrai début : fumée blanche, niveau 1, aucun confort de bac à sable.",
            true,
            "story.new"
        );
        screen.addOption(
            3,
            "Sélectionner le chapitre",
            storyStarted
                ? "Choisir uniquement parmi les chapitres déjà débloqués. Les autres restent verrouillés."
                : "[◘ commence une nouvelle histoire avant de sélectionner un chapitre]",
            storyStarted,
            "story.chapter_select"
        );
        screen.addOption(4, "Infos : ville / PNJ / objectifs", "Lire les informations de contexte regroupées.", true, "story.info_group");
        screen.addOption(5, "Règles : histoire / bac à sable", "Reset, clone éphémère et bascule libre après fin d'histoire.", true, "story.rules_group");
        screen.addOption(6, "Bac à sable éphémère", "Créer un clone non sauvegardé pour jouer librement sans perturber l'histoire.", storyStarted, "story.ephemeral_sandbox");
        addOutOfCombatUtilityOptions(screen, true, true);

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une entrée du mode histoire.");
        Console::clear();

        if (handleOutOfCombatUtilityChoice(choice, true))
        {
            continue;
        }

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            continueStoryRoute();
            continue;
        }
        if (choice == 2)
        {
            startNewStoryFromMenu();
            continue;
        }
        if (choice == 3)
        {
            openStoryChapterSelectionMenu();
            continue;
        }
        if (choice == 4)
        {
            bool infoOpen = true;
            while (infoOpen)
            {
                MenuScreen infoScreen("INFOS HISTOIRE", "story.info_group.menu");
                infoScreen.addSubtitle("Contexte regroupé");
                infoScreen.addBackOption();
                infoScreen.addOption(1, "Lire la longue introduction", "Ton de départ et contexte général.", true, "story.long_intro");
                infoScreen.addOption(2, "Développement de la ville", "Voir ce qui est ouvert, limité ou fermé dans la route histoire.", true, "story.city_development");
                infoScreen.addOption(3, "Clients amis / référents", "Voir les PNJ qui peuvent aider ou orienter la quête principale.", true, "story.referents");
                infoScreen.addOption(4, "Intrigues suivies", "Voir les grands fils narratifs prévus sans révéler la fin.", true, "story.intrigues");
                infoScreen.addOption(5, "Prochains objectifs", "Voir missions principales et secondaires utiles au développement.", true, "story.next_objectives");

                const int infoChoice = TerminalInterface::askMenuChoiceFromOptions(infoScreen, "Choisis une information histoire.");
                Console::clear();

                if (infoChoice == 0)
                {
                    infoOpen = false;
                    continue;
                }
                if (infoChoice == 1)
                {
                    MessageScreen::show("INTRODUCTION", "story.long_intro", StoryCampaign::buildLongIntroductionLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 2)
                {
                    MessageScreen::show("DÉVELOPPEMENT DE LA VILLE", "story.city_development", StoryCampaign::buildDevelopmentLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 3)
                {
                    MessageScreen::show("CLIENTS AMIS / RÉFÉRENTS", "story.referents", StoryCampaign::buildReferentNpcLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 4)
                {
                    MessageScreen::show("INTRIGUES", "story.intrigues", StoryCampaign::buildIntrigueLines(mainPlayer));
                    continue;
                }
                if (infoChoice == 5)
                {
                    MessageScreen::show("OBJECTIFS HISTOIRE", "story.next_objectives", StoryCampaign::buildNextObjectiveLines(mainPlayer));
                    continue;
                }
            }
            continue;
        }
        if (choice == 5)
        {
            bool rulesOpen = true;
            while (rulesOpen)
            {
                MenuScreen rulesScreen("RÈGLES HISTOIRE", "story.rules_group.menu");
                rulesScreen.addSubtitle("Règles regroupées");
                rulesScreen.addBackOption();
                rulesScreen.addOption(1, "Bac à sable / histoire", "Règles de reset, clone éphémère et progression séparée.", true, "story.sandbox_rules");
                rulesScreen.addOption(2, "Fin d'histoire → bac à sable", "Après la conclusion, le personnage continue automatiquement en mode libre.", true, "story.completion_sandbox");
                rulesScreen.addOption(3, "Ordre des chapitres", "Rappelle le rôle de Continuer et de la sélection de chapitre.", true, "story.chapter_order_rules");

                const int rulesChoice = TerminalInterface::askMenuChoiceFromOptions(rulesScreen, "Choisis une règle histoire.");
                Console::clear();

                if (rulesChoice == 0)
                {
                    rulesOpen = false;
                    continue;
                }
                if (rulesChoice == 1)
                {
                    MessageScreen::show("BAC À SABLE / HISTOIRE", "story.sandbox_rules", StoryCampaign::buildSandboxRulesLines(mainPlayer));
                    continue;
                }
                if (rulesChoice == 2)
                {
                    showStoryCompletionSandboxRule();
                    continue;
                }
                if (rulesChoice == 3)
                {
                    MessageScreen::show(
                        "ORDRE DES CHAPITRES",
                        "story.chapter_order_rules",
                        {
                            "Nouvelle histoire lance le vrai début : fumée blanche, forêt, aucun équipement.",
                            "Continuer est volontairement l'option principale : il lance automatiquement la prochaine étape disponible.",
                            "Sélectionner le chapitre sert seulement à revoir/continuer un chapitre déjà débloqué.",
                            "Les chapitres non débloqués restent affichés comme ???? et ne sont pas sélectionnables."
                        },
                        false
                    );
                    continue;
                }
            }
            continue;
        }
        if (choice == 6)
        {
            launchEphemeralSandboxCloneFromStory();
            continue;
        }
    }
}

void Game::startNewStoryFromMenu()
{
    MenuScreen confirm("NOUVELLE HISTOIRE", "story.new.confirm");
    confirm.addSubtitle("Départ commun : fumée blanche");
    confirm.addLine("Une nouvelle histoire remet le personnage au départ narratif commun.");
    confirm.addLine("Niveau 1, inventaire vidé avant le prologue, routes et confort du bac à sable perdus pour cette route histoire.");
    if (mainPlayer.hasStoryModeStarted())
    {
        confirm.addLine("Attention : ce personnage a déjà une histoire en cours. La recommencer effacera cette progression histoire.");
    }
    else if (shouldResetCharacterForStoryStart())
    {
        confirm.addLine("Attention : ce personnage vient du bac à sable ou a déjà progressé. L’histoire le remettra à zéro.");
    }
    confirm.addOption(0, "Annuler", "Retourner au menu histoire.", true, "story.new.cancel");
    confirm.addOption(1, "Commencer depuis la fumée blanche", "Réinitialiser puis jouer le prologue dans l’ordre.", true, "story.new.accept");
    const int confirmChoice = TerminalInterface::askMenuChoiceFromOptions(confirm, "Confirme la nouvelle histoire.");
    Console::clear();

    if (confirmChoice != 1)
    {
        MessageScreen::show("NOUVELLE HISTOIRE ANNULÉE", "story.new.cancelled", {"Le personnage reste dans son état actuel."}, false);
        return;
    }

    resetMainPlayerForStoryStart();
    saveCurrentProgress("Nouvelle histoire initialisée");
    playStoryWhiteFogPrologue();
}

void Game::continueStoryRoute()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "AUCUNE HISTOIRE",
            "story.continue.not_started",
            {
                "Aucune route histoire n’est commencée pour ce personnage.",
                "Utilise Nouvelle histoire pour démarrer depuis la fumée blanche."
            },
            false
        );
        return;
    }

    if (mainPlayer.getStoryStep() < 2 || mainPlayer.getInventory().getWeaponCount() <= 0)
    {
        playStoryWhiteFogPrologue();
        return;
    }

    if (StoryCampaign::canUnlockChapterTwo(mainPlayer) && mainPlayer.getStoryChapter() < 2)
    {
        mainPlayer.setStoryProgress(2, 1, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
        saveCurrentProgress("Chapitre 2 débloqué par progression histoire");
        MessageScreen::show(
            "CHAPITRE 2 DÉBLOQUÉ",
            "story.chapter_2.unlocked",
            {
                "Les preuves, les monstres vaincus ou le premier boss ont assez fait bouger la ville.",
                "La route du relais silencieux peut commencer.",
                "Les chapitres restent joués dans l’ordre : le chapitre 2 est maintenant sélectionnable."
            },
            false
        );
    }

    if (mainPlayer.getStoryChapter() >= 2)
    {
        playStoryChapterTwo();
        return;
    }

    playStoryChapterOne();
}

void Game::openStoryChapterSelectionMenu()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "SÉLECTION IMPOSSIBLE",
            "story.chapter_select.not_started",
            {
                "Aucun chapitre n’est encore débloqué.",
                "Commence une nouvelle histoire pour ouvrir le prologue et le chapitre 1."
            },
            false
        );
        return;
    }

    bool selecting = true;
    while (selecting)
    {
        const int maxUnlocked = StoryCampaign::maxUnlockedChapter(mainPlayer);
        const bool chapterTwoUnlocked = StoryCampaign::isChapterUnlocked(mainPlayer, 2);

        MenuScreen screen("SÉLECTIONNER LE CHAPITRE", "story.chapter_select.menu");
        screen.addSubtitle("Seuls les chapitres débloqués sont sélectionnables");
        screen.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        screen.addLine("Chapitre maximum débloqué : " + std::to_string(maxUnlocked) + ".");
        screen.addBackOption();
        screen.addOption(1, "Prologue : fumée blanche", "Revoir ou jouer le prologue si le kit de départ n’a pas encore été récupéré.", true, "story.chapter_select.prologue");
        screen.addOption(2, "Chapitre 1 : La ville qui tient à peine", "Chapitre débloqué par le départ histoire.", true, "story.chapter_select.chapter_1");
        screen.addOption(
            3,
            chapterTwoUnlocked ? "Chapitre 2 : Le relais silencieux" : "????",
            chapterTwoUnlocked ? "Chapitre débloqué : le relais peut être consulté/continué." : "[◘ chapitre verrouillé : avance dans l’histoire, sauve/débloque les preuves nécessaires]",
            chapterTwoUnlocked,
            "story.chapter_select.chapter_2"
        );
        screen.addOption(4, "Règle d’ordre", "Rappelle que Continuer lance automatiquement la prochaine étape.", true, "story.chapter_select.rules");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un chapitre débloqué.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            playStoryWhiteFogPrologue();
            continue;
        }
        if (choice == 2)
        {
            playStoryChapterOne();
            continue;
        }
        if (choice == 3)
        {
            if (!chapterTwoUnlocked)
            {
                MessageScreen::show("CHAPITRE VERROUILLÉ", "story.chapter_select.chapter_2.locked", {"Le chapitre 2 n’est pas encore débloqué pour ce personnage."}, false);
                continue;
            }
            if (mainPlayer.getStoryChapter() < 2)
            {
                mainPlayer.setStoryProgress(2, 1, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
                saveCurrentProgress("Chapitre 2 sélectionné après déblocage");
            }
            playStoryChapterTwo();
            continue;
        }
        if (choice == 4)
        {
            MessageScreen::show(
                "ORDRE DES CHAPITRES",
                "story.chapter_select.rules",
                {
                    "Nouvelle histoire lance le vrai début : fumée blanche, forêt, aucun équipement.",
                    "Continuer ne demande pas de choisir un chapitre : il lance automatiquement la prochaine étape disponible.",
                    "Sélectionner le chapitre sert seulement à revoir/continuer un chapitre déjà débloqué.",
                    "Les chapitres non débloqués restent affichés comme ???? et ne sont pas sélectionnables."
                },
                false
            );
            continue;
        }
    }
}


bool Game::shouldResetCharacterForStoryStart() const
{
    if (mainPlayer.hasStoryModeStarted())
    {
        return false;
    }

    return mainPlayer.getLevel() > 1
        || mainPlayer.getExperience() > 0
        || mainPlayer.getVictories() > 0
        || mainPlayer.getDefeats() > 0
        || mainPlayer.getEscapes() > 0
        || mainPlayer.getEnemiesKilled() > 0
        || mainPlayer.getBossesKilled() > 0
        || mainPlayer.getDeaths() > 0
        || mainPlayer.getInventory().getGold() > 0
        || mainPlayer.getInventory().getWeaponCount() > 0
        || mainPlayer.getInventory().getArmorCount() > 0
        || mainPlayer.getInventory().getConsumableCount() > 0
        || mainPlayer.getInventory().getMaterialCount() > 0;
}

void Game::resetMainPlayerForStoryStart()
{
    const std::string oldName = mainPlayer.getName().empty() ? playerName : mainPlayer.getName();
    const std::string oldClassName = mainPlayer.getType().empty() ? "Chevalier" : mainPlayer.getType();
    const CharacterRace oldRace = mainPlayer.getRace();
    const std::string hintFrequency = mainPlayer.getInterfaceHintFrequency();
    const std::string creatorAccount = mainPlayer.getCreatorAccountName();
    const std::string ownerAccount = mainPlayer.getCurrentOwnerAccountName();

    PlayerClass baseClass = ClassCatalog::createClassByName(oldClassName);
    mainPlayer = Player(oldName, baseClass);
    mainPlayer.setRace(oldRace);
    mainPlayer.getInventory().clearAll();
    mainPlayer.setInterfaceHintFrequency(hintFrequency);
    mainPlayer.setOwnershipMetadata(creatorAccount.empty() ? accountName : creatorAccount, ownerAccount.empty() ? accountName : ownerAccount);
    mainPlayer.grantTitle("Bienvenue dans Dinotofu");
    mainPlayer.grantTitle("Voix du terminal");
    mainPlayer.startStoryMode();
    mainPlayer.setStoryProgress(1, 1, 0);
    playerName = oldName;

    MessageScreen::show(
        "HISTOIRE — RESET",
        "story.start.reset.done",
        {
            "Le personnage est revenu au départ histoire.",
            "Niveau : 1.",
            "Inventaire : vidé avant le prologue.",
            "La fumée blanche a mangé les souvenirs utiles, les routes connues et le confort du bac à sable."
        },
        false
    );
}

void Game::playStoryChapterOne()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "CHAPITRE 1 IMPOSSIBLE",
            "story.chapter_1.not_started",
            {
                "Le chapitre 1 appartient à la route histoire.",
                "Commence une Nouvelle histoire pour traverser d’abord la fumée blanche."
            },
            false
        );
        return;
    }

    if (mainPlayer.getStoryStep() < 2 || mainPlayer.getInventory().getWeaponCount() <= 0)
    {
        playStoryWhiteFogPrologue();
        return;
    }

    if (mainPlayer.getStoryStep() == 2)
    {
        MessageScreen::show("CHAPITRE 1 — ARRIVÉE", "story.chapter_1.arrival", StoryCampaign::buildChapterOneArrivalLines(mainPlayer), false);

        MenuScreen gate("PORTE DE LA VILLE", "story.chapter_1.gate_choice");
        gate.addSubtitle("Premier refuge, pas encore une maison");
        gate.addLine("Le garde attend une réaction. Derrière lui, la ville grince comme une charrette trop chargée.");
        gate.addOption(1, "Aller directement voir Mira", "Suivre l’indication du garde et chercher une référente utile.", true, "story.chapter_1.to_mira");
        gate.addOption(2, "Observer les murs et les portes", "Comprendre ce qui manque à la ville avant de parler.", true, "story.chapter_1.observe_walls");
        gate.addOption(3, "Demander où se trouve la guilde", "Chercher le registre des missions même s’il est presque vide.", true, "story.chapter_1.ask_guild");
        const int gateChoice = TerminalInterface::askMenuChoiceFromOptions(gate, "Choisis ton premier réflexe en ville.");
        Console::clear();

        std::vector<std::string> gateLines;
        if (gateChoice == 2)
        {
            gateLines.push_back("Tu prends le temps d’observer. Les murs ne sont pas faibles par paresse : ils ont été réparés trop souvent avec trop peu de matériaux.");
            gateLines.push_back("Une porte garde encore des marques de griffes. Une autre a été renforcée avec des plaques qui viennent probablement d’une vieille carriole.");
            gateLines.push_back("Tu comprends une chose simple : ici, chaque peau, chaque os, chaque planche rapportée peut réellement changer quelque chose.");
        }
        else if (gateChoice == 3)
        {
            gateLines.push_back("Le garde montre un bâtiment bas, coincé entre un puits et une forge à moitié fermée.");
            gateLines.push_back("« La guilde ? C’est ce registre humide là-bas. Pour les grandes promesses, il faudra repasser quand on aura assez de monde pour mentir correctement. »");
        }
        else
        {
            gateLines.push_back("Tu suis l’indication du garde. Dans une ville qui manque de tout, perdre du temps semble presque indécent.");
        }
        gateLines.push_back("Quel que soit ton détour, le chemin finit devant Mira.");
        MessageScreen::show("PREMIER REGARD", "story.chapter_1.gate_result", gateLines, false);

        MessageScreen::show("MIRA", "story.chapter_1.mira_intro", StoryCampaign::buildChapterOneMiraLines(mainPlayer), false);

        MenuScreen mira("MIRA — PREMIÈRE DEMANDE", "story.chapter_1.mira_choice");
        mira.addSubtitle("La ville ne cherche pas un héros. Elle cherche quelqu’un qui revient vivant.");
        mira.addLine("Mira pose trois marques sur le registre : portes, soins, matériaux.");
        mira.addOption(1, "Accepter d’aider la ville à tenir", "Entrer dans la première mission principale.", true, "story.chapter_1.accept_help");
        mira.addOption(2, "Demander d’abord des réponses", "Insister sur la fumée blanche et les souvenirs perdus.", true, "story.chapter_1.ask_answers");
        mira.addOption(3, "Rester prudent", "Ne pas promettre trop vite, mais écouter la mission.", true, "story.chapter_1.cautious");
        const int miraChoice = TerminalInterface::askMenuChoiceFromOptions(mira, "Réponds à Mira.");
        Console::clear();

        std::vector<std::string> reply;
        if (miraChoice == 2)
        {
            reply.push_back("Mira baisse les yeux vers le registre.");
            reply.push_back("Mira : « Les réponses sont au bout des routes. Et les routes sont fermées. Aide-nous à les rouvrir, et tu auras le droit de poser des questions qui m’effraient. »");
        }
        else if (miraChoice == 3)
        {
            reply.push_back("Mira hoche la tête, presque rassurée par ta prudence.");
            reply.push_back("Mira : « Bien. Les gens trop sûrs d’eux finissent souvent dehors, et dehors ne rend pas toujours les corps. »");
        }
        else
        {
            reply.push_back("Mira ne sourit pas vraiment, mais quelque chose dans ses épaules se détend.");
            reply.push_back("Mira : « Alors commence petit. Les petites choses sont les seules qui tiennent encore. »");
        }
        MessageScreen::show("RÉPONSE", "story.chapter_1.mira_reply", reply, false);

        mainPlayer.setStoryProgress(1, 3, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
        saveCurrentProgress("Chapitre 1 : Mira rencontrée");
    }

    bool chapterMenuOpen = true;
    while (chapterMenuOpen)
    {
        QuestMenu::syncMainStoryQuests(mainPlayer);
        const bool chapterReady = StoryCampaign::canCompleteChapterOne(mainPlayer);
        MenuScreen chapterMenu("CHAPITRE 1 — LA VILLE QUI TIENT À PEINE", "story.chapter_1.action_menu");
        chapterMenu.addSubtitle("Mission principale : Faire respirer les murs");
        chapterMenu.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        chapterMenu.addLine(chapterReady
            ? "Mira pense que la ville peut ouvrir la route du relais."
            : "Mira garde la suite fermée tant que la chaîne principale n'est pas terminée.");
        chapterMenu.addBackOption();
        chapterMenu.addOption(1, "Voir la mission principale", "Relire les objectifs de Mira et le sens du chapitre 1.", true, "story.chapter_1.view_mission");
        chapterMenu.addOption(2, "Voir l'état de validation", "Voir précisément ce qui manque pour ouvrir le chapitre 2.", true, "story.chapter_1.progress");
        chapterMenu.addOption(3,
            "Demander à Mira ce qu'il reste à faire",
            "Mira récapitule la prochaine quête principale au lieu d'ouvrir directement les PNJ notables.",
            true,
            "story.chapter_1.ask_mira_next"
        );
        chapterMenu.addOption(4, "Retourner voir Mira", chapterReady ? "Notifier Mira que toutes les demandes principales sont terminées." : "Demander ce qui manque encore avant la suite.", true, "story.chapter_1.validate_mira");
        chapterMenu.addOption(5, "État de la ville", "Voir les boutiques, routes et systèmes encore limités par l'histoire.", true, "story.chapter_1.city_state");

        const int chapterChoice = TerminalInterface::askMenuChoiceFromOptions(chapterMenu, "Choisis une action du chapitre 1.");
        Console::clear();

        if (chapterChoice == 0)
        {
            return;
        }
        if (chapterChoice == 1)
        {
            std::vector<std::string> missionLines = StoryCampaign::buildChapterOneMissionLines(mainPlayer);
            missionLines.push_back("");
            missionLines.push_back("Menus histoire débloqués maintenant : PNJ notables, Quêtes > Quête principale, petits contrats F/E proches, forge pauvre, herboristerie pauvre, objectifs de ville.");
            missionLines.push_back("Le chapitre 2 restera verrouillé tant que les quêtes principales de Mira et des quatre référents ne seront pas terminées puis notifiées.");
            MessageScreen::show("CHAPITRE 1 — MISSION PRINCIPALE", "story.chapter_1.main_mission", missionLines, false);
            continue;
        }
        if (chapterChoice == 2)
        {
            MessageScreen::show("VALIDATION — CHAPITRE 1", "story.chapter_1.progress", StoryCampaign::buildChapterOneProgressLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 3)
        {
            if (mainPlayer.getStoryStep() < 4)
            {
                MessageScreen::show(
                    "MIRA — CE QU'IL RESTE À FAIRE",
                    "story.chapter_1.mira_next.first_tour",
                    StoryCampaign::buildChapterOneReferentIntroLines(mainPlayer),
                    false
                );

                MenuScreen referentTour("PREMIÈRE QUÊTE PRINCIPALE", "story.chapter_1.referent_tour_confirm");
                referentTour.addSubtitle("La ville commence par des visages, pas par une liste de courses");
                referentTour.addLine("Mira te donne la première quête principale : faire le tour de la ville et rencontrer les référents cités.");
                referentTour.addOption(1, "Faire le tour de la ville", "Étape histoire obligatoire : Mira, Orren, Lysa, Bram et Soryn.", true, "story.chapter_1.referent_tour.start");
                const int tourChoice = TerminalInterface::askMenuChoiceFromOptions(referentTour, "Choisis 1 pour faire le tour de la ville.");
                Console::clear();

                if (tourChoice == 1)
                {
                    MessageScreen::show(
                        "TOUR DE LA VILLE",
                        "story.chapter_1.referent_tour.done",
                        StoryCampaign::buildChapterOneReferentTourLines(mainPlayer),
                        false
                    );
                    mainPlayer.setStoryProgress(1, 4, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    saveCurrentProgress("Chapitre 1 : tour de ville effectué");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() < 5)
            {
                MenuScreen notifyMira("NOTIFIER MIRA", "story.chapter_1.notify_mira_after_tour");
                notifyMira.addSubtitle("Le tour est fait. Mira doit maintenant transformer les présentations en vraies priorités.");
                notifyMira.addLine("Tu as rencontré les premiers référents. Ils existent aussi dans PNJ notables, mais les demandes principales passent par Mira.");
                notifyMira.addOption(1, "Dire à Mira que tout le monde a été rencontré", "Débloquer les demandes principales non refusables.", true, "story.chapter_1.notify_mira_after_tour.confirm");
                const int notifyChoice = TerminalInterface::askMenuChoiceFromOptions(notifyMira, "Choisis 1 pour notifier Mira.");
                Console::clear();

                if (notifyChoice == 1)
                {
                    mainPlayer.setStoryProgress(1, 5, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    MessageScreen::show(
                        "MIRA — NOUVELLES PRIORITÉS",
                        "story.chapter_1.mira_main_requests",
                        {
                            "Mira coche chaque nom quand tu confirmes le tour de la ville.",
                            "Mira : « Bien. Maintenant ils ne sont plus des inconnus dans un registre. Ils sont la ville. »",
                            "Elle ajoute sa propre demande principale : faire respirer les murs.",
                            "Puis elle ajoute quatre consignes : retourner parler à Orren, Lysa, Bram et Soryn pour leur demander comment les aider, en venant de sa part.",
                            "Ces quêtes principales ne sont pas refusables. Elles sont visibles dans Quêtes > Quête principale et se passent via PNJ notables."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 1 : demandes principales de Mira ajoutées");
                }
                continue;
            }

            std::vector<std::string> lines = {
                "Mira relit le registre principal.",
                chapterReady
                    ? "Mira : « Tout ce que je pouvais demander pour cette étape est là. Reviens me notifier proprement, et j'ouvrirai la suite. »"
                    : "Mira : « Il reste des demandes principales à terminer ou à rendre. Elles sont dans la section Quête principale. »",
                "Pour parler aux personnes concernées, passe par PNJ notables : Orren, Lysa, Bram et Soryn font partie du même monde que le bac à sable."
            };
            const std::vector<std::string> progress = StoryCampaign::buildChapterOneProgressLines(mainPlayer);
            lines.insert(lines.end(), progress.begin(), progress.end());
            MessageScreen::show("MIRA — CE QU'IL RESTE À FAIRE", "story.chapter_1.mira_next_status", lines, false);
            continue;
        }
        if (chapterChoice == 4)
        {
            if (!chapterReady)
            {
                std::vector<std::string> lines = {
                    "Mira lit le registre, puis secoue la tête.",
                    "Mira : « Pas encore. Ce n'est pas que je ne te crois pas, " + mainPlayer.getName() + ". C'est que les portes ne s'ouvrent pas avec de bonnes intentions. »",
                    "Mira : « Commence par connaître les visages qui tiennent la ville. Ensuite, suis les demandes principales dans l'ordre, puis reviens me notifier quand tout sera rendu. »",
                    ""
                };
                const std::vector<std::string> progress = StoryCampaign::buildChapterOneProgressLines(mainPlayer);
                lines.insert(lines.end(), progress.begin(), progress.end());
                MessageScreen::show("MIRA — PAS ENCORE", "story.chapter_1.mira_not_ready", lines, false);
                continue;
            }

            mainPlayer.setStoryProgress(2, 1, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
            saveCurrentProgress("Chapitre 1 terminé : relais silencieux débloqué");
            MessageScreen::show(
                "CHAPITRE 2 DÉBLOQUÉ",
                "story.chapter_1.completed",
                {
                    "Mira ferme le registre avec lenteur, comme si le bruit pouvait réveiller les murs.",
                    "Mira : « D'accord. Ce n'est pas assez pour sauver la ville, mais c'est assez pour arrêter de faire semblant qu'on peut rester ici les yeux fermés. »",
                    "Mira trace un cercle autour d'un nom presque effacé : le relais silencieux.",
                    "Mira : « Si Orren ou les autres sont encore vivants, ils savent pourquoi les routes mentent. Si personne ne répond, alors la route du nord est déjà en train de disparaître. »",
                    "Chapitre 2 débloqué : Le relais silencieux."
                },
                false
            );
            return;
        }
        if (chapterChoice == 5)
        {
            MessageScreen::show("ÉTAT DE LA VILLE", "story.chapter_1.city_state", StoryCampaign::buildDevelopmentLines(mainPlayer), false);
            continue;
        }
    }
}

void Game::playStoryChapterTwo()
{
    if (!mainPlayer.hasStoryModeStarted() || mainPlayer.getStoryChapter() < 2)
    {
        MessageScreen::show(
            "CHAPITRE 2 IMPOSSIBLE",
            "story.chapter_2.not_unlocked",
            {
                "Le relais silencieux appartient à la suite de l'histoire.",
                "Termine d'abord les demandes principales du chapitre 1, puis notifie Mira."
            },
            false
        );
        return;
    }

    bool chapterMenuOpen = true;
    while (chapterMenuOpen)
    {
        QuestMenu::syncMainStoryQuests(mainPlayer);

        MenuScreen chapterMenu("CHAPITRE 2 — LE RELAIS SILENCIEUX", "story.chapter_2.action_menu");
        chapterMenu.addSubtitle("Mission principale : comprendre pourquoi la route ment");
        chapterMenu.addLine("Progression : " + mainPlayer.getStoryProgressLabel());
        if (mainPlayer.getStoryStep() < 2)
        {
            chapterMenu.addLine("Mira et Orren doivent d'abord poser le cadre avant toute sortie.");
        }
        else if (mainPlayer.getStoryStep() < 3)
        {
            chapterMenu.addLine("Priorité : explorer la Route commerciale et rendre la reconnaissance à Orren.");
        }
        else if (mainPlayer.getStoryStep() < 4)
        {
            chapterMenu.addLine("Priorité : obtenir une preuve assez nette pour Soryn, pas seulement une rumeur.");
        }
        else if (mainPlayer.getStoryStep() < 5)
        {
            chapterMenu.addLine("Priorité : affronter les guetteurs sans feu. Cette menace est imposée par l'histoire du relais.");
        }
        else if (mainPlayer.getStoryStep() < 6)
        {
            chapterMenu.addLine("Priorité : faire répondre le relais. Une route dégagée mais muette reste dangereuse.");
        }
        else if (mainPlayer.getStoryStep() < 7)
        {
            chapterMenu.addLine("Priorité : suivre le premier signal vivant du relais et sauver Nell la messagère.");
        }
        else if (mainPlayer.getStoryStep() < 8)
        {
            chapterMenu.addLine("Priorité : exploiter la sacoche de Nell pour transformer le sauvetage en piste exploitable.");
        }
        else if (mainPlayer.getStoryStep() < 9)
        {
            chapterMenu.addLine("Priorité : distribuer les informations de Nell aux comptoirs pour que la ville réagisse concrètement.");
        }
        else if (mainPlayer.getStoryStep() < 10)
        {
            chapterMenu.addLine("Priorité : retourner sur la Route commerciale et suivre l'encre froide jusqu'à une vraie piste.");
        }
        else if (mainPlayer.getStoryStep() < 11)
        {
            chapterMenu.addLine("Priorité : identifier ce qui réécrit les routes avec Soryn et Nell.");
        }
        else if (mainPlayer.getStoryStep() < 12)
        {
            chapterMenu.addLine("Priorité : installer le contre-registre des routes courtes pour protéger stocks et comptoirs.");
        }
        else if (mainPlayer.getStoryStep() < 13)
        {
            chapterMenu.addLine("Priorité : reconnaître le nœud noir, l'endroit où les routes corrigées semblent revenir.");
        }
        else if (mainPlayer.getStoryStep() < 14)
        {
            chapterMenu.addLine("Priorité : tenir pendant les travaux. La ville prépare portes, soins et retours avant la sortie dangereuse.");
        }
        else if (mainPlayer.getStoryStep() < 15)
        {
            chapterMenu.addLine("Priorité : confirmer ce qui garde la borne noire sans écrire son vrai nom trop tôt.");
        }
        else if (mainPlayer.getStoryStep() < 16)
        {
            chapterMenu.addLine("Priorité : briser le premier verrou de la borne noire et revenir avec une preuve.");
        }
        else if (mainPlayer.getStoryStep() < 17)
        {
            chapterMenu.addLine("Priorité : lire les cicatrices du verrou avec Soryn avant de croire la route sauvée.");
        }
        else if (mainPlayer.getStoryStep() < 18)
        {
            chapterMenu.addLine("Priorité : organiser une route gardée pour les premiers retours confirmés.");
        }
        else
        {
            chapterMenu.addLine("La route courte tient sous garde. Le vrai nom derrière la borne noire reste encore à découvrir.");
        }
        chapterMenu.addBackOption();
        chapterMenu.addOption(1, "Voir la mission principale", "Relire le plan du relais silencieux.", true, "story.chapter_2.view_mission");
        chapterMenu.addOption(2, "Voir l'état de validation", "Voir les étapes principales déjà rendues.", true, "story.chapter_2.progress");
        chapterMenu.addOption(3, "Demander à Mira et Orren ce qu'il reste à faire", "Obtenir la prochaine consigne de route.", true, "story.chapter_2.ask_next");
        const int chapterTwoStep = mainPlayer.getStoryStep();
        std::string chapterTwoActionLabel = "Lire le bilan de route";
        std::string chapterTwoActionDetail = "Lire les conséquences validées.";
        if (chapterTwoStep < 2)
        {
            chapterTwoActionLabel = "Faire le briefing du relais";
            chapterTwoActionDetail = "Réunir Mira et Orren autour de la carte.";
        }
        else if (chapterTwoStep < 4)
        {
            chapterTwoActionLabel = "Enquêter sur la Route commerciale";
            chapterTwoActionDetail = "Scènes ciblées : bornes, traces, ornières et relais silencieux.";
        }
        else if (chapterTwoStep < 5)
        {
            chapterTwoActionLabel = "Affronter la menace du relais";
            chapterTwoActionDetail = "Combat imposé par l'histoire, puis rapport à Orren.";
        }
        else if (chapterTwoStep < 6)
        {
            chapterTwoActionLabel = "Faire répondre le relais";
            chapterTwoActionDetail = "Service histoire : cloche, marque et registre du relais.";
        }
        else if (chapterTwoStep < 7)
        {
            chapterTwoActionLabel = "Sauver la voix du relais";
            chapterTwoActionDetail = "Premier sauvetage de route : convoi brisé et Nell la messagère.";
        }
        else if (chapterTwoStep < 8)
        {
            chapterTwoActionLabel = "Exploiter la sacoche de Nell";
            chapterTwoActionDetail = "Trier cartes, bons, haltes et encre froide dans la sacoche.";
        }
        else if (chapterTwoStep < 9)
        {
            chapterTwoActionLabel = "Distribuer les infos aux comptoirs";
            chapterTwoActionDetail = "Faire réagir forge, herboristerie, guilde et relais aux informations de Nell.";
        }
        else if (chapterTwoStep < 10)
        {
            chapterTwoActionLabel = "Suivre l'encre froide";
            chapterTwoActionDetail = "Deux scènes de Route commerciale liées à la sacoche de Nell.";
        }
        else if (chapterTwoStep < 11)
        {
            chapterTwoActionLabel = "Identifier la route réécrite";
            chapterTwoActionDetail = "Analyse d'archives : prouver que la route est corrigée après passage.";
        }
        else if (chapterTwoStep < 12)
        {
            chapterTwoActionLabel = "Installer le contre-registre";
            chapterTwoActionDetail = "Ville/économie : rendre les stocks moins dépendants des cartes corrompues.";
        }
        else if (chapterTwoStep < 13)
        {
            chapterTwoActionLabel = "Reconnaître le nœud noir";
            chapterTwoActionDetail = "Reconnaissance risquée avant la prochaine vraie crise.";
        }
        else if (chapterTwoStep < 14)
        {
            chapterTwoActionLabel = "Tenir pendant les travaux";
            chapterTwoActionDetail = "Patrouilles, services et quêtes utiles pendant que la ville prépare la suite.";
        }
        else if (chapterTwoStep < 15)
        {
            chapterTwoActionLabel = "Confirmer la présence";
            chapterTwoActionDetail = "Piste de menace : identifier la chose qui garde la borne sans révéler son vrai nom.";
        }
        else if (chapterTwoStep < 16)
        {
            chapterTwoActionLabel = "Briser le verrou";
            chapterTwoActionDetail = "Boss d'étape non nommé lié à la borne noire.";
        }
        else if (chapterTwoStep < 17)
        {
            chapterTwoActionLabel = "Lire les cicatrices du verrou";
            chapterTwoActionDetail = "Classer les preuves laissées par le verrou avec Soryn.";
        }
        else if (chapterTwoStep < 18)
        {
            chapterTwoActionLabel = "Organiser la route gardée";
            chapterTwoActionDetail = "Conséquence ville : retours courts, stocks surveillés et relais secondaires.";
        }
        chapterMenu.addOption(4, chapterTwoActionLabel, chapterTwoActionDetail, true, "story.chapter_2.action");
        chapterMenu.addOption(5, "Ouvrir le menu Quêtes", "Accéder à Quête principale et au journal.", true, "story.chapter_2.main_quests");
        chapterMenu.addOption(6, "PNJ notables", "Parler à Mira, Orren, Soryn ou aux autres référents.", true, "story.chapter_2.npcs");

        const int chapterChoice = TerminalInterface::askMenuChoiceFromOptions(chapterMenu, "Choisis une action du chapitre 2.");
        Console::clear();

        if (chapterChoice == 0)
        {
            return;
        }
        if (chapterChoice == 1)
        {
            MessageScreen::show("CHAPITRE 2 — MISSION PRINCIPALE", "story.chapter_2.main_mission", StoryCampaign::buildChapterTwoMissionLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 2)
        {
            MessageScreen::show("VALIDATION — CHAPITRE 2", "story.chapter_2.progress", StoryCampaign::buildChapterTwoProgressLines(mainPlayer), false);
            continue;
        }
        if (chapterChoice == 3)
        {
            std::vector<std::string> lines;
            if (mainPlayer.getStoryStep() < 2)
            {
                lines = {
                    "Mira garde le doigt sur le nom du relais silencieux.",
                    "Mira : « Avant de courir, tu écoutes Orren. Une route qui ment tue surtout les gens pressés. »",
                    "Prochaine étape : faire le briefing du relais."
                };
            }
            else if (mainPlayer.getStoryStep() < 3)
            {
                lines = {
                    "Orren replie une carte déjà trop raturée.",
                    "Orren : « Va sur la Route commerciale. Deux sorties courtes valent mieux qu'une grande déclaration héroïque. Puis reviens me rendre la reconnaissance. »",
                    "Outil : Exploration > Route commerciale, puis PNJ notables > Orren pour rendre la demande quand elle est prête."
                };
            }
            else if (mainPlayer.getStoryStep() < 4)
            {
                lines = {
                    "Soryn refuse d'écrire le mot malédiction sans preuve.",
                    "Soryn : « Une borne retournée, une trace impossible, un témoin cohérent. Un seul élément propre vaut mieux que dix paniques. »",
                    "Outil : Quêtes > Quête principale, exploration de la Route commerciale, puis rendu auprès de Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 5)
            {
                lines = {
                    "Mira, Orren et Soryn ont maintenant assez de preuves pour admettre que le relais ne se contente pas d'être silencieux.",
                    "Suite : les guetteurs sans feu doivent être affrontés depuis le menu histoire. Ce n'est pas un contrat libre à contourner.",
                    "Après le combat, rends la quête principale à Orren pour stabiliser le relais."
                };
            }
            else if (mainPlayer.getStoryStep() < 6)
            {
                lines = {
                    "Les guetteurs sans feu sont repoussés, mais Mira refuse de cocher le relais comme vraiment fiable.",
                    "Mira : « Une route vide ne suffit pas. Il faut qu'elle réponde aux gens qui vont revenir après toi. »",
                    "Suite : faire répondre le relais depuis le menu histoire, puis rendre la quête principale à Mira."
                };
            }
            else if (mainPlayer.getStoryStep() < 7)
            {
                lines = {
                    "La cloche du relais répond. Pour la première fois, elle ne renvoie pas seulement un signal : elle renvoie une urgence.",
                    "Orren : « Trois coups depuis le nord. Ça veut dire vivant, coincé, bientôt trop tard. »",
                    "Suite : sauver Nell la messagère depuis le menu histoire, puis rendre la quête principale auprès d'elle."
                };
            }
            else if (mainPlayer.getStoryStep() < 8)
            {
                lines = {
                    "Nell est revenue avec une sacoche de routes et assez de souffle pour parler.",
                    "Nell : « Si vous lisez seulement la carte du dessus, vous suivrez la mauvaise route. Les papiers du fond disent autre chose. »",
                    "Suite : exploiter la sacoche de Nell depuis le menu histoire, puis rendre l'analyse auprès d'elle."
                };
            }
            else if (mainPlayer.getStoryStep() < 9)
            {
                lines = {
                    "La sacoche a donné assez d'informations pour aider la ville, pas seulement l'histoire principale.",
                    "Mira : « Une route qui revient doit nourrir la forge, l'herboristerie, la guilde et le relais. Sinon elle ne sert qu'à raconter une belle frayeur. »",
                    "Suite : distribuer les informations aux comptoirs, puis notifier Mira."
                };
            }
            else if (mainPlayer.getStoryStep() < 10)
            {
                lines = {
                    "Les comptoirs respirent un peu, mais Soryn ne lâche pas la marque d'encre froide.",
                    "Soryn : « Les stocks prouvent que Nell dit vrai. L'encre, elle, prouve que quelqu'un a réécrit la route. »",
                    "Suite : suivre deux traces scénarisées sur la Route commerciale, puis rendre la preuve à Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 11)
            {
                lines = {
                    "Soryn et Nell gardent la carte entre eux comme une bête malade.",
                    "Soryn : « Maintenant qu'on a l'encre, il faut prouver le procédé. Une route qui se corrige après coup n'est pas une route perdue : c'est une route tenue. »",
                    "Suite : identifier ce qui réécrit la route, puis rendre la preuve auprès de Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 12)
            {
                lines = {
                    "Mira refuse de laisser les comptoirs dépendre de cartes que l'encre froide sait manipuler.",
                    "Mira : « Si la carte ment, nos stocks doivent apprendre à vérifier autrement. »",
                    "Suite : installer le contre-registre des routes courtes, puis notifier Mira."
                };
            }
            else if (mainPlayer.getStoryStep() < 13)
            {
                lines = {
                    "Orren n'aime pas la forme dessinée par les routes corrigées.",
                    "Orren : « Toutes les erreurs reviennent vers une ancienne borne noire. On n'y envoie pas un convoi. On y envoie une preuve. »",
                    "Suite : reconnaître le nœud noir, survivre à l'approche, puis rendre l'alerte auprès d'Orren."
                };
            }
            else if (mainPlayer.getStoryStep() < 14)
            {
                lines = {
                    "Le nœud noir est repéré, mais la ville ne peut pas tout réparer pendant que tu regardes le registre.",
                    "Mira : « Bram renforce, Lysa prépare, Eda vérifie, Orren balise. Toi, tu t'occupes utilement et tu reviens avec assez de gestes sûrs pour tenir la suite. »",
                    "Suite : faire avancer Tenir pendant les travaux avec patrouilles, services, quêtes secondaires ou exploration proche."
                };
            }
            else if (mainPlayer.getStoryStep() < 15)
            {
                lines = {
                    "Les réparations tiennent assez pour regarder la borne noire sans sacrifier la ville derrière toi.",
                    "Soryn : « On ne dira pas son vrai nom tant qu'on ne l'a pas proprement identifié. Mais quelque chose garde la borne. »",
                    "Suite : recouper les témoignages et préparer l'idée d'une menace majeure sans donner son vrai nom trop tôt."
                };
            }
            else if (mainPlayer.getStoryStep() < 16)
            {
                lines = {
                    "Nell est revenue, la ville a réagi, l'encre froide a été classée, le nœud noir est repéré et les réparations ont tenu.",
                    "Mira : « Maintenant on sait assez pour préparer l'affrontement. Pas assez pour donner son vrai nom. »",
                    "Suite : briser le verrou de la borne noire, puis rendre la preuve auprès d'Orren."
                };
            }
            else if (mainPlayer.getStoryStep() < 17)
            {
                lines = {
                    "Le premier verrou de la borne noire a cédé, mais Soryn refuse de le classer comme une victoire simple.",
                    "Soryn : « Ce qui cède laisse toujours une marque. On lit la marque avant de donner un nom. »",
                    "Suite : lire les cicatrices du verrou, puis rendre la preuve auprès de Soryn."
                };
            }
            else if (mainPlayer.getStoryStep() < 18)
            {
                lines = {
                    "Les cicatrices du verrou sont classées.",
                    "Mira : « Une route blessée peut encore tuer. Maintenant on organise les premiers retours, un par un. »",
                    "Suite : organiser une route gardée avec Mira, Nell, Eda, Bram et Lysa."
                };
            }
            else
            {
                lines = {
                    "La route courte tient sous garde.",
                    "Orren : « On a gagné du passage, pas la paix. C'est déjà beaucoup. »",
                    "Suite possible : préparer plus tard l'identification complète de la menace derrière les routes réécrites."
                };
            }
            MessageScreen::show("CE QU'IL RESTE À FAIRE", "story.chapter_2.next", lines, false);
            continue;
        }
        if (chapterChoice == 4)
        {
            if (mainPlayer.getStoryStep() < 2)
            {
                MessageScreen::show("BRIEFING DU RELAIS", "story.chapter_2.briefing", StoryCampaign::buildChapterTwoBriefingLines(mainPlayer), false);
                mainPlayer.setStoryProgress(2, 2, std::max(2, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
                saveCurrentProgress("Chapitre 2 : briefing du relais terminé");
                continue;
            }

            if (mainPlayer.getStoryStep() < 4)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                MenuScreen routeMenu("ROUTE COMMERCIALE — RELAIS", "story.chapter_2.route_events");
                routeMenu.addSubtitle("Enquête ciblée : bornes, ornières et silence du relais");
                routeMenu.addLine("Cette option ne remplace pas le bac à sable : elle ajoute des scènes de Route commerciale liées au relais silencieux.");
                routeMenu.addLine("Reconnaissance Orren : " + storyQuestProgressText(mainPlayer, "story_ch2_north_road_scout") + ".");
                routeMenu.addLine("Preuve Soryn : " + storyQuestProgressText(mainPlayer, "story_ch2_turned_marker") + ".");
                routeMenu.addBackOption("Retour", "story.chapter_2.route.back");
                routeMenu.addOption(1, "Marquer les bornes qui changent de côté", "Avance la reconnaissance d'Orren sur la Route commerciale.", true, "story.chapter_2.route.marker");
                routeMenu.addOption(2, "Suivre les ornières muettes", "Scène plus risquée : cherche une preuve utilisable pour Soryn.", true, "story.chapter_2.route.tracks");
                routeMenu.addOption(3, "Ouvrir l'exploration complète", "Laisser le joueur explorer normalement les biomes, dont la Route commerciale.", true, "story.chapter_2.route.full_exploration");

                const int routeChoice = TerminalInterface::askMenuChoiceFromOptions(routeMenu, "Choisis une scène de route.");
                Console::clear();

                if (routeChoice == 0)
                {
                    continue;
                }
                if (routeChoice == 1)
                {
                    std::vector<std::string> lines = {
                        "Tu plantes un bout de charbon dans la terre, puis tu avances jusqu'à la borne suivante.",
                        "Elle porte le même éclat sur le côté opposé. Pas une erreur de lecture : quelqu'un ou quelque chose retourne les repères après le passage des patrouilles.",
                        "Orren pourra exploiter cette mesure, mais il faudra lui rendre la reconnaissance auprès des PNJ notables."
                    };
                    if (progressStoryQuestById(mainPlayer, "story_ch2_north_road_scout", 1))
                    {
                        lines.push_back("Quête principale mise à jour : La route qui s'allonge — " + storyQuestProgressText(mainPlayer, "story_ch2_north_road_scout") + ".");
                    }
                    else
                    {
                        lines.push_back("Aucune progression directe : la quête est peut-être déjà prête à rendre ou validée.");
                    }
                    MessageScreen::show("BORNES QUI MENTENT", "story.chapter_2.route.marker_result", lines, false);
                    saveCurrentProgress("Chapitre 2 : bornes du relais inspectées");
                    continue;
                }
                if (routeChoice == 2)
                {
                    std::vector<std::string> lines = {
                        "Les ornières d'une charrette s'arrêtent dans la boue sans trace de demi-tour.",
                        "Plus loin, le même dessin réapparaît comme si la route avait recollé deux morceaux de trajet qui ne devraient pas se toucher.",
                        "Tu récupères une preuve propre : pas une panique, pas un témoignage, une contradiction physique."
                    };

                    bool updated = false;
                    updated = progressStoryQuestById(mainPlayer, "story_ch2_north_road_scout", 1) || updated;
                    if (storyQuestTurnedIn(mainPlayer, "story_ch2_north_road_scout") || mainPlayer.getStoryStep() >= 3)
                    {
                        updated = progressStoryQuestById(mainPlayer, "story_ch2_turned_marker", 1) || updated;
                    }

                    lines.push_back("Reconnaissance Orren : " + storyQuestProgressText(mainPlayer, "story_ch2_north_road_scout") + ".");
                    lines.push_back("Preuve Soryn : " + storyQuestProgressText(mainPlayer, "story_ch2_turned_marker") + ".");
                    if (!updated)
                    {
                        lines.push_back("Aucune progression directe : pense à rendre les étapes prêtes auprès d'Orren ou de Soryn.");
                    }
                    MessageScreen::show("ORNIÈRES MUETTES", "story.chapter_2.route.tracks_result", lines, false);
                    saveCurrentProgress("Chapitre 2 : preuve de route relevée");
                    continue;
                }

                QuestMenu::openExploration(mainPlayer, selectedDifficulty, selectedDeathRule);
                QuestMenu::syncMainStoryQuests(mainPlayer);
                saveCurrentProgress("Chapitre 2 : exploration libre depuis la route du relais");
                continue;
            }

            QuestMenu::syncMainStoryQuests(mainPlayer);
            if (mainPlayer.getStoryStep() < 6 && storyQuestTurnedIn(mainPlayer, "story_ch2_relay_signal"))
            {
                mainPlayer.setStoryProgress(2, 6, std::max(3, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 7 && storyQuestTurnedIn(mainPlayer, "story_ch2_first_rescue"))
            {
                mainPlayer.setStoryProgress(2, 7, std::max(4, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 8 && storyQuestTurnedIn(mainPlayer, "story_ch2_route_sack"))
            {
                mainPlayer.setStoryProgress(2, 8, std::max(4, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 9 && storyQuestTurnedIn(mainPlayer, "story_ch2_city_recovery"))
            {
                mainPlayer.setStoryProgress(2, 9, std::max(5, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 10 && storyQuestTurnedIn(mainPlayer, "story_ch2_cold_ink_trail"))
            {
                mainPlayer.setStoryProgress(2, 10, std::max(5, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 11 && storyQuestTurnedIn(mainPlayer, "story_ch2_route_rewrite"))
            {
                mainPlayer.setStoryProgress(2, 11, std::max(5, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 12 && storyQuestTurnedIn(mainPlayer, "story_ch2_short_route_counter"))
            {
                mainPlayer.setStoryProgress(2, 12, std::max(6, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 13 && storyQuestTurnedIn(mainPlayer, "story_ch2_black_knot_warning"))
            {
                mainPlayer.setStoryProgress(2, 13, std::max(6, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 14 && storyQuestTurnedIn(mainPlayer, "story_ch2_repair_downtime"))
            {
                mainPlayer.setStoryProgress(2, 14, std::max(6, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 15 && storyQuestTurnedIn(mainPlayer, "story_ch2_hidden_guardian_hint"))
            {
                mainPlayer.setStoryProgress(2, 15, std::max(7, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 16 && storyQuestTurnedIn(mainPlayer, "story_ch2_black_knot_seal"))
            {
                mainPlayer.setStoryProgress(2, 16, std::max(8, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 17 && storyQuestTurnedIn(mainPlayer, "story_ch2_black_knot_scars"))
            {
                mainPlayer.setStoryProgress(2, 17, std::max(8, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }
            if (mainPlayer.getStoryStep() < 18 && storyQuestTurnedIn(mainPlayer, "story_ch2_guarded_route"))
            {
                mainPlayer.setStoryProgress(2, 18, std::max(9, mainPlayer.getStoryCityDevelopmentLevel()));
                QuestMenu::syncMainStoryQuests(mainPlayer);
            }

            if (mainPlayer.getStoryStep() >= 18 || storyQuestTurnedIn(mainPlayer, "story_ch2_guarded_route"))
            {
                mainPlayer.setStoryProgress(2, 18, std::max(9, mainPlayer.getStoryCityDevelopmentLevel()));
                MessageScreen::show(
                    "ROUTE COURTE SOUS GARDE",
                    "story.chapter_2.guarded_route.done",
                    {
                        "La ville ne crie pas victoire. Elle compte les retours.",
                        "Le verrou de la borne noire a laissé des cicatrices, la route accepte de rendre quelques vivants, et les comptoirs savent enfin distinguer une carte rassurante d'une preuve fiable.",
                        "Mira classe la crise comme contenue, pas résolue. Le vrai nom de ce qui tord les routes reste absent du registre."
                    },
                    false
                );
                saveCurrentProgress("Chapitre 2 : route gardée stabilisée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 17)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_guarded_route"))
                {
                    MessageScreen::show(
                        "ROUTE À NOTIFIER",
                        "story.chapter_2.guarded_route.ready",
                        {
                            "Les premiers retours gardés sont organisés.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville pourra respirer un peu mieux sans croire que le problème est terminé."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoGuardedRouteLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_guarded_route", 1);
                lines.push_back(progressed
                    ? "Quête principale prête : Une route à garder ouverte."
                    : "La quête était peut-être déjà prête à rendre auprès de Mira.");
                MessageScreen::show("UNE ROUTE À GARDER OUVERTE", "story.chapter_2.guarded_route", lines, false);
                saveCurrentProgress("Chapitre 2 : route gardée organisée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 16)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_black_knot_scars"))
                {
                    MessageScreen::show(
                        "CICATRICES À CLASSER",
                        "story.chapter_2.black_knot_scars.ready",
                        {
                            "Les marques du verrou sont assez nettes pour être classées.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville pourra organiser des retours gardés au lieu de juste raconter un combat gagné."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoBlackKnotScarsLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_black_knot_scars", 1);
                lines.push_back(progressed
                    ? "Quête principale prête : Les cicatrices du verrou."
                    : "La quête était peut-être déjà prête à rendre auprès de Soryn.");
                MessageScreen::show("LES CICATRICES DU VERROU", "story.chapter_2.black_knot_scars", lines, false);
                saveCurrentProgress("Chapitre 2 : cicatrices du verrou relevées");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 15)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_black_knot_seal"))
                {
                    MessageScreen::show(
                        "VERROU À RENDRE",
                        "story.chapter_2.black_knot_seal.ready",
                        {
                            "Le verrou de la borne a cédé pendant l'affrontement.",
                            "Il reste à rendre la quête principale auprès d'Orren dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville saura que la menace n'est plus seulement évitée : elle a déjà reculé une fois."
                        },
                        false
                    );
                    continue;
                }

                MessageScreen::show(
                    "LE VERROU DE LA BORNE",
                    "story.chapter_2.black_knot_seal.intro",
                    StoryCampaign::buildChapterTwoBlackKnotSealLines(mainPlayer),
                    false
                );

                Combat combat;
                mainPlayer.recordCombatStarted();
                ShopTransactionSystem::clearBuybackAfterCombat();
                Console::useCombatTheme();
                combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                Console::useNormalTheme();
                ShopRotationSystem::markShopsDirtyAfterCombat();

                if (!mainPlayer.isDead())
                {
                    const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_black_knot_seal", 1);
                    MessageScreen::show(
                        "PREMIER VERROU REPOUSSÉ",
                        "story.chapter_2.black_knot_seal.complete",
                        {
                            "La borne noire ne livre pas son vrai nom. Elle livre une réaction : la route s'est contractée autour de toi, puis a lâché d'un coup sec.",
                            "Orren devra recevoir cette preuve avant que la ville décide jusqu'où poursuivre l'enquête.",
                            progressed ? "Quête principale prête : Le verrou de la borne." : "La quête était peut-être déjà prête à rendre auprès d'Orren."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 2 : verrou de la borne affronté");
                }
                else
                {
                    saveCurrentProgress("Chapitre 2 : échec face au verrou de la borne");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() >= 14)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_hidden_guardian_hint"))
                {
                    MessageScreen::show(
                        "PISTE À RENDRE",
                        "story.chapter_2.hidden_guardian.ready",
                        {
                            "Les témoignages et retours de stock indiquent assez clairement qu'une présence garde la borne noire.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville saura préparer un affrontement sérieux sans nommer trop vite ce qui garde la borne."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoHiddenGuardianHintLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_hidden_guardian_hint", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : La chose qui garde la borne — " + storyQuestProgressText(mainPlayer, "story_ch2_hidden_guardian_hint") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Soryn.");
                lines.push_back("Retourne voir Soryn pour classer la menace sans écrire son vrai nom trop tôt.");
                MessageScreen::show("LA CHOSE QUI GARDE LA BORNE", "story.chapter_2.hidden_guardian", lines, false);
                saveCurrentProgress("Chapitre 2 : menace de la borne recoupée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 13)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_repair_downtime"))
                {
                    MessageScreen::show(
                        "TRAVAUX À NOTIFIER",
                        "story.chapter_2.repair_downtime.ready",
                        {
                            "Les trois actions utiles pendant les réparations sont terminées.",
                            "Il reste à rendre la quête principale auprès d'Eda dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville acceptera de regarder ce qui garde vraiment la borne noire."
                        },
                        false
                    );
                    continue;
                }

                MenuScreen repairMenu("PENDANT LES RÉPARATIONS", "story.chapter_2.repair_downtime_menu");
                repairMenu.addSubtitle("Quête principale : Tenir pendant les travaux");
                repairMenu.addLine("Progression : " + storyQuestProgressText(mainPlayer, "story_ch2_repair_downtime") + ".");
                repairMenu.addLine("Mira ne demande pas d'attendre par confort : Bram, Lysa, Orren, Nell et Eda ont besoin que les portes, les trousses et les registres tiennent avant la prochaine sortie.");
                repairMenu.addBackOption("Retour", "story.chapter_2.repair.back");
                repairMenu.addOption(1, "Faire une patrouille courte", "Combat ou nettoyage proche pendant que les réparations avancent.", true, "story.chapter_2.repair.patrol");
                repairMenu.addOption(2, "Aider les comptoirs", "Service de ville : stocks, registre, herboristerie ou forge.", true, "story.chapter_2.repair.counter");
                repairMenu.addOption(3, "Ouvrir les quêtes secondaires", "Aller au menu Quêtes pour prendre ou rendre des activités utiles.", true, "story.chapter_2.repair.side_quests");

                const int repairChoice = TerminalInterface::askMenuChoiceFromOptions(repairMenu, "Choisis comment t'occuper pendant les réparations.");
                Console::clear();

                if (repairChoice == 0)
                {
                    continue;
                }
                if (repairChoice == 3)
                {
                    QuestMenu::openQuestHub(mainPlayer);
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    saveCurrentProgress("Chapitre 2 : passage par les quêtes secondaires pendant les réparations");
                    continue;
                }

                if (repairChoice == 1)
                {
                    MessageScreen::show(
                        "PATROUILLE DE RÉPARATION",
                        "story.chapter_2.repair.patrol_intro",
                        StoryCampaign::buildChapterTwoRepairDowntimeLines(mainPlayer),
                        false
                    );

                    Combat combat;
                    mainPlayer.recordCombatStarted();
                    ShopTransactionSystem::clearBuybackAfterCombat();
                    Console::useCombatTheme();
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                    Console::useNormalTheme();
                    ShopRotationSystem::markShopsDirtyAfterCombat();

                    if (!mainPlayer.isDead())
                    {
                        const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_repair_downtime", 1);
                        MessageScreen::show(
                            "PATROUILLE UTILE",
                            "story.chapter_2.repair.patrol_done",
                            {
                                "La patrouille n'a pas réglé le nœud noir, mais elle a laissé Bram, Lysa et Eda finir une partie de leurs préparatifs.",
                                progressed ? "Quête principale mise à jour : Tenir pendant les travaux — " + storyQuestProgressText(mainPlayer, "story_ch2_repair_downtime") + "." : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès d'Eda."
                            },
                            false
                        );
                    }
                    saveCurrentProgress("Chapitre 2 : patrouille pendant les réparations");
                    continue;
                }

                if (repairChoice == 2)
                {
                    std::vector<std::string> lines = StoryCampaign::buildChapterTwoRepairDowntimeLines(mainPlayer);
                    const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_repair_downtime", 1);
                    lines.push_back("Tu aides Eda à comparer les stocks revenus, Bram à trier les ferrures et Lysa à préparer les trousses courtes.");
                    lines.push_back(progressed
                        ? "Quête principale mise à jour : Tenir pendant les travaux — " + storyQuestProgressText(mainPlayer, "story_ch2_repair_downtime") + "."
                        : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès d'Eda.");
                    MessageScreen::show("SERVICE DE COMPTOIR", "story.chapter_2.repair.counter_done", lines, false);
                    saveCurrentProgress("Chapitre 2 : service de comptoir pendant les réparations");
                    continue;
                }
            }

            if (mainPlayer.getStoryStep() >= 12)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_black_knot_warning"))
                {
                    MessageScreen::show(
                        "ALERTE À RENDRE",
                        "story.chapter_2.black_knot.ready",
                        {
                            "La reconnaissance du nœud noir est faite.",
                            "Il reste à rendre la quête principale auprès d'Orren dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, le chapitre 2 aura une vraie crise suivante à ouvrir."
                        },
                        false
                    );
                    continue;
                }

                MessageScreen::show(
                    "LE NŒUD NOIR AU BOUT DU RELAIS",
                    "story.chapter_2.black_knot_intro",
                    StoryCampaign::buildChapterTwoBlackKnotWarningLines(mainPlayer),
                    false
                );

                {
                    Combat combat;
                    mainPlayer.recordCombatStarted();
                    ShopTransactionSystem::clearBuybackAfterCombat();
                    Console::useCombatTheme();
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                    Console::useNormalTheme();
                    ShopRotationSystem::markShopsDirtyAfterCombat();
                }

                if (!mainPlayer.isDead())
                {
                    mainPlayer.getQuestLog().completeQuest("story_ch2_black_knot_warning");
                    MessageScreen::show(
                        "BORNE NOIRE LOCALISÉE",
                        "story.chapter_2.black_knot.complete",
                        {
                            "Tu ne détruis pas encore le nœud noir. Tu prouves qu'il existe, qu'il surveille l'approche, et qu'une vraie crise peut partir de là.",
                            "Orren devra recevoir ce rapport avant que la ville décide quoi risquer.",
                            "Quête principale prête : Le nœud noir au bout du relais."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 2 : nœud noir reconnu");
                }
                else
                {
                    saveCurrentProgress("Chapitre 2 : échec de reconnaissance du nœud noir");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() >= 11)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_short_route_counter"))
                {
                    MessageScreen::show(
                        "CONTRE-REGISTRE À RENDRE",
                        "story.chapter_2.counter.ready",
                        {
                            "Le contre-registre des routes courtes est installé.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, les boutiques auront une justification plus solide pour leurs stocks de route."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoShortRouteCounterLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_short_route_counter", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : Le contre-registre des routes courtes — " + storyQuestProgressText(mainPlayer, "story_ch2_short_route_counter") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Mira.");
                lines.push_back("Effet visible : la ville distingue mieux les stocks réellement revenus des promesses écrites sur cartes corrompues.");
                MessageScreen::show("CONTRE-REGISTRE", "story.chapter_2.short_route_counter", lines, false);
                saveCurrentProgress("Chapitre 2 : contre-registre des routes courtes installé");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 10)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_route_rewrite"))
                {
                    MessageScreen::show(
                        "CARTE À RENDRE",
                        "story.chapter_2.route_rewrite.ready",
                        {
                            "Soryn et Nell ont assez d'éléments pour classer la route réécrite.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, Mira pourra protéger les comptoirs contre les cartes corrompues."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoRouteRewriteLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_route_rewrite", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : La carte qui se réécrit — " + storyQuestProgressText(mainPlayer, "story_ch2_route_rewrite") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Soryn.");
                lines.push_back("Retourne voir Soryn pour classer officiellement la preuve de route réécrite.");
                MessageScreen::show("LA CARTE QUI SE RÉÉCRIT", "story.chapter_2.route_rewrite", lines, false);
                saveCurrentProgress("Chapitre 2 : route réécrite identifiée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 9)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_cold_ink_trail"))
                {
                    MessageScreen::show(
                        "PREUVE À RENDRE",
                        "story.chapter_2.cold_ink.ready",
                        {
                            "Les deux traces de l'encre froide sont relevées.",
                            "Il reste à rendre la quête principale auprès de Soryn dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, le chapitre 2 aura une vraie piste pour la crise suivante."
                        },
                        false
                    );
                    continue;
                }

                MenuScreen coldInkMenu("ROUTE COMMERCIALE — ENCRE FROIDE", "story.chapter_2.cold_ink_menu");
                coldInkMenu.addSubtitle("Quête principale : L'encre froide de la route");
                coldInkMenu.addLine("La sacoche de Nell a indiqué deux traces à vérifier avant de choisir la prochaine crise.");
                coldInkMenu.addLine("Progression : " + storyQuestProgressText(mainPlayer, "story_ch2_cold_ink_trail") + ".");
                coldInkMenu.addBackOption("Retour", "story.chapter_2.cold_ink.back");
                coldInkMenu.addOption(1, "Suivre la halte rayée", "Scène de Route commerciale : le nom effacé revient sur le terrain.", true, "story.chapter_2.cold_ink.halt");
                coldInkMenu.addOption(2, "Mesurer la boucle du pont court", "Scène de Route commerciale : la route revient vers le relais au lieu de traverser.", true, "story.chapter_2.cold_ink.loop");
                coldInkMenu.addOption(3, "Ouvrir l'exploration complète", "Explorer librement sans perdre la piste principale.", true, "story.chapter_2.cold_ink.full_exploration");
                const int coldInkChoice = TerminalInterface::askMenuChoiceFromOptions(coldInkMenu, "Choisis une trace de route.");
                Console::clear();

                if (coldInkChoice == 0)
                {
                    continue;
                }
                if (coldInkChoice == 3)
                {
                    QuestMenu::openExploration(mainPlayer, selectedDifficulty, selectedDeathRule);
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                    saveCurrentProgress("Chapitre 2 : exploration libre depuis l'encre froide");
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoColdInkTrailLines(mainPlayer);
                if (coldInkChoice == 1)
                {
                    lines.push_back("Halte rayée : le panneau existe encore, mais son nom a été gratté avec une précision presque administrative.");
                    lines.push_back("Nell reconnaît un bon de convoi : cette halte devait recevoir des plantes, du cuir et deux caisses de clous de forge.");
                }
                else
                {
                    lines.push_back("Boucle du pont court : Orren compte les pas deux fois. Le trajet revient vers le relais sans avoir tourné.");
                    lines.push_back("Soryn note que la route ne ment pas comme une illusion : elle ment comme un registre corrigé après coup.");
                }
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_cold_ink_trail", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : L'encre froide de la route — " + storyQuestProgressText(mainPlayer, "story_ch2_cold_ink_trail") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Soryn.");
                if (storyQuestCompleted(mainPlayer, "story_ch2_cold_ink_trail"))
                {
                    lines.push_back("Les deux traces sont suffisantes. Retourne voir Soryn pour classer la preuve.");
                }
                MessageScreen::show("L'ENCRE FROIDE", "story.chapter_2.cold_ink.result", lines, false);
                saveCurrentProgress("Chapitre 2 : trace d'encre froide suivie");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 8)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_city_recovery"))
                {
                    MessageScreen::show(
                        "VILLE À NOTIFIER",
                        "story.chapter_2.city_recovery.ready",
                        {
                            "Les comptoirs ont reçu les informations de Nell.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la ville considérera les premiers stocks de route comme justifiés."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoCityRecoveryLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_city_recovery", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : Les comptoirs rouvrent un œil — " + storyQuestProgressText(mainPlayer, "story_ch2_city_recovery") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Mira.");
                lines.push_back("Effet visible : les menus de boutique et PNJ peuvent maintenant afficher un palier de ville plus crédible autour des stocks, routes courtes et demandes locales.");
                MessageScreen::show("LA VILLE RÉAGIT", "story.chapter_2.city_recovery", lines, false);
                saveCurrentProgress("Chapitre 2 : informations de Nell distribuées aux comptoirs");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 7)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);
                if (storyQuestCompleted(mainPlayer, "story_ch2_route_sack"))
                {
                    MessageScreen::show(
                        "SACOCHE À RENDRE",
                        "story.chapter_2.route_sack.ready",
                        {
                            "La sacoche de Nell a été triée : cartes, bons de convoi, halte rayée et marque d'encre froide.",
                            "Il reste à rendre la quête principale auprès de Nell dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, Mira pourra utiliser ces informations pour faire réagir la ville."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> lines = StoryCampaign::buildChapterTwoRouteSackLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_route_sack", 1);
                lines.push_back(progressed
                    ? "Quête principale mise à jour : La sacoche qui parle — " + storyQuestProgressText(mainPlayer, "story_ch2_route_sack") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Nell.");
                lines.push_back("Retourne voir Nell pour confirmer ce que la sacoche raconte vraiment.");
                MessageScreen::show("LA SACOCHE QUI PARLE", "story.chapter_2.route_sack", lines, false);
                saveCurrentProgress("Chapitre 2 : sacoche de routes exploitée");
                continue;
            }

            if (mainPlayer.getStoryStep() >= 6)
            {
                QuestMenu::syncMainStoryQuests(mainPlayer);

                if (storyQuestCompleted(mainPlayer, "story_ch2_first_rescue"))
                {
                    MessageScreen::show(
                        "NELL À RAMENER AU REGISTRE",
                        "story.chapter_2.first_rescue.ready",
                        {
                            "Nell la messagère a été sortie du convoi brisé.",
                            "Il reste à rendre la quête principale auprès d'elle dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, la route ne sera plus seulement stabilisée : elle aura ramené une voix vivante."
                        },
                        false
                    );
                    continue;
                }

                MenuScreen rescueMenu("PREMIER APPEL DU RELAIS", "story.chapter_2.first_rescue_menu");
                rescueMenu.addSubtitle("Quête principale : La voix derrière les caisses");
                rescueMenu.addLine("Le signal du relais a répondu par trois coups venus du nord : vivant, coincé, bientôt trop tard.");
                rescueMenu.addLine("Cette scène ouvre la première route plus longue sans quitter le chapitre 2.");
                rescueMenu.addBackOption("Retour", "story.chapter_2.first_rescue.back");
                rescueMenu.addOption(1, "Suivre le signal jusqu'au convoi brisé", "Déclencher le sauvetage histoire de Nell la messagère.", true, "story.chapter_2.first_rescue.start");
                const int rescueChoice = TerminalInterface::askMenuChoiceFromOptions(rescueMenu, "Choisis l'action du relais.");
                Console::clear();

                if (rescueChoice != 1)
                {
                    continue;
                }

                MessageScreen::show(
                    "LA VOIX DERRIÈRE LES CAISSES",
                    "story.chapter_2.first_rescue_intro",
                    StoryCampaign::buildChapterTwoFirstRescueLines(mainPlayer),
                    false
                );

                {
                    Combat combat;
                    mainPlayer.recordCombatStarted();
                    ShopTransactionSystem::clearBuybackAfterCombat();
                    Console::useCombatTheme();
                    combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                    Console::useNormalTheme();
                    ShopRotationSystem::markShopsDirtyAfterCombat();
                }

                if (!mainPlayer.isDead())
                {
                    mainPlayer.getQuestLog().completeQuest("story_ch2_first_rescue");
                    MessageScreen::show(
                        "NELL RESPIRE ENCORE",
                        "story.chapter_2.first_rescue_complete",
                        {
                            "Le convoi ne repartira pas aujourd'hui, mais Nell sort de derrière les caisses avec assez de souffle pour serrer sa sacoche contre elle.",
                            "Nell : « Le relais a sonné... alors j'ai répondu. Je pensais que personne ne comprendrait. »",
                            "Quête principale prête : La voix derrière les caisses.",
                            "Retourne voir Nell dans PNJ notables pour rendre le rapport et ouvrir la suite."
                        },
                        false
                    );
                    saveCurrentProgress("Chapitre 2 : Nell la messagère sauvée");
                }
                else
                {
                    saveCurrentProgress("Chapitre 2 : échec du sauvetage de Nell");
                }
                continue;
            }

            if (mainPlayer.getStoryStep() >= 5 || storyQuestTurnedIn(mainPlayer, "story_ch2_relay_threat"))
            {
                if (mainPlayer.getStoryStep() < 5)
                {
                    mainPlayer.setStoryProgress(2, 5, std::max(3, mainPlayer.getStoryCityDevelopmentLevel()));
                    QuestMenu::syncMainStoryQuests(mainPlayer);
                }

                if (storyQuestCompleted(mainPlayer, "story_ch2_relay_signal"))
                {
                    MessageScreen::show(
                        "SIGNAL À RENDRE",
                        "story.chapter_2.signal.ready",
                        {
                            "La cloche basse répond. Le registre porte une marque propre. La route possède enfin un signe que les prochaines patrouilles pourront comprendre.",
                            "Il reste à rendre la quête principale auprès de Mira dans PNJ notables ou Quêtes > Rendre une quête prête.",
                            "Après ce rapport, le relais sera considéré comme stabilisé pour cette étape."
                        },
                        false
                    );
                    continue;
                }

                std::vector<std::string> signalLines = StoryCampaign::buildChapterTwoRelaySignalLines(mainPlayer);
                const bool progressed = progressStoryQuestById(mainPlayer, "story_ch2_relay_signal", 1);
                signalLines.push_back(progressed
                    ? "Quête principale mise à jour : Le relais doit répondre — " + storyQuestProgressText(mainPlayer, "story_ch2_relay_signal") + "."
                    : "Aucune progression directe : la quête est peut-être déjà prête à rendre auprès de Mira.");
                signalLines.push_back("Retourne notifier Mira pour que le relais soit marqué comme vraiment stabilisé.");
                MessageScreen::show("LE RELAIS RÉPOND", "story.chapter_2.relay_signal", signalLines, false);
                saveCurrentProgress("Chapitre 2 : signal du relais réactivé");
                continue;
            }

            if (storyQuestCompleted(mainPlayer, "story_ch2_relay_threat"))
            {
                MessageScreen::show(
                    "MENACE À RAPPORTER",
                    "story.chapter_2.threat.ready",
                    {
                        "Les guetteurs sans feu ont été repoussés assez nettement.",
                        "Il reste à rendre la quête principale auprès d'Orren dans PNJ notables ou Quêtes > Rendre une quête prête.",
                        "Après le rapport, le relais pourra être marqué comme stabilisé."
                    },
                    false
                );
                continue;
            }

            MenuScreen threatMenu("MENACE DU RELAIS", "story.chapter_2.threat_menu");
            threatMenu.addSubtitle("Combat imposé : Les guetteurs sans feu");
            threatMenu.addLine("La route ne ment plus seulement par ses bornes. Quelqu'un garde le silence autour du relais.");
            threatMenu.addLine("Cette menace fait partie de l'histoire : elle n'est pas proposée comme contrat libre.");
            threatMenu.addBackOption("Retour", "story.chapter_2.threat.back");
            threatMenu.addOption(1, "Entrer dans l'embuscade volontairement", "Déclencher le combat imposé du relais.", true, "story.chapter_2.threat.fight");
            const int threatChoice = TerminalInterface::askMenuChoiceFromOptions(threatMenu, "Choisis l'action du relais.");
            Console::clear();

            if (threatChoice != 1)
            {
                continue;
            }

            {
                std::vector<std::string> threatIntro = {
                    "Orren t'arrête avant la sortie : « Ceux-là ne patrouillent pas. Ils attendent que les autres oublient qu'une route devrait répondre. »",
                    "Mira ajoute seulement : « Tu n'as pas besoin de comprendre toute la fumée blanche aujourd'hui. Juste de ramener assez de silence brisé pour que la ville respire. »",
                    "Tu avances vers le relais sans torche haute. Pour une fois, l'embuscade n'est pas une surprise : c'est le rendez-vous."
                };
                const std::vector<std::string> specialLines = StoryCampaign::buildChapterTwoSpecialThreatLines(mainPlayer);
                threatIntro.insert(threatIntro.end(), specialLines.begin(), specialLines.end());
                MessageScreen::show(
                    "LES GUETTEURS SANS FEU",
                    "story.chapter_2.threat_intro",
                    threatIntro,
                    false
                );
            }

            {
                Combat combat;
                mainPlayer.recordCombatStarted();
                ShopTransactionSystem::clearBuybackAfterCombat();
                Console::useCombatTheme();
                combat.launchMonsterPve(mainPlayer, selectedDifficulty, selectedDeathRule);
                Console::useNormalTheme();
                ShopRotationSystem::markShopsDirtyAfterCombat();
            }

            if (!mainPlayer.isDead())
            {
                mainPlayer.getQuestLog().completeQuest("story_ch2_relay_threat");
                MessageScreen::show(
                    "GUETTEURS REPOUSSÉS",
                    "story.chapter_2.threat_done",
                    {
                        "Le combat ne donne pas toutes les réponses, mais il donne une certitude : le relais était gardé.",
                        "Quête principale prête : Les guetteurs sans feu.",
                        "Retourne voir Orren pour rendre le rapport et stabiliser le relais."
                    },
                    false
                );
                saveCurrentProgress("Chapitre 2 : menace du relais repoussée");
            }
            else
            {
                saveCurrentProgress("Chapitre 2 : échec contre les guetteurs sans feu");
            }
            continue;
        }
        if (chapterChoice == 5)
        {
            QuestMenu::openQuestHub(mainPlayer);
            continue;
        }
        if (chapterChoice == 6)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            continue;
        }
    }
}

void Game::playStoryWhiteFogPrologue()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        mainPlayer.startStoryMode();
    }

    if (mainPlayer.getStoryStep() >= 2 && mainPlayer.getInventory().getWeaponCount() > 0)
    {
        MessageScreen::show(
            "PROLOGUE DÉJÀ FRANCHI",
            "story.white_fog.already_done",
            {
                "La fumée blanche a déjà reculé pour ce personnage.",
                "Tu peux relire l’introduction, mais le kit de départ a déjà été trouvé.",
                "Progression : " + mainPlayer.getStoryProgressLabel()
            },
            false
        );
        return;
    }

    mainPlayer.getInventory().clearAll();
    mainPlayer.setStoryProgress(1, 1, std::max(0, mainPlayer.getStoryCityDevelopmentLevel()));

    MessageScreen::show("PROLOGUE — MISSION ORDINAIRE", "story.white_fog.intro", StoryCampaign::buildWhiteFogPrologueLines(mainPlayer));
    MessageScreen::show("LA FUMÉE MANGE LES NOMS", "story.white_fog.memory_loss", StoryCampaign::buildWhiteFogMemoryLossLines(mainPlayer), false);

    MenuScreen firstChoice("FORÊT BLANCHE", "story.white_fog.choice_1");
    firstChoice.addSubtitle("Aucun équipement. Aucun souvenir fiable.");
    firstChoice.addLine("La brume mange les troncs. Elle ne cache pas la forêt : elle la remplace.");
    firstChoice.addLine("Tes mains sont vides. Même ton nom paraît venir de quelqu’un d’autre.");
    firstChoice.addOption(1, "Avancer lentement vers les arbres moins blancs", "Garder une direction sans courir dans la brume.", true, "story.white_fog.slow");
    firstChoice.addOption(2, "Courir droit devant", "Chercher une sortie immédiate, au risque de perdre le peu de repères restants.", true, "story.white_fog.run");
    firstChoice.addOption(3, "Appeler à l’aide", "Tester si quelqu’un répond dans la fumée.", true, "story.white_fog.call");
    int first = TerminalInterface::askMenuChoiceFromOptions(firstChoice, "Choisis ta première réaction.");
    Console::clear();

    std::vector<std::string> report;
    if (first == 1)
    {
        report.push_back("Tu avances lentement. La fumée tire sur tes souvenirs, mais elle ne parvient pas à te faire tourner en rond tout de suite.");
    }
    else if (first == 2)
    {
        report.push_back("Tu cours. La forêt te laisse faire trois secondes, puis remet un arbre devant toi. Tu comprends que la panique nourrit la brume.");
    }
    else
    {
        report.push_back("Tu appelles. Une voix répond avec ton intonation exacte. Tu décides de ne pas lui confier ta direction.");
    }

    {
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogFirstReactionLines(mainPlayer, first);
        report.insert(report.end(), specialLines.begin(), specialLines.end());
    }
    report.push_back("Un corps bas traverse la brume. Pas assez net pour être nommé. Assez proche pour attaquer.");
    MessageScreen::show("RENCONTRE", "story.white_fog.encounter_1", report, false);

    MenuScreen encounter("RENCONTRE SANS ARME", "story.white_fog.encounter_choice");
    encounter.addSubtitle("Combat impossible : seulement parade ou fuite.");
    encounter.addLine("Tu n’as ni arme, ni armure, ni potion fiable.");
    encounter.addLine("L’objectif n’est pas de gagner. L’objectif est de sortir vivant.");
    encounter.addOption(1, "Parer avec les avant-bras et reculer", "Réduire le choc et chercher une ouverture.", true, "story.white_fog.block");
    encounter.addOption(2, "Fuir vers les arbres moins serrés", "Tenter de casser la ligne d’attaque.", true, "story.white_fog.escape");
    int second = TerminalInterface::askMenuChoiceFromOptions(encounter, "Choisis : parer ou fuir.");
    Console::clear();

    if (second == 1)
    {
        std::vector<std::string> blockLines = {
            "Le choc remonte jusqu’aux épaules.",
            "Tu ne gagnes pas le combat, mais tu refuses de tomber là.",
            "La créature hésite juste assez pour ouvrir un passage."
        };
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogEncounterReactionLines(mainPlayer, second);
        blockLines.insert(blockLines.end(), specialLines.begin(), specialLines.end());
        MessageScreen::show("PARADE", "story.white_fog.block_result", blockLines, false);
    }
    else
    {
        std::vector<std::string> escapeLines = {
            "Tu fuis sans regarder derrière.",
            "La brume essaie de remettre tes pas au même endroit, mais le sol descend enfin.",
            "Quelque chose grogne derrière toi, puis abandonne."
        };
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogEncounterReactionLines(mainPlayer, second);
        escapeLines.insert(escapeLines.end(), specialLines.begin(), specialLines.end());
        MessageScreen::show("FUITE", "story.white_fog.escape_result", escapeLines, false);
    }

    MenuScreen exitChoice("SORTIE DE FORÊT", "story.white_fog.choice_2");
    exitChoice.addSubtitle("Une silhouette au sol");
    exitChoice.addLine("La fumée s’ouvre sur un fossé humide. Un cadavre repose contre une racine, déjà trop froid pour raconter son histoire.");
    exitChoice.addLine("À côté de lui : un paquet simple, abîmé, mais utilisable.");
    exitChoice.addOption(1, "Prendre le paquet et fermer les yeux du mort", "Récupérer le kit de départ sans traiter le corps comme un coffre gratuit.", true, "story.white_fog.take_respect");
    exitChoice.addOption(2, "Prendre seulement ce qui peut sauver ta vie", "Rester pratique : la forêt n’attend pas.", true, "story.white_fog.take_practical");
    exitChoice.addOption(3, "Fouiller vite et partir", "Sortie nerveuse : survivre d’abord, réfléchir ensuite.", true, "story.white_fog.take_fast");
    int third = TerminalInterface::askMenuChoiceFromOptions(exitChoice, "Choisis comment récupérer le premier équipement.");
    Console::clear();

    mainPlayer.initializeStarterInventory(selectedDifficulty);
    mainPlayer.setStoryProgress(1, 2, std::max(1, mainPlayer.getStoryCityDevelopmentLevel()));
    saveCurrentProgress("Prologue fumée blanche terminé");

    std::vector<std::string> endLines;
    if (third == 1)
    {
        endLines.push_back("Tu prends le paquet, puis tu fermes les yeux du mort. Ce n’est pas grand-chose, mais la forêt semble moins blanche pendant une seconde.");
    }
    else if (third == 2)
    {
        endLines.push_back("Tu prends seulement le nécessaire. La honte viendra peut-être plus tard. Pour l’instant, respirer suffit.");
    }
    else
    {
        endLines.push_back("Tu fouilles vite. La peur te rend maladroit, mais vivant. La brume n’aura pas tout aujourd’hui.");
    }
    {
        const std::vector<std::string> specialLines = StoryCampaign::buildWhiteFogKitReactionLines(mainPlayer, third);
        endLines.insert(endLines.end(), specialLines.begin(), specialLines.end());
    }
    endLines.push_back("Kit de départ récupéré. La vraie histoire peut commencer.");
    endLines.push_back("Au loin, une ville tient à peine debout. Ce sera ton premier refuge, pas encore ta maison.");
    MessageScreen::show("PROLOGUE TERMINÉ", "story.white_fog.completed", endLines, false);
}

void Game::launchEphemeralSandboxCloneFromStory()
{
    if (!mainPlayer.hasStoryModeStarted())
    {
        MessageScreen::show(
            "CLONE IMPOSSIBLE",
            "story.ephemeral.no_story",
            {
                "Le clone éphémère sert à quitter temporairement une route histoire déjà engagée.",
                "Commence d’abord l’histoire, puis reviens ici si tu veux une session libre sans sauvegarde."
            },
            false
        );
        return;
    }

    MenuScreen choice("BAC À SABLE ÉPHÉMÈRE", "story.ephemeral.choice");
    choice.addSubtitle("Abandonner le monde à sa destinée, ou cloner juste aujourd’hui ?");
    choice.addLine("Passer réellement en bac à sable abandonnerait la route histoire de ce personnage.");
    choice.addLine("Le clone éphémère permet de jouer librement pendant la session sans sauvegarder ni perturber l’histoire.");
    choice.addOption(0, "Retour", "Ne rien changer.", true, "story.ephemeral.back");
    choice.addOption(1, "Abandonner le monde à sa destinée", "Quitter mentalement la route histoire. Pour l’instant, cette option reste une décision forte à confirmer plus tard.", true, "story.ephemeral.abandon");
    choice.addOption(2, "Créer un clone pour aujourd’hui", "Session non sauvegardée, supprimée à la fin ou au prochain nettoyage si elle existe encore.", true, "story.ephemeral.clone");
    const int selected = TerminalInterface::askMenuChoiceFromOptions(choice, "Choisis la sortie temporaire ou le retour.");
    Console::clear();

    if (selected == 0)
    {
        return;
    }

    if (selected == 1)
    {
        MessageScreen::show(
            "DESTIN REFUSÉ",
            "story.ephemeral.abandon_scaffold",
            {
                "Cette option représentera plus tard un vrai abandon du fil histoire.",
                "Pour éviter une erreur définitive pendant le développement, elle reste informative pour l’instant.",
                "Utilise plutôt le clone éphémère si tu veux te régaler sans casser la campagne."
            },
            false
        );
        return;
    }

    Player savedStoryPlayer = mainPlayer;
    const GameMode savedMode = selectedMode;
    const bool previousEphemeral = ephemeralSandboxSession;

    mainPlayer.setClone(true);
    ephemeralSandboxSession = true;

    MessageScreen::show(
        "CLONE ÉPHÉMÈRE CRÉÉ",
        "story.ephemeral.clone_created",
        {
            "Un clone de session est créé pour aujourd’hui.",
            "Il peut jouer dans le bac à sable, mais aucune sauvegarde réelle ne sera écrite.",
            "À la fin de la session libre, le personnage histoire original sera restauré."
        },
        false
    );

    chooseGameMode();
    if (selectedMode == GameMode::Story)
    {
        MessageScreen::show(
            "CLONE REFUSÉ PAR L’HISTOIRE",
            "story.ephemeral.story_blocked",
            {
                "Le clone éphémère ne sert pas à écrire l’histoire à la place du vrai personnage.",
                "Retour au menu histoire."
            },
            false
        );
    }
    else
    {
        displaySelectedMode();
        launchSelectedMode();
    }

    mainPlayer = savedStoryPlayer;
    selectedMode = savedMode;
    ephemeralSandboxSession = previousEphemeral;
    saveCurrentProgress("Retour du clone éphémère vers la route histoire");

    MessageScreen::show(
        "ROUTE HISTOIRE RESTAURÉE",
        "story.ephemeral.restored",
        {
            "Le clone éphémère se dissipe.",
            "La sauvegarde histoire reprend exactement avec le personnage original.",
            "Aucune ressource gagnée en clone n’a été transférée."
        },
        false
    );
}

void Game::showStoryCompletionSandboxRule() const
{
    MessageScreen::show(
        "APRÈS LA FIN",
        "story.completion.sandbox_rule",
        StoryCampaign::buildStoryCompletionLines(mainPlayer),
        false
    );
}

// EN: launchChallengeBoard declares or implements a focused behavior used by this module.
// FR: launchChallengeBoard déclare ou implémente un comportement précis utilisé par ce module.
void Game::launchChallengeBoard()
{
    QuestMenu::openQuestHub(mainPlayer);
}

void Game::addOutOfCombatUtilityOptions(MenuScreen& screen, bool inventoryAvailable, bool saveAvailable) const
{
    (void)saveAvailable;
    std::string description = "Inventaire, sauvegarde, saisie libre, paramètres et données spéciales.";
    if (!inventoryAvailable)
    {
        description = "Sauvegarde, saisie libre, paramètres et données spéciales. Inventaire indisponible ici.";
    }

    screen.addOption(
        UtilityChoiceOutOfCombatMenu,
        "Menu hors combat",
        description,
        true,
        "utility.out_of_combat_menu",
        makeUtilityItemData(mainPlayer, "menu", "Menu hors combat", description)
    );
}

void Game::openOutOfCombatUtilityMenu(bool inventoryAvailable)
{
    bool menuOpen = true;
    while (menuOpen)
    {
        MenuScreen screen("MENU HORS COMBAT", "utility.out_of_combat.menu");
        screen.addSubtitle("Options regroupées pour éviter d'afficher les utilitaires partout");
        screen.addLine("Fréquence des indications : " + mainPlayer.getInterfaceHintFrequencyLabel());
        screen.addBackOption();
        screen.addOption(
            1,
            "Parler au gardien / saisie libre",
            "Écrire une phrase, un choix ou une commande.",
            true,
            "utility.guardian",
            makeUtilityItemData(mainPlayer, "guardian", "Gardien du monde", "Saisie libre hors combat.")
        );
        screen.addOption(
            2,
            "Inventaire",
            "Gérer objets, équipement et potions hors combat.",
            inventoryAvailable,
            "utility.inventory",
            makeUtilityItemData(mainPlayer, "open", "Inventaire", "Gestion hors combat.", inventoryAvailable ? "Disponible" : "Indisponible")
        );
        screen.addOption(
            3,
            "Paramètres",
            "Changer les réglages du personnage, dont la fréquence des indications.",
            true,
            "utility.settings",
            makeUtilityItemData(mainPlayer, "settings", "Paramètres", "Réglages modifiables à tout moment.")
        );
        screen.addOption(
            4,
            "Sauvegarder",
            "Sauvegarder sans quitter la partie.",
            true,
            "utility.quick_save",
            makeUtilityItemData(mainPlayer, "save", "Sauvegarder", "Sauvegarde rapide.", "Disponible")
        );
        screen.addOption(
            5,
            "Sauvegarder et quitter",
            "Sauvegarder puis fermer Dinotofu.",
            true,
            "utility.save_quit",
            makeUtilityItemData(mainPlayer, "save", "Sauvegarder et quitter", "Fermeture propre après sauvegarde.", "Disponible")
        );

        if (mainPlayer.isAlteredByCheats())
        {
            screen.addOption(
                6,
                "Données altérées",
                "Voir les altérations connues de ce personnage.",
                true,
                "utility.altered_data",
                makeUtilityItemData(mainPlayer, "inspect", "Données altérées", "Informations déjà révélées pour ce personnage.", "Altéré")
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option du menu hors combat.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            openGuardianInputMenu();
            continue;
        }
        if (choice == 2)
        {
            if (!inventoryAvailable)
            {
                MessageScreen::show("INVENTAIRE", "utility.inventory.unavailable", {"L'inventaire n'est pas disponible sur cet écran."});
                continue;
            }
            InventoryMenu::open(mainPlayer);
            saveCurrentProgress("Inventaire hors combat");
            Console::clear();
            continue;
        }
        if (choice == 3)
        {
            openInterfaceSettingsMenu();
            continue;
        }
        if (choice == 4)
        {
            saveCurrentProgress("Sauvegarde rapide hors combat");
            Console::waitForEnter();
            Console::clear();
            continue;
        }
        if (choice == 5)
        {
            saveCurrentProgress("Sauvegarder et quitter");
            MessageScreen::show(
                "SAUVEGARDE",
                "utility.save_quit.done",
                {"Progression sauvegardée. Fermeture de Dinotofu."},
                false
            );
            std::exit(0);
        }
        if (choice == 6 && mainPlayer.isAlteredByCheats())
        {
            CheatManager::openAlteredDataMenu(mainPlayer, selectedDifficulty, selectedDeathRule);
            saveCurrentProgress("Données altérées");
            continue;
        }
    }
}

void Game::openInterfaceSettingsMenu()
{
    bool menuOpen = true;
    while (menuOpen)
    {
        MenuScreen screen("PARAMÈTRES", "utility.settings.menu");
        screen.addSubtitle("Réglages modifiables à tout moment");
        screen.addLine("Fréquence actuelle des indications : " + mainPlayer.getInterfaceHintFrequencyLabel());
        screen.addLine("null : aucune indication volontaire hors inspection ou avertissement vital.");
        screen.addLine("faible : valeur par défaut, seulement les alertes importantes ou contextes très liés.");
        screen.addLine("normal : un peu plus d'indices sur quêtes, routes et équipement.");
        screen.addLine("forte : plus bavard, utile si tu veux beaucoup de guidage.");
        screen.addBackOption();
        screen.addOption(1, "Fréquence : null", "Désactive les indications volontaires autant que possible.", true, "settings.hints.null");
        screen.addOption(2, "Fréquence : faible", "Réglage par défaut : rare, surtout utile et non intrusif.", true, "settings.hints.low");
        screen.addOption(3, "Fréquence : normal", "Affiche davantage d'indices contextuels.", true, "settings.hints.normal");
        screen.addOption(4, "Fréquence : forte", "Affiche beaucoup plus d'indications et rappels.", true, "settings.hints.high");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une fréquence d'indications.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        std::string value;
        if (choice == 1) value = "null";
        else if (choice == 2) value = "faible";
        else if (choice == 3) value = "normal";
        else if (choice == 4) value = "forte";

        if (!value.empty())
        {
            mainPlayer.setInterfaceHintFrequency(value);
            saveCurrentProgress("Paramètres d'indications");
            MessageScreen::show(
                "PARAMÈTRES",
                "utility.settings.hints.changed",
                {"Fréquence des indications réglée sur : " + mainPlayer.getInterfaceHintFrequencyLabel() + "."}
            );
        }
    }
}

bool Game::handleOutOfCombatUtilityChoice(int choice, bool inventoryAvailable)
{
    if (choice == UtilityChoiceOutOfCombatMenu)
    {
        openOutOfCombatUtilityMenu(inventoryAvailable);
        return true;
    }

    if (choice == UtilityChoiceSettings)
    {
        openInterfaceSettingsMenu();
        return true;
    }

    if (choice == UtilityChoiceGuardian)
    {
        openGuardianInputMenu();
        return true;
    }

    if (choice == UtilityChoiceInventory)
    {
        if (!inventoryAvailable)
        {
            MessageScreen::show(
                "INVENTAIRE",
                "utility.inventory.unavailable",
                {"L'inventaire n'est pas disponible sur cet écran."}
            );
            return true;
        }

        InventoryMenu::open(mainPlayer);
        saveCurrentProgress("Inventaire hors combat");
        Console::clear();
        return true;
    }

    if (choice == UtilityChoiceQuickSave)
    {
        saveCurrentProgress("Sauvegarde rapide hors combat");
        Console::waitForEnter();
        Console::clear();
        return true;
    }

    if (choice == UtilityChoiceSaveQuit)
    {
        saveCurrentProgress("Sauvegarder et quitter");
        MessageScreen::show(
            "SAUVEGARDE",
            "utility.save_quit.done",
            {"Progression sauvegardée. Fermeture de Dinotofu."},
            false
        );
        std::exit(0);
    }

    if (choice == UtilityChoiceAlteredData && mainPlayer.isAlteredByCheats())
    {
        CheatManager::openAlteredDataMenu(mainPlayer, selectedDifficulty, selectedDeathRule);
        saveCurrentProgress("Données altérées");
        return true;
    }

    return false;
}

void Game::openGuardianInputMenu()
{
    MenuScreen screen("GARDIEN DU MONDE", "utility.guardian.input");
    screen.addSubtitle("Saisie libre hors combat");
    screen.addLine("Écris ce que tu veux transmettre au bord du monde.");
    screen.addLine("Le gardien répondra si ce n'est pas une commande reconnue.");
    screen.setTextInput("Choix, texte ou commande", "Saisie libre", true, 0, 120);

    TerminalInterface::renderMenuScreen(screen);

    std::string input;
    Console::readLine(input, true);
    Console::clear();

    if (CheatManager::tryActivateHiddenCode(mainPlayer, selectedDifficulty, selectedDeathRule, input))
    {
        saveCurrentProgress("Saisie du gardien");
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MessageScreen::show(
        "GARDIEN DU MONDE",
        "utility.guardian.reply",
        {guardianAnswerFor(input)}
    );
}

// EN: openPostCombatMenu declares or implements a focused behavior used by this module.
// FR: openPostCombatMenu déclare ou implémente un comportement précis utilisé par ce module.
bool Game::openPostCombatMenu()
{
    bool menuOpen = true;

    while (menuOpen)
    {
        const bool hasLastCombatRecap = lastCombatRecap.available;
        MenuScreen screen = PostCombatMenu::buildScreen(mainPlayer, hasLastCombatRecap);
        addOutOfCombatUtilityOptions(screen, true, true);

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir une option affichée."
        );

        Console::clear();

        if (handleOutOfCombatUtilityChoice(choice, true))
        {
            continue;
        }

        if (choice == 0)
        {
            return true;
        }
        else if (choice == 1)
        {
            ShopMenu::open(mainPlayer);
            saveCurrentProgress("Passage en boutique");
        }
        else if (choice == 2)
        {
            StatisticsMenu::open(mainPlayer, selectedDifficulty);
        }
        else if (choice == 3)
        {
            AttributeMenu::displayLockedDevelopmentMessage();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 4)
        {
            QuestMenu::consultOnly(mainPlayer);
        }
        else if (choice == 5)
        {
            QuestMenu::openLocations(mainPlayer);
            saveCurrentProgress("Lieux visitables");
        }
        else if (choice == 6)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            saveCurrentProgress("PNJ notables");
        }
        else if (choice == 7)
        {
            openExchangeMenu();
            saveCurrentProgress("Échange entre personnages");
        }
        else if (choice == 8)
        {
            mainPlayer.displaySkillProgress();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 9)
        {
            mainPlayer.displaySimpleEquipment();
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 10)
        {
            displayLastCombatRecap();
        }
    }

    return false;
}



// EN: openExchangeMenu declares or implements a focused behavior used by this module.
// FR: openExchangeMenu déclare ou implémente un comportement précis utilisé par ce module.
void Game::openExchangeMenu()
{
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();

    if (accounts.empty())
    {
        MenuScreen emptyScreen("ÉCHANGE / DON", "exchange.no_account");
        emptyScreen.addLine("Aucun autre compte disponible pour un échange.");
        TerminalInterface::renderMenuScreen(emptyScreen, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    int accountChoice = askExchangeAccountIndex(accounts, accountName);

    if (accountChoice < 0)
    {
        return;
    }

    std::string targetAccount = accounts[static_cast<std::size_t>(accountChoice)].accountName;
    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(targetAccount);

    if (characters.empty())
    {
        MenuScreen emptyCharacterScreen("PERSONNAGE CIBLE", "exchange.character.empty");
        emptyCharacterScreen.addLine("Ce compte n'a aucun personnage jouable.");
        emptyCharacterScreen.addLine("Compte : " + targetAccount);
        TerminalInterface::renderMenuScreen(emptyCharacterScreen, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    int characterChoice = askExchangeCharacterIndex(
        characters,
        targetAccount,
        accountName,
        mainPlayer.getName()
    );

    if (characterChoice < 0)
    {
        return;
    }

    CharacterSaveSummary targetSummary = characters[static_cast<std::size_t>(characterChoice)];

    if (targetAccount == accountName && targetSummary.characterName == mainPlayer.getName())
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.same_character",
            {"Tu ne peux pas échanger avec le même personnage."}
        );
        return;
    }

    Player targetPlayer;
    DifficultyMode targetDifficulty = DifficultyMode::Normal;
    DeathRuleMode targetDeathRule = DeathRuleRules::defaultForDifficulty(targetDifficulty);

    if (!SaveManager::loadPlayerSnapshot(targetSummary, targetPlayer, targetDifficulty, targetDeathRule))
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.load_target_failed",
            {"Impossible de charger le personnage cible."}
        );
        return;
    }

    if (mainPlayer.isAlteredByCheats() || targetPlayer.isAlteredByCheats())
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.altered",
            {
                "Échange impossible.",
                "Un personnage altéré ne peut pas transférer de ressources réelles."
            }
        );
        return;
    }

    if (mainPlayer.isClone() || targetPlayer.isClone())
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.clone",
            {
                "Un clone ne peut pas donner ou recevoir d'objets réels.",
                "Le registre refuse les silhouettes copiées dans les échanges réels."
            }
        );
        return;
    }

    bool currentIsDefinitive = DifficultyRules::isPermanentDeath(selectedDifficulty, selectedDeathRule);
    bool targetIsDefinitive = DifficultyRules::isPermanentDeath(targetDifficulty, targetDeathRule);

    if (currentIsDefinitive != targetIsDefinitive)
    {
        Console::clear();
        MessageScreen::show(
            "ÉCHANGE IMPOSSIBLE",
            "exchange.forbidden.lethal_mismatch",
            {
                "Un personnage avec mort définitive est considéré comme une vraie existence.",
                "Un personnage sans mort définitive reste une simulation plus sûre.",
                "Pour éviter les abus, il faut deux personnages avec la même règle de mort."
            }
        );
        return;
    }

    bool open = true;

    while (open)
    {
        Console::clear();

        MenuScreen exchangeScreen("ÉCHANGE / DON", "exchange.action");
        exchangeScreen.addLine("Source principale : " + mainPlayer.getName());
        exchangeScreen.addLine("Cible : " + targetPlayer.getName() + " (" + targetAccount + ")");
        exchangeScreen.addBackOption();
        exchangeScreen.addOption(1, "Donner de l'or", "Transfert direct depuis " + mainPlayer.getName() + ".", true, "exchange.give.gold");
        exchangeScreen.addOption(2, "Donner une arme", "Impossible avec l'arme équipée.", true, "exchange.give.weapon");
        exchangeScreen.addOption(3, "Donner une armure", "Impossible avec l'armure portée.", true, "exchange.give.armor");
        exchangeScreen.addOption(4, "Donner un consommable", "Transfert d'un objet consommable.", true, "exchange.give.consumable");
        exchangeScreen.addOption(5, "Donner un matériau", "Transfert avec quantité choisie.", true, "exchange.give.material");
        exchangeScreen.addOption(6, "Recevoir depuis le personnage cible", "Inverse la source et la cible pour cette action.", true, "exchange.receive");
        exchangeScreen.addFooterLine("L'estimation de valeur s'affiche après le choix pour garder l'écran lisible.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            exchangeScreen,
            "Veuillez choisir une option affichée."
        );

        Player* giver = &mainPlayer;
        Player* receiver = &targetPlayer;

        if (choice == 0)
        {
            break;
        }

        if (choice == 6)
        {
            giver = &targetPlayer;
            receiver = &mainPlayer;

            Console::clear();
            MenuScreen receiveScreen("RECEVOIR", "exchange.receive.type");
            receiveScreen.addLine("Depuis : " + giver->getName());
            receiveScreen.addLine("Vers : " + receiver->getName());
            receiveScreen.addBackOption("Annuler");
            receiveScreen.addOption(1, "Or", "Transférer une quantité d'or.", true, "exchange.receive.gold");
            receiveScreen.addOption(2, "Arme", "Choisir une arme non équipée.", true, "exchange.receive.weapon");
            receiveScreen.addOption(3, "Armure", "Choisir une armure non portée.", true, "exchange.receive.armor");
            receiveScreen.addOption(4, "Consommable", "Choisir un consommable.", true, "exchange.receive.consumable");
            receiveScreen.addOption(5, "Matériau", "Choisir un matériau et une quantité.", true, "exchange.receive.material");

            choice = TerminalInterface::askMenuChoiceFromOptions(
                receiveScreen,
                "Veuillez choisir une ressource affichée."
            );

            if (choice == 0)
            {
                continue;
            }
        }

        Console::clear();
        displayExchangeValueEstimation(*giver, *receiver);

        if (choice == 1)
        {
            int amount = MessageScreen::askQuantity(
                "OR À TRANSFÉRER",
                "exchange.gold.quantity",
                {
                    giver->getName() + " possède " + std::to_string(giver->getInventory().getGold()) + " or.",
                    "Montant à transférer ?"
                },
                0,
                giver->getInventory().getGold(),
                "Montant invalide."
            );

            if (amount > 0 && giver->getInventory().spendGold(amount))
            {
                receiver->getInventory().earnGold(amount);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.gold.success", {std::to_string(amount) + " or transféré."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.gold.none", {"Aucun or transféré."}, false);
            }
        }
        else if (choice == 2)
        {
            int index = askExchangeWeaponIndex(*giver);

            if (index >= 0)
            {
                Weapon weapon = giver->getInventory().getWeapon(index);
                receiver->getInventory().addWeapon(weapon);
                giver->getInventory().removeWeapon(index);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.weapon.success", {"Arme transférée : " + weapon.getName() + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.weapon.none", {"Aucune arme transférée."}, false);
            }
        }
        else if (choice == 3)
        {
            int index = askExchangeArmorIndex(*giver);

            if (index >= 0)
            {
                Armor armor = giver->getInventory().getArmor(index);
                receiver->getInventory().addArmor(armor);
                giver->getInventory().removeArmor(index);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.armor.success", {"Armure transférée : " + armor.getName() + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.armor.none", {"Aucune armure transférée."}, false);
            }
        }
        else if (choice == 4)
        {
            int index = askExchangeConsumableIndex(*giver);

            if (index >= 0)
            {
                Consumable consumable = giver->getInventory().getConsumable(index);
                receiver->getInventory().addConsumable(consumable);
                giver->getInventory().removeConsumable(index);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.consumable.success", {"Consommable transféré : " + consumable.getName() + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.consumable.none", {"Aucun consommable transféré."}, false);
            }
        }
        else if (choice == 5)
        {
            int index = askExchangeMaterialIndex(*giver);

            if (index >= 0)
            {
                Material material = giver->getInventory().getMaterial(index);
                int amount = MessageScreen::askQuantity(
                    "QUANTITÉ À TRANSFÉRER",
                    "exchange.material.quantity",
                    {
                        "Matériau : " + material.getName(),
                        "Maximum transférable : x" + std::to_string(material.getQuantity())
                    },
                    1,
                    material.getQuantity(),
                    "Quantité invalide."
                );
                material.setQuantity(amount);
                receiver->getInventory().addMaterial(material);
                giver->getInventory().removeMaterialQuantity(index, amount);
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.material.success", {"Matériau transféré : " + material.getName() + " x" + std::to_string(amount) + "."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.material.none", {"Aucun matériau transféré."}, false);
            }
        }

        SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty, selectedDeathRule);
        SaveManager::savePlayerSnapshot(targetPlayer, targetAccount, targetDifficulty, targetDeathRule);

        MessageScreen::show(
            "ÉCHANGE SAUVEGARDÉ",
            "exchange.saved",
            {"Le transfert est enregistré dans les deux registres."}
        );
    }

    Console::clear();
}

// EN: saveCurrentProgress declares or implements a focused behavior used by this module.
// FR: saveCurrentProgress déclare ou implémente un comportement précis utilisé par ce module.
void Game::saveCurrentProgress(const std::string& reason) const
{
    if (ephemeralSandboxSession)
    {
        MessageScreen::show(
            "SAUVEGARDE IGNORÉE",
            "save.current_progress.ephemeral_skipped",
            {
                "Clone éphémère actif : " + reason + ".",
                "Rien n’est écrit dans les sauvegardes réelles."
            },
            false
        );
        return;
    }

    if (mainPlayer.getName().empty() || mainPlayer.getName() == "Inconnu")
    {
        return;
    }

    if (SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty, selectedDeathRule))
    {
        MessageScreen::show(
            "SAUVEGARDE",
            "save.current_progress.ok",
            {
                "Sauvegarde préparée : " + reason + ".",
                "Chemin : " + SaveManager::getCharacterSavePath(accountName, mainPlayer.getName())
            },
            false
        );
    }
    else
    {
        MessageScreen::show(
            "SAUVEGARDE",
            "save.current_progress.failed",
            {"Sauvegarde impossible pour le moment."},
            false
        );
    }
}

std::string Game::getDifficultyName() const
{
    switch (selectedDifficulty)
    {
        case DifficultyMode::Easy:
            return "Facile";

        case DifficultyMode::Hard:
            return "Difficile";

        case DifficultyMode::Nightmare:
            return "Cauchemar";

        case DifficultyMode::Lethal:
            return "Léthal";

        case DifficultyMode::Normal:
        default:
            return "Normal";
    }
}

std::string Game::getDeathRuleName() const
{
    return DeathRuleRules::displayName(
        DeathRuleRules::normalizeForDifficulty(selectedDifficulty, selectedDeathRule)
    );
}
