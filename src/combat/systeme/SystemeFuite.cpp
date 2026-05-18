#include "combat/systeme/SystemeFuite.hpp"
#include "combat/systeme/SystemeClassesCombat.hpp"

#include "core/Console.hpp"

#include <algorithm>
#include <iostream>

bool SystemeFuite::joueurTenteFuite(Joueur& joueur, Random& random)
{
    std::cout << joueur.getNom() << " cherche une ouverture pour fuir le combat..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    int chanceFuite = SystemeClassesCombat::getChanceFuiteBase(joueur);
    int tirage = random.entre(1, 100);

    if (tirage <= chanceFuite)
    {
        std::cout << "Fuite réussie." << std::endl;
        std::cout << joueur.getNom()
                  << " parvient à quitter l'affrontement avant d'être encerclé."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    std::cout << "Fuite échouée." << std::endl;
    std::cout << "Les ennemis bloquent le passage. Ton tour est perdu." << std::endl;
    std::cout << std::endl;

    return false;
}

bool SystemeFuite::joueurTenteFuiteBoss(const Joueur& joueur, const Boss& boss)
{
    std::cout << joueur.getNom() << " cherche une issue..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << "L'air devient lourd." << std::endl;
    std::cout << "L'arène semble se refermer comme une cage." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << boss.getNom() << " bloque toute échappatoire par sa seule présence." << std::endl;
    std::cout << "Face à une entité de ce niveau, la fuite n'est plus une option." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << "[la fuite est impossible durant ce combat]" << std::endl;
    std::cout << "La tentative échoue. Ton tour est perdu." << std::endl;
    std::cout << std::endl;

    return false;
}

bool SystemeFuite::joueurTenteFuiteDuel(
    Joueur& fuyard,
    Entite& adversaire,
    Random& random
)
{
    std::cout << fuyard.getNom() << " cherche une ouverture pour quitter le duel..." << std::endl;
    std::cout << "En duel, fuir revient à abandonner le combat." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    int chanceFuite = calculerChanceFuiteDuel(fuyard, adversaire);
    int tirage = random.entre(1, 100);

    if (tirage <= chanceFuite)
    {
        std::cout << "Fuite réussie." << std::endl;
        std::cout << fuyard.getNom() << " sort de l'arène avant de se faire achever." << std::endl;
        std::cout << adversaire.getNom() << " remporte l'affrontement." << std::endl;
        std::cout << std::endl;

        std::cout << "Répartition prévue des récompenses :" << std::endl;
        std::cout << "- " << fuyard.getNom() << " : 25%" << std::endl;
        std::cout << "- " << adversaire.getNom() << " : 75%" << std::endl;
        std::cout << std::endl;

        terminerCombatParAbandon(fuyard);
        return true;
    }

    std::cout << "Fuite échouée." << std::endl;
    std::cout << adversaire.getNom()
              << " coupe la trajectoire de "
              << fuyard.getNom()
              << "."
              << std::endl;
    std::cout << "Le duel continue, mais le tour est perdu." << std::endl;
    std::cout << std::endl;

    return false;
}

bool SystemeFuite::monstrePeutTenterFuite(const Monstre& monstre)
{
    if (monstre.estInvocation())
    {
        return false;
    }

    if (monstre.estElite())
    {
        return false;
    }

    if (monstre.getPotionsSoin() > 0)
    {
        return false;
    }

    if (monstre.getPvMax() <= 0)
    {
        return false;
    }

    int pourcentagePv = monstre.getPv() * 100 / monstre.getPvMax();

    return pourcentagePv <= 10;
}

bool SystemeFuite::monstreTenteFuite(Monstre& monstre, Random& random)
{
    if (!monstrePeutTenterFuite(monstre))
    {
        return false;
    }

    int tirage = random.entre(1, 100);

    if (tirage <= CHANCE_FUITE_MONSTRE_FAIBLE)
    {
        std::cout << monstre.getNom() << " panique et tente de fuir..." << std::endl;
        Console::pauseSecondes(1);

        std::cout << monstre.getNom()
                  << " disparaît de l'affrontement avant de recevoir le coup fatal."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    return false;
}

int SystemeFuite::calculerChanceFuiteDuel(
    const Joueur& fuyard,
    const Entite& adversaire
)
{
    int chance = SystemeClassesCombat::getChanceFuiteBase(fuyard);

    const Joueur* joueurAdverse = dynamic_cast<const Joueur*>(&adversaire);

    if (joueurAdverse != nullptr)
    {
        int ecartNiveau = joueurAdverse->getNiveau() - fuyard.getNiveau();
        chance -= ecartNiveau * 5;
    }

    int chanceAdversaire = SystemeClassesCombat::getChanceFuiteBase(adversaire);

    if (chanceAdversaire <= 35)
    {
        chance -= 10;
    }
    else if (chanceAdversaire >= 65)
    {
        chance += 5;
    }

    return std::max(15, std::min(chance, 85));
}

void SystemeFuite::terminerCombatParAbandon(Joueur& fuyard)
{
    fuyard.recevoirDegats(fuyard.getPv());
}