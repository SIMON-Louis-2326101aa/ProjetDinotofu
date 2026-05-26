// EN: DndAttributes.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DndAttributes.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Stores the six DND-inspired attributes used by future progression systems.

#include "progression/DndAttributes.hpp"

// EN: DndAttributes declares or implements a focused behavior used by this module.
// FR: DndAttributes déclare ou implémente un comportement précis utilisé par ce module.
DndAttributes::DndAttributes()
{
    strength = 10;
    dexterity = 10;
    constitution = 10;
    intelligence = 10;
    wisdom = 10;
    charisma = 10;
}

// EN: getStrength declares or implements a focused behavior used by this module.
// FR: getStrength déclare ou implémente un comportement précis utilisé par ce module.
int DndAttributes::getStrength() const
{
    return strength;
}

// EN: getDexterity declares or implements a focused behavior used by this module.
// FR: getDexterity déclare ou implémente un comportement précis utilisé par ce module.
int DndAttributes::getDexterity() const
{
    return dexterity;
}

// EN: getConstitution declares or implements a focused behavior used by this module.
// FR: getConstitution déclare ou implémente un comportement précis utilisé par ce module.
int DndAttributes::getConstitution() const
{
    return constitution;
}

// EN: getIntelligence declares or implements a focused behavior used by this module.
// FR: getIntelligence déclare ou implémente un comportement précis utilisé par ce module.
int DndAttributes::getIntelligence() const
{
    return intelligence;
}

// EN: getWisdom declares or implements a focused behavior used by this module.
// FR: getWisdom déclare ou implémente un comportement précis utilisé par ce module.
int DndAttributes::getWisdom() const
{
    return wisdom;
}

// EN: getCharisma declares or implements a focused behavior used by this module.
// FR: getCharisma déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: increaseByChoice declares or implements a focused behavior used by this module.
// FR: increaseByChoice déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getChoiceCount declares or implements a focused behavior used by this module.
// FR: getChoiceCount déclare ou implémente un comportement précis utilisé par ce module.
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
            return "Augmente légèrement le critique. Influence aussi les réflexes, l'initiative et la fuite.";

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
