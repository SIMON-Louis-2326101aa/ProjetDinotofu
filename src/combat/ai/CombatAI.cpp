// EN: CombatAI.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatAI.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/ai/CombatAI.hpp"

#include "entity/Player.hpp"
#include "entity/Boss.hpp"
#include "entity/Monster.hpp"
#include "combat/role/CombatRoleSystem.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>



namespace
{
    std::string normalizeAIText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

        std::string::size_type position = 0;
        while ((position = value.find("é", position)) != std::string::npos)
        {
            value.replace(position, 2, "e");
            position += 1;
        }
        position = 0;
        while ((position = value.find("è", position)) != std::string::npos)
        {
            value.replace(position, 2, "e");
            position += 1;
        }
        position = 0;
        while ((position = value.find("ê", position)) != std::string::npos)
        {
            value.replace(position, 2, "e");
            position += 1;
        }

        return value;
    }

    bool aiTextContainsAny(const std::string& value, std::initializer_list<const char*> terms)
    {
        for (const char* term : terms)
        {
            if (value.find(term) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    bool aiLooksLikeAny(const Entity& entity, std::initializer_list<const char*> terms)
    {
        return aiTextContainsAny(
            normalizeAIText(entity.getName() + " " + entity.getType()),
            terms
        );
    }

    bool isSpecialNamedCombatant(const Entity& entity)
    {
        std::string normalizedName = normalizeAIText(entity.getName());

        return normalizedName == "hazak"
            || normalizedName == "skuro"
            || normalizedName == "kanade"
            || normalizedName == "aoi"
            || normalizedName == "sanctus"
            || normalizedName == "fail"
            || normalizedName == "hestia"
            || normalizedName == "fire flight"
            || normalizedName == "fireflight"
            || normalizedName == "louis"
            || normalizedName == "mattzelda"
            || normalizedName == "trexof"
            || normalizedName == "henrique"
            || normalizedName == "matt (pro)"
            || normalizedName == "matt pro";
    }

    bool isCredibleHealingProfile(const Entity& entity)
    {
        if (isSpecialNamedCombatant(entity))
        {
            return true;
        }

        if (aiLooksLikeAny(entity, {
            "shaman", "chamane", "oracle", "clerc", "pretre", "prêtre",
            "medecin", "médecin", "apothicaire", "alchimiste", "soigneur",
            "support", "sorcier", "mage", "chef", "pilleur veteran",
            "pilleur vétéran", "grand shaman", "grand chamane", "bandit apothicaire"
        }))
        {
            return true;
        }

        return false;
    }

    bool isCredibleConsumableUser(const Entity& entity)
    {
        if (isCredibleHealingProfile(entity))
        {
            return true;
        }

        return aiLooksLikeAny(entity, {
            "humain", "humano", "gobelin", "hobgobelin", "orc", "demon", "démon",
            "mercenaire", "bandit", "pillard", "receleur", "chasseur", "chevalier",
            "artificier", "tireur", "archer", "frondeur", "mage", "sorcier",
            "elementaire", "élémentaire", "esprit", "anomalie", "draconide"
        });
    }

    int inferAILevel(const Entity& entity)
    {
        const Player* player = dynamic_cast<const Player*>(&entity);
        if (player != nullptr)
        {
            return player->getLevel();
        }

        const Monster* monster = dynamic_cast<const Monster*>(&entity);
        if (monster != nullptr)
        {
            return monster->getLevel();
        }

        const Boss* boss = dynamic_cast<const Boss*>(&entity);
        if (boss != nullptr)
        {
            return 12;
        }

        if (isSpecialNamedCombatant(entity))
        {
            return 8;
        }

        return std::max(1, (entity.getMaxDamage() + entity.getCriticalDamage()) / 12);
    }

    int getClassSkillUnlockRequirement(const Entity& entity)
    {
        std::string profile = normalizeAIText(entity.getName() + " " + entity.getType());

        int requiredLevel = 999;

        if (aiTextContainsAny(profile, {
            "assassin", "ombrelame", "lanceur de dagues",
            "gardien", "tank", "colosse",
            "clerc", "pretre", "prêtre", "paladin",
            "berserker", "barbare", "briseur"
        }))
        {
            requiredLevel = 3;
        }
        else if (aiTextContainsAny(profile, {
            "mage", "sorcier", "arcaniste", "pyromancien",
            "archer", "rodeur", "rôdeur", "tireur",
            "alchim", "artific", "bricoleur",
            "invoc", "dresseur", "necro", "nécro"
        }))
        {
            requiredLevel = 4;
        }

        if (requiredLevel == 999)
        {
            return requiredLevel;
        }

        if (aiTextContainsAny(profile, {
            "apprenti", "novice", "jeune", "petit", "faible", "ramasseur",
            "rouille", "rouillé", "affame", "affamé", "errant", "improvisé"
        }))
        {
            requiredLevel += 2;
        }

        if (aiTextContainsAny(profile, {
            "elite", "élite", "veteran", "vétéran", "chef", "maitre", "maître",
            "oracle", "grand", "champion", "ancien"
        }) || isSpecialNamedCombatant(entity))
        {
            requiredLevel = std::max(2, requiredLevel - 1);
        }

        return requiredLevel;
    }

    bool hasDisciplinedClassSkillProfile(const Entity& entity)
    {
        std::string profile = normalizeAIText(entity.getName() + " " + entity.getType());

        if (isSpecialNamedCombatant(entity))
        {
            return true;
        }

        if (aiTextContainsAny(profile, {
            "apprenti", "novice", "jeune", "petit", "ramasseur", "rouille", "rouillé"
        }))
        {
            return false;
        }

        return aiTextContainsAny(profile, {
            "assassin", "ombrelame", "lanceur de dagues",
            "gardien", "tank", "colosse",
            "clerc", "pretre", "prêtre", "paladin",
            "mage", "sorcier", "arcaniste", "pyromancien",
            "archer", "rodeur", "rôdeur", "tireur",
            "berserker", "barbare", "briseur",
            "alchim", "artific", "bricoleur",
            "invoc", "dresseur", "necro", "nécro",
            "elite", "élite", "veteran", "vétéran", "chef", "oracle", "champion"
        });
    }

}

// EN: canUseHealingPotion declares or implements a focused behavior used by this module.
// FR: canUseHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
bool CombatAI::canUseHealingPotion(const Entity& entity)
{
    const Player* player = dynamic_cast<const Player*>(&entity);

    if (player != nullptr)
    {
        return player->getInventory().countConsumables(ConsumableType::Healing) > 0;
    }

    const Boss* boss = dynamic_cast<const Boss*>(&entity);
    if (boss != nullptr)
    {
        return entity.getHealingPotionCount() > 0;
    }

    // FR: Une bête, un slime ou une plante ne devient pas soigneur par magie.
    // Seuls les profils crédibles savent exploiter une potion/technique de soin.
    return entity.getHealingPotionCount() > 0 && isCredibleHealingProfile(entity);
}

// EN: canUseDamagePotion declares or implements a focused behavior used by this module.
// FR: canUseDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
bool CombatAI::canUseDamagePotion(const Entity& entity)
{
    const Player* player = dynamic_cast<const Player*>(&entity);

    if (player != nullptr)
    {
        return player->getInventory().countConsumables(ConsumableType::Damage) > 0;
    }

    const Boss* boss = dynamic_cast<const Boss*>(&entity);
    if (boss != nullptr)
    {
        return entity.getDamagePotionCount() > 0;
    }

    return entity.getDamagePotionCount() > 0 && isCredibleConsumableUser(entity);
}


bool CombatAI::canUseClassSkill(const Entity& entity)
{
    if (!entity.isClassSkillReady())
    {
        return false;
    }

    std::string profile = normalizeAIText(entity.getName() + " " + entity.getType());

    if (!aiTextContainsAny(profile, {
        "assassin", "ombrelame", "lanceur de dagues",
        "gardien", "tank", "colosse",
        "clerc", "pretre", "prêtre", "paladin",
        "mage", "sorcier", "arcaniste", "pyromancien",
        "archer", "rodeur", "rôdeur", "tireur",
        "berserker", "barbare", "briseur",
        "alchim", "artific", "bricoleur",
        "invoc", "dresseur", "necro", "nécro"
    }))
    {
        return false;
    }

    if (!hasDisciplinedClassSkillProfile(entity))
    {
        return false;
    }

    return inferAILevel(entity) >= getClassSkillUnlockRequirement(entity);
}

int CombatAI::getClassSkillUseChance(const Entity& entity)
{
    std::string profile = normalizeAIText(entity.getName() + " " + entity.getType());
    int hpPercentage = calculateHpPercentage(entity);
    int level = inferAILevel(entity);
    int unlockRequirement = getClassSkillUnlockRequirement(entity);

    if (level < unlockRequirement)
    {
        return 0;
    }

    int levelBonus = std::min(10, std::max(0, level - unlockRequirement));
    int personalityBonus = 0;

    if (aiTextContainsAny(profile, {"elite", "élite", "veteran", "vétéran", "chef", "oracle", "champion"})
        || isSpecialNamedCombatant(entity))
    {
        personalityBonus += 6;
    }

    if (aiTextContainsAny(profile, {"apprenti", "novice", "jeune", "petit", "ramasseur", "rouille", "rouillé"}))
    {
        personalityBonus -= 10;
    }

    auto finalizeChance = [&](int baseChance) {
        return std::max(0, std::min(45, baseChance + levelBonus + personalityBonus));
    };

    if (aiTextContainsAny(profile, {"clerc", "pretre", "prêtre", "paladin", "support", "shaman", "chamane"}))
    {
        return finalizeChance(hpPercentage <= 65 ? 34 : 14);
    }

    if (aiTextContainsAny(profile, {"assassin", "ombrelame", "berserker", "barbare", "briseur"}))
    {
        return finalizeChance(hpPercentage >= 35 ? 26 : 16);
    }

    if (aiTextContainsAny(profile, {"mage", "sorcier", "arcaniste", "pyromancien", "archer", "tireur", "rôdeur", "rodeur"}))
    {
        return finalizeChance(22);
    }

    if (aiTextContainsAny(profile, {"gardien", "tank", "colosse"}))
    {
        return finalizeChance(hpPercentage <= 70 ? 24 : 12);
    }

    if (aiTextContainsAny(profile, {"alchim", "artific", "bricoleur", "invoc", "dresseur", "necro", "nécro"}))
    {
        return finalizeChance(20);
    }

    return 0;
}

// EN: calculateHpPercentage declares or implements a focused behavior used by this module.
// FR: calculateHpPercentage déclare ou implémente un comportement précis utilisé par ce module.
int CombatAI::calculateHpPercentage(const Entity& entity)
{
    if (entity.getMaxHp() <= 0)
    {
        return 0;
    }

    return entity.getHp() * 100 / entity.getMaxHp();
}

// EN: chooseAIAction declares or implements a focused behavior used by this module.
// FR: chooseAIAction déclare ou implémente un comportement précis utilisé par ce module.
AIAction CombatAI::chooseAIAction(const Entity& ai, Random& random)
{
    std::string normalizedName = normalizeName(ai.getName());

    if (normalizedName == "hazak"
        || normalizedName == "skuro"
        || normalizedName == "kanade"
        || normalizedName == "aoi"
        || normalizedName == "sanctus"
        || normalizedName == "fail"
        || normalizedName == "hestia"
        || normalizedName == "fire flight"
        || normalizedName == "louis"
        || normalizedName == "mattzelda"
        || normalizedName == "trexof"
        || normalizedName == "henrique"
        || normalizedName == "matt (pro)")
    {
        return chooseSpecialCharacterAction(ai, random);
    }

    bool healingAvailable = canUseHealingPotion(ai);
    bool damageAvailable = canUseDamagePotion(ai);
    bool classSkillAvailable = canUseClassSkill(ai);

    int hpPercentage = calculateHpPercentage(ai);
    int roll = random.between(1, 100);
    std::string aiFocus = normalizeAIText(ai.getName() + " " + ai.getType());

    bool supportProfile = aiTextContainsAny(aiFocus, {"shaman", "chamane", "oracle", "clerc", "pretre", "apothicaire", "medecin", "support"});
    bool aggressiveProfile = aiTextContainsAny(aiFocus, {"assassin", "berserker", "barbare", "briseur", "loup", "predateur", "pillard", "orc"});
    bool rangedOrCasterProfile = aiTextContainsAny(aiFocus, {"archer", "tireur", "frondeur", "mage", "sorcier", "arcanique", "elementaire"});
    bool heavyProfile = aiTextContainsAny(aiFocus, {"tank", "colosse", "gardien", "golem", "armure", "construction"});

    if (classSkillAvailable && roll <= getClassSkillUseChance(ai))
    {
        return AIAction::ClassSkill;
    }

    if (supportProfile && healingAvailable && hpPercentage <= 70 && roll <= 55)
    {
        return AIAction::HealingPotion;
    }

    if (heavyProfile && healingAvailable && hpPercentage <= 42 && roll <= 45)
    {
        return AIAction::HealingPotion;
    }

    if (aggressiveProfile && damageAvailable && hpPercentage >= 45 && roll <= 32)
    {
        return AIAction::DamagePotion;
    }

    if (rangedOrCasterProfile && damageAvailable && hpPercentage >= 50 && roll <= 26)
    {
        return AIAction::DamagePotion;
    }

    if (hpPercentage <= 30)
    {
        return chooseDangerAIAction(
            healingAvailable,
            damageAvailable,
            roll
        );
    }

    if (hpPercentage <= 60)
    {
        return chooseMidHealthAIAction(
            healingAvailable,
            damageAvailable,
            roll
        );
    }

    return chooseStableAIAction(
        damageAvailable,
        roll
    );
}

// EN: getSummonTargetPriorityChance declares or implements a focused behavior used by this module.
// FR: getSummonTargetPriorityChance déclare ou implémente un comportement précis utilisé par ce module.
int CombatAI::getSummonTargetPriorityChance(const Entity& attacker)
{
    std::string name = normalizeName(attacker.getName());
    std::string type = normalizeName(attacker.getType());

    if (name == "sanctus" || name == "mattzelda")
    {
        return 18;
    }

    if (name == "hazak" || name == "skuro" || name == "trexof")
    {
        return 58;
    }

    if (name == "aoi" || name == "kanade" || name == "fail" || name == "fire flight")
    {
        return 44;
    }

    if (name == "matt (pro)")
    {
        return 50;
    }

    if (CombatRoleSystem::isAssassin(attacker) || type.find("assassin") != std::string::npos)
    {
        return 52;
    }

    if (CombatRoleSystem::isSummoner(attacker)
        || type.find("mage") != std::string::npos
        || type.find("invoc") != std::string::npos)
    {
        return 45;
    }

    if (CombatRoleSystem::isTank(attacker)
        || type.find("tank") != std::string::npos
        || type.find("colosse") != std::string::npos)
    {
        return 20;
    }

    if (type.find("loup") != std::string::npos || type.find("bete") != std::string::npos || type.find("bête") != std::string::npos)
    {
        return 45;
    }

    return 30;
}


// EN: chooseSpecialCharacterAction declares or implements a focused behavior used by this module.
// FR: chooseSpecialCharacterAction déclare ou implémente un comportement précis utilisé par ce module.
AIAction CombatAI::chooseSpecialCharacterAction(const Entity& ai, Random& random)
{
    bool healingAvailable = canUseHealingPotion(ai);
    bool damageAvailable = canUseDamagePotion(ai);
    bool classSkillAvailable = canUseClassSkill(ai);

    int hpPercentage = calculateHpPercentage(ai);
    int roll = random.between(1, 100);
    std::string name = normalizeName(ai.getName());

    if (classSkillAvailable && roll <= getClassSkillUseChance(ai))
    {
        return AIAction::ClassSkill;
    }

    if (name == "hestia")
    {
        if (hpPercentage <= 80 && healingAvailable && roll <= 78) return AIAction::HealingPotion;
        if (roll <= 85) return AIAction::Attack;
        return AIAction::SkipTurn;
    }

    if (name == "sanctus")
    {
        if (hpPercentage <= 45 && healingAvailable && roll <= 45) return AIAction::HealingPotion;
        if (damageAvailable && roll <= 38) return AIAction::DamagePotion;
        return AIAction::Attack;
    }

    if (name == "hazak" || name == "skuro" || name == "trexof")
    {
        if (hpPercentage <= 22 && healingAvailable && roll <= 25) return AIAction::HealingPotion;
        if (damageAvailable && roll <= 55) return AIAction::DamagePotion;
        return AIAction::Attack;
    }

    if (name == "fail" || name == "kanade" || name == "aoi" || name == "fire flight")
    {
        if (hpPercentage <= 35 && healingAvailable && roll <= 35) return AIAction::HealingPotion;
        if (damageAvailable && roll <= 42) return AIAction::DamagePotion;
        return AIAction::Attack;
    }

    if (name == "mattzelda" || name == "henrique")
    {
        if (hpPercentage <= 30 && healingAvailable && roll <= 35) return AIAction::HealingPotion;
        if (damageAvailable && roll <= 30) return AIAction::DamagePotion;
        return AIAction::Attack;
    }

    if (name == "louis" || name == "matt (pro)")
    {
        if (hpPercentage <= 35 && healingAvailable && roll <= 40) return AIAction::HealingPotion;
        if (damageAvailable && roll <= 25) return AIAction::DamagePotion;
        if (roll <= 98) return AIAction::Attack;
        return AIAction::SkipTurn;
    }

    return chooseStableAIAction(damageAvailable, roll);
}

// EN: chooseBossAction declares or implements a focused behavior used by this module.
// FR: chooseBossAction déclare ou implémente un comportement précis utilisé par ce module.
AIAction CombatAI::chooseBossAction(const Boss& boss, Random& random)
{
    bool healingAvailable = boss.getHealingPotionCount() > 0;
    bool damageAvailable = boss.getDamagePotionCount() > 0;
    bool ultimateAvailable = boss.canUseUltimate();

    int hpPercentage = calculateHpPercentage(boss);
    int roll = random.between(1, 100);

    if (hpPercentage <= 25)
    {
        return chooseDangerBossAction(
            healingAvailable,
            damageAvailable,
            ultimateAvailable,
            roll
        );
    }

    if (ultimateAvailable)
    {
        return chooseBossActionWithUltimateAvailable(
            damageAvailable,
            roll
        );
    }

    return chooseStableBossAction(
        damageAvailable,
        roll
    );
}

AIAction CombatAI::chooseDangerAIAction(
    bool healingAvailable,
    bool damageAvailable,
    int roll
)
{
    if (healingAvailable && roll <= 60)
    {
        return AIAction::HealingPotion;
    }

    if (damageAvailable && roll <= 75)
    {
        return AIAction::DamagePotion;
    }

    if (roll <= 97)
    {
        return AIAction::Attack;
    }

    return AIAction::SkipTurn;
}

AIAction CombatAI::chooseMidHealthAIAction(
    bool healingAvailable,
    bool damageAvailable,
    int roll
)
{
    if (healingAvailable && roll <= 30)
    {
        return AIAction::HealingPotion;
    }

    if (damageAvailable && roll <= 48)
    {
        return AIAction::DamagePotion;
    }

    if (roll <= 96)
    {
        return AIAction::Attack;
    }

    return AIAction::SkipTurn;
}

AIAction CombatAI::chooseStableAIAction(
    bool damageAvailable,
    int roll
)
{
    if (damageAvailable && roll <= 18)
    {
        return AIAction::DamagePotion;
    }

    if (roll <= 96)
    {
        return AIAction::Attack;
    }

    return AIAction::SkipTurn;
}

AIAction CombatAI::chooseDangerBossAction(
    bool healingAvailable,
    bool damageAvailable,
    bool ultimateAvailable,
    int roll
)
{
    if (healingAvailable && roll <= 35)
    {
        return AIAction::HealingPotion;
    }

    if (ultimateAvailable && roll <= 55)
    {
        return AIAction::Ultimate;
    }

    if (damageAvailable && roll <= 87)
    {
        return AIAction::DamagePotion;
    }

    return AIAction::Attack;
}

AIAction CombatAI::chooseBossActionWithUltimateAvailable(
    bool damageAvailable,
    int roll
)
{
    if (roll <= 38)
    {
        return AIAction::Ultimate;
    }

    if (damageAvailable && roll <= 70)
    {
        return AIAction::DamagePotion;
    }

    return AIAction::Attack;
}

AIAction CombatAI::chooseStableBossAction(
    bool damageAvailable,
    int roll
)
{
    if (damageAvailable && roll <= 18)
    {
        return AIAction::DamagePotion;
    }

    return AIAction::Attack;
}
std::string CombatAI::normalizeName(const std::string& name)
{
    std::string normalized = name;

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    std::string::size_type position = 0;
    while ((position = normalized.find("é", position)) != std::string::npos)
    {
        normalized.replace(position, 2, "e");
        position += 1;
    }

    return normalized;
}
