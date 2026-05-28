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
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/material/Material.hpp"

#include <algorithm>
#include <iostream>
#include <random>
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

    struct SellableEntryUiInfo
    {
        std::string name;
        std::string quantity;
        std::string price;
        std::string maxQuantity;
        std::string status;
        std::string detail;
        std::string label;
        bool sellable = false;
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


    std::string chooseRandomLine(const std::vector<std::string>& lines)
    {
        if (lines.empty())
        {
            return "";
        }

        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(0, static_cast<int>(lines.size()) - 1);
        return lines[distribution(generator)];
    }

    std::string chooseShopIntroLine(ShopType type)
    {
        if (type == ShopType::BlackMarket)
        {
            return chooseRandomLine({
                "Un rideau se ferme derrière toi. Le contact ne demande pas ton nom, ce qui est rarement bon signe.",
                "Le contact tapote le comptoir : ici, les garanties durent moins longtemps que les mensonges.",
                "Une odeur de métal froid flotte dans l'air. Même les prix ont l'air de cacher quelque chose."
            });
        }

        if (type == ShopType::Library)
        {
            return chooseRandomLine({
                "La bibliothécaire relève les yeux : les livres dangereux sont rangés assez haut pour décourager les idiots motivés.",
                "Des pages bougent toutes seules dans un coin. Personne ne commente, donc tu fais pareil.",
                "Le silence ici pèse plus lourd qu'une armure, mais au moins il ne coûte pas encore de taxe."
            });
        }

        if (type == ShopType::Weapon || type == ShopType::Blacksmith)
        {
            return chooseRandomLine({
                "Le métal chante derrière le comptoir. Le vendeur sourit comme si une bonne lame réglait tous les débats.",
                "On te jauge les bras avant de te montrer les articles. Apparemment, le style ne suffit pas à porter une hache.",
                "Un client teste une lame dans le vide. Tout le monde fait semblant que c'était maîtrisé."
            });
        }

        if (type == ShopType::Armor)
        {
            return chooseRandomLine({
                "L'armurier tape sur une cuirasse : si ça sonne creux, c'est soit fragile, soit toi dedans.",
                "Des protections cabossées attendent réparation. Certaines ont clairement vécu une meilleure histoire que leur propriétaire.",
                "Le vendeur inspecte tes épaules comme s'il savait déjà où le prochain monstre va mordre."
            });
        }

        if (type == ShopType::Plant || type == ShopType::Alchemist || type == ShopType::Consumable)
        {
            return chooseRandomLine({
                "Des flacons frémissent doucement. L'étiquette 'ne pas boire' semble surtout être une suggestion juridique.",
                "L'odeur des plantes couvre presque celle des expériences ratées. Presque.",
                "Le vendeur range une fiole trop vite. Tu décides de ne pas demander ce qu'elle faisait avant ton arrivée."
            });
        }

        return chooseRandomLine({
            "Le marchand t'accueille avec le sourire prudent de quelqu'un qui a déjà vu des aventuriers compter jusqu'à trois avec difficulté.",
            "Le comptoir craque sous les marchandises. Lui, au moins, a une barre de durabilité réaliste.",
            "Quelques clients chuchotent. Visiblement, ici aussi, ton inventaire intéresse plus de monde que ta santé mentale."
        });
    }


    std::vector<std::string> chooseVendorTalkLines(ShopType type)
    {
        std::vector<std::string> lines;

        if (type == ShopType::MonsterMaterial)
        {
            lines.push_back(chooseRandomLine({
                "Le vendeur aligne trois griffes sur le comptoir : deux sont utiles, la troisième est probablement juste là pour impressionner les débutants.",
                "Il explique que les bons composants ne sentent pas toujours bon, mais que les composants trop propres mentent souvent.",
                "Il conseille de noter quelle créature a donné quoi : dans ce métier, confondre une dent et une écaille finit rarement bien."
            }));
            lines.push_back("Rumeur : certaines carcasses réagissent mieux si le coup final n'a pas broyé la matière intéressante.");
            return lines;
        }

        if (type == ShopType::Material || type == ShopType::Blacksmith)
        {
            lines.push_back(chooseRandomLine({
                "Le vendeur parle densité, veines de métal et réparations comme si tout le monde rêvait de dormir dans une forge.",
                "Il te montre une fissure presque invisible : selon lui, c'est là que la moitié des aventuriers perdent leur argent avant de perdre leur bras.",
                "Le comptoir porte des marques de test. Visiblement, taper sur les choses reste une méthode scientifique locale."
            }));
            lines.push_back("Conseil : une arme qui frappe une matière trop dure s'use plus vite, même si elle gagne le duel sur le moment.");
            return lines;
        }

        if (type == ShopType::Weapon)
        {
            lines.push_back(chooseRandomLine({
                "Le vendeur affirme qu'une arme choisie au hasard est une arme qui cherche déjà son prochain propriétaire.",
                "Il parle équilibre, portée et rythme. Puis il regarde ton inventaire avec la tête de quelqu'un qui a envie de tout ranger lui-même.",
                "Il rappelle qu'une lame héroïque mérite un minimum de respect, et idéalement quelqu'un qui sait de quel côté elle coupe."
            }));
            lines.push_back("Rumeur : quelques combattants apprennent de nouvelles techniques seulement après avoir vraiment insisté avec le même type d'arme.");
            return lines;
        }

        if (type == ShopType::Armor)
        {
            lines.push_back(chooseRandomLine({
                "L'armurier assure que la meilleure armure est celle qu'on remarque avant que le monstre ne remarque tes côtes.",
                "Il décrit des protections légères, lourdes, souples, puis soupire en disant que personne ne lit les faiblesses avant le premier impact.",
                "Il tape sur une épaulière : le bruit est rassurant, ou inquiétant, selon ton optimisme."
            }));
            lines.push_back("Conseil : encaisser un choc trop violent peut abîmer l'équipement, même si les PV tiennent encore debout.");
            return lines;
        }

        if (type == ShopType::Plant || type == ShopType::Alchemist || type == ShopType::Consumable)
        {
            lines.push_back(chooseRandomLine({
                "L'herboriste parle de dosage avec le calme d'une personne qui a déjà vu quelqu'un boire une potion offensive par curiosité.",
                "L'alchimiste explique que la couleur d'une fiole ne garantit rien, sauf peut-être la couleur de la panique après usage.",
                "Le vendeur conseille de garder une potion de soin rapide séparée du reste. Il dit ça comme si la survie aimait les raccourcis propres."
            }));
            lines.push_back("Rumeur : certains mélanges rares demanderont des plantes et matériaux que les boutiques ne vendent presque jamais ensemble.");
            return lines;
        }

        if (type == ShopType::Library)
        {
            lines.push_back(chooseRandomLine({
                "La bibliothécaire baisse la voix : certains grimoires ne lancent pas un sort, ils apprennent au lecteur à le mériter.",
                "Elle range un livre qui semble respirer. Elle prétend que c'est normal. Le livre n'a pas l'air d'accord.",
                "Elle rappelle que connaître une faiblesse avant de frapper coûte moins cher qu'apprendre la même chose avec son visage."
            }));
            lines.push_back("Conseil : le bestiaire et les renseignements doivent rester la base des recommandations tactiques, sinon c'est juste de la triche mal habillée.");
            return lines;
        }

        if (type == ShopType::BlackMarket)
        {
            lines.push_back(chooseRandomLine({
                "Le contact parle de stocks oubliés, d'objets sans facture et de garanties qui s'évaporent dès qu'on les relit.",
                "Il te conseille de ne pas poser de questions, ce qui est exactement le genre de phrase qui donne envie d'en poser douze.",
                "Il glisse que les meilleurs prix ne sont pas toujours en or. Parfois, ils coûtent surtout en problèmes futurs."
            }));
            lines.push_back("Rumeur : quelques marchandises spéciales n'apparaissent qu'après des combats ou événements assez rares.");
            return lines;
        }

        lines.push_back(chooseRandomLine({
            "Le marchand parle de clients, de routes et de taxes avec l'énergie de quelqu'un qui a survécu à pire qu'un monstre : la comptabilité.",
            "Il dit que le monde change vite après chaque combat, surtout les prix, les stocks et les excuses des vendeurs.",
            "Il te conseille de ne pas tout acheter juste parce que ça brille. Puis il ajoute que si tu le fais quand même, il ne jugera pas trop fort."
        }));
        lines.push_back("Conseil : reviens après quelques combats, les étals peuvent changer et certaines occasions ne restent pas longtemps.");
        return lines;
    }

    MenuScreen buildShopConfirmationScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& confirmLabel,
        const std::string& cancelLabel,
        const std::string& actionPrefix
    )
    {
        MenuScreen screen(title, screenId);
        screen.setChoiceInput("Choisis 1 pour confirmer ou 2 pour annuler.");

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        screen.addOption(1, confirmLabel, "Valider l'action affichée.", true, actionPrefix + ".confirm");
        screen.addOption(2, cancelLabel, "Revenir sans rien changer.", true, actionPrefix + ".cancel");
        return screen;
    }

    bool askShopConfirmation(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& confirmLabel,
        const std::string& cancelLabel,
        const std::string& actionPrefix
    )
    {
        Console::clear();
        const MenuScreen screen = buildShopConfirmationScreen(
            title,
            screenId,
            lines,
            confirmLabel,
            cancelLabel,
            actionPrefix
        );

        const int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix refusé : utilise 1 pour confirmer ou 2 pour annuler."
        );
        return choice == 1;
    }

    std::vector<std::string> withTransactionNotes(std::vector<std::string> lines)
    {
        const std::vector<std::string> notes = ShopTransactionSystem::consumeLastTransactionNotes();
        if (!notes.empty())
        {
            lines.push_back("");
            lines.push_back("Détails de transaction :");
            for (const std::string& note : notes)
            {
                lines.push_back("- " + note);
            }
        }
        return lines;
    }

    void showShopResult(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        Console::clear();
        MessageScreen::show(title, screenId, lines);
    }

    void showShopTransactionResult(
        const std::string& title,
        const std::string& screenId,
        std::vector<std::string> lines
    )
    {
        showShopResult(title, screenId, withTransactionNotes(lines));
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
        screen.addLine("Accueil : " + chooseShopIntroLine(shop.getType()));

        const int buybackCount = ShopTransactionSystem::getBuybackEntryCount(shop.getType());
        if (buybackCount > 0)
        {
            screen.addLine("Rachat disponible : " + std::to_string(buybackCount) + " vente(s) récupérable(s) avant le prochain combat.");
        }
        else
        {
            screen.addLine("Rachat disponible : aucune vente récente dans cette boutique.");
        }

        screen.addOption(0, "Retour", "", true, "shop.single.back");
        screen.addOption(1, "Acheter", "Voir le stock et les prix de cette boutique.", true, "shop.single.buy");
        screen.addOption(2, "Vendre", "Proposer des objets compatibles avec ce marchand.", true, "shop.single.sell");
        screen.addOption(3, "Discuter avec " + vendorName, "", true, "shop.single.talk");
        screen.addOption(4, "Quêtes de " + vendorName, "", true, "shop.single.quest");
        screen.addOption(
            5,
            "Racheter une vente récente",
            buybackCount > 0
                ? "Seulement avant le prochain combat, avec un surcoût de récupération."
                : "Aucune vente récente n'est récupérable ici pour le moment.",
            buybackCount > 0,
            "shop.single.buyback"
        );
        return screen;
    }

    int getMaxBuyQuantity(const ShopItem& item, const Player& player, int finalPrice);

    MenuScreen buildVendorTalkScreen(const ShopInventory& shop)
    {
        const std::string vendorName = getVendorNameForShop(shop.getType());
        MenuScreen screen("DISCUSSION", "shop.vendor_talk");
        screen.addLine(vendorName + " prend quelques secondes pour parler boutique, rumeurs et besoins du moment.");

        const std::vector<std::string> talkLines = chooseVendorTalkLines(shop.getType());
        for (const std::string& line : talkLines)
        {
            screen.addLine(line);
        }

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

        screen.setPagination(pageIndex, totalPages);
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

            const bool canBuyNow = ShopTransactionSystem::canBeBoughtNow(items[i]);
            const bool soldOut = items[i].isSoldOut();
            const bool barterOffer = hasBlackMarketBarterOffer(shop, items[i]);
            const int barterMax = barterOffer ? getMaxBarterQuantity(items[i], player) : 0;
            const std::string categoryLabel = shopItemCategoryToText(items[i].getCategory());

            std::string label = items[i].getName()
                + " | Catégorie : " + categoryLabel
                + " | Prix : " + std::to_string(finalPrice) + " or";

            if (items[i].getStock() >= 0)
            {
                label += " | Stock : " + std::to_string(items[i].getStock());
            }
            else
            {
                label += " | Stock : non limité";
            }

            if (soldOut)
            {
                label += " | Épuisé";
            }
            else if (!canBuyNow)
            {
                label += " | Indisponible";
            }

            if (barterOffer)
            {
                label += " | Troc : " + formatBarterRequirements(items[i]);
                label += barterMax > 0
                    ? " | Troc possible x" + std::to_string(barterMax)
                    : " | Troc impossible maintenant";
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "shop";
            itemData.section = categoryLabel;
            itemData.actionType = barterOffer ? "buy" : "buy";
            itemData.name = items[i].getName();
            itemData.detail = items[i].getDescription();
            itemData.price = std::to_string(finalPrice) + " or";
            itemData.stock = items[i].getStock() >= 0 ? std::to_string(items[i].getStock()) : "non limité";
            if (soldOut)
            {
                itemData.status = "Épuisé";
            }
            else if (!canBuyNow)
            {
                itemData.status = "Indisponible";
            }
            else if (getMaxBuyQuantity(items[i], player, finalPrice) <= 0)
            {
                itemData.status = "Or insuffisant";
            }
            if (barterOffer)
            {
                itemData.reward = "Troc : " + formatBarterRequirements(items[i]);
                itemData.maxQuantity = barterMax > 0 ? std::to_string(barterMax) : "0";
            }
            itemData.important = soldOut || !canBuyNow || barterOffer;

            screen.addOption(localIndex, label, "", true, "shop.stock.select." + std::to_string(i), itemData);
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


    SellableEntryUiInfo getSellableEntryUiInfo(const Player& player, ShopType shopType, int index)
    {
        SellableEntryUiInfo info;

        if (shopType == ShopType::Weapon && player.getInventory().hasWeapon(index))
        {
            info.name = player.getInventory().getWeapon(index).getName();
            info.detail = "Arme possédée par le personnage.";
        }
        else if (shopType == ShopType::Armor && player.getInventory().hasArmor(index))
        {
            info.name = player.getInventory().getArmor(index).getName();
            info.detail = "Armure ou tenue possédée par le personnage.";
        }
        else if (shopType == ShopType::Consumable && player.getInventory().hasConsumable(index))
        {
            info.name = player.getInventory().getConsumable(index).getName();
            info.detail = "Consommable présent dans l'inventaire.";
        }
        else if (player.getInventory().hasMaterial(index))
        {
            Material material = player.getInventory().getMaterial(index);
            info.name = material.getName();
            info.quantity = "x" + std::to_string(material.getQuantity());
            info.detail = "Matériau présent dans l'inventaire.";

            if (material.hasSpecialQuality())
            {
                info.status = "Qualité : " + material.getQualityLabel();
            }
        }
        else
        {
            info.name = "Entrée inconnue";
            info.status = "Invalide";
            info.detail = "Cette entrée ne peut pas être résolue dans l'inventaire.";
        }

        info.sellable = ShopTransactionSystem::canShopBuyInventoryEntry(player, shopType, index);
        if (!info.sellable)
        {
            if (info.status.empty())
            {
                info.status = "Protégé";
            }
            info.label = info.name;
            if (!info.quantity.empty())
            {
                info.label += " " + info.quantity;
            }
            info.label += " | Statut : " + info.status;
            return info;
        }

        const int sellPrice = ShopTransactionSystem::getSellPriceForEntry(player, shopType, index);
        const int maxQuantity = ShopTransactionSystem::getMaxSellQuantityForEntry(player, shopType, index);
        info.price = std::to_string(sellPrice) + " or";
        info.maxQuantity = "x" + std::to_string(maxQuantity);

        info.label = info.name;
        if (!info.quantity.empty())
        {
            info.label += " " + info.quantity;
        }
        if (!info.status.empty())
        {
            info.label += " | " + info.status;
        }
        info.label += " | Revente : " + info.price;
        info.label += " | Max : " + info.maxQuantity;
        return info;
    }

    std::string sellableEntryLabel(const Player& player, ShopType shopType, int index)
    {
        return getSellableEntryUiInfo(player, shopType, index).label;
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
        screen.addLine("Catégorie : " + std::string(shopItemCategoryToText(item.getCategory())));
        screen.addLine("Description : " + item.getDescription());
        screen.addLine("Prix d'achat : " + std::to_string(finalBuyPrice) + " or");
        screen.addLine(item.getStock() >= 0
            ? "Stock : " + std::to_string(item.getStock())
            : "Stock : non limité");

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
            const bool canBuyWithGold = maxBuyQuantity > 0;
            const int maxBarterQuantity = barterAvailable ? getMaxBarterQuantity(item, player) : 0;

            MenuOptionItemData buyData;
            buyData.structured = true;
            buyData.kind = "shop";
            buyData.section = shopItemCategoryToText(item.getCategory());
            buyData.actionType = "buy";
            buyData.name = item.getName();
            buyData.detail = item.getDescription();
            buyData.price = std::to_string(finalBuyPrice) + " or";
            buyData.stock = item.getStock() >= 0 ? std::to_string(item.getStock()) : "non limité";
            buyData.maxQuantity = std::to_string(maxBuyQuantity);
            buyData.status = canBuyWithGold ? "Disponible" : "Bloqué";
            buyData.important = !canBuyWithGold;

            screen.addOption(0, "Retour", "", true, "shop.item.back");
            screen.addOption(
                1,
                "Acheter avec de l'or",
                canBuyWithGold
                    ? "Acheter cet article avec l'or disponible."
                    : "Achat impossible maintenant : or, stock ou disponibilité insuffisante.",
                canBuyWithGold,
                "shop.item.buy",
                buyData
            );
            screen.addOption(2, "Inspecter encore", "Relire les détails sans transaction.", true, "shop.item.inspect");

            if (barterAvailable)
            {
                MenuOptionItemData barterData;
                barterData.structured = true;
                barterData.kind = "shop";
                barterData.section = "Marché noir";
                barterData.actionType = "barter";
                barterData.name = item.getName();
                barterData.detail = item.getDescription();
                barterData.reward = "Demande : " + formatBarterRequirements(item);
                barterData.stock = item.getStock() >= 0 ? std::to_string(item.getStock()) : "non limité";
                barterData.maxQuantity = std::to_string(maxBarterQuantity);
                barterData.status = maxBarterQuantity > 0 ? "Troc possible" : "Composants insuffisants";
                barterData.important = maxBarterQuantity <= 0;

                screen.addOption(
                    3,
                    "Troquer des objets",
                    maxBarterQuantity > 0
                        ? "Échanger les composants demandés contre cet article."
                        : "Troc impossible maintenant : composants ou stock insuffisants.",
                    maxBarterQuantity > 0,
                    "shop.item.barter",
                    barterData
                );
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
            int maxChoice = ShopTransactionSystem::getSellableEntryCount(player, shop.getType());
            MenuScreen sellScreen("REVENTE", "shop.sell");
            sellScreen.addLine("Boutique : " + shop.getName());
            sellScreen.addLine("Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces");
            sellScreen.addBackOption("Retour", "shop.sell.back");

            if (maxChoice <= 0)
            {
                sellScreen.addLine("Rien à vendre ici pour le moment.");
                TerminalInterface::askMenuChoice(sellScreen, 0, 0, "Entre 0 pour revenir.");
                Console::clear();
                return;
            }

            for (int i = 0; i < maxChoice; ++i)
            {
                const SellableEntryUiInfo entryInfo = getSellableEntryUiInfo(player, shop.getType(), i);
                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "shop";
                itemData.section = "Revente";
                itemData.actionType = "sell";
                itemData.name = entryInfo.name;
                itemData.quantity = entryInfo.quantity;
                itemData.detail = entryInfo.detail;
                itemData.status = entryInfo.status;
                itemData.price = entryInfo.price;
                itemData.maxQuantity = entryInfo.maxQuantity;
                itemData.important = !entryInfo.sellable || !entryInfo.status.empty();

                sellScreen.addOption(
                    i + 1,
                    entryInfo.label,
                    entryInfo.sellable
                        ? "Vendre cet objet ou une quantité si plusieurs exemplaires sont disponibles."
                        : "Cette entrée reste visible, mais le marchand ne peut pas l'acheter.",
                    entryInfo.sellable,
                    "shop.sell.select." + std::to_string(i),
                    itemData
                );
            }

            int choice = TerminalInterface::askMenuChoiceFromOptions(
                sellScreen,
                "Choix refusé : sélectionne une entrée vendable ou 0 pour revenir."
            );

            if (choice == 0)
            {
                selling = false;
                continue;
            }

            int index = choice - 1;

            if (!ShopTransactionSystem::canShopBuyInventoryEntry(player, shop.getType(), index))
            {
                showShopResult(
                    "VENTE IMPOSSIBLE",
                    "shop.sell.blocked",
                    {
                        "Entrée : " + sellableEntryLabel(player, shop.getType(), index),
                        "Statut : protégée ou refusée par cette boutique.",
                        "Raison possible : équipement porté, objet de base, entrée invalide ou mauvais type de marchand.",
                        "Aucun objet n'a été retiré de ton inventaire."
                    }
                );
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
                quantity = MessageScreen::askQuantity(
                    "QUANTITÉ À VENDRE",
                    "shop.sell.quantity",
                    {
                        "Boutique : " + shop.getName(),
                        "Maximum vendable : x" + std::to_string(maxQuantity),
                        "Prix unitaire estimé : " + std::to_string(sellPrice) + " or"
                    },
                    1,
                    maxQuantity,
                    "Veuillez choisir une quantité valide."
                );
            }

            const int goldBeforeSale = player.getInventory().getGold();
            const std::string selectedEntryLabel = sellableEntryLabel(player, shop.getType(), index);
            const int totalSellPrice = sellPrice * quantity;

            const bool confirmSale = askShopConfirmation(
                "CONFIRMER LA VENTE",
                "shop.sell.confirm",
                {
                    "Boutique : " + shop.getName(),
                    "Objet : " + selectedEntryLabel,
                    "Quantité : x" + std::to_string(quantity),
                    "Prix unitaire : " + std::to_string(sellPrice) + " or",
                    "Total reçu : " + std::to_string(totalSellPrice) + " or",
                    "Rachat : l'objet restera récupérable ici jusqu'au prochain combat avec un surcoût."
                },
                "Confirmer la vente",
                "Annuler la vente",
                "shop.sell"
            );

            if (!confirmSale)
            {
                showShopResult(
                    "VENTE ANNULÉE",
                    "shop.sell.cancelled",
                    {
                        "Objet : " + selectedEntryLabel,
                        "Aucun objet n'a quitté ton inventaire.",
                        "Le marchand range déjà sa bourse, légèrement déçu."
                    }
                );
                continue;
            }

            ShopTransactionSystem::clearLastTransactionNotes();
            const bool saleSucceeded = ShopTransactionSystem::sellInventoryEntryQuantity(
                player,
                shop.getType(),
                index,
                sellPrice,
                quantity
            );

            showShopTransactionResult(
                saleSucceeded ? "VENTE TERMINÉE" : "VENTE REFUSÉE",
                saleSucceeded ? "shop.sell.result.success" : "shop.sell.result.failed",
                saleSucceeded
                    ? std::vector<std::string>{
                        "Objet vendu : " + selectedEntryLabel,
                        "Quantité : x" + std::to_string(quantity),
                        "Or reçu : " + std::to_string(totalSellPrice) + " pièces",
                        "Or avant : " + std::to_string(goldBeforeSale) + " pièces",
                        "Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces",
                        "Rachat : disponible dans cette boutique jusqu'au prochain combat."
                    }
                    : std::vector<std::string>{
                        "Objet demandé : " + selectedEntryLabel,
                        "Quantité demandée : x" + std::to_string(quantity),
                        "La transaction a été refusée ou interrompue.",
                        "Aucune confirmation de rachat n'est ajoutée pour cette tentative."
                    }
            );
        }
    }


    void openBuybackMenu(Player& player, const ShopInventory& shop)
    {
        bool buyingBack = true;

        while (buyingBack)
        {
            Console::clear();
            const int count = ShopTransactionSystem::getBuybackEntryCount(shop.getType());
            MenuScreen screen("RACHAT", "shop.buyback");
            screen.addLine("Boutique : " + shop.getName());
            screen.addLine("Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces");
            screen.addLine("Les objets vendus ici peuvent être rachetés jusqu'au prochain combat.");
            screen.addLine("Le prix est plus haut que la revente : frais, paperasse, mauvaise foi du marchand, bref la vie.");
            screen.addBackOption("Retour", "shop.buyback.back");

            if (count <= 0)
            {
                screen.addLine("Aucun objet à racheter dans cette boutique.");
                TerminalInterface::askMenuChoice(screen, 0, 0, "Entre 0 pour revenir.");
                return;
            }

            for (int i = 0; i < count; ++i)
            {
                const std::string name = ShopTransactionSystem::getBuybackEntryName(shop.getType(), i);
                const std::string kindLabel = ShopTransactionSystem::getBuybackEntryKindLabel(shop.getType(), i);
                const int quantity = ShopTransactionSystem::getBuybackEntryQuantity(shop.getType(), i);
                const int price = ShopTransactionSystem::getBuybackEntryPrice(shop.getType(), i);
                const bool affordable = player.getInventory().getGold() >= price;
                const std::string label = name
                    + (quantity > 1 ? " x" + std::to_string(quantity) : "")
                    + " | Type : " + kindLabel
                    + " | Rachat : " + std::to_string(price) + " or"
                    + " | " + (affordable ? "Récupérable" : "Or insuffisant");

                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "shop";
                itemData.section = "Rachat";
                itemData.actionType = "buyback";
                itemData.name = name;
                itemData.quantity = quantity > 1 ? "x" + std::to_string(quantity) : "";
                itemData.price = std::to_string(price) + " or";
                itemData.status = affordable ? "Avant prochain combat" : "Or insuffisant";
                itemData.detail = "Récupérer un objet vendu récemment dans cette boutique.";
                itemData.important = !affordable;

                screen.addOption(
                    i + 1,
                    label,
                    affordable
                        ? itemData.detail
                        : "Entrée visible, mais il manque de l'or pour la récupérer maintenant.",
                    affordable,
                    "shop.buyback.select." + std::to_string(i + 1),
                    itemData
                );
            }

            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choix refusé : sélectionne une entrée récupérable ou 0 pour revenir."
            );
            if (choice == 0)
            {
                buyingBack = false;
                continue;
            }

            const int buybackIndex = choice - 1;
            const std::string buybackName = ShopTransactionSystem::getBuybackEntryName(shop.getType(), buybackIndex);
            const int buybackQuantity = ShopTransactionSystem::getBuybackEntryQuantity(shop.getType(), buybackIndex);
            const int buybackPrice = ShopTransactionSystem::getBuybackEntryPrice(shop.getType(), buybackIndex);
            const int goldBeforeBuyback = player.getInventory().getGold();

            const bool confirmBuyback = askShopConfirmation(
                "CONFIRMER LE RACHAT",
                "shop.buyback.confirm",
                {
                    "Boutique : " + shop.getName(),
                    "Objet : " + buybackName,
                    "Prix de récupération : " + std::to_string(buybackPrice) + " or",
                    "Or disponible : " + std::to_string(goldBeforeBuyback) + " pièces",
                    "Limite : cette occasion disparaît au prochain combat."
                },
                "Racheter l'objet",
                "Annuler le rachat",
                "shop.buyback"
            );

            if (!confirmBuyback)
            {
                showShopResult(
                    "RACHAT ANNULÉ",
                    "shop.buyback.cancelled",
                    {
                        "Objet : " + buybackName,
                        "L'objet reste disponible tant qu'aucun combat n'est lancé.",
                        "Aucun or n'a été dépensé."
                    }
                );
                continue;
            }

            ShopTransactionSystem::clearLastTransactionNotes();
            const bool buybackSucceeded = ShopTransactionSystem::buyBackEntry(player, shop.getType(), buybackIndex);
            showShopTransactionResult(
                buybackSucceeded ? "RACHAT TERMINÉ" : "RACHAT REFUSÉ",
                buybackSucceeded ? "shop.buyback.result.success" : "shop.buyback.result.failed",
                buybackSucceeded
                    ? std::vector<std::string>{
                        "Objet récupéré : " + buybackName,
                        "Quantité récupérée : x" + std::to_string(std::max(1, buybackQuantity)),
                        "Prix payé : " + std::to_string(buybackPrice) + " or",
                        "Or avant : " + std::to_string(goldBeforeBuyback) + " pièces",
                        "Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces",
                        "L'entrée de rachat a été retirée de cette boutique."
                    }
                    : std::vector<std::string>{
                        "Objet demandé : " + buybackName,
                        "Prix demandé : " + std::to_string(buybackPrice) + " or",
                        "Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces",
                        "Raison possible : or insuffisant ou entrée déjà disparue."
                    }
            );
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
            const int buybackCount = ShopTransactionSystem::getBuybackEntryCount(shop.getType());
            int shopChoice = TerminalInterface::askMenuChoiceFromOptions(
                buildShopMainScreen(shop, player),
                buybackCount > 0
                    ? "Veuillez choisir acheter, vendre, discuter, quêtes, rachat, ou 0 pour revenir."
                    : "Veuillez choisir acheter, vendre, discuter, quêtes, ou 0 pour revenir."
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

            if (shopChoice == 5)
            {
                openBuybackMenu(player, shop);
                continue;
            }

            bool buyMenuOpen = true;
            std::size_t pageIndex = 0;
            const std::size_t itemsPerPage = 10;

            while (buyMenuOpen)
            {
                Console::clear();
                const MenuScreen stockScreen = buildShopStockScreen(shop, player, pageIndex, itemsPerPage);

                const std::vector<ShopItem>& items = shop.getItems();
                const std::size_t totalPages = PagedMenu::pageCount(items.size(), itemsPerPage);
                const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
                const std::size_t last = PagedMenu::lastIndexExclusive(items.size(), pageIndex, itemsPerPage);
                const int localCount = static_cast<int>(last - first);

                int itemChoice = TerminalInterface::askMenuChoiceFromOptions(
                    stockScreen,
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
                    showShopResult(
                        "CHOIX INDISPONIBLE",
                        "shop.stock.invalid_choice",
                        {
                            "Cette entrée n'existe pas sur la page actuelle.",
                            "Utilise les choix affichés par le menu pour continuer."
                        }
                    );
                    continue;
                }

                ShopItem& item = shop.getMutableItems()[first + static_cast<std::size_t>(itemChoice - 1)];
                bool itemMenuOpen = true;

                while (itemMenuOpen)
                {
                    Console::clear();
                    bool barterAvailable = hasBlackMarketBarterOffer(shop, item);
                    int actionChoice = TerminalInterface::askMenuChoiceFromOptions(
                        buildShopItemScreen(shop, item, player, true),
                        barterAvailable
                            ? "Choisis une action disponible : retour, achat, inspection ou troc."
                            : "Choisis une action disponible : retour, achat ou inspection."
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
                            showShopResult(
                                "ACHAT IMPOSSIBLE",
                                "shop.buy.blocked",
                                {
                                    "Article : " + item.getName(),
                                    "Or disponible : " + std::to_string(player.getInventory().getGold()) + " pièces",
                                    "Statut : achat refusé pour le moment.",
                                    "Raison possible : or insuffisant, stock épuisé ou article indisponible."
                                }
                            );
                        }
                        else
                        {
                            int quantity = 1;

                            if (maxQuantity > 1)
                            {
                                quantity = MessageScreen::askQuantity(
                                    "QUANTITÉ À ACHETER",
                                    "shop.buy.quantity",
                                    {
                                        "Article : " + item.getName(),
                                        "Maximum achetable : x" + std::to_string(maxQuantity),
                                        "Prix unitaire : " + std::to_string(finalPrice) + " or"
                                    },
                                    1,
                                    maxQuantity,
                                    "Veuillez choisir une quantité valide."
                                );
                            }

                            const int goldBeforePurchase = player.getInventory().getGold();
                            const int stockBeforePurchase = item.getStock();
                            const int expectedTotalPrice = finalPrice * quantity;

                            const bool confirmPurchase = askShopConfirmation(
                                "CONFIRMER L'ACHAT",
                                "shop.buy.confirm",
                                {
                                    "Article : " + item.getName(),
                                    "Quantité : x" + std::to_string(quantity),
                                    "Prix unitaire : " + std::to_string(finalPrice) + " or",
                                    "Total prévu : " + std::to_string(expectedTotalPrice) + " or",
                                    "Or disponible : " + std::to_string(goldBeforePurchase) + " pièces",
                                    stockBeforePurchase >= 0
                                        ? "Stock avant achat : " + std::to_string(stockBeforePurchase)
                                        : "Stock avant achat : non limité"
                                },
                                "Confirmer l'achat",
                                "Annuler l'achat",
                                "shop.buy"
                            );

                            if (!confirmPurchase)
                            {
                                showShopResult(
                                    "ACHAT ANNULÉ",
                                    "shop.buy.cancelled",
                                    {
                                        "Article : " + item.getName(),
                                        "Quantité demandée : x" + std::to_string(quantity),
                                        "Aucun or n'a été dépensé.",
                                        "Le stock du marchand n'a pas changé."
                                    }
                                );
                            }
                            else
                            {
                                ShopTransactionSystem::clearLastTransactionNotes();
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

                                showShopTransactionResult(
                                    boughtCount > 0 ? "ACHAT TERMINÉ" : "ACHAT REFUSÉ",
                                    boughtCount > 0 ? "shop.buy.result.success" : "shop.buy.result.failed",
                                    boughtCount > 0
                                        ? std::vector<std::string>{
                                            "Article : " + item.getName(),
                                            "Quantité obtenue : x" + std::to_string(boughtCount) + " / x" + std::to_string(quantity),
                                            "Or dépensé : " + std::to_string(finalPrice * boughtCount) + " pièces",
                                            "Or avant : " + std::to_string(goldBeforePurchase) + " pièces",
                                            "Or actuel : " + std::to_string(player.getInventory().getGold()) + " pièces",
                                            item.getStock() >= 0
                                                ? "Stock restant : " + std::to_string(item.getStock())
                                                : "Stock restant : non limité"
                                        }
                                        : std::vector<std::string>{
                                            "Article : " + item.getName(),
                                            "Quantité demandée : x" + std::to_string(quantity),
                                            "Aucun exemplaire n'a été ajouté.",
                                            "Raison possible : stock, or ou compatibilité d'inventaire."
                                        }
                                );
                            }
                        }

                        itemMenuOpen = false;
                    }
                    else if (actionChoice == 3)
                    {
                        int maxBarterQuantity = getMaxBarterQuantity(item, player);

                        if (maxBarterQuantity <= 0)
                        {
                            showShopResult(
                                "TROC IMPOSSIBLE",
                                "shop.barter.blocked",
                                {
                                    "Article : " + item.getName(),
                                    "Demande : " + formatBarterRequirements(item),
                                    "Statut : composants insuffisants.",
                                    "Le contact garde l'article sous le comptoir."
                                }
                            );
                        }
                        else
                        {
                            int quantity = 1;

                            if (maxBarterQuantity > 1)
                            {
                                quantity = MessageScreen::askQuantity(
                                    "QUANTITÉ À TROQUER",
                                    "shop.barter.quantity",
                                    {
                                        "Article : " + item.getName(),
                                        "Maximum échangeable : x" + std::to_string(maxBarterQuantity),
                                        "Demande par unité : " + formatBarterRequirements(item)
                                    },
                                    1,
                                    maxBarterQuantity,
                                    "Veuillez choisir une quantité valide."
                                );
                            }

                            const std::string barterRequirements = formatBarterRequirements(item);
                            const int stockBeforeBarter = item.getStock();
                            const bool confirmBarter = askShopConfirmation(
                                "CONFIRMER LE TROC",
                                "shop.barter.confirm",
                                {
                                    "Article : " + item.getName(),
                                    "Quantité : x" + std::to_string(quantity),
                                    quantity > 1
                                        ? "Demande par unité : " + barterRequirements
                                        : "Demande : " + barterRequirements,
                                    "Maximum échangeable maintenant : x" + std::to_string(maxBarterQuantity),
                                    stockBeforeBarter >= 0
                                        ? "Stock avant échange : " + std::to_string(stockBeforeBarter)
                                        : "Stock avant échange : non limité",
                                    "Le marché noir ne promet jamais que l'offre reviendra."
                                },
                                "Confirmer le troc",
                                "Annuler le troc",
                                "shop.barter"
                            );

                            if (!confirmBarter)
                            {
                                showShopResult(
                                    "TROC ANNULÉ",
                                    "shop.barter.cancelled",
                                    {
                                        "Article : " + item.getName(),
                                        "Aucun composant n'a été retiré.",
                                        "Le contact fait semblant de n'avoir jamais proposé l'échange."
                                    }
                                );
                            }
                            else
                            {
                                ShopTransactionSystem::clearLastTransactionNotes();
                                int tradedCount = 0;

                                for (int i = 0; i < quantity; ++i)
                                {
                                    if (getMaxBarterQuantity(item, player) <= 0)
                                    {
                                        break;
                                    }

                                    if (!consumeBarterRequirements(player, item, 1))
                                    {
                                        break;
                                    }

                                    if (!ShopTransactionSystem::buyItem(player, item, 0))
                                    {
                                        refundBarterRequirements(player, item, 1);
                                        break;
                                    }

                                    tradedCount++;
                                }

                                showShopTransactionResult(
                                    tradedCount > 0 ? "TROC TERMINÉ" : "TROC REFUSÉ",
                                    tradedCount > 0 ? "shop.barter.result.success" : "shop.barter.result.failed",
                                    tradedCount > 0
                                        ? std::vector<std::string>{
                                            "Article obtenu : " + item.getName(),
                                            "Quantité obtenue : x" + std::to_string(tradedCount) + " / x" + std::to_string(quantity),
                                            quantity > 1
                                                ? "Coût par unité : " + barterRequirements
                                                : "Coût : " + barterRequirements,
                                            item.getStock() >= 0
                                                ? "Stock restant : " + std::to_string(item.getStock())
                                                : "Stock restant : non limité",
                                            "Le contact range les composants sans demander ton nom."
                                        }
                                        : std::vector<std::string>{
                                            "Article demandé : " + item.getName(),
                                            "Demande : " + barterRequirements,
                                            "Aucun exemplaire n'a été obtenu.",
                                            "Raison possible : composant manquant, stock ou refus d'inventaire."
                                        }
                                );
                            }
                        }

                        itemMenuOpen = false;
                    }
                    else
                    {
                        Console::clear();
                        TerminalInterface::renderMenuScreen(buildShopItemScreen(shop, item, player, false));
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
        MessageScreen::show(
            "ROTATION DES BOUTIQUES",
            "shop.rotation.refreshed",
            {
                "Les marchands changent leurs étals après ton dernier combat.",
                "De nouveaux articles peuvent apparaître, disparaître ou revenir plus cher.",
                "Les rachats des ventes précédentes disparaissent avec cette nouvelle rotation."
            }
        );
        ShopRotationSystem::markShopsRefreshed();
    }

    bool stayInMenu = true;

    while (stayInMenu)
    {
        Console::clear();
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildShopListScreen(shops, &player),
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
