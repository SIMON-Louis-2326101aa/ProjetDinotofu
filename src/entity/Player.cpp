// EN: Player.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Player.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Player.hpp"
#include "core/VersionInfo.hpp"

#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include <algorithm>
#include <iostream>


namespace
{
    bool isDistanceStarterClass(const std::string& className)
    {
        std::string normalized = className;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        return normalized.find("archer") != std::string::npos
            || normalized.find("rôdeur") != std::string::npos
            || normalized.find("rodeur") != std::string::npos
            || normalized.find("arbal") != std::string::npos
            || normalized.find("chasseur") != std::string::npos
            || normalized.find("lanceur de dagues") != std::string::npos
            || normalized.find("tireur") != std::string::npos
            || normalized.find("artificier") != std::string::npos;
    }

    bool usesStarterAmmunition(const std::string& className)
    {
        std::string normalized = className;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        return normalized.find("archer") != std::string::npos
            || normalized.find("rôdeur") != std::string::npos
            || normalized.find("rodeur") != std::string::npos
            || normalized.find("arbal") != std::string::npos
            || normalized.find("chasseur") != std::string::npos
            || normalized.find("lanceur de dagues") != std::string::npos
            || normalized.find("tireur") != std::string::npos
            || normalized.find("artificier") != std::string::npos;
    }

    bool inventoryHasWeaponNamed(const Inventory& inventory, const std::string& name)
    {
        for (const Weapon& weapon : inventory.getWeapons())
        {
            if (weapon.getName() == name) return true;
        }
        return false;
    }

    bool inventoryHasArmorNamed(const Inventory& inventory, const std::string& name)
    {
        for (const Armor& armor : inventory.getArmors())
        {
            if (armor.getName() == name) return true;
        }
        return false;
    }

    bool inventoryHasMaterialId(const Inventory& inventory, const std::string& id)
    {
        return inventory.countMaterialById(id) > 0;
    }

    std::string playerSkillDisplayName(const std::string& skillId)
    {
        if (skillId == "night_vision") return "Vision nocturne";
        if (skillId == "survival_breath") return "Souffle de survie";
        if (skillId == "ranger_eye") return "Œil de rôdeur";
        if (skillId == "steady_guard") return "Garde stable";
        if (skillId == "living_rampart") return "Rempart vivant";
        if (skillId == "sure_hand") return "Main sûre";
        if (skillId == "careful_dosage") return "Dosage prudent";
        if (skillId == "chain_execution") return "Enchaînement";
        if (skillId == "reflex_counter") return "Contre réflexe";
        if (skillId == "cautious_channeling") return "Canalisation prudente";
        if (skillId == "shadow_step") return "Pas de l'ombre";
        if (skillId == "arcane_impulse") return "Élan arcanique";
        if (skillId == "prepared_volley") return "Salve préparée";
        if (skillId == "tracking_mark") return "Marque de pisteur";
        if (skillId == "blade_discipline") return "Discipline de lame";
        if (skillId == "splitting_blow") return "Frappe fendue";
        if (skillId == "armor_crack") return "Fracasse-garde";
        if (skillId == "reach_control") return "Contrôle d'allonge";
        if (skillId == "learned_arcane_mark") return "Marque élémentaire étudiée";
        if (skillId == "learned_arcane_binding") return "Entrave arcanique étudiée";
        if (skillId == "learned_elemental_ward") return "Voile élémentaire étudié";
        if (skillId == "learned_resistance_rift") return "Faille de résistance étudiée";
        if (skillId == "learned_frost_needle") return "Aiguille de givre étudiée";
        if (skillId == "learned_mana_suture") return "Suture de mana étudiée";
        if (skillId == "learned_occult_bramble") return "Ronces occultes étudiées";
        return skillId;
    }

    std::string playerSkillDescription(const std::string& skillId)
    {
        if (skillId == "night_vision") return "Passif racial : tes yeux s'habituent mieux aux zones sombres et aux détails cachés.";
        if (skillId == "survival_breath") return "Passif : petit instinct de survie débloqué par l'expérience. Ton corps apprend à encaisser l'instant où tout bascule.";
        if (skillId == "ranger_eye") return "Passif : meilleure lecture des ouvertures à distance, gagné en utilisant régulièrement un arc.";
        if (skillId == "steady_guard") return "Passif de chevalier : posture plus stable lorsque tu tiens ta ligne.";
        if (skillId == "living_rampart") return "Passif de colosse : présence défensive plus lourde.";
        if (skillId == "sure_hand") return "Passif d'artisan/forgeron : meilleure maîtrise des réparations et gestes précis.";
        if (skillId == "careful_dosage") return "Passif d'alchimiste : meilleure lecture des potions et catalyseurs.";
        if (skillId == "chain_execution") return "Actif, recharge 3 tours : après plusieurs exécutions à la dague, tu cherches l'ouverture pour enchaîner si une cible tombe.";
        if (skillId == "reflex_counter") return "Actif, recharge 4 tours : contre brutal appris en combattant aux mains nues.";
        if (skillId == "cautious_channeling") return "Actif, recharge 3 tours : canalisation plus sûre après usage répété du bâton.";
        if (skillId == "shadow_step") return "Actif d'assassin, recharge 4 tours : déplacement court pour mieux choisir l'ouverture.";
        if (skillId == "arcane_impulse") return "Actif de mage, recharge 4 tours : poussée magique courte.";
        if (skillId == "prepared_volley") return "Actif d'artificier, recharge 5 tours : salve préparée de projectiles et outils.";
        if (skillId == "tracking_mark") return "Actif de rôdeur, recharge 3 tours : marque une cible après lecture des traces.";
        if (skillId == "blade_discipline") return "Passif : habitude des lignes propres à l'épée, gagnée à force de combattre avec une lame.";
        if (skillId == "splitting_blow") return "Actif, recharge 4 tours : coup ample appris en ouvrant les défenses avec une hache.";
        if (skillId == "armor_crack") return "Actif, recharge 4 tours : frappe lourde qui cherche les points faibles d'une garde.";
        if (skillId == "reach_control") return "Passif : meilleure lecture des distances, gagnée en maniant régulièrement une lance.";
        if (skillId == "learned_arcane_mark") return "Sort appris par étude : marque élémentaire simple, réservée aux vrais canalisateurs.";
        if (skillId == "learned_arcane_binding") return "Sort appris par grimoire : entrave la cible sans exister forcément en parchemin commun.";
        if (skillId == "learned_elemental_ward") return "Sort appris par grimoire : voile défensif utilisable avec un catalyseur correct.";
        if (skillId == "learned_resistance_rift") return "Sort rare appris par grimoire : ouvre une faille de résistance après préparation.";
        if (skillId == "learned_frost_needle") return "Sort de bibliothèque sans parchemin courant : givre précis, fragile et exigeant.";
        if (skillId == "learned_mana_suture") return "Sort appris par grimoire, sans parchemin courant : referme lentement les blessures pendant quelques tours.";
        if (skillId == "learned_occult_bramble") return "Sort appris par grimoire, sans parchemin courant : entrave la cible avec ronces, poison et fatigue magique.";
        return "Compétence instable : son usage reste difficile à canaliser.";
    }
}

// EN: Player declares or implements a focused behavior used by this module.
// FR: Player déclare ou implémente un comportement précis utilisé par ce module.
Player::Player() : Entity()
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
    race = CharacterRace::Human;
    unspentAttributePoints = 0;
    daggerKillProgress = 0;
    bowKillProgress = 0;
    bareHandKillProgress = 0;
    staffKillProgress = 0;
    swordKillProgress = 0;
    axeKillProgress = 0;
    hammerKillProgress = 0;
    spearKillProgress = 0;

    combatsStarted = 0;
    victories = 0;
    defeats = 0;
    escapes = 0;
    deaths = 0;
    enemiesKilled = 0;
    bossesKilled = 0;
    pvpVictories = 0;
    pvpDefeats = 0;
    pvpLethalEliminations.clear();
    unlockedBossIds = {1, 2, 3};
    recentBossIds.clear();
    recentCombatEquipmentUsage.clear();
    bossEquipmentSealActive = false;
    bossEquipmentSealReason = "";
    zelefCorrosionPresent = false;
    zelefMaxHpStolen = 0;
    grinkaBossTheftPresent = false;
    grinkaStolenGold = 0;
    grinkaHasStolenWeapon = false;
    grinkaHasStolenArmor = false;
    cloneCharacter = false;

    alteredByCheats = false;
    godModeEnabled = false;
    infiniteConsumablesEnabled = false;
    indestructibleEquipmentEnabled = false;
    equipmentProtectionEnabled = false;
    storySkipEnabled = false;
    specialChallengeAccessUnlocked = false;

    godModeKnown = false;
    infiniteConsumablesKnown = false;
    indestructibleEquipmentKnown = false;
    equipmentProtectionKnown = false;
    storySkipKnown = false;
    specialChallengeAccessKnown = false;
    creatorMessageKnown = false;

    goldCheatUseCount = 0;
    levelCheatUseCount = 0;
    maxLevelCheatUseCount = 0;
    refundCheatUseCount = 0;
    resetCheatUseCount = 0;
    switchClassCheatUseCount = 0;
    lethalCheatAttemptCount = 0;
    worldGazeCombatsRemaining = 0;
    worldGazeMaxHpPenalty = 0;

    refundUsesRemaining = 3;
    createdAtText = VersionInfo::currentDateText();
    createdForVersion = VersionInfo::currentVersion();
    lastAdaptedVersion = VersionInfo::currentVersion();
    creatorAccountName = "";
    currentOwnerAccountName = "";
    nextAmmunitionChoiceId = "";
    lastConsumedAmmunitionId = "";
}

Player::Player(
    const std::string& name,
    const PlayerClass& playerClass
) : Entity(
        name,
        playerClass.getName(),
        playerClass.getMaxHp(),
        playerClass.getMinDamage(),
        playerClass.getMaxDamage(),
        playerClass.getCriticalDamage(),
        playerClass.getHealingPotionCount(),
        // EN: playerClass.getDamagePotionCount declares or implements a focused behavior used by this module.
        // FR: playerClass.getDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
        playerClass.getDamagePotionCount()
    )
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
    race = CharacterRace::Human;
    unspentAttributePoints = 0;
    daggerKillProgress = 0;
    bowKillProgress = 0;
    bareHandKillProgress = 0;
    staffKillProgress = 0;
    swordKillProgress = 0;
    axeKillProgress = 0;
    hammerKillProgress = 0;
    spearKillProgress = 0;

    combatsStarted = 0;
    victories = 0;
    defeats = 0;
    escapes = 0;
    deaths = 0;
    enemiesKilled = 0;
    bossesKilled = 0;
    pvpVictories = 0;
    pvpDefeats = 0;
    pvpLethalEliminations.clear();
    unlockedBossIds = {1, 2, 3};
    recentBossIds.clear();
    recentCombatEquipmentUsage.clear();
    bossEquipmentSealActive = false;
    bossEquipmentSealReason = "";
    zelefCorrosionPresent = false;
    zelefMaxHpStolen = 0;
    grinkaBossTheftPresent = false;
    grinkaStolenGold = 0;
    grinkaHasStolenWeapon = false;
    grinkaHasStolenArmor = false;
    cloneCharacter = false;

    alteredByCheats = false;
    godModeEnabled = false;
    infiniteConsumablesEnabled = false;
    indestructibleEquipmentEnabled = false;
    equipmentProtectionEnabled = false;
    storySkipEnabled = false;
    specialChallengeAccessUnlocked = false;

    godModeKnown = false;
    infiniteConsumablesKnown = false;
    indestructibleEquipmentKnown = false;
    equipmentProtectionKnown = false;
    storySkipKnown = false;
    specialChallengeAccessKnown = false;
    creatorMessageKnown = false;

    goldCheatUseCount = 0;
    levelCheatUseCount = 0;
    maxLevelCheatUseCount = 0;
    refundCheatUseCount = 0;
    resetCheatUseCount = 0;
    switchClassCheatUseCount = 0;
    lethalCheatAttemptCount = 0;
    worldGazeCombatsRemaining = 0;
    worldGazeMaxHpPenalty = 0;

    refundUsesRemaining = 3;
    createdAtText = VersionInfo::currentDateText();
    createdForVersion = VersionInfo::currentVersion();
    lastAdaptedVersion = VersionInfo::currentVersion();
    creatorAccountName = "";
    currentOwnerAccountName = "";
    nextAmmunitionChoiceId = "";
    lastConsumedAmmunitionId = "";
}


void Player::setNextAmmunitionChoice(const std::string& ammunitionId)
{
    nextAmmunitionChoiceId = ammunitionId;
}

std::string Player::getNextAmmunitionChoice() const
{
    return nextAmmunitionChoiceId;
}

void Player::clearNextAmmunitionChoice()
{
    nextAmmunitionChoiceId.clear();
}

void Player::setLastConsumedAmmunition(const std::string& ammunitionId)
{
    lastConsumedAmmunitionId = ammunitionId;
}

std::string Player::getLastConsumedAmmunition() const
{
    return lastConsumedAmmunitionId;
}

void Player::clearLastConsumedAmmunition()
{
    lastConsumedAmmunitionId.clear();
}

const std::string& Player::getCreatedAtText() const
{
    return createdAtText;
}

const std::string& Player::getCreatorAccountName() const
{
    return creatorAccountName;
}

const std::string& Player::getCurrentOwnerAccountName() const
{
    return currentOwnerAccountName;
}

void Player::setOwnershipMetadata(const std::string& creatorAccount, const std::string& currentOwnerAccount)
{
    creatorAccountName = creatorAccount;
    currentOwnerAccountName = currentOwnerAccount;
}


const std::string& Player::getCreatedForVersion() const
{
    return createdForVersion;
}

const std::string& Player::getLastAdaptedVersion() const
{
    return lastAdaptedVersion;
}

void Player::setVersionMetadata(const std::string& createdAt, const std::string& createdFor, const std::string& lastAdapted)
{
    createdAtText = createdAt.empty() ? "Inconnue" : createdAt;
    createdForVersion = createdFor.empty() ? "inconnue" : createdFor;
    lastAdaptedVersion = lastAdapted.empty() ? createdForVersion : lastAdapted;
}

void Player::markAdaptedToCurrentVersion()
{
    lastAdaptedVersion = VersionInfo::currentVersion();
}

const std::vector<std::string>& Player::getStarterKitLog() const
{
    return starterKitLog;
}

void Player::setLoadedStarterKitLog(const std::vector<std::string>& log)
{
    starterKitLog = log;
}

void Player::recordStarterKitEntry(const std::string& entry)
{
    if (!entry.empty())
    {
        starterKitLog.push_back(entry);
    }
}

// EN: applyRaceStartingBonus declares or implements a focused behavior used by this module.
// FR: applyRaceStartingBonus déclare ou implémente un comportement précis utilisé par ce module.
void Player::applyRaceStartingBonus(CharacterRace selectedRace)
{
    RaceStartingBonus bonus = RaceCatalog::getStartingBonus(selectedRace);

    maxHp += bonus.maxHpBonus;
    hp += bonus.maxHpBonus;

    minDamage += bonus.minDamageBonus;
    maxDamage += bonus.maxDamageBonus;
    criticalDamage += bonus.criticalDamageBonus;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp < 1)
    {
        hp = 1;
    }

    if (minDamage < 0)
    {
        minDamage = 0;
    }

    if (maxDamage < minDamage)
    {
        maxDamage = minDamage;
    }

    if (criticalDamage < maxDamage)
    {
        criticalDamage = maxDamage;
    }
}

// EN: getEquippedArmorMaxHpBonus declares or implements a focused behavior used by this module.
// FR: getEquippedArmorMaxHpBonus déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEquippedArmorMaxHpBonus() const
{
    if (!hasEquippedArmor())
    {
        return 0;
    }

    Armor armor = getEquippedArmor();

    if (armor.isBroken())
    {
        return 0;
    }

    return armor.getMaxHpBonus();
}

// EN: getLevel declares or implements a focused behavior used by this module.
// FR: getLevel déclare ou implémente un comportement précis utilisé par ce module.
int Player::getLevel() const
{
    return level;
}

// EN: getExperience declares or implements a focused behavior used by this module.
// FR: getExperience déclare ou implémente un comportement précis utilisé par ce module.
int Player::getExperience() const
{
    return experience;
}

// EN: getUnspentAttributePoints declares or implements a focused behavior used by this module.
// FR: getUnspentAttributePoints déclare ou implémente un comportement précis utilisé par ce module.
int Player::getUnspentAttributePoints() const
{
    return unspentAttributePoints;
}

// EN: getAttributes declares or implements a focused behavior used by this module.
// FR: getAttributes déclare ou implémente un comportement précis utilisé par ce module.
const DndAttributes& Player::getAttributes() const
{
    return attributes;
}

// EN: getRace declares or implements a focused behavior used by this module.
// FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
CharacterRace Player::getRace() const
{
    return race;
}

std::string Player::getRaceText() const
{
    return characterRaceToText(race);
}

// EN: setRace declares or implements a focused behavior used by this module.
// FR: setRace déclare ou implémente un comportement précis utilisé par ce module.
void Player::setRace(CharacterRace selectedRace)
{
    race = selectedRace;
    applyRaceStartingBonus(selectedRace);
}


// EN: setLoadedProgress declares or implements a focused behavior used by this module.
// FR: setLoadedProgress déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedProgress(int loadedLevel, int loadedExperience, int loadedHp)
{
    if (loadedLevel < 1)
    {
        loadedLevel = 1;
    }

    if (loadedLevel > MAX_LEVEL)
    {
        loadedLevel = MAX_LEVEL;
    }

    if (loadedExperience < 0)
    {
        loadedExperience = 0;
    }

    if (loadedHp < 1)
    {
        loadedHp = 1;
    }

    if (loadedHp > maxHp)
    {
        loadedHp = maxHp;
    }

    level = loadedLevel;
    experience = loadedExperience;
    hp = loadedHp;
}


void Player::setLoadedCombatStats(
    int loadedMaxHp,
    int loadedMinDamage,
    int loadedMaxDamage,
    int loadedCriticalDamage,
    int loadedHp
)
{
    if (loadedMaxHp < 1)
    {
        loadedMaxHp = 1;
    }

    if (loadedMinDamage < 0)
    {
        loadedMinDamage = 0;
    }

    if (loadedMaxDamage < loadedMinDamage)
    {
        loadedMaxDamage = loadedMinDamage;
    }

    if (loadedCriticalDamage < loadedMaxDamage)
    {
        loadedCriticalDamage = loadedMaxDamage;
    }

    if (loadedHp < 1)
    {
        loadedHp = 1;
    }

    if (loadedHp > loadedMaxHp)
    {
        loadedHp = loadedMaxHp;
    }

    maxHp = loadedMaxHp;
    minDamage = loadedMinDamage;
    maxDamage = loadedMaxDamage;
    criticalDamage = loadedCriticalDamage;
    hp = loadedHp;
}

// EN: setLoadedAttributes declares or implements a focused behavior used by this module.
// FR: setLoadedAttributes déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedAttributes(const DndAttributes& loadedAttributes, int loadedUnspentPoints)
{
    attributes = loadedAttributes;

    if (loadedUnspentPoints < 0)
    {
        loadedUnspentPoints = 0;
    }

    unspentAttributePoints = loadedUnspentPoints;
}

// EN: spendAttributePoint declares or implements a focused behavior used by this module.
// FR: spendAttributePoint déclare ou implémente un comportement précis utilisé par ce module.
bool Player::spendAttributePoint(int attributeChoice)
{
    if (unspentAttributePoints <= 0)
    {
        return false;
    }

    if (!attributes.increaseByChoice(attributeChoice))
    {
        return false;
    }

    unspentAttributePoints--;

    if (attributeChoice == 1)
    {
        minDamage += 1;
        maxDamage += 1;
    }
    else if (attributeChoice == 2)
    {
        criticalDamage += 2;
    }
    else if (attributeChoice == 3)
    {
        maxHp += 5;
        hp += 5;
    }

    if (maxDamage < minDamage)
    {
        maxDamage = minDamage;
    }

    if (criticalDamage < maxDamage)
    {
        criticalDamage = maxDamage;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }

    return true;
}


// EN: displayAttributes declares or implements a focused behavior used by this module.
// FR: displayAttributes déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayAttributes() const
{
    std::cout << "===== ATTRIBUTS =====" << std::endl;
    std::cout << "Force : " << attributes.getStrength() << std::endl;
    std::cout << "Dextérité : " << attributes.getDexterity() << std::endl;
    std::cout << "Constitution : " << attributes.getConstitution() << std::endl;
    std::cout << "Intelligence : " << attributes.getIntelligence() << std::endl;
    std::cout << "Sagesse : " << attributes.getWisdom() << std::endl;
    std::cout << "Charisme : " << attributes.getCharisma() << std::endl;
    std::cout << "Points disponibles : " << unspentAttributePoints << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << std::endl;
}

// EN: getUnlockedPassiveSkills declares or implements a focused behavior used by this module.
// FR: getUnlockedPassiveSkills déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getUnlockedPassiveSkills() const
{
    return unlockedPassiveSkills;
}

// EN: getUnlockedActiveSkills declares or implements a focused behavior used by this module.
// FR: getUnlockedActiveSkills déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getUnlockedActiveSkills() const
{
    return unlockedActiveSkills;
}

// EN: getDaggerKillProgress declares or implements a focused behavior used by this module.
// FR: getDaggerKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getDaggerKillProgress() const
{
    return daggerKillProgress;
}

// EN: getBowKillProgress declares or implements a focused behavior used by this module.
// FR: getBowKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getBowKillProgress() const
{
    return bowKillProgress;
}

// EN: getBareHandKillProgress declares or implements a focused behavior used by this module.
// FR: getBareHandKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getBareHandKillProgress() const
{
    return bareHandKillProgress;
}

// EN: getStaffKillProgress declares or implements a focused behavior used by this module.
// FR: getStaffKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getStaffKillProgress() const
{
    return staffKillProgress;
}

int Player::getSwordKillProgress() const
{
    return swordKillProgress;
}

int Player::getAxeKillProgress() const
{
    return axeKillProgress;
}

int Player::getHammerKillProgress() const
{
    return hammerKillProgress;
}

int Player::getSpearKillProgress() const
{
    return spearKillProgress;
}

// EN: hasPassiveSkill declares or implements a focused behavior used by this module.
// FR: hasPassiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasPassiveSkill(const std::string& skillId) const
{
    return std::find(unlockedPassiveSkills.begin(), unlockedPassiveSkills.end(), skillId) != unlockedPassiveSkills.end();
}

// EN: hasActiveSkill declares or implements a focused behavior used by this module.
// FR: hasActiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasActiveSkill(const std::string& skillId) const
{
    return std::find(unlockedActiveSkills.begin(), unlockedActiveSkills.end(), skillId) != unlockedActiveSkills.end();
}

// EN: unlockPassiveSkill declares or implements a focused behavior used by this module.
// FR: unlockPassiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::unlockPassiveSkill(const std::string& skillId, const std::string& skillName)
{
    if (hasPassiveSkill(skillId))
    {
        return false;
    }

    unlockedPassiveSkills.push_back(skillId);
    std::cout << "Nouvelle compétence passive : " << skillName << std::endl;
    std::cout << "Elle s'est développée à force de vivre, combattre et apprendre." << std::endl;
    std::cout << std::endl;
    return true;
}

// EN: unlockActiveSkill declares or implements a focused behavior used by this module.
// FR: unlockActiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::unlockActiveSkill(const std::string& skillId, const std::string& skillName)
{
    if (hasActiveSkill(skillId))
    {
        return false;
    }

    unlockedActiveSkills.push_back(skillId);
    std::cout << "Nouvelle compétence active : " << skillName << std::endl;
    std::cout << "Son rythme naturel apparaîtra quand tu sauras mieux la canaliser en combat." << std::endl;
    std::cout << std::endl;
    return true;
}

// EN: recordGameplaySkillProgressForKills declares or implements a focused behavior used by this module.
// FR: recordGameplaySkillProgressForKills déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGameplaySkillProgressForKills(int amount)
{
    if (amount <= 0 || !hasEquippedWeapon())
    {
        return;
    }

    Weapon weapon = getEquippedWeapon();

    if (weapon.getType() == WeaponType::Dagger)
    {
        daggerKillProgress += amount;
        if (daggerKillProgress >= 5)
        {
            unlockActiveSkill("chain_execution", "Enchaînement");
        }
    }
    else if (weapon.getType() == WeaponType::Bow)
    {
        bowKillProgress += amount;
        if (bowKillProgress >= 8)
        {
            unlockPassiveSkill("ranger_eye", "Œil de rôdeur");
        }
    }
    else if (weapon.getType() == WeaponType::BareHands)
    {
        bareHandKillProgress += amount;
        if (bareHandKillProgress >= 10)
        {
            unlockActiveSkill("reflex_counter", "Contre réflexe");
        }
    }
    else if (weapon.getType() == WeaponType::Staff)
    {
        staffKillProgress += amount;
        if (staffKillProgress >= 6)
        {
            unlockActiveSkill("cautious_channeling", "Canalisation prudente");
        }
    }
    else if (weapon.getType() == WeaponType::Sword)
    {
        swordKillProgress += amount;
        if (swordKillProgress >= 7)
        {
            unlockPassiveSkill("blade_discipline", "Discipline de lame");
        }
    }
    else if (weapon.getType() == WeaponType::Axe)
    {
        axeKillProgress += amount;
        if (axeKillProgress >= 7)
        {
            unlockActiveSkill("splitting_blow", "Frappe fendue");
        }
    }
    else if (weapon.getType() == WeaponType::Hammer)
    {
        hammerKillProgress += amount;
        if (hammerKillProgress >= 7)
        {
            unlockActiveSkill("armor_crack", "Fracasse-garde");
        }
    }
    else if (weapon.getType() == WeaponType::Spear)
    {
        spearKillProgress += amount;
        if (spearKillProgress >= 7)
        {
            unlockPassiveSkill("reach_control", "Contrôle d'allonge");
        }
    }
}

// EN: refreshLevelAndIdentitySkills declares or implements a focused behavior used by this module.
// FR: refreshLevelAndIdentitySkills déclare ou implémente un comportement précis utilisé par ce module.
void Player::refreshLevelAndIdentitySkills()
{
    CharacterRace currentRace = getRace();

    if (currentRace == CharacterRace::DarkElf
        || currentRace == CharacterRace::Kitsune
        || currentRace == CharacterRace::Vampire
        || currentRace == CharacterRace::Demon)
    {
        unlockPassiveSkill("night_vision", "Vision nocturne");
    }

    if (level >= 3)
    {
        unlockPassiveSkill("survival_breath", "Souffle de survie");
    }

    std::string className = getType();

    if (level >= 4 && className == "Assassin")
    {
        unlockActiveSkill("shadow_step", "Pas de l'ombre");
    }
    else if (level >= 4 && className == "Chevalier")
    {
        unlockPassiveSkill("steady_guard", "Garde stable");
    }
    else if (level >= 4 && className == "Colosse")
    {
        unlockPassiveSkill("living_rampart", "Rempart vivant");
    }
    else if (level >= 4 && className == "Mage")
    {
        unlockActiveSkill("arcane_impulse", "Élan arcanique");
    }
    else if (level >= 4 && className == "Forgeron")
    {
        unlockPassiveSkill("sure_hand", "Main sûre");
    }
    else if (level >= 4 && className == "Alchimiste")
    {
        unlockPassiveSkill("careful_dosage", "Dosage prudent");
    }
    else if (level >= 4 && className == "Rôdeur")
    {
        unlockActiveSkill("tracking_mark", "Marque de pisteur");
    }
    else if (level >= 4 && className == "Artificier")
    {
        unlockActiveSkill("prepared_volley", "Salve préparée");
    }
}

void Player::setLoadedSkillState(
    const std::vector<std::string>& passiveSkills,
    const std::vector<std::string>& activeSkills,
    int daggerProgress,
    int bowProgress,
    int bareHandProgress,
    int staffProgress,
    int swordProgress,
    int axeProgress,
    int hammerProgress,
    int spearProgress
)
{
    unlockedPassiveSkills = passiveSkills;
    unlockedActiveSkills = activeSkills;
    daggerKillProgress = daggerProgress < 0 ? 0 : daggerProgress;
    bowKillProgress = bowProgress < 0 ? 0 : bowProgress;
    bareHandKillProgress = bareHandProgress < 0 ? 0 : bareHandProgress;
    staffKillProgress = staffProgress < 0 ? 0 : staffProgress;
    swordKillProgress = swordProgress < 0 ? 0 : swordProgress;
    axeKillProgress = axeProgress < 0 ? 0 : axeProgress;
    hammerKillProgress = hammerProgress < 0 ? 0 : hammerProgress;
    spearKillProgress = spearProgress < 0 ? 0 : spearProgress;
}

// EN: displaySkillProgress declares or implements a focused behavior used by this module.
// FR: displaySkillProgress déclare ou implémente un comportement précis utilisé par ce module.
void Player::displaySkillProgress() const
{
    std::cout << "========== COMPÉTENCES ==========" << std::endl;

    std::cout << "Passives débloquées :" << std::endl;
    if (unlockedPassiveSkills.empty())
    {
        std::cout << "- Aucune pour le moment." << std::endl;
    }
    else
    {
        for (const std::string& skillId : unlockedPassiveSkills)
        {
            std::cout << "- " << playerSkillDisplayName(skillId) << " : " << playerSkillDescription(skillId) << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Actives débloquées :" << std::endl;
    if (unlockedActiveSkills.empty())
    {
        std::cout << "- Aucune pour le moment." << std::endl;
    }
    else
    {
        for (const std::string& skillId : unlockedActiveSkills)
        {
            std::cout << "- " << playerSkillDisplayName(skillId) << " : " << playerSkillDescription(skillId) << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Progression passive liée au gameplay :" << std::endl;
    std::cout << "- Kills à la dague vers Enchaînement : " << daggerKillProgress << "/5" << std::endl;
    std::cout << "- Kills à l'arc vers Œil de rôdeur : " << bowKillProgress << "/8" << std::endl;
    std::cout << "- Kills aux mains nues vers Contre réflexe : " << bareHandKillProgress << "/10" << std::endl;
    std::cout << "- Kills au bâton vers Canalisation prudente : " << staffKillProgress << "/6" << std::endl;
    std::cout << "- Kills à l'épée vers Discipline de lame : " << swordKillProgress << "/7" << std::endl;
    std::cout << "- Kills à la hache vers Frappe fendue : " << axeKillProgress << "/7" << std::endl;
    std::cout << "- Kills au marteau vers Fracasse-garde : " << hammerKillProgress << "/7" << std::endl;
    std::cout << "- Kills à la lance vers Contrôle d'allonge : " << spearKillProgress << "/7" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << std::endl;
}


// EN: getCombatsStarted declares or implements a focused behavior used by this module.
// FR: getCombatsStarted déclare ou implémente un comportement précis utilisé par ce module.
int Player::getCombatsStarted() const
{
    return combatsStarted;
}

// EN: getVictories declares or implements a focused behavior used by this module.
// FR: getVictories déclare ou implémente un comportement précis utilisé par ce module.
int Player::getVictories() const
{
    return victories;
}

// EN: getDefeats declares or implements a focused behavior used by this module.
// FR: getDefeats déclare ou implémente un comportement précis utilisé par ce module.
int Player::getDefeats() const
{
    return defeats;
}

// EN: getEscapes declares or implements a focused behavior used by this module.
// FR: getEscapes déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEscapes() const
{
    return escapes;
}

// EN: getDeaths declares or implements a focused behavior used by this module.
// FR: getDeaths déclare ou implémente un comportement précis utilisé par ce module.
int Player::getDeaths() const
{
    return deaths;
}

// EN: getEnemiesKilled declares or implements a focused behavior used by this module.
// FR: getEnemiesKilled déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEnemiesKilled() const
{
    return enemiesKilled;
}

// EN: getBossesKilled declares or implements a focused behavior used by this module.
// FR: getBossesKilled déclare ou implémente un comportement précis utilisé par ce module.
int Player::getBossesKilled() const
{
    return bossesKilled;
}

// EN: getPvpVictories declares or implements a focused behavior used by this module.
// FR: getPvpVictories déclare ou implémente un comportement précis utilisé par ce module.
int Player::getPvpVictories() const
{
    return pvpVictories;
}

// EN: getPvpDefeats declares or implements a focused behavior used by this module.
// FR: getPvpDefeats déclare ou implémente un comportement précis utilisé par ce module.
int Player::getPvpDefeats() const
{
    return pvpDefeats;
}

// EN: getPvpLethalEliminations declares or implements a focused behavior used by this module.
// FR: getPvpLethalEliminations déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getPvpLethalEliminations() const
{
    return pvpLethalEliminations;
}


// EN: getUnlockedBossIds declares or implements a focused behavior used by this module.
// FR: getUnlockedBossIds déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<int>& Player::getUnlockedBossIds() const
{
    return unlockedBossIds;
}

// EN: getRecentBossIds declares or implements a focused behavior used by this module.
// FR: getRecentBossIds déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<int>& Player::getRecentBossIds() const
{
    return recentBossIds;
}

// EN: getRecentCombatEquipmentUsage declares or implements a focused behavior used by this module.
// FR: getRecentCombatEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getRecentCombatEquipmentUsage() const
{
    return recentCombatEquipmentUsage;
}

// EN: recordCurrentEquipmentUsage declares or implements a focused behavior used by this module.
// FR: recordCurrentEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordCurrentEquipmentUsage()
{
    std::string label;

    if (hasEquippedWeapon())
    {
        label += "Arme:" + getEquippedWeapon().getName();
    }

    if (hasEquippedArmor())
    {
        if (!label.empty())
        {
            label += " | ";
        }
        label += "Armure:" + getEquippedArmor().getName();
    }

    if (label.empty())
    {
        label = "Aucun équipement majeur";
    }

    recentCombatEquipmentUsage.push_back(label);

    while (recentCombatEquipmentUsage.size() > 10)
    {
        recentCombatEquipmentUsage.erase(recentCombatEquipmentUsage.begin());
    }
}

// EN: setLoadedRecentCombatEquipmentUsage declares or implements a focused behavior used by this module.
// FR: setLoadedRecentCombatEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedRecentCombatEquipmentUsage(const std::vector<std::string>& usage)
{
    recentCombatEquipmentUsage.clear();

    std::size_t start = usage.size() > 10 ? usage.size() - 10 : 0;
    for (std::size_t i = start; i < usage.size(); ++i)
    {
        if (!usage[i].empty())
        {
            recentCombatEquipmentUsage.push_back(usage[i]);
        }
    }
}

// EN: hasBossEquipmentSeal declares or implements a focused behavior used by this module.
// FR: hasBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasBossEquipmentSeal() const
{
    return bossEquipmentSealActive;
}

std::string Player::getBossEquipmentSealReason() const
{
    return bossEquipmentSealReason;
}

// EN: activateBossEquipmentSeal declares or implements a focused behavior used by this module.
// FR: activateBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
void Player::activateBossEquipmentSeal(const std::string& reason)
{
    bossEquipmentSealActive = true;
    bossEquipmentSealReason = reason;
}

// EN: clearBossEquipmentSeal declares or implements a focused behavior used by this module.
// FR: clearBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
void Player::clearBossEquipmentSeal()
{
    bossEquipmentSealActive = false;
    bossEquipmentSealReason = "";
}

// EN: isBossUnlocked declares or implements a focused behavior used by this module.
// FR: isBossUnlocked déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isBossUnlocked(int bossId) const
{
    for (int id : unlockedBossIds)
    {
        if (id == bossId)
        {
            return true;
        }
    }

    return false;
}

// EN: isBossRecentlyDefeated declares or implements a focused behavior used by this module.
// FR: isBossRecentlyDefeated déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isBossRecentlyDefeated(int bossId) const
{
    for (int id : recentBossIds)
    {
        if (id == bossId)
        {
            return true;
        }
    }

    return false;
}

std::vector<int> Player::getAvailableBossIds() const
{
    std::vector<int> available;

    for (int id : unlockedBossIds)
    {
        if (!isBossRecentlyDefeated(id))
        {
            available.push_back(id);
        }
    }

    if (available.empty())
    {
        return unlockedBossIds;
    }

    return available;
}

// EN: unlockNextBossVariation declares or implements a focused behavior used by this module.
// FR: unlockNextBossVariation déclare ou implémente un comportement précis utilisé par ce module.
bool Player::unlockNextBossVariation()
{
    const std::vector<int> progressionOrder = {
        4, 5, 6,
        7, 8, 9,
        10, 12,
        13, 14, 15,
        16, 17, 18,
        19, 20, 21,
        22, 24, 25,
        28, 29, 31,
        32, 33, 34, 35,
        23,
        30,
        11,
        26,
        27
    };

    for (int nextId : progressionOrder)
    {
        if (!isBossUnlocked(nextId))
        {
            unlockedBossIds.push_back(nextId);
            return true;
        }
    }

    return false;
}

// EN: recordBossVictoryInRegistry declares or implements a focused behavior used by this module.
// FR: recordBossVictoryInRegistry déclare ou implémente un comportement précis utilisé par ce module.
bool Player::recordBossVictoryInRegistry(int bossId)
{
    if (bossId <= 0)
    {
        return false;
    }

    recentBossIds.push_back(bossId);

    while (recentBossIds.size() > 2)
    {
        recentBossIds.erase(recentBossIds.begin());
    }

    return unlockNextBossVariation();
}

// EN: setLoadedBossRegistry declares or implements a focused behavior used by this module.
// FR: setLoadedBossRegistry déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedBossRegistry(const std::vector<int>& unlockedIds, const std::vector<int>& recentIds)
{
    unlockedBossIds.clear();

    if (unlockedIds.empty())
    {
        unlockedBossIds = {1, 2, 3};
    }
    else
    {
        for (int id : unlockedIds)
        {
            if (id > 0 && !isBossUnlocked(id))
            {
                unlockedBossIds.push_back(id);
            }
        }
    }

    for (int baseId = 1; baseId <= 3; ++baseId)
    {
        if (!isBossUnlocked(baseId))
        {
            unlockedBossIds.push_back(baseId);
        }
    }

    recentBossIds.clear();

    for (int id : recentIds)
    {
        if (id > 0 && isBossUnlocked(id))
        {
            recentBossIds.push_back(id);
        }
    }

    while (recentBossIds.size() > 2)
    {
        recentBossIds.erase(recentBossIds.begin());
    }
}


// EN: hasZelefCorrosionPresent declares or implements a focused behavior used by this module.
// FR: hasZelefCorrosionPresent déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasZelefCorrosionPresent() const
{
    return zelefCorrosionPresent;
}

// EN: getZelefMaxHpStolen declares or implements a focused behavior used by this module.
// FR: getZelefMaxHpStolen déclare ou implémente un comportement précis utilisé par ce module.
int Player::getZelefMaxHpStolen() const
{
    return zelefMaxHpStolen;
}

// EN: hasGrinkaBossTheftPresent declares or implements a focused behavior used by this module.
// FR: hasGrinkaBossTheftPresent déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasGrinkaBossTheftPresent() const
{
    return grinkaBossTheftPresent;
}

// EN: getGrinkaStolenGold declares or implements a focused behavior used by this module.
// FR: getGrinkaStolenGold déclare ou implémente un comportement précis utilisé par ce module.
int Player::getGrinkaStolenGold() const
{
    return grinkaStolenGold;
}

// EN: hasGrinkaStolenWeapon declares or implements a focused behavior used by this module.
// FR: hasGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasGrinkaStolenWeapon() const
{
    return grinkaHasStolenWeapon;
}

// EN: hasGrinkaStolenArmor declares or implements a focused behavior used by this module.
// FR: hasGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasGrinkaStolenArmor() const
{
    return grinkaHasStolenArmor;
}

// EN: getGrinkaStolenWeapon declares or implements a focused behavior used by this module.
// FR: getGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon Player::getGrinkaStolenWeapon() const
{
    return grinkaStolenWeapon;
}

// EN: getGrinkaStolenArmor declares or implements a focused behavior used by this module.
// FR: getGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor Player::getGrinkaStolenArmor() const
{
    return grinkaStolenArmor;
}

// EN: recordZelefCorrosionLoss declares or implements a focused behavior used by this module.
// FR: recordZelefCorrosionLoss déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordZelefCorrosionLoss(int maxHpLoss)
{
    if (maxHpLoss <= 0)
    {
        return;
    }

    zelefCorrosionPresent = true;
    zelefMaxHpStolen += maxHpLoss;
}

// EN: restoreZelefCorrosionLoss declares or implements a focused behavior used by this module.
// FR: restoreZelefCorrosionLoss déclare ou implémente un comportement précis utilisé par ce module.
void Player::restoreZelefCorrosionLoss()
{
    if (!zelefCorrosionPresent || zelefMaxHpStolen <= 0)
    {
        zelefCorrosionPresent = false;
        zelefMaxHpStolen = 0;
        return;
    }

    maxHp += zelefMaxHpStolen;
    hp += zelefMaxHpStolen;
    zelefCorrosionPresent = false;
    zelefMaxHpStolen = 0;
}

// EN: recordGrinkaGoldTheft declares or implements a focused behavior used by this module.
// FR: recordGrinkaGoldTheft déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGrinkaGoldTheft(int goldAmount)
{
    if (goldAmount <= 0)
    {
        return;
    }

    grinkaBossTheftPresent = true;
    grinkaStolenGold += goldAmount;
}

// EN: recordGrinkaStolenWeapon declares or implements a focused behavior used by this module.
// FR: recordGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGrinkaStolenWeapon(const Weapon& weapon)
{
    grinkaBossTheftPresent = true;
    grinkaHasStolenWeapon = true;
    grinkaStolenWeapon = weapon;
}

// EN: recordGrinkaStolenArmor declares or implements a focused behavior used by this module.
// FR: recordGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGrinkaStolenArmor(const Armor& armor)
{
    grinkaBossTheftPresent = true;
    grinkaHasStolenArmor = true;
    grinkaStolenArmor = armor;
}

// EN: restoreGrinkaBossTheft declares or implements a focused behavior used by this module.
// FR: restoreGrinkaBossTheft déclare ou implémente un comportement précis utilisé par ce module.
void Player::restoreGrinkaBossTheft()
{
    if (!grinkaBossTheftPresent)
    {
        return;
    }

    if (grinkaStolenGold > 0)
    {
        inventory.earnGold(grinkaStolenGold);
    }

    if (grinkaHasStolenWeapon)
    {
        inventory.addWeapon(grinkaStolenWeapon);
    }

    if (grinkaHasStolenArmor)
    {
        inventory.addArmor(grinkaStolenArmor);
    }

    grinkaBossTheftPresent = false;
    grinkaStolenGold = 0;
    grinkaHasStolenWeapon = false;
    grinkaHasStolenArmor = false;
}

void Player::setLoadedBossConsequences(
    bool zelefCorrosion,
    int zelefMaxHpLoss,
    bool grinkaTheft,
    int grinkaGoldLoss,
    bool hasStolenWeapon,
    const Weapon& stolenWeapon,
    bool hasStolenArmor,
    const Armor& stolenArmor
)
{
    zelefCorrosionPresent = zelefCorrosion;
    zelefMaxHpStolen = zelefMaxHpLoss < 0 ? 0 : zelefMaxHpLoss;

    grinkaBossTheftPresent = grinkaTheft;
    grinkaStolenGold = grinkaGoldLoss < 0 ? 0 : grinkaGoldLoss;
    grinkaHasStolenWeapon = hasStolenWeapon;
    grinkaStolenWeapon = stolenWeapon;
    grinkaHasStolenArmor = hasStolenArmor;
    grinkaStolenArmor = stolenArmor;
}


// EN: getLethalCheatAttemptCount declares or implements a focused behavior used by this module.
// FR: getLethalCheatAttemptCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getLethalCheatAttemptCount() const
{
    return lethalCheatAttemptCount;
}

// EN: getWorldGazeCombatsRemaining declares or implements a focused behavior used by this module.
// FR: getWorldGazeCombatsRemaining déclare ou implémente un comportement précis utilisé par ce module.
int Player::getWorldGazeCombatsRemaining() const
{
    return worldGazeCombatsRemaining;
}

// EN: getWorldGazeMaxHpPenalty declares or implements a focused behavior used by this module.
// FR: getWorldGazeMaxHpPenalty déclare ou implémente un comportement précis utilisé par ce module.
int Player::getWorldGazeMaxHpPenalty() const
{
    return worldGazeMaxHpPenalty;
}

// EN: hasWorldGazePenalty declares or implements a focused behavior used by this module.
// FR: hasWorldGazePenalty déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasWorldGazePenalty() const
{
    return worldGazeCombatsRemaining > 0 || worldGazeMaxHpPenalty > 0;
}

// EN: setLethalCheatPenaltyState declares or implements a focused behavior used by this module.
// FR: setLethalCheatPenaltyState déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLethalCheatPenaltyState(int attempts, int remainingCombats, int maxHpPenalty)
{
    lethalCheatAttemptCount = attempts < 0 ? 0 : attempts;
    worldGazeCombatsRemaining = remainingCombats < 0 ? 0 : remainingCombats;
    worldGazeMaxHpPenalty = maxHpPenalty < 0 ? 0 : maxHpPenalty;
}

// EN: applyLethalCheatAttemptPenalty declares or implements a focused behavior used by this module.
// FR: applyLethalCheatAttemptPenalty déclare ou implémente un comportement précis utilisé par ce module.
void Player::applyLethalCheatAttemptPenalty()
{
    lethalCheatAttemptCount++;

    int directDamage = hp / 2;
    if (directDamage < 1 && hp > 1)
    {
        directDamage = 1;
    }
    hp -= directDamage;
    if (hp < 1)
    {
        hp = 1;
    }

    int currentGold = inventory.getGold();
    int goldLoss = currentGold / 4;
    inventory.setGold(currentGold - goldLoss);

    if (worldGazeMaxHpPenalty > 0)
    {
        applyFlatStatBonus(worldGazeMaxHpPenalty, 0, 0, 0);
        worldGazeMaxHpPenalty = 0;
    }

    int maxHpLoss = maxHp * 15 / 100;
    if (maxHpLoss < 1)
    {
        maxHpLoss = 1;
    }

    reduceMaxHp(maxHpLoss);
    worldGazeMaxHpPenalty = maxHpLoss;
    worldGazeCombatsRemaining = 3;
}

// EN: reduceWorldGazeDurationAfterCombat declares or implements a focused behavior used by this module.
// FR: reduceWorldGazeDurationAfterCombat déclare ou implémente un comportement précis utilisé par ce module.
void Player::reduceWorldGazeDurationAfterCombat()
{
    if (worldGazeCombatsRemaining <= 0)
    {
        return;
    }

    worldGazeCombatsRemaining--;

    if (worldGazeCombatsRemaining == 0 && worldGazeMaxHpPenalty > 0)
    {
        applyFlatStatBonus(worldGazeMaxHpPenalty, 0, 0, 0);
        worldGazeMaxHpPenalty = 0;
        std::cout << "Le Regard du monde se retire. Tes PV maximum reviennent à leur état précédent." << std::endl;
        std::cout << std::endl;
    }
}

// EN: isClone declares or implements a focused behavior used by this module.
// FR: isClone déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isClone() const
{
    return cloneCharacter;
}

// EN: setClone declares or implements a focused behavior used by this module.
// FR: setClone déclare ou implémente un comportement précis utilisé par ce module.
void Player::setClone(bool clone)
{
    cloneCharacter = clone;
}

void Player::setLoadedStatistics(
    int loadedCombatsStarted,
    int loadedVictories,
    int loadedDefeats,
    int loadedEscapes,
    int loadedDeaths,
    int loadedEnemiesKilled,
    int loadedBossesKilled,
    int loadedPvpVictories,
    int loadedPvpDefeats
)
{
    combatsStarted = loadedCombatsStarted < 0 ? 0 : loadedCombatsStarted;
    victories = loadedVictories < 0 ? 0 : loadedVictories;
    defeats = loadedDefeats < 0 ? 0 : loadedDefeats;
    escapes = loadedEscapes < 0 ? 0 : loadedEscapes;
    deaths = loadedDeaths < 0 ? 0 : loadedDeaths;
    enemiesKilled = loadedEnemiesKilled < 0 ? 0 : loadedEnemiesKilled;
    bossesKilled = loadedBossesKilled < 0 ? 0 : loadedBossesKilled;
    pvpVictories = loadedPvpVictories < 0 ? 0 : loadedPvpVictories;
    pvpDefeats = loadedPvpDefeats < 0 ? 0 : loadedPvpDefeats;
}

// EN: recordCombatStarted declares or implements a focused behavior used by this module.
// FR: recordCombatStarted déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordCombatStarted()
{
    combatsStarted++;
    resetClassSkillCooldown();
    recordCurrentEquipmentUsage();
}

// EN: recordVictory declares or implements a focused behavior used by this module.
// FR: recordVictory déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordVictory()
{
    victories++;
    reduceWorldGazeDurationAfterCombat();
}

// EN: recordDefeat declares or implements a focused behavior used by this module.
// FR: recordDefeat déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordDefeat()
{
    defeats++;
    reduceWorldGazeDurationAfterCombat();
}

// EN: recordEscape declares or implements a focused behavior used by this module.
// FR: recordEscape déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordEscape()
{
    escapes++;
    reduceWorldGazeDurationAfterCombat();
}

// EN: recordDeath declares or implements a focused behavior used by this module.
// FR: recordDeath déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordDeath()
{
    deaths++;
}

// EN: recordEnemyKills declares or implements a focused behavior used by this module.
// FR: recordEnemyKills déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordEnemyKills(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    enemiesKilled += amount;
    recordGameplaySkillProgressForKills(amount);
}

// EN: recordBossKill declares or implements a focused behavior used by this module.
// FR: recordBossKill déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordBossKill()
{
    bossesKilled++;
}

// EN: recordPvpVictory declares or implements a focused behavior used by this module.
// FR: recordPvpVictory déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordPvpVictory()
{
    pvpVictories++;
}

// EN: recordPvpDefeat declares or implements a focused behavior used by this module.
// FR: recordPvpDefeat déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordPvpDefeat()
{
    pvpDefeats++;
}

// EN: recordPvpLethalElimination declares or implements a focused behavior used by this module.
// FR: recordPvpLethalElimination déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordPvpLethalElimination(const std::string& characterName, const std::string& accountName)
{
    std::string label = characterName + " | compte : " + accountName;
    pvpLethalEliminations.push_back(label);
}

// EN: setLoadedPvpLethalEliminations declares or implements a focused behavior used by this module.
// FR: setLoadedPvpLethalEliminations déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedPvpLethalEliminations(const std::vector<std::string>& eliminations)
{
    pvpLethalEliminations = eliminations;
}

// EN: displayCareerStatistics declares or implements a focused behavior used by this module.
// FR: displayCareerStatistics déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayCareerStatistics(DifficultyMode difficulty) const
{
    std::cout << "===== STATISTIQUES DE PARCOURS =====" << std::endl;
    std::cout << "Combats lancés : " << combatsStarted << std::endl;
    std::cout << "Victoires : " << victories << std::endl;
    std::cout << "Défaites : " << defeats << std::endl;
    std::cout << "Fuites : " << escapes << std::endl;

    if (difficulty == DifficultyMode::Lethal)
    {
        std::cout << "Morts du personnage : [STATISTIQUE CORROMPUE]" << std::endl;
        std::cout << "But de mission : survivre." << std::endl;
        std::cout << "Vous ne deviez pas mourir." << std::endl;
    }
    else
    {
        std::cout << "Morts du personnage : " << deaths << std::endl;
    }

    std::cout << "Ennemis tués : " << enemiesKilled << std::endl;
    std::cout << "Boss vaincus : " << bossesKilled << std::endl;
    std::cout << "JcJ remportés : " << pvpVictories << std::endl;
    std::cout << "JcJ perdus : " << pvpDefeats << std::endl;

    if (!pvpLethalEliminations.empty())
    {
        std::cout << "Éliminations JcJ Léthal :" << std::endl;

        for (const std::string& elimination : pvpLethalEliminations)
        {
            std::cout << "- " << elimination << std::endl;
        }
    }
    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}

// EN: isAlteredByCheats declares or implements a focused behavior used by this module.
// FR: isAlteredByCheats déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isAlteredByCheats() const
{
    return alteredByCheats;
}

// EN: isGodModeEnabled declares or implements a focused behavior used by this module.
// FR: isGodModeEnabled déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isGodModeEnabled() const
{
    return godModeEnabled;
}

// EN: hasInfiniteConsumables declares or implements a focused behavior used by this module.
// FR: hasInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasInfiniteConsumables() const
{
    return infiniteConsumablesEnabled;
}

// EN: hasIndestructibleEquipment declares or implements a focused behavior used by this module.
// FR: hasIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasIndestructibleEquipment() const
{
    return indestructibleEquipmentEnabled;
}

// EN: hasEquipmentProtection declares or implements a focused behavior used by this module.
// FR: hasEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasEquipmentProtection() const
{
    return equipmentProtectionEnabled;
}

// EN: hasStorySkip declares or implements a focused behavior used by this module.
// FR: hasStorySkip déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasStorySkip() const
{
    return storySkipEnabled;
}

// EN: hasSpecialChallengeAccess declares or implements a focused behavior used by this module.
// FR: hasSpecialChallengeAccess déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasSpecialChallengeAccess() const
{
    return specialChallengeAccessUnlocked;
}

// EN: getRefundUsesRemaining declares or implements a focused behavior used by this module.
// FR: getRefundUsesRemaining déclare ou implémente un comportement précis utilisé par ce module.
int Player::getRefundUsesRemaining() const
{
    return refundUsesRemaining;
}


// EN: isGodModeKnown declares or implements a focused behavior used by this module.
// FR: isGodModeKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isGodModeKnown() const
{
    return godModeKnown;
}

// EN: isInfiniteConsumablesKnown declares or implements a focused behavior used by this module.
// FR: isInfiniteConsumablesKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isInfiniteConsumablesKnown() const
{
    return infiniteConsumablesKnown;
}

// EN: isIndestructibleEquipmentKnown declares or implements a focused behavior used by this module.
// FR: isIndestructibleEquipmentKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isIndestructibleEquipmentKnown() const
{
    return indestructibleEquipmentKnown;
}

// EN: isEquipmentProtectionKnown declares or implements a focused behavior used by this module.
// FR: isEquipmentProtectionKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isEquipmentProtectionKnown() const
{
    return equipmentProtectionKnown;
}

// EN: isStorySkipKnown declares or implements a focused behavior used by this module.
// FR: isStorySkipKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isStorySkipKnown() const
{
    return storySkipKnown;
}

// EN: isSpecialChallengeAccessKnown declares or implements a focused behavior used by this module.
// FR: isSpecialChallengeAccessKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isSpecialChallengeAccessKnown() const
{
    return specialChallengeAccessKnown;
}

// EN: isCreatorMessageKnown declares or implements a focused behavior used by this module.
// FR: isCreatorMessageKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isCreatorMessageKnown() const
{
    return creatorMessageKnown;
}

// EN: getGoldCheatUseCount declares or implements a focused behavior used by this module.
// FR: getGoldCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getGoldCheatUseCount() const
{
    return goldCheatUseCount;
}

// EN: getLevelCheatUseCount declares or implements a focused behavior used by this module.
// FR: getLevelCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getLevelCheatUseCount() const
{
    return levelCheatUseCount;
}

// EN: getMaxLevelCheatUseCount declares or implements a focused behavior used by this module.
// FR: getMaxLevelCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getMaxLevelCheatUseCount() const
{
    return maxLevelCheatUseCount;
}

// EN: getRefundCheatUseCount declares or implements a focused behavior used by this module.
// FR: getRefundCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getRefundCheatUseCount() const
{
    return refundCheatUseCount;
}

// EN: getResetCheatUseCount declares or implements a focused behavior used by this module.
// FR: getResetCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getResetCheatUseCount() const
{
    return resetCheatUseCount;
}

// EN: getSwitchClassCheatUseCount declares or implements a focused behavior used by this module.
// FR: getSwitchClassCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getSwitchClassCheatUseCount() const
{
    return switchClassCheatUseCount;
}

void Player::setCheatState(
    bool altered,
    bool godMode,
    bool infiniteConsumables,
    bool indestructibleEquipment,
    bool equipmentProtection,
    bool storySkip,
    bool specialChallengeAccess,
    int refundUses
)
{
    alteredByCheats = altered;
    godModeEnabled = godMode;
    infiniteConsumablesEnabled = infiniteConsumables;
    indestructibleEquipmentEnabled = indestructibleEquipment;
    equipmentProtectionEnabled = equipmentProtection;
    storySkipEnabled = storySkip;
    specialChallengeAccessUnlocked = specialChallengeAccess;

    if (godMode)
    {
        godModeKnown = true;
    }

    if (infiniteConsumables)
    {
        infiniteConsumablesKnown = true;
    }

    if (indestructibleEquipment)
    {
        indestructibleEquipmentKnown = true;
    }

    if (equipmentProtection)
    {
        equipmentProtectionKnown = true;
    }

    if (storySkip)
    {
        storySkipKnown = true;
    }

    if (specialChallengeAccess)
    {
        specialChallengeAccessKnown = true;
    }

    if (refundUses < 0)
    {
        refundUses = 0;
    }

    if (refundUses > 3)
    {
        refundUses = 3;
    }

    refundUsesRemaining = refundUses;
}


void Player::setCheatKnowledgeState(
    bool godModeWasKnown,
    bool infiniteConsumablesWasKnown,
    bool indestructibleEquipmentWasKnown,
    bool equipmentProtectionWasKnown,
    bool storySkipWasKnown,
    bool specialChallengeAccessWasKnown,
    bool creatorMessageWasKnown
)
{
    godModeKnown = godModeWasKnown;
    infiniteConsumablesKnown = infiniteConsumablesWasKnown;
    indestructibleEquipmentKnown = indestructibleEquipmentWasKnown;
    equipmentProtectionKnown = equipmentProtectionWasKnown;
    storySkipKnown = storySkipWasKnown;
    specialChallengeAccessKnown = specialChallengeAccessWasKnown;
    creatorMessageKnown = creatorMessageWasKnown;

    if (godModeEnabled) godModeKnown = true;
    if (infiniteConsumablesEnabled) infiniteConsumablesKnown = true;
    if (indestructibleEquipmentEnabled) indestructibleEquipmentKnown = true;
    if (equipmentProtectionEnabled) equipmentProtectionKnown = true;
    if (storySkipEnabled) storySkipKnown = true;
}

// EN: markAsAlteredByCheats declares or implements a focused behavior used by this module.
// FR: markAsAlteredByCheats déclare ou implémente un comportement précis utilisé par ce module.
void Player::markAsAlteredByCheats()
{
    alteredByCheats = true;
}

// EN: toggleGodMode declares or implements a focused behavior used by this module.
// FR: toggleGodMode déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleGodMode()
{
    alteredByCheats = true;
    godModeKnown = true;
    godModeEnabled = !godModeEnabled;
    return godModeEnabled;
}

// EN: toggleInfiniteConsumables declares or implements a focused behavior used by this module.
// FR: toggleInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleInfiniteConsumables()
{
    alteredByCheats = true;
    infiniteConsumablesKnown = true;
    infiniteConsumablesEnabled = !infiniteConsumablesEnabled;
    return infiniteConsumablesEnabled;
}

// EN: toggleIndestructibleEquipment declares or implements a focused behavior used by this module.
// FR: toggleIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleIndestructibleEquipment()
{
    alteredByCheats = true;
    indestructibleEquipmentKnown = true;
    indestructibleEquipmentEnabled = !indestructibleEquipmentEnabled;
    return indestructibleEquipmentEnabled;
}

// EN: toggleEquipmentProtection declares or implements a focused behavior used by this module.
// FR: toggleEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleEquipmentProtection()
{
    alteredByCheats = true;
    equipmentProtectionKnown = true;
    equipmentProtectionEnabled = !equipmentProtectionEnabled;
    return equipmentProtectionEnabled;
}

// EN: toggleStorySkip declares or implements a focused behavior used by this module.
// FR: toggleStorySkip déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleStorySkip()
{
    alteredByCheats = true;
    storySkipKnown = true;
    storySkipEnabled = !storySkipEnabled;
    return storySkipEnabled;
}

// EN: unlockSpecialChallengeAccess declares or implements a focused behavior used by this module.
// FR: unlockSpecialChallengeAccess déclare ou implémente un comportement précis utilisé par ce module.
void Player::unlockSpecialChallengeAccess()
{
    alteredByCheats = true;
    specialChallengeAccessKnown = true;
    specialChallengeAccessUnlocked = true;
}

// EN: unlockBossRegistryExceptFinal declares or implements a focused behavior used by this module.
// FR: unlockBossRegistryExceptFinal déclare ou implémente un comportement précis utilisé par ce module.
void Player::unlockBossRegistryExceptFinal(int maximumBossId, int finalBossId)
{
    for (int id = 1; id <= maximumBossId; ++id)
    {
        if (id == finalBossId)
        {
            continue;
        }

        if (!isBossUnlocked(id))
        {
            unlockedBossIds.push_back(id);
        }
    }
}

// EN: enableGodMode declares or implements a focused behavior used by this module.
// FR: enableGodMode déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableGodMode()
{
    alteredByCheats = true;
    godModeKnown = true;
    godModeEnabled = true;
}

// EN: enableInfiniteConsumables declares or implements a focused behavior used by this module.
// FR: enableInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableInfiniteConsumables()
{
    alteredByCheats = true;
    infiniteConsumablesKnown = true;
    infiniteConsumablesEnabled = true;
}

// EN: enableIndestructibleEquipment declares or implements a focused behavior used by this module.
// FR: enableIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableIndestructibleEquipment()
{
    alteredByCheats = true;
    indestructibleEquipmentKnown = true;
    indestructibleEquipmentEnabled = true;
}

// EN: enableEquipmentProtection declares or implements a focused behavior used by this module.
// FR: enableEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableEquipmentProtection()
{
    alteredByCheats = true;
    equipmentProtectionKnown = true;
    equipmentProtectionEnabled = true;
}

// EN: enableStorySkip declares or implements a focused behavior used by this module.
// FR: enableStorySkip déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableStorySkip()
{
    alteredByCheats = true;
    storySkipKnown = true;
    storySkipEnabled = true;
}

// EN: markCreatorMessageSeen declares or implements a focused behavior used by this module.
// FR: markCreatorMessageSeen déclare ou implémente un comportement précis utilisé par ce module.
void Player::markCreatorMessageSeen()
{
    alteredByCheats = true;
    creatorMessageKnown = true;
}

// EN: recordGoldCheatUse declares or implements a focused behavior used by this module.
// FR: recordGoldCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGoldCheatUse()
{
    alteredByCheats = true;
    goldCheatUseCount++;
}

// EN: recordLevelCheatUse declares or implements a focused behavior used by this module.
// FR: recordLevelCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordLevelCheatUse()
{
    alteredByCheats = true;
    levelCheatUseCount++;
}

// EN: recordMaxLevelCheatUse declares or implements a focused behavior used by this module.
// FR: recordMaxLevelCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordMaxLevelCheatUse()
{
    alteredByCheats = true;
    maxLevelCheatUseCount++;
}

// EN: recordRefundCheatUse declares or implements a focused behavior used by this module.
// FR: recordRefundCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordRefundCheatUse()
{
    alteredByCheats = true;
    refundCheatUseCount++;
}

// EN: recordResetCheatUse declares or implements a focused behavior used by this module.
// FR: recordResetCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordResetCheatUse()
{
    alteredByCheats = true;
    resetCheatUseCount++;
}

// EN: recordSwitchClassCheatUse declares or implements a focused behavior used by this module.
// FR: recordSwitchClassCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordSwitchClassCheatUse()
{
    alteredByCheats = true;
    switchClassCheatUseCount++;
}

// EN: consumeRefundUse declares or implements a focused behavior used by this module.
// FR: consumeRefundUse déclare ou implémente un comportement précis utilisé par ce module.
bool Player::consumeRefundUse()
{
    alteredByCheats = true;

    if (refundUsesRemaining <= 0)
    {
        return false;
    }

    refundUsesRemaining--;
    return true;
}

// EN: forceLevelToMaximum declares or implements a focused behavior used by this module.
// FR: forceLevelToMaximum déclare ou implémente un comportement précis utilisé par ce module.
void Player::forceLevelToMaximum()
{
    alteredByCheats = true;

    while (level < MAX_LEVEL)
    {
        levelUp();
    }

    experience = 0;
}

// EN: gainOneLevelByCheat declares or implements a focused behavior used by this module.
// FR: gainOneLevelByCheat déclare ou implémente un comportement précis utilisé par ce module.
void Player::gainOneLevelByCheat()
{
    alteredByCheats = true;
    levelUp();
}

// EN: takeDamage declares or implements a focused behavior used by this module.
// FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
void Player::takeDamage(int damage)
{
    if (godModeEnabled && damage > 0)
    {
        std::cout << name << " devrait perdre " << damage << " PV, mais le mode god refuse la réalité." << std::endl;
        std::cout << std::endl;
        return;
    }

    Entity::takeDamage(damage);
}

void Player::applyFlatStatBonus(
    int maxHpBonus,
    int minDamageBonus,
    int maxDamageBonus,
    int criticalDamageBonus
)
{
    maxHp += maxHpBonus;
    hp += maxHpBonus;

    minDamage += minDamageBonus;
    maxDamage += maxDamageBonus;
    this->criticalDamage += criticalDamageBonus;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp < 1)
    {
        hp = 1;
    }

    if (minDamage < 0)
    {
        minDamage = 0;
    }

    if (maxDamage < minDamage)
    {
        maxDamage = minDamage;
    }

    if (this->criticalDamage < maxDamage)
    {
        this->criticalDamage = maxDamage;
    }
}

// EN: getInventory declares or implements a focused behavior used by this module.
// FR: getInventory déclare ou implémente un comportement précis utilisé par ce module.
Inventory& Player::getInventory()
{
    return inventory;
}

// EN: getInventory declares or implements a focused behavior used by this module.
// FR: getInventory déclare ou implémente un comportement précis utilisé par ce module.
const Inventory& Player::getInventory() const
{
    return inventory;
}

// EN: getQuestLog declares or implements a focused behavior used by this module.
// FR: getQuestLog déclare ou implémente un comportement précis utilisé par ce module.
QuestLog& Player::getQuestLog()
{
    return questLog;
}

// EN: getQuestLog declares or implements a focused behavior used by this module.
// FR: getQuestLog déclare ou implémente un comportement précis utilisé par ce module.
const QuestLog& Player::getQuestLog() const
{
    return questLog;
}

// EN: getEquippedWeaponIndex declares or implements a focused behavior used by this module.
// FR: getEquippedWeaponIndex déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEquippedWeaponIndex() const
{
    return equippedWeaponIndex;
}

// EN: hasEquippedWeapon declares or implements a focused behavior used by this module.
// FR: hasEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasEquippedWeapon() const
{
    return inventory.hasWeapon(equippedWeaponIndex);
}

// EN: getEquippedWeapon declares or implements a focused behavior used by this module.
// FR: getEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon Player::getEquippedWeapon() const
{
    if (!hasEquippedWeapon())
    {
        return Weapon();
    }

    return inventory.getWeapon(equippedWeaponIndex);
}

// EN: equipWeapon declares or implements a focused behavior used by this module.
// FR: equipWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::equipWeapon(int index)
{
    if (!inventory.hasWeapon(index))
    {
        return false;
    }

    equippedWeaponIndex = index;
    return true;
}

// EN: unequipWeapon declares or implements a focused behavior used by this module.
// FR: unequipWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Player::unequipWeapon()
{
    equippedWeaponIndex = -1;
}

// EN: getEquippedArmorIndex declares or implements a focused behavior used by this module.
// FR: getEquippedArmorIndex déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEquippedArmorIndex() const
{
    return equippedArmorIndex;
}

// EN: hasEquippedArmor declares or implements a focused behavior used by this module.
// FR: hasEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasEquippedArmor() const
{
    return inventory.hasArmor(equippedArmorIndex);
}

// EN: getEquippedArmor declares or implements a focused behavior used by this module.
// FR: getEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor Player::getEquippedArmor() const
{
    if (!hasEquippedArmor())
    {
        return Armor();
    }

    return inventory.getArmor(equippedArmorIndex);
}

// EN: equipArmor declares or implements a focused behavior used by this module.
// FR: equipArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::equipArmor(int index)
{
    if (!inventory.hasArmor(index))
    {
        return false;
    }

    int oldMaxHealthBonus = getEquippedArmorMaxHpBonus();

    equippedArmorIndex = index;

    int newMaxHealthBonus = getEquippedArmorMaxHpBonus();
    int maxHealthDifference = newMaxHealthBonus - oldMaxHealthBonus;

    maxHp += maxHealthDifference;
    hp += maxHealthDifference;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp < 1)
    {
        hp = 1;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }

    return true;
}

// EN: unequipArmor declares or implements a focused behavior used by this module.
// FR: unequipArmor déclare ou implémente un comportement précis utilisé par ce module.
void Player::unequipArmor()
{
    int oldMaxHealthBonus = getEquippedArmorMaxHpBonus();

    equippedArmorIndex = -1;

    maxHp -= oldMaxHealthBonus;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

// EN: initializeStarterInventory declares or implements a focused behavior used by this module.
// FR: initializeStarterInventory déclare ou implémente un comportement précis utilisé par ce module.
void Player::initializeStarterInventory()
{
    initializeStarterInventory(DifficultyMode::Normal);
}

// EN: initializeStarterInventory declares or implements a focused behavior used by this module.
// FR: initializeStarterInventory déclare ou implémente un comportement précis utilisé par ce module.
void Player::initializeStarterInventory(DifficultyMode difficulty)
{
    inventory.addWeapon(WeaponCatalog::createBareHands());
    recordStarterKitEntry("Arme de base : Mains nues");

    Weapon classStarterWeapon = WeaponCatalog::createStarterWeaponForClass(type);
    inventory.addWeapon(classStarterWeapon);
    recordStarterKitEntry("Arme de classe : " + classStarterWeapon.getName());

    if (isDistanceStarterClass(type))
    {
        Weapon emergencyKnife = WeaponCatalog::createEmergencyWoodKnife();
        inventory.addWeapon(emergencyKnife);
        recordStarterKitEntry("Défense urgente : " + emergencyKnife.getName());
        std::cout << "Équipement de secours : classe à distance détectée, petit couteau de bois ajouté." << std::endl;
    }

    if (usesStarterAmmunition(type))
    {
        if (type.find("Arbal") != std::string::npos || type.find("arbal") != std::string::npos)
        {
            inventory.addMaterial(MaterialCatalog::createById("training_bolts", 14));
            recordStarterKitEntry("Munitions de départ : carreaux d'entraînement x14");
            std::cout << "Munitions de départ : carreaux d'entraînement x14." << std::endl;
        }
        else if (type.find("dagues") != std::string::npos || type.find("Dagues") != std::string::npos)
        {
            inventory.addMaterial(MaterialCatalog::createById("training_throwing_knives", 10));
            recordStarterKitEntry("Munitions de départ : couteaux de lancer émoussés x10");
            std::cout << "Munitions de départ : couteaux de lancer émoussés x10." << std::endl;
        }
        else
        {
            inventory.addMaterial(MaterialCatalog::createById("training_arrows", 18));
            recordStarterKitEntry("Munitions de départ : flèches d'entraînement x18");
            std::cout << "Munitions de départ : flèches d'entraînement x18." << std::endl;
        }

        std::cout << "Les recettes de munitions spéciales se découvrent par exploration, expérience et expérimentation." << std::endl;
    }

    Weapon* starterWeapon = inventory.getMutableWeapon(1);

    if (starterWeapon != nullptr)
    {
        starterWeapon->loseDurability(
            DifficultyRules::getStarterWeaponDurabilityLoss(difficulty)
        );
    }

    equipWeapon(1);

    Armor simpleOutfit = ArmorCatalog::createSimpleOutfit();
    inventory.addArmor(simpleOutfit);
    recordStarterKitEntry("Tenue de base : " + simpleOutfit.getName());

    Armor classStarterArmor = ArmorCatalog::createStarterArmorForClass(type);
    inventory.addArmor(classStarterArmor);
    recordStarterKitEntry("Protection de classe : " + classStarterArmor.getName());

    Armor* starterArmor = inventory.getMutableArmor(1);

    if (starterArmor != nullptr)
    {
        starterArmor->loseDurability(
            DifficultyRules::getStarterArmorDurabilityLoss(difficulty)
        );
    }

    equipArmor(1);

    int starterHealingPotions =
        DifficultyRules::getStarterHealingPotionCount(
            healingPotionCount,
            difficulty
        );

    int starterDamagePotions =
        DifficultyRules::getStarterDamagePotionCount(
            damagePotionCount,
            difficulty
        );

    for (int i = 0; i < starterHealingPotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicHealingPotion());
    }
    if (starterHealingPotions > 0)
    {
        recordStarterKitEntry("Potions de soin de départ x" + std::to_string(starterHealingPotions));
    }

    for (int i = 0; i < starterDamagePotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicDamagePotion());
    }
    if (starterDamagePotions > 0)
    {
        recordStarterKitEntry("Potions de rage de départ x" + std::to_string(starterDamagePotions));
    }

    int starterGold = DifficultyRules::getStarterGold(difficulty);
    inventory.earnGold(starterGold);
    if (starterGold > 0)
    {
        recordStarterKitEntry("Or de départ : " + std::to_string(starterGold));
    }
}


std::vector<std::string> Player::applyHeavyVersionAdaptation(DifficultyMode difficulty)
{
    (void)difficulty;

    std::vector<std::string> changes;

    if (starterKitLog.empty())
    {
        starterKitLog.push_back("Kit de départ original inconnu : sauvegarde créée avant le journal de départ.");
        changes.push_back("Journal du kit de départ initialisé en mode sauvegarde ancienne.");
    }

    if (!inventoryHasWeaponNamed(inventory, "Mains nues"))
    {
        inventory.addWeapon(WeaponCatalog::createBareHands());
        changes.push_back("Mains nues restaurées comme base de secours.");
    }

    if (isDistanceStarterClass(type) && !inventoryHasWeaponNamed(inventory, "Couteau de bois d'urgence"))
    {
        inventory.addWeapon(WeaponCatalog::createEmergencyWoodKnife());
        changes.push_back("Couteau de bois d'urgence ajouté pour classe à distance.");
    }

    if (usesStarterAmmunition(type))
    {
        if ((type.find("Arbal") != std::string::npos || type.find("arbal") != std::string::npos)
            && !inventoryHasMaterialId(inventory, "training_bolts"))
        {
            inventory.addMaterial(MaterialCatalog::createById("training_bolts", 8));
            changes.push_back("Carreaux d'entraînement x8 ajoutés pour adaptation.");
        }
        else if ((type.find("dagues") != std::string::npos || type.find("Dagues") != std::string::npos)
            && !inventoryHasMaterialId(inventory, "training_throwing_knives"))
        {
            inventory.addMaterial(MaterialCatalog::createById("training_throwing_knives", 6));
            changes.push_back("Couteaux de lancer émoussés x6 ajoutés pour adaptation.");
        }
        else if (!inventoryHasMaterialId(inventory, "training_arrows"))
        {
            inventory.addMaterial(MaterialCatalog::createById("training_arrows", 10));
            changes.push_back("Flèches d'entraînement x10 ajoutées pour adaptation.");
        }
    }

    Armor expectedArmor = ArmorCatalog::createStarterArmorForClass(type);
    if (inventory.getArmorCount() == 0)
    {
        inventory.addArmor(ArmorCatalog::createSimpleOutfit());
        inventory.addArmor(expectedArmor);
        equipArmor(1);
        changes.push_back("Protection de classe ajoutée car aucune armure n'était présente.");
    }
    else if (equippedArmorIndex < 0 && !inventoryHasArmorNamed(inventory, expectedArmor.getName()))
    {
        inventory.addArmor(expectedArmor);
        changes.push_back("Protection de classe ajoutée sans remplacer l'équipement existant.");
    }

    if (inventory.getWeaponCount() <= 1)
    {
        Weapon expectedWeapon = WeaponCatalog::createStarterWeaponForClass(type);
        if (!inventoryHasWeaponNamed(inventory, expectedWeapon.getName()))
        {
            inventory.addWeapon(expectedWeapon);
            changes.push_back("Arme de classe basique ajoutée car l'arsenal était presque vide.");
        }
    }

    if (inventory.countConsumables(ConsumableType::Healing) == 0)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicHealingPotion());
        changes.push_back("Potion de soin ajoutée pour éviter une ancienne sauvegarde sans sécurité.");
    }

    if (changes.empty())
    {
        changes.push_back("Aucun objet ajouté : l'équipement actuel semblait déjà compatible.");
    }

    for (const std::string& change : changes)
    {
        starterKitLog.push_back("Adaptation V" + VersionInfo::currentVersion() + " : " + change);
    }

    markAdaptedToCurrentVersion();
    return changes;
}

// EN: destroyEquippedWeapon declares or implements a focused behavior used by this module.
// FR: destroyEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::destroyEquippedWeapon()
{
    if (!hasEquippedWeapon())
    {
        return false;
    }

    int index = equippedWeaponIndex;
    equippedWeaponIndex = -1;

    return inventory.removeWeapon(index);
}

// EN: destroyEquippedArmor declares or implements a focused behavior used by this module.
// FR: destroyEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::destroyEquippedArmor()
{
    if (!hasEquippedArmor())
    {
        return false;
    }

    Armor armor = getEquippedArmor();

    if (armor.getName() == "Tenue simple")
    {
        return false;
    }

    int index = equippedArmorIndex;
    unequipArmor();

    return inventory.removeArmor(index);
}

// EN: gainExperience declares or implements a focused behavior used by this module.
// FR: gainExperience déclare ou implémente un comportement précis utilisé par ce module.
void Player::gainExperience(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    experience += amount;

    while (level < MAX_LEVEL)
    {
        int experienceRequired = Level::getExperienceRequiredForNextLevel(level);

        if (experienceRequired <= 0 || experience < experienceRequired)
        {
            break;
        }

        experience -= experienceRequired;
        levelUp();
    }

    if (level >= MAX_LEVEL)
    {
        level = MAX_LEVEL;
        experience = 0;
    }
}

// EN: loseExperience declares or implements a focused behavior used by this module.
// FR: loseExperience déclare ou implémente un comportement précis utilisé par ce module.
void Player::loseExperience(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    experience -= amount;

    if (experience < 0)
    {
        experience = 0;
    }
}

// EN: levelUp declares or implements a focused behavior used by this module.
// FR: levelUp déclare ou implémente un comportement précis utilisé par ce module.
void Player::levelUp()
{
    if (level >= MAX_LEVEL)
    {
        level = MAX_LEVEL;
        experience = 0;

        std::cout << name << " est déjà au niveau maximum : 255." << std::endl;
        std::cout << "Tous les bits sont à 1. Plus haut, ce serait de la triche... enfin presque." << std::endl;
        std::cout << std::endl;
        return;
    }

    level++;

    maxHp += 20;
    hp = maxHp;

    minDamage += 1;
    maxDamage += 2;
    criticalDamage += 3;

    std::cout << name << " monte au niveau " << level << " !" << std::endl;
    std::cout << "Ses blessures se referment, et sa puissance augmente." << std::endl;
    std::cout << "Les attributs avancés dorment encore dans les registres du personnage." << std::endl;
    std::cout << std::endl;

    refreshLevelAndIdentitySkills();
}

// EN: attack declares or implements a focused behavior used by this module.
// FR: attack déclare ou implémente un comportement précis utilisé par ce module.
int Player::attack(Random& random, bool& dodged, bool& critical, int damageBonus)
{
    int resultat = random.rollD20();

    dodged = false;
    critical = false;

    if (shockTurns > 0 && random.between(1, 100) <= 18)
    {
        dodged = true;
        clearLastConsumedAmmunition();
        clearNextAmmunitionChoice();
        std::cout << name << " est perturbé par le choc électrique et rate son geste." << std::endl;
        return 0;
    }

    int dodgeThreshold = 3;
    int normalHitThreshold = 16;
    int frostDamagePercent = 100;

    if (frostTurns > 0)
    {
        dodgeThreshold += 1;
        normalHitThreshold += 1;
        frostDamagePercent = 85;
        std::cout << name << " attaque avec des gestes ralentis par le froid." << std::endl;
    }

    int bonusMin = 0;
    int bonusMax = 0;
    int criticalBonus = 0;

    Weapon* equippedWeapon = inventory.getMutableWeapon(equippedWeaponIndex);
    clearLastConsumedAmmunition();

    bool bowWithoutAmmo = false;

    if (equippedWeapon != nullptr && !equippedWeapon->isBroken() && !bossEquipmentSealActive)
    {
        bonusMin = equippedWeapon->getMinDamageBonus();
        bonusMax = equippedWeapon->getMaxDamageBonus();
        criticalBonus = equippedWeapon->getCriticalBonus();

        if (equippedWeapon->getType() == WeaponType::Bow)
        {
            std::string ammoId = "training_arrows";
            std::string weaponText = equippedWeapon->getName();
            std::transform(weaponText.begin(), weaponText.end(), weaponText.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            std::string specialAmmoId = "barbed_arrows";
            std::string elementalAmmoId = "ash_arrows";
            if (weaponText.find("arbal") != std::string::npos || weaponText.find("carreau") != std::string::npos)
            {
                ammoId = "training_bolts";
                specialAmmoId = "piercing_bolts";
                elementalAmmoId = "frozen_bolts";
            }
            else if (weaponText.find("lancer") != std::string::npos || weaponText.find("couteau") != std::string::npos || weaponText.find("bandouli") != std::string::npos)
            {
                ammoId = "training_throwing_knives";
                specialAmmoId = "balanced_throwing_knives";
                elementalAmmoId = "conductive_knives";
            }

            std::string consumedAmmoId = ammoId;
            bool specialAmmo = false;

            bool forcedNoAmmo = false;

            if (!nextAmmunitionChoiceId.empty())
            {
                if (nextAmmunitionChoiceId == "__cancel_attack__")
                {
                    clearNextAmmunitionChoice();
                    dodged = true;
                    std::cout << name << " annule son tir et garde sa munition." << std::endl;
                    return 0;
                }

                if (nextAmmunitionChoiceId == "__no_ammo__" || nextAmmunitionChoiceId == "__emergency_defense__")
                {
                    forcedNoAmmo = true;
                }
                else
                {
                    consumedAmmoId = nextAmmunitionChoiceId;
                    specialAmmo = consumedAmmoId != ammoId;
                }
            }

            clearNextAmmunitionChoice();

            if (!forcedNoAmmo && inventory.countMaterialById(consumedAmmoId) > 0)
            {
                if (!infiniteConsumablesEnabled)
                {
                    inventory.removeMaterialQuantityById(consumedAmmoId, 1);
                }

                setLastConsumedAmmunition(consumedAmmoId);

                if (specialAmmo)
                {
                    if (consumedAmmoId == elementalAmmoId)
                    {
                        bonusMin += 3;
                        bonusMax += 5;
                        criticalBonus += 4;
                        std::cout << name << " consomme une munition élémentaire choisie pour attaquer à distance." << std::endl;
                    }
                    else
                    {
                        bonusMin += 2;
                        bonusMax += 4;
                        criticalBonus += 5;
                        std::cout << name << " consomme une munition spéciale choisie pour attaquer à distance." << std::endl;
                    }
                }
                else
                {
                    std::cout << name << " consomme une munition d'entraînement pour attaquer à distance." << std::endl;
                }
            }
            else
            {
                bowWithoutAmmo = true;
                bonusMin = 0;
                bonusMax = 1;
                criticalBonus = 0;

                setLastConsumedAmmunition("__emergency_defense__");
                std::cout << name << " n'a pas de munition adaptée pour ce tir." << std::endl;
                std::cout << "Défense d'urgence : aucun projectile n'est tiré. Les dégâts représentent seulement un coup de poignée, de branche ou de crosse à très courte portée." << std::endl;
            }
        }
    }

    if (equippedWeapon != nullptr && !equippedWeapon->isBroken() && bossEquipmentSealActive)
    {
        std::cout << "Le sceau de boss bloque les bonus de " << equippedWeapon->getName() << "." << std::endl;
        if (!bossEquipmentSealReason.empty())
        {
            std::cout << bossEquipmentSealReason << std::endl;
        }
        std::cout << std::endl;
    }

    if (resultat <= dodgeThreshold)
    {
        dodged = true;
        return 0;
    }

    if (equippedWeapon != nullptr && !indestructibleEquipmentEnabled)
    {
        equippedWeapon->loseDurability(1);
    }

    if (equippedWeapon != nullptr && equippedWeapon->isBroken())
    {
        std::cout << "L'arme de " << name << " s'abîme sous le choc..." << std::endl;
        std::cout << equippedWeapon->getName() << " est maintenant cassée et ne donnera plus ses bonus." << std::endl;
        std::cout << std::endl;
    }

    if (resultat <= normalHitThreshold)
    {
        int dealtDamage = random.between(
            minDamage + bonusMin,
            maxDamage + bonusMax
        ) + damageBonus;

        if (bowWithoutAmmo)
        {
            dealtDamage = std::max(1, dealtDamage / 2);
        }

        if (frostDamagePercent < 100)
        {
            dealtDamage = std::max(1, dealtDamage * frostDamagePercent / 100);
        }

        return dealtDamage;
    }

    critical = true;
    int criticalResult = criticalDamage + criticalBonus + damageBonus;

    if (bowWithoutAmmo)
    {
        criticalResult = std::max(1, criticalResult / 2);
    }

    if (frostDamagePercent < 100)
    {
        criticalResult = std::max(1, criticalResult * frostDamagePercent / 100);
    }

    return criticalResult;
}

// EN: displayStats declares or implements a focused behavior used by this module.
// FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayStats() const
{
    std::cout << "===== STATS JOUEUR =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Race : " << getRaceText() << std::endl;
    std::cout << "Classe : " << type << std::endl;
    std::cout << "Niveau : " << level << std::endl;
    int nextLevelExperience = Level::getExperienceRequiredForNextLevel(level);

    if (nextLevelExperience > 0)
    {
        std::cout << "Expérience : " << experience << "/" << nextLevelExperience << std::endl;
    }
    else
    {
        std::cout << "Expérience : niveau maximum" << std::endl;
    }
    std::cout << "PV : " << hp << "/" << maxHp << std::endl;
    std::cout << "Dégâts de base : " << minDamage << " - " << maxDamage << std::endl;
    std::cout << "Critique de base : " << criticalDamage << std::endl;

    if (alteredByCheats)
    {
        std::cout << "Statut : Altéré" << std::endl;
        std::cout << "Les souvenirs de ce personnage portent déjà une trace anormale." << std::endl;
    }

    if (hasWorldGazePenalty())
    {
        std::cout << "Effet temporaire : Regard du monde" << std::endl;
        std::cout << "Les règles t'ont vu essayer de les contourner." << std::endl;
        std::cout << "Combats restants : " << worldGazeCombatsRemaining << std::endl;
        std::cout << "PV maximum retenus temporairement : " << worldGazeMaxHpPenalty << std::endl;
    }

    if (bossEquipmentSealActive)
    {
        std::cout << "Effet temporaire de boss : équipement scellé" << std::endl;
        if (!bossEquipmentSealReason.empty())
        {
            std::cout << bossEquipmentSealReason << std::endl;
        }
    }

    if (zelefCorrosionPresent)
    {
        std::cout << "Effet boss : Corrosion présente" << std::endl;
        std::cout << "Zelef garde encore " << zelefMaxHpStolen << " PV maximum quelque part dans son sang noir." << std::endl;
    }

    if (grinkaBossTheftPresent)
    {
        std::cout << "Effet boss : Volé par un boss" << std::endl;
        std::cout << "Grinka possède encore quelque chose qui appartenait à ce personnage." << std::endl;
    }

    if (!unlockedPassiveSkills.empty() || !unlockedActiveSkills.empty())
    {
        std::cout << "Compétences : "
                  << unlockedPassiveSkills.size() << " passives, "
                  << unlockedActiveSkills.size() << " actives" << std::endl;
    }

    std::cout << "Potions de soin : "
              << inventory.countConsumables(ConsumableType::Healing)
              << std::endl;

    std::cout << "Potions de rage : "
              << inventory.countConsumables(ConsumableType::Damage)
              << std::endl;

    std::cout << "Créé le : " << createdAtText
              << " | V" << createdForVersion << std::endl;
    std::cout << "Dernière adaptation faite pour la V"
              << lastAdaptedVersion << std::endl;

    std::cout << "========================" << std::endl;
    std::cout << std::endl;
}

// EN: displayInventory declares or implements a focused behavior used by this module.
// FR: displayInventory déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayInventory() const
{
    inventory.display();
    displaySimpleEquipment();
}

// EN: displaySimpleEquipment declares or implements a focused behavior used by this module.
// FR: displaySimpleEquipment déclare ou implémente un comportement précis utilisé par ce module.
void Player::displaySimpleEquipment() const
{
    std::cout << "===== ÉQUIPEMENT =====" << std::endl;

    if (hasEquippedWeapon())
    {
        Weapon weapon = getEquippedWeapon();

        std::cout << "Arme équipée : " << weapon.getName();

        if (!weapon.isIndestructible())
        {
            std::cout << " (" << weapon.getDurability() << "/" << weapon.getMaxDurability() << ")";
        }

        if (weapon.isBroken())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;

        if (!weapon.isBroken() && !hasBossEquipmentSeal())
        {
            std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(
                *this,
                weapon.getType(),
                weapon.getName()
            );

            if (!affinityLabel.empty())
            {
                std::cout << "Affinité arme/classe : active, " << affinityLabel << "." << std::endl;
            }
            else
            {
                std::cout << "Affinité arme/classe : aucune maîtrise particulière avec cette arme." << std::endl;
            }
        }
        else if (weapon.isBroken())
        {
            std::cout << "Affinité arme/classe : inactive, l'arme est cassée." << std::endl;
        }
        else if (hasBossEquipmentSeal())
        {
            std::cout << "Affinité arme/classe : bloquée par le sceau de boss." << std::endl;
        }
    }
    else
    {
        std::cout << "Arme équipée : Aucune" << std::endl;
    }

    if (hasEquippedArmor())
    {
        Armor armor = getEquippedArmor();

        std::cout << "Armure équipée : " << armor.getName();

        if (!armor.isIndestructible())
        {
            std::cout << " (" << armor.getDurability() << "/" << armor.getMaxDurability() << ")";
        }

        if (armor.isBroken())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Armure équipée : Aucune" << std::endl;
    }

    std::cout << "Or : " << inventory.getGold() << " pièces" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << std::endl;
}

// EN: displayDetailedEquipment declares or implements a focused behavior used by this module.
// FR: displayDetailedEquipment déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayDetailedEquipment() const
{
    std::cout << "========== ÉQUIPEMENT DÉTAILLÉ ==========" << std::endl;
    std::cout << std::endl;

    if (hasEquippedWeapon())
    {
        Weapon weapon = getEquippedWeapon();

        std::cout << "=== Arme équipée ===" << std::endl;
        std::cout << "Nom : " << weapon.getName() << std::endl;
        std::cout << "Description : " << weapon.getDescription() << std::endl;
        std::cout << "Bonus dégâts : +"
                  << weapon.getMinDamageBonus()
                  << " à +"
                  << weapon.getMaxDamageBonus()
                  << std::endl;
        std::cout << "Bonus critique : +" << weapon.getCriticalBonus() << std::endl;

        if (weapon.isIndestructible())
        {
            std::cout << "Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "Durabilité : " << weapon.getDurability() << "/" << weapon.getMaxDurability() << std::endl;
        }

        if (weapon.isBroken())
        {
            std::cout << "État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
            std::cout << "Affinité arme/classe : inactive, l'arme est cassée." << std::endl;
        }
        else if (hasBossEquipmentSeal())
        {
            std::cout << "État : utilisable, mais le sceau de boss bloque ses bonus." << std::endl;
            std::cout << "Affinité arme/classe : bloquée par le sceau de boss." << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;

            std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(
                *this,
                weapon.getType(),
                weapon.getName()
            );

            if (!affinityLabel.empty())
            {
                std::cout << "Affinité arme/classe : active, " << affinityLabel << "." << std::endl;
                std::cout << "Effet : très léger bonus de dégâts en combat." << std::endl;
            }
            else
            {
                std::cout << "Affinité arme/classe : aucune maîtrise particulière avec cette arme." << std::endl;
            }
        }
    }
    else
    {
        std::cout << "=== Arme équipée ===" << std::endl;
        std::cout << "Aucune arme équipée." << std::endl;
    }

    std::cout << std::endl;

    if (hasEquippedArmor())
    {
        Armor armor = getEquippedArmor();

        std::cout << "=== Armure équipée ===" << std::endl;
        std::cout << "Nom : " << armor.getName() << std::endl;
        std::cout << "Description : " << armor.getDescription() << std::endl;
        std::cout << "Bonus PV max : +" << armor.getMaxHpBonus() << std::endl;
        std::cout << "Réduction dégâts : " << armor.getDamageReduction() << std::endl;

        if (armor.isIndestructible())
        {
            std::cout << "Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "Durabilité : " << armor.getDurability() << "/" << armor.getMaxDurability() << std::endl;
        }

        if (armor.isBroken())
        {
            std::cout << "État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }
    }
    else
    {
        std::cout << "=== Armure équipée ===" << std::endl;
        std::cout << "Aucune armure équipée." << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Or : " << inventory.getGold() << " pièces" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}
