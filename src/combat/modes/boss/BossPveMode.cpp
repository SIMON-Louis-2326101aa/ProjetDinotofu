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
#include "combat/loot/LootGenerator.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/summon/SummonControlMode.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "lore/LegendTriggerSystem.hpp"

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
            narration << "Trexof. Assassin, testeur, et probablement déjà en train de calculer où l'arène peut céder." << std::endl;
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

        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            screen.addLine("");
            screen.addLine("Difficulté Léthal détectée.");
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
        const BossPowerAnalysis& analysis
    )
    {
        TerminalInterface::renderMenuScreen(buildBossPowerAnalysisScreen(player, boss, difficulty, analysis), false);
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
                "Le boss final reconnaît ta victoire."
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

        MessageScreen::show(
            "CORROSION ANCRÉE",
            "boss.zelef.victory_penalty",
            {
                "Zelef a gagné. La corrosion ne quitte pas entièrement ton corps.",
                "PV maximum retenus par Zelef : " + std::to_string(lostMaxHp) + ".",
                "Effet visible sur le personnage : Corrosion présente.",
                "Si tu bats Zelef lors d'un prochain affrontement, tu pourras récupérer ce qu'il t'a pris."
            },
            false
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

            if (!player.isBossUnlocked(id))
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
                "FireFlight n'est pas un boss de passage.",
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
        supportScreen.addOption(1, "Utiliser une potion de soin sur un allié", "Le tour de ce personnage sera consommé.", true, "boss.coop.support.heal");
        supportScreen.addOption(0, "Jouer normalement", "Ne pas prendre le rôle de soigneur ce tour-ci.", true, "boss.coop.support.skip");

        int supportChoice = TerminalInterface::askMenuChoiceFromOptions(supportScreen, "Choisis 0 ou 1.");
        Console::clear();

        if (supportChoice == 0)
        {
            return false;
        }

        std::vector<Player*> targets;
        MenuScreen targetScreen("CIBLE DU SOIN", "boss.coop.support.target");
        targetScreen.addLine("Choisis l'allié à soigner ou réveiller.");
        targetScreen.addOption(0, "Annuler", "Revenir au tour normal.", true, "boss.coop.support.target.cancel");

        for (Player* ally : party)
        {
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                targets.push_back(ally);
                std::string label = ally->getName();
                if (ally->isDead())
                {
                    label += " [au sol]";
                }
                targetScreen.addOption(
                    static_cast<int>(targets.size()),
                    label,
                    std::to_string(ally->getHp()) + "/" + std::to_string(ally->getMaxHp()) + " PV",
                    true,
                    "boss.coop.support.target.select"
                );
            }
        }

        int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une cible affichée.");
        Console::clear();

        if (targetChoice == 0)
        {
            return false;
        }

        Player* target = targets[targetChoice - 1];

        MenuScreen potionScreen("POTION DE SOUTIEN", "boss.coop.support.potion");
        potionScreen.addLine("Choisis la potion à utiliser.");
        potionScreen.addOption(0, "Annuler", "Garder la potion pour plus tard.", true, "boss.coop.support.potion.cancel");
        for (int i = 0; i < static_cast<int>(potionIndices.size()); ++i)
        {
            Consumable potion = healer.getInventory().getConsumable(potionIndices[i]);
            potionScreen.addOption(
                i + 1,
                potion.getName(),
                "Soin " + std::to_string(potion.getPower()),
                true,
                "boss.coop.support.potion.select"
            );
        }

        int potionChoice = TerminalInterface::askMenuChoiceFromOptions(potionScreen, "Choisis une potion affichée.");
        Console::clear();

        if (potionChoice == 0)
        {
            return false;
        }

        int consumableIndex = potionIndices[potionChoice - 1];
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
        target->heal(potion.getPower());
        healingDone += std::max(0, target->getHp() - beforeHealHp);
        healer.markHealingThreat();

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
        lines.push_back("Léthal coop boss : " + player.getName() + " est au sol.");
        lines.push_back("3 pastilles vertes le ramènent. 3 rouges le rayent du registre, sauf intervention capable de briser le destin.");

        auto showSave = [&]()
        {
            showBossPveLines("SURVIE LÉTHAL COOP", "boss.coop.lethal.save", lines);
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

        player.recordDeath();
        lines.push_back(player.getName() + " reçoit trois pastilles rouges : mort définitive prévue, sauf exception divine/divination.");
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


    MenuScreen buildBossChoiceTypeScreen(const std::string& playerName, bool coop)
    {
        MenuScreen screen(coop ? "BOSS COOP" : "REGISTRE DES BOSS", coop ? "boss.coop.choice_type" : "boss.choice_type");
        screen.addSubtitle(playerName + ", choisis le type d'apparition du boss");

        if (coop)
        {
            screen.addLine("Le boss choisi doit être compatible avec tout le groupe.");
            screen.addOption(1, "Boss aléatoire compatible", "Le registre choisit parmi les boss débloqués par tous.", true, "boss.choice.random.coop");
            screen.addOption(2, "Choisir le boss", "Afficher les entités accessibles au groupe.", true, "boss.choice.manual.coop");
        }
        else
        {
            screen.addOption(1, "Boss aléatoire", "Le registre choisit une entité disponible.", true, "boss.choice.random");
            screen.addOption(2, "Choisir le boss", "Afficher les entités que tu peux stabiliser.", true, "boss.choice.manual");
        }

        return screen;
    }

    MenuScreen buildBossSelectionScreen(
        const std::vector<int>& visibleBossIds,
        const std::vector<int>& enabledBossIds,
        bool coop
    )
    {
        MenuScreen screen(coop ? "SÉLECTION DU BOSS COOP" : "SÉLECTION DU BOSS", coop ? "boss.coop.selection" : "boss.selection");
        screen.addLine(coop
            ? "Sélectionne l'entité que le groupe veut affronter."
            : "Sélectionne l'entité que tu veux exterminer.");

        for (int id : visibleBossIds)
        {
            bool enabled = std::find(enabledBossIds.begin(), enabledBossIds.end(), id) != enabledBossIds.end();
            std::string hint = BossCatalog::getRegistryHint(id);
            if (!enabled)
            {
                hint += coop
                    ? " L'un des registres du groupe bloque encore cette faille."
                    : " Cette faille est encore verrouillée ou trop instable.";
            }

            screen.addOption(
                id,
                BossCatalog::getRegistryDisplayName(id),
                hint,
                enabled,
                coop ? "boss.coop.select" : "boss.select"
            );
        }

        screen.addOption(0, "Retour", "Refermer le registre.", true, "boss.selection.back");
        screen.addFooterLine("Les boss vaincus récemment sont temporairement instables.");
        screen.addFooterLine("Leurs statistiques exactes restent inconnues avant l'arène.");

        return screen;
    }

}

void BossPveMode::run(
    Player& player1,
    Random& random,
    DifficultyMode difficulty
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

    int bossChoiceType = TerminalInterface::askMenuChoiceFromOptions(
        buildBossChoiceTypeScreen(player1.getName(), false),
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    std::vector<int> availableBossIds = player1.getAvailableBossIds();
    if (!hasAllInvitationsBeforeFireFlight(player1))
    {
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
                "Affronte deux autres boss différents pour laisser le registre respirer."
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
        Console::clear();

        bossChoice = TerminalInterface::askMenuChoiceFromOptions(
            buildBossSelectionScreen(player1.getUnlockedBossIds(), availableBossIds, false),
            "Veuillez entrer un identifiant de boss affiché."
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
    displayBossPowerAnalysis(simulatedPlayer, boss, difficulty, powerAnalysis);

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
        && (boss.getBossId() == 11 || boss.getBossId() == 16 || boss.getBossId() == 26 || boss.getBossId() == 27 || boss.getBossId() == 30))
    {
        std::vector<std::string> lethalLines;
        if (boss.getBossId() == 11)
        {
            lethalLines.push_back("L'Anomalie incline la tête.");
            lethalLines.push_back("Ah. C'était donc toi qui avais essayé de griffer la page depuis le mode Léthal.");
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
            lethalLines.push_back("Ah. C'est toi qui as tenté un code en Léthal. J'avais laissé une note pour ça.");
        }
        else
        {
            lethalLines.push_back("Moiran déroule un fil déjà taché par une tentative interdite.");
            lethalLines.push_back("Tu as voulu sortir du destin en Léthal. Le destin a simplement noté ton nom.");
        }
        showBossPveLines("TRACE LÉTHAL", "boss.pve.lethal_cheat_memory", lethalLines);
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
        player1.recordDefeat();
        player1.recordDeath();

        if (boss.getBossId() == 2)
        {
            applyZelefVictoryPenalty(player1, playerMaxHpBeforeBossFight);
        }

        if (boss.getBossId() == 5)
        {
            applyGrinkaVictoryPenalty(player1, random);
        }

        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

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
            player1.recordDefeat();
            player1.recordDeath();

            if (DifficultyRules::isPermanentDeath(difficulty))
            {
                DeathPenaltySystem::displayLethalDeathCorruption();
                return;
            }

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

        player1.recordVictory();
        player1.recordBossKill();

        bool newEntityDetected = player1.recordBossVictoryInRegistry(boss.getBossId());

        if (newEntityDetected)
        {
            MessageScreen::show(
                "REGISTRE BOSS",
                "boss.registry.new_entity",
                {
                    "Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale.",
                    "Nom : ???",
                    "Statut : brouillé.",
                    "Le registre a ajouté une entrée, mais refuse encore d'en révéler l'identité."
                },
                false
            );
        }

        if (boss.getBossId() == 2 && player1.hasZelefCorrosionPresent())
        {
            int recovered = player1.getZelefMaxHpStolen();
            player1.restoreZelefCorrosionLoss();
            MessageScreen::show(
                "CORROSION RENDUE",
                "boss.zelef.corrosion_recovered",
                {
                    "Tu fais face au sang noir qui t'avait marqué.",
                    "Rends-moi ce que tu m'as pris il y a longtemps.",
                    "PV maximum récupérés : " + std::to_string(recovered) + ".",
                    "Effet retiré : Corrosion présente."
                },
                false
            );
        }

        if (boss.getBossId() == 5 && player1.hasGrinkaBossTheftPresent())
        {
            player1.restoreGrinkaBossTheft();
            MessageScreen::show(
                "BUTIN RÉCUPÉRÉ",
                "boss.grinka.theft_recovered",
                {
                    "Grinka lâche un sac de butin avec un sourire beaucoup moins royal.",
                    "Rends-moi ce que tu m'as pris il y a longtemps.",
                    "Les biens volés par ce boss reviennent dans ton inventaire.",
                    "Effet retiré : Volé par un boss."
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
    DifficultyMode difficulty
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

    int bossChoiceType = TerminalInterface::askMenuChoiceFromOptions(
        buildBossChoiceTypeScreen(leader.getName(), true),
        "Veuillez entrer 1 ou 2."
    );

    std::vector<int> leaderAvailableBossIds = leader.getAvailableBossIds();
    if (!hasAllInvitationsBeforeFireFlight(leader))
    {
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
        Console::clear();
        bossChoice = TerminalInterface::askMenuChoiceFromOptions(
            buildBossSelectionScreen(leader.getUnlockedBossIds(), coopAvailableBossIds, true),
            "Veuillez entrer un identifiant de boss affiché."
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
    std::vector<BossCoopContribution> contributions(party.size());
    std::vector<std::vector<Summon>> partySummons(party.size());
    std::vector<SummonControlMode> summonControlModes(party.size(), SummonControlMode::Automatic);

    for (std::size_t partyIndex = 0; partyIndex < party.size(); ++partyIndex)
    {
        Player* player = party[partyIndex];
        if (player == nullptr)
        {
            initialHp.push_back(0);
            continue;
        }

        classGuards.push_back(std::make_unique<TemporaryClassGuard>(*player));
        PlayerClass evolvedClass = ClassCatalog::createEvolvedClassFromClass(player->getType());
        player->applyClass(evolvedClass);
        classGuards.back()->markActive();
        initialHp.push_back(player->getHp());
        partySummons[partyIndex] = SummonCombatSystem::createInitialSummonsFor(*player);
    }

    Console::clear();

    Player simulatedLeader = leader;
    simulatedLeader.applyClass(ClassCatalog::createEvolvedClassFromClass(leader.getType()));
    scaleBossForCoop(boss, static_cast<int>(party.size()));
    BossPowerAnalysis powerAnalysis = analyzeBossPower(simulatedLeader, boss, difficulty);
    displayBossPowerAnalysis(simulatedLeader, boss, difficulty, powerAnalysis);

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

    while (countAliveBossParty(party) > 0 && !boss.isDead())
    {
        MessageScreen::show(
            "TOUR DE GROUPE BOSS",
            "boss.coop.round." + std::to_string(round),
            {
                "Tour " + std::to_string(round) + ".",
                boss.getName() + " : " + std::to_string(boss.getHp()) + "/" + std::to_string(boss.getMaxHp()) + " PV."
            },
            false
        );
        displayBossCoopPartyStatus(party, extractBossDownedFlags(contributions));

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead() || boss.isDead())
            {
                continue;
            }

            MessageScreen::show(
                "TOUR JOUEUR",
                "boss.coop.player_turn",
                {"Tour de " + player->getName() + " [J" + std::to_string(i + 1) + "]."},
                false
            );

            int healingDoneThisTurn = 0;
            int bossHpBeforeTurn = boss.getHp();
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
                if (!boss.isDead() && SummonCombatSystem::hasActiveSummons(partySummons[i]))
                {
                    int bossHpBeforeSummons = boss.getHp();
                    SummonCombatSystem::playPlayerSummonTurnsAgainstEntity(
                        partySummons[i],
                        boss,
                        random,
                        summonControlModes[i]
                    );
                    if (bossHpBeforeSummons > boss.getHp())
                    {
                        contributions[i].supportActions++;
                    }
                }

                contributions[i].turnsTaken++;
                contributions[i].damageDealt += std::max(0, bossHpBeforeTurn - boss.getHp());
                contributions[i].healingDone += healingDoneThisTurn;
                if (healingDoneThisTurn > 0) contributions[i].supportActions++;
                ++bossCombatTurnCount;
                maybeReviveHitogamiOnce(boss, hitogamiAlreadyRevived, difficulty);
                TurnManager::checkBossDecryption(boss);
            }
        }

        if (boss.isDead() || countAliveBossParty(party) == 0)
        {
            break;
        }

        if (tryBossAttackPartySummon(boss, partySummons, contributions, random))
        {
            boss.reduceUltimateCooldown();
            ++round;
            continue;
        }

        Player* target = chooseAliveBossTarget(party, random, &contributions);
        if (target == nullptr)
        {
            break;
        }

        MessageScreen::show(
            "TOUR DU BOSS",
            "boss.coop.boss_turn",
            {"Tour de " + boss.getName() + " : cible " + target->getName() + "."},
            false
        );
        int targetHpBeforeBossTurn = target->getHp();
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
        ++round;
    }

    if (boss.isDead() && !boss.isIdentityRevealed())
    {
        boss.revealIdentity();
    }

    MessageScreen::show(
        "RÉSULTAT BOSS COOP",
        "combat.boss.coop.result",
        {
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

        for (Player* player : party)
        {
            if (player == nullptr) continue;
            player->recordDefeat();

            if (player->isDead())
            {
                if (DifficultyRules::isPermanentDeath(difficulty))
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

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        if (player == nullptr)
        {
            continue;
        }

        if (player->isDead())
        {
            if (DifficultyRules::isPermanentDeath(difficulty))
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
            bool newEntityDetected = player->recordBossVictoryInRegistry(boss.getBossId());
            if (newEntityDetected)
            {
                showBossPveLines(
                    "REGISTRE BOSS",
                    "boss.coop.reward.new_entity",
                    {"Registre de " + player->getName() + " : une nouvelle entité est devenue détectable."}
                );
            }

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
