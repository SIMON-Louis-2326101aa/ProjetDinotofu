# Dinotofu   

Dinotofu est un RPG / jeu d'arène en C++17. La version terminal reste le socle stable, avec une interface graphique expérimentale HTML/JS pour préparer une version desktop plus confortable.   

## Version actuelle   

- Version actuelle : **V3.49.89**   
- Base de recréation fortement conseillée : **V3.00.00**   
- Les personnages créés avant **V3.00.00** peuvent fonctionner, mais une recréation est conseillée pour profiter proprement du socle histoire.   

## Ce qu'il y a déjà dans le jeu   

Le jeu permet déjà de :   

- créer, charger et sauvegarder des comptes/personnages ;   
- choisir race, âge exact compatible avec la race, présentation visuelle, variante finale, classe, difficulté et certaines identités spéciales protégées ;   
- combattre en PvP IA, PvP deux joueurs, PvE monstres, boss solo et boss coop, avec premières compétences actives multi-coups/multi-cibles en combat de vague, consignes d’équipe, recrues de soutien et retours à l’infirmerie après chute ;   
- gérer inventaire, armes, armures, consommables, matériaux, durabilité, réparation et qualités, dont les Lucky/Unlucky Potions et des variantes de potions fixes/proportionnelles ;   
- acheter, vendre et troquer via un menu unifié **Boutiques et comptoirs** proposant catégories rapides ou liste complète, visiter des lieux, consulter la guilde, la bibliothèque, le poste d’observation, le comptoir mercenaire, Torvald au rang D, louer le stand d’entraînement, et acheter/améliorer des coffres municipaux indépendants selon la ville ;   
- explorer des biomes avec ressources, coffres, pièges, traces, événements, lieux dangereux, rencontres, slimes fusionnés et un mini-boss gélatineux à quatre étages de division ;   
- suivre des quêtes de guilde, de clients, d'exploration, de combat, de livraison, de service, de bestiaire et de quête principale ;   
- accepter jusqu’à trois défis quotidiens de guilde, gagner des Marques de défi et obtenir des titres liés à de véritables contraintes de combat ;   
- consulter trois registres spéciaux impossibles à perdre : Bestiaire pour les êtres, Encyclopédie pour les systèmes/objets/zones et Carnet de découvertes pour le lore, les légendes et les rumeurs ;   
- utiliser des systèmes cachés comme les codes, les données altérées, les personnages spéciaux, les marchands légendaires et certains événements de lore ;   
- survivre exceptionnellement à une mort définitive grâce à une bénédiction compatible, au prix de toutes les bénédictions, de l’inventaire transporté, de l’équipement, de l’or et d’une marque irréversible réduisant légèrement les soins ;   
- lancer un mode histoire construit comme un bac à sable guidé et progressivement bridé, avec prologue, chapitres 1 à 3 jouables et deux phases jouables du chapitre 4 autour du village à la mauvaise date.   

## Mode histoire   

Le mode histoire démarre volontairement limité : peu de boutiques, peu de routes, peu de stocks et des quêtes principales non refusables. Il utilise exactement les mêmes entrées que le bac à sable — **Combats**, **Exploration**, **Personnage** et **Monde / ville** — sans créer de parcours parallèle. La différence vient uniquement du contenu réellement disponible : les PNJ pas encore arrivés, les bâtiments détruits ou en construction, les boutiques non rouvertes et les zones futures restent cachés. **Lieux notables** est classé partout en trois sections : **Ville**, **Extérieur** et **Boutiques**, avec une vue **Tout afficher** pour les joueurs qui veulent accéder directement à la liste complète. **PNJ notables** possède la même vue complète avant ses catégories. Les objectifs à étapes ne révèlent que l’étape actuelle et marquent les étapes terminées avec `[fait]`. Dans le chapitre 1, Mira reste l’unique premier contact obligatoire. Orren, Lysa, Bram et Soryn deviennent ensuite disponibles ensemble ; chacun donne immédiatement une quête principale séparée, qui peut progresser ou être terminée dans n’importe quel ordre. Après les quatre conversations, Mira crée une quête de synthèse qui relit l’état réel des quatre demandes, y compris celles déjà rendues plus tôt. Les objectifs principaux terminés restent consultables dans une archive dédiée.   

Le bac à sable reste disponible à côté pour tester, jouer librement, affronter des boss, explorer et utiliser les systèmes sans suivre l'ordre narratif.   

## Progression du registre des boss   

La progression des boss est ordonnée sans révéler la liste complète au joueur. Une première victoire contre un boss ajoute au maximum la prochaine présence encore inconnue sous le nom `???`. Refaire un boss déjà vaincu ne révèle rien de nouveau. Une exploration peut exceptionnellement faire découvrir un emplacement approximatif sans victoire, mais cette occurrence est rarissime, impose un long délai en jeu et ne révèle aucune identité. FireFlight reste soumis à sa condition finale séparée.   

## Interface graphique   

L’interface graphique reste expérimentale mais jouable et reflète le moteur C++ sans inventer de logique cachée. La V3.49.62 clarifie le routage terminal après combat : **Continuer** ramène au **Menu de base**, puis **Monde / ville** se choisit comme activité explorable normale. Le menu rapide reste concentré sur Personnage, saisie libre, options de partie et sauvegarde. Le terminal reste le secours le plus fiable : les images de gameplay y sont désactivées, non activables, mais les raccourcis/installateurs peuvent utiliser leur icône dédiée.   

## Installer depuis une release GitHub   

Pour installer Dinotofu sans compiler le projet manuellement :   

1. aller sur la page du dépôt GitHub ;   
2. ouvrir la dernière Release affichée à droite du dépôt ;   
3. télécharger uniquement l’archive **DinotofuInstaller** correspondant à ton système d’exploitation : Windows ou Linux ;   
4. dézipper l’archive, puis lancer **Installer-Dinotofu**. Le pack contient aussi le ZIP du jeu en secours local si GitHub ne le fournit pas directement.   

Sur les versions desktop Windows/Linux, l'installateur ou le launcher doit créer deux entrées claires :   

- **ProjetDinotofu Launcher** : lancement normal / Auto, via le launcher adapté à l'OS ;   
- **ProjetDinotofu Launcher Terminal version** : lancement forcé en terminal, via le launcher Windows/Linux correspondant.   


## Structure rapide   

- `src/` : code C++ du jeu ;   
- `include/` : headers C++ ;   
- `assets/config/` : données JSON de référence ;   
- `assets/branding/` : logo, bannière, couverture/menu et icônes officielles des launchers/raccourcis ;   
- `assets/logo/` : copies propres des logos fournis et renommés ;   
- `assets/visuals/` : décors IG fournis, séparés en biomes, villes et intérieurs ;   
- `tools/gui/` : interface graphique expérimentale et serveur local ;   
- `tools/windows/` et `tools/linux/` : launchers/installateurs ;   
- `scripts/` : packaging, version, release et contrôles ;   
- `release/` : manifeste et données de publication ;   
- `assets/saves/` : dossiers de sauvegarde locaux, sans données privées dans les ZIP.   
- `logs/dinotofu_session_latest.txt` : journal local de session bêta, supprimable sans risque, utile à envoyer au dev en cas de bug ou combat étrange.   

## Notes de publication   

Les notes détaillées sont séparées par langue : `PATCHNOTE_DINOTOFU.md` en anglais et `PATCHNOTE_DINOTOFU_FR.md` en français. Les README et fichiers de suivi ne servent pas de journal de patch : ils gardent seulement les informations utiles aux joueurs et au projet.   


### V3.49.89 - Rivaux et destin instable   

La V3.49.89 ajoute les serments des Rivaux et du Destin instable. Les fuites, paniques et compétences signatures peuvent laisser des traces plus nettes de rival potentiel, tandis que le destin instable crée de petites oscillations seulement lorsqu'une trace existe déjà : mémoire, rupture, rival ou objet marqué.   

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




### V3.49.86 - Maîtrise tactique liée au build   

La V3.49.86 relie davantage les maîtrises actives à la cohérence de l’équipement. Une arme ou armure cohérente avec la classe soutient légèrement les gestes liés, tandis qu’un [malus de classe] peut rendre une action moins propre malgré la maîtrise. L’audit de classe affiche aussi survie, critique et équipement attendu, et les passifs d’observation réduisent un peu les mauvaises surprises face aux compétences signatures ennemies.   

### V3.49.36 - Visée des créatures et textes plus immersifs   

La V3.49.36 ajoute une lecture de visée aux profils ennemis : les petites créatures comme rats, fées, chauves-souris, insectes et profils sournois sont plus difficiles à cadrer, tandis que les brutes, constructions, dragons, plantes enracinées et gardiens ouvrent parfois des fenêtres plus faciles à toucher. L'observation et le bestiaire indiquent maintenant cette lecture de visée. Plusieurs textes en combat ont aussi été reformulés pour rester dans l'univers du jeu au lieu de parler comme une note de mise à jour.   

### V3.49.36 - Variantes ennemies et affinité féerique   

La V3.49.36 étend les profils comportementaux avec des variantes plus précises : chauve-souris, rat/nuisible, chargeur massif, araignée, kobold, archer, alchimiste, fée, spectre, construction, dragon et serment sacré. Les profils ont maintenant une attaque signature, des réactions plus propres et une ligne de contre-jeu. Les fées gagnent aussi une vraie règle : 50% de résistance magique, mais 50% de faiblesse physique. Les parchemins offensifs comptent comme dégâts magiques pour que cette faiblesse/résistance soit réellement visible.   

### V3.49.34 - Profils de monstres et attaques signature   

La V3.49.34 ajoute une première couche centrale de profils comportementaux pour les ennemis. Les slimes, voleurs, gobelins, brutes, prédateurs, gardiens, plantes, insectoïdes, supports et entités instables gagnent des descriptions d'attaque, forces, failles et réactions plus propres. L'observation active et le bestiaire affichent maintenant ces profils pour aider le joueur à comprendre pourquoi un ennemi agit différemment d'un autre.   

### V3.49.33 - Formations ennemies et rupture de formation   

La V3.49.33 ajoute une couche de combat autour des tours de formation ennemie. Certaines vagues coordonnées peuvent maintenant utiliser couverture, précision courte, petite garde ou posture défensive au lieu d'attaquer basiquement. Le joueur reçoit la réponse **Casser la formation**, une action tactique qui perturbe plusieurs ennemis, peut retirer des postures défensives, et peut débloquer **Briseur de formation** après plusieurs vrais usages.   

### V3.49.31 - Brise-garde et posture de soutien   

La V3.49.31 avait ajouté deux actions tactiques : **Brise-garde**, une action de contrôle courte avec affaiblissement/vulnérabilité, et **Tenir la ligne / couvrir**, une posture de soutien avec provocation courte, garde élémentaire et précision. Ces actions commençaient la progression passive vers **Casseur de garde** et **Rythme de soutien**.   

### V3.49.30 - Rumeur Hero Villager retardée et pression ennemie   

La V3.49.30 corrige la rumeur Hero Villager trop précoce : la guilde ne parle plus de lui directement après l'inscription au jour 0. La rumeur et la rencontre rare sur route demandent maintenant une vraie progression : jours passés, niveau, contrats, histoire, actions tactiques ou observation. Les ennemis gagnent aussi une petite pression non basique : profils intelligents capables de feinte, créatures opportunistes qui exploitent les ouvertures visibles, et pression bestiale pouvant créer une vulnérabilité au lieu de toujours faire une attaque simple.   

### V3.49.29 - Préparation d'arme, lecture voleur et coffres reliés   

La V3.49.29 ajoute l'action tactique **Enduire / fusionner vite l'arme**, qui consomme un composant pour appliquer un effet temporaire sur une cible : poison, choc, givre, vulnérabilité, précision ou puissance selon le matériau. Les dagues gagnent une synergie spéciale proche de l'idée de dague empoisonnée. L'observation active peut aussi apprendre **Lecture des serrures et failles** aux profils discrets, puis cette lecture aide réellement sur les coffres suspects d'exploration.   

### V3.49.28 - Ouvertures tactiques et artisanat de combat   

La V3.49.28 enrichit le menu **Actions tactiques** avec **Exploiter une ouverture** et **Piège improvisé d'artisan**. Les blessures et statuts peuvent maintenant créer des réactions tactiques utiles, et certains petits matériaux deviennent consommables en combat pour gêner la ligne ennemie. Les actions tactiques font aussi progresser des passifs comme **Lecture du terrain** et **Improvisateur de combat**.   

### V3.49.27 - Audit retours, compagnon Dinotofu et combos de statuts   

La V3.49.27 ajoute un premier **Compagnon Dinotofu** accessible depuis les activités et le menu hors combat. Il donne des conseils courts selon les PV, les quêtes, les lanternes, les compétences et le journal beta local. Les techniques d'arme profitent aussi de réactions de statuts, par exemple brûlure + givre, poison + saignement, choc + vulnérabilité, ou affaiblissement + vulnérabilité. Cette passe corrige aussi la cohérence des fichiers de version après la V3.49.26.   

### V3.49.26 - Actions tactiques, panneau de guilde et catégories rapides   

La V3.49.26 ajoute le menu **Actions tactiques** en combat de vague : lancer une lanterne, jeter une lanterne au sol, repousser, utiliser la poussière ou observer activement. Le panneau de guilde utilise un libellé d'offre séparé du journal actif, affiche le rang et clarifie le cas même lieu. Le menu **Boutiques et comptoirs** gagne aussi plusieurs catégories rapides utiles.   

### V3.49.25 — Consignes alliées plus tactiques   

La V3.49.25 ajoute des consignes de combat plus utiles pour les recrues : forcer une technique prête sur une recrue précise, demander une percée coordonnée de groupe, ou répartir les cibles pendant 1 tour contre plusieurs ennemis. Ces consignes ne consomment pas le tour du joueur, mais elles expirent après le tour allié, sauf la priorité de cible qui reste jusqu’à mort/disparition de la cible.   

### V3.49.25 — Variété combat et journal bêta   

La V3.49.25 ajoute Rupture de ligne, Suture de fortune et Signal de focus, stabilise légèrement les attaques normales avec une énorme plage de dégâts, et ajoute un accès Journal bêta dans l’après-combat pour retrouver facilement `logs/dinotofu_session_latest.txt`.   

### V3.49.25 — Quêtes personnelles de recrues et contributions alliées plus riches   

La V3.49.25 ajoute une première boucle de quêtes personnelles pour les recrues. Une recrue peut maintenant avoir un problème personnel lié à son profil, la progression peut se faire sur plusieurs tentatives, et la réussite améliore légèrement la loyauté tout en ajoutant de la réputation de clan. Les contributions alliées deviennent aussi plus lisibles : actions de soutien et coups de finition sont suivis, avec des techniques plus distinctes pour gardiens/roublards.   

### V3.49.19 — Dettes d’infirmerie bloquantes   

La V3.49.19 rend les dettes d’infirmerie réellement contraignantes : tant qu’une dette existe, les soins payants sont bloqués. Si la dette totale dépasse 100 or, la récupération/réanimation d’une recrue prête à sortir est aussi bloquée jusqu’au remboursement partiel.   

### V3.49.17 — Infirmerie complète et auberge moins cheatée   

La V3.49.17 ajoute un vrai service d’infirmerie : se soigner, soigner un membre d’équipe, ou gérer l’entrée/sortie des recrues KO. Les soins payants montent jusqu’à 90% PV maximum. L’auberge est rééquilibrée : les lits communs/simples plafonnent à 50% PV, tandis que la chambre sûre plus chère peut monter jusqu’à 90% sans full heal gratuit.   

### V3.49.16 — Recrues persistantes et évacuation infirmerie   

La V3.49.16 continue d’améliorer les combats : les recrues ont maintenant de vrais PV persistants sauvegardés, plus des PV temporaires de combat. Elles peuvent commencer entre 70% et 100% de PV, garder leurs PV/potions entre les combats, et si une recrue tombe à 0 PV elle doit être amenée à l’infirmerie au lieu d’y être envoyée automatiquement. Si le joueur tombe aussi, plusieurs jours peuvent passer selon la gravité, et les recrues KO suivent le transfert vers les soins.   

### V3.49.11 — Recrues en combat PvE   

Les recrues équipées commencent à agir dans les vrais combats PvE standard : soutien, dégâts simples, soin contextuel et partage de récompenses. Le joueur reste premier et conserve toujours la plus grosse part ; l’ordre manuel d’équipe reste prioritaire sur le tri automatique.   




### V3.49.70 - Marque mortelle collective de FireFlight   

La V3.49.70 transforme la marque mortelle de FireFlight : elle ne vise plus une seule cible, mais tout le camp adverse en même temps. Pendant 2 tours de boss, chaque adversaire marqué qui tombe subit une vraie mort définitive, avec des lignes RP indiquant que FireFlight joue avec la panique du groupe entier.   

### V3.49.69 - Marque mortelle FireFlight et hésitation des recrues   

La V3.49.69 transforme la fenêtre de panique de FireFlight en vraie **marque mortelle** ciblée sur le joueur. Pendant 2 tours de boss, tomber sous cette marque est traité comme une mort définitive et non comme une défaite non létale classique. La passe ajoute aussi une petite règle d’hésitation pour les recrues bas rang : sans ordre clair, elles peuvent renoncer à une technique trop avancée et revenir à une action plus simple.   

### V3.49.68 - Paliers variables et FireFlight mortel   

La V3.49.68 corrige l’idée de plafond fixe : 10 niveaux actifs et 5 niveaux passifs restent seulement les maximums absolus. Les compétences simples peuvent avoir moins de paliers, et l’affichage montre maintenant le plafond local. FireFlight gagne aussi une fenêtre rare de 2 tours où les règles de mortel peuvent s’imposer temporairement, pour ajouter panique et stress sans changer la difficulté sauvegardée.   

### V3.49.67 - Progression actifs/passifs plus lisible   

La V3.49.67 améliore les retours de progression des compétences. La maîtrise active affiche maintenant le prochain palier d’usage et le type de petit effet gagné, tandis que les passifs de maîtrise montrent leur progression en 5 niveaux et leur effet léger. Le menu des actions tactiques rappelle aussi que les paliers restent équilibrés : surtout fiabilité, précision, souffle, rythme ou contrôle léger, pas de montée cheat trop tôt. Les menus de consignes alliées rappellent aussi qu’une recrue faible rang comprend mieux un ordre clair qu’une intention de groupe trop vague.   

### V3.49.66 - Maturité de combat des recrues   

La V3.49.66 ajoute une couche de maturité de combat aux recrues. Une recrue faible rang lit moins souvent le terrain, choisit des cibles plus simples, utilise rarement ses techniques avancées et profite davantage d’un ordre clair que de son instinct. Le rang, le niveau et l’équipement améliorent progressivement les réactions, commentaires tactiques, choix de cible et fiabilité des techniques pour mieux sentir son évolution.   

### V3.49.65 - Lectures vivantes ennemies et alliées   

La V3.49.65 ajoute des lectures de combat plus contextuelles sans changer la structure des menus. Les lignes ennemies réagissent mieux à l’archétype, aux états visibles et au danger côté cible, tandis que les recrues peuvent commenter le premier profil ennemi, le danger sur le joueur, la pression de groupe et leur propre rôle ou race. **Monde / ville** gagne aussi de courtes lignes d’ambiance et de rumeur selon le moment, l’état du joueur et le contexte de quête.   

### V3.49.62 - Routage clair vers le menu de base   

La V3.49.62 clarifie le flux après combat. **Continuer** signifie maintenant revenir au **Menu de base**. **Monde / ville** reste une activité normale du menu de base pour les lieux explorables, boutiques, guilde, PNJ et services, tandis que le menu rapide reste un hub constant pour personnage/session/sauvegarde.   

### V3.49.61 - Refonte du menu rapide et rangement personnage/monde   

La V3.49.61 range les accès hors combat autour d’un menu rapide plus lisible. **Personnage** regroupe inventaire, titres, compétences actifs/passifs, quêtes acceptées, statistiques, équipement rapide, équipe et échange. Correction V3.49.61 : **Monde / ville** redevient une activité de lieux explorables accessible depuis les activités, pas depuis le menu rapide. L’après-combat renvoie vers le menu rapide pour le personnage/options/sauvegarde, ou vers Continuer pour retourner aux activités et visiter la ville.   

### V3.49.59 - Menu de charge et audit actifs/passifs   

La V3.49.59 ajoute le menu hors combat de gestion des compétences dans le hub statistiques. Les actifs peuvent être équipés ou déséquipés, les passifs peuvent être activés ou désactivés, et les statistiques ouvertes pendant un combat restent seulement consultatives pour éviter de changer de build au milieu d’un tour. La passe vérifie aussi les techniques tactiques récentes pour que les identifiants d’actifs ne soient plus traités comme des effets passifs.   

### V3.49.58 - Charge de compétences et maîtrise progressive   

La V3.49.58 ajoute une vraie base pour différencier les compétences connues, équipées et activées. Un personnage peut connaître plus de compétences, mais seules 10 compétences actives peuvent être équipées et seuls 10 passifs peuvent être activés en même temps. Les actifs restent des actions choisies, tandis que les passifs restent des effets automatiques ou semi-automatiques pouvant aussi servir hors combat selon leur nature.   

Les actifs gagnent maintenant une maîtrise sur 10 paliers maximum, avec des seuils espacés pour ne pas devenir trop forts trop tôt dans une progression prévue jusqu'au niveau 255. Les passifs issus d'une pratique répétée doivent d'abord passer par trois essais réussis visibles avant de se débloquer réellement.   

### V3.49.57 - Séparation actif / maîtrise passive   

La V3.49.57 clarifie la différence entre technique de combat sélectionnable et maîtrise passive. Les techniques d'affinité restent des actions actives du menu tactique, tandis que les vrais usages répétés débloquent désormais des maîtrises passives avec un nom séparé : **Maîtrise élémentaire**, **Garde circulaire**, **Trait entravant**, **Voix revigorante**, **Instinct canalisé** et **Rythme de lame**. Les anciens identifiants de sauvegarde restent reconnus, mais l'affichage ne donne plus l'impression que **Danse de lame** ou les autres techniques deviennent automatiques.   

### V3.49.56 - Techniques variées d'affinité   

La V3.49.56 ajoute six techniques d’affinité supplémentaires pour éviter que les combats ne tournent seulement autour des ruptures/débuffs : **Lame élémentaire**, **Cercle protecteur**, **Flèche entravante**, **Chant revigorant**, **Instinct de bête** et **Danse de lame**. Les profils élémentaires, protecteurs, pisteurs, bardes/chefs, sauvages et duellistes gagnent chacun une option dédiée avec passif après vrais usages.   

### V3.49.55 - Affinités de classe élargies   

La V3.49.55 ajoute cinq techniques d’affinité supplémentaires : **Rage maîtrisée**, **Ordre de bataille**, **Totem de souffle**, **Bombe d’atelier** et **Prière d’acier**. Cette passe élargit les classes spécialisées sans rendre toutes les actions universelles : front, commandement, nature, atelier et sacré gagnent chacun une option avec passif après vrais usages.   

### V3.49.53 - Techniques d’affinité de classe   

La V3.49.53 ajoute une première couche de **techniques d’affinité de classe** : **Pas de l’ombre** n’est plus universel, et les profils sournois, soutien, arcanique, rempart et pisteur gagnent chacun une technique dédiée avec progression après vrais usages.   

### V3.49.50 - Allonge, remparts et ancrages   

La V3.49.50 ajoute trois actions tactiques : **Rompre l’allonge**, **Percer le rempart** et **Rompre l’ancrage occulte**. Elle ajoute aussi les profils **Allonge prudente**, **Ancre nécrotique** et **Drain de vie**, avec réactions de distance, froid de tombe, récupération courte et contre-jeu par rupture d’espace ou d’ancrage.   

### V3.49.49 - Chaînes d’états, proies marquées et profils ennemis   

La V3.49.49 ajoute trois actions tactiques : **Forcer une chaîne d’états**, **Marquer une proie** et **Retrait contrôlé**. Elle ajoute aussi les profils **Soigneur de fortune**, **Porte-bouclier**, **Berserker blessé** et **Porte-malédiction**, avec observations, faiblesses, attaques signatures et réactions mécaniques propres.   

### V3.49.38 - Taille, matière et résistance physique   

La V3.49.38 ajoute une vraie lecture de résistance liée au corps des créatures : petit fragile, petit protégé, grande masse organique, grande masse dure, matière mauvaise, slime, spectre ou fée ne se comportent plus pareil face aux dégâts physiques. L'observation et le bestiaire affichent cette lecture, et le rapport de dégâts montre une phrase immersive quand le corps modifie l'impact. Les techniques de recrues gagnent aussi des gestes plus liés à leur profil.   


### V3.49.74 - Maîtrises élargies et préparation de sortie   

La V3.49.74 continue les six chantiers actifs en même temps : plusieurs anciennes techniques d’affinité reçoivent de vrais effets de maîtrise, les passifs tactiques restent des soutiens et non des actions automatiques, la pression ennemie gagne des intentions contextuelles, les recrues utilisent davantage leurs techniques selon leur maturité, et Monde / ville gagne une lecture de préparation de sortie pour soin, outils, observation, coffres, artisanat combat et renforts mercenaires.   

### V3.49.73 - Passifs de maîtrise actifs et ville plus lisible   

La V3.49.73 branche davantage les passifs de maîtrise dans les effets réels des actions tactiques : un passif débloqué et activé peut maintenant soutenir légèrement la puissance, la chance secondaire, la durée ou le rythme, sans jamais lancer l’actif à la place du joueur. Monde / ville affiche aussi plus de repères locaux : ville actuelle, inscription de guilde et temps de voyage.   


### V3.49.81 - Synergies lisibles et impact de maîtrise   

La V3.49.81 ajoute une lecture globale du build dans le menu Personnage et la préparation de sortie : arme, armure, bonus/malus de classe et cohérence générale. Les maîtrises actives/passives utilisent des libellés plus clairs sur l’impact réel, les techniques de recrues profitent davantage de la maturité et des ordres, et les compétences signatures ennemies deviennent un peu plus présentes quand l’ennemi est entraîné, élite ou face à un joueur déjà ouvert.   

