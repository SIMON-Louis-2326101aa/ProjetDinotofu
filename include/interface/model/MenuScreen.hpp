// EN: MenuScreen.hpp centralizes menu data before rendering it in terminal or future GUI.
// FR: MenuScreen.hpp centralise les données d'un menu avant rendu terminal ou future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MODEL_MENUSCREEN_HPP
#define INCLUDE_INTERFACE_MODEL_MENUSCREEN_HPP

#include "interface/model/MenuOption.hpp"
#include "interface/model/GuiMenuSnapshot.hpp"

#include <cstddef>
#include <string>
#include <vector>

class MenuScreen
{
private:
    std::string screenId;
    std::string title;
    std::string inputMode = "choice";
    std::string inputPlaceholder;
    std::string inputHint;
    std::string confirmationKeyword;
    bool hasNumericRange = false;
    int numericMin = 0;
    int numericMax = 0;
    bool acceptsEmptyInput = false;
    bool hasTextLengthRange = false;
    int textMinLength = 0;
    int textMaxLength = 0;
    bool hasPagination = false;
    std::size_t pageIndex = 0;
    std::size_t totalPages = 1;
    std::vector<std::string> subtitles;
    std::vector<std::string> lines;
    std::vector<MenuOption> options;
    std::vector<std::string> footerLines;

public:
    MenuScreen(const std::string& title = "", const std::string& screenId = "");

    void setTitle(const std::string& value);
    void setScreenId(const std::string& value);
    void setChoiceInput(const std::string& hint = "");
    void setTextInput(
        const std::string& placeholder = "",
        const std::string& hint = "",
        bool allowEmpty = false,
        int minLength = 0,
        int maxLength = 0
    );
    void setQuantityInput(int minValue, int maxValue, const std::string& hint = "");
    void setConfirmationInput(const std::string& keyword, const std::string& hint = "");
    void setContinueInput(const std::string& hint = "Appuie sur Entrée pour continuer.");
    void setDisplayOnlyInput(const std::string& hint = "");
    void setPagination(std::size_t pageIndexValue, std::size_t totalPagesValue);
    void clearPagination();

    void addSubtitle(const std::string& value);
    void addLine(const std::string& value);
    void addFooterLine(const std::string& value);
    void addOption(
        int number,
        const std::string& label,
        const std::string& hint = "",
        bool enabled = true,
        const std::string& actionId = "",
        const MenuOptionItemData& itemData = MenuOptionItemData()
    );
    void addBackOption(const std::string& label = "Retour", const std::string& actionId = "back");

    const std::string& getScreenId() const;
    const std::string& getTitle() const;
    const std::string& getInputMode() const;
    const std::string& getInputPlaceholder() const;
    const std::string& getInputHint() const;
    const std::string& getConfirmationKeyword() const;
    bool getHasNumericRange() const;
    int getNumericMin() const;
    int getNumericMax() const;
    bool getAcceptsEmptyInput() const;
    bool getHasTextLengthRange() const;
    int getTextMinLength() const;
    int getTextMaxLength() const;
    bool getHasPagination() const;
    std::size_t getPageIndex() const;
    std::size_t getTotalPages() const;
    const std::vector<std::string>& getSubtitles() const;
    const std::vector<std::string>& getLines() const;
    const std::vector<MenuOption>& getOptions() const;
    const std::vector<std::string>& getFooterLines() const;

    int getHighestOptionNumber() const;
    GuiMenuSnapshot toGuiSnapshot() const;
};

#endif
