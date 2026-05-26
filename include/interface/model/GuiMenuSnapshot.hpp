// EN: GuiMenuSnapshot.hpp defines a serializable-ish view of a menu for future GUI renderers.
// FR: GuiMenuSnapshot.hpp définit une vue de menu proche du sérialisable pour les futurs rendus IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MODEL_GUIMENUSNAPSHOT_HPP
#define INCLUDE_INTERFACE_MODEL_GUIMENUSNAPSHOT_HPP

#include <string>
#include <vector>

struct GuiMenuActionSnapshot
{
    int number = 0;
    std::string label;
    std::string hint;
    bool enabled = true;
    std::string actionId;
};

struct GuiMenuSnapshot
{
    std::string screenId;
    std::string title;
    std::vector<std::string> subtitles;
    std::vector<std::string> lines;
    std::vector<GuiMenuActionSnapshot> actions;
    std::vector<std::string> footerLines;
};

#endif
