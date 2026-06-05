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
#include "interface/CombatDisplay.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
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
    bool isSpecialCharacterMonster(const Monster& monster)
    {
        return SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName());
    }

    std::string buildMonsterBestiaryCategory(const Monster& monster)
    {
        if (isSpecialCharacterMonster(monster))
        {
            return "Personnages spéciaux";
        }

        return "Entités hostiles / ennemis";
    }

    std::string buildMonsterBestiaryDescription(const Monster& monster)
    {
        std::string description = monster.getName()
            + " | Race : "
            + monster.getRaceText()
            + " | Niveau : "
            + std::to_string(monster.getLevel())
            + ".";

        if (isSpecialCharacterMonster(monster))
        {
            description += " Personnage spécial découvert en rencontre PvE/arène. Le registre révèle son nom seulement après rencontre réelle, pas gratuitement dès le départ.";
        }

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

    std::string toLowerForDialogue(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text;
    }

    bool textContainsAny(const std::string& text, const std::vector<std::string>& needles)
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

    bool looksMindlessForDialogue(const Monster& monster)
    {
        const std::string text = toLowerForDialogue(monster.getName() + " " + monster.getType());
        return textContainsAny(text, {
            "zombie",
            "squelette",
            "ossement",
            "cadavre",
            "carcasse",
            "automate",
            "golem",
            "tourelle"
        });
    }

    bool raceCanSpeakForDialogue(Race race)
    {
        switch (race)
        {
            case Race::Humain:
            case Race::SemiHumain:
            case Race::Elfe:
            case Race::ElfeNoir:
            case Race::Nain:
            case Race::Gnome:
            case Race::Halfelin:
            case Race::Tieffelin:
            case Race::Aasimar:
            case Race::Kitsune:
            case Race::Fee:
            case Race::SemiDragon:
            case Race::Gobelin:
            case Race::Hobgobelin:
            case Race::Orc:
            case Race::Demon:
            case Race::Ange:
            case Race::Dragon:
            case Race::Draconide:
            case Race::Esprit:
            case Race::Aberration:
            case Race::AnomalieArcanique:
                return true;
            default:
                return false;
        }
    }

    std::string languageFamilyForMonsterRace(Race race)
    {
        switch (race)
        {
            case Race::Gobelin:
            case Race::Hobgobelin:
                return "gobelin";
            case Race::Orc:
                return "orc";
            case Race::Demon:
            case Race::Tieffelin:
                return "infernal";
            case Race::Dragon:
            case Race::Draconide:
            case Race::SemiDragon:
                return "draconique";
            case Race::Elfe:
            case Race::ElfeNoir:
            case Race::Fee:
            case Race::Kitsune:
            case Race::Esprit:
                return "sylvain";
            case Race::Ange:
            case Race::Aasimar:
                return "céleste";
            case Race::Aberration:
            case Race::AnomalieArcanique:
                return "anormal";
            default:
                return "commun";
        }
    }

    std::string languageFamilyForPlayerRace(CharacterRace race)
    {
        switch (race)
        {
            case CharacterRace::Orc:
                return "orc";
            case CharacterRace::Demon:
            case CharacterRace::Tiefling:
                return "infernal";
            case CharacterRace::HalfDragon:
                return "draconique";
            case CharacterRace::Elf:
            case CharacterRace::DarkElf:
            case CharacterRace::Fairy:
            case CharacterRace::Kitsune:
                return "sylvain";
            case CharacterRace::Aasimar:
                return "céleste";
            default:
                return "commun";
        }
    }

    bool playerUnderstandsMonsterRace(const Player& player, Race monsterRace)
    {
        const std::string monsterLanguage = languageFamilyForMonsterRace(monsterRace);
        if (monsterLanguage == "commun")
        {
            return true;
        }

        const std::string playerLanguage = languageFamilyForPlayerRace(player.getRace());
        if (playerLanguage == monsterLanguage)
        {
            return true;
        }

        // FR: certaines proximités de lore donnent assez de bases pour comprendre le ton et les mots simples.
        // EN: some lore-adjacent races understand enough simple wording to make the line readable.
        return (playerLanguage == "commun" && (monsterLanguage == "sylvain" || monsterLanguage == "céleste"));
    }

    std::string foreignDialogueLineForRace(Race race, Random& random)
    {
        const std::string language = languageFamilyForMonsterRace(race);

        if (language == "gobelin")
        {
            return random.between(1, 2) == 1 ? "\"Grik tak narok ! Skree val !\"" : "\"Rakka-til ! Nosh griba !\"";
        }
        if (language == "orc")
        {
            return random.between(1, 2) == 1 ? "\"Urg drah kor. Mak'thar !\"" : "\"Gor ash muk !\"";
        }
        if (language == "infernal")
        {
            return random.between(1, 2) == 1 ? "\"Vel'khara noss tiren...\"" : "\"Shaal ven dorakh.\"";
        }
        if (language == "draconique")
        {
            return random.between(1, 2) == 1 ? "\"Tharun vek siira.\"" : "\"Kraav nor elthar.\"";
        }
        if (language == "sylvain")
        {
            return random.between(1, 2) == 1 ? "\"Leth aen silva, mori.\"" : "\"Elyn thar vae.\"";
        }
        if (language == "céleste")
        {
            return random.between(1, 2) == 1 ? "\"Aurel na venia.\"" : "\"Lum aster, solenne.\"";
        }
        if (language == "anormal")
        {
            return random.between(1, 2) == 1 ? "\"// voix non conforme // sujet observé //\"" : "\"La phrase se plie avant d'atteindre tes oreilles.\"";
        }

        return "\"...\"";
    }

    std::string randomLineFromChoices(const std::vector<std::string>& choices, Random& random)
    {
        if (choices.empty())
        {
            return "\"...\"";
        }

        return choices[static_cast<std::size_t>(random.between(0, static_cast<int>(choices.size()) - 1))];
    }

    std::string understoodDialogueLineForRace(Race race, Random& random)
    {
        if (race == Race::Gobelin || race == Race::Hobgobelin)
        {
            return randomLineFromChoices({
                "\"Tu as l'air d'avoir une bourse et peu d'amis. Mauvaise combinaison.\"",
                "\"On prend les vivants, les sacs, puis on discute du reste.\"",
                "\"Pas besoin d'être grand pour compter l'or mieux que toi.\""
            }, random);
        }
        if (race == Race::Orc)
        {
            return randomLineFromChoices({
                "\"Tiens ta ligne. Si tu recules, je le verrai.\"",
                "\"Un bon combat vaut mieux qu'une longue excuse.\"",
                "\"Crie si tu veux. Le bruit ne bloque pas les haches.\""
            }, random);
        }
        if (race == Race::Demon)
        {
            return randomLineFromChoices({
                "\"Ta peur fait plus de bruit que ton arme.\"",
                "\"Approche. Les pactes les plus courts sont les plus honnêtes.\"",
                "\"Je ne promets rien. C'est déjà plus franc que la plupart des contrats.\""
            }, random);
        }
        if (race == Race::Dragon || race == Race::Draconide || race == Race::SemiDragon)
        {
            return randomLineFromChoices({
                "\"Chaque pas de plus sera gravé dans tes os.\"",
                "\"Je respecte le courage. Je punis l'arrogance.\"",
                "\"Un souffle suffit parfois à corriger une légende trop sûre d'elle.\""
            }, random);
        }
        if (race == Race::Elfe || race == Race::ElfeNoir || race == Race::Fee || race == Race::Kitsune)
        {
            return randomLineFromChoices({
                "\"La forêt t'a laissé entrer. Elle ne t'a pas promis la sortie.\"",
                "\"Joli pas. Mauvais silence.\"",
                "\"Tu portes l'odeur des chemins qui dérangent les anciens lieux.\""
            }, random);
        }
        if (race == Race::Ange || race == Race::Aasimar)
        {
            return randomLineFromChoices({
                "\"La lumière n'excuse pas tout. Elle révèle surtout ce que tu fais maintenant.\"",
                "\"Avance proprement, ou tombe proprement.\"",
                "\"Même la grâce garde une lame pour les intrus.\""
            }, random);
        }
        if (race == Race::Esprit || race == Race::AnomalieArcanique || race == Race::Aberration)
        {
            return randomLineFromChoices({
                "\"Tu entres dans une histoire qui ne t'a pas encore choisi.\"",
                "\"Ton nom tremble dans la marge du monde.\"",
                "\"Je parle depuis un endroit où tes règles arrivent en retard.\""
            }, random);
        }

        return randomLineFromChoices({
            "\"Pas un pas de plus. Les problèmes commencent toujours comme ça.\"",
            "\"Rentre chez toi pendant que tu as encore assez de jambes pour le faire.\"",
            "\"Je ne te connais pas. Ça rendra ce combat plus simple.\""
        }, random);
    }

    const Monster* pickDialogueCandidateFromWave(const EnemyCombatQueue& wave, Random& random)
    {
        std::vector<const Monster*> candidates;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& monster = wave.getActiveEnemy(i);
            if (raceCanSpeakForDialogue(monster.getRace()) && !looksMindlessForDialogue(monster))
            {
                candidates.push_back(&monster);
            }
        }

        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            const Monster& monster = wave.getWaitingEnemy(i);
            if (raceCanSpeakForDialogue(monster.getRace()) && !looksMindlessForDialogue(monster))
            {
                candidates.push_back(&monster);
            }
        }

        if (candidates.empty())
        {
            return nullptr;
        }

        return candidates[static_cast<std::size_t>(random.between(0, static_cast<int>(candidates.size()) - 1))];
    }

    void displayEncounterDialogue(const Player& player, const EnemyCombatQueue& wave, Random& random, const std::string& screenIdPrefix)
    {
        const Monster* speaker = pickDialogueCandidateFromWave(wave, random);
        if (speaker == nullptr)
        {
            return;
        }

        const bool understood = playerUnderstandsMonsterRace(player, speaker->getRace());
        const std::string language = languageFamilyForMonsterRace(speaker->getRace());
        std::vector<std::string> lines;
        lines.push_back("Interlocuteur : " + speaker->getName());
        lines.push_back("Langue probable : " + language);
        lines.push_back(std::string("Compréhension : ") + (understood ? "mots compris" : "mots non compris"));
        lines.push_back("");
        lines.push_back(speaker->getName() + " s'avance assez pour parler avant que le combat ne commence vraiment.");

        if (understood)
        {
            lines.push_back(understoodDialogueLineForRace(speaker->getRace(), random));
            lines.push_back("Tu comprends les mots. Le ton, lui, ne laisse pas beaucoup de place à la négociation.");
        }
        else
        {
            lines.push_back(foreignDialogueLineForRace(speaker->getRace(), random));
            lines.push_back("Tu ne comprends pas les mots, mais l'intention hostile passe très bien.");
        }

        MessageScreen::show(
            "DIALOGUE D'INTRODUCTION",
            screenIdPrefix + ".enemy_dialogue",
            lines,
            false
        );
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
                buildMonsterBestiaryCategory(monster),
                buildMonsterBestiaryDescription(monster)
            );
        }

        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            const Monster& monster = wave.getWaitingEnemy(i);
            BestiaryRuntimeProgress::recordEncounter(
                monster.getName(),
                buildMonsterBestiaryCategory(monster),
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
                buildMonsterBestiaryCategory(monster),
                buildMonsterBestiaryDescription(monster)
            );
        }
    }

    void displayWaveCombatSnapshot(
        const Player& player,
        const EnemyCombatQueue& wave,
        const std::vector<Summon>& summons,
        const std::string& title,
        const std::string& phase,
        int turnNumber,
        bool waitAndClear = false
    )
    {
        GuiCombatStateSnapshot snapshot = CombatDisplay::buildWaveSnapshot(
            player,
            wave,
            summons,
            title,
            phase,
            turnNumber
        );

        if (!player.isDead())
        {
            snapshot.currentActorName = player.getName();
        }

        CombatDisplay::displayCombatState(snapshot, waitAndClear);
    }

    void showCombatPhaseGate(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MessageScreen::show(title, screenId, lines, true);
    }


    void showPostCombatRouteScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MenuScreen screen(title, screenId);
        screen.addSubtitle("Bilan de route de combat");

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        screen.setDisplayOnlyInput("Résumé affiché sans saisie directe.");
        TerminalInterface::renderMenuScreen(screen, false);
    }

}

void MonsterPveMode::run(
    Player& player,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule
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
    displayEncounterDialogue(player, wave, random, "pve.encounter");

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
            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DU COMBAT",
                "Tour du joueur",
                combatTurnCount + 1
            );

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
                MessageScreen::show(
                    "ACTION NON CONSOMMÉE",
                    "combat.pve.turn.not_consumed",
                    {"Ton tour n'est pas encore consommé."},
                    true
                );
            }
        }

        if (playerTurnFinished && !escapeSucceeded && !player.isDead())
        {
            showCombatPhaseGate(
                "FIN DU TOUR JOUEUR",
                "combat.pve.phase.player_resolved",
                {"Action du joueur résolue.", "Valide pour passer à la phase suivante."}
            );
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
            showCombatPhaseGate(
                "TOUR DES INVOCATIONS ALLIÉES",
                "combat.pve.phase.player_summons",
                {"Les invocations alliées encore liées agissent séparément du joueur."}
            );
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            showCombatPhaseGate(
                "TOUR DES ENNEMIS VIVANTS",
                "combat.pve.phase.enemies",
                {"Les ennemis actifs agissent maintenant, un groupe après l'autre."}
            );
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );

            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DU COMBAT",
                "Après la riposte ennemie",
                combatTurnCount,
                true
            );
        }
    }

    if (escapeSucceeded)
    {
        showPostCombatRouteScreen(
            "FUITE RÉUSSIE",
            "combat.pve.escape.success",
            {
                "Résultat : fuite validée",
                "Route : combat PvE",
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Récompense : partielle, basée seulement sur ce qui s'est réellement passé",
                "Conséquence : aucune mort enregistrée"
            }
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
        showPostCombatRouteScreen(
            "DÉFAITE",
            "combat.pve.defeat",
            {
                "Résultat : défaite",
                "Route : combat PvE",
                "Personnage : " + player.getName(),
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus avant chute : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Mort : enregistrée selon la difficulté",
                "Action suivante : application de la règle de mort"
            }
        );

        player.recordDefeat();
        player.recordDeath();

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
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

        showPostCombatRouteScreen(
            "RETOUR À LA VIE",
            "combat.pve.revive",
            {
                "Résultat : personnage restauré",
                "Personnage : " + player.getName(),
                "PV après retour : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                "Difficulté : mort non définitive",
                "Conséquence : pénalité de mort déjà appliquée",
                "Note : tu as survécu, mais la mort a laissé sa trace"
            }
        );

        return;
    }

    displaySpecialDefeatDialogues(wave);

    showPostCombatRouteScreen(
        "VICTOIRE PVE",
        "combat.pve.victory",
        {
            "Résultat : victoire",
            "Route : combat PvE",
            "Personnage : " + player.getName(),
            "Tours joués : " + std::to_string(combatTurnCount),
            "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
            "Action suivante : récompenses complètes puis butin"
        }
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
            MessageScreen::show(
                "PROGRESSION BESTIAIRE",
                "combat.pve.evolved_quest.progress",
                {"Le bestiaire et les quêtes liées aux créatures évoluées progressent."},
                false
            );
        }
    }
}


bool MonsterPveMode::runExplorationWave(
    Player& player,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule,
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

    MessageScreen::show(
        "ÉVÉNEMENT D'EXPLORATION",
        "exploration.wave.intro",
        {
            title,
            "La rencontre se referme autour de toi : il faut tenir la ligne."
        },
        false
    );

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);
    displayEncounterDialogue(player, wave, random, "exploration.wave");

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
            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DE L'ÉVÉNEMENT",
                "Tour du joueur",
                combatTurnCount + 1
            );

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
                MessageScreen::show(
                    "ACTION NON CONSOMMÉE",
                    "exploration.wave.turn.not_consumed",
                    {"Ton tour n'est pas encore consommé."},
                    true
                );
            }
        }

        if (playerTurnFinished && !escapeSucceeded && !player.isDead())
        {
            showCombatPhaseGate(
                "FIN DU TOUR JOUEUR",
                "exploration.wave.phase.player_resolved",
                {"Action du joueur résolue.", "Valide pour passer à la phase suivante."}
            );
        }

        if (!player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
            && SummonCombatSystem::hasActiveSummons(playerSummons))
        {
            showCombatPhaseGate(
                "TOUR DES INVOCATIONS ALLIÉES",
                "exploration.wave.phase.player_summons",
                {"Les invocations alliées encore liées agissent avant les ennemis de l'événement."}
            );
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            showCombatPhaseGate(
                "TOUR DES ENNEMIS VIVANTS",
                "exploration.wave.phase.enemies",
                {"Les ennemis actifs de l'événement agissent maintenant."}
            );
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );

            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DE L'ÉVÉNEMENT",
                "Après la riposte ennemie",
                combatTurnCount,
                true
            );
        }
    }

    if (escapeSucceeded)
    {
        showPostCombatRouteScreen(
            "FUITE D'EXPLORATION",
            "exploration.wave.escape.success",
            {
                "Résultat : fuite d'événement",
                "Route : exploration",
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Récompense : partielle, actions réelles uniquement",
                "Conséquence : exploration interrompue proprement"
            }
        );

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
        showPostCombatRouteScreen(
            "DÉFAITE D'EXPLORATION",
            "exploration.wave.defeat",
            {
                "Résultat : défaite d'événement",
                "Route : exploration",
                "Personnage : " + player.getName(),
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus avant chute : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Mort : enregistrée selon la difficulté",
                "Action suivante : application de la règle de mort"
            }
        );

        player.recordDefeat();
        player.recordDeath();

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
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

        showPostCombatRouteScreen(
            "RETOUR À LA VIE",
            "exploration.wave.revive",
            {
                "Résultat : personnage restauré",
                "Route : exploration",
                "Personnage : " + player.getName(),
                "PV après retour : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                "Difficulté : mort non définitive",
                "Conséquence : pénalité de mort déjà appliquée"
            }
        );
        return false;
    }

    displaySpecialDefeatDialogues(wave);

    showPostCombatRouteScreen(
        "ÉVÉNEMENT TERMINÉ",
        "exploration.wave.victory",
        {
            "Résultat : événement terminé",
            "Route : exploration",
            "Personnage : " + player.getName(),
            "Tours joués : " + std::to_string(combatTurnCount),
            "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
            "Action suivante : récompenses d'exploration"
        }
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
            MessageScreen::show(
                "PROGRESSION BESTIAIRE",
                "exploration.wave.evolved_quest.progress",
                {"Le bestiaire et les quêtes liées aux créatures évoluées progressent."},
                false
            );
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

    std::vector<Summon> flattenCoopSummons(const std::vector<std::vector<Summon>>& partySummons)
    {
        std::vector<Summon> flattened;

        for (const std::vector<Summon>& summons : partySummons)
        {
            for (const Summon& summon : summons)
            {
                if (!summon.isDead() && !summon.isExpired())
                {
                    flattened.push_back(summon);
                }
            }
        }

        return flattened;
    }

    void displayPartyWaveCombatSnapshot(
        std::vector<Player*>& party,
        const EnemyCombatQueue& wave,
        const std::vector<std::vector<Summon>>& partySummons,
        const std::string& phase,
        int round,
        const std::string& actorName = ""
    )
    {
        std::vector<Entity*> entities;
        for (Player* player : party)
        {
            if (player != nullptr)
            {
                entities.push_back(player);
            }
        }

        std::vector<Summon> flattenedSummons = flattenCoopSummons(partySummons);
        GuiCombatStateSnapshot snapshot = CombatDisplay::buildWavePartySnapshot(
            entities,
            wave,
            flattenedSummons,
            "ÉTAT DU COMBAT COOP",
            phase,
            round
        );

        snapshot.currentActorName = actorName;
        CombatDisplay::displayCombatState(snapshot, false);
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
                MessageScreen::show(
                    "SOIN ENNEMI",
                    "pve.monster.healing.ally",
                    {
                        monster.getName() + " utilise une potion/technique de soin sur " + ally.getName() + ".",
                        "Ce n'est pas un réflexe animal : seul un ennemi capable de comprendre le soin peut faire ça.",
                        ally.getName() + " récupère " + std::to_string(healAmount) + " PV et possède maintenant " + std::to_string(ally.getHp()) + "/" + std::to_string(ally.getMaxHp()) + " PV."
                    },
                    false
                );
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

        monster.useHealingPotion(healAmount);
        MessageScreen::show(
            "SOIN ENNEMI",
            "pve.monster.healing.self",
            {
                monster.getName() + " utilise une potion de secours sur lui-même.",
                "Ce geste ne protège personne d'autre : c'est un pur réflexe de survie.",
                "PV actuels : " + std::to_string(monster.getHp()) + "/" + std::to_string(monster.getMaxHp()) + "."
            },
            false
        );
        return true;
    }

    void displayCoopPartyStatus(const std::vector<Player*>& party, const std::vector<bool>& wasDowned)
    {
        std::vector<std::string> lines;
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr)
            {
                continue;
            }

            std::string line = "J" + std::to_string(i + 1)
                + " [" + CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(i)) + "] - "
                + player->getName()
                + " : " + std::to_string(player->getHp()) + "/" + std::to_string(player->getMaxHp()) + " PV";
            if (player->isDead())
            {
                line += " [au sol]";
            }
            else if (i < wasDowned.size() && wasDowned[i])
            {
                line += " [a déjà chuté]";
            }
            lines.push_back(line);
        }

        MessageScreen::show("ÉTAT DU GROUPE", "pve.coop.party_status", lines, false);
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

    constexpr std::size_t PVE_PARTY_SUPPORT_PAGE_SIZE = 8;

    MenuOptionItemData makePvePartySupportData(
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
        itemData.kind = "pve_party_support";
        itemData.section = "Soutien PvE coop";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = healer.getName();
        itemData.progress = "PV : " + std::to_string(healer.getHp()) + "/" + std::to_string(healer.getMaxHp());
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makePvePartyHealingTargetData(
        const Player& healer,
        const Player& target,
        std::size_t partyIndex
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "ally";
        itemData.section = "Cibles de soin PvE";
        itemData.actionType = "support";
        itemData.name = target.getName();
        itemData.detail = target.isDead() ? "Allié au sol à relever" : "Allié blessé à soigner";
        itemData.status = "PV : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp());
        itemData.owner = healer.getName();
        itemData.progress = "J" + std::to_string(partyIndex + 1)
            + " - " + CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(partyIndex));
        itemData.important = target.isDead()
            || (target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 35);
        return itemData;
    }

    MenuOptionItemData makePvePartyPotionData(
        const Player& healer,
        const Consumable& potion,
        int inventoryIndex,
        int amount = 1
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "potion";
        itemData.section = "Potions de soutien PvE";
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

        MenuScreen supportScreen("SOUTIEN D'ÉQUIPE", "pve.party.support.choice");
        supportScreen.addSubtitle("Tour de " + healer.getName());
        supportScreen.addLine("Un allié peut recevoir une potion de soin avant l'action normale.");
        supportScreen.addOption(
            0,
            "Jouer normalement",
            "Ne consomme pas de potion.",
            true,
            "party.support.skip",
            makePvePartySupportData(healer, "skip", "Jouer normalement", "Ne consomme pas de potion.", "Action normale")
        );
        supportScreen.addOption(
            1,
            "Utiliser une potion de soin sur un allié",
            "Consomme le tour de soutien de " + healer.getName() + ".",
            true,
            "party.support.heal_ally",
            makePvePartySupportData(healer, "heal", "Potion de soutien", "Soigner ou relever un allié avant l'action normale.", "Consomme le tour", true)
        );
        int supportChoice = TerminalInterface::askMenuChoiceFromOptions(supportScreen, "Choisis une option affichée.");
        Console::clear();

        if (supportChoice == 0)
        {
            return false;
        }

        std::vector<Player*> targets;
        std::vector<std::size_t> targetPartyIndexes;
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* ally = party[i];
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                targets.push_back(ally);
                targetPartyIndexes.push_back(i);
            }
        }

        if (targets.empty())
        {
            return false;
        }

        std::size_t targetPageIndex = 0;
        Player* target = nullptr;
        while (target == nullptr)
        {
            const std::size_t totalPages = PagedMenu::pageCount(targets.size(), PVE_PARTY_SUPPORT_PAGE_SIZE);
            if (targetPageIndex >= totalPages) targetPageIndex = totalPages - 1;
            const std::size_t firstIndex = PagedMenu::firstIndex(targetPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(targets.size(), targetPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);

            MenuScreen targetScreen("CHOIX DE L'ALLIÉ", "pve.party.support.target");
            targetScreen.addSubtitle("Potion de soutien de " + healer.getName());
            targetScreen.addLine("Alliés affichés : " + PagedMenu::rangeText(firstIndex, lastIndex, targets.size()));
            targetScreen.addLine("Choisis l'allié à soigner ou à relever.");
            targetScreen.addOption(
                0,
                "Annuler",
                "Retour au tour normal.",
                true,
                "party.support.target.cancel",
                makePvePartySupportData(healer, "cancel", "Annuler", "Retour au tour normal.", "Annulé")
            );

            for (std::size_t i = firstIndex; i < lastIndex; ++i)
            {
                Player* ally = targets[i];
                std::string label = ally->getName();
                if (ally->isDead())
                {
                    label += " [au sol]";
                }
                targetScreen.addOption(
                    static_cast<int>(i - firstIndex + 1),
                    label,
                    std::to_string(ally->getHp()) + "/" + std::to_string(ally->getMaxHp()) + " PV",
                    true,
                    "party.support.target",
                    makePvePartyHealingTargetData(healer, *ally, targetPartyIndexes[i])
                );
            }
            PagedMenu::addNavigationOptions(targetScreen, targetPageIndex, totalPages);

            int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une cible affichée.");
            Console::clear();

            if (targetChoice == 0)
            {
                return false;
            }
            if (targetChoice == 98 && targetPageIndex > 0)
            {
                --targetPageIndex;
                continue;
            }
            if (targetChoice == 99 && targetPageIndex + 1 < totalPages)
            {
                ++targetPageIndex;
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(targetChoice - 1);
            if (selectedIndex < targets.size() && selectedIndex < lastIndex)
            {
                target = targets[selectedIndex];
            }
        }

        std::size_t potionPageIndex = 0;
        int consumableIndex = -1;
        while (consumableIndex < 0)
        {
            std::vector<PotionStack> potionStacks = CombatPotionUtils::groupPotionIndices(healer, potionIndices);
            if (potionStacks.empty())
            {
                return false;
            }

            const std::size_t totalPages = PagedMenu::pageCount(potionStacks.size(), PVE_PARTY_SUPPORT_PAGE_SIZE);
            if (potionPageIndex >= totalPages) potionPageIndex = totalPages - 1;
            const std::size_t firstIndex = PagedMenu::firstIndex(potionPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(potionStacks.size(), potionPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);

            MenuScreen potionScreen("CHOIX DE LA POTION", "pve.party.support.potion");
            potionScreen.addSubtitle("Cible : " + target->getName());
            potionScreen.addLine("Piles affichées : " + PagedMenu::rangeText(firstIndex, lastIndex, potionStacks.size()));
            potionScreen.addLine("Choisis la potion de soin à utiliser.");
            potionScreen.addOption(
                0,
                "Annuler",
                "Ne consomme rien.",
                true,
                "party.support.potion.cancel",
                makePvePartySupportData(healer, "cancel", "Annuler", "Ne consomme rien.", "Annulé")
            );
            for (std::size_t i = firstIndex; i < lastIndex; ++i)
            {
                const PotionStack& stack = potionStacks[i];
                Consumable potion = healer.getInventory().getConsumable(stack.firstIndex);
                potionScreen.addOption(
                    static_cast<int>(i - firstIndex + 1),
                    CombatPotionUtils::stackLabel(potion.getName(), stack.amount),
                    "Soin : " + potion.getPowerDisplayText() + " | Quantité : " + std::to_string(stack.amount),
                    true,
                    "party.support.potion.healing",
                    makePvePartyPotionData(healer, potion, stack.firstIndex, stack.amount)
                );
            }
            PagedMenu::addNavigationOptions(potionScreen, potionPageIndex, totalPages);

            int potionChoice = TerminalInterface::askMenuChoiceFromOptions(potionScreen, "Choisis une potion affichée.");
            Console::clear();

            if (potionChoice == 0)
            {
                return false;
            }
            if (potionChoice == 98 && potionPageIndex > 0)
            {
                --potionPageIndex;
                continue;
            }
            if (potionChoice == 99 && potionPageIndex + 1 < totalPages)
            {
                ++potionPageIndex;
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(potionChoice - 1);
            if (selectedIndex < potionStacks.size() && selectedIndex < lastIndex)
            {
                consumableIndex = potionStacks[selectedIndex].firstIndex;
            }
        }
        if (!healer.getInventory().hasConsumable(consumableIndex))
        {
            MessageScreen::show(
                "POTION INTROUVABLE",
                "pve.party.support.potion.missing",
                {
                    "Cette potion n'est plus disponible.",
                    "Le soutien est annulé."
                },
                false
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

        if (!healer.hasInfiniteConsumables())
        {
            healer.getInventory().removeConsumable(consumableIndex);
        }

        std::vector<std::string> resultLines;
        resultLines.push_back(healer.getName() + " devient soigneur ce tour-ci.");
        resultLines.push_back("Potion utilisée : " + potion.getName() + ".");
        resultLines.push_back("Cible : " + target->getName() + ".");
        if (revivedTarget)
        {
            resultLines.push_back(target->getName() + " est réveillé par la potion avant de récupérer ses forces.");
        }
        resultLines.push_back(target->getName() + " récupère " + std::to_string(target->getHp() - beforeHealHp) + " PV (soin annoncé : " + potion.getPowerDisplayText() + ").");
        resultLines.push_back("PV actuels : " + std::to_string(target->getHp()) + "/" + std::to_string(target->getMaxHp()) + ".");
        resultLines.push_back("Le tour de " + healer.getName() + " est consommé.");
        MessageScreen::show("SOUTIEN RÉUSSI", "pve.party.support.result", resultLines, false);
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
        std::vector<std::string> lines;
        lines.push_back("Mort définitive coop : " + player.getName() + " est au sol.");
        lines.push_back("Les dés de survie commencent : 3 pastilles vertes pour revenir, 3 rouges pour disparaître.");

        while (green < 3 && red < 3)
        {
            int roll = random.between(1, 20);
            lines.push_back("Dé de survie : " + std::to_string(roll) + ".");

            if (roll == 20)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                lines.push_back("20 naturel : " + player.getName() + " se relève immédiatement à 1 PV et pourra rejouer.");
                MessageScreen::show("SURVIE EN MORT DÉFINITIVE COOP", "pve.coop.lethal_death_save.success_natural", lines, false);
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

            if (green >= 3)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                lines.push_back(player.getName() + " revient à 1 PV. La mort n'est pas comptée.");
                MessageScreen::show("SURVIE EN MORT DÉFINITIVE COOP", "pve.coop.lethal_death_save.success", lines, false);
                return;
            }
        }

        player.recordDeath();
        lines.push_back(player.getName() + " reçoit trois pastilles rouges : mort définitive en approche, sauf exception divine ou divination.");
        MessageScreen::show("SURVIE EN MORT DÉFINITIVE COOP", "pve.coop.lethal_death_save.failure", lines, false);
    }
}
void MonsterPveMode::runTeam(
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
        "PvE COOP",
        "pve.coop.intro",
        {
            "Joueur principal : " + leader.getName() + ".",
            "Les données de voyage, niveau de session, événements et monstres suivent le joueur 1.",
            "Les récompenses resteront individuelles selon participation, chance et écart de niveau."
        },
        false
    );
    CombatGroupBuilder::displayFormationRules();

    WaveCombatSystem::displayWaveIntroduction();
    EnemyCombatQueue wave = WaveCombatSystem::createWaveForPlayer(leader, random, difficulty);

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);
    displayEncounterDialogue(leader, wave, random, "pve.coop");

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
        MessageScreen::show(
            "TOUR DE GROUPE " + std::to_string(round),
            "pve.coop.round." + std::to_string(round),
            {"Le groupe se replace avant la prochaine séquence d'actions."},
            false
        );
        displayCoopPartyStatus(party, extractDownedFlags(contributions));
        displayPartyWaveCombatSnapshot(
            party,
            wave,
            partySummons,
            "Début du tour de groupe",
            round
        );

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead() || !wave.hasEnemiesLeft() || escapeSucceeded)
            {
                continue;
            }

            MessageScreen::show(
                "TOUR ALLIÉ",
                "pve.coop.player_turn." + std::to_string(i + 1),
                {"Tour de " + player->getName() + " [joueur " + std::to_string(i + 1) + "]."},
                false
            );

            displayPartyWaveCombatSnapshot(
                party,
                wave,
                partySummons,
                "Action d'un joueur allié",
                round,
                player->getName()
            );

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

            MessageScreen::show(
                "TOUR ENNEMI",
                "pve.coop.enemy_turn." + std::to_string(enemyIndex),
                {"Tour de " + monster.getName() + " : cible " + target->getName() + "."},
                false
            );
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

        displayPartyWaveCombatSnapshot(
            party,
            wave,
            partySummons,
            "Après la riposte ennemie",
            round
        );

        wave.removeDeadAndReplace();
        ++round;
    }

    if (escapeSucceeded)
    {
        MessageScreen::show(
            "FUITE DE GROUPE",
            "pve.coop.escape.success",
            {
                "Le groupe a ouvert une sortie.",
                "Chaque personnage récupère seulement une part de ce qu'il a réellement aidé à obtenir."
            },
            false
        );
    }

    if (countAlivePlayers(party) == 0 && wave.hasEnemiesLeft())
    {
        MessageScreen::show(
            "GROUPE AU SOL",
            "pve.coop.party_defeat",
            {"Tout le groupe est tombé."},
            false
        );

        for (Player* player : party)
        {
            if (player == nullptr) continue;

            player->recordDefeat();
            if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
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

    MessageScreen::show(
        "RÉCOMPENSES INDIVIDUELLES COOP",
        "pve.coop.rewards.start",
        {"Les récompenses sont calculées selon la participation réelle de chaque personnage."},
        false
    );

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        if (player == nullptr)
        {
            continue;
        }

        if (player->isDead() && !DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            player->recordDeath();
            player->reviveWithHealthPercentage(
                DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
            );
            MessageScreen::show(
                "RÉVEIL DE FIN DE COMBAT",
                "pve.coop.reward.revive." + std::to_string(i + 1),
                {player->getName() + " est réveillé à la fin du combat. La mort est comptabilisée."},
                false
            );
        }

        if (player->isDead())
        {
            MessageScreen::show(
                "AUCUNE RÉCOMPENSE",
                "pve.coop.reward.down." + std::to_string(i + 1),
                {player->getName() + " reste au sol : aucune récompense supplémentaire après sa chute."},
                false
            );
            continue;
        }

        CombatReward individualReward = buildIndividualCoopReward(
            baseReward,
            *player,
            leader,
            contributions[i]
        );

        MessageScreen::show(
            "RÉCOMPENSE DE " + player->getName(),
            "pve.coop.reward.player." + std::to_string(i + 1),
            {"Résumé de participation et récompense individuelle."},
            false
        );
        CombatRewardSystem::displayReward(individualReward);
        CombatRewardSystem::giveRewardToPlayer(*player, individualReward);
        player->recordVictory();
        player->recordEnemyKills(wave.getDefeatedEnemyCount());
        MessageScreen::show(
            "PARTICIPATION",
            "pve.coop.reward.participation." + std::to_string(i + 1),
            {
                "Tours joués : " + std::to_string(contributions[i].turnsTaken) + ".",
                "Dégâts infligés : " + std::to_string(contributions[i].damageDealt) + ".",
                "Soins effectués : " + std::to_string(contributions[i].healingDone) + ".",
                "Dégâts encaissés : " + std::to_string(contributions[i].damageTaken) + "."
            },
            false
        );
        LootGenerator::giveDefeatedWaveLoot(*player, wave, random, difficulty);
    }

    recordWaveKillsInBestiary(wave);
}
