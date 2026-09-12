// EN: MonsterWaveCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterWaveCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "combat/TurnManager.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/system/EscapeSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/role/CombatRoleSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include "entity/Monster.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
    // EN: calculateHpPercentage declares or implements a focused behavior used by this module.
    // FR: calculateHpPercentage déclare ou implémente un comportement précis utilisé par ce module.
    int calculateHpPercentage(const Entity& entity)
    {
        if (entity.getMaxHp() <= 0)
        {
            return 0;
        }

        return entity.getHp() * 100 / entity.getMaxHp();
    }

    // EN: findMostInjuredAllyIndex declares or implements a focused behavior used by this module.
    // FR: findMostInjuredAllyIndex déclare ou implémente un comportement précis utilisé par ce module.
    void showWaveTurnNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MessageScreen::show(title, screenId, lines, false);
    }

    int findMostInjuredAllyIndex(EnemyCombatQueue& wave, int healerIndex)
    {
        int bestIndex = -1;
        int bestPercentage = 101;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (i == healerIndex)
            {
                continue;
            }

            Monster& ally = wave.getActiveEnemy(i);

            if (ally.isDead() || ally.getHp() >= ally.getMaxHp())
            {
                continue;
            }

            int percentage = calculateHpPercentage(ally);

            if (percentage < bestPercentage)
            {
                bestPercentage = percentage;
                bestIndex = i;
            }
        }

        return bestIndex;
    }

    bool tryHealerSupportAction(
        Monster& healer,
        EnemyCombatQueue& wave,
        int healerIndex,
        Random& random
    )
    {
        if (!CombatRoleSystem::isHealer(healer))
        {
            return false;
        }

        int allyIndex = findMostInjuredAllyIndex(wave, healerIndex);

        if (allyIndex < 0)
        {
            return false;
        }

        Monster& ally = wave.getActiveEnemy(allyIndex);
        int allyHpPercent = calculateHpPercentage(ally);
        int chance = allyHpPercent <= 35 ? 85 : 45;

        if (random.between(1, 100) > chance)
        {
            return false;
        }

        int healAmount = random.between(28, 44);
        ally.heal(healAmount);
        ThreatSystem::markAllyHealingAction(healer, ally);

        showWaveTurnNotice(
            "SOUTIEN DE VAGUE",
            "wave.monster.support.heal",
            {
                healer.getName() + " protège " + ally.getName() + " avec un soin rapide.",
                "Soin : +" + std::to_string(healAmount) + " PV.",
                ally.getName() + " possède maintenant " + std::to_string(ally.getHp()) + "/" + std::to_string(ally.getMaxHp()) + " PV."
            }
        );
        return true;
    }

    bool tryTankProtectionAction(
        Monster& tank,
        EnemyCombatQueue& wave,
        int tankIndex,
        Random& random
    )
    {
        int allyIndex = findMostInjuredAllyIndex(wave, tankIndex);

        if (allyIndex < 0)
        {
            return false;
        }

        return CombatRoleActionSystem::tryActivateAllyProtection(
            tank,
            wave.getActiveEnemy(allyIndex),
            random
        );
    }

    bool trySupportRecoveryAction(
        Monster& support,
        EnemyCombatQueue& wave,
        int supportIndex,
        Random& random
    )
    {
        int allyIndex = findMostInjuredAllyIndex(wave, supportIndex);

        if (allyIndex < 0)
        {
            return false;
        }

        return CombatRoleActionSystem::tryActivateSupportRecovery(
            support,
            wave.getActiveEnemy(allyIndex),
            random
        );
    }

    std::string normalizeMonsterWaveText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool monsterProfileContainsAny(const Monster& monster, const std::vector<std::string>& needles)
    {
        const std::string profile = normalizeMonsterWaveText(monster.getName() + " " + monster.getType() + " " + monster.getRaceText());
        for (const std::string& needle : needles)
        {
            if (profile.find(normalizeMonsterWaveText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    bool monsterCanUseTacticalPressure(const Monster& monster)
    {
        return monsterProfileContainsAny(monster, {
            "humain", "elfe", "gobelin", "orc", "bandit", "mercenaire", "soldat",
            "chevalier", "mage", "sorcier", "chaman", "assassin", "chef", "vétéran", "veteran"
        });
    }

    bool playerHasVisibleOpeningForMonster(const Player& player)
    {
        const bool lowHp = player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 45;
        return lowHp
            || player.hasPoison()
            || player.hasBleeding()
            || player.hasBurning()
            || player.hasFrost()
            || player.hasShock()
            || player.hasWeakening()
            || player.hasVulnerability();
    }

    bool monsterCanUseFormationPressure(const Monster& monster)
    {
        return monsterProfileContainsAny(monster, {
            "chef", "capitaine", "sergent", "soldat", "mercenaire", "gobelin", "orc",
            "bandit", "pillard", "meute", "gardien", "chevalier", "veteran", "vétéran"
        });
    }

    bool waveHasAnotherActiveEnemy(EnemyCombatQueue& wave, int selfIndex)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (index != selfIndex && !wave.getActiveEnemy(index).isDead())
            {
                return true;
            }
        }
        return false;
    }

    bool monsterIsDedicatedReinforcementCaller(const Monster& monster)
    {
        return monsterProfileContainsAny(monster, {
            "rameuteur", "rameuteuse", "rameut", "hurleur", "hurleuse", "corneur", "corne",
            "crieur", "crieuse", "guetteur", "guetteuse", "éclaireur", "eclaireur",
            "sentinelle", "alarme", "tambour", "chef", "capitaine", "sergent", "alpha",
            "reine", "matriarche", "nid", "scribe", "chaman", "chamane", "shaman"
        });
    }

    bool monsterIsCommonReinforcementRace(const Monster& monster)
    {
        return monsterProfileContainsAny(monster, {
            "gobelin", "loup", "chien", "meute", "kobold", "rat", "nuisible",
            "insecte", "insectoïde", "insectoide", "araignée", "araignee", "slime"
        });
    }

    bool monsterCanCallReinforcements(const Monster& monster)
    {
        if (monster.isInvocation())
        {
            return false;
        }

        if (monster.wasSpawnedByReinforcementCall() && !monster.canUseWeakenedReinforcementSignal())
        {
            return false;
        }

        if (monsterProfileContainsAny(monster, {"renfort", "sous-forme", "invocation"})
            && !monster.canUseWeakenedReinforcementSignal())
        {
            return false;
        }

        return monsterIsDedicatedReinforcementCaller(monster)
            || monster.canUseWeakenedReinforcementSignal()
            || monsterIsCommonReinforcementRace(monster);
    }

    int getLivingEnemyCountForReinforcement(EnemyCombatQueue& wave)
    {
        return wave.getActiveEnemyCount() + wave.getWaitingEnemyCount();
    }

    int getFallbackReinforcementSpawnGroupSize(EnemyCombatQueue& wave)
    {
        return std::max(1,
            wave.getDefeatedEnemyCount()
            + wave.getEscapedEnemyCount()
            + wave.getActiveEnemyCount()
            + wave.getWaitingEnemyCount()
        );
    }

    int getReinforcementLineBreakThreshold(const Monster& monster)
    {
        // This is not a random chance. It is the remaining group ratio measured
        // against the group size recorded when the caller entered the line/file.
        return monster.canUseWeakenedReinforcementSignal() ? 25 : 50;
    }

    bool waveHasEnoughLossesForReinforcement(Monster& caller, EnemyCombatQueue& wave)
    {
        if (caller.getReinforcementSpawnGroupSize() <= 0)
        {
            caller.setReinforcementSpawnGroupSize(getFallbackReinforcementSpawnGroupSize(wave));
        }

        const int baseline = std::max(1, caller.getReinforcementSpawnGroupSize());
        const int living = std::max(0, getLivingEnemyCountForReinforcement(wave));
        const int threshold = getReinforcementLineBreakThreshold(caller);
        return baseline >= 4 && living * 100 <= baseline * threshold;
    }

    std::string describeReinforcementLossTrigger(const Monster& caller)
    {
        if (caller.canUseWeakenedReinforcementSignal())
        {
            return "ligne tombée sous le quart du groupe qu'il avait en arrivant";
        }
        if (monsterIsDedicatedReinforcementCaller(caller))
        {
            return "ligne tombée sous la moitié du groupe qu'il avait en arrivant";
        }
        return "instinct de survie, pas rupture de ligne";
    }

    int getCurrentReinforcementPressure(EnemyCombatQueue& wave)
    {
        return wave.getActiveEnemyCount() + wave.getWaitingEnemyCount();
    }

    bool waveHasLivingReinforcementCaller(EnemyCombatQueue& wave, int selfIndex)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (index == selfIndex)
            {
                continue;
            }

            const Monster& other = wave.getActiveEnemy(index);
            if (!other.isDead() && (monsterIsDedicatedReinforcementCaller(other) || other.canUseWeakenedReinforcementSignal()))
            {
                return true;
            }
        }

        for (int index = 0; index < wave.getWaitingEnemyCount(); ++index)
        {
            const Monster& other = wave.getWaitingEnemy(index);
            if (!other.isDead() && (monsterIsDedicatedReinforcementCaller(other) || other.canUseWeakenedReinforcementSignal()))
            {
                return true;
            }
        }

        return false;
    }

    int getReinforcementLowHpThreshold(const Monster& monster)
    {
        if (monster.canUseWeakenedReinforcementSignal())
        {
            return 15;
        }

        return monsterIsDedicatedReinforcementCaller(monster) ? 30 : 20;
    }

    int getRequiredReinforcementLowHpTurns(const Monster& monster)
    {
        if (monster.canUseWeakenedReinforcementSignal())
        {
            return 4;
        }

        if (monsterIsDedicatedReinforcementCaller(monster))
        {
            return 2;
        }

        if (monsterProfileContainsAny(monster, {"rat", "nuisible", "slime", "insecte", "insectoïde", "insectoide", "araignée", "araignee"}))
        {
            return 4;
        }

        return 3;
    }

    Race chooseReinforcementRaceForCaller(const Monster& caller, Random& random)
    {
        if (!monsterIsDedicatedReinforcementCaller(caller) || random.between(1, 100) <= 72)
        {
            return caller.getRace();
        }

        if (monsterProfileContainsAny(caller, {"gobelin", "kobold"}))
        {
            return random.between(1, 100) <= 65 ? Race::Gobelin : Race::Hobgobelin;
        }

        if (monsterProfileContainsAny(caller, {"orc", "bandit", "pillard", "mercenaire"}))
        {
            return random.between(1, 100) <= 55 ? Race::Orc : Race::Humain;
        }

        if (monsterProfileContainsAny(caller, {"mort-vivant", "mort vivant"}))
        {
            return Race::MortVivant;
        }

        if (monsterProfileContainsAny(caller, {"plante", "champignon"}))
        {
            return Race::Plante;
        }

        if (monsterProfileContainsAny(caller, {"insecte", "insectoïde", "insectoide", "araignée", "araignee", "nid"}))
        {
            return Race::Insectoide;
        }

        return caller.getRace();
    }

    int getReinforcementLimitForMonster(const Monster& caller, Random& random)
    {
        if (caller.canUseWeakenedReinforcementSignal())
        {
            return 1;
        }

        const bool dedicatedCaller = monsterIsDedicatedReinforcementCaller(caller);

        if (monsterProfileContainsAny(caller, {"gobelin", "loup", "chien", "meute", "kobold", "rat", "nuisible"}))
        {
            return dedicatedCaller ? random.between(2, 3) : 1;
        }

        if (monsterProfileContainsAny(caller, {"slime", "insecte", "insectoïde", "insectoide", "araignée", "araignee"}))
        {
            return dedicatedCaller ? random.between(2, 3) : 1;
        }

        if (monsterProfileContainsAny(caller, {"mort-vivant", "mort vivant"}))
        {
            return dedicatedCaller ? random.between(2, 3) : 1;
        }

        if (monsterProfileContainsAny(caller, {"orc", "bandit", "pillard", "plante", "champignon"}))
        {
            return dedicatedCaller ? random.between(2, 3) : 1;
        }

        if (monsterIsCommonReinforcementRace(caller))
        {
            return 1;
        }

        return 1;
    }

    std::string getReinforcementCallLine(const Monster& caller)
    {
        if (monsterProfileContainsAny(caller, {"slime"}))
        {
            return caller.getName() + " fait vibrer son noyau et la gelée voisine répond.";
        }
        if (monsterProfileContainsAny(caller, {"rat", "nuisible"}))
        {
            if (monsterIsDedicatedReinforcementCaller(caller))
            {
                return caller.getName() + " couine avec un rythme précis, et plusieurs petits corps répondent dans les murs.";
            }
            return caller.getName() + " pousse un cri aigu qui court sous les planches.";
        }
        if (monsterProfileContainsAny(caller, {"insecte", "insectoïde", "insectoide", "araignée", "araignee", "nid"}))
        {
            return caller.getName() + " gratte le sol comme si tout le nid venait d'entendre.";
        }
        if (monsterProfileContainsAny(caller, {"meute", "loup", "chien", "alpha"}))
        {
            if (monsterIsDedicatedReinforcementCaller(caller))
            {
                return caller.getName() + " lance un hurlement long, celui qu'une meute reconnaît sans hésiter.";
            }
            return caller.getName() + " hurle assez fort pour refermer la chasse autour de toi.";
        }
        if (monsterProfileContainsAny(caller, {"mort-vivant", "mort vivant", "squelette", "zombie"}))
        {
            return caller.getName() + " laisse sortir un râle froid, et d'autres pas morts répondent.";
        }
        if (monsterProfileContainsAny(caller, {"plante", "champignon", "racine", "spore"}))
        {
            return caller.getName() + " secoue ses fibres, et le sol répond par des pousses hostiles.";
        }
        if (monsterProfileContainsAny(caller, {"gobelin", "kobold", "orc", "bandit", "pillard", "chef", "capitaine", "sergent"}))
        {
            if (monsterIsDedicatedReinforcementCaller(caller))
            {
                return caller.getName() + " donne le signal prévu, et les retardataires comprennent où frapper.";
            }
            return caller.getName() + " crie un ordre bref vers l'arrière de la ligne.";
        }
        return caller.getName() + " appelle ce qui rôde encore autour du combat.";
    }

    Monster createReinforcementFromCaller(const Monster& caller, int index, Random& random)
    {
        const bool swarm = monsterProfileContainsAny(caller, {"slime", "rat", "nuisible", "insecte", "insectoïde", "insectoide", "araignée", "araignee"});
        const bool pack = monsterProfileContainsAny(caller, {"meute", "loup", "chien", "gobelin", "kobold", "mort-vivant", "mort vivant"});
        const bool secondaryCaller = !caller.wasSpawnedByReinforcementCall()
            && monsterIsDedicatedReinforcementCaller(caller)
            && index == 1
            && random.between(1, 100) <= 7;
        const int levelLoss = swarm ? random.between(1, 3) : random.between(0, 2);
        const int level = std::max(1, caller.getLevel() - levelLoss);
        const int hpPercent = secondaryCaller ? random.between(44, 58) : (swarm ? random.between(24, 34) : (pack ? random.between(36, 48) : random.between(42, 56)));
        const int damagePercent = secondaryCaller ? random.between(44, 58) : (swarm ? random.between(32, 44) : (pack ? random.between(42, 56) : random.between(48, 64)));
        const int maxHp = std::max(swarm ? 6 : 10, caller.getMaxHp() * hpPercent / 100);
        const int minDamage = std::max(1, caller.getMinDamage() * damagePercent / 100);
        const int maxDamage = std::max(minDamage, caller.getMaxDamage() * damagePercent / 100);
        const int criticalDamage = std::max(maxDamage, caller.getCriticalDamage() * damagePercent / 100);

        std::string baseName;
        std::string type;
        if (secondaryCaller)
        {
            if (monsterProfileContainsAny(caller, {"meute", "loup", "chien", "alpha"}))
            {
                baseName = "Hurleur tardif";
                type = "Appelant de second rang, essoufflé et prudent";
            }
            else if (monsterProfileContainsAny(caller, {"gobelin", "kobold"}))
            {
                baseName = "Guetteur tardif";
                type = "Appelant de second rang, arrivé après le premier signal";
            }
            else if (monsterProfileContainsAny(caller, {"rat", "nuisible"}))
            {
                baseName = "Couineur tardif";
                type = "Appelant de second rang, petit et déjà nerveux";
            }
            else
            {
                baseName = "Rameuteur tardif";
                type = "Appelant de second rang, moins fiable que le premier";
            }
        }
        else if (monsterProfileContainsAny(caller, {"slime"}))
        {
            baseName = "Gelée appelée";
            type = "Renfort gélatineux attiré par un noyau en détresse";
        }
        else if (monsterProfileContainsAny(caller, {"rat", "nuisible"}))
        {
            baseName = monsterIsDedicatedReinforcementCaller(caller) ? "Nuisible rameuté" : "Nuisible appelé";
            type = monsterIsDedicatedReinforcementCaller(caller)
                ? "Petit renfort habitué à répondre aux couinements de groupe"
                : "Renfort minuscule sorti d'un recoin";
        }
        else if (monsterProfileContainsAny(caller, {"meute", "loup", "chien", "alpha"}))
        {
            baseName = monsterIsDedicatedReinforcementCaller(caller) ? "Chasseur rameuté" : "Membre de meute";
            type = monsterIsDedicatedReinforcementCaller(caller)
                ? "Renfort de meute qui suit un hurlement précis"
                : "Renfort de chasse attiré par le hurlement";
        }
        else if (monsterProfileContainsAny(caller, {"insecte", "insectoïde", "insectoide", "araignée", "araignee", "nid"}))
        {
            baseName = "Gardien du nid";
            type = "Renfort du nid arrivé par les fissures";
        }
        else if (monsterProfileContainsAny(caller, {"mort-vivant", "mort vivant", "squelette", "zombie"}))
        {
            baseName = "Mort rappelé";
            type = "Renfort froid tiré par le râle";
        }
        else if (monsterProfileContainsAny(caller, {"plante", "champignon", "racine", "spore"}))
        {
            baseName = "Pousse hostile";
            type = "Renfort végétal arraché au terrain";
        }
        else if (monsterProfileContainsAny(caller, {"gobelin", "kobold"}))
        {
            baseName = monsterIsDedicatedReinforcementCaller(caller) ? "Coureur rameuté" : "Coureur appelé";
            type = monsterIsDedicatedReinforcementCaller(caller)
                ? "Renfort commun qui attendait le signal d'un rameuteur"
                : "Renfort nerveux attiré par le cri";
        }
        else if (monsterProfileContainsAny(caller, {"orc"}))
        {
            baseName = "Brute appelée";
            type = "Renfort lourd rameuté par l'ordre";
        }
        else if (monsterProfileContainsAny(caller, {"bandit", "pillard", "mercenaire"}))
        {
            baseName = "Lame appelée";
            type = "Renfort opportuniste resté proche de l'embuscade";
        }
        else
        {
            baseName = "Renfort appelé";
            type = "Renfort attiré par le désordre de la ligne";
        }

        Monster reinforcement(
            baseName + " " + std::to_string(std::max(1, index)),
            type,
            chooseReinforcementRaceForCaller(caller, random),
            level,
            maxHp,
            minDamage,
            maxDamage,
            criticalDamage,
            0,
            0,
            false,
            false,
            false,
            false
        );
        reinforcement.markSpawnedByReinforcementCall(secondaryCaller);
        return reinforcement;
    }

    bool tryMonsterReinforcementCall(Monster& monster, EnemyCombatQueue& wave, int monsterIndex, Player& player, Random& random)
    {
        if (!monsterCanCallReinforcements(monster) || monster.hasCalledReinforcements())
        {
            return false;
        }

        if (getCurrentReinforcementPressure(wave) >= 6)
        {
            return false;
        }

        const bool dedicatedCaller = monsterIsDedicatedReinforcementCaller(monster)
            || monster.canUseWeakenedReinforcementSignal();
        const bool commonOnlyCaller = !dedicatedCaller && monsterIsCommonReinforcementRace(monster);

        if (commonOnlyCaller && waveHasLivingReinforcementCaller(wave, monsterIndex))
        {
            return false;
        }

        if (monster.canUseWeakenedReinforcementSignal() && waveHasLivingReinforcementCaller(wave, monsterIndex))
        {
            return false;
        }

        const int lowHpThreshold = getReinforcementLowHpThreshold(monster);
        const int requiredLowHpTurns = getRequiredReinforcementLowHpTurns(monster);
        const bool lowHp = monster.getMaxHp() > 0
            && monster.getHp() * 100 <= monster.getMaxHp() * lowHpThreshold;
        if (lowHp)
        {
            monster.increaseReinforcementLowHpTurns();
        }
        else
        {
            monster.resetReinforcementLowHpTurns();
        }

        const bool prolongedDanger = lowHp && monster.getReinforcementLowHpTurns() >= requiredLowHpTurns;
        const bool lineBroken = dedicatedCaller && waveHasEnoughLossesForReinforcement(monster, wave);

        if ((prolongedDanger || lineBroken) && monster.getReinforcementEntryCooldown() > 0)
        {
            monster.reduceReinforcementEntryCooldown();
            return false;
        }

        if (!prolongedDanger && !lineBroken)
        {
            return false;
        }

        int chance = 0;
        if (monster.canUseWeakenedReinforcementSignal())
        {
            chance = lineBroken ? 34 : 22;
        }
        else if (monsterIsDedicatedReinforcementCaller(monster))
        {
            chance = lineBroken ? 68 : 56;
            if (monster.isElite())
            {
                chance += 8;
            }
        }
        else
        {
            chance = 22;
            if (monsterProfileContainsAny(monster, {"gobelin", "loup", "chien", "meute", "kobold"}))
            {
                chance += 5;
            }
        }

        if (random.between(1, 100) > std::min(90, chance))
        {
            return false;
        }

        const int freePressure = std::max(0, 6 - getCurrentReinforcementPressure(wave));
        const int desiredCount = getReinforcementLimitForMonster(monster, random);
        const int reinforcementCount = std::max(1, std::min(desiredCount, freePressure));
        if (reinforcementCount <= 0)
        {
            return false;
        }

        monster.markReinforcementCall();
        std::vector<std::string> lines;
        lines.push_back(getReinforcementCallLine(monster));
        if (monster.canUseWeakenedReinforcementSignal())
        {
            lines.push_back(lineBroken
                ? "Le second signal existe, mais il ne porte qu'après une vraie débâcle de la ligne."
                : "La blessure dure, et le signal tardif hésite avant de trouver une seule réponse.");
            lines.push_back("Seuil senti : " + describeReinforcementLossTrigger(monster) + ".");
        }
        else if (monsterIsDedicatedReinforcementCaller(monster))
        {
            lines.push_back(lineBroken
                ? "Le signal était attendu : la ligne brisée attire les retardataires."
                : "La blessure dure, et le rameuteur choisit de ne plus rester seul.");
            lines.push_back("Seuil senti : " + describeReinforcementLossTrigger(monster) + ".");
        }
        else
        {
            lines.push_back("La blessure est trop profonde. L'instinct appelle seulement les siens, sans vrai ordre de groupe.");
        }

        for (int count = 1; count <= reinforcementCount; ++count)
        {
            Monster reinforcement = createReinforcementFromCaller(monster, count, random);
            lines.push_back(reinforcement.getName() + " rejoint le combat.");
            wave.addWaitingEnemy(reinforcement);
        }

        player.recordCanonicalEvent("pressions_ennemies_variees", "appel_renforts", "Un ennemi a appelé des renforts", reinforcementCount);

        showWaveTurnNotice(
            "RENFORTS ENNEMIS",
            "wave.monster.reinforcements",
            lines
        );
        return true;
    }

    bool monsterCanLeadCompatibleAlly(const Monster& leader, const Monster& ally)
    {
        if (leader.getRace() == ally.getRace())
        {
            return true;
        }

        if (monsterProfileContainsAny(leader, {"gobelin", "hobgobelin", "kobold"}))
        {
            return monsterProfileContainsAny(ally, {"gobelin", "hobgobelin", "kobold"});
        }

        if (monsterProfileContainsAny(leader, {"orc"}))
        {
            return monsterProfileContainsAny(ally, {"orc", "gobelin", "hobgobelin", "pillard"});
        }

        if (monsterProfileContainsAny(leader, {"loup", "meute", "chien", "alpha"}))
        {
            return monsterProfileContainsAny(ally, {"loup", "meute", "chien", "bête", "bete", "prédateur", "predateur"});
        }

        if (monsterProfileContainsAny(leader, {"rat", "nuisible"}))
        {
            return monsterProfileContainsAny(ally, {"rat", "nuisible"});
        }

        if (monsterProfileContainsAny(leader, {"insecte", "insectoïde", "insectoide", "araignée", "araignee", "nid"}))
        {
            return monsterProfileContainsAny(ally, {"insecte", "insectoïde", "insectoide", "araignée", "araignee", "nid"});
        }

        if (monsterProfileContainsAny(leader, {"plante", "champignon", "racine", "spore"}))
        {
            return monsterProfileContainsAny(ally, {"plante", "champignon", "racine", "spore"});
        }

        if (monsterProfileContainsAny(leader, {"mort-vivant", "mort vivant", "squelette", "zombie"}))
        {
            return monsterProfileContainsAny(ally, {"mort-vivant", "mort vivant", "squelette", "zombie"});
        }

        if (monsterProfileContainsAny(leader, {"bandit", "pillard", "mercenaire", "humain"}))
        {
            return monsterProfileContainsAny(ally, {"bandit", "pillard", "mercenaire", "humain", "voleur", "brigand"});
        }

        if (monsterProfileContainsAny(leader, {"slime", "gelée", "gelee"}))
        {
            return monsterProfileContainsAny(ally, {"slime", "gelée", "gelee"});
        }

        return false;
    }

    int countCompatibleFormationAllies(EnemyCombatQueue& wave, int monsterIndex, const Monster& leader)
    {
        int count = 0;
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (index == monsterIndex)
            {
                continue;
            }

            const Monster& ally = wave.getActiveEnemy(index);
            if (!ally.isDead() && monsterCanLeadCompatibleAlly(leader, ally))
            {
                ++count;
            }
        }

        return count;
    }

    std::vector<std::string> applyLeaderSpecificFormationSignature(
        Monster& leader,
        EnemyCombatQueue& wave,
        int leaderIndex,
        Player& player,
        Random& random,
        int compatibleAllies
    )
    {
        std::vector<std::string> lines;
        if (compatibleAllies <= 0)
        {
            return lines;
        }

        const int level = std::max(1, leader.getLevel());
        if (monsterProfileContainsAny(leader, {"gobelin", "hobgobelin", "kobold"}))
        {
            player.applyNextHitVulnerability(1, 5 + level / 36);
            lines.push_back("Manoeuvre sale : la petite troupe cherche surtout tes chevilles et ton angle mort.");
        }
        else if (monsterProfileContainsAny(leader, {"loup", "meute", "chien", "alpha"}))
        {
            if (random.between(1, 100) <= 45)
            {
                player.applyBleeding(1, std::max(1, level / 36 + 1));
                lines.push_back("Cercle de chasse : la meute resserre assez pour laisser une morsure basse.");
            }
            else
            {
                player.applyWeakening(1, 4 + level / 34);
                lines.push_back("Cercle de chasse : les pas autour de toi mangent une partie de ton élan.");
            }
        }
        else if (monsterProfileContainsAny(leader, {"insecte", "insectoïde", "insectoide", "araignée", "araignee", "nid"}))
        {
            player.applyWeakening(1, 5 + level / 34);
            lines.push_back("Ordre de nid : les pattes et les fils ferment les sorties faciles.");
        }
        else if (monsterProfileContainsAny(leader, {"mort-vivant", "mort vivant", "squelette", "zombie"}))
        {
            player.applyFrost(1);
            lines.push_back("Râle de tombe : les corps froids bougent ensemble et l'air se glace devant toi.");
        }
        else if (monsterProfileContainsAny(leader, {"slime", "gelée", "gelee"}))
        {
            player.applyWeakening(1, 4 + level / 36);
            lines.push_back("Onde molle : les gelées changent de forme ensemble et le sol devient moins sûr.");
        }
        else if (monsterProfileContainsAny(leader, {"orc"}))
        {
            leader.applyPrecisionBoost(1, 2);
            lines.push_back("Rugissement lourd : les plus proches frappent moins proprement, mais plus franchement.");
        }
        else if (monsterProfileContainsAny(leader, {"bandit", "pillard", "mercenaire", "humain"}))
        {
            player.applyVulnerability(1, 5 + level / 36);
            lines.push_back("Signe de main : les lames autour de toi attendent la même ouverture.");
        }
        else if (monsterProfileContainsAny(leader, {"plante", "champignon", "racine", "spore"}))
        {
            player.applyWeakening(1, 4 + level / 35);
            lines.push_back("Poussée de racines : le terrain répond assez pour ralentir tes appuis.");
        }

        int touched = 0;
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (index == leaderIndex)
            {
                continue;
            }
            Monster& ally = wave.getActiveEnemy(index);
            if (ally.isDead() || !monsterCanLeadCompatibleAlly(leader, ally))
            {
                continue;
            }
            ally.applyPrecisionBoost(1, 1);
            ++touched;
            if (touched >= 2)
            {
                break;
            }
        }

        if (touched > 0)
        {
            lines.push_back("Réponse de bande : " + std::to_string(touched) + " allié(s) compatible(s) suivent le rythme.");
            if (compatibleAllies >= 3)
            {
                leader.applyElementalWard(1, 5 + level / 40);
                lines.push_back("Ligne serrée : assez de corps répondent pour protéger brièvement le meneur.");
            }
        }
        return lines;
    }


    bool tryMonsterMoraleReaction(Monster& monster, EnemyCombatQueue& wave, int monsterIndex, Player& player, Random& random)
    {
        if (monster.isElite() || monster.getMaxHp() <= 0 || monster.getHp() * 100 > monster.getMaxHp() * 18)
        {
            return false;
        }

        const std::string text = normalizeMonsterWaveText(monster.getName() + " " + monster.getType() + " " + monster.getRaceText());
        const bool canFear = text.find("gobelin") != std::string::npos
            || text.find("voleur") != std::string::npos
            || text.find("bandit") != std::string::npos
            || text.find("humain") != std::string::npos
            || text.find("loup") != std::string::npos
            || text.find("bete") != std::string::npos
            || text.find("bête") != std::string::npos
            || text.find("mercenaire") != std::string::npos;
        const bool almostNever = text.find("mort-vivant") != std::string::npos
            || text.find("anomalie") != std::string::npos
            || text.find("golem") != std::string::npos
            || text.find("spectre") != std::string::npos
            || text.find("serment") != std::string::npos;

        if (!canFear || almostNever)
        {
            return false;
        }

        int chance = 10;
        if (player.getHp() > monster.getHp()) chance += 6;
        if (player.hasPassiveSkill("war_cry_caller") || player.hasPassiveSkill("battle_order_mastery")) chance += 5;
        if (player.hasPassiveSkill("church_oath_bonds")) chance += 3;
        if (player.hasPassiveSkill("church_oath_king")) chance += 2;
        if (player.hasPassiveSkill("church_oath_rivals")) chance += 3;
        if (wave.getActiveEnemyCount() <= 1) chance += 4;
        chance = std::clamp(chance, 0, 32);

        if (random.between(1, 100) > chance)
        {
            return false;
        }

        std::vector<std::string> lines;
        lines.push_back(monster.getName() + " ne lit plus le combat comme une simple bagarre : la peur entre dans son tour.");

        if (random.between(1, 100) <= 45)
        {
            lines.push_back("Réaction : fuite courte. L'ennemi préfère survivre plutôt que mourir proprement.");
            lines.push_back("Logique : tous les ennemis ne sont pas des suicidaires ; morts-vivants, anomalies ou serments brisés réagissent autrement.");
            player.recordCanonicalEvent("morale_ennemie", "fuite_ou_panique", "Un ennemi a paniqué ou fui", 1);
            player.recordCanonicalEvent("rivaux_potentiels", monster.getName(), monster.getName() + " a survécu à une fuite/panique et pourrait revenir changé", 1);
            if (player.hasPassiveSkill("church_oath_witness") || player.hasPassiveSkill("church_oath_memory"))
            {
                player.recordCanonicalEvent("rumeurs_temoin", monster.getName(), monster.getName() + " a été vu en train de fuir ou paniquer", 1);
                lines.push_back("Témoin logique : la fuite peut devenir rumeur parce qu'elle a été vue, pas parce que le monde est omniscient.");
            }
            if (player.hasPassiveSkill("church_oath_bonds"))
            {
                player.recordCanonicalEvent("techniques_combinees_alliees", "pression_morale", "Le groupe a pesé sur une fuite ennemie", 1);
                lines.push_back("Serment des Liens : la pression vient aussi du groupe, ce qui prépare les futures techniques combinées.");
            }
            if (player.hasPassiveSkill("church_oath_rivals"))
            {
                player.recordCanonicalEvent("rivaux_potentiels", "serment_rivaux:" + monster.getName(), monster.getName() + " a fui sous un serment qui garde les noms", 1);
                lines.push_back("Serment des Rivaux : cette fuite garde un nom. Si l'ennemi revient, il ne reviendra pas comme un mob anonyme.");
            }
            if (player.hasPassiveSkill("church_oath_unstable_fate") && random.between(1, 100) <= 20)
            {
                player.recordCanonicalEvent("destin_instable", "fuite_marquee", "Une fuite ennemie a déplacé un fil du destin", 1);
                lines.push_back("Destin instable : la fuite ne devient importante que parce qu'elle a été vécue et pourrait recroiser ta route.");
            }
            lines.push_back("Mémoire du monde : si quelqu'un voit cette fuite ou si l'ennemi survit vraiment, il pourra devenir une rumeur ou un rival plus tard.");

            if (monster.isPersistentRival())
            {
                player.recordRivalEscape(monster.getRivalId(), player.getCurrentCityId());
                lines.push_back("Rival connu : cette fuite appartient au même individu. Sa trace reste attachée à son identité.");
            }
            else
            {
                int rivalChance = 12;
                if (player.hasPassiveSkill("church_oath_rivals")) rivalChance += 48;
                if (player.hasPassiveSkill("church_oath_witness")) rivalChance += 18;
                if (player.hasPassiveSkill("church_oath_memory")) rivalChance += 12;
                if (monster.isElite() || monster.isEvolved()) rivalChance += 10;
                if (random.between(1, 100) <= std::min(90, rivalChance))
                {
                    const std::string rivalId = player.createRivalFromEnemy(
                        monster.getName(),
                        monster.getType(),
                        monster.getLevel(),
                        monster.getMaxHp(),
                        std::max(1, monster.getMaxDamage()),
                        "Fuite vécue après une rupture de morale"
                    );
                    monster.setRivalId(rivalId);
                    player.recordRivalEscape(rivalId, player.getCurrentCityId());
                    lines.push_back("Trace persistante : cet ennemi n'est plus seulement un type de monstre. Son identité peut survivre à cette rencontre.");
                }
            }

            showWaveTurnNotice("MORALE ENNEMIE", "wave.monster.morale.flee", lines);
            wave.removeActiveEnemyAsEscaped(monsterIndex);
            return true;
        }

        monster.applyWeakening(1, 10 + monster.getLevel() / 20);
        monster.applyPrecisionBoost(1, 1);
        lines.push_back("Réaction : panique contrôlée. Il ne fuit pas, mais son prochain geste devient nerveux et moins propre.");
        lines.push_back("Contre-jeu : une pression de groupe, un cri, une posture ou une mort de chef pourra rendre ces réactions plus fréquentes plus tard.");
        player.recordCanonicalEvent("morale_ennemie", "fuite_ou_panique", "Un ennemi a paniqué ou fui", 1);
        showWaveTurnNotice("MORALE ENNEMIE", "wave.monster.morale.panic", lines);
        return true;
    }


    bool tryMonsterSignatureSkill(Monster& monster, EnemyCombatQueue& wave, int monsterIndex, Player& player, Random& random)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const bool trained = monsterCanUseTacticalPressure(monster);
        const bool playerOpened = playerHasVisibleOpeningForMonster(player);
        int chance = 4 + std::min(8, monster.getLevel() / 28);
        if (trained) chance += 5;
        if (monster.isElite()) chance += 6;
        if (playerOpened) chance += 4;
        if (player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 40) chance += 3;
        if (wave.getActiveEnemyCount() >= 3 && monsterProfileContainsAny(monster, {"chef", "capitaine", "reine", "matriarche", "alpha"})) chance += 4;
        if (player.hasPassiveSkill("threat_reader")) chance -= 2;
        if (player.hasPassiveSkill("body_reader")) chance -= 1;
        if (player.hasPassiveSkill("terrain_reader")) chance -= 1;
        if (player.hasPassiveSkill("church_oath_silence")) chance -= 1;
        if (player.hasPassiveSkill("church_oath_open_sky") && monsterProfileContainsAny(monster, {"vol", "aile", "ailé", "ailee", "harpie", "oiseau", "fée", "fee"})) chance -= 1;
        if (player.hasPassiveSkill("church_oath_roots") && monsterProfileContainsAny(monster, {"araignée", "araignee", "liane", "racine", "ronce", "plante"})) chance -= 1;
        if (player.hasPassiveSkill("church_oath_broken_mirror")) chance -= 2;
        if (player.hasPassiveSkill("church_oath_witness")) chance -= 1;
        if (player.hasPassiveSkill("church_oath_scars") && player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 45) chance -= 1;
        chance = std::clamp(chance, 0, 30);

        if (random.between(1, 100) > chance)
        {
            return false;
        }

        const std::string profileText = normalizeMonsterWaveText(monster.getName() + " " + monster.getType() + " " + monster.getRaceText() + " " + profile.archetype + " " + profile.signatureMove);
        auto hasToken = [&](const std::string& token) {
            return profileText.find(normalizeMonsterWaveText(token)) != std::string::npos;
        };

        const int signatureTier = std::clamp(
            (monster.getLevel() >= 25 ? 1 : 0)
            + (monster.getLevel() >= 55 ? 1 : 0)
            + (monster.getLevel() >= 90 ? 1 : 0)
            + (monster.isElite() ? 1 : 0),
            0,
            4
        );

        std::vector<std::string> lines;
        lines.push_back(monster.getName() + " utilise une vraie compétence ennemie au lieu d'une attaque simple.");
        lines.push_back("Signature lue : " + profile.signatureMove + " | " + profile.counterplayLine);
        if (player.hasPassiveSkill("threat_reader") || player.hasPassiveSkill("body_reader") || player.hasPassiveSkill("terrain_reader"))
        {
            lines.push_back("Contre-lecture : tes passifs d'observation ne bloquent pas la compétence, mais ils réduisent les mauvaises surprises et rendent le contre-jeu plus lisible.");
        }
        if (player.hasPassiveSkill("church_oath_silence") || player.hasPassiveSkill("church_oath_broken_mirror") || player.hasPassiveSkill("church_oath_memory") || player.hasPassiveSkill("church_oath_witness") || player.hasPassiveSkill("church_oath_scars") || player.hasPassiveSkill("church_oath_legacy") || player.hasPassiveSkill("church_oath_bound_forge") || player.hasPassiveSkill("church_oath_bonds"))
        {
            lines.push_back("Serment lisible : le registre intérieur aide surtout contre illusions, panique, blessures vécues, objets qui ont servi ou annonces, sans annuler la compétence ennemie.");
        }
        if (signatureTier <= 0)
        {
            lines.push_back("Maîtrise ennemie : geste instinctif, dangereux mais encore lisible.");
        }
        else if (signatureTier >= 4)
        {
            lines.push_back("Maîtrise ennemie : technique de vétéran, l'effet force une vraie réponse et ne ressemble plus à une attaque décorative.");
        }
        else if (signatureTier >= 3)
        {
            lines.push_back("Maîtrise ennemie : technique entraînée, l'effet se ressent vraiment et demande un vrai contre-jeu.");
        }
        else
        {
            lines.push_back("Maîtrise ennemie : geste préparé, plus fiable qu'une attaque basique.");
        }
        if (monster.getMaxHp() > 0 && monster.getHp() * 100 <= monster.getMaxHp() * 35)
        {
            lines.push_back("État ennemi : blessé, il ne devient pas juste plus faible ; certains profils deviennent plus dangereux ou plus désespérés.");
        }

        const bool heavyOrLongSignature = hasToken("colosse") || hasToken("golem") || hasToken("dragon") || hasToken("brute")
            || hasToken("orc") || hasToken("géant") || hasToken("geant") || hasToken("rituel") || hasToken("serment")
            || hasToken("sphinx") || hasToken("marteau") || hasToken("massue") || hasToken("charge") || hasToken("canon");
        if (heavyOrLongSignature
            && !monster.hasPowerBoost()
            && signatureTier >= 1
            && random.between(1, 100) <= 16 + signatureTier * 5)
        {
            monster.applyPowerBoost(1, 12 + signatureTier * 4);
            monster.applyPrecisionBoost(1, 1 + signatureTier / 3);
            lines.push_back("Compétence annoncée : " + monster.getName() + " prend un temps de trop pour charger quelque chose de lourd.");
            lines.push_back("Ce n'est pas un texte décoratif : le prochain impact gagne puissance/précision si personne ne casse le rythme.");
            lines.push_back("Contre-jeu : défense, retrait, brise-garde, choc/givre, entrave, ordre allié ou pression immédiate.");
            player.recordCanonicalEvent("competences_ennemies", "attaque_annoncee", "Une grosse compétence ennemie a été annoncée", 1);
            showWaveTurnNotice("COMPÉTENCE ANNONCÉE", "wave.monster.signature_announced", lines);
            return true;
        }

        if ((hasToken("vol") || hasToken("aile") || hasToken("ailé") || hasToken("ailee") || hasToken("harpie") || hasToken("chauve") || hasToken("oiseau") || hasToken("fée") || hasToken("fee"))
            && !monster.hasFlight())
        {
            const int duration = monster.isElite() || monster.getLevel() >= 35 ? 2 : 1;
            monster.applyFlight(duration);
            lines.push_back("Compétence : Vol. " + monster.getName() + " prend assez de hauteur pour refuser les armes courtes pendant " + std::to_string(duration) + " tour(s).");
            lines.push_back("Contre-jeu : allonge, tir, magie, entrave, choc/givre ou attendre la redescente.");
        }
        else if ((hasToken("araignée") || hasToken("araignee") || hasToken("toile") || hasToken("liane") || hasToken("racine") || hasToken("ronce") || hasToken("plante entravante"))
            && random.between(1, 100) <= 65)
        {
            const int damage = std::max(2, monster.getLevel() / 6 + random.between(2, 6) + signatureTier);
            player.takeDamage(damage);
            player.applyEntanglement(1);
            player.applyWeakening(1, 5 + signatureTier * 2);
            lines.push_back("Compétence : Entrave. Les fils, racines ou lianes bloquent le prochain tour de " + player.getName() + ".");
            lines.push_back("Dégâts d'étranglement : " + std::to_string(damage) + ".");
        }
        else if ((hasToken("illusion") || hasToken("mirage") || hasToken("kitsune") || hasToken("renarde") || hasToken("lanterne"))
            && !monster.hasIllusion())
        {
            monster.applyIllusion(2);
            lines.push_back("Compétence : Miroirs troubles. " + monster.getName() + " se divise en trois silhouettes pendant 2 tours.");
            lines.push_back("Règle : frapper le mauvais reflet inflige un contrecoup ; observation et patience réduisent le piège.");
        }
        else if (hasToken("soigneur") || hasToken("chaman") || hasToken("clerc") || hasToken("chef") || hasToken("capitaine") || hasToken("reine") || hasToken("matriarche") || hasToken("alpha"))
        {
            int alliesTouched = 0;
            for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
            {
                if (index == monsterIndex || !wave.isActiveIndexValid(index)) continue;
                Monster& ally = wave.getActiveEnemy(index);
                if (ally.isDead()) continue;
                ally.applyPrecisionBoost(2, 1 + monster.getLevel() / 45 + signatureTier / 2);
                if (alliesTouched < 2)
                {
                    ally.applyElementalWard(1, 5 + monster.getLevel() / 25 + signatureTier * 2);
                }
                ++alliesTouched;
            }
            if (alliesTouched <= 0)
            {
                monster.applyPrecisionBoost(2, 2 + monster.getLevel() / 50 + signatureTier);
                lines.push_back("Technique : il garde son propre rythme en vie, faute d'allié à conduire.");
            }
            else
            {
                lines.push_back("Technique : ordre court, " + std::to_string(alliesTouched) + " allié(s) reprennent un meilleur tempo.");
                if (player.isInDefensePosture() || player.hasVulnerability())
                {
                    player.applyWeakening(1 + (signatureTier >= 3 ? 1 : 0), 8 + monster.getLevel() / 22 + signatureTier * 2);
                    lines.push_back("Lecture adverse : ton angle est annoncé aux autres, ta prochaine réponse sera moins propre.");
                }
            }
        }
        else if (hasToken("archer") || hasToken("tireur") || hasToken("rôdeur") || hasToken("rodeur") || hasToken("harceleur"))
        {
            const int damage = std::max(4, monster.getLevel() / 3 + random.between(5, 11) + signatureTier * 2);
            player.takeDamage(damage);
            player.applyWeakening(1 + (signatureTier >= 3 ? 1 : 0), 8 + monster.getLevel() / 18 + signatureTier * 2);
            if (random.between(1, 100) <= 35 + std::min(25, monster.getLevel() / 4) + signatureTier * 4)
            {
                player.applyBleeding(2, std::max(1, monster.getLevel() / 30 + 1));
                lines.push_back("Trait précis : le tir gêne ton appui et laisse une vraie entaille.");
            }
            lines.push_back("Dégâts de tir technique : " + std::to_string(damage) + ".");
        }
        else if (hasToken("mage") || hasToken("sorcier") || hasToken("rune") || hasToken("spectre") || hasToken("spectral") || hasToken("fée") || hasToken("fee"))
        {
            const int damage = std::max(5, monster.getLevel() / 4 + random.between(7, 13) + signatureTier * 2);
            player.takeDamage(damage);
            player.applyShock(1);
            player.applyVulnerability(1 + (signatureTier >= 4 ? 1 : 0), 8 + monster.getLevel() / 18 + signatureTier * 2);
            if (monster.getLevel() >= 45 || monster.isElite())
            {
                player.applyFrost(1);
                lines.push_back("Résonance haute : le choc accroche aussi les mouvements, comme si l'air refusait une esquive propre.");
            }
            lines.push_back("Dégâts arcaniques : " + std::to_string(damage) + ".");
        }
        else if (hasToken("assassin") || hasToken("voleur") || hasToken("roublard") || hasToken("bandit") || hasToken("pilleur"))
        {
            int damage = std::max(4, monster.getLevel() / 3 + random.between(6, 12));
            if (player.hasVulnerability() || player.hasWeakening())
            {
                damage += 4 + monster.getLevel() / 18 + signatureTier * 2;
                lines.push_back("Angle trouvé : l'ennemi exploite une faille déjà ouverte, pas seulement sa vitesse.");
            }
            player.takeDamage(damage);
            player.applyBleeding(2, std::max(1, monster.getLevel() / 28 + 1));
            if (random.between(1, 100) <= 45)
            {
                player.applyVulnerability(1 + (signatureTier >= 4 ? 1 : 0), 7 + monster.getLevel() / 24 + signatureTier * 2);
            }
            lines.push_back("Dégâts de technique sournoise : " + std::to_string(damage) + ".");
        }
        else if (hasToken("slime") || hasToken("fongique") || hasToken("poison") || hasToken("venin") || hasToken("insect"))
        {
            const int damage = std::max(3, monster.getLevel() / 5 + random.between(4, 9) + signatureTier);
            player.takeDamage(damage);
            player.applyPoison(3 + (signatureTier >= 2 ? 1 : 0), std::max(1, monster.getLevel() / 35 + 1 + signatureTier / 2));
            player.applyWeakening(1, 6 + monster.getLevel() / 25 + signatureTier * 2);
            lines.push_back("Projection toxique : ce n'est pas le choc qui fait peur, c'est ce qui reste dans les veines.");
            lines.push_back("Dégâts corrosifs : " + std::to_string(damage) + ".");
        }
        else if (hasToken("brute") || hasToken("colosse") || hasToken("orc") || hasToken("dragon") || hasToken("golem") || hasToken("gardien"))
        {
            int damage = std::max(7, monster.getLevel() / 2 + random.between(8, 15) + signatureTier * 3);
            if (player.isInDefensePosture())
            {
                damage += 3 + monster.getLevel() / 20 + signatureTier * 2;
                lines.push_back("Choc de masse : la garde encaisse, mais le poids cherche à la fendre plutôt qu'à la contourner.");
            }
            player.takeDamage(damage);
            player.applyVulnerability(1 + (signatureTier >= 3 ? 1 : 0), 10 + monster.getLevel() / 18 + signatureTier * 2);
            if (random.between(1, 100) <= 45)
            {
                player.applyWeakening(1, 8 + monster.getLevel() / 24 + signatureTier * 2);
            }
            lines.push_back("Dégâts de charge lourde : " + std::to_string(damage) + ".");
        }
        else
        {
            return false;
        }

        if (signatureTier >= 4)
        {
            player.applyWeakening(1, 5 + monster.getLevel() / 35);
            lines.push_back("Pression de vétéran : même si tu survis au geste, il laisse une hésitation courte dans ta réponse.");
        }
        if (player.hasPassiveSkill("church_oath_witness") && random.between(1, 100) <= 18)
        {
            player.recordCanonicalEvent("serments_eglise", "temoin_competence_ennemie", "Le Serment du Témoin a cadré une compétence ennemie observée", 1);
            lines.push_back("Serment du Témoin : la compétence devient une trace exploitable plus tard, parce qu'elle a été vue en vrai.");
        }
        if (player.hasPassiveSkill("church_oath_legacy") && signatureTier >= 3 && random.between(1, 100) <= 12)
        {
            player.recordCanonicalEvent("objets_avec_memoire", "pression_heritee", "Une pression ennemie forte a laissé une trace d'héritage possible", 1);
            lines.push_back("Serment de l'Héritage : ce genre de choc pourra plus tard nourrir objets, tombes ou souvenirs en Mortel/Léthal.");
        }
        if (player.hasPassiveSkill("church_oath_bound_forge") && signatureTier >= 2 && random.between(1, 100) <= 14)
        {
            player.recordCanonicalEvent("objets_avec_memoire", "arme_marquee_par_signature", "Une arme ou armure a subi une compétence ennemie marquante", 1);
            lines.push_back("Serment de la Forge liée : l'équipement pourra se souvenir de ce choc seulement parce qu'il l'a vraiment vécu.");
        }
        if (player.hasPassiveSkill("church_oath_bonds") && wave.getActiveEnemyCount() >= 2 && random.between(1, 100) <= 12)
        {
            player.recordCanonicalEvent("techniques_combinees_alliees", "signature_encadree", "Le groupe a appris d'une compétence signature ennemie", 1);
            lines.push_back("Serment des Liens : le groupe apprend le rythme adverse pour de futurs combos, mais personne n'agit gratuitement ce tour-ci.");
        }
        if (player.hasPassiveSkill("church_oath_rivals") && signatureTier >= 2 && random.between(1, 100) <= 14)
        {
            player.recordCanonicalEvent("rivaux_potentiels", "signature_rivale:" + monster.getName(), monster.getName() + " a utilisé une compétence assez marquante pour nourrir une rivalité", 1);
            lines.push_back("Serment des Rivaux : une compétence aussi marquée peut devenir une rancune reconnaissable, pas juste une ligne de dégâts.");
        }
        if (player.hasPassiveSkill("church_oath_unstable_fate") && signatureTier >= 3 && random.between(1, 100) <= 10)
        {
            player.recordCanonicalEvent("destin_instable", "signature_deplacee", "Une compétence signature a déplacé un fil du destin", 1);
            lines.push_back("Destin instable : ce geste ne change pas l'histoire à lui seul, mais il ajoute une trace possible au prochain croisement.");
        }
        lines.push_back("Contre-jeu : " + profile.counterplayLine);
        player.recordCanonicalEvent("competences_ennemies", "signature_ennemie", "Un ennemi a utilisé une compétence signature", 1);
        showWaveTurnNotice("COMPÉTENCE ENNEMIE", "wave.monster.signature_skill", lines);
        ThreatSystem::consumeForcedTargetIfNeeded(player);
        return true;
    }

    bool tryMonsterFormationAction(Monster& monster, EnemyCombatQueue& wave, int monsterIndex, Player& player, Random& random)
    {
        if (!waveHasAnotherActiveEnemy(wave, monsterIndex))
        {
            return false;
        }

        const bool formationProfile = monsterCanUseFormationPressure(monster);
        const bool defensiveProfile = monsterProfileContainsAny(monster, {"tank", "colosse", "gardien", "chevalier", "soldat", "bouclier", "armure"});
        const bool leaderProfile = monsterProfileContainsAny(monster, {"chef", "capitaine", "sergent", "veteran", "vétéran", "oracle", "alpha", "reine", "matriarche"});
        const int compatibleAllies = leaderProfile ? countCompatibleFormationAllies(wave, monsterIndex, monster) : 0;

        if (leaderProfile && compatibleAllies <= 0 && !defensiveProfile)
        {
            return false;
        }

        if (!formationProfile && !defensiveProfile && !leaderProfile)
        {
            return false;
        }

        int chance = 6;
        if (formationProfile) chance += 7;
        if (defensiveProfile) chance += 5;
        if (leaderProfile) chance += 7;
        if (monster.isElite()) chance += 5;
        if (monster.getMaxHp() > 0 && monster.getHp() * 100 <= monster.getMaxHp() * 50) chance += 4;
        if (monster.getReinforcementEntryCooldown() > 0) chance -= leaderProfile ? 7 : 3;

        if (chance <= 0 || random.between(1, 100) > chance)
        {
            return false;
        }

        std::vector<std::string> lines;
        lines.push_back(monster.getName() + " essaie de remettre sa ligne en ordre au lieu de frapper directement.");
        if (monster.getReinforcementEntryCooldown() > 0)
        {
            lines.push_back("Le souffle coupé rend l'ordre moins net, mais pas encore impossible.");
        }
        const MonsterBehaviorProfile formationProfileInfo = MonsterBehaviorProfileCatalog::build(monster);
        lines.push_back("Style de formation : " + formationProfileInfo.archetype + " - " + formationProfileInfo.behaviorLine);

        if (defensiveProfile && random.between(1, 100) <= 55)
        {
            monster.startDefensePosture(16, 5, "Couverture de formation");
            monster.startProvocation(2);
            lines.push_back("Couverture : " + monster.getName() + " prend une posture défensive courte et attire l'attention.");
        }
        else
        {
            monster.applyPrecisionBoost(2, 1);
            lines.push_back("Ordre bref : " + monster.getName() + " prépare mieux son prochain geste.");
        }

        int alliesEncouraged = 0;
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (index == monsterIndex)
            {
                continue;
            }

            Monster& ally = wave.getActiveEnemy(index);
            if (ally.isDead())
            {
                continue;
            }

            if (leaderProfile && !monsterCanLeadCompatibleAlly(monster, ally))
            {
                continue;
            }

            if (alliesEncouraged >= 2)
            {
                break;
            }

            ally.applyPrecisionBoost(2, leaderProfile ? 2 : 1);
            if (defensiveProfile || leaderProfile)
            {
                ally.applyElementalWard(1, leaderProfile ? 8 : 5);
            }
            ++alliesEncouraged;
        }

        if (alliesEncouraged > 0)
        {
            lines.push_back("Alliés coordonnés : " + std::to_string(alliesEncouraged) + ".");
            if (leaderProfile)
            {
                lines.push_back("Ses ordres ne prennent que sur les créatures qui comprennent son rythme.");
                const std::vector<std::string> signatureLines = applyLeaderSpecificFormationSignature(monster, wave, monsterIndex, player, random, compatibleAllies);
                lines.insert(lines.end(), signatureLines.begin(), signatureLines.end());
            }
        }

        if (player.isInDefensePosture() && random.between(1, 100) <= 35)
        {
            player.applyNextHitVulnerability(1, 8 + monster.getLevel() / 18);
            lines.push_back("Lecture de ta garde : la formation cherche un angle pour le prochain coup reçu.");
        }

        player.recordCanonicalEvent("pressions_ennemies_variees", "formation_ennemie", "Une vague ennemie a utilisé une formation", 1);

        showWaveTurnNotice(
            "FORMATION ENNEMIE",
            "wave.monster.formation",
            lines
        );
        return true;
    }

    bool tryMonsterTacticalAction(Monster& monster, Player& player, Random& random)
    {
        const bool hasOpening = playerHasVisibleOpeningForMonster(player);
        const bool trainedProfile = monsterCanUseTacticalPressure(monster);
        const bool woundedMonster = monster.getMaxHp() > 0 && monster.getHp() * 100 <= monster.getMaxHp() * 45;

        int chance = 5;
        if (hasOpening) chance += 14;
        if (trainedProfile) chance += 8;
        if (woundedMonster) chance += 5;
        if (monster.isElite()) chance += 5;

        if (random.between(1, 100) > chance)
        {
            return false;
        }

        std::vector<std::string> lines;
        lines.push_back(monster.getName() + " ne lance pas une attaque directe : il change le rythme du combat.");
        const MonsterBehaviorProfile pressureProfileInfo = MonsterBehaviorProfileCatalog::build(monster);
        lines.push_back("Lecture du comportement : " + pressureProfileInfo.archetype + " - " + pressureProfileInfo.behaviorLine);

        const std::string pressureArchetype = pressureProfileInfo.archetype;
        const std::string pressureText = pressureArchetype + " " + monster.getName() + " " + monster.getType();
        auto containsPressureToken = [&](const std::string& token) {
            return pressureText.find(token) != std::string::npos;
        };

        if (containsPressureToken("rameuteur") || containsPressureToken("chef") || containsPressureToken("capitaine"))
        {
            lines.push_back("Intention ennemie : il ne cherche pas seulement à blesser, il veut garder les autres dans son tempo.");
        }
        else if (containsPressureToken("soigneur") || containsPressureToken("transfuseur"))
        {
            lines.push_back("Intention ennemie : la pression sert surtout à acheter du temps pour réparer la ligne adverse.");
        }
        else if (containsPressureToken("piégeur") || containsPressureToken("illusion"))
        {
            lines.push_back("Intention ennemie : la première ouverture ressemble trop à une invitation pour être honnête.");
        }
        else if (containsPressureToken("dragon") || containsPressureToken("colosse") || containsPressureToken("brute"))
        {
            lines.push_back("Intention ennemie : la menace est simple, massive, et assez lourde pour casser une mauvaise garde.");
        }

        if (hasOpening)
        {
            int damage = std::max(2, monster.getLevel() / 4 + random.between(2, 6));
            if (player.hasWeakening() && player.hasVulnerability())
            {
                damage += 4 + monster.getLevel() / 12;
                player.applyBleeding(2, std::max(1, monster.getLevel() / 25 + 1));
                lines.push_back("Lecture ennemie : ta garde et ta faille se superposent. La pression ouvre une blessure.");
            }
            else if (player.hasPoison() || player.hasBleeding())
            {
                damage += 3 + monster.getLevel() / 14;
                player.applyVulnerability(1, 6 + monster.getLevel() / 24);
                lines.push_back("Lecture ennemie : l'altération déjà présente est poussée au mauvais moment.");
            }
            else if (player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 45)
            {
                damage += 2 + monster.getLevel() / 16;
                player.applyWeakening(1, 7 + monster.getLevel() / 22);
                lines.push_back("Instinct de chasse : la créature force un recul parce que tes blessures se voient.");
            }
            else
            {
                player.applyWeakening(1, 6 + monster.getLevel() / 24);
                lines.push_back("Pression opportuniste : l'ennemi transforme un statut visible en perte de rythme.");
            }

            player.takeDamage(damage);
            lines.push_back("Dégâts de pression : " + std::to_string(damage) + ".");
            player.recordCanonicalEvent("pressions_ennemies_variees", "exploitation_ouverture", "Un ennemi a exploité une ouverture visible", 1);
        }
        else if (trainedProfile)
        {
            const int damage = std::max(1, monster.getLevel() / 5 + random.between(1, 4));
            player.takeDamage(damage);
            player.applyWeakening(1, 6 + monster.getLevel() / 24);
            if (random.between(1, 100) <= 35)
            {
                player.applyVulnerability(1, 5 + monster.getLevel() / 30);
            }
            lines.push_back("Feinte ennemie : profil intelligent, mauvais angle, petite blessure et perte de tempo.");
            if (containsPressureToken("archer") || containsPressureToken("tireur") || containsPressureToken("harceleur"))
            {
                lines.push_back("Lecture de distance : l'ennemi ne s'engage pas vraiment, il te force surtout à répondre trop tôt.");
            }
            else if (containsPressureToken("mage") || containsPressureToken("rune") || containsPressureToken("spectral"))
            {
                lines.push_back("Pression occulte : ce n'est pas le choc qui compte, c'est le moment où ton rythme se dérègle.");
            }
            lines.push_back("Dégâts de feinte : " + std::to_string(damage) + ".");
            player.recordCanonicalEvent("pressions_ennemies_variees", "feinte_intelligente", "Un ennemi intelligent a utilisé une feinte", 1);
        }
        else
        {
            const int damage = std::max(1, monster.getLevel() / 6 + random.between(1, 3));
            player.takeDamage(damage);
            player.applyVulnerability(1, 4 + monster.getLevel() / 30);
            lines.push_back("Pression bestiale : pas de grande stratégie, mais assez de brutalité pour ouvrir une fenêtre.");
            if (woundedMonster)
            {
                lines.push_back("Réaction blessée : la créature ne calcule plus bien, mais sa panique rend l'impact moins prévisible.");
            }
            lines.push_back("Dégâts de pression : " + std::to_string(damage) + ".");
            player.recordCanonicalEvent("pressions_ennemies_variees", "pression_bestiale", "Une créature a utilisé une pression non basique", 1);
        }

        ThreatSystem::consumeForcedTargetIfNeeded(player);
        MessageScreen::show(
            "PRESSION ENNEMIE",
            "wave.monster.tactical_pressure",
            lines,
            false
        );
        return true;
    }
}

void MonsterWaveCombatTurn::playMonsterTurns(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random
)
{
    int i = 0;

    while (i < wave.getActiveEnemyCount())
    {
        if (player.isDead())
        {
            return;
        }

        if (!wave.isActiveIndexValid(i))
        {
            ++i;
            continue;
        }

        Monster& monster = wave.getActiveEnemy(i);

        if (monster.isDead())
        {
            ++i;
            continue;
        }

        if (monster.consumeEntanglementTurn())
        {
            showWaveTurnNotice(
                "ENNEMI ENTRAVÉ",
                "wave.monster.entangled_skip",
                {
                    monster.getName() + " perd son tour : l'entrave tient encore son corps.",
                    "Les blocages d'araignée, liane ou contrôle ne sont courts, mais ils changent vraiment le tempo."
                }
            );
            ++i;
            continue;
        }

        if (EscapeSystem::monsterAttemptsEscape(monster, random))
        {
            wave.removeActiveEnemyAsEscaped(i);
            continue;
        }

        std::vector<std::string> turnStartLines;
        turnStartLines.push_back("Tour de " + monster.getName() + ".");
        const std::vector<std::string> turnFlavorLines = MonsterBehaviorProfileCatalog::buildTurnFlavorLines(monster, player);
        turnStartLines.insert(turnStartLines.end(), turnFlavorLines.begin(), turnFlavorLines.end());
        showWaveTurnNotice(
            "TOUR DE VAGUE",
            "wave.monster.turn.start",
            turnStartLines
        );

        Console::pauseSeconds(1);

        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player, random);

        if (tryMonsterReinforcementCall(monster, wave, i, player, random))
        {
            Console::pauseSeconds(1);
            ++i;
            continue;
        }

        if (tryTankProtectionAction(monster, wave, i, random)
            // EN: tryHealerSupportAction declares or implements a focused behavior used by this module.
            // FR: tryHealerSupportAction déclare ou implémente un comportement précis utilisé par ce module.
            || tryHealerSupportAction(monster, wave, i, random)
            // EN: trySupportRecoveryAction declares or implements a focused behavior used by this module.
            // FR: trySupportRecoveryAction déclare ou implémente un comportement précis utilisé par ce module.
            || trySupportRecoveryAction(monster, wave, i, random))
        {
            Console::pauseSeconds(1);
            ++i;
            continue;
        }

        if (!tryMonsterMoraleReaction(monster, wave, i, player, random)
            && !tryMonsterSignatureSkill(monster, wave, i, player, random)
            && !tryMonsterFormationAction(monster, wave, i, player, random)
            && !tryMonsterTacticalAction(monster, player, random))
        {
            TurnManager::executeAttack(
                monster,
                player,
                random
            );

            ThreatSystem::consumeForcedTargetIfNeeded(player);
        }

        Console::pauseSeconds(1);

        ++i;
    }

    wave.removeDeadAndReplace();
}

void MonsterWaveCombatTurn::playMonsterTurns(
    Player& player,
    EnemyCombatQueue& wave,
    std::vector<Summon>& playerSummons,
    Random& random
)
{
    int i = 0;

    while (i < wave.getActiveEnemyCount())
    {
        if (player.isDead())
        {
            return;
        }

        if (!wave.isActiveIndexValid(i))
        {
            ++i;
            continue;
        }

        Monster& monster = wave.getActiveEnemy(i);

        if (monster.isDead())
        {
            ++i;
            continue;
        }

        if (monster.consumeEntanglementTurn())
        {
            showWaveTurnNotice(
                "ENNEMI ENTRAVÉ",
                "wave.monster.entangled_skip",
                {
                    monster.getName() + " perd son tour : l'entrave tient encore son corps.",
                    "Les blocages d'araignée, liane ou contrôle ne sont courts, mais ils changent vraiment le tempo."
                }
            );
            ++i;
            continue;
        }

        if (EscapeSystem::monsterAttemptsEscape(monster, random))
        {
            wave.removeActiveEnemyAsEscaped(i);
            continue;
        }

        std::vector<std::string> turnStartLines;
        turnStartLines.push_back("Tour de " + monster.getName() + ".");
        const std::vector<std::string> turnFlavorLines = MonsterBehaviorProfileCatalog::buildTurnFlavorLines(monster, player);
        turnStartLines.insert(turnStartLines.end(), turnFlavorLines.begin(), turnFlavorLines.end());
        showWaveTurnNotice(
            "TOUR DE VAGUE",
            "wave.monster.turn.start",
            turnStartLines
        );

        Console::pauseSeconds(1);

        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player, random);

        if (tryMonsterReinforcementCall(monster, wave, i, player, random))
        {
            Console::pauseSeconds(1);
            ++i;
            continue;
        }

        if (tryTankProtectionAction(monster, wave, i, random)
            // EN: tryHealerSupportAction declares or implements a focused behavior used by this module.
            // FR: tryHealerSupportAction déclare ou implémente un comportement précis utilisé par ce module.
            || tryHealerSupportAction(monster, wave, i, random)
            // EN: trySupportRecoveryAction declares or implements a focused behavior used by this module.
            // FR: trySupportRecoveryAction déclare ou implémente un comportement précis utilisé par ce module.
            || trySupportRecoveryAction(monster, wave, i, random))
        {
            Console::pauseSeconds(1);
            ++i;
            continue;
        }

        bool attackedSummon = false;

        if (ThreatSystem::shouldForceTargetMainEntity(player, monster.getName()))
        {
            ThreatSystem::notifyForcedTarget(player, monster.getName());
        }
        else if (SummonCombatSystem::hasTargetableSummons(playerSummons)
            && random.between(1, 100) <= CombatAI::getSummonTargetPriorityChance(monster))
        {
            int summonIndex = SummonCombatSystem::chooseStrategicTargetableSummonIndex(
                playerSummons,
                monster,
                random
            );

            if (summonIndex >= 0)
            {
                showWaveTurnNotice(
                    "CIBLE CHANGÉE",
                    "wave.monster.target.summon",
                    {
                        monster.getName() + " se jette sur une invocation.",
                        player.getName() + " n'est pas visé directement pendant cette action."
                    }
                );

                SummonCombatSystem::entityAttacksSummon(
                    monster,
                    playerSummons[summonIndex],
                    random
                );

                SummonCombatSystem::removeInactiveSummons(playerSummons);
                attackedSummon = true;
            }
        }

        if (!attackedSummon)
        {
            if (!tryMonsterMoraleReaction(monster, wave, i, player, random)
                && !tryMonsterSignatureSkill(monster, wave, i, player, random)
                && !tryMonsterFormationAction(monster, wave, i, player, random)
                && !tryMonsterTacticalAction(monster, player, random))
            {
                TurnManager::executeAttack(
                    monster,
                    player,
                    random
                );

                ThreatSystem::consumeForcedTargetIfNeeded(player);
            }
        }

        Console::pauseSeconds(1);

        ++i;
    }

    wave.removeDeadAndReplace();
}
