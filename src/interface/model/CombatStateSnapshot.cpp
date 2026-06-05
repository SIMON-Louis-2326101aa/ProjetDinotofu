// EN: CombatStateSnapshot.cpp builds GUI-ready combat state snapshots.
// FR: CombatStateSnapshot.cpp construit des instantanés de combat prêts pour la future IG.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.

#include "interface/model/CombatStateSnapshot.hpp"

#include "entity/Monster.hpp"
#include "entity/Player.hpp"

#include "combat/group/CombatSide.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/group/CombatUnitSlot.hpp"

#include <algorithm>
#include <sstream>

GuiCombatUnitSnapshot CombatStateSnapshot::fromEntity(
    const Entity& entity,
    const std::string& side,
    const std::string& kind,
    bool targetable
)
{
    GuiCombatUnitSnapshot unit;
    unit.unitId = side + ":" + kind + ":" + entity.getName();
    unit.name = entity.getName();
    unit.type = entity.getType();
    unit.side = side;
    unit.kind = kind;
    if (const Monster* monster = dynamic_cast<const Monster*>(&entity))
    {
        unit.level = monster->getLevel();
    }
    else if (const Player* player = dynamic_cast<const Player*>(&entity))
    {
        unit.level = player->getLevel();
    }
    unit.hp = entity.getHp();
    unit.maxHp = entity.getMaxHp();
    unit.minDamage = entity.getMinDamage();
    unit.maxDamage = entity.getMaxDamage();
    unit.criticalDamage = entity.getCriticalDamage();
    unit.healingPotionCount = entity.getHealingPotionCount();
    unit.damagePotionCount = entity.getDamagePotionCount();
    unit.dead = entity.isDead();
    unit.targetable = targetable && !unit.dead;
    unit.healingThreatMarked = entity.hasHealingThreat();
    unit.provoking = entity.isProvoking();
    unit.defensePostureActive = entity.isInDefensePosture();
    unit.defensePostureLabel = entity.getDefensePostureLabel();
    unit.statusLabels = statusLabelsForEntity(entity);
    return unit;
}

GuiCombatUnitSnapshot CombatStateSnapshot::fromSummon(
    const Summon& summon,
    const std::string& side,
    bool targetable
)
{
    GuiCombatUnitSnapshot unit;
    unit.unitId = side + ":invocation:" + summon.getName();
    unit.name = summon.getName();
    unit.type = "Invocation de " + summon.getOwnerName();
    unit.side = side;
    unit.kind = "Invocation";
    unit.hp = summon.getHp();
    unit.maxHp = summon.getMaxHp();
    unit.minDamage = summon.getMinDamage();
    unit.maxDamage = summon.getMaxDamage();
    unit.criticalDamage = 0;
    unit.dead = summon.isDead() || summon.isExpired();
    unit.targetable = targetable && !unit.dead;

    if (summon.getDurationTurns() > 0)
    {
        unit.statusLabels.push_back("Durée " + std::to_string(summon.getDurationTurns()) + " tour(s)");
    }

    if (summon.canBeSacrificed())
    {
        unit.statusLabels.push_back("Sacrifiable");
    }

    return unit;
}

GuiCombatStateSnapshot CombatStateSnapshot::fromDuel(
    const Entity& playerSideEntity,
    const Entity& enemySideEntity,
    const std::string& title,
    const std::string& phase,
    int turnNumber
)
{
    GuiCombatStateSnapshot snapshot;
    snapshot.screenId = "combat.snapshot.duel";
    snapshot.title = title;
    snapshot.phase = phase;
    snapshot.turnNumber = turnNumber;
    snapshot.playerUnits.push_back(fromEntity(playerSideEntity, "Allié", "Combattant", true));
    snapshot.enemyUnits.push_back(fromEntity(enemySideEntity, "Adversaire", "Combattant", true));
    return snapshot;
}

GuiCombatStateSnapshot CombatStateSnapshot::fromGroups(
    const CombatGroup& playerGroup,
    const CombatGroup& enemyGroup,
    const std::string& title,
    const std::string& phase,
    int turnNumber
)
{
    GuiCombatStateSnapshot snapshot;
    snapshot.screenId = "combat.snapshot.group";
    snapshot.title = title;
    snapshot.phase = phase;
    snapshot.turnNumber = turnNumber;

    for (const CombatUnitSlot& slot : playerGroup.getSlots())
    {
        const Entity* entity = slot.getEntity();
        const Summon* summon = slot.getSummon();

        if (entity != nullptr)
        {
            addUnitToSnapshot(snapshot, fromEntity(*entity, sideToText(slot.getSide()), kindToText(slot.getKind()), slot.isTargetable()));
        }
        else if (summon != nullptr)
        {
            addUnitToSnapshot(snapshot, fromSummon(*summon, sideToText(slot.getSide()), slot.isTargetable()));
        }
    }

    for (const CombatUnitSlot& slot : enemyGroup.getSlots())
    {
        const Entity* entity = slot.getEntity();
        const Summon* summon = slot.getSummon();

        if (entity != nullptr)
        {
            addUnitToSnapshot(snapshot, fromEntity(*entity, sideToText(slot.getSide()), kindToText(slot.getKind()), slot.isTargetable()));
        }
        else if (summon != nullptr)
        {
            addUnitToSnapshot(snapshot, fromSummon(*summon, sideToText(slot.getSide()), slot.isTargetable()));
        }
    }

    return snapshot;
}

GuiCombatStateSnapshot CombatStateSnapshot::fromWave(
    const Entity& playerSideEntity,
    const EnemyCombatQueue& wave,
    const std::vector<Summon>& playerSummons,
    const std::string& title,
    const std::string& phase,
    int turnNumber
)
{
    std::vector<Entity*> playerSideEntities;
    playerSideEntities.push_back(const_cast<Entity*>(&playerSideEntity));

    return fromWaveParty(
        playerSideEntities,
        wave,
        playerSummons,
        title,
        phase,
        turnNumber
    );
}

GuiCombatStateSnapshot CombatStateSnapshot::fromWaveParty(
    const std::vector<Entity*>& playerSideEntities,
    const EnemyCombatQueue& wave,
    const std::vector<Summon>& playerSummons,
    const std::string& title,
    const std::string& phase,
    int turnNumber
)
{
    GuiCombatStateSnapshot snapshot;
    snapshot.screenId = "combat.snapshot.wave";
    snapshot.title = title;
    snapshot.phase = phase;
    snapshot.turnNumber = turnNumber;
    snapshot.escapeAvailable = true;

    for (const Entity* entity : playerSideEntities)
    {
        if (entity == nullptr)
        {
            continue;
        }

        const bool mainFighter = snapshot.playerUnits.empty();
        addUnitToSnapshot(
            snapshot,
            fromEntity(
                *entity,
                "Allié",
                mainFighter ? "Combattant" : "Allié",
                !entity->isDead()
            )
        );
    }

    for (const Summon& summon : playerSummons)
    {
        addUnitToSnapshot(snapshot, fromSummon(summon, "Allié", !summon.isDead() && !summon.isExpired()));
    }

    addWaveEnemiesToSnapshot(snapshot, wave);
    addWaveSummaryToSnapshot(snapshot, wave);

    return snapshot;
}

std::vector<std::string> CombatStateSnapshot::toDisplayLines(const GuiCombatStateSnapshot& snapshot)
{
    std::vector<std::string> lines;

    if (!snapshot.phase.empty())
    {
        lines.push_back("Phase : " + snapshot.phase);
    }

    if (snapshot.turnNumber > 0)
    {
        lines.push_back("Tour : " + std::to_string(snapshot.turnNumber));
    }

    if (!snapshot.currentActorName.empty())
    {
        lines.push_back("Acteur : " + snapshot.currentActorName);
    }

    if (!snapshot.currentTargetName.empty())
    {
        lines.push_back("Cible : " + snapshot.currentTargetName);
    }

    if (snapshot.escapeAvailable)
    {
        lines.push_back("Fuite : tentative possible si le mode de combat l'autorise.");
    }

    if (!snapshot.summaryLines.empty())
    {
        if (!lines.empty())
        {
            lines.push_back("");
        }
        lines.push_back("Résumé");
        for (const std::string& line : snapshot.summaryLines)
        {
            lines.push_back("- " + line);
        }
    }

    auto appendGroup = [&lines](const std::string& title, const std::vector<GuiCombatUnitSnapshot>& units)
    {
        if (units.empty())
        {
            return;
        }

        if (!lines.empty())
        {
            lines.push_back("");
        }

        lines.push_back(title);

        for (const GuiCombatUnitSnapshot& unit : units)
        {
            std::string line = "- " + unit.name + " [" + unit.kind + (unit.level > 0 ? " | Niv. " + std::to_string(unit.level) : "") + "] " + healthText(unit);

            if (!unit.type.empty())
            {
                line += " | " + unit.type;
            }

            if (!unit.targetable && !unit.dead)
            {
                line += " | non ciblable";
            }

            lines.push_back(line);

            std::vector<std::string> flags = unit.statusLabels;
            if (unit.healingThreatMarked) flags.push_back("menace de soin");
            if (unit.provoking) flags.push_back("provocation");
            if (unit.defensePostureActive)
            {
                std::string defense = "posture défensive";
                if (!unit.defensePostureLabel.empty()) defense += " : " + unit.defensePostureLabel;
                flags.push_back(defense);
            }

            if (!flags.empty())
            {
                std::string flagLine = "    États : ";
                for (std::size_t i = 0; i < flags.size(); ++i)
                {
                    if (i > 0) flagLine += ", ";
                    flagLine += flags[i];
                }
                lines.push_back(flagLine);
            }
        }
    };

    appendGroup("Camp joueur", snapshot.playerUnits);
    appendGroup("Camp ennemi", snapshot.enemyUnits);
    appendGroup("Unités neutres", snapshot.neutralUnits);

    if (!snapshot.logLines.empty())
    {
        lines.push_back("");
        lines.push_back("Journal récent");
        for (const std::string& line : snapshot.logLines)
        {
            lines.push_back("- " + line);
        }
    }

    return lines;
}

std::string CombatStateSnapshot::healthText(const GuiCombatUnitSnapshot& unit)
{
    if (unit.maxHp <= 0)
    {
        return "PV inconnus";
    }

    std::ostringstream stream;
    stream << "PV " << unit.hp << "/" << unit.maxHp;

    int percent = std::max(0, std::min(100, unit.hp * 100 / std::max(1, unit.maxHp)));
    stream << " (" << percent << "%)";

    if (unit.dead)
    {
        stream << " | hors combat";
    }

    return stream.str();
}

std::vector<std::string> CombatStateSnapshot::statusLabelsForEntity(const Entity& entity)
{
    std::vector<std::string> labels;

    if (entity.hasBurning()) labels.push_back("Brûlure");
    if (entity.hasPoison()) labels.push_back("Poison");
    if (entity.hasFrost()) labels.push_back("Froid");
    if (entity.hasShock()) labels.push_back("Choc");
    if (entity.hasBleeding()) labels.push_back("Saignement");
    if (entity.hasWeakening()) labels.push_back("Affaiblissement");
    if (entity.hasVulnerability()) labels.push_back("Faille ouverte");
    if (entity.hasElementalWard()) labels.push_back("Voile élémentaire");
    if (entity.hasRegeneration()) labels.push_back("Suture de mana");

    if (!entity.isClassSkillReady())
    {
        labels.push_back("Compétence en récupération " + std::to_string(entity.getClassSkillCooldownTurns()) + " tour(s)");
    }

    return labels;
}

std::string CombatStateSnapshot::sideToText(CombatSide side)
{
    switch (side)
    {
        case CombatSide::PlayerSide: return "Allié";
        case CombatSide::EnemySide: return "Adversaire";
        case CombatSide::Neutral:
        default: return "Neutre";
    }
}

std::string CombatStateSnapshot::kindToText(CombatUnitKind kind)
{
    switch (kind)
    {
        case CombatUnitKind::MainFighter: return "Combattant";
        case CombatUnitKind::Ally: return "Allié";
        case CombatUnitKind::Enemy: return "Ennemi";
        case CombatUnitKind::Summon: return "Invocation";
        case CombatUnitKind::Boss: return "Boss";
        case CombatUnitKind::Empty:
        default: return "Emplacement";
    }
}

void CombatStateSnapshot::addUnitToSnapshot(GuiCombatStateSnapshot& snapshot, const GuiCombatUnitSnapshot& unit)
{
    if (unit.side == "Allié")
    {
        snapshot.playerUnits.push_back(unit);
        return;
    }

    if (unit.side == "Adversaire")
    {
        snapshot.enemyUnits.push_back(unit);
        return;
    }

    snapshot.neutralUnits.push_back(unit);
}


void CombatStateSnapshot::addWaveEnemiesToSnapshot(GuiCombatStateSnapshot& snapshot, const EnemyCombatQueue& wave)
{
    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        const Monster& monster = wave.getActiveEnemy(i);
        GuiCombatUnitSnapshot unit = fromEntity(monster, "Adversaire", "Ennemi actif", !monster.isDead());
        unit.unitId = "wave.enemy.active." + std::to_string(i);
        unit.targetChoice = i + 1;
        unit.targetActionId = "combat.target.select." + std::to_string(i);

        if (monster.isElite())
        {
            unit.statusLabels.push_back("Élite");
        }

        if (monster.isEvolved())
        {
            unit.statusLabels.push_back("Variation évoluée");
        }

        addUnitToSnapshot(snapshot, unit);
    }

    for (int i = 0; i < wave.getWaitingEnemyCount(); ++i)
    {
        const Monster& monster = wave.getWaitingEnemy(i);
        GuiCombatUnitSnapshot unit = fromEntity(monster, "Adversaire", "En attente", false);
        unit.unitId = "wave.enemy.waiting." + std::to_string(i);
        unit.statusLabels.push_back("Hors première ligne");

        if (monster.isElite())
        {
            unit.statusLabels.push_back("Élite");
        }

        if (monster.isEvolved())
        {
            unit.statusLabels.push_back("Variation évoluée");
        }

        addUnitToSnapshot(snapshot, unit);
    }
}

void CombatStateSnapshot::addWaveSummaryToSnapshot(GuiCombatStateSnapshot& snapshot, const EnemyCombatQueue& wave)
{
    snapshot.summaryLines.push_back("Ennemis actifs : " + std::to_string(wave.getActiveEnemyCount()));
    snapshot.summaryLines.push_back("Ennemis en attente : " + std::to_string(wave.getWaitingEnemyCount()));
    snapshot.summaryLines.push_back("Ennemis blessés encore en vie : " + std::to_string(wave.getDamagedAliveEnemyCount()));
    snapshot.summaryLines.push_back("Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()));
    snapshot.summaryLines.push_back("Ennemis en fuite : " + std::to_string(wave.getEscapedEnemyCount()));
    snapshot.summaryLines.push_back("Total restant : " + std::to_string(wave.getTotalRemainingEnemyCount()));
}
