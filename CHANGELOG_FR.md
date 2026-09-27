# Journal des modifications Dinotofu   

Historique détaillé des versions de Dinotofu en français. Le journal anglais équivalent se trouve dans `CHANGELOG.md`.   

## V3.50.09 — Point de sauvegarde obligatoire et gros découpage de l'exploration   
- **V3.50.09 devient un nouveau point de sauvegarde important.** `VersionInfo::importantSaveUpdateVersion()` pointe désormais vers 3.50.09. Un personnage dont `lastAdaptedVersion` est antérieur à ce jalon ne peut plus reprendre silencieusement une partie normale : le menu des personnages l'identifie explicitement comme nécessitant une mise à jour importante et impose le rituel d'adaptation lourde déjà connu.   
- Avant toute adaptation, `SaveManager::createImportantUpdateBackup()` crée un checkpoint dédié dans `assets/saves/update_backups/V3.50.09/<compte>/`, avec la sauvegarde du personnage, celle du compte et un petit manifeste. Les copies `__before_V3.50.09` sont volontairement **non écrasables**, y compris après de nouveaux essais ou la rotation normale des `.bak`. Si cette copie de sécurité ne peut pas être créée, Dinotofu refuse l'adaptation/recréation au lieu de risquer une migration destructive.   
- Une adaptation lourde réussie marque le personnage comme adapté à la version courante : le rituel n'est donc demandé qu'une fois pour ce jalon. Les anciennes règles de compatibilité/recréation des sauvegardes antérieures à V3.00.00 restent séparées. Le schéma de sauvegarde reste **23**, car le checkpoint protège/migre l'état existant sans ajouter de nouveau champ sérialisé.   
- Ajout de `ImportantSaveCheckpointTest.cpp` : détection du jalon, création des copies personnage/compte, manifeste, puis vérification que le checkpoint initial n'est pas écrasé après modification de la sauvegarde active. `make test` protège aussi le câblage et les messages obligatoires du point de sauvegarde.   
- Poursuite de la modularisation sans `.inc` avec le plus gros découpage de `QuestMenu` à ce jour. Le moteur d'exploration — biomes, distance/intensité, voyage, nuit/température, indices de recherche de quête, combats/événements, micro-épreuves, coffres, mini-boss, sites dangereux, découvertes, récompenses et menus d'exploration — vit maintenant dans le vrai module `QuestExplorationMenu.cpp`.   
- `QuestMenu.cpp` passe d'environ **12 334 à ~6 176 lignes**. `QuestExplorationMenu.cpp` contient ~6 347 lignes et communique via `QuestExplorationSupport.hpp` ainsi qu'un pont volontairement étroit `QuestMenuInternalSupport.hpp` pour les quelques helpers encore partagés avec la guilde/les clients. Les comportements communs sont exposés par interface au lieu d'être copiés ; aucun `.inc` n'est réintroduit.   
- Les tests structurels exigent maintenant les nouveaux modules d'exploration et imposent une limite `< 7 000` lignes à `QuestMenu.cpp`, afin d'empêcher le moteur extrait de revenir discrètement dans le monolithe. Les contrôles des contenus vivants, événements ambiants, traces multilingues et interactions non-combat ciblent désormais le module d'exploration qui porte réellement ces responsabilités.   
- La compilation/édition de liens C++17 complète et toute la suite `make test` passent avec `-Wall -Wextra`. La progression principale reste verrouillée juste après l'introduction du chapitre 3 : cette passe sécurise les sauvegardes et l'architecture sans avancer l'histoire.   

## V3.50.08 — Séparation histoire/runtime, supports tactiques et choix de biomes enrichis   
- Poursuite de la modularisation sans `.inc`, uniquement avec de vraies frontières `.cpp/.hpp`. Le runtime histoire **déjà existant** quitte `Game.cpp` pour `GameStory.cpp` ; aucune nouvelle scène de l'histoire principale n'est ajoutée et le verrou après l'introduction du chapitre 3 reste inchangé. `Game.cpp` passe d'environ 6 225 à **3 250 lignes**.   
- `QuestMenu.cpp` descend encore d'environ 14 100 à **12 334 lignes**. La synchronisation/navigation de l'histoire principale vit maintenant dans `QuestStoryMenu.cpp`, les helpers propres à l'histoire dans `QuestStorySupport.cpp/.hpp`, et la présentation partagée type/progression des quêtes dans `QuestPresentationSupport.cpp/.hpp`. Le découpage a révélé des dépendances cachées entre fichiers, qui ont été centralisées au lieu d'être dupliquées.   
- `PlayerWaveTacticalActionMenu.cpp` passe d'environ 4 410 à **2 901 lignes**. Soixante-quatorze helpers d'affinité, ciblage, maîtrise tactique, synergies d'équipement, formations, enduits, lanternes, pièges et sélection états/parties du corps vivent désormais dans `PlayerWaveTacticalSupport.cpp/.hpp`. Le module d'action garde ainsi surtout l'exécution des techniques.   
- Ajout de **10 interactions non-combat supplémentaires propres aux biomes** : charrette renversée de la Plaine sauvage, lanterne éteinte du Bocage aux lanternes, double balisage de puits du Désert d'argile rouge, fenêtre marquée du Quartier abandonné, étai fendu de la Mine sifflante, outil magnétisé du Verger des lucioles de fer, écorce noire récente du Bois de la Corruption, cercle d'offrandes du Bosquet des Fées du Mana, rubans aux neuf versions du sanctuaire kitsuné et amarre dérivante de l'Archipel des îles flottantes. Chaque interaction propose des choix locaux, conséquences d'exploration limitées, traces historiques et anti-farm le même jour.   
- Les techniques ennemies préparées gagnent trois familles mécaniques : **souffle élémentaire**, **piqué aérien** et **venin préparé**. Chacune possède son télégraphe visible, son seuil d'interruption et sa résolution propre. Le souffle affaiblit et ouvre la cible, le piqué échange un fort engagement contre dégâts/repositionnement, et le venin privilégie l'empoisonnement aux dégâts instantanés.   
- Extension des tests C++ ciblés pour les nouvelles interactions de biome et familles de techniques préparées. Les tests structurels protègent désormais `GameStory`, les modules histoire/présentation de quêtes et le support tactique, avec des plafonds de taille plus stricts sur les orchestrateurs restants.   
- Construction C++17 complète/édition de liens et suite `make test` intégrale réussies avec `-Wall -Wextra` avant le changement de version. Le schéma de sauvegarde reste **23**, cette passe n'ajoutant aucun nouveau champ persistant.   
- La progression principale reste bloquée immédiatement après l'introduction du chapitre 3. La priorité demeure le monde vivant, le contenu utile, la variété de combat, la stabilité et la réduction progressive des fichiers surdimensionnés.   

## V3.50.07 — Tour joueur modularisé, interactions de biome et tactiques ennemies enrichies   
- Passe structurelle majeure sur le combat : `PlayerWaveCombatTurn.cpp` passe d'environ 4 769 à **394 lignes**. Les milliers de lignes d'actions tactiques du joueur vivent maintenant dans le vrai module `PlayerWaveTacticalActionMenu.cpp/.hpp` (~4 410 lignes), tandis que le fichier de tour principal redevient un orchestrateur lisible. Aucun `.inc` n'est introduit et les tests d'invariants imposent désormais cette séparation.   
- Nouveau `BiomeNonCombatInteractionSystem` réellement branché à l'exploration. Plusieurs biomes peuvent proposer un petit choix local contextuel — borne routière contradictoire, collet oublié, passerelle de marais, tombe entretenue, registre noyé, corde près d'un nid de drake, dette disputée, cloche de serment — avec conséquence d'exploration, trace historique et parfois progression de quête. Une même interaction ne peut pas être farmée en rouvrant le menu le même jour.   
- Les grosses techniques ennemies préparées sont maintenant différenciées par familles observables : charge lourde, tir lourd, entrave massive, cri de meute, rituel et frappe engagée. Chaque famille possède son seuil d'interruption par dégâts, son effet et son télégraphe propres ; choc, givre et entrave restent aussi de vraies interruptions.   
- Certains protecteurs/alliés coordonnés peuvent sacrifier leur tour pour **couvrir un ennemi qui prépare une technique**. La couverture donne une défense/ward temporaire au préparateur mais ne crée jamais d'immunité ; elle consomme bien le tour du protecteur et produit une réaction visible.   
- Ajout de **18 nouvelles créatures/variantes** liées aux biomes et aux nouvelles mécaniques, notamment Arbalétrier de relais, Tisseuse de racines anciennes, Chaman de vase au souffle long, Copiste de sceau noyé, Drake gris plongeur, Sonneur de grand serment, Huissier des trois versions et Maître-carillonneur du pacte.   
- Nouveau `NpcRelationshipSystem` : des PNJ nommés peuvent posséder une relation réciproque explicite (confiance professionnelle, coordination locale, réseau savant, partenariat, prudence ou méfiance utile). Cette relation modifie la priorité de transmission d'une information existante sans jamais fabriquer un fait ou supprimer la nécessité d'une source.   
- Davantage de PNJ nommés ont maintenant un profil explicite plutôt qu'un simple profil déduit : Mira, Orren, Lysa, Bram, Soryn, Eda, Nell, Meron, Prunigil, Bob et Maurice. Intendance, soigneuse, logistique, messagère et marchand itinérant ont également leurs propres règles/canaux de relais au lieu de retomber sur le bouche-à-oreille générique.   
- Les tests couvrent désormais les interactions de biome, l'anti-farm journalier, les relations PNJ, les profils nommés, les 18 ajouts de monstres, les variantes de techniques préparées et la protection d'un préparateur. La compilation/link C++17 complète et `make test` passent avec `-Wall -Wextra`.   
- Le schéma de sauvegarde reste **23** : ces ajouts réutilisent les mémoires PNJ persistantes existantes, tandis que les interactions journalières sont enregistrées via l'historique canonique et les préparations restent un état transitoire de combat.   
- Aucun avancement de la progression principale après l'introduction du chapitre 3. La priorité reste monde vivant, variété, conséquences, systèmes et réduction progressive des fichiers surdimensionnés.   

## V3.50.06 — Connaissances inter-villes, menaces interruptibles et modularisation approfondie   
- Poursuite du chantier structurel sans aucun `.inc`, uniquement avec de vraies frontières `.cpp/.hpp`. La création de personnage, le choix difficulté/règle de mort, race/classe/apparence et la configuration du groupe local quittent `Game.cpp` pour `GameSetup.cpp` ; `Game.cpp` passe d'environ 7 642 à 6 225 lignes.   
- `QuestMenu.cpp` redescend d'environ 14 989 à 14 017 lignes. La navigation des lieux et des PNJ notables vit maintenant dans `QuestLocationNpcMenu.cpp`, tandis que les états de demandes, règles « prête à rendre », livraisons matérielles et clients recommandés partagés vivent dans `QuestClientNavigationSupport.cpp`. Des invariants de test protègent désormais ces nouvelles limites de taille.   
- Les connaissances PNJ peuvent maintenant voyager entre de vraies villes sans téléportation. La propagation inter-ville utilise les distances de `CityTravelRules` et un transporteur/délai propre au réseau : messager de garde, courrier de guilde, caravane marchande, pèlerin itinérant, voyageur de relais, copie d'archive, convoi artisan, etc. Une information fraîche ne peut donc pas apparaître ailleurs avant que suffisamment de jours du monde aient réellement passé.   
- Les informations inter-villes conservent l'identité de leur source et leurs variantes contradictoires tout en perdant de la confiance selon distance/réseau. Une simple rumeur ne devient jamais une preuve dure juste parce qu'elle a voyagé ; certains rapports savants/de guilde peuvent seulement conserver une preuve limitée si leur source en possédait réellement.   
- Ajout de `MonsterPreparedActionSystem` : certaines grosses signatures ennemies sont maintenant préparées visiblement pendant un tour ennemi au lieu de devenir un buff instantané caché. La préparation peut être interrompue par choc, givre, entrave ou assez de dégâts immédiats. Une interruption réussie consomme la technique préparée et expose brièvement l'ennemi.   
- Si la technique préparée aboutit, elle déclenche son coup engagé et provoque un repositionnement/ouverture abstrait sans inventer une grille qui n'existe pas dans le jeu. L'interface n'annonce que la préparation réellement observée par le joueur et n'affiche toujours aucune intention IA future cachée.   
- Ajout de tests C++ légers pour les délais/transporteurs inter-villes et pour la résolution/interruption des techniques préparées, désormais intégrés à `make test`. Les tests d'architecture ont aussi été adaptés afin que le déplacement réel de responsabilités hors des monolithes soit considéré comme le design attendu.   
- Build/link C++17 complet validé avec `-Wall -Wextra`, et toute la suite `make test` passe. Le schéma de sauvegarde reste 23 : la propagation inter-ville réutilise les métadonnées de faits PNJ déjà persistées et les préparations de combat restent un état transitoire de combat.   
- La progression principale reste verrouillée juste après l'introduction du chapitre 3. La priorité reste la vie du monde, les systèmes, le contenu, la stabilité et la poursuite du découpage des fichiers surdimensionnés.   

## V3.50.05 — Réseaux d’information, tactiques de groupe et nouvelle modularisation   
- Nouveau passage structurel sans `.inc` : le bloc ville/voyage/auberge/coffre municipal/découvertes de route est extrait de `QuestMenu.cpp` dans `QuestWorldMenuSupport.cpp`, avec `QuestDeadlineSupport.cpp` pour centraliser l’expiration des délais. `QuestMenu.cpp` descend d’environ 16 457 à 14 989 lignes.   
- `ShopMenu.cpp` descend d’environ 6 123 à 4 075 lignes. Les services urbains, abonnements, événements locaux, logement, transport et travaux de ville vivent désormais dans `ShopCityServiceMenu.cpp`, en réutilisant `ShopServiceSupport` plutôt que de dupliquer paiement et affichage.   
- Les profils PNJ possèdent maintenant un réseau d’information structuré : garde, commerce, guilde, savants, contacts, auberges, artisans, temple ou voisinage. Les relais tiennent compte du métier, du réseau de la source et du destinataire, du niveau de preuve et de la fraîcheur réelle du fait.   
- Les réseaux ont des délais différents : garde/guilde/contacts peuvent relayer immédiatement, commerce/savants/auberges prennent typiquement un jour, artisans/temple davantage. Une rumeur trop récente reste donc locale au lieu de se téléporter instantanément.   
- Une preuve locale forte peut maintenant affaiblir les variantes contradictoires plus faibles connues par CE PNJ, sans supprimer l’ancienne version ni corriger magiquement tous les habitants. Les contradictions restent inspectables avec leurs sources et leur niveau de confiance.   
- Les profils de groupe ennemis gagnent `coversRetreat` et `controlsTerrain`. Certains archers, sentinelles et protecteurs peuvent couvrir brièvement la fuite d’un allié ; la réaction a un vrai coût pour le couvreur et n’accorde pas une protection gratuite.   
- Les créatures de toile, racines, ronces, spores, boue, gel ou pièges peuvent utiliser leur tour de formation pour contrôler localement le terrain : affaiblissement bref et ouverture plus dangereuse, au prix de leur attaque directe.   
- `EnemyGroupBehaviorTest` vérifie les nouveaux rôles et `NpcInformationPropagationSystemTest` couvre les délais de réseau, les contradictions, l’absence de téléportation inter-ville et la correction locale par preuve forte.   
- Compilation complète C++17 et édition de liens validées avec `-Wall -Wextra`; `make test` repasse intégralement après adaptation des tests aux délais réels de transmission.   
- Aucun avancement de la progression principale après l’introduction du chapitre 3. La priorité reste systèmes vivants, contenu libre, cohérence, stabilité et séparation des gros fichiers.   


## V3.50.04 — Modularisation majeure, rumeurs contradictoires et monde vivant   
- Passe structurelle majeure sans aucun `.inc` : les gros fichiers ont été découpés uniquement en vrais modules `.cpp/.hpp` avec responsabilités identifiables et compilation séparée.   
- `Player.cpp` passe d’environ 8 394 à 4 507 lignes. Les compétences, cheats, suivi de challenges, coffres/stockage municipal, cycle de vie de l’équipement et résolution d’attaque sont maintenant dans `PlayerSkills.cpp`, `PlayerCheats.cpp`, `PlayerChallengeTracking.cpp`, `PlayerCityStorage.cpp`, `PlayerEquipmentLifecycle.cpp` et `PlayerCombatAttack.cpp`.   
- `MonsterPveMode.cpp` passe d’environ 4 917 à 3 608 lignes. Le mode coop PvE vit désormais dans `MonsterPveCoopMode.cpp` et le support bestiaire/journal/dialogues de vague dans `PveWaveNarrativeSupport.cpp`.   
- `ShopMenu.cpp` passe d’environ 8 800 à 6 123 lignes. Les services d’église et d’enchanteur ont été extraits dans `ChurchServiceMenu.cpp` et `EnchanterServiceMenu.cpp`, avec `ShopServiceSupport.cpp` pour les comportements communs. Les tests de serments ont été adaptés à cette architecture au lieu de dépendre de l’ancien fichier monolithique.   
- `QuestMenu.cpp` passe d’environ 21 500 à 16 457 lignes. Les prestataires/missions déléguées et sanctions de guilde sont dans `QuestContractorMenu.cpp`; toute la gestion équipe/recrues/clan/infirmerie/Torvald/ordre et missions de groupe est dans `QuestTeamMenu.cpp`. Les hooks encore partagés sont exposés proprement par leurs headers.   
- La mémoire des PNJ accepte maintenant plusieurs variantes contradictoires d’un même sujet au lieu d’écraser silencieusement la précédente. Chaque variante garde sa source, sa fiabilité, son niveau de preuve et son canal (`poste_de_garde`, marché, auberge, guilde, bibliothèque, temple, bouche-à-oreille, etc.). Le jeu peut donc signaler un fait contesté sans décider magiquement quelle version est vraie.   
- Le schéma de sauvegarde passe à 23 pour persister les variantes/canaux de connaissance PNJ. Les anciennes mémoires restent compatibles et reçoivent une variante par défaut sans inventer de nouvelles informations.   
- Les contenus vivants de biome sont étendus aux zones de début et milieu de jeu : Plaine sauvage, Route commerciale, Mares gélatineuses, Forêt ancienne, Montagne froide, Marais trouble, Cimetière oublié, Ruines effondrées, Bocage aux lanternes, Désert d’argile rouge, Quartier abandonné, Mine sifflante, Verger des lucioles de fer et Falaises des drakes gris disposent maintenant de davantage d’identité visuelle, dangers, ressources, faune neutre, traces sociales et détails inhabituels.   
- Les comportements de groupe ennemis gagnent une protection structurée du meneur blessé pour certaines formations compatibles, en complément des redditions, paniques et retraites déjà présentes.   
- Nettoyage des reliquats après extraction : helpers devenus inutiles supprimés, tests d’invariants mis à jour pour viser les nouveaux modules, compilation complète et édition de liens validées avec `-Wall -Wextra`.   
- Aucun avancement de la progression principale après l’introduction du chapitre 3. La priorité reste la vie du monde, les systèmes, le contenu libre, la stabilité et la modularisation.   

## V3.50.03 — Rumeurs locales, micro-événements et mémoire des objets   
- Ajout d’une circulation locale sourcée des informations entre PNJ. Un PNJ ne transmet que des faits qu’il connaît réellement, dans la même ville, avec perte de confiance à chaque relais et conservation de l’identité de la source précédente. Les métiers privilégient des sujets cohérents : gardes pour menaces/attaques/rivaux, érudits pour écrits/créatures inhabituelles, guildes pour quêtes et dangers, etc.   
- Les transmissions ne créent pas d’omniscience : une information ne se téléporte pas entre villes et une rumeur relayée devient explicitement une `rumeur_locale`. La répétition depuis la même source est bloquée afin d’éviter qu’un PNJ fabrique artificiellement de la certitude en rouvrant un menu.   
- Les PNJ capables d’initiative peuvent désormais engager occasionnellement eux-mêmes la conversation. Leurs introductions dépendent de leur métier, profil et souvenirs réellement possédés ; un PNJ distingue ainsi ce qu’il croit savoir d’un état global invisible du monde.   
- Ajout de micro-événements ambiants propres aux biomes : passage récent, terrain déplacé, faune active, ressource exposée ou détail inhabituel. Ils décrivent uniquement une situation observable au moment présent, sans prédire l’avenir, et peuvent légèrement modifier cette exploration. Un événement biome/jour déjà observé est journalisé et ne peut pas être exploité en rouvrant le menu.   
- Enrichissement du cycle de vie visuel des créatures avec descriptions spécifiques de mort, fuite et reddition selon leur grande famille et leur archétype. Une construction ne « meurt » plus comme un humain, un mort-vivant ne fuit pas avec le même langage corporel qu’un bandit, et une reddition conserve sa propre identité narrative.   
- La vente et le rachat d’équipement exploitent maintenant réellement l’identité persistante de l’exemplaire. Une arme ou armure vendue puis rachetée conserve son `persistentId`, reçoit des souvenirs de transfert puis de récupération, et peut faire émerger les surnoms `la Revenante` / `la Revenue` sans renommer toutes les copies du même objet.   
- Correction d’un défaut découvert pendant les tests de circulation PNJ : conserver un pointeur vers un fait puis enrichir le vecteur de mémoire pouvait invalider ce pointeur. La propagation copie maintenant le fait sélectionné avant toute mutation, supprimant ce risque de plantage.   
- Ajout de tests C++ légers pour la propagation locale des connaissances, l’absence de téléportation inter-ville, les descriptions de mort/fuite/reddition et les micro-événements. Les invariants projet vérifient aussi le branchement réel de ces systèmes et la mémoire de vente/rachat d’équipement.   
- Le contrôle final de portabilité couvre maintenant tous les scripts shell du projet ; un ancien CRLF restant dans `tools/linux/DinotofuLauncher.sh` a été normalisé en LF afin d’éviter une erreur de syntaxe sous Linux.   
- Aucun avancement de la progression principale après l’introduction du chapitre 3. La priorité reste les systèmes, la cohérence, la vie du monde, la variété et le contenu libre.   

## V3.50.02 — Mémoire PNJ, morale de groupe et réparation locale   
- Ajout d'une vraie mémoire factuelle persistante pour les PNJ. Un PNJ nommé peut retenir un fait avec source, sujet, lieu, premier/dernier jour de renforcement, confiance, niveau de preuve et nombre de fois entendu. Cette mémoire est sauvegardée avec le schéma 22 ; les anciennes sauvegardes ne reçoivent aucune mémoire inventée.   
- Les connaissances des PNJ restent volontairement non omniscientes. Témoignage du joueur, observation directe, preuve, registre et rumeur sont distingués ; entendre plusieurs fois exactement la même source ne fabrique pas magiquement une certitude.   
- Ajout du vieillissement de la mémoire : les preuves et interactions vécues résistent beaucoup mieux au temps, alors qu'un témoignage non confirmé ou une rumeur perd progressivement de la fiabilité effective. La source d'origine reste conservée.   
- Les clients de quêtes et contacts de boutique peuvent maintenant montrer des réactions issues de leur mémoire réelle. Accepter ou terminer une quête de client devient une interaction mémorisable, et le joueur peut transmettre volontairement un fait récent à un PNJ nommé.   
- Correction d'un vrai bug des langues : `studyProgress` était écrit dans la sauvegarde mais perdu au chargement. Les langues au niveau conversation peuvent désormais progresser vers courant grâce à des séances guidées à la bibliothèque ; le niveau 3 demande plusieurs pratiques et du temps, pas un achat instantané. La notation anormale reste non parlable couramment.   
- Extraction de la maîtrise des duos dans le vrai module `DuoMasterySystem`. Ajout de plans selon les rôles, dont Tank+Tank `Mur en mouvement`, Soutien+Soutien `Relais vital`, tir croisé et assaut sous garde, en gardant la consommation réelle des deux tours et les échecs de coordination.   
- Ajout de rôles de groupe structurés pour les ennemis : meneur, protecteur, membre coordonné, capacité de reddition et tendance à abandonner les blessés. La morale peut maintenant produire une vraie reddition, séparée de la mort et de la fuite : l'ennemi reste vivant, ne donne aucun butin de mort, donne seulement une expérience réduite et ne devient jamais rival juste parce qu'il s'est rendu.   
- Ajout d'un choc de groupe après la mort réellement observée d'un meneur compatible. Certains ennemis paniquent, s'affaiblissent ou cassent leur formation ; les plus lâches peuvent battre en retraite sans être promus artificiellement en rivaux. Correction de l'itération de combat pour qu'une reddition/fuite ne fasse plus sauter le tour de l'ennemi décalé à l'index suivant.   
- Ajout d'observations de biomes vivants : identité visuelle, danger de terrain, ressources visibles, vie neutre, traces sociales et signes inhabituels. Les observations tournent selon le jour courant sans annoncer l'avenir. Les toits d'assassins, nids draconiques, coulées de lave noire, bosquet féerique de mana, glacier des serments et autres zones avancées ont maintenant leur propre identité environnementale.   
- Enrichissement de sept pools de monstres communs encore trop pauvres avec de nouveaux humains, gobelins, bêtes et créatures corrompues qui profitent automatiquement des systèmes de langue, morale, groupe et descriptions.   
- Ajout d'une vraie réhabilitation de réputation locale. Un bureau municipal de médiation propose amendes réparatrices proportionnelles et service communautaire limité à une fois par jour, consommant du temps de jeu : une mauvaise réputation peut donc être réparée par du gameplay au lieu de rester seulement une surtaxe permanente.   
- Extension des tests round-trip avec mémoire PNJ, progression linguistique, cycle complet d'un rival, maîtrise de duo et mémoire d'un exemplaire précis d'équipement. Ajout de tests dédiés aux duos, à la reddition/profils de groupe ennemis et à la réparation de réputation.   
- Poursuite de la modularisation uniquement avec de vrais `.cpp/.hpp` ; aucun `.inc`. La progression principale reste bloquée juste après l'introduction du chapitre 3.   

## V3.50.01   
- Ajout de traces d'exploration multilingues persistantes dans plusieurs biomes avancés. Les écrits utilisent réellement la maîtrise des langues du joueur : incompréhensible, fragments, ou traduction complète.   
- Ajout d'une base structurée de PNJ vivants : métier, tempérament, langue native, capacité à initier une conversation, mémoire locale et réactions basées uniquement sur des faits connus.   
- Intégration des traces linguistiques dans l'observation prudente en exploration et dans l'historique du monde, sans omniscience.   
- Extension du round-trip de sauvegarde avec un rival complet (fuite, blessures, marque visible, retour) et la maîtrise persistante d'un duo allié.   
- Ajout de tests C++ légers pour le contenu vivant et les traces linguistiques.   
- La progression de l'histoire reste bloquée après l'introduction du chapitre 3.   

## V3.50.00 — Langues, rencontres vivantes et identité de l’exploration   

- Ajout d’un système de langues persistant. Au départ, chaque personnage maîtrise le Commun ainsi que sa langue raciale quand elle existe. Pour cette version, tout le monde garde bien le Commun au spawn comme demandé afin de ne jamais rendre le début de partie injouable. Les niveaux sont : inconnue, notions, conversation et courant/natif.   

- La bibliothèque vend désormais des initiations et cours avancés en gobelin, orc, infernal, draconique, elfique, elfique noir, céleste, féerique, kitsune, nain, gnome, halfelin, vampirique et spirituel. La notation anormale reste seulement reconnaissable par fragments : elle n’est pas traitée comme une langue normalement parlable.   

- Les adversaires doués de conscience peuvent maintenant faire une introduction ou une réaction avant certains combats, avec une fréquence liée à leur importance. Ils parlent réellement leur langue raciale : texte étranger si elle est inconnue, bribes avec de simples notions, sens compris à partir du niveau conversation. Les créatures sans conscience ne se mettent pas artificiellement à parler.   

- Certains contrats de guilde peuvent désormais contenir une annexe étrangère cohérente avec leur cible. La guilde refuse de faire signer un texte que le personnage ne peut pas vérifier : apprendre une langue à la bibliothèque a donc un vrai intérêt de quête. Les quêtes principales ne sont pas bloquées par ce système.   

- La maîtrise des langues et les métadonnées linguistiques des quêtes sont sauvegardées. Le schéma de sauvegarde passe à 21 ; une ancienne sauvegarde reçoit proprement le Commun et la langue raciale de départ. Les langues connues sont visibles dans l’écran de progression/statistiques.   

- Ajout de `MonsterFlavorCatalog` : les monstres nommés, espèces et attaques obtiennent des descriptions stables d’apparence, posture, mouvement et impact au lieu d’un même texte générique. La variation respecte la race, le type, l’état élite/évolué et les blessures visibles.   

- Davantage de monstres de bas et moyen niveau ont été ajoutés : nouveaux gobelins, bêtes, plantes, esprits, kobolds, hobgobelins, fées, orcs et constructions.   

- La carte du monde et les routes des villes exposent maintenant beaucoup plus de biomes qui existaient déjà côté combat mais restaient sous-utilisés : Marais trouble, Cimetière oublié, Archives noyées, Foire abandonnée, Jardin des statues qui pleurent, Sanctuaire kitsuné, Confluence du Mana pur, Archipel des îles flottantes, etc. Chacun reçoit des lieux et descriptions propres.   

- La logique de saveur des biomes d’exploration a été sortie de l’énorme `QuestMenu.cpp` dans un vrai module `.cpp/.hpp`. Les dialogues/langues de rencontre ont aussi été extraits de `MonsterPveMode.cpp`. Aucun `.inc` n’a été réintroduit.   

- Ajout de tests C++ légers pour les correspondances de langues et l’affectation thématique des contrats étrangers, plus des contrôles de projet sur le schéma de sauvegarde, les cours de bibliothèque, les rencontres multilingues et le blocage logique des contrats.   

- La progression de l’histoire reste volontairement limitée après l’introduction du chapitre 3. Cette passe développe les systèmes, l’exploration libre et le contenu sans avancer le scénario principal.   

---   

## V3.49.93 — Rivaux vivants, conséquences sociales et maîtrises persistantes   

- La fuite ennemie et la création d’un rival sont désormais deux résultats distincts. Une fuite ordinaire enregistre seulement la survie ; un rival persistant demande un vrai tirage fondé sur les traces vécues, le statut élite/évolué, les témoins, la mémoire et les serments concernés.   

- Même avec toutes les conditions favorables, la naissance d’un rival reste plafonnée à 65 %. Le Serment des Rivaux augmente la probabilité sans transformer automatiquement chaque fuyard en futur mini-boss.   

- La sensibilité à la peur vient maintenant du profil comportemental central. Les morts-vivants sans volonté, anomalies, constructions, spectres et serments tordus ne reçoivent plus artificiellement une peur humaine dans le tour de combat.   

- Les rivaux sauvegardent maintenant leur tempérament, leurs marques visibles, leur dernier résultat connu, la force de leur naissance et leur notoriété. Les blessures peuvent laisser des cicatrices, les retours rendent l’individu plus reconnaissable et sa mort reste définitive.   

- Les retours de rivaux respectent un délai minimal invisible et une probabilité décroissante. L’écran de mémoire montre seulement les faits connus, jamais la date du prochain retour ni une intention cachée.   

- Les techniques combinées alliées exploitent désormais l’expérience persistante du duo. La coordination peut échouer tout en consommant les deux tours, le Serment des Liens aide sans garantir et les duos atteignent des paliers de maîtrise commune après 3, 7 et 12 réussites.   

- Les exemplaires précis d’équipement mémorisent maintenant les frappes de Forge liée et les compétences signatures ennemies grâce à leur identifiant persistant. Réparations, impacts et coups liés répétés peuvent produire une renommée et un surnom émergents lors de l’inspection.   

- La mauvaise réputation locale provoque maintenant des réactions sociales visibles, des majorations de services, une pression réduite mais réelle dans les boutiques ordinaires et le refus de certaines ventes coûteuses lorsqu’un personnage est indésirable.   

- Ajout d’un vrai test C++ léger pour la naissance et les délais de retour des rivaux. Les contrôles couvrent aussi les fuites ordinaires, les deux CHANGELOG, la maîtrise des duos, la mémoire exacte des équipements et les effets de réputation négative.   

- Correction du warning lié au résultat ignoré de `system()` lors du nettoyage de la console et renforcement de la validation après reconstruction propre.   

- Normalisation de tous les scripts shell en fins de ligne Unix et ajout d’un contrôle anti-régression, afin que les commandes Linux de test et de publication ne cassent plus sur un retour chariot parasite.   

---   

## V3.49.92 — Consolidation persistante, contrats et contenu vivant   

- Reconstruction et consolidation de la branche de travail sur une base compilable afin de conserver les ajouts de mémoire persistante sans réintroduire les anciens PATCHNOTE.   

- Les serments d’église sont désormais traités comme de vrais contrats persistants : ils ne consomment plus de place dans les 10 passifs, ne peuvent pas être activés/désactivés depuis le loadout et un serment rompu ne peut pas être réactivé gratuitement.   

- Le journal canonique distingue mieux les événements locaux : une action attachée à une ville ne migre plus vers une autre ville lorsqu’une clé identique est réutilisée. Les catégories narratives importantes restent prioritaires lors du nettoyage du journal.   

- La réputation locale reste centralisée dans `LocalReputationSystem` afin que villes et boutiques lisent la même réputation positive ou négative.   

- Les rivaux possèdent une identité individuelle persistante, un historique de fuites/retours/blessures et peuvent revenir comme le même adversaire. Une mort enregistrée met fin à leurs retours vivants.   

- Les statistiques proposent une vue `Mémoire du monde / Rivaux` qui affiche uniquement les faits déjà vécus ou enregistrés, sans prédire les intentions ou prochaines actions des entités.   

- Les armes et armures disposent d’identifiants persistants et peuvent accumuler des souvenirs liés à l’exemplaire exact, notamment via la Forge liée, certaines attaques marquantes et des réparations importantes.   

- Ajout d’une vraie action de technique combinée pour deux recrues : elle exige deux alliés disponibles, consomme leurs deux tours, met leurs techniques en récupération et choisit un effet selon les profils du duo.   

- Le mode histoire normal est volontairement limité après l’introduction du chapitre 3. Les anciennes scènes ultérieures restent dans le code pour refonte future mais ne sont plus injectées dans une progression normale.   

- Ajout d’un `make test` léger couvrant version, CHANGELOG, absence de PATCHNOTE et `.inc`, mémoire/rivaux, identité d’équipement, combos alliés, serments-contracts, réputation locale et limite histoire.   

- Nettoyage des warnings préparatoires afin de viser une compilation `-Wall -Wextra` sans warning.   

---   

## V3.49.91 — Mémoire persistante, vrais rivaux et identité des équipements   

- Séparation du journal canonique agrégé et des événements historiques persistants : les compteurs servent aux statistiques tandis que les faits narratifs importants disposent désormais d’événements individuels sauvegardés avec identifiant, sujet, lieu, jour et état résolu/non résolu.   

- Migration automatique des anciennes traces narratives importantes vers la nouvelle mémoire persistante lors du chargement des sauvegardes antérieures compatibles.   

- Ajout de vrais rivaux ennemis persistants : un ennemi ayant réellement survécu peut recevoir une identité individuelle, conserver son origine, son niveau, ses statistiques, ses blessures, ses fuites, ses retours, sa dernière localisation et son état vivant/mort.   

- Un rival qui revient est le même individu, identifié par le moteur et sauvegardé ; s’il est vaincu, il est marqué mort définitivement et ne peut plus être réinjecté comme rival vivant.   

- Ajout d’une inspection « Mémoire du monde / Rivaux » dans les statistiques afin de consulter uniquement les faits déjà vécus ou connus, sans révéler les intentions futures des rivaux.   

- Ajout d’une identité persistante propre à chaque exemplaire d’arme et d’armure lors de son entrée dans un inventaire ; cette identité suit l’objet pendant les copies, transferts, équipements et sauvegardes pris en charge.   

- Le Serment de la Forge liée et les compétences signatures ennemies peuvent désormais attacher une mémoire à l’exemplaire exact d’une arme ou d’une armure au lieu de confondre tous les objets portant le même nom.   

- L’inspection d’une arme ou d’une armure affiche ses souvenirs individuels connus ; une réparation après forte usure, une réparation d’un objet déjà marqué ou une réparation sous Serment de la Forge liée peut devenir une nouvelle trace historique de cet exemplaire.   

- Ajout de la première vraie mécanique de techniques combinées des recrues : une consigne explicite engage deux alliés disponibles dans une action commune et consomme leurs deux tours au lieu d’ajouter une attaque gratuite.   

- Les combinaisons varient déjà selon les profils du duo : Brèche sous garde, Faille relayée, Feu croisé, Croisement de lignes ou Assaut synchronisé ; le Serment des Liens facilite légèrement la coordination des duos moins expérimentés sans garantir le résultat gratuitement.   

- Les techniques combinées réellement exécutées sont enregistrées dans la mémoire persistante avec l’identité des deux recrues, afin de préparer leur évolution future selon l’expérience commune.   

- Abandon des fichiers `.inc` temporaires : aucun `.inc` ne doit rester dans `src/` ou `include/`. La réduction de `QuestMenu.cpp` doit se poursuivre uniquement avec de vrais modules `.cpp/.hpp`.   

- Le mode histoire possède maintenant une limite de développement centralisée : la progression normale s’arrête volontairement à l’introduction du chapitre 3 avec un message de fin temporaire. Les chapitres et quêtes déjà écrits après ce point restent dans le code pour leur future réécriture, mais ne sont plus injectés dans une nouvelle progression normale.   

- Renforcement de `make test` : contrôle des modules QuestMenu, de l’absence de `.inc`, de la mémoire persistante, des rivaux, de l’identité des équipements, des techniques combinées, de la limite histoire et de la cohérence de version.   

---   

## V3.49.90 — Consolidation technique et documentation   

- Migration de l’historique détaillé des versions depuis les README vers ce CHANGELOG unique.   

- Remplacement des anciens fichiers PATCHNOTE par ce CHANGELOG comme source d’historique de version.   

- Séparation progressive du très volumineux `QuestMenu.cpp` en fichiers de détail thématiques sans changer le comportement du jeu.   

- Consolidation des serments d’église afin qu’ils soient traités comme des contrats persistants et non comme de simples passifs activables ou désactivables.   

- Première consolidation du journal canonique : les traces historiques importantes sont protégées du nettoyage automatique et certaines statistiques locales restent attachées à leur ville réelle.   

- Centralisation du calcul de réputation locale dans `CityTravelRules`, utilisé maintenant par les règles de ville et les boutiques afin d’éviter deux scores concurrents.   

- Nettoyage de plusieurs warnings de compilation liés à des helpers préparatoires ou variables inutilisées, sans modifier le gameplay.   

- Ajout d’une cible `make test` légère pour vérifier les invariants essentiels du projet sans créer une infrastructure de tests lourde.   

---   


## V3.49.89 — Rivaux et destin instable   

- La V3.49.89 ajoute les serments des Rivaux et du Destin instable. Les fuites, paniques et compétences signatures peuvent laisser des traces plus nettes de rival potentiel, tandis que le destin instable crée de petites oscillations seulement lorsqu'une trace existe déjà : mémoire, rupture, rival ou objet marqué.   


## V3.30.00 — routes et poids d’équipement   

- Les villes disposent désormais de distances entre elles, de distances vers les biomes et de conditions d’accès par niveau ou boss vaincu.   

- Les coffres restent indépendants par ville : le voyage change la ville active, mais ne mélange pas les contenus.   

- La carte d’exploration textuelle prépare les futurs fonds pixel-art par biome, avec lieux inconnus grisés ou enfumés.   

- Les armes et armures ont maintenant un poids léger, moyen ou lourd. Les bonus et contreparties sont appliqués au combat, aux dégâts et à la fuite, avec des malus volontairement modérés.   

## V3.31.00 — hubs de ville et journal canonique   

- La V3.31.00 rend la ville actuelle plus structurée : bâtiments locaux, contacts, verrous et indices pixel-art viennent maintenant des règles du monde plutôt que d'une liste fixe. Les destinations de ville fournissent des métadonnées IG structurées pour l'accès, la distance, le temps de trajet et le coût futur. Un journal canonique sauvegardé enregistre les événements importants comme les lieux visités, routes prises et mouvements de coffre, afin que les futurs Top 3 utilisent des données moteur au lieu de texte deviné.   

## V3.49.86 — Maîtrise tactique liée au build   

- La V3.49.86 relie davantage les maîtrises actives à la cohérence de l’équipement. Une arme ou armure cohérente avec la classe soutient légèrement les gestes liés, tandis qu’un [malus de classe] peut rendre une action moins propre malgré la maîtrise. L’audit de classe affiche aussi survie, critique et équipement attendu, et les passifs d’observation réduisent un peu les mauvaises surprises face aux compétences signatures ennemies.   

## V3.49.36 — Visée des créatures et textes plus immersifs   

- La V3.49.36 ajoute une lecture de visée aux profils ennemis : les petites créatures comme rats, fées, chauves-souris, insectes et profils sournois sont plus difficiles à cadrer, tandis que les brutes, constructions, dragons, plantes enracinées et gardiens ouvrent parfois des fenêtres plus faciles à toucher. L'observation et le bestiaire indiquent maintenant cette lecture de visée. Plusieurs textes en combat ont aussi été reformulés pour rester dans l'univers du jeu au lieu de parler comme une note de mise à jour.   

## V3.49.36 — Variantes ennemies et affinité féerique   

- La V3.49.36 étend les profils comportementaux avec des variantes plus précises : chauve-souris, rat/nuisible, chargeur massif, araignée, kobold, archer, alchimiste, fée, spectre, construction, dragon et serment sacré. Les profils ont maintenant une attaque signature, des réactions plus propres et une ligne de contre-jeu. Les fées gagnent aussi une vraie règle : 50% de résistance magique, mais 50% de faiblesse physique. Les parchemins offensifs comptent comme dégâts magiques pour que cette faiblesse/résistance soit réellement visible.   

## V3.49.34 — Profils de monstres et attaques signature   

- La V3.49.34 ajoute une première couche centrale de profils comportementaux pour les ennemis. Les slimes, voleurs, gobelins, brutes, prédateurs, gardiens, plantes, insectoïdes, supports et entités instables gagnent des descriptions d'attaque, forces, failles et réactions plus propres. L'observation active et le bestiaire affichent maintenant ces profils pour aider le joueur à comprendre pourquoi un ennemi agit différemment d'un autre.   

## V3.49.33 — Formations ennemies et rupture de formation   

- La V3.49.33 ajoute une couche de combat autour des tours de formation ennemie. Certaines vagues coordonnées peuvent maintenant utiliser couverture, précision courte, petite garde ou posture défensive au lieu d'attaquer basiquement. Le joueur reçoit la réponse **Casser la formation**, une action tactique qui perturbe plusieurs ennemis, peut retirer des postures défensives, et peut débloquer **Briseur de formation** après plusieurs vrais usages.   

## V3.49.31 — Brise-garde et posture de soutien   

- La V3.49.31 avait ajouté deux actions tactiques : **Brise-garde**, une action de contrôle courte avec affaiblissement/vulnérabilité, et **Tenir la ligne / couvrir**, une posture de soutien avec provocation courte, garde élémentaire et précision. Ces actions commençaient la progression passive vers **Casseur de garde** et **Rythme de soutien**.   

## V3.49.30 — Rumeur Hero Villager retardée et pression ennemie   

- La V3.49.30 corrige la rumeur Hero Villager trop précoce : la guilde ne parle plus de lui directement après l'inscription au jour 0. La rumeur et la rencontre rare sur route demandent maintenant une vraie progression : jours passés, niveau, contrats, histoire, actions tactiques ou observation. Les ennemis gagnent aussi une petite pression non basique : profils intelligents capables de feinte, créatures opportunistes qui exploitent les ouvertures visibles, et pression bestiale pouvant créer une vulnérabilité au lieu de toujours faire une attaque simple.   

## V3.49.29 — Préparation d'arme, lecture voleur et coffres reliés   

- La V3.49.29 ajoute l'action tactique **Enduire / fusionner vite l'arme**, qui consomme un composant pour appliquer un effet temporaire sur une cible : poison, choc, givre, vulnérabilité, précision ou puissance selon le matériau. Les dagues gagnent une synergie spéciale proche de l'idée de dague empoisonnée. L'observation active peut aussi apprendre **Lecture des serrures et failles** aux profils discrets, puis cette lecture aide réellement sur les coffres suspects d'exploration.   

## V3.49.28 — Ouvertures tactiques et artisanat de combat   

- La V3.49.28 enrichit le menu **Actions tactiques** avec **Exploiter une ouverture** et **Piège improvisé d'artisan**. Les blessures et statuts peuvent maintenant créer des réactions tactiques utiles, et certains petits matériaux deviennent consommables en combat pour gêner la ligne ennemie. Les actions tactiques font aussi progresser des passifs comme **Lecture du terrain** et **Improvisateur de combat**.   

## V3.49.27 — Audit retours, compagnon Dinotofu et combos de statuts   

- La V3.49.27 ajoute un premier **Compagnon Dinotofu** accessible depuis les activités et le menu hors combat. Il donne des conseils courts selon les PV, les quêtes, les lanternes, les compétences et le journal beta local. Les techniques d'arme profitent aussi de réactions de statuts, par exemple brûlure + givre, poison + saignement, choc + vulnérabilité, ou affaiblissement + vulnérabilité. Cette passe corrige aussi la cohérence des fichiers de version après la V3.49.26.   

## V3.49.26 — Actions tactiques, panneau de guilde et catégories rapides   

- La V3.49.26 ajoute le menu **Actions tactiques** en combat de vague : lancer une lanterne, jeter une lanterne au sol, repousser, utiliser la poussière ou observer activement. Le panneau de guilde utilise un libellé d'offre séparé du journal actif, affiche le rang et clarifie le cas même lieu. Le menu **Boutiques et comptoirs** gagne aussi plusieurs catégories rapides utiles.   

## V3.49.25 — Consignes alliées plus tactiques   

- La V3.49.25 ajoute des consignes de combat plus utiles pour les recrues : forcer une technique prête sur une recrue précise, demander une percée coordonnée de groupe, ou répartir les cibles pendant 1 tour contre plusieurs ennemis. Ces consignes ne consomment pas le tour du joueur, mais elles expirent après le tour allié, sauf la priorité de cible qui reste jusqu’à mort/disparition de la cible.   

## V3.49.25 — Variété combat et journal bêta   

- La V3.49.25 ajoute Rupture de ligne, Suture de fortune et Signal de focus, stabilise légèrement les attaques normales avec une énorme plage de dégâts, et ajoute un accès Journal bêta dans l’après-combat pour retrouver facilement `logs/dinotofu_session_latest.txt`.   

## V3.49.25 — Quêtes personnelles de recrues et contributions alliées plus riches   

- La V3.49.25 ajoute une première boucle de quêtes personnelles pour les recrues. Une recrue peut maintenant avoir un problème personnel lié à son profil, la progression peut se faire sur plusieurs tentatives, et la réussite améliore légèrement la loyauté tout en ajoutant de la réputation de clan. Les contributions alliées deviennent aussi plus lisibles : actions de soutien et coups de finition sont suivis, avec des techniques plus distinctes pour gardiens/roublards.   

## V3.49.19 — Dettes d’infirmerie bloquantes   

- La V3.49.19 rend les dettes d’infirmerie réellement contraignantes : tant qu’une dette existe, les soins payants sont bloqués. Si la dette totale dépasse 100 or, la récupération/réanimation d’une recrue prête à sortir est aussi bloquée jusqu’au remboursement partiel.   

## V3.49.17 — Infirmerie complète et auberge moins cheatée   

- La V3.49.17 ajoute un vrai service d’infirmerie : se soigner, soigner un membre d’équipe, ou gérer l’entrée/sortie des recrues KO. Les soins payants montent jusqu’à 90% PV maximum. L’auberge est rééquilibrée : les lits communs/simples plafonnent à 50% PV, tandis que la chambre sûre plus chère peut monter jusqu’à 90% sans full heal gratuit.   

## V3.49.16 — Recrues persistantes et évacuation infirmerie   

- La V3.49.16 continue d’améliorer les combats : les recrues ont maintenant de vrais PV persistants sauvegardés, plus des PV temporaires de combat. Elles peuvent commencer entre 70% et 100% de PV, garder leurs PV/potions entre les combats, et si une recrue tombe à 0 PV elle doit être amenée à l’infirmerie au lieu d’y être envoyée automatiquement. Si le joueur tombe aussi, plusieurs jours peuvent passer selon la gravité, et les recrues KO suivent le transfert vers les soins.   

## V3.49.11 — Recrues en combat PvE   

- Les recrues équipées commencent à agir dans les vrais combats PvE standard : soutien, dégâts simples, soin contextuel et partage de récompenses. Le joueur reste premier et conserve toujours la plus grosse part ; l’ordre manuel d’équipe reste prioritaire sur le tri automatique.   

## V3.49.70 — Marque mortelle collective de FireFlight   

- La V3.49.70 transforme la marque mortelle de FireFlight : elle ne vise plus une seule cible, mais tout le camp adverse en même temps. Pendant 2 tours de boss, chaque adversaire marqué qui tombe subit une vraie mort définitive, avec des lignes RP indiquant que FireFlight joue avec la panique du groupe entier.   

## V3.49.69 — Marque mortelle FireFlight et hésitation des recrues   

- La V3.49.69 transforme la fenêtre de panique de FireFlight en vraie **marque mortelle** ciblée sur le joueur. Pendant 2 tours de boss, tomber sous cette marque est traité comme une mort définitive et non comme une défaite non létale classique. La passe ajoute aussi une petite règle d’hésitation pour les recrues bas rang : sans ordre clair, elles peuvent renoncer à une technique trop avancée et revenir à une action plus simple.   

## V3.49.68 — Paliers variables et FireFlight mortel   

- La V3.49.68 corrige l’idée de plafond fixe : 10 niveaux actifs et 5 niveaux passifs restent seulement les maximums absolus. Les compétences simples peuvent avoir moins de paliers, et l’affichage montre maintenant le plafond local. FireFlight gagne aussi une fenêtre rare de 2 tours où les règles de mortel peuvent s’imposer temporairement, pour ajouter panique et stress sans changer la difficulté sauvegardée.   

## V3.49.67 — Progression actifs/passifs plus lisible   

- La V3.49.67 améliore les retours de progression des compétences. La maîtrise active affiche maintenant le prochain palier d’usage et le type de petit effet gagné, tandis que les passifs de maîtrise montrent leur progression en 5 niveaux et leur effet léger. Le menu des actions tactiques rappelle aussi que les paliers restent équilibrés : surtout fiabilité, précision, souffle, rythme ou contrôle léger, pas de montée cheat trop tôt. Les menus de consignes alliées rappellent aussi qu’une recrue faible rang comprend mieux un ordre clair qu’une intention de groupe trop vague.   

## V3.49.66 — Maturité de combat des recrues   

- La V3.49.66 ajoute une couche de maturité de combat aux recrues. Une recrue faible rang lit moins souvent le terrain, choisit des cibles plus simples, utilise rarement ses techniques avancées et profite davantage d’un ordre clair que de son instinct. Le rang, le niveau et l’équipement améliorent progressivement les réactions, commentaires tactiques, choix de cible et fiabilité des techniques pour mieux sentir son évolution.   

## V3.49.65 — Lectures vivantes ennemies et alliées   

- La V3.49.65 ajoute des lectures de combat plus contextuelles sans changer la structure des menus. Les lignes ennemies réagissent mieux à l’archétype, aux états visibles et au danger côté cible, tandis que les recrues peuvent commenter le premier profil ennemi, le danger sur le joueur, la pression de groupe et leur propre rôle ou race. **Monde / ville** gagne aussi de courtes lignes d’ambiance et de rumeur selon le moment, l’état du joueur et le contexte de quête.   

## V3.49.62 — Routage clair vers le menu de base   

- La V3.49.62 clarifie le flux après combat. **Continuer** signifie maintenant revenir au **Menu de base**. **Monde / ville** reste une activité normale du menu de base pour les lieux explorables, boutiques, guilde, PNJ et services, tandis que le menu rapide reste un hub constant pour personnage/session/sauvegarde.   

## V3.49.61 — Refonte du menu rapide et rangement personnage/monde   

- La V3.49.61 range les accès hors combat autour d’un menu rapide plus lisible. **Personnage** regroupe inventaire, titres, compétences actifs/passifs, quêtes acceptées, statistiques, équipement rapide, équipe et échange. Correction V3.49.61 : **Monde / ville** redevient une activité de lieux explorables accessible depuis les activités, pas depuis le menu rapide. L’après-combat renvoie vers le menu rapide pour le personnage/options/sauvegarde, ou vers Continuer pour retourner aux activités et visiter la ville.   

## V3.49.59 — Menu de charge et audit actifs/passifs   

- La V3.49.59 ajoute le menu hors combat de gestion des compétences dans le hub statistiques. Les actifs peuvent être équipés ou déséquipés, les passifs peuvent être activés ou désactivés, et les statistiques ouvertes pendant un combat restent seulement consultatives pour éviter de changer de build au milieu d’un tour. La passe vérifie aussi les techniques tactiques récentes pour que les identifiants d’actifs ne soient plus traités comme des effets passifs.   

## V3.49.58 — Charge de compétences et maîtrise progressive   

- La V3.49.58 ajoute une vraie base pour différencier les compétences connues, équipées et activées. Un personnage peut connaître plus de compétences, mais seules 10 compétences actives peuvent être équipées et seuls 10 passifs peuvent être activés en même temps. Les actifs restent des actions choisies, tandis que les passifs restent des effets automatiques ou semi-automatiques pouvant aussi servir hors combat selon leur nature.   

- Les actifs gagnent maintenant une maîtrise sur 10 paliers maximum, avec des seuils espacés pour ne pas devenir trop forts trop tôt dans une progression prévue jusqu'au niveau 255. Les passifs issus d'une pratique répétée doivent d'abord passer par trois essais réussis visibles avant de se débloquer réellement.   

## V3.49.57 — Séparation actif / maîtrise passive   

- La V3.49.57 clarifie la différence entre technique de combat sélectionnable et maîtrise passive. Les techniques d'affinité restent des actions actives du menu tactique, tandis que les vrais usages répétés débloquent désormais des maîtrises passives avec un nom séparé : **Maîtrise élémentaire**, **Garde circulaire**, **Trait entravant**, **Voix revigorante**, **Instinct canalisé** et **Rythme de lame**. Les anciens identifiants de sauvegarde restent reconnus, mais l'affichage ne donne plus l'impression que **Danse de lame** ou les autres techniques deviennent automatiques.   

## V3.49.56 — Techniques variées d'affinité   

- La V3.49.56 ajoute six techniques d’affinité supplémentaires pour éviter que les combats ne tournent seulement autour des ruptures/débuffs : **Lame élémentaire**, **Cercle protecteur**, **Flèche entravante**, **Chant revigorant**, **Instinct de bête** et **Danse de lame**. Les profils élémentaires, protecteurs, pisteurs, bardes/chefs, sauvages et duellistes gagnent chacun une option dédiée avec passif après vrais usages.   

## V3.49.55 — Affinités de classe élargies   

- La V3.49.55 ajoute cinq techniques d’affinité supplémentaires : **Rage maîtrisée**, **Ordre de bataille**, **Totem de souffle**, **Bombe d’atelier** et **Prière d’acier**. Cette passe élargit les classes spécialisées sans rendre toutes les actions universelles : front, commandement, nature, atelier et sacré gagnent chacun une option avec passif après vrais usages.   

## V3.49.53 — Techniques d’affinité de classe   

- La V3.49.53 ajoute une première couche de **techniques d’affinité de classe** : **Pas de l’ombre** n’est plus universel, et les profils sournois, soutien, arcanique, rempart et pisteur gagnent chacun une technique dédiée avec progression après vrais usages.   

## V3.49.50 — Allonge, remparts et ancrages   

- La V3.49.50 ajoute trois actions tactiques : **Rompre l’allonge**, **Percer le rempart** et **Rompre l’ancrage occulte**. Elle ajoute aussi les profils **Allonge prudente**, **Ancre nécrotique** et **Drain de vie**, avec réactions de distance, froid de tombe, récupération courte et contre-jeu par rupture d’espace ou d’ancrage.   

## V3.49.49 — Chaînes d’états, proies marquées et profils ennemis   

- La V3.49.49 ajoute trois actions tactiques : **Forcer une chaîne d’états**, **Marquer une proie** et **Retrait contrôlé**. Elle ajoute aussi les profils **Soigneur de fortune**, **Porte-bouclier**, **Berserker blessé** et **Porte-malédiction**, avec observations, faiblesses, attaques signatures et réactions mécaniques propres.   

## V3.49.38 — Taille, matière et résistance physique   

- La V3.49.38 ajoute une vraie lecture de résistance liée au corps des créatures : petit fragile, petit protégé, grande masse organique, grande masse dure, matière mauvaise, slime, spectre ou fée ne se comportent plus pareil face aux dégâts physiques. L'observation et le bestiaire affichent cette lecture, et le rapport de dégâts montre une phrase immersive quand le corps modifie l'impact. Les techniques de recrues gagnent aussi des gestes plus liés à leur profil.   

## V3.49.74 — Maîtrises élargies et préparation de sortie   

- La V3.49.74 continue les six chantiers actifs en même temps : plusieurs anciennes techniques d’affinité reçoivent de vrais effets de maîtrise, les passifs tactiques restent des soutiens et non des actions automatiques, la pression ennemie gagne des intentions contextuelles, les recrues utilisent davantage leurs techniques selon leur maturité, et Monde / ville gagne une lecture de préparation de sortie pour soin, outils, observation, coffres, artisanat combat et renforts mercenaires.   

## V3.49.73 — Passifs de maîtrise actifs et ville plus lisible   

- La V3.49.73 branche davantage les passifs de maîtrise dans les effets réels des actions tactiques : un passif débloqué et activé peut maintenant soutenir légèrement la puissance, la chance secondaire, la durée ou le rythme, sans jamais lancer l’actif à la place du joueur. Monde / ville affiche aussi plus de repères locaux : ville actuelle, inscription de guilde et temps de voyage.   

## V3.49.81 — Synergies lisibles et impact de maîtrise   

- La V3.49.81 ajoute une lecture globale du build dans le menu Personnage et la préparation de sortie : arme, armure, bonus/malus de classe et cohérence générale. Les maîtrises actives/passives utilisent des libellés plus clairs sur l’impact réel, les techniques de recrues profitent davantage de la maturité et des ordres, et les compétences signatures ennemies deviennent un peu plus présentes quand l’ennemi est entraîné, élite ou face à un joueur déjà ouvert.   

---   
