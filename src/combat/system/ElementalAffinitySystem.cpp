// EN: ElementalAffinitySystem centralizes elemental status affinities.
// FR: ElementalAffinitySystem centralise les affinités élémentaires des statuts.

#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include "character/CharacterRace.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::string normalizeAffinityText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& value, const std::vector<std::string>& terms)
    {
        const std::string normalized = normalizeAffinityText(value);
        for (const std::string& term : terms)
        {
            if (normalized.find(normalizeAffinityText(term)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    void addRaceAffinity(const Player& player, const std::string& elementId, int& resistance, int& weakness)
    {
        CharacterRace race = player.getRace();

        // Les passifs raciaux débloqués servent aussi en combat : une résistance feu de race réduit les brûlures,
        // une faiblesse aux flammes les rend plus violentes. L'environnement et le combat partagent donc la même logique.
        if (player.hasPassiveSkill("minor_fire_resistance") && elementId == "fire") resistance += 10;
        if (player.hasPassiveSkill("infernal_fire_resistance") && elementId == "fire") resistance += 22;
        if (player.hasPassiveSkill("minor_cold_resistance") && elementId == "frost") resistance += 12;
        if (player.hasPassiveSkill("fire_vulnerability") && elementId == "fire") weakness += 14;

        if (race == CharacterRace::Tiefling || race == CharacterRace::Demon)
        {
            if (elementId == "fire") resistance += 25;
            if (elementId == "poison") resistance += 10;
        }

        if (race == CharacterRace::Aasimar)
        {
            if (elementId == "poison") resistance += 15;
            if (elementId == "fire") resistance += 8;
        }

        if (race == CharacterRace::Dwarf)
        {
            if (elementId == "poison") resistance += 25;
            if (elementId == "frost") resistance += 10;
            if (elementId == "bleeding") resistance += 10;
        }

        if (race == CharacterRace::Kitsune)
        {
            if (elementId == "fire") resistance += 18;
            if (elementId == "frost") weakness += 10;
        }

        if (race == CharacterRace::Fairy)
        {
            if (elementId == "shock") resistance += 10;
            if (elementId == "fire") weakness += 12;
            if (elementId == "bleeding") weakness += 8;
        }

        if (race == CharacterRace::SemiLizard)
        {
            if (elementId == "fire") resistance += 12;
            if (elementId == "frost") weakness += 10;
        }

        if (race == CharacterRace::SemiBird)
        {
            if (elementId == "fire") weakness += 18;
            if (elementId == "shock") resistance += 6;
        }

        if (race == CharacterRace::SemiWolf || race == CharacterRace::SemiDog)
        {
            if (elementId == "frost") resistance += 6;
            if (elementId == "bleeding") resistance += 4;
        }

        if (race == CharacterRace::SemiCat || race == CharacterRace::SemiFox)
        {
            if (elementId == "poison") resistance += 4;
            if (elementId == "bleeding") weakness += 3;
        }

        if (race == CharacterRace::HalfDragon)
        {
            if (elementId == "fire" || elementId == "frost" || elementId == "shock") resistance += 12;
        }

        if (race == CharacterRace::Orc)
        {
            if (elementId == "bleeding") resistance += 18;
            if (elementId == "poison") resistance += 8;
        }

        if (race == CharacterRace::Vampire)
        {
            if (elementId == "fire") weakness += 25;
            if (elementId == "poison") resistance += 20;
            if (elementId == "bleeding") weakness += 10;
        }

        if (race == CharacterRace::Elf || race == CharacterRace::DarkElf)
        {
            if (elementId == "poison") resistance += 8;
            if (elementId == "bleeding") weakness += 6;
        }
    }

    void addClassAffinity(const Entity& target, const std::string& elementId, int& resistance, int& weakness)
    {
        const std::string className = CombatClassSystem::normalizeClassText(target.getType());

        if (containsAny(className, {"pyromancien", "mage flame"}))
        {
            if (elementId == "fire") resistance += 25;
            if (elementId == "frost") weakness += 8;
        }

        if (containsAny(className, {"mage", "arcaniste", "sorcier", "ensorceleur", "occultiste"}))
        {
            if (elementId == "shock" || elementId == "frost") resistance += 8;
        }

        if (containsAny(className, {"assassin", "ombrelame", "voleur"}))
        {
            if (elementId == "poison") resistance += 12;
            if (elementId == "bleeding") resistance += 6;
        }

        if (containsAny(className, {"gardien", "colosse", "tank", "paladin", "templier"}))
        {
            if (elementId == "bleeding") resistance += 15;
            if (elementId == "frost") resistance += 6;
            if (elementId == "shock") weakness += 6;
        }

        if (containsAny(className, {"clerc", "prêtre", "pretre", "druide"}))
        {
            if (elementId == "poison" || elementId == "fire") resistance += 10;
        }
    }

    void addEquipmentAffinity(const Player& player, const std::string& elementId, int& resistance, int& weakness)
    {
        std::string equipmentText;
        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();
            equipmentText += " " + weapon.getName() + " " + weapon.getDescription() + " " + weapon.getEnchantmentSummaryText();
        }
        if (player.hasEquippedArmor())
        {
            Armor armor = player.getEquippedArmor();
            equipmentText += " " + armor.getName() + " " + armor.getDescription() + " " + armor.getEnchantmentSummaryText();
        }

        if (equipmentText.empty()) return;

        if (containsAny(equipmentText, {"fer", "acier", "métal", "metal", "plaques", "lourde", "conducteur"}))
        {
            if (elementId == "shock") weakness += 18;
            if (elementId == "bleeding") resistance += 10;
            if (elementId == "frost") resistance += 6;
        }

        if (containsAny(equipmentText, {"cuir", "peau", "fourrure"}))
        {
            if (elementId == "shock") resistance += 8;
            if (elementId == "fire") weakness += 8;
        }

        if (containsAny(equipmentText, {"robe", "tissu", "lin", "soie"}))
        {
            if (elementId == "shock") resistance += 10;
            if (elementId == "fire") weakness += 12;
            if (elementId == "bleeding") weakness += 5;
        }

        if (containsAny(equipmentText, {"draconique", "écaille", "ecaille"}))
        {
            if (elementId == "fire" || elementId == "frost") resistance += 18;
        }

        if (containsAny(equipmentText, {"ignifug", "anti-chaleur", "braises", "voile anti-chaleur", "rune anti-feu", "rune thermique", "charme d\'équilibre thermique"}))
        {
            if (elementId == "fire") resistance += 26;
        }

        if (containsAny(equipmentText, {"parka", "glaciale", "isolant", "anti-givre", "thermique", "rune anti-froid", "rune thermique", "charme d\'équilibre thermique"}))
        {
            if (elementId == "frost") resistance += 24;
            if (elementId == "fire" && containsAny(equipmentText, {"isolant", "thermique"})) resistance += 6;
        }
    }

    void addMonsterAffinity(const Monster& monster, const std::string& elementId, int& resistance, int& weakness)
    {
        const std::string raceText = monster.getRaceText();
        const std::string combined = monster.getName() + " " + monster.getType() + " " + raceText;

        if (containsAny(raceText, {"slime"}))
        {
            if (elementId == "poison" || elementId == "bleeding") resistance += 45;
            if (elementId == "shock") weakness += 16;
            if (elementId == "frost") weakness += 8;
        }

        if (containsAny(raceText, {"plante"}))
        {
            if (elementId == "fire") weakness += 28;
            if (elementId == "poison" || elementId == "bleeding") resistance += 24;
            if (elementId == "frost") resistance += 8;
        }

        if (containsAny(raceText, {"mort-vivant"}))
        {
            if (elementId == "poison" || elementId == "bleeding") resistance += 50;
            if (elementId == "frost") resistance += 15;
            if (elementId == "fire") weakness += 16;
        }

        if (containsAny(raceText, {"bête", "bete", "insectoïde", "insectoide"}))
        {
            if (elementId == "bleeding" || elementId == "poison") weakness += 8;
            if (elementId == "fire") weakness += 5;
        }

        if (containsAny(raceText, {"dragon", "draconide"}))
        {
            if (elementId == "fire" || elementId == "frost" || elementId == "shock") resistance += 25;
            if (elementId == "poison") resistance += 12;
        }

        if (containsAny(raceText, {"construction", "golem"}))
        {
            if (elementId == "poison" || elementId == "bleeding") resistance += 60;
            if (elementId == "shock") weakness += 12;
        }

        if (containsAny(combined, {"braise", "cendre", "feu", "brûl", "brul", "rouge", "chaud"}))
        {
            if (elementId == "fire") resistance += 45;
            if (elementId == "frost") weakness += 18;
        }

        if (containsAny(combined, {"givre", "gel", "glace", "froid", "bleu"}))
        {
            if (elementId == "frost") resistance += 45;
            if (elementId == "fire") weakness += 18;
        }

        if (containsAny(combined, {"élect", "elect", "orage", "conduct", "vibrant"}))
        {
            if (elementId == "shock") resistance += 45;
        }

        if (monster.isElite())
        {
            resistance += 4;
        }
    }

    std::string elementToText(const std::string& elementId)
    {
        if (elementId == "fire") return "au feu";
        if (elementId == "poison") return "au poison";
        if (elementId == "frost") return "au froid";
        if (elementId == "shock") return "à l'électricité";
        if (elementId == "bleeding") return "au saignement";
        return "à cet effet";
    }

    void printAffinityIfChanged(const Entity& target, const std::string& elementId, int modifier)
    {
        std::vector<std::string> lines;

        if (modifier <= 85)
        {
            lines.push_back("Affinité élémentaire : " + target.getName() + " résiste " + elementToText(elementId) + ".");
            lines.push_back("L'effet est réduit.");
            if (target.hasElementalWard())
            {
                lines.push_back("Le voile élémentaire absorbe une partie de l'altération.");
            }
        }
        else if (modifier >= 115)
        {
            lines.push_back("Affinité élémentaire : " + target.getName() + " est vulnérable " + elementToText(elementId) + ".");
            lines.push_back("L'effet mord plus fort.");
        }

        if (!lines.empty())
        {
            MessageScreen::show("AFFINITÉ ÉLÉMENTAIRE", "combat.elemental_affinity.modifier", lines, false);
        }
    }

    int scaleTurnsByModifier(int turns, int modifier)
    {
        if (turns <= 0) return 0;
        int result = turns;

        if (modifier <= 55) result -= 2;
        else if (modifier <= 82) result -= 1;
        else if (modifier >= 140) result += 2;
        else if (modifier >= 118) result += 1;

        if (result < 0) result = 0;
        return result;
    }
}

int ElementalAffinitySystem::getElementalModifierPercent(const Entity& target, const std::string& elementId)
{
    int resistance = 0;
    int weakness = 0;

    addClassAffinity(target, elementId, resistance, weakness);

    const Player* player = dynamic_cast<const Player*>(&target);
    if (player != nullptr)
    {
        addRaceAffinity(*player, elementId, resistance, weakness);
        addEquipmentAffinity(*player, elementId, resistance, weakness);
    }

    const Monster* monster = dynamic_cast<const Monster*>(&target);
    if (monster != nullptr)
    {
        addMonsterAffinity(*monster, elementId, resistance, weakness);
    }

    int modifier = 100 + weakness - resistance;
    if (target.hasElementalWard())
    {
        modifier -= target.getElementalWardResistancePercent();
    }
    if (modifier < 25) modifier = 25;
    if (modifier > 175) modifier = 175;
    return modifier;
}

std::string ElementalAffinitySystem::getAffinitySummary(const Entity& target, const std::string& elementId)
{
    int modifier = getElementalModifierPercent(target, elementId);
    if (modifier <= 85) return "résistance " + elementToText(elementId);
    if (modifier >= 115) return "faiblesse " + elementToText(elementId);
    return "affinité neutre";
}

void ElementalAffinitySystem::applyDamageStatus(Entity& target, const std::string& elementId, int turns, int damage)
{
    if (turns <= 0 || damage <= 0) return;

    int modifier = getElementalModifierPercent(target, elementId);
    int finalTurns = scaleTurnsByModifier(turns, modifier);
    int finalDamage = std::max(1, damage * modifier / 100);

    printAffinityIfChanged(target, elementId, modifier);

    if (finalTurns <= 0)
    {
        MessageScreen::show(
            "ALTÉRATION ABSORBÉE",
            "combat.elemental_affinity.damage_status.negated",
            {target.getName() + " encaisse l'effet sans le laisser s'installer."},
            false
        );
        return;
    }

    if (elementId == "fire") target.applyBurning(finalTurns, finalDamage);
    else if (elementId == "poison") target.applyPoison(finalTurns, finalDamage);
    else if (elementId == "bleeding") target.applyBleeding(finalTurns, finalDamage);
}

void ElementalAffinitySystem::applyTurnStatus(Entity& target, const std::string& elementId, int turns)
{
    if (turns <= 0) return;

    int modifier = getElementalModifierPercent(target, elementId);
    int finalTurns = scaleTurnsByModifier(turns, modifier);

    printAffinityIfChanged(target, elementId, modifier);

    if (finalTurns <= 0)
    {
        MessageScreen::show(
            "ALTÉRATION SECOUÉE",
            "combat.elemental_affinity.turn_status.negated",
            {target.getName() + " secoue l'effet avant qu'il ne prenne vraiment."},
            false
        );
        return;
    }

    if (elementId == "frost") target.applyFrost(finalTurns);
    else if (elementId == "shock") target.applyShock(finalTurns);
}

void ElementalAffinitySystem::applyBurning(Entity& target, int turns, int damage)
{
    applyDamageStatus(target, "fire", turns, damage);
}

void ElementalAffinitySystem::applyPoison(Entity& target, int turns, int damage)
{
    applyDamageStatus(target, "poison", turns, damage);
}

void ElementalAffinitySystem::applyFrost(Entity& target, int turns)
{
    applyTurnStatus(target, "frost", turns);
}

void ElementalAffinitySystem::applyShock(Entity& target, int turns)
{
    applyTurnStatus(target, "shock", turns);
}

void ElementalAffinitySystem::applyBleeding(Entity& target, int turns, int damage)
{
    applyDamageStatus(target, "bleeding", turns, damage);
}
