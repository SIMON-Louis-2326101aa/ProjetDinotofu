// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays a preview of the future shop menu.
// Français : Affiche une prévisualisation du futur menu de boutique.

#include "interface/menu/shop/ShopMenu.hpp"

#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopItemCategory.hpp"

#include <iostream>
#include <vector>

void ShopMenu::displayPreview()
{
    std::vector<ShopInventory> shops = ShopCatalog::createAllPreviewShops();

    std::cout << "========== BOUTIQUES ==========" << std::endl;
    std::cout << "Les boutiques seront renouvelées après chaque combat." << std::endl;
    std::cout << std::endl;

    for (std::size_t i = 0; i < shops.size(); ++i)
    {
        std::cout << i + 1 << " : " << shops[i].getName() << std::endl;
    }

    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}
