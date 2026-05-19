// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Stores the six DND-inspired attributes used by future progression systems.

#include "progression/DndAttributes.hpp"

DndAttributes::DndAttributes()
{
    strength = 10;
    dexterity = 10;
    constitution = 10;
    intelligence = 10;
    wisdom = 10;
    charisma = 10;
}

int DndAttributes::getStrength() const
{
    return strength;
}

int DndAttributes::getDexterity() const
{
    return dexterity;
}

int DndAttributes::getConstitution() const
{
    return constitution;
}

int DndAttributes::getIntelligence() const
{
    return intelligence;
}

int DndAttributes::getWisdom() const
{
    return wisdom;
}

int DndAttributes::getCharisma() const
{
    return charisma;
}

void DndAttributes::setValues(
    int newStrength,
    int newDexterity,
    int newConstitution,
    int newIntelligence,
    int newWisdom,
    int newCharisma
)
{
    strength = newStrength;
    dexterity = newDexterity;
    constitution = newConstitution;
    intelligence = newIntelligence;
    wisdom = newWisdom;
    charisma = newCharisma;

    if (strength < 1) strength = 1;
    if (dexterity < 1) dexterity = 1;
    if (constitution < 1) constitution = 1;
    if (intelligence < 1) intelligence = 1;
    if (wisdom < 1) wisdom = 1;
    if (charisma < 1) charisma = 1;
}

bool DndAttributes::increaseByChoice(int choice)
{
    switch (choice)
    {
        case 1:
            strength++;
            return true;

        case 2:
            dexterity++;
            return true;

        case 3:
            constitution++;
            return true;

        case 4:
            intelligence++;
            return true;

        case 5:
            wisdom++;
            return true;

        case 6:
            charisma++;
            return true;

        default:
            return false;
    }
}

int DndAttributes::getChoiceCount()
{
    return 6;
}

std::string DndAttributes::getChoiceName(int choice)
{
    switch (choice)
    {
        case 1: return "Force";
        case 2: return "Dextérité";
        case 3: return "Constitution";
        case 4: return "Intelligence";
        case 5: return "Sagesse";
        case 6: return "Charisme";
        default: return "Inconnu";
    }
}

std::string DndAttributes::getChoiceDescription(int choice)
{
    switch (choice)
    {
        case 1:
            return "Augmente légèrement les dégâts de base.";

        case 2:
            return "Augmente légèrement le critique. Plus tard : esquive, initiative et fuite.";

        case 3:
            return "Augmente les PV maximum.";

        case 4:
            return "Prépare la magie, les analyses et certaines évolutions.";

        case 5:
            return "Prépare les soins, protections, perception et résistances mentales.";

        case 6:
            return "Prépare les invocations, pactes, dialogues et interactions sociales.";

        default:
            return "Aucune description.";
    }
}
