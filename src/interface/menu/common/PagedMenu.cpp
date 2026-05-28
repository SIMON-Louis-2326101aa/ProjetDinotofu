// EN: PagedMenu.cpp centralizes terminal pagination helpers.
// FR: PagedMenu.cpp centralise les helpers de pagination terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/common/PagedMenu.hpp"

#include "interface/model/MenuScreen.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

std::size_t PagedMenu::pageCount(std::size_t totalItems, std::size_t itemsPerPage)
{
    if (itemsPerPage == 0 || totalItems == 0)
    {
        return 1;
    }

    return (totalItems + itemsPerPage - 1) / itemsPerPage;
}

std::size_t PagedMenu::firstIndex(std::size_t pageIndex, std::size_t itemsPerPage)
{
    return pageIndex * itemsPerPage;
}

std::size_t PagedMenu::lastIndexExclusive(std::size_t totalItems, std::size_t pageIndex, std::size_t itemsPerPage)
{
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
    output << "Page " << (pageIndex + 1) << " / " << totalPages
           << " | Entrées : " << totalItems;
    return output.str();
}

void PagedMenu::addNavigationOptions(MenuScreen& screen, std::size_t pageIndex, std::size_t totalPages)
{
    screen.setPagination(pageIndex, totalPages);
    screen.addBackOption();

    if (totalPages > 1)
    {
        if (pageIndex > 0)
        {
            screen.addOption(98, "Page précédente", "", true, "page.previous");
        }

        if (pageIndex + 1 < totalPages)
        {
            screen.addOption(99, "Page suivante", "", true, "page.next");
        }
    }
}

void PagedMenu::printPageInfo(std::size_t pageIndex, std::size_t totalPages, std::size_t totalItems)
{
    std::cout << pageInfoText(pageIndex, totalPages, totalItems) << std::endl;
}

void PagedMenu::printNavigation(std::size_t pageIndex, std::size_t totalPages)
{
    std::cout << "0 : Retour" << std::endl;

    if (totalPages > 1)
    {
        if (pageIndex > 0)
        {
            std::cout << "98 : Page précédente" << std::endl;
        }

        if (pageIndex + 1 < totalPages)
        {
            std::cout << "99 : Page suivante" << std::endl;
        }
    }
}
