// EN: GuiSchemaVersion.hpp centralizes the JSON contract versions used by the experimental GUI.
// FR: GuiSchemaVersion.hpp centralise les versions de contrat JSON utilisées par l'IG expérimentale.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MODEL_GUISCHEMAVERSION_HPP
#define INCLUDE_INTERFACE_MODEL_GUISCHEMAVERSION_HPP

namespace GuiSchemaVersion
{
    // FR: Ces nombres ne sont pas la version du jeu. Ils décrivent le contrat JSON
    // compris par l'IG expérimentale. On les garde pour le debug et les fallbacks,
    // mais on ne les augmente que lorsqu'un champ structurant est ajouté/retiré ou
    // change de sens. Un simple changement visuel HTML/CSS ne doit pas les modifier.
    inline constexpr int Menu = 18;
    inline constexpr int Combat = 2;
    inline constexpr int State = 13;
}

#endif
