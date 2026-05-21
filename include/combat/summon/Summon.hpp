// EN: Summon.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Summon.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores basic summoned combat entity data for future 1v1-to-PvE transitions.

#ifndef INCLUDE_COMBAT_SUMMON_SUMMON_HPP
#define INCLUDE_COMBAT_SUMMON_SUMMON_HPP

#include <string>

class Summon
{
private:
    std::string name;
    std::string ownerName;
    int hp;
    int maxHp;
    int minDamage;
    int maxDamage;
    int durationTurns;
    int slotCost;
    int maintenanceCost;
    bool sacrificial;

public:
    // EN: Summon declares or implements a focused behavior used by this module.
    // FR: Summon déclare ou implémente un comportement précis utilisé par ce module.
    Summon();

    Summon(
        const std::string& name,
        const std::string& ownerName,
        int maxHp,
        int minDamage,
        int maxDamage,
        int durationTurns,
        int slotCost = 1,
        int maintenanceCost = 1,
        bool sacrificial = true
    );

    std::string getName() const;
    std::string getOwnerName() const;
    // EN: getHp declares or implements a focused behavior used by this module.
    // FR: getHp déclare ou implémente un comportement précis utilisé par ce module.
    int getHp() const;
    // EN: getMaxHp declares or implements a focused behavior used by this module.
    // FR: getMaxHp déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxHp() const;
    // EN: getMinDamage declares or implements a focused behavior used by this module.
    // FR: getMinDamage déclare ou implémente un comportement précis utilisé par ce module.
    int getMinDamage() const;
    // EN: getMaxDamage declares or implements a focused behavior used by this module.
    // FR: getMaxDamage déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxDamage() const;
    // EN: getDurationTurns declares or implements a focused behavior used by this module.
    // FR: getDurationTurns déclare ou implémente un comportement précis utilisé par ce module.
    int getDurationTurns() const;
    // EN: getSlotCost declares or implements a focused behavior used by this module.
    // FR: getSlotCost déclare ou implémente un comportement précis utilisé par ce module.
    int getSlotCost() const;
    // EN: getMaintenanceCost declares or implements a focused behavior used by this module.
    // FR: getMaintenanceCost déclare ou implémente un comportement précis utilisé par ce module.
    int getMaintenanceCost() const;
    // EN: canBeSacrificed declares or implements a focused behavior used by this module.
    // FR: canBeSacrificed déclare ou implémente un comportement précis utilisé par ce module.
    bool canBeSacrificed() const;

    // EN: isDead declares or implements a focused behavior used by this module.
    // FR: isDead déclare ou implémente un comportement précis utilisé par ce module.
    bool isDead() const;
    // EN: isExpired declares or implements a focused behavior used by this module.
    // FR: isExpired déclare ou implémente un comportement précis utilisé par ce module.
    bool isExpired() const;

    // EN: takeDamage declares or implements a focused behavior used by this module.
    // FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
    void takeDamage(int damage);
    // EN: heal declares or implements a focused behavior used by this module.
    // FR: heal déclare ou implémente un comportement précis utilisé par ce module.
    void heal(int healAmount);
    // EN: setDurationTurns declares or implements a focused behavior used by this module.
    // FR: setDurationTurns déclare ou implémente un comportement précis utilisé par ce module.
    void setDurationTurns(int turns);
    // EN: decreaseDuration declares or implements a focused behavior used by this module.
    // FR: decreaseDuration déclare ou implémente un comportement précis utilisé par ce module.
    void decreaseDuration();
    // EN: extendDuration declares or implements a focused behavior used by this module.
    // FR: extendDuration déclare ou implémente un comportement précis utilisé par ce module.
    void extendDuration(int turns);
    // EN: markSacrificed declares or implements a focused behavior used by this module.
    // FR: markSacrificed déclare ou implémente un comportement précis utilisé par ce module.
    void markSacrificed();
};

#endif
