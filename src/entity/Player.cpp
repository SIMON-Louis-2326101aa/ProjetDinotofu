// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Player.hpp"

#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"

#include <iostream>

Player::Player() : Entity()
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
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
        playerClass.getHealingPotions(),
        playerClass.getDamagePotions()
    )
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
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

    int ancienBonusPv = getEquippedArmorMaxHpBonus();

    equippedArmorIndex = index;

    int nouveauBonusPv = getEquippedArmorMaxHpBonus();
    int differencePvMax = nouveauBonusPv - ancienBonusPv;

    maxHp += differencePvMax;
    hp += differencePvMax;

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
    int ancienBonusPv = getEquippedArmorMaxHpBonus();

    equippedArmorIndex = -1;

    maxHp -= ancienBonusPv;

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
    inventory.addWeapon(WeaponCatalog::creerMainsNues());
    inventory.addWeapon(WeaponCatalog::creerEpeeRouillee());

    equipWeapon(1);

    inventory.addArmor(ArmorCatalog::creerTenueSimple());
    inventory.addArmor(ArmorCatalog::creerArmureCuirUsee());

    equipArmor(0);

    for (int i = 0; i < healingPotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::creerPotionSoinBasique());
    }

    for (int i = 0; i < damagePotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::creerPotionDegatsBasique());
    }

    inventory.earnGold(50);
}

void Player::gainExperience(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    experience += amount;

    while (experience >= 100)
    {
        experience -= 100;
        levelUp();
    }
}

void Player::levelUp()
{
    level++;

    maxHp += 20;
    hp = maxHp;

    minDamage += 1;
    maxDamage += 2;
    criticalDamage += 3;

    std::cout << name << " monte au niveau " << level << " !" << std::endl;
    std::cout << "Ses blessures se referment, et sa puissance augmente." << std::endl;
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

    Weapon* armeEquipee = inventory.getMutableWeapon(equippedWeaponIndex);

    if (armeEquipee != nullptr && !armeEquipee->isBroken())
    {
        bonusMin = armeEquipee->getMinDamageBonus();
        bonusMax = armeEquipee->getMaxDamageBonus();
        criticalBonus = armeEquipee->getCriticalBonus();
    }

    if (resultat <= 3)
    {
        dodged = true;
        return 0;
    }

    if (armeEquipee != nullptr)
    {
        armeEquipee->loseDurability(1);
    }

    if (armeEquipee != nullptr && armeEquipee->isBroken())
    {
        std::cout << "L'arme de " << name << " s'abîme sous le choc..." << std::endl;
        std::cout << armeEquipee->getName() << " est maintenant cassée et ne donnera plus ses bonus." << std::endl;
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
    std::cout << "Classe : " << type << std::endl;
    std::cout << "Niveau : " << level << std::endl;
    std::cout << "Expérience : " << experience << "/100" << std::endl;
    std::cout << "PV : " << hp << "/" << maxHp << std::endl;
    std::cout << "Dégâts de base : " << minDamage << " - " << maxDamage << std::endl;
    std::cout << "Critique de base : " << criticalDamage << std::endl;

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