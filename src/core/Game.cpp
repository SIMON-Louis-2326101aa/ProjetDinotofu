// EN: Game.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Game.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "core/Game.hpp"
#include "diagnostic/RuntimeLog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "core/VersionInfo.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "boss/BossCatalog.hpp"
#include "entity/Monster.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/Money.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/progression/AttributeMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/inventory/InventorySelection.hpp"
#include "interface/menu/PostCombatMenu.hpp"
#include "interface/menu/training/TrainingGroundMenu.hpp"
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
#include <exception>
#include <set>
#include <array>


namespace
{
    constexpr int UtilityChoiceOutOfCombatMenu = 8;
    constexpr int UtilityChoiceGuardian = 90;
    constexpr int UtilityChoiceInventory = 91;
    constexpr int UtilityChoiceQuickSave = 92;
    constexpr int UtilityChoiceSaveQuit = 93;
    constexpr int UtilityChoiceAlteredData = 94;
    constexpr int UtilityChoiceSettings = 95;
    constexpr int UtilityChoiceSaveReturnMenu = 96;
    constexpr int UtilityChoiceTeam = 97;

    struct ReturnToActivityMenuRequest final : public std::exception
    {
        const char* what() const noexcept override { return "return_to_activity_menu"; }
    };

    std::vector<std::string> buildCurrentLoadoutSynergyLines(const Player& player)
    {
        std::vector<std::string> lines;
        int score = 0;
        int warning = 0;

        if (player.hasEquippedWeapon())
        {
            const Weapon weapon = player.getEquippedWeapon();
            const bool weaponBonus = CombatClassSystem::hasWeaponAffinity(player, weapon.getType(), weapon.getName());
            const bool weaponMalus = CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, weapon.getType(), weapon.getName()) < 0
                || CombatClassSystem::getWeaponHandlingDamagePercent(player, weapon.getType(), weapon.getName()) < 100;
            if (weaponBonus)
            {
                score += 2;
                lines.push_back("Synergie arme : " + weapon.getName() + " [bonus de classe] — " + CombatClassSystem::getWeaponAffinityLabel(player, weapon.getType(), weapon.getName()) + ".");
            }
            else if (weaponMalus)
            {
                warning += 2;
                lines.push_back("Synergie arme : " + weapon.getName() + " [malus de classe] — " + CombatClassSystem::getWeaponHandlingLabel(player, weapon.getType(), weapon.getName()) + ".");
            }
            else
            {
                lines.push_back("Synergie arme : " + weapon.getName() + " — neutre, utilisable sans vraie affinité ni gros contresens.");
            }
        }
        else
        {
            warning += 1;
            lines.push_back("Synergie arme : aucune arme équipée, donc les bonus/malus de classe ne peuvent pas vraiment s'exprimer.");
        }

        if (player.hasEquippedArmor())
        {
            const Armor armor = player.getEquippedArmor();
            const bool armorBonus = CombatClassSystem::hasArmorAffinity(player, armor.getType(), armor.getName());
            const bool armorMalus = CombatClassSystem::getArmorHandlingDamageReductionAdjustment(player, armor.getType(), armor.getName(), 24) < 0
                || CombatClassSystem::getArmorHandlingEscapeAdjustment(player, armor.getType(), armor.getName()) < 0;
            if (armorBonus)
            {
                score += 2;
                lines.push_back("Synergie armure : " + armor.getName() + " [bonus de classe] — " + CombatClassSystem::getArmorHandlingLabel(player, armor.getType(), armor.getName()) + ".");
            }
            else if (armorMalus)
            {
                warning += 2;
                lines.push_back("Synergie armure : " + armor.getName() + " [malus de classe] — " + CombatClassSystem::getArmorHandlingLabel(player, armor.getType(), armor.getName()) + ".");
            }
            else
            {
                lines.push_back("Synergie armure : " + armor.getName() + " — neutre, correcte sans raconter parfaitement le rôle.");
            }
        }
        else
        {
            warning += 1;
            lines.push_back("Synergie armure : aucune protection équipée, donc le rôle défensif repose surtout sur la classe et les actifs.");
        }

        if (score >= 4 && warning == 0)
        {
            lines.push_back("Lecture globale : build très cohérent, les effets de classe devraient se sentir sans avoir besoin de gonfler les nombres gratuitement.");
        }
        else if (score >= 2 && warning <= 1)
        {
            lines.push_back("Lecture globale : build plutôt cohérent, quelques choix restent perfectibles mais le rôle est lisible.");
        }
        else if (warning >= 3)
        {
            lines.push_back("Lecture globale : build contradictoire, les malus risquent de rendre certaines compétences moins propres malgré de bonnes statistiques brutes.");
        }
        else
        {
            lines.push_back("Lecture globale : build neutre, jouable, mais sans vraie identité mécanique forte pour l'instant.");
        }
        return lines;
    }

    std::vector<std::string> buildWorldVisitAmbienceLines(const Player& player, bool questHubLikely, bool locationNpcQuestLikely)
    {
        std::vector<std::string> lines;
        const std::string dayPart = player.formatWorldDayPartLine();
        const int hpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;
        const std::string cityId = player.getCurrentCityId().empty() ? "valebrume" : player.getCurrentCityId();

        lines.push_back("Repère local : ville actuelle " + cityId + " | guilde locale " + (player.isRegisteredAtCurrentCityGuild() ? "connue" : "non enregistrée") + ".");
        if (player.getWorldDaysElapsed() > 0)
        {
            lines.push_back("Temps de voyage : " + std::to_string(player.getWorldDaysElapsed()) + " jour(s) se sont déjà inscrits dans ton carnet.");
        }

        if (dayPart.find("Nuit") != std::string::npos || dayPart.find("nuit") != std::string::npos)
        {
            lines.push_back("Ambiance : les lanternes prennent plus de place que les voix, et les comptoirs parlent plus bas.");
        }
        else if (dayPart.find("Matin") != std::string::npos || dayPart.find("matin") != std::string::npos)
        {
            lines.push_back("Ambiance : les volets s'ouvrent, les commandes repartent, et la guilde trie déjà les demandes urgentes.");
        }
        else if (dayPart.find("Soir") != std::string::npos || dayPart.find("soir") != std::string::npos)
        {
            lines.push_back("Ambiance : la ville ralentit, mais les auberges, forges et rumeurs deviennent plus faciles à trouver.");
        }
        else
        {
            lines.push_back("Ambiance : la ville garde son bruit de fond, entre pas pressés, outils, marchands et affiches de guilde.");
        }

        if (hpPercent <= 35)
        {
            lines.push_back("Regard local : ton état attire plus vite l'infirmerie, les aubergistes et ceux qui savent lire une mauvaise sortie.");
        }
        if (!player.isRegisteredAtCurrentCityGuild())
        {
            lines.push_back("Comptoir local : sans inscription de guilde ici, certains panneaux restent plus froids et moins précis.");
        }
        if (questHubLikely)
        {
            lines.push_back("Rumeur utile : la guilde semble avoir quelque chose qui correspond à une quête ou une validation en cours.");
        }
        if (locationNpcQuestLikely)
        {
            lines.push_back("Rumeur utile : un lieu ou un PNJ connu semble lié à un objectif actuel.");
        }

        return lines;
    }


    std::vector<std::string> buildWorldPreparationLines(const Player& player, bool questHubLikely, bool locationNpcQuestLikely)
    {
        std::vector<std::string> lines;
        const int hpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;
        lines.push_back("Tu prends quelques minutes pour préparer la prochaine sortie sans encore quitter la ville.");
        if (hpPercent <= 35)
        {
            lines.push_back("Priorité corps : ton état rend l'auberge, l'infirmerie ou un soin plus pertinent qu'un départ immédiat.");
        }
        else if (hpPercent <= 65)
        {
            lines.push_back("Priorité prudente : ton corps peut repartir, mais une potion ou une réparation éviterait une mauvaise surprise.");
        }
        else
        {
            lines.push_back("Priorité terrain : ton état permet de penser aux outils, aux contrats et aux informations plutôt qu'à survivre à court terme.");
        }
        lines.push_back("Compétences : une maîtrise correcte doit changer le résultat, pas seulement ajouter une poussière de pourcentage. Les gros effets restent plutôt liés à l'expérience, au rang ou aux techniques obtenues tard.");
        lines.push_back("Classe : les PV, dégâts, critique, fuite et résistance ne doivent pas raconter la même histoire pour tout le monde. Un assassin doit chercher l'ouverture, un colosse doit tenir, un support doit sécuriser et un mage doit gérer sa fenêtre.");
        lines.push_back("Équipement : une arme cohérente affiche [bonus de classe] et transmet mieux dégâts/précision ; une arme vraiment contraire affiche [malus de classe] pour prévenir qu'elle peut gâcher une bonne action même avec de beaux chiffres bruts.");
        lines.push_back("Armure : une tenue cohérente avec la classe protège mieux ou gêne moins la fuite ; une armure contraire peut réduire l'absorption réelle ou casser la mobilité, même si les chiffres bruts semblent beaux.");
        for (const std::string& line : buildCurrentLoadoutSynergyLines(player))
        {
            lines.push_back(line);
        }
        lines.push_back("Ennemis : les créatures entraînées, chefs, mages, assassins, brutes ou toxiques peuvent avoir leurs propres compétences. Observer évite de les prendre comme de simples sacs à PV.");
        lines.push_back("Artisanat combat : préparer une arme, un piège de terrain ou une bombe d'atelier demande surtout les bons matériaux, pas seulement de l'or.");
        lines.push_back("Forge / services : réparer avant une sortie difficile peut compter autant qu'acheter une potion, surtout si ta classe dépend fortement de son arme ou de son armure.");
        lines.push_back("Synergie de rôle : un colosse en plaque, un rôdeur en cuir, un mage en robe ou un paladin en maille ne racontent pas le même combat ; l'équipement doit soutenir le rôle, pas juste additionner des nombres.");
        lines.push_back("Maîtrises actives : une technique bien maîtrisée profite davantage si l'arme ou l'armure soutient vraiment la classe ; à l'inverse, un [malus de classe] peut rendre le geste moins propre même avec un bon niveau de maîtrise.");
        lines.push_back("Passifs : un passif de maîtrise aide le geste choisi, mais ne remplace pas le choix du joueur. Les effets hors combat doivent rester logiques : lecture, préparation, résistance, récupération, repérage.");
        lines.push_back("Observation : une rumeur, un bestiaire ou une lecture de corps peut éviter de gaspiller une action contre une mauvaise matière ou une compétence ennemie mal lue.");
        lines.push_back("Coffres / voleur : si la zone sent le piège, un profil discret, une lecture de serrure ou une vraie prudence vaut mieux qu'un clic héroïque.");
        lines.push_back("Guilde mercenaire : les groupes utiles ne remplacent pas le joueur, mais peuvent traiter escorte, service local, route ou pression de crise.");
        lines.push_back("Quêtes / lore : quand un client décrit un danger, la phrase peut être plus utile qu'elle en a l'air : elle annonce parfois type d'ennemi, piège, terrain ou service à préparer.");
        lines.push_back("Vol / entraves / illusions : certaines créatures changent les règles du tour. Une cible en Vol refuse les armes courtes ; une entrave peut voler un tour ; une illusion peut transformer un duel en choix risqué.");
        lines.push_back("Contre-jeu : Sens du retrait peut casser une entrave si le personnage sait vraiment reculer ; les passifs de lecture réduisent le risque de frapper un faux reflet sans donner une vérité gratuite.");
        lines.push_back("Grosses compétences : certains monstres lourds peuvent annoncer une action. Le texte annonce alors un vrai boost temporaire à casser par défense, choc/givre, entrave, brise-garde, ordre allié ou pression immédiate.");
        lines.push_back("Moral ennemi : certains humains, gobelins, bêtes ou voleurs blessés peuvent paniquer, fuir ou se désorganiser. Morts-vivants, anomalies, serments et golems ne réagissent pas pareil.");
        lines.push_back("Information : observer ne donne pas une vérité divine. Une info claire doit venir d'une trace, d'un bestiaire, d'un témoin, d'une rumeur ou d'une vraie logique de terrain.");
        lines.push_back("Quêtes : l'affichage reste efficace, mais Inspecter / demander plus d'informations peut donner le contexte RP, la peur du client et les indices utiles.");
        lines.push_back("Église : les serments sont des contrats passifs soumis à conditions. Ils peuvent apporter un avantage, mais gardent un prix et une rupture à l'église.");
        lines.push_back("Serments de contre-jeu : Ciel ouvert aide contre Vol, Racines aide contre entraves, Miroir brisé aide contre illusions. Ce sont des contrats, pas des immunités gratuites.");
        if (player.hasPassiveSkill("church_oath_shield") || player.hasPassiveSkill("church_oath_blood") || player.hasPassiveSkill("church_oath_hunter") || player.hasPassiveSkill("church_oath_king")
            || player.hasPassiveSkill("church_oath_guarded_flame") || player.hasPassiveSkill("church_oath_shadow") || player.hasPassiveSkill("church_oath_pilgrim") || player.hasPassiveSkill("church_oath_memory") || player.hasPassiveSkill("church_oath_silence")
            || player.hasPassiveSkill("church_oath_open_sky") || player.hasPassiveSkill("church_oath_roots") || player.hasPassiveSkill("church_oath_broken_mirror")
            || player.hasPassiveSkill("church_oath_witness") || player.hasPassiveSkill("church_oath_scars") || player.hasPassiveSkill("church_oath_legacy")
            || player.hasPassiveSkill("church_oath_bound_forge") || player.hasPassiveSkill("church_oath_bonds")
            || player.hasPassiveSkill("church_oath_rivals") || player.hasPassiveSkill("church_oath_unstable_fate"))
        {
            lines.push_back("Serment porté : l'église a déjà une promesse inscrite à ton nom. La rupture reste un acte volontaire et laisse une trace.");
        }
        else
        {
            lines.push_back("Serment possible : l'église propose plusieurs promesses selon ton niveau, tes traces, ton rôle ou tes voyages.");
        }
        lines.push_back("Serments disponibles : Flamme gardée, Ombres franches, Pèlerin, Mémoire, Silence, Ciel ouvert, Racines, Miroir brisé, Témoin, Cicatrices, Héritage, Forge liée, Liens, Rivaux et Destin instable couvrent des voies très différentes.");
        lines.push_back("Rupture : rompre un serment à l'église coûte un rite ou de l'or, désactive le contrat et laisse une trace dans le registre.");
        lines.push_back("Conséquences longues : promesses, ruptures, témoins, rivaux et objets marqués laissent des traces lorsqu'un témoin, un survivant, un objet ou un registre peut réellement les porter.");
        lines.push_back("Forge liée / Liens : une arme ou un groupe ne gagne pas une légende parce qu'un menu l'affirme ; il faut des coups vécus, des réparations, des témoins, des recrues, des ordres ou des combats communs.");
        lines.push_back("Mémoire du monde : témoins, cicatrices, rivaux, réputation locale et objets marqués peuvent conserver les conséquences de tes actes.");
        lines.push_back("Rivaux / destin : un ennemi qui fuit, une compétence signature ou une trace d'objet ne devient importante que s'il existe une raison de la porter : témoin, mémoire, survivant, registre ou cicatrice.");
        lines.push_back("Classes évolutives : les actes réels, les maîtrises et les habitudes de combat comptent davantage qu'un simple choix de menu.");
        lines.push_back("Réputation locale : les villages réagissent à ce qui a réellement été vu, rapporté ou inscrit dans leurs registres.");
        if (questHubLikely)
        {
            lines.push_back("Signal de quête : le comptoir de guilde semble avoir une validation, une offre ou un retour à traiter.");
        }
        if (locationNpcQuestLikely)
        {
            lines.push_back("Signal de PNJ : un lieu ou une personne connue semble plus important qu'une sortie aléatoire.");
        }
        if (!player.isRegisteredAtCurrentCityGuild())
        {
            lines.push_back("Inscription locale : tant que la guilde de cette ville ne te connaît pas, certaines infos et validations restent moins nettes.");
        }
        return lines;
    }







    std::string normalizeClassAuditText(std::string value)
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

    bool classAuditContainsAny(const std::string& value, const std::vector<std::string>& words)
    {
        const std::string normalized = normalizeClassAuditText(value);
        for (const std::string& word : words)
        {
            if (normalized.find(normalizeClassAuditText(word)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
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

    // EN: valueNameContainsAny declares or implements a focused behavior used by this module.
    // FR: valueNameContainsAny déclare ou implémente un comportement précis utilisé par ce module.
    // EN: rarityEstimateMultiplier declares or implements a focused behavior used by this module.
    // FR: rarityEstimateMultiplier déclare ou implémente un comportement précis utilisé par ce module.
    int rarityEstimateMultiplier(const std::string& name)
    {
        if (classAuditContainsAny(name, {"Relique", "Unique", "Divin", "God"})) return 5;
        if (classAuditContainsAny(name, {"Héroïque", "Heroique", "Légendaire", "Legendaire"})) return 3;
        if (classAuditContainsAny(name, {"Rare", "Mystique"})) return 2;
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
        int total = static_cast<int>(std::min<long long>(2147483647LL, player.getInventory().getEconomyUnits()));

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
                "Valeur estimée de " + first.getName() + " : " + Money::formatEconomyUnits(estimatePlayerTradeValue(first)) + ".",
                "Valeur estimée de " + second.getName() + " : " + Money::formatEconomyUnits(estimatePlayerTradeValue(second)) + "."
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
                        "Valeur estimée " + Money::formatEconomyUnits(estimateWeaponTradeValue(weapon)),
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
                        "Valeur estimée " + Money::formatEconomyUnits(estimateArmorTradeValue(armor)),
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
                    "Puissance " + consumable.getPowerDisplayText() + " | Valeur " + Money::formatEconomyUnits(consumable.getValue()),
                    true,
                    "exchange.consumable.select",
                    makeExchangeItemData(
                        giver,
                        "consumable",
                        "Consommables transférables",
                        "select_consumable",
                        consumable.getName(),
                        "Puissance " + consumable.getPowerDisplayText(),
                        "Transférable",
                        "Valeur " + Money::formatEconomyUnits(consumable.getValue()),
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
                    material.getCategory() + " | Qualité " + material.getQualityLabel() + " | Valeur " + Money::formatEconomyUnits(material.getValue()),
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
                        "Valeur unitaire " + Money::formatEconomyUnits(material.getValue()),
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
    selectedAge = 18;
    selectedVisualPresentation = "Non précisé";
    selectedVisualVariant = "Variante A";
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
            choosePlayerAppearance();
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

    bool sessionOpen = true;
    while (sessionOpen)
    {
        try
        {
            chooseGameMode();
            displaySelectedMode();
            launchSelectedMode();
            sessionOpen = false;
        }
        catch (const ReturnToActivityMenuRequest&)
        {
            Console::clear();
            MessageScreen::show(
                "BIENVENUE DANS DINOTOFU",
                "game.return.activity_menu",
                {
                    "Progression sauvegardée.",
                    "Retour au Menu de voyage.",
                    "Le personnage reste chargé : cet écran ne recrée pas le personnage."
                },
                false
            );
        }
    }
}

// EN: displayIntroduction declares or implements a focused behavior used by this module.
// FR: displayIntroduction déclare ou implémente un comportement précis utilisé par ce module.


// EN: askAccountName declares or implements a focused behavior used by this module.
// FR: askAccountName déclare ou implémente un comportement précis utilisé par ce module.

// EN: askPlayerName declares or implements a focused behavior used by this module.
// FR: askPlayerName déclare ou implémente un comportement précis utilisé par ce module.

// EN: chooseDifficulty declares or implements a focused behavior used by this module.
// FR: chooseDifficulty déclare ou implémente un comportement précis utilisé par ce module.



// EN: choosePlayerRace declares or implements a focused behavior used by this module.
// FR: choosePlayerRace déclare ou implémente un comportement précis utilisé par ce module.


// EN: choosePlayerClass declares or implements a focused behavior used by this module.
// FR: choosePlayerClass déclare ou implémente un comportement précis utilisé par ce module.



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

        MenuScreen screen("MENU DE VOYAGE", "activity.main");
        screen.addSubtitle("Activités principales : histoire, combats, exploration, personnage et lieux visitables.");
        screen.addLine("Date : " + mainPlayer.formatWorldDateLine() + " | Moment : " + mainPlayer.formatWorldDayPartLine());
        screen.addLine("Exploration = sorties par biome. Monde / ville = lieux visitables, boutiques, forge, guilde, PNJ et services.");
        screen.addLine("Menu rapide = personnage, saisie libre, options de partie et sauvegarde. L’artisanat est aussi visible dans Personnage.");
        screen.addOption(
            1,
            "Histoire",
            "Route principale : prologue, progression du village, quêtes principales et chapitres.",
            true,
            "activity.story",
            makeActivityItemData("Menu de voyage", "story", "Histoire", "Bac à sable guidé par chapitres, avec contenus visibles selon l'état réel du monde.", mainPlayer.getStoryProgressLabel(), "Progression narrative", true)
        );
        screen.addOption(
            2,
            "Combats" + questActivityTag(combatQuestLikely),
            "PvP IA, JcJ local, monstres et boss." + questActivityTag(combatQuestLikely),
            true,
            "activity.combat",
            makeActivityItemData("Menu de voyage", "combat", "Combats", "Affrontements volontaires contre IA, joueurs, monstres ou boss.", combatQuestLikely ? "Quête probable" : "Disponible", "Combat volontaire", combatQuestLikely)
        );
        screen.addOption(
            3,
            "Exploration" + questActivityTag(explorationQuestLikely),
            "Biomes, plantes, matériaux, coffres, pièges, mimics et rencontres imprévues." + questActivityTag(explorationQuestLikely),
            true,
            "activity.exploration",
            makeActivityItemData("Menu de voyage", "travel", "Exploration", "Sorties par biome avec risques, ressources, traces et événements.", explorationQuestLikely ? "Quête probable" : "Disponible", "Sortie d'exploration", explorationQuestLikely)
        );
        screen.addOption(
            4,
            "Personnage",
            "Inventaire, craft, compétences, actifs/passifs, titres, quêtes acceptées, stats, équipe et échange.",
            true,
            "activity.character",
            makeActivityItemData("Menu de voyage", "inspect", "Personnage", "Inventaire, artisanat et progression personnelle.", "Disponible", "Personnage")
        );
        screen.addOption(
            5,
            "Monde / ville" + questActivityTag(questHubLikely || locationNpcQuestLikely),
            "Quêtes, guilde, PNJ, lieux, boutiques, forge, services et entraînement." + questActivityTag(questHubLikely || locationNpcQuestLikely),
            true,
            "activity.world",
            makeActivityItemData("Menu de voyage", "travel", "Monde / ville", "Services, contacts, comptoirs et lieux précis.", (questHubLikely || locationNpcQuestLikely) ? "Quête probable" : "Disponible", "Monde / services", questHubLikely || locationNpcQuestLikely)
        );
        screen.addOption(
            6,
            "Infos utiles / aide",
            "Journées, argent, quêtes, exploration, PNJ et lieux notables.",
            true,
            "activity.info",
            makeActivityItemData("Menu de voyage", "inspect", "Infos utiles", "Guide court des routes jouables, du temps, de l'économie et des quêtes.", "Aide", "Lecture")
        );
        screen.addOption(
            7,
            "Compagnon Dinotofu",
            "Petit guide du logo : conseils courts selon ton état, tes quêtes et ta session.",
            true,
            "activity.dinotofu_companion",
            makeActivityItemData("Menu de voyage", "inspect", "Compagnon Dinotofu", "Assistant non obligatoire, pensé comme un petit repère façon Clipper, mais moins envahissant.", "Conseil", "Guide")
        );
        addOutOfCombatUtilityOptions(screen, true, true);

        const int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir une activité affichée."
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
                    makeActivityItemData("Combats", "combat", "PvP 2 joueurs / JcJ", "Duel local entre personnages compatibles.", "Disponible", "Combat volontaire")
                );
                combatScreen.addOption(
                    3,
                    "PvE monstres" + questActivityTag(currentMonsterQuestLikely),
                    "Affrontement contre monstres, groupes, vagues et rencontres spéciales." + questActivityTag(currentMonsterQuestLikely),
                    true,
                    "combat.monster_pve",
                    makeActivityItemData("Combats", "combat", "PvE monstres", "Monstres, groupes, vagues et rencontres spéciales.", currentMonsterQuestLikely ? "Quête probable" : "Disponible", "Combat volontaire", currentMonsterQuestLikely)
                );
                combatScreen.addOption(
                    4,
                    "PvE boss" + questActivityTag(currentBossQuestLikely),
                    "Boss, sous-boss et combats particuliers. La fuite y est impossible." + questActivityTag(currentBossQuestLikely),
                    true,
                    "combat.boss_pve",
                    makeActivityItemData("Combats", "combat", "PvE boss", "Boss, sous-boss et combats particuliers sans fuite.", currentBossQuestLikely ? "Quête probable" : "Disponible", "Expédition de boss", currentBossQuestLikely)
                );

                const int combatChoice = TerminalInterface::askMenuChoiceFromOptions(
                    combatScreen,
                    "Veuillez choisir un type de combat affiché."
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
            continue;
        }

        if (choice == 3)
        {
            selectedMode = GameMode::Exploration;
            return;
        }
        if (choice == 4)
        {
            openQuickCharacterMenu(true);
            continue;
        }
        if (choice == 5)
        {
            openQuickWorldMenu();
            continue;
        }
        if (choice == 6)
        {
            displayActivityInformation();
            continue;
        }

        if (choice == 7)
        {
            displayDinotofuCompanion();
            saveCurrentProgress("Consultation du compagnon Dinotofu");
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
            return "Lieux notables";
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
    snapshot.totalCopper = mainPlayer.getInventory().getTotalCopper();
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
                + " | Argent " + Money::formatCopper(before.totalCopper),
            "Après : niveau " + std::to_string(after.level)
                + " | XP " + std::to_string(after.experience)
                + " | PV " + std::to_string(after.hp) + "/" + std::to_string(after.maxHp)
                + " | Argent " + Money::formatCopper(after.totalCopper),
            "",
            "Variations :",
            "- Niveau : " + std::to_string(after.level - before.level),
            "- Expérience : " + std::to_string(after.experience - before.experience),
            "- PV actuels : " + std::to_string(after.hp - before.hp),
            "- PV max : " + std::to_string(after.maxHp - before.maxHp),
            "- Argent : " + Money::formatCopper(after.totalCopper >= before.totalCopper ? after.totalCopper - before.totalCopper : before.totalCopper - after.totalCopper) + (after.totalCopper < before.totalCopper ? " perdus" : " gagnés"),
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
        screen.addLine("Argent séparé : " + mainPlayer.getInventory().getWalletLine());
        screen.addLine("Argent total : " + mainPlayer.getInventory().getWalletTotalLine());
        screen.addBackOption("Retour", "activity.info.back");
        screen.addOption(1, "Où aller ?", "Rappel des grandes catégories du menu principal.", true, "activity.info.where");
        screen.addOption(2, "Journées / temps", "Comprendre les jours, moments et conséquences des activités.", true, "activity.info.time");
        screen.addOption(3, "Argent / économie", "Or, boutiques, stocks, prix, revente et récompenses.", true, "activity.info.money");
        screen.addOption(4, "Combat / exploration", "Différence entre combat volontaire, boss et sortie par biome.", true, "activity.info.terrain");
        screen.addOption(5, "Quêtes / rendre objectifs", "Où voir les quêtes prêtes, principales, secondaires et rendues.", true, "activity.info.quests");
        screen.addOption(6, "PNJ / lieux notables", "Contacts, endroits précis, boutiques et services du monde.", true, "activity.info.locations");

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
                    "Combats : affrontements volontaires contre IA, joueurs, monstres ou boss.",
                    "Exploration : sorties par biome, ressources, traces, coffres et rencontres.",
                    "Personnage : inventaire, titres, compétences actifs/passifs, quêtes acceptées, statistiques et équipe.",
                    "Monde / ville : activité d'exploration sociale pour visiter la guilde, les lieux, PNJ, boutiques, forge, services et entraînement.",
                    "Menu rapide : accès constant au personnage, à la saisie libre, aux options de partie et à la sauvegarde.",
                    "Important : Monde / ville n'est pas dans le menu rapide, car ce sont des lieux à visiter dans le monde."
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
                    "Argent séparé : " + mainPlayer.getInventory().getWalletLine(),
                    "Argent total : " + mainPlayer.getInventory().getWalletTotalLine(),
                    "L'argent vient surtout des combats, quêtes, explorations, reventes et événements.",
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
                    "Une quête de terrain, de traces, de plantes ou de route passe généralement par Exploration.",
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
                "PNJ / LIEUX NOTABLES",
                "activity.info.locations.detail",
                {
                    "Monde / ville est une activité de visite/exploration sociale, pas une option du menu rapide.",
                    "Boutiques et comptoirs gardent achat, vente, discussion et quêtes du vendeur dans la même visite.",
                    "Lieux, PNJ et services sert pour la guilde, la forge, l'infirmerie, l'auberge, les archives et les contacts de ville.",
                    "Les sorties par biome restent dans Exploration, car ce n'est pas la même action que visiter un lieu précis."
                }
            );
            continue;
        }
    }
}

void Game::displayDinotofuCompanion()
{
    const QuestLog& questLog = mainPlayer.getQuestLog();
    int activeQuestCount = 0;
    int readyQuestCount = 0;
    int guildQuestCount = 0;

    for (const Quest& quest : questLog.getQuests())
    {
        if (quest.turnedIn || quest.failed)
        {
            continue;
        }

        ++activeQuestCount;
        if (quest.guildQuest)
        {
            ++guildQuestCount;
        }
        if (quest.completed || (quest.target > 0 && quest.progress >= quest.target))
        {
            ++readyQuestCount;
        }
    }

    const int lanternCount = mainPlayer.getInventory().countMaterialById("fire_lantern")
        + mainPlayer.getInventory().countMaterialById("mycelium_lantern");

    std::vector<std::string> lines;
    lines.push_back("Le petit Dinotofu du logo trottine près de ton sac et pointe une direction avec sa patte.");
    lines.push_back("Rôle actuel : guide léger. Il conseille, mais il ne joue jamais à ta place.");
    lines.push_back("Personnage : " + mainPlayer.getName() + " | Niveau " + std::to_string(mainPlayer.getLevel()) + " | PV " + std::to_string(mainPlayer.getHp()) + "/" + std::to_string(mainPlayer.getMaxHp()) + ".");
    lines.push_back("Quêtes actives : " + std::to_string(activeQuestCount) + " dont " + std::to_string(guildQuestCount) + " de guilde. Prêtes à rendre : " + std::to_string(readyQuestCount) + ".");
    lines.push_back("Journal beta : " + RuntimeLog::currentLogPath() + ".");

    if (mainPlayer.getHp() * 3 <= std::max(1, mainPlayer.getMaxHp()))
    {
        lines.push_back("Conseil soin : tes PV sont bas. Passe par l'infirmerie, une auberge, une potion ou une activité moins risquée avant de forcer un boss.");
    }
    else if (mainPlayer.getHp() * 2 <= std::max(1, mainPlayer.getMaxHp()))
    {
        lines.push_back("Conseil prudence : tu peux encore agir, mais évite d'empiler exploration dangereuse + boss sans pause.");
    }
    else
    {
        lines.push_back("Conseil rythme : ton état est correct. Tu peux choisir entre combat, exploration ou validation de quêtes selon ton objectif.");
    }

    if (readyQuestCount > 0)
    {
        lines.push_back("Conseil quête : tu as au moins un objectif prêt. Va dans Quêtes pour rendre avant d'oublier la récompense.");
    }
    else if (activeQuestCount == 0)
    {
        lines.push_back("Conseil départ : aucune quête active. Va voir la guilde, les PNJ notables ou le comptoir mercenaire pour cadrer une sortie.");
    }
    else
    {
        lines.push_back("Conseil objectif : regarde le journal complet si tu ne sais plus si la suite demande combat, exploration, livraison ou dialogue.");
    }

    if (lanternCount <= 0)
    {
        lines.push_back("Conseil combat : aucune lanterne dans le sac. Les Actions tactiques restent utiles, mais les options lanterne seront verrouillées.");
    }
    else
    {
        lines.push_back("Conseil combat : tu as " + std::to_string(lanternCount) + " lanterne(s). En PvE, Actions tactiques peut les lancer sur une cible ou au sol.");
    }

    if (mainPlayer.isClassSkillReady())
    {
        lines.push_back("Conseil compétence : ta compétence active est disponible. Pense à l'utiliser pour éviter le spam attaque normale.");
    }
    else
    {
        lines.push_back("Conseil compétence : récupération active encore " + std::to_string(mainPlayer.getClassSkillCooldownTurns()) + " tour(s). Les actions tactiques peuvent combler ce temps.");
    }

    if (!mainPlayer.getUnlockedActiveSkills().empty() || !mainPlayer.getUnlockedPassiveSkills().empty())
    {
        lines.push_back("Conseil progression : ouvre Menu rapide > Personnage > Compétences - actifs / passifs pour gérer ton loadout.");
    }
    else
    {
        lines.push_back("Conseil progression : tes compétences vont surtout venir du niveau, des armes jouées, du stand et de certaines expériences de terrain.");
    }

    lines.push_back("Conseil retour beta : si un testeur veut expliquer un bug, demande-lui aussi le fichier de journal local indiqué plus haut.");

    RuntimeLog::recordScreen("COMPAGNON DINOTOFU", "utility.dinotofu_companion", lines);
    mainPlayer.recordCanonicalEvent("compagnon_dinotofu", "consultation", "Consultation du compagnon Dinotofu", 1);
    mainPlayer.grantTitle("Ami du petit Dinotofu");

    MessageScreen::show(
        "COMPAGNON DINOTOFU",
        "utility.dinotofu_companion",
        lines,
        false
    );
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
        saveCurrentProgress("Lieux notables");
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
        QuestDeadlineSupport::synchronizeQuestConsequences(mainPlayer);
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
void Game::launchChallengeBoard()
{
    QuestMenu::openQuestHub(mainPlayer);
}

void Game::addOutOfCombatUtilityOptions(MenuScreen& screen, bool inventoryAvailable, bool saveAvailable) const
{
    (void)saveAvailable;
    std::string description = "Personnage, saisie libre, options de partie et sauvegarde.";
    if (!inventoryAvailable)
    {
        description = "Personnage, saisie libre, options de partie et sauvegarde. Inventaire indisponible ici.";
    }

    screen.addOption(
        UtilityChoiceOutOfCombatMenu,
        "Menu rapide",
        description,
        true,
        "utility.quick_menu",
        makeUtilityItemData(mainPlayer, "menu", "Menu rapide", description)
    );
}

void Game::openQuickCharacterMenu(bool inventoryAvailable)
{
    while (true)
    {
        MenuScreen screen("PERSONNAGE", "utility.quick.character");
        screen.addSubtitle("Tout ce qui appartient directement au personnage");
        screen.addLine(mainPlayer.getName() + " | Niveau " + std::to_string(mainPlayer.getLevel()) + " | " + mainPlayer.getRaceText() + " / " + mainPlayer.getType());
        screen.addLine("PV : " + std::to_string(mainPlayer.getHp()) + "/" + std::to_string(mainPlayer.getMaxHp()));
        screen.addLine("Actifs équipés : " + std::to_string(mainPlayer.getEquippedActiveSkills().size()) + "/" + std::to_string(Player::MAX_EQUIPPED_ACTIVE_SKILLS)
            + " | Passifs activés : " + std::to_string(mainPlayer.getEnabledPassiveSkills().size()) + "/" + std::to_string(Player::MAX_ENABLED_PASSIVE_SKILLS));
        for (const std::string& line : buildCurrentLoadoutSynergyLines(mainPlayer))
        {
            screen.addLine(line);
        }
        screen.addBackOption();
        screen.addOption(
            1,
            "Inventaire",
            "Gérer objets, équipement et potions hors combat. Le craft reste aussi accessible directement ci-dessous.",
            inventoryAvailable,
            "utility.character.inventory",
            makeUtilityItemData(mainPlayer, "open", "Inventaire", "Gestion hors combat.", inventoryAvailable ? "Disponible" : "Indisponible")
        );
        screen.addOption(
            2,
            "Compétences - actifs / passifs",
            "Équiper ou déséquiper les actifs, activer ou désactiver les passifs.",
            true,
            "utility.character.skills",
            makeUtilityItemData(mainPlayer, "equip", "Compétences - actifs / passifs", "Limites actuelles : 10 actifs équipés et 10 passifs activés.")
        );
        screen.addOption(
            3,
            "Titres",
            "Voir, comprendre et équiper les titres du personnage.",
            true,
            "utility.character.titles",
            makeUtilityItemData(mainPlayer, "inspect", "Titres", "Identité, réputation et titres équipés.")
        );
        screen.addOption(
            4,
            "Quêtes acceptées / journal",
            "Afficher les quêtes actives, prêtes, principales, de guilde et terminées.",
            true,
            "utility.character.quests",
            makeUtilityItemData(mainPlayer, "quest", "Quêtes acceptées", "Journal du personnage.")
        );
        screen.addOption(
            5,
            "Statistiques / progression",
            "Résumé, Top 3, compétences connues, équipement et états spéciaux.",
            true,
            "utility.character.statistics",
            makeUtilityItemData(mainPlayer, "inspect", "Statistiques / progression", "Résumé complet du personnage.")
        );
        screen.addOption(
            6,
            "Équipement rapide",
            "Afficher l'équipement actuel sans ouvrir tout l'inventaire.",
            true,
            "utility.character.quick_equipment",
            makeUtilityItemData(mainPlayer, "inspect", "Équipement rapide", "Vue courte des armes et protections.")
        );
        screen.addOption(
            7,
            "Attributs",
            "Section encore scellée, conservée dans le menu personnage.",
            true,
            "utility.character.attributes",
            makeUtilityItemData(mainPlayer, "open", "Attributs", "Cette voie reste scellée pour l'instant.", "Scellé")
        );
        screen.addOption(
            8,
            "Équipe",
            "Inspecter les recrues, parts, ordre de groupe, bilan hebdomadaire et note multi en ligne.",
            true,
            "utility.character.team",
            makeUtilityItemData(mainPlayer, "team", "Équipe", "Gestion de groupe / clan.")
        );
        screen.addOption(
            9,
            "Échange / don",
            "Transférer des ressources entre personnages compatibles.",
            true,
            "utility.character.exchange",
            makeUtilityItemData(mainPlayer, "barter", "Échange / don", "Transfert protégé entre personnages.")
        );
        screen.addOption(
            10,
            "Artisanat / craft",
            "Ouvrir directement les schémas de fabrication connus sans devoir chercher l'option au fond de l'inventaire.",
            inventoryAvailable,
            "utility.character.craft",
            makeUtilityItemData(mainPlayer, "create", "Artisanat / craft", "Fabrication à partir des recettes dont les composants sont connus.", inventoryAvailable ? "Disponible" : "Indisponible")
        );

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option personnage.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            if (!inventoryAvailable)
            {
                MessageScreen::show("INVENTAIRE", "utility.character.inventory.unavailable", {"L'inventaire n'est pas disponible sur cet écran."});
                continue;
            }
            InventoryMenu::open(mainPlayer);
            saveCurrentProgress("Inventaire depuis Personnage");
            Console::clear();
            continue;
        }
        if (choice == 2)
        {
            StatisticsMenu::openSkillLoadoutMenu(mainPlayer);
            saveCurrentProgress("Gestion actifs et passifs");
            Console::clear();
            continue;
        }
        if (choice == 3)
        {
            StatisticsMenu::displayTitleCatalog(mainPlayer);
            saveCurrentProgress("Consultation des titres");
            continue;
        }
        if (choice == 4)
        {
            QuestMenu::consultOnly(mainPlayer);
            continue;
        }
        if (choice == 5)
        {
            StatisticsMenu::open(mainPlayer, selectedDifficulty, true);
            saveCurrentProgress("Consultation des statistiques et compétences");
            continue;
        }
        if (choice == 6)
        {
            mainPlayer.displaySimpleEquipment();
            Console::waitForEnter();
            Console::clear();
            continue;
        }
        if (choice == 7)
        {
            AttributeMenu::displayLockedDevelopmentMessage();
            Console::waitForEnter();
            Console::clear();
            continue;
        }
        if (choice == 8)
        {
            QuestMenu::openTeamMenu(mainPlayer);
            saveCurrentProgress("Menu Équipe depuis Personnage");
            Console::clear();
            continue;
        }
        if (choice == 9)
        {
            openExchangeMenu();
            saveCurrentProgress("Échange entre personnages");
            continue;
        }
        if (choice == 10)
        {
            if (!inventoryAvailable)
            {
                MessageScreen::show("ARTISANAT", "utility.character.craft.unavailable", {"L'inventaire n'est pas disponible sur cet écran, donc les composants ne peuvent pas être utilisés."});
                continue;
            }
            InventorySelection::openCraft(mainPlayer);
            saveCurrentProgress("Artisanat depuis Personnage");
            Console::clear();
            continue;
        }
    }
}

void Game::openQuickWorldMenu()
{
    while (true)
    {
        const bool questHubLikely = hasLikelyQuestHubObjective(mainPlayer);
        const bool locationNpcQuestLikely = hasLikelyLocationOrNpcQuest(mainPlayer);
        MenuScreen screen("MONDE / VILLE", "activity.world.menu");
        screen.addSubtitle("Activité de visite : guilde, lieux, PNJ, boutiques et services regroupés");
        screen.addLine("Date : " + mainPlayer.formatWorldDateLine() + " | Moment : " + mainPlayer.formatWorldDayPartLine());
        screen.addLine("Cette section représente des lieux visitables, pas un raccourci de poche du menu rapide.");
        screen.addLine("Les boutiques gardent leurs achats, ventes, discussions et quêtes du vendeur dans le même comptoir.");
        for (const std::string& line : buildWorldVisitAmbienceLines(mainPlayer, questHubLikely, locationNpcQuestLikely))
        {
            screen.addLine(line);
        }
        screen.addBackOption();
        screen.addOption(
            1,
            "Quêtes / guilde" + questActivityTag(questHubLikely),
            "Quête principale, journal, panneau de guilde, demandes et objectifs à rendre." + questActivityTag(questHubLikely),
            true,
            "utility.world.quests",
            makeActivityItemData("Monde / ville", "quest", "Quêtes / guilde", "Journal, panneau, demandes et validations.", questHubLikely ? "Quête probable" : "Disponible", "Progression", questHubLikely)
        );
        screen.addOption(
            2,
            "Lieux, PNJ et services" + questActivityTag(locationNpcQuestLikely),
            "Ville, extérieur, contacts, forge, guilde, auberge, infirmerie et services précis." + questActivityTag(locationNpcQuestLikely),
            true,
            "utility.world.locations",
            makeActivityItemData("Monde / ville", "travel", "Lieux, PNJ et services", "Endroits précis et contacts associés.", locationNpcQuestLikely ? "Quête probable" : "Disponible", "Ville / services", locationNpcQuestLikely)
        );
        screen.addOption(
            3,
            "Boutiques et comptoirs",
            "Acheter, vendre, discuter, voir les quêtes du vendeur ou utiliser un service spécial.",
            true,
            "utility.world.shops",
            makeActivityItemData("Monde / ville", "shop", "Boutiques et comptoirs", "Achat, vente, discussion, quêtes et services spéciaux du même vendeur.", "Disponible", "Économie")
        );
        screen.addOption(
            4,
            "PNJ notables / personnages spéciaux" + questActivityTag(locationNpcQuestLikely),
            "Parler aux personnages importants, contacts connus, habitants et figures spéciales." + questActivityTag(locationNpcQuestLikely),
            true,
            "utility.world.notable_npcs",
            makeActivityItemData("Monde / ville", "talk", "PNJ notables / personnages spéciaux", "Contacts du monde classés par rôle.", locationNpcQuestLikely ? "Quête probable" : "Disponible", "Dialogues", locationNpcQuestLikely)
        );
        screen.addOption(
            5,
            "Stand d'entraînement",
            "Apprendre une technique, observer, travailler appuis ou résistance environnementale.",
            true,
            "utility.world.training",
            makeActivityItemData("Monde / ville", "train", "Stand d'entraînement", "Entraînement court hors combat.", "Disponible", "Progression")
        );
        screen.addOption(
            6,
            "Rumeurs et priorités locales",
            "Relire les signaux de ville : moment, état du corps, quêtes probables et lieu à visiter ensuite.",
            true,
            "utility.world.local_priorities",
            makeActivityItemData("Monde / ville", "inspect", "Rumeurs et priorités locales", "Lecture courte des signaux locaux sans quitter la ville.", "Lecture", "Ville / aide", questHubLikely || locationNpcQuestLikely)
        );
        screen.addOption(
            7,
            "Préparer la prochaine sortie",
            "Relire les priorités avant départ : soin, outils, observation, coffres, artisanat et renforts mercenaires.",
            true,
            "utility.world.prepare_next_run",
            makeActivityItemData("Monde / ville", "inspect", "Préparer la prochaine sortie", "Rappel court des préparatifs utiles avant de repartir.", "Préparation", "Ville / aide", true)
        );

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option monde / ville.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            QuestMenu::openQuestHub(mainPlayer);
            saveCurrentProgress("Quêtes depuis Monde / ville");
            continue;
        }
        if (choice == 2)
        {
            QuestMenu::openLocations(mainPlayer);
            saveCurrentProgress("Lieux et services depuis Monde / ville");
            continue;
        }
        if (choice == 3)
        {
            ShopMenu::open(mainPlayer);
            saveCurrentProgress("Boutiques depuis Monde / ville");
            continue;
        }
        if (choice == 4)
        {
            QuestMenu::openNotableNpcMenu(mainPlayer);
            saveCurrentProgress("PNJ notables depuis Monde / ville");
            continue;
        }
        if (choice == 5)
        {
            if (TrainingGroundMenu::open(mainPlayer))
            {
                saveCurrentProgress("Stand d'entraînement depuis Monde / ville");
            }
            continue;
        }
        if (choice == 6)
        {
            std::vector<std::string> lines;
            lines.push_back("Tu prends quelques secondes pour relire la ville au lieu de courir vers le prochain comptoir.");
            const std::vector<std::string> ambience = buildWorldVisitAmbienceLines(mainPlayer, questHubLikely, locationNpcQuestLikely);
            lines.insert(lines.end(), ambience.begin(), ambience.end());
            if (questHubLikely)
            {
                lines.push_back("Priorité probable : la guilde ou le journal ont quelque chose à régler avant de repartir.");
            }
            if (locationNpcQuestLikely)
            {
                lines.push_back("Priorité probable : un lieu précis ou un PNJ semble lié à une quête active.");
            }
            if (!questHubLikely && !locationNpcQuestLikely)
            {
                lines.push_back("Aucune priorité urgente ne ressort. Boutique, forge, entraînement ou préparation restent de bons choix.");
                lines.push_back("Lecture de ville : sans urgence, c'est le bon moment pour vérifier équipement, réparation, coffres suspects, mercenaires ou informations de bestiaire.");
                lines.push_back("Lecture de build : si plusieurs actions importantes affichent un malus de classe, mieux vaut passer par boutique/forge avant une sortie difficile.");
            }
            if (mainPlayer.getMaxHp() > 0 && mainPlayer.getHp() * 100 <= mainPlayer.getMaxHp() * 35)
            {
                lines.push_back("Ton état attire les regards : l'auberge, l'infirmerie ou une vraie pause seraient plus sages qu'une nouvelle sortie.");
            }
            MessageScreen::show("RUMEURS ET PRIORITÉS", "activity.world.local_priorities", lines, false);
            continue;
        }
        if (choice == 7)
        {
            std::vector<std::string> lines = buildWorldPreparationLines(mainPlayer, questHubLikely, locationNpcQuestLikely);
            lines.push_back("Rappel : cette préparation ne consomme pas de sortie. Elle sert à mieux choisir entre guilde, boutique, forge, entraînement ou exploration.");
            MessageScreen::show("PRÉPARER LA SORTIE", "activity.world.prepare_next_run", lines, false);
            continue;
        }
    }
}

void Game::openQuickSessionOptionsMenu()
{
    while (true)
    {
        MenuScreen screen("OPTIONS DE PARTIE", "utility.quick.session_options");
        screen.addSubtitle("Réglages, gardien, aide et données spéciales");
        screen.addLine("Fréquence des indications : " + mainPlayer.getInterfaceHintFrequencyLabel());
        screen.addBackOption();
        screen.addOption(1, "Paramètres", "Changer la fréquence des indications et les réglages disponibles.", true, "utility.session.settings", makeUtilityItemData(mainPlayer, "settings", "Paramètres", "Réglages modifiables à tout moment."));
        screen.addOption(2, "Parler au gardien / saisie libre", "Écrire une phrase, un choix ou une commande.", true, "utility.session.guardian", makeUtilityItemData(mainPlayer, "guardian", "Gardien du monde", "Saisie libre hors combat."));
        screen.addOption(3, "Compagnon Dinotofu", "Conseils courts selon la situation actuelle.", true, "utility.session.companion", makeUtilityItemData(mainPlayer, "inspect", "Compagnon Dinotofu", "Assistant du logo, utile pour savoir quoi faire ensuite.", "Guide"));
        screen.addOption(4, "Journal bêta", "Voir où trouver le journal de session à envoyer au dev.", true, "utility.session.beta_log", makeUtilityItemData(mainPlayer, "inspect", "Journal bêta", "Chemin du fichier de logs local à envoyer en cas de bug.", "Logs"));

        if (mainPlayer.isAlteredByCheats())
        {
            screen.addOption(5, "Données altérées", "Voir les altérations connues de ce personnage.", true, "utility.session.altered_data", makeUtilityItemData(mainPlayer, "inspect", "Données altérées", "Informations déjà révélées pour ce personnage.", "Altéré"));
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option de partie.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            openInterfaceSettingsMenu();
            continue;
        }
        if (choice == 2)
        {
            openGuardianInputMenu();
            continue;
        }
        if (choice == 3)
        {
            displayDinotofuCompanion();
            saveCurrentProgress("Consultation du compagnon Dinotofu");
            continue;
        }
        if (choice == 4)
        {
            MessageScreen::show(
                "JOURNAL BÊTA",
                "utility.session.beta_log.detail",
                {
                    "Fichier local à envoyer au dev si un combat, une exploration ou un menu bug :",
                    RuntimeLog::currentLogPath(),
                    "Le fichier est recréé pendant la session et peut être supprimé sans danger."
                },
                false
            );
            continue;
        }
        if (choice == 5 && mainPlayer.isAlteredByCheats())
        {
            CheatManager::openAlteredDataMenu(mainPlayer, selectedDifficulty, selectedDeathRule);
            saveCurrentProgress("Données altérées");
            continue;
        }
    }
}

void Game::openQuickSaveOptionsMenu()
{
    while (true)
    {
        MenuScreen screen("OPTIONS DE SAUVEGARDE", "utility.quick.save_options");
        screen.addSubtitle("Sauvegarde et sortie");
        screen.addLine("Personnage : " + mainPlayer.getName());
        screen.addLine("Version de création : " + mainPlayer.getCreatedForVersion());
        screen.addLine("Dernière adaptation : " + mainPlayer.getLastAdaptedVersion());
        screen.addBackOption();
        screen.addOption(1, "Sauvegarder", "Sauvegarder sans quitter la partie.", true, "utility.save.quick", makeUtilityItemData(mainPlayer, "save", "Sauvegarder", "Sauvegarde rapide.", "Disponible"));
        screen.addOption(2, "Sauvegarder et retourner au Menu de voyage", "Sauvegarder puis revenir au Menu de voyage.", true, "utility.save.return_menu", makeUtilityItemData(mainPlayer, "save", "Sauvegarder et retourner au Menu de voyage", "Retour au Menu de voyage sans recréation.", "Disponible"));
        screen.addOption(3, "Sauvegarder et quitter", "Sauvegarder puis fermer Dinotofu.", true, "utility.save.quit", makeUtilityItemData(mainPlayer, "save", "Sauvegarder et quitter", "Fermeture propre après sauvegarde.", "Disponible"));

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option de sauvegarde.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            saveCurrentProgress("Sauvegarde rapide hors combat");
            Console::waitForEnter();
            Console::clear();
            continue;
        }
        if (choice == 2)
        {
            savePartyProgress("Sauvegarder et retourner au Menu de voyage");
            throw ReturnToActivityMenuRequest();
        }
        if (choice == 3)
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
    }
}

void Game::openOutOfCombatUtilityMenu(bool inventoryAvailable)
{
    while (true)
    {
        MenuScreen screen("MENU RAPIDE", "utility.quick.menu");
        screen.addSubtitle("Hub constant, rangé par rôle");
        screen.addLine("Date : " + mainPlayer.formatWorldDateLine() + " | Moment : " + mainPlayer.formatWorldDayPartLine());
        screen.addLine("Fréquence des indications : " + mainPlayer.getInterfaceHintFrequencyLabel());
        screen.addBackOption();
        screen.addOption(1, "Personnage", "Inventaire, compétences, actifs/passifs, titres, quêtes, stats, équipe et échange.", true, "utility.quick.character", makeUtilityItemData(mainPlayer, "menu", "Personnage", "Ce qui appartient directement au personnage."));
        screen.addOption(2, "Parler au gardien / saisie libre", "Écrire une phrase, un choix ou une commande.", true, "utility.quick.guardian", makeUtilityItemData(mainPlayer, "guardian", "Gardien du monde", "Saisie libre hors combat."));
        screen.addOption(3, "Compagnon Dinotofu", "Conseils courts selon la situation actuelle.", true, "utility.quick.companion", makeUtilityItemData(mainPlayer, "inspect", "Compagnon Dinotofu", "Assistant du logo.", "Guide"));
        screen.addOption(4, "Options de partie", "Paramètres, journal bêta, données altérées et options de confort.", true, "utility.quick.session_options", makeUtilityItemData(mainPlayer, "settings", "Options de partie", "Réglages et informations de session."));
        screen.addOption(5, "Options de sauvegarde", "Sauvegarder, retourner au menu ou quitter proprement.", true, "utility.quick.save_options", makeUtilityItemData(mainPlayer, "save", "Options de sauvegarde", "Sauvegarde et sortie.", "Fin de menu"));

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option du menu hors combat.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }
        if (choice == 1)
        {
            openQuickCharacterMenu(inventoryAvailable);
            continue;
        }
        if (choice == 2)
        {
            openGuardianInputMenu();
            continue;
        }
        if (choice == 3)
        {
            displayDinotofuCompanion();
            saveCurrentProgress("Consultation du compagnon Dinotofu");
            continue;
        }
        if (choice == 4)
        {
            openQuickSessionOptionsMenu();
            continue;
        }
        if (choice == 5)
        {
            openQuickSaveOptionsMenu();
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
        mainPlayer.forceTerminalImagePolicy();
        screen.addLine("Fréquence actuelle des indications : " + mainPlayer.getInterfaceHintFrequencyLabel());
        screen.addLine("Images : désactivées en terminal, non activables depuis le terminal.");
        screen.addLine("Règle images : elles seront toujours un supplément visuel. Aucune info ne doit disparaître si elles sont activées en IG.");
        screen.addLine("null : aucune indication volontaire hors inspection ou avertissement vital.");
        screen.addLine("faible : valeur par défaut, seulement les alertes importantes ou contextes très liés.");
        screen.addLine("normal : un peu plus d'indices sur quêtes, routes et équipement.");
        screen.addLine("forte : plus bavard, utile si tu veux beaucoup de guidage.");
        screen.addBackOption();
        screen.addOption(1, "Fréquence : null", "Désactive les indications volontaires autant que possible.", true, "settings.hints.null");
        screen.addOption(2, "Fréquence : faible", "Réglage par défaut : rare, surtout utile et non intrusif.", true, "settings.hints.low");
        screen.addOption(3, "Fréquence : normal", "Affiche davantage d'indices contextuels.", true, "settings.hints.normal");
        screen.addOption(4, "Fréquence : forte", "Affiche beaucoup plus d'indications et rappels.", true, "settings.hints.high");
        screen.addOption(5, "Images IG : désactivées", "Verrouillé en terminal : les images ne servent qu'à l'IG et ne remplacent jamais les informations textuelles.", false, "settings.images.terminal_locked");

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

        if (choice == 5)
        {
            mainPlayer.forceTerminalImagePolicy();
            MessageScreen::show(
                "IMAGES VERROUILLÉES",
                "utility.settings.images.terminal_locked",
                {
                    "Le terminal n'affiche pas d'images et ne peut pas les activer.",
                    "L'IG pourra utiliser des images plus tard, mais seulement comme couche décorative/supplémentaire.",
                    "Toutes les informations importantes restent toujours écrites dans les textes, menus, cartes et descriptions."
                }
            );
            continue;
        }

        if (!value.empty())
        {
            mainPlayer.setInterfaceHintFrequency(value);
            mainPlayer.forceTerminalImagePolicy();
            saveCurrentProgress("Paramètres d'indications");
            MessageScreen::show(
                "PARAMÈTRES",
                "utility.settings.hints.changed",
                {"Fréquence des indications réglée sur : " + mainPlayer.getInterfaceHintFrequencyLabel() + ". Images terminal : désactivées."}
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

    if (choice == UtilityChoiceTeam)
    {
        QuestMenu::openTeamMenu(mainPlayer);
        saveCurrentProgress("Menu Équipe hors combat");
        Console::clear();
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

    if (choice == UtilityChoiceSaveReturnMenu)
    {
        savePartyProgress("Sauvegarder et retourner au Menu de voyage");
        throw ReturnToActivityMenuRequest();
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
    while (true)
    {
        const bool hasLastCombatRecap = lastCombatRecap.available;
        MenuScreen screen("INTERMÈDE APRÈS SORTIE", "post_combat.intermission");
        screen.addSubtitle(mainPlayer.getName() + " | Niveau " + std::to_string(mainPlayer.getLevel()));
        screen.addLine("La poussière retombe quelques secondes avant de reprendre la route.");
        screen.addLine("PV : " + std::to_string(mainPlayer.getHp()) + "/" + std::to_string(mainPlayer.getMaxHp()));
        screen.addLine("Argent séparé : " + mainPlayer.getInventory().getWalletLine());
        screen.addLine("Argent total : " + mainPlayer.getInventory().getWalletTotalLine());

        if (hasLastCombatRecap)
        {
            const CombatRecapSnapshot& before = lastCombatRecap.before;
            const CombatRecapSnapshot& after = lastCombatRecap.after;
            const int xpDelta = after.experience - before.experience;
            const long long copperDelta = after.totalCopper - before.totalCopper;
            const int hpDelta = after.hp - before.hp;
            const int victoryDelta = after.victories - before.victories;
            const int defeatDelta = after.defeats - before.defeats;
            const int escapeDelta = after.escapes - before.escapes;
            const int enemyDelta = after.enemiesKilled - before.enemiesKilled;
            const int bossDelta = after.bossesKilled - before.bossesKilled;
            const int hpPercent = after.maxHp > 0 ? (after.hp * 100 / after.maxHp) : 0;

            screen.addLine("Résumé de la sortie : " + lastCombatRecap.modeName + " | " + lastCombatRecap.difficultyName + ".");
            screen.addLine(
                "Bilan : XP " + std::to_string(xpDelta)
                + " | Argent " + Money::formatCopper(copperDelta >= 0 ? copperDelta : -copperDelta) + (copperDelta < 0 ? " perdus" : " gagnés")
                + " | PV " + std::to_string(hpDelta)
                + " | Ennemis " + std::to_string(enemyDelta)
                + " | Boss " + std::to_string(bossDelta)
                + "."
            );

            if (defeatDelta > 0)
            {
                screen.addLine("Phrase de retour : le registre garde une trace froide de cette chute, mais la route n'est pas encore terminée.");
            }
            else if (bossDelta > 0)
            {
                screen.addLine("Phrase de retour : quelque chose de plus ancien que les monstres ordinaires vient de perdre son souffle.");
            }
            else if (victoryDelta > 0 && hpPercent <= 30)
            {
                screen.addLine("Phrase de retour : tu reviens debout, mais ton souffle dit clairement que ce n'était pas une balade.");
            }
            else if (victoryDelta > 0)
            {
                screen.addLine("Phrase de retour : les traces derrière toi racontent assez bien qui a dominé l'affrontement.");
            }
            else if (escapeDelta > 0)
            {
                screen.addLine("Phrase de retour : parfois, survivre vaut mieux qu'une tombe héroïque au mauvais endroit.");
            }
            else
            {
                screen.addLine("Phrase de retour : le calme revient, assez longtemps pour vérifier ton sac et reprendre tes repères.");
            }
        }
        else
        {
            screen.addLine("Résumé de la sortie : aucun bilan récent enregistré dans cette session.");
            screen.addLine("Phrase de retour : le monde attend encore de savoir ce que tu vas lui arracher.");
        }

        screen.addLine("Continuer ramène au Menu de voyage. Monde / ville y reste l'activité des lieux visitables.");
        screen.addOption(0, "Continuer", "Retourner au Menu de voyage.", true, "post_combat.continue", makeUtilityItemData(mainPlayer, "continue", "Continuer", "Retourner au Menu de voyage."));
        screen.addOption(1, "Menu rapide", "Personnage, saisie libre, options de partie et sauvegarde.", true, "post_combat.quick_menu", makeUtilityItemData(mainPlayer, "menu", "Menu rapide", "Hub constant hors combat."));
        screen.addOption(2, "Personnage", "Inventaire, compétences, titres, quêtes acceptées, statistiques, équipe et échange.", true, "post_combat.character", makeUtilityItemData(mainPlayer, "menu", "Personnage", "Accès direct au sous-menu personnage."));
        screen.addOption(3, "Dernier récap détaillé", hasLastCombatRecap ? "Relire le bilan complet avant/après combat." : "Aucun combat récent enregistré dans cette session.", hasLastCombatRecap, "post_combat.last_recap", makeUtilityItemData(mainPlayer, "inspect", "Dernier récap", "Relire le bilan complet avant/après combat.", hasLastCombatRecap ? "Disponible" : "Indisponible"));
        screen.addOption(4, "Journal bêta", "Voir où trouver le journal de session à envoyer au dev.", true, "post_combat.beta_log", makeUtilityItemData(mainPlayer, "inspect", "Journal bêta", "Chemin du fichier de logs local à envoyer en cas de bug.", "Logs"));

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Veuillez choisir une option affichée."
        );

        Console::clear();

        if (choice == 0)
        {
            return true;
        }
        if (choice == 1)
        {
            openOutOfCombatUtilityMenu(true);
            continue;
        }
        if (choice == 2)
        {
            openQuickCharacterMenu(true);
            continue;
        }
        if (choice == 3)
        {
            displayLastCombatRecap();
            continue;
        }
        if (choice == 4)
        {
            MessageScreen::show(
                "JOURNAL BÊTA",
                "post_combat.beta_log.detail",
                {
                    "Fichier local à envoyer au dev si un combat, une exploration ou un menu bug :",
                    RuntimeLog::currentLogPath(),
                    "Le fichier est recréé pendant la session et peut être supprimé sans danger.",
                    "Le dernier récap de combat reste disponible dans ce même menu pour compléter ce journal."
                },
                false
            );
            continue;
        }
    }
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
        exchangeScreen.addOption(1, "Donner de l'argent", "Transfert direct depuis " + mainPlayer.getName() + ".", true, "exchange.give.gold");
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
            receiveScreen.addOption(1, "Argent", "Transférer une quantité d'or.", true, "exchange.receive.gold");
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
                "ARGENT À TRANSFÉRER",
                "exchange.gold.quantity",
                {
                    giver->getName() + " possède " + giver->getInventory().getWalletLine() + ".",
                    "Montant à transférer ?"
                },
                0,
                static_cast<int>(std::min<long long>(2147483647LL, giver->getInventory().getEconomyUnits())),
                "Montant invalide."
            );

            if (amount > 0 && giver->getInventory().spendEconomyUnits(amount))
            {
                receiver->getInventory().earnEconomyUnits(amount);
                receiver->refreshCurrencyTitles();
                MessageScreen::show("ÉCHANGE EFFECTUÉ", "exchange.gold.success", {Money::formatEconomyUnits(amount) + " transféré."}, false);
            }
            else
            {
                MessageScreen::show("ÉCHANGE ANNULÉ", "exchange.gold.none", {"Aucun argent transféré."}, false);
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
