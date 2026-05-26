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
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    struct BarterRequirement
    {
        std::string materialId;
        std::string label;
        int quantity;
    };

    std::vector<BarterRequirement> getBlackMarketBarterRequirements(const ShopItem& item)
    {
        const std::string id = item.getId();

        if (id == "experimental_damage_potion")
        {
            return {
                {"slime_residue", "Résidu de slime", 4},
                {"arcane_dust", "Poussière arcanique", 2}
            };
        }

        if (id == "smoke_escape_vial")
        {
            return {
                {"slime_residue", "Résidu de slime", 3},
                {"shadow_thread", "Fil d'ombre", 1}
            };
        }

        if (id == "major_healing_potion")
        {
            return {
                {"bitter_healing_leaf", "Feuille amère de soin", 6},
                {"mountain_blue_flower", "Fleur bleue de montagne", 1}
            };
        }

        if (id == "greater_defensive_potion")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 4},
                {"arcane_dust", "Poussière arcanique", 2}
            };
        }

        if (id == "balanced_throwing_knives")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 2},
                {"worn_leather_piece", "Morceau de cuir abîmé", 1}
            };
        }

        if (id == "barbed_arrows")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 2},
                {"wolf_fang", "Croc de loup", 1}
            };
        }

        if (id == "piercing_bolts")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 3},
                {"cracked_bone", "Os fissuré", 1}
            };
        }

        if (id == "ash_arrows")
        {
            return {
                {"arcane_dust", "Poussière arcanique", 2},
                {"bitter_healing_leaf", "Feuille amère de soin", 2}
            };
        }

        if (id == "frozen_bolts")
        {
            return {
                {"mountain_blue_flower", "Fleur bleue de montagne", 2},
                {"arcane_dust", "Poussière arcanique", 1}
            };
        }

        if (id == "conductive_knives")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 3},
                {"unstable_core", "Noyau instable", 1}
            };
        }

        if (id == "venom_arrows")
        {
            return {
                {"bitter_healing_leaf", "Feuille amère de soin", 3},
                {"slime_residue", "Résidu de slime", 2}
            };
        }

        if (id == "shock_bolts")
        {
            return {
                {"rusted_metal_fragment", "Fragment de métal rouillé", 4},
                {"unstable_core", "Noyau instable", 1}
            };
        }

        if (id == "smoke_knives")
        {
            return {
                {"balanced_throwing_knives", "Couteaux équilibrés", 1},
                {"slime_residue", "Résidu de slime", 3}
            };
        }

        if (id == "unstable_core")
        {
            return {
                {"slime_residue", "Résidu de slime", 6},
                {"arcane_dust", "Poussière arcanique", 3}
            };
        }

        if (id == "shadow_thread")
        {
            return {
                {"wolf_fang", "Croc de loup", 3},
                {"arcane_dust", "Poussière arcanique", 2}
            };
        }

        if (id == "kitsune_ember")
        {
            return {
                {"mountain_blue_flower", "Fleur bleue de montagne", 2},
                {"arcane_dust", "Poussière arcanique", 4}
            };
        }

        if (id == "draconic_scale_fragment")
        {
            return {
                {"beast_hide", "Peau de bête", 3},
                {"rusted_metal_fragment", "Fragment de métal rouillé", 5}
            };
        }

        if (id == "precision_harvest_tools")
        {
            return {
                {"worn_leather_piece", "Morceau de cuir abîmé", 3},
                {"rusted_metal_fragment", "Fragment de métal rouillé", 6},
                {"arcane_dust", "Poussière arcanique", 1}
            };
        }

        if (id == "preservation_vials")
        {
            return {
                {"slime_residue", "Résidu de slime", 4},
                {"arcane_dust", "Poussière arcanique", 4}
            };
        }

        if (id == "anomaly_glitch_fragment")
        {
            return {
                {"unstable_core", "Noyau instable", 1},
                {"shadow_thread", "Fil d'ombre", 2},
                {"arcane_dust", "Poussière arcanique", 5}
            };
        }

        if (id == "tinkerer_complete_repair_kit")
        {
            return {
                {"medium_repair_kit", "Kit de réparation moyen", 1},
                {"draconic_scale_fragment", "Fragment d'écaille draconique", 1},
                {"rusted_metal_fragment", "Fragment de métal rouillé", 8}
            };
        }

        return {};
    }

    bool hasBlackMarketBarterOffer(const ShopInventory& shop, const ShopItem& item)
    {
        return shop.getType() == ShopType::BlackMarket
            && ShopTransactionSystem::canBeBoughtNow(item)
            && !getBlackMarketBarterRequirements(item).empty();
    }

    int getMaxBarterQuantity(const ShopItem& item, const Player& player)
    {
        if (!ShopTransactionSystem::canBeBoughtNow(item) || item.isSoldOut())
        {
            return 0;
        }

        std::vector<BarterRequirement> requirements = getBlackMarketBarterRequirements(item);
        if (requirements.empty())
        {
            return 0;
        }

        int maxQuantity = item.getStock() > 0 ? item.getStock() : 99;

        for (const BarterRequirement& requirement : requirements)
        {
            if (requirement.quantity <= 0)
            {
                continue;
            }

            int available = player.getInventory().countMaterialById(requirement.materialId);
            maxQuantity = std::min(maxQuantity, available / requirement.quantity);
        }

        if (item.isCommonInformation())
        {
            maxQuantity = std::min(maxQuantity, 1);
        }

        return std::max(0, maxQuantity);
    }

    std::string formatBarterRequirements(const ShopItem& item)
    {
        std::vector<BarterRequirement> requirements = getBlackMarketBarterRequirements(item);
        std::string text;

        for (std::size_t i = 0; i < requirements.size(); ++i)
        {
            if (i > 0)
            {
                text += " + ";
            }

            text += requirements[i].label + " x" + std::to_string(requirements[i].quantity);
        }

        return text;
    }

    bool consumeBarterRequirements(Player& player, const ShopItem& item, int quantity)
    {
        if (quantity <= 0)
        {
            return false;
        }

        std::vector<BarterRequirement> requirements = getBlackMarketBarterRequirements(item);
        if (requirements.empty())
        {
            return false;
        }

        for (const BarterRequirement& requirement : requirements)
        {
            if (player.getInventory().countMaterialById(requirement.materialId) < requirement.quantity * quantity)
            {
                return false;
            }
        }

        for (const BarterRequirement& requirement : requirements)
        {
            if (!player.getInventory().removeMaterialQuantityById(requirement.materialId, requirement.quantity * quantity))
            {
                return false;
            }
        }

        return true;
    }

    void refundBarterRequirements(Player& player, const ShopItem& item, int quantity)
    {
        if (quantity <= 0)
        {
            return;
        }

        for (const BarterRequirement& requirement : getBlackMarketBarterRequirements(item))
        {
            if (requirement.quantity <= 0)
            {
                continue;
            }

            player.getInventory().addMaterial(
                MaterialCatalog::createById(requirement.materialId, requirement.quantity * quantity)
            );
        }
    }

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

    MenuScreen buildShopListScreen(const std::vector<ShopInventory>& shops, const Player* player)
    {
        MenuScreen screen("BOUTIQUES", "shop.hub");

        if (player != nullptr)
        {
            screen.addLine("Or : " + std::to_string(player->getInventory().getGold()) + " pièces");
            screen.addLine("Les stocks changent après les combats, et certaines ventes restent rares.");
            screen.addLine("La revente protège l’équipement porté et les objets de base.");
            screen.addLine("Le marché noir vend parfois des composants interdits, expérimentaux ou instables.");
        }
        else
        {
            screen.addLine("Les boutiques seront renouvelées après chaque combat.");
        }

        screen.addOption(0, "Retour", "", true, "shop.back");

        for (std::size_t i = 0; i < shops.size(); ++i)
        {
            screen.addOption(
                static_cast<int>(i + 1),
                shops[i].getName(),
                "",
                true,
                "shop.open." + std::to_string(i + 1)
            );
        }

        return screen;
    }

    MenuScreen buildShopMainScreen(const ShopInventory& shop, const Player& player)
    {
        const std::string vendorName = getVendorNameForShop(shop.getType());
        MenuScreen screen(shop.getName(), "shop.single");
        screen.addLine("Or disponible : " + std::to_string(player.getInventory().getGold()) + " pièces");
        screen.addLine("Interlocuteur : " + vendorName);
        screen.addOption(0, "Retour", "", true, "shop.single.back");
        screen.addOption(1, "Acheter", "", true, "shop.single.buy");
        screen.addOption(2, "Vendre", "", true, "shop.single.sell");
        screen.addOption(3, "Discuter avec " + vendorName, "", true, "shop.single.talk");
        screen.addOption(4, "Quêtes de " + vendorName, "", true, "shop.single.quest");
        return screen;
    }

    MenuScreen buildVendorTalkScreen(const ShopInventory& shop)
    {
        const std::string vendorName = getVendorNameForShop(shop.getType());
        MenuScreen screen("DISCUSSION", "shop.vendor_talk");
        screen.addLine(vendorName + " prend quelques secondes pour parler boutique, rumeurs et besoins du moment.");
        screen.addLine("S'il a une vraie demande, utilise l'option de quêtes juste en dessous.");
        screen.addOption(0, "Continuer", "", true, "shop.vendor_talk.continue");
        return screen;
    }

    MenuScreen buildShopStockScreen(const ShopInventory& shop, const Player& player, std::size_t pageIndex, std::size_t itemsPerPage)
    {
        const std::vector<ShopItem>& items = shop.getItems();
        const std::size_t totalPages = PagedMenu::pageCount(items.size(), itemsPerPage);
        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(items.size(), pageIndex, itemsPerPage);

        MenuScreen screen(shop.getName(), "shop.stock");
        screen.addLine("Or disponible : " + std::to_string(player.getInventory().getGold()) + " pièces");
        screen.addLine("Race : " + player.getRaceText());

        if (items.empty())
        {
            screen.addLine("Aucun article disponible pour le moment.");
            screen.addOption(0, "Retour", "", true, "shop.stock.back");
            return screen;
        }

        screen.addLine("Page " + std::to_string(pageIndex + 1) + " / " + std::to_string(totalPages));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, items.size()));

        for (std::size_t i = first; i < last; ++i)
        {
            const int localIndex = static_cast<int>(i - first + 1);
            int finalPrice = ShopPriceRules::applyBuyModifier(
                items[i].getBuyPrice(),
                player.getRaceText(),
                player.getType()
            );

            std::string label = items[i].getName()
                + " | Prix : " + std::to_string(finalPrice) + " or";

            if (items[i].getStock() > 0)
            {
                label += " | Stock : " + std::to_string(items[i].getStock());
            }

            if (items[i].isSoldOut())
            {
                label += " | Épuisé";
            }
            else if (!ShopTransactionSystem::canBeBoughtNow(items[i]))
            {
                label += " | Indisponible";
            }

            if (hasBlackMarketBarterOffer(shop, items[i]))
            {
                const int barterMax = getMaxBarterQuantity(items[i], player);
                label += " | Troc : " + formatBarterRequirements(items[i]);
                label += barterMax > 0
                    ? " | Troc possible x" + std::to_string(barterMax)
                    : " | Troc impossible maintenant";
            }

            screen.addOption(localIndex, label, "", true, "shop.stock.select." + std::to_string(i));
        }

        if (pageIndex > 0)
        {
            screen.addOption(98, "Page précédente", "", true, "shop.stock.previous");
        }
        if (pageIndex + 1 < totalPages)
        {
            screen.addOption(99, "Page suivante", "", true, "shop.stock.next");
        }
        screen.addOption(0, "Retour", "", true, "shop.stock.back");

        return screen;
    }

    void displayShopStockPage(const ShopInventory& shop, const Player& player, std::size_t pageIndex, std::size_t itemsPerPage)
    {
        TerminalInterface::renderMenuScreen(buildShopStockScreen(shop, player, pageIndex, itemsPerPage));
    }

    // EN: inspectShopItem declares or implements a focused behavior used by this module.
    // FR: inspectShopItem déclare ou implémente un comportement précis utilisé par ce module.

    int getMaxBuyQuantity(const ShopItem& item, const Player& player, int finalPrice)
    {
        if (!ShopTransactionSystem::canBeBoughtNow(item) || item.isSoldOut())
        {
            return 0;
        }

        int affordable = finalPrice <= 0 ? 99 : player.getInventory().getGold() / finalPrice;
        if (affordable <= 0)
        {
            return 0;
        }

        int stockLimit = item.getStock() > 0 ? item.getStock() : 99;
        int maxQuantity = std::min(stockLimit, affordable);

        if (item.isCommonInformation())
        {
            maxQuantity = std::min(maxQuantity, 1);
        }

        return std::max(0, maxQuantity);
    }

    MenuScreen buildShopItemScreen(const ShopInventory& shop, const ShopItem& item, const Player& player, bool withActions)
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

        MenuScreen screen("ARTICLE", "shop.item");
        screen.addLine("Nom : " + item.getName());
        screen.addLine("Description : " + item.getDescription());
        screen.addLine("Prix d'achat : " + std::to_string(finalBuyPrice) + " or");

        int maxBuyQuantity = getMaxBuyQuantity(item, player, finalBuyPrice);
        if (maxBuyQuantity > 0)
        {
            screen.addLine("Quantité achetable maintenant : max x" + std::to_string(maxBuyQuantity));
        }

        if (hasBlackMarketBarterOffer(shop, item))
        {
            int maxBarterQuantity = getMaxBarterQuantity(item, player);
            screen.addLine("Troc du marché noir : " + formatBarterRequirements(item));

            if (maxBarterQuantity > 0)
            {
                screen.addLine("Quantité échangeable maintenant : max x" + std::to_string(maxBarterQuantity));
            }
            else
            {
                screen.addLine("Troc impossible maintenant : composants insuffisants.");
            }
        }

        screen.addLine("Prix de revente estimé : " + std::to_string(finalSellPrice) + " or");

        if (player.getRaceText().find("Démon") != std::string::npos
            || player.getRaceText().find("démon") != std::string::npos)
        {
            screen.addLine("Note : ton apparence démoniaque influence déjà certains prix.");
        }

        if (ShopPriceRules::hasCraftClassTradeBonus(player.getType()))
        {
            screen.addLine("Note : ta classe d'artisanat négocie légèrement mieux les prix.");
        }

        if (!ShopTransactionSystem::canBeBoughtNow(item))
        {
            screen.addLine("Statut : le marchand refuse de sortir cet article pour le moment.");
        }

        if (withActions)
        {
            const bool barterAvailable = hasBlackMarketBarterOffer(shop, item);
            screen.addOption(0, "Retour", "", true, "shop.item.back");
            screen.addOption(1, "Acheter avec de l'or", "", true, "shop.item.buy");
            screen.addOption(2, "Inspecter encore", "", true, "shop.item.inspect");
            if (barterAvailable)
            {
                screen.addOption(3, "Troquer des objets", "", true, "shop.item.barter");
            }
        }

        return screen;
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

            int maxQuantity = ShopTransactionSystem::getMaxSellQuantityForEntry(player, shop.getType(), index);
            int quantity = 1;

            if (maxQuantity > 1)
            {
                std::cout << "Quantité à vendre ? Max : x" << maxQuantity << std::endl;
                std::cout << "> ";
                quantity = Console::askNumberBetween(
                    1,
                    maxQuantity,
                    "Veuillez choisir une quantité valide."
                );
                std::cout << std::endl;
            }

            std::cout << "Confirmer la vente de x" << quantity
                      << " pour " << sellPrice * quantity << " or ?" << std::endl;
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
                ShopTransactionSystem::sellInventoryEntryQuantity(
                    player,
                    shop.getType(),
                    index,
                    sellPrice,
                    quantity
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
            TerminalInterface::renderMenuScreen(buildShopMainScreen(shop, player));
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
                TerminalInterface::renderMenuScreen(buildVendorTalkScreen(shop));
                Console::waitForEnter();
                continue;
            }

            if (shopChoice == 4)
            {
                Console::clear();
                QuestMenu::talkToClient(player, getVendorNameForShop(shop.getType()));
                continue;
            }

            bool buyMenuOpen = true;
            std::size_t pageIndex = 0;
            const std::size_t itemsPerPage = 10;

            while (buyMenuOpen)
            {
                Console::clear();
                displayShopStockPage(shop, player, pageIndex, itemsPerPage);
                std::cout << "> ";

                const std::vector<ShopItem>& items = shop.getItems();
                const std::size_t totalPages = PagedMenu::pageCount(items.size(), itemsPerPage);
                const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
                const std::size_t last = PagedMenu::lastIndexExclusive(items.size(), pageIndex, itemsPerPage);
                const int localCount = static_cast<int>(last - first);

                int itemChoice = Console::askNumberBetween(
                    0,
                    99,
                    "Veuillez choisir un article affiché, 98/99 pour tourner les pages, ou 0 pour revenir."
                );

                if (itemChoice == 0)
                {
                    buyMenuOpen = false;
                    continue;
                }

                if (itemChoice == 98 && pageIndex > 0)
                {
                    pageIndex--;
                    continue;
                }

                if (itemChoice == 99 && pageIndex + 1 < totalPages)
                {
                    pageIndex++;
                    continue;
                }

                if (itemChoice < 1 || itemChoice > localCount)
                {
                    std::cout << "Choix indisponible sur cette page." << std::endl;
                    Console::waitForEnter();
                    continue;
                }

                ShopItem& item = shop.getMutableItems()[first + static_cast<std::size_t>(itemChoice - 1)];
                bool itemMenuOpen = true;

                while (itemMenuOpen)
                {
                    Console::clear();
                    bool barterAvailable = hasBlackMarketBarterOffer(shop, item);
                    int maxActionChoice = barterAvailable ? 3 : 2;
                    TerminalInterface::renderMenuScreen(buildShopItemScreen(shop, item, player, true));
                    std::cout << "> ";

                    int actionChoice = Console::askNumberBetween(
                        0,
                        maxActionChoice,
                        barterAvailable ? "Veuillez choisir 0, 1, 2 ou 3." : "Veuillez choisir 0, 1 ou 2."
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

                        int maxQuantity = getMaxBuyQuantity(item, player, finalPrice);

                        if (maxQuantity <= 0)
                        {
                            std::cout << "Achat impossible maintenant." << std::endl;
                            std::cout << "Vérifie ton or, le stock ou le statut réel de l'article." << std::endl;
                            std::cout << std::endl;
                        }
                        else
                        {
                            int quantity = 1;

                            if (maxQuantity > 1)
                            {
                                std::cout << "Combien veux-tu acheter ? Max : x" << maxQuantity << std::endl;
                                std::cout << "> ";
                                quantity = Console::askNumberBetween(
                                    1,
                                    maxQuantity,
                                    "Veuillez choisir une quantité valide."
                                );
                                std::cout << std::endl;
                            }

                            int boughtCount = 0;
                            for (int i = 0; i < quantity; ++i)
                            {
                                if (ShopTransactionSystem::buyItem(player, item, finalPrice))
                                {
                                    boughtCount++;
                                }
                                else
                                {
                                    break;
                                }
                            }

                            if (quantity > 1)
                            {
                                std::cout << "Résumé achat groupé : x" << boughtCount
                                          << " / x" << quantity << " obtenu(s)." << std::endl;
                                std::cout << std::endl;
                            }
                        }

                        Console::waitForEnter();
                        itemMenuOpen = false;
                    }
                    else if (actionChoice == 3)
                    {
                        int maxBarterQuantity = getMaxBarterQuantity(item, player);

                        if (maxBarterQuantity <= 0)
                        {
                            std::cout << "Troc impossible maintenant." << std::endl;
                            std::cout << "Il te manque au moins un composant demandé par ce contact douteux." << std::endl;
                            std::cout << std::endl;
                        }
                        else
                        {
                            int quantity = 1;

                            if (maxBarterQuantity > 1)
                            {
                                std::cout << "Combien veux-tu troquer ? Max : x" << maxBarterQuantity << std::endl;
                                std::cout << "> ";
                                quantity = Console::askNumberBetween(
                                    1,
                                    maxBarterQuantity,
                                    "Veuillez choisir une quantité valide."
                                );
                                std::cout << std::endl;
                            }

                            std::cout << "Le contact demande : " << formatBarterRequirements(item);
                            if (quantity > 1)
                            {
                                std::cout << " par unité";
                            }
                            std::cout << "." << std::endl;
                            std::cout << "Confirmer le troc pour x" << quantity << " ?" << std::endl;
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
                                int tradedCount = 0;

                                for (int i = 0; i < quantity; ++i)
                                {
                                    if (getMaxBarterQuantity(item, player) <= 0)
                                    {
                                        break;
                                    }

                                    if (!consumeBarterRequirements(player, item, 1))
                                    {
                                        std::cout << "Le contact vérifie les composants, puis referme sa sacoche : il manque quelque chose." << std::endl;
                                        break;
                                    }

                                    if (!ShopTransactionSystem::buyItem(player, item, 0))
                                    {
                                        refundBarterRequirements(player, item, 1);
                                        std::cout << "Le contact refuse finalement l'échange et rend les composants." << std::endl;
                                        break;
                                    }

                                    tradedCount++;
                                }

                                std::cout << "Résumé du troc : x" << tradedCount
                                          << " / x" << quantity << " obtenu(s)." << std::endl;
                                std::cout << "Le contact range les composants sans demander ton nom." << std::endl;
                                std::cout << std::endl;
                            }
                            else
                            {
                                std::cout << "Troc annulé." << std::endl;
                                std::cout << std::endl;
                            }
                        }

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
}

// EN: displayPreview declares or implements a focused behavior used by this module.
// FR: displayPreview déclare ou implémente un comportement précis utilisé par ce module.
void ShopMenu::displayPreview()
{
    std::vector<ShopInventory> shops = ShopCatalog::createAllPreviewShops();
    TerminalInterface::renderMenuScreen(buildShopListScreen(shops, nullptr));
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
        TerminalInterface::renderMenuScreen(buildShopListScreen(shops, &player));
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
