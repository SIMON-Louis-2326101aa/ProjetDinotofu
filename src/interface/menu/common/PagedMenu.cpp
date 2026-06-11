// EN: PagedMenu.cpp centralizes terminal pagination helpers.
// FR: PagedMenu.cpp centralise les helpers de pagination terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/common/PagedMenu.hpp"

#include "interface/model/MenuScreen.hpp"

#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <sstream>

namespace
{
    bool guiScrollableMenuEnabled()
    {
        const char* value = std::getenv("DINOTOFU_GUI_INPUT_MODE");
        if (value == nullptr)
        {
            return false;
        }

        std::string text(value);
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return text == "1" || text == "true" || text == "yes" || text == "on" || text == "gui";
    }

    MenuOptionItemData makeNavigationItemData(const std::string& actionType, const std::string& name, const std::string& detail)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "navigation";
        itemData.section = "Navigation";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = "Disponible";
        return itemData;
    }
}

std::size_t PagedMenu::pageCount(std::size_t totalItems, std::size_t itemsPerPage)
{
    if (guiScrollableMenuEnabled())
    {
        return 1;
    }

    if (itemsPerPage == 0 || totalItems == 0)
    {
        return 1;
    }

    return (totalItems + itemsPerPage - 1) / itemsPerPage;
}

std::size_t PagedMenu::firstIndex(std::size_t pageIndex, std::size_t itemsPerPage)
{
    if (guiScrollableMenuEnabled())
    {
        return 0;
    }

    return pageIndex * itemsPerPage;
}

std::size_t PagedMenu::lastIndexExclusive(std::size_t totalItems, std::size_t pageIndex, std::size_t itemsPerPage)
{
    if (guiScrollableMenuEnabled())
    {
        return totalItems;
    }

    if (itemsPerPage == 0)
    {
        return totalItems;
    }

    return std::min(totalItems, firstIndex(pageIndex, itemsPerPage) + itemsPerPage);
}

std::string PagedMenu::rangeText(std::size_t firstInclusive, std::size_t lastExclusive, std::size_t totalItems)
{
    if (totalItems == 0)
    {
        return "aucune entrée";
    }

    std::ostringstream output;
    output << (firstInclusive + 1) << "-" << lastExclusive << " / " << totalItems;
    return output.str();
}

std::string PagedMenu::pageInfoText(std::size_t pageIndex, std::size_t totalPages, std::size_t totalItems)
{
    std::ostringstream output;
    if (guiScrollableMenuEnabled())
    {
        output << "Liste déroulante complète | Entrées : " << totalItems;
    }
    else
    {
        output << "Page " << (pageIndex + 1) << " / " << totalPages
               << " | Entrées : " << totalItems;
    }
    return output.str();
}

void PagedMenu::addNavigationOptions(MenuScreen& screen, std::size_t pageIndex, std::size_t totalPages)
{
    addNavigationOptions(
        screen,
        pageIndex,
        totalPages,
        "back",
        "page.previous",
        "page.next"
    );
}

void PagedMenu::addNavigationOptions(
    MenuScreen& screen,
    std::size_t pageIndex,
    std::size_t totalPages,
    const std::string& backActionId,
    const std::string& previousActionId,
    const std::string& nextActionId,
    const std::string& backHint,
    const std::string& previousHint,
    const std::string& nextHint
)
{
    screen.setPagination(pageIndex, totalPages);
    screen.addOption(
        0,
        "Retour",
        backHint,
        true,
        backActionId,
        makeNavigationItemData("back", "Retour", backHint)
    );

    if (totalPages > 1)
    {
        if (pageIndex > 0)
        {
            screen.addOption(
                98,
                "Page précédente",
                previousHint,
                true,
                previousActionId,
                makeNavigationItemData("previous_page", "Page précédente", previousHint)
            );
        }

        if (pageIndex + 1 < totalPages)
        {
            screen.addOption(
                99,
                "Page suivante",
                nextHint,
                true,
                nextActionId,
                makeNavigationItemData("next_page", "Page suivante", nextHint)
            );
        }
    }
}

void PagedMenu::printPageInfo(std::size_t pageIndex, std::size_t totalPages, std::size_t totalItems)
{
    std::cout << pageInfoText(pageIndex, totalPages, totalItems) << std::endl;
}

void PagedMenu::printNavigation(std::size_t pageIndex, std::size_t totalPages)
{
    std::cout << "0 : Retour" << std::endl << std::endl;

    if (totalPages > 1)
    {
        if (pageIndex > 0)
        {
            std::cout << "98 : Page précédente" << std::endl << std::endl;
        }

        if (pageIndex + 1 < totalPages)
        {
            std::cout << "99 : Page suivante" << std::endl << std::endl;
        }
    }
}
