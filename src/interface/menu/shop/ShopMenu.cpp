// EN: ShopMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable shop menu, with some real purchases and prepared future categories.
// Français : Affiche le premier menu de boutique utilisable, avec certains achats réels et des catégories futures préparées.

#include "interface/menu/shop/ShopMenu.hpp"

#include "core/Console.hpp"
#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopItemCategory.hpp"
#include "economy/shop/ShopPriceRules.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/quest/QuestMenu.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{

    std::string getVendorNameForShop(ShopType type)
    {
        switch (type)
        {
            case ShopType::MonsterMaterial:
                return "Vendeur de composants";
            case ShopType::Material:
                return "Vendeur de matériaux";
            case ShopType::Plant:
                return "Herboriste";
            case ShopType::Armor:
                return "Armurier";
            case ShopType::Weapon:
                return "Vendeur d'armes";
            case ShopType::Consumable:
                return "Vendeur de consommables";
            case ShopType::Library:
                return "Bibliothécaire";
            case ShopType::Blacksmith:
                return "Forgeron";
            case ShopType::Alchemist:
                return "Alchimiste";
            case ShopType::BlackMarket:
                return "Contact du marché noir";
            default:
                return "Marchand inquiet";
        }
    }

    // EN: displayShopList declares or implements a focused behavior used by this module.
    // FR: displayShopList déclare ou implémente un comportement précis utilisé par ce module.
    void displayShopList(const std::vector<ShopInventory>& shops)
    {
        std::cout << "========== BOUTIQUES ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;

        for (std::size_t i = 0; i < shops.size(); ++i)
        {
            std::cout << i + 1 << " : " << shops[i].getName() << std::endl;
        }

        std::cout << "===============================" << std::endl;
        std::cout << std::endl;
    }

    // EN: displayShopStock declares or implements a focused behavior used by this module.
    // FR: displayShopStock déclare ou implémente un comportement précis utilisé par ce module.
    void displayShopStock(const ShopInventory& shop, const Player& player)
    {
        const std::vector<ShopItem>& items = shop.getItems();

        std::cout << "========== " << shop.getName() << " ==========" << std::endl;
        std::cout << "Or disponible : " << player.getInventory().getGold() << " pièces" << std::endl;
        std::cout << "Race : " << player.getRaceText() << std::endl;
        std::cout << std::endl;
        std::cout << "0 : Retour" << std::endl;

        if (items.empty())
        {
            std::cout << "Aucun article disponible pour le moment." << std::endl;
        }

        for (std::size_t i = 0; i < items.size(); ++i)
        {
            int finalPrice = ShopPriceRules::applyBuyModifier(
                items[i].getBuyPrice(),
                player.getRaceText(),
                player.getType()
            );

            std::cout << i + 1 << " : "
                      << items[i].getName()
                      << " | Prix : "
                      << finalPrice
                      << " or";

            if (items[i].getStock() > 0)
            {
                std::cout << " | Stock : " << items[i].getStock();
            }

            if (items[i].isSoldOut())
            {
                std::cout << " | Épuisé";
            }
            else if (!ShopTransactionSystem::canBeBoughtNow(items[i]))
            {
                std::cout << " | Prévu plus tard";
            }

            std::cout << std::endl;
        }

        std::cout << "================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: inspectShopItem declares or implements a focused behavior used by this module.
    // FR: inspectShopItem déclare ou implémente un comportement précis utilisé par ce module.
    void inspectShopItem(const ShopItem& item, const Player& player)
    {
        int finalBuyPrice = ShopPriceRules::applyBuyModifier(
            item.getBuyPrice(),
            player.getRaceText(),
            player.getType()
        );

        int finalSellPrice = ShopPriceRules::applySellModifier(
            item.getSellPrice(),
            player.getRaceText(),
            player.getType()
        );

        std::cout << "========== ARTICLE ==========" << std::endl;
        std::cout << "Nom : " << item.getName() << std::endl;
        std::cout << "Description : " << item.getDescription() << std::endl;
        std::cout << "Prix d'achat : " << finalBuyPrice << " or" << std::endl;
        std::cout << "Prix de revente prévu : " << finalSellPrice << " or" << std::endl;

        if (player.getRaceText().find("Démon") != std::string::npos
            || player.getRaceText().find("démon") != std::string::npos)
        {
            std::cout << "Note : ton apparence démoniaque influence déjà certains prix." << std::endl;
        }

        if (ShopPriceRules::hasCraftClassTradeBonus(player.getType()))
        {
            std::cout << "Note : ta classe d'artisanat négocie légèrement mieux les prix." << std::endl;
        }

        if (!ShopTransactionSystem::canBeBoughtNow(item))
        {
            std::cout << "Statut : article préparé, mais pas encore stocké réellement dans l'inventaire." << std::endl;
        }

        std::cout << "=============================" << std::endl;
        std::cout << std::endl;
    }


    // EN: openSellMenu declares or implements a focused behavior used by this module.
    // FR: openSellMenu déclare ou implémente un comportement précis utilisé par ce module.
    void openSellMenu(Player& player, const ShopInventory& shop)
    {
        bool selling = true;

        while (selling)
        {
            Console::clear();
            std::cout << "========== REVENTE ==========" << std::endl;
            std::cout << "Boutique : " << shop.getName() << std::endl;
            std::cout << "Or actuel : " << player.getInventory().getGold() << " pièces" << std::endl;
            std::cout << "0 : Retour" << std::endl;
            std::cout << std::endl;

            ShopTransactionSystem::displaySellableEntries(player, shop.getType());

            int maxChoice = ShopTransactionSystem::getSellableEntryCount(player, shop.getType());

            if (maxChoice <= 0)
            {
                std::cout << std::endl;
                Console::waitForEnter();
                return;
            }

            std::cout << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(
                0,
                maxChoice,
                "Veuillez choisir une entrée à vendre, ou 0 pour revenir."
            );

            if (choice == 0)
            {
                selling = false;
                continue;
            }

            int index = choice - 1;

            if (!ShopTransactionSystem::canShopBuyInventoryEntry(player, shop.getType(), index))
            {
                std::cout << "Cette entrée est protégée ou impossible à vendre ici." << std::endl;
                std::cout << std::endl;
                Console::waitForEnter();
                continue;
            }

            int sellPrice = ShopTransactionSystem::getSellPriceForEntry(
                player,
                shop.getType(),
                index
            );

            std::cout << "Confirmer la vente pour " << sellPrice << " or ?" << std::endl;
            std::cout << "1 : Oui" << std::endl;
            std::cout << "2 : Non" << std::endl;
            std::cout << "> ";

            int confirm = Console::askNumberBetween(
                1,
                2,
                "Veuillez choisir 1 ou 2."
            );

            if (confirm == 1)
            {
                ShopTransactionSystem::sellInventoryEntry(
                    player,
                    shop.getType(),
                    index,
                    sellPrice
                );
            }
            else
            {
                std::cout << "Vente annulée." << std::endl;
                std::cout << std::endl;
            }

            Console::waitForEnter();
        }
    }

    // EN: openSingleShop declares or implements a focused behavior used by this module.
    // FR: openSingleShop déclare ou implémente un comportement précis utilisé par ce module.
    void openSingleShop(Player& player, ShopInventory& shop)
    {
        bool stayInShop = true;

        while (stayInShop)
        {
            Console::clear();
            std::cout << "========== " << shop.getName() << " ==========" << std::endl;
            std::cout << "Or disponible : " << player.getInventory().getGold() << " pièces" << std::endl;
            std::cout << "0 : Retour" << std::endl;
            std::cout << "1 : Acheter" << std::endl;
            std::cout << "2 : Vendre" << std::endl;
            std::cout << "3 : Discuter avec " << getVendorNameForShop(shop.getType()) << std::endl;
            std::cout << "4 : Quêtes de " << getVendorNameForShop(shop.getType()) << std::endl;
            std::cout << "================================" << std::endl;
            std::cout << "> ";

            int shopChoice = Console::askNumberBetween(
                0,
                4,
                "Veuillez choisir acheter, vendre, discuter, quêtes, ou 0 pour revenir."
            );

            if (shopChoice == 0)
            {
                stayInShop = false;
                continue;
            }

            if (shopChoice == 2)
            {
                openSellMenu(player, shop);
                continue;
            }

            if (shopChoice == 3)
            {
                Console::clear();
                std::cout << getVendorNameForShop(shop.getType()) << " prend quelques secondes pour parler boutique, rumeurs et besoins du moment." << std::endl;
                std::cout << "S'il a une vraie demande, utilise l'option de quêtes juste en dessous." << std::endl;
                std::cout << std::endl;
                Console::waitForEnter();
                continue;
            }

            if (shopChoice == 4)
            {
                Console::clear();
                QuestMenu::talkToClient(player, getVendorNameForShop(shop.getType()));
                continue;
            }

            Console::clear();
            displayShopStock(shop, player);
            std::cout << "> ";

            int itemChoice = Console::askNumberBetween(
                0,
                static_cast<int>(shop.getItems().size()),
                "Veuillez choisir un article affiché, ou 0 pour revenir."
            );

            if (itemChoice == 0)
            {
                continue;
            }

            ShopItem& item = shop.getMutableItems()[itemChoice - 1];
            bool itemMenuOpen = true;

            while (itemMenuOpen)
            {
                Console::clear();
                inspectShopItem(item, player);

                std::cout << "0 : Retour" << std::endl;
                std::cout << "1 : Acheter" << std::endl;
                std::cout << "2 : Inspecter encore" << std::endl;
                std::cout << "> ";

                int actionChoice = Console::askNumberBetween(
                    0,
                    2,
                    "Veuillez choisir 0, 1 ou 2."
                );

                if (actionChoice == 0)
                {
                    itemMenuOpen = false;
                }
                else if (actionChoice == 1)
                {
                    int finalPrice = ShopPriceRules::applyBuyModifier(
                        item.getBuyPrice(),
                        player.getRaceText(),
                        player.getType()
                    );

                    ShopTransactionSystem::buyItem(player, item, finalPrice);
                    Console::waitForEnter();
                    itemMenuOpen = false;
                }
                else
                {
                    Console::waitForEnter();
                }
            }
        }
    }
}

// EN: displayPreview declares or implements a focused behavior used by this module.
// FR: displayPreview déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
void ShopMenu::open(Player& player)
{
    std::vector<ShopInventory> shops = ShopCatalog::createAllPreviewShops();

    if (ShopRotationSystem::shouldRefreshShops())
    {
        std::cout << "Les marchands changent leurs étals après ton dernier combat." << std::endl;
        std::cout << "De nouveaux articles peuvent apparaître, disparaître ou revenir plus cher." << std::endl;
        std::cout << std::endl;
        ShopRotationSystem::markShopsRefreshed();
        Console::waitForEnter();
    }

    bool stayInMenu = true;

    while (stayInMenu)
    {
        Console::clear();
        displayShopList(shops);

        std::cout << "Or : " << player.getInventory().getGold() << " pièces" << std::endl;
        std::cout << "Note : les stocks changent après les combats, et certaines ventes sont rares." << std::endl;
        std::cout << "La revente protège l’équipement porté et les objets de base." << std::endl;
        std::cout << "Le marché noir vend parfois des composants interdits, expérimentaux ou instables." << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            static_cast<int>(shops.size()),
            "Veuillez choisir une boutique affichée, ou 0 pour revenir."
        );

        if (choice == 0)
        {
            stayInMenu = false;
            continue;
        }

        openSingleShop(player, shops[choice - 1]);
    }
}
