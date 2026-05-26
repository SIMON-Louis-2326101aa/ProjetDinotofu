// EN: SpecialCharacterGroupCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterGroupCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Roadmap catalog for special character relationships and future group encounters.

#include "character/relationship/SpecialCharacterGroupCatalog.hpp"

#include <iostream>

// EN: displayRoadmap declares or implements a focused behavior used by this module.
// FR: displayRoadmap déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterGroupCatalog::displayRoadmap()
{
    std::cout << "========== GROUPES SPECIAUX ==========" << std::endl;
    std::cout << "Hazak / Henrique : meilleurs amis." << std::endl;
    std::cout << "Mattzelda / Louis / Trexof : meilleurs potes." << std::endl;
    std::cout << "Aoi / Kanadé / Sanctus : groupe proche." << std::endl;
    std::cout << "Hazak / Hestia : Hazak l'a sauvée et évite les combats à mort avec elle." << std::endl;
    std::cout << "Fail / Hazak : contrat de non-agression." << std::endl;
    std::cout << "Skuro / personnage spécial compatible : Hazak, Henrique, Aoi, Fail ou Kanadé uniquement." << std::endl;
    std::cout << "Skuro / deux personnages spéciaux compatibles : même liste, sans doublon." << std::endl;
    std::cout << "Fire Flight / Matt (PRO) : duo de commandement et de pression." << std::endl;
    std::cout << "Hestia / Sanctus / Hazak : protection sacrée, aucun massacre accepté." << std::endl;
    std::cout << "Fail / Aoi / Kanadé : laboratoire magique, instable mais puissant." << std::endl;
    std::cout << "Louis / Fire Flight / Trexof : test de créateur entre amis, projectiles et failles utiles." << std::endl;
    std::cout << "Henrique / Mattzelda / Skuro : charge instable, humour, endurance et violence lourde." << std::endl;
    std::cout << "Matt (PRO) peut aussi apparaître seul en Combat IA ou via un tirage spécial d'arène." << std::endl;
    std::cout << "Groupe aléatoire normal : aventuriers random sans personnage spécial." << std::endl;
    std::cout << "Sanctus et Skuro sont liés : même âme, deux réponses possibles face à la violence." << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;
}
