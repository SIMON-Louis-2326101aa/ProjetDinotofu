#include "story/HestiaCompanionSystem.hpp"

#include "class_system/ClassCatalog.hpp"
#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "story/HestiaArcSystem.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
    std::string lowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    std::string latestConfigLabel(const Player& player)
    {
        std::string label;
        int latestDay = -1;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "hestia_companion_config" || record.key != "loadout") continue;
            if (record.lastDay >= latestDay)
            {
                latestDay = record.lastDay;
                label = record.label;
            }
        }
        return label;
    }

    std::string fieldValue(const std::string& label, const std::string& key, const std::string& fallback)
    {
        const std::string token = key + "=";
        const std::size_t pos = label.find(token);
        if (pos == std::string::npos) return fallback;
        const std::size_t start = pos + token.size();
        const std::size_t end = label.find(" | ", start);
        return label.substr(start, end == std::string::npos ? std::string::npos : end - start);
    }

    int categoryCount(const Player& player, const std::string& category, const std::string& key)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
            if (record.category == category && record.key == key) total += std::max(0, record.count);
        return total;
    }

    void writeLoadout(Player& player, const std::string& className, const std::string& weaponStyle, const std::string& armorStyle)
    {
        player.recordCanonicalEvent(
            "hestia_companion_config",
            "loadout",
            "class=" + className + " | weapon=" + weaponStyle + " | armor=" + armorStyle,
            1);
    }

    bool valueIn(const std::string& value, const std::vector<std::string>& choices)
    {
        return std::find(choices.begin(), choices.end(), value) != choices.end();
    }

    int weaponQualityFromStyle(const std::string& style)
    {
        const std::string p = lowerCopy(style);
        if (p.find("bâton") != std::string::npos || p.find("baton") != std::string::npos || p.find("catal") != std::string::npos) return 2;
        if (p.find("arc") != std::string::npos || p.find("arbal") != std::string::npos) return 2;
        if (p.find("épée") != std::string::npos || p.find("epee") != std::string::npos || p.find("lance") != std::string::npos) return 2;
        if (p.find("aucun") != std::string::npos || p.find("mains") != std::string::npos) return 1;
        return 1;
    }

    int armorQualityFromStyle(const std::string& style)
    {
        const std::string p = lowerCopy(style);
        if (p.find("lourde") != std::string::npos) return 3;
        if (p.find("interm") != std::string::npos) return 2;
        if (p.find("lég") != std::string::npos || p.find("leg") != std::string::npos) return 1;
        return 1;
    }
}

HestiaCompanionProfile HestiaCompanionSystem::profile(const Player& player)
{
    HestiaCompanionProfile out;
    const PlayerHestiaStoryState& h = player.getHestiaStoryState();
    out.available = h.rescued && h.materialized;
    out.activeInParty = out.available && h.residence == "active_party";
    out.awakeningStage = std::clamp(h.awakeningStage, 0, 3);

    const std::string config = latestConfigLabel(player);
    out.className = fieldValue(config, "class", "Indéfinie");
    out.weaponStyle = fieldValue(config, "weapon", "Aucun");
    out.armorStyle = fieldValue(config, "armor", "Aucune");
    out.practiceSessions = categoryCount(player, "hestia_companion_training", "practice");
    out.combatParticipations = categoryCount(player, "hestia_companion_training", "combat");

    const int adaptation = std::min(5, out.practiceSessions / 4 + out.combatParticipations / 12);
    out.generalAptitude = -4 + std::min(4, adaptation);
    out.magicAptitude = HestiaArcSystem::combatAptitudeScore(player, "magie") + std::min(8, out.practiceSessions / 3);
    out.divineLightAptitude = HestiaArcSystem::combatAptitudeScore(player, "magie divine lumière") + std::min(10, out.practiceSessions / 2);

    int mishap = 46 - out.practiceSessions * 2 - out.combatParticipations / 3 - out.awakeningStage * 8 - h.trust / 10;
    if (looksMagicOriented(out) && out.awakeningStage == 0) mishap += 4;
    out.mishapChancePercent = std::clamp(mishap, 4, 50);
    return out;
}

bool HestiaCompanionSystem::isActive(const Player& player)
{
    return profile(player).activeInParty;
}

std::vector<std::string> HestiaCompanionSystem::classChoices()
{
    return ClassCatalog::getPlayableClassNames();
}

std::vector<std::string> HestiaCompanionSystem::weaponStyleChoices()
{
    return {"Mains nues", "Épée", "Dague", "Lance", "Arc", "Arbalète", "Bâton / catalyseur", "Marteau", "Hache"};
}

std::vector<std::string> HestiaCompanionSystem::armorStyleChoices()
{
    return {"Tenue légère", "Protection intermédiaire", "Protection lourde"};
}

bool HestiaCompanionSystem::chooseClass(Player& player, const std::string& className)
{
    if (!player.getHestiaStoryState().rescued || !player.getHestiaStoryState().materialized) return false;
    if (!valueIn(className, classChoices())) return false;
    HestiaCompanionProfile current = profile(player);
    writeLoadout(player, className, current.weaponStyle, current.armorStyle);
    player.recordCanonicalEvent("hestia_companion_choices", "class", "Classe actuelle choisie pour Hestia : " + className, 1);
    return true;
}

bool HestiaCompanionSystem::chooseWeaponStyle(Player& player, const std::string& weaponStyle)
{
    if (!player.getHestiaStoryState().rescued || !player.getHestiaStoryState().materialized) return false;
    if (!valueIn(weaponStyle, weaponStyleChoices())) return false;
    HestiaCompanionProfile current = profile(player);
    writeLoadout(player, current.className, weaponStyle, current.armorStyle);
    player.recordCanonicalEvent("hestia_companion_choices", "weapon", "Style d'arme actuel d'Hestia : " + weaponStyle, 1);
    return true;
}

bool HestiaCompanionSystem::chooseArmorStyle(Player& player, const std::string& armorStyle)
{
    if (!player.getHestiaStoryState().rescued || !player.getHestiaStoryState().materialized) return false;
    if (!valueIn(armorStyle, armorStyleChoices())) return false;
    HestiaCompanionProfile current = profile(player);
    writeLoadout(player, current.className, current.weaponStyle, armorStyle);
    player.recordCanonicalEvent("hestia_companion_choices", "armor", "Protection actuelle d'Hestia : " + armorStyle, 1);
    return true;
}

bool HestiaCompanionSystem::practice(Player& player)
{
    const PlayerHestiaStoryState& h = player.getHestiaStoryState();
    if (!h.rescued || !h.materialized) return false;
    player.recordCanonicalEvent("hestia_companion_training", "practice", "Séance d'apprentissage volontaire avec Hestia", 1);
    return true;
}

void HestiaCompanionSystem::recordCombatParticipation(Player& player)
{
    if (!isActive(player)) return;
    player.recordCanonicalEvent("hestia_companion_training", "combat", "Hestia a réellement participé à un combat", 1);
}

bool HestiaCompanionSystem::looksMagicOriented(const HestiaCompanionProfile& value)
{
    const std::string p = lowerCopy(value.className + " " + value.weaponStyle);
    return p.find("mage") != std::string::npos
        || p.find("mancien") != std::string::npos
        || p.find("sorc") != std::string::npos
        || p.find("prêtre") != std::string::npos
        || p.find("pretre") != std::string::npos
        || p.find("clerc") != std::string::npos
        || p.find("paladin") != std::string::npos
        || p.find("bâton") != std::string::npos
        || p.find("baton") != std::string::npos
        || p.find("catal") != std::string::npos;
}

RecruitedAllyCombatSupport HestiaCompanionSystem::toCombatSupport(const Player& player)
{
    const HestiaCompanionProfile p = profile(player);
    RecruitedAllyCombatSupport ally;
    ally.name = "Hestia";
    ally.race = "Humaine";
    ally.job = p.className == "Indéfinie" ? "Apprentie indéfinie" : p.className;
    ally.trait = p.awakeningStage > 0
        ? "Compagne spéciale | maladroite | lumière devenue anormale"
        : "Compagne spéciale | maladroite | apprentissage irrégulier";
    ally.motivation = "Aider son sauveur même quand elle ne sait pas encore comment";
    ally.recruitedLevel = 1;
    ally.estimatedLevel = std::max(1, 1 + p.practiceSessions / 3 + p.combatParticipations / 5 + p.awakeningStage * 8);
    ally.originRankIndex = 0;
    ally.currentRankIndex = std::clamp(p.awakeningStage, 0, 3);
    ally.requestedShare = 0;
    ally.normalizedShare = 0;
    ally.maxHp = std::max(28, 36 + ally.estimatedLevel * 3 + armorQualityFromStyle(p.armorStyle) * 9);
    ally.currentHp = ally.maxHp;
    ally.healingPotionCharges = 0;
    ally.weaponQuality = weaponQualityFromStyle(p.weaponStyle);
    ally.armorQuality = armorQualityFromStyle(p.armorStyle);
    ally.supportKitQuality = looksMagicOriented(p) && p.awakeningStage > 0
        ? std::max(1, 1 + p.awakeningStage * 2)
        : 0;
    ally.fatiguePercent = 0;
    ally.moralePercent = std::clamp(45 + player.getHestiaStoryState().trust / 2, 45, 95);
    ally.readinessPercent = std::clamp(28 + p.practiceSessions * 3 + p.combatParticipations + p.awakeningStage * 15, 20, 100);
    ally.teamOrigin = AllyTeamOrigin::PersonalRecruit;
    ally.persistentMemberId = "special:hestia";
    ally.temporaryNoReward = true;
    return ally;
}

HestiaCombatMishap HestiaCompanionSystem::rollMishap(const Player& player, Random& random, bool magicalAction)
{
    HestiaCombatMishap out;
    const HestiaCompanionProfile p = profile(player);
    if (!p.activeInParty) return out;
    int chance = p.mishapChancePercent;
    if (magicalAction && p.awakeningStage == 0) chance = std::min(58, chance + 7);
    if (random.between(1, 100) > chance) return out;

    out.occurred = true;
    const int roll = random.between(1, 3);
    if (roll == 1)
    {
        out.mayHitLeader = true;
        out.maxFriendlyDamagePercent = magicalAction ? 8 : 4;
        out.line = magicalAction
            ? "Hestia libère trop tôt son énergie : l'éclat part dans la mauvaise direction et frôle violemment le chef."
            : "Hestia veut aider trop vite, perd son appui et percute la ligne alliée.";
    }
    else if (roll == 2)
    {
        out.mayHitAlly = true;
        out.maxFriendlyDamagePercent = magicalAction ? 7 : 4;
        out.line = "Hestia corrige son geste une seconde trop tard : un allié doit encaisser une partie de son erreur.";
    }
    else
    {
        out.maxFriendlyDamagePercent = 0;
        out.line = "Hestia rate complètement son geste, rougit, puis fait comme si personne n'avait rien vu.";
    }
    return out;
}

HestiaAwakeningCombatResult HestiaCompanionSystem::tryAwakening(Player& player, Random& random, bool magicalAction)
{
    HestiaAwakeningCombatResult out;
    HestiaCompanionProfile p = profile(player);
    if (!p.activeInParty || !magicalAction || p.awakeningStage >= 3) return out;

    const int trust = player.getHestiaStoryState().trust;
    const int nextStage = p.awakeningStage + 1;
    int requiredPractice = nextStage == 1 ? 5 : (nextStage == 2 ? 14 : 28);
    int requiredCombats = nextStage == 1 ? 4 : (nextStage == 2 ? 16 : 36);
    int requiredTrust = nextStage == 1 ? 18 : (nextStage == 2 ? 42 : 68);
    if (p.practiceSessions < requiredPractice || p.combatParticipations < requiredCombats || trust < requiredTrust) return out;

    const int chance = nextStage == 1 ? 18 : (nextStage == 2 ? 12 : 8);
    if (random.between(1, 100) > chance) return out;
    if (!HestiaArcSystem::setAwakeningStage(player, nextStage)) return out;

    out.triggered = true;
    out.stage = nextStage;
    out.bonusDamage = nextStage == 1 ? 28 : (nextStage == 2 ? 55 : 95);
    out.returnsToHeavens = nextStage >= 3;
    out.lines.push_back("Une lumière beaucoup trop forte pour son niveau répond à Hestia sans qu'elle sache comment l'appeler.");
    if (nextStage == 1)
        out.lines.push_back("Premier éveil : un fragment de souvenir revient avec la magie. Hestia ne comprend pas encore ce qu'elle vient de faire.");
    else if (nextStage == 2)
        out.lines.push_back("Deuxième éveil : les souvenirs deviennent plus nets, et certains sont visiblement douloureux.");
    else
        out.lines.push_back("Troisième éveil : tout revient d'un seul coup. Hestia se souvient enfin de ce qu'elle est et sa matérialisation commence à quitter ce monde.");
    return out;
}

std::vector<std::string> HestiaCompanionSystem::statusLines(const Player& player)
{
    const HestiaCompanionProfile p = profile(player);
    if (!p.available) return {"Hestia n'est pas actuellement disponible."};
    std::vector<std::string> lines = {
        "Classe actuelle : " + p.className + ".",
        "Style d'arme : " + p.weaponStyle + " | protection : " + p.armorStyle + ".",
        "Apprentissage : " + std::to_string(p.practiceSessions) + " séance(s) | combats vécus : " + std::to_string(p.combatParticipations) + ".",
        "Contrôle actuel : " + std::to_string(100 - p.mishapChancePercent) + "% environ. Elle peut encore faire n'importe quoi au mauvais moment."
    };
    if (p.practiceSessions + p.combatParticipations < 6)
        lines.push_back("Il est encore trop tôt pour savoir dans quoi Hestia pourrait réellement devenir bonne.");
    else if (looksMagicOriented(p) && p.magicAptitude > p.generalAptitude + 3)
        lines.push_back("À force d'essais, sa canalisation semble progresser nettement plus vite que ses capacités physiques. Le jeu ne lui attribue pourtant aucune classe recommandée automatiquement.");
    else
        lines.push_back("Ses résultats restent irréguliers : sa spécialité doit encore être découverte par l'expérience.");

    if (p.awakeningStage > 0)
        lines.push_back("Quelque chose dans sa lumière a déjà changé " + std::to_string(p.awakeningStage) + " fois, sans explication complète.");
    else
        lines.push_back("Aucun éveil lumineux anormal n'a encore été observé.");
    return lines;
}
