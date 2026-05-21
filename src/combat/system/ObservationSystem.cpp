// EN: ObservationSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ObservationSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/ObservationSystem.hpp"

#include <iostream>

// EN: displayFailedDecryptionAttempt declares or implements a focused behavior used by this module.
// FR: displayFailedDecryptionAttempt déclare ou implémente un comportement précis utilisé par ce module.
void ObservationSystem::displayFailedDecryptionAttempt(const Entity& target)
{
    std::cout << "Tentative de décryptage échouée." << std::endl;
    std::cout << "Les données de " << target.getName()
              << " restent brouillées par une force beaucoup trop élevée." << std::endl;
    std::cout << "L'interface refuse encore d'afficher ses statistiques." << std::endl;
    std::cout << std::endl;
}

// EN: displayTerminalStats declares or implements a focused behavior used by this module.
// FR: displayTerminalStats déclare ou implémente un comportement précis utilisé par ce module.
void ObservationSystem::displayTerminalStats(const Entity& target)
{
    if (target.areStatsVisible())
    {
        target.displayStats();
        return;
    }

    displayFailedDecryptionAttempt(target);
}
