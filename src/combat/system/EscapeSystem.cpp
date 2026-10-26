// EN: EscapeSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EscapeSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/EscapeSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "progression/DifficultyRules.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include "item/inventory/CarryLoadSystem.hpp"

#include <algorithm>
#include <cctype>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::string normalizeEscapeText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool escapeTextContainsAny(const std::string& value, std::initializer_list<const char*> terms)
    {
        const std::string normalized = normalizeEscapeText(value);
        for (const char* term : terms)
        {
            if (normalized.find(normalizeEscapeText(term)) != std::string::npos) return true;
        }
        return false;
    }
}

int EscapeSystem::escapeOpeningsRequired(int opponentCount)
{
    opponentCount = std::max(1, opponentCount);
    if (opponentCount <= 3) return 1;

    // Rounded to nearest for opponentCount / 3.0, with .5 and above rounded up.
    // 5 opponents therefore require two successive openings, 8 require three, etc.
    return std::max(1, (opponentCount + 1) / 3);
}

bool EscapeSystem::rollEscapeAgainstOpposition(Random& random, int perOpeningChancePercent, int opponentCount)
{
    const int chance = std::clamp(perOpeningChancePercent, 1, 99);
    const int required = escapeOpeningsRequired(opponentCount);
    for (int attempt = 0; attempt < required; ++attempt)
    {
        if (random.between(1, 100) > chance) return false;
    }
    return true;
}

int EscapeSystem::monsterEscapeMobilityModifier(const Monster& monster)
{
    int modifier = 0;

    switch (monster.getRace())
    {
        case Race::Fee: modifier += 20; break;
        case Race::Gobelin: modifier += 11; break;
        case Race::Gnome:
        case Race::Halfelin: modifier += 9; break;
        case Race::Kitsune: modifier += 10; break;
        case Race::Elfe:
        case Race::ElfeNoir: modifier += 6; break;
        case Race::Insectoide: modifier += 9; break;
        case Race::Bete: modifier += 5; break;
        case Race::Esprit: modifier += 7; break;
        case Race::Nain: modifier -= 4; break;
        case Race::Orc: modifier -= 7; break;
        case Race::SemiDragon: modifier -= 7; break;
        case Race::Draconide: modifier -= 10; break;
        case Race::Dragon: modifier -= 18; break;
        case Race::Construction: modifier -= 18; break;
        case Race::Plante: modifier -= 14; break;
        case Race::Slime: modifier -= 5; break;
        default: break;
    }

    const std::string profileText = monster.getName() + " " + monster.getType() + " " + monster.getRaceText();
    if (escapeTextContainsAny(profileText, {
            "petit", "petite", "minuscule", "nerveux", "agile", "rapide", "léger", "legere",
            "fée", "fee", "rat", "chauve-souris", "insecte", "moustique", "kobold"}))
    {
        modifier += 8;
    }
    if (escapeTextContainsAny(profileText, {
            "massif", "massive", "lourd", "lourde", "colosse", "golem", "gardien", "tank",
            "brute", "géant", "geant", "sentinelle", "cuirassé", "cuirasse"}))
    {
        modifier -= 12;
    }

    return std::clamp(modifier, -25, 25);
}

MonsterEscapeAttemptResult EscapeSystem::attemptMonsterEscapeStep(
    Monster& monster,
    Random& random,
    int opponentCount,
    int baseChancePercent,
    bool requireLowHpEligibility)
{
    MonsterEscapeAttemptResult result;
    if (requireLowHpEligibility && !monsterCanAttemptEscape(monster))
    {
        return result;
    }
    if (monster.isInvocation())
    {
        return result;
    }

    opponentCount = std::max(1, opponentCount);
    result.requiredOpenings = escapeOpeningsRequired(opponentCount);
    result.progressBefore = std::clamp(monster.getEscapeOpeningProgress(), 0, result.requiredOpenings);
    result.progressAfter = result.progressBefore;
    result.baseChancePercent = std::clamp(baseChancePercent, 1, 99);
    result.mobilityModifierPercent = monsterEscapeMobilityModifier(monster);
    result.retryBonusPercent = std::min(36, result.progressBefore * 18);
    result.effectiveChancePercent = std::clamp(
        result.baseChancePercent + result.mobilityModifierPercent + result.retryBonusPercent, 5, 92);

    // If the opposition shrank after the monster already created enough distance, the next committed
    // escape action completes the retreat immediately instead of asking it to recreate old openings.
    if (result.progressBefore >= result.requiredOpenings)
    {
        result.state = MonsterEscapeAttemptState::Escaped;
        return result;
    }

    result.roll = random.between(1, 100);
    if (result.roll > result.effectiveChancePercent)
    {
        result.state = MonsterEscapeAttemptState::Blocked;
        return result;
    }

    monster.recordSuccessfulEscapeOpening();
    result.progressAfter = std::min(result.requiredOpenings, monster.getEscapeOpeningProgress());
    result.state = result.progressAfter >= result.requiredOpenings
        ? MonsterEscapeAttemptState::Escaped
        : MonsterEscapeAttemptState::Progressed;
    return result;
}

// EN: playerAttemptsEscape declares or implements a focused behavior used by this module.
// FR: playerAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
int EscapeSystem::playerEscapeMobilityModifier(const Player& player)
{
    switch (player.getRace())
    {
        case CharacterRace::Fairy: return 12;
        case CharacterRace::Kitsune: return 8;
        case CharacterRace::Gnome:
        case CharacterRace::Halfling: return 7;
        case CharacterRace::Elf:
        case CharacterRace::DarkElf: return 5;
        case CharacterRace::SemiFox:
        case CharacterRace::SemiCat:
        case CharacterRace::SemiBird: return 6;
        case CharacterRace::SemiWolf:
        case CharacterRace::SemiDog: return 3;
        case CharacterRace::Dwarf: return -4;
        case CharacterRace::Orc: return -7;
        case CharacterRace::HalfDragon: return -8;
        case CharacterRace::Demon: return -2;
        default: return 0;
    }
}

MonsterEscapeAttemptResult EscapeSystem::attemptPlayerEscapeStep(
    Player& player,
    Random& random,
    DifficultyMode difficulty,
    int opponentCount)
{
    MonsterEscapeAttemptResult result;
    opponentCount = std::max(1, opponentCount);
    result.requiredOpenings = escapeOpeningsRequired(opponentCount);
    result.progressBefore = std::clamp(player.getEscapeOpeningProgress(), 0, result.requiredOpenings);
    result.progressAfter = result.progressBefore;

    int equipmentEscapeModifier = 0;
    if (player.hasEquippedWeapon())
    {
        const Weapon weapon = player.getEquippedWeapon();
        if (!weapon.isBroken()) equipmentEscapeModifier += EquipmentWeightRules::getWeaponEscapeModifier(weapon);
    }
    if (player.hasEquippedArmor())
    {
        const Armor armor = player.getEquippedArmor();
        if (!armor.isBroken())
        {
            equipmentEscapeModifier += EquipmentWeightRules::getArmorEscapeModifier(armor);
            equipmentEscapeModifier += CombatClassSystem::getArmorHandlingEscapeAdjustment(player, armor.getType(), armor.getName());
        }
    }

    result.baseChancePercent = std::clamp(
        CombatClassSystem::getBaseEscapeChance(player)
            + DifficultyRules::getPlayerEscapeChanceModifier(difficulty)
            + equipmentEscapeModifier,
        10, 90);
    result.mobilityModifierPercent = playerEscapeMobilityModifier(player);
    result.carryLoadModifierPercent = CarryLoadSystem::escapeModifierPercent(player);
    result.retryBonusPercent = std::min(36, result.progressBefore * 18);
    result.effectiveChancePercent = std::clamp(
        result.baseChancePercent + result.mobilityModifierPercent + result.carryLoadModifierPercent + result.retryBonusPercent,
        8, 92);

    // The player has already created enough distance and the opposition became smaller. A new
    // committed escape action converts that existing advantage into a full retreat.
    if (result.progressBefore >= result.requiredOpenings)
    {
        result.state = MonsterEscapeAttemptState::Escaped;
        return result;
    }

    result.roll = random.between(1, 100);
    if (result.roll > result.effectiveChancePercent)
    {
        result.state = MonsterEscapeAttemptState::Blocked;
        return result;
    }

    player.recordSuccessfulEscapeOpening();
    result.progressAfter = std::min(result.requiredOpenings, player.getEscapeOpeningProgress());
    result.state = result.progressAfter >= result.requiredOpenings
        ? MonsterEscapeAttemptState::Escaped
        : MonsterEscapeAttemptState::Progressed;
    return result;
}

bool EscapeSystem::playerAttemptsEscape(Player& player, Random& random)
{
    return playerAttemptsEscape(player, random, DifficultyMode::Normal);
}

bool EscapeSystem::playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty)
{
    return playerAttemptsEscape(player, random, difficulty, 1);
}

bool EscapeSystem::playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty, int enemyCount)
{
    enemyCount = std::max(1, enemyCount);
    const MonsterEscapeAttemptResult result = attemptPlayerEscapeStep(player, random, difficulty, enemyCount);

    std::vector<std::string> lines;
    lines.push_back(player.getName() + " engage son tour pour chercher une ouverture de fuite...");
    lines.push_back("Adversaires encore capables de bloquer la fuite : " + std::to_string(enemyCount));
    lines.push_back("Progression : " + std::to_string(result.progressBefore) + "/" + std::to_string(result.requiredOpenings) + " ouverture(s) avant cette tentative.");
    lines.push_back("Chance de base : " + std::to_string(result.baseChancePercent) + "%.");
    if (result.mobilityModifierPercent != 0)
    {
        lines.push_back("Mobilité / gabarit racial : "
            + std::string(result.mobilityModifierPercent > 0 ? "+" : "")
            + std::to_string(result.mobilityModifierPercent) + "%.");
    }
    if (result.carryLoadModifierPercent != 0)
    {
        const CarryLoadSummary load = CarryLoadSystem::evaluate(player);
        lines.push_back("Encombrement : " + load.label + " — "
            + std::to_string(result.carryLoadModifierPercent) + "% sur la fuite. Le coffre, la vente ou le dépôt peuvent alléger le sac.");
    }
    if (result.retryBonusPercent > 0)
    {
        lines.push_back("Distance déjà créée : +" + std::to_string(result.retryBonusPercent) + "% sur cette ouverture.");
    }
    lines.push_back("Chance effective de cette ouverture : " + std::to_string(result.effectiveChancePercent) + "%.");

    if (result.state == MonsterEscapeAttemptState::Escaped)
    {
        lines.push_back("Résultat : fuite réussie.");
        lines.push_back(player.getName() + " transforme l'ouverture acquise en vraie sortie et quitte l'affrontement.");
        player.resetEscapeOpeningProgress();
        MessageScreen::show("FUITE RÉUSSIE", "combat.escape.result.success", lines);
        return true;
    }

    if (result.state == MonsterEscapeAttemptState::Progressed)
    {
        lines.push_back("Ouverture réussie : tu crées de la distance, mais tu n'es pas encore hors de portée.");
        lines.push_back("Cette avance est conservée pour une prochaine tentative dans ce combat et augmente sa probabilité de réussite.");
        lines.push_back("Le tour est consommé.");
        MessageScreen::show("DÉCROCHAGE", "combat.escape.result.progress", lines);
        return false;
    }

    lines.push_back("Ouverture bloquée : la fuite ne progresse pas.");
    if (result.progressBefore > 0)
    {
        lines.push_back("La distance déjà créée n'est pas effacée automatiquement par ce seul échec.");
    }
    lines.push_back("Le tour est consommé.");
    MessageScreen::show("FUITE ÉCHOUÉE", "combat.escape.result.failed", lines);
    return false;
}

// EN: playerAttemptsBossEscape declares or implements a focused behavior used by this module.
// FR: playerAttemptsBossEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::playerAttemptsBossEscape(const Player& player, const Boss& boss)
{
    MessageScreen::show(
        "FUITE IMPOSSIBLE",
        "combat.escape.boss.blocked",
        {
            player.getName() + " cherche une issue...",
            "L'air devient lourd.",
            "L'arène semble se refermer comme une cage.",
            boss.getName() + " bloque toute échappatoire par sa seule présence.",
            "L'arène ne possède plus de sortie : elle n'attend qu'un vainqueur, ou un corps.",
            "La tentative échoue. Le tour est perdu."
        }
    );

    return false;
}

bool EscapeSystem::playerAttemptsDuelEscape(
    Player& runner,
    Entity& opponent,
    Random& random
)
{
    int escapeChance = calculateDuelEscapeChance(runner, opponent);
    int roll = random.between(1, 100);
    std::vector<std::string> lines;

    lines.push_back(runner.getName() + " cherche une ouverture pour quitter le duel...");
    lines.push_back("En duel, fuir revient à abandonner le combat.");
    lines.push_back("Chance estimée : " + std::to_string(escapeChance) + "%");

    if (roll <= escapeChance)
    {
        lines.push_back("Résultat : fuite réussie.");
        lines.push_back(runner.getName() + " sort de l'arène avant de se faire achever.");
        lines.push_back(opponent.getName() + " remporte l'affrontement.");
        lines.push_back("Répartition prévue des récompenses :");
        lines.push_back("- " + runner.getName() + " : 25%");
        lines.push_back("- " + opponent.getName() + " : 75%");

        MessageScreen::show("ABANDON DU DUEL", "combat.escape.duel.success", lines);
        endCombatBySurrender(runner);
        return true;
    }

    lines.push_back("Résultat : fuite échouée.");
    lines.push_back(opponent.getName() + " coupe la trajectoire de " + runner.getName() + ".");
    lines.push_back("Le duel continue, mais le tour est perdu.");

    MessageScreen::show("FUITE DE DUEL ÉCHOUÉE", "combat.escape.duel.failed", lines);
    return false;
}

// EN: monsterCanAttemptEscape declares or implements a focused behavior used by this module.
// FR: monsterCanAttemptEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::monsterCanAttemptEscape(const Monster& monster)
{
    if (monster.isInvocation())
    {
        return false;
    }

    if (monster.isElite())
    {
        return false;
    }

    if (monster.getHealingPotionCount() > 0)
    {
        return false;
    }

    if (monster.getMaxHp() <= 0)
    {
        return false;
    }

    int hpPercentage = monster.getHp() * 100 / monster.getMaxHp();

    return hpPercentage <= 10;
}

// EN: monsterAttemptsEscape declares or implements a focused behavior used by this module.
// FR: monsterAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::monsterAttemptsEscape(Monster& monster, Random& random)
{
    return monsterAttemptsEscape(monster, random, 1);
}

bool EscapeSystem::monsterAttemptsEscape(Monster& monster, Random& random, int opponentCount)
{
    const MonsterEscapeAttemptResult result = attemptMonsterEscapeStep(
        monster, random, opponentCount, LOW_MONSTER_ESCAPE_CHANCE, true);
    if (result.state == MonsterEscapeAttemptState::NotEligible) return false;

    std::vector<std::string> lines = {
        monster.getName() + " engage réellement son tour pour décrocher du combat.",
        "Adversaires encore capables de fermer la sortie : " + std::to_string(std::max(1, opponentCount)) + ".",
        "Progression de fuite : " + std::to_string(result.progressBefore) + "/" + std::to_string(result.requiredOpenings) + " avant cette tentative.",
        "Mobilité du profil : " + std::string(result.mobilityModifierPercent >= 0 ? "+" : "") + std::to_string(result.mobilityModifierPercent) + "%.",
        "Bonus de décrochage déjà acquis : +" + std::to_string(result.retryBonusPercent) + "%.",
        "Chance de cette ouverture : " + std::to_string(result.effectiveChancePercent) + "%."
    };

    if (result.state == MonsterEscapeAttemptState::Escaped)
    {
        lines.push_back(monster.getName() + " transforme l'ouverture en vraie sortie et quitte l'affrontement.");
        MessageScreen::show("ENNEMI EN FUITE", "combat.escape.monster.success", lines);
        return true;
    }

    if (result.state == MonsterEscapeAttemptState::Progressed)
    {
        lines.push_back("Ouverture réussie : la créature crée de la distance sans être encore hors de portée.");
        lines.push_back("Une prochaine tentative bénéficiera de cette avance au lieu de repartir de zéro.");
        MessageScreen::show("DÉCROCHAGE ENNEMI", "combat.escape.monster.progress", lines);
        return false;
    }

    lines.push_back("Ouverture bloquée : la fuite ne progresse pas et le tour est consommé.");
    MessageScreen::show("FUITE ENNEMIE BLOQUÉE", "combat.escape.monster.blocked", lines);
    return false;
}

int EscapeSystem::calculateDuelEscapeChance(
    const Player& runner,
    const Entity& opponent
)
{
    int chance = CombatClassSystem::getBaseEscapeChance(runner);

    if (runner.hasEquippedWeapon())
    {
        const Weapon weapon = runner.getEquippedWeapon();
        if (!weapon.isBroken())
        {
            chance += EquipmentWeightRules::getWeaponEscapeModifier(weapon);
        }
    }
    if (runner.hasEquippedArmor())
    {
        const Armor armor = runner.getEquippedArmor();
        if (!armor.isBroken())
        {
            chance += EquipmentWeightRules::getArmorEscapeModifier(armor);
            chance += CombatClassSystem::getArmorHandlingEscapeAdjustment(runner, armor.getType(), armor.getName());
        }
    }

    const Player* opposingPlayer = dynamic_cast<const Player*>(&opponent);

    if (opposingPlayer != nullptr)
    {
        int ecartNiveau = opposingPlayer->getLevel() - runner.getLevel();
        chance -= ecartNiveau * 5;
    }

    int chanceAdversaire = CombatClassSystem::getBaseEscapeChance(opponent);

    if (chanceAdversaire <= 35)
    {
        chance -= 10;
    }
    else if (chanceAdversaire >= 65)
    {
        chance += 5;
    }

    return std::max(15, std::min(chance, 85));
}

// EN: endCombatBySurrender declares or implements a focused behavior used by this module.
// FR: endCombatBySurrender déclare ou implémente un comportement précis utilisé par ce module.
void EscapeSystem::endCombatBySurrender(Player& runner)
{
    runner.takeDamage(runner.getHp());
}
