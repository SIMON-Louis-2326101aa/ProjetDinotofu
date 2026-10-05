// EN: StoryPrologueMemory.cpp implements the playable pre-fog memory without touching persistent progression.
// FR: StoryPrologueMemory.cpp implémente le souvenir jouable pré-brume sans toucher à la progression persistante.
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
            MessageScreen::show("ÉQUIPE — SYNCHRONISATION", "story.white_fog.memory.allies", lines, false);
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

        MessageScreen::show("ÉQUIPE — SYNCHRONISATION", "story.white_fog.memory.allies", lines, false);
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
    static const std::vector<std::string> firstLabels = {"Sca—", "S…lett?", "[nom arraché]", "S…"};
    static const std::vector<std::string> secondLabels = {"Lor—", "L?ren…", "[nom perdu]", "L…"};
    const std::vector<std::string>& labels = firstCompanion ? firstLabels : secondLabels;
    const int safePhase = std::max(0, phase);
    return labels[static_cast<std::size_t>(safePhase) % labels.size()];
}

std::vector<std::string> StoryPrologueMemory::buildMissionFragmentLines(const Player& memoryPlayer)
{
    std::vector<std::string> lines = {
        "Ce n'est pas le début de ton aventure.",
        "C'est un souvenir qui essaie encore de se rappeler comment il commençait.",
        "[CONTRAT — fragment mémoriel]",
        "Zone : Glacier des Serments froids.",
        "Objectif : éliminer le chef de meute [portion effacée].",
        "Menace confirmée : un meneur et deux garde-crocs lourds.",
        "Retour prévu : avant [heure illisible].",
        "Groupe : " + memoryPlayer.getName() + " / " + obscuredCompanionLabel(true, 0) + " / " + obscuredCompanionLabel(false, 0) + ".",
        "Tu connais cette route. Ton corps aussi. Pourtant, certains mots disparaissent dès que tu essaies de les lire deux fois.",
        "Niveau dont tu te souviens : " + std::to_string(memoryPlayer.getLevel()) + ".",
        "Classe : " + memoryPlayer.getType() + "."
    };

    if (memoryPlayer.hasEquippedWeapon())
    {
        lines.push_back("Arme en mémoire : " + memoryPlayer.getEquippedWeapon().getName() + ".");
    }
    if (memoryPlayer.hasEquippedArmor())
    {
        lines.push_back("Armure en mémoire : " + memoryPlayer.getEquippedArmor().getName() + ".");
    }
    lines.push_back("Tu ne découvres pas ces gestes : tu les retrouves. Ce groupe s'est déjà battu ensemble des dizaines de fois.");
    return lines;
}

std::vector<std::string> StoryPrologueMemory::buildTravelLines(const Player& memoryPlayer, int approachChoice)
{
    std::vector<std::string> lines = {
        "La neige du glacier craque sous trois rythmes de pas qui savent déjà éviter de se gêner.",
        obscuredCompanionLabel(true, 0) + " : « Les traces se resserrent. Il nous a sentis. »",
        obscuredCompanionLabel(false, 0) + " : « Alors on ne lui donne pas le temps de choisir le terrain. »",
        memoryPlayer.getName() + " reconnaît leur façon de parler avant de reconnaître leurs visages. Ça devrait te rassurer. Ça ne le fait pas."
    };

    if (approachChoice == 1)
    {
        lines.push_back(memoryPlayer.getName() + " : « Formation serrée. On le laisse venir sur nous. »");
        lines.push_back("Une réponse courte vient des deux côtés. Aucun débat : c'était probablement votre manière habituelle de travailler.");
    }
    else if (approachChoice == 2)
    {
        lines.push_back(memoryPlayer.getName() + " : « On accélère. Je veux le chef avant qu'il rassemble le reste. »");
        lines.push_back(obscuredCompanionLabel(true, 1) + " rit une demi-seconde. Tu te souviens du son, pas de la raison.");
    }
    else
    {
        lines.push_back(memoryPlayer.getName() + " : « Comme d'habitude. Je vous suis. »");
        lines.push_back("Le silence qui suit est trop confortable pour appartenir à des inconnus.");
    }

    lines.push_back("Puis trois masses grises sortent du vent blanc. Celle du centre ne baisse pas les yeux.");
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
            "Le chef de meute s'effondre enfin. Les deux autres ne se relèvent plus.",
            obscuredCompanionLabel(true, 1) + " laisse retomber son arme : « Voilà. On rentre, on mange, et cette fois tu paies. »",
            obscuredCompanionLabel(false, 1) + " répond quelque chose que tu as forcément déjà entendu cent fois.",
            "Tu souris avant même de comprendre la blague.",
            "C'est exactement à cet instant que le vent s'arrête."
        };
    }
    else if (result.outcome == StoryPrologueOutcome::Retreat)
    {
        lines = {
            "Tu donnes le signal de décrocher avant que la meute vous enferme.",
            obscuredCompanionLabel(false, 1) + " : « Repli propre. Je ferme derrière ! »",
            obscuredCompanionLabel(true, 1) + " se replace à ton épaule sans te demander pourquoi tu as choisi de partir.",
            "Vous avez déjà quitté des combats ensemble. La honte n'entre pas dans vos gestes ; seulement l'urgence.",
            "C'est exactement à cet instant que le vent s'arrête."
        };
    }
    else
    {
        lines = {
            "Tes jambes lâchent avant que la meute ne cède.",
            obscuredCompanionLabel(false, 1) + " : « On décroche ! Maintenant ! »",
            obscuredCompanionLabel(true, 1) + " revient vers toi au lieu de courir vers la sortie.",
            "Une main passe sous ton bras et te relève à moitié. Quelqu'un refuse manifestement de te laisser ici.",
            "Tu voudrais te souvenir de la première fois où cette personne t'a déjà ramené vivant. Le souvenir n'arrive pas.",
            "C'est exactement à cet instant que le vent s'arrête."
        };
    }

    const std::vector<std::string> corruption = {
        "Au bord du glacier, quelque chose de blanc monte contre la pente.",
        "Ce n'est pas de la neige. Ce n'est pas du brouillard. Ça avance même quand l'air recule.",
        "Zone : Glacier des Serments f—",
        "Objectif : éliminer [                            ]",
        "Groupe : " + sourcePlayer.getName() + " / " + obscuredCompanionLabel(true, 2) + " / " + obscuredCompanionLabel(false, 2) + ".",
        obscuredCompanionLabel(true, 3) + " crie ton nom.",
        obscuredCompanionLabel(false, 3) + " essaie d'en dire un autre. Le mot disparaît avant d'arriver jusqu'à toi.",
        "Ton arme devient trop lourde parce que tu ne te rappelles plus depuis combien d'années tu la portes.",
        "Ton niveau n'est plus un nombre. Tes compétences deviennent des réflexes sans origine. Puis même les réflexes commencent à blanchir.",
        "Tu essaies de regarder les deux autres. La Brume retire d'abord les détails, ensuite les visages, ensuite la certitude qu'ils étaient deux.",
        "BLANC.",
        "Encore blanc.",
        "Puis plus rien ne sait où finit le souvenir."
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
        MenuScreen orders("ORDRES DU GROUPE — SOUVENIR", "story.white_fog.memory.orders");
        orders.addSubtitle("Les noms s'effacent. Les automatismes, pas encore.");
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
