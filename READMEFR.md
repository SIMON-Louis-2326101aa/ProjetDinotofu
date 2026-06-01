# Dinotofu   

Dinotofu est un RPG / jeu d'arène en C++17. La version terminal reste le socle stable, et l'interface graphique expérimentale HTML/JS est maintenant considérée comme assez complète pour porter le jeu vers le palier V2.   

## Version actuelle   

- Version actuelle : **V2.04.06**   
- Dernière version de personnage encore acceptable avant recommandation forte de recréation : **V2.01.03**   
- Personnages antérieurs à **V2.01.03** : recréation conseillée, car le socle IG/sauvegarde/bestiaire/légendes a trop changé depuis les anciens paliers.   
- Passage V1 vers V2 ou sauvegarde V2 trop ancienne : recommandation forte de recréation, car le jeu a trop changé avant le socle V2.01.03.   
- Socle de sauvegarde important : **V2.01.03**, car le registre des légendes et une partie du bestiaire lore ont été restructurés.   

## Ce qu'il y a déjà dans le jeu   

Le jeu permet déjà de :   

- créer, charger et sauvegarder des comptes/personnages ;   
- choisir race, classe, difficulté et certaines identités spéciales protégées ;   
- combattre en PvP IA, PvP deux joueurs, PvE monstres, boss solo et boss coop ;   
- gérer inventaire, armes, armures, consommables, matériaux, durabilité, réparation et qualités ;   
- acheter, vendre, troquer, visiter des lieux, consulter la guilde et la bibliothèque ;   
- explorer des biomes avec ressources, coffres, pièges, traces, événements, lieux dangereux et rencontres ;   
- suivre des quêtes de guilde, de clients, d'exploration, de combat, de livraison et de bestiaire ;   
- consulter un bestiaire évolutif avec niveaux de connaissance, dangers, habitats, faiblesses/résistances connues et butin ;   
- utiliser des systèmes cachés comme les codes, les données altérées, les personnages spéciaux et certains événements de lore ;   
- préparer les futures compétences durables liées aux classes, races, armes et habitudes de jeu.   

Le mode Histoire est visible mais volontairement en attente. Il deviendra une vraie grande phase plus tard.   

## Interface graphique expérimentale   

L'IG actuelle sert à rendre le jeu jouable depuis une page locale tout en gardant le terminal comme secours. Elle lit les snapshots JSON générés par le jeu, affiche menus/combat/cartes, puis renvoie les choix au terminal via une file de commandes.   

État du palier V2 :   

- la V2.04.06 met volontairement à jour la cible de version et améliore la navigation paginée commune, les longues listes potions/boutique/bestiaire et les snapshots de dialogues séparés des actions de combat ;   
- la V2.04.05 améliore les écrans de récompense structurés, les métadonnées de navigation paginée et les pistes visibles de compétences/progression ;   
- la V2.01.34 rectifie les métadonnées de version et améliore les snapshots IG des routes PvP IA, résultats de fuite/rôle, notices de listes chargées et transitions difficulté/session ;   
- les menus principaux, activités, sessions, création, sauvegardes, inventaire, équipement, potions, boutiques, quêtes, exploration, progression, catalogues, combats et boss exposent des snapshots IG ;   
- les longues listes d’équipement, d’inventaire, de potions, de stock, de revente et de rachat utilisent une pagination claire avec retour `0` et navigation `98/99` quand nécessaire ;   
- les cartes cliquables gardent le numéro visible pour conserver le lien avec le terminal ;   
- les actions JSON exposent leurs métadonnées métier : type, statut, prix, quantité, stock, progression, propriétaire, récompense ou détail ;   
- les choix contextuels encore génériques d'exploration, coffres, pièges, PNJ, contrats et lieux sont maintenant convertis en cartes IG plus sûres au lieu de simples lignes sans contexte ;   
- les dialogues d’introduction des ennemis intelligents exposent interlocuteur, langue probable et compréhension, ce qui les rend plus lisibles côté IG ;   
- deux nouvelles légendes de salle sont ajoutées au registre : Azelanos / couronne fissurée et Anomalie / menu qui ment ;   
- les choix structurés du registre des légendes affichent maintenant un court détail IG, leur section, leur source et leur progression quand c'est utile ;   
- la bibliothèque peut maintenant vendre un carnet de conteurs itinérants, ouvrant deux nouvelles rumeurs de registre sur les déclencheurs de lore par bibliothèque et PNJ ;   
- les choix de cibles en combat exposent maintenant des cartes IG avec PV, race/type et statut de menace quand l'écran le permet ;   
- les actions principales de combat et les styles d'attaque exposent maintenant des cartes IG dédiées : attaque, soin rapide, potions, équipement, défense, attente et fuite ;   
- plusieurs anciens affichages texte du tour joueur, comme statistiques, équipement et observation, sont capturés puis réaffichés via des écrans IG de lecture au lieu de rester seulement dans le terminal ;   
- les longues sélections de boss solo/coop sont paginées, avec cartes boss structurées et navigation `98/99` ;   
- la sélection des personnages spéciaux en PvP IA est paginée et expose des cartes IG avec race, classe native, style et statut de défi ;   
- le bestiaire possède maintenant un hub dédié aux personnages spéciaux avec fiches découvertes, suivi verrouillé/déverrouillé et accès aux rumeurs de groupe ;   
- les adversaires spéciaux rencontrés en PvP IA alimentent le bestiaire progressif au lieu de rester invisibles hors combat ;   
- les choix de classes en PvP local J2 et en PvP IA manuel sont maintenant paginés et exposent des cartes IG avec rôle, catégorie, potions et statistiques de base ;   
- les résultats de combat, boss coop et récompenses exposent maintenant des lignes plus structurées pour produire de meilleures cartes IG ;   
- les sorties PvE/exploration, bilans PvP IA/local, conséquences Grinka/Zelef et groupes spéciaux exposent maintenant plus de cartes de bilan IG ;   
- les recommandations IG évitent encore plus strictement les actions liées à une faiblesse/résistance non connue ;   
- le soutien boss coop et le soutien PvE coop exposent maintenant des cartes IG pour choisir l’action de soin, la cible alliée et la potion, avec pagination `98/99` si les listes deviennent longues ;   
- les ordres d’invocation manuels exposent maintenant des cartes IG plus propres : mode automatique/manuel, actions, cible, durée, lien restant et sacrifice possible ;   
- les sélections de cibles des invocations et des groupes de combat sont paginées en `98/99`, ce qui évite les listes trop longues quand plusieurs unités/invocations sont présentes ;   
- l'inspection d'une cible déjà décryptée passe maintenant par un écran IG lisible au lieu d'un affichage brut terminal ;   
- les écrans de cible sélectionnée affichent mieux le joueur, la cible, la priorité de menace et le résumé de vague en pied d'écran ;   
- les lieux visitables et les PNJ notables utilisent maintenant aussi la pagination `98/99`, afin que les futurs contacts recommandés ne cassent pas l'affichage ;   
- la bibliothèque peut déclencher rarement deux nouveaux fragments de lore doux après achat du carnet de conteurs, sans donner de faiblesse gratuite ;   
- les cartes d’action affichent mieux leur type d’action, et la navigation/pagination est séparée des actions principales pour réduire les mauvais clics ;   
- les échanges/dons paginent aussi les comptes et personnages cibles, et les armes, armures, consommables et matériaux transférables exposent des cartes IG avec valeur, statut et propriétaire ;   
- les écrans de butin, récompenses et récupérations post-combat sont mieux classés côté IG, avec cartes dédiées pour fragments, matériaux et conséquences ;   
- le panneau de guilde utilise maintenant une pagination `98/99` pour éviter les longues listes compactes et les mauvais choix sur les contrats ;   
- la saisie libre, les confirmations, les quantités, les codes cachés et les choix numériques sont décrits dans le contrat IG et dans des cartes de saisie dédiées ;   
- plusieurs écrans transitoires de récompense, mort, boss, craft, échanges et données altérées sont marqués en lecture seule pour éviter les faux clics ;   
- le navigateur et le serveur local refusent les commandes envoyées depuis un ancien écran ou une file déjà occupée ;   
- pour le moment, la version terminal est la version la plus stable ; l'IG reste expérimentale et doit encore être stabilisée écran par écran ;   
- le terminal reste disponible comme affichage de secours, sans être supprimé ;   

## Commandes utiles   

Compiler :   

```bash   
make   
```   

Lancer :   

```bash   
make run   
# ou   
make launch   
```   

Afficher uniquement la version sans lancer le jeu interactif :   

```bash   
./output/Dinotofu --version   
# ou   
./output/Dinotofu -v   
```   

Vérifier rapidement les accents/UTF-8 dans le terminal :   

```bash   
./output/Dinotofu --encoding-check   
```   

Nettoyer :   

```bash   
make clean   
```   

Reconstruire :   

```bash   
make rebuild   
```   

Lancer l'aperçu IG local :   

```bash   
make gui-preview   
```   

## Installer depuis une release GitHub   

Pour installer Dinotofu sans compiler le projet manuellement :   

1. aller sur la page du dépôt GitHub ;   
2. ouvrir la dernière **Release** affichée à droite du dépôt ;   
3. télécharger le launcher correspondant à ton système d'exploitation : Windows ou Linux ;   
4. lancer ce launcher, qui s'occupe ensuite d'ouvrir la bonne version du jeu.   

Sur les versions desktop Windows/Linux, l'installateur ou le launcher doit créer deux entrées claires :   

- **ProjetDinotofu Launcher** : lancement normal / Auto, via le launcher adapté à l'OS ;   
- **ProjetDinotofu Launcher Terminal version** : lancement forcé en terminal, via le launcher Windows/Linux correspondant.   

## Structure rapide   

- `src/` : code C++ du jeu ;   
- `include/` : headers C++ ;   
- `assets/config/` : données JSON de référence ;   
- `tools/gui/` : interface graphique expérimentale et serveur local ;   
- `tools/windows/` et `tools/linux/` : launchers/installateurs ;   
- `scripts/` : packaging, version, release et contrôles ;   
- `release/` : manifeste et données de publication ;   
- `assets/saves/` : dossiers de sauvegarde locaux, sans données privées dans les ZIP.   

## Grandes directions seulement   

1. Stabiliser encore les derniers chemins IG de combat et d'invocation, surtout les cas rares avec beaucoup d'unités ou de retours à la liste.   
2. Continuer à structurer les menus complexes : quêtes longues, groupes spéciaux, événements rares, récompenses et conséquences.   
3. Renforcer l'équilibrage global : classes, monstres, boss, XP/or, loot, récompenses, prix et difficultés.   
4. Ajouter ensuite contenu, dialogues/lore vivants, compétences et polish progressif.   

## Note spéciale du développeur   



Salut à toi, futur aventurier, futur héros, futur cadavre statistiquement probable.   

Bienvenue dans Dinotofu. Ici, on peut gagner avec une vraie stratégie, mourir parce qu'on a voulu faire le malin, ou découvrir qu'un gobelin fiscalement motivé est parfois plus dangereux qu'un boss mythique. Le jeu est encore en construction, mais il existe déjà pour être joué, cassé, critiqué, trollé, amélioré et retesté.   

J'ai envie que Dinotofu reste libre : tu peux optimiser, tenter des trucs stupides, mourir comme une légende ou survivre alors que tu ne méritais clairement pas. Si une mécanique te frustre, si un boss te semble injuste, si une stratégie détruit l'équilibrage, ou si une idée te donne envie de continuer l'aventure, dis-le. Le jeu doit grandir avec les retours des joueurs, pas seulement avec mes idées dans mon coin.   

Défi numéro un : survivre assez longtemps pour comprendre ce qu'il se passe.   

Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais au moins il le fait avec style.   

Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.   

Jouez sérieusement, mais pas trop sérieusement. Plaignez-vous si besoin, proposez des idées, insultez poliment les boss, et surtout dites-moi ce qui vous donne envie de relancer une partie. Si vous trouvez un combo trop fort, je nierai sûrement avoir paniqué en voyant les chiffres, puis je dirai que c'était un test d'équilibrage avancé et que tout était prévu d'avance. C'est faux, mais c'est plus professionnel.   

Si vous arrivez à devenir une légende dans Dinotofu, félicitations. Si vous mourez contre les premiers ennemis, félicitations aussi, mais différemment. Si vous perdez contre un coffre, un piège ou votre propre confiance en vous, sachez que le jeu ne vous juge pas. Moi un peu, mais avec affection.   
