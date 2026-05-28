// EN: SpecialCharacterNativeBonus.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterNativeBonus.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Applies native bonuses for special characters when their intended identity/class is used.

#include "character/SpecialCharacterNativeBonus.hpp"

#include "character/SpecialCharacterCatalog.hpp"

#include <algorithm>
#include <cctype>
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>

// EN: applyIfNativeMatch declares or implements a focused behavior used by this module.
// FR: applyIfNativeMatch déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterNativeBonus::applyIfNativeMatch(Player& player)
{
    SpecialCharacter character;

    if (!SpecialCharacterCatalog::findByName(player.getName(), character))
    {
        return false;
    }

    return applyForSpecialCharacter(player, character);
}

bool SpecialCharacterNativeBonus::applyForSpecialCharacter(
    Player& player,
    const SpecialCharacter& character
)
{
    if (character.getName() == "Matt (PRO)")
    {
        // Matt is globally better, not bound to a native class.
        // Matt est meilleur globalement, sans dépendre d'une classe native.
        player.applyFlatStatBonus(20, 2, 4, 6);
        displayNativeBonusApplied(character);
        return true;
    }

    if (!classMatchesNativeClass(player.getType(), character.getNativeClass()))
    {
        return false;
    }

    std::string normalizedName = normalize(character.getName());

    if (normalizedName == normalize("Hazak"))
    {
        player.applyFlatStatBonus(5, 2, 5, 12);
    }
    else if (normalizedName == normalize("Mattzelda"))
    {
        player.applyFlatStatBonus(40, 1, 2, 4);
    }
    else if (normalizedName == normalize("Aoi"))
    {
        player.applyFlatStatBonus(5, 1, 5, 8);
    }
    else if (normalizedName == normalize("Kanadé"))
    {
        player.applyFlatStatBonus(10, 0, 6, 10);
    }
    else if (normalizedName == normalize("Fail"))
    {
        player.applyFlatStatBonus(-5, 0, 8, 15);
    }
    else if (normalizedName == normalize("Trexof"))
    {
        player.applyFlatStatBonus(10, 2, 4, 8);
    }
    else if (normalizedName == normalize("Skuro"))
    {
        player.applyFlatStatBonus(45, 5, 12, 15);
    }
    else if (normalizedName == normalize("Sanctus"))
    {
        player.applyFlatStatBonus(45, 0, 2, 4);
    }
    else if (normalizedName == normalize("Hestia"))
    {
        player.applyFlatStatBonus(-20, 0, 6, 15);
    }
    else if (normalizedName == normalize("Fire Flight"))
    {
        player.applyFlatStatBonus(5, 1, 1, 3);
    }
    else if (normalizedName == normalize("Louis"))
    {
        player.applyFlatStatBonus(10, 2, 5, 6);
    }
    else if (normalizedName == normalize("Henrique"))
    {
        player.applyFlatStatBonus(25, 2, 4, 5);
    }
    else
    {
        return false;
    }

    displayNativeBonusApplied(character);
    return true;
}

// EN: displayNativeBonusApplied declares or implements a focused behavior used by this module.
// FR: displayNativeBonusApplied déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNativeBonus::displayNativeBonusApplied(const SpecialCharacter& character)
{
    MessageScreen::show(
        "BONUS NATIF",
        "character.native_bonus.applied",
        {
            "Bonus natif reconnu : " + character.getName() + ".",
            "L'identité ne copie pas seulement une classe : elle réveille une façon de combattre."
        },
        false
    );
}

bool SpecialCharacterNativeBonus::classMatchesNativeClass(
    const std::string& playerClass,
    const std::string& nativeClass
)
{
    return normalize(playerClass) == normalize(nativeClass);
}

std::string SpecialCharacterNativeBonus::normalize(const std::string& value)
{
    std::string normalized = value;

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    std::string::size_type position = 0;
    while ((position = normalized.find("é", position)) != std::string::npos)
    {
        normalized.replace(position, 2, "e");
        position += 1;
    }

    return normalized;
}
