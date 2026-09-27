// EN: Shared PvE wave bestiary, journal and special-character narrative support.
// FR: Support PvE partagé pour bestiaire, journal et dialogues des personnages spéciaux.
#include "combat/modes/pve/PveWaveNarrativeSupport.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "combat/flavor/MonsterFlavorCatalog.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"
#include "entity/Player.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace PveWaveNarrativeSupport
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
    description += MonsterFlavorCatalog::buildBestiaryFlavor(monster);

    if (!monster.areStatsVisible())
    {
        description += " Certaines statistiques restent troubles pour le moment.";
    }

    return description;
}

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

void displaySpecialDefeatDialogues(const EnemyCombatQueue& wave)
{
    std::vector<std::string> names = collectDefeatedSpecialNames(wave);
    SpecialCharacterGroupDialogueCatalog::displayDefeatDialogue(names);

    for (const std::string& name : names)
    {
        SpecialCharacterDialogueCatalog::displayDefeatDialogue(name);
    }
}

void displaySpecialVictoryDialogues(const EnemyCombatQueue& wave)
{
    std::vector<std::string> names = collectSurvivingSpecialNames(wave);
    SpecialCharacterGroupDialogueCatalog::displayVictoryDialogue(names);

    for (const std::string& name : names)
    {
        SpecialCharacterDialogueCatalog::displayVictoryDialogue(name);
    }
}

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

}
