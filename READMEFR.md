# Dinotofu   
   
Dinotofu est un RPG / jeu d'arène en C++17. La version terminal reste le socle stable, et l'interface graphique expérimentale HTML/JS est maintenant considérée comme assez complète pour porter le jeu vers le palier V2.   
   
## Version actuelle   
   
- Version actuelle : **V2.0.08**   
- Dernière version de personnage encore acceptable avant recommandation forte de recréation : **V1.32.03**   
- Personnages beaucoup plus anciens que **V1.32.03** : recréation conseillée, car trop de systèmes ont changé.   
- Passage V1 récent vers V2 : adaptation moyenne seulement, car le palier V2 valide surtout l'IG et ne doit pas forcer les sauvegardes récentes à repartir de zéro.   
   
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
   
- les menus principaux, activités, sessions, création, sauvegardes, inventaire, équipement, potions, boutiques, quêtes, exploration, progression, catalogues, combats et boss exposent des snapshots IG ;   
- les cartes cliquables gardent le numéro visible pour conserver le lien avec le terminal ;   
- les actions JSON exposent leurs métadonnées métier : type, statut, prix, quantité, progression, propriétaire, récompense ou détail ;   
- la saisie libre, les confirmations, les quantités, les codes cachés et les choix numériques sont décrits dans le contrat IG ;   
- le navigateur et le serveur local refusent les commandes envoyées depuis un ancien écran ou une file déjà occupée ;   
- le terminal reste disponible comme affichage de secours, sans être supprimé.   
   
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
   
1. Stabiliser les retours joueurs sur le palier **V2.0.08**.   
2. Ajouter ensuite contenu, dialogues/lore vivants, compétences et polish progressif.   
3. Passer en **V3.0.0** quand le premier chapitre d'histoire est codé.   
4. Garder le multijoueur en ligne pour une grosse version future beaucoup plus tard.   
   
## Note spéciale du développeur   
   
   
   
Salut à toi, futur aventurier, futur héros, futur cadavre statistiquement probable.   
   
Bienvenue dans Dinotofu. Ici, on peut gagner avec une vraie stratégie, mourir parce qu'on a voulu faire le malin, ou découvrir qu'un gobelin fiscalement motivé est parfois plus dangereux qu'un boss mythique. Le jeu est encore en construction, mais il existe déjà pour être joué, cassé, critiqué, trollé, amélioré et retesté.   
   
J'ai envie que Dinotofu reste libre : tu peux optimiser, tenter des trucs stupides, mourir comme une légende ou survivre alors que tu ne méritais clairement pas. Si une mécanique te frustre, si un boss te semble injuste, si une stratégie détruit l'équilibrage, ou si une idée te donne envie de continuer l'aventure, dis-le. Le jeu doit grandir avec les retours des joueurs, pas seulement avec mes idées dans mon coin.   
   
Défi numéro un : survivre assez longtemps pour comprendre ce qu'il se passe.   
   
Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais au moins il le fait avec style.   
   
Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.   
   
Jouez sérieusement, mais pas trop sérieusement. Plaignez-vous si besoin, proposez des idées, insultez poliment les boss, et surtout dites-moi ce qui vous donne envie de relancer une partie. Si vous trouvez un combo trop fort, je nierai sûrement avoir paniqué en voyant les chiffres, puis je dirai que c'était un test d'équilibrage avancé et que tout était prévu d'avance. C'est faux, mais c'est plus professionnel.   
   
Si vous arrivez à devenir une légende dans Dinotofu, félicitations. Si vous mourez contre les premiers ennemis, félicitations aussi, mais différemment. Si vous perdez contre un coffre, un piège ou votre propre confiance en vous, sachez que le jeu ne vous juge pas. Moi un peu, mais avec affection.   
