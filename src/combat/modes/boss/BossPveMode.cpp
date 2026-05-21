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

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>


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
            std::cout << "Conseil du dev : certains secrets ne se donnent pas dans les menus. Ils apparaissent quand tu reviens lire les registres après une victoire qui semblait inutile." << std::endl;
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
            std::cout << "Trexof. Bêta-testeur, assassin, et probablement déjà en train de calculer si ce combat est équilibré." << std::endl;
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
            std::cout << "Même en tant que MJ, même en tant que dev, même en tant que créateur... il y a des personnages qu'on protège un peu trop." << std::endl;
            std::cout << "Indice : ton dôme n'est pas seulement une défense. Bien placé, il peut transformer un tour perdu en tour survivant." << std::endl;
        }
        else
        {
            std::cout << "Même si ton nom n'a pas de note personnelle dans mes fichiers, ta trace existe maintenant." << std::endl;
            std::cout << "Conseil : un boss vaincu ne donne pas seulement du butin. Il ouvre parfois une porte que le menu ne nomme pas encore." << std::endl;
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
                std::cout << "Trexof, trouver une faille n'est pas pareil que la valider en production." << std::endl;
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

    void displayBossPowerAnalysis(
        const Player& player,
        const Boss& boss,
        DifficultyMode difficulty,
        const BossPowerAnalysis& analysis
    )
    {
        std::cout << "Le registre analyse la variation d'énergie devant toi..." << std::endl;
        std::cout << "Il compare ton niveau, ton état, ton équipement, tes ressources et la pression dégagée par l'entité." << std::endl;
        std::cout << std::endl;

        std::cout << "Résonance de " << player.getName() << " : ";

        if (analysis.ratioPercent >= 120)
        {
            std::cout << "stable" << std::endl;
        }
        else if (analysis.ratioPercent >= 95)
        {
            std::cout << "incertaine" << std::endl;
        }
        else if (analysis.ratioPercent >= 75)
        {
            std::cout << "fragile" << std::endl;
        }
        else if (analysis.ratioPercent >= 55)
        {
            std::cout << "brisée par endroits" << std::endl;
        }
        else
        {
            std::cout << "presque étouffée" << std::endl;
        }

        std::cout << "Puissance estimée de l'entité : " << getPowerLabel(analysis.risk) << "." << std::endl;
        std::cout << std::endl;

        switch (analysis.risk)
        {
            case BossPowerRisk::PlayerAdvantage:
                std::cout << "Ton corps ne tremble pas. Pour une fois, l'arène semble te reconnaître comme une vraie menace." << std::endl;
                break;

            case BossPowerRisk::Balanced:
                std::cout << "Le registre ne voit aucun vainqueur évident. Ce combat peut basculer sur une seule bonne décision." << std::endl;
                break;

            case BossPowerRisk::Risky:
                std::cout << "Ton instinct hésite. Ce boss est prenable, mais une erreur pourrait coûter très cher." << std::endl;
                break;

            case BossPowerRisk::MajorDanger:
                std::cout << "Ton corps comprend avant ton esprit : cette entité est au-dessus de toi." << std::endl;
                std::cout << "Ce n'est pas impossible... mais l'arène sent déjà le sang." << std::endl;
                break;

            case BossPowerRisk::ProbableExtermination:
            default:
                std::cout << "Ton instinct te hurle de reculer." << std::endl;
                std::cout << "Cette chose ne ressemble pas à un adversaire. Elle ressemble à une fin de partie." << std::endl;
                break;
        }

        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            std::cout << std::endl;
            std::cout << "Difficulté Léthal détectée." << std::endl;
            std::cout << "Si tu meurs ici, ce personnage peut devenir une simple trace dans le registre des morts." << std::endl;
        }

        if (boss.getName() == "???")
        {
            std::cout << std::endl;
            std::cout << "Le nom reste brouillé, mais sa pression suffit déjà à salir les pages du registre." << std::endl;
        }

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
            std::cout << "5 : Patch d'urgence : survivre trois tours" << std::endl;
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
        std::cout << "Si tu bats Zelef plus tard, tu pourras récupérer ce qu'il t'a pris." << std::endl;
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
        if (analysis.risk == BossPowerRisk::MajorDanger
            || analysis.risk == BossPowerRisk::ProbableExtermination)
        {
            std::cout << "Procéder malgré tout ?" << std::endl;
        }
        else
        {
            std::cout << "Procéder au combat ?" << std::endl;
        }

        std::cout << "1 : Entrer dans l'arène" << std::endl;
        std::cout << "0 : Reculer" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int confirmation = Console::askNumberBetween(0, 1, "Choix invalide.");
        return confirmation == 1;
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

    std::cout << player1.getName() << ", choisis le type d'apparition du boss :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Boss aléatoire" << std::endl;
    std::cout << "2 : Choisir le boss toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int bossChoiceType = Console::askNumberBetween(
        1,
        2,
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

        std::cout << "Sélectionne l'entité que tu veux exterminer :" << std::endl;
        std::cout << std::endl;

        BossCatalog::displayAvailableBosses(player1.getUnlockedBossIds());

        std::cout << "Les boss vaincus récemment sont temporairement instables." << std::endl;
        std::cout << "Pour refaire un boss, il faut vaincre au moins deux autres boss avant." << std::endl;
        std::cout << "Leurs statistiques resteront inconnues pour le moment." << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        bossChoice = Console::askNumberBetween(
            1,
            BossCatalog::getMaximumBossId(),
            "Veuillez entrer un identifiant de boss valide."
        );

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

    std::cout << player1.getName() << ", ta classe évolue en : " << player1.getType() << "." << std::endl;
    std::cout << "Tes PV et tes objets ont été renforcés pour ce combat." << std::endl;
    std::cout << "Tes dégâts, eux, restent bloqués : même l'arène semble avoir ses limites." << std::endl;
    std::cout << std::endl;

    player1.displayStats();

    Console::pauseSeconds(3);

    if (boss.getBossId() == 27)
    {
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
                turn = 1;
            }
        }
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

        std::cout << player1.getName()
                  << " revient à lui avec "
                  << player1.getHp()
                  << "/"
                  << player1.getMaxHp()
                  << " PV."
                  << std::endl;
        std::cout << "Même vaincu, tu n'es pas encore sorti du registre des vivants." << std::endl;
        std::cout << std::endl;

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
            std::cout << std::endl;
            std::cout << "Une nouvelle entité a été détectée dans le registre des variations d'énergie anormale." << std::endl;
            std::cout << "Nom : ???" << std::endl;
            std::cout << "Statut : brouillé." << std::endl;
            std::cout << "Le registre a ajouté une entrée, mais refuse encore d'en révéler l'identité." << std::endl;
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

        LootGenerator::giveDefeatedBossLoot(player1, boss, random, difficulty);
    }
}
