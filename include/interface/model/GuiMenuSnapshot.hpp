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
    std::string actionDisplayLabel;
    std::string hint;
    bool enabled = true;
    std::string actionId;
    std::string role = "normal";
    std::string style = "neutral";
    std::string group = "main";
    bool dangerous = false;
    bool knowledgeSensitive = false;
    bool recommended = false;
    std::string recommendationReason;
    bool hasItemMetadata = false;
    std::string itemKind;
    std::string itemSection;
    std::string itemActionType;
    std::string itemName;
    std::string itemQuantity;
    std::string itemDetail;
    std::string itemStatus;
    std::string itemPrice;
    std::string itemStock;
    std::string itemMaxQuantity;
    std::string itemReward;
    std::string itemProgress;
    std::string itemOwner;
    bool itemImportant = false;
};

struct GuiMenuFocusSnapshot
{
    std::string id;
    std::string label;
    std::string detail;
    int actionNumber = -1;
    std::string actionId;
    std::string actionLabel;
    std::string actionDisplayLabel;
    bool available = false;
};

struct GuiMenuInfoSnapshot
{
    std::string id;
    std::string label;
    std::string value;
    std::string source = "line";
    std::string kind = "info";
    bool important = false;
};


struct GuiMenuItemSnapshot
{
    std::string id;
    std::string kind = "item";
    std::string section = "Général";
    std::string actionType = "open";
    std::string actionTypeLabel = "Ouvrir";
    std::string name;
    std::string quantity;
    std::string detail;
    std::string status;
    std::string price;
    std::string stock;
    std::string maxQuantity;
    std::string reward;
    std::string progress;
    std::string owner;
    int actionNumber = -1;
    std::string actionId;
    std::string actionLabel;
    std::string actionDisplayLabel;
    bool available = false;
    bool important = false;
    std::string metadataSource = "label";
};

struct GuiMenuNoticeSnapshot
{
    std::string id;
    std::string text;
    std::string source = "line";
    std::string kind = "info";
    bool important = false;
};

struct GuiMenuSnapshot
{
    std::string screenId;
    std::string title;
    std::string screenCategory = "menu";
    std::string inputMode = "choice";
    std::string inputIntent = "choix";
    std::string inputPlaceholder;
    std::string inputHint;
    std::string confirmationKeyword;
    std::string expectedInputPattern = "";
    std::string primarySubmitLabel = "Envoyer";
    bool inputRequired = true;
    bool acceptsEmptyInput = false;
    bool hasNumericRange = false;
    bool hasTextLengthRange = false;
    int textMinLength = 0;
    int textMaxLength = 0;
    int numericMin = 0;
    int numericMax = 0;
    bool hasDangerAction = false;
    bool hasDisabledAction = false;
    int actionCount = 0;
    int enabledActionCount = 0;
    int disabledActionCount = 0;
    int mainActionCount = 0;
    int navigationActionCount = 0;
    int dangerActionCount = 0;
    int lockedActionCount = 0;
    bool hasPagination = false;
    int pageIndex = 0;
    int pageNumber = 1;
    int totalPages = 1;
    bool hasPreviousPage = false;
    bool hasNextPage = false;
    std::vector<std::string> contextTags;
    std::vector<int> validChoices;
    int recommendedChoice = -1;
    std::string recommendedChoiceLabel;
    std::string recommendationMode = "ui_default";
    std::string recommendationSafety = "no_hidden_knowledge";
    std::string recommendationReason;
    bool hasKnowledgeSensitiveAction = false;
    std::string specializedView = "generic";
    std::string specializedTitle;
    std::string specializedHint;
    std::vector<GuiMenuFocusSnapshot> focusCards;
    std::vector<GuiMenuInfoSnapshot> infoCards;
    std::vector<GuiMenuItemSnapshot> itemCards;
    std::vector<GuiMenuNoticeSnapshot> noticeCards;
    std::vector<std::string> subtitles;
    std::vector<std::string> lines;
    std::vector<GuiMenuActionSnapshot> actions;
    std::vector<std::string> footerLines;
};

#endif
