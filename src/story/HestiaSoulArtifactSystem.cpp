#include "story/HestiaSoulArtifactSystem.hpp"

#include "entity/Player.hpp"
#include "item/weapon/Weapon.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalized(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }


    std::string latestArtifactEquipmentState(const Player& player)
    {
        std::string state;
        int latestDay = -1;
        int latestCount = -1;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "hestia_artefact_state" || record.key != "equipped") continue;
            if (record.lastDay > latestDay || (record.lastDay == latestDay && record.count >= latestCount))
            {
                latestDay = record.lastDay;
                latestCount = record.count;
                state = record.label;
            }
        }
        return normalized(state);
    }

    WeaponType weaponTypeForForm(const std::string& form)
    {
        const std::string value = normalized(form);
        if (value.find("dague") != std::string::npos || value.find("couteau") != std::string::npos) return WeaponType::Dagger;
        if (value.find("hache") != std::string::npos) return WeaponType::Axe;
        if (value.find("marteau") != std::string::npos || value.find("masse") != std::string::npos) return WeaponType::Hammer;
        if (value.find("lance") != std::string::npos || value.find("hallebarde") != std::string::npos) return WeaponType::Spear;
        if (value.find("bâton") != std::string::npos || value.find("baton") != std::string::npos || value.find("catal") != std::string::npos) return WeaponType::Staff;
        if (value.find("arc") != std::string::npos || value.find("arbal") != std::string::npos) return WeaponType::Bow;
        if (value.find("poing") != std::string::npos || value.find("mains") != std::string::npos || value.find("gant") != std::string::npos) return WeaponType::BareHands;
        return WeaponType::Sword;
    }

    HestiaArtifactEssenceRecord* findEssence(PlayerHestiaArtifactState& state, const std::string& familyId)
    {
        const std::string key = normalized(familyId);
        for (HestiaArtifactEssenceRecord& essence : state.absorbedEssences)
            if (normalized(essence.familyId) == key) return &essence;
        return nullptr;
    }
}

bool HestiaSoulArtifactSystem::grant(Player& player, const std::string& adaptedPrimaryForm)
{
    PlayerHestiaArtifactState& state = player.getMutableHestiaArtifactState();
    if (state.granted) return false;
    state = PlayerHestiaArtifactState{};
    state.granted = true;
    if (!adaptedPrimaryForm.empty()) state.primaryForm = adaptedPrimaryForm;
    player.recordCanonicalEvent("hestia_artefact", "granted", "Hestia a confié son artefact évolutif au joueur", 1);
    return true;
}

bool HestiaSoulArtifactSystem::setAdaptedPrimaryForm(Player& player, const std::string& form)
{
    PlayerHestiaArtifactState& state = player.getMutableHestiaArtifactState();
    if (!state.granted || form.empty()) return false;
    state.primaryForm = form;
    return true;
}

bool HestiaSoulArtifactSystem::chooseSecondForm(Player& player, const std::string& form)
{
    PlayerHestiaArtifactState& state = player.getMutableHestiaArtifactState();
    if (!state.granted || state.level < 16 || !state.secondFormUnlocked || form.empty()) return false;
    if (!state.secondaryForm.empty()) return false; // permanent choice once made
    state.secondaryForm = form;
    player.recordCanonicalEvent("hestia_artefact", "second_form", "Seconde forme choisie : " + form, 1);
    return true;
}

bool HestiaSoulArtifactSystem::switchToPrimary(Player& player)
{
    PlayerHestiaArtifactState& state = player.getMutableHestiaArtifactState();
    if (!state.granted) return false;
    state.activeForm = "primary";
    return true;
}

bool HestiaSoulArtifactSystem::switchToSecondary(Player& player)
{
    PlayerHestiaArtifactState& state = player.getMutableHestiaArtifactState();
    if (!state.granted || state.secondaryForm.empty() || state.level < 16) return false;
    state.activeForm = "secondary";
    return true;
}

std::vector<std::string> HestiaSoulArtifactSystem::weaponFormChoices()
{
    return {"Épée", "Dague", "Hache", "Marteau", "Lance", "Bâton / catalyseur", "Arc", "Arbalète", "Gantelets"};
}

bool HestiaSoulArtifactSystem::isEquipped(const Player& player)
{
    if (!player.getHestiaArtifactState().granted) return false;
    return latestArtifactEquipmentState(player) == "equipped";
}

bool HestiaSoulArtifactSystem::equip(Player& player)
{
    if (!player.getHestiaArtifactState().granted) return false;
    if (isEquipped(player)) return true;
    player.unequipWeapon();
    player.recordCanonicalEvent("hestia_artefact_state", "equipped", "equipped", 1);
    player.recordCanonicalEvent("equipements_portes", "hestia_soul_artifact", "Artefact d'Hestia", 1);
    return true;
}

bool HestiaSoulArtifactSystem::unequip(Player& player)
{
    if (!player.getHestiaArtifactState().granted || !isEquipped(player)) return false;
    player.recordCanonicalEvent("hestia_artefact_state", "equipped", "unequipped", 1);
    return true;
}

Weapon HestiaSoulArtifactSystem::buildWeapon(const Player& player)
{
    const std::string form = activeWeaponForm(player);
    const int rawDamage = std::max(1, damage(player));
    Weapon weapon(
        "Artefact d'Hestia — " + form,
        "Artefact évolutif lié au porteur. Au repos, il redevient un gant fixé à sa peau. Seuls ses propres coups fatals nourrissent sa progression.",
        0,
        weaponTypeForForm(form),
        rawDamage,
        rawDamage,
        std::max(1, rawDamage / 2),
        -1
    );
    weapon.setPersistentId("hestia_soul_artifact");
    weapon.setSoulBound(true);
    weapon.setUniqueRecoveryItem(true);
    weapon.setPersonalRecoveryItem(true);
    weapon.setRecoveryPriority(100);
    return weapon;
}

int HestiaSoulArtifactSystem::damage(const Player& player)
{
    const PlayerHestiaArtifactState& state = player.getHestiaArtifactState();
    return state.granted ? 3 * std::max(1, state.level) : 0;
}

int HestiaSoulArtifactSystem::killsRemaining(const Player& player)
{
    const PlayerHestiaArtifactState& state = player.getHestiaArtifactState();
    if (!state.granted) return 0;
    return std::max(0, state.level - state.killsTowardNextLevel);
}

std::string HestiaSoulArtifactSystem::restingForm(const Player& player)
{
    return player.getHestiaArtifactState().granted ? "gant fixé à la peau du porteur" : "absent";
}

std::string HestiaSoulArtifactSystem::activeWeaponForm(const Player& player)
{
    const PlayerHestiaArtifactState& state = player.getHestiaArtifactState();
    if (!state.granted) return "aucune";
    if (state.activeForm == "secondary" && !state.secondaryForm.empty()) return state.secondaryForm;
    return state.primaryForm;
}

std::vector<std::string> HestiaSoulArtifactSystem::statusLines(const Player& player)
{
    const PlayerHestiaArtifactState& state = player.getHestiaArtifactState();
    if (!state.granted) return {"Artefact d'Hestia : non obtenu."};
    std::vector<std::string> lines = {
        "Au repos : gant fixé à la peau du porteur.",
        "Niveau : " + std::to_string(state.level) + " | dégâts : " + std::to_string(damage(player)) + ".",
        "Âmes avant évolution : " + std::to_string(killsRemaining(player)) + ".",
        "Forme active : " + activeWeaponForm(player) + "."
    };
    if (state.level >= 16)
        lines.push_back(state.secondaryForm.empty()
            ? "Éveil niveau 16 : une seconde forme peut être choisie une seule fois."
            : "Seconde forme : " + state.secondaryForm + " | interchangeable à volonté.");
    return lines;
}

HestiaArtifactKillResult HestiaSoulArtifactSystem::recordKillingBlow(
    Player& player,
    const std::string& targetFamilyId,
    bool artifactDealtKillingBlow
)
{
    HestiaArtifactKillResult out;
    PlayerHestiaArtifactState& state = player.getMutableHestiaArtifactState();
    out.level = std::max(1, state.level);
    out.damage = state.granted ? 3 * out.level : 0;
    out.killsRemaining = state.granted ? std::max(0, out.level - state.killsTowardNextLevel) : 0;

    if (!state.granted)
    {
        out.reason = "L'artefact n'a pas encore été confié au joueur.";
        return out;
    }
    if (!artifactDealtKillingBlow)
    {
        out.reason = "La mort n'a pas été portée par l'artefact : aucune âme n'est absorbée.";
        return out;
    }
    if (targetFamilyId.empty())
    {
        out.reason = "L'essence de la cible n'est pas identifiable : aucune progression.";
        return out;
    }

    HestiaArtifactEssenceRecord* essence = findEssence(state, targetFamilyId);
    if (essence && essence->acceptedKills >= SoulsPerFamilyLimit)
    {
        out.reason = "Cette essence est devenue trop familière : l'artefact refuse désormais ces âmes.";
        return out;
    }
    if (!essence)
    {
        state.absorbedEssences.push_back({targetFamilyId, 0});
        essence = &state.absorbedEssences.back();
    }

    ++essence->acceptedKills;
    ++state.killsTowardNextLevel;
    out.counted = true;
    out.reason = "L'artefact absorbe l'âme portée par son coup fatal.";

    const int required = std::max(1, state.level);
    if (state.killsTowardNextLevel >= required)
    {
        state.killsTowardNextLevel = 0;
        ++state.level;
        out.levelUp = true;
        if (state.level >= 16 && !state.secondFormUnlocked)
        {
            state.secondFormUnlocked = true;
            out.secondFormJustUnlocked = true;
        }
        player.recordCanonicalEvent("hestia_artefact", "level_" + std::to_string(state.level), "Artefact d'Hestia niveau " + std::to_string(state.level), 1);
    }

    out.level = state.level;
    out.damage = 3 * state.level;
    out.killsRemaining = std::max(0, state.level - state.killsTowardNextLevel);
    return out;
}
