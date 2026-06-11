// EN: BossPveMode.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossPveMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/boss/BossPveMode.hpp"

#include "combat/TurnManager.hpp"

#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "boss/BossCatalog.hpp"
#include "interface/CombatDisplay.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltyResult.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "progression/blessing/BlessingSystem.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/InitiativeSystem.hpp"
#include "combat/group/TurnOrder.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/summon/SummonControlMode.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"
#include "lore/LegendTriggerSystem.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <numeric>


namespace
{
    enum class BossPowerRisk
    {
        PlayerAdvantage,
        Balanced,
        Risky,
        MajorDanger,
        ProbableExtermination
    };

    struct BossPowerAnalysis
    {
        int playerScore;
        int bossScore;
        int ratioPercent;
        BossPowerRisk risk;
    };

    struct TemporaryClassGuard
    {
        Player& player;
        PlayerClass baseClass;
        bool active;

        TemporaryClassGuard(Player& target)
            : player(target),
              baseClass(
                  target.getType(),
                  target.getMaxHp(),
                  target.getMinDamage(),
                  target.getMaxDamage(),
                  target.getCriticalDamage(),
                  target.getHealingPotionCount(),
                  target.getDamagePotionCount()
              ),
              active(false)
        {
        }

        void markActive()
        {
            active = true;
        }

        ~TemporaryClassGuard()
        {
            if (active)
            {
                int currentHp = player.getHp();
                player.restoreClassState(baseClass, currentHp);
            }
        }
    };

    std::string toLowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    // EN: isAnyName declares or implements a focused behavior used by this module.
    // FR: isAnyName déclare ou implémente un comportement précis utilisé par ce module.
    bool isAnyName(const std::string& name, const std::vector<std::string>& aliases)
    {
        for (const std::string& alias : aliases)
        {
            if (name == alias)
            {
                return true;
            }
        }

        return false;
    }


    class BossPveNarration
    {
    public:
        template <typename T>
        BossPveNarration& operator<<(const T& value)
        {
            current << value;
            return *this;
        }

        using StreamManipulator = std::ostream& (*)(std::ostream&);

        BossPveNarration& operator<<(StreamManipulator manipulator)
        {
            if (manipulator == static_cast<StreamManipulator>(std::endl<char, std::char_traits<char>>))
            {
                flushLine();
            }
            else
            {
                manipulator(current);
            }
            return *this;
        }

        std::vector<std::string> takeLines()
        {
            flushLine();
            return lines;
        }

    private:
        std::ostringstream current;
        std::vector<std::string> lines;

        void flushLine()
        {
            std::string line = current.str();
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            {
                line.pop_back();
            }
            if (!line.empty())
            {
                lines.push_back(line);
            }
            current.str(std::string());
            current.clear();
        }
    };

    void showBossPveLines(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, waitAndClear);
        }
    }

    bool maybeTriggerFireFlightCheatPurgeAtSeventyFive(Boss& boss, Player& player, bool& alreadyTriggered)
    {
        if (alreadyTriggered || boss.getBossId() != 27 || boss.getMaxHp() <= 0 || boss.isDead())
        {
            return false;
        }

        if (boss.getHp() * 100 > boss.getMaxHp() * 75)
        {
            return false;
        }

        alreadyTriggered = true;
        const int cleared = player.clearActiveCheatPowersForFireFlight();
        if (cleared <= 0)
        {
            showBossPveLines(
                "INTERFACE FIREFLIGHT",
                "boss.fireflight.cheat_purge.empty",
                {
                    "FireFlight ouvre une interface que tu ne contrôles pas.",
                    "Seuil atteint : 75% de PV restants.",
                    "Il cherche des altérations actives... puis referme la fenêtre.",
                    "Aucun cheat actif à supprimer. Il sourit : \"Bien. Alors on peut continuer proprement.\""
                },
                false
            );
            return false;
        }

        showBossPveLines(
            "INTERFACE FIREFLIGHT",
            "boss.fireflight.cheat_purge.75",
            {
                "FireFlight s'arrête au moment exact où ses PV passent sous le seuil des 75%.",
                "Une fenêtre apparaît au-dessus de l'arène : [ACTIVE_CHEATS] -> PURGE.",
                "Il dit : \"Je tolère les traces. Pas les raccourcis qui jouent à ta place.\"",
                "Cheats actifs annulés pour ce combat : " + std::to_string(cleared) + ".",
                "État Altéré conservé : le passé reste écrit, mais les effets actifs viennent d'être coupés."
            },
            false
        );
        return true;
    }

    void maybeTriggerFireFlightCheatPurgeForParty(Boss& boss, std::vector<Player*>& party, bool& alreadyTriggered)
    {
        if (alreadyTriggered || boss.getBossId() != 27 || boss.getMaxHp() <= 0 || boss.isDead())
        {
            return;
        }

        if (boss.getHp() * 100 > boss.getMaxHp() * 75)
        {
            return;
        }

        alreadyTriggered = true;
        int cleared = 0;
        for (Player* member : party)
        {
            if (member != nullptr)
            {
                cleared += member->clearActiveCheatPowersForFireFlight();
            }
        }

        showBossPveLines(
            "INTERFACE FIREFLIGHT",
            "boss.fireflight.coop.cheat_purge.75",
            {
                "FireFlight coupe le tour de groupe au seuil des 75%.",
                "Interface : [PARTY_ACTIVE_CHEATS] -> PURGE.",
                "Il dit : \"Plus vous êtes nombreux, moins je laisse les raccourcis parler à votre place.\"",
                "Cheats actifs annulés dans le groupe : " + std::to_string(cleared) + ".",
                "Les personnages restent Altérés si leur histoire l'était déjà, mais les effets actifs sont neutralisés."
            },
            false
        );
    }

    // EN: displayFireFlightSpecialCharacterDialogue declares or implements a focused behavior used by this module.
    // FR: displayFireFlightSpecialCharacterDialogue déclare ou implémente un comportement précis utilisé par ce module.
    void displayFireFlightSpecialCharacterDialogue(const Player& player)
    {
        BossPveNarration narration;

        std::string name = toLowerAscii(player.getName());
        bool altered = player.isAlteredByCheats();

        narration << std::endl;
        narration << "FireFlight baisse légèrement la voix." << std::endl;

        if (isAnyName(name, {"louis"}))
        {
            narration << "Louis... là, ce n'est plus seulement un personnage." << std::endl;
            narration << "C'est la petite signature du créateur qui descend dans sa propre arène." << std::endl;
            narration << "Conseil du créateur : certains secrets ne se donnent pas dans les menus. Ils apparaissent quand tu reviens lire les registres après une victoire qui semblait inutile." << std::endl;
        }
        else if (isAnyName(name, {"fireflight", "fire flight"}))
        {
            narration << "FireFlight contre FireFlight..." << std::endl;
            narration << "Même le monde hésite à savoir lequel de nous deux est l'avatar et lequel tient vraiment le clavier." << std::endl;
            narration << "Conseil du créateur : quand une option paraît trop propre pour être vraie, cherche la ligne qui n'a pas été affichée." << std::endl;
        }
        else if (name == "hazak")
        {
            narration << "Hazak. Toujours l'ombre, toujours la lame, toujours cette façon de croire que gagner suffit." << std::endl;
            narration << "Je sais d'où tu viens. Une table, des dés, une histoire qui a refusé de rester dans un seul jeu." << std::endl;
            narration << "Indice : les morts récents ne sont pas toujours perdus pour un assassin qui comprend les ombres." << std::endl;
        }
        else if (name == "fail")
        {
            narration << "Fail... même ici, tu cherches sûrement comment transformer le problème en expérience dangereuse." << std::endl;
            narration << "Je préfère te prévenir : certaines instabilités deviennent plus fortes quand tu les traites comme des jouets." << std::endl;
            narration << "Indice : les noyaux instables n'aiment pas être conservés trop proprement." << std::endl;
        }
        else if (name == "aoi")
        {
            narration << "Aoi. Tu n'as pas besoin de parler fort pour que tes flammes soient entendues." << std::endl;
            narration << "Protège ton incantation. Le monde respecte parfois plus la patience que la puissance." << std::endl;
            narration << "Indice : une braise kitsune devient plus intéressante quand elle est gardée plutôt que vendue." << std::endl;
        }
        else if (name == "kanadé" || name == "kanade")
        {
            narration << "Kanadé. Je sais, ce combat t'énerve déjà." << std::endl;
            narration << "Mais même ta rage peut devenir une constellation si tu la laisses choisir sa forme." << std::endl;
            narration << "Indice : les effets zodiacaux aimeront rarement la stabilité parfaite." << std::endl;
        }
        else if (name == "skuro")
        {
            narration << "Skuro. Ta lame cherche toujours la matière avant la raison." << std::endl;
            narration << "Je ne vais pas te demander d'être doux. Juste de comprendre ce que tu découpes." << std::endl;
            narration << "Indice : certaines armures ne se brisent pas en tapant plus fort, mais en frappant au bon tour." << std::endl;
        }
        else if (name == "sanctus")
        {
            narration << "Sanctus. Toi, tu protèges même quand le monde préférerait te voir tomber." << std::endl;
            narration << "Garde cette lumière. Mais n'oublie pas qu'une protection devient une prison si tu ne choisis jamais d'avancer." << std::endl;
            narration << "Indice : Provocation et Posture de défense sont plus liées que beaucoup d'ennemis ne veulent l'admettre." << std::endl;
        }
        else if (name == "trexof")
        {
            narration << "Trexof. Assassin, calme, et probablement déjà en train de lire l'endroit où l'arène peut céder." << std::endl;
            narration << "Je te connais assez pour savoir que tu vas chercher la faille propre." << std::endl;
            narration << "Indice : les boss qui semblent punir une habitude révèlent souvent leur contre si tu changes de rythme deux tours avant l'ulti." << std::endl;
        }
        else if (name == "mattzelda")
        {
            narration << "Mattzelda. Même dans un combat final, tu réussirais presque à faire croire que tout ça est une blague." << std::endl;
            narration << "Mais je sais ce que ce nom représente : un pote que le jeu n'a pas voulu transformer en simple statistique." << std::endl;
            narration << "Indice : les colosses gagnent parfois plus en encaissant le bon coup qu'en cherchant le plus gros chiffre." << std::endl;
        }
        else if (name == "hestia")
        {
            narration << "Hestia..." << std::endl;
            narration << "Je vais être honnête : je n'ai pas vraiment envie de te voir tomber." << std::endl;
            narration << "Même derrière les dés, même derrière les règles, même derrière la création... il y a des personnages qu'on protège un peu trop." << std::endl;
            narration << "Indice : ton dôme n'est pas seulement une défense. Bien placé, il peut transformer un tour perdu en tour survivant." << std::endl;
        }
        else
        {
            narration << "Même si ton nom n'a pas de note personnelle dans mes fichiers, ta trace existe maintenant." << std::endl;
            narration << "Indice : un boss vaincu ne laisse pas seulement du butin. Certaines portes ne s'ouvrent qu'après sa chute." << std::endl;
        }

        if (altered)
        {
            narration << std::endl;
            narration << "Puis son regard accroche une cicatrice dans tes données." << std::endl;

            if (isAnyName(name, {"louis"}))
            {
                narration << "Louis... tricher avec ton propre reflet, c'est presque drôle. Presque." << std::endl;
                narration << "Tu voulais aider la création, ou seulement éviter qu'elle te réponde ?" << std::endl;
            }
            else if (isAnyName(name, {"fireflight", "fire flight"}))
            {
                narration << "FireFlight altéré. Voilà donc ce que ça donne quand le créateur laisse une commande ouverte dans sa propre poche." << std::endl;
                narration << "Je ne vais pas effacer la faute. Je vais la rendre intéressante." << std::endl;
            }
            else if (name == "hazak")
            {
                narration << "Hazak, tu sais mieux que personne qu'un contrat avec l'ombre finit toujours par réclamer une ligne en plus." << std::endl;
                narration << "Tes raccourcis ne t'ont pas rendu plus discret. Ils ont rendu ton ombre plus bavarde." << std::endl;
            }
            else if (name == "fail")
            {
                narration << "Fail, évidemment que tu as touché à l'interdit. Le problème, c'est que cette expérience-là a touché en retour." << std::endl;
                narration << "Ton altération n'est pas une réussite. C'est un résultat qui n'a pas encore explosé." << std::endl;
            }
            else if (name == "aoi")
            {
                narration << "Aoi, tes flammes n'aiment pas cette trace dans tes données." << std::endl;
                narration << "Une incantation tremble quand la main qui la porte a déjà forcé une règle." << std::endl;
            }
            else if (name == "kanadé" || name == "kanade")
            {
                narration << "Kanadé, même les étoiles refusent de servir de prétexte à une triche." << std::endl;
                narration << "Ta rage est légitime. Ton altération, elle, devra être assumée." << std::endl;
            }
            else if (name == "skuro")
            {
                narration << "Skuro, ce n'est pas la première fois qu'une lame veut couper la règle au lieu de l'ennemi." << std::endl;
                narration << "Mais ici, même la violence garde une mémoire." << std::endl;
            }
            else if (name == "sanctus")
            {
                narration << "Sanctus, une protection obtenue par raccourci protège moins bien l'âme que le corps." << std::endl;
                narration << "Si ta lumière tremble, ce n'est pas parce qu'elle est faible. C'est parce qu'elle sait." << std::endl;
            }
            else if (name == "trexof")
            {
                narration << "Trexof, trouver une faille n'est pas pareil que survivre à son retour." << std::endl;
                narration << "Merci pour le test. Maintenant, survis au correctif." << std::endl;
            }
            else if (name == "mattzelda")
            {
                narration << "Mattzelda, même tes blagues ont l'air moins légères quand le personnage porte une trace altérée." << std::endl;
                narration << "Mais je te laisse une chance : fais-en une histoire, pas une excuse." << std::endl;
            }
            else if (name == "hestia")
            {
                narration << "Hestia... je voulais te protéger, pas te voir devenir une donnée altérée." << std::endl;
                narration << "Alors écoute bien : survivre compte plus que prouver quoi que ce soit. Même ici." << std::endl;
            }
            else
            {
                narration << "Tu as utilisé des raccourcis. Je ne vais pas faire semblant de ne pas l'avoir vu." << std::endl;
                narration << "Ce n'est pas une condamnation. C'est une annotation. Et les annotations, ici, peuvent mordre." << std::endl;
            }
        }

        narration << std::endl;

        showBossPveLines(
            "MESSAGE DE FIREFLIGHT",
            "boss.fireflight.special_character_dialogue",
            narration.takeLines()
        );
    }

    // EN: estimateExpectedDamage declares or implements a focused behavior used by this module.
    // FR: estimateExpectedDamage déclare ou implémente un comportement précis utilisé par ce module.
    int estimateExpectedDamage(int minDamage, int maxDamage, int criticalDamage)
    {
        int normalAverage = (minDamage + maxDamage) / 2;
        return (normalAverage * 13 + criticalDamage * 4) / 20;
    }

    // EN: estimatePlayerExpectedDamage declares or implements a focused behavior used by this module.
    // FR: estimatePlayerExpectedDamage déclare ou implémente un comportement précis utilisé par ce module.
    int estimatePlayerExpectedDamage(const Player& player)
    {
        int minDamage = player.getMinDamage();
        int maxDamage = player.getMaxDamage();
        int criticalDamage = player.getCriticalDamage();

        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();

            if (!weapon.isBroken())
            {
                minDamage += weapon.getMinDamageBonus();
                maxDamage += weapon.getMaxDamageBonus();
                criticalDamage += weapon.getCriticalBonus();
            }
        }

        return estimateExpectedDamage(minDamage, maxDamage, criticalDamage);
    }

    // EN: estimateEquipmentStability declares or implements a focused behavior used by this module.
    // FR: estimateEquipmentStability déclare ou implémente un comportement précis utilisé par ce module.
    int estimateEquipmentStability(const Player& player)
    {
        int stability = 0;

        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();
            int weaponValue = weapon.getValue();

            if (!weapon.isBroken())
            {
                stability += weaponValue / 3;
            }
            else
            {
                stability += weaponValue / 10;
            }

            if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0)
            {
                stability += std::max(0, weapon.getDurability()) * 4;
            }
        }

        if (player.hasEquippedArmor())
        {
            Armor armor = player.getEquippedArmor();
            int armorValue = armor.getValue();

            if (!armor.isBroken())
            {
                stability += armorValue / 3;
                stability += armor.getDamageReduction() * 18;
            }
            else
            {
                stability += armorValue / 10;
            }

            if (!armor.isIndestructible() && armor.getMaxDurability() > 0)
            {
                stability += std::max(0, armor.getDurability()) * 4;
            }
        }

        return stability;
    }

    // EN: estimateDifficultyPressure declares or implements a focused behavior used by this module.
    // FR: estimateDifficultyPressure déclare ou implémente un comportement précis utilisé par ce module.
    int estimateDifficultyPressure(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return -120;

            case DifficultyMode::Hard:
                return 120;

            case DifficultyMode::Nightmare:
                return 220;

            case DifficultyMode::Lethal:
                return 260;

            case DifficultyMode::Normal:
            default:
                return 0;
        }
    }

    // EN: getRiskFromRatio declares or implements a focused behavior used by this module.
    // FR: getRiskFromRatio déclare ou implémente un comportement précis utilisé par ce module.
    BossPowerRisk getRiskFromRatio(int ratioPercent)
    {
        if (ratioPercent >= 120)
        {
            return BossPowerRisk::PlayerAdvantage;
        }

        if (ratioPercent >= 95)
        {
            return BossPowerRisk::Balanced;
        }

        if (ratioPercent >= 75)
        {
            return BossPowerRisk::Risky;
        }

        if (ratioPercent >= 55)
        {
            return BossPowerRisk::MajorDanger;
        }

        return BossPowerRisk::ProbableExtermination;
    }

    std::string getPowerLabel(BossPowerRisk risk)
    {
        switch (risk)
        {
            case BossPowerRisk::PlayerAdvantage:
                return "avantage joueur";

            case BossPowerRisk::Balanced:
                return "combat équilibré";

            case BossPowerRisk::Risky:
                return "combat risqué";

            case BossPowerRisk::MajorDanger:
                return "danger majeur";

            case BossPowerRisk::ProbableExtermination:
            default:
                return "extermination probable";
        }
    }

    // EN: analyzeBossPower declares or implements a focused behavior used by this module.
    // FR: analyzeBossPower déclare ou implémente un comportement précis utilisé par ce module.
    BossPowerAnalysis analyzeBossPower(const Player& player, const Boss& boss, DifficultyMode difficulty)
    {
        int playerDamagePressure = estimatePlayerExpectedDamage(player) * 28;
        int playerSurvival = player.getMaxHp() * 2;
        int playerResources = player.getHealingPotionCount() * 45 + player.getDamagePotionCount() * 30;
        int playerExperience = player.getLevel() * 24 + player.getBossesKilled() * 40;
        int playerEquipment = estimateEquipmentStability(player);

        int playerScore = playerDamagePressure
            + playerSurvival
            + playerResources
            + playerExperience
            + playerEquipment;

        int bossDamagePressure = estimateExpectedDamage(
            boss.getMinDamage(),
            boss.getMaxDamage(),
            boss.getCriticalDamage()
        ) * 34;
        int bossSurvival = boss.getMaxHp() * 3 / 2;
        int bossResources = boss.getHealingPotionCount() * 65 + boss.getDamagePotionCount() * 40;
        int bossUltimatePressure = boss.getMaxUltimateTurns() * 45 + boss.getMaxUltimateCooldown() * 8;
        int bossRegistryPressure = boss.getBossId() * 30;

        int bossScore = bossDamagePressure
            + bossSurvival
            + bossResources
            + bossUltimatePressure
            + bossRegistryPressure
            + estimateDifficultyPressure(difficulty);

        if (bossScore < 1)
        {
            bossScore = 1;
        }

        if (playerScore < 1)
        {
            playerScore = 1;
        }

        int ratioPercent = playerScore * 100 / bossScore;

        return BossPowerAnalysis{
            playerScore,
            bossScore,
            ratioPercent,
            getRiskFromRatio(ratioPercent)
        };
    }

    std::string getResonanceLabel(int ratioPercent)
    {
        if (ratioPercent >= 120) return "stable";
        if (ratioPercent >= 95) return "incertaine";
        if (ratioPercent >= 75) return "fragile";
        if (ratioPercent >= 55) return "brisée par endroits";
        return "presque étouffée";
    }

    std::string getPowerWarningText(BossPowerRisk risk)
    {
        switch (risk)
        {
            case BossPowerRisk::PlayerAdvantage:
                return "Ton corps ne tremble pas. Pour une fois, l'arène semble te reconnaître comme une vraie menace.";
            case BossPowerRisk::Balanced:
                return "Le registre ne voit aucun vainqueur évident. Ce combat peut basculer sur une seule bonne décision.";
            case BossPowerRisk::Risky:
                return "Ton instinct hésite. Ce boss est prenable, mais une erreur pourrait coûter très cher.";
            case BossPowerRisk::MajorDanger:
                return "Ton corps comprend avant ton esprit : cette entité est au-dessus de toi. Ce n'est pas impossible... mais l'arène sent déjà le sang.";
            case BossPowerRisk::ProbableExtermination:
            default:
                return "Ton instinct te hurle de reculer. Cette chose ne ressemble pas à un adversaire. Elle ressemble à une fin de partie.";
        }
    }

    MenuScreen buildBossPowerAnalysisScreen(
        const Player& player,
        const Boss& boss,
        DifficultyMode difficulty,
        DeathRuleMode deathRule,
        const BossPowerAnalysis& analysis
    )
    {
        MenuScreen screen("ANALYSE DE PUISSANCE", "boss.power_analysis");
        screen.addLine("Le registre analyse la variation d'énergie devant toi...");
        screen.addLine("Il compare ton niveau, ton état, ton équipement, tes ressources et la pression dégagée par l'entité.");
        screen.addLine("");
        screen.addLine("Résonance de " + player.getName() + " : " + getResonanceLabel(analysis.ratioPercent) + ".");
        screen.addLine("Puissance estimée de l'entité : " + getPowerLabel(analysis.risk) + ".");
        screen.addLine(getPowerWarningText(analysis.risk));

        const int recommendedLevel = BossCatalog::getRecommendedLevel(boss.getBossId());
        screen.addLine("");
        screen.addLine("Niveau du personnage : " + std::to_string(player.getLevel())
            + " | Niveau conseillé : [" + std::to_string(recommendedLevel) + "].");
        if (player.getLevel() < recommendedLevel)
        {
            const int missingLevels = recommendedLevel - player.getLevel();
            if (boss.getBossId() == 27)
            {
                screen.addLine("FireFlight est marqué [255] : tu peux forcer la porte si elle est débloquée, mais le registre appelle ça un dernier test, pas un duel normal.");
            }
            else if (missingLevels >= 40)
            {
                screen.addLine("Avertissement de niveau : tu es très loin du palier conseillé. Le combat reste possible, mais l'arène ressemble presque à un suicide volontaire.");
            }
            else if (missingLevels >= 15)
            {
                screen.addLine("Avertissement de niveau : tu es nettement sous le palier conseillé. Prépare potions, équipement et plan de secours.");
            }
            else
            {
                screen.addLine("Avertissement de niveau : tu es un peu sous le palier conseillé. C'est jouable, mais pas gratuit.");
            }
        }
        else
        {
            screen.addLine("Palier conseillé atteint ou dépassé : le risque réel dépend encore de l'équipement, des potions, de la difficulté et de ta stratégie.");
        }

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            screen.addLine("");
            screen.addLine("Mort définitive détectée.");
            screen.addLine("Si tu meurs ici, ce personnage peut devenir une simple trace dans le registre des morts.");
        }

        if (!boss.isIdentityRevealed())
        {
            screen.addLine("");
            screen.addLine("Le nom reste brouillé, mais sa pression suffit déjà à salir les pages du registre.");
        }

        screen.setDisplayOnlyInput("Analyse de boss affichée sans saisie directe.");
        return screen;
    }

    void displayBossPowerAnalysis(
        const Player& player,
        const Boss& boss,
        DifficultyMode difficulty,
        DeathRuleMode deathRule,
        const BossPowerAnalysis& analysis
    )
    {
        TerminalInterface::renderMenuScreen(buildBossPowerAnalysisScreen(player, boss, difficulty, deathRule, analysis), false);
    }

    // EN: runFireFlightFinalTest declares or implements a focused behavior used by this module.
    // FR: runFireFlightFinalTest déclare ou implémente un comportement précis utilisé par ce module.
    bool runFireFlightFinalTest(Player& player, Random& random)
    {
        MessageScreen::show(
            "TEST FINAL DU CRÉATEUR",
            "boss.fireflight.final_test.intro",
            {
                "FireFlight tombe. Puis le monde refuse de valider la victoire tout de suite.",
                "Tous les personnages créés par ce compte sont appelés comme des silhouettes figées.",
                "Même ton propre reflet apparaît de l'autre côté du champ de bataille.",
                "Les alliés non invoqués restent suspendus dans le temps, inanimés."
            },
            false
        );

        for (int round = 1; round <= 3 && !player.isDead(); ++round)
        {
            std::vector<std::string> lines;
            lines.push_back("Cycle final " + std::to_string(round) + "/3.");
            lines.push_back("4E_MUR::INTERFACE_BRISEE");
            lines.push_back("1 : Sentence céleste / Chasse sans lune / Dette royale");
            lines.push_back("2 : Couronne des ténèbres / Souffle de l'Origine / La forêt se souvient");
            lines.push_back("3 : Erreur critique : le récit se réécrit");
            lines.push_back("4 : Inversion des probabilités / Verdict des actes répétés");
            lines.push_back("5 : Sursis d'urgence : survivre trois tours");

            if (player.isAlteredByCheats())
            {
                lines.push_back("? : idontwanttodie");
                lines.push_back("? : whereismyplotarmor");
                lines.push_back("? : talktothedev");
                lines.push_back("Les anciens codes apparaissent dans l'interface comme des cicatrices, pas comme des solutions.");
            }

            int damage = 18 + random.between(8, 22) + player.getLevel() / 2;
            if (round == 2) damage += 8;
            if (round == 3) damage += 14;

            if (player.isInDefensePosture())
            {
                damage = damage * 75 / 100;
                lines.push_back("Ta posture réduit un peu le chaos, sans pouvoir l'annuler.");
            }

            const int hpBefore = player.getHp();
            player.takeDamage(damage);
            lines.push_back(player.getName() + " subit " + std::to_string(damage) + " dégâts de test final.");
            lines.push_back("PV : " + std::to_string(hpBefore) + " -> " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".");

            MessageScreen::show(
                "CYCLE FINAL " + std::to_string(round),
                "boss.fireflight.final_test.round." + std::to_string(round),
                lines,
                false
            );
        }

        if (player.isDead())
        {
            MessageScreen::show(
                "TEST FINAL ÉCHOUÉ",
                "boss.fireflight.final_test.defeat",
                {
                    "Le test final s'arrête.",
                    "Le monde n'a pas accepté ta version de la fin."
                },
                false
            );
            return false;
        }

        MessageScreen::show(
            "VICTOIRE RECONNUE",
            "boss.fireflight.final_test.success",
            {
                "FireFlight sourit.",
                "Cette fois, je n'ai plus de test à ajouter.",
                "Le test final reconnaît ta victoire."
            },
            false
        );
        return true;
    }

    // EN: applyGrinkaVictoryPenalty declares or implements a focused behavior used by this module.
    // FR: applyGrinkaVictoryPenalty déclare ou implémente un comportement précis utilisé par ce module.
    void applyGrinkaVictoryPenalty(Player& player, Random& random)
    {
        std::vector<std::string> lines;
        lines.push_back("Conséquence : Dette royale encaissée");
        lines.push_back("Boss : Grinka");
        lines.push_back("Effet visible : Volé par un boss si un bien est conservé par Grinka");
        lines.push_back("Récupération future : rebattre Grinka pour réclamer ce qui a été pris");
        lines.push_back("Grinka a gagné. Ses taxes ne sont plus une menace de combat : elles deviennent réelles.");

        int gold = player.getInventory().getGold();

        if (gold > 0)
        {
            int stolenGold = std::max(1, gold * random.between(20, 45) / 100);
            if (stolenGold > gold)
            {
                stolenGold = gold;
            }

            player.getInventory().spendGold(stolenGold);
            player.recordGrinkaGoldTheft(stolenGold);
            lines.push_back("Or confisqué : " + std::to_string(stolenGold) + " pièce(s).");
        }
        else
        {
            int xpLoss = 20 + player.getLevel() * 8;
            player.loseExperience(xpLoss);
            lines.push_back("Tu n'avais pas d'or sur toi.");
            lines.push_back("Grinka transforme la dette en humiliation administrative : perte de " + std::to_string(xpLoss) + " XP.");
        }

        if (!player.hasEquipmentProtection() && player.hasEquippedWeapon() && random.between(1, 100) <= 25)
        {
            Weapon stolenWeapon = player.getEquippedWeapon();
            if (player.destroyEquippedWeapon())
            {
                player.recordGrinkaStolenWeapon(stolenWeapon);
                lines.push_back("Un collecteur gobelin repart avec ton arme équipée.");
            }
        }

        if (!player.hasEquipmentProtection() && player.hasEquippedArmor() && random.between(1, 100) <= 18)
        {
            Armor stolenArmor = player.getEquippedArmor();
            if (player.destroyEquippedArmor())
            {
                player.recordGrinkaStolenArmor(stolenArmor);
                lines.push_back("Un autre collecteur arrache une pièce majeure de ton armure équipée.");
            }
        }

        lines.push_back("Dette négative : impossible");
        lines.push_back("Aucune dette ne descend sous zéro : Grinka vole ce qui existe, pas ce qui n'existe pas.");
        MessageScreen::show("DETTE ROYALE ENCAISSÉE", "boss.grinka.victory_penalty", lines, false);
    }

    // EN: applyZelefVictoryPenalty declares or implements a focused behavior used by this module.
    // FR: applyZelefVictoryPenalty déclare ou implémente un comportement précis utilisé par ce module.
    void applyZelefVictoryPenalty(Player& player, int maxHpBeforeFight)
    {
        int lostMaxHp = maxHpBeforeFight - player.getMaxHp();

        if (lostMaxHp <= 0)
        {
            lostMaxHp = std::max(5, maxHpBeforeFight / 20);
            player.reduceMaxHp(lostMaxHp);
        }

        player.recordZelefCorrosionLoss(lostMaxHp);

        PlayerCurse curse;
        curse.id = "zelef_black_blood_stain";
        curse.name = "Souillure de sang noir de Zelef";
        curse.severity = "majeure";
        curse.origin = "Défaite contre Zelef";
        curse.description = "La corrosion de Zelef ne vole pas seulement des PV maximum : elle laisse une trace sombre qui refuse les rites ordinaires.";
        curse.removalHint = "rebattre Zelef : seul le sang noir affronté à sa source rend la souillure muette.";
        curse.symptomCategories = "corruption,health,spirit";
        curse.discoveredSymptomCategories = "";
        curse.excludedSymptomCategories = "";
        curse.diagnosisLevel = 0;
        curse.appliedAtDay = player.getWorldDaysElapsed();
        curse.expiresAtDay = -1;
        curse.exorcismProgress = 0;
        curse.exorcismRequiredVisits = 0;
        curse.curseLevel = 2;
        curse.maxCurseLevel = 2;
        curse.evolvesOverTime = false;
        curse.escalationIntervalDays = 0;
        curse.nextEscalationDay = -1;
        curse.churchRemovalMaxLevel = 0;
        curse.becomesSpecialRemovalWhenTooHigh = false;
        curse.removableByChurch = false;
        curse.bossIdRequiredToBreak = 2;
        curse.lifeLong = false;
        player.addOrRefreshCurse(curse);
        const bool titleUnlocked = player.grantTitle("Porteur de trace");

        std::vector<std::string> zelefLines = {
                "Conséquence : corrosion ancrée",
                "Boss : Zelef",
                "PV maximum retenus : " + std::to_string(lostMaxHp),
                "Effet visible : Corrosion présente",
                "Trace maudite : ????? tant que l'église ne l'a pas diagnostiquée.",
                "Récupération future : rebattre Zelef pour récupérer les PV maximum volés et faire taire la souillure",
                "Ancrage : la souillure reste sur le personnage même hors combat, jusqu'à ce que Zelef soit rebattu.",
                "Zelef a gagné. La corrosion ne quitte pas entièrement ton corps.",
                "Si tu bats Zelef lors d'un prochain affrontement, tu pourras récupérer ce qu'il t'a pris."
        };
        if (titleUnlocked)
        {
            zelefLines.push_back("Titre obtenu : Porteur de trace.");
        }

        MessageScreen::show(
            "CORROSION ANCRÉE",
            "boss.zelef.victory_penalty",
            zelefLines,
            false
        );
    }

    void showNonCursingBossDefeatReminder(const Boss& boss)
    {
        MessageScreen::show(
            "DÉFAITE CONTRE UN BOSS",
            "boss.defeat.no_automatic_curse",
            {
                "Conséquence : défaite de boss enregistrée.",
                "Boss : " + boss.getName(),
                "Règle actuelle : tous les boss ne posent pas une malédiction.",
                "Seules les entités réellement obscures, corruptrices ou capables de maudire peuvent laisser une trace persistante."
            },
            false
        );
    }

    void applyLyknirVictoryCurse(Player& player)
    {
        const bool alreadyMarked = player.hasActiveCurse("lyknir_prey_mark");
        player.recordLyknirDefeatCurse();
        const bool titleUnlocked = player.grantTitle("Porteur de trace");

        std::vector<std::string> lyknirLines = {
            "Conséquence : malédiction de chasse",
            "Boss : Lyknir",
            "Effet visible : Marque de proie de Lyknir",
            "Durée : indéfinie tant que Lyknir n'est pas vaincu.",
            "Église : diagnostic possible, exorcisme impossible.",
            "Récupération future : rebattre Lyknir et reprendre ta place de chasseur."
        };
        if (titleUnlocked)
        {
            lyknirLines.push_back("Titre obtenu : Porteur de trace.");
        }

        MessageScreen::show(
            alreadyMarked ? "MARQUE DE PROIE RENFORCÉE" : "MARQUE DE PROIE",
            "boss.lyknir.victory_curse",
            lyknirLines,
            false
        );
    }

    struct BossDefeatCurseBlueprint
    {
        int bossId = 0;
        std::string id;
        std::string name;
        std::string origin;
        std::string description;
        std::string removalHint;
        std::string categories;
        int level = 1;
        int maxLevel = 1;
        bool evolves = false;
        int escalationIntervalDays = 0;
    };

    bool getBossDefeatCurseBlueprint(const Boss& boss, BossDefeatCurseBlueprint& blueprint)
    {
        switch (boss.getBossId())
        {
            case 6:
                blueprint = {
                    6,
                    "azelanos_shadow_crown",
                    "Couronne obscure d'Azelanos",
                    "Défaite contre l'Avatar d'Azelanos",
                    "Un morceau de couronne d'ombre reste posé derrière les pensées. Il alourdit la défense, la magie et la résistance morale.",
                    "vaincre l'Avatar d'Azelanos et laisser la couronne se fissurer à sa source.",
                    "corruption,spirit,defense,mana",
                    2,
                    4,
                    true,
                    3
                };
                return true;
            case 9:
                blueprint = {
                    9,
                    "inakari_false_reflection",
                    "Reflet mensonger d'Inakari",
                    "Défaite contre le Reflet d'Inakari",
                    "Un miroir minuscule reste dans le regard. Les gestes, la chance et les réactions sociales deviennent moins fiables.",
                    "vaincre Inakari et briser le reflet qui a appris ton visage.",
                    "hallucination,precision,luck,social",
                    1,
                    3,
                    true,
                    4
                };
                return true;
            case 11:
                blueprint = {
                    11,
                    "anomaly_interface_desync",
                    "Désynchronisation de l'Anomalie",
                    "Défaite contre l'Anomalie",
                    "L'Anomalie accroche une erreur persistante au personnage : l'interface peut clignoter, mentir, mélanger les cibles, provoquer le joueur, afficher des caractères corrompus ou faire croire à un faux PvE, même hors combat.",
                    "rebattre l'Anomalie pour forcer l'interface à reconnaître la vraie cible.",
                    "interface,hallucination,spirit,precision,social,corruption",
                    3,
                    5,
                    true,
                    2
                };
                return true;
            case 36:
                blueprint = {
                    36,
                    "anomaly_source_core_desync",
                    "Désynchronisation de la Source",
                    "Défaite contre la Source stable de l'Anomalie",
                    "La Source ne crée plus seulement des parasites : elle inscrit une erreur stable dans la perception du personnage. Hors combat, l'interface peut se décaler, choisir de faux intitulés, provoquer, masquer une option ou afficher une cible impossible.",
                    "rebattre la Source stable de l'Anomalie pour dissiper l'erreur dans l'air et les textures.",
                    "interface,hallucination,spirit,precision,social,corruption",
                    4,
                    6,
                    true,
                    1
                };
                return true;
            case 17:
                blueprint = {
                    17,
                    "luna_onyrae_waking_eclipse",
                    "Éclipse onirique persistante",
                    "Défaite contre le Fragment de Luna / Onyrae",
                    "Le rêve ne s'arrête plus exactement au réveil. Les cauchemars, la magie et la perception restent fissurés.",
                    "vaincre le fragment lunaire et laisser le rêve accepter la fin du combat.",
                    "sleep,hallucination,spirit,mana",
                    2,
                    4,
                    true,
                    3
                };
                return true;
            case 24:
                blueprint = {
                    24,
                    "aldebaroth_abyss_grudge",
                    "Rancune abyssale d'Aldebaroth",
                    "Défaite contre Aldebaroth",
                    "Le négatif du monde trouve une accroche dans le corps. La santé, l'attaque, l'aura sociale et l'esprit deviennent plus lourds à porter.",
                    "vaincre Aldebaroth et faire taire la rancune à sa source.",
                    "corruption,spirit,health,attack,social",
                    3,
                    4,
                    true,
                    3
                };
                return true;
            case 35:
                blueprint = {
                    35,
                    "velyssia_split_mirror",
                    "Reflet fendu de Velyssia",
                    "Défaite contre les Jumelles du Miroir Fendu",
                    "Une version mensongère du personnage répond parfois avant lui. Les reflets sociaux, la précision et la chance deviennent instables.",
                    "vaincre les Jumelles et obliger vérité et mensonge à se séparer.",
                    "hallucination,social,precision,luck",
                    2,
                    3,
                    true,
                    4
                };
                return true;
            default:
                return false;
        }
    }

    PlayerCurse createBossLockedCurse(Player& player, const BossDefeatCurseBlueprint& blueprint)
    {
        PlayerCurse curse;
        curse.id = blueprint.id;
        curse.name = blueprint.name;
        curse.severity = blueprint.level >= 3 ? "majeure" : (blueprint.level == 2 ? "moyenne" : "mineure");
        curse.origin = blueprint.origin;
        curse.description = blueprint.description;
        curse.removalHint = blueprint.removalHint;
        curse.symptomCategories = blueprint.categories;
        curse.discoveredSymptomCategories = "";
        curse.excludedSymptomCategories = "";
        curse.diagnosisLevel = 0;
        curse.appliedAtDay = player.getWorldDaysElapsed();
        curse.expiresAtDay = -1;
        curse.exorcismProgress = 0;
        curse.exorcismRequiredVisits = 0;
        curse.curseLevel = std::max(1, blueprint.level);
        curse.maxCurseLevel = std::max(curse.curseLevel, blueprint.maxLevel);
        curse.evolvesOverTime = blueprint.evolves;
        curse.escalationIntervalDays = blueprint.evolves ? std::max(1, blueprint.escalationIntervalDays) : 0;
        curse.nextEscalationDay = blueprint.evolves ? player.getWorldDaysElapsed() + curse.escalationIntervalDays : -1;
        curse.churchRemovalMaxLevel = 0;
        curse.becomesSpecialRemovalWhenTooHigh = false;
        curse.highLevelRemovalHint = blueprint.removalHint;
        curse.removableByChurch = false;
        curse.bossIdRequiredToBreak = blueprint.bossId;
        curse.lifeLong = false;
        return curse;
    }

    bool applyConfiguredBossDefeatCurse(Player& player, const Boss& boss)
    {
        BossDefeatCurseBlueprint blueprint;
        if (!getBossDefeatCurseBlueprint(boss, blueprint))
        {
            return false;
        }

        const bool alreadyActive = player.hasActiveCurse(blueprint.id);
        PlayerCurse curse = createBossLockedCurse(player, blueprint);
        player.addOrRefreshCurse(curse);
        const bool titleUnlocked = player.grantTitle("Porteur de trace");

        std::vector<std::string> lines = {
            "Conséquence : malédiction de boss",
            "Boss : " + boss.getName(),
            "Effet visible : " + blueprint.name,
            "Durée : indéfinie tant que la source n'est pas vaincue.",
            "Ancrage : la trace reste sur le personnage même hors combat, jusqu'à ce que ce boss soit rebattu.",
            "Église : diagnostic possible, exorcisme impossible tant que la source reste debout.",
            "Récupération future : " + blueprint.removalHint
        };

        if (boss.getBossId() == 11 || boss.getBossId() == 36)
        {
            lines.push_back("Perturbation spéciale : l'interface peut afficher de fausses cibles, un faux combat PvE, des caractères corrompus, des provocations ou une impression de te battre contre toi-même.");
            lines.push_back("Symptôme récurrent : trouble de la vision — puis vérifie deux fois qui est vraiment devant toi, même hors combat.");
            if (boss.getBossId() == 36)
            {
                lines.push_back("Source : cette version est stable juste assez longtemps pour être combattue ; si elle gagne, la trace est plus profonde que celle du fragment précoce.");
            }
        }
        if (titleUnlocked)
        {
            lines.push_back("Titre obtenu : Porteur de trace.");
        }

        MessageScreen::show(
            alreadyActive ? "MALÉDICTION RAVIVÉE" : "MALÉDICTION ACCROCHÉE",
            "boss.defeat.configured_curse." + std::to_string(boss.getBossId()),
            lines,
            false
        );

        return true;
    }

    void applyBossVictoryConsequenceAfterDefeat(
        Player& player,
        const Boss& boss,
        Random& random,
        int playerMaxHpBeforeFight,
        bool showNonCursingReminder
    )
    {
        if (boss.getBossId() == 1 || boss.getBossId() == 3)
        {
            if (showNonCursingReminder)
            {
                showNonCursingBossDefeatReminder(boss);
            }
            return;
        }

        if (boss.getBossId() == 2)
        {
            applyZelefVictoryPenalty(player, playerMaxHpBeforeFight);
            return;
        }

        if (boss.getBossId() == 5)
        {
            applyGrinkaVictoryPenalty(player, random);
            return;
        }

        if (boss.getBossId() == 4)
        {
            applyLyknirVictoryCurse(player);
            return;
        }

        applyConfiguredBossDefeatCurse(player, boss);
    }

    void recoverConfiguredBossCurseAfterVictory(Player& player, const Boss& boss)
    {
        BossDefeatCurseBlueprint blueprint;
        if (!getBossDefeatCurseBlueprint(boss, blueprint))
        {
            return;
        }

        if (!player.hasActiveCurse(blueprint.id))
        {
            return;
        }

        const int removedCurses = player.removeCursesLockedByBoss(boss.getBossId());
        if (removedCurses <= 0)
        {
            return;
        }

        std::vector<std::string> lines = {
            "Récupération : malédiction de boss rompue",
            "Boss : " + boss.getName(),
            "Effet retiré : " + blueprint.name,
            "La source a été affrontée en vrai : la trace n'a plus assez d'autorité pour rester."
        };

        if (boss.getBossId() == 11 || boss.getBossId() == 36)
        {
            lines.push_back("L'interface tremble une dernière fois, puis rend la vraie cible au joueur.");
            if (boss.getBossId() == 36)
            {
                lines.push_back("La Source ne meurt pas : elle se dissipe dans l'air et les textures, comme un décor qui refuse de rester chargé.");
            }
        }

        MessageScreen::show(
            "SOURCE BRISÉE",
            "boss.victory.configured_curse_recovered." + std::to_string(boss.getBossId()),
            lines,
            false
        );
    }


    bool hasCurseLockedByBoss(const Player& player, int bossId)
    {
        if (bossId <= 0)
        {
            return false;
        }

        for (const PlayerCurse& curse : player.getActiveCurses())
        {
            if (curse.bossIdRequiredToBreak == bossId)
            {
                return true;
            }
        }

        return false;
    }

    bool wouldHaveDefeatedBoss(const Player& player, int currentBossId, int searchedBossId)
    {
        if (currentBossId == searchedBossId)
        {
            return true;
        }
        const std::vector<int>& defeated = player.getDefeatedBossIds();
        return std::find(defeated.begin(), defeated.end(), searchedBossId) != defeated.end();
    }

    bool wouldHaveDefeatedEveryBossExceptFinal(const Player& player, int currentBossId, int finalBossId)
    {
        const int maximumBossId = BossCatalog::getMaximumBossId();
        for (int id = 1; id <= maximumBossId; ++id)
        {
            if (id == finalBossId)
            {
                continue;
            }
            if (!wouldHaveDefeatedBoss(player, currentBossId, id))
            {
                return false;
            }
        }
        return true;
    }

    bool wouldHaveDefeatedEveryBossIncludingFinal(const Player& player, int currentBossId)
    {
        const int maximumBossId = BossCatalog::getMaximumBossId();
        for (int id = 1; id <= maximumBossId; ++id)
        {
            if (!wouldHaveDefeatedBoss(player, currentBossId, id))
            {
                return false;
            }
        }
        return true;
    }

    std::string bossVictoryTitleForId(int bossId)
    {
        switch (bossId)
        {
            case 1: return "Briseur de chaînes célestes";
            case 2: return "Survivant du sang noir";
            case 3: return "Fissure du dernier rempart";
            case 4: return "Proie qui mord";
            case 5: return "Voleur de reine";
            case 6: return "Couronne brisée";
            case 7: return "Écaille de tempête";
            case 8: return "Ami des forêts furieuses";
            case 9: return "Renard du vrai reflet";
            case 10: return "Acquitté du silence";
            case 11: return "Erreur de surface";
            case 12: return "Minute volée";
            case 13: return "Nom sous les os";
            case 14: return "Survivant de la guerre";
            case 15: return "Cœur non soumis";
            case 16: return "Justiciable debout";
            case 17: return "Éveillé du cauchemar";
            case 18: return "Main des quatre éléments";
            case 19: return "Humain hors du registre";
            case 20: return "Chance retournée";
            case 21: return "Passe-seuil";
            case 22: return "Sujet sans royaume";
            case 23: return "Nom donné à l'inconnu";
            case 24: return "Rancune éteinte";
            case 25: return "Entre création et ruine";
            case 26: return "Écho d'univers";
            case 27: return "Regard du créateur";
            case 36: return "Source dissipée";
            case 28: return "Respiration volée";
            case 29: return "Clou retiré";
            case 30: return "Destin écarté";
            case 31: return "Berger des âmes";
            case 32: return "Briseur de horde";
            case 33: return "Sang royal refusé";
            case 34: return "Filet arraché";
            case 35: return "Vérité sans miroir";
            default: return "";
        }
    }

    void grantBossVictoryTitles(Player& player, const Boss& boss, bool coopParticipant)
    {
        std::vector<std::string> obtained;
        auto grant = [&](const std::string& title) {
            if (!title.empty() && player.grantTitle(title))
            {
                obtained.push_back(title);
            }
        };

        const bool hadBossLockedCurse = hasCurseLockedByBoss(player, boss.getBossId());
        grant("Tombeur de boss");
        grant(bossVictoryTitleForId(boss.getBossId()));

        const int finalBossId = 27;
        if (boss.getBossId() != finalBossId
            && wouldHaveDefeatedEveryBossExceptFinal(player, boss.getBossId(), finalBossId))
        {
            grant("Avant la dernière porte");
            grant("Une invitation ?");
        }
        if (boss.getBossId() == finalBossId && wouldHaveDefeatedEveryBossIncludingFinal(player, boss.getBossId()))
        {
            grant("Celui qui a tout vaincu");
            grant("La fin ???");
        }

        if (boss.getBossId() == 2 && (player.hasActiveCurse("zelef_black_blood_stain") || player.hasZelefCorrosionPresent()))
        {
            grant("Sang noir rendu");
        }
        if (hadBossLockedCurse)
        {
            grant("Exorcisé par revanche");
            grant("Ancre brisée");
        }
        if (boss.getBossId() == 11)
        {
            grant("Débogueur de cauchemar");
            grant("Menu qui répond");
        }
        if (boss.getBossId() == 36)
        {
            grant("Texture qui respire");
            grant("Source dissipée");
        }
        if (coopParticipant)
        {
            grant("Partage de prime propre");
            if (hadBossLockedCurse)
            {
                grant("Compagnon de revanche");
            }
        }

        if (obtained.empty())
        {
            return;
        }

        std::vector<std::string> lines;
        lines.push_back("La carte du personnage révèle de nouveaux titres. Les titres secrets obtenus ne restent plus en ????.");
        for (const std::string& title : obtained)
        {
            lines.push_back("Titre obtenu : " + title + ".");
        }

        showBossPveLines(
            "TITRES DE BOSS",
            "boss.victory.titles." + std::to_string(boss.getBossId()),
            lines
        );
    }

    // EN: getHitogamiRevivePercent declares or implements a focused behavior used by this module.
    // FR: getHitogamiRevivePercent déclare ou implémente un comportement précis utilisé par ce module.
    int getHitogamiRevivePercent(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return 10;
            case DifficultyMode::Hard:
                return 18;
            case DifficultyMode::Nightmare:
                return 22;
            case DifficultyMode::Lethal:
                return 25;
            case DifficultyMode::Normal:
            default:
                return 15;
        }
    }

    // EN: maybeReviveHitogamiOnce declares or implements a focused behavior used by this module.
    // FR: maybeReviveHitogamiOnce déclare ou implémente un comportement précis utilisé par ce module.
    void maybeReviveHitogamiOnce(Boss& boss, bool& alreadyRevived, DifficultyMode difficulty)
    {
        if (boss.getBossId() != 19 || alreadyRevived || !boss.isDead())
        {
            return;
        }

        alreadyRevived = true;
        int revivePercent = getHitogamiRevivePercent(difficulty);
        boss.reviveWithHealthPercentage(revivePercent);

        MessageScreen::show(
            "ÉCHO D'HITOGAMI",
            "boss.hitogami.revive_once",
            {
                "Hitogami tombe à genoux.",
                "Pendant une seconde, le combat semble terminé.",
                "Puis il respire encore.",
                "Tu n'es pas spécial parce que tu tombes.",
                "Tu es spécial parce que tu te relèves.",
                "L'écho d'Hitogami se relève avec " + std::to_string(boss.getHp()) + "/" + std::to_string(boss.getMaxHp()) + " PV."
            },
            false
        );
    }


    // EN: hasAllInvitationsBeforeFireFlight declares or implements a focused behavior used by this module.
    // FR: hasAllInvitationsBeforeFireFlight déclare ou implémente un comportement précis utilisé par ce module.
    bool hasAllInvitationsBeforeFireFlight(const Player& player)
    {
        for (int id = 1; id <= BossCatalog::getMaximumBossId(); ++id)
        {
            if (id == 27)
            {
                continue;
            }

            if (!player.isBossDefeated(id))
            {
                return false;
            }
        }

        return true;
    }

    // EN: displayFireFlightLockedGate declares or implements a focused behavior used by this module.
    // FR: displayFireFlightLockedGate déclare ou implémente un comportement précis utilisé par ce module.
    void displayFireFlightLockedGate()
    {
        MessageScreen::show(
            "SEUIL DE FIREFLIGHT",
            "boss.fireflight.locked_gate",
            {
                "L'entrée de FireFlight apparaît dans le registre.",
                "Mais la porte ne s'ouvre pas.",
                "Une phrase est gravée dans l'air :",
                "Prouve ta valeur devant chaque variation majeure. Rassemble toutes les invitations.",
                "FireFlight n'est pas une rencontre de passage.",
                "C'est le dernier test du jeu actuel."
            },
            false
        );
    }

    // EN: displayFireFlightFirstEntrance declares or implements a focused behavior used by this module.
    // FR: displayFireFlightFirstEntrance déclare ou implémente un comportement précis utilisé par ce module.
    void displayFireFlightFirstEntrance(const Player& player)
    {
        MessageScreen::show(
            "ENTRÉE DU CRÉATEUR",
            "boss.fireflight.first_entrance",
            {
                "Le registre cesse de faire semblant d'être neutre.",
                "Toutes les lettres récoltées brûlent en même temps dans ton inventaire.",
                "Une silhouette attend au centre d'une arène qui ressemble à un menu cassé.",
                "FireFlight te regarde, puis regarde au-dessus de toi, comme s'il voyait le code du monde.",
                "Tu as battu assez de règles pour mériter d'en rencontrer une vraie.",
                "Bienvenue dans le test final, " + player.getName() + "."
            },
            false
        );
        displayFireFlightSpecialCharacterDialogue(player);
    }


    // EN: askBossFightConfirmation declares or implements a focused behavior used by this module.
    // FR: askBossFightConfirmation déclare ou implémente un comportement précis utilisé par ce module.
    bool askBossFightConfirmation(const BossPowerAnalysis& analysis)
    {
        MenuScreen screen("CONFIRMATION BOSS", "boss.confirmation");

        if (analysis.risk == BossPowerRisk::MajorDanger
            || analysis.risk == BossPowerRisk::ProbableExtermination)
        {
            screen.addLine("Procéder malgré tout ?");
            screen.addLine("Le registre a déjà senti que cette arène pouvait devenir une tombe.");
        }
        else
        {
            screen.addLine("Procéder au combat ?");
        }

        screen.addOption(1, "Entrer dans l'arène", "Le tour sera lancé après cette confirmation.", true, "boss.confirm.enter");
        screen.addOption(0, "Reculer", "Refermer la faille pour l'instant.", true, "boss.confirm.back");

        int confirmation = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        return confirmation == 1;
    }

    int countAliveBossParty(const std::vector<Player*>& party)
    {
        int alive = 0;
        for (Player* player : party)
        {
            if (player != nullptr && !player->isDead())
            {
                ++alive;
            }
        }
        return alive;
    }

    struct BossCoopContribution
    {
        int turnsTaken = 0;
        int damageDealt = 0;
        int healingDone = 0;
        int damageTaken = 0;
        int supportActions = 0;
        bool wasDowned = false;
    };

    constexpr std::size_t BOSS_COOP_SUPPORT_PAGE_SIZE = 8;

    MenuOptionItemData makeBossCoopSupportData(
        const Player& healer,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "boss_coop_support";
        itemData.section = "Soutien boss coop";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = healer.getName();
        itemData.progress = "PV : " + std::to_string(healer.getHp()) + "/" + std::to_string(healer.getMaxHp());
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeBossCoopHealingTargetData(
        const Player& healer,
        const Player& target,
        std::size_t partyIndex
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "ally";
        itemData.section = "Cibles de soin";
        itemData.actionType = "support";
        itemData.name = target.getName();
        itemData.detail = target.isDead() ? "Allié au sol à réveiller" : "Allié blessé à soigner";
        itemData.status = "PV : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp());
        itemData.owner = healer.getName();
        itemData.progress = "J" + std::to_string(partyIndex + 1)
            + " - " + CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(partyIndex));
        itemData.important = target.isDead()
            || (target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 35);
        return itemData;
    }

    MenuOptionItemData makeBossCoopPotionData(
        const Player& healer,
        const Consumable& potion,
        int inventoryIndex,
        int amount = 1
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "potion";
        itemData.section = "Potions de soutien";
        itemData.actionType = "heal";
        itemData.name = potion.getName();
        itemData.quantity = std::to_string(std::max(1, amount));
        itemData.detail = potion.getDescription();
        itemData.status = "Soin : " + potion.getPowerDisplayText();
        itemData.price = "Valeur : " + std::to_string(potion.getValue()) + " or";
        itemData.stock = "Index inventaire : " + std::to_string(inventoryIndex + 1);
        itemData.owner = healer.getName();
        itemData.important = potion.getPower() >= 35;
        return itemData;
    }

    Player* askBossCoopHealingTarget(Player& healer, std::vector<Player*>& party)
    {
        std::vector<Player*> targets;
        std::vector<std::size_t> partyIndexes;

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* ally = party[i];
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                targets.push_back(ally);
                partyIndexes.push_back(i);
            }
        }

        if (targets.empty())
        {
            return nullptr;
        }

        std::size_t pageIndex = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(targets.size(), BOSS_COOP_SUPPORT_PAGE_SIZE);
            const std::size_t safePageIndex = std::min(pageIndex, totalPages - 1);
            const std::size_t start = PagedMenu::firstIndex(safePageIndex, BOSS_COOP_SUPPORT_PAGE_SIZE);
            const std::size_t end = PagedMenu::lastIndexExclusive(targets.size(), safePageIndex, BOSS_COOP_SUPPORT_PAGE_SIZE);

            MenuScreen targetScreen("CIBLE DU SOIN", "boss.coop.support.target");
            targetScreen.setPagination(safePageIndex, totalPages);
            targetScreen.addSubtitle(PagedMenu::pageInfoText(safePageIndex, totalPages, targets.size()));
            targetScreen.addLine("Choisis l'allié à soigner ou réveiller.");
            targetScreen.addOption(0, "Annuler", "Revenir au tour normal.", true, "boss.coop.support.target.cancel");

            for (std::size_t index = start; index < end; ++index)
            {
                Player* ally = targets[index];
                const std::size_t partyIndex = partyIndexes[index];
                std::string label = "J" + std::to_string(partyIndex + 1) + " - " + ally->getName();
                if (ally->isDead())
                {
                    label += " [au sol]";
                }

                targetScreen.addOption(
                    static_cast<int>(index - start + 1),
                    label,
                    std::to_string(ally->getHp()) + "/" + std::to_string(ally->getMaxHp()) + " PV",
                    true,
                    "boss.coop.support.target.select." + std::to_string(partyIndex + 1),
                    makeBossCoopHealingTargetData(healer, *ally, partyIndex)
                );
            }

            if (safePageIndex > 0)
            {
                targetScreen.addOption(98, "Page précédente", "Revoir les alliés précédents.", true, "boss.coop.support.target.previous_page");
            }
            if (safePageIndex + 1 < totalPages)
            {
                targetScreen.addOption(99, "Page suivante", "Voir les alliés suivants.", true, "boss.coop.support.target.next_page");
            }

            int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une cible affichée.");
            Console::clear();

            if (targetChoice == 0)
            {
                return nullptr;
            }
            if (targetChoice == 98 && safePageIndex > 0)
            {
                pageIndex = safePageIndex - 1;
                continue;
            }
            if (targetChoice == 99 && safePageIndex + 1 < totalPages)
            {
                pageIndex = safePageIndex + 1;
                continue;
            }

            const std::size_t selectedIndex = start + static_cast<std::size_t>(targetChoice - 1);
            if (selectedIndex < targets.size())
            {
                return targets[selectedIndex];
            }
        }
    }

    int askBossCoopSupportPotionIndex(Player& healer, const std::vector<int>& potionIndices)
    {
        if (potionIndices.empty())
        {
            return -1;
        }

        std::size_t pageIndex = 0;

        while (true)
        {
            std::vector<PotionStack> potionStacks = CombatPotionUtils::groupPotionIndices(healer, potionIndices);
            if (potionStacks.empty())
            {
                return -1;
            }

            const std::size_t totalPages = PagedMenu::pageCount(potionStacks.size(), BOSS_COOP_SUPPORT_PAGE_SIZE);
            const std::size_t safePageIndex = std::min(pageIndex, totalPages - 1);
            const std::size_t start = PagedMenu::firstIndex(safePageIndex, BOSS_COOP_SUPPORT_PAGE_SIZE);
            const std::size_t end = PagedMenu::lastIndexExclusive(potionStacks.size(), safePageIndex, BOSS_COOP_SUPPORT_PAGE_SIZE);

            MenuScreen potionScreen("POTION DE SOUTIEN", "boss.coop.support.potion");
            potionScreen.setPagination(safePageIndex, totalPages);
            potionScreen.addSubtitle(PagedMenu::pageInfoText(safePageIndex, totalPages, potionStacks.size()));
            potionScreen.addLine("Choisis la potion à utiliser.");
            potionScreen.addOption(0, "Annuler", "Garder la potion pour plus tard.", true, "boss.coop.support.potion.cancel");

            for (std::size_t index = start; index < end; ++index)
            {
                const PotionStack& stack = potionStacks[index];
                const int inventoryIndex = stack.firstIndex;
                Consumable potion = healer.getInventory().getConsumable(inventoryIndex);
                potionScreen.addOption(
                    static_cast<int>(index - start + 1),
                    CombatPotionUtils::stackLabel(potion.getName(), stack.amount),
                    "Soin " + potion.getPowerDisplayText() + " | quantité " + std::to_string(stack.amount) + " | valeur " + std::to_string(potion.getValue()),
                    true,
                    "boss.coop.support.potion.select." + std::to_string(inventoryIndex + 1),
                    makeBossCoopPotionData(healer, potion, inventoryIndex, stack.amount)
                );
            }

            if (safePageIndex > 0)
            {
                potionScreen.addOption(98, "Page précédente", "Revoir les potions précédentes.", true, "boss.coop.support.potion.previous_page");
            }
            if (safePageIndex + 1 < totalPages)
            {
                potionScreen.addOption(99, "Page suivante", "Voir les potions suivantes.", true, "boss.coop.support.potion.next_page");
            }

            int potionChoice = TerminalInterface::askMenuChoiceFromOptions(potionScreen, "Choisis une potion affichée.");
            Console::clear();

            if (potionChoice == 0)
            {
                return -1;
            }
            if (potionChoice == 98 && safePageIndex > 0)
            {
                pageIndex = safePageIndex - 1;
                continue;
            }
            if (potionChoice == 99 && safePageIndex + 1 < totalPages)
            {
                pageIndex = safePageIndex + 1;
                continue;
            }

            const std::size_t selectedIndex = start + static_cast<std::size_t>(potionChoice - 1);
            if (selectedIndex < potionStacks.size())
            {
                return potionStacks[selectedIndex].firstIndex;
            }
        }
    }

    std::vector<bool> extractBossDownedFlags(const std::vector<BossCoopContribution>& contributions)
    {
        std::vector<bool> flags;
        for (const BossCoopContribution& contribution : contributions) flags.push_back(contribution.wasDowned);
        return flags;
    }

    int scoreBossTargetThreat(Player& player, const BossCoopContribution& contribution)
    {
        if (player.isProvoking()) return 10000 + player.getProvocationTurns() * 100;

        int score = 20;
        if (player.hasHealingThreat()) score += 130;
        score += std::min(180, contribution.damageDealt / 2);
        score += std::min(120, contribution.healingDone);
        score += std::min(80, contribution.damageTaken / 3);

        if (player.getMaxHp() > 0)
        {
            int missingPercent = (player.getMaxHp() - player.getHp()) * 100 / player.getMaxHp();
            if (missingPercent >= 70) score += 45;
            else if (missingPercent >= 40) score += 25;
        }

        std::string type = CombatClassSystem::normalizeClassText(player.getType());
        if (type.find("clerc") != std::string::npos || type.find("pretre") != std::string::npos || type.find("prêtre") != std::string::npos || type.find("alchimiste") != std::string::npos) score += 50;
        if (type.find("gardien") != std::string::npos || type.find("tank") != std::string::npos || type.find("colosse") != std::string::npos || player.isInDefensePosture()) score += 25;

        return score;
    }

    Player* chooseAliveBossTarget(std::vector<Player*>& party, Random& random, const std::vector<BossCoopContribution>* contributions = nullptr)
    {
        std::vector<Player*> candidates;
        std::vector<int> scores;

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead()) continue;

            BossCoopContribution empty;
            const BossCoopContribution& contribution = (contributions != nullptr && i < contributions->size()) ? (*contributions)[i] : empty;
            candidates.push_back(player);
            scores.push_back(scoreBossTargetThreat(*player, contribution));
        }

        if (candidates.empty()) return nullptr;

        int totalScore = std::accumulate(scores.begin(), scores.end(), 0);
        int roll = random.between(1, std::max(1, totalScore));
        int cursor = 0;
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            cursor += scores[i];
            if (roll <= cursor) return candidates[i];
        }
        return candidates.back();
    }

    void scaleBossForCoop(Boss& boss, int partySize)
    {
        if (partySize <= 1) return;

        int hpPercent = 100 + (partySize - 1) * 55;
        int damagePercent = 100 + (partySize - 1) * 12;
        boss.scaleCombatStats(hpPercent, damagePercent);

        showBossPveLines(
            "STABILISATION COOP",
            "boss.coop.scaling",
            {
                "Stabilisation coop : le boss adapte son enveloppe.",
                "PV x" + std::to_string(hpPercent) + "% | puissance x" + std::to_string(damagePercent) + "% selon le nombre de joueurs réels.",
                "Le sceau refuse qu'un adversaire majeur perde sa forme face au nombre."
            }
        );
    }

    void displayBossCoopPartyStatus(const std::vector<Player*>& party, const std::vector<bool>& wasDowned)
    {
        std::vector<std::string> lines;
        lines.push_back("État actuel du groupe face au boss.");
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr)
            {
                continue;
            }

            std::ostringstream line;
            line << "J" << (i + 1) << " ["
                 << CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(i))
                 << "] - " << player->getName()
                 << " : " << player->getHp() << "/" << player->getMaxHp() << " PV";
            if (player->isDead())
            {
                line << " [au sol]";
            }
            else if (i < wasDowned.size() && wasDowned[i])
            {
                line << " [a déjà chuté]";
            }
            lines.push_back(line.str());
        }

        showBossPveLines("ÉTAT DU GROUPE", "boss.coop.party_status", lines);
    }

    bool tryUseBossHealingPotionOnAlly(Player& healer, std::vector<Player*>& party, int& healingDone)
    {
        bool hasTarget = false;
        for (Player* ally : party)
        {
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                hasTarget = true;
                break;
            }
        }

        if (!hasTarget)
        {
            return false;
        }

        std::vector<int> potionIndices = CombatPotionUtils::getPotionIndices(healer, ConsumableType::Healing);
        if (potionIndices.empty())
        {
            return false;
        }

        MenuScreen supportScreen("SOUTIEN COOP BOSS", "boss.coop.support.choice");
        supportScreen.addLine("Action de soutien disponible pour " + healer.getName() + ".");
        supportScreen.addLine("Le soin coop consomme le tour du personnage qui aide.");
        supportScreen.addOption(
            1,
            "Utiliser une potion de soin sur un allié",
            "Le tour de ce personnage sera consommé.",
            true,
            "boss.coop.support.heal",
            makeBossCoopSupportData(
                healer,
                "heal",
                "Potion de soin allié",
                "Choisir un allié blessé ou au sol, puis une potion.",
                "Action consommée",
                true
            )
        );
        supportScreen.addOption(
            0,
            "Jouer normalement",
            "Ne pas prendre le rôle de soigneur ce tour-ci.",
            true,
            "boss.coop.support.skip",
            makeBossCoopSupportData(
                healer,
                "attack",
                "Jouer normalement",
                "Garde le tour normal : attaque, potion personnelle, défense ou autre action disponible.",
                "Sans soutien"
            )
        );

        int supportChoice = TerminalInterface::askMenuChoiceFromOptions(supportScreen, "Choisis 0 ou 1.");
        Console::clear();

        if (supportChoice == 0)
        {
            return false;
        }

        Player* target = askBossCoopHealingTarget(healer, party);

        if (target == nullptr)
        {
            return false;
        }

        int consumableIndex = askBossCoopSupportPotionIndex(healer, potionIndices);

        if (consumableIndex < 0)
        {
            return false;
        }
        if (!healer.getInventory().hasConsumable(consumableIndex))
        {
            showBossPveLines(
                "POTION INDISPONIBLE",
                "boss.coop.support.potion.missing",
                {"Cette potion n'est plus disponible."}
            );
            return false;
        }

        Consumable potion = healer.getInventory().getConsumable(consumableIndex);
        bool revivedTarget = target->isDead();
        if (revivedTarget)
        {
            target->reviveWithHealthPercentage(1);
            if (target->getHp() <= 0)
            {
                target->heal(1);
            }
        }
        int beforeHealHp = target->getHp();
        const int announcedHeal = potion.getHealingAmountForMaxHp(target->getMaxHp());
        target->heal(announcedHeal);
        healingDone += std::max(0, target->getHp() - beforeHealHp);
        healer.markHealingThreat();
        healer.recordChallengeCombatAction("ally_consumable");

        if (!healer.hasInfiniteConsumables())
        {
            healer.getInventory().removeConsumable(consumableIndex);
        }

        std::vector<std::string> supportLines;
        supportLines.push_back(healer.getName() + " devient soigneur ce tour-ci et utilise " + potion.getName() + " sur " + target->getName() + ".");
        if (revivedTarget)
        {
            supportLines.push_back(target->getName() + " est réveillé à 1 PV avant de recevoir le soin.");
        }
        supportLines.push_back(target->getName() + " possède maintenant " + std::to_string(target->getHp()) + "/" + std::to_string(target->getMaxHp()) + " PV.");
        supportLines.push_back("Le tour de " + healer.getName() + " est consommé.");
        showBossPveLines("SOIN COOP", "boss.coop.support.result", supportLines);
        return true;
    }

    void resolveBossLethalGroupDeathSave(Player& player, Random& random)
    {
        if (!player.isDead())
        {
            return;
        }

        int green = 0;
        int red = 0;
        std::vector<std::string> lines;
        lines.push_back("Mort définitive coop boss : " + player.getName() + " est au sol.");
        lines.push_back("3 pastilles vertes le ramènent. 3 rouges le rayent du registre, sauf intervention capable de briser le destin.");

        auto showSave = [&]()
        {
            showBossPveLines("SURVIE EN MORT DÉFINITIVE COOP", "boss.coop.lethal.save", lines);
        };

        while (green < 3 && red < 3)
        {
            int roll = random.between(1, 20);
            lines.push_back("Dé de survie : " + std::to_string(roll));

            if (roll == 20)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                lines.push_back("20 naturel : " + player.getName() + " se relève immédiatement à 1 PV et pourra rejouer.");
                showSave();
                return;
            }

            if (roll == 1)
            {
                red += 2;
                lines.push_back("1 naturel : deux pastilles rouges apparaissent d'un coup.");
            }
            else if (roll >= 11)
            {
                ++green;
                lines.push_back("Pastille verte : " + std::to_string(green) + "/3.");
            }
            else
            {
                ++red;
                lines.push_back("Pastille rouge : " + std::to_string(red) + "/3.");
            }
        }

        if (green >= 3)
        {
            player.reviveWithHealthPercentage(1);
            if (player.getHp() <= 0) player.heal(1);
            lines.push_back(player.getName() + " revient à 1 PV. La mort n'est pas comptée.");
            showSave();
            return;
        }

        if (BlessingSystem::tryTriggerLethalSurvival(player))
        {
            lines.push_back(player.getName() + " reçoit trois pastilles rouges, mais toutes ses bénédictions se consument avant l'effacement.");
            lines.push_back("Retour à 1 PV, inventaire et équipement détruits, marque irréversible appliquée.");
            showSave();
            DeathPenaltySystem::displayLethalSurvivalAnomaly();
            return;
        }

        player.recordDeath();
        lines.push_back(player.getName() + " reçoit trois pastilles rouges : mort définitive. Aucune bénédiction capable de briser le verdict n'a répondu.");
        showSave();
    }

    bool isBossUnlockedForWholeParty(const std::vector<Player*>& party, int bossId)
    {
        for (Player* player : party)
        {
            if (player == nullptr || !player->isBossUnlocked(bossId))
            {
                return false;
            }
        }
        return true;
    }

    void displayBossLockForParty(const std::vector<Player*>& party, int bossId)
    {
        std::vector<std::string> lines;
        lines.push_back("Ce boss ne peut pas être stabilisé en coop.");
        lines.push_back("Tous les joueurs doivent l'avoir débloqué.");
        lines.push_back("Joueurs qui bloquent l'accès :");
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || !player->isBossUnlocked(bossId))
            {
                lines.push_back("- J" + std::to_string(i + 1) + " : " + (player == nullptr ? "slot vide" : player->getName()));
            }
        }

        showBossPveLines("BOSS COOP VERROUILLÉ", "boss.coop.locked", lines);
    }

    CombatReward buildIndividualBossCoopReward(
        const CombatReward& baseReward,
        const Player& player,
        const Player& leader,
        const BossCoopContribution& contribution
    )
    {
        int participation = contribution.turnsTaken > 0 ? 35 : 0;
        participation += std::min(45, contribution.damageDealt / 10);
        participation += std::min(30, contribution.healingDone / 6);
        participation += std::min(20, contribution.damageTaken / 10);
        participation += contribution.supportActions * 8;
        if (contribution.wasDowned)
        {
            participation = std::max(15, participation - 25);
        }

        int levelGap = leader.getLevel() - player.getLevel();
        if (levelGap >= 30) participation = std::min(participation, 25);
        else if (levelGap >= 20) participation = std::min(participation, 45);
        else if (levelGap >= 12) participation = std::min(participation, 70);

        participation = std::max(0, std::min(100, participation));
        return baseReward.getPercentage(participation);
    }

    bool bossCanTargetAnySummon(const std::vector<std::vector<Summon>>& partySummons)
    {
        for (const std::vector<Summon>& summons : partySummons)
        {
            if (SummonCombatSystem::hasTargetableSummons(summons))
            {
                return true;
            }
        }

        return false;
    }

    bool tryBossAttackPartySummon(
        Boss& boss,
        std::vector<std::vector<Summon>>& partySummons,
        std::vector<BossCoopContribution>& contributions,
        Random& random
    )
    {
        if (!bossCanTargetAnySummon(partySummons) || random.between(1, 100) > 25)
        {
            return false;
        }

        std::vector<std::size_t> ownerIndexes;
        for (std::size_t i = 0; i < partySummons.size(); ++i)
        {
            if (SummonCombatSystem::hasTargetableSummons(partySummons[i]))
            {
                ownerIndexes.push_back(i);
            }
        }

        if (ownerIndexes.empty())
        {
            return false;
        }

        std::size_t ownerIndex = ownerIndexes[random.between(0, static_cast<int>(ownerIndexes.size()) - 1)];
        int summonIndex = SummonCombatSystem::chooseStrategicTargetableSummonIndex(
            partySummons[ownerIndex],
            boss,
            random
        );

        if (summonIndex < 0)
        {
            return false;
        }

        showBossPveLines(
            "LIEN D'INVOCATION",
            "boss.coop.summon_target",
            {boss.getName() + " sent le lien d'invocation et tente de couper le soutien avant le joueur."}
        );

        SummonCombatSystem::entityAttacksSummon(
            boss,
            partySummons[ownerIndex][static_cast<std::size_t>(summonIndex)],
            random
        );
        SummonCombatSystem::removeInactiveSummons(partySummons[ownerIndex]);

        if (ownerIndex < contributions.size())
        {
            contributions[ownerIndex].damageTaken += 4;
            contributions[ownerIndex].supportActions++;
        }

        return true;
    }


    MenuOptionItemData makeBossChoiceTypeItemData(
        const std::string& name,
        const std::string& detail,
        const std::string& actionType,
        bool coop
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "boss";
        itemData.section = coop ? "Boss coop" : "Boss";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = coop ? "Compatible groupe" : "Registre personnel";
        itemData.owner = coop ? "Groupe" : "Joueur";
        itemData.important = true;
        return itemData;
    }

    MenuScreen buildBossChoiceTypeScreen(const std::string& playerName, bool coop)
    {
        MenuScreen screen(coop ? "BOSS COOP" : "REGISTRE DES BOSS", coop ? "boss.coop.choice_type" : "boss.choice_type");
        screen.addSubtitle(playerName + ", choisis le type d'apparition du boss");

        if (coop)
        {
            screen.addLine("Le boss choisi doit être compatible avec tout le groupe.");
            screen.addOption(
                1,
                "Boss aléatoire compatible",
                "Le registre choisit parmi les boss débloqués par tous.",
                true,
                "boss.choice.random.coop",
                makeBossChoiceTypeItemData("Boss aléatoire compatible", "Le registre choisit parmi les boss débloqués par tous.", "select", true)
            );
            screen.addOption(
                2,
                "Choisir le boss",
                "Afficher les entités accessibles au groupe.",
                true,
                "boss.choice.manual.coop",
                makeBossChoiceTypeItemData("Choisir le boss", "Afficher les entités accessibles au groupe.", "select", true)
            );
        }
        else
        {
            screen.addOption(
                1,
                "Boss aléatoire",
                "Le registre choisit une entité disponible.",
                true,
                "boss.choice.random",
                makeBossChoiceTypeItemData("Boss aléatoire", "Le registre choisit une entité disponible.", "select", false)
            );
            screen.addOption(
                2,
                "Choisir le boss",
                "Afficher les entités que tu peux stabiliser.",
                true,
                "boss.choice.manual",
                makeBossChoiceTypeItemData("Choisir le boss", "Afficher les entités que tu peux stabiliser.", "select", false)
            );
        }

        return screen;
    }

    int askBossChoiceType(const std::string& playerName, bool coop)
    {
        return TerminalInterface::askMenuChoiceFromOptions(
            buildBossChoiceTypeScreen(playerName, coop),
            "Veuillez entrer 1 ou 2."
        );
    }

    constexpr std::size_t BOSS_SELECTION_PAGE_SIZE = 8;

    bool idInList(const std::vector<int>& ids, int bossId)
    {
        return std::find(ids.begin(), ids.end(), bossId) != ids.end();
    }

    std::string bossSelectionDisplayName(int bossId, const std::vector<int>& defeatedBossIds)
    {
        std::string name = BossCatalog::getRegistryDisplayName(bossId);

        if (idInList(defeatedBossIds, bossId))
        {
            Boss knownBoss = BossCatalog::createBoss(bossId);
            name = knownBoss.getName() + " [Battu]";
        }

        return name + " " + BossCatalog::getRecommendedLevelText(bossId);
    }

    MenuOptionItemData makeBossSelectionItemData(
        int bossId,
        bool enabled,
        bool coop,
        const std::string& hint,
        const std::vector<int>& defeatedBossIds,
        const std::string& discoveryLocation
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "boss";
        const bool identityKnown = idInList(defeatedBossIds, bossId)
            || BossCatalog::getRegistryDisplayName(bossId) != "???";
        itemData.section = idInList(defeatedBossIds, bossId)
            ? "Boss vaincus"
            : (identityKnown ? "Boss identifiés" : "Entités détectées");
        itemData.actionType = "select";
        itemData.name = bossSelectionDisplayName(bossId, defeatedBossIds);
        itemData.detail = hint;
        itemData.status = idInList(defeatedBossIds, bossId)
            ? "Déjà battu"
            : (enabled
                ? (identityKnown
                    ? (coop ? "Boss identifié — accessible au groupe" : "Boss identifié — accessible")
                    : (coop ? "Emplacement détecté — identité inconnue" : "Emplacement détecté — identité inconnue"))
                : (coop ? "Bloqué par le groupe" : "Trace encore instable"));
        itemData.progress = "Danger estimé : niveau conseillé "
            + std::to_string(BossCatalog::getRecommendedLevel(bossId));
        if (!discoveryLocation.empty())
        {
            itemData.progress += " | Emplacement approximatif : " + discoveryLocation;
        }
        itemData.owner = coop ? "Registre commun" : "Registre personnel";
        itemData.important = enabled;
        return itemData;
    }

    MenuScreen buildBossSelectionScreen(
        const std::vector<int>& visibleBossIds,
        const std::vector<int>& enabledBossIds,
        const std::vector<int>& defeatedBossIds,
        const Player& registryOwner,
        bool coop,
        std::size_t pageIndex
    )
    {
        MenuScreen screen(coop ? "SÉLECTION DU BOSS COOP" : "SÉLECTION DU BOSS", coop ? "boss.coop.selection" : "boss.selection");
        screen.addLine(coop
            ? "Sélectionne l'entité que le groupe veut affronter."
            : "Sélectionne l'entité que tu veux exterminer.");
        screen.addLine("Les niveaux affichés sont recommandés, pas bloquants : tenter avant ce niveau reste à tes risques et périls.");
        screen.addLine("Le registre n'affiche que les entités déjà détectées pour ce personnage.");
        screen.addLine("Le niveau conseillé est une estimation de danger, pas une connaissance complète de l'entité.");

        const std::size_t totalPages = PagedMenu::pageCount(visibleBossIds.size(), BOSS_SELECTION_PAGE_SIZE);
        const std::size_t safePageIndex = std::min(pageIndex, totalPages - 1);
        const std::size_t start = PagedMenu::firstIndex(safePageIndex, BOSS_SELECTION_PAGE_SIZE);
        const std::size_t end = PagedMenu::lastIndexExclusive(visibleBossIds.size(), safePageIndex, BOSS_SELECTION_PAGE_SIZE);

        screen.setPagination(safePageIndex, totalPages);
        screen.addSubtitle(PagedMenu::pageInfoText(safePageIndex, totalPages, visibleBossIds.size()));

        for (std::size_t index = start; index < end; ++index)
        {
            const int id = visibleBossIds[index];
            const bool enabled = std::find(enabledBossIds.begin(), enabledBossIds.end(), id) != enabledBossIds.end();
            const std::string discoveryLocation = registryOwner.getBossDiscoveryLocation(id);
            std::string hint = BossCatalog::getRegistryHint(id)
                + " Danger estimé : niveau conseillé " + std::to_string(BossCatalog::getRecommendedLevel(id))
                + " (non bloquant).";
            if (!discoveryLocation.empty())
            {
                hint += " Emplacement approximatif mémorisé : " + discoveryLocation + ".";
            }
            if (!enabled)
            {
                hint += coop
                    ? " L'un des registres du groupe bloque encore cette faille."
                    : " Cette faille est encore verrouillée ou trop instable.";
            }

            screen.addOption(
                id,
                bossSelectionDisplayName(id, defeatedBossIds),
                hint,
                enabled,
                coop ? "boss.coop.select." + std::to_string(id) : "boss.select." + std::to_string(id),
                makeBossSelectionItemData(id, enabled, coop, hint, defeatedBossIds, discoveryLocation)
            );
        }

        screen.addOption(0, "Retour", "Refermer le registre.", true, "boss.selection.back");

        if (safePageIndex > 0)
        {
            screen.addOption(98, "Page précédente", "Revoir les boss précédents.", true, "boss.selection.previous_page");
        }
        if (safePageIndex + 1 < totalPages)
        {
            screen.addOption(99, "Page suivante", "Voir les boss suivants.", true, "boss.selection.next_page");
        }

        screen.addFooterLine("Les boss vaincus récemment sont temporairement instables.");
        screen.addFooterLine("Leurs statistiques exactes restent inconnues avant l'arène.");

        return screen;
    }

    int askBossSelectionChoice(
        const std::vector<int>& visibleBossIds,
        const std::vector<int>& enabledBossIds,
        const std::vector<int>& defeatedBossIds,
        const Player& registryOwner,
        bool coop
    )
    {
        std::size_t pageIndex = 0;

        while (true)
        {
            Console::clear();
            MenuScreen screen = buildBossSelectionScreen(visibleBossIds, enabledBossIds, defeatedBossIds, registryOwner, coop, pageIndex);
            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Veuillez entrer un identifiant de boss affiché."
            );

            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }

            const std::size_t totalPages = PagedMenu::pageCount(visibleBossIds.size(), BOSS_SELECTION_PAGE_SIZE);
            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }

            return choice;
        }
    }

}

void BossPveMode::run(
    Player& player1,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule
)
{
    MessageScreen::show(
        "AURA DE BOSS",
        "boss.pve.intro",
        {
            "Vous sentez une aura maléfique autour de vous.",
            "Par chance, votre âme est encore pure.",
            "À mesure que vous avancez, votre force change, comme si l'arène refusait de vous laisser mourir trop vite."
        },
        false
    );

    Console::pauseSeconds(2);

    int bossChoiceType = askBossChoiceType(player1.getName(), false);

    std::vector<int> visibleBossIds = player1.getUnlockedBossIds();
    std::vector<int> availableBossIds = player1.getAvailableBossIds();
    if (!hasAllInvitationsBeforeFireFlight(player1))
    {
        visibleBossIds.erase(
            std::remove(visibleBossIds.begin(), visibleBossIds.end(), 27),
            visibleBossIds.end()
        );
        availableBossIds.erase(
            std::remove(availableBossIds.begin(), availableBossIds.end(), 27),
            availableBossIds.end()
        );
    }

    if (availableBossIds.empty())
    {
        MessageScreen::show(
            "AUCUN BOSS STABLE",
            "boss.pve.none_available",
            {
                "Aucune entité stable n'est disponible pour le moment.",
                "Affronte d'autres boss ou laisse quelques jours passer pour stabiliser le registre."
            }
        );
        return;
    }

    int bossChoice;

    if (bossChoiceType == 1)
    {
        int index = random.between(0, static_cast<int>(availableBossIds.size()) - 1);
        bossChoice = availableBossIds[index];
    }
    else
    {
        bossChoice = askBossSelectionChoice(
            visibleBossIds,
            availableBossIds,
            player1.getDefeatedBossIds(),
            player1,
            false
        );

        if (bossChoice == 0)
        {
            return;
        }

        if (!player1.isBossUnlocked(bossChoice))
        {
            MessageScreen::show(
                "ENTITÉ INCONNUE",
                "boss.pve.locked_unknown",
                {
                    "Cette entité n'est pas encore détectée dans le registre.",
                    "Un boss vaincu peut provoquer l'apparition d'une nouvelle variation anormale."
                }
            );
            return;
        }

        if (player1.isBossRecentlyDefeated(bossChoice))
        {
            MessageScreen::show(
                "ENTITÉ INSTABLE",
                "boss.pve.recently_defeated",
                {
                    "Cette entité vient d'être affrontée.",
                    "Le registre refuse de la stabiliser tout de suite : affronte deux autres boss avant."
                }
            );
            return;
        }
    }

    if (bossChoice == 27 && !hasAllInvitationsBeforeFireFlight(player1))
    {
        displayFireFlightLockedGate();
        Console::waitForEnter();
        return;
    }

    Boss boss = BossCatalog::createBoss(bossChoice);

    TemporaryClassGuard temporaryClassGuard(player1);
    PlayerClass evolvedClass = ClassCatalog::createEvolvedClassFromClass(player1.getType());
    Player simulatedPlayer = player1;
    simulatedPlayer.applyClass(evolvedClass);

    Console::clear();

    BossPowerAnalysis powerAnalysis = analyzeBossPower(simulatedPlayer, boss, difficulty);
    displayBossPowerAnalysis(simulatedPlayer, boss, difficulty, deathRule, powerAnalysis);

    if (!askBossFightConfirmation(powerAnalysis))
    {
        MessageScreen::show(
            "FAILLE REFERMÉE",
            "boss.pve.confirmation.back",
            {
                "Le registre referme la faille avant qu'elle ne devienne une tombe.",
                "L'entité reste quelque part dans le brouillard, en attendant que tu sois vraiment prêt."
            }
        );
        return;
    }

    Console::clear();

    player1.applyClass(evolvedClass);
    temporaryClassGuard.markActive();

    MessageScreen::show(
        "ÉVOLUTION TEMPORAIRE",
        "boss.pve.temporary_class",
        {
            player1.getName() + ", ta classe évolue temporairement en : " + player1.getType() + ".",
            "Tes PV et tes objets ont été renforcés pour ce combat.",
            "Après le combat, cette forme devra se dissiper et ta classe réelle reviendra."
        },
        false
    );

    player1.displayStats();

    Console::pauseSeconds(3);

    if (boss.getBossId() == 27)
    {
        boss.revealIdentity();
        displayFireFlightFirstEntrance(player1);
        Console::pauseSeconds(3);
    }

    LegendTriggerSystem::maybeDisplayBossRoomLegend(boss, random);

    MessageScreen::show(
        "PRÉPARATION DU BOSS",
        "boss.pve.preparation",
        {
            "Préparation du boss...",
            boss.getName() + " est entré dans l'arène.",
            boss.getName() + " est de type : " + boss.getType() + ".",
            "Fais attention : ce combat pourrait vraiment te coûter cher."
        },
        false
    );
    Console::pauseSeconds(2);

    if (player1.getLethalCheatAttemptCount() > 0
        && (boss.getBossId() == 11 || boss.getBossId() == 16 || boss.getBossId() == 26 || boss.getBossId() == 27 || boss.getBossId() == 30 || boss.getBossId() == 36))
    {
        std::vector<std::string> lethalLines;
        if (boss.getBossId() == 11 || boss.getBossId() == 36)
        {
            lethalLines.push_back(boss.getBossId() == 36 ? "La Source de l'Anomalie stabilise ton ancien refus." : "L'Anomalie incline la tête.");
            lethalLines.push_back("Ah. C'était donc toi qui avais essayé de griffer la page depuis une mort définitive.");
        }
        else if (boss.getBossId() == 16)
        {
            lethalLines.push_back("Lexior ouvre un dossier déjà marqué.");
            lethalLines.push_back("Tentative risquée précédemment constatée. Le verdict s'en souvient.");
        }
        else if (boss.getBossId() == 26)
        {
            lethalLines.push_back("L'écho d'Obérion te reconnaît sans colère.");
            lethalLines.push_back("Tu as déjà tenté de séparer la vie de ses conséquences.");
        }
        else if (boss.getBossId() == 27)
        {
            lethalLines.push_back("FireFlight soupire comme quelqu'un qui relit un vieux log.");
            lethalLines.push_back("Ah. C'est toi qui as tenté un code en mort définitive. J'avais laissé une note pour ça.");
        }
        else
        {
            lethalLines.push_back("Moiran déroule un fil déjà taché par une tentative interdite.");
            lethalLines.push_back("Tu as voulu sortir du destin en mort définitive. Le destin a simplement noté ton nom.");
        }
        showBossPveLines("TRACE DE MORT DÉFINITIVE", "boss.pve.lethal_cheat_memory", lethalLines);
    }

    Console::pauseSeconds(3);

    int turn = random.chooseFirstTurn();

    MessageScreen::show(
        "COMBAT BOSS",
        "boss.pve.combat_start",
        {
            "Prépare-toi...",
            "Le combat contre " + boss.getName() + " commence maintenant."
        },
        false
    );
    Console::pauseSeconds(2);

    int playerMaxHpBeforeBossFight = player1.getMaxHp();
    int playerHpBeforeBossFight = player1.getHp();
    int bossCombatTurnCount = 0;
    bool hitogamiAlreadyRevived = false;
    bool fireFlightCheatPurgeAt75Done = false;
    player1.beginChallengeCombatTracking();

    while (!player1.isDead() && !boss.isDead())
    {
        bool turnFinished = false;

        if (turn == 1)
        {
            turnFinished = TurnManager::playHumanTurn(
                player1,
                boss,
                random,
                BOSS_POTION_HEAL_AMOUNT,
                BOSS_POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                ++bossCombatTurnCount;
                maybeTriggerFireFlightCheatPurgeAtSeventyFive(boss, player1, fireFlightCheatPurgeAt75Done);
                maybeReviveHitogamiOnce(boss, hitogamiAlreadyRevived, difficulty);
                TurnManager::checkBossDecryption(boss);
                boss.reduceUltimateCooldown();
                turn = 2;
            }
        }
        else
        {
            turnFinished = TurnManager::playBossTurn(
                boss,
                player1,
                random
            );

            if (turnFinished)
            {
                ++bossCombatTurnCount;
                turn = 1;
            }
        }
    }

    if (boss.isDead() && !boss.isIdentityRevealed())
    {
        boss.revealIdentity();
    }

    CombatDisplay::displayCombatResult(player1, boss);

    player1.clearBossEquipmentSeal();

    if (player1.isDead())
    {
        player1.finishChallengeCombatTracking(false, true, true, 0);
        player1.recordDefeat();

        applyBossVictoryConsequenceAfterDefeat(
            player1,
            boss,
            random,
            playerMaxHpBeforeBossFight,
            true
        );

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            if (BlessingSystem::tryTriggerLethalSurvival(player1))
            {
                DeathPenaltySystem::displayLethalSurvivalAnomaly();
                return;
            }
            player1.recordDeath();
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

        player1.recordDeath();
        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player1,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);

        player1.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        MessageScreen::show(
            "RETOUR À LA VIE",
            "combat.boss.revive",
            {
                player1.getName() + " revient à lui avec " + std::to_string(player1.getHp()) + "/" + std::to_string(player1.getMaxHp()) + " PV.",
                "Même vaincu, tu n'es pas encore sorti du registre des vivants."
            },
            false
        );

        return;
    }

    if (boss.isDead())
    {
        if (boss.getBossId() == 27 && !runFireFlightFinalTest(player1, random))
        {
            player1.finishChallengeCombatTracking(false, true, true, 0);
            player1.recordDefeat();

            if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
            {
                if (BlessingSystem::tryTriggerLethalSurvival(player1))
                {
                    DeathPenaltySystem::displayLethalSurvivalAnomaly();
                    return;
                }
                player1.recordDeath();
                DeathPenaltySystem::displayLethalDeathCorruption();
                return;
            }

            player1.recordDeath();
            DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
                player1,
                difficulty,
                random
            );

            DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);
            player1.reviveWithHealthPercentage(
                DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
            );
            return;
        }

        player1.finishChallengeCombatTracking(true, true, true, 1);
        player1.recordVictory();
        player1.recordBossKill();
        player1.recordBossKillByName(boss.getName());

        grantBossVictoryTitles(player1, boss, false);

        bool newEntityDetected = player1.recordBossVictoryInRegistry(boss.getBossId());

        if (player1.hasActiveCurse("boss_threshold_omen"))
        {
            player1.getInventory().addMaterial(MaterialCatalog::createById("source_defeat_notice", 1));
            MessageScreen::show(
                "TRACE DE SEUIL",
                "boss.threshold_omen.source_defeated",
                {
                    "La victoire laisse une preuve nette : la source liée à un seuil a réellement été affrontée.",
                    "Objet reçu : Note de source vaincue x1.",
                    "Avec un diagnostic total, l'église pourra confirmer la rupture au lieu de sceller au hasard."
                },
                false
            );
        }

        if (newEntityDetected)
        {
            MessageScreen::show(
                "REGISTRE BOSS",
                "boss.registry.new_entity",
                {
                    "Cette première victoire fournit une preuve assez forte pour stabiliser une nouvelle signature.",
                    "Nom : ???",
                    "Statut : prochaine présence inconnue détectée.",
                    "Une seule entrée a été ajoutée. Son identité, son emplacement précis et ses mécaniques restent inconnus."
                },
                false
            );
        }



        if (boss.getBossId() == 4 && player1.hasActiveCurse("lyknir_prey_mark"))
        {
            const int removedCurses = player1.removeCursesLockedByBoss(4);
            if (removedCurses > 0)
            {
                MessageScreen::show(
                    "MARQUE BRISÉE",
                    "boss.lyknir.curse_recovered",
                    {
                        "Récupération : malédiction de chasse rompue",
                        "Boss : Lyknir",
                        "Effet retiré : Marque de proie de Lyknir",
                        "La meute garde le silence. Cette fois, tu n'es plus la proie."
                    },
                    false
                );
            }
        }

        if (boss.getBossId() == 2 && player1.hasZelefCorrosionPresent())
        {
            int recovered = player1.getZelefMaxHpStolen();
            player1.restoreZelefCorrosionLoss();
            const int removedZelefCurses = player1.removeCursesLockedByBoss(2);
            std::vector<std::string> lines = {
                "Récupération : corruption rendue",
                "Boss : Zelef",
                "PV maximum récupérés : " + std::to_string(recovered),
                "Effet retiré : Corrosion présente",
                "Tu fais face au sang noir qui t'avait marqué.",
                "Rends-moi ce que tu m'as pris il y a longtemps."
            };
            if (removedZelefCurses > 0)
            {
                lines.push_back("Trace maudite rompue : Souillure de sang noir de Zelef.");
            }
            MessageScreen::show(
                "CORROSION RENDUE",
                "boss.zelef.corrosion_recovered",
                lines,
                false
            );
        }

        recoverConfiguredBossCurseAfterVictory(player1, boss);

        if (boss.getBossId() == 5 && player1.hasGrinkaBossTheftPresent())
        {
            player1.restoreGrinkaBossTheft();
            MessageScreen::show(
                "BUTIN RÉCUPÉRÉ",
                "boss.grinka.theft_recovered",
                {
                    "Récupération : butin volé rendu",
                    "Boss : Grinka",
                    "Effet retiré : Volé par un boss",
                    "Inventaire : biens volés réinjectés",
                    "Grinka lâche un sac de butin avec un sourire beaucoup moins royal.",
                    "Rends-moi ce que tu m'as pris il y a longtemps."
                },
                false
            );
        }

        CombatReward bossReward = CombatRewardSystem::calculateBossReward(
            boss,
            difficulty,
            std::max(0, playerHpBeforeBossFight - player1.getHp()),
            bossCombatTurnCount
        );
        CombatRewardSystem::displayReward(bossReward);
        CombatRewardSystem::giveRewardToPlayer(player1, bossReward);

        LootGenerator::giveDefeatedBossLoot(player1, boss, random, difficulty);
    }
}

void BossPveMode::runTeam(
    std::vector<Player*>& party,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule
)
{
    if (party.empty() || party[0] == nullptr)
    {
        return;
    }

    Player& leader = *party[0];

    Console::clear();
    MessageScreen::show(
        "BOSS COOP",
        "boss.coop.intro",
        {
            "Joueur principal : " + leader.getName(),
            "Les données de boss, niveau de session et validation d'arène suivent le joueur 1.",
            "Mais l'accès au boss exige que tous les joueurs l'aient débloqué.",
            "Les récompenses resteront individuelles selon participation, niveau et survie."
        }
    );

    int bossChoiceType = askBossChoiceType(leader.getName(), true);

    std::vector<int> leaderVisibleBossIds = leader.getUnlockedBossIds();
    std::vector<int> leaderAvailableBossIds = leader.getAvailableBossIds();
    if (!hasAllInvitationsBeforeFireFlight(leader))
    {
        leaderVisibleBossIds.erase(
            std::remove(leaderVisibleBossIds.begin(), leaderVisibleBossIds.end(), 27),
            leaderVisibleBossIds.end()
        );
        leaderAvailableBossIds.erase(
            std::remove(leaderAvailableBossIds.begin(), leaderAvailableBossIds.end(), 27),
            leaderAvailableBossIds.end()
        );
    }

    std::vector<int> coopAvailableBossIds;
    for (int id : leaderAvailableBossIds)
    {
        if (isBossUnlockedForWholeParty(party, id))
        {
            coopAvailableBossIds.push_back(id);
        }
    }

    if (coopAvailableBossIds.empty())
    {
        MessageScreen::show(
            "AUCUN BOSS COMMUN",
            "boss.coop.none_available",
            {
                "Aucun boss commun n'est disponible pour ce groupe.",
                "Un joueur secondaire bloque probablement l'accès à tous les boss accessibles au joueur 1.",
                "Faites progresser les registres de boss des personnages secondaires, ou lancez un boss déjà commun."
            }
        );
        return;
    }

    int bossChoice = 0;

    if (bossChoiceType == 1)
    {
        int index = random.between(0, static_cast<int>(coopAvailableBossIds.size()) - 1);
        bossChoice = coopAvailableBossIds[index];
    }
    else
    {
        bossChoice = askBossSelectionChoice(
            leaderVisibleBossIds,
            coopAvailableBossIds,
            leader.getDefeatedBossIds(),
            leader,
            true
        );

        if (bossChoice == 0)
        {
            return;
        }

        if (!leader.isBossUnlocked(bossChoice))
        {
            MessageScreen::show(
                "ENTITÉ INCONNUE",
                "boss.coop.locked_leader_unknown",
                {"Cette entité n'est pas encore détectée dans le registre du joueur principal."}
            );
            return;
        }

        if (leader.isBossRecentlyDefeated(bossChoice))
        {
            MessageScreen::show(
                "ENTITÉ INSTABLE",
                "boss.coop.recently_defeated",
                {
                    "Cette entité vient d'être affrontée par le joueur principal.",
                    "Le registre refuse de la stabiliser tout de suite."
                }
            );
            return;
        }

        if (bossChoice == 27 && !hasAllInvitationsBeforeFireFlight(leader))
        {
            displayFireFlightLockedGate();
            Console::waitForEnter();
            return;
        }

        if (!isBossUnlockedForWholeParty(party, bossChoice))
        {
            displayBossLockForParty(party, bossChoice);
            Console::waitForEnter();
            return;
        }
    }

    Boss boss = BossCatalog::createBoss(bossChoice);

    std::vector<std::unique_ptr<TemporaryClassGuard>> classGuards;
    std::vector<int> initialHp;
    std::vector<int> initialMaxHp;
    std::vector<BossCoopContribution> contributions(party.size());
    std::vector<std::vector<Summon>> partySummons(party.size());
    std::vector<SummonControlMode> summonControlModes(party.size(), SummonControlMode::Automatic);

    for (std::size_t partyIndex = 0; partyIndex < party.size(); ++partyIndex)
    {
        Player* player = party[partyIndex];
        if (player == nullptr)
        {
            initialHp.push_back(0);
            initialMaxHp.push_back(0);
            continue;
        }

        classGuards.push_back(std::make_unique<TemporaryClassGuard>(*player));
        PlayerClass evolvedClass = ClassCatalog::createEvolvedClassFromClass(player->getType());
        player->applyClass(evolvedClass);
        classGuards.back()->markActive();
        initialHp.push_back(player->getHp());
        initialMaxHp.push_back(player->getMaxHp());
        partySummons[partyIndex] = SummonCombatSystem::createInitialSummonsFor(*player);
    }

    Console::clear();

    Player simulatedLeader = leader;
    simulatedLeader.applyClass(ClassCatalog::createEvolvedClassFromClass(leader.getType()));
    scaleBossForCoop(boss, static_cast<int>(party.size()));
    BossPowerAnalysis powerAnalysis = analyzeBossPower(simulatedLeader, boss, difficulty);
    displayBossPowerAnalysis(simulatedLeader, boss, difficulty, deathRule, powerAnalysis);

    MessageScreen::show(
        "ANALYSE COOP",
        "boss.coop.power_context",
        {
            "Analyse coop : le boss reste calibré depuis le joueur principal, mais il pourra cibler n'importe quel allié vivant.",
            "Si un joueur secondaire n'aide presque pas, son registre ne progressera pas vraiment."
        },
        false
    );

    if (!askBossFightConfirmation(powerAnalysis))
    {
        MessageScreen::show(
            "FAILLE REFERMÉE",
            "boss.coop.confirmation.back",
            {"Le groupe recule avant que la faille ne se referme sur lui."}
        );
        return;
    }

    Console::clear();

    for (Player* player : party)
    {
        if (player != nullptr) player->beginChallengeCombatTracking();
    }

    if (boss.getBossId() == 27)
    {
        boss.revealIdentity();
        displayFireFlightFirstEntrance(leader);
        Console::pauseSeconds(3);
    }

    MessageScreen::show(
        "PRÉPARATION BOSS COOP",
        "boss.coop.preparation",
        {
            "Préparation du boss coop...",
            boss.getName() + " est entré dans l'arène.",
            boss.getName() + " est de type : " + boss.getType() + ".",
            "Chaque joueur a temporairement évolué selon sa classe, puis reviendra à son état réel après le combat.",
            "Les invocations actives peuvent maintenant aider en boss coop, mais restent en dernière priorité de placement."
        },
        false
    );
    Console::pauseSeconds(1);

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        if (party[i] != nullptr && !partySummons[i].empty())
        {
            SummonCombatSystem::displaySummonArrival(*party[i], partySummons[i]);
            summonControlModes[i] = SummonCombatSystem::askPlayerSummonControlMode(*party[i], partySummons[i]);
        }
    }

    int round = 1;
    int bossCombatTurnCount = 0;
    bool hitogamiAlreadyRevived = false;
    bool fireFlightCheatPurgeAt75Done = false;

    while (countAliveBossParty(party) > 0 && !boss.isDead())
    {
        MessageScreen::show(
            "TOUR DE GROUPE BOSS",
            "boss.coop.round." + std::to_string(round),
            {
                "Tour " + std::to_string(round) + ".",
                boss.getName() + " : " + std::to_string(boss.getHp()) + "/" + std::to_string(boss.getMaxHp()) + " PV.",
                "Joueurs, invocations et boss agissent maintenant selon une initiative commune."
            },
            false
        );
        displayBossCoopPartyStatus(party, extractBossDownedFlags(contributions));

        InitiativeQueue initiative = InitiativeSystem::buildBossQueue(party, boss, partySummons, random);
        showBossPveLines(
            "ORDRE D'INITIATIVE",
            "boss.coop.initiative." + std::to_string(round),
            InitiativeSystem::buildDisplayLines(initiative)
        );

        for (const InitiativeRoll& entry : initiative.getEntries())
        {
            if (boss.isDead() || countAliveBossParty(party) <= 0) break;

            if (TurnOrder::isPlayer(entry.id))
            {
                const std::size_t i = static_cast<std::size_t>(std::max(0, entry.slotIndex));
                if (i >= party.size()) continue;
                Player* player = party[i];
                if (player == nullptr || player->isDead()) continue;

                MessageScreen::show(
                    "TOUR JOUEUR",
                    "boss.coop.player_turn",
                    {"Tour de " + player->getName() + " [J" + std::to_string(i + 1) + "] — initiative " + std::to_string(entry.totalScore) + "."},
                    false
                );

                int healingDoneThisTurn = 0;
                const int bossHpBeforeTurn = boss.getHp();
                bool finished = tryUseBossHealingPotionOnAlly(*player, party, healingDoneThisTurn);
                while (!finished && !player->isDead() && !boss.isDead())
                {
                    finished = TurnManager::playHumanTurn(
                        *player,
                        boss,
                        random,
                        BOSS_POTION_HEAL_AMOUNT,
                        BOSS_POTION_DAMAGE_BONUS
                    );
                }

                if (finished)
                {
                    contributions[i].turnsTaken++;
                    contributions[i].damageDealt += std::max(0, bossHpBeforeTurn - boss.getHp());
                    contributions[i].healingDone += healingDoneThisTurn;
                    if (healingDoneThisTurn > 0) contributions[i].supportActions++;
                    ++bossCombatTurnCount;
                    maybeTriggerFireFlightCheatPurgeForParty(boss, party, fireFlightCheatPurgeAt75Done);
                    maybeReviveHitogamiOnce(boss, hitogamiAlreadyRevived, difficulty);
                    TurnManager::checkBossDecryption(boss);
                }
            }
            else if (TurnOrder::isSummonGroup(entry.id))
            {
                const std::size_t i = static_cast<std::size_t>(std::max(0, entry.slotIndex));
                if (i >= party.size() || i >= partySummons.size()) continue;
                Player* owner = party[i];
                if (owner == nullptr || owner->isDead() || !SummonCombatSystem::hasActiveSummons(partySummons[i])) continue;

                showBossPveLines(
                    "TOUR DES INVOCATIONS",
                    "boss.coop.summon_turn." + std::to_string(i + 1),
                    {entry.label + " agissent à leur propre initiative : " + std::to_string(entry.totalScore) + "."}
                );
                const int before = boss.getHp();
                SummonCombatSystem::playPlayerSummonTurnsAgainstEntity(
                    partySummons[i],
                    boss,
                    random,
                    summonControlModes[i]
                );
                const int summonDamage = std::max(0, before - boss.getHp());
                owner->recordChallengeSummonAction(summonDamage);
                if (summonDamage > 0)
                {
                    contributions[i].supportActions++;
                    contributions[i].damageDealt += summonDamage;
                }
                maybeTriggerFireFlightCheatPurgeForParty(boss, party, fireFlightCheatPurgeAt75Done);
                maybeReviveHitogamiOnce(boss, hitogamiAlreadyRevived, difficulty);
                TurnManager::checkBossDecryption(boss);
            }
            else if (TurnOrder::isBoss(entry.id))
            {
                if (tryBossAttackPartySummon(boss, partySummons, contributions, random))
                {
                    boss.reduceUltimateCooldown();
                    continue;
                }

                Player* target = chooseAliveBossTarget(party, random, &contributions);
                if (target == nullptr) break;

                MessageScreen::show(
                    "TOUR DU BOSS",
                    "boss.coop.boss_turn",
                    {"Tour de " + boss.getName() + " : cible " + target->getName() + " — initiative " + std::to_string(entry.totalScore) + "."},
                    false
                );
                const int targetHpBeforeBossTurn = target->getHp();
                bool bossTurnFinished = false;
                while (!bossTurnFinished && !target->isDead())
                {
                    bossTurnFinished = TurnManager::playBossTurn(boss, *target, random);
                }

                for (std::size_t i = 0; i < party.size(); ++i)
                {
                    if (party[i] == target)
                    {
                        contributions[i].damageTaken += std::max(0, targetHpBeforeBossTurn - target->getHp());
                    }
                    if (party[i] != nullptr && party[i]->isDead())
                    {
                        contributions[i].wasDowned = true;
                    }
                }
                boss.reduceUltimateCooldown();
            }
        }
        ++round;
    }

    const int aliveAtCombatEnd = countAliveBossParty(party);
    int groupConsumables = 0;
    int groupSkills = 0;
    int groupNonBasic = 0;
    int groupBasic = 0;
    int groupDamageTaken = 0;
    int groupSummonActions = 0;
    int realPartySize = 0;
    for (Player* member : party)
    {
        if (member == nullptr) continue;
        ++realPartySize;
        groupConsumables += member->getChallengeCombatConsumablesUsed();
        groupSkills += member->getChallengeCombatSkillsUsed();
        groupNonBasic += member->getChallengeCombatNonBasicAttacksUsed();
        groupBasic += member->getChallengeCombatBasicAttacksUsed();
        groupDamageTaken += member->getChallengeCombatDamageTaken();
        groupSummonActions += member->getChallengeCombatSummonActions();
    }

    const bool groupVictory = boss.isDead() && aliveAtCombatEnd > 0;
    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* member = party[i];
        if (member == nullptr || !member->isChallengeCombatTrackingActive()) continue;
        member->applyChallengeCombatGroupSummary(
            realPartySize,
            aliveAtCombatEnd,
            groupConsumables,
            groupSkills,
            groupNonBasic,
            groupBasic,
            groupDamageTaken,
            groupSummonActions
        );
        const bool participated = i < contributions.size() && contributions[i].turnsTaken > 0;
        member->finishChallengeCombatTracking(groupVictory && participated, true, true, 1);
    }

    if (boss.isDead() && !boss.isIdentityRevealed())
    {
        boss.revealIdentity();
    }

    MessageScreen::show(
        "RÉSULTAT BOSS COOP",
        "combat.boss.coop.result",
        {
            "Résultat : " + std::string(boss.isDead() ? "victoire du groupe" : "défaite du groupe"),
            "Boss : " + boss.getName(),
            "État du boss : " + std::string(boss.isDead() ? "vaincu" : "encore debout"),
            "PV du boss : " + std::to_string(boss.getHp()) + "/" + std::to_string(boss.getMaxHp()),
            "Tours de boss enregistrés : " + std::to_string(bossCombatTurnCount),
            boss.getName() + " : " + std::string(boss.isDead() ? "vaincu" : "encore debout") + "."
        },
        false
    );

    for (Player* player : party)
    {
        if (player != nullptr)
        {
            player->clearBossEquipmentSeal();
        }
    }

    if (!boss.isDead())
    {
        MessageScreen::show(
            "DÉFAITE DU GROUPE",
            "combat.boss.coop.defeat",
            {"Le groupe a été brisé par le boss."},
            false
        );

        for (std::size_t partyIndex = 0; partyIndex < party.size(); ++partyIndex)
        {
            Player* player = party[partyIndex];
            if (player == nullptr) continue;
            player->recordDefeat();
            const int maxHpBeforeFight = partyIndex < initialMaxHp.size() && initialMaxHp[partyIndex] > 0
                ? initialMaxHp[partyIndex]
                : player->getMaxHp();
            applyBossVictoryConsequenceAfterDefeat(
                *player,
                boss,
                random,
                maxHpBeforeFight,
                player == &leader
            );
            if (player->isDead())
            {
                if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
                {
                    resolveBossLethalGroupDeathSave(*player, random);
                }
                else
                {
                    player->recordDeath();
                    player->reviveWithHealthPercentage(DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty));
                    showBossPveLines(
                        "RÉVEIL APRÈS DÉFAITE",
                        "boss.coop.defeat.revive",
                        {player->getName() + " est réveillé après la défaite du groupe. Mort comptabilisée."}
                    );
                }
            }
        }
        Console::waitForEnter();
        return;
    }

    CombatReward baseReward = CombatRewardSystem::calculateBossReward(
        boss,
        difficulty,
        std::max(0, initialHp.empty() ? 0 : initialHp[0] - leader.getHp()),
        bossCombatTurnCount
    );

    MessageScreen::show(
        "RÉCOMPENSES INDIVIDUELLES",
        "combat.boss.coop.individual_rewards",
        {"Chaque personnage reçoit selon sa participation, sa survie et son rôle dans le combat."},
        false
    );

    bool atLeastOnePartyMemberDownAtReward = false;
    for (Player* partyMember : party)
    {
        if (partyMember != nullptr && partyMember->isDead())
        {
            atLeastOnePartyMemberDownAtReward = true;
            break;
        }
    }

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        if (player == nullptr)
        {
            continue;
        }

        if (player->isDead())
        {
            if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
            {
                resolveBossLethalGroupDeathSave(*player, random);
                if (player->isDead())
                {
                    showBossPveLines(
                        "AUCUNE RÉCOMPENSE",
                        "boss.coop.reward.dead",
                        {player->getName() + " reste au sol : aucune récompense supplémentaire après sa chute."}
                    );
                    continue;
                }
            }
            else
            {
                player->recordDeath();
                player->reviveWithHealthPercentage(DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty));
                showBossPveLines(
                    "RÉVEIL APRÈS COMBAT",
                    "boss.coop.reward.revive",
                    {player->getName() + " est réveillé à la fin du combat. La mort est comptabilisée."}
                );
            }
        }

        CombatReward individualReward = buildIndividualBossCoopReward(
            baseReward,
            *player,
            leader,
            contributions[i]
        );

        std::vector<std::string> contributionLines;
        contributionLines.push_back(player->getName() + " :");
        if (contributions[i].turnsTaken <= 0)
        {
            contributionLines.push_back("Participation insuffisante : le registre ne donne presque rien.");
        }
        contributionLines.push_back(
            "Participation boss : tours " + std::to_string(contributions[i].turnsTaken)
            + ", dégâts " + std::to_string(contributions[i].damageDealt)
            + ", soins " + std::to_string(contributions[i].healingDone)
            + ", dégâts encaissés " + std::to_string(contributions[i].damageTaken) + "."
        );
        showBossPveLines("PARTICIPATION BOSS", "boss.coop.reward.contribution", contributionLines);

        CombatRewardSystem::displayReward(individualReward);
        CombatRewardSystem::giveRewardToPlayer(*player, individualReward);

        if (contributions[i].turnsTaken > 0)
        {
            player->recordVictory();
            player->recordBossKill();
            player->recordBossKillByName(boss.getName());
            grantBossVictoryTitles(*player, boss, true);
            if (!player->isDead() && atLeastOnePartyMemberDownAtReward && player->grantTitle("Dernier debout du groupe"))
            {
                showBossPveLines(
                    "TITRE DE COOP",
                    "boss.coop.title.last_standing",
                    {player->getName() + " obtient le titre : Dernier debout du groupe."}
                );
            }
            bool newEntityDetected = player->recordBossVictoryInRegistry(boss.getBossId());
            if (player->hasActiveCurse("boss_threshold_omen"))
            {
                player->getInventory().addMaterial(MaterialCatalog::createById("source_defeat_notice", 1));
                showBossPveLines(
                    "TRACE DE SEUIL",
                    "boss.coop.threshold_omen.source_defeated",
                    {
                        player->getName() + " reçoit une Note de source vaincue : la victoire de groupe compte comme vraie preuve.",
                        "L'église pourra l'utiliser seulement avec un diagnostic total de la trace concernée."
                    }
                );
            }
            if (newEntityDetected)
            {
                showBossPveLines(
                    "REGISTRE BOSS",
                    "boss.coop.reward.new_entity",
                    {
                        "Registre de " + player->getName() + " : cette première victoire stabilise la prochaine présence inconnue.",
                        "Une seule nouvelle entrée est ajoutée, sous l'identité ???."
                    }
                );
            }



            if (boss.getBossId() == 4 && player->hasActiveCurse("lyknir_prey_mark"))
            {
                int removedCurses = player->removeCursesLockedByBoss(4);
                if (removedCurses > 0)
                {
                    showBossPveLines(
                        "MARQUE BRISÉE",
                        "boss.coop.lyknir.curse_recovered",
                        {player->getName() + " perd la Marque de proie de Lyknir : la victoire de groupe compte vraiment."}
                    );
                }
            }

            if (boss.getBossId() == 2 && player->hasActiveCurse("zelef_black_blood_stain"))
            {
                int removedCurses = player->removeCursesLockedByBoss(2);
                if (removedCurses > 0)
                {
                    showBossPveLines(
                        "SANG NOIR APAISÉ",
                        "boss.coop.zelef.curse_recovered",
                        {player->getName() + " perd la Souillure de sang noir de Zelef : la source a été affrontée en vrai."}
                    );
                }
            }

            recoverConfiguredBossCurseAfterVictory(*player, boss);

            LootGenerator::giveDefeatedBossLoot(*player, boss, random, difficulty);
        }
        else
        {
            showBossPveLines(
                "AUCUN LOOT",
                "boss.coop.reward.no_loot",
                {"Aucun loot de boss : le personnage n'a pas réellement participé."}
            );
        }
    }
}
