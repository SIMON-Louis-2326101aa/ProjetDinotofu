// EN: SpecialCharacterGroupCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterGroupCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Roadmap catalog for special character relationships and future group encounters.

#include "character/relationship/SpecialCharacterGroupCatalog.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>

// EN: displayRoadmap declares or implements a focused behavior used by this module.
// FR: displayRoadmap déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> SpecialCharacterGroupCatalog::getRoadmapLines()
{
    return {
        "Hazak / Henrique : meilleurs amis.",
        "Mattzelda / Louis / Trexof : meilleurs potes.",
        "Aoi / Kanadé / Sanctus : groupe proche.",
        "Hazak / Hestia : Hazak l'a sauvée et évite les combats à mort avec elle.",
        "Fail / Hazak : contrat de non-agression.",
        "Skuro / personnage spécial compatible : Hazak, Henrique, Aoi, Fail ou Kanadé uniquement.",
        "Skuro / deux personnages spéciaux compatibles : même liste, sans doublon.",
        "Fire Flight / Matt (PRO) : duo de commandement et de pression.",
        "Hestia / Sanctus / Hazak : protection sacrée, aucun massacre accepté.",
        "Fail / Aoi / Kanadé : laboratoire magique, instable mais puissant.",
        "Louis / Fire Flight / Trexof : duel d'improvisation entre amis, projectiles et failles utiles.",
        "Henrique / Mattzelda / Skuro : charge instable, humour, endurance et violence lourde.",
        "Matt (PRO) peut aussi apparaître seul en Combat IA ou via un tirage spécial d'arène.",
        "Groupe aléatoire normal : aventuriers random sans personnage spécial.",
        "Sanctus et Skuro sont liés : même âme, deux réponses possibles face à la violence."
    };
}

// EN: displayRoadmap declares or implements a focused behavior used by this module.
// FR: displayRoadmap déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterGroupCatalog::displayRoadmap()
{
    MessageScreen::show(
        "GROUPES SPÉCIAUX",
        "catalog.special_groups.roadmap",
        getRoadmapLines(),
        false
    );
}
