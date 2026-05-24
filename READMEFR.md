# Dinotofu

Dinotofu est un RPG / jeu d'arène en terminal développé en C++17. Il mélange création de personnage, progression, combats tactiques, exploration, quêtes, craft, boutiques, boss, personnages spéciaux, bestiaire et sauvegardes de comptes/personnages.

Le projet vient d'un ancien prototype en un seul gros fichier et il est progressivement reconstruit avec une architecture orientée objet plus propre. Le jeu reste volontairement en terminal pour le moment : la priorité est de stabiliser les systèmes RPG, puis les systèmes inspirés DND, puis de préparer une interface graphique, et seulement ensuite de construire le vrai mode histoire complet.

## Ce qu'on peut faire dans le jeu

Dinotofu permet déjà de jouer ou de tester plusieurs activités :

- créer, charger et sauvegarder des comptes et personnages ;
- choisir une race, une classe, une difficulté et certaines identités spéciales protégées ;
- combattre en PvP IA, PvP deux joueurs, PvE monstres et PvE boss ;
- explorer des biomes avec ressources, événements, coffres, pièges, lieux dangereux, mini-boss et traces rares ;
- suivre des quêtes de guilde, de clients, d'exploration, de combat, de livraison ou liées au bestiaire ;
- visiter des boutiques et lieux comme la forge, l'herboristerie, la boutique de monstres, les équipements, les consommables et la bibliothèque ;
- gérer inventaire, armes, armures, consommables, matériaux, kits de réparation, qualités et durabilité ;
- fabriquer des objets à partir de recettes et de matériaux de différentes qualités ;
- consulter le bestiaire, le journal des matériaux, le journal des invocations, les objets rares et le lore ;
- rencontrer ou affronter des personnages spéciaux et des groupes spéciaux ;
- utiliser ou subir les systèmes cachés de données altérées ;
- progresser vers des compétences durables selon la manière de jouer, la classe, la race, le personnage et les armes utilisées ;
- suivre les statistiques du personnage, du combat, des boss, du JcJ, de l'équipement utilisé et des états spéciaux.

Le mode Histoire est visible, mais il reste volontairement en attente. La vraie histoire arrivera beaucoup plus tard, après les systèmes terminal et après l'interface graphique.

## Activités principales

Le menu principal est organisé autour des activités suivantes :

1. Histoire
2. Combats
3. Exploration
4. Quêtes
5. Boutiques / lieux visitables
6. PNJ notables
7. Échange / don
8. Information sur toutes les options
0. Sauvegarder et quitter

L'option d'information sert à expliquer clairement ce que chaque activité fait et ce qui est prévu ensuite.

## Combat

Les combats sont au tour par tour. Selon le mode, le joueur peut attaquer, utiliser des potions, gérer son équipement, prendre une posture de défense, provoquer en tank, ouvrir l'inventaire ou le bestiaire, contrôler des invocations, utiliser des actions d'interface/rôle, tenter de fuir quand le mode l'autorise, ou passer son tour.

En combat de boss, la fuite est impossible. Ce refus doit être présenté avec un texte de lore et non comme un simple message technique. Les ultimes de boss se débloquent seulement après le passage sous le seuil des 50% PV.

## Boss

Dinotofu possède un grand roster de boss validés. Les trois premiers boss sont visibles par nom dès le départ, tandis que les entités suivantes apparaissent souvent comme des variations d'énergie inconnues jusqu'à leur découverte.

FireFlight est le boss final. Son entrée reste verrouillée jusqu'à ce que le personnage ait suffisamment prouvé sa valeur dans la progression des boss. Il possède des dialogues méta/dev, réagit aux personnages altérés et reconnaît certains personnages spéciaux.

## Exploration

L'exploration est une activité principale séparée du combat volontaire. Le joueur choisit un biome et peut y trouver plantes, matériaux, trésors, événements, traces, PNJ, mini-boss ou lieux dangereux. Certains événements peuvent lancer un combat, mais l'exploration doit surtout donner l'impression de chercher et découvrir.

Les directions de biomes prévues incluent forêt ancienne, montagne froide, marais trouble, route commerciale, ruines effondrées et plaine sauvage.

## Quêtes

Les quêtes sont séparées entre quêtes de guilde et quêtes personnelles / clients / événements.

Les quêtes de guilde sont limitées et se gèrent à la guilde. Les quêtes personnelles peuvent venir de PNJ, clients, exploration, boutiques ou événements de combat. Les objectifs peuvent demander du combat, de l'exploration, de la livraison, des ressources ou des connaissances de bestiaire.

## Inventaire, craft et économie

L'inventaire gère armes, armures, consommables, matériaux, kits de réparation, objets spéciaux, recettes de craft et accès au bestiaire. Les matériaux et plantes peuvent avoir plusieurs qualités : faible qualité, normal, haute qualité, impur, pur ou exceptionnel. Deux qualités différentes ne se stackent pas ensemble.

Le craft prend en compte les recettes, les matériaux requis, leur qualité et certains bonus de métier. Les matériaux exceptionnels pourront donner des particularités spéciales aux objets fabriqués.

Les boutiques et lieux visitables préparent l'économie longue durée : achat, vente, stock tournant, rareté, réparation, informations, livres et quêtes de PNJ.

## Bestiaire et connaissance

Le bestiaire n'est pas seulement une liste de monstres. Il sert aussi pour les boss, invocations, matériaux, plantes, races, personnages spéciaux, objets rares, divinités et lore.

La connaissance est progressive : le joueur peut rencontrer une entité, la vaincre, acheter des informations, lire du lore, trouver une trace rare ou débloquer des détails par l'exploration et le combat.

## Invocations et alliés

Les invocations utilisent une base avec slots et lien d'invocation. Certaines peuvent être maintenues manuellement ou sacrifiées. Ce système prépare plus tard le mana, le maintien, les sacrifices avancés, l'IA alliée, les ombres liées à Hazak, les groupes complets et l'évolution des invocations.

## Compétences durables

Dinotofu distingue les compétences passives et actives.

Les compétences passives progressent ou se débloquent naturellement selon la façon de jouer. Par exemple, enchaîner des kills à la dague peut mener vers une compétence active comme Enchaînement, tandis que les kills à l'arc peuvent progresser vers Œil de rôdeur. Les mains nues, le bâton, la race, la classe et certains personnages peuvent aussi influencer cette progression.

Les compétences actives auront un délai de réutilisation et de vrais effets en combat. La base actuelle suit déjà des déblocages et de la progression, mais le vrai menu de compétences en combat et la gestion complète des délais restent à enrichir.

À terme, certaines compétences viendront du gameplay, des montées de niveau, de la race, du personnage, de la classe, des armes les plus utilisées et de défis ou conditions cachées.

## Personnages spéciaux

Des personnages spéciaux protégés existent avec validation d'identité, race native, comportements particuliers, relations de groupe et moments uniques en combat. Certains sont liés aux personnages DND ou amis du créateur, et FireFlight possède des dialogues spéciaux pour eux.

Matt (PRO) reste un adversaire spécial et n'est pas destiné à être une identité jouable normale.

## Codes cachés et données altérées

Certaines commandes cachées peuvent marquer un personnage comme Altéré. Un personnage altéré garde ce statut même si les effets actifs sont désactivés plus tard.

En difficulté Léthal, les cheats sont bloqués. Une tentative déclenche un événement de lore, des pénalités temporaires et des réactions d'entités liées à la justice, au destin, à l'anomalie, à l'origine ou au côté développeur du monde.

Certains codes servent au test, à l'anti-grind ou à des interactions cachées, mais les utiliser change l'identité du personnage.

## Difficultés

Les difficultés prévues sont Facile, Normal, Difficile, Cauchemar et Léthal.

La difficulté peut influencer ressources de départ, récompenses, durabilité, fuite, mort, respawn, loot et boutiques. Le Léthal est le mode sérieux : la mort y est pensée comme définitive, sauf très rares exceptions narratives futures.

## Sauvegardes et Git

Le projet utilise des sauvegardes JSON pour les comptes et personnages. Les sauvegardes personnelles ne doivent pas être envoyées sur Git.

Les dossiers de sauvegarde restent présents grâce aux `.gitkeep`, tandis que les fichiers générés de sauvegarde sont ignorés.

## Prérequis

Nécessaire sous Linux :

- `g++`
- `make`
- support C++17

Installation possible sur Debian / Ubuntu :

```bash
sudo apt update
sudo apt install -y build-essential make g++
```

Installation WSL pour Windows :

```bash
wsl --install
# Si Windows demande une distribution, prendre Ubuntu.
wsl
sudo apt update
sudo apt install build-essential
sudo apt install cmake
```

## Commandes

Compiler :

```bash
make
```

Lancer le jeu directement après compilation :

```bash
make run
```

Lancer avec vérification automatique de version/rebuild :

```bash
./run_dinotofu.sh
# ou
make launch
```

Sous Windows, double-cliquer sur :

```bash
run_dinotofu_windows.bat
```

Ce lanceur appelle WSL, vérifie si la version du build a changé, reconstruit si besoin, nettoie l’écran puis lance le jeu.

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
make desktop
# ou
make install-desktop
```

Le lanceur intelligent crée aussi un raccourci bureau si possible.

## Structure du projet

Le projet est organisé autour de dossiers comme :

- `src/core/`
- `src/combat/`
- `src/combat/modes/`
- `src/combat/boss/`
- `src/character/`
- `src/entity/`
- `src/item/`
- `src/weapon/`
- `src/armor/`
- `src/consumable/`
- `src/material/`
- `src/interface/`
- `src/menu/`
- `src/progression/`
- `src/death/`
- `src/bestiary/`
- `src/attribute/`
- `src/economy/`
- `src/shop/`
- `src/save/`
- `src/cheat/`
- `assets/config/`
- `assets/saves/`

## Direction de développement

Les priorités actuelles sont :

1. Enrichir les compétences durables avec progression passive, compétences actives et délais.
2. Continuer à nettoyer l'interface terminal sans cacher les options utiles.
3. Polir le craft et l'économie.
4. Compléter les attributs inspirés DND quand les autres systèmes seront stables.
5. Préparer une interface graphique.
6. Construire le vrai mode Histoire après l'interface graphique.

## Note personnelle

Défi numéro un : survivre assez longtemps pour comprendre ce qu'il se passe.

Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais au moins il le fait avec style.

Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.

Ce projet existe pour être joué, critiqué, trollé, amélioré et retesté. Jouez sérieusement, mais pas trop sérieusement. Plaignez-vous si besoin, proposez des idées, insultez poliment les boss, et surtout dites-moi ce qui vous donne envie de continuer.

Si vous arrivez à devenir une légende dans Dinotofu, félicitations. Si vous mourez contre les premiers ennemis, félicitations aussi, mais différemment.

Entrez dans l'arène. On verra si vous jouez vraiment bien, ou si vous parlez juste fort sur Discord.

Pendant la vraie bêta ouverte, vous aurez peut-être même une chance d'être directement intégré au jeu de base si votre contribution le mérite. Prouvez que vous en valez la peine. Bonne chance jusque-là.
