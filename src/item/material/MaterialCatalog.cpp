// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements early stackable materials, plants, books and information entries.
// Français : Implémente les premiers matériaux, plantes, livres et renseignements empilables.

#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalizeId(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return value;
    }
}

Material MaterialCatalog::createById(const std::string& id, int quantity)
{
    std::string normalized = normalizeId(id);

    if (normalized == "goblin_ear") return createGoblinEar(quantity);
    if (normalized == "wolf_fang") return createWolfFang(quantity);
    if (normalized == "rusted_metal_fragment") return createRustedMetalFragment(quantity);
    if (normalized == "worn_leather_piece") return createWornLeatherPiece(quantity);
    if (normalized == "mountain_blue_flower") return createMountainBlueFlower(quantity);
    if (normalized == "bitter_healing_leaf") return createBitterHealingLeaf(quantity);
    if (normalized == "common_goblin_notes") return createCommonGoblinNotes(quantity);
    if (normalized == "common_wolf_notes") return createCommonWolfNotes(quantity);
    if (normalized == "basic_plant_manual") return createBasicPlantManual(quantity);
    if (normalized == "basic_magic_manual") return createBasicMagicManual(quantity);

    return Material(
        id,
        "Renseignement inconnu",
        "Une entrée préparée, mais pas encore détaillée dans le catalogue.",
        "Inconnu",
        0,
        quantity
    );
}

Material MaterialCatalog::createGoblinEar(int quantity)
{
    return Material("goblin_ear", "Oreille de gobelin", "Matériau commun récupéré sur certains gobelins.", "Matériau de monstre", 4, quantity);
}

Material MaterialCatalog::createWolfFang(int quantity)
{
    return Material("wolf_fang", "Croc de loup", "Matériau utile pour de petites améliorations d'armes.", "Matériau de monstre", 6, quantity);
}

Material MaterialCatalog::createRustedMetalFragment(int quantity)
{
    return Material("rusted_metal_fragment", "Fragment de métal rouillé", "Reste d'équipement brisé, encore utile pour des réparations basiques.", "Matériau", 3, quantity);
}

Material MaterialCatalog::createWornLeatherPiece(int quantity)
{
    return Material("worn_leather_piece", "Morceau de cuir abîmé", "Matière simple pour armures légères.", "Matériau", 5, quantity);
}

Material MaterialCatalog::createMountainBlueFlower(int quantity)
{
    return Material("mountain_blue_flower", "Fleur bleue de montagne", "Une fleur rare et calme, souvent liée aux remèdes et aux secrets anciens.", "Plante", 10, quantity);
}

Material MaterialCatalog::createBitterHealingLeaf(int quantity)
{
    return Material("bitter_healing_leaf", "Feuille amère de soin", "Plante commune utilisée dans les potions simples.", "Plante", 5, quantity);
}

Material MaterialCatalog::createCommonGoblinNotes(int quantity)
{
    return Material("common_goblin_notes", "Notes communes sur les gobelins", "Renseignement acheté : les gobelins communs préfèrent survivre plutôt que mourir héroïquement.", "Renseignement", 0, quantity);
}

Material MaterialCatalog::createCommonWolfNotes(int quantity)
{
    return Material("common_wolf_notes", "Notes communes sur les loups", "Renseignement acheté : les loups testent souvent les proies blessées ou isolées.", "Renseignement", 0, quantity);
}

Material MaterialCatalog::createBasicPlantManual(int quantity)
{
    return Material("basic_plant_manual", "Petit guide des plantes communes", "Livre de terrain préparant le futur journal des plantes.", "Livre", 10, quantity);
}

Material MaterialCatalog::createBasicMagicManual(int quantity)
{
    return Material("basic_magic_manual", "Manuel de magie basique", "Livre simple qui servira plus tard aux premières notions de magie.", "Livre", 10, quantity);
}
