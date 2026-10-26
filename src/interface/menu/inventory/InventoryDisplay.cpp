// EN: InventoryDisplay.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryDisplay.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventoryDisplay.hpp"

#include "interface/menu/inventory/InventoryUtils.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "economy/Money.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"

#include "item/Inventory.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "item/consumable/Consumable.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace
{

    std::string inventoryWeaponClassCompatibilityTag(const Player& player, const Weapon& weapon)
    {
        if (CombatClassSystem::hasWeaponAffinity(player, weapon.getType(), weapon.getName()))
        {
            return " [bonus de classe]";
        }

        if (CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, weapon.getType(), weapon.getName()) < 0
            || CombatClassSystem::getWeaponHandlingDamagePercent(player, weapon.getType(), weapon.getName()) < 100)
        {
            return " [malus de classe]";
        }

        return "";
    }

    std::string inventoryArmorClassCompatibilityTag(const Player& player, const Armor& armor)
    {
        if (CombatClassSystem::hasArmorAffinity(player, armor.getType(), armor.getName()))
        {
            return " [bonus de classe]";
        }

        if (CombatClassSystem::getArmorHandlingDamageReductionAdjustment(player, armor.getType(), armor.getName(), 24) < 0
            || CombatClassSystem::getArmorHandlingEscapeAdjustment(player, armor.getType(), armor.getName()) < 0)
        {
            return " [malus de classe]";
        }

        return "";
    }

    MenuOptionItemData makeInventoryRouteItemData(
        const std::string& kind,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = "Inventaire - menu principal";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = "Sac du personnage";
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeInventoryWeaponItemData(const Weapon& weapon, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "weapon";
        itemData.section = "Inventaire - arme sélectionnée";
        itemData.actionType = actionType;
        itemData.name = weapon.getName();
        itemData.detail = "Dégâts : +" + std::to_string(weapon.getMinDamageBonus())
            + " à +" + std::to_string(weapon.getMaxDamageBonus())
            + " | Critique : +" + std::to_string(weapon.getCriticalBonus());
        itemData.status = weapon.isBroken() ? "Cassée" : "Utilisable";
        itemData.progress = "Durabilité : " + InventoryUtils::weaponDurabilityText(weapon);
        itemData.price = Money::formatEconomyUnits(weapon.getValue());
        itemData.important = weapon.isBroken();
        return itemData;
    }

    MenuOptionItemData makeInventoryArmorItemData(const Armor& armor, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "armor";
        itemData.section = "Inventaire - armure sélectionnée";
        itemData.actionType = actionType;
        itemData.name = armor.getName();
        itemData.detail = "PV max : +" + std::to_string(armor.getMaxHpBonus())
            + " | Réduction : " + std::to_string(armor.getDamageReduction());
        itemData.status = armor.isBroken() ? "Cassée" : "Utilisable";
        itemData.progress = "Durabilité : " + InventoryUtils::armorDurabilityText(armor);
        itemData.price = Money::formatEconomyUnits(armor.getValue());
        itemData.important = armor.isBroken();
        return itemData;
    }

    MenuOptionItemData makeInventoryConsumableItemData(const Consumable& consumable, const std::string& actionType, const std::string& status = "", int amount = 1)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "consumable";
        itemData.section = "Inventaire - consommable sélectionné";
        itemData.actionType = actionType;
        itemData.name = consumable.getName();
        itemData.quantity = std::to_string(std::max(1, amount));
        itemData.detail = InventoryUtils::consumableTypeToText(consumable.getType());
        itemData.status = status;
        itemData.progress = "Puissance : " + consumable.getPowerDisplayText();
        itemData.price = Money::formatEconomyUnits(consumable.getValue());
        itemData.important = consumable.isHealing();
        return itemData;
    }

    MenuOptionItemData makeInventoryMaterialItemData(const Material& material, const std::string& actionType)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "material";
        itemData.section = "Inventaire - entrée sélectionnée";
        itemData.actionType = actionType;
        itemData.name = material.getName();
        itemData.quantity = std::to_string(material.getQuantity());
        itemData.detail = material.getCategory();
        itemData.status = material.hasSpecialQuality() ? material.getQualityLabel() : "Qualité normale";
        itemData.price = Money::formatEconomyUnits(material.getValue()) + "/unité";
        itemData.important = material.hasSpecialQuality();
        return itemData;
    }
}

// EN: displayMainMenu declares or implements a focused behavior used by this module.
// FR: displayMainMenu déclare ou implémente un comportement précis utilisé par ce module.
MenuScreen InventoryDisplay::buildMainScreen()
{
    MenuScreen screen("INVENTAIRE", "inventory.main");
    screen.addOption(
        1,
        "Bestiaire (objet spécial)",
        "Consulter les créatures, ennemis, alliés, invocations, boss et personnages connus.",
        true,
        "inventory.bestiary",
        makeInventoryRouteItemData("bestiary", "inspect", "Bestiaire", "Registre progressif réservé aux êtres et créatures rencontrés.", "Objet spécial impossible à perdre", true)
    );
    screen.addOption(
        2,
        "Encyclopédie (objet spécial)",
        "Consulter les races, matériaux, objets, effets, zones, classes et craft.",
        true,
        "inventory.encyclopedia",
        makeInventoryRouteItemData("encyclopedia", "inspect", "Encyclopédie", "Registre progressif des connaissances utiles au jeu hors créatures et hors lore pur.", "Objet spécial impossible à perdre", true)
    );
    screen.addOption(
        3,
        "Carnet de découvertes (objet spécial)",
        "Consulter les légendes, divinités, rumeurs, histoires et découvertes narratives.",
        true,
        "inventory.discovery_notebook",
        makeInventoryRouteItemData("discovery_notebook", "inspect", "Carnet de découvertes", "Registre narratif séparé pour éviter que l'Encyclopédie devienne un fourre-tout.", "Objet spécial impossible à perdre", true)
    );
    screen.addOption(
        4,
        "Voir tout (affichage simple)",
        "Résumé court de l'or, des objets et des ressources.",
        true,
        "inventory.full_simple",
        makeInventoryRouteItemData("inventory_summary", "inspect", "Voir tout", "Résumé court : or, armes, armures, consommables et matériaux sans descriptions longues.", "Affichage simple")
    );
    screen.addOption(
        5,
        "Armes",
        "Voir, inspecter, équiper ou réparer les armes.",
        true,
        "inventory.weapons",
        makeInventoryRouteItemData("weapon", "open", "Armes", "Liste des armes avec inspection, équipement, réparation et lien bestiaire.", "Équipement offensif")
    );
    screen.addOption(
        6,
        "Armures",
        "Voir, inspecter, équiper ou réparer les protections.",
        true,
        "inventory.armors",
        makeInventoryRouteItemData("armor", "open", "Armures", "Liste des protections avec inspection, équipement, réparation et lien bestiaire.", "Équipement défensif")
    );
    screen.addOption(
        7,
        "Consommables",
        "Potions et objets utilisables.",
        true,
        "inventory.consumables",
        makeInventoryRouteItemData("consumable", "use", "Consommables", "Potions et objets utilisables, avec actions qui peuvent parfois consommer le tour.", "Utilisable")
    );
    screen.addOption(
        8,
        "Matériaux / plantes / infos",
        "Ressources, notes, fragments et composants.",
        true,
        "inventory.materials",
        makeInventoryRouteItemData("material", "inspect", "Matériaux / plantes / infos", "Ressources, composants, notes, fragments et connaissances liées aux objets.", "Ressources")
    );
    screen.addOption(
        9,
        "Craft / schémas de fabrication",
        "Fabriquer ou améliorer uniquement avec les recettes dont tous les composants sont connus.",
        true,
        "inventory.craft",
        makeInventoryRouteItemData("craft", "create", "Craft / schémas", "Les recettes restent masquées tant que tous leurs composants n'ont pas été découverts.", "Fabrication")
    );
    screen.addOption(
        10,
        "Consulter mes quêtes",
        "Ouvrir le journal de quêtes.",
        true,
        "inventory.quests",
        makeInventoryRouteItemData("quest", "quest", "Journal de quêtes", "Voir les quêtes suivies, terminées ou à rendre.", "Progression")
    );
    screen.addOption(
        11,
        "Préférences de sélection",
        "Choix des munitions et supports magiques : toujours demander par défaut.",
        true,
        "inventory.preferences",
        makeInventoryRouteItemData("inventory_preferences", "configure", "Préférences de sélection", "Règle le comportement des choix répétitifs sans sacrifier le contrôle du joueur.", "Toujours demander par défaut")
    );
    screen.addBackOption("Retour", "inventory.back");
    return screen;
}

void InventoryDisplay::displayMainMenu()
{
    TerminalInterface::renderMenuScreen(buildMainScreen());
}

// EN: displaySimpleFullInventory declares or implements a focused behavior used by this module.
// FR: displaySimpleFullInventory déclare ou implémente un comportement précis utilisé par ce module.
MenuScreen InventoryDisplay::buildSimpleFullInventoryScreen(const Player& player)
{
    const Inventory& inventory = player.getInventory();

    MenuScreen screen("INVENTAIRE - RÉSUMÉ SIMPLE", "inventory.full_simple.summary");
    screen.setContinueInput("Valide pour revenir à l'inventaire.");
    screen.addSubtitle("Affichage volontairement court : noms, quantités et états importants. Les objets épinglés sont placés en premier.");
    screen.addLine("Argent séparé : " + inventory.getWalletLine());
    screen.addLine("Argent total : " + inventory.getWalletTotalLine());
    screen.addLine("Perception sociale de la bourse : " + Money::socialStandingLabel(inventory.getCoinStacks()));
    const CarryLoadSummary load = CarryLoadSystem::evaluate(player);
    screen.addLine("Encombrement : " + std::to_string(load.used) + "/" + std::to_string(load.capacity) + " — " + std::to_string(load.percent) + " % — " + load.label + ".");
    screen.addLine("Sac : palier " + std::to_string(player.getBackpackTier()) + " — base " + std::to_string(player.getBackpackBaseCapacity()) + " unités | Morphologie : " + CarryLoadSystem::morphologyLabel(player) + ".");
    if (load.percent >= 85 && load.percent <= 100)
        screen.addLine("Alerte : le sac dépasse 85 %. Aucun malus avant 100 %, mais il approche de la surcharge.");
    else if (load.percent > 100 && load.percent < 200)
        screen.addLine("Surcharge : fuite et précision de mêlée réduites, exploration/voyage ralentis ; petite protection physique supplémentaire.");
    else if (load.movementBlocked)
        screen.addLine("Charge critique >= 200 % : impossible de voyager ou de repartir en exploration avant d'alléger le sac.");
    screen.addLine("Objets spéciaux : Bestiaire + Encyclopédie + Carnet de découvertes (impossibles à perdre)");
    screen.addLine("Armes : " + std::to_string(inventory.getWeaponCount()));

    std::vector<int> orderedWeaponIndices;
    for (int i = 0; i < inventory.getWeaponCount(); ++i) orderedWeaponIndices.push_back(i);
    std::stable_sort(orderedWeaponIndices.begin(), orderedWeaponIndices.end(), [&](int left, int right) {
        const bool leftPinned = inventory.isPinnedItemKey(Inventory::weaponItemKey(inventory.getWeapon(left)));
        const bool rightPinned = inventory.isPinnedItemKey(Inventory::weaponItemKey(inventory.getWeapon(right)));
        return leftPinned && !rightPinned;
    });

    for (int index : orderedWeaponIndices)
    {
        Weapon weapon = inventory.getWeapon(index);
        const std::string key = Inventory::weaponItemKey(weapon);
        std::string prefix;
        if (inventory.isPinnedItemKey(key)) prefix += "[ÉPINGLÉ] ";
        if (inventory.isLockedItemKey(key)) prefix += "[VERROUILLÉ] ";
        std::string line = "[" + std::to_string(index) + "] " + prefix + weapon.getName() + inventoryWeaponClassCompatibilityTag(player, weapon)
            + " | Durabilité : " + InventoryUtils::weaponDurabilityText(weapon);

        if (weapon.isBroken())
        {
            line += " | Cassée";
        }

        screen.addLine(line);
    }

    screen.addLine("");
    screen.addLine("Armures : " + std::to_string(inventory.getArmorCount()));

    std::vector<int> orderedArmorIndices;
    for (int i = 0; i < inventory.getArmorCount(); ++i) orderedArmorIndices.push_back(i);
    std::stable_sort(orderedArmorIndices.begin(), orderedArmorIndices.end(), [&](int left, int right) {
        const bool leftPinned = inventory.isPinnedItemKey(Inventory::armorItemKey(inventory.getArmor(left)));
        const bool rightPinned = inventory.isPinnedItemKey(Inventory::armorItemKey(inventory.getArmor(right)));
        return leftPinned && !rightPinned;
    });

    for (int index : orderedArmorIndices)
    {
        Armor armor = inventory.getArmor(index);
        const std::string key = Inventory::armorItemKey(armor);
        std::string prefix;
        if (inventory.isPinnedItemKey(key)) prefix += "[ÉPINGLÉ] ";
        if (inventory.isLockedItemKey(key)) prefix += "[VERROUILLÉ] ";
        std::string line = "[" + std::to_string(index) + "] " + prefix + armor.getName() + inventoryArmorClassCompatibilityTag(player, armor)
            + " | Durabilité : " + InventoryUtils::armorDurabilityText(armor);

        if (armor.isBroken())
        {
            line += " | Cassée";
        }

        screen.addLine(line);
    }

    screen.addLine("");
    screen.addLine("Consommables : " + std::to_string(inventory.getConsumableCount()));

    std::vector<ConsumableGroup> groups = InventoryUtils::groupConsumables(player);
    std::stable_sort(groups.begin(), groups.end(), [&](const ConsumableGroup& left, const ConsumableGroup& right) {
        const bool leftPinned = inventory.isPinnedItemKey(Inventory::consumableItemKey(inventory.getConsumable(left.firstIndex)));
        const bool rightPinned = inventory.isPinnedItemKey(Inventory::consumableItemKey(inventory.getConsumable(right.firstIndex)));
        return leftPinned && !rightPinned;
    });

    for (int i = 0; i < static_cast<int>(groups.size()); ++i)
    {
        const ConsumableGroup& group = groups[i];
        const Consumable& sample = inventory.getConsumable(group.firstIndex);
        const std::string key = Inventory::consumableItemKey(sample);
        std::string prefix;
        if (inventory.isPinnedItemKey(key)) prefix += "[ÉPINGLÉ] ";
        if (inventory.isLockedItemKey(key)) prefix += "[VERROUILLÉ] ";
        screen.addLine(
            "[" + std::to_string(i) + "] " + prefix + InventoryUtils::stackLabel(group.name, group.amount)
            + " | " + InventoryUtils::consumableTypeToText(group.type)
            + " | Puissance : " + std::to_string(group.power)
        );
    }

    screen.addLine("");
    screen.addLine("Matériaux / plantes / infos : " + std::to_string(inventory.getMaterialCount()));

    std::vector<int> orderedMaterialIndices;
    for (int i = 0; i < static_cast<int>(inventory.getMaterials().size()); ++i) orderedMaterialIndices.push_back(i);
    std::stable_sort(orderedMaterialIndices.begin(), orderedMaterialIndices.end(), [&](int left, int right) {
        const bool leftPinned = inventory.isPinnedItemKey(Inventory::materialItemKey(inventory.getMaterial(left)));
        const bool rightPinned = inventory.isPinnedItemKey(Inventory::materialItemKey(inventory.getMaterial(right)));
        return leftPinned && !rightPinned;
    });

    for (int index : orderedMaterialIndices)
    {
        Material material = inventory.getMaterial(index);
        const std::string key = Inventory::materialItemKey(material);
        std::string prefix;
        if (inventory.isPinnedItemKey(key)) prefix += "[ÉPINGLÉ] ";
        if (inventory.isFavoriteAmmunitionId(material.getId())) prefix += "[MUNITION FAVORITE] ";
        if (inventory.isLockedItemKey(key)) prefix += "[VERROUILLÉ] ";
        if (inventory.isTrackedMaterialId(material.getId())) prefix += "[SUIVI] ";
        std::string line = "[" + std::to_string(index) + "] " + prefix + material.getName()
            + " x" + std::to_string(material.getQuantity())
            + " | " + material.getCategory();

        if (material.hasSpecialQuality())
        {
            line += " | " + material.getQualityLabel();
        }

        screen.addLine(line);
    }

    screen.addFooterLine("Pour agir sur un objet, reviens au menu et choisis sa catégorie.");
    return screen;
}

// EN: displaySimpleFullInventory declares or implements a focused behavior used by this module.
// FR: displaySimpleFullInventory déclare ou implémente un comportement précis utilisé par ce module.
void InventoryDisplay::displaySimpleFullInventory(const Player& player)
{
    TerminalInterface::renderMenuScreen(buildSimpleFullInventoryScreen(player), false);
    Console::waitForEnter();
    Console::clear();
}

MenuScreen InventoryDisplay::buildSelectedWeaponScreen(const Weapon& weapon, bool pinned, bool locked)
{
    MenuScreen screen("ARME SÉLECTIONNÉE", "inventory.weapon.selected");
    screen.addLine("Arme : " + weapon.getName());
    screen.addBackOption("Retour", "inventory.weapon.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'arme.", true, "inventory.weapon.inspect", makeInventoryWeaponItemData(weapon, "inspect"));
    screen.addOption(2, "Équiper", "Équiper cette arme maintenant.", true, "inventory.weapon.equip", makeInventoryWeaponItemData(weapon, "equip"));
    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.weapon.bestiary", makeInventoryWeaponItemData(weapon, "inspect"));
    screen.addOption(4, "Réparer", "Utiliser un kit ou des matériaux compatibles.", true, "inventory.weapon.repair", makeInventoryWeaponItemData(weapon, "repair"));
    screen.addOption(5, pinned ? "Désépingler" : "Épingler", pinned ? "L'arme reprendra sa place normale dans la liste." : "L'arme sera affichée en haut de l'inventaire.", true, "inventory.weapon.pin");
    screen.addOption(6, locked ? "Déverrouiller" : "Verrouiller", locked ? "Retirer la protection supplémentaire avant vente/utilisation." : "Demander une confirmation supplémentaire avant toute vente ou utilisation sensible.", true, "inventory.weapon.lock");
    return screen;
}

void InventoryDisplay::displaySelectedWeapon(const Weapon& weapon)
{
    TerminalInterface::renderMenuScreen(buildSelectedWeaponScreen(weapon));
}

MenuScreen InventoryDisplay::buildSelectedArmorScreen(const Armor& armor, bool pinned, bool locked)
{
    MenuScreen screen("ARMURE SÉLECTIONNÉE", "inventory.armor.selected");
    screen.addLine("Armure : " + armor.getName());
    screen.addBackOption("Retour", "inventory.armor.back");
    screen.addOption(1, "Inspecter", "Voir la description et les détails de l'armure.", true, "inventory.armor.inspect", makeInventoryArmorItemData(armor, "inspect"));
    screen.addOption(2, "Équiper", "Équiper cette protection maintenant.", true, "inventory.armor.equip", makeInventoryArmorItemData(armor, "equip"));
    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.armor.bestiary", makeInventoryArmorItemData(armor, "inspect"));
    screen.addOption(4, "Réparer", "Utiliser un kit ou des matériaux compatibles.", true, "inventory.armor.repair", makeInventoryArmorItemData(armor, "repair"));
    screen.addOption(5, pinned ? "Désépingler" : "Épingler", pinned ? "L'armure reprendra sa place normale dans la liste." : "L'armure sera affichée en haut de l'inventaire.", true, "inventory.armor.pin");
    screen.addOption(6, locked ? "Déverrouiller" : "Verrouiller", locked ? "Retirer la protection supplémentaire avant vente/utilisation." : "Demander une confirmation supplémentaire avant toute vente ou utilisation sensible.", true, "inventory.armor.lock");
    return screen;
}

void InventoryDisplay::displaySelectedArmor(const Armor& armor)
{
    TerminalInterface::renderMenuScreen(buildSelectedArmorScreen(armor));
}

MenuScreen InventoryDisplay::buildSelectedConsumableScreen(const Consumable& consumable, int amount, bool pinned, bool locked)
{
    amount = std::max(1, amount);

    MenuScreen screen("CONSOMMABLE SÉLECTIONNÉ", "inventory.consumable.selected");
    screen.addLine("Consommable : " + InventoryUtils::stackLabel(consumable.getName(), amount));
    screen.addLine("Quantité dans la pile : " + std::to_string(amount));
    screen.addLine("Type : " + InventoryUtils::consumableTypeToText(consumable.getType()));
    screen.addLine("Puissance : " + consumable.getPowerDisplayText());
    screen.addBackOption("Retour", "inventory.consumable.back");
    screen.addOption(1, "Inspecter", "Lire la description complète.", true, "inventory.consumable.inspect", makeInventoryConsumableItemData(consumable, "inspect", "", amount));

    if (consumable.getType() == ConsumableType::Healing)
    {
        screen.addOption(2, "Utiliser", "Boire une potion de cette pile hors combat.", true, "inventory.consumable.use", makeInventoryConsumableItemData(consumable, "use", "Utilisable hors combat", amount));
    }
    else
    {
        screen.addOption(2, "Utiliser depuis le menu Potions en combat", "Ce type demande une situation de combat.", false, "inventory.consumable.use_locked", makeInventoryConsumableItemData(consumable, "use", "Combat requis", amount));
    }

    screen.addOption(3, "Voir dans le bestiaire", "Consulter les informations connues liées à cette entrée.", true, "inventory.consumable.bestiary", makeInventoryConsumableItemData(consumable, "inspect", "", amount));
    screen.addOption(4, pinned ? "Désépingler" : "Épingler", pinned ? "Ce consommable reprendra sa place normale." : "Ce consommable sera affiché en haut de l'inventaire.", true, "inventory.consumable.pin");
    screen.addOption(5, locked ? "Déverrouiller" : "Verrouiller", locked ? "Retirer la protection supplémentaire." : "Demander une confirmation supplémentaire avant utilisation ou vente.", true, "inventory.consumable.lock");
    return screen;
}

void InventoryDisplay::displaySelectedConsumable(const Consumable& consumable)
{
    TerminalInterface::renderMenuScreen(buildSelectedConsumableScreen(consumable));
}

MenuScreen InventoryDisplay::buildSelectedMaterialScreen(const Material& material, bool pinned, bool locked, bool tracked, bool favoriteAmmunition)
{
    MenuScreen screen("ENTRÉE SÉLECTIONNÉE", "inventory.material.selected");
    screen.addLine("Nom : " + material.getName());
    screen.addLine("Catégorie : " + material.getCategory());
    screen.addLine("Quantité : " + std::to_string(material.getQuantity()));
    screen.addBackOption("Retour", "inventory.material.back");
    screen.addOption(1, "Inspecter", "Lire la description complète.", true, "inventory.material.inspect", makeInventoryMaterialItemData(material, "inspect"));
    screen.addOption(2, "Voir dans le bestiaire", "Consulter la connaissance liée à cette ressource.", true, "inventory.material.bestiary", makeInventoryMaterialItemData(material, "inspect"));
    screen.addOption(3, "Voir les usages connus", "Recettes, réparations ou pistes déjà découvertes.", true, "inventory.material.uses", makeInventoryMaterialItemData(material, "inspect"));
    screen.addOption(4, "Lire / utiliser", "Exploiter cette entrée si elle contient une note ou un usage direct.", true, "inventory.material.use", makeInventoryMaterialItemData(material, "use"));
    screen.addOption(5, pinned ? "Désépingler" : "Épingler", pinned ? "Ce matériau reprendra sa place normale." : "Ce matériau sera affiché tout en haut de la liste.", true, "inventory.material.pin");
    screen.addOption(6, locked ? "Déverrouiller" : "Verrouiller", locked ? "Retirer la protection supplémentaire." : "Demander une confirmation supplémentaire avant vente ou consommation.", true, "inventory.material.lock");
    screen.addOption(7, tracked ? "Arrêter le suivi" : "Suivre ce matériau", tracked ? "Les prochaines obtentions ne seront plus mises en évidence." : "Chaque prochaine obtention sera affichée immédiatement en évidence.", true, "inventory.material.track");
    if (Inventory::isAmmunitionMaterialId(material.getId()))
    {
        screen.addOption(8, favoriteAmmunition ? "Retirer comme munition favorite" : "Définir comme munition favorite",
            favoriteAmmunition
                ? "Cette munition ne sera plus choisie automatiquement par le mode Favori."
                : "Le mode Munitions — Favori pourra choisir automatiquement cette munition quand elle est compatible.",
            true, "inventory.material.ammunition_favorite");
    }
    return screen;
}

void InventoryDisplay::displaySelectedMaterial(const Material& material)
{
    TerminalInterface::renderMenuScreen(buildSelectedMaterialScreen(material));
}

MenuScreen InventoryDisplay::buildUnavailableMaterialsScreen()
{
    MenuScreen screen("MATÉRIAUX", "inventory.materials.empty");
    screen.setDisplayOnlyInput("Section vide affichée sans saisie directe.");
    screen.addLine("Aucun matériau exploitable dans cette section.");
    screen.addLine("Quand tu en possèdes, ils peuvent servir à réparer, améliorer ou fabriquer de l'équipement.");
    return screen;
}

void InventoryDisplay::displayUnavailableMaterials()
{
    TerminalInterface::renderMenuScreen(buildUnavailableMaterialsScreen(), false);
}
