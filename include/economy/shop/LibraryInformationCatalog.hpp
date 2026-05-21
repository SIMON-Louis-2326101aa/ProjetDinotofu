// EN: LibraryInformationCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: LibraryInformationCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Prepares purchasable common knowledge for the future bestiary and magic systems.
// Français : Prépare les connaissances communes achetables pour le futur bestiaire et les systèmes de magie.

#ifndef INCLUDE_ECONOMY_SHOP_LIBRARYINFORMATIONCATALOG_HPP
#define INCLUDE_ECONOMY_SHOP_LIBRARYINFORMATIONCATALOG_HPP

#include "economy/shop/ShopItem.hpp"

#include <vector>

class LibraryInformationCatalog
{
public:
    // EN: createCommonInformationOffers declares or implements a focused behavior used by this module.
    // FR: createCommonInformationOffers déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<ShopItem> createCommonInformationOffers();
};

#endif
