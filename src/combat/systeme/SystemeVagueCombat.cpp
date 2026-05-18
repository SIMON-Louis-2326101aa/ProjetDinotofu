#include "combat/systeme/SystemeVagueCombat.hpp"

#include "core/Console.hpp"
#include "entite/CatalogueMonstres.hpp"

#include <iostream>

FileEnnemisCombat SystemeVagueCombat::creerVagueDemo()
{
    FileEnnemisCombat vague;

    vague.ajouterDansFile(CatalogueMonstres::creerGobelin());
    vague.ajouterDansFile(CatalogueMonstres::creerGobelinBrutal());
    vague.ajouterDansFile(CatalogueMonstres::creerLoupAffame());
    vague.ajouterDansFile(CatalogueMonstres::creerSquelette());
    vague.ajouterDansFile(CatalogueMonstres::creerOrcMineur());

    vague.remplirActifs();

    return vague;
}

void SystemeVagueCombat::afficherIntroductionVague()
{
    Console::clear();

    std::cout << "Une vague de monstres approche." << std::endl;
    std::cout << "Ils ne viennent pas tous en même temps..." << std::endl;
    std::cout << "Mais tant que la file n'est pas vide, le combat continue." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);
}

void SystemeVagueCombat::afficherArriveePremiereLigne(const FileEnnemisCombat& vague)
{
    std::cout << "La première ligne ennemie entre dans l'arène." << std::endl;
    std::cout << std::endl;

    vague.afficherEnnemisActifs();
    vague.afficherFileResume();
}