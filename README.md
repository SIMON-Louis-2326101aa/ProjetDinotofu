# Projet Dinotofu !

## Ce Projet est en cours de réalisation

ProjetDinotofu est un projet mis en place pour s'amuser et s'entrainer a coder en C++. 
Ce projet est personelle, crée et inventé par Louis (Maitre du projet)

Le monde de Dinotofu est un jeux vidéo basé sur l'univers medieval fantaisique ou regne combat, amusement, intrigue, decouverte et sentiments autant unic les uns que les autres. 
il dispose de 5 mods differents : Joueur contre Joueur, Joueur contre Ordinateur, Joueur contre Boss, Joueur contre Monstre/Groupe de monstre et Une campagne unic (pas encore réalisé). 
Le style de combat a été pensé tel ceux de pokemon : liste de choix possible : attaquer, boir une potion, fuir... Et les entitées possedent donc des PV, et des degats variants. 
Il existe faiblesse, resistances, capacité, et meme des races et classes qui donnerons un coté unic a votre gameplay. 
Toute variables tel que les monstres, l'ordinateur, le nombre de degat, si le coup reussit ou non.... est fait celon L'ALEATOIRE tel un jeux de DND. 

Note du dévelopeur créateur : 
J'espere que ce projet plairat a un grand nombre et laisseras de bons souvenirs et rires entre les joueurs. 

### Merci pour votre attention et votre soutien !

en voici plus ou moins l'architecture de base.

ProjetDinotofu/
├── Makefile
├── README.md
├── .gitignore
│
├── build/
│   └── (fichiers objets générés automatiquement)
│
├── output/
│   └── ProjetDinotofu
│
├── assets/
│   ├── textes/
│   │   ├── dialogues/
│   │   ├── descriptions/
│   │   └── lore/
│   │
│   ├── sauvegardes/
│   │   ├── campagnes/
│   │   └── joueurs/
│   │
│   └── config/
│       ├── classes.json
│       ├── monstres.json
│       ├── boss.json
│       ├── armes.json
│       ├── armures.json
│       ├── consommables.json
│       └── materiaux.json
│
├── include/
│   ├── core/
│   │   ├── Jeu.hpp
│   │   ├── Console.hpp
│   │   ├── Random.hpp
│   │   └── Config.hpp
│   │
│   ├── entite/
│   │   ├── Entite.hpp
│   │   ├── Joueur.hpp
│   │   ├── Monstre.hpp
│   │   ├── Boss.hpp
│   │   └── PNJ.hpp
│   │
│   ├── classe/
│   │   ├── ClasseJoueur.hpp
│   │   ├── EvolutionClasse.hpp
│   │   └── CatalogueClasses.hpp
│   │
│   ├── combat/
│   │   ├── Combat.hpp
│   │   ├── Tour.hpp
│   │   ├── IA.hpp
│   │   └── Cible.hpp
│   │
│   ├── action/
│   │   ├── Action.hpp
│   │   ├── Attaque.hpp
│   │   ├── Competence.hpp
│   │   ├── Ultime.hpp
│   │   ├── UtiliserObjet.hpp
│   │   └── PasserTour.hpp
│   │
│   ├── effet/
│   │   ├── Effet.hpp
│   │   ├── Saignement.hpp
│   │   ├── Corrosion.hpp
│   │   ├── Regeneration.hpp
│   │   ├── Immobilisation.hpp
│   │   ├── BuffDegats.hpp
│   │   └── ArmureSpeciale.hpp
│   │
│   ├── item/
│   │   ├── Item.hpp
│   │   ├── Inventaire.hpp
│   │   ├── arme/
│   │   │   ├── Arme.hpp
│   │   │   ├── TypeArme.hpp
│   │   │   └── CatalogueArmes.hpp
│   │   │
│   │   ├── armure/
│   │   │   ├── Armure.hpp
│   │   │   ├── TypeArmure.hpp
│   │   │   └── CatalogueArmures.hpp
│   │   │
│   │   ├── consommable/
│   │   │   ├── Consommable.hpp
│   │   │   ├── TypeConsommable.hpp
│   │   │   └── CatalogueConsommables.hpp
│   │   │
│   │   └── materiau/
│   │       ├── Materiau.hpp
│   │       └── CatalogueMateriaux.hpp
│   │
│   ├── progression/
│   │   ├── Niveau.hpp
│   │   ├── Experience.hpp
│   │   ├── Statistique.hpp
│   │   └── CaracteristiquesDnd.hpp
│   │
│   ├── economie/
│   │   ├── Argent.hpp
│   │   └── Boutique.hpp
│   │
│   ├── sauvegarde/
│   │   ├── Sauvegarde.hpp
│   │   ├── SauvegardeJoueur.hpp
│   │   ├── SauvegardeCampagne.hpp
│   │   └── Compte.hpp
│   │
│   ├── histoire/
│   │   ├── Campagne.hpp
│   │   ├── Chapitre.hpp
│   │   ├── Dialogue.hpp
│   │   ├── Evenement.hpp
│   │   ├── Reputation.hpp
│   │   └── ChoixJoueur.hpp
│   │
│   ├── interface/
│   │   ├── Interface.hpp
│   │   ├── InterfaceTerminal.hpp
│   │   └── InterfaceGraphique.hpp
│   │
│   └── utils/
│       ├── Utils.hpp
│       ├── StringUtils.hpp
│       └── FileUtils.hpp
│
└── src/
    ├── main.cpp
    │
    ├── core/
    │   ├── Jeu.cpp
    │   ├── Console.cpp
    │   ├── Random.cpp
    │   └── Config.cpp
    │
    ├── entite/
    │   ├── Entite.cpp
    │   ├── Joueur.cpp
    │   ├── Monstre.cpp
    │   ├── Boss.cpp
    │   └── PNJ.cpp
    │
    ├── classe/
    │   ├── ClasseJoueur.cpp
    │   ├── EvolutionClasse.cpp
    │   └── CatalogueClasses.cpp
    │
    ├── combat/
    │   ├── Combat.cpp
    │   ├── Tour.cpp
    │   ├── IA.cpp
    │   └── Cible.cpp
    │
    ├── action/
    │   ├── Action.cpp
    │   ├── Attaque.cpp
    │   ├── Competence.cpp
    │   ├── Ultime.cpp
    │   ├── UtiliserObjet.cpp
    │   └── PasserTour.cpp
    │
    ├── effet/
    │   ├── Effet.cpp
    │   ├── Saignement.cpp
    │   ├── Corrosion.cpp
    │   ├── Regeneration.cpp
    │   ├── Immobilisation.cpp
    │   ├── BuffDegats.cpp
    │   └── ArmureSpeciale.cpp
    │
    ├── item/
    │   ├── Item.cpp
    │   ├── Inventaire.cpp
    │   │
    │   ├── arme/
    │   │   ├── Arme.cpp
    │   │   ├── TypeArme.cpp
    │   │   └── CatalogueArmes.cpp
    │   │
    │   ├── armure/
    │   │   ├── Armure.cpp
    │   │   ├── TypeArmure.cpp
    │   │   └── CatalogueArmures.cpp
    │   │
    │   ├── consommable/
    │   │   ├── Consommable.cpp
    │   │   ├── TypeConsommable.cpp
    │   │   └── CatalogueConsommables.cpp
    │   │
    │   └── materiau/
    │       ├── Materiau.cpp
    │       └── CatalogueMateriaux.cpp
    │
    ├── progression/
    │   ├── Niveau.cpp
    │   ├── Experience.cpp
    │   ├── Statistique.cpp
    │   └── CaracteristiquesDnd.cpp
    │
    ├── economie/
    │   ├── Argent.cpp
    │   └── Boutique.cpp
    │
    ├── sauvegarde/
    │   ├── Sauvegarde.cpp
    │   ├── SauvegardeJoueur.cpp
    │   ├── SauvegardeCampagne.cpp
    │   └── Compte.cpp
    │
    ├── histoire/
    │   ├── Campagne.cpp
    │   ├── Chapitre.cpp
    │   ├── Dialogue.cpp
    │   ├── Evenement.cpp
    │   ├── Reputation.cpp
    │   └── ChoixJoueur.cpp
    │
    ├── interface/
    │   ├── Interface.cpp
    │   ├── InterfaceTerminal.cpp
    │   └── InterfaceGraphique.cpp
    │
    └── utils/
        ├── Utils.cpp
        ├── StringUtils.cpp
        └── FileUtils.cpp