// EN: PlayerMana.cpp owns the persistent mana resource and its exhaustion state.
// FR: PlayerMana.cpp gère la ressource de mana persistante et son état d'épuisement.
#include "entity/Player.hpp"
#include "combat/MagicMaterialRules.hpp"
#include "item/weapon/WeaponType.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    std::string normalizedMagicClassName(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text;
    }

    bool containsAny(const std::string& text, const std::initializer_list<const char*>& needles)
    {
        for (const char* needle : needles)
        {
            if (text.find(needle) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
}

int Player::getMaxMana() const
{
    const std::string className = normalizedMagicClassName(getType());
    const int mentalBonus = std::max(0, (attributes.getIntelligence() + attributes.getWisdom() - 20) / 2);

    // Material/mineral casters trade reservoir size for extremely efficient formulae.
    if (containsAny(className, {"géom", "geom", "crist", "run", "alchim", "joaill"}))
    {
        return std::max(25, 42 + level * 2 + mentalBonus);
    }

    // Pure incantation classes carry a larger raw reserve because they have no material/catalyst discount.
    if (containsAny(className, {"ensor", "sorc", "arcan", "occult", "chronom", "démon", "demon", "mage fou", "sang", "nécro", "necro", "pactis"}))
    {
        return std::max(35, 82 + level * 3 + mentalBonus * 2);
    }

    // Conventional wand/staff users sit in the middle and rely on catalyst durability.
    if (containsAny(className, {"mage", "magicien", "mancien", "mirag", "pyro", "cryo", "foudre", "glace", "feu", "invoc", "clerc", "prêtre", "pretre", "paladin", "templier", "druide", "shaman", "chaman", "oracle"}))
    {
        return std::max(30, 68 + level * 3 + mentalBonus * 2);
    }

    // Everyone has a tiny internal reserve: enough for scrolls/ritual interactions, not sustained spellcasting.
    return std::max(15, 20 + level + mentalBonus);
}

int Player::getMana() const
{
    return std::clamp(currentMana, 0, getMaxMana());
}

int Player::getManaExhaustionTurns() const
{
    return std::max(0, manaExhaustionTurns);
}

bool Player::spendMana(int amount)
{
    amount = std::max(0, amount);
    if (amount > getMana())
    {
        return false;
    }

    currentMana = getMana() - amount;
    return true;
}

void Player::restoreMana(int amount)
{
    if (amount <= 0)
    {
        return;
    }
    currentMana = std::min(getMaxMana(), getMana() + amount);
}

void Player::restoreManaToFull()
{
    currentMana = getMaxMana();
    manaExhaustionTurns = 0;
}

void Player::regenerateManaForCombatTurn()
{
    if (getMana() >= getMaxMana())
    {
        return;
    }

    // Early progression keeps the original ~4% rhythm, but the absolute gain is capped.
    // Without this ceiling, level-255 mana pools regenerate more than several real spell
    // costs every turn, making high-level catalyst casting effectively free.
    const int regeneration = std::clamp(getMaxMana() / 25, 1, 5);
    restoreMana(regeneration);
}

void Player::applyManaExhaustion(int turns)
{
    manaExhaustionTurns = std::max(manaExhaustionTurns, std::max(0, turns));
}

bool Player::consumeManaExhaustionTurn()
{
    if (manaExhaustionTurns <= 0)
    {
        return false;
    }

    --manaExhaustionTurns;
    return true;
}

void Player::setLoadedManaState(int mana, int exhaustionTurns)
{
    currentMana = std::clamp(mana, 0, getMaxMana());
    manaExhaustionTurns = std::max(0, exhaustionTurns);
}

std::string Player::getMagicChannelingStyleLabel() const
{
    const std::string className = normalizedMagicClassName(getType());
    if (containsAny(className, {"géom", "geom", "crist", "run", "alchim", "joaill"}))
    {
        return "Minerais / matériaux";
    }
    if (containsAny(className, {"ensor", "sorc", "arcan", "occult", "chronom", "démon", "demon", "mage fou", "sang", "nécro", "necro", "pactis"}))
    {
        return "Incantation pure";
    }
    if (containsAny(className, {"mage", "magicien", "mancien", "mirag", "pyro", "cryo", "foudre", "glace", "feu", "invoc", "clerc", "prêtre", "pretre", "paladin", "templier", "druide", "shaman", "chaman", "oracle"}))
    {
        return "Baguette / catalyseur";
    }
    return "Réserve mineure";
}

std::string Player::getMagicRecoveryLabel() const
{
    const int adjustment = getMagicRecoveryAdjustment();
    if (adjustment < 0)
    {
        return "rapide";
    }
    if (adjustment > 0)
    {
        return "lent";
    }
    return "standard";
}

std::string Player::getMagicSupportStatusLine() const
{
    const std::string style = getMagicChannelingStyleLabel();

    if (style == "Minerais / matériaux")
    {
        const int total = MagicMaterialRules::countAvailableMaterials(inventory);
        if (total <= 0)
        {
            return "Support magique : aucun composant adapté — fallback en incantation pure.";
        }

        return "Support magique : " + std::to_string(total) + " composant(s) adapté(s) ("
            + MagicMaterialRules::formatAvailableMaterialStocks(inventory) + ").";
    }

    if (style == "Baguette / catalyseur")
    {
        if (hasEquippedWeapon() && getEquippedWeapon().getType() == WeaponType::Staff)
        {
            const Weapon& catalyst = getEquippedWeapon();
            if (catalyst.isBroken())
            {
                return "Support magique : " + catalyst.getName() + " brisé — fallback en incantation pure.";
            }
            return "Support magique : " + catalyst.getName() + " ["
                + std::to_string(catalyst.getDurability()) + "/" + std::to_string(catalyst.getMaxDurability())
                + "] — catalyseur fonctionnel.";
        }
        return "Support magique : aucun catalyseur équipé — fallback en incantation pure.";
    }

    if (style == "Incantation pure")
    {
        return "Support magique : aucun support obligatoire.";
    }

    return "Support magique : réserve mineure, aucune voie spécialisée.";
}
int Player::getMagicRecoveryAdjustment() const
{
    const std::string style = getMagicChannelingStyleLabel();
    if (style == "Incantation pure")
    {
        // Pure chanting needs no external support, but the caster must recover their breath/focus.
        return 1;
    }

    if (style == "Baguette / catalyseur")
    {
        // The faster rhythm only exists while a functional catalyst is actually equipped.
        if (hasEquippedWeapon()
            && getEquippedWeapon().getType() == WeaponType::Staff
            && !getEquippedWeapon().isBroken())
        {
            return -1;
        }
        // Missing/broken support forces a slower improvised incantation.
        return 1;
    }

    // Material casters save mana through their component, not through recovery speed.
    return 0;
}
