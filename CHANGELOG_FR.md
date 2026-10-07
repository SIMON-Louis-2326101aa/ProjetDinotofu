# Journal des modifications Dinotofu   

Historique détaillé des versions de Dinotofu en français. Le journal anglais équivalent se trouve dans `CHANGELOG.md`.   

## V3.50.36 — Notes de mise à jour   

- Correction de script de mise à jour.   
- Amélioration des scripts de compilations.   
- Compilation plus rapide.   

---   

## V3.50.35 — Notes de mise à jour   

- Les personnes peuvent rechoisir le mode d'affichage du jeu.   
- Correction de bug lors du lancement en mode graphique.   

---   

## V3.50.34 — Stabilisation du prologue et checkpoint important V3.50.33   

- **V3.50.33 devient officiellement le nouveau checkpoint important de sauvegarde.** Les personnages dont la dernière adaptation est antérieure à V3.50.33 passent par le backup pré-mise-à-jour non écrasant et le rituel de transition déjà existants ; une sauvegarde déjà adaptée en V3.50.33 ou plus récente n'est pas redemandée. Le schéma reste **26**.   
- Le combat du souvenir applique désormais les multiplicateurs officiels de difficulté aux **PV et dégâts de la meute** : Facile allège réellement le combat, tandis que Difficile/Cauchemar/Létal renforcent progressivement le Chef de meute et ses garde-crocs. La difficulté ne se limite donc plus presque uniquement aux chances de fuite dans ce prologue spécial.   
- Les transitions **victoire / retraite / défaite** sont davantage différenciées avant leur convergence vers la Brume : la retraite ressemble à une décision d'équipe expérimentée, tandis que la défaite insiste sur le fait qu'un compagnon revient physiquement chercher le joueur plutôt que de l'abandonner.   
- Le vocabulaire du nouvel effacement est harmonisé autour de la **Brume blanche** dans la séquence mémoire ; le contraste avec le réveil niveau 1 est renforcé par le réflexe corporel de chercher une arme et une puissance qui ne sont plus là.   
- `StoryPrologueMemoryTest` protège maintenant l'échelonnement réel de la meute selon la difficulté et l'existence de trois transitions distinctes, en plus des garde-fous déjà présents sur le build temporaire, les équipements et l'oubli des noms.   

---   

## V3.50.33 — Dernière chasse avant la Brume blanche   

- Le prologue d'ouverture est entièrement reconstruit comme un **souvenir jouable de la dernière journée du personnage avant l'effacement**, et non comme une interface informatique qui dysfonctionne. Les textes cassés, noms tronqués et informations manquantes représentent des trous de mémoire déjà rongés par la Brume blanche.   
- Le souvenir utilise une **copie temporaire avancée** du personnage créé : même nom, classe et identité, niveau mémoriel 42, statistiques renforcées, équipement de haut niveau cohérent avec la classe, consommables et compétences déjà acquises. Cette copie est isolée puis détruite après le souvenir ; elle ne modifie jamais le vrai personnage sauvegardé.   
- Le moteur de compétences accepte désormais un rafraîchissement silencieux réservé aux reconstructions temporaires : le build mémoriel dispose de ses techniques sans afficher de faux écrans « NOUVELLE COMPÉTENCE », tandis que le comportement normal des vrais personnages reste inchangé.   
- La sélection d'équipement du souvenir distingue les grandes familles réellement jouables : arbalétriers avec une vraie arbalète et des carreaux gelés, archers/tireurs avec arc et flèches, profils magie/invocation/soin avec bâton et robe, assassins avec dague, lanciers avec lance, profils lourds avec marteau, puis fallback runique pour les cas martiaux non spécialisés. Des tests représentatifs empêchent le retour d'un équipement manifestement incohérent.   
- La dernière mission mène vers une zone dangereuse déjà existante, le **Glacier des Serments froids**, pour chasser un **Chef de meute du Serment froid** accompagné de deux **Garde-crocs givrés**. Le combat utilise le vrai moteur de tours/vagues, pas une simple scène textuelle.   
- Trois approches précèdent l'affrontement — formation serrée, chasse rapide ou rythme habituel — et appliquent de petits avantages différents au groupe afin que le choix de voyage ait un effet concret sans décider automatiquement du combat.   
- Les deux alliés expérimentés possèdent des identités de conception fixes, mais leurs vrais noms sont volontairement absents du runtime. Le souvenir n'affiche que des variantes rongées comme « Sca— », « S…lett? », « Lor— », « L?ren… », « [nom arraché] » ou « [nom perdu] ». `scripts/test_project.sh` interdit explicitement toute fuite de leurs noms réels dans les fichiers runtime du prologue.   
- Pendant le combat, le joueur peut donner des **ordres de groupe** : concentrer le chef, achever la cible la plus faible, demander une couverture ou laisser les alliés agir librement. Les compagnons tirent, soignent, posent vulnérabilité/ward/régénération et déclenchent périodiquement des attaques coordonnées afin de donner l'impression d'une équipe qui se connaît déjà.   
- **Victoire, retraite volontaire et défaite sont toutes acceptées.** La victoire produit un bref relâchement ; la retraite/défaite force le groupe à décrocher. Les trois branches convergent ensuite vers la même montée de la Brume, qui efface objectif, route, équipements, compétences puis identités avant le blanc total.   
- Le combat-souvenir est volontairement **hors progression** : aucun XP, argent, butin, entrée de bestiaire ou statistique durable n'est accordé au personnage du présent. Les réactions personnalisées à la Brume des identités spéciales déjà existantes sont conservées.   
- Après l'effacement, la séquence de survie existante dans la forêt reprend avec le **vrai personnage débutant sans équipement**. Le kit trouvé ensuite sur le cadavre reste le kit générique lié à la classe et n'est pas présenté comme l'équipement avancé utilisé dans le souvenir.   
- `HISTOIRE_DINOTOFU.txt` documente désormais explicitement la chasse, l'effacement progressif, les identités de conception des deux alliés, les trois issues admises et la séparation stricte entre build mémoriel et progression réelle.   
- Nouveau module `StoryPrologueMemory` et nouveau `StoryPrologueMemoryTest` : création du personnage temporaire, meute, masquage des alliés, fragments de mission et transition de Brume sont protégés par tests. La suite globale `scripts/test_project.sh` passe jusqu'au message final **« Tous les tests Dinotofu sont passés. »** avant versionnement.   
- Schéma de sauvegarde **26 inchangé** ; checkpoint important toujours **V3.50.30**.   

---   

## V3.50.32 — Récompenses exactes et bureau de change volontaire   

- Les quêtes peuvent désormais définir une **prime en piles de pièces exactes** via `Quest::rewardCoins`. Lorsqu'une quête promet par exemple **17 PC + 2 PF + 3 PO**, ces pièces précises sont ajoutées telles quelles au porte-monnaie : aucune normalisation ou conversion silencieuse n'est appliquée. Les récompenses historiques exprimées en valeur économique conservent leur comportement compact séparé.   
- Les primes exactes sont sérialisées avec la quête (`rewardCoinCopper`, `rewardCoinIron`, `rewardCoinElectrum`, `rewardCoinGold`, `rewardCoinPlatinum`) et survivent à un aller-retour sauvegarde/rechargement sans changer de dénomination. Ces champs sont optionnels, donc le schéma de sauvegarde reste **26**.   
- Le **Bureau de change de la guilde** propose maintenant trois intentions avant toute sélection de pièce : **Conversion personnalisée**, **Tout vers les pièces les plus élevées**, ou **Tout vers la pièce la plus faible**. La « bourse idéale » reste donc disponible, mais uniquement sur décision du joueur.   
- La conversion personnalisée permet de choisir directement la **pièce source**, la **pièce cible**, puis le **nombre de conversions**. Le menu affiche le lot exact requis/produit et le **maximum possible** avant la saisie ; les conversions peuvent sauter plusieurs rangs (par exemple PC -> PO ou PO -> PF) sans perte de valeur.   
- **Tout vers les pièces les plus élevées** compacte volontairement la richesse en PP/PO/PE/PF avec le reliquat PC exact ; **Tout vers la pièce la plus faible** transforme volontairement toute la bourse en PC. Dans les deux cas le total en cuivre est contrôlé avant/après.   
- Le jalon **« Premier éclat de platine »** ne dépend plus d'un futur rafraîchissement de compétences : les gains, ventes, butins, récompenses et conversions monétaires déclenchent maintenant un rafraîchissement dédié des titres économiques. Obtenir ou fabriquer sa première PP suffit donc immédiatement.   
- `EconomyScaleTest`, `SaveRoundTripTest` et `scripts/test_project.sh` protègent désormais les conversions arbitraires, les deux normalisations volontaires, les primes de quête exactes, leur persistance et le rafraîchissement du titre platine.   
- Build C++23 complet validé ; les tests économiques et de sauvegarde ciblés passent, puis l'intégralité de `scripts/test_project.sh` a été validée jusqu'au message final **« Tous les tests Dinotofu sont passés. »** (en deux exécutions à cause de la limite de durée de l'environnement).   
- Schéma de sauvegarde **26 inchangé** ; checkpoint important toujours **V3.50.30**. Aucun scénario principal n'est avancé : **V3.50.33 reste réservée à la grosse mise à jour / prologue pré-Brume**.   

---   

## V3.50.31 — Monnaie physique, statut social et départ modeste   

- Le porte-monnaie de départ Normal n'est plus 50 PF abstraits : il contient physiquement **5 PF + 15 PC**, soit **65 PC** exacts. Les autres difficultés utilisent elles aussi des piles de départ physiques.   
- Les gains exprimés par les anciens systèmes en unités économiques sont désormais versés en **dénominations physiques compactes** sans normaliser les piles déjà possédées : 13 PF de valeur arrivent comme 1 PE + 3 PF, 100 PF comme 1 PO.   
- Les pièces prennent une vraie signification sociale : cuivre = moyens modestes, fer = monnaie courante, électrum = aisance visible, or = notable/noble potentiel, platine = fortune exceptionnelle. Les boutiques et l'inventaire exposent cette perception à partir des vraies piles détenues.   
- Nouveau titre secret **« Premier éclat de platine »**, accordé lorsqu'un personnage possède pour la première fois une pièce de platine.   
- Les menus utilisant le portefeuille courant affichent désormais les **piles physiques réelles** au lieu de recalculer une décomposition théorique depuis le total.   
- La sélection des personnages affiche le **temps total de jeu et la richesse exacte en PC côte à côte** ; `CharacterSaveSummary` lit maintenant `totalCopperCurrency`.   
- Références de petit train de vie recalibrées vers une économie médiéval-fantasy plus serrée : à Valebrume niveau 1, repas chaud **50 PC**, lit commun **100 PC**, chambre sûre **192 PC**. Avec 65 PC au départ, le joueur peut manger mais doit déjà gagner sa nuit.   
- Les garde-fous de `EconomyScaleTest` vérifient désormais le départ physique, la hiérarchie sociale, les paiements PE/PO naturels et le pouvoir d'achat en cuivre exact.   
- Schéma de sauvegarde **26 inchangé** ; checkpoint important toujours **V3.50.30**. **V3.50.32** reste disponible pour stabilisation et **V3.50.33** est réservée à la grosse passe/prologue pré-Brume.   

---   

## V3.50.30 — Checkpoint économique majeur : pouvoir d’achat, unités et garde-fous   

- **Audit économique transversal terminé** : revenus de quêtes, combats ordinaires, boss, PvP, exploration, ventes/rachats, services de ville et d’église, entraînement, réputation, pénalités de mort, transports, auberges et coffre municipal ont été relus sur la même échelle. L’audit confirme que l’économie actuelle est globalement cohérente en **unités PF** ; aucun rescale global x10/x100 n’est appliqué, afin de ne pas détruire des rapports de prix déjà corrects pour quelques outliers supposés.   
- Le pouvoir d’achat est désormais protégé à **5 jalons de progression : niveaux 1, 5, 10, 25 et 50**. `EconomyScaleTest` échantillonne les panneaux de guilde, contrôle leurs récompenses moyennes et maximales, puis vérifie qu’un portefeuille représentatif débloque progressivement davantage d’offres de boutique sans rendre tout le catalogue trivial.   
- Le **coffre municipal** reste explicitement un objectif de moyen terme : il est hors de portée d’un portefeuille représentatif de niveau 10 après quelques contrats, mais devient raisonnablement atteignable à progression plus avancée.   
- Les récompenses de **combat** sont intégrées au même audit : un humanoïde ordinaire reste de l’argent de poche cohérent avec son niveau, tandis qu’un boss étalon paie sensiblement davantage sans financer instantanément les achats structurants. Les gains PvP amicaux restent symboliques et les duels à butin dangereux transfèrent la richesse du perdant au gagnant au lieu de créer de la monnaie.   
- Les jackpots d’exploration et les bourses de butin ont été contrôlés avec leurs caps/chances actuels. Ils restent volontairement excitants mais ne justifient pas un changement d’échelle global ; les soft caps d’exploration continuent d’absorber les valeurs extrêmes.   
- Plusieurs **affichages trompeurs** ont été corrigés : le stand d’entraînement, les listes d’équipement, certaines sélections d’inventaire, les potions, le rachat marchand et le cheat d’argent n’affichent plus une valeur PF brute comme si elle était directement en `PO`/« or ». Ils passent maintenant par `Money::formatEconomyUnits`.   
- Nettoyage des noms d’API qui pouvaient recréer une confusion d’unité : `starterGold` devient `starterEconomyUnits`, `getStarterGold` devient `getStarterEconomyUnits`, `fineCostGold` devient `fineCostEconomyUnits`, `CombatReward` stocke/expose des `economyUnits`, les pourcentages de difficulté parlent explicitement de récompense/perte économique et `DeathPenaltyResult` expose `lostEconomyUnits`. Le champ historique `Quest::rewardGold` reste volontairement inchangé car il est sérialisé ; son commentaire rappelle qu’il représente des unités PF et non des PO.   
- La hiérarchie monétaire est verrouillée jusqu’aux grosses valeurs : **1 unité économique = 1 PF = 10 PC**, **100 unités = 1 PO**, **1000 unités = 1 PP**. Les prix exacts déjà conçus en cuivre (PC) restent exacts et ne sont pas réinterprétés comme des unités PF.   
- `scripts/test_project.sh` refuse désormais la réintroduction de plusieurs anciens noms ambigus et de certains affichages `PO/or` bruts dans les menus économiques. Il vérifie également que les README suivent réellement le checkpoint important au lieu de pouvoir rester bloqués sur un ancien jalon.   
- `bump_version.py` synchronise maintenant les lignes de checkpoint des README par motif de statut courant, même si leur valeur avait dérivé d’un ancien checkpoint ; les entrées historiques de changelog ne sont jamais réécrites.   
- Build C++23 complet et suite `scripts/test_project.sh` : **tous les tests passent avant le versionnement 3.50.30**. Une seconde validation post-versionnement protège le nouveau checkpoint.   
- Schéma de sauvegarde **26 inchangé**. **V3.50.30 devient le nouveau checkpoint important obligatoire** : les personnages plus anciens passent par le backup pré-mise-à-jour non écrasant et le rituel de transition déjà existants.   
- Aucun avancement du scénario principal dans cette passe. Le **prologue pré-Brume reste réservé à V3.50.31**, comme prévu.   

---   

## V3.50.29 — Pré-checkpoint : personnages spéciaux, navigation, combat lisible et unités explicites   

- **Willow**, **Dwarf** et **Badr** rejoignent le catalogue des personnages spéciaux avec la date protégée commune **15/12/2025**. Willow est une humaine archère de 20 ans, solitaire et râleuse ; Dwarf un guerrier nain de 24 ans, blagueur provocateur dont les défauts peuvent être désapprouvés par les autres ; Badr un clerc semi-humain de 48 ans, croyant, inquiétant et lié au ver organique **Second**.   
- Le modèle `SpecialCharacter` peut désormais porter un âge et un genre connus sans obliger les anciens personnages à en posséder. Les nouveaux profils sont couverts par le garde de nom, la validation de date, les dialogues, le bonus de classe native et le bestiaire progressif.   
- Le trio **Willow / Dwarf / Badr** et deux sous-groupes cohérents rejoignent les rencontres spéciales. Les dialogues de groupe couvrent entrée, victoire et défaite. Des synergies modestes rendent le trio utile : Dwarf crée des ouvertures pour Willow, Badr stabilise ses alliés et peut leur rendre quelques PV. La compétence **« En avant Second »** reste volontairement réservée à une future passe.   
- La sélection Terminal devient plus tolérante : les deux lecteurs de menus comprennent désormais `aide`, `?`, `retour`, `r` (quand 0 est une sortie valide), rappellent les choix/plages autorisés après une erreur et réaffichent les descriptions sur demande. Les anciens menus personnage/compte bénéficient donc aussi de cette ergonomie.   
- Le menu de combat affiche un résumé des états actifs du joueur et détaille la posture défensive encore en attente. Une posture déjà active est clairement signalée afin d'éviter de la remplacer sans comprendre ce qui restait.   
- Préparation du checkpoint économie V3.50.30 : plusieurs fonctions retournant du **cuivre exact (PC)** ont reçu un suffixe `Copper` explicite (`innCommonBedCostCopper`, `innSafeRoomCostCopper`, `innWarmMealCostCopper`, `cityVaultMaterialTransferCostCopper`, `routeRewardBudgetCopperForDistance`). Les achats/améliorations de coffre restent documentés comme **unités économiques PF**.   
- `EconomyScaleTest` protège maintenant plusieurs repères concrets : lit/repas accessibles avec le portefeuille de départ normal, achat de coffre municipal restant un objectif de moyen terme, et séparation claire entre coûts PC exacts et unités PF.   
- Nettoyage d'une duplication accidentelle dans le hook de combat de Sanctus détectée pendant l'audit.   
- `scripts/test_project.sh` protège la navigation Terminal, la lisibilité des états de combat, les suffixes économiques explicites et l'intégration du nouveau trio.   
- Build C++23 complet et suite `scripts/test_project.sh` : **tous les tests Dinotofu passent** avant versionnement.   
- Schéma de sauvegarde **26 inchangé** ; checkpoint important toujours **V3.50.20**. **V3.50.30 reste réservé au gros audit/checkpoint économique.**   

---   

## V3.50.28 — Monnaie physique, mémoire dosée, craft visible et défense préparée   

- Les interactions non-combat ne rejoignent plus systématiquement l'historique durable du personnage. Le journal local/anti-farm continue de suivre les interactions résolues, mais seules quelques découvertes réellement inhabituelles sont désormais marquées comme `notableForLongTermHistory`. Une action banale, isolée ou sans témoin ne devient donc plus artificiellement un événement important du monde.   
- La guilde reçoit un **Bureau de change** qui manipule les piles physiques sans créer ni détruire de valeur : le joueur peut casser une pièce d'un rang en dix pièces du rang inférieur, ou regrouper dix pièces en une pièce du rang supérieur. Le total en cuivre est vérifié avant/après chaque opération.   
- La légende complète des monnaies n'est plus répétée partout. Elle est centralisée au Bureau de change : **Pièce de cuivre (PC), Pièce de fer (PF), Pièce d'électrum (PE), Pièce d'or (PO), Pièce de platine (PP)**, avec la règle **10 pièces d'un rang = 1 pièce du rang supérieur**. Les autres écrans peuvent continuer d'utiliser naturellement abréviations ou noms selon le contexte.   
- `Money` et `Inventory` exposent maintenant une API explicite de dénominations physiques (`CoinType`, comptage, rang inférieur/supérieur, cassage/regroupement), afin que les futurs systèmes économiques ne soient pas obligés de réinventer les conversions.   
- Correction d'un reliquat économique dans les données structurées de l'inventaire : armes, armures, consommables et matériaux ne présentent plus une ancienne valeur d'unité économique comme si elle était directement en « or ». Ils utilisent maintenant les mêmes formats économiques que l'interface Terminal.   
- L'**artisanat/craft** devient découvrable sans devoir deviner son existence dans l'inventaire : le menu Personnage possède un accès direct « Artisanat / craft », les descriptions de navigation le mentionnent, et le menu rapide a été renuméroté de façon continue au lieu de sauter artificiellement vers 8/9.   
- En combat, la **posture défensive** s'applique désormais aussi aux compétences ennemies préparées/télégraphiées (charge, souffle, tir lourd, rituel, etc.). Les dégâts passent par `DefensePostureSystem` avant d'être appliqués. Les effets secondaires propres à la compétence peuvent toujours se produire : défendre atténue l'impact, mais n'annule pas gratuitement toute la mécanique.   
- Tests renforcés : conservation exacte de la valeur lors des conversions physiques, dénominations/abréviations, unicité de la légende monétaire complète, visibilité du craft, distinction interaction locale / événement durable et réduction réelle d'une attaque préparée sous posture défensive.   
- Build C++23 complet et suite `scripts/test_project.sh` validés.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**). La version **V3.50.30** est volontairement réservée comme future grosse passe/checkpoint d'audit économique, après une V3.50.29 de préparation si nécessaire.   

---   

## V3.50.27 — Jardin des statues pleureuses, diversité de guilde et traces d'exploration   

- Le **Jardin des statues qui pleurent** reçoit enfin une mécanique signature propre au lieu : certaines statues peuvent changer de position uniquement entre deux observations, sans trace de déplacement dans le gravier.   
- Un événement spécial du Jardin permet de marquer physiquement les positions, prélever une larme minérale sans toucher aux statues, approcher directement le cercle de pierre ou quitter prudemment les lieux. Les conséquences diffèrent et peuvent déclencher observation, ressource, progression de quête ou combat.   
- Les observations répétées du Jardin sont persistantes via l'historique du personnage. Après plusieurs relevés, le système peut confirmer un déplacement impossible au lieu de traiter chaque visite comme une scène sans mémoire.   
- Les descriptions de lieu dangereux, avertissement, trace de boss, obstacle environnemental et observation de terrain du Jardin ont été spécialisées pour renforcer son identité.   
- Une nouvelle interaction non-combat **Bouquet devant l'ange** ajoute des traces humaines récentes au mystère : comparaison des pas, offrande déplacée ou observation sans intervention.   
- Six biomes supplémentaires reçoivent une interaction non-combat propre : **Mares gélatineuses** (migration de slimes neutres), **Montagne froide** (cairn écroulé), **Ruines effondrées** (mosaïque sous structure instable), **Canaux de brume bleue** (barque sans passeur), **Foire abandonnée** (manège d’un seul tour) et **Carrière des os blancs** (marque de taille récente).   
- Les interactions non-combat de biome résolues laissent désormais aussi une trace dans l'historique du personnage, en plus du journal canonique anti-farm.   
- Le panneau de guilde maximise maintenant la diversité des familles d'objectifs disponibles. Lorsqu'au moins trois offres sont affichées et que le catalogue le permet, le panneau cherche à mélanger au moins trois types parmi combat, exploration, service et bestiaire.   
- Les petits panneaux de trois offres ne sont plus forcés à contenir trois missions terrain : à partir du niveau 7, ils conservent au moins deux missions combat/exploration et libèrent une place pour un service ou un dossier de bestiaire. Les panneaux plus grands conservent le minimum de terrain existant.   
- Tests ajoutés/renforcés : identité du Jardin, contenu non-combat propre au Jardin, diversité du panneau sur plusieurs niveaux et conservation du minimum de missions terrain.   
- Build C++23 complet validé. La suite `scripts/test_project.sh` a été exécutée par tranches à cause de la limite d'exécution de l'environnement et toutes ses sections ont passé.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.26 — Mémoire sociale des quêtes   

- Les quêtes expirées ne disparaissent plus socialement après leur archivage : le client concerné enregistre désormais un fait `quest_failed` dans sa mémoire locale.   
- Les échecs de délai sont aussi inscrits dans le journal canonique local via `quetes_echouees`, avec protection contre le double comptage lors des synchronisations répétées.   
- Les quêtes rendues enregistrent désormais explicitement `quetes_reussies`, en complément de la mémoire PNJ `quest_completed` déjà existante.   
- Les PNJ réagissent différemment à un échec selon leur profil : garde, marchand, guilde/intendance, tempérament méthodique ou comportement générique. Un échec n'entraîne pas automatiquement une hostilité ou un gros malus arbitraire.   
- Refuser une demande personnelle crée maintenant un souvenir `quest_declined` : le contact peut se rappeler que le joueur n'était pas disponible, mais distingue clairement ce refus d'une promesse rompue.   
- Les réseaux d'information locaux peuvent relayer certains échecs de quête lorsque leur profession s'y prête. La transmission reste sourcée, locale et perd en certitude comme les autres rumeurs ; aucune omniscience n'est ajoutée.   
- Tous les chemins principaux qui font avancer le temps et expirent des quêtes synchronisent maintenant ces conséquences sociales.   
- Tests ajoutés : expiration -> mémoire client, journal local idempotent, réaction PNJ et relais de `quest_failed`.   
- Build C++23 complet et suite `scripts/test_project.sh` validés.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.25 — Priorité locale du panneau de guilde   

- Le panneau de guilde ne se contente plus de vérifier que le niveau d’un lieu est compatible : les offres déjà générées sont maintenant ordonnées selon leur **cohérence avec la ville actuelle**.   
- Les services au comptoir restent naturellement locaux, même lorsqu’un dossier mentionne un objet ou un document provenant d’une zone éloignée. Les contrats génériques de village, route ou famille de créatures restent également bien placés tant qu’aucun biome précis n’impose un long déplacement.   
- Lorsqu’un contrat nomme un biome concret, le système consulte les distances régionales de la ville : les biomes proches et déjà connus passent avant les zones plus éloignées ou moins connues. Un contrat lointain reste possible ; il est simplement moins prioritaire au lieu d’être supprimé artificiellement.   
- Cette pondération complète le filtre V3.50.21 sur le niveau minimum réel des lieux : un débutant évite donc à la fois les zones trop hautes et, parmi les lieux autorisés, les propositions inutilement éloignées.   
- La logique est recalculée à l’ouverture du panneau depuis la ville courante, ce qui permet au même ensemble d’offres de retrouver un ordre plus logique après un changement de ville sans modifier la structure de sauvegarde.   
- Test ajouté : depuis Valebrume, une mission locale de Plaine et un service au comptoir sont prioritaires face à une exploration des Falaises des drakes gris.   
- Build C++23 et suite de tests complète validés avant versionnement.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.24 — Accueil graphique minimal et bascule propre vers le Terminal   

- L’ancienne grosse interface HTML expérimentale a été retirée de l’accueil provisoire. La page graphique ne conserve plus que l’identité visuelle officielle, la bannière, le logo et le bouton **Jouer**.   
- **Jouer** n’essaie plus d’ouvrir une pseudo-interface de partie : il affiche clairement **Interface graphique en production** puis propose **Arrêter et passer à la version terminale**.   
- Le placeholder graphique est passé d’environ 6 300 lignes de HTML/JS à une page légère d’environ 140 lignes, sans panneaux de combat, inventaire, quêtes ou polling de snapshots inutiles pendant la reconstruction.   
- Un endpoint local `/gui/switch-terminal` crée une demande de bascule, vide les anciennes entrées GUI puis arrête proprement le petit serveur local après avoir répondu au navigateur.   
- Les launchers Linux et Windows surveillent cette demande : ils ferment le placeholder puis lancent réellement la version Terminal. Le moteur C++ de partie n’est plus démarré caché en arrière-plan simplement pour afficher l’accueil graphique.   
- Le mode Terminal reste le mode par défaut. Le serveur conserve provisoirement ses anciens endpoints de debug pour les outils de développement, mais l’accueil public ne les utilise plus.   
- Les README et le guide d’interface distinguent maintenant explicitement l’état actuel minimal de la cible future : une vraie application desktop, plutôt qu’une seconde grosse interface web maintenue en parallèle.   
- Ajout de gardes de non-régression pour la bascule IG → Terminal et pour empêcher le retour du moteur caché derrière le placeholder. L’endpoint a également été testé avec création réelle du signal et arrêt du serveur.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.23 — Catégories contextuelles des épreuves de service   

- Les épreuves intellectuelles des contrats de service affichent maintenant une **catégorie de service** explicite liée à l’action réellement demandée : transport et logistique, estimation et négociation, registre et administration, calcul commercial, français et rédaction, équipement et morphologie, inventaire et logistique, procédure de guilde, etc.   
- Les familles internes déjà utilisées par les dossiers marchands deviennent visibles au joueur au lieu de rester une simple règle de sélection cachée. Les autres épreuves déduisent leur catégorie depuis leur titre et leur question.   
- L’écran explique également que l’épreuve correspond à l’étape concrète du contrat en cours, afin d’éviter l’impression d’un quiz aléatoire posé hors contexte.   
- Les épreuves génériques de service reçoivent désormais un identifiant stable dérivé de leur titre et utilisent `serviceChallengeHistory`. Une question déjà vue pour ce contrat est évitée tant qu’il reste des questions inédites dans le pool pertinent.   
- Lorsque toutes les épreuves adaptées au contrat ont été vues, l’historique local peut repartir proprement au lieu de bloquer la progression. Le système marchand existant conserve sa sélection spécialisée et ses questions rares/confidentielles.   
- `scripts/test_project.sh` protège la présence de la catégorie de service et de la logique `serviceChallengeCategory`.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.22 — Tarifs RP, affichage monétaire cohérent et garde anti-inflation   

- Audit des paniers de début de jeu après les normalisations V3.50.19/V3.50.20 : les montants principaux restent volontairement sur l’échelle PF actuelle. L’Arc d’entraînement reste autour de 85 PF, l’Arc de chasse autour de 205 PF, la ration autour de 15–17 PF et les petites potions autour de 12–13 PF ; aucune nouvelle division globale n’a été appliquée.   
- Les quêtes de départ sont désormais protégées par des tests plus stricts : les contrats F niveau 1 restent sous 20 PF et les petits services F sous 5 PF ; les contrats E proposés au niveau 2 restent sous 30 PF. Cela empêche le retour involontaire des primes absurdes en PO/PP sur des tâches locales.   
- Les armuriers, boutiques d’armes et forgerons utilisent maintenant un **tarif d’artisan** : après les modificateurs de race, classe, ville, crise ou promotion, ils peuvent arrondir le prix réellement demandé à une dénomination simple lorsque l’écart reste inférieur à environ 12 %. Exemple : 205 PF peut devenir exactement 2 PO.   
- Les autres comptoirs gardent un **montant exact** et rendent la monnaie si nécessaire. Le texte de boutique indique le style de tarification utilisé afin que le joueur sache si le marchand annonce un prix rond ou compte précisément les pièces.   
- L’arrondi marchand modifie le prix réellement débité, pas seulement son affichage. Les tests vérifient aussi que cet arrondi ne crée pas de revente plus rentable que l’achat sous les principaux modificateurs de race/classe.   
- Nettoyage des anciens affichages « X pièces » ambigus : fiches d’objets, armes, armures, consommables, matériaux, journal de matériaux, inventaire, échanges entre personnages, loot monétaire, estimation PvP, coffre municipal et transfert de rune utilisent désormais `Money::formatEconomyUnits(...)` lorsque la valeur est exprimée en unités économiques.   
- Les valeurs physiques/lore qui parlent réellement de pièces au sens générique restent inchangées. Les cheats explicitement décrits en pièces d’or restent également des PO historiques explicites.   
- Ajout de gardes de non-régression sur les prix de quelques références de début de jeu : Arc d’entraînement <= 100 PF, Épée rouillée <= 80 PF, Armure de cuir usée <= 100 PF, Petite potion <= 20 PF et Ration de survie <= 20 PF.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.21 — Quêtes cohérentes, variété des épreuves et autonomie d’exploration   

- Les contrats de guilde tiennent désormais compte du **niveau minimum réel des lieux explicitement cités**, en plus du rang et du niveau propre au modèle de quête. Les offres F/E ne peuvent donc plus envoyer un débutant fouiller directement une zone prévue plusieurs paliers plus haut. Les services réalisés en ville restent exemptés lorsqu’ils manipulent simplement un objet venant d’une zone dangereuse.   
- La localisation suggérée des quêtes a été rendue plus précise : une zone concrète citée par le contrat reste prioritaire, tandis que les routes, relais et livraisons ne sont plus confondus avec des ruines à cause d’un mot générique.   
- Une quête de **fouille/exploration qui nomme un biome précis** ne progresse plus dans un autre biome à cause d’un mot vague comme « plantes », « traces » ou « matériaux ». Les chasses restent volontairement basées sur la famille de cible : vaincre le bon type d’ennemi ailleurs continue donc de compter lorsque cela reste logique.   
- Les petites épreuves d’exploration possèdent maintenant une **catégorie contextuelle** (orientation, observation, survie, cartographie, météo, premiers secours, logistique, pistage, etc.). Douze nouvelles épreuves génériques ont été ajoutées et le choix évite aussi les catégories récemment vues, pas seulement les questions identiques.   
- L’historique anti-répétition des épreuves passe de 4 à **10 questions récentes**, avec maintien des cooldowns existants. Les épreuves propres à un biome sont identifiées comme « terrain local » afin que l’écran explique pourquoi cette question arrive à ce moment-là.   
- L’autonomie d’exploration est maintenant vérifiée **avant le départ de chaque nouvelle fouille**. L’écran d’intensité affiche l’autonomie restante et le nombre de Rations de survie ; si la sortie prévue dépasse la réserve, les rations nécessaires sont consommées avant le départ ou la sortie est refusée sans faire avancer le temps.   
- Le résumé de poursuite indique clairement les segments d’autonomie restants et les rations encore disponibles. Une expédition sans autonomie ni ration doit rentrer au lieu de dépasser silencieusement la limite puis de la découvrir après coup.   
- Ajout de tests de non-régression qui génèrent des tableaux de guilde aux niveaux 1–12 pour interdire les lieux trop hauts, vérifient l’historique anti-répétition étendu et protègent les nouvelles gardes de cohérence exploration/quêtes.   
- Aucun changement du schéma de sauvegarde (**26**) ni du checkpoint obligatoire (**V3.50.20**).   

---   

## V3.50.20 — Portefeuille physique et piles de pièces persistantes   

- Le portefeuille ne normalise plus automatiquement les pièces vers les plus grosses dénominations. Une pile de **51 PO reste 51 PO** dans l’inventaire au lieu d’être affichée comme 5 PP + 1 PO.   
- `Inventory` conserve désormais cinq piles physiques indépendantes : PC, PF, PE, PO et PP. Les gains historiques en unités économiques ajoutent des PF, les gains explicitement en cuivre ajoutent des PC, et les anciens gains explicites en or ajoutent des PO.   
- Les paiements utilisent d’abord les pièces disponibles sans modifier les autres piles. Lorsqu’une pièce plus grosse doit être cassée, le système simule la monnaie rendue dans les dénominations inférieures tout en conservant exactement la valeur totale.   
- Le total en PC reste disponible pour toutes les vérifications de fonds et pour la compatibilité des systèmes existants ; seule la représentation physique du portefeuille devient plus fidèle au monde.   
- Le schéma de sauvegarde passe de **25 à 26**. Les nouvelles sauvegardes enregistrent chaque pile de pièces séparément. Les sauvegardes antérieures, qui ne connaissaient qu’un total, conservent exactement leur valeur et reçoivent une décomposition initiale lors de la migration.   
- Le checkpoint obligatoire avance à **V3.50.20** afin de protéger la nouvelle représentation persistante du portefeuille avant adaptation d’une ancienne sauvegarde.   
- Les tests économiques couvrent maintenant explicitement la pile de 51 PO, la monnaie rendue lors d’un paiement avec une pièce trop grosse, le départ à 50 PF et la persistance exacte des cinq piles après sauvegarde/rechargement.   

---   

## V3.50.19 — Normalisation complète de l’économie et migration monétaire sûre   

- Recentrage de l’économie historique sur l’échelle de pièces voulue : **1 unité économique écrite dans les anciens prix/récompenses = 1 PF = 10 PC**. Les prix de boutique, récompenses de quêtes, combats et services historiques gardent ainsi leurs proportions sans être interprétés comme des PO entiers.   
- L’argent de départ normal reste `50` unités écrites, mais signifie désormais **50 PF = 5 PE = 500 PC**, et non plus 50 PO.   
- Achats/ventes, récompenses de quêtes, combats, exploration, transferts PvP/boss, entraînement, réparation de réputation et autres anciens flux `earnGold/spendGold` passent par les unités économiques. Les micro-prix déjà réellement conçus en cuivre (auberge, repas, taxes de voyage, infirmerie, missions déléguées et systèmes similaires) restent des valeurs PC exactes.   
- Ajout d’API explicites `Money`/`Inventory` pour les unités économiques, tout en conservant les helpers PO historiques uniquement pour compatibilité. L’affichage du portefeuille n’utilise plus de total décimal trompeur en PO : le total PC exact et le détail des pièces sont privilégiés.   
- Le schéma de sauvegarde passe de **24 à 25**. Les sauvegardes de schéma <25 reçoivent une normalisation monétaire unique afin de ramener les fortunes créées par l’ancienne interprétation en PO vers la nouvelle échelle PF. Une sauvegarde réécrite en schéma 25 ne peut pas subir la migration une seconde fois.   
- Comme cette normalisation modifie une valeur persistante importante, le checkpoint obligatoire avance à **V3.50.19** afin de déclencher la protection/backup pré-mise-à-jour avant migration.   
- Conservation du cuivre exact corrigée dans les récapitulatifs de combat, snapshots PvP, changement de classe via cheat, pénalités de mort et résumés d’exploration : les petites valeurs ne sont plus perdues par arrondi en PO.   
- Ajout de tests de non-régression sur la conversion PF/PC, l’argent de départ normal, les flux boutique/quêtes/combats, la migration des anciennes sauvegardes et une garde empêchant le retour de `earnGold/spendGold` dans le gameplay.   
- Poursuite du nettoyage des textes RP liés à l’argent : les récompenses génériques parlent d’« argent » lorsque la pièce exacte n’est pas imposée ; les vraies références de lore ou de dénomination restent intactes.   

---   
   

## V3.50.18 — Nettoyage RP, progression des compétences et densité des élites   

- Nettoyage des textes affichés au joueur qui ressemblaient à des notes de mise à jour ou à des commentaires de développement : mentions de `Future IG`, assets futurs, priorités de développement, « méta-lore », fonctionnalités annoncées comme futures et comparaisons inutiles avec un ancien comportement ont été retirées ou reformulées dans le ton du monde. Les vrais messages de progression (déblocage, quête qui progresse, état qui change) restent affichés. Les messages techniques indispensables de migration de sauvegarde restent également explicites.   
- La limite actuelle de l'histoire après l'introduction du chapitre 3 est présentée comme une **suite indisponible**, sans exposer au joueur la refonte interne, les scènes conservées dans le code ou les priorités de développement. Le monde libre reste accessible.   
- Progression des compétences de classe étalée : la plupart des premières techniques commencent autour du **niveau 5**, l'invocation au niveau 6, puis les techniques suivantes sont réparties sur des paliers plus espacés. La branche arcanique s'étend notamment jusqu'aux niveaux 14–17 pour ses techniques avancées. Les sorts réellement appris par grimoire conservent leurs déblocages spécifiques.   
- Les déblocages d'identité de classe de `PlayerSkills` passent du niveau 4 au niveau 5 et les profils d'IA sont alignés pour éviter que les ennemis ordinaires utilisent systématiquement leurs techniques de classe avant le joueur.   
- Densité d'élites ambiantes limitée au moment du tirage : aucune élite aléatoire aux niveaux 1–2, environ 5 % aux niveaux 3–4, puis une hausse progressive avec le niveau. Les régions très haut niveau composées uniquement d'élites peuvent toujours en produire, et les mini-boss/élites explicitement scénarisés ne sont pas touchés.   
- Une vague normale ne peut plus appliquer une variante évoluée à un monstre déjà élite, ce qui évite l'empilement involontaire « élite de catalogue + évolution ».   
- Ajout d'un test statistique de non-régression sur la densité d'élites de début de jeu ainsi que d'invariants sur les paliers de compétences et les principaux textes RP nettoyés.   
- `scripts/validate_release_tree.sh --skip-branding` permet de valider explicitement un backup essentiel sans images. Le contrôle branding reste strict par défaut pour une vraie release.   
- Aucun changement du schéma de sauvegarde (**24**) ni du checkpoint obligatoire (**V3.50.12**). L'économie et le filtrage complet des quêtes bas niveau restent des passes séparées afin de ne pas mélanger une refonte monétaire risquée avec cet équilibrage.   

---   

## V3.50.17 — Stabilisation installateurs, C++23 et clarification des outils   

- Migration de la toolchain vers **C++23**. Le Makefile, les tests et les builds Windows/Linux utilisent maintenant un détecteur commun : `-std=c++23` lorsqu'il est accepté, avec repli sur l'alias historique `-std=c++2b` pour les compilateurs GCC/MinGW plus anciens qui implémentent C++23 sous ce nom. Aucun changement du schéma de sauvegarde : il reste **24**, checkpoint obligatoire **V3.50.12**.   
- Correction Windows importante : l'installateur **et** le launcher réparaient encore parfois `Lancer-Dinotofu.cmd` en mode `Auto`, ce qui pouvait réactiver le choix GUI. Le lanceur principal est désormais systématiquement reconstruit en **Terminal** ; `Auto`/`Gui` ne restent accessibles que sur demande explicite.   
- Correction de l'installateur Windows : le fallback `AssetPattern` par défaut visait encore `Dinotofu-Windows-v*.7z` alors que la release Windows est un ZIP. Le fallback vise maintenant `Dinotofu-Windows-v*.zip`.   
- Releases plus lisibles : les deux téléchargements joueur portent désormais le préfixe très visible **`INSTALLER-DINOTOFU-...`**. Les deux gros fichiers nécessaires aux installateurs restent publiés, mais leur nom se termine par **`-TECHNICAL-PAYLOAD`** afin d'éviter que les joueurs les prennent pour l'installateur. Leur préfixe historique `Dinotofu-<OS>-v*` est volontairement conservé afin que les anciens installateurs utilisant ce wildcard puissent encore les trouver.   
- Les petits packs joueurs contiennent toujours exactement **un fichier `INSTALLER-DINOTOFU` + `Documentation/`**, et `Documentation/` ne contient que des `.txt`. Le jeu complet n'est pas inclus dans ce téléchargement joueur.   
- `scripts/bump_version.py` pose toujours la question finale avant de commiter, mais **Oui devient maintenant la réponse par défaut en mode interactif**, comme demandé. Le mode non interactif reste sûr : aucun commit n'est créé sans `--commit`; pour les passes ChatGPT/dev on peut donc utiliser explicitement `--no-commit`.   
- Ajout de `tools/windows/README_TOOLS.txt` et `tools/linux/README_TOOLS.txt` pour expliquer clairement la différence entre les wrappers cliquables (`Installer/Lancer`) et les moteurs internes (`DinotofuInstaller/DinotofuLauncher`). Les anciens alias Terminal restent présents uniquement pour compatibilité, pas comme deuxième version du jeu.   
- La grosse refonte de l'interface graphique, l'économie, les compétences de classe, la densité d'élites et le filtrage complet des quêtes bas niveau restent volontairement séparés de cette passe technique.   

---   

## V3.50.16 — Stabilisation sauvegardes, quêtes et distribution   

- Sauvegardes personnage : ajout des métadonnées persistantes `realPlayTimeSeconds` et `lastSavedAt`, avec lecture compatible des anciennes sauvegardes. L'écran de sélection affiche désormais date de création, temps réel de jeu et dernière sauvegarde. Le schéma de sauvegarde passe à **24** ; le checkpoint obligatoire reste **V3.50.12**, car cette évolution est additive et ne nécessite pas de rituel destructif.   
- Renforcement du test de round-trip : il vérifie désormais explicitement le **niveau**, l'expérience, le **montant exact en cuivre** et le temps réel joué après sauvegarde/rechargement. Cela couvre directement la régression signalée où le niveau ou l'argent pouvaient sembler retomber après reprise.   
- Tableau de guilde : accepter une quête laisse maintenant réellement son emplacement vide jusqu'au **jour de jeu suivant**. `ensureGuildBoardReady()` tient compte des remplacements différés au lieu de remplir immédiatement le trou dans la même journée. Un test de non-régression couvre ce comportement.   
- Auberge : ajout d'un repos gratuit au **fond de l'écurie**. Il fait passer un jour mais ne rend que jusqu'à 25 % des PV maximum afin de préserver l'intérêt des lits payants.   
- Lanceurs Windows/Linux : le **Terminal devient le mode par défaut**. L'ancienne interface graphique reste accessible explicitement pour le développement mais est signalée comme en cours de refonte ; sa grosse reconstruction est volontairement reportée à une passe dédiée.   
- Distribution : rétablissement de **quatre assets** par release. Les joueurs reçoivent `Installer-Dinotofu-Windows-v*.zip` ou `Installer-Dinotofu-Linux-v*.7z`, dont la racine contient uniquement l'installateur + `Documentation/` en `.txt`. Les gros `Dinotofu-Windows-v*.zip` et `Dinotofu-Linux-v*.7z` restent des payloads techniques afin de ne pas casser les anciens launchers/updaters.   
- `scripts/bump_version.py` ne commit plus implicitement en mode interactif : après toutes les modifications, il demande explicitement s'il faut créer un commit Git, avec **Non par défaut**. En non-interactif, aucun commit n'est créé sans `--commit`.   
- Documentation et validations de release alignées sur le Terminal par défaut et le modèle 2 installateurs + 2 payloads.   
- Cette passe ne change pas encore l'économie de départ, la courbe de déblocage des compétences, la fréquence des élites ni le filtrage complet des quêtes trop éloignées pour les bas niveaux : ces sujets restent séparés pour éviter de mélanger équilibrage massif et stabilisation technique.   

---   

## V3.50.15 — Notes de mise à jour   

- Correction des scripts d'installation en filtrant correctement les processus actifs (ça tué les scripts eux-même).   
- Liaison statique de certaines librairies et rétrogradation de la version d'OS à ubuntu-22.04 pour la compatibilité d'autres libraires.   

---   

## V3.50.14 — Notes de mise à jour   

- Correction du bogue d'auto-terminaison du lanceur Linux dans `tools/linux/DinotofuLauncher.sh`.   
- Dans `stop_dinotofu_background_processes`, remplacement du filtre large `pkill -f "${target_dir}/(output/)?Dinotofu"` par un ciblage précis via `pgrep` avec ancrage et exclusion des PIDs et des scripts shell, évitant que le script du lanceur ne s'interrompe lui-même lors du lancement de l'interface graphique depuis un dossier contenant `Dinotofu`.   
- Validation complète de la suite de tests et de la structure des releases.   
- Aucun changement du schéma de sauvegarde (`saveVersion = 23`) ni du jalon obligatoire (`V3.50.12`).   

---   

## V3.50.13 — Notes de mise à jour   

- Correction et fiabilisation des installateurs et lanceurs Windows et Linux.   
- Windows : synchronisation systématique des scripts du launcher (`DinotofuLauncher.ps1`, `Lancer-Dinotofu.cmd`, `Lancer-Dinotofu-Terminal.cmd`), des outils GUI et des assets pour écraser les versions antérieures extraites des archives de release.   
- Windows : arrêt complet et robuste des processus en arrière-plan (`Stop-DinotofuBackgroundProcesses` avec PID, WMI/CIM, détection par nom .NET et filet de sécurité `taskkill`) pour éliminer les verrous de fichiers lors de la copie (`Copy-Item`). Ajout de plusieurs tentatives de copie avec temporisation.   
- Windows : correction du bogue de chemin relatif qui créait un dossier parasite `C\` dans le dépôt, avec suppression automatique des liens orphelins et vérification de chemins absolus (`IsPathRooted`).   
- Windows & Linux : interface interactive au lancement (choix entre Interface Graphique et Mode Terminal classique), gestion de session propre en console pour couper le serveur web et le moteur de jeu avec Entrée ou Ctrl+C.   
- Aucune modification du schéma de sauvegarde (`saveVersion = 23`) ni du jalon obligatoire (`V3.50.12`).   

---   

## V3.50.12 — Notes de mise à jour   

- Synchronisation de version. Remplacer par les notes détaillées avant publication.   

---   

## V3.50.11 — Notes de mise à jour   

- Refonte de `scripts/bump_version.py` : le changement de version du jeu, le schéma interne des sauvegardes (`saveVersion`) et le checkpoint obligatoire sont désormais trois décisions indépendantes. En mode interactif, le script demande explicitement s'il faut incrémenter le schéma de sauvegarde et si la nouvelle version doit devenir un nouveau checkpoint avec backup + rituel. Des options `--save-schema`, `--checkpoint` et `--non-interactive` permettent aussi l'automatisation.   
- Centralisation du schéma de sauvegarde dans `include/save/SaveSchemaVersion.hpp`. `SaveManager` écrit maintenant `SaveSchemaVersion::Current` au lieu de dupliquer la valeur numérique. Les tests de round-trip utilisent la même constante, ce qui évite les désynchronisations lors d'un futur changement de schéma. Pour V3.50.11, le schéma reste **23** et le checkpoint important reste **V3.50.09**.   
- Fiabilisation de la publication GitHub : le workflow ne pousse plus un tag avant les compilations. Les jobs Windows/Linux construisent le commit qui a déclenché le workflow ; le tag/release est créé seulement après succès des builds via `gh release create --target $GITHUB_SHA`. Cela évite les tags vides et réduit les blocages liés aux règles de protection de tags.   
- Le workflow garde le déclenchement automatique sur `main`/`master` et le déclenchement manuel `workflow_dispatch`. Ajout de `scripts/trigger_release.sh` et de la cible `make release-trigger` pour forcer/réparer facilement une release avec GitHub CLI (`force_release=true`). Les guides expliquent désormais clairement qu'un push sans nouveau numéro de version ne produit volontairement pas de nouvelle release.   
- Nouveau format de distribution joueur : chaque plateforme publie désormais un pack `Installer-Dinotofu-<Plateforme>-vX.YY.ZZ.7z` dont la racine contient **exactement un fichier d'installation + `Documentation/`**, et uniquement des `.txt` dans ce dossier. Windows utilise un bootstrap `.cmd` unique qui récupère le moteur PowerShell depuis le tag de release (avec fallback `main`) puis télécharge le payload technique ; Linux utilise l'installateur shell autonome.   
- Les archives historiques `Dinotofu-Windows-vX.YY.ZZ.7z` et `Dinotofu-Linux-vX.YY.ZZ.7z` restent publiées comme payloads techniques afin de ne casser aucun ancien launcher/updater. Leur documentation est regroupée dans `Documentation/` au lieu d'éparpiller README/changelogs/fichiers texte à la racine.   
- Le workflow considère maintenant les quatre assets (deux packs installateur + deux payloads) comme nécessaires à une release complète et sait les reconstruire avec `force_release=true`. Les validations protègent le nouveau layout propre, les nouveaux helpers de release et l'indépendance version jeu / saveVersion / checkpoint.   
- Aucun avancement de l'histoire principale et aucun nouveau champ de sauvegarde dans cette passe : le verrou reste immédiatement après l'introduction du chapitre 3.   

---   

## V3.50.10 — Stabilisation des merges, réparation de l’updater legacy et migration forcée des anciennes sauvegardes   
- Audit fichier par fichier de la V3.50.09 mergée par l’utilisateur face à la V3.50.09 livrée précédemment. Les systèmes de gameplay/contenu n’ont pas été écrasés : le merge ajoute 3 fichiers et modifie 14 fichiers, presque uniquement autour de la CI, du Makefile, du packaging, des installateurs/launchers et de `main.cpp`. Les bons ajouts (`.gitattributes`, CI multi-plateforme, détection de bureaux localisés, repo par défaut, support runtime `data/assets`) sont conservés.   
- Correction d’une régression importante expliquant pourquoi certaines anciennes installations ne trouvaient plus les mises à jour : leurs configs contiennent encore littéralement `TON_COMPTE/TON_REPO`. Les launchers/installateurs Linux et Windows reconnaissent maintenant ce placeholder — ainsi que les valeurs de repo invalides — comme une ancienne configuration et basculent automatiquement vers `SIMON-Louis-2326101aa/ProjetDinotofu`.   
- Le launcher Linux compare désormais réellement les versions au lieu de mettre à jour dès qu’elles sont simplement différentes ; un build local plus récent que la release ne sera donc plus rétrogradé accidentellement. Windows utilisait déjà une comparaison sémantique et conserve ce comportement.   
- Réparation du workflow GitHub Release : si le tag existe mais que la Release ou une archive `.7z` Windows/Linux manque, le workflow reconstruit/répare désormais la publication au lieu de déclarer qu’aucune action n’est nécessaire. Les builds repartent du tag exact pour que les artefacts correspondent bien à la version publiée.   
- Correction de plusieurs bugs issus du merge dans le packaging. `package_linux_release.sh` utilisait `REPO_NAME` sans le définir alors que `set -u` était actif ; Linux et Windows utilisent maintenant le même `detect_repo_name()`. Pour cette version de stabilisation, le layout historique `assets/` reste volontairement à la racine : déplacer maintenant `assets/saves` vers `data/assets/saves` risquerait de préserver une ancienne sauvegarde au mauvais endroit. Le moteur et les installateurs savent toujours reconnaître les deux layouts pour une future migration explicite. Les outils GUI sont de nouveau inclus et les sauvegardes locales sont explicitement retirées du staging.   
- Renforcement du checkpoint obligatoire V3.50.09. `VersionInfo::requiresImportantSaveUpdate()` considère désormais les métadonnées inconnues/non versionnées comme anciennes au lieu de renvoyer faux. Les sauvegardes antérieures à V3.00 et celles sans `lastAdaptedVersion` ne peuvent plus contourner le checkpoint via l’ancien écran de recommandation : backup dédié non écrasable + adaptation lourde obligatoire passent avant toute reprise normale.   
- Les métadonnées anciennes sont maintenant résolues dans l’ordre `lastAdaptedVersion` → `createdForVersion` → ancien `gameVersion`. Ajout de `LegacySaveMigrationTest.cpp` et extension de `ImportantSaveCheckpointTest.cpp` pour tester versions 1.x/2.x, métadonnées inconnues, fallback `gameVersion`, création du checkpoint, adaptation lourde et marquage final sur la version courante. Le schéma de sauvegarde reste **23**.   
- Correction du nouveau `bump_version.py` issu du merge : il ne remplace plus aveuglément toutes les références à l’ancienne version. Le checkpoint important reste donc **V3.50.09** quand le jeu passe en 3.50.10+, l’historique n’est plus réécrit, les deux changelogs restent synchronisés et les textes libres des manifests ne sont plus altérés.   
- Conservation des améliorations utiles du merge : Makefile plus pratique, `.gitattributes`, CI Linux/Windows, meilleure détection des bureaux, repo GitHub explicite, échec clair si un asset de release manque, et support dans `main.cpp`/launchers des layouts historique `assets/` et futur `data/assets`.   
- Aucun avancement de l’histoire principale : le verrou reste immédiatement après l’introduction du chapitre 3. Cette version stabilise merges, mises à jour et migrations.   

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
