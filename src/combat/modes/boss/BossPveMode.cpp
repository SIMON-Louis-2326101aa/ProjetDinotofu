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

#include <algorithm>
#include <cctype>
#include <iostream>
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

    // EN: displayFireFlightSpecialCharacterDialogue declares or implements a focused behavior used by this module.
    // FR: displayFireFlightSpecialCharacterDialogue déclare ou implémente un comportement précis utilisé par ce module.
    void displayFireFlightSpecialCharacterDialogue(const Player& player)
    {
        std::string name = toLowerAscii(player.getName());
        bool altered = player.isAlteredByCheats();

        std::cout << std::endl;
        std::cout << "FireFlight baisse légèrement la voix." << std::endl;

        if (isAnyName(name, {"louis"}))
        {
            std::cout << "Louis... là, ce n'est plus seulement un personnage." << std::endl;
            std::cout << "C'est la petite signature du créateur qui descend dans sa propre arène." << std::endl;
            std::cout << "Conseil du créateur : certains secrets ne se donnent pas dans les menus. Ils apparaissent quand tu reviens lire les registres après une victoire qui semblait inutile." << std::endl;
        }
        else if (isAnyName(name, {"fireflight", "fire flight"}))
        {
            std::cout << "FireFlight contre FireFlight..." << std::endl;
            std::cout << "Même le monde hésite à savoir lequel de nous deux est l'avatar et lequel tient vraiment le clavier." << std::endl;
            std::cout << "Conseil du créateur : quand une option paraît trop propre pour être vraie, cherche la ligne qui n'a pas été affichée." << std::endl;
        }
        else if (name == "hazak")
        {
            std::cout << "Hazak. Toujours l'ombre, toujours la lame, toujours cette façon de croire que gagner suffit." << std::endl;
            std::cout << "Je sais d'où tu viens. Une table, des dés, une histoire qui a refusé de rester dans un seul jeu." << std::endl;
            std::cout << "Indice : les morts récents ne sont pas toujours perdus pour un assassin qui comprend les ombres." << std::endl;
        }
        else if (name == "fail")
        {
            std::cout << "Fail... même ici, tu cherches sûrement comment transformer le problème en expérience dangereuse." << std::endl;
            std::cout << "Je préfère te prévenir : certaines instabilités deviennent plus fortes quand tu les traites comme des jouets." << std::endl;
            std::cout << "Indice : les noyaux instables n'aiment pas être conservés trop proprement." << std::endl;
        }
        else if (name == "aoi")
        {
            std::cout << "Aoi. Tu n'as pas besoin de parler fort pour que tes flammes soient entendues." << std::endl;
            std::cout << "Protège ton incantation. Le monde respecte parfois plus la patience que la puissance." << std::endl;
            std::cout << "Indice : une braise kitsune devient plus intéressante quand elle est gardée plutôt que vendue." << std::endl;
        }
        else if (name == "kanadé" || name == "kanade")
        {
            std::cout << "Kanadé. Je sais, ce combat t'énerve déjà." << std::endl;
            std::cout << "Mais même ta rage peut devenir une constellation si tu la laisses choisir sa forme." << std::endl;
            std::cout << "Indice : les effets zodiacaux aimeront rarement la stabilité parfaite." << std::endl;
        }
        else if (name == "skuro")
        {
            std::cout << "Skuro. Ta lame cherche toujours la matière avant la raison." << std::endl;
            std::cout << "Je ne vais pas te demander d'être doux. Juste de comprendre ce que tu découpes." << std::endl;
            std::cout << "Indice : certaines armures ne se brisent pas en tapant plus fort, mais en frappant au bon tour." << std::endl;
        }
        else if (name == "sanctus")
        {
            std::cout << "Sanctus. Toi, tu protèges même quand le monde préférerait te voir tomber." << std::endl;
            std::cout << "Garde cette lumière. Mais n'oublie pas qu'une protection devient une prison si tu ne choisis jamais d'avancer." << std::endl;
            std::cout << "Indice : Provocation et Posture de défense sont plus liées que beaucoup d'ennemis ne veulent l'admettre." << std::endl;
        }
        else if (name == "trexof")
        {
            std::cout << "Trexof. Assassin, testeur, et probablement déjà en train de calculer où l'arène peut céder." << std::endl;
            std::cout << "Je te connais assez pour savoir que tu vas chercher la faille propre." << std::endl;
            std::cout << "Indice : les boss qui semblent punir une habitude révèlent souvent leur contre si tu changes de rythme deux tours avant l'ulti." << std::endl;
        }
        else if (name == "mattzelda")
        {
            std::cout << "Mattzelda. Même dans un combat final, tu réussirais presque à faire croire que tout ça est une blague." << std::endl;
            std::cout << "Mais je sais ce que ce nom représente : un pote que le jeu n'a pas voulu transformer en simple statistique." << std::endl;
            std::cout << "Indice : les colosses gagnent parfois plus en encaissant le bon coup qu'en cherchant le plus gros chiffre." << std::endl;
        }
        else if (name == "hestia")
        {
            std::cout << "Hestia..." << std::endl;
            std::cout << "Je vais être honnête : je n'ai pas vraiment envie de te voir tomber." << std::endl;
            std::cout << "Même derrière les dés, même derrière les règles, même derrière la création... il y a des personnages qu'on protège un peu trop." << std::endl;
            std::cout << "Indice : ton dôme n'est pas seulement une défense. Bien placé, il peut transformer un tour perdu en tour survivant." << std::endl;
        }
        else
        {
            std::cout << "Même si ton nom n'a pas de note personnelle dans mes fichiers, ta trace existe maintenant." << std::endl;
            std::cout << "Indice : un boss vaincu ne laisse pas seulement du butin. Certaines portes ne s'ouvrent qu'après sa chute." << std::endl;
        }

        if (altered)
        {
            std::cout << std::endl;
            std::cout << "Puis son regard accroche une cicatrice dans tes données." << std::endl;

            if (isAnyName(name, {"louis"}))
            {
                std::cout << "Louis... tricher avec ton propre reflet, c'est presque drôle. Presque." << std::endl;
                std::cout << "Tu voulais aider la création, ou seulement éviter qu'elle te réponde ?" << std::endl;
            }
            else if (isAnyName(name, {"fireflight", "fire flight"}))
            {
                std::cout << "FireFlight altéré. Voilà donc ce que ça donne quand le créateur laisse une commande ouverte dans sa propre poche." << std::endl;
                std::cout << "Je ne vais pas effacer la faute. Je vais la rendre intéressante." << std::endl;
            }
            else if (name == "hazak")
            {
                std::cout << "Hazak, tu sais mieux que personne qu'un contrat avec l'ombre finit toujours par réclamer une ligne en plus." << std::endl;
                std::cout << "Tes raccourcis ne t'ont pas rendu plus discret. Ils ont rendu ton ombre plus bavarde." << std::endl;
            }
            else if (name == "fail")
            {
                std::cout << "Fail, évidemment que tu as touché à l'interdit. Le problème, c'est que cette expérience-là a touché en retour." << std::endl;
                std::cout << "Ton altération n'est pas une réussite. C'est un résultat qui n'a pas encore explosé." << std::endl;
            }
            else if (name == "aoi")
            {
                std::cout << "Aoi, tes flammes n'aiment pas cette trace dans tes données." << std::endl;
                std::cout << "Une incantation tremble quand la main qui la porte a déjà forcé une règle." << std::endl;
            }
            else if (name == "kanadé" || name == "kanade")
            {
                std::cout << "Kanadé, même les étoiles refusent de servir de prétexte à une triche." << std::endl;
                std::cout << "Ta rage est légitime. Ton altération, elle, devra être assumée." << std::endl;
            }
            else if (name == "skuro")
            {
                std::cout << "Skuro, ce n'est pas la première fois qu'une lame veut couper la règle au lieu de l'ennemi." << std::endl;
                std::cout << "Mais ici, même la violence garde une mémoire." << std::endl;
            }
            else if (name == "sanctus")
            {
                std::cout << "Sanctus, une protection obtenue par raccourci protège moins bien l'âme que le corps." << std::endl;
                std::cout << "Si ta lumière tremble, ce n'est pas parce qu'elle est faible. C'est parce qu'elle sait." << std::endl;
            }
            else if (name == "trexof")
            {
                std::cout << "Trexof, trouver une faille n'est pas pareil que survivre à son retour." << std::endl;
                std::cout << "Merci pour le test. Maintenant, survis au correctif." << std::endl;
            }
            else if (name == "mattzelda")
            {
                std::cout << "Mattzelda, même tes blagues ont l'air moins légères quand le personnage porte une trace altérée." << std::endl;
                std::cout << "Mais je te laisse une chance : fais-en une histoire, pas une excuse." << std::endl;
            }
            else if (name == "hestia")
            {
                std::cout << "Hestia... je voulais te protéger, pas te voir devenir une donnée altérée." << std::endl;
                std::cout << "Alors écoute bien : survivre compte plus que prouver quoi que ce soit. Même ici." << std::endl;
            }
            else
            {
                std::cout << "Tu as utilisé des raccourcis. Je ne vais pas faire semblant de ne pas l'avoir vu." << std::endl;
                std::cout << "Ce n'est pas une condamnation. C'est une annotation. Et les annotations, ici, peuvent mordre." << std::endl;
            }
        }

        std::cout << std::endl;
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
        std::cout << std::endl;
    }

    // EN: runFireFlightFinalTest declares or implements a focused behavior used by this module.
    // FR: runFireFlightFinalTest déclare ou implémente un comportement précis utilisé par ce module.
    bool runFireFlightFinalTest(Player& player, Random& random)
    {
        std::cout << std::endl;
        std::cout << "========== TEST FINAL DU CRÉATEUR ==========" << std::endl;
        std::cout << "FireFlight tombe. Puis le monde refuse de valider la victoire tout de suite." << std::endl;
        std::cout << "Tous les personnages créés par ce compte sont appelés comme des silhouettes figées." << std::endl;
        std::cout << "Même ton propre reflet apparaît de l'autre côté du champ de bataille." << std::endl;
        std::cout << "Les alliés non invoqués restent suspendus dans le temps, inanimés." << std::endl;
        std::cout << std::endl;

        for (int round = 1; round <= 3 && !player.isDead(); ++round)
        {
            std::cout << "--- Cycle final " << round << "/3 ---" << std::endl;
            std::cout << "========== 4E_MUR::INTERFACE_BRISEE ==========" << std::endl;
            std::cout << "1 : Sentence céleste / Chasse sans lune / Dette royale" << std::endl;
            std::cout << "2 : Couronne des ténèbres / Souffle de l'Origine / La forêt se souvient" << std::endl;
            std::cout << "3 : Erreur critique : le récit se réécrit" << std::endl;
            std::cout << "4 : Inversion des probabilités / Verdict des actes répétés" << std::endl;
            std::cout << "5 : Sursis d'urgence : survivre trois tours" << std::endl;
            if (player.isAlteredByCheats())
            {
                std::cout << "? : idontwanttodie" << std::endl;
                std::cout << "? : whereismyplotarmor" << std::endl;
                std::cout << "? : talktothedev" << std::endl;
                std::cout << "Les anciens codes apparaissent dans l'interface comme des cicatrices, pas comme des solutions." << std::endl;
            }

            int damage = 18 + random.between(8, 22) + player.getLevel() / 2;
            if (round == 2) damage += 8;
            if (round == 3) damage += 14;

            if (player.isInDefensePosture())
            {
                damage = damage * 75 / 100;
                std::cout << "Ta posture réduit un peu le chaos, sans pouvoir l'annuler." << std::endl;
            }

            player.takeDamage(damage);
            std::cout << player.getName() << " subit " << damage << " dégâts de test final." << std::endl;
            std::cout << player.getName() << " possède maintenant " << player.getHp() << "/" << player.getMaxHp() << " PV." << std::endl;
            std::cout << std::endl;
        }

        if (player.isDead())
        {
            std::cout << "Le test final s'arrête. Le monde n'a pas accepté ta version de la fin." << std::endl;
            std::cout << "==============================================" << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::cout << "FireFlight sourit." << std::endl;
        std::cout << "Cette fois, je n'ai plus de test à ajouter." << std::endl;
        std::cout << "Le boss final reconnaît ta victoire." << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: applyGrinkaVictoryPenalty declares or implements a focused behavior used by this module.
    // FR: applyGrinkaVictoryPenalty déclare ou implémente un comportement précis utilisé par ce module.
    void applyGrinkaVictoryPenalty(Player& player, Random& random)
    {
        std::cout << "========== DETTE ROYALE ENCAISSÉE ==========" << std::endl;
        std::cout << "Grinka a gagné. Ses taxes ne sont plus une menace de combat : elles deviennent réelles." << std::endl;

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
            std::cout << "Or confisqué : " << stolenGold << " pièce(s)." << std::endl;
        }
        else
        {
            int xpLoss = 20 + player.getLevel() * 8;
            player.loseExperience(xpLoss);
            std::cout << "Tu n'avais pas d'or sur toi." << std::endl;
            std::cout << "Grinka transforme la dette en humiliation administrative : perte de "
                      << xpLoss << " XP." << std::endl;
        }

        if (!player.hasEquipmentProtection() && player.hasEquippedWeapon() && random.between(1, 100) <= 25)
        {
            Weapon stolenWeapon = player.getEquippedWeapon();
            if (player.destroyEquippedWeapon())
            {
                player.recordGrinkaStolenWeapon(stolenWeapon);
                std::cout << "Un collecteur gobelin repart avec ton arme équipée." << std::endl;
            }
        }

        if (!player.hasEquipmentProtection() && player.hasEquippedArmor() && random.between(1, 100) <= 18)
        {
            Armor stolenArmor = player.getEquippedArmor();
            if (player.destroyEquippedArmor())
            {
                player.recordGrinkaStolenArmor(stolenArmor);
                std::cout << "Un autre collecteur arrache une pièce majeure de ton armure équipée." << std::endl;
            }
        }

        std::cout << "Aucune dette ne descend sous zéro : Grinka vole ce qui existe, pas ce qui n'existe pas." << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << std::endl;
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

        std::cout << "========== CORROSION ANCRÉE ==========" << std::endl;
        std::cout << "Zelef a gagné. La corrosion ne quitte pas entièrement ton corps." << std::endl;
        std::cout << "PV maximum retenus par Zelef : " << lostMaxHp << std::endl;
        std::cout << "Effet visible sur le personnage : Corrosion présente." << std::endl;
        std::cout << "Si tu bats Zelef lors d'un prochain affrontement, tu pourras récupérer ce qu'il t'a pris." << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << std::endl;
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

        std::cout << std::endl;
        std::cout << "Hitogami tombe à genoux." << std::endl;
        std::cout << "Pendant une seconde, le combat semble terminé." << std::endl;
        std::cout << "Puis il respire encore." << std::endl;
        std::cout << "Tu n'es pas spécial parce que tu tombes." << std::endl;
        std::cout << "Tu es spécial parce que tu te relèves." << std::endl;
        std::cout << "L'écho d'Hitogami se relève avec " << boss.getHp() << "/" << boss.getMaxHp() << " PV." << std::endl;
        std::cout << std::endl;
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
        std::cout << "L'entrée de FireFlight apparaît dans le registre." << std::endl;
        std::cout << "Mais la porte ne s'ouvre pas." << std::endl;
        std::cout << std::endl;
        std::cout << "Une phrase est gravée dans l'air :" << std::endl;
        std::cout << "Prouve ta valeur devant chaque variation majeure. Rassemble toutes les invitations." << std::endl;
        std::cout << std::endl;
        std::cout << "FireFlight n'est pas un boss de passage." << std::endl;
        std::cout << "C'est le dernier test du jeu actuel." << std::endl;
        std::cout << std::endl;
    }

    // EN: displayFireFlightFirstEntrance declares or implements a focused behavior used by this module.
    // FR: displayFireFlightFirstEntrance déclare ou implémente un comportement précis utilisé par ce module.
    void displayFireFlightFirstEntrance(const Player& player)
    {
        std::cout << "Le registre cesse de faire semblant d'être neutre." << std::endl;
        std::cout << "Toutes les lettres récoltées brûlent en même temps dans ton inventaire." << std::endl;
        std::cout << std::endl;
        std::cout << "Une silhouette attend au centre d'une arène qui ressemble à un menu cassé." << std::endl;
        std::cout << "FireFlight te regarde, puis regarde au-dessus de toi, comme s'il voyait le code du monde." << std::endl;
        std::cout << std::endl;
        std::cout << "Tu as battu assez de règles pour mériter d'en rencontrer une vraie." << std::endl;
        std::cout << "Bienvenue dans le test final, " << player.getName() << "." << std::endl;
        displayFireFlightSpecialCharacterDialogue(player);
        std::cout << std::endl;
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

        std::cout << "Stabilisation coop : le boss adapte son enveloppe." << std::endl;
        std::cout << "PV x" << hpPercent << "% | puissance x" << damagePercent << "% selon le nombre de joueurs réels." << std::endl;
        std::cout << "Le sceau refuse qu'un adversaire majeur perde sa forme face au nombre." << std::endl;
        std::cout << std::endl;
    }

    void displayBossCoopPartyStatus(const std::vector<Player*>& party, const std::vector<bool>& wasDowned)
    {
        std::cout << "========== ÉTAT DU GROUPE ==========" << std::endl;
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr)
            {
                continue;
            }

            std::cout << "J" << (i + 1) << " ["
                      << CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(i))
                      << "] - " << player->getName()
                      << " : " << player->getHp() << "/" << player->getMaxHp() << " PV";
            if (player->isDead())
            {
                std::cout << " [au sol]";
            }
            else if (i < wasDowned.size() && wasDowned[i])
            {
                std::cout << " [a déjà chuté]";
            }
            std::cout << std::endl;
        }
        std::cout << "====================================" << std::endl;
        std::cout << std::endl;
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
            std::cout << "Cette potion n'est plus disponible." << std::endl;
            std::cout << std::endl;
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

        std::cout << healer.getName() << " devient soigneur ce tour-ci et utilise "
                  << potion.getName() << " sur " << target->getName() << "." << std::endl;
        if (revivedTarget)
        {
            std::cout << target->getName() << " est réveillé à 1 PV avant de recevoir le soin." << std::endl;
        }
        std::cout << target->getName() << " possède maintenant "
                  << target->getHp() << "/" << target->getMaxHp() << " PV." << std::endl;
        std::cout << "Le tour de " << healer.getName() << " est consommé." << std::endl;
        std::cout << std::endl;
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

        std::cout << "Léthal coop boss : " << player.getName() << " est au sol." << std::endl;
        std::cout << "3 pastilles vertes le ramènent. 3 rouges le rayent du registre, sauf intervention capable de briser le destin." << std::endl;
        std::cout << std::endl;

        while (green < 3 && red < 3)
        {
            int roll = random.between(1, 20);
            std::cout << "Dé de survie : " << roll << std::endl;

            if (roll == 20)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                std::cout << "20 naturel : " << player.getName() << " se relève immédiatement à 1 PV et pourra rejouer." << std::endl;
                std::cout << std::endl;
                return;
            }

            if (roll == 1)
            {
                red += 2;
                std::cout << "1 naturel : deux pastilles rouges apparaissent d'un coup." << std::endl;
            }
            else if (roll >= 11)
            {
                ++green;
                std::cout << "Pastille verte : " << green << "/3." << std::endl;
            }
            else
            {
                ++red;
                std::cout << "Pastille rouge : " << red << "/3." << std::endl;
            }
        }

        if (green >= 3)
        {
            player.reviveWithHealthPercentage(1);
            if (player.getHp() <= 0) player.heal(1);
            std::cout << player.getName() << " revient à 1 PV. La mort n'est pas comptée." << std::endl;
            std::cout << std::endl;
            return;
        }

        player.recordDeath();
        std::cout << player.getName() << " reçoit trois pastilles rouges : mort définitive prévue, sauf exception divine/divination." << std::endl;
        std::cout << std::endl;
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
        std::cout << "Ce boss ne peut pas être stabilisé en coop." << std::endl;
        std::cout << "Tous les joueurs doivent l'avoir débloqué." << std::endl;
        std::cout << "Joueurs qui bloquent l'accès :" << std::endl;
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || !player->isBossUnlocked(bossId))
            {
                std::cout << "- J" << (i + 1) << " : "
                          << (player == nullptr ? "slot vide" : player->getName())
                          << std::endl;
            }
        }
        std::cout << std::endl;
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

        std::cout << boss.getName()
                  << " sent le lien d'invocation et tente de couper le soutien avant le joueur."
                  << std::endl;

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
    std::cout << "Vous sentez une aura maléfique autour de vous." << std::endl;
    Console::pauseSeconds(2);

    std::cout << "Par chance, votre âme est encore pure." << std::endl;
    std::cout << "À mesure que vous avancez, votre force change, comme si l'arène refusait de vous laisser mourir trop vite." << std::endl;
    std::cout << std::endl;

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
        std::cout << "Aucune entité stable n'est disponible pour le moment." << std::endl;
        std::cout << "Affronte deux autres boss différents pour laisser le registre respirer." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
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
            std::cout << "Cette entité n'est pas encore détectée dans le registre." << std::endl;
            std::cout << "Un boss vaincu peut provoquer l'apparition d'une nouvelle variation anormale." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            return;
        }

        if (player1.isBossRecentlyDefeated(bossChoice))
        {
            std::cout << "Cette entité vient d'être affrontée." << std::endl;
            std::cout << "Le registre refuse de la stabiliser tout de suite : affronte deux autres boss avant." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
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
        std::cout << std::endl;
        std::cout << "Le registre referme la faille avant qu'elle ne devienne une tombe." << std::endl;
        std::cout << "L'entité reste quelque part dans le brouillard, en attendant que tu sois vraiment prêt." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        return;
    }

    Console::clear();

    player1.applyClass(evolvedClass);
    temporaryClassGuard.markActive();

    std::cout << player1.getName() << ", ta classe évolue temporairement en : " << player1.getType() << "." << std::endl;
    std::cout << "Tes PV et tes objets ont été renforcés pour ce combat." << std::endl;
    std::cout << "Après le combat, cette forme devra se dissiper et ta classe réelle reviendra." << std::endl;
    std::cout << std::endl;

    player1.displayStats();

    Console::pauseSeconds(3);

    if (boss.getBossId() == 27)
    {
        boss.revealIdentity();
        displayFireFlightFirstEntrance(player1);
        Console::pauseSeconds(3);
    }

    std::cout << "Préparation du boss..." << std::endl;
    Console::pauseSeconds(2);

    std::cout << boss.getName() << " est entré dans l'arène." << std::endl;
    std::cout << boss.getName() << " est de type : " << boss.getType() << "." << std::endl;
    std::cout << "Fais attention : ce combat pourrait vraiment te coûter cher." << std::endl;

    if (player1.getLethalCheatAttemptCount() > 0
        && (boss.getBossId() == 11 || boss.getBossId() == 16 || boss.getBossId() == 26 || boss.getBossId() == 27 || boss.getBossId() == 30))
    {
        std::cout << std::endl;
        if (boss.getBossId() == 11)
        {
            std::cout << "L'Anomalie incline la tête." << std::endl;
            std::cout << "Ah. C'était donc toi qui avais essayé de griffer la page depuis le mode Léthal." << std::endl;
        }
        else if (boss.getBossId() == 16)
        {
            std::cout << "Lexior ouvre un dossier déjà marqué." << std::endl;
            std::cout << "Tentative risquée précédemment constatée. Le verdict s'en souvient." << std::endl;
        }
        else if (boss.getBossId() == 26)
        {
            std::cout << "L'écho d'Obérion te reconnaît sans colère." << std::endl;
            std::cout << "Tu as déjà tenté de séparer la vie de ses conséquences." << std::endl;
        }
        else if (boss.getBossId() == 27)
        {
            std::cout << "FireFlight soupire comme quelqu'un qui relit un vieux log." << std::endl;
            std::cout << "Ah. C'est toi qui as tenté un code en Léthal. J'avais laissé une note pour ça." << std::endl;
        }
        else
        {
            std::cout << "Moiran déroule un fil déjà taché par une tentative interdite." << std::endl;
            std::cout << "Tu as voulu sortir du destin en Léthal. Le destin a simplement noté ton nom." << std::endl;
        }
    }

    std::cout << std::endl;

    Console::pauseSeconds(3);

    int turn = random.chooseFirstTurn();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSeconds(2);
    std::cout << "Le combat contre " << boss.getName() << " commence maintenant." << std::endl;
    std::cout << std::endl;

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
            std::cout << std::endl;
            std::cout << "Tu fais face au sang noir qui t'avait marqué." << std::endl;
            std::cout << "Rends-moi ce que tu m'as pris il y a longtemps." << std::endl;
            std::cout << "PV maximum récupérés : " << recovered << std::endl;
            std::cout << "Effet retiré : Corrosion présente." << std::endl;
        }

        if (boss.getBossId() == 5 && player1.hasGrinkaBossTheftPresent())
        {
            player1.restoreGrinkaBossTheft();
            std::cout << std::endl;
            std::cout << "Grinka lâche un sac de butin avec un sourire beaucoup moins royal." << std::endl;
            std::cout << "Rends-moi ce que tu m'as pris il y a longtemps." << std::endl;
            std::cout << "Les biens volés par ce boss reviennent dans ton inventaire." << std::endl;
            std::cout << "Effet retiré : Volé par un boss." << std::endl;
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
        std::cout << "Aucun boss commun n'est disponible pour ce groupe." << std::endl;
        std::cout << "Un joueur secondaire bloque probablement l'accès à tous les boss accessibles au joueur 1." << std::endl;
        std::cout << "Faites progresser les registres de boss des personnages secondaires, ou lancez un boss déjà commun." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
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
            std::cout << "Cette entité n'est pas encore détectée dans le registre du joueur principal." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            return;
        }

        if (leader.isBossRecentlyDefeated(bossChoice))
        {
            std::cout << "Cette entité vient d'être affrontée par le joueur principal." << std::endl;
            std::cout << "Le registre refuse de la stabiliser tout de suite." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
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

    std::cout << "Analyse coop : le boss reste calibré depuis le joueur principal, mais il pourra cibler n'importe quel allié vivant." << std::endl;
    std::cout << "Si un joueur secondaire n'aide presque pas, son registre ne progressera pas vraiment." << std::endl;
    std::cout << std::endl;

    if (!askBossFightConfirmation(powerAnalysis))
    {
        std::cout << "Le groupe recule avant que la faille ne se referme sur lui." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        return;
    }

    Console::clear();

    if (boss.getBossId() == 27)
    {
        boss.revealIdentity();
        displayFireFlightFirstEntrance(leader);
        Console::pauseSeconds(3);
    }

    std::cout << "Préparation du boss coop..." << std::endl;
    Console::pauseSeconds(1);
    std::cout << boss.getName() << " est entré dans l'arène." << std::endl;
    std::cout << boss.getName() << " est de type : " << boss.getType() << "." << std::endl;
    std::cout << "Chaque joueur a temporairement évolué selon sa classe, puis reviendra à son état réel après le combat." << std::endl;
    std::cout << "Les invocations actives peuvent maintenant aider en boss coop, mais restent en dernière priorité de placement." << std::endl;
    std::cout << std::endl;

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
        std::cout << "========== TOUR DE GROUPE BOSS " << round << " ==========" << std::endl;
        std::cout << boss.getName() << " : " << boss.getHp() << "/" << boss.getMaxHp() << " PV." << std::endl;
        std::cout << std::endl;
        displayBossCoopPartyStatus(party, extractBossDownedFlags(contributions));

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead() || boss.isDead())
            {
                continue;
            }

            std::cout << "Tour de " << player->getName() << " [J" << (i + 1) << "]." << std::endl;
            std::cout << std::endl;

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

        std::cout << "Tour de " << boss.getName() << " : cible " << target->getName() << "." << std::endl;
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
                    std::cout << player->getName() << " est réveillé après la défaite du groupe. Mort comptabilisée." << std::endl;
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
                    std::cout << player->getName() << " reste au sol : aucune récompense supplémentaire après sa chute." << std::endl;
                    continue;
                }
            }
            else
            {
                player->recordDeath();
                player->reviveWithHealthPercentage(DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty));
                std::cout << player->getName() << " est réveillé à la fin du combat. La mort est comptabilisée." << std::endl;
            }
        }

        CombatReward individualReward = buildIndividualBossCoopReward(
            baseReward,
            *player,
            leader,
            contributions[i]
        );

        std::cout << player->getName() << " :" << std::endl;
        if (contributions[i].turnsTaken <= 0)
        {
            std::cout << "Participation insuffisante : le registre ne donne presque rien." << std::endl;
        }
        CombatRewardSystem::displayReward(individualReward);
        CombatRewardSystem::giveRewardToPlayer(*player, individualReward);
        std::cout << "Participation boss : tours " << contributions[i].turnsTaken
                  << ", dégâts " << contributions[i].damageDealt
                  << ", soins " << contributions[i].healingDone
                  << ", dégâts encaissés " << contributions[i].damageTaken << "." << std::endl;

        if (contributions[i].turnsTaken > 0)
        {
            player->recordVictory();
            player->recordBossKill();
            bool newEntityDetected = player->recordBossVictoryInRegistry(boss.getBossId());
            if (newEntityDetected)
            {
                std::cout << "Registre de " << player->getName() << " : une nouvelle entité est devenue détectable." << std::endl;
            }

            LootGenerator::giveDefeatedBossLoot(*player, boss, random, difficulty);
        }
        else
        {
            std::cout << "Aucun loot de boss : le personnage n'a pas réellement participé." << std::endl;
        }

        std::cout << std::endl;
    }
}
