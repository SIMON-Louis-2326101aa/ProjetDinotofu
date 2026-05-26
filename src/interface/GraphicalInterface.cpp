// EN: GraphicalInterface.cpp keeps future GUI data extraction close to the current terminal models.
// FR: GraphicalInterface.cpp rapproche l'extraction de données IG des modèles terminal actuels.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/GraphicalInterface.hpp"

#include <sstream>


GuiMenuSnapshot GraphicalInterface::snapshotScreen(const MenuScreen& screen)
{
    GuiMenuSnapshot snapshot;
    snapshot.screenId = screen.getScreenId();
    snapshot.title = screen.getTitle();
    snapshot.subtitles = screen.getSubtitles();
    snapshot.lines = screen.getLines();
    snapshot.footerLines = screen.getFooterLines();

    for (const MenuOption& option : screen.getOptions())
    {
        GuiMenuActionSnapshot action;
        action.number = option.getNumber();
        action.label = option.getLabel();
        action.hint = option.getHint();
        action.enabled = option.isEnabled();
        action.actionId = option.getActionId();
        snapshot.actions.push_back(action);
    }

    return snapshot;
}

GuiCombatStateSnapshot GraphicalInterface::snapshotCombatState(const GuiCombatStateSnapshot& snapshot)
{
    return snapshot;
}

std::vector<std::string> GraphicalInterface::collectActionIds(const MenuScreen& screen)
{
    std::vector<std::string> actionIds;

    for (const MenuOption& option : screen.getOptions())
    {
        if (!option.getActionId().empty())
        {
            actionIds.push_back(option.getActionId());
        }
    }

    return actionIds;
}

std::string GraphicalInterface::describeScreenContract(const MenuScreen& screen)
{
    std::ostringstream output;
    output << "screen=" << screen.getScreenId()
           << "; title=" << screen.getTitle()
           << "; options=" << screen.getOptions().size();

    return output.str();
}


std::string GraphicalInterface::describeCombatContract(const GuiCombatStateSnapshot& snapshot)
{
    std::ostringstream output;
    output << "combat=" << snapshot.screenId
           << "; title=" << snapshot.title
           << "; player_units=" << snapshot.playerUnits.size()
           << "; enemy_units=" << snapshot.enemyUnits.size()
           << "; neutral_units=" << snapshot.neutralUnits.size();

    if (!snapshot.phase.empty())
    {
        output << "; phase=" << snapshot.phase;
    }

    return output.str();
}
