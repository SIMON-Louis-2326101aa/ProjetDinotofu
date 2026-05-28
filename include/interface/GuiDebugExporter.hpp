// EN: GuiDebugExporter.hpp optionally writes GUI-ready snapshots to JSON files for renderer/debug work.
// FR: GuiDebugExporter.hpp écrit optionnellement des instantanés prêts pour l'IG dans des fichiers JSON.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_GUIDEBUGEXPORTER_HPP
#define INCLUDE_INTERFACE_GUIDEBUGEXPORTER_HPP

#include "interface/model/MenuScreen.hpp"
#include "interface/model/CombatStateSnapshot.hpp"

#include <string>

class GuiDebugExporter
{
public:
    // EN: Enabled only when DINOTOFU_GUI_DEBUG_DIR points to an output directory.
    // FR: Actif uniquement si DINOTOFU_GUI_DEBUG_DIR pointe vers un dossier de sortie.
    static bool isEnabled();

    static void exportMenu(const MenuScreen& screen, const std::string& fileStem = "latest_menu");
    static void exportCombat(const GuiCombatStateSnapshot& snapshot, const std::string& fileStem = "latest_combat");

private:
    static std::string outputDirectory();
    static std::string guiInputFilePath();
    static std::string guiInputQueueDirectoryPath();
    static void updateGuiState(const std::string& kind, const std::string& fileStem, const MenuScreen* screen, const GuiCombatStateSnapshot* combat);
    static std::string sanitizeFileStem(const std::string& fileStem);
    static void writeJsonFile(const std::string& fileStem, const std::string& jsonContent);
};

#endif
