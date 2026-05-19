// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Player.hpp"

#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "character/RaceCatalog.hpp"

#include <iostream>

Player::Player() : Entity()
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
    race = CharacterRace::Human;
    unspentAttributePoints = 0;

    combatsStarted = 0;
    victories = 0;
    defeats = 0;
    escapes = 0;
    deaths = 0;
    enemiesKilled = 0;
    bossesKilled = 0;

    alteredByCheats = false;
    godModeEnabled = false;
    infiniteConsumablesEnabled = false;
    indestructibleEquipmentEnabled = false;
    equipmentProtectionEnabled = false;
    storySkipEnabled = false;

    godModeKnown = false;
    infiniteConsumablesKnown = false;
    indestructibleEquipmentKnown = false;
    equipmentProtectionKnown = false;
    storySkipKnown = false;
    creatorMessageKnown = false;

    goldCheatUseCount = 0;
    levelCheatUseCount = 0;
    maxLevelCheatUseCount = 0;
    refundCheatUseCount = 0;
    resetCheatUseCount = 0;
    switchClassCheatUseCount = 0;

    refundUsesRemaining = 3;
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
        playerClass.getDamagePotionCount()
    )
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
    race = CharacterRace::Human;
    unspentAttributePoints = 0;

    combatsStarted = 0;
    victories = 0;
    defeats = 0;
    escapes = 0;
    deaths = 0;
    enemiesKilled = 0;
    bossesKilled = 0;

    alteredByCheats = false;
    godModeEnabled = false;
    infiniteConsumablesEnabled = false;
    indestructibleEquipmentEnabled = false;
    equipmentProtectionEnabled = false;
    storySkipEnabled = false;

    godModeKnown = false;
    infiniteConsumablesKnown = false;
    indestructibleEquipmentKnown = false;
    equipmentProtectionKnown = false;
    storySkipKnown = false;
    creatorMessageKnown = false;

    goldCheatUseCount = 0;
    levelCheatUseCount = 0;
    maxLevelCheatUseCount = 0;
    refundCheatUseCount = 0;
    resetCheatUseCount = 0;
    switchClassCheatUseCount = 0;

    refundUsesRemaining = 3;
}

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

int Player::getLevel() const
{
    return level;
}

int Player::getExperience() const
{
    return experience;
}

int Player::getUnspentAttributePoints() const
{
    return unspentAttributePoints;
}

const DndAttributes& Player::getAttributes() const
{
    return attributes;
}

CharacterRace Player::getRace() const
{
    return race;
}

std::string Player::getRaceText() const
{
    return characterRaceToText(race);
}

void Player::setRace(CharacterRace selectedRace)
{
    race = selectedRace;
    applyRaceStartingBonus(selectedRace);
}


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

void Player::setLoadedAttributes(const DndAttributes& loadedAttributes, int loadedUnspentPoints)
{
    attributes = loadedAttributes;

    if (loadedUnspentPoints < 0)
    {
        loadedUnspentPoints = 0;
    }

    unspentAttributePoints = loadedUnspentPoints;
}

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


int Player::getCombatsStarted() const
{
    return combatsStarted;
}

int Player::getVictories() const
{
    return victories;
}

int Player::getDefeats() const
{
    return defeats;
}

int Player::getEscapes() const
{
    return escapes;
}

int Player::getDeaths() const
{
    return deaths;
}

int Player::getEnemiesKilled() const
{
    return enemiesKilled;
}

int Player::getBossesKilled() const
{
    return bossesKilled;
}

void Player::setLoadedStatistics(
    int loadedCombatsStarted,
    int loadedVictories,
    int loadedDefeats,
    int loadedEscapes,
    int loadedDeaths,
    int loadedEnemiesKilled,
    int loadedBossesKilled
)
{
    combatsStarted = loadedCombatsStarted < 0 ? 0 : loadedCombatsStarted;
    victories = loadedVictories < 0 ? 0 : loadedVictories;
    defeats = loadedDefeats < 0 ? 0 : loadedDefeats;
    escapes = loadedEscapes < 0 ? 0 : loadedEscapes;
    deaths = loadedDeaths < 0 ? 0 : loadedDeaths;
    enemiesKilled = loadedEnemiesKilled < 0 ? 0 : loadedEnemiesKilled;
    bossesKilled = loadedBossesKilled < 0 ? 0 : loadedBossesKilled;
}

void Player::recordCombatStarted()
{
    combatsStarted++;
}

void Player::recordVictory()
{
    victories++;
}

void Player::recordDefeat()
{
    defeats++;
}

void Player::recordEscape()
{
    escapes++;
}

void Player::recordDeath()
{
    deaths++;
}

void Player::recordEnemyKills(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    enemiesKilled += amount;
}

void Player::recordBossKill()
{
    bossesKilled++;
}

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
    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}

bool Player::isAlteredByCheats() const
{
    return alteredByCheats;
}

bool Player::isGodModeEnabled() const
{
    return godModeEnabled;
}

bool Player::hasInfiniteConsumables() const
{
    return infiniteConsumablesEnabled;
}

bool Player::hasIndestructibleEquipment() const
{
    return indestructibleEquipmentEnabled;
}

bool Player::hasEquipmentProtection() const
{
    return equipmentProtectionEnabled;
}

bool Player::hasStorySkip() const
{
    return storySkipEnabled;
}

int Player::getRefundUsesRemaining() const
{
    return refundUsesRemaining;
}


bool Player::isGodModeKnown() const
{
    return godModeKnown;
}

bool Player::isInfiniteConsumablesKnown() const
{
    return infiniteConsumablesKnown;
}

bool Player::isIndestructibleEquipmentKnown() const
{
    return indestructibleEquipmentKnown;
}

bool Player::isEquipmentProtectionKnown() const
{
    return equipmentProtectionKnown;
}

bool Player::isStorySkipKnown() const
{
    return storySkipKnown;
}

bool Player::isCreatorMessageKnown() const
{
    return creatorMessageKnown;
}

int Player::getGoldCheatUseCount() const
{
    return goldCheatUseCount;
}

int Player::getLevelCheatUseCount() const
{
    return levelCheatUseCount;
}

int Player::getMaxLevelCheatUseCount() const
{
    return maxLevelCheatUseCount;
}

int Player::getRefundCheatUseCount() const
{
    return refundCheatUseCount;
}

int Player::getResetCheatUseCount() const
{
    return resetCheatUseCount;
}

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
    int refundUses
)
{
    alteredByCheats = altered;
    godModeEnabled = godMode;
    infiniteConsumablesEnabled = infiniteConsumables;
    indestructibleEquipmentEnabled = indestructibleEquipment;
    equipmentProtectionEnabled = equipmentProtection;
    storySkipEnabled = storySkip;

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

void Player::markAsAlteredByCheats()
{
    alteredByCheats = true;
}

bool Player::toggleGodMode()
{
    alteredByCheats = true;
    godModeKnown = true;
    godModeEnabled = !godModeEnabled;
    return godModeEnabled;
}

bool Player::toggleInfiniteConsumables()
{
    alteredByCheats = true;
    infiniteConsumablesKnown = true;
    infiniteConsumablesEnabled = !infiniteConsumablesEnabled;
    return infiniteConsumablesEnabled;
}

bool Player::toggleIndestructibleEquipment()
{
    alteredByCheats = true;
    indestructibleEquipmentKnown = true;
    indestructibleEquipmentEnabled = !indestructibleEquipmentEnabled;
    return indestructibleEquipmentEnabled;
}

bool Player::toggleEquipmentProtection()
{
    alteredByCheats = true;
    equipmentProtectionKnown = true;
    equipmentProtectionEnabled = !equipmentProtectionEnabled;
    return equipmentProtectionEnabled;
}

bool Player::toggleStorySkip()
{
    alteredByCheats = true;
    storySkipKnown = true;
    storySkipEnabled = !storySkipEnabled;
    return storySkipEnabled;
}

void Player::enableGodMode()
{
    alteredByCheats = true;
    godModeKnown = true;
    godModeEnabled = true;
}

void Player::enableInfiniteConsumables()
{
    alteredByCheats = true;
    infiniteConsumablesKnown = true;
    infiniteConsumablesEnabled = true;
}

void Player::enableIndestructibleEquipment()
{
    alteredByCheats = true;
    indestructibleEquipmentKnown = true;
    indestructibleEquipmentEnabled = true;
}

void Player::enableEquipmentProtection()
{
    alteredByCheats = true;
    equipmentProtectionKnown = true;
    equipmentProtectionEnabled = true;
}

void Player::enableStorySkip()
{
    alteredByCheats = true;
    storySkipKnown = true;
    storySkipEnabled = true;
}

void Player::markCreatorMessageSeen()
{
    alteredByCheats = true;
    creatorMessageKnown = true;
}

void Player::recordGoldCheatUse()
{
    alteredByCheats = true;
    goldCheatUseCount++;
}

void Player::recordLevelCheatUse()
{
    alteredByCheats = true;
    levelCheatUseCount++;
}

void Player::recordMaxLevelCheatUse()
{
    alteredByCheats = true;
    maxLevelCheatUseCount++;
}

void Player::recordRefundCheatUse()
{
    alteredByCheats = true;
    refundCheatUseCount++;
}

void Player::recordResetCheatUse()
{
    alteredByCheats = true;
    resetCheatUseCount++;
}

void Player::recordSwitchClassCheatUse()
{
    alteredByCheats = true;
    switchClassCheatUseCount++;
}

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

void Player::forceLevelToMaximum()
{
    alteredByCheats = true;

    while (level < MAX_LEVEL)
    {
        levelUp();
    }

    experience = 0;
}

void Player::gainOneLevelByCheat()
{
    alteredByCheats = true;
    levelUp();
}

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

Inventory& Player::getInventory()
{
    return inventory;
}

const Inventory& Player::getInventory() const
{
    return inventory;
}

int Player::getEquippedWeaponIndex() const
{
    return equippedWeaponIndex;
}

bool Player::hasEquippedWeapon() const
{
    return inventory.hasWeapon(equippedWeaponIndex);
}

Weapon Player::getEquippedWeapon() const
{
    if (!hasEquippedWeapon())
    {
        return Weapon();
    }

    return inventory.getWeapon(equippedWeaponIndex);
}

bool Player::equipWeapon(int index)
{
    if (!inventory.hasWeapon(index))
    {
        return false;
    }

    equippedWeaponIndex = index;
    return true;
}

void Player::unequipWeapon()
{
    equippedWeaponIndex = -1;
}

int Player::getEquippedArmorIndex() const
{
    return equippedArmorIndex;
}

bool Player::hasEquippedArmor() const
{
    return inventory.hasArmor(equippedArmorIndex);
}

Armor Player::getEquippedArmor() const
{
    if (!hasEquippedArmor())
    {
        return Armor();
    }

    return inventory.getArmor(equippedArmorIndex);
}

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

void Player::initializeStarterInventory()
{
    initializeStarterInventory(DifficultyMode::Normal);
}

void Player::initializeStarterInventory(DifficultyMode difficulty)
{
    inventory.addWeapon(WeaponCatalog::createBareHands());
    inventory.addWeapon(WeaponCatalog::createRustySword());

    Weapon* rustySword = inventory.getMutableWeapon(1);

    if (rustySword != nullptr)
    {
        rustySword->loseDurability(
            DifficultyRules::getStarterWeaponDurabilityLoss(difficulty)
        );
    }

    equipWeapon(1);

    inventory.addArmor(ArmorCatalog::createSimpleOutfit());
    inventory.addArmor(ArmorCatalog::createWornLeatherArmor());

    Armor* wornLeatherArmor = inventory.getMutableArmor(1);

    if (wornLeatherArmor != nullptr)
    {
        wornLeatherArmor->loseDurability(
            DifficultyRules::getStarterArmorDurabilityLoss(difficulty)
        );
    }

    equipArmor(0);

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

    for (int i = 0; i < starterDamagePotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicDamagePotion());
    }

    inventory.earnGold(
        DifficultyRules::getStarterGold(difficulty)
    );
}

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
    std::cout << "Les attributs avancés sont préparés, mais pas encore actifs dans cette version." << std::endl;
    std::cout << std::endl;
}

int Player::attack(Random& random, bool& dodged, bool& critical, int damageBonus)
{
    int resultat = random.rollD20();

    dodged = false;
    critical = false;

    int bonusMin = 0;
    int bonusMax = 0;
    int criticalBonus = 0;

    Weapon* equippedWeapon = inventory.getMutableWeapon(equippedWeaponIndex);

    if (equippedWeapon != nullptr && !equippedWeapon->isBroken())
    {
        bonusMin = equippedWeapon->getMinDamageBonus();
        bonusMax = equippedWeapon->getMaxDamageBonus();
        criticalBonus = equippedWeapon->getCriticalBonus();
    }

    if (resultat <= 3)
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

    if (resultat <= 16)
    {
        return random.between(
            minDamage + bonusMin,
            maxDamage + bonusMax
        ) + damageBonus;
    }

    critical = true;
    return criticalDamage + criticalBonus + damageBonus;
}

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

    std::cout << "Potions de soin : "
              << inventory.countConsumables(ConsumableType::Healing)
              << std::endl;

    std::cout << "Potions de rage : "
              << inventory.countConsumables(ConsumableType::Damage)
              << std::endl;

    std::cout << "========================" << std::endl;
    std::cout << std::endl;
}

void Player::displayInventory() const
{
    inventory.display();
    displaySimpleEquipment();
}

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
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
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