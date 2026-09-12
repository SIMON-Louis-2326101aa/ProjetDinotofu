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
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "combat/reward/CombatReward.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/group/InitiativeSystem.hpp"
#include "combat/group/TurnOrder.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/TurnManager.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/CombatDisplay.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"
#include "economy/Money.hpp"

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltyResult.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "progression/blessing/BlessingSystem.hpp"
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
#include <map>
#include <set>
#include <sstream>

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

        description += MonsterBehaviorProfileCatalog::buildBestiarySentence(monster);

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


    void recordWaveEncountersInJournal(Player& player, const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            player.recordEnemyEncounter(wave.getActiveEnemy(i).getName());
        }
        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
        {
            player.recordEnemyEncounter(wave.getWaitingEnemy(i).getName());
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

    bool bobMauriceProtectionQuestActive(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id.rfind("bob_maurice_protection_", 0) == 0
                && quest.accepted
                && !quest.completed
                && !quest.turnedIn
                && !quest.failed)
            {
                return true;
            }
        }
        return false;
    }

    void playBobMauriceAlliedTurn(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        if (!bobMauriceProtectionQuestActive(player) || !wave.hasActiveEnemies())
        {
            return;
        }

        std::vector<std::string> lines;

        Monster& bobTarget = wave.getActiveEnemy(0);
        const std::string bobTargetName = bobTarget.getName();
        bobTarget.takeDamage(1);
        lines.push_back("Bob : Hannnn... hummm... hammmm.");
        lines.push_back("Maurice : « Mon collègue Bob a dit qu'il vient d'infliger exactement 1 dégât à " + bobTargetName + ". Il insiste sur le mot exactement. »");
        wave.removeDeadAndReplace();

        if (wave.hasActiveEnemies())
        {
            Monster& mauriceTarget = wave.getActiveEnemy(0);
            const std::string mauriceTargetName = mauriceTarget.getName();
            mauriceTarget.takeDamage(1);
            lines.push_back("Maurice : Huuuhhhhh... hannn... hummm.");
            lines.push_back("Bob : « Maurice demande que son dégât contre " + mauriceTargetName + " soit inscrit séparément. Il en a fait 1 aussi. »");
            wave.removeDeadAndReplace();
        }
        else
        {
            lines.push_back("Maurice : Huuuhhhhh...");
            lines.push_back("Bob : « Maurice demande s'il peut garder son dégât pour plus tard. La réponse est probablement non. »");
        }

        const int effectRoll = random.between(1, 8);
        if (effectRoll == 1)
        {
            const int healed = std::max(2, player.getMaxHp() / 20);
            player.heal(healed);
            lines.push_back("Bob lance une petite fiole correctement étiquetée : tu récupères " + std::to_string(healed) + " PV.");
        }
        else if (effectRoll == 2)
        {
            player.applyElementalWard(2, 15);
            lines.push_back("Maurice renverse une poudre protectrice au bon endroit : protection élémentaire légère pendant 2 tours.");
        }
        else if (effectRoll == 3 && wave.hasActiveEnemies())
        {
            wave.getActiveEnemy(0).applyPoison(2, 1);
            lines.push_back("Une bouteille mal fermée roule sous un ennemi : poison léger pendant 2 tours.");
        }
        else if (effectRoll == 4)
        {
            player.takeDamage(1);
            lines.push_back("Bob te lance une potion. Maurice oublie de préciser qu'elle est encore dans sa bouteille : tu subis 1 dégât.");
        }
        else if (effectRoll == 5 && wave.hasActiveEnemies())
        {
            const int healed = std::max(2, wave.getActiveEnemy(0).getMaxHp() / 25);
            wave.getActiveEnemy(0).heal(healed);
            lines.push_back("Maurice soigne accidentellement l'ennemi actif de " + std::to_string(healed) + " PV. Bob prétend que c'était un test de loyauté.");
        }
        else if (effectRoll == 6)
        {
            player.applyWeakening(1, 10);
            lines.push_back("Une fumée commerciale te pique les yeux : affaiblissement léger pendant 1 tour.");
        }
        else if (effectRoll == 7)
        {
            player.applyRegeneration(2, 2);
            lines.push_back("Le tonique de Maurice fonctionne contre toute attente : régénération légère pendant 2 tours.");
        }
        else
        {
            lines.push_back("Les deux agitent une caisse vide avec conviction. Aucun effet mesurable, mais ils semblent satisfaits.");
        }

        MessageScreen::show(
            "TOUR DE BOB ET MAURICE",
            "combat.pve.bob_maurice.allied_turn",
            lines,
            false
        );
    }

    bool defeatedWaveContainedElite(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            if (wave.getDefeatedEnemy(i).isElite() || wave.getDefeatedEnemy(i).isEvolved())
            {
                return true;
            }
        }
        return false;
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

    void recordWaveKillsInJournal(Player& player, const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            const Monster& monster = wave.getDefeatedEnemy(i);
            player.recordEnemyKillByName(monster.getName());
            if (monster.isPersistentRival())
            {
                player.markRivalDefeated(monster.getRivalId(), player.getCurrentCityId());
            }
        }
    }

    std::string returningRivalDisplayName(const PlayerRivalRecord& rival)
    {
        if (rival.returns >= 4) return rival.enemyName + " — Némésis";
        if (rival.returns >= 3) return rival.enemyName + " — Fléau familier";
        if (rival.returns >= 2) return rival.enemyName + " — Ombre revenue";
        return rival.enemyName + " — Rancune tenace";
    }

    bool maybeAppendReturningRival(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        std::vector<std::string> candidates;
        for (const PlayerRivalRecord& rival : player.getRivalRecords())
        {
            if (!rival.alive || rival.lastSeenDay >= player.getWorldDaysElapsed()) continue;
            const bool localTrace = rival.lastKnownLocationId.empty() || rival.lastKnownLocationId == player.getCurrentCityId();
            int chance = localTrace ? 24 : 7;
            if (player.hasPassiveSkill("church_oath_rivals")) chance += 10;
            if (player.hasPassiveSkill("church_oath_memory")) chance += 5;
            if (random.between(1, 100) <= chance) candidates.push_back(rival.rivalId);
        }
        if (candidates.empty()) return false;

        const std::string rivalId = candidates[static_cast<std::size_t>(random.between(0, static_cast<int>(candidates.size()) - 1))];
        const PlayerRivalRecord* rival = player.findRival(rivalId);
        if (rival == nullptr || !rival->alive) return false;

        player.recordRivalReturn(rivalId, player.getCurrentCityId());
        rival = player.findRival(rivalId);
        if (rival == nullptr) return false;

        const int hp = std::max(8, rival->baseMaxHp + rival->returns * std::max(4, rival->baseMaxHp / 5));
        const int attack = std::max(2, rival->baseAttack + rival->returns * 2);
        Monster returning(
            returningRivalDisplayName(*rival),
            rival->enemyFamily.empty() ? "Rival" : rival->enemyFamily,
            Race::Unknown,
            rival->currentLevel,
            hp,
            std::max(1, attack - 2),
            attack,
            attack + 4,
            rival->returns >= 2 ? 1 : 0,
            rival->returns >= 3 ? 1 : 0,
            false,
            rival->returns >= 2,
            false,
            rival->returns >= 3
        );
        returning.setRivalId(rivalId);
        wave.addWaitingEnemy(returning);

        MessageScreen::show(
            "TRACE FAMILIÈRE",
            "combat.pve.rival.return",
            {
                "Une présence déjà connue suit la rencontre au lieu d'apparaître comme un ennemi anonyme.",
                returning.getName() + " | retour " + std::to_string(rival->returns) + " | niveau " + std::to_string(rival->currentLevel) + ".",
                "Dernière trace connue : " + (rival->lastKnownLocationId.empty() ? std::string("inconnue") : rival->lastKnownLocationId) + ".",
                "Ce retour existe parce que cet individu a survécu auparavant. Sa mort, elle, restera définitive."
            },
            false
        );
        return true;
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
        MessageScreen::show(title, screenId, lines, false);
        Console::pauseSeconds(1);
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


    struct TemporaryAdventurerSupport
    {
        bool active = false;
        std::string groupName;
        int remainingTurns = 0;
        int initiativeSlot = 0;
    };

    std::string rareCombatAidGroupName(Random& random)
    {
        const std::vector<std::string> groups = {
            "Les Lanternes de Prunigil",
            "Les Chasseurs du Croc Tordu",
            "Deux Lames et un Chariot",
            "L'Ordo de Pierre",
            "Les Éclats d'Azur"
        };
        return groups[static_cast<std::size_t>(random.between(0, static_cast<int>(groups.size()) - 1))];
    }

    int localJournalCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.count;
            }
        }
        return 0;
    }

    std::string localJournalLabel(const Player& player, const std::string& category, const std::string& key, const std::string& fallback = "")
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.label.empty() ? fallback : record.label;
            }
        }
        return fallback;
    }

    bool recruitedAllyIsInInfirmary(const Player& player, const std::string& recruitName)
    {
        if (recruitName.empty())
        {
            return false;
        }
        return localJournalCount(player, "recrues_infirmerie_sejours", recruitName)
            > localJournalCount(player, "recrues_infirmerie_recuperees", recruitName);
    }

    bool recruitedAllyIsAwaitingInfirmaryTransfer(const Player& player, const std::string& recruitName)
    {
        if (recruitName.empty())
        {
            return false;
        }
        return localJournalCount(player, "recrues_a_evacuer", recruitName)
            > localJournalCount(player, "recrues_evacuees_infirmerie", recruitName);
    }

    std::string extractLabelStringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos)
        {
            return fallback;
        }
        const std::size_t start = pos + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos)
        {
            end = label.size();
        }
        if (end <= start)
        {
            return fallback;
        }
        return label.substr(start, end - start);
    }

    int extractLabelIntField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos)
        {
            return fallback;
        }
        std::size_t start = pos + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-')
        {
            ++end;
        }
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end])))
        {
            ++end;
        }
        if (end <= start)
        {
            return fallback;
        }
        try
        {
            return std::stoi(label.substr(start, end - start));
        }
        catch (...)
        {
            return fallback;
        }
    }

    unsigned int stableAllyHash(const std::string& seed)
    {
        unsigned int hash = 2166136261u;
        for (unsigned char c : seed)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }

    int recruitedAllyInitialHealthPercent(const std::string& recruitName)
    {
        return 70 + static_cast<int>(stableAllyHash(recruitName + ":initial_hp_percent") % 31u);
    }

    int recruitedAllyRespawnHealthPercent(DifficultyMode difficulty)
    {
        return std::clamp(DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty), 1, 100);
    }

    int recruitedAllyRankIndexFromLevel(int level)
    {
        if (level >= 70) return 6;
        if (level >= 52) return 5;
        if (level >= 38) return 4;
        if (level >= 26) return 3;
        if (level >= 15) return 2;
        if (level >= 8) return 1;
        return 0;
    }

    std::string recruitedAllyRankName(int index)
    {
        static const std::vector<std::string> ranks = {"F", "E", "D", "C", "B", "A", "S"};
        index = std::clamp(index, 0, static_cast<int>(ranks.size()) - 1);
        return ranks[static_cast<std::size_t>(index)];
    }

    bool recruitedAllyManualOrderEnabled(const Player& player)
    {
        const std::string mode = localJournalLabel(player, "ordre_manuel_recrues", "mode", "auto");
        return mode.find("manuel") != std::string::npos || mode.find("manual") != std::string::npos;
    }

    int recruitedAllyManualOrderIndex(const Player& player, const std::string& name)
    {
        if (!recruitedAllyManualOrderEnabled(player))
        {
            return 10000;
        }
        for (int slot = 1; slot <= 12; ++slot)
        {
            if (localJournalLabel(player, "ordre_manuel_recrues", "slot_" + std::to_string(slot), "") == name)
            {
                return slot;
            }
        }
        return 10000;
    }

    struct RecruitedAllyCombatSupport
    {
        std::string name;
        std::string race;
        std::string job;
        std::string trait;
        int recruitedLevel = 1;
        int estimatedLevel = 1;
        int originRankIndex = 0;
        int currentRankIndex = 0;
        int requestedShare = 10;
        int normalizedShare = 0;
        int damageDealt = 0;
        int healingDone = 0;
        int turnsTaken = 0;
        int maxHp = 1;
        int currentHp = 1;
        int healingPotionCharges = 0;
        int weaponQuality = 1;
        int armorQuality = 1;
        int supportKitQuality = 0;
        int activeSkillCooldown = 0;
        int supportActions = 0;
        int finishBlows = 0;
        bool protectedThisRound = false;
    };

    struct RecruitedAllyUniqueOrder
    {
        bool healOneTurn = false;
        bool guardOneTurn = false;
        bool forceTechniqueOneTurn = false;
        bool hasPriorityTarget = false;
        std::string priorityTargetName;
        bool switchRequested = false;
        std::string switchInName;
    };

    struct RecruitedAllyOrderState
    {
        bool groupHealOneTurn = false;
        bool groupGuardOneTurn = false;
        bool groupTechniqueOneTurn = false;
        bool groupCombinedTechniqueOneTurn = false;
        bool groupSpreadTargetsOneTurn = false;
        bool groupPriorityTarget = false;
        std::string groupPriorityTargetName;
        std::map<std::string, RecruitedAllyUniqueOrder> uniqueOrders;
    };

    std::string lowerCopy(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text;
    }

    int recruitedAllyMaxShareCapForGroupSize(int groupSizeIncludingPlayer)
    {
        if (groupSizeIncludingPlayer >= 5) return 22;
        if (groupSizeIncludingPlayer == 4) return 25;
        if (groupSizeIncludingPlayer == 3) return 30;
        if (groupSizeIncludingPlayer == 2) return 35;
        return 0;
    }

    int recruitedAllyPlayerMinimumShareForGroupSize(int groupSizeIncludingPlayer)
    {
        if (groupSizeIncludingPlayer <= 1) return 100;
        const int averageShare = 100 / std::max(1, groupSizeIncludingPlayer);
        return std::clamp(averageShare + 10, 30, 65);
    }

    int normalizeRecruitedAllyShares(std::vector<RecruitedAllyCombatSupport>& allies)
    {
        if (allies.empty())
        {
            return 100;
        }
        const int groupSize = static_cast<int>(allies.size()) + 1;
        int playerShare = recruitedAllyPlayerMinimumShareForGroupSize(groupSize);
        const int perAllyCap = std::min(recruitedAllyMaxShareCapForGroupSize(groupSize), std::max(6, playerShare - 1));
        const int available = std::max(0, 100 - playerShare);
        int totalWeight = 0;
        std::vector<int> weights;
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            const int powerWeight = ally.currentRankIndex * 3 + ally.estimatedLevel / 8;
            const int weight = std::clamp(ally.requestedShare + powerWeight, 5, perAllyCap * 2);
            weights.push_back(weight);
            totalWeight += weight;
        }

        int assigned = 0;
        for (std::size_t i = 0; i < allies.size(); ++i)
        {
            int share = totalWeight <= 0
                ? available / static_cast<int>(allies.size())
                : (weights[i] * available) / totalWeight;
            share = std::clamp(share, allies.size() <= 1 ? 0 : 5, perAllyCap);
            allies[i].normalizedShare = share;
            assigned += share;
        }

        int safety = 0;
        while (assigned < available && safety < 200)
        {
            bool changed = false;
            for (RecruitedAllyCombatSupport& ally : allies)
            {
                if (assigned >= available) break;
                if (ally.normalizedShare < perAllyCap)
                {
                    ++ally.normalizedShare;
                    ++assigned;
                    changed = true;
                }
            }
            if (!changed) break;
            ++safety;
        }

        playerShare = 100 - assigned;
        const auto biggest = std::max_element(allies.begin(), allies.end(), [](const RecruitedAllyCombatSupport& a, const RecruitedAllyCombatSupport& b) {
            return a.normalizedShare < b.normalizedShare;
        });
        if (biggest != allies.end() && playerShare <= biggest->normalizedShare)
        {
            int needed = biggest->normalizedShare + 1 - playerShare;
            for (RecruitedAllyCombatSupport& ally : allies)
            {
                if (needed <= 0) break;
                if (ally.normalizedShare > 5)
                {
                    const int taken = std::min(needed, ally.normalizedShare - 5);
                    ally.normalizedShare -= taken;
                    playerShare += taken;
                    needed -= taken;
                }
            }
        }
        return std::clamp(playerShare, 0, 100);
    }

    RecruitedAllyCombatSupport recruitedAllyFromRecord(const Player& player, const PlayerJournalRecord& record)
    {
        RecruitedAllyCombatSupport ally;
        ally.name = record.key.empty() ? record.label : record.key;
        ally.race = extractLabelStringField(record.label, "race", "Inconnue");
        ally.job = extractLabelStringField(record.label, "profil", "profil libre");
        ally.trait = extractLabelStringField(record.label, "trait", "trait non relu");
        ally.recruitedLevel = std::max(1, extractLabelIntField(record.label, "niveau_recrutement", std::max(1, player.getLevel() - 2)));
        const int gained = std::min(18, std::max(0, record.count - 1) * 2 + static_cast<int>(stableAllyHash(ally.name + ":combat_growth") % 5u));
        ally.estimatedLevel = std::max(ally.recruitedLevel, std::min(100, ally.recruitedLevel + gained));
        ally.originRankIndex = std::clamp(extractLabelIntField(record.label, "rang_origine_index", recruitedAllyRankIndexFromLevel(ally.recruitedLevel)), 0, 6);

        int promotionCount = 0;
        const std::string prefix = ally.name + ":";
        for (const PlayerJournalRecord& promotion : player.getCanonicalJournalRecords())
        {
            if (promotion.category == "rangs_recrues_obtenus" && promotion.key.rfind(prefix, 0) == 0 && promotion.count > 0)
            {
                promotionCount += promotion.count;
            }
        }
        ally.currentRankIndex = std::max(ally.originRankIndex, recruitedAllyRankIndexFromLevel(ally.estimatedLevel));
        ally.currentRankIndex = std::clamp(ally.currentRankIndex + promotionCount, ally.originRankIndex, 6);
        int baseShare = std::clamp(extractLabelIntField(record.label, "part_base", 12 + ally.currentRankIndex * 4), 6, 40);
        const int earlyDiscount = std::max(0, ally.currentRankIndex - ally.originRankIndex) * 2;
        const int storedDiscount = extractLabelIntField(record.label, "remise_fidelite", std::max(0, 7 - ally.originRankIndex));
        ally.requestedShare = std::clamp(baseShare - std::clamp(storedDiscount + earlyDiscount, 0, 14), 6, 40);
        ally.maxHp = std::max(18, 34 + ally.estimatedLevel * 3 + ally.currentRankIndex * 15);
        const std::string profile = lowerCopy(ally.job + " " + ally.trait);
        if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos)
        {
            ally.maxHp = ally.maxHp * 120 / 100;
        }
        if (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos)
        {
            ally.maxHp = ally.maxHp * 95 / 100;
        }
        const int defaultPotions = std::clamp(1 + ally.currentRankIndex / 2 + (profile.find("intendant") != std::string::npos ? 1 : 0), 1, 4);
        const int initialHealthPercent = recruitedAllyInitialHealthPercent(ally.name);
        const std::string vitalsLabel = localJournalLabel(player, "pv_recrues_persistants", ally.name, "");
        const int savedHp = extractLabelIntField(vitalsLabel, "hp", std::max(1, ally.maxHp * initialHealthPercent / 100));
        ally.currentHp = std::clamp(savedHp, 0, ally.maxHp);
        ally.healingPotionCharges = std::clamp(extractLabelIntField(vitalsLabel, "potions", defaultPotions), 0, 5);
        const std::string equipmentLabel = localJournalLabel(player, "equipement_recrues", ally.name, "");
        const int naturalQuality = std::clamp(1 + ally.currentRankIndex + ally.estimatedLevel / 28, 1, 9);
        ally.weaponQuality = std::clamp(extractLabelIntField(equipmentLabel, "arme", naturalQuality), 1, 10);
        ally.armorQuality = std::clamp(extractLabelIntField(equipmentLabel, "armure", std::max(1, naturalQuality - 1)), 1, 10);
        const std::string supportProfile = lowerCopy(ally.job + " " + ally.trait);
        ally.supportKitQuality = std::clamp(extractLabelIntField(equipmentLabel, "kit", supportProfile.find("soigneur") != std::string::npos ? 2 : 1), 0, 10);
        ally.activeSkillCooldown = 0;
        return ally;
    }

    std::vector<RecruitedAllyCombatSupport> collectRecruitedAllyRoster(const Player& player, bool equippedOnly)
    {
        std::vector<RecruitedAllyCombatSupport> allies;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "pnj_recrutables_retenus" || record.count <= 0)
            {
                continue;
            }
            const std::string name = record.key.empty() ? record.label : record.key;
            if (name.empty() || localJournalCount(player, "recrues_renvoyees", name) > 0)
            {
                continue;
            }
            if (recruitedAllyIsInInfirmary(player, name) || recruitedAllyIsAwaitingInfirmaryTransfer(player, name))
            {
                continue;
            }
            const bool equipped = (localJournalCount(player, "recrues_equipees_toggle", name) % 2) == 1;
            if (equippedOnly != equipped)
            {
                continue;
            }
            allies.push_back(recruitedAllyFromRecord(player, record));
        }

        std::sort(allies.begin(), allies.end(), [&](const RecruitedAllyCombatSupport& a, const RecruitedAllyCombatSupport& b) {
            const int manualA = recruitedAllyManualOrderIndex(player, a.name);
            const int manualB = recruitedAllyManualOrderIndex(player, b.name);
            if (manualA != manualB) return manualA < manualB;
            if (a.currentRankIndex != b.currentRankIndex) return a.currentRankIndex > b.currentRankIndex;
            if (a.estimatedLevel != b.estimatedLevel) return a.estimatedLevel > b.estimatedLevel;
            return a.name < b.name;
        });

        if (equippedOnly && allies.size() > 2)
        {
            allies.resize(2);
        }
        if (equippedOnly)
        {
            normalizeRecruitedAllyShares(allies);
        }
        return allies;
    }

    std::vector<RecruitedAllyCombatSupport> collectActiveRecruitedAllies(const Player& player)
    {
        return collectRecruitedAllyRoster(player, true);
    }

    std::vector<RecruitedAllyCombatSupport> collectReserveRecruitedAllies(const Player& player)
    {
        return collectRecruitedAllyRoster(player, false);
    }

    bool recruitedAllyJobContains(const RecruitedAllyCombatSupport& ally, const std::string& token)
    {
        std::string text = ally.job + " " + ally.trait;
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text.find(token) != std::string::npos;
    }


    int recruitedAllyMaturityScore(const RecruitedAllyCombatSupport& ally)
    {
        return std::clamp(ally.currentRankIndex * 16 + ally.estimatedLevel / 4 + ally.weaponQuality + ally.armorQuality + ally.supportKitQuality, 0, 140);
    }

    int recruitedAllyMaturityTier(const RecruitedAllyCombatSupport& ally)
    {
        const int score = recruitedAllyMaturityScore(ally);
        if (score >= 92) return 5;
        if (score >= 72) return 4;
        if (score >= 52) return 3;
        if (score >= 34) return 2;
        if (score >= 18) return 1;
        return 0;
    }

    std::string recruitedAllyMaturityName(const RecruitedAllyCombatSupport& ally)
    {
        switch (recruitedAllyMaturityTier(ally))
        {
            case 0: return "débutant";
            case 1: return "apprenti";
            case 2: return "habitué";
            case 3: return "aguerri";
            case 4: return "vétéran";
            default: return "maître de terrain";
        }
    }

    bool recruitedAllyIsStillLearning(const RecruitedAllyCombatSupport& ally)
    {
        return ally.currentRankIndex <= 1 || recruitedAllyMaturityTier(ally) <= 1;
    }

    bool recruitedAllyHasAdvancedReading(const RecruitedAllyCombatSupport& ally)
    {
        return ally.currentRankIndex >= 2 && recruitedAllyMaturityTier(ally) >= 2;
    }

    int recruitedAllyOrderTechniqueChance(const RecruitedAllyCombatSupport& ally, bool forced)
    {
        const int base = forced ? 56 : 28;
        const int rankBonus = ally.currentRankIndex * (forced ? 7 : 8);
        const int levelBonus = ally.estimatedLevel / (forced ? 10 : 12);
        return std::clamp(base + rankBonus + levelBonus, forced ? 58 : 25, forced ? 96 : 86);
    }

    int recruitedAllyNaturalTechniqueChance(const RecruitedAllyCombatSupport& ally, EnemyCombatQueue& wave)
    {
        int chance = 7 + ally.currentRankIndex * 4 + ally.estimatedLevel / 20 + recruitedAllyMaturityTier(ally) * 4;
        if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "assassin") || recruitedAllyJobContains(ally, "brigand"))
        {
            chance += 11;
        }
        else if ((recruitedAllyJobContains(ally, "archer") || recruitedAllyJobContains(ally, "mage") || recruitedAllyJobContains(ally, "lancier")) && wave.getActiveEnemyCount() >= 2)
        {
            chance += 9;
        }
        else if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
        {
            chance += 6;
        }
        if (recruitedAllyIsStillLearning(ally))
        {
            chance = chance * 55 / 100;
        }
        if (recruitedAllyMaturityTier(ally) >= 4)
        {
            chance += 5;
        }
        return std::clamp(chance, 3, 56);
    }

    int recruitedAllyNaturalSupportChance(const RecruitedAllyCombatSupport& ally)
    {
        int chance = 12 + ally.currentRankIndex * 4 + ally.supportKitQuality * 2 + ally.estimatedLevel / 22 + recruitedAllyMaturityTier(ally) * 3;
        if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
        {
            chance += 12;
        }
        if (recruitedAllyIsStillLearning(ally))
        {
            chance = chance * 65 / 100;
        }
        return std::clamp(chance, 5, 60);
    }

    std::string recruitedAllyHealthLine(const RecruitedAllyCombatSupport& ally)
    {
        return "PV " + std::to_string(std::max(0, ally.currentHp)) + "/" + std::to_string(std::max(1, ally.maxHp))
            + " | potions " + std::to_string(std::max(0, ally.healingPotionCharges))
            + " | arme " + std::to_string(std::max(1, ally.weaponQuality))
            + " | armure " + std::to_string(std::max(1, ally.armorQuality))
            + " | maturité " + recruitedAllyMaturityName(ally) + " (" + std::to_string(recruitedAllyMaturityScore(ally)) + "/140)"
            + " | technique CD " + std::to_string(std::max(0, ally.activeSkillCooldown));
    }

    bool recruitedAllyShouldUseTechnique(const RecruitedAllyCombatSupport& ally, EnemyCombatQueue& wave, Random& random, bool hasDirectOrder, bool forceTechnique)
    {
        if (ally.activeSkillCooldown > 0 || !wave.hasActiveEnemies())
        {
            return false;
        }
        if (forceTechnique)
        {
            return random.between(1, 100) <= recruitedAllyOrderTechniqueChance(ally, true);
        }
        if (hasDirectOrder)
        {
            return random.between(1, 100) <= recruitedAllyOrderTechniqueChance(ally, false);
        }
        return random.between(1, 100) <= recruitedAllyNaturalTechniqueChance(ally, wave);
    }

    void recordRecruitedAllyHitContribution(RecruitedAllyCombatSupport& ally, int damage, bool killedTarget)
    {
        ally.damageDealt += std::max(0, damage);
        if (killedTarget)
        {
            ++ally.finishBlows;
        }
    }

    void recordRecruitedAllySupportContribution(RecruitedAllyCombatSupport& ally, int amount = 1)
    {
        ally.supportActions += std::max(1, amount);
    }

    std::string recruitedAllyTechniqueGesture(const RecruitedAllyCombatSupport& ally, const std::string& defaultGesture)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos || profile.find("brigand") != std::string::npos)
        {
            if (profile.find("elfe") != std::string::npos)
            {
                return ally.name + " disparaît dans un pas trop silencieux pour être honnête";
            }
            return ally.name + " baisse l'épaule, montre une fausse ouverture et passe sous la garde";
        }
        if (profile.find("archer") != std::string::npos || profile.find("tireur") != std::string::npos)
        {
            return ally.name + " cale sa respiration, puis coupe la ligne ennemie avec un tir sec";
        }
        if (profile.find("lancier") != std::string::npos)
        {
            return ally.name + " avance d'un demi-pas et force les ennemis à respecter la pointe";
        }
        if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos || profile.find("protecteur") != std::string::npos)
        {
            return ally.name + " plante sa garde devant la ligne et transforme l'avancée en mur";
        }
        if (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("intendant") != std::string::npos)
        {
            return ally.name + " pose un repère calme au milieu du chaos et désigne la faille";
        }
        if (profile.find("mage") != std::string::npos || profile.find("sorcier") != std::string::npos)
        {
            return ally.name + " serre les doigts autour d'une formule courte et laisse l'air se tendre";
        }
        if (profile.find("kitsune") != std::string::npos)
        {
            return ally.name + " laisse une flamme courte suivre son mouvement";
        }
        if (profile.find("dragon") != std::string::npos || profile.find("dracon") != std::string::npos)
        {
            return ally.name + " fait vibrer sa force ancienne dans l'impact";
        }
        return defaultGesture;
    }

    std::string recruitedAllyReadinessLine(const RecruitedAllyCombatSupport& ally, const Player& player)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const int allyHpPercent = ally.maxHp > 0 ? ally.currentHp * 100 / ally.maxHp : 0;
        const int playerHpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;
        if (recruitedAllyMaturityTier(ally) <= 0)
        {
            return ally.name + " reste proche de la ligne et attend surtout de comprendre ce que tu vas ordonner";
        }
        if (recruitedAllyMaturityTier(ally) == 1 && playerHpPercent > 35)
        {
            return ally.name + " connaît son rôle, mais garde encore plus les yeux sur toi que sur tout le champ de bataille";
        }
        if (allyHpPercent <= 30)
        {
            return ally.name + " arrive en serrant les dents : elle peut encore agir, mais pas encaisser n'importe quoi";
        }
        if (playerHpPercent <= 35 && (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("gardien") != std::string::npos))
        {
            return ally.name + " garde déjà un oeil sur ton état avant même que la ligne bouge";
        }
        if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos)
        {
            return ally.name + " prend une place légèrement décalée, assez loin pour chercher un angle mort";
        }
        if (profile.find("archer") != std::string::npos || profile.find("tireur") != std::string::npos)
        {
            return ally.name + " vérifie les distances comme si chaque pas ennemi comptait déjà";
        }
        if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos || profile.find("protecteur") != std::string::npos)
        {
            return ally.name + " se place naturellement entre le danger et le reste du groupe";
        }
        if (profile.find("mage") != std::string::npos || profile.find("chaman") != std::string::npos || profile.find("sorcier") != std::string::npos)
        {
            return ally.name + " attend le premier désordre pour poser son rythme magique";
        }
        return ally.name + " rejoint la ligne sans grand discours, mais avec une intention claire";
    }

    std::string recruitedAllyPresenceLine(const RecruitedAllyCombatSupport& ally, const Player& player, EnemyCombatQueue& wave)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const int allyHpPercent = ally.maxHp > 0 ? ally.currentHp * 100 / ally.maxHp : 0;
        const int playerHpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;

        if (recruitedAllyMaturityTier(ally) <= 0)
        {
            return ally.name + " suit la cadence avec prudence, sans encore lire toute la scène";
        }
        if (recruitedAllyMaturityTier(ally) == 1 && ally.turnsTaken % 2 == 1)
        {
            return ally.name + " hésite une demi-seconde, puis se replace sur une consigne simple";
        }
        if (allyHpPercent <= 30)
        {
            return ally.name + " garde un bras près de ses côtes, mais reste dans la ligne tant que ses jambes tiennent";
        }
        if (playerHpPercent <= 35 && (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("intendant") != std::string::npos))
        {
            return ally.name + " surveille ton souffle plus que la cible et prépare déjà une couverture";
        }
        if (playerHpPercent <= 35 && (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos || profile.find("protecteur") != std::string::npos))
        {
            return ally.name + " se décale d'un pas pour couper l'angle le plus dangereux vers toi";
        }
        if (wave.getActiveEnemyCount() >= 3 && (profile.find("chef") != std::string::npos || profile.find("capitaine") != std::string::npos || profile.find("stratège") != std::string::npos || profile.find("stratege") != std::string::npos))
        {
            return ally.name + " compte les lignes ennemies avant de choisir qui doit vraiment tomber";
        }
        if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos || profile.find("brigand") != std::string::npos)
        {
            return ally.name + " ne fixe pas la cible principale, seulement l'espace où elle va finir par se tromper";
        }
        if (profile.find("archer") != std::string::npos || profile.find("tireur") != std::string::npos || profile.find("éclaireur") != std::string::npos || profile.find("eclaireur") != std::string::npos)
        {
            return ally.name + " suit les épaules ennemies et attend une respiration trop longue";
        }
        if (profile.find("kitsune") != std::string::npos)
        {
            return ally.name + " laisse passer un sourire nerveux, comme si la prochaine feinte était déjà prête";
        }
        if (profile.find("orc") != std::string::npos || profile.find("barbare") != std::string::npos || profile.find("berserker") != std::string::npos)
        {
            return ally.name + " serre sa prise et cherche le moment où la ligne ennemie fera enfin du bruit";
        }
        if (profile.find("mage") != std::string::npos || profile.find("sorcier") != std::string::npos || profile.find("chaman") != std::string::npos)
        {
            return ally.name + " écoute le combat comme une formule qui n'a pas encore choisi sa fin";
        }
        return ally.name + " relit rapidement la distance avant de reprendre son rôle dans la ligne";
    }

    std::string recruitedAllyTacticalCommentLine(const RecruitedAllyCombatSupport& ally, const Player& player, EnemyCombatQueue& wave, bool hasActiveOrder)
    {
        if (!wave.hasActiveEnemies())
        {
            return "";
        }

        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const int playerHpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;
        const Monster& frontEnemy = wave.getActiveEnemy(0);
        const MonsterBehaviorProfile enemyProfile = MonsterBehaviorProfileCatalog::build(frontEnemy);
        const std::string enemyArchetype = lowerCopy(enemyProfile.archetype);
        const int enemyHpPercent = frontEnemy.getMaxHp() > 0 ? frontEnemy.getHp() * 100 / frontEnemy.getMaxHp() : 100;

        if (recruitedAllyMaturityTier(ally) <= 0 && !hasActiveOrder)
        {
            return ally.turnsTaken % 3 == 0 ? ally.name + " attend une indication claire avant de tenter quelque chose de trop technique" : "";
        }
        if (recruitedAllyMaturityTier(ally) == 1 && !hasActiveOrder && ally.turnsTaken % 2 == 1)
        {
            return "";
        }
        if (recruitedAllyMaturityTier(ally) <= 1 && hasActiveOrder)
        {
            return ally.name + " suit l'ordre donné, même si sa lecture du terrain reste encore limitée";
        }
        if (!recruitedAllyHasAdvancedReading(ally) && enemyArchetype.find("soigneur") == std::string::npos && enemyArchetype.find("rameuteur") == std::string::npos)
        {
            return "";
        }

        if (playerHpPercent <= 35 && (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("intendant") != std::string::npos))
        {
            return ally.name + " garde sa main libre, comme si le prochain mauvais souffle du chef était déjà prévu";
        }
        if (playerHpPercent <= 35 && (profile.find("gardien") != std::string::npos || profile.find("protecteur") != std::string::npos || profile.find("tank") != std::string::npos))
        {
            return ally.name + " resserre la ligne vers toi au lieu de chercher un duel héroïque";
        }
        if (enemyArchetype.find("rameuteur") != std::string::npos)
        {
            if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos)
            {
                return ally.name + " fixe surtout la gorge du rameuteur : couper le signal vaut plus qu'un joli coup";
            }
            return ally.name + " garde le signal ennemi dans son champ de vision pour éviter une mauvaise surprise";
        }
        if (enemyArchetype.find("soigneur") != std::string::npos || enemyArchetype.find("transfuseur") != std::string::npos)
        {
            return ally.name + " cherche la main qui soigne, pas seulement le corps le plus proche";
        }
        if (enemyArchetype.find("porte-bouclier") != std::string::npos || enemyArchetype.find("garde runique") != std::string::npos)
        {
            if (profile.find("lancier") != std::string::npos || profile.find("archer") != std::string::npos || profile.find("mage") != std::string::npos)
            {
                return ally.name + " lit les bords de la garde et attend la jointure plutôt que le centre";
            }
            return ally.name + " évite de gaspiller son impact sur la partie la plus solide";
        }
        if (enemyArchetype.find("slime") != std::string::npos || enemyArchetype.find("fongique") != std::string::npos || enemyArchetype.find("champignon") != std::string::npos)
        {
            return ally.name + " surveille surtout le sol, parce que la cible attaque presque autant avec l'espace qu'avec son corps";
        }
        if (enemyArchetype.find("illusionniste") != std::string::npos || enemyArchetype.find("piégeur") != std::string::npos)
        {
            return ally.name + " ralentit volontairement son geste pour ne pas répondre à la première fausse ouverture";
        }
        if (wave.getActiveEnemyCount() >= 3 && (profile.find("chef") != std::string::npos || profile.find("capitaine") != std::string::npos || profile.find("stratège") != std::string::npos || profile.find("stratege") != std::string::npos))
        {
            return ally.name + " découpe la mêlée en petites priorités au lieu de regarder le groupe comme une masse";
        }
        if (enemyHpPercent <= 30 && (profile.find("orc") != std::string::npos || profile.find("barbare") != std::string::npos || profile.find("berserker") != std::string::npos))
        {
            return ally.name + " sent que la cible peut tomber et serre sa prise avec beaucoup trop d'envie";
        }
        if (profile.find("fée") != std::string::npos || profile.find("fee") != std::string::npos)
        {
            return ally.name + " laisse ses appuis légers, prêt à punir le moindre angle trop lourd";
        }
        if (profile.find("dragon") != std::string::npos || profile.find("dracon") != std::string::npos)
        {
            return ally.name + " respire plus lentement, comme si son sang refusait de paniquer devant la ligne";
        }

        return "";
    }

    std::string recruitedAllyTargetResultLine(const RecruitedAllyCombatSupport& ally, const Monster& target, bool killed)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const MonsterBehaviorProfile targetProfile = MonsterBehaviorProfileCatalog::build(target);
        if (recruitedAllyIsStillLearning(ally))
        {
            if (killed)
            {
                return "La cible tombe, mais la recrue semble surtout surprise d'avoir trouvé le bon angle.";
            }
            return "La recrue touche, sans encore comprendre tout ce que la cible vient de révéler.";
        }
        if (killed)
        {
            if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos)
            {
                return "La cible tombe sans avoir eu le temps de refermer son angle.";
            }
            if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos)
            {
                return "La ligne alliée avance d'un cran derrière ce choc.";
            }
            return "La cible quitte la ligne ennemie et laisse un vide exploitable.";
        }
        if (target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 35)
        {
            return "La cible reste debout, mais " + targetProfile.archetype + " montre une vraie fissure.";
        }
        if (recruitedAllyMaturityTier(ally) >= 4 && !targetProfile.counterplayLine.empty())
        {
            return "Lecture vétérane : " + targetProfile.counterplayLine;
        }
        if (!targetProfile.durabilityLine.empty())
        {
            return "Lecture de cible : " + targetProfile.durabilityLine;
        }
        return "Le coup force surtout la cible à révéler un peu plus son rythme.";
    }

    int chooseRecruitedAllyTargetIndexWithOrders(
        RecruitedAllyCombatSupport& ally,
        EnemyCombatQueue& wave,
        Random& random,
        RecruitedAllyOrderState& orders
    );


    int recruitedAllyTechniqueImpactBonus(const RecruitedAllyCombatSupport& ally, bool hasDirectOrder, bool forceTechnique)
    {
        int bonus = recruitedAllyMaturityTier(ally) * 3 + ally.currentRankIndex + ally.weaponQuality / 3;
        if (hasDirectOrder)
        {
            bonus += 3;
        }
        if (forceTechnique)
        {
            bonus += recruitedAllyMaturityTier(ally) <= 1 ? 2 : 4;
        }
        return std::clamp(bonus, 0, 24);
    }

    int recruitedAllyTechniqueCooldown(const RecruitedAllyCombatSupport& ally, int baseCooldown, bool hasDirectOrder, bool forceTechnique)
    {
        int reduction = 0;
        if (recruitedAllyMaturityTier(ally) >= 3) ++reduction;
        if (recruitedAllyMaturityTier(ally) >= 5) ++reduction;
        if (hasDirectOrder && recruitedAllyMaturityTier(ally) >= 2) ++reduction;
        if (forceTechnique && recruitedAllyMaturityTier(ally) <= 1) reduction = std::max(0, reduction - 1);
        return std::max(1, baseCooldown - reduction);
    }

    std::string recruitedAllyTechniqueMaturityLine(const RecruitedAllyCombatSupport& ally, bool hasDirectOrder)
    {
        if (recruitedAllyMaturityTier(ally) >= 4)
        {
            return "  Lecture alliée : " + ally.name + " transforme son expérience en vrai tempo de combat, pas seulement en dégâts. Sa technique doit changer le tour.";
        }
        if (recruitedAllyMaturityTier(ally) >= 2)
        {
            return "  Lecture alliée : " + ally.name + " comprend assez la scène pour choisir un effet utile.";
        }
        if (hasDirectOrder)
        {
            return "  Ordre clair : la recrue reste limitée, mais l'ordre évite une partie de l'hésitation.";
        }
        return "  Lecture alliée : recrue encore basse maturité, technique volontairement simple.";
    }

    bool executeRecruitedAllyActiveTechnique(
        RecruitedAllyCombatSupport& ally,
        EnemyCombatQueue& wave,
        Random& random,
        RecruitedAllyOrderState& orders,
        std::vector<std::string>& lines
    )
    {
        RecruitedAllyUniqueOrder& techniqueOrder = orders.uniqueOrders[ally.name];
        const bool forceTechnique = techniqueOrder.forceTechniqueOneTurn || orders.groupTechniqueOneTurn;
        const bool hasDirectOrder = techniqueOrder.hasPriorityTarget || orders.groupPriorityTarget || forceTechnique;
        if (!recruitedAllyShouldUseTechnique(ally, wave, random, hasDirectOrder, forceTechnique))
        {
            return false;
        }

        const int techniqueImpactBonus = recruitedAllyTechniqueImpactBonus(ally, hasDirectOrder, forceTechnique);
        const int baseDamage = std::max(3, ally.estimatedLevel / 2 + ally.currentRankIndex * 4 + ally.weaponQuality * 2 + techniqueImpactBonus + random.between(2, 7));

        if (recruitedAllyIsStillLearning(ally) && !hasDirectOrder && random.between(1, 100) <= 25)
        {
            lines.push_back("- " + ally.name + " commence une technique, voit trop d'informations d'un coup, puis revient à une attaque simple. Une consigne claire l'aurait probablement stabilisée.");
            return false;
        }

        if (recruitedAllyIsStillLearning(ally))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int damage = std::max(2, baseDamage * (forceTechnique ? 68 : (hasDirectOrder ? 58 : 50)) / 100 + random.between(0, 2));
            target.takeDamage(damage);
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, forceTechnique ? 3 : (hasDirectOrder ? 3 : 4), hasDirectOrder, forceTechnique);
            lines.push_back("- " + ally.name + (forceTechnique ? " applique l'ordre avec une technique encore hésitante" : (hasDirectOrder ? " transforme la consigne en technique simple" : " tente une technique simple")) + " : " + targetName + " subit " + std::to_string(damage) + " dégâts. Maîtrise " + recruitedAllyMaturityName(ally) + ", CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if (recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank"))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int damage = std::max(2, baseDamage * 50 / 100 + ally.armorQuality);
            target.takeDamage(damage);
            target.applyWeakening(2, 10 + ally.currentRankIndex * 2);
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);
            recordRecruitedAllySupportContribution(ally, 2);
            ally.protectedThisRound = true;
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 3, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " plante sa garde devant la ligne") + " : " + targetName + " subit " + std::to_string(damage) + " dégâts et perd de la force. Couverture alliée active. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "assassin") || recruitedAllyJobContains(ally, "brigand"))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            int totalDamage = 0;
            const int hitCount = 3 + (ally.currentRankIndex >= 4 ? 1 : 0);
            for (int hit = 0; hit < hitCount && !target.isDead(); ++hit)
            {
                const int damage = std::max(2, baseDamage * (hit == 0 ? 60 : 42) / 100 + random.between(0, 3));
                target.takeDamage(damage);
                totalDamage += damage;
            }
            ElementalAffinitySystem::applyPoison(target, 2, std::max(1, 1 + ally.currentRankIndex / 2));
            ElementalAffinitySystem::applyBleeding(target, 2, std::max(1, 1 + ally.weaponQuality / 4));
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, totalDamage, killed);
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 4, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " glisse dans l'angle mort") + " : " + targetName + " subit " + std::to_string(hitCount) + " coups, " + std::to_string(totalDamage) + " dégâts, poison et saignement. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if ((recruitedAllyJobContains(ally, "archer") || recruitedAllyJobContains(ally, "mage") || recruitedAllyJobContains(ally, "lancier")) && wave.getActiveEnemyCount() >= 2)
        {
            const int maxHits = std::min(wave.getActiveEnemyCount(), 2 + (ally.currentRankIndex >= 3 ? 1 : 0));
            int totalDamage = 0;
            std::vector<std::string> touched;
            for (int i = 0; i < maxHits && wave.hasActiveEnemies(); ++i)
            {
                const int targetIndex = std::min(i, wave.getActiveEnemyCount() - 1);
                Monster& target = wave.getActiveEnemy(targetIndex);
                const std::string targetName = target.getName();
                const int damage = std::max(2, baseDamage * 70 / 100 + random.between(0, 3));
                target.takeDamage(damage);
                totalDamage += damage;
                const bool killed = target.isDead();
                if (killed) ++ally.finishBlows;
                touched.push_back(targetName + " (" + std::to_string(damage) + ")");
                wave.removeDeadAndReplace();
            }
            ally.damageDealt += totalDamage;
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 3, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " coupe plusieurs lignes ennemies") + " : " + std::to_string(totalDamage) + " dégâts répartis sur " + std::to_string(static_cast<int>(touched.size())) + " cible(s). CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui") || recruitedAllyJobContains(ally, "intendant"))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int damage = std::max(2, baseDamage * 55 / 100);
            target.takeDamage(damage);
            target.applyVulnerability(2, 15 + std::min(8, ally.supportKitQuality));
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);
            const int supportValue = std::max(1, ally.currentRankIndex + 1 + ally.supportKitQuality / 2);
            ally.healingDone += supportValue;
            recordRecruitedAllySupportContribution(ally, 2);
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 4, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " désigne une faille") + " : " + targetName + " subit " + std::to_string(damage) + " dégâts et une vulnérabilité 2 tours. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        int totalDamage = 0;
        const int hitCount = 2;
        for (int hit = 0; hit < hitCount && !target.isDead(); ++hit)
        {
            const int damage = std::max(2, baseDamage * (hit == 0 ? 75 : 55) / 100 + random.between(0, 3));
            target.takeDamage(damage);
            totalDamage += damage;
        }
        const bool killed = target.isDead();
        wave.removeDeadAndReplace();
        recordRecruitedAllyHitContribution(ally, totalDamage, killed);
        ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 3, hasDirectOrder, forceTechnique);
        lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " cherche son propre rythme") + " : " + targetName + " subit " + std::to_string(hitCount) + " frappe(s), " + std::to_string(totalDamage) + " dégâts. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
        lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
        return true;
    }

    int chooseRecruitedAllyTargetIndex(const RecruitedAllyCombatSupport& ally, EnemyCombatQueue& wave, Random& random)
    {
        const int activeCount = wave.getActiveEnemyCount();
        if (activeCount <= 1)
        {
            return 0;
        }
        if (recruitedAllyIsStillLearning(ally))
        {
            return 0;
        }
        if (recruitedAllyMaturityTier(ally) == 2 && random.between(1, 100) <= 45)
        {
            return random.between(0, activeCount - 1);
        }
        if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "archer"))
        {
            return random.between(0, activeCount - 1);
        }
        int bestIndex = 0;
        int bestHp = wave.getActiveEnemy(0).getHp();
        for (int i = 1; i < activeCount; ++i)
        {
            const int hp = wave.getActiveEnemy(i).getHp();
            if (hp < bestHp)
            {
                bestHp = hp;
                bestIndex = i;
            }
        }
        return bestIndex;
    }


    int findActiveEnemyIndexByName(EnemyCombatQueue& wave, const std::string& targetName)
    {
        if (targetName.empty())
        {
            return -1;
        }
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (wave.getActiveEnemy(i).getName() == targetName)
            {
                return i;
            }
        }
        return -1;
    }

    std::string selectActiveEnemyNameForOrder(EnemyCombatQueue& wave, const std::string& title, const std::string& id)
    {
        if (!wave.hasActiveEnemies())
        {
            MessageScreen::show(title, id + ".empty", {"Aucun ennemi actif à prioriser pour le moment."}, false);
            return "";
        }

        MenuScreen targetScreen(title, id);
        targetScreen.addLine("La priorité reste active jusqu'à la mort/disparition de cette cible.");
        targetScreen.addBackOption("Retour", id + ".back");
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& enemy = wave.getActiveEnemy(i);
            targetScreen.addOption(
                i + 1,
                enemy.getName(),
                "PV " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()) + " | niv. " + std::to_string(enemy.getLevel()),
                true,
                id + ".target"
            );
        }
        const int choice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une cible active.");
        Console::clear();
        if (choice <= 0 || choice > wave.getActiveEnemyCount())
        {
            return "";
        }
        return wave.getActiveEnemy(choice - 1).getName();
    }

    int chooseRecruitedAllyTargetIndexWithOrders(
        RecruitedAllyCombatSupport& ally,
        EnemyCombatQueue& wave,
        Random& random,
        RecruitedAllyOrderState& orders
    )
    {
        RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
        if (unique.hasPriorityTarget)
        {
            const int index = findActiveEnemyIndexByName(wave, unique.priorityTargetName);
            if (index >= 0)
            {
                return index;
            }
            unique.hasPriorityTarget = false;
            unique.priorityTargetName.clear();
        }

        if (orders.groupPriorityTarget)
        {
            const int index = findActiveEnemyIndexByName(wave, orders.groupPriorityTargetName);
            if (index >= 0)
            {
                return index;
            }
            orders.groupPriorityTarget = false;
            orders.groupPriorityTargetName.clear();
        }

        if (orders.groupSpreadTargetsOneTurn && wave.getActiveEnemyCount() > 1)
        {
            return static_cast<int>(stableAllyHash(ally.name + ":spread:" + std::to_string(ally.turnsTaken)) % static_cast<unsigned>(wave.getActiveEnemyCount()));
        }

        return chooseRecruitedAllyTargetIndex(ally, wave, random);
    }

    bool executeRequestedAllySwitch(
        Player& player,
        RecruitedAllyCombatSupport& ally,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyUniqueOrder& unique,
        std::vector<std::string>& lines
    )
    {
        if (!unique.switchRequested || unique.switchInName.empty())
        {
            return false;
        }

        auto reserveIt = std::find_if(reserveAllies.begin(), reserveAllies.end(), [&](const RecruitedAllyCombatSupport& reserve) {
            return reserve.name == unique.switchInName;
        });
        if (reserveIt == reserveAllies.end())
        {
            lines.push_back("- " + ally.name + " devait switch, mais la réserve demandée n'est plus disponible.");
            unique.switchRequested = false;
            unique.switchInName.clear();
            return true;
        }

        const std::string outgoingName = ally.name;
        const std::string incomingName = reserveIt->name;
        RecruitedAllyCombatSupport incoming = *reserveIt;
        reserveAllies.erase(reserveIt);
        reserveAllies.push_back(ally);

        player.recordCanonicalEvent("switch_recrues_combat", outgoingName + "->" + incomingName, "Switch tactique temporaire en combat", 1);

        lines.push_back("- " + outgoingName + " applique le switch temporaire demandé avec " + incomingName + ". La place hors combat ne change pas.");
        ally = incoming;
        unique.switchRequested = false;
        unique.switchInName.clear();
        return true;
    }

    void clearOneTurnTeamOrders(RecruitedAllyOrderState& orders)
    {
        orders.groupHealOneTurn = false;
        orders.groupGuardOneTurn = false;
        orders.groupTechniqueOneTurn = false;
        orders.groupCombinedTechniqueOneTurn = false;
        orders.groupSpreadTargetsOneTurn = false;
        for (auto& entry : orders.uniqueOrders)
        {
            entry.second.healOneTurn = false;
            entry.second.guardOneTurn = false;
            entry.second.forceTechniqueOneTurn = false;
        }
    }

    void openUniqueAllyOrderMenu(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<RecruitedAllyCombatSupport>& activeAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders
    )
    {
        (void)player;
        if (activeAllies.empty())
        {
            MessageScreen::show("CONSIGNE CIBLÉE", "combat.team_orders.unique.empty", {"Aucune recrue active à commander."}, false);
            return;
        }

        MenuScreen allyScreen("CONSIGNE CIBLÉE", "combat.team_orders.unique.select");
        allyScreen.addLine("Choisis une recrue précise. Donner une consigne ne consomme pas le tour du joueur.");
        allyScreen.addLine("Une recrue faible rang lit mal le combat seule, mais un ordre clair augmente ses chances d'agir correctement.");
        allyScreen.addLine("Switch existe seulement ici, jamais dans les consignes de groupe, et se déclenche au tour de la recrue concernée.");
        allyScreen.addBackOption("Retour", "combat.team_orders.unique.back");
        for (std::size_t i = 0; i < activeAllies.size(); ++i)
        {
            const RecruitedAllyCombatSupport& ally = activeAllies[i];
            allyScreen.addOption(
                static_cast<int>(i + 1),
                ally.name,
                ally.job + " | rang " + recruitedAllyRankName(ally.currentRankIndex) + " | niv. " + std::to_string(ally.estimatedLevel),
                true,
                "combat.team_orders.unique.ally"
            );
        }
        const int allyChoice = TerminalInterface::askMenuChoiceFromOptions(allyScreen, "Choisis une recrue.");
        Console::clear();
        if (allyChoice <= 0 || allyChoice > static_cast<int>(activeAllies.size()))
        {
            return;
        }

        RecruitedAllyCombatSupport& ally = activeAllies[static_cast<std::size_t>(allyChoice - 1)];
        RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
        MenuScreen orderScreen("CONSIGNE CIBLÉE — " + ally.name, "combat.team_orders.unique.action");
        orderScreen.addLine("Consignes de soin/protection : actives 1 tour, à redire chaque fois.");
        orderScreen.addLine("Maturité : " + recruitedAllyMaturityName(ally) + " | les ordres aident surtout les recrues peu expérimentées.");
        orderScreen.addLine("Priorité de cible : reste active jusqu'à la mort/disparition de la cible.");
        orderScreen.addBackOption("Retour", "combat.team_orders.unique.action.back");
        orderScreen.addOption(1, "Demander du soin", "La recrue tente de soigner/couvrir le chef à son prochain tour. Consigne 1 tour.", true, "combat.team_orders.unique.heal");
        orderScreen.addOption(2, "Prioriser une cible", "La recrue visera cette cible jusqu'à sa mort/disparition.", wave.hasActiveEnemies(), "combat.team_orders.unique.focus");
        orderScreen.addOption(3, "Switch", "Échange cette recrue avec une recrue en réserve. Déclenché au tour de la recrue.", !reserveAllies.empty(), "combat.team_orders.unique.switch");
        orderScreen.addOption(4, "Protéger le chef", "Posture prudente pendant 1 tour : un peu moins de dégâts, un peu plus de couverture.", true, "combat.team_orders.unique.guard");
        orderScreen.addOption(5, "Forcer une technique", "Si sa technique est prête, la recrue l'utilisera à son prochain tour. Consigne 1 tour.", ally.activeSkillCooldown <= 0, "combat.team_orders.unique.technique");
        orderScreen.addOption(6, "Annuler la consigne ciblée", "Retire soin/protection/switch/priorité/technique pour cette recrue.", true, "combat.team_orders.unique.clear");
        const int orderChoice = TerminalInterface::askMenuChoiceFromOptions(orderScreen, "Choisis la consigne ciblée.");
        Console::clear();

        if (orderChoice == 1)
        {
            unique.healOneTurn = true;
            MessageScreen::show("CONSIGNE DONNÉE", "combat.team_orders.unique.heal.done", {ally.name + " tentera de soigner/couvrir au prochain tour.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (orderChoice == 2)
        {
            const std::string targetName = selectActiveEnemyNameForOrder(wave, "PRIORITÉ UNIQUE", "combat.team_orders.unique.focus.target");
            if (!targetName.empty())
            {
                unique.hasPriorityTarget = true;
                unique.priorityTargetName = targetName;
                MessageScreen::show("PRIORITÉ ENREGISTRÉE", "combat.team_orders.unique.focus.done", {ally.name + " priorisera " + targetName + " jusqu'à sa mort/disparition."}, false);
            }
        }
        else if (orderChoice == 3)
        {
            MenuScreen reserveScreen("SWITCH — RÉSERVE", "combat.team_orders.unique.switch.reserve");
            reserveScreen.addLine("Le switch consommera le tour de " + ally.name + ", pas le tour du joueur.");
            reserveScreen.addBackOption("Retour", "combat.team_orders.unique.switch.back");
            for (std::size_t i = 0; i < reserveAllies.size(); ++i)
            {
                const RecruitedAllyCombatSupport& reserve = reserveAllies[i];
                reserveScreen.addOption(
                    static_cast<int>(i + 1),
                    reserve.name,
                    reserve.job + " | rang " + recruitedAllyRankName(reserve.currentRankIndex) + " | niv. " + std::to_string(reserve.estimatedLevel),
                    true,
                    "combat.team_orders.unique.switch.target"
                );
            }
            const int reserveChoice = TerminalInterface::askMenuChoiceFromOptions(reserveScreen, "Choisis la recrue en réserve.");
            Console::clear();
            if (reserveChoice > 0 && reserveChoice <= static_cast<int>(reserveAllies.size()))
            {
                unique.switchRequested = true;
                unique.switchInName = reserveAllies[static_cast<std::size_t>(reserveChoice - 1)].name;
                MessageScreen::show("SWITCH PLANIFIÉ", "combat.team_orders.unique.switch.done", {ally.name + " switchera avec " + unique.switchInName + " pendant son propre tour."}, false);
            }
        }
        else if (orderChoice == 4)
        {
            unique.guardOneTurn = true;
            MessageScreen::show("CONSIGNE DONNÉE", "combat.team_orders.unique.guard.done", {ally.name + " jouera plus prudemment pendant 1 tour."}, false);
        }
        else if (orderChoice == 5)
        {
            unique.forceTechniqueOneTurn = true;
            MessageScreen::show("TECHNIQUE DEMANDÉE", "combat.team_orders.unique.technique.done", {ally.name + " utilisera sa technique active au prochain tour si la situation le permet.", "Maturité " + recruitedAllyMaturityName(ally) + " : l'ordre aide, mais ne transforme pas une recrue en vétéran.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (orderChoice == 6)
        {
            orders.uniqueOrders.erase(ally.name);
            MessageScreen::show("CONSIGNE ANNULÉE", "combat.team_orders.unique.clear.done", {"Les consignes ciblées de " + ally.name + " sont retirées."}, false);
        }
    }

    void openGroupAllyOrderMenu(EnemyCombatQueue& wave, RecruitedAllyOrderState& orders)
    {
        MenuScreen screen("CONSIGNE DE GROUPE", "combat.team_orders.group");
        screen.addLine("Consignes de soin/protection : actives 1 tour, à redire chaque fois.");
        screen.addLine("Les recrues faibles réagissent mieux avec une consigne directe qu'avec une consigne de groupe trop vague.");
        screen.addLine("Priorité de cible : reste active jusqu'à la mort/disparition de la cible.");
        screen.addLine("Switch indisponible ici : le switch est uniquement une consigne ciblée.");
        screen.addBackOption("Retour", "combat.team_orders.group.back");
        screen.addOption(1, "Demander du soin au groupe", "Les soutiens/alliés disponibles tenteront de couvrir le chef ce tour. Consigne 1 tour.", true, "combat.team_orders.group.heal");
        screen.addOption(2, "Prioriser une cible", "Toute l'équipe active visera cette cible jusqu'à sa mort/disparition.", wave.hasActiveEnemies(), "combat.team_orders.group.focus");
        screen.addOption(3, "Tenir la ligne", "Consigne prudente 1 tour : moins d'agression, plus de couverture.", true, "combat.team_orders.group.guard");
        screen.addOption(4, "Percée coordonnée", "Les recrues dont la technique est prête tenteront chacune leur technique ce tour. Consigne 1 tour.", true, "combat.team_orders.group.technique");
        screen.addOption(5, "Technique combinée", "Les deux recrues actives dépensent leurs deux tours dans une seule technique de duo si leurs techniques sont prêtes.", true, "combat.team_orders.group.combo");
        screen.addOption(6, "Répartir les cibles", "L'équipe évite de tous taper la même cible ce tour, utile contre plusieurs ennemis.", wave.getActiveEnemyCount() >= 2, "combat.team_orders.group.spread");
        screen.addOption(7, "Annuler la priorité de groupe", "Retire la cible prioritaire globale.", orders.groupPriorityTarget, "combat.team_orders.group.clear_focus");
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis la consigne de groupe.");
        Console::clear();
        if (choice == 1)
        {
            orders.groupHealOneTurn = true;
            MessageScreen::show("CONSIGNE DE GROUPE", "combat.team_orders.group.heal.done", {"Le groupe tentera de couvrir/soigner ce tour.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (choice == 2)
        {
            const std::string targetName = selectActiveEnemyNameForOrder(wave, "PRIORITÉ DE GROUPE", "combat.team_orders.group.focus.target");
            if (!targetName.empty())
            {
                orders.groupPriorityTarget = true;
                orders.groupPriorityTargetName = targetName;
                MessageScreen::show("PRIORITÉ DE GROUPE", "combat.team_orders.group.focus.done", {"Toute l'équipe priorisera " + targetName + " jusqu'à sa mort/disparition."}, false);
            }
        }
        else if (choice == 3)
        {
            orders.groupGuardOneTurn = true;
            MessageScreen::show("CONSIGNE DE GROUPE", "combat.team_orders.group.guard.done", {"L'équipe tiendra la ligne pendant 1 tour."}, false);
        }
        else if (choice == 4)
        {
            orders.groupTechniqueOneTurn = true;
            MessageScreen::show("PERCÉE COORDONNÉE", "combat.team_orders.group.technique.done", {"Les recrues dont la technique est prête tenteront une action active ce tour.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (choice == 5)
        {
            orders.groupCombinedTechniqueOneTurn = true;
            MessageScreen::show("TECHNIQUE COMBINÉE", "combat.team_orders.group.combo.done", {"Les deux recrues actives tenteront une vraie technique de duo.", "Le duo consomme leurs deux tours alliés et demande leurs deux techniques prêtes.", "Le résultat dépend de leurs profils : garde, soutien, distance ou assaut."}, false);
        }
        else if (choice == 6)
        {
            orders.groupSpreadTargetsOneTurn = true;
            MessageScreen::show("CIBLES RÉPARTIES", "combat.team_orders.group.spread.done", {"L'équipe répartira ses cibles pendant 1 tour au lieu de tunnel automatiquement.", "Cette consigne est utile contre plusieurs ennemis, moins contre un boss seul."}, false);
        }
        else if (choice == 7)
        {
            orders.groupPriorityTarget = false;
            orders.groupPriorityTargetName.clear();
            MessageScreen::show("PRIORITÉ ANNULÉE", "combat.team_orders.group.clear_focus.done", {"La priorité de groupe est retirée."}, false);
        }
    }

    bool openTeamOrdersMenu(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<RecruitedAllyCombatSupport>& activeAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders
    )
    {
        if (activeAllies.empty())
        {
            MessageScreen::show("CONSIGNES D'ÉQUIPE", "combat.team_orders.empty", {"Aucune recrue équipée ne peut recevoir de consigne."}, false);
            return false;
        }

        MenuScreen screen("CONSIGNES D'ÉQUIPE", "combat.team_orders");
        screen.addLine("Donner une consigne ne consomme pas le tour du joueur.");
        screen.addLine("Une recrue faible rang lit moins bien le combat, utilise moins de techniques et suit mieux une consigne claire qu'un instinct flou.");
        screen.addLine("Consigne ciblée : vise une recrue précise. Switch disponible uniquement ici.");
        screen.addLine("Consigne de groupe : consigne globale. Soin/protection/technique/répartition = 1 tour ; priorité cible = jusqu'à mort/disparition.");
        if (orders.groupPriorityTarget)
        {
            screen.addLine("Priorité de groupe actuelle : " + orders.groupPriorityTargetName + ".");
        }
        screen.addBackOption("Retour", "combat.team_orders.back");
        screen.addOption(1, "Consigne ciblée", "Demander du soin, prioriser une cible, protéger le chef ou switch une recrue précise.", true, "combat.team_orders.unique");
        screen.addOption(2, "Consigne de groupe", "Soin global, cible prioritaire, tenir la ligne, percée coordonnée ou cibles réparties. Pas de switch ici.", true, "combat.team_orders.group");
        screen.addOption(3, "Voir l'état des consignes", "Récapitulatif des priorités et consignes 1 tour.", true, "combat.team_orders.status");
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un type de consigne.");
        Console::clear();
        if (choice == 1)
        {
            openUniqueAllyOrderMenu(player, wave, activeAllies, reserveAllies, orders);
        }
        else if (choice == 2)
        {
            openGroupAllyOrderMenu(wave, orders);
        }
        else if (choice == 3)
        {
            std::vector<std::string> lines;
            lines.push_back("Actifs : " + std::to_string(activeAllies.size()) + " | réserve : " + std::to_string(reserveAllies.size()) + ".");
            lines.push_back("Soin groupe 1 tour : " + std::string(orders.groupHealOneTurn ? "oui" : "non") + ".");
            lines.push_back("Tenir ligne 1 tour : " + std::string(orders.groupGuardOneTurn ? "oui" : "non") + ".");
            lines.push_back("Percée coordonnée 1 tour : " + std::string(orders.groupTechniqueOneTurn ? "oui" : "non") + ".");
            lines.push_back("Technique combinée 1 tour : " + std::string(orders.groupCombinedTechniqueOneTurn ? "oui" : "non") + ".");
            lines.push_back("Répartition cibles 1 tour : " + std::string(orders.groupSpreadTargetsOneTurn ? "oui" : "non") + ".");
            lines.push_back("Priorité groupe : " + std::string(orders.groupPriorityTarget ? orders.groupPriorityTargetName : "aucune") + ".");
            for (const auto& entry : orders.uniqueOrders)
            {
                const RecruitedAllyUniqueOrder& order = entry.second;
                lines.push_back("- " + entry.first + " : soin=" + std::string(order.healOneTurn ? "oui" : "non")
                    + " | protection=" + std::string(order.guardOneTurn ? "oui" : "non")
                    + " | technique=" + std::string(order.forceTechniqueOneTurn ? "oui" : "non")
                    + " | priorité=" + std::string(order.hasPriorityTarget ? order.priorityTargetName : "aucune")
                    + " | switch=" + std::string(order.switchRequested ? order.switchInName : "non") + ".");
            }
            MessageScreen::show("ÉTAT DES CONSIGNES", "combat.team_orders.status", lines, false);
        }
        return false;
    }

    void displayRecruitedAllyCombatStart(const Player& player, const std::vector<RecruitedAllyCombatSupport>& allies)
    {
        if (allies.empty())
        {
            return;
        }
        std::vector<std::string> lines;
        lines.push_back("Les recrues équipées entrent maintenant dans le vrai combat PvE, en soutien simple joueur + 2 alliés max.");
        lines.push_back("Une recrue bas rang observe moins de choses, utilise moins de techniques et progresse surtout avec des ordres clairs.");
        lines.push_back("Placement : chef " + player.getName() + ", puis chaîne manuelle si elle existe, sinon fallback rang/niveau.");
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            lines.push_back("- " + ally.name + " | " + ally.job + " | rang " + recruitedAllyRankName(ally.currentRankIndex) + " | niv. " + std::to_string(ally.estimatedLevel) + " | " + recruitedAllyHealthLine(ally) + " | part combat " + std::to_string(ally.normalizedShare) + "%.");
            lines.push_back("  Présence : " + recruitedAllyReadinessLine(ally, player) + ".");
        }
        MessageScreen::show("RECRUES ÉQUIPÉES", "combat.pve.recruited_allies.start", lines, false);
    }

    std::string recruitedAllyPairKey(const RecruitedAllyCombatSupport& a, const RecruitedAllyCombatSupport& b)
    {
        if (a.name <= b.name) return a.name + "|" + b.name;
        return b.name + "|" + a.name;
    }

    bool recruitedAllyIsRangedProfile(const RecruitedAllyCombatSupport& ally)
    {
        return recruitedAllyJobContains(ally, "archer")
            || recruitedAllyJobContains(ally, "mage")
            || recruitedAllyJobContains(ally, "tireur");
    }

    bool recruitedAllyIsSupportProfile(const RecruitedAllyCombatSupport& ally)
    {
        return recruitedAllyJobContains(ally, "soigneur")
            || recruitedAllyJobContains(ally, "mage d'appui")
            || recruitedAllyJobContains(ally, "intendant");
    }

    bool recruitedAllyIsTankProfile(const RecruitedAllyCombatSupport& ally)
    {
        return recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank");
    }

    bool recruitedAllyIsAssaultProfile(const RecruitedAllyCombatSupport& ally)
    {
        return recruitedAllyJobContains(ally, "assassin")
            || recruitedAllyJobContains(ally, "roublard")
            || recruitedAllyJobContains(ally, "brigand")
            || recruitedAllyJobContains(ally, "lancier");
    }

    bool executeRecruitedAllyCombinedTechnique(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        std::vector<RecruitedAllyCombatSupport>& allies,
        RecruitedAllyOrderState& orders,
        std::vector<std::string>& lines
    )
    {
        if (!orders.groupCombinedTechniqueOneTurn || allies.size() < 2 || !wave.hasActiveEnemies())
        {
            return false;
        }

        RecruitedAllyCombatSupport& first = allies[0];
        RecruitedAllyCombatSupport& second = allies[1];
        if (first.currentHp <= 0 || second.currentHp <= 0)
        {
            lines.push_back("Technique combinée annulée : les deux recrues doivent être encore debout.");
            return false;
        }
        if (first.activeSkillCooldown > 0 || second.activeSkillCooldown > 0)
        {
            lines.push_back("Technique combinée impossible : les deux techniques personnelles doivent être prêtes.");
            return false;
        }

        ++first.turnsTaken;
        ++second.turnsTaken;

        const bool firstTank = recruitedAllyIsTankProfile(first);
        const bool secondTank = recruitedAllyIsTankProfile(second);
        const bool firstSupport = recruitedAllyIsSupportProfile(first);
        const bool secondSupport = recruitedAllyIsSupportProfile(second);
        const bool firstRanged = recruitedAllyIsRangedProfile(first);
        const bool secondRanged = recruitedAllyIsRangedProfile(second);
        const bool firstAssault = recruitedAllyIsAssaultProfile(first);
        const bool secondAssault = recruitedAllyIsAssaultProfile(second);

        std::string techniqueName = "Assaut synchronisé";
        int powerPercent = 100;
        int vulnerability = 0;
        int weakening = 0;

        if ((firstTank && secondAssault) || (secondTank && firstAssault))
        {
            techniqueName = "Brèche sous garde";
            powerPercent = 112;
            vulnerability = 18;
            weakening = 8;
        }
        else if ((firstSupport && !secondSupport) || (secondSupport && !firstSupport))
        {
            techniqueName = "Faille relayée";
            powerPercent = 92;
            vulnerability = 24;
        }
        else if (firstRanged && secondRanged)
        {
            techniqueName = "Feu croisé";
            powerPercent = 120;
            vulnerability = 10;
        }
        else if (firstRanged != secondRanged)
        {
            techniqueName = "Croisement de lignes";
            powerPercent = 108;
            weakening = 10;
        }

        Monster& target = wave.getActiveEnemy(0);
        const std::string targetName = target.getName();
        const int maturity = recruitedAllyMaturityTier(first) + recruitedAllyMaturityTier(second);
        int baseDamage = std::max(6,
            first.estimatedLevel / 2 + second.estimatedLevel / 2
            + first.weaponQuality + second.weaponQuality
            + first.currentRankIndex * 3 + second.currentRankIndex * 3
            + maturity * 2 + random.between(4, 11));
        if (player.hasPassiveSkill("church_oath_bonds"))
        {
            baseDamage += 2;
        }
        const int damage = std::max(3, baseDamage * powerPercent / 100);
        target.takeDamage(damage);
        if (!target.isDead() && vulnerability > 0) target.applyVulnerability(2, vulnerability);
        if (!target.isDead() && weakening > 0) target.applyWeakening(2, weakening);
        const bool killed = target.isDead();
        wave.removeDeadAndReplace();

        first.damageDealt += damage / 2;
        second.damageDealt += damage - damage / 2;
        if (killed)
        {
            ++first.finishBlows;
            ++second.finishBlows;
        }
        first.activeSkillCooldown = std::max(2, 4 - recruitedAllyMaturityTier(first) / 2);
        second.activeSkillCooldown = std::max(2, 4 - recruitedAllyMaturityTier(second) / 2);

        const std::string pairKey = recruitedAllyPairKey(first, second);
        player.recordCanonicalEvent(
            "techniques_combinees_alliees",
            pairKey + ":" + techniqueName,
            first.name + " + " + second.name + " : " + techniqueName,
            1
        );
        player.recordHistoricalEvent(
            "ally_combo",
            "duo:" + pairKey,
            first.name + " et " + second.name + " exécutent " + techniqueName + " sur " + targetName + "."
        );

        lines.push_back("TECHNIQUE COMBINÉE — " + techniqueName + " : " + first.name + " et " + second.name + " consomment leurs deux tours alliés.");
        lines.push_back("- Cible : " + targetName + " | dégâts : " + std::to_string(damage) + (killed ? " | cible vaincue." : "."));
        if (vulnerability > 0 && !killed) lines.push_back("- Effet duo : vulnérabilité " + std::to_string(vulnerability) + "% pendant 2 tours.");
        if (weakening > 0 && !killed) lines.push_back("- Effet duo : affaiblissement " + std::to_string(weakening) + " pendant 2 tours.");
        lines.push_back("- Récupération : " + first.name + " CD " + std::to_string(first.activeSkillCooldown) + " | " + second.name + " CD " + std::to_string(second.activeSkillCooldown) + ".");
        return true;
    }

    void playRecruitedAllyCombatTurns(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        std::vector<RecruitedAllyCombatSupport>& allies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders
    )
    {
        if (allies.empty() || !wave.hasActiveEnemies())
        {
            clearOneTurnTeamOrders(orders);
            return;
        }

        std::vector<std::string> lines;
        lines.push_back("Les recrues équipées agissent après le joueur et les invocations, avant la riposte ennemie.");
        lines.push_back("Les consignes de soin/protection/technique/répartition expirent après ce tour allié. Les priorités de cible restent jusqu'à mort/disparition.");

        if (executeRecruitedAllyCombinedTechnique(player, wave, random, allies, orders, lines))
        {
            clearOneTurnTeamOrders(orders);
            MessageScreen::show("TOUR DES RECRUES ÉQUIPÉES", "combat.pve.recruited_allies.combo", lines, false);
            return;
        }

        for (RecruitedAllyCombatSupport& ally : allies)
        {
            if (!wave.hasActiveEnemies())
            {
                break;
            }
            ++ally.turnsTaken;
            if (ally.activeSkillCooldown > 0)
            {
                --ally.activeSkillCooldown;
            }
            ally.protectedThisRound = false;
            RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
            const bool hasActiveOrder = unique.healOneTurn
                || unique.guardOneTurn
                || unique.forceTechniqueOneTurn
                || unique.hasPriorityTarget
                || unique.switchRequested
                || orders.groupHealOneTurn
                || orders.groupGuardOneTurn
                || orders.groupTechniqueOneTurn
                || orders.groupCombinedTechniqueOneTurn
                || orders.groupSpreadTargetsOneTurn
                || orders.groupPriorityTarget;
            lines.push_back("- Présence alliée : " + recruitedAllyPresenceLine(ally, player, wave) + ".");
            const std::string tacticalComment = recruitedAllyTacticalCommentLine(ally, player, wave, hasActiveOrder);
            if (!tacticalComment.empty())
            {
                lines.push_back("  Lecture alliée : " + tacticalComment + ".");
            }

            if (executeRequestedAllySwitch(player, ally, reserveAllies, unique, lines))
            {
                continue;
            }

            const bool orderedHeal = unique.healOneTurn || orders.groupHealOneTurn;
            const bool orderedGuard = unique.guardOneTurn || orders.groupGuardOneTurn;
            const bool naturalSupport = (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
                && player.getHp() * 10 < player.getMaxHp() * 7
                && random.between(1, 100) <= recruitedAllyNaturalSupportChance(ally);

            if (ally.currentHp * 3 < ally.maxHp && ally.healingPotionCharges > 0)
            {
                const int selfHeal = std::max(5, ally.maxHp / 3);
                ally.currentHp = std::min(ally.maxHp, ally.currentHp + selfHeal);
                --ally.healingPotionCharges;
                ally.healingDone += selfHeal;
                recordRecruitedAllySupportContribution(ally, 1);
                lines.push_back("- " + ally.name + " boit une potion personnelle : +" + std::to_string(selfHeal) + " PV (" + recruitedAllyHealthLine(ally) + ").");
                continue;
            }

            if ((orderedHeal || naturalSupport) && player.getHp() < player.getMaxHp())
            {
                const int healAmount = std::max(3, player.getMaxHp() / 18 + ally.currentRankIndex * 2 + ally.supportKitQuality + random.between(0, 4) + (orderedHeal ? 2 : 0));
                player.heal(healAmount);
                ally.healingDone += healAmount;
                recordRecruitedAllySupportContribution(ally, 2);
                lines.push_back("- " + ally.name + (orderedHeal ? " suit la consigne de soin : +" : " couvre le chef : +") + std::to_string(healAmount) + " PV.");
                continue;
            }

            if (orderedHeal && player.getHp() >= player.getMaxHp())
            {
                lines.push_back("- " + ally.name + " garde la consigne de soin en tête, mais le chef est déjà au maximum. Passage en attaque prudente.");
            }

            if (executeRecruitedAllyActiveTechnique(ally, wave, random, orders, lines))
            {
                continue;
            }

            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            int damage = std::max(3, ally.estimatedLevel / 2 + ally.currentRankIndex * 5 + ally.weaponQuality * 2 + player.getLevel() / 5 + random.between(2, 10));
            if (recruitedAllyJobContains(ally, "gardien"))
            {
                damage = std::max(2, damage - 2);
            }
            if (recruitedAllyIsStillLearning(ally))
            {
                damage = std::max(2, damage * 78 / 100);
            }
            else if (recruitedAllyMaturityTier(ally) == 2)
            {
                damage = std::max(2, damage * 92 / 100);
            }
            if (orderedGuard)
            {
                ally.protectedThisRound = true;
                damage = std::max(2, damage - 3);
                const int guardHeal = std::max(1, ally.currentRankIndex + 1 + ally.supportKitQuality / 3);
                if (player.getHp() < player.getMaxHp())
                {
                    player.heal(guardHeal);
                    ally.healingDone += guardHeal;
                }
                recordRecruitedAllySupportContribution(ally, 1);
                lines.push_back("- " + ally.name + " tient la ligne : couverture légère du chef" + (player.getHp() < player.getMaxHp() ? "" : "") + ".");
            }
            if (recruitedAllyJobContains(ally, "roublard") && target.getHp() * 2 < target.getMaxHp())
            {
                damage += std::max(2, ally.currentRankIndex + 3);
            }
            const std::string targetName = target.getName();
            target.takeDamage(damage);
            const bool killed = target.isDead();
            const std::string targetResultLine = recruitedAllyTargetResultLine(ally, target, killed);
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);

            const bool hasUniqueFocus = orders.uniqueOrders[ally.name].hasPriorityTarget;
            const bool hasGroupFocus = orders.groupPriorityTarget;
            std::string orderHint;
            if (hasUniqueFocus || hasGroupFocus)
            {
                orderHint = " (priorité suivie).";
            }
            else if (orders.groupSpreadTargetsOneTurn)
            {
                orderHint = " (cibles réparties).";
            }
            else if (recruitedAllyJobContains(ally, "archer") || recruitedAllyJobContains(ally, "roublard"))
            {
                orderHint = " (instinct/cible parfois aléatoire).";
            }
            else
            {
                orderHint = ".";
            }
            lines.push_back("- " + ally.name + " attaque " + targetName + " : " + std::to_string(damage) + " dégâts" + orderHint + " " + targetResultLine);
        }
        clearOneTurnTeamOrders(orders);
        MessageScreen::show("TOUR DES RECRUES ÉQUIPÉES", "combat.pve.recruited_allies.turn", lines, false);
    }

    void persistRecruitedAllyVitals(Player& player, const RecruitedAllyCombatSupport& ally, const std::string& status)
    {
        if (ally.name.empty())
        {
            return;
        }
        std::ostringstream label;
        label << ally.name
              << " | hp=" << std::clamp(ally.currentHp, 0, std::max(1, ally.maxHp))
              << " | max=" << std::max(1, ally.maxHp)
              << " | potions=" << std::clamp(ally.healingPotionCharges, 0, 9)
              << " | statut=" << (status.empty() ? "stable" : status)
              << " | jour=" << player.getWorldDaysElapsed();
        player.recordCanonicalEvent("pv_recrues_persistants", ally.name, label.str(), 1);
    }

    void persistRecruitedAllyVitals(Player& player, const std::vector<RecruitedAllyCombatSupport>& allies, const std::string& status)
    {
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            persistRecruitedAllyVitals(player, ally, status);
        }
    }

    void sendRecruitedAllyNameToInfirmary(
        Player& player,
        const std::string& recruitName,
        int recoveryDays,
        const std::string& severity,
        int exitHealthPercent,
        const std::string& origin,
        std::vector<std::string>& lines
    )
    {
        if (recruitName.empty())
        {
            return;
        }
        const int readyDay = player.getWorldDaysElapsed() + std::max(1, recoveryDays);
        std::ostringstream label;
        label << recruitName
              << " | pret_jour=" << readyDay
              << " | gravite=" << severity
              << " | ville=" << player.getCurrentCityId()
              << " | sortie_pct=" << std::clamp(exitHealthPercent, 1, 100)
              << " | origine=" << origin;
        player.recordCanonicalEvent("recrues_infirmerie_sejours", recruitName, label.str(), 1);
        player.recordCanonicalEvent("recrues_evacuees_infirmerie", recruitName, recruitName + " amené à l'infirmerie", 1);
        lines.push_back("- " + recruitName + " est pris en charge par l'infirmerie. Retour possible jour " + std::to_string(readyDay) + ", au seuil de PV imposé par la difficulté.");
    }

    void markRecruitedAllyAwaitingInfirmaryTransfer(
        Player& player,
        const RecruitedAllyCombatSupport& ally,
        int recoveryDaysHint,
        const std::string& severity,
        DifficultyMode difficulty,
        std::vector<std::string>& lines
    )
    {
        persistRecruitedAllyVitals(player, ally, "ko_a_evacuer");
        std::ostringstream label;
        label << ally.name
              << " | gravite=" << severity
              << " | ville=" << player.getCurrentCityId()
              << " | jour_chute=" << player.getWorldDaysElapsed()
              << " | repos_base=" << std::max(1, recoveryDaysHint)
              << " | sortie_pct=" << recruitedAllyRespawnHealthPercent(difficulty);
        player.recordCanonicalEvent("recrues_a_evacuer", ally.name, label.str(), 1);
        player.recordCanonicalEvent("recrues_tombees_combat", ally.name, ally.name + " tombe au combat et attend une évacuation", 1);
        lines.push_back("- " + ally.name + " tombe à 0 PV. Elle n'est pas téléportée : il faudra l'amener à l'infirmerie avant qu'elle redevienne opérationnelle.");
    }

    void transferPendingFallenRecruitsToInfirmary(
        Player& player,
        DifficultyMode difficulty,
        Random& random,
        const std::string& contextId,
        std::vector<std::string>& lines
    )
    {
        int moved = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "recrues_a_evacuer" || record.count <= 0)
            {
                continue;
            }
            const std::string recruitName = record.key.empty() ? record.label : record.key;
            if (recruitName.empty())
            {
                continue;
            }
            if (localJournalCount(player, "recrues_evacuees_infirmerie", recruitName) >= record.count)
            {
                continue;
            }
            const std::string severity = extractLabelStringField(record.label, "gravite", "surveillance");
            int recoveryDays = extractLabelIntField(record.label, "repos_base", severity == "grave" ? random.between(2, 5) : random.between(1, 2));
            if (contextId.find("joueur_ko") != std::string::npos || contextId.find("player") != std::string::npos)
            {
                recoveryDays = std::max(recoveryDays, severity == "grave" ? 3 : 1);
            }
            const int exitPercent = extractLabelIntField(record.label, "sortie_pct", recruitedAllyRespawnHealthPercent(difficulty));
            sendRecruitedAllyNameToInfirmary(player, recruitName, recoveryDays, severity, exitPercent, contextId, lines);
            ++moved;
        }
        if (moved > 0)
        {
            lines.push_back("Les recrues tombées suivent le transfert vers l'infirmerie : personne ne redevient disponible sans soin ni récupération.");
        }
    }

    void resolveRecruitedAllyInjuriesAfterEnemyTurn(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<RecruitedAllyCombatSupport>& allies,
        RecruitedAllyOrderState& orders,
        Random& random,
        DifficultyMode difficulty,
        int playerHpBeforeEnemyTurn
    )
    {
        if (allies.empty())
        {
            return;
        }

        const int hpLost = std::max(0, playerHpBeforeEnemyTurn - player.getHp());
        int baseRisk = 2 + std::min(12, hpLost / std::max(1, player.getMaxHp() / 12));
        if (player.isDead())
        {
            baseRisk += 18;
        }
        if (baseRisk <= 2 && random.between(1, 100) > 5)
        {
            return;
        }

        std::vector<std::string> lines;
        for (auto it = allies.begin(); it != allies.end();)
        {
            RecruitedAllyCombatSupport& ally = *it;
            if (ally.turnsTaken <= 0)
            {
                ++it;
                continue;
            }

            int risk = baseRisk + std::max(0, ally.turnsTaken - 1) * 2;
            if (recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank"))
            {
                risk += 3;
            }
            if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
            {
                risk += 2;
            }
            risk = std::clamp(risk, 1, 35);
            if (random.between(1, 100) > risk)
            {
                ++it;
                continue;
            }

            const int enemyPressure = wave.hasActiveEnemies()
                ? std::max(1, wave.getActiveEnemy(0).getLevel())
                : std::max(1, player.getLevel());
            int damage = std::max(3, enemyPressure / 2 + random.between(3, 10) + hpLost / std::max(4, player.getMaxHp() / 8));
            if (orders.groupGuardOneTurn || orders.uniqueOrders[ally.name].guardOneTurn || ally.protectedThisRound)
            {
                damage = std::max(1, damage * 60 / 100);
            }
            damage = std::max(1, damage - std::max(0, ally.armorQuality / 2));
            if (recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank"))
            {
                damage = std::max(1, damage * 85 / 100);
            }
            ally.currentHp -= damage;
            lines.push_back("- " + ally.name + " encaisse " + std::to_string(damage) + " dégâts collatéraux (" + recruitedAllyHealthLine(ally) + ").");
            if (ally.currentHp > 0)
            {
                ++it;
                continue;
            }

            const bool severe = player.isDead() || hpLost > player.getMaxHp() / 3 || damage > ally.maxHp / 3 || random.between(1, 100) <= 25;
            const int recoveryDays = severe ? random.between(2, 4) : 1;
            const std::string severity = severe ? "grave" : "surveillance";
            markRecruitedAllyAwaitingInfirmaryTransfer(player, ally, recoveryDays, severity, difficulty, lines);
            orders.uniqueOrders.erase(ally.name);
            it = allies.erase(it);
        }

        if (!lines.empty())
        {
            lines.push_back("Une recrue tombée reste hors de la ligne tant que l'infirmerie ne l'a pas remise sur pied.");
            MessageScreen::show("INFIRMERIE DES RECRUES", "combat.pve.recruited_allies.infirmary", lines, false);
        }
    }


    int estimatePlayerInfirmaryCostCopper(const Player& player, DifficultyMode difficulty, bool rescuedByTeam)
    {
        int base = 28 + player.getLevel() * 7;
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                base = base * 70 / 100;
                break;
            case DifficultyMode::Normal:
                break;
            case DifficultyMode::Hard:
                base = base * 125 / 100;
                break;
            case DifficultyMode::Nightmare:
                base = base * 145 / 100;
                break;
            case DifficultyMode::Lethal:
                base = base * 160 / 100;
                break;
        }
        if (rescuedByTeam)
        {
            base = base * 75 / 100;
        }
        return std::max(15, base);
    }

    long long recruitedAllyWeeklyGoldPartsTotal(const Player& player)
    {
        const std::string currentWeekSuffix = ":semaine_" + std::to_string(std::max(0, player.getWorldDaysElapsed() / 7));
        long long total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "parts_or_recrues")
            {
                continue;
            }
            if (record.key.find(currentWeekSuffix) != std::string::npos)
            {
                total += std::max(0, record.count);
            }
        }
        return total;
    }

    void resolvePlayerInfirmaryTransferAfterNonLethalDeath(
        Player& player,
        DifficultyMode difficulty,
        Random& random,
        bool rescuedByTeam,
        bool teamFinishedFight,
        const std::vector<RecruitedAllyCombatSupport>& remainingAllies,
        const std::string& contextId
    )
    {
        const int costCopper = estimatePlayerInfirmaryCostCopper(player, difficulty, rescuedByTeam);
        int recoveryDays = teamFinishedFight
            ? random.between(1, 2)
            : random.between(2, 4);
        if (!rescuedByTeam)
        {
            recoveryDays += random.between(0, 1);
        }
        if (difficulty == DifficultyMode::Hard || difficulty == DifficultyMode::Nightmare)
        {
            recoveryDays += 1;
        }
        recoveryDays = std::clamp(recoveryDays, 1, 6);
        const int startDay = player.getWorldDaysElapsed();
        const int readyDay = startDay + std::max(1, recoveryDays);
        std::vector<std::string> lines;

        if (rescuedByTeam)
        {
            const long long teamCopper = recruitedAllyWeeklyGoldPartsTotal(player);
            const bool coveredByTeam = teamCopper >= costCopper || !remainingAllies.empty();
            player.recordCanonicalEvent(
                "infirmerie_joueur_transferts",
                contextId + ":equipe",
                "Le groupe ramène le chef à l'infirmerie | pret_jour=" + std::to_string(readyDay),
                1
            );
            player.recordCanonicalEvent(
                coveredByTeam ? "frais_infirmerie_couverts_par_groupe" : "dettes_infirmerie_groupe",
                contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                coveredByTeam
                    ? "Le groupe couvre le transport et les soins du chef."
                    : "Le groupe n'a pas assez de réserve : dette d'infirmerie notée.",
                costCopper
            );

            lines.push_back("Ton équipe te récupère après ta chute et te ramène à l'infirmerie de " + player.getCurrentCityId() + ".");
            lines.push_back(teamFinishedFight
                ? "Les recrues ont réussi à finir le combat avant le transport."
                : "Les recrues ont surtout sécurisé ton extraction avant de décrocher.");
            lines.push_back(coveredByTeam
                ? "Coût joueur : 0. Le groupe couvre les frais avec ses parts/réserves."
                : "Coût joueur immédiat : 0, mais une dette de groupe est notée : " + Money::formatCopper(costCopper) + ".");
        }
        else
        {
            const bool paid = player.getInventory().spendCopper(costCopper);
            player.recordCanonicalEvent(
                "infirmerie_joueur_transferts",
                contextId + ":secours_lambda",
                "Un groupe lambda ramène le joueur à l'infirmerie | pret_jour=" + std::to_string(readyDay),
                1
            );
            player.recordCanonicalEvent(
                paid ? "frais_infirmerie_payes" : "dettes_infirmerie_joueur",
                contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                paid ? "Facture d'infirmerie payée après sauvetage lambda." : "Facture d'infirmerie impayée après sauvetage lambda.",
                costCopper
            );

            lines.push_back("Tu étais seul : un groupe lambda te retrouve et te transporte à l'infirmerie de " + player.getCurrentCityId() + ".");
            lines.push_back(paid
                ? "Facture payée : " + Money::formatCopper(costCopper) + "."
                : "Tu n'avais pas assez d'argent : dette d'infirmerie notée pour " + Money::formatCopper(costCopper) + ".");
        }

        transferPendingFallenRecruitsToInfirmary(player, difficulty, random, contextId + ":joueur_ko", lines);
        persistRecruitedAllyVitals(player, remainingAllies, "chef_infirmerie");

        lines.push_back("Repos estimé : " + std::to_string(recoveryDays) + " jour(s). Retour opérationnel prévu jour " + std::to_string(readyDay) + ".");
        lines.push_back("Le monde avance réellement de " + std::to_string(recoveryDays) + " jour(s) pendant les soins.");
        lines.push_back("Note gameplay : en solo, le secours inconnu coûte cher ; en équipe, le groupe amortit le retour du chef, sauf manque de réserve.");
        MessageScreen::show("INFIRMERIE DU JOUEUR", "combat.pve.player.infirmary." + contextId, lines, false);
        player.advanceWorldDays(recoveryDays);
    }

    bool simulateRecruitedAlliesAfterPlayerDown(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        DifficultyMode difficulty,
        std::vector<RecruitedAllyCombatSupport>& allies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders,
        const std::string& contextId
    )
    {
        if (allies.empty() || !wave.hasEnemiesLeft())
        {
            return false;
        }

        std::vector<std::string> intro;
        intro.push_back("Tu tombes, mais le combat ne se coupe pas immédiatement : ton équipe prend le relais.");
        intro.push_back("Résolution automatique : les recrues actives tentent de finir ou de sécuriser l'extraction.");
        intro.push_back("Le switch reste temporaire au combat ; l'ordre permanent de l'équipe ne change pas.");
        MessageScreen::show("CHEF À TERRE", "combat.pve.player.down.team_continues." + contextId, intro, false);

        int rounds = 0;
        const int maxRounds = std::clamp(3 + static_cast<int>(allies.size()) + static_cast<int>(reserveAllies.size()) / 2, 3, 7);
        while (wave.hasEnemiesLeft() && !allies.empty() && rounds < maxRounds)
        {
            ++rounds;
            std::vector<std::string> lines;
            lines.push_back("Round automatique des recrues " + std::to_string(rounds) + "/" + std::to_string(maxRounds) + ".");

            for (auto it = allies.begin(); it != allies.end() && wave.hasEnemiesLeft(); )
            {
                RecruitedAllyCombatSupport& ally = *it;
                ++ally.turnsTaken;
                if (ally.activeSkillCooldown > 0)
                {
                    --ally.activeSkillCooldown;
                }
                RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
                if (executeRequestedAllySwitch(player, ally, reserveAllies, unique, lines))
                {
                    ++it;
                    continue;
                }

                if (ally.currentHp * 3 < ally.maxHp && ally.healingPotionCharges > 0)
                {
                    const int selfHeal = std::max(5, ally.maxHp / 3);
                    ally.currentHp = std::min(ally.maxHp, ally.currentHp + selfHeal);
                    --ally.healingPotionCharges;
                    ally.healingDone += selfHeal;
                    lines.push_back("- " + ally.name + " se stabilise avant de continuer : +" + std::to_string(selfHeal) + " PV.");
                    ++it;
                    continue;
                }

                if (executeRecruitedAllyActiveTechnique(ally, wave, random, orders, lines))
                {
                    ++it;
                    continue;
                }

                const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
                Monster& target = wave.getActiveEnemy(targetIndex);
                int damage = std::max(4, ally.estimatedLevel / 2 + ally.currentRankIndex * 6 + random.between(3, 13));
                if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "assassin"))
                {
                    damage += random.between(2, 7);
                }
                if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
                {
                    damage = std::max(2, damage - 3);
                    ally.healingDone += std::max(2, ally.currentRankIndex + 2);
                }
                const std::string targetName = target.getName();
                target.takeDamage(damage);
                wave.removeDeadAndReplace();
                ally.damageDealt += damage;
                lines.push_back("- " + ally.name + " garde la ligne et frappe " + targetName + " : " + std::to_string(damage) + " dégâts.");
                ++it;
            }

            if (wave.hasEnemiesLeft() && !allies.empty())
            {
                int injuryRisk = 14 + wave.getActiveEnemyCount() * 6 + std::max(0, rounds - 1) * 4;
                if (random.between(1, 100) <= injuryRisk)
                {
                    const int victimIndex = random.between(0, static_cast<int>(allies.size()) - 1);
                    RecruitedAllyCombatSupport victim = allies[static_cast<std::size_t>(victimIndex)];
                    const bool severe = random.between(1, 100) <= 35 || rounds >= maxRounds - 1;
                    markRecruitedAllyAwaitingInfirmaryTransfer(player, victim, severe ? random.between(2, 5) : 1, severe ? "grave" : "surveillance", difficulty, lines);
                    orders.uniqueOrders.erase(victim.name);
                    allies.erase(allies.begin() + victimIndex);
                }
            }

            if (allies.empty() && !reserveAllies.empty())
            {
                RecruitedAllyCombatSupport incoming = reserveAllies.front();
                reserveAllies.erase(reserveAllies.begin());
                allies.push_back(incoming);
                lines.push_back("- " + incoming.name + " quitte la réserve et couvre l'extraction du chef.");
            }

            MessageScreen::show("COMBAT AUTO DE L'ÉQUIPE", "combat.pve.player.down.team_round." + contextId, lines, false);
        }

        const bool teamFinishedFight = !wave.hasEnemiesLeft();
        player.recordCanonicalEvent(
            teamFinishedFight ? "combats_finies_par_recrues" : "extractions_par_recrues",
            contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
            teamFinishedFight ? "Les recrues finissent le combat après la chute du chef." : "Les recrues extraient le chef sans finir proprement le combat.",
            1
        );
        return teamFinishedFight;
    }

    CombatReward splitCombatRewardWithRecruitedAllies(Player& player, const CombatReward& reward, std::vector<RecruitedAllyCombatSupport>& allies, const std::string& contextId)
    {
        if (allies.empty())
        {
            return reward;
        }
        if (reward.getGold() <= 0)
        {
            persistRecruitedAllyVitals(player, allies, "combat_sans_or");
            return reward;
        }

        const int playerShare = normalizeRecruitedAllyShares(allies);
        const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
        std::vector<std::string> lines;
        lines.push_back("Récompense de groupe : " + Money::formatCopper(reward.getGold()) + " et " + std::to_string(reward.getExperience()) + " XP.");
        lines.push_back("Part du chef : " + std::to_string(playerShare) + "%. Le joueur garde la plus grosse part, même avec plusieurs alliés.");
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            const int copper = std::max(0, (reward.getGold() * ally.normalizedShare) / 100);
            const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
            const int xp = std::max(0, (reward.getExperience() * std::max(5, activityScore)) / std::max(100, activityScore + player.getLevel() * 8));
            if (copper > 0)
            {
                player.recordCanonicalEvent("recrues_equipees_semaines", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " a été équipé cette semaine", 1);
                player.recordCanonicalEvent("parts_or_recrues", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " reçoit une part de combat réel", copper);
            }
            if (xp > 0)
            {
                player.recordCanonicalEvent("xp_recrues", ally.name, ally.name + " gagne de l'expérience de combat réel", xp);
            }
            player.recordCanonicalEvent("participation_recrues", ally.name, ally.name + " participe au combat réel", std::max(1, ally.turnsTaken));
            persistRecruitedAllyVitals(player, ally, "combat_recompense");
            lines.push_back("- " + ally.name + " : " + Money::formatCopper(copper) + " (" + std::to_string(ally.normalizedShare) + "%), " + std::to_string(xp) + " XP, dégâts " + std::to_string(ally.damageDealt) + ", soutien " + std::to_string(ally.healingDone) + ", actions utiles " + std::to_string(ally.supportActions) + ", finitions " + std::to_string(ally.finishBlows) + ".");
        }
        lines.push_back("Le salaire hebdo pourra baisser seulement si la recrue était équipée ET si ces parts couvrent son salaire.");
        MessageScreen::show("PARTAGE AVEC RECRUES", "combat.pve.recruited_allies.reward." + contextId, lines, false);
        return reward.getModified(100, playerShare);
    }

    TemporaryAdventurerSupport maybeTriggerRareAdventurerCombatAid(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        TemporaryAdventurerSupport support;
        if (!wave.hasActiveEnemies())
        {
            return support;
        }

        const int roll = random.between(1, 1000);
        if (roll > 18)
        {
            return support;
        }

        support.active = true;
        support.groupName = rareCombatAidGroupName(random);
        support.remainingTurns = random.between(1, 2);
        support.initiativeSlot = 2;

        const int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
        Monster& target = wave.getActiveEnemy(targetIndex);
        const int aidDamage = std::max(4, player.getLevel() * 2 + random.between(2, 6));
        target.takeDamage(aidDamage);
        wave.removeDeadAndReplace();

        const bool healHelp = player.getHp() < player.getMaxHp();
        const int healAmount = healHelp ? std::max(3, player.getMaxHp() / 14) : 0;
        if (healAmount > 0)
        {
            player.heal(healAmount);
        }

        player.recordCanonicalEvent("aides_rares_groupes_combat", support.groupName, support.groupName + " intervient pendant un combat PvE");
        player.recordCanonicalEvent("alliés_temporaires_combat", support.groupName, support.groupName + " agit brièvement avec initiative limitée");
        MessageScreen::show(
            "AIDE RARE D'AVENTURIERS",
            "combat.pve.rare_adventurer_aid",
            {
                "Événement très rare : " + support.groupName + " passent près du combat.",
                "Ils rejoignent temporairement la file alliée : initiative limitée après le joueur et les invocations, avant les ennemis.",
                "Impact d'arrivée : " + std::to_string(aidDamage) + " dégâts infligés à une cible active." + (healAmount > 0 ? " Soin de secours : +" + std::to_string(healAmount) + " PV." : ""),
                "Durée : " + std::to_string(support.remainingTurns) + " tour(s) allié(s). C'est volontairement rare pour ne pas voler le combat au joueur."
            },
            false
        );
        return support;
    }

    void playTemporaryAdventurerSupportTurn(Player& player, EnemyCombatQueue& wave, Random& random, TemporaryAdventurerSupport& support)
    {
        if (!support.active || support.remainingTurns <= 0 || !wave.hasActiveEnemies())
        {
            return;
        }

        const int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
        Monster& target = wave.getActiveEnemy(targetIndex);
        const int damage = std::max(3, player.getLevel() * 2 + random.between(1, 7));
        target.takeDamage(damage);
        wave.removeDeadAndReplace();

        int healAmount = 0;
        if (player.getHp() * 3 < player.getMaxHp() * 2 && random.between(1, 100) <= 35)
        {
            healAmount = std::max(2, player.getMaxHp() / 18);
            player.heal(healAmount);
        }

        --support.remainingTurns;
        player.recordCanonicalEvent("tours_allies_temporaires", support.groupName, support.groupName + " prend un tour allié temporaire");
        MessageScreen::show(
            "TOUR ALLIÉ TEMPORAIRE",
            "combat.pve.temporary_adventurer_turn",
            {
                support.groupName + " agit dans une fenêtre d'initiative alliée limitée.",
                "Dégâts infligés : " + std::to_string(damage) + "." + (healAmount > 0 ? " Soin rapide : +" + std::to_string(healAmount) + " PV." : ""),
                support.remainingTurns > 0 ? "Ils restent encore un instant dans la mêlée." : "Ils quittent le combat avant de devenir ton équipe permanente."
            },
            false
        );

        if (support.remainingTurns <= 0)
        {
            support.active = false;
        }
    }

    bool monsterLooksLikeCoordinator(const Monster& monster)
    {
        const std::string text = lowerCopy(monster.getName() + " " + monster.getType() + " " + monster.getRaceText());
        if (text.find("chef") != std::string::npos || text.find("capitaine") != std::string::npos ||
            text.find("strat") != std::string::npos || text.find("shaman") != std::string::npos ||
            text.find("chamane") != std::string::npos || text.find("oracle") != std::string::npos ||
            text.find("gobelin") != std::string::npos || text.find("hobgobelin") != std::string::npos ||
            text.find("humain") != std::string::npos || text.find("orc") != std::string::npos ||
            text.find("elfe") != std::string::npos || text.find("démon") != std::string::npos)
        {
            return true;
        }
        return false;
    }

    void maybeEnemyUsesTacticalSwitch(EnemyCombatQueue& wave, Random& random)
    {
        if (!wave.hasWaitingEnemies() || wave.getActiveEnemyCount() <= 0)
        {
            return;
        }

        bool hasCoordinator = false;
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (monsterLooksLikeCoordinator(wave.getActiveEnemy(i)))
            {
                hasCoordinator = true;
                break;
            }
        }
        if (!hasCoordinator)
        {
            return;
        }

        int candidateIndex = -1;
        int lowestPercent = 101;
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& monster = wave.getActiveEnemy(i);
            if (monster.getMaxHp() <= 0 || monster.isDead())
            {
                continue;
            }
            const int percent = monster.getHp() * 100 / std::max(1, monster.getMaxHp());
            if (percent < lowestPercent)
            {
                lowestPercent = percent;
                candidateIndex = i;
            }
        }

        if (candidateIndex < 0 || lowestPercent > 45)
        {
            return;
        }

        const int chance = lowestPercent <= 25 ? 35 : 18;
        if (random.between(1, 100) > chance)
        {
            return;
        }

        const std::string outgoingName = wave.getActiveEnemy(candidateIndex).getName();
        const std::string incomingName = wave.getWaitingEnemy(0).getName();
        if (wave.switchActiveEnemyWithWaiting(candidateIndex, 0))
        {
            MessageScreen::show(
                "SWITCH ENNEMI",
                "combat.pve.enemy_tactical_switch",
                {
                    "Un ennemi assez malin pour diriger la ligne donne un ordre bref.",
                    outgoingName + " recule de la première ligne et " + incomingName + " prend sa place.",
                    "Même les ennemis pourront parfois utiliser le switch si leur groupe possède un chef ou une intelligence suffisante."
                },
                false
            );
        }
    }

    void maybeTriggerRareScavengerAfterCombat(Player& player, Random& random, const std::string& contextId)
    {
        const int roll = random.between(1, 1000);
        if (roll > 10)
        {
            return;
        }

        const std::vector<std::string> scavengers = {
            "deux novices trop confiants",
            "un ramasse-miette solitaire",
            "une petite bande opportuniste",
            "un faux secouriste beaucoup trop intéressé"
        };
        const std::string group = scavengers[static_cast<std::size_t>(random.between(0, static_cast<int>(scavengers.size()) - 1))];
        const bool vulnerable = player.getHp() * 4 < player.getMaxHp();
        int stolenCopper = 0;
        if (vulnerable)
        {
            stolenCopper = std::max(4, std::min(45, player.getLevel() * 5 + random.between(0, 12)));
            if (!player.getInventory().spendCopper(stolenCopper))
            {
                stolenCopper = 0;
            }
        }

        player.recordCanonicalEvent("tentatives_ramasse_miettes", contextId, group + " tente de profiter de la fin d'un combat");
        MessageScreen::show(
            "RAMASSE-MIETTES APRÈS COMBAT",
            "combat.pve.rare_scavenger",
            {
                "Événement très rare : " + group + " attend la fin du combat pour tenter de profiter de l'état du joueur.",
                vulnerable
                    ? "Tu es assez amoché pour qu'ils osent approcher."
                    : "Tu tiens encore assez debout : ils comprennent vite que ce n'est pas le bon cadavre à dépouiller.",
                stolenCopper > 0
                    ? "Perte légère : " + Money::formatCopper(stolenCopper) + " arrachés dans la confusion."
                    : "Perte : aucune. Ils reculent avant que ça devienne une vraie mauvaise idée.",
                "Note : l'événement reste volontairement rarissime pour rendre le monde vivant sans spammer le joueur."
            },
            false
        );
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
        maybeAppendReturningRival(player, wave, random);
    }

    WaveCombatSystem::displayFrontLineArrival(wave);
    recordWaveEncountersInBestiary(wave);
    recordWaveEncountersInJournal(player, wave);
    displayEncounterDialogue(player, wave, random, "pve.encounter");
    TemporaryAdventurerSupport temporaryAdventurerSupport = maybeTriggerRareAdventurerCombatAid(player, wave, random);

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

    std::vector<RecruitedAllyCombatSupport> recruitedAllies = collectActiveRecruitedAllies(player);
    std::vector<RecruitedAllyCombatSupport> reserveRecruitedAllies = collectReserveRecruitedAllies(player);
    RecruitedAllyOrderState recruitedAllyOrders;
    displayRecruitedAllyCombatStart(player, recruitedAllies);

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;
    int initialPlayerHp = player.getHp();
    int combatTurnCount = 0;
    player.beginChallengeCombatTracking();

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

            auto openOrders = [&]() -> bool {
                return openTeamOrdersMenu(player, wave, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders);
            };

            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty,
                !recruitedAllies.empty(),
                openOrders
            );

            if (playerTurnFinished)
            {
                ++combatTurnCount;
                player.reduceClassSkillCooldown();
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
                {"Action du joueur résolue.", "Le souffle du combat reprend, sans casser le rythme."}
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
            playBobMauriceAlliedTurn(player, wave, random);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playTemporaryAdventurerSupportTurn(player, wave, random, temporaryAdventurerSupport);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playRecruitedAllyCombatTurns(player, wave, random, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            maybeEnemyUsesTacticalSwitch(wave, random);
            showCombatPhaseGate(
                "TOUR DES ENNEMIS VIVANTS",
                "combat.pve.phase.enemies",
                {"Les ennemis actifs agissent maintenant, un groupe après l'autre."}
            );
            const int playerHpBeforeEnemyTurn = player.getHp();
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );
            resolveRecruitedAllyInjuriesAfterEnemyTurn(player, wave, recruitedAllies, recruitedAllyOrders, random, difficulty, playerHpBeforeEnemyTurn);

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
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
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

        reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "escape");

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite réussie : les récompenses sont calculées selon la difficulté, les ennemis vaincus, et les ennemis encore en vie déjà blessés."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        recordWaveKillsInBestiary(wave);
        recordWaveKillsInJournal(player, wave);

        return;
    }

    if (player.isDead())
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
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

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            if (BlessingSystem::tryTriggerLethalSurvival(player))
            {
                DeathPenaltySystem::displayLethalSurvivalAnomaly();
                return;
            }
            player.recordDeath();
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

        const bool rescuedByTeam = !recruitedAllies.empty() || !reserveRecruitedAllies.empty();
        bool teamFinishedFight = false;
        if (rescuedByTeam)
        {
            teamFinishedFight = simulateRecruitedAlliesAfterPlayerDown(
                player,
                wave,
                random,
                difficulty,
                recruitedAllies,
                reserveRecruitedAllies,
                recruitedAllyOrders,
                "pve_standard"
            );
        }

        player.recordDeath();
        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);
        resolvePlayerInfirmaryTransferAfterNonLethalDeath(
            player,
            difficulty,
            random,
            rescuedByTeam,
            teamFinishedFight,
            recruitedAllies,
            "pve_standard"
        );

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

    player.finishChallengeCombatTracking(
        true,
        false,
        defeatedWaveContainedElite(wave),
        wave.getDefeatedEnemyCount()
    );

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

    reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "victory");
    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    recordWaveKillsInBestiary(wave);
    recordWaveKillsInJournal(player, wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty);
    maybeTriggerRareScavengerAfterCombat(player, random, "pve_standard");

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
    const std::string& title,
    bool friendlyTrial
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
    recordWaveEncountersInJournal(player, wave);
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

    std::vector<RecruitedAllyCombatSupport> recruitedAllies = collectActiveRecruitedAllies(player);
    std::vector<RecruitedAllyCombatSupport> reserveRecruitedAllies = collectReserveRecruitedAllies(player);
    RecruitedAllyOrderState recruitedAllyOrders;
    displayRecruitedAllyCombatStart(player, recruitedAllies);

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;
    int initialPlayerHp = player.getHp();
    int combatTurnCount = 0;
    player.beginChallengeCombatTracking();

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

            auto openOrders = [&]() -> bool {
                return openTeamOrdersMenu(player, wave, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders);
            };

            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty,
                !recruitedAllies.empty(),
                openOrders
            );

            if (playerTurnFinished)
            {
                ++combatTurnCount;
                player.reduceClassSkillCooldown();
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
                {"Action du joueur résolue.", "Le souffle du combat reprend, sans casser le rythme."}
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
            playBobMauriceAlliedTurn(player, wave, random);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playRecruitedAllyCombatTurns(player, wave, random, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            maybeEnemyUsesTacticalSwitch(wave, random);
            showCombatPhaseGate(
                "TOUR DES ENNEMIS VIVANTS",
                "exploration.wave.phase.enemies",
                {"Les ennemis actifs de l'événement agissent maintenant."}
            );
            const int playerHpBeforeEnemyTurn = player.getHp();
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random
            );
            resolveRecruitedAllyInjuriesAfterEnemyTurn(player, wave, recruitedAllies, recruitedAllyOrders, random, difficulty, playerHpBeforeEnemyTurn);

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
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        if (friendlyTrial)
        {
            showPostCombatRouteScreen(
                "ÉPREUVE ABANDONNÉE",
                "exploration.wave.friendly_trial.escape",
                {
                    "Résultat : défi amical abandonné",
                    "Aucune mort, pénalité ou statistique réelle n'est enregistrée.",
                    "Les objets des caisses restent dans le sous-inventaire temporaire et disparaissent avec l'épreuve."
                }
            );
            return false;
        }
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

        reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "exploration_escape");

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite d'exploration : seules les actions réelles comptent."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        recordWaveKillsInBestiary(wave);
        recordWaveKillsInJournal(player, wave);
        return false;
    }

    if (player.isDead())
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        if (friendlyTrial)
        {
            showPostCombatRouteScreen(
                "DÉFAITE AMICALE",
                "exploration.wave.friendly_trial.defeat",
                {
                    "Résultat : personnage mis hors combat pendant l'épreuve",
                    "Cette chute n'est pas une mort et ne modifie ni les statistiques, ni l'équipement réel, ni les règles létales.",
                    "Le combat s'arrête avant toute pénalité."
                }
            );
            return false;
        }
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

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            if (BlessingSystem::tryTriggerLethalSurvival(player))
            {
                DeathPenaltySystem::displayLethalSurvivalAnomaly();
                return false;
            }
            player.recordDeath();
            DeathPenaltySystem::displayLethalDeathCorruption();
            return false;
        }

        const bool rescuedByTeam = !recruitedAllies.empty() || !reserveRecruitedAllies.empty();
        bool teamFinishedFight = false;
        if (rescuedByTeam)
        {
            teamFinishedFight = simulateRecruitedAlliesAfterPlayerDown(
                player,
                wave,
                random,
                difficulty,
                recruitedAllies,
                reserveRecruitedAllies,
                recruitedAllyOrders,
                "exploration_wave"
            );
        }

        player.recordDeath();
        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);
        resolvePlayerInfirmaryTransferAfterNonLethalDeath(
            player,
            difficulty,
            random,
            rescuedByTeam,
            teamFinishedFight,
            recruitedAllies,
            "exploration_wave"
        );
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

    if (friendlyTrial)
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        displaySpecialDefeatDialogues(wave);
        showPostCombatRouteScreen(
            "ÉPREUVE AMICALE RÉUSSIE",
            "exploration.wave.friendly_trial.victory",
            {
                "Résultat : mini-boss vaincu avec le contenu des caisses",
                "Aucun butin standard n'est généré dans le sous-inventaire temporaire.",
                "Bob et Maurice calculent maintenant leur récompense à leur manière."
            }
        );
        return true;
    }

    player.finishChallengeCombatTracking(
        true,
        false,
        defeatedWaveContainedElite(wave),
        wave.getDefeatedEnemyCount()
    );

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

    reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "exploration_victory");
    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    recordWaveKillsInBestiary(wave);
    recordWaveKillsInJournal(player, wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty);
    maybeTriggerRareScavengerAfterCombat(player, random, "exploration_wave");

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
        healer.recordChallengeCombatAction("ally_consumable");

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

        if (BlessingSystem::tryTriggerLethalSurvival(player))
        {
            lines.push_back(player.getName() + " reçoit trois pastilles rouges, mais toutes ses bénédictions se consument avant l'effacement.");
            lines.push_back("Le personnage revient à 1 PV, sans inventaire ni équipement, avec une marque irréversible.");
            MessageScreen::show("INTERVENTION DIVINE", "pve.coop.lethal_death_save.blessing", lines, false);
            DeathPenaltySystem::displayLethalSurvivalAnomaly();
            return;
        }

        player.recordDeath();
        lines.push_back(player.getName() + " reçoit trois pastilles rouges : mort définitive. Aucune bénédiction capable de briser le verdict n'a répondu.");
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
    recordWaveEncountersInJournal(leader, wave);
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
            player->beginChallengeCombatTracking();
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
            {"L'initiative mélange joueurs, invocations et ennemis selon la Dextérité, la vitesse et un d20."},
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

        InitiativeQueue initiative = InitiativeSystem::buildWaveQueue(party, wave, partySummons, random);
        MessageScreen::show(
            "ORDRE D'INITIATIVE",
            "pve.coop.initiative." + std::to_string(round),
            InitiativeSystem::buildDisplayLines(initiative),
            false
        );

        for (const InitiativeRoll& entry : initiative.getEntries())
        {
            if (!wave.hasEnemiesLeft() || escapeSucceeded || countAlivePlayers(party) <= 0)
            {
                break;
            }

            if (TurnOrder::isPlayer(entry.id))
            {
                const std::size_t i = static_cast<std::size_t>(std::max(0, entry.slotIndex));
                if (i >= party.size()) continue;
                Player* player = party[i];
                if (player == nullptr || player->isDead()) continue;

                MessageScreen::show(
                    "TOUR ALLIÉ",
                    "pve.coop.player_turn." + std::to_string(i + 1),
                    {"Tour de " + player->getName() + " [joueur " + std::to_string(i + 1) + "] — initiative " + std::to_string(entry.totalScore) + "."},
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
                    player->reduceClassSkillCooldown();
                    const int enemyHpAfterTurn = sumActiveEnemyHp(wave);
                    contributions[i].turnsTaken++;
                    contributions[i].damageDealt += std::max(0, enemyHpBeforeTurn - enemyHpAfterTurn);
                    contributions[i].healingDone += healingDoneThisTurn;
                    if (healingDoneThisTurn > 0) contributions[i].supportActions++;
                }
            }
            else if (TurnOrder::isSummonGroup(entry.id))
            {
                const std::size_t i = static_cast<std::size_t>(std::max(0, entry.slotIndex));
                if (i >= party.size() || i >= partySummons.size()) continue;
                Player* owner = party[i];
                if (owner == nullptr || owner->isDead() || !SummonCombatSystem::hasActiveSummons(partySummons[i])) continue;

                MessageScreen::show(
                    "TOUR DES INVOCATIONS",
                    "pve.coop.summon_turn." + std::to_string(i + 1),
                    {entry.label + " agissent à leur propre initiative : " + std::to_string(entry.totalScore) + "."},
                    false
                );
                const int before = sumActiveEnemyHp(wave);
                SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                    partySummons[i],
                    wave,
                    random,
                    summonControlModes[i]
                );
                const int summonDamage = std::max(0, before - sumActiveEnemyHp(wave));
                owner->recordChallengeSummonAction(summonDamage);
                if (summonDamage > 0)
                {
                    contributions[i].supportActions++;
                    contributions[i].damageDealt += summonDamage;
                }
            }
            else if (TurnOrder::isEnemy(entry.id))
            {
                const int enemyIndex = entry.slotIndex;
                if (!wave.isActiveIndexValid(enemyIndex)) continue;
                Monster& monster = wave.getActiveEnemy(enemyIndex);
                if (monster.isDead()) continue;

                Player* target = chooseAlivePlayerTarget(party, random, &contributions);
                if (target == nullptr) break;

                MessageScreen::show(
                    "TOUR ENNEMI",
                    "pve.coop.enemy_turn." + std::to_string(enemyIndex),
                    {"Tour de " + monster.getName() + " : cible " + target->getName() + " — initiative " + std::to_string(entry.totalScore) + "."},
                    false
                );
                if (!tryMonsterUseRareHealing(monster, wave, enemyIndex, random))
                {
                    const int targetHpBefore = target->getHp();
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
            }
        }

        displayPartyWaveCombatSnapshot(
            party,
            wave,
            partySummons,
            "Après le tour d'initiative",
            round
        );

        wave.removeDeadAndReplace();
        ++round;
    }

    const int aliveAtCombatEnd = countAlivePlayers(party);
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

    bool eliteDefeated = false;
    for (int defeatedIndex = 0; defeatedIndex < wave.getDefeatedEnemyCount(); ++defeatedIndex)
    {
        const Monster& defeated = wave.getDefeatedEnemy(defeatedIndex);
        if (defeated.isElite() || defeated.isEvolved())
        {
            eliteDefeated = true;
            break;
        }
    }

    const bool groupVictory = !escapeSucceeded && !wave.hasEnemiesLeft() && aliveAtCombatEnd > 0;
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
        member->finishChallengeCombatTracking(
            groupVictory && participated,
            false,
            eliteDefeated,
            wave.getDefeatedEnemyCount()
        );
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
        recordWaveKillsInJournal(*player, wave);
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
