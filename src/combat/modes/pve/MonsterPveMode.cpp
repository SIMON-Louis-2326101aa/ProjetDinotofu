// EN: MonsterPveMode.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterPveMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pve/MonsterPveMode.hpp"

#include "combat/EnemyCombatQueue.hpp"
#include "combat/encounter/AdventurerGroupEncounter.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/system/WaveCombatSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/reward/CombatReward.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/TurnManager.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltyResult.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "core/Console.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <cctype>

namespace
{
    std::string buildMonsterBestiaryDescription(const Monster& monster)
    {
        std::string description = monster.getName()
            + " | Race : "
            + monster.getRaceText()
            + " | Niveau : "
            + std::to_string(monster.getLevel())
            + ".";

        if (monster.isElite())
        {
            description += " Cette entité est considérée comme élite.";
        }

        if (monster.isEvolved())
        {
            description += " Des signes d'évolution anormale sont visibles : masse renforcée, instincts plus nets, énergie plus dense.";
        }

        std::string traits = monster.getName() + " " + monster.getType();
        std::transform(traits.begin(), traits.end(), traits.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (traits.find("slime") != std::string::npos)
        {
            description += " Famille slime : la couleur influence souvent le danger, le statut possible et la zone préférée.";
        }
        if (traits.find("rouge") != std::string::npos || traits.find("chaud") != std::string::npos)
        {
            description += " Teinte rouge/chaude : risque de brûlure.";
        }
        if (traits.find("violet") != std::string::npos || traits.find("toxique") != std::string::npos || traits.find("putride") != std::string::npos)
        {
            description += " Teinte toxique : risque de poison.";
        }
        if (traits.find("bleu") != std::string::npos || traits.find("blanc") != std::string::npos || traits.find("givre") != std::string::npos)
        {
            description += " Teinte froide : risque de ralentissement par le givre.";
        }
        if (traits.find("jaune") != std::string::npos || traits.find("orage") != std::string::npos)
        {
            description += " Teinte électrique : risque de choc, dangereux avec équipement métallique.";
        }
        if (traits.find("shaman") != std::string::npos || traits.find("chamane") != std::string::npos || traits.find("oracle") != std::string::npos)
        {
            description += " Profil soigneur/support : peut parfois prioriser un allié blessé plutôt qu'attaquer.";
        }

        if (!monster.areStatsVisible())
        {
            description += " Certaines statistiques restent troubles pour le moment.";
        }

        return description;
    }

    // EN: recordWaveEncountersInBestiary declares or implements a focused behavior used by this module.
    // FR: recordWaveEncountersInBestiary déclare ou implémente un comportement précis utilisé par ce module.
    void recordWaveEncountersInBestiary(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& monster = wave.getActiveEnemy(i);
            BestiaryRuntimeProgress::recordEncounter(
                monster.getName(),
                "Entités hostiles / ennemis",
                buildMonsterBestiaryDescription(monster)
            );
        }

        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            const Monster& monster = wave.getWaitingEnemy(i);
            BestiaryRuntimeProgress::recordEncounter(
                monster.getName(),
                "Entités hostiles / ennemis",
                buildMonsterBestiaryDescription(monster)
            );
        }
    }


    void collectSpecialNamesFromWavePart(
        const EnemyCombatQueue& wave,
        std::vector<std::string>& names,
        int count,
        bool defeated
    )
    {
        for (int i = 0; i < count; ++i)
        {
            const Monster& monster = defeated ? wave.getDefeatedEnemy(i) : wave.getActiveEnemy(i);

            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName()))
            {
                names.push_back(monster.getName());
            }
        }
    }

    std::vector<std::string> collectDefeatedSpecialNames(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names;
        collectSpecialNamesFromWavePart(wave, names, wave.getDefeatedEnemyCount(), true);
        return names;
    }

    std::vector<std::string> collectSurvivingSpecialNames(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& monster = wave.getActiveEnemy(i);
            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName())) names.push_back(monster.getName());
        }

        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            const Monster& monster = wave.getWaitingEnemy(i);
            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName())) names.push_back(monster.getName());
        }

        return names;
    }

    // EN: displaySpecialDefeatDialogues declares or implements a focused behavior used by this module.
    // FR: displaySpecialDefeatDialogues déclare ou implémente un comportement précis utilisé par ce module.
    void displaySpecialDefeatDialogues(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names = collectDefeatedSpecialNames(wave);
        SpecialCharacterGroupDialogueCatalog::displayDefeatDialogue(names);

        for (const std::string& name : names)
        {
            SpecialCharacterDialogueCatalog::displayDefeatDialogue(name);
        }
    }

    // EN: displaySpecialVictoryDialogues declares or implements a focused behavior used by this module.
    // FR: displaySpecialVictoryDialogues déclare ou implémente un comportement précis utilisé par ce module.
    void displaySpecialVictoryDialogues(const EnemyCombatQueue& wave)
    {
        std::vector<std::string> names = collectSurvivingSpecialNames(wave);
        SpecialCharacterGroupDialogueCatalog::displayVictoryDialogue(names);

        for (const std::string& name : names)
        {
            SpecialCharacterDialogueCatalog::displayVictoryDialogue(name);
        }
    }


    // EN: countDefeatedEvolvedMonsters declares or implements a focused behavior used by this module.
    // FR: countDefeatedEvolvedMonsters déclare ou implémente un comportement précis utilisé par ce module.
    int countDefeatedEvolvedMonsters(const EnemyCombatQueue& wave)
    {
        int total = 0;

        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            if (wave.getDefeatedEnemy(i).isEvolved())
            {
                ++total;
            }
        }

        return total;
    }

    // EN: recordWaveKillsInBestiary declares or implements a focused behavior used by this module.
    // FR: recordWaveKillsInBestiary déclare ou implémente un comportement précis utilisé par ce module.
    void recordWaveKillsInBestiary(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            const Monster& monster = wave.getDefeatedEnemy(i);
            BestiaryRuntimeProgress::recordKill(
                monster.getName(),
                "Entités hostiles / ennemis",
                buildMonsterBestiaryDescription(monster)
            );
        }
    }
}

void MonsterPveMode::run(
    Player& player,
    Random& random,
    DifficultyMode difficulty
)
{
    Console::clear();

    MenuScreen encounterScreen("RENCONTRE PVE", "combat.pve.encounter_type");
    encounterScreen.addLine("Choisis le type de rencontre PvE.");
    encounterScreen.addOption(
        1,
        "Vague de monstres",
        "Une file d'ennemis classiques, avec maximum trois ennemis actifs.",
        true,
        "pve.encounter.wave"
    );
    encounterScreen.addOption(
        2,
        "Groupe d'aventuriers aléatoire",
        "Humains, semi-humains ou groupe spécial. Normalement pas un combat à mort, sauf cas dangereux.",
        true,
        "pve.encounter.adventurers"
    );

    int encounterChoice = TerminalInterface::askMenuChoiceFromOptions(
        encounterScreen,
        "Veuillez entrer 1 ou 2."
    );

    EnemyCombatQueue wave;

    if (encounterChoice == 2)
    {
        AdventurerGroupEncounter::displayGroupEncounterIntroduction();
        wave = AdventurerGroupEncounter::createRandomGroupForPlayer(player, random);
    }
    else
    {
        WaveCombatSystem::displayWaveIntroduction();
        wave = WaveCombatSystem::createWaveForPlayer(player, random, difficulty);
    }

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);

    CombatGroup enemyFrontPreview = CombatGroupBuilder::buildSideFromWave(
        wave,
        CombatSide::EnemySide
    );

    CombatGroupBuilder::displayGroup(
        enemyFrontPreview,
        "LIGNE ENNEMIE ACTIVE"
    );

    std::vector<Summon> playerSummons = SummonCombatSystem::createInitialSummonsFor(player);
    SummonCombatSystem::displaySummonArrival(player, playerSummons);

    CombatGroup playerGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        player,
        playerSummons,
        CombatSide::PlayerSide,
        CombatUnitKind::MainFighter
    );

    CombatGroupBuilder::displayGroup(
        playerGroupPreview,
        "GROUPE DU JOUEUR"
    );

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;
    int initialPlayerHp = player.getHp();
    int combatTurnCount = 0;

    while (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        bool playerTurnFinished = false;

        while (!playerTurnFinished
            // EN: !player.isDead declares or implements a focused behavior used by this module.
            // FR: !player.isDead déclare ou implémente un comportement précis utilisé par ce module.
            && !player.isDead()
            // EN: wave.hasEnemiesLeft declares or implements a focused behavior used by this module.
            // FR: wave.hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty
            );

            if (playerTurnFinished)
            {
                ++combatTurnCount;
            }

            if (!playerTurnFinished && !escapeSucceeded)
            {
                std::cout << "Ton tour n'est pas encore consommé." << std::endl;
                std::cout << std::endl;
            }
        }

        if (!player.isDead()
            // EN: wave.hasEnemiesLeft declares or implements a focused behavior used by this module.
            // FR: wave.hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
            // EN: hasActiveSummons declares or implements a focused behavior used by this module.
            // FR: hasActiveSummons déclare ou implémente un comportement précis utilisé par ce module.
            && SummonCombatSystem::hasActiveSummons(playerSummons))
        {
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );
        }
    }

    if (escapeSucceeded)
    {
        MessageScreen::show(
            "FUITE RÉUSSIE",
            "combat.pve.escape.success",
            {
                "Tu as quitté le combat.",
                "Tu ne récupéreras qu'une partie des récompenses liées à ce qui s'est réellement passé."
            },
            false
        );

        CombatReward reward = CombatRewardSystem::calculatePlayerEscapeReward(
            wave,
            difficulty
        );

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite réussie : les récompenses sont calculées selon la difficulté, les ennemis vaincus, et les ennemis encore en vie déjà blessés."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        recordWaveKillsInBestiary(wave);

        return;
    }

    if (player.isDead())
    {
        MessageScreen::show(
            "DÉFAITE",
            "combat.pve.defeat",
            {
                player.getName() + " tombe face à la vague ennemie.",
                "L'arène se referme dans un silence brutal."
            },
            false
        );

        player.recordDefeat();
        player.recordDeath();

        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);

        displaySpecialVictoryDialogues(wave);

        player.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        MessageScreen::show(
            "RETOUR À LA VIE",
            "combat.pve.revive",
            {
                player.getName() + " revient à lui avec " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " PV.",
                "Tu as survécu, mais la mort a laissé sa trace."
            },
            false
        );

        return;
    }

    displaySpecialDefeatDialogues(wave);

    MessageScreen::show(
        "VICTOIRE PVE",
        "combat.pve.victory",
        {
            "Tous les monstres de la vague ont été vaincus.",
            player.getName() + " reste debout au milieu des corps et de la poussière."
        },
        false
    );

    CombatReward reward = CombatRewardSystem::calculateWaveReward(
        wave,
        difficulty,
        player,
        initialPlayerHp,
        combatTurnCount,
        random
    );

    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    recordWaveKillsInBestiary(wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty);

    int evolvedKilled = countDefeatedEvolvedMonsters(wave);
    if (evolvedKilled > 0)
    {
        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolvedKilled, "Créature évoluée");
        if (updated > 0)
        {
            std::cout << "Le bestiaire et les quêtes liées aux créatures évoluées progressent." << std::endl;
            std::cout << std::endl;
        }
    }
}


bool MonsterPveMode::runExplorationWave(
    Player& player,
    Random& random,
    DifficultyMode difficulty,
    const std::vector<Monster>& monsters,
    const std::string& title
)
{
    Console::clear();

    EnemyCombatQueue wave;
    for (const Monster& monster : monsters)
    {
        wave.addWaitingEnemy(monster);
    }
    wave.initializeFrontLine();

    std::cout << "========== ÉVÉNEMENT D'EXPLORATION ==========" << std::endl;
    std::cout << title << std::endl;
    std::cout << "La rencontre se referme autour de toi : il faut tenir la ligne." << std::endl;
    std::cout << std::endl;

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);

    CombatGroup enemyFrontPreview = CombatGroupBuilder::buildSideFromWave(
        wave,
        CombatSide::EnemySide
    );

    CombatGroupBuilder::displayGroup(
        enemyFrontPreview,
        "LIGNE ENNEMIE ACTIVE"
    );

    std::vector<Summon> playerSummons = SummonCombatSystem::createInitialSummonsFor(player);
    SummonCombatSystem::displaySummonArrival(player, playerSummons);

    CombatGroup playerGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        player,
        playerSummons,
        CombatSide::PlayerSide,
        CombatUnitKind::MainFighter
    );

    CombatGroupBuilder::displayGroup(
        playerGroupPreview,
        "GROUPE DU JOUEUR"
    );

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;
    int initialPlayerHp = player.getHp();
    int combatTurnCount = 0;

    while (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        bool playerTurnFinished = false;

        while (!playerTurnFinished
            && !player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty
            );

            if (playerTurnFinished)
            {
                ++combatTurnCount;
            }

            if (!playerTurnFinished && !escapeSucceeded)
            {
                std::cout << "Ton tour n'est pas encore consommé." << std::endl;
                std::cout << std::endl;
            }
        }

        if (!player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
            && SummonCombatSystem::hasActiveSummons(playerSummons))
        {
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );
        }
    }

    if (escapeSucceeded)
    {
        std::cout << "Tu as fui l'événement d'exploration." << std::endl;
        std::cout << "Les récompenses sont limitées à ce qui a réellement été accompli." << std::endl;
        std::cout << std::endl;

        CombatReward reward = CombatRewardSystem::calculatePlayerEscapeReward(
            wave,
            difficulty
        );

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite d'exploration : seules les actions réelles comptent."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        recordWaveKillsInBestiary(wave);
        return false;
    }

    if (player.isDead())
    {
        std::cout << player.getName() << " tombe pendant l'événement d'exploration." << std::endl;
        std::cout << "La zone ne faisait pas que menacer : elle a vraiment frappé." << std::endl;
        std::cout << std::endl;

        player.recordDefeat();
        player.recordDeath();

        if (DifficultyRules::isPermanentDeath(difficulty))
        {
            DeathPenaltySystem::displayLethalDeathCorruption();
            return false;
        }

        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);
        displaySpecialVictoryDialogues(wave);

        player.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        std::cout << player.getName()
                  << " revient à lui avec "
                  << player.getHp()
                  << "/"
                  << player.getMaxHp()
                  << " PV."
                  << std::endl;
        std::cout << std::endl;
        return false;
    }

    displaySpecialDefeatDialogues(wave);

    MessageScreen::show(
        "ÉVÉNEMENT TERMINÉ",
        "exploration.wave.victory",
        {
            "L'événement d'exploration est terminé : les ennemis sont vaincus."
        },
        false
    );

    CombatReward reward = CombatRewardSystem::calculateWaveReward(
        wave,
        difficulty,
        player,
        initialPlayerHp,
        combatTurnCount,
        random
    );

    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    recordWaveKillsInBestiary(wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty);

    int evolvedKilled = countDefeatedEvolvedMonsters(wave);
    if (evolvedKilled > 0)
    {
        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolvedKilled, "Créature évoluée");
        if (updated > 0)
        {
            std::cout << "Le bestiaire et les quêtes liées aux créatures évoluées progressent." << std::endl;
            std::cout << std::endl;
        }
    }

    return true;
}


namespace
{
    int countAlivePlayers(const std::vector<Player*>& party)
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

    struct CoopContribution
    {
        int turnsTaken = 0;
        int damageDealt = 0;
        int healingDone = 0;
        int damageTaken = 0;
        int supportActions = 0;
        bool wasDowned = false;
    };

    int scoreTargetThreat(Player& player, const CoopContribution& contribution)
    {
        int score = 10;

        if (player.isProvoking())
        {
            return 10000 + player.getProvocationTurns() * 100;
        }

        if (player.hasHealingThreat()) score += 85;
        score += std::min(120, contribution.healingDone / 2);
        score += std::min(90, contribution.damageDealt / 3);
        score += std::min(60, contribution.damageTaken / 4);

        if (player.getMaxHp() > 0)
        {
            int missingPercent = (player.getMaxHp() - player.getHp()) * 100 / player.getMaxHp();
            if (missingPercent >= 60) score += 35;
            else if (missingPercent >= 35) score += 20;
        }

        const std::string type = CombatClassSystem::normalizeClassText(player.getType());
        if (type.find("clerc") != std::string::npos || type.find("pretre") != std::string::npos || type.find("prêtre") != std::string::npos || type.find("alchimiste") != std::string::npos)
        {
            score += 35;
        }
        if (type.find("gardien") != std::string::npos || type.find("tank") != std::string::npos || type.find("colosse") != std::string::npos || player.isInDefensePosture())
        {
            score += 18;
        }

        return score;
    }

    Player* chooseAlivePlayerTarget(std::vector<Player*>& party, Random& random, const std::vector<CoopContribution>* contributions = nullptr)
    {
        std::vector<Player*> candidates;
        std::vector<int> scores;

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead())
            {
                continue;
            }

            CoopContribution empty;
            const CoopContribution& contribution = (contributions != nullptr && i < contributions->size()) ? (*contributions)[i] : empty;
            candidates.push_back(player);
            scores.push_back(scoreTargetThreat(*player, contribution));
        }

        if (candidates.empty())
        {
            return nullptr;
        }

        int totalScore = std::accumulate(scores.begin(), scores.end(), 0);
        int roll = random.between(1, std::max(1, totalScore));
        int cursor = 0;

        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            cursor += scores[i];
            if (roll <= cursor)
            {
                return candidates[i];
            }
        }

        return candidates.back();
    }

    int sumActiveEnemyHp(const EnemyCombatQueue& wave)
    {
        int total = 0;
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i) total += std::max(0, wave.getActiveEnemy(i).getHp());
        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i) total += std::max(0, wave.getWaitingEnemy(i).getHp());
        return total;
    }

    bool monsterCanUseHealingTools(const Monster& monster)
    {
        std::string profile = monster.getName() + " " + monster.getRaceText() + " " + monster.getType();
        std::transform(profile.begin(), profile.end(), profile.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

        return profile.find("soigneur") != std::string::npos
            || profile.find("prêtre") != std::string::npos
            || profile.find("pretre") != std::string::npos
            || profile.find("clerc") != std::string::npos
            || profile.find("chaman") != std::string::npos
            || profile.find("alchimiste") != std::string::npos
            || profile.find("sorcier") != std::string::npos
            || profile.find("mage putride") != std::string::npos
            || profile.find("chef bandit") != std::string::npos
            || profile.find("pilleur vétéran") != std::string::npos
            || profile.find("pilleur veteran") != std::string::npos;
    }

    int findMostInjuredMonsterAllyIndex(EnemyCombatQueue& wave, int healerIndex)
    {
        int bestIndex = -1;
        int bestPercent = 101;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (i == healerIndex || !wave.isActiveIndexValid(i))
            {
                continue;
            }

            Monster& ally = wave.getActiveEnemy(i);
            if (ally.isDead() || ally.getHp() >= ally.getMaxHp())
            {
                continue;
            }

            int percent = ally.getMaxHp() <= 0 ? 100 : ally.getHp() * 100 / ally.getMaxHp();
            if (percent < bestPercent)
            {
                bestPercent = percent;
                bestIndex = i;
            }
        }

        return bestIndex;
    }

    bool tryMonsterUseRareHealing(Monster& monster, EnemyCombatQueue& wave, int monsterIndex, Random& random)
    {
        if (monster.getHealingPotionCount() <= 0 || !monsterCanUseHealingTools(monster))
        {
            return false;
        }

        int healAmount = 35 + monster.getLevel() * 6;
        int allyIndex = findMostInjuredMonsterAllyIndex(wave, monsterIndex);

        if (allyIndex >= 0)
        {
            Monster& ally = wave.getActiveEnemy(allyIndex);
            int allyPercent = ally.getMaxHp() <= 0 ? 100 : ally.getHp() * 100 / ally.getMaxHp();
            int chance = allyPercent <= 35 ? 70 : 32;

            if (random.between(1, 100) <= chance)
            {
                monster.useHealingPotion(0);
                ally.heal(healAmount);
                monster.markHealingThreat();
                std::cout << monster.getName() << " utilise une potion/technique de soin sur " << ally.getName() << "." << std::endl;
                std::cout << "Ce n'est pas un réflexe animal : seul un ennemi capable de comprendre le soin peut faire ça." << std::endl;
                std::cout << ally.getName() << " récupère " << healAmount << " PV et possède maintenant "
                          << ally.getHp() << "/" << ally.getMaxHp() << " PV." << std::endl;
                std::cout << std::endl;
                return true;
            }
        }

        if (monster.getHp() * 100 > monster.getMaxHp() * 35)
        {
            return false;
        }

        int selfChance = 18;
        if (random.between(1, 100) > selfChance)
        {
            return false;
        }

        std::cout << monster.getName() << " utilise une potion de secours sur lui-même." << std::endl;
        std::cout << "Ce geste ne protège personne d'autre : c'est un pur réflexe de survie." << std::endl;
        monster.useHealingPotion(healAmount);
        std::cout << std::endl;
        return true;
    }

    void displayCoopPartyStatus(const std::vector<Player*>& party, const std::vector<bool>& wasDowned)
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

    std::vector<bool> extractDownedFlags(const std::vector<CoopContribution>& contributions)
    {
        std::vector<bool> flags;
        for (const CoopContribution& contribution : contributions) flags.push_back(contribution.wasDowned);
        return flags;
    }

    CombatReward buildIndividualCoopReward(
        const CombatReward& baseReward,
        const Player& player,
        const Player& sessionLeader,
        const CoopContribution& contribution
    )
    {
        int participation = contribution.turnsTaken > 0 ? 40 : 15;
        participation += std::min(35, contribution.damageDealt / 6);
        participation += std::min(25, contribution.healingDone / 5);
        participation += std::min(20, contribution.damageTaken / 7);
        participation += contribution.supportActions * 8;

        if (contribution.wasDowned)
        {
            participation = std::max(20, participation - 20);
        }

        int levelGap = sessionLeader.getLevel() - player.getLevel();
        if (levelGap >= 25) participation = std::min(participation, 35);
        else if (levelGap >= 15) participation = std::min(participation, 55);
        else if (levelGap >= 10) participation = std::min(participation, 75);

        participation = std::max(0, std::min(100, participation));
        return baseReward.getPercentage(participation);
    }


    bool hasAllyNeedingPotion(const std::vector<Player*>& party, const Player& healer)
    {
        for (Player* ally : party)
        {
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                return true;
            }
        }
        return false;
    }

    bool tryUseHealingPotionOnAlly(Player& healer, std::vector<Player*>& party, int& healingDone)
    {
        if (!hasAllyNeedingPotion(party, healer))
        {
            return false;
        }

        std::vector<int> potionIndices = CombatPotionUtils::getPotionIndices(
            healer,
            ConsumableType::Healing
        );

        if (potionIndices.empty())
        {
            return false;
        }

        std::cout << "Action de soutien disponible pour " << healer.getName() << "." << std::endl;
        std::cout << "0 : Jouer normalement" << std::endl;
        std::cout << "1 : Utiliser une potion de soin sur un allié" << std::endl;
        std::cout << "> ";

        int supportChoice = Console::askNumberBetween(0, 1, "Choisis 0 ou 1.");
        Console::clear();

        if (supportChoice == 0)
        {
            return false;
        }

        std::vector<Player*> targets;
        std::cout << "Choisis l'allié à soigner." << std::endl;
        std::cout << "0 : Annuler" << std::endl;

        for (Player* ally : party)
        {
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                targets.push_back(ally);
                std::cout << targets.size() << " : " << ally->getName();
                if (ally->isDead())
                {
                    std::cout << " [au sol]";
                }
                std::cout << " (" << ally->getHp() << "/" << ally->getMaxHp() << " PV)"
                          << std::endl;
            }
        }

        std::cout << "> ";
        int targetChoice = Console::askNumberBetween(0, static_cast<int>(targets.size()), "Choisis une cible affichée.");
        Console::clear();

        if (targetChoice == 0)
        {
            return false;
        }

        Player* target = targets[targetChoice - 1];

        std::cout << "Choisis la potion à utiliser." << std::endl;
        std::cout << "0 : Annuler" << std::endl;
        for (int i = 0; i < static_cast<int>(potionIndices.size()); ++i)
        {
            Consumable potion = healer.getInventory().getConsumable(potionIndices[i]);
            std::cout << (i + 1) << " : " << potion.getName()
                      << " | soin " << potion.getPower()
                      << std::endl;
        }

        std::cout << "> ";
        int potionChoice = Console::askNumberBetween(0, static_cast<int>(potionIndices.size()), "Choisis une potion affichée.");
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
            std::cout << target->getName() << " est réveillé par la potion avant de récupérer ses forces." << std::endl;
        }
        std::cout << target->getName() << " récupère " << potion.getPower()
                  << " PV et possède maintenant " << target->getHp()
                  << "/" << target->getMaxHp() << " PV." << std::endl;
        std::cout << "Le tour de " << healer.getName() << " est consommé." << std::endl;
        std::cout << std::endl;
        return true;
    }

    void resolveLethalGroupDeathSaves(Player& player, Random& random)
    {
        if (!player.isDead())
        {
            return;
        }

        int green = 0;
        int red = 0;

        std::cout << "Léthal coop : " << player.getName() << " est au sol." << std::endl;
        std::cout << "Les dés de survie commencent : 3 pastilles vertes pour revenir, 3 rouges pour disparaître." << std::endl;
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

            if (green >= 3)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                std::cout << player.getName() << " revient à 1 PV. La mort n'est pas comptée." << std::endl;
                std::cout << std::endl;
                return;
            }
        }

        player.recordDeath();
        std::cout << player.getName() << " reçoit trois pastilles rouges : mort définitive en approche, sauf exception divine ou divination." << std::endl;
        std::cout << std::endl;
    }
}

void MonsterPveMode::runTeam(
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
    std::cout << "========== PvE COOP ==========" << std::endl;
    std::cout << "Joueur principal : " << leader.getName() << std::endl;
    std::cout << "Les données de voyage, niveau de session, événements et monstres suivent le joueur 1." << std::endl;
    std::cout << "Les récompenses resteront individuelles selon participation, chance et écart de niveau." << std::endl;
    std::cout << std::endl;
    CombatGroupBuilder::displayFormationRules();

    WaveCombatSystem::displayWaveIntroduction();
    EnemyCombatQueue wave = WaveCombatSystem::createWaveForPlayer(leader, random, difficulty);

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);

    std::vector<int> initialHp;
    std::vector<CoopContribution> contributions(party.size());
    std::vector<std::vector<Summon>> partySummons(party.size());
    std::vector<SummonControlMode> summonControlModes(party.size(), SummonControlMode::Automatic);

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        initialHp.push_back(player != nullptr ? player->getHp() : 0);

        if (player != nullptr)
        {
            partySummons[i] = SummonCombatSystem::createInitialSummonsFor(*player);
            SummonCombatSystem::displaySummonArrival(*player, partySummons[i]);
            summonControlModes[i] = SummonCombatSystem::askPlayerSummonControlMode(*player, partySummons[i]);
        }
    }

    bool escapeSucceeded = false;
    int round = 1;

    while (countAlivePlayers(party) > 0 && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        std::cout << "========== TOUR DE GROUPE " << round << " ==========" << std::endl;
        std::cout << std::endl;
        displayCoopPartyStatus(party, extractDownedFlags(contributions));

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead() || !wave.hasEnemiesLeft() || escapeSucceeded)
            {
                continue;
            }

            std::cout << "Tour de " << player->getName() << " [joueur " << (i + 1) << "]." << std::endl;
            std::cout << std::endl;

            int healingDoneThisTurn = 0;
            int enemyHpBeforeTurn = sumActiveEnemyHp(wave);
            bool finished = tryUseHealingPotionOnAlly(*player, party, healingDoneThisTurn);
            while (!finished && !player->isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
            {
                finished = PlayerWaveCombatTurn::play(
                    *player,
                    wave,
                    random,
                    escapeSucceeded,
                    difficulty
                );
            }

            if (finished)
            {
                if (wave.hasEnemiesLeft() && SummonCombatSystem::hasActiveSummons(partySummons[i]))
                {
                    int enemyHpBeforeSummons = sumActiveEnemyHp(wave);
                    SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                        partySummons[i],
                        wave,
                        random,
                        summonControlModes[i]
                    );
                    int summonDamage = std::max(0, enemyHpBeforeSummons - sumActiveEnemyHp(wave));
                    if (summonDamage > 0)
                    {
                        contributions[i].supportActions++;
                    }
                }

                int enemyHpAfterTurn = sumActiveEnemyHp(wave);
                contributions[i].turnsTaken++;
                contributions[i].damageDealt += std::max(0, enemyHpBeforeTurn - enemyHpAfterTurn);
                contributions[i].healingDone += healingDoneThisTurn;
                if (healingDoneThisTurn > 0) contributions[i].supportActions++;
            }
        }

        if (!wave.hasEnemiesLeft() || escapeSucceeded)
        {
            break;
        }

        int enemyIndex = 0;
        while (enemyIndex < wave.getActiveEnemyCount() && countAlivePlayers(party) > 0)
        {
            if (!wave.isActiveIndexValid(enemyIndex))
            {
                ++enemyIndex;
                continue;
            }

            Monster& monster = wave.getActiveEnemy(enemyIndex);
            if (monster.isDead())
            {
                ++enemyIndex;
                continue;
            }

            Player* target = chooseAlivePlayerTarget(party, random, &contributions);
            if (target == nullptr)
            {
                break;
            }

            std::cout << "Tour de " << monster.getName() << " : cible " << target->getName() << "." << std::endl;
            if (!tryMonsterUseRareHealing(monster, wave, enemyIndex, random))
            {
                int targetHpBefore = target->getHp();
                TurnManager::executeAttack(monster, *target, random);
                for (std::size_t partyIndex = 0; partyIndex < party.size(); ++partyIndex)
                {
                    if (party[partyIndex] == target)
                    {
                        contributions[partyIndex].damageTaken += std::max(0, targetHpBefore - target->getHp());
                        break;
                    }
                }
            }

            for (std::size_t i = 0; i < party.size(); ++i)
            {
                if (party[i] != nullptr && party[i]->isDead())
                {
                    contributions[i].wasDowned = true;
                }
            }

            Console::pauseSeconds(1);
            ++enemyIndex;
        }

        wave.removeDeadAndReplace();
        ++round;
    }

    if (escapeSucceeded)
    {
        std::cout << "Le groupe a ouvert une sortie." << std::endl;
        std::cout << "Chaque personnage récupère seulement une part de ce qu'il a réellement aidé à obtenir." << std::endl;
        std::cout << std::endl;
    }

    if (countAlivePlayers(party) == 0 && wave.hasEnemiesLeft())
    {
        std::cout << "Tout le groupe est tombé." << std::endl;
        std::cout << std::endl;

        for (Player* player : party)
        {
            if (player == nullptr) continue;

            player->recordDefeat();
            if (DifficultyRules::isPermanentDeath(difficulty))
            {
                resolveLethalGroupDeathSaves(*player, random);
            }
            else
            {
                player->recordDeath();
                player->reviveWithHealthPercentage(
                    DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
                );
            }
        }

        return;
    }

    displaySpecialDefeatDialogues(wave);
    CombatReward baseReward = escapeSucceeded
        ? CombatRewardSystem::calculatePlayerEscapeReward(wave, difficulty)
        : CombatRewardSystem::calculateWaveReward(wave, difficulty, leader, initialHp[0], round, random);

    std::cout << "========== RÉCOMPENSES INDIVIDUELLES COOP ==========" << std::endl;

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        if (player == nullptr)
        {
            continue;
        }

        if (player->isDead() && !DifficultyRules::isPermanentDeath(difficulty))
        {
            player->recordDeath();
            player->reviveWithHealthPercentage(
                DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
            );
            std::cout << player->getName() << " est réveillé à la fin du combat. La mort est comptabilisée." << std::endl;
        }

        if (player->isDead())
        {
            std::cout << player->getName() << " reste au sol : aucune récompense supplémentaire après sa chute." << std::endl;
            continue;
        }

        CombatReward individualReward = buildIndividualCoopReward(
            baseReward,
            *player,
            leader,
            contributions[i]
        );

        std::cout << player->getName() << " :" << std::endl;
        CombatRewardSystem::displayReward(individualReward);
        CombatRewardSystem::giveRewardToPlayer(*player, individualReward);
        player->recordVictory();
        player->recordEnemyKills(wave.getDefeatedEnemyCount());
        std::cout << "Participation : tours " << contributions[i].turnsTaken
                  << ", dégâts " << contributions[i].damageDealt
                  << ", soins " << contributions[i].healingDone
                  << ", dégâts encaissés " << contributions[i].damageTaken << "." << std::endl;
        LootGenerator::giveDefeatedWaveLoot(*player, wave, random, difficulty);
    }

    recordWaveKillsInBestiary(wave);
}
