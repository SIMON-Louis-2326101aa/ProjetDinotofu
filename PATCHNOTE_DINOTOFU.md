# Dinotofu — patchnote et état complet   

Ce fichier sert à regrouper les notes de publication et l’état détaillé du jeu.   

Les README, guides et fichiers de conception doivent rester propres : ils expliquent le jeu, l’installation et les systèmes, mais ne doivent pas devenir un journal de développement.   

## Règle du patchnote   

- Tout ce qui existe depuis le début du projet est listé ici comme un état global du jeu, sans versionnage.   
- Le versionnage détaillé commence seulement à partir de **V3.00.00**, parce que c’est le premier vrai socle histoire/publication.   
- Les anciennes versions avant V3 ne sont pas listées une par une, pour éviter un historique incohérent qui commence au milieu du projet.   

---   

# État global du jeu — sans versionnage   

Cette section résume ce que Dinotofu permet déjà de faire, sans rattacher chaque élément à une ancienne version.   

## Base du jeu   

Dinotofu est un RPG / jeu d’arène en C++17 avec deux façons principales de jouer :   

- un **mode bac à sable**, libre, fait pour tester, explorer, combattre, farmer et utiliser les systèmes du jeu sans ordre narratif imposé ;   
- un **mode histoire**, plus guidé, avec prologue, progression par chapitres, quêtes principales non refusables et routes/boutiques limitées au départ.   

La version terminal reste le socle le plus stable. L’interface graphique HTML/JS existe comme base expérimentale pour préparer une expérience desktop plus confortable.   

## Comptes, personnages et sauvegarde   

Le jeu permet déjà de :   

- créer un compte ;   
- créer un personnage ;   
- sauvegarder et charger les données ;   
- reprendre une partie ;   
- gérer plusieurs personnages ;   
- conserver des statistiques de personnage et de compte ;   
- suivre les morts, la progression, les titres, les connaissances et certaines données cachées ;   
- utiliser des sauvegardes locales sans inclure de données privées dans les ZIP de publication.   

La création de personnage comprend :   

- choix du nom ;   
- choix de la race ;   
- choix de la classe ;   
- choix de la difficulté ;   
- gestion des identités spéciales protégées ;   
- avertissements selon les versions importantes et la compatibilité des anciens personnages.   

## Races jouables et identités raciales   

Le jeu contient un système de races avec statistiques, descriptions, spécialités et affinités.   

Races principales et hybrides déjà présentes :   

- Humain ;   
- Elfe ;   
- Elfe noir ;   
- Nain ;   
- Gnome ;   
- Halfelin ;   
- Tieffelin ;   
- Aasimar ;   
- Kitsune ;   
- Fée ;   
- Drakéide / demi-dragon ;   
- Orc ;   
- Vampire ;   
- Démon / demi-démon ;   
- Semi-humain générique ;   
- Semi-humain loup ;   
- Semi-humain renard ;   
- Semi-humain chien ;   
- Semi-humain chat ;   
- Semi-humain lézard ;   
- Semi-humain piaf.   

Les races ne sont pas seulement cosmétiques. Elles peuvent influencer :   

- PV ;   
- force ;   
- magie ;   
- vitesse ;   
- résistance au feu, au froid, au poison, aux brûlures ou aux saignements selon les cas ;   
- vision nocturne ;   
- flair ;   
- réactions sociales ;   
- adaptation aux routes, biomes ou situations spécifiques ;   
- compatibilité future avec les équipements et les dialogues de morphologie.   

## Classes jouables   

Le jeu contient un grand catalogue de classes jouables, réparties par familles. Chaque classe possède ses PV, dégâts, critique, potions de départ et rôle.   

### Classes de mêlée   

Chevalier, Assassin, Colosse, Barbare, Guerrier, Moine, Duelliste, Berserker, Lancier, Briseur lourd, Gladiateur, Ravageur, Maître d’armes, Pugiliste, Faucheur, Sabreur rapide, Chevalier bouclier, Hallebardier, Épéiste errant, Cogneur de taverne, Gardien de porte, Lame brisée, Martelier.   

### Classes à distance   

Archer, Rôdeur, Arbalétrier, Chasseur, Lanceur de dagues, Tireur, Pisteur, Frondeur tactique, Éclaireur d’élite, Javelinier, Trappeur, Guetteur, Messager armé, Arquebusier expérimental, Fauconnier, Sentinelle de rempart, Chasseur de reliques, Tireur nomade.   

### Classes magiques   

Mage, Ensorceleur, Sorcier, Magicien, Pyromancien, Cryomancien, Arcaniste, Occultiste, Mage fou, Mage flame, Mage zodiacal, Électromancien, Gravemage, Miragien, Hydromancien, Géomancien, Chronomancien, Démoniste, Aéromancien, Lumomancien, Umbromancien, Cristallomancien, Sonomancien, Mage de sang, Bibliomancien.   

### Classes d’invocation   

Invocateur, Nécromancien, Dompteur, Pactisant, Marionnettiste, Totémiste, Corbeaumancien, Dresseur spectral, Gardien de familiers, Conjurateur de ruche, Belluaire, Porte-essaim, Reliquaire vivant, Gardien de totems, Appelant draconique.   

### Classes de soutien   

Barde, Clerc, Prêtre, Paladin, Gardien, Templier, Tank sacré, Oracle, Protecteur, Médecin de terrain, Infirmier de guilde, Porte-bannière, Juge novice, Chantre de bataille, Archiviste de terrain, Aumônier itinérant, Porte-lanterne, Sénéchal de guilde.   

### Classes hybrides   

Druide, Chevalier runique, Ombrelame, Mage-lame, Shaman, Chevalier draconique, Rôdeur arcanique, Moine solaire, Chaman de guerre, Lame tellurique, Danseur lunaire, Corsaire arcanique, Pèlerin martial, Fauche-âme, Brise-rune, Chevalier des brumes.   

### Classes artisanat / utilitaires   

Artificier, Forgeron, Alchimiste, Cuisinier de guilde, Cartographe, Récupérateur, Runiste, Enchanteur, Bricoleur de siège, Intendant, Joaillier runique, Charpentier de guerre, Scribe d’atelier, Herboriste de combat, Mécanicien de reliques.   

## Modes de combat   

Le jeu permet déjà plusieurs formes de combat :   

- PvP contre IA ;   
- PvP deux joueurs ;   
- PvE contre monstres ;   
- combats contre boss ;   
- boss solo ;   
- boss coop ;   
- combats de groupe ;   
- combats contre groupes d’aventuriers spéciaux ;   
- vagues de monstres ;   
- rencontres spéciales ;   
- combats d’histoire imposés.   

Les systèmes de combat déjà présents incluent :   

- dégâts et critiques ;   
- défense ;   
- posture défensive ;   
- initiative ;   
- ordre des tours ;   
- ciblage ;   
- menace ;   
- provocation ;   
- attaques spéciales ;   
- potions de combat ;   
- fuite selon contexte ;   
- récompenses ;   
- butin ;   
- observation ;   
- effets spéciaux de classes ou personnages ;   
- invocations ;   
- systèmes anti-passivité pour certains boss ;   
- décryptage partiel des boss ou ennemis très supérieurs ;   
- affichage progressif des informations connues.   

## Mort, difficultés et règles létales   

Le jeu gère plusieurs logiques de difficulté et de mort.   

Systèmes déjà présents ou préparés :   

- difficulté classique ;   
- règles de mort définitive ;   
- difficulté Léthal ;   
- blocage de certaines règles trop injustes en Facile ;   
- obligation de mort définitive en Léthal ;   
- compatibilité entre personnages selon règles de mort ;   
- statistiques de mort ;   
- textes corrompus en Léthal ;   
- pénalités de mort non définitive ;   
- perte ou endommagement d’équipement ;   
- durabilité fortement réduite après une mort ;   
- risque de casse critique ;   
- exception pour matériaux très solides ;   
- survie miraculeuse rare sous forme d’anomalie de survie ;   
- bénédictions perdues lors de certaines survies anormales ;   
- jets de survie inspirés DND en coop Léthal ;   
- réveil à 1 PV selon conditions ;   
- mort définitive si trop d’échecs en Léthal.   

## Inventaire, objets et équipement   

Le jeu possède déjà une base complète autour de l’inventaire et de l’équipement.   

Systèmes présents :   

- inventaire ;   
- affichage d’inventaire ;   
- sélection d’objets ;   
- consommables ;   
- potions ;   
- armes ;   
- armures ;   
- munitions ;   
- matériaux ;   
- raretés ;   
- qualités ;   
- durabilité ;   
- réparation ;   
- comparaison d’équipement ;   
- effets d’armes ;   
- effets d’armures ;   
- objets indestructibles selon règles ;   
- perte ou vol d’équipement selon contextes ;   
- affichage simple ou détaillé de l’équipement ;   
- logique de sac et de tri à continuer plus tard.   

## Économie, boutiques et lieux de ville   

Le jeu permet déjà de :   

- acheter ;   
- vendre ;   
- troquer ;   
- visiter des lieux ;   
- consulter des boutiques ;   
- utiliser des rotations de stocks ;   
- appliquer des prix selon règles ;   
- afficher des boutiques différentes selon contexte ;   
- consulter la bibliothèque ;   
- consulter la guilde ;   
- parler à des PNJ notables ;   
- déclencher des services de ville ;   
- faire évoluer légèrement les stocks et prix selon la progression histoire.   

Les boutiques et comptoirs peuvent déjà réagir à l’histoire :   

- ville pauvre au départ ;   
- stocks limités ;   
- forge faible avant soutien ;   
- herboristerie pauvre avant routes plus sûres ;   
- effet de Nell sauvée ;   
- sacoche de routes exploitée ;   
- contre-registre actif ;   
- route courte sous garde ;   
- léger bonus de crédibilité sur certains stocks quand les routes reviennent vraiment.   

Le troc existe déjà comme système à développer davantage. Le marché noir devra plus tard proposer plus d’achats par échange d’objets, pas seulement par argent.   

## Exploration et aventure   

Le jeu contient un mode aventure/exploration avec :   

- biomes ;   
- routes ;   
- ressources ;   
- traces ;   
- événements ;   
- coffres ;   
- pièges ;   
- lieux dangereux ;   
- rencontres ;   
- indices ;   
- observations ;   
- opportunités de récolte ;   
- progressions de quêtes ;   
- routes scénarisées en histoire ;   
- zones comme Route commerciale, Plaine sauvage, Forêt ancienne et autres lieux d’exploration.   

L’exploration peut servir à :   

- récupérer des matériaux ;   
- trouver des plantes ;   
- faire avancer des quêtes ;   
- découvrir des traces de monstres ;   
- remplir le bestiaire ;   
- obtenir des preuves pour les PNJ ;   
- déclencher des combats ;   
- avancer l’histoire.   

## Quêtes   

Le jeu possède plusieurs familles de quêtes :   

- quêtes de guilde ;   
- quêtes de clients ;   
- quêtes de combat ;   
- quêtes d’exploration ;   
- quêtes de livraison ;   
- quêtes de service ;   
- quêtes de bestiaire ;   
- quêtes de soutien de ville ;   
- quêtes principales d’histoire ;   
- quêtes d’attente pendant des réparations ou mises en place.   

Les quêtes principales du mode histoire sont non refusables. Elles représentent ce qui doit être fait pour continuer l’histoire.   

Certaines quêtes servent à dire au joueur de s’occuper pendant que des réparations, comptages ou préparatifs ont lieu. Le but est de pousser naturellement le personnage à progresser, gagner de l’XP, faire des quêtes secondaires et renforcer son équipement sans casser le lore.   

Certaines quêtes peuvent aussi pointer vers une menace ou un boss obligatoire sans révéler son vrai nom trop tôt. Si une quête parle d’une menace, c’est qu’elle est connue par rumeur, preuve, trace ou registre, mais pas forcément identifiée complètement.   

## Bestiaire et connaissances   

Le bestiaire est évolutif.   

Il peut gérer :   

- connaissances par niveaux ;   
- noms inconnus ou partiels ;   
- monstres découverts progressivement ;   
- boss dont le vrai nom n’est pas donné trop tôt ;   
- habitats ;   
- dangers ;   
- faiblesses ;   
- résistances ;   
- butins ;   
- nombre de morts ou victoires ;   
- preuves de terrain ;   
- déblocage progressif ;   
- PNJ spéciaux à découvrir progressivement.   

Règle importante : un boss ne doit pas forcément être nommé dans le bestiaire dès le premier contact. Son nom peut être ajouté seulement lorsqu’il le dit, lorsqu’un seuil important est atteint, lors d’une phrase avant défaite ou lorsqu’une preuve claire existe.   

## Titres et progression longue   

Le jeu contient un système de titres avec effets surtout narratifs/contextuels.   

Les titres peuvent être liés à :   

- progression ;   
- contrats ;   
- rangs ;   
- ville ;   
- guilde ;   
- exploration ;   
- biomes ;   
- bestiaire ;   
- matériaux ;   
- inventaire ;   
- équipement ;   
- boss ;   
- survie ;   
- actions particulières ;   
- titres cachés ou scellés.   

Certains titres sont visibles, d’autres restent secrets jusqu’à obtention.   

## Personnages spéciaux jouables   

Le jeu contient des personnages spéciaux avec noms protégés.   

Règle générale :   

- entrer un nom spécial déclenche une vérification ;   
- une date spéciale peut être demandée ;   
- si la date est correcte, la race est verrouillée selon le personnage ;   
- la classe reste généralement choisissable ;   
- une classe native peut donner un bonus ;   
- certains personnages ne sont pas incarnables normalement.   

Personnages spéciaux documentés :   

- Hazak : assassin elfe noir, sombre, efficace, direct, proche d’Henrique, protecteur d’Hestia ;   
- Skuro : tank violent à épée lourde, forme sombre liée à Sanctus ;   
- Sanctus : tank/semi-mage protecteur, lumière, provocation et posture défensive ;   
- Kanadé : mage semi-dragonne, magie zodiacale, caractère explosif ;   
- Aoi : kitsune, mage flame/invocatrice avec katana, timide et protectrice de ses incantations ;   
- Fail : mage fou, fée, expérimentateur instable ;   
- Hestia : ancienne divinité amnésique, humaine actuelle, peureuse, magie très forte et dôme protecteur ;   
- Louis : artificier précis, projectiles multiples, naïf, cherche des amis ;   
- Mattzelda : colosse humain, blagueur ;   
- Trexof : assassin humain, calme, instinctif, capable de lire les failles sans vocabulaire méta ;   
- Henrique : chevalier frontal, peut revenir une fois à la vie ;   
- FireFlight : non incarnable, boss/test final/créateur ;   
- Matt (PRO) : non jouable normalement, adversaire spécial très fort et respectueux.   

Les personnages spéciaux peuvent avoir :   

- dialogues personnalisés ;   
- réactions de groupe ;   
- synergies ;   
- effets en combat ;   
- bonus natifs ;   
- restrictions ;   
- réactions de lore ;   
- prologue personnalisé en histoire ;   
- réactions au chapitre 2 selon identité.   

## Groupes spéciaux   

Certains groupes ou relations sont déjà prévus :   

- Hazak / Henrique ;   
- Mattzelda / Louis / Trexof ;   
- Aoi / Kanadé / Sanctus ;   
- Hazak / Hestia ;   
- Fail / Hazak ;   
- Skuro avec personnages compatibles ;   
- FireFlight / Matt (PRO).   

Effets ou thèmes possibles :   

- Hazak protège davantage Hestia ;   
- Hazak et Henrique ont une coordination offensive ;   
- Aoi et Kanadé se renforcent, Sanctus stabilise ;   
- Mattzelda, Louis et Trexof ont une dynamique de trio ;   
- Fail profite de son contrat de non-agression avec Hazak ;   
- certains groupes peuvent rendre un combat beaucoup plus dangereux.   

## Cheats, altérations et secrets   

Les cheats existent, mais restent cachés tant que le personnage n’est pas marqué **Altéré**.   

Règles :   

- le premier cheat accepté marque le personnage comme Altéré ;   
- un personnage Altéré reste définitivement marqué, même si les altérations sont désactivées ;   
- les cheats peuvent être togglés selon les cas ;   
- les cheats peuvent être refusés ou punis en mort définitive ;   
- certains boss ou entités peuvent réagir aux cheats ;   
- le menu de cheats ne doit pas être visible au départ ;   
- les codes peuvent être saisis discrètement dans des menus hors combat.   

Codes connus :   

- `idontwanttodie` : bloque la perte de PV ;   
- `infinituseforeverything` : empêche les consommables de diminuer ;   
- `bedrockequipment` : rend l’équipement indestructible ;   
- `myweaponandnotother` : empêche perte et vol d’équipement ;   
- `skipallstory` : débloque certaines connaissances de récit/bestiaire ;   
- `skipalllevels` : donne le niveau maximum ;   
- `1302313` : message personnel du créateur ;   
- `13023131302313` : séquence divine de combat ;   
- `iamnotgoblinfood` : effet fun lié aux gobelins ;   
- `letmecook` : bonus temporaire de chance critique ;   
- `whereismyplotarmor` : bénédiction temporaire anti-mort ;   
- `talktothedev` : note secrète du créateur ;   
- `oopsallbosses` : chaos orienté boss/vagues spéciales ;   
- `randomizemyfate` : classe, arme et inventaire réattribués ;   
- `resetmycharacter` : réinitialisation/destruction-recréation du personnage courant ;   
- `switchmyclassandweapon` : change la classe et donne un équipement cohérent ;   
- `helpmerefundmyaction` : annule une action ou vente accidentelle, limité ;   
- `givemesomegolds` : donne de l’or ;   
- `givemealevel` : donne un niveau ;   
- `fuckgrindinggimifight` : ouvre les combats spéciaux en arène et révèle les boss non finaux.   

## Boss et grandes menaces   

Le jeu possède un catalogue de boss avec niveaux recommandés, affichage progressif et rôles.   

Boss et menaces documentés :   

- Fitoria ;   
- Zelef ;   
- Atlas ;   
- Écho de Lyknir ;   
- Grinka ;   
- Avatar d’Azelanos ;   
- Fragment de Thamarys ;   
- Mojo ;   
- Reflet d’Inakari ;   
- Le Jugement Silencieux ;   
- Émanation de l’Anomalie ;   
- L’Horloge des Chuchotements ;   
- L’Enfant des Os Enfouis ;   
- Avatar de Boros ;   
- Avatar d’Anastasia ;   
- Avatar de Lexior ;   
- Fragment de Luna / Onyrae ;   
- Fragment de Syvaranelya ;   
- Écho d’Hitogami ;   
- Écho de Sérendys ;   
- Le Gardien du Seuil ;   
- Le Roi Sans Salle ;   
- La Bête qui n’avait pas de nom ;   
- Aldebaroth ;   
- Asterion et Nihilon ;   
- Écho fragmenté d’Obérion ;   
- Avatar affaibli de FireFlight ;   
- Le Souffle sans Visage ;   
- La Marionnette aux Mille Clous ;   
- Manifestation de Moiran ;   
- Le Cerf des Âmes Égarées ;   
- Gorvald ;   
- Serana ;   
- Draiite ;   
- Les Jumelles du Miroir Fendu ;   
- Source stable de l’Anomalie.   

Certains boss ont des malédictions ou effets persistants :   

- sang noir ;   
- corrosion ;   
- marque de proie ;   
- couronne obscure ;   
- reflet mensonger ;   
- désynchronisation ;   
- éclipse onirique ;   
- rancune abyssale ;   
- reflet fendu ;   
- réactions aux cheats ;   
- réactions à la mort définitive.   

FireFlight est prévu comme vrai boss final. Son accès doit être verrouillé par des invitations/preuves obtenues après les boss importants. Le combat final doit mélanger chaos, ultimes de boss, interface perturbée et affrontement avec les personnages du compte.   

## Mode coop / groupe   

Le jeu possède une logique coop/groupe à développer et déjà documentée.   

Règles prévues ou partiellement présentes :   

- le joueur principal définit la session, le voyage, les boss, monstres et événements ;   
- les autres joueurs participent surtout en combat ;   
- récompenses individuelles selon participation ;   
- progression possible du déblocage de boss si le joueur participe réellement ;   
- blocage d’un boss si tous les joueurs ne l’ont pas débloqué ;   
- potions utilisables sur alliés ;   
- mort et récompenses gérées individuellement ;   
- règles spéciales en Léthal de groupe.   

## Interface graphique expérimentale   

L’interface graphique existe comme chantier expérimental.   

Elle repose sur :   

- moteur C++ comme source de vérité ;   
- snapshots JSON ;   
- interface HTML/JS ;   
- serveur local ;   
- terminal gardé comme secours ;   
- contrats de schéma JSON ;   
- affichage des menus, choix, combats, inventaires, quêtes et boutiques.   

Priorité actuelle : rendre l’IG jouable et stable avant de gros ajouts visuels.   

## Installation, launchers et publication   

Le projet contient déjà des éléments pour publier proprement :   

- scripts de packaging ;   
- scripts de release ;   
- manifeste de release ;   
- guides GitHub Release ;   
- launchers/installateurs Windows ;   
- launchers/installateurs Linux ;   
- contrôle de ZIP propre ;   
- validation anti-build/output/exécutables dans l’archive source ;   
- commande `--version` ;   
- commande `--help` ;   
- commande `--encoding-check`.   

Le ZIP source ne doit pas contenir :   

- `build/` ;   
- `output/` ;   
- exécutables ;   
- fichiers de reprise locaux ;   
- caches ;   
- données privées de sauvegarde ;   
- dossiers de debug inutiles.   

---   

# Mode histoire — état actuel détaillé   

## Principe général   

Le mode histoire est séparé du bac à sable.   

Il commence limité : peu de routes, peu de boutiques, peu de stocks, peu de libertés. Le but est de faire sentir que la ville est fragile et que le personnage doit reconstruire l’accès au monde petit à petit.   

À la fin de l’histoire, le personnage réel doit pouvoir basculer vers le bac à sable.   

Si un personnage de bac à sable commence l’histoire, le jeu doit demander confirmation, car cela peut remettre le personnage dans un départ commun.   

Depuis l’histoire, le joueur peut utiliser un clone éphémère de bac à sable, non sauvegardé, pour tester librement sans casser la progression réelle.   

## Prologue   

Le prologue commence avant la fumée blanche.   

Structure actuelle :   

- mission ordinaire en groupe ;   
- apparition de la fumée blanche ;   
- séparation ;   
- perte des souvenirs ;   
- le prénom reste ;   
- arrivée seul près des portes ;   
- réactions différentes selon personnage spécial ;   
- premiers choix dans la brume ;   
- rencontre avec une créature ;   
- récupération d’un kit de départ ;   
- arrivée à la ville ;   
- première réaction de Mira.   

Les personnages spéciaux reçoivent des textes personnalisés, surtout dans le prologue.   

## Chapitre 1 — La ville respire encore   

Le chapitre 1 sert à installer la ville, les PNJ principaux et les premières quêtes utiles.   

PNJ centraux :   

- Mira — intendante de quartier / référente histoire ;   
- Orren — vieux garde / référent de route ;   
- Lysa — soigneuse de fortune ;   
- Bram — forgeron fatigué ;   
- Soryn — archiviste.   

Logique du chapitre :   

1. faire connaissance avec Mira ;   
2. Mira présente la ville, les noms et les professions ;   
3. Mira donne la première quête principale : faire le tour de la ville ;   
4. le joueur rencontre Orren, Lysa, Bram et Soryn ;   
5. le joueur notifie Mira ;   
6. Mira donne sa demande principale et demande de retourner voir les autres de sa part ;   
7. chaque PNJ ajoute sa quête principale ;   
8. le joueur termine les demandes ;   
9. le joueur revient notifier Mira ;   
10. le chapitre peut ouvrir la suite vers le relais silencieux.   

Demandes principales du chapitre 1 :   

- **Faire le tour de la ville** : rencontrer les référents et comprendre à quoi ils servent ;   
- **Faire respirer les murs** : repousser les menaces proches des murs ;   
- **Les bornes qui mentent** : explorer les repères de route avec Orren ;   
- **Les blessés de la nuit** : rapporter des Feuilles amères de soin à Lysa ;   
- **Les plaques qui tiennent encore** : aider Bram à trier/renforcer ce qui peut tenir ;   
- **Une rumeur à clouer au sol** : donner à Soryn une preuve de terrain au lieu d’une panique.   

Les Feuilles amères de soin peuvent être obtenues via :   

- herboriste ;   
- stocks de plantes ;   
- services/événements de ville ;   
- exploration végétale ;   
- zones comme la Forêt ancienne.   

## Chapitre 2 — Le relais silencieux   

Le chapitre 2 est jouable et avancé jusqu’à la route courte sous garde.   

PNJ importants du chapitre :   

- Mira ;   
- Orren ;   
- Lysa ;   
- Bram ;   
- Soryn ;   
- Nell la messagère ;   
- Eda la comptable des routes courtes.   

Étapes principales actuelles :   

1. **Le nom du relais silencieux** : briefing Mira/Orren ;   
2. **La route qui s’allonge** : reconnaissance de la Route commerciale ;   
3. **La borne retournée** : preuve de terrain pour Soryn ;   
4. **Les guetteurs sans feu** : première menace imposée du relais ;   
5. **Le relais doit répondre** : remettre un signal minimal ;   
6. **La voix derrière les caisses** : suivre le signal et sauver Nell ;   
7. **La sacoche qui parle** : exploiter les informations de Nell ;   
8. **Les comptoirs rouvrent un œil** : transformer les infos en conséquences de ville ;   
9. **L’encre froide de la route** : suivre une trace anormale ;   
10. **La carte qui se réécrit** : prouver que la route se corrige après le passage des vivants ;   
11. **Le contre-registre des routes courtes** : suivre les stocks réels plutôt que les cartes corrompues ;   
12. **Le nœud noir au bout du relais** : repérer une convergence inquiétante ;   
13. **Tenir pendant les travaux** : pousser le joueur à s’occuper pendant les réparations ;   
14. **La chose qui garde la borne** : annoncer une menace sans donner son vrai nom ;   
15. **Le verrou de la borne** : crise/boss d’étape de la borne noire ;   
16. **Les cicatrices du verrou** : lire les traces laissées par le verrou ;   
17. **Une route à garder ouverte** : organiser les premiers retours surveillés.   

État narratif actuel : la route courte tient pour les premiers retours, mais le vrai nom derrière la borne noire reste hors du registre.   

---   

# Historique versionné — à partir de la V3 uniquement   

## V3.00.00 — Socle histoire publiable   

- Passage au premier vrai jalon histoire.   
- Ajout d’une boucle histoire structurée avec prologue, chapitre 1 jouable et chapitre 2 lancé.   
- Ajout du relais silencieux comme objectif majeur.   
- Ajout des guetteurs sans feu comme première menace imposée du chapitre 2.   
- Ajout d’états de ville liés aux boutiques, à la forge, à l’herboristerie et aux routes.   
- Définition de **V3.00.00** comme base importante / recréation conseillée.   

## V3.01.00 — Personnages spéciaux dans l’histoire   

- Détection des personnages spéciaux jouables dans le mode histoire.   
- Personnalisation de l’intro, du prologue, de la perte de mémoire et de l’arrivée en ville.   
- Ajout de réactions de Mira selon l’identité spéciale.   
- Couverture explicite des personnages spéciaux principaux : Hazak, Mattzelda, Aoi, Kanadé, Fail, Trexof, Skuro, Sanctus, Hestia, Louis, Henrique.   
- Fallback propre pour futurs personnages spéciaux.   

## V3.02.00 — Réactions spéciales renforcées   

- Ajout de réactions personnalisées supplémentaires dans la brume.   
- Ajout de réactions spécifiques face à la créature du prologue.   
- Ajout de réactions au kit récupéré.   
- Ajout de nuances d’identité dans le chapitre 2.   
- Ajout de la quête principale **Le relais doit répondre**.   
- Création d’un rapport séparé des personnalisations d’histoire.   

## V3.03.00 — Nell et premier sauvetage de route   

- Ajout du signal des trois coups venus du nord.   
- Ajout de la quête principale **La voix derrière les caisses**.   
- Ajout de Nell la messagère comme première survivante de route.   
- Ajout de Nell dans les PNJ notables.   
- La route commence à ramener une personne vivante, pas seulement une preuve.   

## V3.04.00 — Sacoche, comptoirs et encre froide   

- Ajout de **La sacoche qui parle**.   
- Ajout de **Les comptoirs rouvrent un œil**.   
- Ajout de **L’encre froide de la route**.   
- Ajout de conséquences de ville liées à Nell, sa sacoche et les informations de route.   
- Ajout d’éléments scénarisés sur la Route commerciale : halte rayée, boucle du pont court, trace d’encre froide.   
- Début d’effets plus visibles sur boutiques, stocks et comptoirs selon progression.   

## V3.05.00 — Route réécrite et contre-registre   

- Ajout de **La carte qui se réécrit**.   
- Ajout de **Le contre-registre des routes courtes**.   
- Ajout de **Le nœud noir au bout du relais**.   
- Ajout d’Eda, comptable des routes courtes.   
- Preuve que la route est réécrite après le passage des vivants.   
- Mise en place du contre-registre pour suivre les stocks réellement revenus.   
- Préparation de la borne noire comme prochaine crise.   

## V3.06.00 — Attente utile et piste de menace   

- Ajout de **Tenir pendant les travaux**.   
- Ajout d’une logique d’attente/réparations pour pousser le joueur à faire des activités utiles.   
- Ajout de progressions via patrouille, service, quête secondaire ou activité de ville.   
- Eda devient un vrai PNJ notable.   
- Ajout de **La chose qui garde la borne**.   
- La menace de la borne est annoncée sans révéler son vrai nom trop tôt.   

## V3.07.00 — Verrou de borne et route gardée   

- Transformation de la menace de la borne noire en vraie crise jouable.   
- Ajout de **Le verrou de la borne**.   
- Ajout de **Les cicatrices du verrou**.   
- Ajout de **Une route à garder ouverte**.   
- Le chapitre 2 avance jusqu’à l’état **route courte sous garde**.   
- Ajout de nouvelles réactions ville/PNJ autour des cicatrices, retours surveillés et route gardée.   
- Correction du ton de Trexof : retrait des formulations trop méta/testeur au profit d’un ton naturel et lore.   

## V3.07.01 — Patch publication Git   

- Patch de stabilité et de cohérence avant publication.   
- Nettoyage de formulations trop méta dans le chapitre 2.   
- Remplacement de termes visibles comme “piste boss” par des formulations plus lore : piste de menace, registre, preuves, rumeurs, menace non nommée.   
- Ajout d’un état clair : la boucle actuelle du chapitre 2 est complète jusqu’à la route courte sous garde.   
- Préparation d’un fichier séparé pour écrire les chapitres 1 à 5 avant de coder la suite.   

## V3.07.02 — Documentation propre et patchnote séparé   

- Nettoyage des README, guides `.md` et fichiers `.txt` pour retirer les historiques de versions hors patchnote.   
- Restauration de la section **Installer depuis une release GitHub**.   
- Restauration de la **Note spéciale du développeur** en fin de README.   
- Création/organisation de `PATCHNOTE_DINOTOFU.md` comme seul endroit autorisé pour les notes de version.   
- Conservation de la base importante de recréation : **V3.00.00**.   


## V3.07.03 — Réparation des launchers si l’exécutable manque   

- Correction du launcher Windows : si `version.txt` annonce la bonne version mais qu’aucun `Dinotofu.exe` ou `DinotofuGUI.exe` n’est présent, le launcher force une réparation depuis la release GitHub.   
- Correction équivalente côté Linux : si la version locale est à jour mais que l’exécutable manque, le launcher relance l’installation/réparation.   
- Ajout d’un message plus clair si la release GitHub ne contient pas l’asset Windows attendu : `Dinotofu-Windows-v*.zip`.   
- Objectif : éviter les installations hybrides où le dossier annonce une version récente sans contenir de jeu lançable.   

## V3.07.04 — Releases GitHub générées au push   

- Correction du workflow GitHub Actions pour correspondre au vrai fonctionnement du projet : pousser le source doit permettre de générer les releases jouables automatiquement.   
- Le workflow lit la version du projet, crée le tag si nécessaire, compile Linux et Windows, puis publie les ZIP jouables dans la release GitHub.   
- Si une release existe déjà mais qu’il manque les ZIP jouables Windows/Linux, elle est considérée comme incomplète et les assets sont régénérés.   
- Les assets attendus sont vérifiés explicitement : `Dinotofu-Windows-v*.zip`, `DinotofuInstaller-Windows-v*.zip`, `Dinotofu-Linux-v*.zip` et `DinotofuInstaller-Linux-v*.zip`.   
- Objectif : que le ZIP source reste propre, sans exécutable, pendant que GitHub Actions fabrique les versions Windows/Linux destinées aux joueurs.   

## V3.07.05 — Installateur plus clair si GitHub est inaccessible   

- Correction de l’installateur Windows : si `github.com` ne peut pas être résolu, le script ne doit plus finir sur une exception PowerShell incompréhensible.   
- Ajout d’un message clair demandant de vérifier la connexion Internet, le DNS, le proxy ou le pare-feu, puis de relancer l’installateur.   
- Ajout d’un conseil explicite : tester l’ouverture de `https://github.com` dans le navigateur.   
- Ajout d’un secours pratique : si GitHub est bloqué mais que le joueur possède déjà un ZIP `Dinotofu-Windows-vX.YY.ZZ.zip`, il peut le placer à côté de l’installateur pour l’utiliser localement.   
- Amélioration du launcher Windows : si la vérification GitHub échoue et qu’aucun exécutable local n’est présent, il explique que la réparation ne pourra pas se faire tant que GitHub reste inaccessible.   
- Le patchnote complet reste inclus dans le ZIP source et dans les futures releases jouables.   

## V3.07.06 — Menus plus lisibles et catégories claires   

- Regroupement du menu principal par grandes catégories : Histoire, Terrain, Ville, Gestion et Aide.   
- Combat et Exploration sont de nouveau visibles dans une catégorie claire : **Terrain : Combats / Exploration**.   
- Ajout d’un rappel explicite : les quêtes d’exploration passent par **Exploration**, pas forcément par les lieux visitables.   
- Réorganisation du menu Histoire : **Continuer** est maintenant l’option principale en haut de page, avec les informations longues regroupées dans des sous-menus.   
- Réorganisation du choix de race : la création commence par une catégorie, puis affiche seulement les races de cette catégorie.   
- Les semi-humains et leurs sous-types sont rangés ensemble dans la catégorie **Semi-humains**.   
- Le catalogue des races affiche aussi des groupes courts au lieu d’une très longue page détaillée.   
- Objectif : réduire les pages trop chargées et éviter que les choix importants soient noyés dans le texte.   

## V3.07.07 — Retour de test terminal : lisibilité, rythme et équilibrage   

- Combat terminal mieux découpé : tour joueur, résolution, invocations alliées, ennemis vivants et état après riposte sont séparés par des validations.   
- Thème terminal normal forcé en fond noir / texte blanc au lancement ; le combat garde un fond très sombre légèrement rougeâtre.   
- Menu Quêtes clarifié avec une section visible **Quêtes à rendre / terminées**.   
- Infos utiles transformées en vrai menu court : où aller, journées/temps, argent/économie, combat/exploration, quêtes, ville/PNJ/lieux.   
- Documents et renseignements lus affichent maintenant **[lu]** dans l'inventaire quand leur information est déjà connue.   
- Les documents de bestiaire déclenchent mieux les entrées correspondantes, y compris les manuels de récolte propre et de dissection de monstre.   
- Loot de bêtes corrigé : les **Crocs de loup** tombent seulement sur des créatures de type loup/meute ; les autres bêtes donnent plutôt de la peau de bête.   
- Apparitions d'élites/évolués calmées en tout début de partie, pour éviter les pics absurdes type niveau 10 dès le niveau 1.   
- Élites, évolués et premiers boss renforcés pour mieux correspondre à leur statut quand ils apparaissent vraiment.   

## V3.07.08 — Loot de bêtes plus cohérent   

- Correction du loot des bêtes sans supprimer la logique des crocs : les loups et créatures explicitement lupines peuvent toujours donner des **Crocs de loup**.   
- Ajout de matériaux plus cohérents selon la famille de bête : **Croc de prédateur**, **Défense de sanglier** et **Dent de rongeur**.   
- Les chiens, renards, chacals, ours ou autres bêtes carnassières non lupines ne donnent plus des crocs de loup par défaut.   
- Les rats et rongeurs donnent plutôt des dents de rongeur ; les sangliers donnent plutôt des défenses.   
- Les boutiques de matériaux de monstres peuvent afficher ponctuellement ces nouveaux composants.   
- Le patchnote reste inclus dans le ZIP source et ne doit pas être envoyé séparément à chaque passe.   

---   

# Prochaine phase proposée   

Pour la suite, l’objectif n’est plus de coder l’histoire au fil de l’eau.   

Méthode proposée :   

1. écrire la trame complète jusqu’au chapitre 5 ;   
2. définir les quêtes principales ;   
3. définir les quêtes d’attente/réparation ;   
4. définir les PNJ importants ;   
5. définir les boss obligatoires de quête ;   
6. décider quels noms de boss sont cachés, partiels ou connus ;   
7. seulement ensuite coder proprement les chapitres.   

Cela devrait rendre les conversations plus rapides, éviter de reconstruire le ZIP à chaque idée, et garder l’histoire plus cohérente.   
