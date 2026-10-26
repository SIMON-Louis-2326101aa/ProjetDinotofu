// EN: StoryPrologueMemory.cpp implements the playable pre-fog memory without touching persistent progression.
// FR: StoryPrologueMemory.cpp implémente la séquence jouable pré-brume sans toucher à la progression persistante.
#include "story/StoryPrologueMemory.hpp"

#include "combat/EnemyCombatQueue.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"
#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "progression/DifficultyRules.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
    std::string normalizeClassName(const std::string& className)
    {
        std::string normalized = className;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return normalized;
    }

    bool containsAny(const std::string& text, const std::vector<std::string>& needles)
    {
        for (const std::string& needle : needles)
        {
            if (text.find(needle) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    void addMemoryEquipment(Player& player)
    {
        Inventory& inventory = player.getInventory();
        const std::string className = normalizeClassName(player.getType());

        if (containsAny(className, {"arbal", "crossbow"}))
        {
            inventory.addWeapon(WeaponCatalog::createPatrolCrossbow());
            inventory.addArmor(ArmorCatalog::createColdSurvivalParka());
            inventory.addMaterial(MaterialCatalog::createFrozenBolts(80));
        }
        else if (containsAny(className, {
            "archer", "tireur", "chasseur", "rodeur", "rôdeur", "pisteur", "guetteur", "fauconnier",
            "frondeur", "éclaireur", "eclaireur", "trappeur", "messager", "arquebusier", "sentinelle",
            "reliques", "nomade"
        }))
        {
            inventory.addWeapon(WeaponCatalog::createColdLanternBow());
            inventory.addArmor(ArmorCatalog::createColdSurvivalParka());
            inventory.addMaterial(MaterialCatalog::createBarbedArrows(80));
        }
        else if (containsAny(className, {
            "mage", "sorc", "mancien", "arcan", "enchante", "runiste", "shaman", "chaman", "druide",
            "clerc", "prêtre", "pretre", "oracle", "invoc", "pactisant", "marionnett", "totém", "totem",
            "reliquaire", "aumônier", "aumonier", "alchim"
        }))
        {
            inventory.addWeapon(WeaponCatalog::createSingingResinStaff());
            inventory.addArmor(ArmorCatalog::createLivingFiberRobe());
        }
        else if (containsAny(className, {"assassin", "voleur", "ombre", "dague", "duelliste", "sabreur", "danseur"}))
        {
            inventory.addWeapon(WeaponCatalog::createAmberEdgeDagger());
            inventory.addArmor(ArmorCatalog::createShadowThreadCoat());
            inventory.addMaterial(MaterialCatalog::createTrainingThrowingKnives(60));
        }
        else if (containsAny(className, {"lancier", "lance", "hallebard", "javel"}))
        {
            inventory.addWeapon(WeaponCatalog::createGreyCliffSpear());
            inventory.addArmor(ArmorCatalog::createPolishedScaleHarness());
        }
        else if (containsAny(className, {"barbare", "colosse", "briseur", "marte", "forger", "nain", "berserker", "faucheur", "ravageur"}))
        {
            inventory.addWeapon(WeaponCatalog::createWhistlingMineHammer());
            inventory.addArmor(ArmorCatalog::createWhistlingMinerHarness());
        }
        else
        {
            inventory.addWeapon(WeaponCatalog::createRunicIronBlade());
            inventory.addArmor(ArmorCatalog::createRunicChainmail());
        }

        inventory.addConsumable(ConsumableCatalog::createMajorHealingPotion());
        inventory.addConsumable(ConsumableCatalog::createMajorHealingPotion());
        inventory.addConsumable(ConsumableCatalog::createGreaterHealingPotion());
        inventory.addConsumable(ConsumableCatalog::createGreaterDamagePotion());
        inventory.addConsumable(ConsumableCatalog::createGreaterDefensivePotion());
        inventory.addConsumable(ConsumableCatalog::createPrecisionPotion());
        if (player.getMagicChannelingStyleLabel() != "Réserve mineure")
        {
            inventory.addConsumable(ConsumableCatalog::createManaPotion());
            inventory.addConsumable(ConsumableCatalog::createManaPotion());
            if (player.getMagicChannelingStyleLabel() == "Minerais / matériaux")
            {
                inventory.addMaterial(MaterialCatalog::createArcaneDust(24));
            }
        }

        player.equipWeapon(0);
        player.equipArmor(0);
    }

    int chooseAllyTarget(const EnemyCombatQueue& wave, int order)
    {
        if (wave.getActiveEnemyCount() <= 0)
        {
            return -1;
        }

        if (order == 1)
        {
            for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
            {
                if (wave.getActiveEnemy(i).getName().find("Chef de meute") != std::string::npos)
                {
                    return i;
                }
            }
        }

        if (order == 2)
        {
            int target = 0;
            int lowestHp = wave.getActiveEnemy(0).getHp();
            for (int i = 1; i < wave.getActiveEnemyCount(); ++i)
            {
                if (wave.getActiveEnemy(i).getHp() < lowestHp)
                {
                    target = i;
                    lowestHp = wave.getActiveEnemy(i).getHp();
                }
            }
            return target;
        }

        return 0;
    }

    void playCompanionActions(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        int turn,
        int order
    )
    {
        if (!wave.hasActiveEnemies() || player.isDead())
        {
            return;
        }

        std::vector<std::string> lines;
        const std::string first = StoryPrologueMemory::obscuredCompanionLabel(true, std::min(2, turn / 3));
        const std::string second = StoryPrologueMemory::obscuredCompanionLabel(false, std::min(2, turn / 3));

        int targetIndex = chooseAllyTarget(wave, order);
        if (targetIndex >= 0 && wave.isActiveIndexValid(targetIndex))
        {
            Monster& target = wave.getActiveEnemy(targetIndex);
            int damage = random.between(52, 82);
            if (order == 1)
            {
                damage += 8;
            }
            if (turn % 3 == 0)
            {
                damage += 18;
                target.applyNextHitVulnerability(2, 18);
                lines.push_back(first + " coupe la trajectoire du chef de meute et ouvre une fenêtre nette.");
            }
            else
            {
                lines.push_back(first + " frappe " + target.getName() + " dans l'angle mort.");
            }
            target.takeDamage(damage);
            lines.push_back("Dégâts alliés : " + std::to_string(damage) + ".");
            wave.removeDeadAndReplace();
        }

        if (!wave.hasEnemiesLeft())
        {
            MessageScreen::show("AUTOMATISMES DU GROUPE", "story.white_fog.memory.allies", lines, false);
            return;
        }

        const bool needsHeal = player.getHp() <= player.getMaxHp() * 55 / 100;
        if (needsHeal)
        {
            const int healing = std::max(45, player.getMaxHp() / 10 + random.between(12, 30));
            player.heal(healing);
            lines.push_back(second + " te stabilise sans casser la formation : +" + std::to_string(healing) + " PV.");
        }
        else if (order == 3)
        {
            player.applyElementalWard(2, 18);
            player.applyRegeneration(2, std::max(12, player.getMaxHp() / 45));
            lines.push_back(second + " ferme votre flanc et te couvre pour les deux prochaines respirations.");
        }
        else
        {
            targetIndex = chooseAllyTarget(wave, order);
            if (targetIndex >= 0 && wave.isActiveIndexValid(targetIndex))
            {
                Monster& target = wave.getActiveEnemy(targetIndex);
                int damage = random.between(38, 68);
                if (turn % 3 == 0)
                {
                    damage += 14;
                    lines.push_back(second + " reprend exactement l'ouverture créée une seconde plus tôt.");
                }
                else
                {
                    lines.push_back(second + " retient une charge puis renvoie le mouvement sur " + target.getName() + ".");
                }
                target.takeDamage(damage);
                lines.push_back("Dégâts alliés : " + std::to_string(damage) + ".");
                wave.removeDeadAndReplace();
            }
        }

        if (turn % 3 == 0 && wave.hasEnemiesLeft())
        {
            lines.push_back("Le groupe retrouve un automatisme ancien. Personne n'a besoin de compter jusqu'à trois.");
        }

        MessageScreen::show("AUTOMATISMES DU GROUPE", "story.white_fog.memory.allies", lines, false);
    }
}

Player StoryPrologueMemory::createTemporaryPlayer(const Player& source, DifficultyMode difficulty)
{
    (void)difficulty;
    Player memoryPlayer = source;
    memoryPlayer.getInventory().clearAll();

    const int memoryMaxHp = std::max(920, source.getMaxHp() + 720);
    const int memoryMinDamage = std::max(68, source.getMinDamage() + 58);
    const int memoryMaxDamage = std::max(132, source.getMaxDamage() + 102);
    const int memoryCriticalDamage = std::max(190, source.getCriticalDamage() + 125);

    memoryPlayer.setLoadedProgress(MEMORY_LEVEL, 0, memoryMaxHp);
    memoryPlayer.setLoadedCombatStats(
        memoryMaxHp,
        memoryMinDamage,
        memoryMaxDamage,
        memoryCriticalDamage,
        memoryMaxHp
    );
    memoryPlayer.refreshLevelAndIdentitySkills(true);
    addMemoryEquipment(memoryPlayer);
    memoryPlayer.heal(memoryPlayer.getMaxHp());
    memoryPlayer.restoreManaToFull();
    return memoryPlayer;
}

std::vector<Monster> StoryPrologueMemory::createPackHunt(DifficultyMode difficulty)
{
    const int healthPercentage = DifficultyRules::getMonsterHealthPercentage(difficulty);
    const int damagePercentage = DifficultyRules::getMonsterDamagePercentage(difficulty);

    auto scaled = [&](
        const std::string& name,
        int level,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotionCount,
        int damagePotionCount,
        bool evolved
    ) {
        return Monster(
            name,
            "Prédateur de meute",
            Race::Bete,
            level,
            std::max(1, maxHp * healthPercentage / 100),
            std::max(1, minDamage * damagePercentage / 100),
            std::max(1, maxDamage * damagePercentage / 100),
            std::max(1, criticalDamage * damagePercentage / 100),
            healingPotionCount,
            damagePotionCount,
            false,
            true,
            false,
            evolved
        );
    };

    return {
        scaled("Chef de meute du Serment froid", 40, 980, 44, 92, 148, 1, 1, true),
        scaled("Garde-croc givré", 38, 560, 34, 70, 116, 0, 0, false),
        scaled("Garde-croc givré", 38, 560, 34, 70, 116, 0, 0, false)
    };
}

std::string StoryPrologueMemory::obscuredCompanionLabel(bool firstCompanion, int phase)
{
    static const std::vector<std::string> firstLabels = {"Sca---", "--arle--", "S--rlett", "---lett", "Sc--l--", "S---ett"};
    static const std::vector<std::string> secondLabels = {"Lor---", "--ren--", "L--enzo", "---enzo", "Lo--n--", "L---zo"};
    const std::vector<std::string>& labels = firstCompanion ? firstLabels : secondLabels;
    const int safePhase = std::max(0, phase);
    return labels[static_cast<std::size_t>(safePhase) % labels.size()];
}

std::vector<std::string> StoryPrologueMemory::buildOrdinaryRoutineLines(const Player& memoryPlayer)
{
    std::vector<std::string> lines = {
        "Avant la montée, vous perdez encore du temps pour des choses parfaitement ordinaires.",
        obscuredCompanionLabel(false, 0) + " recompte les rations et jure qu'il en manque une. "
            + obscuredCompanionLabel(true, 0) + " lui rappelle qu'il a mangé la ration manquante pendant la garde de nuit.",
        "Une sangle trop lâche est resserrée. Une boucle gelée refuse de fermer. Quelqu'un prête ses gants pendant trente secondes et exige de les récupérer avant d'avoir froid.",
        memoryPlayer.getName() + " vérifie les fioles par habitude, puis l'état de l'arme et des attaches. Personne ne te demande pourquoi : chacun fait sa partie du rituel.",
        "Vous croisez deux convoyeurs qui redescendent déjà. Un salut, une plaisanterie sur le temps, puis chacun reprend sa route.",
        obscuredCompanionLabel(true, 1) + " parle du repas du soir comme si la seule vraie difficulté de la journée était de choisir l'auberge au retour.",
        obscuredCompanionLabel(false, 1) + " répond qu'il refuse encore de payer pour toi. Tu lui rappelles qu'il dit ça après chaque contrat.",
        "Rien dans ces gestes n'a l'importance d'un adieu. C'est précisément ce qui les rend familiers : pour vous trois, ce matin ressemble à des dizaines d'autres."
    };

    if (memoryPlayer.hasEquippedWeapon())
    {
        lines.push_back("Dernier contrôle : " + memoryPlayer.getEquippedWeapon().getName() + " est en place. Tu n'y penses déjà plus une fois la marche reprise.");
    }
    return lines;
}

std::vector<std::string> StoryPrologueMemory::buildMissionFragmentLines(const Player& memoryPlayer)
{
    std::vector<std::string> lines = {
        "[CONTRAT DE CHASSE — GUILDE]",
        "Zone : Glacier des Serments froids.",
        "Objectif : éliminer le chef de meute et sécuriser la route [---].",
        "Signalement : un meneur, deux garde-crocs lourds, traces secondaires [illisible].",
        "Prime : -- PF + [partie effacée].",
        "Retour prévu : avant --:--.",
        "Groupe : " + memoryPlayer.getName() + " / " + obscuredCompanionLabel(true, 0) + " / " + obscuredCompanionLabel(false, 0) + ".",
        "Tu relis la ligne du groupe. Les lettres manquantes ne reviennent pas. Étrange... mais les deux personnes à côté de toi, elles, n'ont rien d'étrange.",
        "Le froid presse davantage que cette anomalie. Vous avez une chasse à terminer."
    };

    if (memoryPlayer.hasEquippedWeapon())
    {
        lines.push_back("Arme : " + memoryPlayer.getEquippedWeapon().getName() + ".");
    }
    if (memoryPlayer.hasEquippedArmor())
    {
        lines.push_back("Protection : " + memoryPlayer.getEquippedArmor().getName() + ".");
    }
    return lines;
}

std::vector<std::string> StoryPrologueMemory::buildTravelLines(const Player& memoryPlayer, int approachChoice)
{
    std::vector<std::string> lines = {
        "La neige craque sous trois rythmes de pas qui savent déjà se partager le chemin.",
        obscuredCompanionLabel(true, 0) + " : « Les traces se resserrent. Il nous a sentis. »",
        obscuredCompanionLabel(false, 0) + " : « Tant mieux. J'avais peur qu'on ait grimpé tout ça pour rien. »",
        obscuredCompanionLabel(true, 1) + " : « Dis ça encore une fois et tu portes les peaux au retour. »",
        "La réponse te fait sourire. Tu ne saurais pas expliquer pourquoi cette dispute te paraît si familière."
    };

    if (approachChoice == 1)
    {
        lines.push_back(memoryPlayer.getName() + " : « Formation serrée. On le laisse venir sur nous. »");
        lines.push_back("Deux réponses brèves. Personne ne demande où se placer.");
    }
    else if (approachChoice == 2)
    {
        lines.push_back(memoryPlayer.getName() + " : « On accélère. Je veux le chef avant qu'il rassemble le reste. »");
        lines.push_back(obscuredCompanionLabel(true, 2) + " : « Enfin une bonne idée. »");
    }
    else
    {
        lines.push_back(memoryPlayer.getName() + " : « Comme d'habitude. »");
        lines.push_back(obscuredCompanionLabel(false, 2) + " : « Tu aurais pu choisir une phrase plus rassurante. »");
    }

    lines.push_back("Trois masses grises émergent ensuite du vent blanc. Celle du centre s'arrête et vous jauge.");
    lines.push_back("La conversation s'éteint d'un coup. Le travail commence.");
    return lines;
}

std::vector<std::string> StoryPrologueMemory::buildFogTransitionLines(
    const Player& sourcePlayer,
    const StoryPrologueCombatResult& result
)
{
    std::vector<std::string> lines;
    if (result.outcome == StoryPrologueOutcome::Victory)
    {
        lines = {
            "Le chef de meute s'effondre enfin. Le glacier retrouve lentement son silence.",
            obscuredCompanionLabel(true, 2) + " : « Je vote pour qu'on mange avant de redescendre. Et cette fois, tu paies. »",
            obscuredCompanionLabel(false, 2) + " : « Il paie depuis la dernière fois. C'est toi qui oublies toujours. »",
            "Un peu plus bas, à l'abri d'un rocher, vous rallumez un petit feu. La viande sèche, le pain dur et quelque chose de beaucoup trop épicé passent de main en main.",
            "Vous parlez du contrat comme d'un travail terminé. Une histoire idiote ressort. Quelqu'un imite le cri d'un ancien client. Tu manques de t'étouffer en riant.",
            "Pendant quelques minutes, il n'y a plus de chasse, plus de monstre, seulement trois personnes fatiguées qui mangent avant le chemin du retour.",
            "Puis la flamme du feu se couche brusquement alors qu'il n'y a plus de vent."
        };
    }
    else if (result.outcome == StoryPrologueOutcome::Retreat)
    {
        lines = {
            "Tu donnes le signal de décrocher avant que la meute vous enferme.",
            obscuredCompanionLabel(false, 2) + " : « Repli propre. Je ferme derrière ! »",
            obscuredCompanionLabel(true, 2) + " se replace à ton épaule sans discuter.",
            "Vous trouvez enfin un replat protégé. Personne ne célèbre quoi que ce soit, mais une gourde passe quand même de main en main.",
            obscuredCompanionLabel(true, 3) + " : « On rentre. On pourra revenir mieux préparés. »",
            "Tu hoches la tête. La défaite du jour peut encore attendre demain.",
            "Puis votre souffle cesse soudain de partir dans la même direction que le vent."
        };
    }
    else
    {
        lines = {
            "Tes jambes lâchent avant que la meute ne cède.",
            obscuredCompanionLabel(false, 2) + " : « On décroche ! Maintenant ! »",
            obscuredCompanionLabel(true, 2) + " revient vers toi au lieu de courir.",
            "Une main passe sous ton bras. Une autre couvre la retraite. Le glacier devient une suite de pas mal assurés et de voix trop proches.",
            "Quand tu reprends vraiment conscience, tu es adossé à une paroi, enveloppé dans une couverture de voyage.",
            obscuredCompanionLabel(true, 3) + " : « Tu nous referas jamais ça. Compris ? »",
            "Tu essaies de répondre quelque chose de drôle. Ta gorge ne produit qu'un souffle.",
            "C'est alors qu'une fumée blanche glisse derrière les rochers, à contre-pente."
        };
    }

    const std::vector<std::string> corruption = {
        "Quelque chose de blanc avance sur le glacier.",
        "Ce n'est ni la neige soulevée par le vent, ni un brouillard ordinaire. La masse remonte la pente sans se disperser.",
        "Le contrat dans ta poche te revient soudain en tête : Glacier des Serments f---.",
        "Objectif : éliminer [                              ].",
        "Groupe : " + sourcePlayer.getName() + " / " + obscuredCompanionLabel(true, 4) + " / " + obscuredCompanionLabel(false, 4) + ".",
        obscuredCompanionLabel(true, 5) + " crie ton nom.",
        obscuredCompanionLabel(false, 5) + " essaie d'en dire un autre. Tu entends le début, puis plus rien.",
        "Tu regardes ton arme. Tu sais t'en servir. Tu sais qu'elle est à toi. Mais impossible de te rappeler depuis quand.",
        "Une technique te traverse l'esprit sans nom. Puis une autre disparaît avant même que tu puisses la formuler.",
        "Tu lèves les yeux vers les deux autres. Les détails de leurs visages deviennent difficiles à retenir.",
        "Le blanc gagne les bords de ta vision.",
        "BLANC.",
        "Encore blanc.",
        "Puis plus rien."
    };
    lines.insert(lines.end(), corruption.begin(), corruption.end());
    return lines;
}

StoryPrologueCombatResult StoryPrologueMemory::runPackHunt(
    Player& memoryPlayer,
    Random& random,
    DifficultyMode difficulty,
    int approachChoice
)
{
    EnemyCombatQueue wave;
    for (const Monster& monster : createPackHunt(difficulty))
    {
        wave.addWaitingEnemy(monster);
    }
    wave.initializeFrontLine();

    int teamOrder = 4;
    if (approachChoice == 1)
    {
        teamOrder = 3;
        memoryPlayer.applyElementalWard(3, 15);
    }
    else if (approachChoice == 2)
    {
        teamOrder = 1;
        if (wave.hasActiveEnemies())
        {
            wave.getActiveEnemy(0).applyNextHitVulnerability(2, 15);
        }
    }
    else
    {
        memoryPlayer.applyRegeneration(3, std::max(10, memoryPlayer.getMaxHp() / 55));
    }

    bool escapeSucceeded = false;
    int turns = 0;
    memoryPlayer.beginChallengeCombatTracking();

    auto openOrders = [&]() -> bool {
        MenuScreen orders("ORDRES DU GROUPE", "story.white_fog.memory.orders");
        orders.addSubtitle("Vous avez l'habitude de vous comprendre vite. Choisis le rythme du groupe.");
        orders.addOption(1, "Concentrer le chef", "Les deux alliés privilégient le meneur de la meute.", true, "story.white_fog.memory.orders.leader");
        orders.addOption(2, "Achever le plus faible", "Réduire vite le nombre d'ennemis actifs.", true, "story.white_fog.memory.orders.weak");
        orders.addOption(3, "Me couvrir", "Le soutien privilégie défense, soin et stabilité.", true, "story.white_fog.memory.orders.cover");
        orders.addOption(4, "Rythme libre", "Laisser les automatismes du groupe décider.", true, "story.white_fog.memory.orders.free");
        teamOrder = TerminalInterface::askMenuChoiceFromOptions(orders, "Donne un ordre sans consommer ton tour.");
        MessageScreen::show(
            "ORDRE TRANSMIS",
            "story.white_fog.memory.orders.confirmed",
            {"Le groupe change de rythme sans poser de question. C'est un vieux langage entre vous."},
            true
        );
        return true;
    };

    while (!memoryPlayer.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        bool playerTurnFinished = false;
        while (!playerTurnFinished && !memoryPlayer.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playerTurnFinished = PlayerWaveCombatTurn::play(
                memoryPlayer,
                wave,
                random,
                escapeSucceeded,
                difficulty,
                true,
                openOrders
            );

            if (playerTurnFinished)
            {
                ++turns;
                memoryPlayer.reduceClassSkillCooldown();
            }
        }

        wave.removeDeadAndReplace();
        if (memoryPlayer.isDead() || !wave.hasEnemiesLeft() || escapeSucceeded)
        {
            break;
        }

        playCompanionActions(memoryPlayer, wave, random, turns, teamOrder);
        wave.removeDeadAndReplace();
        if (!wave.hasEnemiesLeft())
        {
            break;
        }

        MonsterWaveCombatTurn::playMonsterTurns(memoryPlayer, wave, random);
        wave.removeDeadAndReplace();
    }

    StoryPrologueCombatResult result;
    result.turns = turns;
    result.defeatedEnemies = wave.getDefeatedEnemyCount();
    result.remainingHp = std::max(0, memoryPlayer.getHp());

    if (!wave.hasEnemiesLeft())
    {
        result.outcome = StoryPrologueOutcome::Victory;
    }
    else if (escapeSucceeded)
    {
        result.outcome = StoryPrologueOutcome::Retreat;
    }
    else
    {
        result.outcome = StoryPrologueOutcome::Defeat;
    }
    return result;
}
