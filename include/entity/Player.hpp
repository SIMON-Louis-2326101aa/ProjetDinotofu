// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_PLAYER_HPP
#define INCLUDE_ENTITY_PLAYER_HPP

#include "entity/Entity.hpp"
#include "class_system/PlayerClass.hpp"
#include "item/Inventory.hpp"

class Player : public Entity
{
private:
    int level;
    int experience;

    Inventory inventory;
    int equippedWeaponIndex;
    int equippedArmorIndex;

    int getEquippedArmorMaxHpBonus() const;

public:
    Player();

    Player(
        const std::string& name,
        const PlayerClass& playerClass
    );

    int getLevel() const;
    int getExperience() const;

    Inventory& getInventory();
    const Inventory& getInventory() const;

    int getEquippedWeaponIndex() const;
    bool hasEquippedWeapon() const;
    Weapon getEquippedWeapon() const;
    bool equipWeapon(int index);
    void unequipWeapon();

    int getEquippedArmorIndex() const;
    bool hasEquippedArmor() const;
    Armor getEquippedArmor() const;
    bool equipArmor(int index);
    void unequipArmor();

    void initializeStarterInventory();

    void gainExperience(int amount);
    void loseExperience(int amount);
    void levelUp();

    int attack(Random& random, bool& dodged, bool& critical, int damageBonus = 0) override;

    void displayStats() const override;
    void displayInventory() const;
    void displaySimpleEquipment() const;
    void displayDetailedEquipment() const;
};

#endif
