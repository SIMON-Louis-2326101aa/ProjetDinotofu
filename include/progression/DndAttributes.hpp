// EN: DndAttributes.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DndAttributes.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Stores the six DND-inspired attributes used by future progression systems.

#ifndef INCLUDE_PROGRESSION_DNDATTRIBUTES_HPP
#define INCLUDE_PROGRESSION_DNDATTRIBUTES_HPP

#include <string>

class DndAttributes
{
private:
    int strength;
    int dexterity;
    int constitution;
    int intelligence;
    int wisdom;
    int charisma;

public:
    // EN: DndAttributes declares or implements a focused behavior used by this module.
    // FR: DndAttributes déclare ou implémente un comportement précis utilisé par ce module.
    DndAttributes();

    // EN: getStrength declares or implements a focused behavior used by this module.
    // FR: getStrength déclare ou implémente un comportement précis utilisé par ce module.
    int getStrength() const;
    // EN: getDexterity declares or implements a focused behavior used by this module.
    // FR: getDexterity déclare ou implémente un comportement précis utilisé par ce module.
    int getDexterity() const;
    // EN: getConstitution declares or implements a focused behavior used by this module.
    // FR: getConstitution déclare ou implémente un comportement précis utilisé par ce module.
    int getConstitution() const;
    // EN: getIntelligence declares or implements a focused behavior used by this module.
    // FR: getIntelligence déclare ou implémente un comportement précis utilisé par ce module.
    int getIntelligence() const;
    // EN: getWisdom declares or implements a focused behavior used by this module.
    // FR: getWisdom déclare ou implémente un comportement précis utilisé par ce module.
    int getWisdom() const;
    // EN: getCharisma declares or implements a focused behavior used by this module.
    // FR: getCharisma déclare ou implémente un comportement précis utilisé par ce module.
    int getCharisma() const;

    void setValues(
        int newStrength,
        int newDexterity,
        int newConstitution,
        int newIntelligence,
        int newWisdom,
        int newCharisma
    );

    // EN: increaseByChoice declares or implements a focused behavior used by this module.
    // FR: increaseByChoice déclare ou implémente un comportement précis utilisé par ce module.
    bool increaseByChoice(int choice);

    // EN: getChoiceCount declares or implements a focused behavior used by this module.
    // FR: getChoiceCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getChoiceCount();
    // EN: getChoiceName declares or implements a focused behavior used by this module.
    // FR: getChoiceName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getChoiceName(int choice);
    // EN: getChoiceDescription declares or implements a focused behavior used by this module.
    // FR: getChoiceDescription déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getChoiceDescription(int choice);
};

#endif
