# Dinotofu   

Dinotofu est un RPG / jeu d'arène en C++17. La version terminal reste le socle stable, avec une interface graphique expérimentale HTML/JS pour préparer une version desktop plus confortable.   

## Version actuelle   

- Version actuelle : **V3.42.00**   
- Base de recréation fortement conseillée : **V3.00.00**   
- Les personnages créés avant **V3.00.00** peuvent fonctionner, mais une recréation est conseillée pour profiter proprement du socle histoire.   

## Ce qu'il y a déjà dans le jeu   

Le jeu permet déjà de :   

- créer, charger et sauvegarder des comptes/personnages ;   
- choisir race, âge exact compatible avec la race, présentation visuelle, variante finale, classe, difficulté et certaines identités spéciales protégées ;   
- combattre en PvP IA, PvP deux joueurs, PvE monstres, boss solo et boss coop ;   
- gérer inventaire, armes, armures, consommables, matériaux, durabilité, réparation et qualités, dont les Lucky/Unlucky Potions à effets aléatoires ;   
- acheter, vendre, troquer, visiter des lieux, consulter la guilde et la bibliothèque, et acheter/améliorer des coffres municipaux indépendants selon la ville ;   
- explorer des biomes avec ressources, coffres, pièges, traces, événements, lieux dangereux, rencontres, slimes fusionnés et un mini-boss gélatineux à quatre étages de division ;   
- suivre des quêtes de guilde, de clients, d'exploration, de combat, de livraison, de service, de bestiaire et de quête principale ;   
- accepter jusqu’à trois défis quotidiens de guilde, gagner des Marques de défi et obtenir des titres liés à de véritables contraintes de combat ;   
- consulter trois registres spéciaux impossibles à perdre : Bestiaire pour les êtres, Encyclopédie pour les systèmes/objets/zones et Carnet de découvertes pour le lore, les légendes et les rumeurs ;   
- utiliser des systèmes cachés comme les codes, les données altérées, les personnages spéciaux, les marchands légendaires et certains événements de lore ;   
- survivre exceptionnellement à une mort définitive grâce à une bénédiction compatible, au prix de toutes les bénédictions, de l’inventaire transporté, de l’équipement, de l’or et d’une marque irréversible réduisant légèrement les soins ;   
- lancer un mode histoire construit comme un bac à sable guidé et progressivement bridé, avec prologue, chapitres 1 à 3 jouables et deux phases jouables du chapitre 4 autour du village à la mauvaise date.   

## Mode histoire   

Le mode histoire démarre volontairement limité : peu de boutiques, peu de routes, peu de stocks et des quêtes principales non refusables. Il utilise exactement les mêmes entrées que le bac à sable — **Combats**, **Exploration**, **Quêtes**, **PNJ notables** et **Lieux notables** — sans créer de parcours parallèle. La différence vient uniquement du contenu réellement disponible : les PNJ pas encore arrivés, les bâtiments détruits ou en construction, les boutiques non rouvertes et les zones futures restent cachés. **Lieux notables** est classé partout en trois sections : **Ville**, **Extérieur** et **Boutiques**, avec une vue **Tout afficher** pour les joueurs qui veulent accéder directement à la liste complète. **PNJ notables** possède la même vue complète avant ses catégories. Les objectifs à étapes ne révèlent que l’étape actuelle et marquent les étapes terminées avec `[fait]`. Dans le chapitre 1, Mira reste l’unique premier contact obligatoire. Orren, Lysa, Bram et Soryn deviennent ensuite disponibles ensemble ; chacun donne immédiatement une quête principale séparée, qui peut progresser ou être terminée dans n’importe quel ordre. Après les quatre conversations, Mira crée une quête de synthèse qui relit l’état réel des quatre demandes, y compris celles déjà rendues plus tôt. Les objectifs principaux terminés restent consultables dans une archive dédiée.   

Le bac à sable reste disponible à côté pour tester, jouer librement, affronter des boss, explorer et utiliser les systèmes sans suivre l'ordre narratif.   

## Progression du registre des boss   

La progression des boss est ordonnée sans révéler la liste complète au joueur. Une première victoire contre un boss ajoute au maximum la prochaine présence encore inconnue sous le nom `???`. Refaire un boss déjà vaincu ne révèle rien de nouveau. Une exploration peut exceptionnellement faire découvrir un emplacement approximatif sans victoire, mais cette occurrence est rarissime, impose un long délai en jeu et ne révèle aucune identité. FireFlight reste soumis à sa condition finale séparée.   

## Interface graphique   

L’interface graphique reste expérimentale mais jouable et reflète le moteur C++ sans inventer de logique cachée. La V3.42.00 avance le pont vers l’IG/pixel-art et stabilise plusieurs services de ville : Top 3 principal filtré, registre avancé pour l’illégal et les incidents, médiateur de guilde, conseil de guilde, pardon progressif, probation, réputation souterraine, contrats encadrés bas rang, rumeurs de groupes, auberge locale, résolution de micro-quêtes de route, transport entre coffres municipaux et panneaux IG de préparation pixel-art. Les dossiers de groupes, relations, résultats partiels et systèmes délégués précédents restent disponibles. Les ramasse-miettes très rares, rencontres de route, choix de route, fermeture nocturne, carte schématique, trois registres permanents, coffres municipaux indépendants, consultation distante en lecture seule, profils automatiques, précets, mode concentration, silhouettes textuelles, jeux d’équipement, comparaisons, marqueurs, wishlist limitée aux objets déjà possédés, Top 3 et niveaux de fiabilité restent disponibles. Le terminal reste le secours le plus fiable : les images y sont désactivées, non activables, et ne remplacent jamais les informations écrites.   

## Installer depuis une release GitHub   

Pour installer Dinotofu sans compiler le projet manuellement :   

1. aller sur la page du dépôt GitHub ;   
2. ouvrir la dernière Release affichée à droite du dépôt ;   
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

## Notes de publication   

Les notes détaillées sont séparées par langue : `PATCHNOTE_DINOTOFU.md` en anglais et `PATCHNOTE_DINOTOFU_FR.md` en français. Les README et fichiers de suivi ne servent pas de journal de patch : ils gardent seulement les informations utiles aux joueurs et au projet.   


## Note spéciale du développeur   

Salut à toi, futur aventurier, futur héros, futur cadavre statistiquement probable.   

Bienvenue dans Dinotofu. Ici, on peut gagner avec une vraie stratégie, mourir parce qu'on a voulu faire le malin, ou découvrir qu'un gobelin fiscalement motivé est parfois plus dangereux qu'un boss mythique. Le jeu est encore en construction, mais il existe déjà pour être joué, cassé, critiqué, trollé, amélioré et retesté.   

J'ai envie que Dinotofu reste libre : tu peux optimiser, tenter des trucs stupides, mourir comme une légende ou survivre alors que tu ne méritais clairement pas. Si une mécanique te frustre, si un boss te semble injuste, si une stratégie détruit l'équilibrage, ou si une idée te donne envie de continuer l'aventure, dis-le. Le jeu doit grandir avec les retours des joueurs, pas seulement avec mes idées dans mon coin.   

Défi numéro un : survivre assez longtemps pour comprendre ce qu'il se passe.   

Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais au moins il le fait avec style.   

Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.   

Jouez sérieusement, mais pas trop sérieusement. Plaignez-vous si besoin, proposez des idées, insultez poliment les boss, et surtout dites-moi ce qui vous donne envie de relancer une partie. Si vous trouvez un combo trop fort, je nierai sûrement avoir paniqué en voyant les chiffres, puis je dirai que c'était un test d'équilibrage avancé et que tout était prévu d'avance. C'est faux, mais c'est plus professionnel.   

Si vous arrivez à devenir une légende dans Dinotofu, félicitations. Si vous mourez contre les premiers ennemis, félicitations aussi, mais différemment. Si vous perdez contre un coffre, un piège ou votre propre confiance en vous, sachez que le jeu ne vous juge pas. Moi un peu, mais avec affection.

## Note V3.30.00 — routes et poids d’équipement

- Les villes disposent désormais de distances entre elles, de distances vers les biomes et de conditions d’accès par niveau ou boss vaincu.
- Les coffres restent indépendants par ville : le voyage change la ville active, mais ne mélange pas les contenus.
- La carte d’exploration textuelle prépare les futurs fonds pixel-art par biome, avec lieux inconnus grisés ou enfumés.
- Les armes et armures ont maintenant un poids léger, moyen ou lourd. Les bonus et contreparties sont appliqués au combat, aux dégâts et à la fuite, avec des malus volontairement modérés.



## Note V3.31.00 — hubs de ville et journal canonique

La V3.31.00 rend la ville actuelle plus structurée : bâtiments locaux, contacts, verrous et indices pixel-art viennent maintenant des règles du monde plutôt que d'une liste fixe. Les destinations de ville fournissent des métadonnées IG structurées pour l'accès, la distance, le temps de trajet et le coût futur. Un journal canonique sauvegardé enregistre les événements importants comme les lieux visités, routes prises et mouvements de coffre, afin que les futurs Top 3 utilisent des données moteur au lieu de texte deviné.
