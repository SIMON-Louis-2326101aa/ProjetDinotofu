// EN: CombatPotionDisplay.cpp centralizes potion screens for terminal and future GUI rendering.
// FR: CombatPotionDisplay.cpp centralise les écrans de potions pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionDisplay.hpp"

#include "core/Console.hpp"
#include "economy/Money.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>
#include <algorithm>
#include <string>

namespace
{
    MenuOptionItemData makePotionItemData(const Consumable& potion, const std::string& actionType, const std::string& section = "Potions", int amount = 1)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "consumable";
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = potion.getName();
        itemData.quantity = std::to_string(std::max(1, amount));
        itemData.detail = CombatPotionUtils::typeToText(potion.getType());
        itemData.progress = "Puissance : " + potion.getPowerDisplayText();
        itemData.price = std::to_string(potion.getValue()) + " or";
        itemData.important = potion.isHealing();
        return itemData;
    }
}

MenuScreen CombatPotionDisplay::buildMainScreen()
{
    MenuScreen screen("POTIONS", "potions.main");
    screen.addBackOption("Retour", "potions.back");
    screen.addOption(1, "Voir les potions", "Liste toutes les potions de l'inventaire.", true, "potions.list");
    screen.addOption(2, "Utiliser une potion curative", "Soin et récupération.", true, "potions.healing");
    screen.addOption(3, "Utiliser une potion défensive", "Protection, réduction ou résistance.", true, "potions.defensive");
    screen.addOption(4, "Utiliser une potion offensive", "Dégâts ou effets lancés sur l'ennemi.", true, "potions.offensive");
    screen.addOption(5, "Utiliser une potion de buff", "Renforcer temporairement le personnage.", true, "potions.buff");
    screen.addOption(6, "Utiliser une potion de debuff", "Affaiblir ou gêner la cible.", true, "potions.debuff");
    screen.addOption(7, "Utiliser une potion spéciale", "Effets rares, instables ou situationnels.", true, "potions.special");
    return screen;
}

MenuScreen CombatPotionDisplay::buildMainScreen(const Player& player)
{
    MenuScreen screen("POTIONS", "potions.main");

    const bool hasAnyPotion = !player.getInventory().getConsumables().empty();
    const bool hasHealing = !CombatPotionUtils::getPotionIndices(player, ConsumableType::Healing).empty();
    const bool hasBuff = !CombatPotionUtils::getPotionIndices(player, ConsumableType::Buff).empty();
    const bool hasDamage = !CombatPotionUtils::getPotionIndices(player, ConsumableType::Damage).empty();
    const bool hasDebuff = !CombatPotionUtils::getPotionIndices(player, ConsumableType::Debuff).empty();
    const bool hasSpecial = !CombatPotionUtils::getPotionIndices(player, ConsumableType::Special).empty();

    screen.addSubtitle(
        "Potions disponibles : "
        + std::to_string(CombatPotionUtils::groupPotions(player).size())
        + " piles / "
        + std::to_string(player.getInventory().getConsumables().size())
        + " objets"
    );
    screen.addBackOption("Retour", "potions.back");
    screen.addOption(1, "Voir les potions", hasAnyPotion ? "Liste toutes les potions de l'inventaire." : "Aucune potion à afficher.", hasAnyPotion, "potions.list");
    screen.addOption(2, "Utiliser une potion curative", hasHealing ? "Soin et récupération." : "Aucune potion curative disponible.", hasHealing, "potions.healing");
    screen.addOption(3, "Utiliser une potion défensive", hasBuff ? "Protection, réduction ou résistance." : "Aucune potion défensive disponible.", hasBuff, "potions.defensive");
    screen.addOption(4, "Utiliser une potion offensive", hasDamage ? "Dégâts ou effets lancés sur l'ennemi." : "Aucune potion offensive disponible.", hasDamage, "potions.offensive");
    screen.addOption(5, "Utiliser une potion de buff", hasBuff ? "Renforcer temporairement le personnage." : "Aucune potion de buff disponible.", hasBuff, "potions.buff");
    screen.addOption(6, "Utiliser une potion de debuff", hasDebuff ? "Affaiblir ou gêner la cible." : "Aucune potion de debuff disponible.", hasDebuff, "potions.debuff");
    screen.addOption(7, "Utiliser une potion spéciale", hasSpecial ? "Effets rares, instables ou situationnels." : "Aucune potion spéciale disponible.", hasSpecial, "potions.special");
    return screen;
}

MenuScreen CombatPotionDisplay::buildQuickHealingScreen(
    const Player& player,
    const std::vector<int>& indices,
    std::size_t pageIndex,
    std::size_t itemsPerPage
)
{
    MenuScreen screen("POTION DE SOIN RAPIDE", "potions.quick_heal");
    std::vector<PotionStack> stacks = CombatPotionUtils::groupPotionIndices(player, indices);
    const std::size_t totalPages = PagedMenu::pageCount(stacks.size(), itemsPerPage);
    const std::size_t safePageIndex = std::min(pageIndex, totalPages - 1);
    const std::size_t first = PagedMenu::firstIndex(safePageIndex, itemsPerPage);
    const std::size_t last = PagedMenu::lastIndexExclusive(stacks.size(), safePageIndex, itemsPerPage);

    screen.addSubtitle("Soins affichés : " + PagedMenu::rangeText(first, last, stacks.size()));
    screen.setPagination(safePageIndex, totalPages);

    for (std::size_t i = first; i < last; ++i)
    {
        const PotionStack& stack = stacks[i];
        Consumable potion = player.getInventory().getConsumable(stack.firstIndex);

        screen.addOption(
            static_cast<int>(i - first + 1),
            CombatPotionUtils::stackLabel(potion.getName(), stack.amount),
            "Soin : " + potion.getPowerDisplayText() + " | Quantité : " + std::to_string(stack.amount),
            true,
            "potions.quick_heal.use",
            makePotionItemData(potion, "use", "Soin rapide", stack.amount)
        );
    }

    PagedMenu::addNavigationOptions(
        screen,
        safePageIndex,
        totalPages,
        "potions.quick_heal.back",
        "potions.quick_heal.previous",
        "potions.quick_heal.next",
        "Revenir au menu des potions.",
        "Voir les potions de soin précédentes.",
        "Voir les potions de soin suivantes."
    );
    return screen;
}

MenuScreen CombatPotionDisplay::buildSelectedHealingPotionScreen(const Consumable& potion, int amount)
{
    amount = std::max(1, amount);

    MenuScreen screen("POTION SÉLECTIONNÉE", "potions.healing.selected");
    screen.addLine("Potion : " + CombatPotionUtils::stackLabel(potion.getName(), amount));
    screen.addLine("Quantité dans la pile : " + std::to_string(amount));
    screen.addLine("Description : " + potion.getDescription());
    screen.addLine("Soin : " + potion.getPowerDisplayText());
    screen.addBackOption("Retour", "potions.healing.back");
    screen.addOption(1, "Inspecter", "Lire les détails de la potion.", true, "potions.healing.inspect", makePotionItemData(potion, "inspect", "Potion sélectionnée", amount));
    screen.addOption(2, "Utiliser", "Consommer une potion de cette pile maintenant.", true, "potions.healing.use", makePotionItemData(potion, "use", "Potion sélectionnée", amount));
    return screen;
}

MenuScreen CombatPotionDisplay::buildSelectedPotionScreen(const Consumable& potion, int amount)
{
    amount = std::max(1, amount);

    MenuScreen screen("POTION SÉLECTIONNÉE", "potions.selected");
    screen.addLine("Potion : " + CombatPotionUtils::stackLabel(potion.getName(), amount));
    screen.addLine("Quantité dans la pile : " + std::to_string(amount));
    screen.addLine("Type : " + CombatPotionUtils::typeToText(potion.getType()));
    screen.addLine("Puissance : " + potion.getPowerDisplayText());
    screen.addBackOption("Retour", "potions.selected.back");
    screen.addOption(1, "Inspecter", "Lire les détails de la potion.", true, "potions.selected.inspect", makePotionItemData(potion, "inspect", "Potion sélectionnée", amount));
    screen.addOption(2, "Utiliser", "Consommer ou lancer une potion de cette pile selon son type.", true, "potions.selected.use", makePotionItemData(potion, "use", "Potion sélectionnée", amount));
    return screen;
}

MenuScreen CombatPotionDisplay::buildFilteredPotionsScreen(
    const Player& player,
    const std::vector<int>& indices,
    std::size_t pageIndex,
    std::size_t itemsPerPage
)
{
    MenuScreen screen("LISTE DES POTIONS", "potions.filtered");
    std::vector<PotionStack> stacks = CombatPotionUtils::groupPotionIndices(player, indices);
    const std::size_t totalPages = PagedMenu::pageCount(stacks.size(), itemsPerPage);
    const std::size_t safePageIndex = std::min(pageIndex, totalPages - 1);
    const std::size_t first = PagedMenu::firstIndex(safePageIndex, itemsPerPage);
    const std::size_t last = PagedMenu::lastIndexExclusive(stacks.size(), safePageIndex, itemsPerPage);

    screen.addSubtitle("Potions affichées : " + PagedMenu::rangeText(first, last, stacks.size()));
    screen.setPagination(safePageIndex, totalPages);

    for (std::size_t i = first; i < last; ++i)
    {
        const PotionStack& stack = stacks[i];
        Consumable potion = player.getInventory().getConsumable(stack.firstIndex);
        screen.addOption(
            static_cast<int>(i - first + 1),
            CombatPotionUtils::stackLabel(potion.getName(), stack.amount),
            "Puissance : " + potion.getPowerDisplayText() + " | Quantité : " + std::to_string(stack.amount),
            true,
            "potions.filtered.select",
            makePotionItemData(potion, "select", "Potions filtrées", stack.amount)
        );
    }

    screen.addFooterLine("Choisis une potion visible sur cette page.");
    screen.addFooterLine("0 revient au menu des potions, 98/99 changent de page si disponible.");
    PagedMenu::addNavigationOptions(
        screen,
        safePageIndex,
        totalPages,
        "potions.filtered.back",
        "potions.filtered.previous",
        "potions.filtered.next",
        "Revenir au menu des potions.",
        "Voir les potions précédentes.",
        "Voir les potions suivantes."
    );
    return screen;
}

MenuScreen CombatPotionDisplay::buildPotionOverviewScreen(
    const Player& player,
    std::size_t pageIndex,
    std::size_t itemsPerPage
)
{
    MenuScreen screen("POTIONS DISPONIBLES", "potions.overview");
    const std::vector<Consumable>& consumables = player.getInventory().getConsumables();
    std::vector<PotionStack> stacks = CombatPotionUtils::groupPotions(player);

    if (consumables.empty())
    {
        screen.addLine("Aucune potion dans l'inventaire.");
        screen.setContinueInput("Valide pour revenir au combat.");
        return screen;
    }

    const std::size_t totalPages = PagedMenu::pageCount(stacks.size(), itemsPerPage);
    const std::size_t safePageIndex = std::min(pageIndex, totalPages - 1);
    const std::size_t first = PagedMenu::firstIndex(safePageIndex, itemsPerPage);
    const std::size_t last = PagedMenu::lastIndexExclusive(stacks.size(), safePageIndex, itemsPerPage);

    screen.addSubtitle("Piles affichées : " + PagedMenu::rangeText(first, last, stacks.size()));
    screen.setPagination(safePageIndex, totalPages);

    for (std::size_t i = first; i < last; ++i)
    {
        const PotionStack& stack = stacks[i];
        Consumable potion = player.getInventory().getConsumable(stack.firstIndex);
        screen.addOption(
            static_cast<int>(i - first + 1),
            CombatPotionUtils::stackLabel(potion.getName(), stack.amount),
            CombatPotionUtils::typeToText(potion.getType()) + " | Puissance : " + potion.getPowerDisplayText() + " | Quantité : " + std::to_string(stack.amount),
            false,
            "potions.overview.item",
            makePotionItemData(potion, "overview", "Potions disponibles", stack.amount)
        );
    }

    PagedMenu::addNavigationOptions(
        screen,
        safePageIndex,
        totalPages,
        "potions.overview.back",
        "potions.overview.previous",
        "potions.overview.next",
        "Revenir au menu des potions.",
        "Voir les potions précédentes.",
        "Voir les potions suivantes."
    );
    screen.addFooterLine("Cette vue est consultative : les potions listées ne consomment pas d'action.");
    return screen;
}

void CombatPotionDisplay::displayMainMenu()
{
    TerminalInterface::renderMenuScreen(buildMainScreen());
}

void CombatPotionDisplay::displayQuickHealing(
    const Player& player,
    const std::vector<int>& indices
)
{
    TerminalInterface::renderMenuScreen(buildQuickHealingScreen(player, indices));
}

void CombatPotionDisplay::displaySelectedHealingPotion(const Consumable& potion)
{
    TerminalInterface::renderMenuScreen(buildSelectedHealingPotionScreen(potion));
}

void CombatPotionDisplay::displaySelectedPotion(const Consumable& potion)
{
    TerminalInterface::renderMenuScreen(buildSelectedPotionScreen(potion));
}

void CombatPotionDisplay::displayPotions(const Player& player)
{
    if (player.getInventory().getConsumables().empty())
    {
        TerminalInterface::renderMenuScreen(buildPotionOverviewScreen(player), false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    const std::size_t itemsPerPage = 10;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::size_t totalPages = PagedMenu::pageCount(
            CombatPotionUtils::groupPotions(player).size(),
            itemsPerPage
        );

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildPotionOverviewScreen(player, pageIndex, itemsPerPage),
            "Choix invalide. Utilise 0, 98 ou 99."
        );

        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }
    }
}

void CombatPotionDisplay::showPotionDetails(const Consumable& potion)
{
    MessageScreen::show(
        "DÉTAILS POTION",
        "potions.details",
        {
            "Nom : " + potion.getName(),
            "Description : " + potion.getDescription(),
            "Type : " + CombatPotionUtils::typeToText(potion.getType()),
            "Puissance : " + potion.getPowerDisplayText(),
            "Valeur estimée : " + Money::formatGoldWithRaw(potion.getValue())
        }
    );
}

void CombatPotionDisplay::showEmptyCategory(const std::string& typeName)
{
    MessageScreen::show(
        "POTION INDISPONIBLE",
        "potions.category.empty",
        {
            "Aucune potion de type " + typeName + " n'est disponible.",
            "L'action ne consomme pas le tour."
        }
    );
}

void CombatPotionDisplay::showPotionMissing()
{
    MessageScreen::show(
        "POTION INTROUVABLE",
        "potions.missing",
        {
            "Cette potion n'existe plus dans l'inventaire.",
            "L'action est annulée proprement."
        }
    );
}

void CombatPotionDisplay::displayFilteredPotions(
    const Player& player,
    const std::vector<int>& indices
)
{
    TerminalInterface::renderMenuScreen(buildFilteredPotionsScreen(player, indices));
}
