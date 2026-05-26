// EN: MenuScreen.hpp centralizes menu data before rendering it in terminal or future GUI.
// FR: MenuScreen.hpp centralise les données d'un menu avant rendu terminal ou future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MODEL_MENUSCREEN_HPP
#define INCLUDE_INTERFACE_MODEL_MENUSCREEN_HPP

#include "interface/model/MenuOption.hpp"
#include "interface/model/GuiMenuSnapshot.hpp"

#include <string>
#include <vector>

class MenuScreen
{
private:
    std::string screenId;
    std::string title;
    std::vector<std::string> subtitles;
    std::vector<std::string> lines;
    std::vector<MenuOption> options;
    std::vector<std::string> footerLines;

public:
    MenuScreen(const std::string& title = "", const std::string& screenId = "");

    void setTitle(const std::string& value);
    void setScreenId(const std::string& value);

    void addSubtitle(const std::string& value);
    void addLine(const std::string& value);
    void addFooterLine(const std::string& value);
    void addOption(
        int number,
        const std::string& label,
        const std::string& hint = "",
        bool enabled = true,
        const std::string& actionId = ""
    );
    void addBackOption(const std::string& label = "Retour", const std::string& actionId = "back");

    const std::string& getScreenId() const;
    const std::string& getTitle() const;
    const std::vector<std::string>& getSubtitles() const;
    const std::vector<std::string>& getLines() const;
    const std::vector<MenuOption>& getOptions() const;
    const std::vector<std::string>& getFooterLines() const;

    int getHighestOptionNumber() const;
    GuiMenuSnapshot toGuiSnapshot() const;
};

#endif
