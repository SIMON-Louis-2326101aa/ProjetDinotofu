# Dinotofu

Dinotofu est un RPG / jeu d'arène en terminal développé en C++17. Il mélange création de personnage, progression, combats tactiques, exploration, quêtes, craft, boutiques, boss, personnages spéciaux, bestiaire et sauvegardes de comptes/personnages.

Le projet vient d'un ancien prototype en un seul gros fichier et il est progressivement reconstruit avec une architecture orientée objet plus propre. La version terminal sert maintenant de base de gameplay stable pendant la préparation de l'interface graphique. L'objectif est de garder les systèmes existants jouables, lisibles et réutilisables avant de construire le vrai mode histoire complet.

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
- consulter le bestiaire, les zones/habitats, les fiches de familles hostiles, le journal des matériaux, le journal des invocations, les objets rares et le lore ;
- rencontrer ou affronter des personnages spéciaux et des groupes spéciaux ;
- utiliser ou subir les systèmes cachés de données altérées ;
- progresser vers des compétences durables selon la manière de jouer, la classe, la race, le personnage et les armes utilisées ;
- suivre les statistiques du personnage, du combat, des boss, du JcJ, de l'équipement utilisé et des états spéciaux.

Le mode Histoire est visible, mais il reste volontairement en attente. La vraie histoire arrivera beaucoup plus tard, après les systèmes terminal et après l'interface graphique.

Le bestiaire affiche aussi une synthèse de progression, des niveaux de connaissance, des dangers estimés, des habitats, des faiblesses/résistances probables, du butin lié et des conseils de chasse.

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

FireFlight est le boss final. Son entrée reste verrouillée jusqu'à ce que le personnage ait suffisamment prouvé sa valeur dans la progression des boss. Il parle parfois comme une présence située au-dessus du monde, réagit aux personnages altérés et reconnaît certains personnages spéciaux.

## Exploration

L'exploration est une activité principale séparée du combat volontaire. Le joueur choisit un biome et peut y trouver plantes, matériaux, trésors, événements, traces, PNJ, mini-boss ou lieux dangereux. Certains événements peuvent lancer un combat, mais l'exploration doit surtout donner l'impression de chercher et découvrir.

Les directions de biomes prévues incluent forêt ancienne, montagne froide, marais trouble, route commerciale, ruines effondrées et plaine sauvage.

## Quêtes

Les quêtes sont séparées entre quêtes de guilde et quêtes personnelles / clients / événements.

Les quêtes de guilde sont limitées et se gèrent à la guilde. Le panneau de guilde garde ses offres pendant plusieurs combats, retire une annonce acceptée et remplace les places prises après un combat. Les quêtes personnelles peuvent venir de PNJ, clients, exploration, boutiques ou événements de combat. Les objectifs peuvent demander du combat, de l'exploration, de la livraison, des ressources ou des connaissances de bestiaire.

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

En difficulté Léthal, les cheats sont bloqués. Une tentative déclenche un événement de lore, des pénalités temporaires et des réactions d'entités liées à la justice, au destin, à l'anomalie, à l'origine ou au côté créateur du monde.

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

Lancer avec la cible de lancement du projet :

```bash
make launch
```

Sous Windows, double-cliquer sur :

```bash
Installer-Dinotofu.cmd / Lancer-Dinotofu.cmd
```

Le launcher de release vérifie GitHub, télécharge une version plus récente si elle existe, puis lance le jeu.

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

1. Préparer la couche d'interface graphique à partir des systèmes terminal existants.
2. Garder les menus, listes paginées et retours de navigation cohérents pour pouvoir les réutiliser proprement.
3. Continuer les compétences durables et l'identité des classes sans casser les sauvegardes.
4. Ajouter du contenu progressivement quand un système en a besoin : monstres, bestiaire, recettes, boutiques et événements.
5. Compléter les attributs inspirés DND après la base d'interface.
6. Construire le vrai mode Histoire après l'interface graphique.

## Note personnelle

Défi numéro un : survivre assez longtemps pour comprendre ce qu'il se passe.

Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais au moins il le fait avec style.

Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.

Ce projet existe pour être joué, critiqué, trollé, amélioré et retesté. Jouez sérieusement, mais pas trop sérieusement. Plaignez-vous si besoin, proposez des idées, insultez poliment les boss, et surtout dites-moi ce qui vous donne envie de continuer.

Si vous arrivez à devenir une légende dans Dinotofu, félicitations. Si vous mourez contre les premiers ennemis, félicitations aussi, mais différemment.

Entrez dans l'arène. On verra si vous jouez vraiment bien, ou si vous parlez juste fort sur Discord.

Pendant la vraie bêta ouverte, vous aurez peut-être même une chance d'être directement intégré au jeu de base si votre contribution le mérite. Prouvez que vous en valez la peine. Bonne chance jusque-là.

Ajout 1.32.24 :
- Ajout d'un statut persistant de fragilisation : une cible fragilisée reçoit davantage de dégâts pendant quelques tours, puis la faille se referme naturellement.
- Les soins/purifications adaptés peuvent maintenant refermer aussi cette faille, en plus de retirer l'affaiblissement classique.
- Les mages/arcanistes gagnent un nouveau palier de sort : Faille de résistance, qui combine fragilisation et élément choisi.
- Les alchimistes/artificiers gagnent Poudre de fragilisation, une option de contrôle qui prépare les prochains impacts au lieu de chercher seulement le dégât brut.
- Les IA avancées peuvent utiliser ces nouveaux effets selon leur niveau et leur profil, sans les lancer automatiquement à chaque disponibilité.
- Ajout de la Fiole de fragilisation chez l'alchimiste : elle ouvre une faille défensive différente de la Fiole d'affaiblissement.

Ajout 1.32.25 :
- Ajout du Voile élémentaire : un effet persistant défensif qui réduit la force et la durée des altérations élémentaires reçues.
- Les mages/arcanistes gagnent un nouveau palier défensif niveau 8 : Voile élémentaire. Il consomme le tour, donne une garde légère et protège des brûlures, poisons, givre, choc et saignement pendant quelques tours.
- Les mages IA peuvent utiliser ce voile quand leurs PV baissent, mais seulement selon leur niveau/profil et sans le lancer automatiquement à chaque disponibilité.
- Les affinités élémentaires prennent maintenant en compte le voile actif en plus de la race, classe, équipement et type de monstre.
- Ajout de la Potion de voile élémentaire dans les consommables, les sauvegardes et certaines boutiques.
- Les potions anti-givre et isolantes donnent aussi une petite protection élémentaire courte en plus de leur stabilisation.
- L'affichage des statistiques d'une entité montre maintenant les états actifs importants, dont brûlure, poison, froid, choc, saignement, affaiblissement, faille ouverte et voile élémentaire.

Ajout 1.32.26 :
- Les sorts ne sont plus pensés uniquement comme des techniques de classe fixes : les mages peuvent débloquer certains sorts via grimoires de bibliothèque ou de marché noir, avec prérequis de niveau et compatibilité de classe.
- Ajout de parchemins magiques consommables à usage unique pour les non-mages : étincelle arcanique, voile élémentaire et faille fragile.
- Les sorts de mage lancés avec un bâton/catalyseur abîmé, brisé ou non adapté ont maintenant une chance de canalisation réduite.


Ajout 1.32.27 :
- Clarification concrète du système magie/parchemins/grimoires : certains sorts existent seulement en parchemin, certains seulement en grimoire, certains peuvent exister dans les deux formats.
- Les mages peuvent eux aussi utiliser les parchemins consommables, même quand le sort n'est pas appris durablement.
- Ajout du Parchemin de braise errante, magie jetable sans grimoire stable équivalent, avec un léger risque de retour de flamme.
- Ajout du Grimoire d'aiguille de givre, sort apprenable sans parchemin courant, réservé aux profils de mage compatibles avec prérequis.
- Ajout de notes de bibliothèque/bestiaire sur les supports magiques, catalyseurs, résistances et faiblesses élémentaires.

Ajout 1.32.28 :
- Ajout de la Suture de mana : effet persistant de régénération courte, affiché dans les états actifs et traité au début des tours.
- Ajout du Grimoire de suture de mana, un sort apprenable sans parchemin courant, limité aux vrais profils canalisateurs avec prérequis de niveau et catalyseur cohérent.
- Ajout du Parchemin de purification mineure, consommable utilisable par les mages comme par les non-mages pour retirer des altérations simples.
- Les profils compatibles avec l'étude magique sont élargis aux classes naturellement mystiques, sacrées, d'invocation ou hybrides cohérentes, sans ouvrir la magie durable à tout le monde.
- Ajout d'entrées de bestiaire/bibliothèque sur purification mineure, suture de mana et supports magiques non équivalents.


### Patch 1.32.29
- Ajout du Parchemin de venin rampant : magie jetable sans grimoire courant, utilisable par mages et non-mages.
- Ajout du Grimoire des ronces occultes : apprentissage réservé aux profils magiques compatibles, avec prérequis de niveau et catalyseur conseillé.
- Bestiaire/bibliothèque enrichis pour distinguer davantage sorts apprenables, parchemins jetables et catalyseurs.

Ajout 1.32.30 :
- Grosse passe bestiaire : les fiches affichent maintenant danger estimé, habitat/origine, faiblesses, résistances, butin/ressource liée et conseil de chasse selon le niveau de connaissance.
- Ajout d'une catégorie dédiée aux effets et altérations : brûlure, poison, givre, choc, saignement, affaiblissement, faille ouverte, voile élémentaire et suture de mana.
- Ajout de nouvelles entrées de familles/variantes : slimes colorés, racine étrangleuse, squelette archer rouillé, oracle fissuré et bandit apothicaire.
- Ajout d'un index tactique dans le bestiaire pour lire rapidement les grandes familles, leurs contres et les pièges d'interprétation.
- Les entrées découvertes pendant la partie stockent aussi des indices persistants : danger, habitat, faiblesses, résistances, butin et stratégie.

### V1.32.32
- Bestiaire enrichi : carnet de traque, registre par niveau de connaissance et nouvelles fiches de terrain.
- Les entrées peuvent être consultées par progression : inconnues, existence confirmée, informations utiles ou fiches complètes.


### V1.32.33
- Ajout d'une première infrastructure de publication GitHub : scripts de packaging, workflow GitHub Actions, guide d'installation et base installer/launcher PowerShell.
- Ajout de scripts pour créer une archive source sans exécutable et une release Linux compilée.
- Préparation d'un launcher Windows capable de vérifier les releases GitHub, télécharger une mise à jour avec progression et lancer le jeu quand une release Windows précompilée sera disponible.

### V1.32.35

- Automatisation des tags/releases GitHub depuis la version du code.
- Ajout d'un pack installer/launcher Linux en plus du pack Windows.
- Les releases peuvent maintenant fournir jeu Windows, jeu Linux, installateur Windows, installateur Linux et archive source propre.

V1.32.40 - Préparation interface / longues listes
- Les listes d'armes, armures, consommables et matériaux de l'inventaire utilisent maintenant la pagination commune au lieu de dérouler toute la réserve d'un coup.
- Le journal de quêtes ouvre directement sur les quêtes actives, avec pagination, et garde les quêtes terminées dans une vue séparée.
- Les choix page précédente / page suivante sont harmonisés avec le bestiaire pour préparer une future interface graphique plus propre.
- Nettoyage de quelques phrases trop méta afin de préserver l'immersion pendant les événements et menus.


V1.32.41 - Préparation interface / boutiques paginées
- Les listes d’articles en boutique sont maintenant paginées, comme le bestiaire, l’inventaire et les quêtes.
- Les choix d’achat utilisent une sélection locale par page pour éviter les listes trop longues.
- Le contrôle de packaging reste propre et ne contient plus d’outils liés à la configuration locale du développeur.

V1.35.06 - Mise à niveau version/interface
- Version du projet alignée sur V1.35.06 après la branche de préparation interface, devenue plus importante que de simples patchs.
- Liste de priorités mise à jour : la préparation de l’interface graphique devient la direction visible ; l’équilibrage continue naturellement en fond, sans être répété comme tâche séparée.
- Les JSON de réserve vides sont maintenant des tableaux valides, les dialogues des personnages spéciaux incluent Trexof, les archives de cheats sont synchronisées et la console/entrée est sécurisée.

## Version 1.35.07
- Ajout des modèles neutres `MenuScreen` / `MenuOption` pour centraliser les données de menus avant affichage.
- Les menus terminal peuvent maintenant passer par `TerminalInterface`, ce qui prépare la future interface graphique.
- Les menus de tour de combat, choix de cible, choix de cible de groupe et après-combat ont été migrés vers ce modèle partagé.


## Version 1.35.08
- Préparation IG poursuivie : les écrans équipement, sélection d'inventaire, potions, compétences de rôle et statistiques passent par les données partagées `MenuScreen`.
- Davantage de menus exposent des `screenId` et `actionId` stables, pour que la future interface graphique consomme les mêmes actions que le terminal.
- Le rendu terminal reste actif via `TerminalInterface`, afin que le jeu reste jouable pendant la transition.


## Version 1.35.09
- Préparation IG poursuivie : boutiques, stock de boutique, inspection d'article, bestiaire, listes/fiches de bestiaire, hub de quêtes, guilde, lieux visitables, PNJ notables et choix d'exploration passent davantage par `MenuScreen`.
- Les écrans de boutique, bestiaire, quêtes et exploration exposent plus de `screenId` / `actionId` stables pour la future interface graphique.
- Le terminal reste jouable comme avant, mais davantage de menus passent maintenant par `TerminalInterface`.


## Version 1.35.10
- Ajout d'un snapshot `GuiMenuSnapshot` / `GuiMenuActionSnapshot` pour donner à la future interface graphique une vue propre des écrans `MenuScreen` sans dépendre du rendu terminal.
- Ajout de `TerminalInterface::askMenuChoice`, afin de centraliser le rendu + la saisie sur les menus migrés et d'éviter de dupliquer le prompt partout.
- Les menus de comptes et personnages sauvegardés passent maintenant davantage par `MenuScreen` avec des `screenId` / `actionId` stables.
- Plusieurs choix d'événements d'exploration passent par un helper d'écran commun, avec nettoyage d'une phrase trop méta pendant les vagues hostiles.

## Version 1.35.11
- Préparation IG poursuivie sur les listes paginées de l'inventaire : armes, armures, consommables et matériaux utilisent maintenant davantage les données partagées `MenuScreen`.
- Ajout de helpers de pagination capables d'ajouter directement les options de navigation dans un `MenuScreen`, pour réduire le code uniquement terminal.
- Les entrées d'inventaire exposent des actions stables comme `inventory.weapon.select`, `inventory.armor.select`, `inventory.consumable.select` et `inventory.material.select`.
- Suppression d'un ancien helper d'affichage terminal des kits de réparation devenu inutile après la centralisation de la page consommables.



## Version 1.35.12
- Préparation IG poursuivie : choix de difficulté, choix de session, choix d'activité, confirmation d'activité et écran d'information passent maintenant par les données partagées `MenuScreen`.
- La sélection des comptes/personnages secondaires en coop utilise maintenant des écrans paginés `MenuScreen` au lieu de longues listes terminal brutes.
- Le craft utilise maintenant des écrans partagés avec choix locaux par page, navigation 98/99, fiche détaillée du schéma et confirmation avant fabrication.
- Les écrans de compte, personnage et action de l'échange/don exposent des `screenId` / `actionId` pour le futur renderer graphique.
- `MenuScreen` peut maintenant exporter un `GuiMenuSnapshot`, et `TerminalInterface` peut demander uniquement une option réellement présente et active.

## Version 1.35.13
- Création de personnage : les choix de race et de classe passent maintenant par des MenuScreen structurés au lieu de listes terminal brutes.
- Catalogue des classes : ajout de ClassOptionInfo pour exposer les noms, rôles et statistiques de départ sans dépendre d'un affichage console.
- Échange / don : sélection d'armes, armures, consommables et matériaux via menus paginés structurés.
- Les armes/armures équipées sont bloquées directement dans les choix désactivés du menu.


## Version 1.35.14
- Ajout d'un helper commun `MessageScreen` pour les écrans d'information, de saisie texte et de confirmation par mot-clé.
- Les flux de comptes locaux, import, création, connexion, extraction et suppression utilisent davantage des écrans d'interface structurés.
- Les flux de personnages, nom, avertissement de version, adaptation d'ancienne sauvegarde, refus de maîtrise, transfert, extraction, clone et suppression utilisent davantage des écrans partagés.
- Les confirmations dangereuses SUPPRIMER / TRANSFERER sont rapprochées du modèle commun, ce qui prépare mieux le futur renderer graphique.

## Version 1.35.15
- Préparation IG poursuivie sur les boss : choix solo/coop, sélection manuelle, analyse de puissance et confirmation passent davantage par `MenuScreen`.
- La sélection de boss expose maintenant des options structurées avec `screenId` / `actionId`, au lieu d'une liste console brute.
- Les soins de soutien en boss coop utilisent des écrans partagés pour choisir l'action, la cible et la potion.
- `BossCatalog` expose les noms et indices de registre sans imposer l'affichage terminal, ce qui prépare mieux le futur renderer graphique.

## Version 1.35.16
- Préparation IG poursuivie sur les écrans de résultat de combat : choix de rencontre PvE, fuite, victoire, défaite et réveil passent davantage par des écrans communs.
- Les récompenses complètes et partielles sont maintenant rendues via les données d'interface partagées au lieu de cadres terminal écrits en dur.
- Les conséquences de mort non létale, la corruption létale et l'anomalie de survie sont centralisées dans des écrans réutilisables.
- Plusieurs résultats de boss et entêtes de récompenses coop utilisent maintenant `MessageScreen`, tout en gardant le terminal jouable pendant la préparation de l'IG.

## Version 1.35.17
- Ajout d'un modèle d'état de combat prêt pour l'IG : duels, groupes, invocations, PV, statuts et ciblage.
- Reprise de plusieurs écrans JcJ/PvP IA, exploration et dialogues spéciaux via MenuScreen/MessageScreen.
- Début et impacts d'ultimes de boss centralisés via des écrans de message partagés.

## Version 1.35.18
- Passe installer/launcher Windows : scripts Windows en ASCII pour eviter les accents casses dans PowerShell/CMD.
- Dossier d'installation par defaut : `Downloads/ProjetDinotofu` ; le joueur peut choisir un autre dossier parent, mais le dossier final reste toujours `ProjetDinotofu`.
- Le launcher Windows n'utilise pas WSL ; la release Windows doit contenir `Dinotofu.exe`.
- L'installer/launcher Linux suit maintenant la meme regle `ProjetDinotofu`.
