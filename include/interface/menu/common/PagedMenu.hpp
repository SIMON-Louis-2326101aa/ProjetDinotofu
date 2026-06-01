// EN: PagedMenu.hpp provides shared helpers for long terminal lists.
// FR: PagedMenu.hpp fournit des helpers communs pour les longues listes terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_COMMON_PAGEDMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMMON_PAGEDMENU_HPP

#include <cstddef>
#include <string>

class MenuScreen;

class PagedMenu
{
public:
    static std::size_t pageCount(std::size_t totalItems, std::size_t itemsPerPage);
    static std::size_t firstIndex(std::size_t pageIndex, std::size_t itemsPerPage);
    static std::size_t lastIndexExclusive(std::size_t totalItems, std::size_t pageIndex, std::size_t itemsPerPage);

    static std::string pageInfoText(std::size_t pageIndex, std::size_t totalPages, std::size_t totalItems);
    static std::string rangeText(std::size_t firstInclusive, std::size_t lastExclusive, std::size_t totalItems);
    static void addNavigationOptions(MenuScreen& screen, std::size_t pageIndex, std::size_t totalPages);
    static void addNavigationOptions(
        MenuScreen& screen,
        std::size_t pageIndex,
        std::size_t totalPages,
        const std::string& backActionId,
        const std::string& previousActionId,
        const std::string& nextActionId,
        const std::string& backHint = "Revenir à l'écran précédent.",
        const std::string& previousHint = "Revoir les entrées précédentes.",
        const std::string& nextHint = "Voir les entrées suivantes."
    );

    static void printPageInfo(std::size_t pageIndex, std::size_t totalPages, std::size_t totalItems);
    static void printNavigation(std::size_t pageIndex, std::size_t totalPages);
};

#endif
