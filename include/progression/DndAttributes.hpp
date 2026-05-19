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
    DndAttributes();

    int getStrength() const;
    int getDexterity() const;
    int getConstitution() const;
    int getIntelligence() const;
    int getWisdom() const;
    int getCharisma() const;

    void setValues(
        int newStrength,
        int newDexterity,
        int newConstitution,
        int newIntelligence,
        int newWisdom,
        int newCharisma
    );

    bool increaseByChoice(int choice);

    static int getChoiceCount();
    static std::string getChoiceName(int choice);
    static std::string getChoiceDescription(int choice);
};

#endif
