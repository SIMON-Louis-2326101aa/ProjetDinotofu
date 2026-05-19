# Dinotofu   
   
Dinotofu est un RPG / jeu d'arène en terminal développé en C++17.   
   
Le projet est actuellement reconstruit à partir d'un ancien prototype en un seul gros fichier, vers une architecture plus propre en orienté objet. Le jeu reste entièrement en terminal pour l'instant, mais la base du code prépare déjà des systèmes futurs comme les sauvegardes de campagne, l'évolution de classe, une meilleure gestion d'inventaire, les mécaniques de boss, les vagues d'ennemis, la coop, les alliés IA, les invocations, le bestiaire progressif, les statistiques de compte et les sauvegardes JSON.   
   
## État actuel   
   
Dinotofu est encore en développement.   
   
Les modes de combat actuellement jouables sont :   
   
- PvP : joueur contre joueur   
- PvP IA : joueur contre adversaire contrôlé par l'ordinateur   
- PvE monstres : combat contre des vagues d'ennemis   
- PvE Boss : combat d'arène contre un boss   
   
Le mode campagne est prévu, mais pas encore entièrement implémenté.   
   
## Concept du jeu   
   
Dinotofu est pensé comme un RPG médiéval-fantastique en terminal, avec des combats de type arène.   
   
Le combat fonctionne dans un esprit tour par tour : le joueur choisit des actions comme attaquer, utiliser des potions, consulter son équipement, ouvrir son inventaire, passer son tour ou tenter de fuir.   
   
Le jeu utilise beaucoup d'aléatoire, dans un esprit proche du JDR / DND, pour les dégâts, les choix de l'IA, les tentatives de fuite, la future initiative et les rencontres spéciales.   
   
Systèmes prévus sur le long terme :   
   
- création de personnage et sauvegardes JSON   
- progression de campagne   
- monstres et boss débloqués progressivement   
- attributs façon DND : Force, Dextérité, Constitution, Intelligence, Sagesse, Charisme   
- évolution de classe selon les statistiques   
- types de dégâts et résistances   
- armes, armures, matériaux, durabilité, effets spéciaux, équipements héroïques et reliques   
- bestiaire et connaissance progressive des matériaux   
- statistiques globales de compte, tous personnages confondus   
- difficulté Léthal avec mort définitive du personnage   
- coop, alliés IA, invocations, groupes de combat et initiative   
- personnages spéciaux semi-humains avec chances d'apparition rares   
   
## Prérequis de compilation   
   
Requis sur Linux :   
   
- `g++`   
- `make`   
- support C++17   
   
Commande d'installation exemple sur Debian / Ubuntu :   
   
```bash   
sudo apt update   
sudo apt install -y build-essential make g++   
```   
   
## Commandes Make   
   
Compiler le projet :   
   
```bash   
make   
```   
Lancer le jeu :   
   
```bash   
make run   
```   
Nettoyer les fichiers générés :   
   
```bash   
make clean   
```   
Reconstruire depuis zéro :   
   
```bash   
make rebuild   
```   
Créer un lanceur Linux cliquable :   
   
```bash   
make install-desktop   
```   
Supprimer le lanceur Linux :   
   
```bash   
make remove-desktop   
```   
   
## Lancement sur Linux   
   
Méthode recommandée :   
   
```bash   
make run   
```   
Ou compiler puis lancer l'exécutable directement :   
   
```bash   
make   
./output/Dinotofu   
```   
   
## Lancement sur Windows   
   
Le Makefile est principalement prévu pour Linux.   
   
Options recommandées sur Windows :   
   
### Option 1 : WSL   
   
Installer WSL, ouvrir le projet dans un terminal Linux, puis utiliser :   
   
```bash   
make run   
```   
### Option 2 : MSYS2 / MinGW   
   
Installer une toolchain MinGW et `make`, puis lancer le projet depuis un terminal MSYS2 :   
   
```bash   
make run   
```   
### Option 3 : commande g++ manuelle   
   
Si tous les includes sont correctement configurés, une commande manuelle peut être utilisée, mais c'est moins confortable que le Makefile :   
   
```bash   
g++ -std=c++17 -Wall -Wextra -Iinclude $(find src -name "*.cpp") -o output/Dinotofu   
```   
Cette commande directe est surtout pratique dans un shell de type Linux. Sur Windows CMD ou PowerShell classique, WSL ou MSYS2 restent préférables.   
   
## Notes Git   
   
Les dossiers générés ne doivent pas être commit :   
   
- `build/`   
- `output/`   
   
Le vrai `Makefile` doit rester dans Git.   
   
## Organisation du code   
   
Le projet utilise une architecture `include/` et `src/`.   
   
Les headers sont dans `include/`, les fichiers d'implémentation sont dans `src/`, et les deux dossiers suivent presque la même structure.   
   
Les identifiants de code et les dossiers sont écrits en anglais. Les textes affichés au joueur peuvent rester en français.   
   
Le nom préféré est `item/`, pas `object/`. Le projet ne doit plus contenir d'ancien dossier racine `action/`. Le dossier `combat/action/` est conservé car il contient une vraie logique d'actions de combat.   
   
## Passe de stabilisation actuelle   
   
Points vérifiés et stabilisés :   
   
- `make clean` et `make rebuild` fonctionnent correctement.   
- `build/` et `output/` sont ignorés par Git.   
- L'ancien dossier racine `action/` n'est plus nécessaire.   
- `combat/action/` est gardé car il contient la logique réelle d'actions de combat.   
- Les quatre modes de combat actuels utilisent une structure de menu de combat partagée.   
- Les menus équipement et inventaire peuvent être ouverts sans consommer le tour de combat.   
- Les actions de potion ratées ne consomment pas le tour de combat.   
- Les potions offensives utilisent la vraie puissance de la potion au lieu d'une valeur fixe codée en dur.   
- La sélection de cible en PvE monstres correspond aux numéros affichés.   
- Les tentatives de fuite contre un boss consomment correctement le tour tout en gardant la fuite impossible.   
- Les dossiers futurs de sauvegarde/progression sont préparés pour les systèmes JSON.   
- Les vagues PvE monstres sont générées selon le niveau du joueur.   
- Les récompenses PvE monstres distinguent les ennemis vaincus et les ennemis en fuite.   
- La fuite du joueur en PvE monstres donne une récompense partielle selon la difficulté.   
- Les ennemis encore en vie mais déjà blessés peuvent donner une petite partie de récompense.   
- Les pénalités de mort non définitive sont préparées : or, expérience, consommables, durabilité d'équipement, casse, destruction irréparable et vol futur selon contexte.   
- La difficulté est maintenant sélectionnée avant la classe et peut modifier le kit de départ.   
   
## Systèmes futurs préparés   
   
Le projet contient maintenant des dossiers et fichiers placeholder pour les systèmes futurs :   
   
- `character/` : personnages spéciaux semi-humains et génération aléatoire de personnages   
- `combat/encounter/` : rencontres spéciales rares et génération de rencontres   
- `combat/group/` : future coop, alliés IA, invocations, slots d'unités, ordre de tour et initiative   
- `progression/` : statistiques, bestiaire, connaissance des matériaux, difficulté, révélation d'identité et pénalités de mort   
- `save/` : futur gestionnaire de sauvegarde JSON et modèles de sauvegarde   
- `interface/menu/progression/` : futurs menus de statistiques, bestiaire, matériaux et compte   
- `assets/saves/accounts/` : futures sauvegardes JSON de compte   
- `assets/saves/characters/playable/` : futurs personnages jouables   
- `assets/saves/characters/graveyard/` : futurs personnages morts en Léthal   
- `assets/saves/bestiary/` : futures sauvegardes de connaissance du bestiaire   
- `assets/saves/materials/` : futures sauvegardes de connaissance des matériaux   
   
## Difficulté et mort   
   
Les récompenses en cas de fuite ne doivent pas être tout ou rien. Le mode Facile peut être généreux, Normal reste équilibré, et les difficultés plus élevées deviennent plus dures.   
   
La mort non définitive doit quand même faire mal : perte d'or, d'expérience, de consommables, de matériaux plus tard, et dégâts sur l'équipement équipé.   
   
En difficulté Léthal, les statistiques de mort deviennent corrompues au lieu d'afficher un compteur normal :   
   
```text   
Morts du personnage : [STATISTIQUE CORROMPUE]   
Vous ne deviez pas mourir.   
Statut : personnage supprimé du registre des vivants.   
Connais-tu quelqu’un capable d’échapper à la mort ? Moi non...   
```   
   
Même avant de mourir, l'affichage de statistiques en Léthal doit prévenir le joueur :   
   
```text   
Morts du personnage : [STATISTIQUE CORROMPUE]   
But de mission : survivre.   
```   
   
## Règle de connaissance des boss   
   
Rencontrer un boss ne suffit pas à l'identifier complètement.   
   
Le nom d'un boss ne doit être ajouté au bestiaire que lorsque le boss dit son propre nom, lorsqu'un personnage fiable le nomme, lorsqu'un seuil narratif comme 50% PV le révèle, ou lorsqu'une phrase avant défaite donne assez d'informations.   
   
Avant ça, le bestiaire peut afficher inconnu, une description physique, des indices ou des identités supposées liées à des écrits et légendes déjà découverts.   
   
## Prepared future systems   
   
   
```text   
Dinotofu/   
├── Makefile   
├── README.md   
├── .gitignore   
│   
├── assets/   
│   ├── config/   
│   │   ├── difficulties/   
│   │   ├── encounters/   
│   │   ├── loot/   
│   │   └── pantheon/   
│   ├── saves/   
│   │   ├── accounts/   
│   │   ├── bestiary/   
│   │   ├── campaign/   
│   │   ├── characters/   
│   │   │   ├── graveyard/   
│   │   │   └── playable/   
│   │   ├── encounters/   
│   │   ├── lore/   
│   │   ├── materials/   
│   │   ├── player/   
│   │   └── statistics/   
│   └── texts/   
│       ├── bosses/   
│       ├── descriptions/   
│       ├── dialogues/   
│       ├── lore/   
│       └── special_characters/   
│   
├── include/   
│   ├── adventure/   
│   ├── boss/   
│   ├── character/   
│   ├── class_system/   
│   ├── combat/   
│   │   ├── action/   
│   │   ├── ai/   
│   │   ├── boss/   
│   │   ├── death/   
│   │   ├── encounter/   
│   │   ├── escape/   
│   │   ├── group/   
│   │   ├── initiative/   
│   │   ├── loot/   
│   │   ├── modes/   
│   │   ├── reward/   
│   │   ├── summon/   
│   │   ├── system/   
│   │   ├── turn/   
│   │   ├── unit/   
│   │   └── wave/   
│   ├── core/   
│   ├── economy/   
│   ├── effect/   
│   ├── entity/   
│   ├── interface/   
│   │   └── menu/   
│   │       ├── equipment/   
│   │       ├── inventory/   
│   │       ├── potions/   
│   │       ├── progression/   
│   │       └── save/   
│   ├── item/   
│   │   ├── ammunition/   
│   │   ├── armor/   
│   │   ├── consumable/   
│   │   ├── durability/   
│   │   ├── effect/   
│   │   ├── material/   
│   │   ├── rarity/   
│   │   └── weapon/   
│   ├── lore/   
│   ├── progression/   
│   │   ├── bestiary/   
│   │   ├── death/   
│   │   ├── difficulty/   
│   │   ├── material/   
│   │   └── statistics/   
│   ├── save/   
│   │   ├── json/   
│   │   └── registry/   
│   ├── story/   
│   ├── utils/   
│   └── world/   
│   
└── src/   
    └── same structure as include/, with .cpp implementation files   
```   
   
## Note développeur   
   
Ce projet est personnel et expérimental. Le but est de garder l'esprit fun du prototype tout en rendant progressivement le code plus propre, plus maintenable et prêt pour des systèmes plus grands.   
   
## Note Spéciale   
   
Toi qui lis ça, oui, c'est bien moi, le dev, qui te parle. Ce jeu est encore en chantier, il a sûrement deux-trois boulons qui tremblent, des monstres qui font les malins, et des boss qui pensent vraiment être les personnages principaux. Mais justement : viens tester.   
   
Ton objectif est simple : survivre, comprendre les menus, trouver les meilleures décisions, et surtout prouver que tu n'es pas juste là pour te faire plier par un gobelin random. Si tu perds contre Matt, je ne juge pas... enfin si, un peu quand même.   
   
Défi numéro un : gagner un combat sans paniquer dans l'inventaire comme si tu cherchais tes clés un lundi matin.   
   
Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais avec style.   
   
Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.   
   
Ce projet existe surtout pour être essayé, critiqué, vanné, amélioré, puis retesté. Donc joue sérieusement, mais pas trop. Râle si tu veux, propose des idées, insulte poliment les boss, et surtout dis-moi ce qui te donne envie de continuer.   
   
Si tu arrives à devenir une légende dans Dinotofu, bravo. Si tu meurs contre les premiers ennemis, bravo aussi, mais différemment.   
   
Allez, entre dans l'arène. On va voir si tu joues vraiment bien, ou si tu parles juste fort sur Discord.   
   
Je tiens à noter que lors de la vraie bêta ouverte, tu auras une chance d'être directement intégré au jeu de base si ta contribution le mérite. Alors prouve-moi que tu en vaux le coup. Bon courage d'ici là.   

## Systèmes préparés : personnages spéciaux et codes de triche   

Cette version prépare aussi les prochains systèmes de Dinotofu sans les activer complètement pour l’instant.   
Le but est de garder le projet compilable tout en rendant l’architecture future visible.   

Systèmes de personnages spéciaux préparés :   
- identités spéciales protégées comme Hazak, Aoi, Trexof, Skuro, Sanctus, Hestia, Fire Flight et Louis ;   
- future validation par date spéciale au format `DD/MM/YYYY` ;   
- affichage futur du bonus natif sur la classe prévue du personnage ;   
- rencontres spéciales séparées des monstres classiques.   

Systèmes de codes de triche préparés :   
- statut `Altéré` après activation d’un code ;   
- avertissement avant le premier code ;   
- codes réutilisables et codes à usage unique ;   
- secrets du créateur et séquence cachée en combat ;   
- futur système d’annulation d’action avec utilisations limitées par personnage.   

Le résumé complet est disponible dans `SPECIAL_CHARACTERS_AND_CHEATS.txt`.   
