// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Creates early stackable materials, plants, books and information entries.
// Français : Crée les premiers matériaux, plantes, livres et renseignements empilables.

#ifndef INCLUDE_ITEM_MATERIAL_MATERIALCATALOG_HPP
#define INCLUDE_ITEM_MATERIAL_MATERIALCATALOG_HPP

#include "item/material/Material.hpp"

#include <string>

class MaterialCatalog
{
public:
    static Material createById(const std::string& id, int quantity = 1);
    static Material createGoblinEar(int quantity = 1);
    static Material createWolfFang(int quantity = 1);
    static Material createRustedMetalFragment(int quantity = 1);
    static Material createWornLeatherPiece(int quantity = 1);
    static Material createMountainBlueFlower(int quantity = 1);
    static Material createBitterHealingLeaf(int quantity = 1);
    static Material createCommonGoblinNotes(int quantity = 1);
    static Material createCommonWolfNotes(int quantity = 1);
    static Material createBasicPlantManual(int quantity = 1);
    static Material createBasicMagicManual(int quantity = 1);
};

#endif
