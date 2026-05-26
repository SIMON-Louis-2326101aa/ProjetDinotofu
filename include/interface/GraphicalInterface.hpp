// EN: GraphicalInterface.hpp exposes GUI-preparation helpers without replacing the terminal UI yet.
// FR: GraphicalInterface.hpp expose des helpers de préparation IG sans remplacer l'interface terminal pour l'instant.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_GRAPHICALINTERFACE_HPP
#define INCLUDE_INTERFACE_GRAPHICALINTERFACE_HPP

#include "interface/model/MenuScreen.hpp"
#include "interface/model/GuiMenuSnapshot.hpp"
#include "interface/model/CombatStateSnapshot.hpp"

#include <string>
#include <vector>

class GraphicalInterface
{
public:
    static GuiMenuSnapshot snapshotScreen(const MenuScreen& screen);
    static GuiCombatStateSnapshot snapshotCombatState(const GuiCombatStateSnapshot& snapshot);
    static std::vector<std::string> collectActionIds(const MenuScreen& screen);
    static std::string describeScreenContract(const MenuScreen& screen);
    static std::string describeCombatContract(const GuiCombatStateSnapshot& snapshot);
};

#endif
