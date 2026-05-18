// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "boss/BossCatalog.hpp"

#include <iostream>

void BossCatalog::displayAvailableBosses()
{
    std::cout << "1 : Ange" << std::endl;
    std::cout << "    Une entité lumineuse, calme, presque trop pure pour être honnête." << std::endl;
    std::cout << std::endl;

    std::cout << "2 : Démon" << std::endl;
    std::cout << "    Une présence sombre, brutale, qui semble se nourrir de la peur." << std::endl;
    std::cout << std::endl;

    std::cout << "3 : Protecteur universel déchu" << std::endl;
    std::cout << "    Une ancienne force sacrée, devenue mur vivant contre l'humanité." << std::endl;
    std::cout << std::endl;
}

Boss BossCatalog::creerBoss(int choice)
{
    if (choice == 1)
    {
        return Boss(1, "Fitoria", "Ange", 800, 3, 10, 15, 3, 5, 3, 14);
    }

    if (choice == 2)
    {
        return Boss(2, "Zelef", "Démon", 600, 5, 15, 22, 5, 4, 4, 10);
    }

    return Boss(3, "Atlas", "Protecteur universel déchu", 1200, 2, 5, 8, 2, 5, 3, 12);
}