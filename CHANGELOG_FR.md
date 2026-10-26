# Journal des modifications Dinotofu   

## V3.57.08 — Passe pré-push : suites de quêtes typées, services urbains et garde-fous Git   

- Passe conçue comme dernier jalon de consolidation avant le premier push Git : privilégier des systèmes terminés, lisibles et testables plutôt que lancer un nouveau chantier massif juste avant la publication du dépôt.   
- `QuestSecondPhaseSystem` distingue désormais réellement les suites de **service/livraison, sauvetage, enquête/vérification, récolte/matériaux, chasse/combat/bestiaire et exploration**. Chaque famille possède plusieurs suites ordinaires cohérentes au lieu de retomber trop souvent sur une surprise générique.   
- Certaines suites sont explicitement facultatives ou obligatoires selon leur logique : par exemple un dernier service après sauvetage peut être refusé, alors qu'un mauvais guichet, un échantillon à vérifier ou un recoupement indispensable peut rester une étape contractuelle. Les règles de rémunération V3.57.07 restent inchangées : obligatoire/inter-ville payé, facultatif local éventuellement non monétaire.   
- Le journal de phase 2 expose maintenant **Type de suite**, **Difficulté estimée** et **Risques connus**. Les risques décrivent uniquement ce que le personnage peut raisonnablement anticiper et rappellent notamment qu'une rumeur ou un témoignage n'est pas une preuve ; les dangers réellement inconnus ne sont pas spoilés.   
- `CityEconomyDynamicsSystem` fournit désormais un score partagé de qualité locale pour **forge/réparations, alchimie, enchantement et transport**, combinant spécialisation structurelle de la ville, richesse actuelle et conjoncture économique. Le rapport économique de ville affiche ces différences de façon qualitative et chiffrée ; le branchement détaillé de tous les artisans restera progressif après le premier push.   
- Nouveau contrôle `make pre-push` : nettoyage, audit source, validation JSON, build/tests et contrôles Git quand un dépôt est présent. Un backup essentiel sans images de branding peut être validé comme **source prête à pousser**, tandis qu'une vraie release/installateur continue d'exiger les assets officiels.   
- Nouveau workflow `.github/workflows/ci.yml` minimal : audit source et suite de tests sur push, pull request et lancement manuel. Il ne remplace pas le futur workflow de release.   
- `scripts/release_push.sh` accepte désormais `current` / `--no-bump` pour pousser exactement une version déjà validée sans créer automatiquement une version patch supplémentaire. Le guide `PREMIER_PUSH_GIT.md` documente le premier push et ce mode.   
- `.gitignore` exclut explicitement les reprises de chat et backups essentiels locaux afin d'éviter de publier par erreur des artefacts de handoff.   
- `QuestSecondPhaseSystemTest` et `CityEconomyDynamicsSystemTest` couvrent les nouvelles familles, les informations de journal et les différences de qualité de service entre profils/conjonctures de villes.   
- Simulation du premier dépôt dans une copie vierge : **1003 fichiers** indexés, aucun fichier privé/généré interdit, `git diff --cached --check` propre hors convention Markdown, règles `.gitignore` confirmées et commit initial simulé avec succès. Le contrôle a aussi permis de nettoyer 74 fins de fichiers historiques et 2 scripts PowerShell ; ces 76 corrections ont été vérifiées comme strictement limitées aux espaces/fins de ligne.   
- Schéma de sauvegarde **26** inchangé. Checkpoint important **V3.57.00** inchangé. **Titres : aucun ajout, aucune modification, aucun rebranchement** ; catalogue toujours **V3.57.02 / 419 titres**.   
- Validation finale : build complet **V3.57.08 OK** ; runner global **119 lancés / 119 attendus / 119 PASS** avec contrôles statiques/versioning PASS. Le parcours `make pre-push` a validé nettoyage, audit source et JSON ; sa recompilation intégrale dépasse la fenêtre d’exécution de l’environnement, mais les mêmes étapes build/tests ont été validées séparément sur l’état final. Audit final de l’arbre nettoyé et backup ré-extrait requis avant remise.   

---   

## V3.57.07 — Écurie anti-abus, collecte communautaire et suites facultatives non monétaires   

- Passe volontairement recentrée sur les systèmes généraux **B — quêtes / contrats / voyages** et **C — villes / économie / monde vivant** : aucun nouveau développement de prime ou de réseau criminel n'est ajouté dans cette version afin que D reste un sous-système et non le mode principal de Dinotofu.   
- Nouveau `LodgingRestSystem` pour le couchage gratuit en écurie : jusqu'à **3 nuits gratuites sur une fenêtre glissante de 5 jours** sont tolérées. Une nouvelle tentative au-delà de cette tolérance provoque l'expulsion et une **interdiction locale de 3 jours**. L'interdiction ne touche pas les autres villes ni les hébergements payants/privés.   
- Le menu d'hébergement affiche désormais l'état réel de l'écurie, le nombre d'utilisations gratuites récentes et la durée d'une éventuelle interdiction. Une tentative refusée n'avance pas le temps et ne soigne pas le personnage.   
- La **Fête des jardins** est retirée du calendrier actif des événements réguliers. Elle est remplacée par une **Collecte communautaire** centrée sur les dons utiles, matériaux, consommables, plantes et services locaux. La Fête des moissons reste l'événement saisonnier principal lié aux récoltes.   
- Les effets économiques et de stock de la Collecte communautaire utilisent les systèmes dynamiques existants et restent bornés. Participer à la collecte récompense surtout la reconnaissance locale et le service rendu plutôt qu'un paiement direct.   
- Les **phases 2 facultatives locales** peuvent désormais récompenser principalement la **réputation locale**, une **relation PNJ**, une **information recoupée** ou une **opportunité future** plutôt qu'un supplément monétaire systématique. Le contrat principal conserve sa récompense normale.   
- Les phases 2 **obligatoires** et les suites **inter-ville** conservent une rémunération monétaire/logistique : le nouveau système ne recrée donc pas de transport gratuit déguisé. Les récompenses non monétaires sont enregistrées de façon idempotente et ne peuvent pas être réclamées plusieurs fois.   
- Les nouveaux champs de récompense de phase 2 sont optionnels et rétrocompatibles ; le schéma de sauvegarde reste **26**. Nouveau test `LodgingRestSystemTest` et extensions des tests `QuestSecondPhaseSystemTest`, `SaveRoundTripTest` et `CityEconomyDynamicsSystemTest`.   
- Build final **V3.57.07 : OK**. Runner global : **119 tests lancés / 119 attendus / 119 PASS** ; contrôles statiques et versioning : PASS.   
- **Titres : aucun ajout, aucune modification, aucun rebranchement dans cette version.** Le catalogue reste **V3.57.02**, avec **419 titres**. Le checkpoint important reste **V3.57.00**.   

---   

## V3.57.06 — Récupération progressive après les perturbations criminelles   

- La pression criminelle locale n'est plus figée : plusieurs jours relativement calmes réduisent progressivement la pression observable des contacts connus. Les rivalités historiques restent plus lentes et ne disparaissent pas automatiquement.   
- Nouveau `CrimeLocalStabilizationSystem` : terminer et rendre une vraie quête issue de **Suivis locaux** réduit de façon bornée la pression de contacts connus dans la ville et enregistre une stabilisation locale persistante.   
- Les trois types de suivis gardent une force différente : contrôle des registres, surveillance des relais et recoupement de témoignages n'ont pas exactement le même poids, mais aucun ne remet instantanément la ville à zéro.   
- `CrimeLocalImpactSystem` distingue désormais le poids des perturbations récentes du poids des stabilisations récentes. Les effets économiques diminuent donc naturellement avec le temps et plus vite si le joueur aide réellement à documenter/stabiliser la situation.   
- `CrimeNetworkCompetitionSystem` tient désormais compte de la baisse progressive de la pression après plusieurs jours calmes. La rivalité entre réseaux reste néanmoins une mémoire durable tant qu'aucun événement ne justifie sa disparition.   
- Le rendu d'une quête locale de conséquence appelle directement la stabilisation depuis le flux normal de remise de quête ; le même suivi ne peut pas être utilisé plusieurs fois pour farmer artificiellement la récupération.   
- Nouveau test `CrimeLocalStabilizationSystemTest`, ajouté au runner global. Le runner passe à **118 tests lancés / 118 attendus / 118 PASS** ; contrôles statiques et versioning : PASS.   
- **Titres : aucun ajout, aucune modification, aucun rebranchement dans cette version.** Le catalogue reste **V3.57.02**, avec **419 titres**. Le schéma de sauvegarde reste **26** et le checkpoint important reste **V3.57.00**.   
- Build final **V3.57.06 : OK**.   

---   

## V3.57.05 — Conséquences criminelles locales et petits suivis de guilde   

- Première passerelle volontaire entre **D — primes / crime / réseaux / rumeurs**, **C — villes / économie / monde vivant** et **B — quêtes / contrats / voyages** : une perturbation criminelle réellement documentée peut désormais laisser un effet local temporaire au lieu de disparaître aussitôt après l'arrestation ou la résolution d'une prime.   
- Nouveau `CrimeLocalImpactSystem` : il mesure uniquement ce que le personnage peut observer à partir de contacts et réseaux déjà connus, des arrestations/indisponibilités réelles, de la concurrence connue et des perturbations récentes. Aucun réseau inconnu n'est révélé et aucune petite affaire n'est transformée artificiellement en crise de ville.   
- Les effets économiques sont volontairement bornés : selon la pression locale, certains services peuvent perdre **1 à 5 points** de qualité, la demande commerciale locale peut ajouter **1 à 3 %** au prix de vente de certains échanges et le risque logistique indicatif peut monter de **4 à 18 %**. Les tensions fortes peuvent aussi créer une petite demande de matériel de transport/relais.   
- `CrimeJusticeSystem` enregistre désormais une perturbation locale lorsqu'une arrestation durable réorganise réellement des relais connus. `BountySocialConsequenceSystem` peut faire de même après une capture ou une mort seulement si le personnage connaissait déjà des contacts du réseau concerné.   
- Les boutiques et certains services urbains consomment cette pression par le système économique existant. Le rapport d'économie locale expose aussi la **pression criminelle observée** lorsqu'elle est suffisamment documentée.   
- Les perturbations créent une rumeur locale sourcée via `WorldRumorSystem`. Cette rumeur reste explicitement distincte d'un fait confirmé et ne devient pas une preuve judiciaire par magie.   
- Nouveau `CrimeLocalOpportunitySystem` : une perturbation suffisamment importante peut laisser à la guilde un petit suivi local persistant. Trois formes existent actuellement : **Registres sous tension**, **Relais sous surveillance** et **Après le remous**. Elles deviennent de vraies quêtes de service lorsqu'elles sont acceptées, avec récompense modeste et sans générer une longue chaîne artificielle à chaque incident.   
- Le menu de guilde reçoit **Suivis locaux**, qui affiche les conséquences observables et les opportunités disponibles dans la ville courante. Les opportunités identiques sont anti-spam et disposent d'un délai avant de pouvoir réapparaître.   
- Nouveau test `CrimeLocalImpactSystemTest` et nouveau test `CrimeLocalOpportunitySystemTest`, ajoutés au runner global. Un défaut du test d'impact a également été corrigé : il conservait une référence vers un élément d'un `vector` après une insertion susceptible de réallouer ce `vector`; le moteur n'était pas en cause.   
- **Titres : aucun ajout, aucune modification, aucun rebranchement dans cette version.** Le catalogue reste **V3.57.02**, avec **419 titres**. Le schéma de sauvegarde reste **26** et le checkpoint important reste **V3.57.00**.   
- Build final **V3.57.05 : OK**. Runner global corrigé : **117 tests lancés / 117 attendus / 117 PASS** ; contrôles statiques et versioning : PASS.   

---   

## V3.57.04 — Entretiens de garde et concurrence criminelle observable   

- Troisième passe prioritaire sur **D — primes / crime / réseaux / rumeurs** : une cible nommée capturée vivante peut désormais laisser un **entretien de garde facultatif** à la guilde. Une cible tuée ne crée évidemment aucun entretien.   
- Nouveau `BountyCustodySystem` : le joueur peut mener un débriefing calme, questionner la cible sur son réseau ou proposer une coopération formelle. Un témoignage reste un **témoignage sourcé**, jamais une vérité automatique : il alimente rumeurs, pistes et journal canonique, mais doit être recoupé avant de devenir une preuve.   
- Les entretiens respectent l'absence d'omniscience : interroger une cible sur son réseau ne révèle et ne modifie que les contacts criminels que le joueur connaissait déjà. Les contacts inconnus restent inconnus.   
- Nouvelle couche `CrimeNetworkCompetitionSystem` : lorsque le joueur connaît plusieurs réseaux dans une même ville, leur tension locale peut être évaluée. Une perturbation documentée d'un réseau peut accroître rivalité, pression et opportunisme chez les rivaux **déjà connus**, sans inventer de nouveaux criminels pour remplir le vide.   
- Une arrestation durable validée par `CrimeJusticeSystem` peut désormais provoquer cette perturbation locale. La conséquence reste sourcée par la vraie arrestation et n'agit que sur des réseaux déjà identifiés par le personnage.   
- `CrimeContactRelationshipSystem` peut choisir un **relais de remplacement déjà connu** lorsqu'un contact est arrêté, détenu, en fuite ou en rupture. `CrimeContactDialogueSystem` l'annonce alors explicitement ; aucun relais inconnu n'est révélé gratuitement.   
- Le menu de cambriolage affiche, lorsqu'elle est réellement observable, la tension entre réseaux criminels déjà connus dans la ville courante.   
- Nouveaux tests `BountyCustodySystemTest` et `CrimeNetworkCompetitionSystemTest`, ajoutés au runner global. Les tests justice, dialogue criminel et conséquences sociales existants restent couverts.   
- **Titres : aucun ajout, aucune modification, aucun rebranchement dans cette version.** Le catalogue reste **V3.57.02**, avec **419 titres**. Le schéma de sauvegarde reste **26** et le checkpoint important reste **V3.57.00**.   

- Build final **V3.57.04 : OK**. Runner global : **115 tests lancés / 115 attendus / 115 PASS** ; contrôles statiques et versioning : PASS.   

---   

## V3.57.03 — Suivis de primes jouables, contacts contextuels et justice sur preuves   

- Deuxième passe prioritaire sur **D — primes / crime / réseaux / rumeurs** : les conséquences sociales d'une prime nommée ne restent plus de simples compteurs. `BountyAftermathSystem` crée des suivis persistants pour le commanditaire, son organisation et, avec un délai crédible, un proche de la cible.   
- La guilde propose désormais un écran **Suivis de primes**. Le joueur peut répondre de façon professionnelle, conciliante, ferme, proposer son aide ou clore l'échange. Une proposition d'aide mémorise une opportunité ; elle ne fabrique pas immédiatement une quête artificielle.   
- Les proches ne se téléportent pas au moment de la résolution : leur suivi n'est disponible qu'à partir du jour suivant. Un même dénouement ne peut pas être exploité pour générer plusieurs fois la même réaction.   
- Nouveau `CrimeContactDialogueSystem` : les contacts déjà connus parlent désormais selon leur fiabilité, dette/faveur, rivalité, pression, rupture temporaire, détention, fuite et situation judiciaire. Le système ne révèle ni contact ni réseau que le joueur ne connaît pas déjà.   
- Nouveau `CrimeJusticeSystem` : une arrestation durable ne peut être appliquée qu'après constitution d'un dossier suffisamment solide. Le seuil actuel exige notamment au moins **6 indices**, **2 observations du détenteur**, **2 confrontations avec la garde**, **1 détention provisoire**, un dossier officiellement documenté et un score de preuve élevé.   
- La justice distingue explicitement le **recel/complicité documenté** du vol initial : un receleur ne devient jamais rétroactivement l'auteur du cambriolage simplement parce qu'il est suspect ou antipathique.   
- Les arrestations durables sont persistantes, idempotentes et historisées. Les dossiers de cambriolage affichent désormais le score de preuve et indiquent si le seuil d'arrestation durable est atteint.   
- Nouveaux tests : `BountyAftermathSystemTest`, `CrimeContactDialogueSystemTest` et `CrimeJusticeSystemTest`. L'ancien `BountySocialConsequenceSystemTest` reste vert, ce qui verrouille la compatibilité avec la première passe D.   
- **Titres : aucun ajout ni modification dans cette version.** Le catalogue reste **V3.57.02**, avec **419 titres**. Le schéma de sauvegarde reste **26** et le checkpoint important reste **V3.57.00**.   
- Build final **V3.57.03 : OK**. Runner global étendu à **113 tests lancés / 113 attendus / 113 PASS** ; contrôles statiques et versioning : PASS.   

---   

## V3.57.02 — Primes nommées, réseaux criminels persistants et mariage distingué   

- Première passe prioritaire sur **D — primes / crime / réseaux / rumeurs** : les primes ciblées peuvent désormais s'appuyer sur un petit roster persistant de fugitifs nommés avec ville d'origine, réseau, commanditaire, organisation et proche identifié. Les données sociales sont sauvegardées et les anciennes primes connues peuvent récupérer leur profil sans reroll de cible.   
- La fiche de prime affiche uniquement ce que le contrat rend légitime de connaître : commanditaire, organisation et préférence éventuelle pour une capture vivante. Les liens privés, proches et affiliations criminelles ne sont pas révélés gratuitement au joueur.   
- La résolution d'une prime produit maintenant des **conséquences sociales persistantes** : réaction du commanditaire, de son organisation, d'un proche de la cible et des contacts du réseau que le joueur connaissait déjà. Une capture vivante et une mort ne produisent pas la même mémoire. Les contacts inconnus ne sont jamais révélés par omniscience.   
- Nouveau `CrimeContactRelationshipSystem` : fiabilité, faveurs/dettes, rivalité, accords respectés ou rompus et ruptures temporaires modifient désormais les relations avec les contacts criminels. Un relais alternatif ne peut être choisi que parmi les contacts réellement connus et disponibles. Ces nouvelles données survivent aux sauvegardes sans changer le schéma 26.   
- Les pistes de cambriolage affichent la relation persistante avec un contact connu ; une restitution négociée réellement honorée est enregistrée comme accord respecté. `CrimeNetworkSystem` tient compte des ruptures, de la fiabilité, de la rivalité et des faveurs pour évaluer un relais.   
- `WorldRumorCatalog` reçoit trois rumeurs supplémentaires fondées sur des éléments déjà présents : **Courriers nocturnes**, **Torvald** et **Grinka**. Elles restent des rumeurs, jamais des vérités automatiques.   
- Titres V3.57.02 — **AJOUTÉ : « Jusqu'à ce que le ping nous sépare »**, obtenu par un mariage joueur-joueur et volontairement hors prérequis ultime. **MODIFIÉ : « Jusqu'à ce que la sauvegarde nous sépare »**, désormais réservé au mariage avec un conjoint PNJ ; il reste hors ultime tant que la route mariage/divorce complète n'est pas jouable et auditée. Catalogue : **418 -> 419 titres**, version **V3.57.02**.   
- Design mariage enregistré pour le futur chantier F : mariage PNJ + mariage joueur-joueur simultanés autorisés ; si le mariage joueur-joueur est actif, les bonus du conjoint PNJ sont suspendus par jalousie sans annuler le mariage PNJ. Le divorce devra coûter des frais administratifs et un futur remariage exigera l'achat d'une nouvelle bague. Aucun gameplay complet de divorce n'est encore ajouté dans cette passe.   
- Nouveaux tests dédiés `CrimeContactRelationshipSystemTest`, `BountyIdentityCatalogTest` et `BountySocialConsequenceSystemTest`; `SaveRoundTripTest`, `WorldRumorCatalogTest`, `TitleCatalogUpdateTest` et `TitleProgressSystemTest` étendus.   
- Build final **V3.57.02 : OK**. Runner global final : **110 tests lancés / 110 attendus / 110 PASS** ; contrôles statiques et versioning : PASS.   

---   

## V3.57.01 — Audit complet des titres depuis V3.51.51   

- Vraie passe de consolidation du catalogue après audit des systèmes ajoutés depuis la dernière grande révision **V3.51.51** : **375 -> 418 titres**, soit **43 nouveaux titres**. Le catalogue passe en **V3.57.01** ; le jeu passe en **V3.57.01**, le schéma de sauvegarde reste **26** et le checkpoint important reste **V3.57.00**.   
- Titres ajoutés dans cette passe : **« On a un QG ! »**; **« Ce n'est plus une cabane »**; **« Ça devient sérieux »**; **« Il reste une chambre ? »**; **« Le tableau travaille aussi »**; **« La caisse du patron »**; **« Une tournée pour le clan ! »**; **« On avait dit petite fête »**; **« Comme à la maison »**; **« Ça va forger. »**; **« Forgeron maison »**; **« C'est en forgeant... »**; **« Chacun sa spécialité »**; **« Prix d'ami »**; **« Enfin chez moi »**; **« Le loyer, cette quête hebdomadaire »**; **« Plus de propriétaire »**; **« Mon lit, mes règles »**; **« J'ai dit oui »**; **« Une place pour deux »**; **« Jusqu'à ce que la sauvegarde nous sépare »**; **« Dossier : retrouvée »**; **« Une colocataire de plus »**; **« Elle apprend... doucement »**; **« Un gant un peu trop attachant »**; **« Deux formes valent mieux qu'une »**; **« Elle ne m'a pas laissé mourir »**; **« C'était ma première vraie amie »**; **« Sur la piste »**; **« Viens sans faire d'histoire »**; **« Il fallait vraiment en arriver là ? »**; **« Pas assez loin »**; **« Tu peux courir... »**; **« J'avais gardé la piste »**; **« Évidemment, ce n'était pas fini. »**; **« Puisqu'on y est... »**; **« Facteur officiel »**; **« Ce n'est pas une lettre, ça. »**; **« Avec correspondance »**; **« J'ai touché de l'herbe »**; **« On barricade d'abord »**; **« Le convoi est arrivé... à peu près »**; **« Il manquait quelque chose au chargement »**.   
- **« Rencontre avec notre cobaye »** et **« J'ai lu le manuel »** comptent désormais pour la complétion ultime. Le calcul de **« Complétionniste de l'impossible »** réunit maintenant les titres du personnage et les trophées légitimes du compte, ce qui permet aux titres du tutoriel de compter sans les copier artificiellement dans chaque personnage.   
- Nouveau mécanisme de **groupes alternatifs pour l'ultime** : des routes réellement incompatibles peuvent valider un même prérequis. Le foyer utilise déjà `home_financing_route` : payer le parcours locatif ou acheter définitivement son logement valide cette branche sans exiger les deux.   
- Les nouveaux déclencheurs sont reliés aux événements canoniques réels du QG, du foyer, d'Hestia, des primes, des phases 2, des contrats inter-ville, des événements de ville et des incidents commerciaux. **« Livreur sous tension »** bénéficie aussi d'un déclencheur supplémentaire via un incident commercial réellement résolu ; **« Commis de crise »** accepte les événements canoniques d'aide économique en plus de ses chemins de quête existants.   
- Les titres Hestia postérieurs au sauvetage restent volontairement hors prérequis ultime pour le moment : en mode mort définitive, les alliés IA sont eux aussi mortels, Hestia comprise. Ils ne deviendront obligatoires que lorsqu'une route équivalente de mémoire/héritage empêchera une mort définitive de rendre le 100 % injustement impossible. **« Dossier : retrouvée »**, obtenu avant ces branches risquées, compte bien pour l'ultime.   
- Les titres de mariage restent hors ultime tant que la route complète n'est pas honnêtement jouable ; **« Jusqu'à ce que la sauvegarde nous sépare »** restera hors ultime car il dépend d'un autre joueur humain.   
- Nouveau `TitleProgressSystem` centralisé et nouveaux tests `TitleProgressSystemTest` et `AccountAwareUltimateTitleTest`; `TitleCatalogUpdateTest` et `TutorialSystemTest` sont étendus pour verrouiller le catalogue, les groupes alternatifs et les trophées de compte.   
- Build final **V3.57.01 : OK**. Runner étendu à **107 tests** : les 107 exécutables ont été recompilés et exécutés sur l'état final, **107/107 PASS** ; contrôles statiques/versioning : PASS. Le runner ferme désormais le stdin des exécutables de test afin que les écrans interactifs ne puissent plus bloquer une exécution CI.   

---   

## V3.57.00 — Tutoriel Jean-Michel, consolidation des titres et checkpoint d'entrée de jeu   

- Nouveau **tutoriel rejouable** directement présenté comme une fiche supplémentaire dans la sélection des personnages : `Jean-Michel Test`, profession officielle et hautement prestigieuse de **Cobaye**. Sur un compte neuf sans personnage, la fiche est recommandée ; elle reste disponible ensuite pour refaire le parcours.   
- Le tutoriel est volontairement **isolé de la progression réelle** : personnage temporaire, contrat d'entraînement, objet temporaire, combat d'entraînement et récompenses fictives. Aucun inventaire, XP, blessure, économie, quête ou relation de Jean-Michel n'est sauvegardé.   
- Parcours court d'onboarding : rôle des lieux de ville, lecture d'un contrat et de l'accessibilité du lieu, exploration avec indice, bases du combat et retour de mission. L'instructeur reste présent pour expliquer mais ne résout jamais l'action à la place du joueur.   
- Deux trophées de compte accompagnent le tutoriel : **« Rencontre avec notre cobaye »** au premier lancement et **« J'ai lu le manuel »** à la première complétion. Ils sont attribués au compte avec `Jean-Michel Test` comme personnage de déblocage et restent hors prérequis du titre ultime.   
- **« Bienvenue dans Dinotofu »** conserve son rôle existant : il n'est accordé qu'au lancement d'une vraie partie avec un personnage chargé ou créé ; le tutoriel ne le vole pas.   
- Consolidation du catalogue des titres : ajout des deux titres du tutoriel et référencement de **« Ami du petit Dinotofu »** et **« Œil de serrure »**, qui étaient déjà accordés par le moteur mais absents du catalogue. Le moteur contient maintenant **375 titres** ; catalogue synchronisé en **V3.57.00**.   
- `scripts/bump_version.py` sait maintenant gérer séparément la version du catalogue de titres et pose, juste après la question du checkpoint important : **« Des titres ont-ils été ajoutés ou modifiés ? »**. Le mode non interactif reçoit aussi `--titles keep|current|X.Y.Z`. La mise à jour du test de checkpoint a été sécurisée pour ne plus remplacer aveuglément les anciennes versions historiques.   
- Nouveau checkpoint de sauvegarde important **V3.57.00**. Le schéma de sauvegarde reste **26** : aucune migration structurelle n'est nécessaire, mais le backup de transition protège les anciennes sauvegardes avant adaptation à ce jalon d'entrée de jeu et de progression de compte.   
- Nouveau `TutorialSystemTest` et extension des tests de catalogue/checkpoint. Runner global final sur l’état exact de V3.57.00 : **105 tests lancés / 105 attendus / 105 PASS**, puis audit release complet réussi sur le backup essentiel.   

---   

## V3.56.00 — Consolidation des lieux de quête et forfaits logistiques anti-abus   

- Nouveau `QuestLocationAccessSystem` appliqué aux offres de quête via l'écran commun d'acceptation : une fiche peut être **Accessible**, **Chance réduite de trouvaille**, ou **Actuellement inaccessible/inconnue** selon la progression réelle du personnage et le caractère exact ou flexible de l'objectif.   
- Une quête qui exige précisément une ville, un biome ou un bâtiment actuellement inaccessible est désormais bloquée à l'acceptation avec une explication de Diane, plutôt que d'être ajoutée au journal puis de frustrer le joueur.   
- Une chasse, exploration ou recherche de matériau dont le lieu conseillé est inaccessible peut rester acceptée lorsqu'une cible/ressource peut plausiblement exister ailleurs : l'interface affiche alors **Chance réduite de trouvaille** et demande une seconde confirmation explicite.   
- Les offres du panneau de guilde affichent directement cet état de lieu sans empêcher le joueur d'ouvrir la fiche pour comprendre pourquoi elle est bloquée ou risquée. Les demandes de PNJ, événements de quête et défis qui utilisent le même écran bénéficient de la même vérification.   
- Les contrats inter-ville ne « remboursent » plus implicitement le voyage : leur supplément devient une **prime logistique unique**, plafonnée et volontairement modeste. Les taxes normales, détours, retours personnels et trajets supplémentaires restent toujours payés par le joueur.   
- Les suppléments de phase 2 inter-ville utilisent la même philosophie anti-abus : petit forfait unique lié au service rendu, jamais un pass de voyage gratuit.   
- Nouveau `QuestLocationAccessSystemTest`; `IntercityQuestSystemTest` étendu avec plafonds de prime logistique. Runner global : **104 tests lancés / 104 attendus / tous passés**, code retour 0.   
- Saut volontaire de jalon `3.51.89 -> 3.56.00` : de nombreux systèmes majeurs s'étaient accumulés tout en restant sous `V3.51.xx`, simplement parce que la numérotation n'avait pas été relevée plus tôt. **V3.56.00** réaligne donc le numéro sur l'ampleur réelle du projet ; il n'existe pas de releases publiques `V3.52` à `V3.55` manquantes. Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.89 — Missions inter-ville payées et phases 2 de service/logistique   

- Nouveau `IntercityQuestSystem` : la guilde peut proposer de vrais **contrats inter-ville payés** pour transmettre des informations ou transporter des matériaux. Le trajet n'est jamais validé par un bouton abstrait : chaque étape progresse uniquement lorsque le personnage atteint physiquement la ville demandée.   
- Les contrats inter-ville ne sont pas proposés au rang F. Les premiers modèles actuels apparaissent au **rang D** (un trajet) puis au **rang C** (jusqu'à deux relais), sous réserve que les villes soient réellement accessibles au personnage. Une offre impossible reste au tableau au lieu de créer une quête bloquée.   
- La rémunération ajoute une part dédiée au **trajet, à la distance et au nombre de relais** ; le transport de matériaux paie légèrement plus que la simple transmission d'informations. Les délais sont également prolongés en fonction de la distance contractuelle.   
- Les phases 2 de service sont désormais plus ordinaires et contextuelles : `Avant de partir...`, `Le mauvais guichet`, `Signature obtenue, remise ailleurs`, `Transmission complémentaire`, `Le dernier paquet` et un dernier service demandé par une personne secourue. Une suite n'a donc plus besoin de révéler systématiquement un complot ou un danger supérieur.   
- Certaines de ces suites deviennent de **vrais déplacements inter-ville** : le journal conserve d'abord ce qui a été accompli en phase 1 et le déclencheur, puis indique la ville de remise. Arriver dans la bonne ville valide réellement la suite et le supplément de distance augmente sa récompense.   
- Protection débutant : une quête/révélation de **rang F reste locale**. Si une phase 2 normalement inter-ville apparaît alors qu'aucune autre ville n'est légalement accessible, elle bascule sur un relais local au lieu de créer un softlock.   
- `Quest`, `QuestMenu`, `QuestWorldMenuSupport`, `QuestPresentationSupport` et `SaveManager` conservent maintenant la route, le type de transport, la charge, les relais déjà effectués, la distance et les suppléments de récompense, ainsi que les destinations inter-ville des phases 2.   
- Nouveau `IntercityQuestSystemTest`; `QuestSecondPhaseSystemTest` et `SaveRoundTripTest` étendus. Tests ciblés `IntercityQuestSystemTest`, `QuestSecondPhaseSystemTest`, `SaveRoundTripTest`, `GuildQuestAccessPolicyTest`, `EconomyScaleTest` et `TeamEmploymentMissionSystemTest` : PASS.   
- Le runner global contient désormais **103 tests lancés / 103 attendus**. Il a été lancé pré-bump mais sa compilation complète dépasse la fenêtre de cette sandbox avant verdict final ; aucun faux 103/103 n'est revendiqué.   
- Bump `3.51.88 -> 3.51.89` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.88 — Quêtes à phase 2 dynamique et suites obligatoires/facultatives   

- Nouveau `QuestSecondPhaseSystem` : certaines quêtes ordinaires peuvent paraître complètement normales puis révéler, uniquement après la réussite de leur objectif initial, une **phase 2** liée à une conséquence, un témoin tardif, une piste cachée, une réaction adverse ou une dernière incohérence. L'existence de cette suite est déterministe par quête : rouvrir le menu ne reroll jamais la surprise.   
- Deux natures existent : **phase 2 obligatoire**, qui rouvre immédiatement la quête et bloque sa remise tant que la complication n'est pas résolue ; **phase 2 facultative**, que le joueur peut refuser et rendre la quête normalement, ou accepter afin d'obtenir une récompense supérieure.   
- Le journal conserve explicitement le contexte demandé : `Phase 1 terminée` rappelle **ce qui vient d'être accompli**, puis `Déclencheur` explique **ce qui a mené à la phase 2** avant d'afficher le nouvel objectif, son caractère obligatoire/facultatif, sa progression et son bonus.   
- Les suites actives possèdent une vraie interaction depuis le hub de quêtes : **approche prudente** (plus longue, plus fiable) ou **approche rapide** (moins de temps, davantage de risque). Un échec consomme réellement le temps sans donner la progression ni le bonus.   
- Terminer la phase 2 augmente réellement la récompense de la quête. Le bonus d'XP reste payé même sur un contrat dont l'objectif principal n'avait pas d'XP rédigée ; les récompenses physiques contractuelles restent intactes et ne sont jamais remplacées silencieusement.   
- Les phases 2 génériques excluent les quêtes principales, primes, défis de guilde, missions d'incident commercial et ordres contractuels d'une équipe employeuse : ces systèmes possèdent déjà leur propre cycle narratif/administratif et ne doivent jamais être bloqués par une complication aléatoire.   
- `Quest`, `QuestLog`, `QuestMenu` et `SaveManager` conservent maintenant l'offre/activation/refus/résolution de la phase 2, le résumé de la phase précédente, la cause, le nouvel objectif, les tentatives, la progression et le bonus sans changement de schéma de sauvegarde.   
- Nouveau `QuestSecondPhaseSystemTest`; `SaveRoundTripTest` étendu. Pendant le runner global, **101 tests** avaient déjà terminé en PASS et `TeamEmploymentMissionSystemTest` a révélé une vraie régression d'éligibilité ; celle-ci a été corrigée puis ce test, `QuestSecondPhaseSystemTest` et `SaveRoundTripTest` ont tous repassé. Le runner contient **102 tests lancés / 102 attendus**.   
- Bump `3.51.87 -> 3.51.88` (la fonctionnalité V3.51.87 a été reconstituée depuis le dernier backup complet V3.51.86 disponible dans cette session) : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.87 — Incidents commerciaux jouables et primes issues d'enquêtes   

- Nouveau `CityTradeIncidentMissionSystem` : un incident de convoi réellement présent dans `CityTradeFlowSystem` peut devenir une mission temporaire au Bureau d'information locale. Aucune mission n'est inventée sans route/incidence réelle.   
- Les interventions utilisent au minimum deux étapes : **préparation locale** puis action sur la route. L'approche prudente coûte davantage de temps mais augmente la fiabilité ; l'approche rapide économise du temps au prix d'un risque supérieur. Un échec consomme le temps et ne donne ni récompense ni résolution gratuite.   
- Les missions d'incident sont persistantes, expirent selon l'urgence et respectent les rangs de quête existants. Les vols/disparitions assez graves peuvent révéler une piste criminelle et publier ensuite une **prime distincte** au tableau de guilde, que le joueur reste libre d'accepter ou non.   
- La publication respecte la capacité normale du tableau et ne duplique jamais la prime spéciale. Les missions d'incident gardent les villes, la marchandise, le type d'incident, la gravité, l'étape, les tentatives et l'état de publication dans la sauvegarde.   
- Nouveau `CityTradeIncidentMissionSystemTest`; fonctionnalité reconstituée dans cette passe car le dernier backup complet disponible localement était V3.51.86.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.86 — Incidents commerciaux rares et conséquences de route   

- Nouveau `CityTradeIncidentSystem` : une route commerciale réellement active ou déjà perturbée peut désormais générer **rarement** un incident supplémentaire. Les incidents sont déterministes par route/cycle : rouvrir un menu ne reroll jamais le monde.   
- Incidents actuels : **renfort d'escorte recherché**, **cargaison partiellement volée**, **convoi introuvable**, **arrivée endommagée** et **route temporairement obstruée**. Un retard simple reste souvent un retard simple : tout trafic n'est pas transformé en catastrophe.   
- Les incidents modifient le **stock réel à destination** lorsqu'ils détruisent, immobilisent ou font disparaître une partie d'un arrivage. Une demande d'escorte ne crée aucune pénurie artificielle. Les convois prioritaires de crise restent mieux protégés et subissent un impact économique légèrement amorti.   
- `CityTradeFlowSystem::routeBetween(...)` expose maintenant l'identité, la gravité et les effets d'un incident éventuel. `pressureFor(...)` réutilise exactement le même incident : aucun second tirage indépendant n'est effectué entre l'économie et l'affichage.   
- Le Bureau d'information locale fait remonter les incidents importants parmi les flux commerciaux. Une demande d'escorte signale que des convoyeurs cherchent des bras ; un vol/convoi introuvable signale que les gardes et marchands possèdent des témoignages qui pourront alimenter une enquête ou une prime.   
- `BountyTradeTransitSystem` tient compte de cette couche : un convoi cherchant davantage de gardes devient moins confortable pour une fuite discrète ; une cargaison déjà volée peut au contraire être exploitée par une cible dangereuse à court de ressources, mais produit une trace particulièrement visible.   
- Aucun nouvel état de sauvegarde n'est nécessaire : les incidents sont dérivés de façon stable du jour, de la route, de la filière et du cycle commercial.   
- Nouveau `CityTradeIncidentSystemTest`; tests ciblés `CityTradeIncidentSystemTest`, `CityTradeFlowSystemTest`, `BountyTradeTransitSystemTest`, `BountyRelocationSystemTest`, `CityStockDynamicsSystemTest` et `CityEconomyDynamicsSystemTest` : PASS.   
- Le runner global contient désormais **100 tests lancés / 100 attendus**. Il a été lancé pré-bump mais sa compilation globale dépasse la fenêtre de cette sandbox avant verdict final ; aucun faux 100/100 n'est revendiqué.   
- Bump `3.51.85 -> 3.51.86` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.85 — Primes mobiles et convois commerciaux réels   

- Nouveau `BountyTradeTransitSystem` : les cibles de prime mobiles peuvent désormais exploiter les **routes commerciales réellement actives du jour** entre leur ville de départ et une destination accessible. Le système interroge `CityTradeFlowSystem` au lieu d'inventer une caravane pratique pour la fuite.   
- `CityTradeFlowSystem` expose maintenant `routeBetween(...)`, qui décrit une route précise (départ, arrivée, retard, priorité d'urgence, distance) tout en réutilisant le même réseau déterministe que les stocks/importations. Les exportations suspendues pendant un besoin critique ne deviennent donc pas magiquement des moyens de fuite.   
- Lors du choix d'une nouvelle ville, une cible peut préférer légèrement une destination reliée par un trafic exploitable. Une vraie opportunité commerciale peut aussi ajouter un petit bonus à la chance de relocalisation, plafonnée afin qu'un convoi ne garantisse jamais la fuite.   
- Selon ses ressources, sa mobilité et son danger, la cible peut : **se cacher dans un convoi marchand**, **soudoyer un conducteur**, profiter d'un **convoi retardé**, laisser des **témoins de route**, ou, si elle manque de moyens et reste dangereuse, **voler/infiltrer une cargaison**.   
- Ces choix créent de vraies traces inter-villes réutilisant l'enquête existante : le vol de cargaison laisse une trace plutôt fiable, tandis qu'un convoyeur soudoyé rend le passage plus discret. Aucune de ces traces ne donne une interception gratuite ; elles accélèrent seulement une future enquête locale.   
- La destination reste soumise à toutes les règles existantes : ville réellement accessible au joueur, anti-ping-pong, sécurité locale, distance, richesse/soutiens, réseaux de transport et marché noir. Le commerce enrichit la décision sans remplacer ces critères.   
- Nouveau `BountyTradeTransitSystemTest` et extension de `CityTradeFlowSystemTest`. Tests ciblés `CityTradeFlowSystemTest`, `BountyTradeTransitSystemTest`, `BountyRelocationSystemTest`, `BountyQuestSystemTest`, `CityStockDynamicsSystemTest` et `CityEconomyDynamicsSystemTest` : PASS.   
- Le runner global contient désormais **99 tests lancés / 99 attendus**. Il a été lancé pré-bump mais sa phase de compilation globale dépasse la fenêtre de cette sandbox avant verdict ; aucun faux 99/99 n'est revendiqué.   
- Bump `3.51.84 -> 3.51.85` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.84 — Flux commerciaux régionaux, importations et convois   

- Nouveau `CityTradeFlowSystem` : les villes utilisent désormais leurs spécialisations économiques pour alimenter de vrais **flux commerciaux inter-villes**. Une ville moins autonome dans une filière peut recevoir des arrivages d'une ville spécialisée, tandis qu'une ville productrice peut expédier une partie de son stock public.   
- Les routes sont déterministes et tiennent compte de la **distance**, de la **sécurité des villes** et d'un cycle de convois. Les arrivages peuvent être livrés normalement ou subir un retard ; une ville sans production locale ressent davantage un convoi manquant qu'une ville déjà spécialisée dans le domaine.   
- Les importations/exportations modifient le **stock réel des boutiques** via `CityStockDynamicsSystem`, en plus de la spécialisation et des événements existants. Les bonus restent bornés pour ne pas écraser les pénuries, et les exportations spécialisées exercent seulement une légère pression sur les stocks locaux.   
- Les crises savent désormais influencer le commerce : pendant une mobilisation anti-raid ou une reconstruction, les catégories critiques peuvent recevoir des convois prioritaires, tandis que les **exportations critiques locales sont suspendues** afin d'éviter qu'une ville vende ses dernières armes/matériaux pendant qu'elle en a elle-même besoin.   
- Le Bureau d'information locale affiche désormais jusqu'à quatre informations commerciales utiles : **arrivage**, **expédition**, **convoi retardé**, disponibilité renforcée ou tension associée. Aucun écran n'affiche toutes les routes en permanence.   
- Le système ne nécessite aucune nouvelle donnée de sauvegarde : les cycles de convois sont dérivés de façon stable du jour du monde, des villes et du type de marchandise.   
- Nouveau `CityTradeFlowSystemTest`. Les tests ciblés `CityTradeFlowSystemTest`, `CityStockDynamicsSystemTest` et `CityEconomyDynamicsSystemTest` passent. Le runner global contient désormais **98 tests** mais sa recompilation complète dépasse la fenêtre de cette sandbox avant verdict final ; aucun faux 98/98 n'est revendiqué.   
- Bump `3.51.83 -> 3.51.84` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.83 — Spécialisation des artisans, prix d’ami et phase QG niveau 3   

- Le forgeron du clan débloque maintenant au **niveau artisan 3** un choix de spécialisation permanent : **Forge d’armes**, **Armures et protections** ou **Entretien économique**. Ce choix appartient au PNJ et prépare l’intérêt futur d’avoir plusieurs postes/artisans complémentaires plutôt qu’un seul forgeron universel.   
- La spécialisation armes ajoute **+5 points** au plafond de restauration des armes avec la Forge I (plafond final 95 %) ; la spécialisation armures fait de même sur les protections ordinaires ; l’entretien économique ajoute **3 points de remise** et économise un matériau sur les grosses réparations qui en demanderaient au moins trois. Les armures magiques restent hors compétence de la Forge I.   
- Les artisans de clan commencent à récompenser l’investissement du joueur avec un **prix d’ami progressif** : 0 % aux niveaux 1–2, environ **4 % / 7 % / 10 %** aux niveaux 3/4/5 sur la main-d’œuvre du forgeron. Cela reste volontairement léger afin de ne pas tuer les commerces extérieurs.   
- Les devis de forge affichent maintenant le tarif normal, la remise de clan réellement appliquée et le coût final. La spécialisation est visible dans le menu de l’artisan et sauvegardée dans `clan_hq_blacksmith_specializations`.   
- Le QG possède désormais un plafond structurel explicite de **niveau 5**. La phase réellement ouverte dans cette version monte jusqu’au **niveau 3** ; les niveaux 4–5 restent verrouillés tant que leurs futurs modules/prérequis avancés n’existent pas.   
- Le passage **QG niveau 2 -> niveau 3** n’est pas achetable uniquement avec de l’argent : il demande **tableau de quêtes II**, **au moins deux modules de service réels** (caisse/bar/forge...) et **au moins 3 lits construits**. Le chantier coûte provisoirement **48 000 PC**, dure **10 jours** et demande beaucoup plus de matériaux.   
- Le QG niveau 3 augmente les plafonds à **8 lits** et **9 emplacements de modules**. Il ne donne toujours pas automatiquement les chambres : celles-ci doivent continuer à être construites séparément.   
- Les devis de chantier utilisent maintenant le coût/délai/matériaux correspondant à la phase structurelle visée, tout en conservant la compatibilité des appels existants pour le niveau 2.   
- `ClanBlacksmithSystemTest`, `ClanHeadquartersUpgradeSystemTest`, `SaveRoundTripTest` et les tests de fête/QG directement concernés passent. Le runner global démarre correctement mais sa recompilation des **97 tests** dépasse encore la fenêtre de cette sandbox ; aucun faux 97/97 n’est revendiqué.   
- Bump `3.51.82 -> 3.51.83` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.82 — Première forge de clan et forgeron évolutif   

- Nouveau vrai module **Atelier de forge niveau 1** pour le QG : nécessite le bâtiment niveau 2, occupe un emplacement de module, coûte provisoirement **18 000 PC**, dure **4 jours** et consomme de nombreux matériaux pour le foyer, l’enclume, les outils, le soufflet et les protections.   
- Construire l’atelier ne crée aucun artisan gratuit : le joueur doit recruter un **vrai forgeron de clan** pour **3 200 PC**. Son nom, son contrat, son expérience et son nombre de commandes sont persistants.   
- Le forgeron possède **5 niveaux de compétence** et progresse uniquement grâce aux vrais travaux effectués. Les seuils actuels sont 0 / 60 / 160 / 320 / 560 XP.   
- La forge I sert réellement à réparer les armes et armures de l’inventaire. Chaque réparation consomme des matériaux adaptés et une petite somme d’argent ; le travail donne ensuite de l’XP au forgeron.   
- Le plafond de réparation reste volontairement inférieur à celui des maîtres spécialisés : environ **60 % de durabilité max au niveau 1**, puis 68 / 76 / 84 / **90 % au niveau 5**. Le QG devient pratique sans rendre les forgerons de villes spécialisées inutiles.   
- Les matériaux de réparation dépendent de la pièce : métal pour la majorité des armes et armures métalliques, cuir pour les armures légères et certaines armes, cordage pour les arcs. Les quantités dépendent de la durabilité réellement restaurée.   
- Les **armures magiques** sont volontairement hors capacité de la Forge I : elles devront plus tard nécessiter un artisan/module spécialisé ou un service d’enchantement. Les objets indestructibles ne peuvent évidemment pas être réparés.   
- Le menu du QG affiche maintenant l’atelier, le candidat artisan, le forgeron recruté, son niveau/XP/commandes, son plafond de réparation et les devis de réparation des armes/armures. Les grosses confirmations utilisent les mots-clés `FORGERON` et `REPARER`.   
- Nouvelles catégories persistantes/protégées : `clan_hq_forge_workshop_upgrades`, `clan_hq_blacksmith_hires`, `clan_hq_artisans_joined`, `clan_hq_blacksmith_contracts`, `clan_hq_blacksmith_xp`, `clan_hq_blacksmith_jobs`, `clan_hq_blacksmith_payments` et `clan_hq_blacksmith_milestones`.   
- Nouveau `ClanBlacksmithSystemTest`. Les tests ciblés forge/forgeron, upgrades du QG, fêtes et `SaveRoundTripTest` passent. Le runner contient désormais **97 tests lancés / 97 attendus** ; sa recompilation globale dépasse la fenêtre de la sandbox avant verdict, donc aucun faux `97/97` global n’est revendiqué.   
- Bump `3.51.81 -> 3.51.82` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.81 — Confort du QG, décoration et fêtes de clan   

- Nouveau chantier **Confort & décoration niveau 1** pour le QG : nécessite le bâtiment niveau 2, coûte provisoirement **7 500 PC**, dure **2 jours** et consomme argile, métal, cuir/literie et cordages.   
- Cette amélioration améliore les pièces existantes (literie, mobilier, éclairage, décoration commune) et **ne consomme aucun emplacement de module/atelier**, afin de ne pas bloquer artificiellement les futurs ateliers de forge/alchimie.   
- Avec ce confort, le bar/salle commune soigne légèrement mieux : environ **10 % des PV max** au lieu d'environ 8 %, toujours une seule fois par jour et contre paiement. Aucun buff de combat permanent n'est ajouté.   
- Les **fêtes de QG** sont maintenant réellement organisables : au moins deux résidents, présence dans la ville du QG, **900 PC** de coût de base, réduit à **750 PC** si le bar est installé, et **14 jours** minimum entre deux grandes fêtes.   
- Une fête enregistre ses participants, donne seulement **1 à 3 points de réputation de clan** selon la taille du groupe et attribue à chaque résident présent une petite motivation sociale consommée sur sa prochaine mission autonome.   
- Cette motivation ajoute seulement **+2 points de pourcentage** à l'XP de la prochaine mission autonome, tout en restant strictement plafonnée par la règle globale **15–25 % de l'XP d'une quête active équivalente**. Elle ne donne ni argent direct ni bonus de combat.   
- Nouvelles catégories persistantes/protégées : `clan_hq_comfort_decor_upgrades`, `clan_hq_furnishings`, `clan_hq_festivals`, `clan_hq_festival_participants`, `clan_hq_festive_morale` et `clan_hq_festive_morale_consumed`.   
- Nouveau `ClanHeadquartersCelebrationSystemTest`; les tests ciblés confort/fête, bar, upgrades QG, rangs et save/reload passent. Le runner global contient désormais **96 tests lancés / 96 attendus** ; sa recompilation complète dépasse la fenêtre de cette sandbox, donc aucun faux `96/96` global n'est revendiqué.   
- Bump `3.51.80 -> 3.51.81` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.80 — Moyenne de rang à 5+ membres et bar / salle commune du QG   

- La politique centrale `GuildQuestAccessPolicy` distingue désormais les **petits groupes de 1 à 4 membres** des groupes plus nombreux. Jusqu’à 4 membres, le plafond reste strict : en solo le rang propre est la limite ; en groupe, le membre le plus faible fixe au maximum son rang +1.   
- À partir de **5 membres**, le plafond est calculé sur le **rang moyen du groupe**, arrondi au rang le plus proche. Si cette moyenne est à **au plus un rang du meilleur membre**, le groupe peut viser **moyenne +1** ; sinon le plafond reste la moyenne. Cela permet aux grandes équipes cohérentes d’être un peu plus ambitieuses sans qu’un ou deux très hauts rangs ne tirent artificiellement plusieurs débutants trop haut.   
- La règle moyenne 5+ est réutilisée par les quêtes de groupe du joueur et les missions autonomes du QG. Un test d’intégration vérifie notamment **4 rang E + 1 rang F -> plafond D** au QG.   
- Les écrans de guilde et de groupe expliquent maintenant cette différence de calcul au lieu d’afficher l’ancien message générique « +1 rang maximum ».   
- Nouveau vrai module **Bar / salle commune niveau 1** pour le QG : exige un QG niveau 2 et un emplacement de module libre, coûte provisoirement **10 500 PC**, dure **3 jours** et consomme des matériaux de construction/aménagement.   
- Une visite au bar coûte provisoirement **35 PC**, peut rendre une petite quantité de PV et enregistre un moment social avec les résidents présents. Le service est limité à **une utilisation par jour du monde**, donc aucune boucle de soins gratuits ou de buff permanent n’est créée.   
- Nouvelles catégories persistantes protégées : `clan_hq_common_room_bar_upgrades`, `clan_hq_bar_visits` et `clan_hq_social_moments`. `SaveRoundTripTest` vérifie désormais aussi le module bar après sauvegarde/rechargement.   
- Nouveaux tests `ClanHeadquartersCommonRoomSystemTest` et `ClanHeadquartersRankPolicyTest`. Les validations ciblées `GuildQuestAccessPolicyTest`, QG de base/upgrades, bar, politique 5+, sauvegarde/rechargement et roster recrues passent. Le runner global contient désormais **95 tests lancés / 95 attendus**, mais sa recompilation complète dépasse la fenêtre de cette sandbox ; aucun faux `95/95` n’est revendiqué.   
- Bump `3.51.79 -> 3.51.80` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   


## V3.51.79 — Rangs de quêtes cohérents, XP autonome proportionnelle et caisse du QG   

- Nouvelle politique centrale `GuildQuestAccessPolicy` : en **solo**, le joueur ne peut accepter que des quêtes jusqu’à son propre rang de guilde ; avec un **vrai groupe actif**, il peut tenter au maximum **un rang au-dessus**. Les restrictions de fiabilité orange/rouge peuvent toujours abaisser ce plafond.   
- Les quêtes de groupe ne se contentent plus de la moyenne de rang : **chaque membre présent** doit individuellement respecter la limite `+1 rang`. Un allié de rang F ne peut donc plus être tiré vers une quête D simplement parce qu’un aventurier très haut rang l’accompagne.   
- La même règle s’applique aux résidents du QG : une recrue seule reste limitée à son rang ; au moins deux recrues disponibles peuvent former un groupe et accepter au maximum `+1 rang`, dans la limite du niveau du tableau de quêtes.   
- Les missions autonomes du QG ne donnent plus un montant fixe de 5/7 XP. Elles accordent désormais **15 à 25 % de l’XP estimée d’une quête active équivalente**, selon le rang de la mission et le niveau estimé de la recrue, tout en restant bien moins rentables que jouer réellement avec elle.   
- Première **caisse de missions** réellement constructible : exige **QG niveau 2 + tableau niveau 2 + un emplacement de module libre**, coûte provisoirement **9 000 PC**, dure **2 jours** et consomme des matériaux. Sa capacité initiale est plafonnée à **2 500 PC**.   
- Les missions autonomes peuvent verser une petite part de leurs gains dans la caisse, mais **aucun argent n’est crédité directement au joueur**. Il faut revenir physiquement dans la ville du QG pour récupérer le contenu. Si la caisse est pleine, l’excédent n’est pas stocké et les aventuriers restent libres de prendre des contrats de guilde.   
- Persistance canonique ajoutée via `clan_hq_mission_chest_upgrades`, `clan_hq_mission_chest_deposits` et `clan_hq_mission_chest_withdrawals`, désormais également protégées contre les nettoyages génériques du journal. `SaveRoundTripTest` vérifie installation, dépôt, retrait et recharge de la caisse.   
- Nouveau `GuildQuestAccessPolicyTest`; `ClanHeadquartersUpgradeSystemTest` vérifie aussi la caisse, les rangs de missions autonomes et la récupération manuelle. Les tests directement touchés passent. Le runner global démarre/attend désormais **93 tests**, mais sa recompilation complète dépasse la fenêtre d’exécution de cette sandbox ; aucun faux `93/93` n’est revendiqué pour cette passe.   
- Bump `3.51.78 -> 3.51.79` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.78 — Agrandissement réel du QG, chambres et tableau niveau 2   

- Le QG de clan peut désormais être **agrandi réellement au niveau 2** après sa construction initiale. Le chantier coûte provisoirement **26 000 PC**, dure **7 jours du monde** et consomme un volume important d'argile, métal, cuir et cordages.   
- L'agrandissement niveau 2 porte la structure à **5 lits maximum** et **6 emplacements de modules**, mais ne construit pas gratuitement toutes les chambres : les deux lits de départ restent présents et chaque chambre supplémentaire doit être financée séparément.   
- Les **chambres supplémentaires** sont maintenant jouables une par une : **4 500 PC**, **2 jours de travaux** et des matériaux dédiés. Le bâtiment niveau 2 permet ainsi de monter progressivement de 2 à 5 résidents permanents.   
- Le **tableau de quêtes niveau 2** est maintenant constructible pour **12 000 PC** et **3 jours** après l'agrandissement. Il ouvre des contrats autonomes de clan jusqu'aux rangs **E/D** et fait passer le gain actuel d'une petite mission autonome de **5 à 7 XP de recrue**.   
- Un seul chantier d'amélioration du QG peut être actif à la fois. Les travaux doivent être commandés depuis la **ville d'implantation du QG**, afin que le bâtiment reste ancré physiquement dans le monde.   
- Les gains autonomes restent volontairement limités : **aucun revenu AFK direct n'est encore versé au joueur**. La future caisse/banque de missions plafonnée restera le point d'entrée des gains récupérables et le QG ne remplace toujours pas la guilde.   
- Le menu du QG affiche maintenant le niveau du bâtiment, la capacité actuelle/maximale des lits, la capacité de modules, le niveau du tableau, le chantier actif, son jour de fin et les devis complets avant confirmation par le mot-clé `TRAVAUX`.   
- Persistance canonique ajoutée pour les chantiers via `clan_hq_upgrade_jobs_started` et `clan_hq_upgrade_jobs_completed`, sans changement de schéma. `SaveRoundTripTest` vérifie désormais un QG niveau 2, une troisième chambre, un tableau niveau 2 et un chantier suivant encore en cours après sauvegarde/rechargement.   
- Nouveau `ClanHeadquartersUpgradeSystemTest`. La suite complète exécute et attend désormais explicitement **92 tests C++** : **92/92 PASS**, code retour 0, avec « Tous les tests Dinotofu sont passés. » avant le bump.   
- Bump `3.51.77 -> 3.51.78` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.77 — Premier QG de clan, recrues conservées et historique des titres   

- Nouveau `ClanHeadquartersSystem` : un personnage peut lancer la construction de son **unique QG de clan** après avoir validé le jeu de groupe de la guilde et enregistré un nom de clan. Le chantier demande argent, matériaux et plusieurs jours du monde.   
- Le QG de base ouvre volontairement petit : **comptoir, tableau de quêtes niveau 1 et deux lits/chambres de recrues**. La ville d'implantation est persistante et un second QG ne peut pas être lancé en parallèle ou après construction.   
- Les recrues appréciées peuvent être rattachées au QG sans devoir rester dans l'équipe active. Elles restent dans l'entourage du clan ; quitter une chambre libère une place sans effacer leur historique.   
- Le tableau de base traite de petites missions autonomes environ tous les trois jours. Une recrue actuellement avec le joueur, blessée ou en attente d'évacuation ne peut pas simultanément farmer ces missions. La progression autonome est volontairement lente et ne verse aucun revenu AFK direct au joueur ; une future caisse/banque de quêtes gérera des gains récupérables plafonnés.   
- L'XP réelle enregistrée dans `xp_recrues` participe maintenant au **niveau estimé des recrues**. Elles peuvent donc réellement progresser hors groupe via le QG, mais beaucoup moins vite qu'en aventure active.   
- Le menu d'équipe expose désormais le QG, son chantier, ses places de résidents, son activité autonome et une feuille de route explicite pour les futurs agrandissements : taille générale, lits, tableau supérieur, bar, caisse, confort/fêtes, forge, alchimie et artisans. Ces modules ne sont pas encore actifs.   
- Les titres débloqués au niveau du compte conservent maintenant, pour les nouveaux déblocages, la **date/heure de déblocage** et le **nom du personnage** qui les a obtenus. L'archive des titres affiche ces informations tout en restant compatible avec les anciens enregistrements ne contenant que le nom du titre.   
- Les catégories persistantes du QG sont protégées dans le journal canonique. Nouveau `ClanHeadquartersSystemTest`; extensions de `AccountTitleProgressTest` et `RecruitedAllyCombatRosterTest`.   
- Bump `3.51.76 -> 3.51.77` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.76 — Stocks locaux réels et pistes inter-ville   

- Nouveau `CityStockDynamicsSystem` : les spécialisations d'une ville modifient maintenant le **stock physique réellement proposé** par certains commerces, pas seulement les prix ou la qualité abstraite. Une ville spécialisée peut disposer de davantage d'offre dans son domaine même si elle est moins riche.   
- Les événements locaux et leurs préparatifs peuvent tendre ou enrichir temporairement les stocks : une mobilisation anti-raid raréfie notamment matériaux, armes, armures, forge et consommables, tandis qu'une foire ou une fête agricole peut élargir certaines disponibilités.   
- La spécialisation amortit une pénurie sans l'annuler : Rocheveille peut mieux encaisser une tension militaire qu'une ville généraliste, mais une mobilisation importante reste visible dans ses rayons. Les stocks illimités ne sont pas artificiellement rendus finis et un article déjà épuisé n'est pas recréé gratuitement.   
- L'interface de boutique indique désormais la situation locale de disponibilité lorsqu'un profil de stock particulier s'applique.   
- Les cibles de prime qui changent réellement de ville laissent maintenant une **piste inter-ville persistante** : registre de diligence, témoin routier, achat suspect, contact criminel, passeur, etc.   
- Certaines cibles expérimentées qui savent être poursuivies peuvent au contraire préparer une **fausse piste**. Une piste possède une fiabilité et n'est exploitable qu'une fois : elle aide l'enquête mais ne révèle jamais gratuitement l'interception finale.   
- Les méthodes d'enquête n'exploitent pas toutes ces traces avec la même efficacité : interroger ou contrôler les sorties est plus adapté qu'une préparation d'embuscade déjà locale. Une mauvaise piste peut ajouter du bruit à l'enquête.   
- L'écran de suivi d'une prime signale lorsqu'une piste inter-ville reste encore exploitable.   
- Sauvegarde : ajout optionnel de `bountyTransitTrailKind`, `bountyTransitTrailFromCityId`, `bountyTransitTrailReliability` et `bountyTransitTrailAvailable`, sans changement du schéma de sauvegarde.   
- Nouveau `CityStockDynamicsSystemTest`; extensions de `BountyRelocationSystemTest`, `BountyQuestSystemTest` et `SaveRoundTripTest`. Les quatre tests directement touchés passent après intégration. Le runner global de 90 tests a été lancé mais sa recompilation complète dépasse la fenêtre d'exécution de l'environnement ; aucune régression fonctionnelle n'a été observée dans les tests ciblés.   
- Bump `3.51.75 -> 3.51.76` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.75 — Événements locaux vivants et fugitifs inter-ville   

- Le cycle d’événements locaux passe de 8 à **10 temps forts**, sans transformer chaque journée en festival : ajout d’une **Fête des jardins** communautaire et d’une **mobilisation contre un raid annoncé**.   
- La Fête des jardins crée une demande temporaire en plantes, graines, fleurs, récoltes décoratives, petit bois/aménagement et ingrédients botaniques. Elle peut aussi donner de petites récompenses communautaires lorsqu’on aide sur place.   
- Les préparatifs d’un futur raid créent une vraie tension logistique avant l’attaque : forte demande en **fer/matériaux de barricade, armes, armures, forge et consommables**. Le service d’information local signale ces besoins avant l’événement.   
- Les aides communautaires existantes tiennent maintenant compte de ces nouveaux événements selon les aptitudes/races pertinentes, sans faire du joueur le commandant automatique d’un raid.   
- Nouveau `BountyRelocationSystem` : certaines cibles expérimentées, dangereuses ou très mobiles peuvent désormais **quitter réellement une ville après une évasion** et chercher refuge dans une autre région.   
- Le choix de destination tient compte de la sécurité locale, des spécialisations marché noir/transport, de la richesse, de la distance et d’un léger aléa déterministe. Les zones instables/chaotiques sont naturellement plus attractives pour une fuite que les villes très surveillées.   
- Une cible ne peut pas se déplacer derrière une ville encore inaccessible au joueur à cause d’un niveau, d’un boss ou d’un laissez-passer. Les villes déjà visitées par la cible sont mémorisées afin d’éviter les ping-pong absurdes.   
- Lors d’un changement de ville, les préparatifs trop précis de l’ancienne zone expirent : la cible conserve une piste générale, mais le joueur doit reconstruire son renseignement local au lieu de bénéficier d’une omniscience gratuite.   
- Les primes acceptées depuis une autre ville reçoivent maintenant une **ville courante et un refuge/biome courant persistants** ; l’ancien fallback « Route commerciale » n’enferme plus artificiellement les contrats à Valebrume.   
- L’adaptation régionale d’une cible est réappliquée une seule fois lorsqu’elle entre dans une nouvelle ville, sous forme de delta, pour éviter d’empiler indéfiniment les bonus/malus de sécurité. Compatibilité assurée avec les sauvegardes V3.51.74 qui possèdent l’ancien booléen sans le nouvel identifiant de ville.   
- Sauvegarde : ajout optionnel de `bountyCurrentCityId`, `bountyCurrentBiome`, `bountyVisitedCityIds`, `bountyRelocationCount` et `bountyRegionalProfileCityId`, sans changement de schéma.   
- Nouveau `BountyRelocationSystemTest`; extensions de `CityEconomyDynamicsSystemTest`, `BountyRegionalConsequenceSystemTest` et `SaveRoundTripTest`. Suite pré-bump : **89/89 tests C++ + contrôles statiques passés**.   
- Bump `3.51.74 -> 3.51.75` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.74 — Conjoncture locale, besoins commerciaux et filières de revente   

- Nouveau `CityEconomyDynamicsSystem` : la richesse **structurelle** d’une ville est désormais séparée de sa richesse **effective/temporaire**. Une foire, une fête, une préparation locale ou une reconstruction peut créer un boom ou une baisse passagère sans réécrire l’identité de fond de la ville.   
- La ville revient naturellement vers son niveau structurel lorsque la pression temporaire disparaît. Les futures règles de gouvernance pourront prolonger ou répéter ces écarts sans transformer arbitrairement « riche » en « meilleur partout ».   
- Les événements hebdomadaires et leurs préparatifs créent maintenant des **demandes commerciales ciblées** : chasse aux monstres, foire marchande, journée du savoir, bourse aux missions, jeux, fête des moissons, etc. Vendre la bonne catégorie pendant ce besoin donne un bonus borné.   
- Une reconstruction réelle peut temporairement appauvrir l’activité ressentie et dégrader certains services tout en créant une forte demande en matériaux, outils, plantes de soin et consommables.   
- Le **Bureau d’information locale** expose richesse structurelle/actuelle, sécurité, spécialités, conjoncture, besoins du moment et horizon de crise/préparation.   
- Les matériaux sont maintenant mieux valorisés chez le **bon acheteur** : plantes chez herboriste/alchimiste, renseignements chez librairie, outils chez forgeron, etc. Les composants de monstres sont particulièrement adaptés au `MonsterMaterial`.   
- La guilde possède un **Comptoir de collecte de matériaux de monstres** accessible depuis son menu. Une boutique spécialisée de même filière peut jouer exactement le même rôle. Les autres commerces restent capables de racheter ces matériaux lorsqu’ils sont compatibles, mais moins avantageusement.   
- Les calculs de revente utilisent un arrondi plus propre afin qu’un très petit bonus de spécialisation ne disparaisse pas systématiquement sur les objets bon marché.   
- La qualité de l’auberge est maintenant affectée par la conjoncture temporaire en plus du profil structurel de la ville.   
- Nouveau `CityEconomyDynamicsSystemTest`; le runner attend désormais explicitement `CityProfileSystemTest` et ce nouveau test. Suite complète pré-bump : **tous les tests Dinotofu + contrôles statiques passés**.   
- Bump `3.51.73 -> 3.51.74` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.73 — Villes vivantes : sécurité, richesse, spécialisations et valeur des primes   

- Nouveau `CityProfileSystem` : chaque ville peut maintenant porter un niveau de **sécurité** (`très sécurisée`, `surveillée`, `neutre / indépendante`, `instable`, `anarchique / chaotique`) et un niveau de **richesse** distinct. Ces deux axes ne sont pas confondus.   
- Les villes possèdent des **spécialisations économiques** par type de commerce/service avec une force propre. La spécialisation est volontairement capable de dépasser la richesse brute : **Rocheveille**, ville seulement modeste mais spécialisée forge/armes/armures, propose un meilleur contexte d'armement qu'une ville riche généraliste.   
- Première répartition jouable : Valebrume = artisanat/plantes/survie ; Rocheveille = forge/armes/armures ; Port-Lanterne = transport/marchandises/réseaux parallèles ; Lys-de-Givre = enchantement/alchimie/soins rares.   
- Prix d'achat, valeur de revente et **qualité des services** reçoivent maintenant le contexte de ville. L'auberge est le premier service complet à exploiter la qualité locale : efficacité du repas et plafond de récupération d'un lit varient dans des bornes contrôlées.   
- Le profil local est visible dans le hub/réseau des villes, les boutiques et les écrans de prime : richesse, sécurité et spécialisation ne sont plus des valeurs cachées.   
- Les primes ciblées ont désormais une valeur persistante calculée une seule fois à partir de la **dangerosité**, de la **notoriété/importance de la cible** et de son **temps de cavale**. Le bonus total reste borné pour protéger l'échelle PF/PC et éviter qu'une prime extrême casse l'économie.   
- Le niveau de sécurité de la ville influence aussi les fugitifs : une zone très sécurisée réduit leur soutien local et leur mobilité mais augmente leur vigilance ; les profils instables/chaotiques font l'inverse sans rendre les criminels omniscients.   
- Les carnets d'auberge restent réellement moins chers que cinq achats séparés après application des nouveaux prix locaux ; le carnet de repas est légèrement rééquilibré pour conserver ce principe.   
- Sauvegarde : nouveaux champs de valorisation des primes (`bountyDangerRating`, `bountyNotorietyRating`, `bountyDaysAtLarge`, `bountyBaseRewardGold`, `bountyRewardProfileApplied`) persistés sans changement de schéma.   
- Nouveau `CityProfileSystemTest`, extensions de `BountyQuestSystemTest`, `BountyRegionalConsequenceSystemTest`, `SaveRoundTripTest` et `EconomyScaleTest`. Suite complète : **87/87 tests C++ + contrôles statiques passés** avant et après bump.   
- Bump `3.51.72 -> 3.51.73` : schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

---   

## V3.51.72 — Primes ciblées : mémoire régionale, capture vivante et conséquences durables   

- Nouveau `BountyRegionalConsequenceSystem` : chaque région conserve désormais une mémoire réelle des primes ciblées résolues localement, sans rendre les criminels omniscients.   
- Les confrontations distinguent maintenant trois issues : **reddition et capture vivante**, **capture vivante après combat**, ou **approche létale assumée**. Un refus de reddition ne bascule jamais automatiquement le joueur en mode létal : la suite reste une tentative de capture tant qu'il ne choisit pas explicitement le contraire.   
- L'issue est persistée dans la quête (`captured_surrender`, `captured_combat`, `killed_combat`) et protégée contre les doubles applications après sauvegarde/rechargement.   
- Les captures vivantes, résolutions mortelles, traques très bruyantes et anciennes évasions sont mémorisées **par ville/région** dans le journal canonique.   
- Une prime résolue modifie réellement la **réputation locale** et peut tendre les contacts criminels que le joueur connaît déjà dans cette ville : pression en hausse, coopération en baisse. Aucun contact inconnu n'est révélé ou créé artificiellement.   
- Chaque issue génère une rumeur persistante crédible sur le sort réel de la cible.   
- Les futures cibles de la même région s'adaptent une seule fois à cet historique : vigilance accrue, soutien local pouvant se tarir après des captures propres ou au contraire se resserrer après des chasses violentes/bruyantes, ressources et mobilité influencées par les précédents locaux. Les valeurs restent bornées.   
- Les nouvelles données de prime sont optionnelles dans les sauvegardes : **schéma 26 inchangé** et anciennes sauvegardes compatibles.   
- `BountyRegionalConsequenceSystemTest` couvre capture vivante, réaction d'un contact criminel connu, rumeur, anti-double-application, adaptation d'une future cible, issue mortelle/bruyante et apprentissage lié aux évasions. `SaveRoundTripTest` couvre aussi les nouveaux champs persistants.   
- Canon Hestia complété sans en faire le centre de la version : après un peu de découverte du monde avec le joueur, elle peut goûter les **pammes**, les adore et finit par en avoir une obsession comique et affectueuse, sans dépendance, jauge, manque ni obligation. La règle d'univers « lot de pammes = prix unitaire légèrement inférieur » est également consignée dans `LORE_HESTIA_DINOTOFU.txt`.   

- Build C++23 pré/post-bump : **OK**. Suite complète : **86 / 86 tests C++ PASS** via runner isolé avec stdin fermé ; contrôles statiques du projet : **PASS**.   
- Bump `3.51.71 -> 3.51.72` : **OK**. Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

## V3.51.71 — Anatomie étendue et failles de monstres réellement différenciées   

- `MonsterAnatomySystem` ne se limite plus au squelette suicidaire, au squelette ordinaire et au slime : l’action tactique **« Viser une faille de corps »** reconnaît maintenant plusieurs grandes familles du bestiaire à partir de leur race et de leur anatomie crédible.   
- Les **constructions** exposent articulations mécaniques ou jonctions de plaques : réussite = lecture des stats, affaiblissement léger, ouverture du prochain coup et rupture éventuelle d’une posture défensive.   
- Les **dragons** peuvent être lus à la base des ailes ; les **draconides/semi-dragons** aux jointures d’écailles. Ces zones ne transforment pas la cible en papier mais créent une courte fenêtre tactique.   
- Les **plantes** possèdent un collet racinaire pouvant casser leur ancrage ; les **insectoïdes** ciblent articulation de patte ou joint de carapace selon leur protection ; les **bêtes cuirassées/coquillées** exposent les bords mobiles ou charnières de leur protection naturelle.   
- Les **élémentaires** possèdent un foyer énergétique temporairement lisible ; les **esprits/anges/démons** un ancrage spirituel. Ces réussites peuvent révéler la cible et appliquer une petite vulnérabilité cohérente avec leur nature.   
- Les **anomalies arcaniques** disposent désormais d’un nœud instable : le point faible est réellement utile, mais le toucher provoque aussi un contrecoup de dégâts et une brève hallucination. Une faille n’est donc pas toujours une option gratuite.   
- Le **Squelette suicidaire explosif** conserve son comportement dangereux existant : son crâne piégé reste prioritaire et peut toujours exploser sur le joueur avec poison, brûlure et hallucination.   
- Les effets anatomiques restent volontairement modestes car l’action tactique applique déjà ses propres bonus génériques : le nouveau système ajoute de la personnalité aux créatures sans doubler brutalement la puissance du joueur.   
- `MonsterAnatomySystemTest` couvre maintenant les réactions positives et négatives : squelette piégé/normal, construction standard et blindée, dragon, draconide, plante, insectoïde cuirassé, bête cuirassée, élémentaire, esprit, anomalie et humanoïde générique.   

- Build C++23 post-bump : **OK**, `Dinotofu V3.51.71` confirmé. Les **85 tests C++** référencés par `test_project.sh` ont été compilés/exécutés avec succès via un runner isolé afin de contourner un problème de job-control propre au conteneur ; les contrôles statiques du même script passent également intégralement.   
- Bump `3.51.70 -> 3.51.71` : **OK**. Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   

## V3.51.70 — Hestia combattante, artefact réellement équipé et anatomie tactique   

- Nouveau `HestiaCompanionSystem` : lorsqu’Hestia voyage réellement avec le joueur, elle occupe maintenant **un vrai emplacement d’allié actif** au lieu de fournir un soutien gratuit. Elle ne crée ni part de récompense ni faux salaire/recrue persistante.   
- Le joueur choisit librement sa **classe actuelle**, son style d’arme et sa protection. Le jeu ne révèle pas automatiquement qu’Hestia est faite pour la magie : elle commence mauvaise partout, magie comprise, puis ses séances, combats et éveils laissent progressivement apparaître l’écart.   
- Diane propose un entraînement facultatif pour Hestia lorsqu’elle est présente à la guilde ou dans l’équipe. Une séance consomme du temps réel du monde mais aucun entretien n’est imposé si le joueur préfère simplement la laisser à Diane.   
- Hestia peut désormais provoquer de vraies **maladresses de combat** : tour perdu ou dégâts alliés accidentels. Ces dégâts directs sont plafonnés afin qu’une maladresse seule ne puisse pas tuer le joueur ou une recrue. L’entraînement, l’expérience, la confiance et les éveils réduisent progressivement le risque.   
- Les trois éveils lumineux/divins peuvent maintenant se produire pendant ses actions magiques lorsque les conditions de progression sont réunies, en combat normal comme contre les boss. Le troisième éveil appelle réellement son retour aux cieux.   
- `HestiaSoulArtifactSystem` est relié au véritable équipement : l’artefact peut être équipé comme une arme réelle, est indestructible, lié à l’âme, unique/personnel et se met au repos sous forme de gant. Équiper une arme normale le remet au repos.   
- Les dégâts de l’artefact suivent réellement `3 × niveau`. Les formes arc/arbalète matérialisées ne consomment pas de munitions ordinaires. Une âme n’est comptée que si **l’impact direct de l’artefact** porte réellement le coup fatal : poison, brûlure, munition secondaire ou autre effet retardé ne valide pas le kill.   
- Le menu d’équipement expose maintenant l’artefact : équiper/reposer, alterner les formes et, au niveau 16, choisir définitivement une seconde forme puis passer librement de l’une à l’autre.   
- Nouveau système générique `MonsterAnatomySystem` branché sur l’action tactique **« Viser une faille de corps »**. Il fournit des points faibles/réactions réutilisables pour le bestiaire au lieu de scripts isolés par quête.   
- Le **Squelette suicidaire explosif** possède désormais un crâne réellement piégé : réussir à viser cette faille le fait exploser sur le joueur, détruit le squelette et peut infliger dégâts, poison, brûlure et hallucination. Des bases anatomiques existent aussi pour squelette ordinaire et slime.   
- Les choix/configurations d’Hestia et l’état équipé de l’artefact survivent au rechargement via les données persistantes existantes, sans changement du schéma de sauvegarde 26.   
- Nouveaux tests : compagnon Hestia, découverte non spoilée de ses aptitudes, slot de combat, maladresses/éveil, artefact réellement équipé et progressif, persistance, anatomie et crâne explosif.   

- Build C++23 pré-bump : **OK**. Suite complète pré-bump : **TOUS LES TESTS DINOTOFU PASSENT (`TEST_EXIT=0`)**.   
- Bump `3.51.69 -> 3.51.70` : **OK**, schéma 26, checkpoint V3.51.15 et catalogue titres V3.51.51 conservés. Rebuild post-bump : **OK**, `Dinotofu V3.51.70` confirmé. Suite complète post-bump : **TOUS LES TESTS DINOTOFU PASSENT (`POST_BUMP_TEST_EXIT=0`)**.   

## V3.51.69 — Disparition d'Hestia jouable, maison active et sacrifice réel   

- La rencontre d'Hestia n'apparaît plus tôt dans la partie : `HestiaMissingPersonQuestSystem` exige désormais **20 quêtes réellement validées** après le début de l'histoire avant que Diane ressorte un ancien dossier. Le journal présente seulement **« Une personne disparue »** ; aucun nom divin, pouvoir de lumière ou destin exceptionnel n'est révélé.   
- La piste est réellement jouable depuis l'exploration : sous-sol abandonné, approche frontale ou infiltration difficile, puis variante stable par personnage entre bande de gobelins + **Gobelin ninja** et morts-vivants + **Squelette suicidaire explosif**. La quête n'a pas de délai et peut rester ignorée sans bloquer les autres chapitres.   
- Après le sauvetage, Hestia peut voyager avec le joueur, vivre chez lui ou rester auprès de **Diane**. Ces choix sont réversibles. Laisser Hestia à Diane ou à la maison ne crée **aucun entretien**, aucune perte de confiance, aucune culpabilisation, aucun rappel obligatoire et aucune pénalité de progression.   
- La guilde possède désormais une entrée `Diane / Hestia` lorsque cela a du sens : visite facultative, retour dans l'équipe ou transfert vers la maison. La maison propose les mêmes changements de résidence sans forcer Hestia dans le roster actif.   
- Le `PlayerHomeSystem` devient utilisable : achat avec loyer ou achat définitif prélèvent maintenant réellement le coût initial. Le logement avec loyer suspend seulement ses services gratuits à échéance ; **aucun arriéré ne s'accumule**, aucune dette/expulsion n'est créée et les résidents restent en sécurité. Un paiement réactive simplement 7 jours de services.   
- Dormir dans sa maison, lorsqu'elle se trouve dans la ville actuelle et que ses services sont actifs, est gratuit et restaure jusqu'à 90% des PV ainsi que le mana. Une seule maison principale reste autorisée par personnage.   
- `PlayerDeathResolver` est enfin actif pour Hestia : après son premier éveil, si elle voyage réellement dans l'équipe au moment où le joueur tombe, son sacrifice se déclenche **avant les autres protections de mort** en PvE standard, exploration et boss standard. Le joueur est extrait et réanimé à 20% PV ; Hestia perd sa matérialisation. Aucun sacrifice à distance depuis Diane ou la maison.   
- Nouveaux tests : seuil des 20 quêtes et absence de spoiler, variantes de défense/infiltration, logement payant sans dette cumulative, absence de dégradation d'Hestia et sacrifice réel via le résolveur de mort.   

- Build C++23 pré-bump : **OK**. Suite complète pré-bump : **TOUS LES TESTS DINOTOFU PASSENT (`TEST_EXIT=0`)**.   
- Bump `3.51.68 -> 3.51.69` : **OK**, schéma 26 et checkpoint V3.51.15 conservés. Rebuild post-bump : **OK**, `Dinotofu V3.51.69` confirmé. Suite complète post-bump : **TOUS LES TESTS DINOTOFU PASSENT (`POST_BUMP_TEST_EXIT=0`)**.   

## V3.51.68 — Hestia persistante, logement, artefact d'âmes, Arone et doctrine Reaper   

- Nouvelle fondation persistante `HestiaArcSystem` : Hestia est sauvée dans un sous-sol abandonné, enchaînée, inconsciente et au bord de la mort. Deux variantes de sauvetage sont décrites (gobelins + ninja / squelettes + suicidaire explosif), avec une route de discrétion difficile.   
- Hestia peut ensuite être active dans l'équipe, rester auprès de **Diane** à l'accueil de la guilde ou vivre dans la maison du joueur. Les visites font progresser sa confiance ; à domicile elle peut rapporter rarement de petites trouvailles sans puissance notable. Les visites sont uniquement un bonus de confiance : aucune absence ne fait baisser la relation, ne crée de malus ou ne transforme Hestia en corvée d'entretien.   
- Sa progression respecte désormais la réadaptation : elle commence réellement mauvaise partout, **magie comprise**. Sa magie divine/lumineuse ne devient exceptionnelle qu'à travers trois grands éveils ; le troisième lui rend toute sa mémoire et met fin à sa matérialisation terrestre par retour aux cieux.   
- Après le premier éveil, `HestiaArcSystem` autorise son sacrifice uniquement si elle est réellement présente/active dans l'équipe : elle peut alors perdre sa vie/matérialisation pour téléporter le joueur en sécurité. Aucun sauvetage à distance depuis la guilde ou la maison.   
- Les anciens bonus automatiques donnant à Hestia une magie ou des soins anormalement puissants dès son seul nom ont été retirés. Son bestiaire/rumeur ne révèle plus sa nature divine à l'avance et l'ancien lien **Hazak sauve/protège Hestia** a été supprimé : dans Dinotofu, son sauveur est le joueur.   
- Nouveau `PlayerHomeSystem` : une seule maison principale par personnage, dans une seule ville, avec fondation d'une formule à loyer ou d'un achat définitif bien plus coûteux. Hestia peut y cohabiter avec un conjoint PNJ ou joueur. Les états mariage sont posés sans encore implémenter la quête de bague et tous les services du conjoint.   
- Nouvel `HestiaSoulArtifactSystem` : artefact offert à terme sous forme de **gant fixé à la peau au repos**. Il prend une forme principale adaptée au porteur, inflige `3 × niveau` dégâts de base et ne progresse que si l'artefact lui-même porte le coup fatal. Une famille d'âme trop absorbée cesse définitivement de nourrir l'arme afin d'empêcher le farm trivial.   
- La progression de l'artefact demande `niveau actuel` âmes avant le niveau suivant ; au niveau 16, le joueur choisit une seconde forme permanente, puis peut alterner librement entre les deux. Niveau, compteur, formes et essences absorbées sont sauvegardés sans changement du schéma 26.   
- **Arone Aster** rejoint le catalogue des personnages spéciaux (13/02/1313, humain, guerrier protecteur/anti-autorité absolue), sans spoiler public sur son histoire familiale.   
- Nouveau `ReaperFamilySecretSystem` : le nom enterré de Reaper-13 ne peut jamais sortir d'une enquête générique. Seul Arone peut le reconnaître après une vraie rencontre face-à-face, un échange significatif et une reconnaissance personnelle.   
- `ReaperDoctrineSystem` est corrigé vers le rôle réel des REAPERS-13 : chasseurs de primes **exécuteurs**. Le jugement se fait avant l'acceptation ; une mort volontaire injustifiée peut créer une dette de sang « une vie pour une vie ». Une reddition ne gomme pas automatiquement une condamnation déjà établie, tandis qu'une exécution de prime légitime peut valoriser le dossier d'un chasseur.   
- Le lore Reaper précise également le rejet du chantage/de l'oppression et des autorités utilisant rang, noblesse ou couronne pour prétendre à des droits absolus ; la discrétion reste une arme mais le chaos est acceptable lorsqu'il faut réellement défendre l'unité ou les leurs.   
- `Sanctus` / `Skuro` sont maintenant explicitement deux états de **la même personne** : ils ne peuvent jamais apparaître ensemble au même moment.   
- Nouveau `LORE_HESTIA_DINOTOFU.txt` et mise à jour de `LORE_REAPERS13.txt`, `PERSONNAGES_SPECIAUX_DINOTOFU.txt` et `SYSTEMES_PREVUS.txt`.   
- `SaveRoundTripTest` couvre logement, état Hestia et artefact complet ; `HestiaArcAndHomeSystemTest`, `ReaperFamilySecretSystemTest`, `SanctusSkuroIdentityRuleTest`, `ReaperDoctrineSystemTest` et `SpecialCharacterExpansionTest` couvrent les nouvelles règles.   
- Build C++23 pré-bump : **OK**. Suite complète pré-bump : **TOUS LES TESTS DINOTOFU PASSENT (`TEST_EXIT=0`)**.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés.   


- Validation finale post-bump : rebuild C++23 OK, `Dinotofu V3.51.68` confirmé, suite complète `POST_BUMP_TEST_EXIT=0`.   
- Nettoyage final : suppression des anciens bonus Hestia par nom (esquive gratuite et ancien dôme de résurrection), afin que sa puissance dépende réellement de sa réadaptation et de ses éveils.   

## V3.51.67 — Primes ciblées jouables, conscience de traque et interception crédible   

- Nouvelle distinction explicite entre **être recherché** et **savoir qu'on est activement traqué**. `BountyHuntAwarenessSystem` classe désormais une cible en `Unaware`, `Suspicious` ou `ConfirmedHunt` à partir de ce qu'elle peut réellement observer : bruit de l'enquête, exposition des informateurs, filature repérée, avertissement direct et poursuites déjà ratées.   
- Une cible **inconsciente de la chasse ne prépare aucune contre-mesure anti-chasseur**, même si elle est naturellement prudente ou possède beaucoup de ressources. Une cible simplement soupçonneuse reste limitée à des réactions prudentes ; les leurres et changements de refuge complets n'arrivent qu'après une raison crédible de croire à la chasse.   
- L'ancienne formulation technique **« mise au vert »** est remplacée partout dans le moteur par **« se cacher ou changer de refuge »** (`HideOrChangeRefuge` / `refugeConcealment`).   
- Nouveau `BountyQuestSystem` : premières **missions de prime ciblée persistantes et réellement jouables** avec étapes enquête → préparation → interception/poursuite → confrontation. Le joueur peut observer discrètement, poser des questions plus rapides mais bruyantes, repérer les sorties, préparer une interception puis tenter de verrouiller la cible lorsqu'il possède assez d'indices.   
- Plusieurs contrats de guilde de rang E à B peuvent maintenant générer une cible humaine mobile déterministe avec nom, tempérament, ressources, mobilité, soutien local, connaissance du terrain et état de vigilance propres. Les traits d'une cible sont générés une seule fois et ne sont jamais rerollés pendant l'enquête.   
- L'enquête possède un vrai compromis **information / discrétion** : poser beaucoup de questions avance vite mais augmente le risque que la cible découvre la chasse ; observer et repérer les sorties est plus discret mais généralement plus lent.   
- En cas d'interception ratée, la cible sait désormais qu'une chasse existe, gagne une tentative de fuite connue, une partie des indices précis expire et les préparatifs trop locaux se dégradent. Une interception réussie mène à une vraie confrontation : demande de reddition ou combat PvE contre la cible.   
- Le journal traite désormais les primes ciblées comme des contrats de terrain/combat visibles au lieu d'un type technique caché.   
- Tous les champs de mission de prime (cible, tempérament, indices, bruit, conscience de la chasse, ressources, tentatives de fuite et préparations des deux camps) sont sauvegardés comme propriétés optionnelles rétrocompatibles ; le schéma reste 26. `SaveRoundTripTest` vérifie désormais explicitement leur round-trip.   
- Lore : les formulations trop associées aux armes à feu ont été corrigées (`pourquoi tu combats`, `savoir tuer`) et l'arsenal de Reaper-13 est documenté : **faux = artefact principal**, faucille, dague, arbalète et gadgets ; certaines armes secondaires peuvent reprendre des mécanismes proches d'artefacts sans remplacer la faux comme pièce centrale.   
- Les Reapers restent des utilisateurs experts des systèmes génériques de chasse : aucune omniscience, aucun `no escape` magique et aucune contre-mesure déclenchée par une cible qui ignore totalement être suivie.   

- Build C++23 pré-bump : **OK**. Suite complète pré-bump : **TOUS LES TESTS DINOTOFU PASSENT (`TEST_EXIT=0`)**.   

- Bump `3.51.66 -> 3.51.67` avec `--no-commit` : **OK**. Rebuild C++23 post-bump : **OK**. Suite complète post-bump : **TOUS LES TESTS DINOTOFU PASSENT (`POST_BUMP_TEST_EXIT=0`)**.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

## V3.51.66 — Cibles autonomes, aspirations des recrues, relais criminels faillibles et folklore élargi   

- Nouveau `BountyTargetBehaviorSystem` : une cible de prime choisit maintenant ses contre-mesures depuis ses **propres** informations et moyens — temps de préparation, ressources, mobilité, connaissance du terrain, soutien local, blessures, tempérament et précédentes tentatives de fuite. Elle n'accède jamais au build ni au plan du chasseur.   
- Les cibles prudentes/méthodiques et bien préparées peuvent combiner changement de route, leurres, contre-surveillance et se cacher / changer de refuge ; une cible pauvre, surprise ou blessée dispose de beaucoup moins d'options. Le résultat alimente le `BountyPursuitSystem` générique et reste borné : aucun anti-chasseur magique n'est créé.   
- Nouveau `RecruitedAllyAspirationSystem` : première aspiration légère déduite uniquement de la motivation, du métier, du trait déjà écrits et du vécu canonique réel de la recrue. Profils actuels : protéger le groupe, progresser, faire ses preuves, être justement reconnue/récompensée, ou simplement survivre ensemble.   
- La satisfaction d'aspiration dépend modestement de la relation et de faits comme participation, soins, sauvetages, quêtes personnelles et refus d'ordres dangereux. Elle modifie légèrement le moral, sans personnalité cachée ni microgestion punitive. Le menu d'équipe affiche aspiration + satisfaction.   
- `CrimeNetworkSystem` gagne une évaluation explicite des relais connus. Un avertissement inter-ville peut maintenant échouer si le contact est arrêté, détenu, en fuite, déjà sous forte pression/cooperation ou saturé par une alerte récente. Un réseau n'est donc plus un téléphone parfait ; aucun contact inconnu n'est révélé.   
- `WorldRumorCatalog` s'étend au-delà des villes/biomes : premières rumeurs sur les guildes, le Marché gris, les chasseurs de primes et des boss déjà visibles comme Fitoria, Zelef et Atlas. Elles restent des récits entendus avec preuve initiale nulle ; aucune vérité n'est canonisée par le catalogue.   
- Correction de robustesse des aspirations : les formulations françaises `soigneuse`, `protéger/proteger` sont reconnues comme profils de protection sans dépendre d'un libellé exact.   
- `WorldRumorCatalogTest` ne suppose plus qu'aucune autre rumeur bas niveau n'existe dans le monde ; il vérifie directement le niveau minimum de la rumeur des Cieux des Légendes, ce qui rend le test compatible avec l'enrichissement futur du catalogue.   
- Nouveaux tests dédiés `BountyTargetBehaviorSystemTest` et `RecruitedAllyAspirationSystemTest`; `CrimeNetworkSystemTest`, `WorldRumorCatalogTest`, `RecruitedAllyConditionSystemTest`, `SaveRoundTripTest` et `CityVaultSecurityLogisticsTest` couvrent les interactions sensibles.   
- Build C++23 pré-bump : **OK**. Suite complète pré-bump : **TOUS LES TESTS DINOTOFU PASSENT (`TEST_EXIT=0`)**.   
- Bump `3.51.65 -> 3.51.66` avec `--no-commit` : **OK**. Rebuild C++23 post-bump : **OK**. Suite complète post-bump : **TOUS LES TESTS DINOTOFU PASSENT (`POST_BUMP_TEST_EXIT=0`)**.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

## V3.51.65 — Contre-pistage, réseau criminel inter-ville, récupération des recrues et folklore du monde   

- `BountyPursuitSystem` gagne une vraie phase de **préparation / contre-jeu** commune à tous les chasseurs : repérage des sorties, informateurs, embuscade et filature côté chasseur ; changement de route, leurres, contre-surveillance et se cacher / changer de refuge côté cible.   
- Les préparations sont bornées et cumulables sans devenir absolues. Un excellent chasseur peut améliorer sa traque par du travail réel ; une cible préparée peut rouvrir des issues et réduire les ouvertures supplémentaires imposées. La chance de fuite reste toujours bornée à au moins 5 %.   
- Les Reapers continuent d'utiliser ce système partagé comme experts ; aucun bonus magique `no escape` n'a été ajouté.   
- Nouveau `CrimeNetworkSystem` : les contacts criminels déjà **connus** peuvent maintenant recevoir des avertissements inter-ville lorsque deux PNJ appartiennent au même réseau plausible (`marché gris`, `courriers nocturnes`). Un contact inconnu reste totalement inconnu.   
- `PlayerCrimeContactState` mémorise désormais ville d'origine, réseau, nombre/date/source des avertissements reçus. Ces champs optionnels sont sauvegardés/rechargés sans changement du schéma 26.   
- Les confrontations de garde continuent d'alerter localement les contacts connus et peuvent désormais faire circuler une pression plus faible vers les relais connus d'autres villes ; un échec diffuse une alerte plus forte qu'une coopération réussie.   
- `RecruitedAllyConditionSystem` distingue maintenant un état simple de récupération (`aucune blessure persistante`, `contusions`, `blessure importante`, `séquelles sérieuses`) à partir des faits de combat déjà enregistrés, sans diagnostic psychologique ni malus permanent arbitraire.   
- L'infirmerie gagne **Suivi de récupération** : coût physique, une unité de temps du monde, réduction de fatigue/stress de blessure, mais aucun soin magique de PV. Les soins de PV et la récupération restent deux systèmes séparés.   
- Nouveau `WorldRumorCatalog` générique avec un premier ensemble de rumeurs sur Valebrume, Rocheveille, Port-Lanterne, Lys-de-Givre, la Mine sifflante, les statues, les fées du mana, les Lames Muettes et d'autres lieux. Ces récits sont explicitement des rumeurs : les entendre ne les confirme jamais.   
- Les explorations peuvent rarement enregistrer une rumeur locale/biome non-Reaper avec un cooldown dédié. Les Reapers ne sont donc plus le seul sujet possédant du folklore via `WorldRumorSystem`.   
- Nouveaux tests : `CrimeNetworkSystemTest`, `WorldRumorCatalogTest`; `BountyPursuitSystemTest`, `RecruitedAllyConditionSystemTest` et `SaveRoundTripTest` couvrent les extensions de cette passe.   
- Build C++23 pré-bump : **OK**. Tests ciblés sensibles (poursuite, réseau criminel, état des recrues, rumeurs, sauvegarde, coffre, Reapers) : **OK**.   
- Les tentatives pré-bump de suite globale dépassaient la fenêtre d'exécution sans produire de `[FAIL]`. Après bump et rebuild, la suite complète a été relancée jusqu'à son terme : **TOUS LES TESTS DINOTOFU PASSENT (`POST_BUMP_TEST_EXIT=0`)**.   
- Bump `3.51.64 -> 3.51.65` avec `--no-commit` : **OK**. Rebuild C++23 post-bump : **OK**.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15**, catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

## V3.51.64 — Traque de prime, anti-fuite crédible, refus d’ordres suicidaires et réseau criminel réactif   

- Nouvelle fondation générique `BountyPursuitSystem` pour les chasseurs de primes : compétence de pistage, contrôle des routes, discrétion, coordination, mobilité de la cible, connaissance du terrain, sorties préparées et tentatives de fuite déjà observées produisent un vrai plan de poursuite.   
- La traque n’accorde jamais un verrouillage magique : elle peut réduire une chance de fuite de façon bornée (maximum -30 points actuellement) et imposer jusqu’à deux ouvertures supplémentaires lorsque les issues ont réellement été préparées. Une cible mobile/préparée conserve donc une chance crédible de s’échapper.   
- `ReaperDoctrineSystem` formalise désormais l’identité validée par l’utilisateur : REAPERS-13 est avant tout un clan important de **chasseurs de primes de l’ombre**. Leur profil de poursuite est très élevé en pistage/contrôle/discrétion/coordination, avec préférence capture/neutralisation ; leur puissance anti-fuite vient du système générique, pas d’une immunité spéciale.   
- Le catalogue doctrine/légendes ajoute **« Chasseurs de l’ombre »** comme doctrine forte. Les anciennes rumeurs restent séparées des faits et peuvent toujours être fausses/exagérées.   
- Une trace naturelle Reaper en exploration montre maintenant surtout une chasse préparée : routes secondaires fermées, angles de retraite surveillés et interception organisée, sans prétendre que les Reapers savent surnaturellement où se trouve une cible.   
- Nouveau `RecruitedAllyOrderSafetySystem` : une recrue peut enfin refuser un ordre offensif forcé objectivement proche du suicide. PV, préparation, moral, nombre d’ennemis, boss et relation influencent le danger/refus.   
- Les ordres normaux ou simplement risqués restent obéis. Le refus ne vole pas le tour de la recrue : elle ignore seulement la consigne offensive catastrophique et choisit ensuite une action de survie crédible via l’IA alliée commune.   
- Le PvE visible branche cette sécurité sans supprimer les ordres du reste du groupe. Un refus est historisé lorsque le joueur peut réellement l’observer.   
- Réseau criminel : une confrontation de garde ne modifie plus uniquement le receleur/complice ciblé. Les autres contacts **déjà connus dans la même ville** gagnent un peu de pression/méfiance ; une confrontation ratée alerte davantage le réseau qu’une confrontation coopérative. Aucun PNJ inconnu n’est créé pour simuler cette réaction.   
- `CityVaultSecurityLogisticsTest` protège cette réaction de réseau. Nouveaux `BountyPursuitSystemTest` et `RecruitedAllyOrderSafetySystemTest`; `ReaperDoctrineSystemTest` couvre aussi la doctrine de traque. Le runner attend désormais explicitement `reaper_doctrine`, qui était auparavant lancé sans attente dédiée.   
- Build C++23 pré-bump : **OK**. Suite complète pré-bump : **TOUS LES TESTS DINOTOFU PASSENT (`TEST_EXIT=0`)**.   
- Bump `3.51.63 -> 3.51.64` avec `--no-commit` : **OK**. Rebuild C++23 post-bump : **OK**. Suite complète post-bump : **TOUS LES TESTS DINOTOFU PASSENT (`POST_BUMP_TEST_EXIT=0`)**.   
- La validation complète de cette passe revalide également les fondations introduites en V3.51.63 qui n’avaient pas pu terminer leur rebuild global dans l’environnement précédent.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15**, catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

## V3.51.63 — Doctrine Reaper, légendes séparées des faits et blessures persistantes légères   

- REAPERS-13 reste une **grosse faction/clan importante mais optionnelle** : aucun recentrage du scénario principal, aucun recrutement automatique, aucun grade gratuit.   
- Nouveau `ReaperDoctrineSystem` : catalogue explicite de légendes adapté du corpus Discord utilisateur. Chaque récit possède un identifiant, un titre, une fiabilité initiale et une crédibilité de départ ; entendre une légende l'enregistre via `WorldRumorSystem` sans la transformer en vérité.   
- Légendes initiales intégrées : *Il n'était personne*, *Le treizième signal*, *Ne prononce pas son nom*, *La prime qui n'a jamais été payée*, *Les Reapers ne sauvent personne*, *Le recrutement*, *Le Reaper qui ne prend pas toutes les primes*, *La règle des Reapers*, origines contradictoires et doctrine responsabilité/nécessité.   
- Les origines incompatibles (agent déchu / clone / survivant de projet / simple chasseur parti de rien) restent volontairement **contradictoires** ; le moteur ne choisit pas arbitrairement une version canonique.   
- Première vraie logique de sélection de chasse : la valeur d'une prime n'entre pas dans la décision. Preuves, danger, récidive, menace active et atteinte volontaire aux innocents déterminent si une chasse mérite d'être prise.   
- Une erreur isolée avec correction/renoncement peut être refusée ou déclassée. Une cible acceptée n'est pas automatiquement une cible à tuer : capture/neutralisation restent prioritaires si la force létale n'est pas nécessaire.   
- Les recrues personnelles gagnent une première couche légère de **stress de blessure** distincte de la fatigue : KO / très bas PV / boss peuvent laisser une trace persistante bornée. Elle récupère plus lentement que la fatigue et influe modestement moral/préparation.   
- `RecruitedAllyConditionSnapshot` expose `injuryStressPercent` et `shaken`; l'affichage peut signaler « encore secoué », « marqué par les blessures » ou « très marqué ». Ce n'est ni un diagnostic psychologique ni un malus permanent automatique.   
- Nouveau `ReaperDoctrineSystemTest`; `RecruitedAllyConditionSystemTest` couvre aussi la récupération plus lente du stress de blessure.   
- Validation technique limitée dans cet environnement : les gros rebuilds C++23 n'ont pas terminé dans les fenêtres d'exécution disponibles malgré l'absence d'erreur C++ affichée avant timeout. Les nouvelles unités ont été ajoutées au runner de tests, mais la suite complète n'est donc **pas prétendue passée** sur cette passe.   
- Schéma de sauvegarde **26**, checkpoint **V3.51.15**, catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

## V3.51.62 — Pistes vivantes, cycle criminel, rumeurs du monde et état léger des recrues   

- Les pistes de récupération de champ de bataille ne restent plus figées au même endroit. Après plusieurs recherches infructueuses, un lot exact protégé peut passer d’un champ de bataille à une **cache proche**, être **sécurisé par la garde**, être **ramassé par un civil**, ou finir chez un **récupérateur encore non identifié**.   
- Ce changement de garde/localisation est persistant (`custodyState`, libellé, dernier jour de mouvement, nombre de déplacements, localisation connue) et historisé. Le moteur ne crée jamais automatiquement un voleur ou rival nommé uniquement parce qu’un objet a été déplacé.   
- Si la garde a physiquement sécurisé le lot, la récupération devient réellement **garantie à 100 %** : les pénalités génériques de perturbation ne peuvent plus faire disparaître artificiellement un objet déjà placé sous protection municipale.   
- Les contacts criminels possèdent maintenant un **cycle de vie global** plus complet : rôle connu receleur/complice, nombre de détentions provisoires, libérations, retours après fuite et dates correspondantes. Une libération ou un retour devient un vrai événement persistant au lieu d’un timer qui s’efface silencieusement.   
- La fin d’une détention provisoire peut diminuer la pression et légèrement modifier la coopération ; le retour après fuite conserve la mémoire du contact et de ses réactions passées. Une arrestation durable reste réservée à de futures preuves/condamnations explicites.   
- Nouveau `WorldRumorSystem`, volontairement **générique** : toute faction, ville, créature ou personne peut porter des rumeurs/légendes avec source, crédibilité, niveau de preuve, confirmations et contradictions. La fiche historique expose une vue **« Rumeurs & légendes »** sans transformer une histoire populaire en fait omniscient.   
- Les sources sont différenciées (taverne, voyageur, guilde, garde, archive, témoin, observation directe, source inconnue) et modifient la crédibilité. Plusieurs transmissions peuvent renforcer, confirmer ou contredire un même récit.   
- REAPERS-13 utilise seulement cette couche générique : une opération réellement observée peut confirmer le noyau factuel « la faction existe et opère », sans valider automatiquement les légendes sur ses méthodes ou son chef.   
- Règle de design précisée : **REAPERS-13 est une des grosses factions/clans importants du monde**, autonome et souvent perçue de l’extérieur comme clandestine ou comme des justiciers masqués. Ses membres suivent leurs propres principes/codes. La faction peut avoir une forte aura, des rumeurs et des légendes — notamment autour de Reaper-13 — mais elle reste indépendante de l’histoire principale et ne doit pas monopoliser le jeu.   
- Les limitations V3.51.61 restent donc actives : traces naturelles ~2 % seulement dans des biomes dangereux cohérents, au moins 12 jours entre deux observations naturelles, aucun recrutement/grade/roster gratuit. Aucun nouveau grand arc Reaper n’a été développé dans cette passe.   
- Les recrues personnelles gagnent un **état de préparation persistant léger** : fatigue, moral et préparation sont reconstruits depuis leur vécu réel. Un combat ajoute une fatigue bornée selon tours réellement joués, PV restants, KO éventuel et boss ; le repos du monde la réduit naturellement d’environ 18 points par jour.   
- Cette fatigue modifie seulement légèrement l’initiative alliée, l’humeur et la préparation : pas de microgestion punitive, aucun tour supplémentaire et la priorité du chef opérationnel reste intacte. Le menu d’équipe affiche maintenant un résumé lisible de l’état de chaque recrue.   
- Les collègues d’une équipe employeuse conservent volontairement leur propre modèle mood/loyauté au lieu d’être fusionnés artificiellement avec cette nouvelle couche de recrues personnelles.   
- `BattlefieldRecoverySystemTest`, `CrimeContactLifecycleTest`, `WorldRumorSystemTest`, `RecruitedAllyConditionSystemTest`, `ReaperWorldSystemTest` et `SaveRoundTripTest` couvrent les nouveaux comportements/persistances ; `scripts/test_project.sh` les attend explicitement et ajoute des garde-fous structurels.   
- Build C++23 et suite complète **avant puis après bump : OK / tous les tests Dinotofu passent (`TEST_EXIT=0`, puis `POST_BUMP_TEST_EXIT=0`)**. Bump `3.51.61 -> 3.51.62` avec `--no-commit`. Schéma **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** restent inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

---   

## V3.51.61 — Récupération différée, mémoire criminelle persistante et REAPERS-13 volontairement secondaires   

- Les objets exacts **Quête / Personnel / Unique** que l’équipe identifie mais ne peut pas transporter après un KO ordinaire ne sont plus seulement mentionnés comme abandonnés. Ils peuvent désormais être placés dans une vraie `PlayerBattlefieldRecoveryCache` persistante liée à la ville, au contexte de combat et au lieu narratif du champ de bataille.   
- Une nouvelle entrée locale **« Pistes de récupération »** apparaît uniquement lorsqu’un cache non résolu existe dans la ville courante. Le joueur peut revenir physiquement sur la zone et chercher les biens laissés, au lieu de recevoir automatiquement le loot ou de le perdre silencieusement.   
- La récupération différée consomme actuellement 2 unités de temps. Chance de base : 100 % le jour même/J+1, 90 % jusqu’à J+3, 75 % jusqu’à J+7, puis 60 %. Les échecs indiquent une zone fouillée/déplacée/piste froide et créent une vraie perturbation ; les tentatives précédentes apportent aussi une petite connaissance du terrain. Valeurs à playtester.   
- Un objet protégé n’est **jamais supprimé silencieusement** uniquement parce que la piste vieillit. Le moteur peut signaler qu’il a probablement été déplacé, mais une future couche crédible de PNJ/rival/cache devra porter ce déplacement avant toute disparition réelle. Le loot ordinaire non protégé reste abandonné comme auparavant afin d’éviter de transformer chaque champ de bataille en inventaire permanent.   
- Les caches conservent les **objets exacts** : `persistentId`, durabilité, enchantements, liaison de propriété et métadonnées de récupération survivent au save/reload. Une récupération résolue ne peut pas dupliquer le même objet lors d’une seconde tentative.   
- Les receleurs/complices disposent maintenant d’une **mémoire globale par contact**, distincte d’un dossier de cambriolage : première rencontre, dernières interactions, observations, négociations, confrontations de garde, pression, coopération, fuite, signalement et état de détention survivent d’un dossier à l’autre.   
- Un contact ayant fui ne réapparaît donc pas artificiellement dans un nouveau dossier avant son retour prévu. De même, la pression/cooperation déjà vécue est reprise lorsqu’un nouveau dossier pointe vers le même PNJ.   
- Correction de logique judiciaire : une confrontation réussie de la garde peut provoquer une **détention provisoire limitée dans le temps**, mais ne marque plus automatiquement le receleur comme arrêté pour toujours. `arrested` reste réservé à une future arrestation durable réellement justifiée ; la détention actuelle possède `detainedUntilDay` et doit être réexaminée.   
- **Règle de design REAPERS-13 explicitement verrouillée : la faction reste secondaire et optionnelle.** Elle n’est ni le groupe principal, ni les personnages principaux, ni une dépendance de l’histoire centrale. Le joueur doit pouvoir faire une partie normale en les croisant très peu, voire pas du tout.   
- Les observations naturelles REAPERS-13 sont donc limitées aux biomes plausiblement dangereux, avec seulement **~2 %** de chance et un **cooldown minimum de 12 jours** entre deux traces naturelles. Les biomes ordinaires/sûrs ne peuvent pas produire de rencontre naturelle, même sur un jet RNG favorable.   
- Un Protocol 13 transmis volontairement reste une exception contextuelle : sa réponse physique possède sa propre probabilité et n’est pas bloquée par la rareté des rencontres naturelles. Même alors, aucun recrutement/grade/roster/équipe gratuite n’est accordé. Le développement de roster/missions Reaper n’est volontairement **pas** priorisé dans cette passe.   
- Nouveau `BattlefieldRecoverySystemTest`; `SaveRoundTripTest` couvre le cache exact + mémoire criminelle + détention ; `ReaperWorldSystemTest` protège biome sûr, cooldown 12 jours et absence de recrutement. Le script global attend le nouveau test et ajoute des garde-fous structurels.   
- Build C++23 et suite complète **avant puis après bump : OK / tous les tests Dinotofu passent (`TEST_EXIT=0`, puis `POST_BUMP_TEST_EXIT=0`)**. Bump `3.51.60 -> 3.51.61` avec `--no-commit`. Schéma **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** restent inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

---   


## V3.51.60 — Passe large : butin post-KO exact, initiative alliée, pression criminelle et première présence REAPERS-13   

- Les vagues ordinaires terminées pendant le KO du joueur ne choisissent plus d’abord des « lots de monstres » abstraits. Pour chaque ennemi vaincu, le moteur génère désormais dans une zone temporaire le **butin exact** qu’il aurait produit : matériaux, monnaie physique, arme exacte et armure exacte lorsque les jets correspondants réussissent.   
- Le budget de transport post-KO s’applique **après** cette génération détaillée. Chaque candidat exact possède son coût réel et passe par `selectPostKoRecoveryCandidates` : Quête/Personnel/Unique et priorité explicite restent prioritaires, puis le groupe favorise les objets réellement transportables. La capacité peut donc ramener une arme et une bourse tout en laissant une armure, au lieu de récupérer ou perdre artificiellement tout le contenu d’un cadavre.   
- Le rapport post-KO distingue maintenant précisément ce qui a été ramené et ce qui a dû être laissé sur place ; l’événement canonique `butin_post_ko_transport` conserve le nombre d’objets exacts candidats/récupérés et la capacité utilisée.   
- **Limite volontaire encore ouverte :** un objet important non transportable est maintenant identifié exactement et n’est plus confondu avec du loot banal, mais les champs de bataille ordinaires ne possèdent pas encore de conteneur persistant permettant de revenir physiquement chercher cet objet plus tard. Cette persistance devra être ajoutée avant de promettre une récupération différée générique.   
- Nouvelle `RecruitedAllyInitiativeSystem` : les recrues actives possèdent maintenant une initiative minimale au sein de leur fenêtre alliée. Niveau/rang/qualité d’arme comptent modestement ; profils éclaireur/rapide/agile sont favorisés, tanks/lourds/lents pénalisés.   
- L’initiative alliée est utilisée en PvE standard et contre les boss sans déplacer la riposte ennemie, sans créer de tour supplémentaire et sans contourner le délai d’entrée depuis la réserve. La priorité d’un chef opérationnel reste explicitement supérieure au simple score de vitesse.   
- Le réseau de cambriolage gagne une vraie **confrontation par la garde** pour un receleur/complice humain suffisamment documenté (au moins 4 indices). La garde agit sur des faits observés et ne transforme jamais automatiquement le détenteur en auteur du cambriolage.   
- Chaque dossier peut désormais conserver tentatives de confrontation, pression, coopération, signalement à la garde et fermeture temporaire du contact. Une confrontation crédible peut produire de nouveaux indices, pousser le contact à coopérer et parfois restituer les biens ; un échec peut au contraire l’alerter et fermer temporairement sa piste locale. Tous ces états survivent au save/reload.   
- Première vraie couche **REAPERS-13 dans le monde**, indépendante de tout cheat : une exploration peut très rarement laisser observer une opération inconnue, disciplinée et ciblée. Cette scène donne seulement une connaissance crédible/observation ; elle ne révèle ni roster secret, ni grades, ni objectifs internes et ne propose aucun recrutement.   
- Un personnage ayant transmis légitimement `DINOTOFU:13-02313` peut plus rarement recevoir plus tard une **réponse physique d’un relais Echo Unit** en exploration. Le relais demande notamment comment le personnage connaît le canal, puis repart ; aucun grade, membre d’équipe, victoire ou accès de faction n’est accordé. La réponse est historisée et le signal n’est plus considéré comme sans réponse.   
- `RecruitedAllyInitiativeSystemTest`, `PostKoExactWaveLootTest` et `ReaperWorldSystemTest` sont ajoutés. `CityVaultSecurityLogisticsTest` couvre la confrontation garde ; `SaveRoundTripTest` couvre les nouveaux états criminels persistants. Des garde-fous structurels protègent aussi le nouveau chemin exact du loot, l’initiative, la présence Reaper naturelle et l’absence de recrutement automatique.   
- Build C++23 et suite complète **avant puis après bump : OK / tous les tests Dinotofu passent (`TEST_EXIT=0`, puis `POST_BUMP_TEST_EXIT=0`)**. Schéma **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** restent inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

---   


## V3.51.59 — Passe large : objets prioritaires, récupération post-KO, réseau criminel interactif et rotation boss   

- Les objets héritant de `Item` possèdent désormais des métadonnées de récupération génériques **Unique / Quête / Personnel** et une priorité explicite **0–100**, volontairement séparées de la liaison à l’âme. La liaison décide si un objet peut changer de propriétaire ; ces nouvelles métadonnées décident à quel point il doit être protégé d’un abandon automatique.   
- La priorité automatique donne un plancher de 90 aux objets uniques, 95 aux biens personnels et 100 aux objets de quête. Un objet peut cumuler les traits ; la valeur la plus protectrice gagne sans le rendre artificiellement non transférable.   
- Ces métadonnées sont persistées pour inventaire, coffres municipaux, équipement exact volé par Grinka et équipement exact conservé dans les dossiers de cambriolage. Les anciennes saves sans champs restent compatibles avec un objet ordinaire/priorité 0 ; schéma 26 inchangé.   
- Les piles de matériaux de même ID/qualité ne fusionnent plus si leurs métadonnées de récupération diffèrent. Une pile personnelle/de quête ne peut donc pas perdre sa protection en étant empilée avec une pile banale.   
- Le cambriolage de coffre utilise maintenant ces métadonnées : **Quête** et **Personnel** sont exclus du vol générique ; **Unique** ou haute priorité reste théoriquement transférable mais devient extrêmement improbable au lieu d’être confondu avec « lié à l’âme ». Les anciennes heuristiques textuelles restent seulement comme fallback de compatibilité pour d’anciens contenus.   
- Nouveau sélecteur générique `selectPostKoRecoveryCandidates` pour les objets exacts lors d’une extraction après KO : capacité physique réelle conservée, mais les objets protégés passent avant le loot ordinaire, puis priorité décroissante et coût de transport. Un objet non transportable est différé/abandonné selon son registre au lieu de disparaître silencieusement.   
- Les armes/armures exactement confisquées par Grinka sont désormais marquées **Personnel / priorité 100** dans son registre et sa récupération post-KO utilise ce sélecteur générique. Si la capacité manque, l’objet reste chez Grinka et demeure récupérable plus tard.   
- Les fragments principaux de boss sont désormais de vrais objets **Unique / priorité 100** côté moteur, pas seulement « uniques » dans le texte. Cette métadonnée suit ensuite le fragment dans l’inventaire et les sauvegardes.   
- Le réseau de cambriolage devient réellement **interactif** : lorsqu’un dossier mène à un receleur/complice/cache ancré au monde, le coffre propose « Suivre la piste locale ». Le joueur peut observer discrètement, négocier une restitution ou fouiller une cache selon le type de détenteur et les preuves disponibles.   
- Observer consomme du temps et ajoute 1–2 indices sans révélation magique. Une approche trop visible peut alerter le contact. Un même dossier ne peut pas être forcé plusieurs fois la même journée.   
- À partir de 3 indices, un receleur/complice peut accepter une **restitution négociée** contre une part bornée de la valeur estimée. L’argent n’est prélevé que si l’accord réussit. Une restitution ne vaut jamais confession : le contact peut seulement détenir le bien sans avoir commis le cambriolage.   
- Une cache suffisamment documentée peut être fouillée. Une réussite restitue les biens encore traçables sans inventer de coupable humain ; un échec peut confirmer une nouvelle piste. Les compteurs d’observation/négociation, alerte du contact et dernier jour d’interaction sont persistants.   
- La **rotation manuelle du joueur-chef contre les boss** est enfin active pour l’équipe personnelle. Avec une réserve disponible, le joueur peut sacrifier son tour pour échanger 1 ou 2 recrues ; le double échange est atomique et un refus éventuel d’un membre critique ne consomme pas le tour mais verrouille une nouvelle tentative jusqu’au tour suivant.   
- Une recrue entrée depuis la réserve pendant cette rotation reçoit le même délai anti-tour-gratuit que les autres systèmes. Les autres recrues peuvent encore agir normalement, puis le boss garde sa prochaine phase : aucune rotation ne déplace artificiellement l’initiative ennemie.   
- `SaveRoundTripTest` couvre les nouvelles métadonnées et l’état des interactions criminelles ; `RecruitedAllyAftermathSystemTest` couvre le choix sous capacité limitée ; `CityVaultSecurityLogisticsTest` couvre restitution négociée et fouille de cache sans faux rival ; `GrinkaEquipmentRecoveryTest` reste vert avec la sélection générique. Des garde-fous structurels protègent aussi la rotation boss et la persistance.   
- Limite volontaire restante : les vagues ordinaires post-KO sélectionnent encore des **lots ennemis abstraits** avant génération détaillée de tous les objets. Le sélecteur exact est désormais prêt et utilisé là où des objets persistants existent ; migrer toutes les vagues vers des candidats exacts reste un chantier séparé afin de ne pas casser le loot hors écran.   
- Build C++23 et suite complète **avant puis après bump : OK / tous les tests Dinotofu passent**. Schéma **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent en pause.   

---   

## V3.51.58 — Passe large : réseau de recel, morphologie en partie et titres logistiques   

- Les biens issus d’un cambriolage qui passent à un **receleur, complice ou cache** ne restent plus de simples libellés abstraits. Un nouveau `VaultTheftNetworkCatalog` les rattache à des contacts et lieux cohérents de chaque ville : marché/porte de Valebrume, forges/rails de Rocheveille, quais/marché sous les ponts de Port-Lanterne et porte/marché/temple de Lys-de-Givre.   
- Les dossiers de cambriolage conservent désormais l’ID du contact monde, l’ID/nom du lieu et le biome d’exploration associé. Ces champs optionnels survivent au save/reload sans changer le schéma 26.   
- La reconnaissance directe d’un équipement volé respecte maintenant ce lieu réel : tenter une reconnaissance dans le mauvais biome est dégradé en simple piste de déplacement. Le moteur ne peut donc plus faire apparaître le bien exact au mauvais endroit uniquement parce qu’il connaît sa position interne.   
- Une cache localisée garde un vrai lieu mais ne crée toujours jamais de rival humain artificiel. Un receleur/complice identifié peut au contraire utiliser son nom monde et sa localisation comme ancrage de la piste.   
- Première vraie **évolution morphologique en partie** : Fine / Standard / Robuste ne sont plus figées à la création. Le joueur peut lancer volontairement à l’auberge un programme vers le palier adjacent uniquement (Fine ↔ Standard ↔ Robuste). Aucun saut Fine ↔ Robuste direct.   
- Un programme demande actuellement **8 séances distinctes** et au minimum **14 jours de monde**. Une seule séance peut compter par jour ; chaque séance représente entraînement + alimentation suivie, coûte 35 PC et consomme 2 unités de temps. Valeurs prévues pour playtest, pas pour transformation instantanée.   
- Tant que le programme n’est pas terminé, la morphologie et ses effets restent inchangés. Annuler ne transforme rien. La validation finale applique seulement alors la nouvelle morphologie, puis `CarryLoadSystem` utilise immédiatement ce nouvel état pour capacité/hitbox/mobilité.   
- État du programme (morphologie actuelle, cible, séances, jour de départ et dernière séance) sauvegardé/rechargé via champs optionnels compatibles. Les anciennes saves continuent de dériver Fine/Standard/Robuste de leur variante de création si aucun nouvel état n’existe.   
- Les titres morphologiques dormants commencent enfin à vivre sur de vrais changements : `Un corps façonné`, `Plus solide qu'avant`, `Taillé pour porter`, `Plus léger qu'avant`, `Affûté` et, après plusieurs transformations réelles, `Silhouette travaillée`. Le catalogue lui-même ne change pas et reste V3.51.51.   
- La fiche Statistiques affiche maintenant la morphologie courante et, lorsqu’il existe, le programme en cours avec sa cible et ses séances.   
- Quatre titres de rangement/logistique jusque-là dormants sont branchés sur des **événements de session** plutôt que sur un simple instantané : `Grand ménage` pour une grosse session réelle de déchargement au coffre, `Ça respire enfin` pour passer d’une charge >150 % à <50 %, `Je garde tout` pour rentrer d’exploration à 180–199 % avec ramassage complet sans avoir franchi le blocage 200 %, et `Déménageur` après une sortie ayant réellement transporté au moins quatre équipements lourds de réserve sans atteindre 200 %.   
- La logique de ces titres a été centralisée côté `Player` afin d’être testable indépendamment de l’interface. `StorageTitleSessionTest` protège les seuils et empêche notamment d’accorder les titres d’exploration après un pic à 200 %+.   
- `CityVaultSecurityLogisticsTest` protège désormais l’ancrage receleur/cache, le refus de reconnaissance exacte dans le mauvais biome et l’identification au bon lieu. `MorphologyProgressionTest` protège paliers adjacents, 8 séances/14 jours, absence de transformation anticipée, annulation et titres. `SaveRoundTripTest` couvre programme morphologique et localisation criminelle persistants.   
- `scripts/test_project.sh` attend explicitement les nouveaux tests et ajoute des garde-fous structurels pour le catalogue criminel, sa persistance, les titres de session et l’affichage de morphologie.   
- Build C++23 et suite complète **avant puis après bump** : **OK / tous les tests Dinotofu passent**. Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement et histoire restent volontairement en pause.   

---   

## V3.51.57 — Pistes de cambriolage rencontrées pendant l’exploration   

- Un dossier de cambriolage encore non résolu peut désormais produire une **rencontre fortuite crédible pendant une exploration** rattachée à la même ville. Le moteur ne fait donc plus progresser cette enquête uniquement via l’option du coffre.   
- La scène reste rare et contextuelle : environ **28 %** de chance sur les zones de passage humain (route, plaine, marché, quartier, ruines) et **14 %** ailleurs, uniquement lorsqu’un dossier local à cible encore non identifiée existe.   
- Trois formes d’observation existent : **piste de déplacement**, **échange suspect** et, plus rarement, **reconnaissance directe d’un équipement volé distinctif**. Une trace ou un échange donnent 1 indice ; reconnaître réellement une arme/armure exacte donne 2 indices.   
- Reconnaître un bien ne transforme jamais automatiquement son porteur en auteur du cambriolage : le texte précise qu’il peut s’agir d’un détenteur, complice ou receleur. L’identité ne devient traçable qu’une fois les preuves cumulées suffisantes, en réutilisant le même seuil d’enquête crédible.   
- Les biens génériques (monnaie, matériaux, consommables) ne sont jamais prétendus « reconnus exactement » sans raison. Les scènes parlent seulement de conditionnement, lot ou comportement compatible avec le dossier.   
- Une **cache secondaire** peut désormais être localisée sans créer artificiellement un rival humain : atteindre le seuil d’indices alors que les biens sont dans une cache ne fabrique plus de personne à chasser. Il faut attendre une nouvelle piste ou un nouveau détenteur réel.   
- Un cooldown persistant par dossier empêche de farmer plusieurs fois la même piste pendant la même journée d’exploration. La scène ne consomme pas une unité de temps supplémentaire : elle fait partie de la sortie déjà en cours.   
- `CityVaultSecurityLogisticsTest` couvre trace faible sans rival, reconnaissance d’une arme exacte menant à l’identification après assez d’indices, refus d’une piste depuis la mauvaise ville et absence de cible inventée pour une cache. `scripts/test_project.sh` protège aussi le raccord exploration/cooldown/anti-omniscience.   
- Build C++23 et suite complète : **OK avant puis après bump**. Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue titres **V3.51.51** inchangés. Finance prêts/dettes/découvert/recouvrement toujours en pause ; histoire toujours en pause.   

---   

## V3.51.56 — Liaison de propriété générique et persistante   

- Tous les objets héritant de `Item` possèdent désormais une vraie métadonnée générique de propriété : **transférable** ou **lié à l’âme**. La logique ne dépend plus du nom, de la description ou d’un enchantement textuel pour savoir si un objet peut changer de propriétaire.   
- `canChangeOwner()` centralise la règle : un objet lié peut toujours être déplacé dans les stockages appartenant au même personnage, mais ne peut pas être donné, volé, confisqué ou transféré à un autre propriétaire.   
- Les échanges entre personnages refusent désormais armes, armures, consommables et matériaux liés, avec protection côté moteur même si une interface tente de contourner le filtre.   
- Les cambriolages de coffre, vols pendant une fuite, confiscations de Grinka et prises d’équipement en JcJ respectent la même règle générique. Un coffre ne crée donc plus de faux dossier de cambriolage si aucun bien réellement transférable n’a été emporté.   
- Les transferts entre coffres municipaux du même personnage restent autorisés pour les objets liés : il s’agit d’un déplacement de stockage, pas d’un changement de propriétaire.   
- Les piles de matériaux liés et non liés de même ID/qualité restent séparées afin qu’un empilement ne puisse jamais effacer ou diluer la protection.   
- La liaison est persistée dans les sauvegardes pour inventaire, coffres, équipements volés et dossiers de cambriolage. Les anciennes sauvegardes sans champ restent compatibles et chargent les objets comme transférables par défaut ; schéma **26** inchangé.   
- `SaveRoundTripTest`, `ExchangeTransferSystemTest` et `CityVaultSecurityLogisticsTest` couvrent persistance, séparation des piles, refus de transfert, protection contre le pillage et transport entre coffres du même propriétaire.   
- `scripts/test_project.sh` protège maintenant structurellement cette métadonnée. Il ignore proprement les contrôles du workflow GitHub lorsque `.github` est absent d’un backup essentiel, et attend désormais explicitement `TitleCatalogUpdateTest` avant le nettoyage des fichiers temporaires.   
- Build C++23 et suite complète **avant puis après bump** : **OK / tous les tests Dinotofu passent**. Aucun nouveau développement prêts/dettes/découvert n’est repris. Catalogue de titres **V3.51.51**, checkpoint **V3.51.15** et schéma de sauvegarde **26** inchangés.   

---   

## V3.51.55 — Défense de coffre alignée sur les règles de partie   

- La défense d’un coffre municipal n’utilise plus le fallback caché **Normal / mort non définitive**. Elle reçoit désormais la vraie `DifficultyMode` et la vraie `DeathRuleMode` de la partie active.   
- Le contexte est propagé depuis `Game` jusqu’aux menus de lieux, au relais des routes, au hub de ville puis au coffre. Entrer dans le coffre par un chemin secondaire ne peut donc plus changer silencieusement les règles du combat.   
- `MonsterPveMode::runExplorationWave` reçoit maintenant directement la difficulté et la règle de mort actives pour la défense. Les multiplicateurs de difficulté, pénalités, règles de mort et comportements Léthal/Mortel déjà centralisés restent donc cohérents avec les autres combats.   
- `QuestMenu::openLocations`, `QuestWorldMenuSupport::openCityTravelMenu` et `openCityVault` exposent explicitement ce contexte au lieu de dépendre d’un état global ou d’une valeur par défaut implicite.   
- `scripts/test_project.sh` protège la chaîne de propagation et échoue si le vieux couple `DifficultyMode::Normal / DeathRuleMode::NonDefinitive` réapparaît dans ce chemin.   
- Build C++23 et suite complète moteur/tests : OK. Le backup essentiel fourni en entrée ne contenait pas `.github`, donc seules les quatre assertions structurelles visant le workflow GitHub ont été ignorées pendant cette reprise depuis archive ; tous les tests de code Dinotofu passent.   
- Aucun nouveau développement prêts/dettes/découvert n’est repris. Schéma de sauvegarde **26**, checkpoint important **V3.51.15** et catalogue de titres **V3.51.51** inchangés.   

---   

## V3.51.54 — Cambriolages à auteur inconnu, enquêtes et circulation du butin   

- Un coffre pillé pendant l'absence du joueur ne crée plus magiquement un rival identifié. Si le délai du messager est ignoré et que des biens sont réellement emportés, le moteur crée désormais un **dossier de cambriolage persistant à auteur inconnu**.   
- Le dossier conserve les biens volés réels : monnaie, pile de matériau, consommable et jusqu'à un équipement précis avec son `persistentId`, sa durabilité et ses enchantements. Le vol automatisé de coffre ne transforme donc plus un équipement en simple texte irrécupérable.   
- Le choix des biens volés est maintenant plus nuancé : les stocks abondants/communs restent très favorisés, mais armes et armures transférables peuvent aussi être prises ; plus leur valeur/enchantement est élevé, plus leur poids de sélection baisse fortement. Les objets explicitement détectables comme liés à l'âme/non transférables sont exclus.   
- Nouvelle option **Enquêter sur un cambriolage** dans le coffre de la ville concernée. Une enquête consomme du temps local et accumule des indices issus de témoins, traces, garde et receleurs.   
- L'auteur n'est révélé qu'après assez d'indices crédibles. À partir de 3 indices, une cible devient identifiable et est alors seulement créée comme rival/traque persistante. Avant cela, aucun nom ni identité ne sont révélés au joueur.   
- Les biens volés peuvent **changer de main ou de cache avec le temps** si le joueur tarde : complice, receleur ou cache secondaire. Ce déplacement peut faire perdre un peu de certitude et oblige l'enquête à retrouver la piste actuelle, sans supprimer le bien du monde.   
- Vaincre ensuite le rival actuellement relié au dossier récupère les biens encore traçables ; `Rends-moi ça.` reste attribuable à cette récupération.   
- Le système distingue maintenant clairement deux contextes : **vol opportuniste pendant une fuite** (prise rapide de ce qui passe sous la main, objets précieux moins accessibles) et **cambriolage de coffre** (davantage de temps pour fouiller et possibilité réelle de tomber sur un bien plus important).   
- `SaveRoundTripTest` couvre la persistance d'un dossier de vol inconnu et d'une arme exacte volée ; `CityVaultSecurityLogisticsTest` couvre auteur inconnu → enquête → identification → récupération.   
- RESTE : créer une vraie métadonnée générique d'objet `lié à l'âme/non transférable` au lieu de dépendre seulement d'indices textuels ; permettre une rencontre aléatoire crédible avec un voleur encore non identifié pendant exploration ; relier plus tard les receleurs à de vrais PNJ/lieux quand cette couche de monde sera assez riche.   
- La défense de coffre utilise encore le fallback difficulté Normal / mort non définitive dans ce menu isolé ; ce point reste prioritaire pour une prochaine passe.   
- Aucun nouveau développement prêts/dettes/découvert n'est repris. Schéma de sauvegarde **26**, checkpoint **V3.51.15** et catalogue de titres **V3.51.51** inchangés.   

---   

## V3.51.53 — Coffres exposés, défense légale et messagerie mutualisée   

- Le risque quotidien d'une tentative de pillage vise désormais une fréquence moyenne réellement significative sans calendrier fixe : **1,5 % par jour éloigné** pour un coffre non vide, non couvert et hors présence locale, soit environ **une alerte tous les 67 jours éloignés en moyenne**. Le messager et le délai de 7 jours restent obligatoires avant toute conséquence.   
- Le prix des coffres supplémentaires suit maintenant la progression cumulative décidée : premier coffre **2 PP**, puis ajout successif de **+2 PO, +3 PO, +4 PO, +5 PO...**. Les premiers tarifs sont donc **2 PP ; 2 PP 2 PO ; 2 PP 5 PO ; 2 PP 9 PO ; 3 PP 4 PO...**.   
- La ristourne du maire sur le tout premier coffre devient beaucoup plus exigeante : le personnage doit déjà disposer d'une richesse monétaire liquide totale d'au moins **1 PP** entre bourse physique et solde réellement déposé à la guilde, puis remplir les conditions locales. Environ **15 missions/services ordinaires** ou **4 missions confiées par un personnage important de la ville** sont nécessaires, en plus de la réputation ; les paliers de remise restent 1,5 PP puis 1,25 PP pour un dossier exceptionnel.   
- Les quêtes réellement confiées par un PNJ notable sont maintenant enregistrées localement dans `quetes_personnage_important`, afin que la remise repose sur des services crédibles et non sur une simple quantité de petites tâches anonymes.   
- Une défense de coffre réussie est désormais traitée comme une intervention sous autorité municipale : le joueur ne reçoit qu'environ **40 % de la récompense légale et du loot ordinaire** des assaillants, le reste étant remis aux autorités. Les biens appartenant déjà au joueur et récupérés sur les voleurs restent hors de cette limite.   
- Le **Garde municipal** temporaire ne prend toujours aucun équipement ni objet. Comme il participe réellement au combat, sa présence réduit seulement l'XP finale d'environ **10 % supplémentaires**, sans réduire davantage la part matérielle légale.   
- La génération de loot autorise maintenant un pourcentage de récupération légale : pour une défense à 40 %, seul un sous-ensemble cohérent des assaillants fournit du butin standard ; l'interface précise que le reste est saisi/scellé par la ville.   
- En cas de pillage réussi ou de délai ignoré, les voleurs ciblent en priorité les **grosses piles de matériaux communs et peu précieux**, ainsi qu'une petite part de monnaie physique. Les objets de quête, clés, informations et biens remarquables restent protégés ; les matériaux rares/artefactuels ne deviennent candidats qu'en ultime recours avec une probabilité volontairement minuscule.   
- Les voleurs qui réussissent un larcin pendant leur propre fuite utilisent désormais la même logique de préférence : une grosse pile commune est prioritaire sur une ressource rare. Les objets protégés restent exclus.   
- Les armes/armures ne sont toujours pas supprimées automatiquement lors d'un pillage de coffre : avant d'autoriser ce cas, le moteur devra pouvoir rattacher l'objet exact à un voleur/rival récupérable au lieu de transformer une mauvaise RNG en suppression définitive.   
- Les **rappels inter-ville** peuvent désormais mutualiser le coût du messager initial. Plusieurs demandes sur le même trajet lancées dans une fenêtre d'environ **2 unités de temps sur 5 par jour** réutilisent le messager déjà parti et ne paient qu'un petit surcoût administratif ; au-delà, un nouveau messager complet doit partir.   
- L'état du messager de rappel (moment, ville source, destination) est persisté en sauvegarde. Le calcul du coût indique quand le messager est mutualisé ; save/reload ne permet ni de perdre ni de recréer artificiellement la réduction.   
- `CityVaultSecurityLogisticsTest`, `EconomyScaleTest`, `RivalEscapeTheftRecoveryTest` et `SaveRoundTripTest` couvrent les nouveaux tarifs, seuils de ristourne, priorité de vol, défense légale et mutualisation du messager.   
- Aucun développement supplémentaire des prêts/dettes/découverts n'est repris. Le schéma de sauvegarde reste **26**, le checkpoint important **V3.51.15**, et le catalogue de titres reste **V3.51.51** car aucun titre n'est ajouté ou modifié dans cette passe.   

---   

## V3.51.52 — Coffres patrimoniaux, sécurité locale et logistique inter-ville   

- Le coffre municipal devient un **investissement patrimonial** : tarif normal du premier coffre fixé à **2 PP** (2000 PF). Chaque nouveau coffre acheté dans une autre ville ajoute au minimum **+2 PO** au prix d'achat : 2 PP, 2 PP 2 PO, 2 PP 4 PO, etc. Le prix d'achat n'est plus réduit par un coefficient économique de ville.   
- Le maire peut accorder une ristourne uniquement sur le **premier coffre du personnage** à partir de faits locaux réels : bonne réputation + plusieurs missions/services réussis donnent **1,5 PP** ; réputation exceptionnelle + historique local plus conséquent donnent **1,25 PP**. Dès qu'un premier coffre existe, les achats suivants reviennent au tarif réseau normal.   
- Les améliorations de capacité sont recalibrées pour rester cohérentes avec cet achat important : N1→N2 **1 PP**, N2→N3 **1,5 PP**, N3→N4 **2,25 PP**, N4→N5 **3,5 PP** avant éventuel coefficient local. `Place optimisée` coûte actuellement **1 PP** et `Rangement anticipé` **2,5 PP** ; valeurs à playtester.   
- Nouveau service par coffre **Sécurité et entretien**. Il achète 7 jours de couverture réellement consommés uniquement lorsque le personnage est loin de la ville du coffre ; le compteur est donc **en pause dans la ville et pendant les explorations locales rattachées à cette ville**. Cumul actuel limité à 35 jours.   
- Coût actuel de sécurité selon le niveau du coffre : **30 / 40 / 50 / 60 / 70 PF pour 7 jours éloignés**. L'objectif est d'en faire un petit coût de délégation/gardiennage, pas une seconde taxe d'achat.   
- Un coffre éloigné, non protégé et non vide possède désormais une chance volontairement rare (**0,5 % par jour éloigné actuellement**) d'attirer une tentative de pillage. Aucun objet n'est perdu immédiatement : un **messager urgent** prévient le joueur et ouvre un délai de **7 jours** pour revenir.   
- Une alerte de pillage est persistante et survit désormais réellement à save/reload : jours de sécurité restants, alerte active, échéance et dernier contrôle sont sérialisés. Une correction de pré-bump a ajouté les champs d'écriture JSON qui manquaient alors que le chargement savait déjà les lire.   
- Une fois revenu dans la ville concernée, l'option **Défendre le coffre** devient disponible. Le combat oppose majoritairement des pillards/voleurs humanoïdes adaptés au niveau du joueur ; une faible chance actuelle d'incident monstrueux existe, la garde de ville étant supposée absorber la majorité des menaces monstrueuses ordinaires.   
- Si l'équipe n'est pas pleine, une défense peut recevoir l'aide temporaire d'un **Garde municipal**. Il participe physiquement au combat mais n'est jamais recruté, n'est pas persisté comme compagnon et **ne prend aucune part de récompense** ; un test protège explicitement cette absence de dilution.   
- Réussir la défense ferme l'alerte. Échouer ne vide jamais brutalement le coffre : la première conséquence actuelle est une petite perte de monnaie physique accessible (environ 5 %). Ignorer entièrement le délai peut coûter environ 10 % de la monnaie stockée. Objets uniques, de quête et équipements ne sont jamais supprimés silencieusement par cette couche tant que leurs métadonnées génériques ne sont pas assez fiables.   
- Acheter la sécurité pendant qu'une alerte est déjà active **n'annule pas rétroactivement l'attaque** : l'alerte en cours doit être défendue ; la couverture protège les jours futurs.   
- La logistique inter-ville est maintenant bidirectionnelle et générale : **Envoyer** depuis le coffre courant ou **Rappeler** depuis un coffre distant pour armes, armures, consommables, piles de matériaux et monnaie physique. Le rappel coûte un peu plus et prend plus de temps car un messager doit d'abord porter la demande avant le retour du convoi.   
- Le coût du convoi dépend de la distance et de **l'encombrement réel du chargement** : transporter une armure lourde coûte davantage qu'un petit lot compact. La monnaie conserve ses dénominations physiques et les armes/armures gardent exactement leur identité persistante.   
- Avant confirmation, l'interface affiche l'occupation de la **source avant → après** et de la **destination avant → après**, avec coût et durée. Les transferts utilisent la vraie capacité du coffre destinataire et ses optimisations de rangement, et retirent réellement l'objet de la source avant de l'ajouter à destination.   
- `CityVaultSecurityLogisticsTest` couvre ristournes du maire, progression +2 PO, abonnement local/en déplacement, alerte protégée/non protégée, transfert dans les deux sens, conservation de l'identité d'une arme et des dénominations monétaires. `SaveRoundTripTest` couvre maintenant la persistance de la sécurité et de l'alerte ; `RecruitedAllyCombatRosterTest` protège la part nulle du garde municipal temporaire.   
- Aucun nouveau développement prêts/dettes/découvert/recouvrement n'est repris. L'équilibre économique global continue d'être audité. Schéma de sauvegarde **26** et checkpoint **V3.51.15** inchangés.   

---   

## V3.51.51 — Encombrement complet, sacs, coffres optimisables et morphologies   

- L'encombrement devient une mécanique lisible mais volontairement non simulationniste : le menu affiche désormais `INVENTAIRE [utilisé/capacité]`, puis `utilisé/capacité`, pourcentage et état en tête d'inventaire. Un avertissement apparaît au franchissement des 85 % sans être répété en boucle.   
- Capacité de sac de base fixée à **40 unités**, avec sacs achetables à **50 / 60 / 70 / 80**. La capacité finale reçoit ensuite de petits modificateurs de race, morphologie, Force et classe ; un mage pur porte environ 10 % de moins, tandis que le Bibliomancien, archétype magique fondé sur les objets, est exempté.   
- Valeurs de référence d'encombrement : arme légère/moyenne/lourde `3/5/8`, armure `8/12/18`, munitions par paquets de 20, consommables identiques par paquets de 5 et matériaux par piles selon densité. La monnaie physique compte le **nombre réel de pièces** : 100 pièces = 1 unité, quelle que soit leur valeur de change.   
- Les paliers de surcharge deviennent réellement perceptibles : 101–125 % = `-10 %` fuite, `-5 %` précision mêlée, `+2 %` protection physique et `+10 %` temps d'exploration/voyage ; 126–199 % = `-20 %`, `-10 %`, `+4 %` et `+25 %`. À **200 % ou plus**, ces malus ne s'aggravent plus mais tout nouveau voyage/changement volontaire de zone/exploration est bloqué jusqu'à allègement. L'inventaire reste accessible afin d'éviter tout softlock.   
- Le gabarit naturel de la race et la **morphologie choisie** sont maintenant deux couches distinctes. La création propose trois morphologies : **Fine / Standard / Robuste**. Fine réduit légèrement capacité/hitbox et favorise la mobilité ; Robuste fait l'inverse. Les anciennes variantes A/B restent interprétées de manière compatible.   
- Les personnages naturellement petits sont légèrement plus difficiles à toucher et portent moins ; les grands/massifs portent davantage mais offrent une hitbox légèrement plus facile. Ces effets restent modestes et s'ajoutent à la morphologie sans remplacer l'identité raciale.   
- La surcharge ralentit aussi le temps du monde : les durées affichées et réellement consommées des voyages et explorations suivent le modificateur d'encombrement. Chaque fin d'exploration affiche désormais l'évolution du sac **avant → après** et le pic d'encombrement observé pendant la sortie.   
- Les coffres municipaux utilisent désormais la même unité d'encombrement que le sac et passent à **250 / 300 / 350 / 400 / 500** unités selon le niveau. L'achat reste un investissement de moyen terme à **350 PF de base** à Valebrume, avec les coefficients locaux existants ; les améliorations de niveau restent coûteuses.   
- Nouvelle amélioration **Place optimisée**, achetable à partir du coffre N2 : `-10 %` de l'encombrement de base stocké. Nouvelle amélioration **Rangement anticipé**, uniquement au N5 après la première : `-15 % supplémentaires par rapport à la valeur de base`. Les deux donnent donc **exactement -25 %**, et non un cumul multiplicatif.   
- Les coffres peuvent maintenant stocker de la monnaie physique avec ses dénominations exactes. Dépôt/retrait conserve les piles et leur encombrement ; la persistance utilise des entiers 64 bits pour éviter une perte sur de très grosses collections.   
- Dinotofu conseille contextuellement l'achat d'un coffre quand l'inventaire dépasse 85 % et qu'aucun coffre n'est possédé. Si le personnage n'a clairement pas les moyens, le conseil devient plutôt de vendre/faire du tri avant d'économiser pour le coffre.   
- Le catalogue de titres passe à **V3.51.51** et ajoute toute la branche stockage/encombrement/sacs/morphologie acceptée : notamment `Enfin de la place`, `On pousse les murs`, `Chaque chose à sa place`, `Optimisation maximale`, `Plein à craquer`, `Ça rentre encore.`, `Minimaliste`, `L'essentiel seulement`, `Conservateur`, `Réseau de stockage`, `Sous le matelas`, `Prêt pour le long voyage`, les parcours Fine/Robuste et les trophées de collection/logistique. Les titres de parcours incompatibles restent hors complétion ultime.   
- `Minimaliste` est vérifié au départ sous 10 %. `L'essentiel seulement`, `Voyageur organisé` et `Préparation impeccable` utilisent maintenant le **pic réel de charge de la sortie**, pas seulement l'état final. `Ça pèse moins comme ça` est relié à un change volontaire réduisant fortement le nombre de pièces physiques.   
- Le futur changement de morphologie en cours de partie par entraînement/alimentation reste volontairement **à développer** : il devra être long, explicitement voulu par le joueur et permettre aussi bien de mincir que de se renforcer. Les titres correspondants sont catalogués mais ne sont pas simulés avant ce vrai système.   
- Correction collatérale : la détection des matériaux osseux ne considère plus le mot générique `composant` comme un matériau dense simplement parce qu'il contient les lettres `os`.   
- `CarryLoadSystemTest`, `CityVaultIdentityTest`, `SaveRoundTripTest`, `EconomyScaleTest` et `TitleCatalogUpdateTest` protègent les nouvelles capacités, les -25 % exacts, les piles monétaires, la persistance et le catalogue. Schéma de sauvegarde **26** et checkpoint **V3.51.15** inchangés.   

---   

## V3.51.50 — Encombrement léger du joueur et voleurs devenant rivaux   

- Nouveau `CarryLoadSystem` : l'inventaire transporté utilise des **unités simples d'encombrement**, pas des kilogrammes. L'objectif est de donner une utilité réelle au coffre municipal sans transformer l'inventaire en simulation de poids.   
- L'équipement actuellement porté/équipé n'est pas recompté dans le sac afin d'éviter un double malus : son poids possède déjà ses propres compromis de combat. Les armes/armures de rechange, consommables groupés et piles de matériaux contribuent à l'encombrement.   
- Capacité volontairement généreuse, légèrement modulée par Force et gabarit/race. États joueur : `Confortable` jusqu'à 75 %, `Chargé` jusqu'à 100 %, `Surchargé` jusqu'à 125 %, puis `Très surchargé`. Jusqu'à 100 %, aucun malus.   
- Au-dessus de la capacité, seule une pression douce est active pour l'instant : `-5 %` de chance de fuite en Surchargé, `-10 %` en Très surchargé. Aucun objet n'est supprimé, aucun loot n'est bloqué et une ancienne sauvegarde surchargée reste jouable. Valeurs à playtester avant tout durcissement.   
- L'inventaire affiche l'état et les unités simples ; le coffre municipal affiche aussi l'encombrement porté et rappelle que déposer doublons/réserves le réduit. Le coffre gagne donc une fonction de gestion de charge en plus de la sécurité.   
- Les ennemis explicitement capables de voler (brigands, bandits, voleurs, roublards, pillards, contrebandiers, pickpockets...) peuvent désormais, après une **fuite réellement réussie**, avoir une faible chance actuelle de 9 % de subtiliser un petit bien accessible.   
- Cette première couche de vol de fuite ne touche jamais silencieusement objets de quête, uniques, identités de boss, renseignements/informations, notices ou clés. Elle vise de petites quantités de monnaie physique et/ou quelques matériaux ordinaires récupérables.   
- Un ennemi qui réussit ce vol devient **automatiquement un rival persistant** s'il ne l'était pas déjà. Le bien volé est enregistré sur ce rival afin qu'il puisse être retrouvé puis récupéré ; save/reload conserve réellement cette dette de rival.   
- La vue Mémoire du monde / Rivaux indique désormais explicitement les biens encore détenus par un rival. Vaincre réellement ce rival récupère les biens enregistrés sans duplication.   
- Le titre secret existant **« Rends-moi ça. »** s'élargit : il peut désormais être obtenu en récupérant un bien réellement volé par Grinka **ou** par un rival voleur retrouvé plus tard. Cette modification réelle de condition fait passer le catalogue de titres à **V3.51.50** sans changer le nombre de titres.   
- `CarryLoadSystemTest`, `RivalEscapeTheftRecoveryTest`, `SaveRoundTripTest` et `TitleCatalogUpdateTest` protègent l'encombrement, la différence de gabarit, la récupération idempotente du vol, sa persistance et la nouvelle version du catalogue.   
- Aucun nouveau développement de prêt/dette/découvert n'est repris. L'équilibre monétaire général reste audité. Schéma de sauvegarde **26** et checkpoint **V3.51.15** inchangés.   

---   

## V3.51.49 — Portage post-KO simplifié et récupération exacte des biens de Grinka   

- La gestion du poids reste volontairement **hors microgestion joueur** : aucun inventaire quotidien en kilogrammes, aucune jauge d'encombrement permanente et aucun malus automatique parce que le joueur conscient transporte quelques objets de plus. Les unités de portage restent un outil interne servant uniquement à résoudre les extractions automatiques après KO.   
- L'écran de butin post-KO n'affiche plus systématiquement les calculs `x/y unités` : la capacité est résumée qualitativement (`très limitée`, `limitée`, `correcte`, `confortable`) et le moteur garde le détail numérique uniquement pour ses calculs/journaux.   
- Grinka utilise désormais cette logique physique lorsqu'elle est vaincue par l'équipe pendant l'inconscience du joueur. Les biens personnels qu'elle avait réellement confisqués sont recherchés avant le butin ordinaire.   
- Une arme/armure de Grinka récupérée est **l'objet exact** conservé dans l'état persistant : même identifiant individuel, même durabilité et mêmes enchantements. Aucun clone équivalent n'est recréé.   
- Si l'équipe n'a pas assez de capacité pour transporter un bien confisqué en plus du joueur inconscient, le bien reste enregistré chez Grinka pour une récupération future au lieu d'être supprimé.   
- Priorité automatique actuelle en extraction Grinka : équipement personnel confisqué -> fragment notable du boss -> argent confisqué. L'argent est traité comme une bourse compacte afin d'éviter une simulation pièce par pièce.   
- Une victoire personnelle consciente contre Grinka garde le comportement simple : le joueur récupère ses biens normalement sans mini-jeu de portage. Le titre existant **« Rends-moi ça. »** est maintenant réellement accordé lorsqu'une arme ou armure exactement confisquée est récupérée. Le catalogue de titres reste V3.51.40 car le titre et sa condition n'ont pas été modifiés, seulement reliés au moteur.   
- Nouveaux accès sélectifs `recoverGrinkaStolenWeapon/Armor/Gold` : ils permettent une récupération partielle sans effacer le reste du registre de confiscation et empêchent une duplication lors d'une seconde tentative.   
- `GrinkaEquipmentRecoveryTest` vérifie identité persistante, durabilité, enchantements, récupération partielle, argent et absence de duplication.   
- Audit Zelef/Lyknir : leurs conséquences persistantes restent séparées du registre Grinka et du loot post-KO. Aucun chemin actuel de récupération post-KO ne recrée une arme détruite par une autre mécanique ni ne retire silencieusement une malédiction. Les audits plus fins de corrosion/malédiction d'équipement restent à poursuivre lorsque ces états seront attachés directement aux objets.   
- Aucun nouveau développement de prêt/dette/découvert n'est repris. L'équilibre monétaire général reste audité. Schéma de sauvegarde **26** et checkpoint **V3.51.15** inchangés.   

---   

## V3.51.48 — Fuite progressive joueur et portage post-KO par capacité réelle   

- La fuite du joueur rejoint la logique progressive déjà appliquée aux monstres : face à plusieurs bloqueurs, une tentative ne résout plus forcément toutes les ouvertures dans le même tour. Une ouverture réussie mais insuffisante reste acquise pour le combat et donne actuellement `+18 %` par ouverture conservée, borné à `+36 %`, sur la tentative suivante.   
- Une tentative de fuite du joueur reste une vraie action : progression partielle, blocage ou sortie complète consomment le tour. La progression est transitoire et remise à zéro au début d'un nouveau combat ; aucun nouveau champ de sauvegarde n'est nécessaire.   
- Le gabarit/race du joueur influence désormais légèrement le décrochage en plus de sa classe et de son équipement : profils naturellement petits/mobiles comme Fée/Kitsune/Gnome/Halfelin sont avantagés, tandis qu'Orc/Semi-dragon/Nain sont moins mobiles. Les valeurs restent modestes et à playtester.   
- `EscapeOppositionPressureTest` couvre maintenant également la progression multi-tours du joueur et vérifie qu'un profil Fée bénéficie d'un meilleur modificateur de mobilité qu'un profil Orc. La pression numérique reste la même pour les deux camps.   
- Le transport du butin post-KO n'est plus basé uniquement sur le nombre brut de survivants. Chaque survivant encore conscient fournit une **capacité de portage effective** selon son état de santé et son profil/gabarit : un allié très blessé ou très petit porte moins, tandis qu'un profil robuste/guerrier/porteur peut porter davantage.   
- Transporter le joueur inconscient consomme maintenant des unités de cette même capacité. Le reste constitue un budget commun de transport pour tout le loot : les lots compacts coûtent 1 unité, puis une arme/armure récupérée coûte réellement 1/2/3 unités selon sa classe de poids légère/moyenne/lourde.   
- Les équipements lourds ne possèdent donc plus une réserve indépendante pouvant contourner le poids des lots compacts. Ramener une armure lourde peut réellement empêcher le groupe de rapporter d'autres dépouilles derrière.   
- `LootGenerator::givePostKoRecoveredWaveLoot` consomme ce budget partagé au fil de la récupération et indique la capacité utilisée/restante ; le surplus reste physiquement abandonné. La priorité élite/évolution/niveau de V3.51.47 est conservée.   
- `RecruitedAllyAftermathSystemTest` verrouille un porteur moyen, deux porteurs, un porteur robuste Orc/guerrier et une petite Fée très blessée. L'audit économique général reste actif : aucune capacité de portage supplémentaire ne crée de loot ou d'argent qui n'existait pas déjà.   
- Aucun nouveau développement de prêt/dette/découvert n'est repris. Aucun titre n'a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.47 — Fuite progressive et transport physique du butin post-KO   

- La fuite ennemie n'est plus un jet plat qui oublie immédiatement une ouverture réussie. Chaque monstre conserve désormais une **progression de décrochage temporaire** pendant le combat : face à plusieurs bloqueurs, une première ouverture réussie crée de la distance sans retirer encore la créature du combat.   
- Une ouverture déjà acquise donne un bonus important à la tentative suivante (`+18 %` par ouverture conservée, borné actuellement à `+36 %`). Ainsi, une créature ayant déjà commencé à décrocher a réellement davantage tendance à retenter puis réussir sa sortie, au lieu de repartir exactement de zéro.   
- La mobilité du profil influence réellement cette fuite : petites créatures/races mobiles et profils décrits comme agiles/légers sont avantagés ; constructions, dragons, géants, gardiens, tanks et profils massifs/lourds sont désavantagés. Les valeurs sont bornées et restent à playtester.   
- La pression numérique de V3.51.46 reste commune : environ 5 adversaires capables de fermer les issues demandent 2 ouvertures. Si le nombre de bloqueurs diminue après une première percée, la progression déjà réellement acquise n'est pas effacée artificiellement.   
- Une tentative de fuite de monstre est maintenant une **vraie action de tour**. Fuite totale, progression partielle ou ouverture bloquée consomment toutes le tour : un monstre ne peut plus rater sa fuite puis attaquer gratuitement dans la même phase. Les fuites de morale utilisent la même règle.   
- `EscapeOppositionPressureTest` couvre désormais progression sur plusieurs tours, bonus de nouvelle tentative et avantage mobilité d'un petit profil agile face à un profil massif/tank. Les principes restent volontairement proches des règles joueur/PNJ : même pression du nombre et même logique de mobilité, avec des modificateurs propres au profil.   
- Première couche de **transport physique du butin après KO** : lorsque l'équipe termine un combat pendant l'inconscience du joueur, le butin récupérable dépend des survivants encore capables de porter et n'est plus supposé se téléporter intégralement dans l'inventaire.   
- Transporter le joueur inconscient consomme une partie de cette capacité. Un seul survivant peut ramener au mieux un petit lot compact en plus du joueur ; les survivants supplémentaires augmentent la capacité. Les équipements lourds utilisent une capacité séparée et plus stricte.   
- Pour les vagues PvE standard/exploration, seuls les lots correspondant à la capacité physique sont récupérés ; le reste est explicitement abandonné. La sélection privilégie actuellement les ennemis élites/évolués puis de plus haut niveau afin d'éviter un abandon économiquement absurde.   
- Les matériaux, petites monnaies et autres récompenses compactes restent plus faciles à transporter que des armes/armures. Le nombre d'équipements récupérés est borné par les places lourdes réellement disponibles.   
- Les boss vaincus après KO utilisent également cette couche de transport : leur butin n'est remis au réveil que si au moins un survivant capable peut réellement ramener le lot avec le joueur. Une victoire observée entièrement depuis le banc sans participation personnelle reste, comme avant, sans loot de boss personnel.   
- `RecruitedAllyAftermathSystemTest` verrouille la capacité avec 1/2 survivants, le coût du transport du joueur et l'exclusion d'un allié KO. Cette passe continue aussi l'audit économique général en empêchant le gain automatique de tout le loot post-KO.   
- Aucun nouveau développement de prêt/dette/découvert n'est repris. Aucun titre n'a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.46 — Réserve observable et pression symétrique sur la fuite   

- La **réserve/file d’attente n’est plus assimilée à une absence de visibilité**. Dans les combats où elle reste physiquement au bord de la scène — PvE standard, exploration et arènes/boss employeurs actuels — une recrue peut observer directement des rounds sans jamais prendre de tour. Participation active et observation deviennent deux notions distinctes.   
- Les fenêtres de connaissance post-KO enregistrent désormais les rounds observés depuis la réserve séparément des rounds vécus sur la ligne. Un reporter à `0` tour joué peut donc fournir un témoignage direct complet s’il a réellement assisté à toute la scène.   
- Cette correction supersède la lecture trop stricte de V3.51.45 : **entrer tard sur le terrain ne signifie pas forcément découvrir tard le combat**. Si la réserve avait une vue crédible, les rounds antérieurs peuvent être connus directement ; si elle était éloignée, cachée ou absente, les lacunes restent inconnues/seconde main comme avant.   
- Observer ne donne aucun droit économique automatique : les personnages n’ayant effectué **aucun tour actif** sont exclus du partage normal des récompenses de combat, même s’ils ont tout vu. Aucune observation ne crée non plus un clear, un kill ou une participation personnelle artificielle.   
- Un joueur salarié qui passe un boss entièrement sur le banc obtient maintenant une vraie trace d’**observation/rencontre** dans le bestiaire et le journal lorsque la scène est visible, tout en restant exclu des récompenses et clears personnels du boss.   
- La fuite des monstres utilise maintenant la même pression numérique que celle du joueur : plus il reste d’opposants vivants et capables de fermer les issues, plus il faut créer d’ouvertures successives pour s’échapper. Les monstres ne conservent plus une chance plate indépendante du nombre de poursuivants.   
- En PvE standard/exploration, le calcul des opposants capables de gêner la fuite d’un monstre compte le joueur vivant, les recrues actives, les recrues en réserve physiquement proches et les invocations encore actives. Exemple actuel : face à 5 opposants, le fuyard doit réussir **2 ouvertures successives**.   
- Les réactions de morale ennemies qui choisissent la fuite passent elles aussi par cette pression d’opposition. Un échec peut donc laisser la créature paniquée/coincée au lieu de la téléporter hors du combat.   
- `EscapeOppositionPressureTest` verrouille la montée de pression avec le nombre d’opposants. Les tests d’aftermath et de boss post-KO couvrent aussi un observateur intégralement resté dans la réserve, sans participation active.   
- Aucun nouveau développement de prêt/dette/découvert n’est repris. L’équilibre monétaire général reste audité : cette passe protège notamment le partage de récompense contre un spectateur n’ayant pas combattu.   
- Aucun titre n’a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.45 — Témoignages post-KO sourcés et victoire d’équipe distincte   

- Les comptes rendus après KO ne reçoivent plus le bilan omniscient de toute la simulation. Chaque participant conserve uniquement pendant le combat sa fenêtre d’observation post-KO : premier/dernier round réellement vus, nombre d’ennemis vus tomber, résultat final réellement constaté et capacité à participer au débrief.   
- Une recrue arrivée tard ne connaît donc plus magiquement les rounds précédents. Si un autre survivant a réellement observé depuis le début, le reporter peut citer cette personne comme **source de seconde main** ; sinon la lacune reste explicitement inconnue.   
- `RecruitedAllyAftermathSystem::buildKnowledgeReport` centralise cette distinction entre observation directe, seconde main sourcée et information inconnue. Le moteur peut connaître le résultat complet sans l’injecter automatiquement dans le dialogue du personnage.   
- Les simulations post-KO PvE standard/exploration et boss alimentent désormais ces fenêtres d’observation. Les alliés tombés sont exclus du choix du reporter ; les survivants encore capables de parler peuvent confirmer la fin qu’ils ont réellement vue.   
- Les boss à seconde phase, notamment Hitogami, conservent une numérotation de rounds continue et réinitialisent correctement le statut « résultat final observé » lorsqu’une nouvelle phase invalide la fin précédente.   
- Le rapport RP et la **part économique** sont présentés comme deux opérations distinctes. Refuser ou limiter un débrief ne crée ni ne supprime automatiquement l’argent dû au joueur.   
- Nouvelle trace canonique `team_victories_without_personal_clear` lorsqu’une équipe termine un combat pendant l’inconscience du joueur. Cette victoire d’équipe reste distincte d’un clear personnel et ne donne aucun titre/clear gratuit.   
- `RecruitedAllyAftermathSystemTest` couvre désormais reporter tardif, source directe depuis le round 1, absence de source crédible et témoin couvrant toute la phase. `RecruitedAllyBossPostKoTest` vérifie aussi les fenêtres d’observation réellement produites pendant une simulation de boss.   
- Le développement des prêts/dettes/découverts reste en pause. L’équilibre monétaire général continue cependant d’être audité, notamment la séparation entre récompense de groupe, part du joueur et absence de clear personnel.   
- Aucun titre n’a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.44 — Réserve sans tour gratuit et récupération minimale des techniques combinées   

- La règle **« entrer depuis la réserve ne donne jamais un tour gratuit »** n'est plus limitée aux boss : elle est maintenant centralisée dans `RecruitedAllyCombatRoster` avec un marqueur transitoire consommé exactement une fois.   
- Une rotation manuelle du joueur-chef effectuée avant la phase alliée marque chaque recrue entrante : elle saute uniquement cette fenêtre immédiate, puis retrouve son rythme normal au tour suivant.   
- Les rotations du chef employeur en PvE standard appliquent la même règle. Une recrue appelée pendant la phase du chef ne peut donc plus agir quelques instants plus tard pendant la phase des autres collègues du même round.   
- Les switchs individuels demandés ou autonomes restent distincts : ils consomment déjà le tour du membre sortant dans leur propre phase, donc le remplaçant n'est pas puni par un second tour perdu artificiellement.   
- Les boss réutilisent désormais le même helper de réserve au lieu d'entretenir une logique parallèle. `RecruitedAllyCombatRosterTest` verrouille la consommation exacte d'une seule fenêtre.   
- Les techniques combinées disposent maintenant d'une **récupération de combat explicite** indépendante de la simple disponibilité de technique personnelle. Après toute tentative réelle — réussie, partielle ou ratée — il faut laisser passer la riposte immédiate puis un round intermédiaire avant une nouvelle tentative.   
- Le menu de consignes affiche cette récupération et désactive proprement « Technique combinée » tant que la paire n'a pas retrouvé sa fenêtre. Une recrue venant juste d'entrer depuis la réserve ne peut pas contourner son délai via une technique combinée.   
- `DuoMasterySystem` expose la règle de récupération de façon testable : état initial après tentative = 2 ripostes, puis 1, puis 0/prêt. `DuoMasterySystemTest` couvre cette progression.   
- Cette cadence empêche les boucles de type `A -> B -> A -> B` avant une vraie réponse ennemie, sans déplacer artificiellement l'initiative des ennemis ni leur retirer un tour.   
- Aucun développement nouveau prêt/dette/découvert n'a été repris. **L'équilibre monétaire global reste audité** en permanence, y compris les conséquences économiques des combats, récompenses et mécaniques de groupe.   
- Aucun titre n'a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.43 — Focus Fire exact et relecture contextuelle des switchs autonomes   

- `Focus Fire` conserve désormais la **cible exacte désignée par le chef** au lieu de se rabattre silencieusement sur le premier ennemi encore présent. Si cette cible quitte réellement le combat, l'ordre expire proprement.   
- Chaque monstre reçoit à son entrée dans une file de combat un **identifiant d'instance temporaire** (`enemy_1`, `enemy_2`, ...), non sauvegardé. Deux ennemis portant exactement le même nom restent donc distinguables sans modifier le schéma de sauvegarde.   
- `PlayerWaveCombatTurn` remonte maintenant une observation structurée de l'action : type d'action, nom/identité de la cible quand elle est réellement connue et indication d'un ciblage imposé par une règle de combat.   
- Les attaques et compétences réellement ciblées peuvent donc être comparées à la cible `Focus Fire`. Bonne cible = ordre obéi ; autre cible choisie librement = désobéissance injustifiée ; cible différente imposée par provocation/contrainte de combat = écart justifié.   
- Une compétence multi-cible, aléatoire ou dont le moteur ne connaît pas une cible unique reste **inconclusive** : aucune sanction n'est inventée à partir d'une information absente.   
- Les switchs individuels sans autorisation des **collègues employeurs PNJ** reçoivent maintenant une relecture contextuelle : témoins, PV, peur, urgence de survie, qualité du remplaçant et opportunité tactique. Survie ou vrai gain tactique peuvent justifier l'écart ; peur/urgence ambiguë reste inconclusive ; aucun témoin reste non observé.   
- Cette relecture de switch est historisée dans `team_employer_unauthorized_switch_reviews` et n'applique pas automatiquement une sanction : le futur chemin de switch volontaire du joueur devra réutiliser la même logique avant de clore totalement ce chantier.   
- `EmployerTeamDisciplineSystemTest` couvre maintenant `Focus Fire` exact, cible forcée, cible expirée, absence de cible connue et switchs autonomes justifiés/injustifiés/non observés. `EnemyGroupBehaviorTest` verrouille aussi l'unicité des identifiants de combat malgré des noms identiques.   
- Aucun nouveau développement prêts/dettes/découvert n'est repris. **L'équilibre monétaire général reste cependant audité en permanence** comme contrainte transversale du jeu.   
- Aucun titre n'a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.42 — Réserve employeur contre boss et aucune entrée avec tour gratuit   

- Les combats de boss avec équipe employeuse permettent maintenant au joueur salarié de **commencer réellement sur le banc** au lieu d'occuper automatiquement une place principale.   
- Tant que le joueur est en réserve, les ennemis ciblent les PNJ réellement présents sur la ligne ; le joueur n'est pas traité comme une cible active cachée.   
- Le chef employeur peut sacrifier un vrai tour pour rappeler le joueur sans attendre un KO, tandis qu'une place libérée naturellement peut aussi déclencher son entrée.   
- Entrer depuis la réserve ne donne jamais d'action bonus immédiate. Les rotations PNJ contre boss suivent aussi ce principe d'absence de tour gratuit.   
- Si l'escouade employeuse abat le boss avant l'entrée personnelle du joueur, le résultat devient une **victoire d'escouade**, pas un faux clear personnel : aucune XP de boss, aucun butin ni titre personnel artificiel. Le salaire du contrat reste séparé.   
- Les blessures des collègues employeurs pendant que le joueur observe depuis le banc sont persistées normalement.   
- Clarification : le développement prêts/dettes de guilde reste en pause, mais **l'équilibre monétaire global reste une contrainte permanente** pour récompenses, salaires, loot, prix et prévention des exploits.   
- Aucun titre n'a changé : catalogue toujours **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.41 — Obéissance tactique observable et retour du chef   

- La branche **finance/prêts/dettes est volontairement mise en pause** à la demande de l’utilisateur. Cette version reprend donc les chantiers combat/commandement sans ajouter de nouvelle mécanique financière.   
- `EmployerTeamDisciplineSystem` sait maintenant évaluer les actions du joueur face à un ordre tactique employeur selon des faits réellement observables : ordre actif, type d’action, présence du chef, témoins crédibles, état de survie et résultat d’une tentative de fuite.   
- Nouveau principe dur : **aucun témoin crédible = aucune sanction disciplinaire**. Si le chef n’a pas vu l’action mais qu’un collègue employeur vivant était présent, le fait peut être conservé comme rapport d’équipe au lieu de rendre le chef omniscient.   
- Les verdicts sont désormais séparés en `non observé`, `inconclusif`, `obéi`, `désobéissance justifiée` et `désobéissance injustifiée`. Une action ambiguë ne baisse jamais automatiquement la confiance.   
- Les tours perdus à cause d’un état subi (`forced_skip`) sont explicitement inconclusifs : le joueur n’est pas puni pour une impossibilité d’agir.   
- Une fuite sous ordre actif n’est plus traitée uniformément : hors urgence vitale observable elle peut devenir une désobéissance injustifiée ; à très bas PV / danger immédiat elle peut au contraire être reconnue comme un écart justifié, notamment si l’extraction réussit.   
- Les ordres simples disposent de premières correspondances sûres : défense/attente pour `Tenir la ligne`, attaque/compétence pour `Percée contrôlée`, coordination d’équipe pendant `Extraire les blessés`. `Focus Fire` reste volontairement **inconclusif** tant que la cible exacte choisie par le joueur n’est pas conservée par ce suivi.   
- `PlayerWaveCombatTurn` expose maintenant un observateur d’action non intrusif utilisé par le PvE employeur. Les combats PvE standards et d’exploration enregistrent les actions réellement consommées sans modifier les autres appels existants.   
- La discipline n’est appliquée qu’une fois au **bilan du combat**, évitant de farmer la confiance en répétant des actions conformes tour après tour. Le chef peut alors produire un retour RP après une victoire ou une fuite lorsque le joueur est conscient.   
- Les observations et bilans sont historisés dans `team_employer_order_observations` et `team_employer_order_reviews`, avec distinction vision directe du chef / rapport de collègues / absence de témoin.   
- `EmployerTeamDisciplineSystemTest` couvre désormais ordre tenu, action ambiguë, absence de témoin, fuite injustifiée et fuite de survie justifiée.   
- Aucun nouveau titre n’a été ajouté : la version du catalogue de titres reste donc **V3.51.40**. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.40 — Audit massif des titres, mode spoilers, nouveaux cheats et premiers rappels de dette   

- Le catalogue de titres possède désormais une **version indépendante** : **V3.51.40**. Elle ne change que lorsqu'un titre est réellement ajouté, retiré ou modifié, afin de savoir précisément depuis quand la collection n'a pas été réauditée.   
- Audit rétrospectif des systèmes récents : le catalogue passe d'environ **181 à 326 titres**, avec de nouvelles familles finance/guilde, compagnons, commandement, duo, survie, mana/hybride, langues, rumeurs, exploration, rivaux, mémoire d'équipement, serments, réputation, magie matérielle, emploi, FireFlight et REAPERS-13.   
- `Reaper-13` n'est volontairement **pas** un titre joueur normal tant que le chef existe. Le scénario extrême où le joueur tue ce chef mène plutôt au titre secret **« Cible prioritaire »**, puisque le clan entier aurait de très bonnes raisons de réagir.   
- Nouveau paramètre **Titres cachés** : désactivé par défaut. L'activer demande de saisir explicitement `SPOILER` après un avertissement indiquant que noms et conditions peuvent révéler histoire, boss, factions, systèmes, résultats et secrets. L'option est réversible et persistante par personnage.   
- Les écrans de titres affichent maintenant la version du catalogue et respectent ce mode : comportement normal en `????` par défaut, noms + conditions révélés seulement après consentement explicite.   
- Les codes validés `thatsalotofpower`, `icanfeelit`, `bornforit`, `nottoday`, `fearmeinstead`, `trustmebro`, `fillmypocket`, `justanotherday`, `luckytofu`, `onemorechance` et `everybodylovesme` sont maintenant réellement branchés, insensibles à la casse et tracés dans le registre des altérations.   
- `luckytofu` n'accorde jamais **« Luck is a skill »** ; `fillmypocket` ne génère aucun objet unique/quête/boss ; `onemorechance` réarme exactement une tentative supplémentaire de résistance spirituelle ; `trustmebro` agit déjà sur la loyauté des équipes employeuses et sera étendu aux autres loyautés persistantes lorsque celles-ci seront unifiées.   
- Protocol 13 légitime peut maintenant débloquer **« Canal interdit »**. Plusieurs titres financiers sont branchés aux événements réels ; **« Bon payeur »** exige désormais 3 prêts remboursés dans les temps au lieu d'un seul. Les titres dépendant de systèmes encore futurs restent catalogués mais ne sont pas simulés artificiellement.   
- Premier niveau de gestion des **retards de prêt** : un prêt arrivé à échéance suspend tout nouveau découvert/crédit jusqu'à régularisation, sans effacer ni aggraver artificiellement la dette. Un rappel canonique est créé au premier jour puis périodiquement.   
- Le dossier financier distingue désormais `Difficulté financière probable`, `Capacité de paiement partielle — situation à discuter` et `Fonds suffisants — situation à régulariser`. Le jeu **n'accuse jamais automatiquement de mauvaise foi** : cette conclusion exigera plus tard des refus explicites, tentatives de négociation et autres preuves comportementales.   
- Nouveau `TitleCatalogUpdateTest` et extensions des tests cheats/finance : version du catalogue, 320+ titres, persistance du mode spoilers, absence volontaire de `Reaper-13`, effets des nouveaux codes, restriction de crédit et rappels de retard.   
- Aucun changement de schéma : sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.39 — Prêts de guilde réels, longues durées et priorité de remboursement   

- Les **prêts de guilde deviennent réels** : un adhérent suffisamment fiable peut recevoir un capital physique, avec un seul prêt actif à la fois et un plafond progressif selon la confiance financière jusqu'à un maximum théorique de **5 PO**.   
- Les durées disponibles sont **1 / 2 / 3 / 4 / 6 / 8 semaines**. Taux de base : 12 / 14 / 16 / 18 / 21 / 24 %. Les durées longues ajoutent dès la signature une majoration distincte : **3 sem. +2 %, 4 sem. +2 %, 6 sem. +3 %, 8 sem. +4 %**. Exemple : 3 semaines = 16 % + 2 % = **18 % signé**.   
- Les longues durées demandent davantage de confiance : seuils progressifs de 55 / 60 / 65 / 70 / 80 / 90 selon la durée. Un nouveau membre ne peut donc pas prendre immédiatement le prêt maximal.   
- Le coût signé est **fixe** : remboursement anticipé autorisé, mais les intérêts signés ne diminuent pas rétroactivement. Aucun intérêt composé automatique n'est ajouté.   
- Deux modes de remboursement : **manuel**, ou **automatique à l'échéance** depuis le solde réellement déposé auprès de la guilde. L'automatique ne téléporte jamais les pièces de la bourse distante.   
- Les prolongations demandées **avant échéance** sont actives : +1 semaine = +3 %, +2 = +5 %, +3 = +7 % du capital initial. Le coût global est plafonné à **30 %** ; une prolongation qui dépasserait ce plafond est tronquée à la marge restante.   
- Un prêt arrivé à échéance avec reste dû passe en retard sans générer d'intérêts composés supplémentaires. La future négociation/recouvrement reste séparée afin d'éviter les spirales de dette.   
- Les remboursements de prêts fonctionnent depuis la bourse ou le compte de guilde. La confiance financière tient désormais compte des prêts remboursés dans les temps et pénalise un prêt en retard.   
- Si **découvert + prêt** coexistent, un nouveau menu permet un remboursement global depuis la bourse ou le compte de guilde avec priorité explicite : **découvert d'abord [recommandé]**, prêt d'abord ou répartition. Le jeu ne décide jamais silencieusement à la place du joueur.   
- Les catégories canoniques de découvert et de prêt sont protégées contre la compaction du journal afin qu'une dette persistante ne disparaisse jamais au milieu d'une longue partie.   
- Le cheat **`whatdebt`** est maintenant connecté : il efface artificiellement découvert et prêt sans prélever de pièces, marque le personnage Altéré et ne compte jamais comme un remboursement honorable.   
- `GuildFinanceSystemTest` couvre désormais taux/majorations, prêt unique, capital physique, prolongation, plafond 30 %, remboursement anticipé, save/reload d'un prêt actif, paiement automatique, retard sans composition et priorité multi-dettes. `CheatLegacyCodesTest` couvre aussi `whatdebt`.   
- Le bloc financier terminal a été extrait de `QuestMenu.cpp` vers `GuildFinanceMenu.cpp` : `QuestMenu.cpp` repasse sous la limite de modularisation et les prochains ajouts financiers disposent désormais de leur propre module. Les tests de structure suivent ce nouvel emplacement.   
- Aucun changement de schéma : sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.38 — Compte de guilde plafonné et vrai découvert réseau   

- Le compte de guilde n'est plus un stockage monétaire illimité : capacité de **2 PO** pour un Aventurier non adhérent et **5 PO** pour un adhérent actif. Ces valeurs restent à calibrer après playtest afin de préserver l'intérêt du coffre personnel.   
- Une ancienne sauvegarde V3.51.37 déjà au-dessus du nouveau plafond ne perd rien : aucun argent n'est confisqué et les retraits restent possibles ; seuls les nouveaux dépôts sont bloqués jusqu'à repasser sous la limite.   
- `GuildFinanceSystem` expose maintenant capacité totale et marge restante du compte. Le menu **Gestion de l'argent** affiche explicitement `solde / capacité` et borne la quantité maximale proposée lors d'un dépôt.   
- Le **découvert de guilde devient réel** : dette persistante `guild_finance_overdraft_draws - guild_finance_overdraft_repayments`, plafond déterminé par l'adhésion et la confiance financière, et marge restante calculée sans jamais rendre la bourse négative.   
- Le découvert reste un **crédit réseau** et ne peut jamais être retiré comme argent physique. Un service affilié prélève d'abord les pièces réellement portées puis, seulement après autorisation explicite, la guilde règle le manque et l'inscrit comme dette.   
- Première intégration partenaire : la **chambre sûre de l'auberge** peut proposer le découvert si la bourse ne suffit pas, avec détail avant validation de la part physique, du crédit nécessaire, de la dette actuelle et de la marge restante.   
- Le découvert peut être remboursé manuellement depuis la bourse ou depuis le solde déjà déposé à la guilde. Le remboursement depuis le compte réduit simultanément le dépôt et la dette sans téléporter de pièces.   
- L'expiration de l'adhésion met le plafond de nouveau crédit à zéro mais ne supprime jamais un découvert déjà utilisé et n'empêche pas son remboursement.   
- `GuildFinanceSystemTest` couvre désormais plafonds de dépôt, refus de dépassement, paiement partenaire mixte bourse/crédit, remboursements, persistance save/reload et conservation de la dette après expiration de l'adhésion.   
- Correction de `scripts/test_project.sh` : le test financier lancé en parallèle est maintenant explicitement attendu avant le message final de succès.   
- Aucun changement de schéma : sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.37 — Gestion de l’argent et adhésion financière de guilde   

- Le **Bureau de change** est remplacé dans le menu de guilde par une option unique **Gestion de l’argent** regroupant bourse physique, conversions PC/PF/PE/PO/PP, compte de guilde, adhésion et aperçu des futurs découvert/prêts.   
- La `Cotisation aventurier standard` déjà existante devient l’adhésion de guilde de référence au lieu de créer un deuxième abonnement concurrent. Les anciennes sauvegardes déjà adhérentes restent compatibles ; l’adhésion conserve ses anciens avantages de services.   
- L’adhésion aventurier est désormais gérée depuis le comptoir financier de la guilde et n’est plus vendue depuis les listes d’abonnements de l’auberge ou des cotisations de ville. Elle garde une période de 7 jours, renouvellement/cancellation et renouvellement automatique existants.   
- Nouveau `GuildFinanceSystem` : un Aventurier peut déposer une somme physique au compte de guilde puis la retirer plus tard. Le dépôt retire réellement les pièces de la bourse, le retrait les rend physiquement ; la bourse ne devient jamais négative.   
- Le solde de guilde est reconstruit depuis des événements canoniques protégés `guild_finance_deposits` / `guild_finance_withdrawals`, ce qui conserve l’historique et permet un round-trip de sauvegarde sans changer le schéma 26.   
- Première **confiance financière** calculée à partir du dossier réel : inscription, contrats de guilde réussis/échoués, adhésion active et présence d’un solde déposé. Elle prépare les futurs plafonds de crédit sans encore créer de dette.   
- L’écran membre utilise les libellés non ambigus validés : `Découvert utilisé`, `Plafond de découvert`, `Marge restante`. Le plafond affiché est encore explicitement une estimation de préparation : aucun découvert réel n’est créé dans cette version.   
- Les prêts affichent uniquement les règles déjà validées (adhésion, plafond futur ~5 PO selon dossier, durées, remboursement manuel/automatique depuis un solde déposé) sans encore pouvoir créer de contrat.   
- Nouveau test `GuildFinanceSystemTest` : inscription requise, dépôt/retrait, refus de solde insuffisant, adhésion, confiance, futur plafond membre uniquement et persistance save/reload.   
- Aucun changement de schéma : sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.36 — Registre des cheats, Protocol 13 contextuel et anciens codes reconnectés   

- Le trophée spécial **« On ne dira rien... »** est désormais synchronisé au niveau du compte lors de la première altération. C'est volontairement le seul trophée de compte obtenable par triche et il reste explicitement exclu des prérequis du titre ultime. Les titres normaux forcés par un futur cheat restent, eux, strictement locaux au personnage.   
- `DINOTOFU:13-02313` devient une ouverture **unique par personnage**. Une deuxième saisie n'émet jamais une nouvelle balise et indique que le protocole a déjà été transmis ou que le canal n'est plus joignable.   
- Protocol 13 est désormais contextuel : si le personnage possède déjà une trace crédible des Reapers (PNJ, rumeur ou rencontre), l'usage reste diégétique et non altérant ; sans aucune connaissance en jeu, le code est traité comme du métagame et rend le personnage Altéré. En mort définitive, une tentative métagame refusée consomme aussi l'unique ouverture afin d'empêcher le spam.   
- La fiche/statistiques du personnage affiche maintenant un registre persistant **Codes / protocoles utilisés** avec nombre d'utilisations et, quand l'état est déterminable, `Actif`, `Inactif`, `Ponctuel`, `Consommé` ou un statut dédié.   
- Les anciens codes historiques documentés mais déconnectés ont été reliés à `CheatManager` : `13023131302313`, `iamnotgoblinfood`, `letmecook`, `whereismyplotarmor`, `talktothedev`, `oopsallbosses` et `randomizemyfate`.   
- `13023131302313` restaure immédiatement PV/mana puis arme une exécution sur la prochaine cible hostile ordinaire réellement attaquée. Les boss restent soumis à leurs mécaniques propres au lieu d'être supprimés par un raccourci hors contexte.   
- `iamnotgoblinfood` influence réellement la morale des gobelins ; `letmecook` élargit temporairement la fenêtre de critique jusqu'au prochain jour du monde ; `whereismyplotarmor` ajoute une vraie bénédiction de survie ; `oopsallbosses` révèle le registre des boss ; `randomizemyfate` réattribue classe et kit de départ en conservant niveau/identité de propriété ; `talktothedev` reste un secret non altérant.   
- Nouveau squelette `SystemCallManager` : `system_call` ouvre réellement uniquement l'écran **SYSTEM CALL** et son invite, sans aide, liste ni autocomplétion. Les futures sous-commandes `generate_*` restent volontairement secrètes et encore à connecter.   
- Nouveau test `CheatLegacyCodesTest` et extension de `AccountTitleProgressTest` pour couvrir trophée de compte, Protocol 13 unique/contextuel et reconnexion des anciens cheats.   
- Aucun changement de schéma : sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.35 — Titres de compte, personnages Altérés et canal Protocol 13   

- Les titres légitimement obtenus sont désormais archivés au niveau du **compte** dans une collection persistante « Titres débloqués ». Cette collection survit à la suppression du personnage qui avait obtenu le titre et possède une vue dédiée depuis le menu de compte.   
- Un personnage devenu **Altéré** par un cheat conserve ses titres déjà obtenus mais les attributions normales de nouveaux titres sont bloquées. Sa sauvegarde ne peut plus enrichir la collection légitime du compte. Les titres gagnés légitimement avant la première altération sont figés dans le compte juste avant l'application du premier cheat.   
- Nouveau titre local d'altération **« On ne dira rien... »** : il est accordé au premier passage en Altéré, reste propre au personnage, n'entre jamais dans les trophées du compte et est explicitement exclu des prérequis du titre ultime. Les anciennes sauvegardes déjà Altérées récupèrent ce marqueur au chargement si nécessaire.   
- Une API explicite `grantCharacterOnlyTitleFromCheat` permet aux futurs cheats de type skip/unlock d'accorder un titre uniquement au personnage courant sans contaminer les trophées du compte. L'audit confirme qu'aucun cheat « débloquer tous les titres » n'existe actuellement ; `skipallstory` ne débloque pas les titres.   
- Correction d'une incohérence historique : le secret `1302313`, documenté comme non altérant, ne marque plus le personnage Altéré lorsqu'il affiche le message du créateur.   
- Nouveau protocole diégétique non altérant `DINOTOFU:13-02313` : il ouvre **PROTOCOL 13**, enregistre une balise/canal interdit REAPERS-13 dans l'historique canonique et n'accorde ni recrutement, ni victoire, ni apparition gratuite. La future réponse doit venir du monde et de la faction réelle.   
- Les cheats altérants en mort définitive restent sanctionnés ; les protocoles secrets explicitement non altérants ne déclenchent pas cette sanction.   
- `CHEATS_DINOTOFU.txt`, `TITRES_DINOTOFU.txt` et `SYSTEMES_PREVUS.txt` documentent maintenant les règles validées : futurs `SYSTEM CALL`/`generate_*`, génération toujours altérante, anti-tamper de sauvegarde à venir, codes validés à connecter, système financier de guilde désormais débloqué, lore REAPERS-13 et future sélection FR/EN au premier lancement.   
- Nouveau test `AccountTitleProgressTest` : conservation des trophées légitimes après suppression du personnage, blocage des titres normaux après altération, attribution locale par cheat, exclusion du marqueur d'altération du compte et validation de Protocol 13 non altérant. La suite complète passe avant le bump.   
- Aucun changement de schéma : sauvegarde **26** et checkpoint important **V3.51.15** inchangés. L'histoire reste en pause.   

---   

## V3.51.34 — Switch individuel autorisé, débordements de zone et rotation de boss   

- Nouveau `CombatIndividualSwitchSystem` : un membre qui veut quitter la ligne de sa propre initiative demande normalement l'autorisation au chef encore présent. Un refus du chef laisse le membre en ligne et ne consomme pas son action normale.   
- Peur, instinct de survie, panique ou jugement tactique immédiat peuvent exceptionnellement pousser un membre à switcher sans attendre l'autorisation. Cette initiative reste volontairement rare (contournement borné à 18 %) et laisse une trace distincte afin de pouvoir être jugée plus tard comme initiative justifiée ou désobéissance.   
- Les recrues personnelles demandent réellement l'autorisation au joueur-chef via un écran dédié ; les collègues d'une équipe employeuse passent par la décision du chef PNJ. Les ennemis non-chefs suivent la même logique : demande au meneur lorsqu'il existe, refus possible, ou mouvement instinctif rare selon peur, intelligence, discipline et cohésion.   
- Nouveau `CombatReserveSpilloverSystem` : les effets réellement assez larges peuvent déborder vers la réserve à seulement **5 à 15 %** de leur puissance. Une zone de bord touche au maximum 1 membre en attente, une zone large 2, une zone massive 3. Les effets sans portée suffisante ne touchent jamais la file.   
- La consigne alliée « soin de groupe » déclenche désormais un vrai soin de zone résolu par un soutien capable. La ligne reçoit l'effet principal ; la réserve ne reçoit qu'un écho de 5 à 15 % selon la portée, le rang et le profil du soigneur.   
- Les soigneurs ennemis expérimentés peuvent eux aussi lancer occasionnellement un soin de groupe. Leurs renforts en attente ne reçoivent que le même débordement réduit : la règle est symétrique et ne favorise pas artificiellement le joueur.   
- La Bombe d'atelier est le premier effet explosif joueur explicitement relié à la file ennemie : son explosion peut toucher la réserve à 5-15 %, avec un nombre de cibles borné par la taille réelle de la zone. `EnemyCombatQueue` sait désormais blesser/soigner un membre en attente sans le faire entrer sur la ligne et retire proprement une cible de réserve tuée par une zone.   
- Les grosses pressions de boss peuvent également atteindre faiblement la réserve alliée lorsqu'elles représentent réellement une zone assez large ; une pression locale ne traverse pas automatiquement la formation. Un membre de réserve tombé ainsi ne peut pas entrer ensuite comme remplaçant.   
- Le chef PNJ d'une équipe employeuse peut maintenant sacrifier son tour **face à un boss** pour faire reculer un collègue blessé et faire entrer un membre de réserve, sans attendre un KO. La rotation conserve les phases du boss et ne donne aucune action supplémentaire.   
- Nouveaux tests `CombatIndividualSwitchSystemTest` et `CombatReserveSpilloverSystemTest`; `EmployerTeamBossSupportTest` couvre maintenant aussi la rotation de chef contre boss. `test_project.sh` attend explicitement ces tests avant d'annoncer le succès. La suite complète passe avant le bump.   
- Limites volontaires : tous les anciens effets de zone du jeu ne sont pas encore classés par portée ; l'évaluation disciplinaire automatique d'un switch sans autorisation reste à relier aux faits observables ; la rotation manuelle du joueur-chef contre boss reste à intégrer proprement à l'interface de boss.   
- Aucun changement de dette/prêt ni d'histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.33 — Rotations de chef et premiers ordres tactiques réels   

- Nouveau `CombatLeaderRotationSystem` : un chef opérationnel peut désormais utiliser son propre tour pour échanger **1 ou 2 membres** entre la ligne active et la réserve. Une rotation réussie consomme exactement le tour du chef et ne crée jamais d’action supplémentaire.   
- Le joueur chef dispose d’une rotation manuelle dans les consignes d’équipe. Les deux échanges sont préparés avant mutation afin qu’une rotation double reste atomique. Un membre très mal en point peut rarement refuser ; le chef conserve alors son tour normal, mais l’action Rotation est verrouillée jusqu’au prochain tour afin d’empêcher le spam de l’ordre.   
- Le chef PNJ d’une équipe employeuse peut maintenant réorganiser ses collègues ou **rappeler volontairement le joueur depuis la liste d’attente sans attendre qu’un membre tombe**. La décision n’est pas systématique et dépend de la situation ; une rotation réussie remplace l’action normale du chef.   
- Les vagues ennemies utilisent la même règle : lorsqu’un vrai meneur est présent, il est replacé en tête de l’ordre d’action et peut sacrifier son tour pour faire reculer un ou deux alliés blessés au profit de la réserve. Intelligence, cohésion, solidarité et comportement de groupe influencent cette décision ; les créatures/groupes qui abandonnent leurs blessés l’emploient beaucoup moins. L’ancien switch ennemi gratuit est supprimé.   
- `EnemyCombatQueue` sait désormais replacer explicitement le chef actif en tête sans créer/supprimer de tour. Les rotations ennemies conservent les mêmes effectifs et ne sont possibles qu’avec une vraie réserve compatible.   
- Nouveau `EmployerTacticalOrderSystem` : un chef employeur peut émettre au début d’un engagement une consigne crédible parmi protéger le soutien, concentrer une cible, tenir la ligne, extraire les blessés ou lancer une percée contrôlée. Les collègues appliquent réellement cette consigne via le `RecruitedAllyActionPlanner` et les systèmes d’ordres existants.   
- Le joueur salarié reste libre de son personnage. La discipline n’interprète pas encore automatiquement chaque action : seules des désobéissances suffisamment mesurables devront être sanctionnées dans une passe suivante afin d’éviter des pénalités arbitraires.   
- Nouveaux tests `CombatLeaderRotationSystemTest` et `EmployerTacticalOrderSystemTest`, plus branchement à `test_project.sh`. La suite complète passe avant le bump.   
- Limites volontaires : la rotation générique des chefs n’est pas encore branchée à toutes les formations spéciales de boss, et l’obéissance/désobéissance du joueur aux ordres tactiques n’est pas encore entièrement qualifiée automatiquement.   
- Aucun changement de dette/prêt ni d’histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.32 — Discipline d’équipe employeuse, liste d’attente et boss sous contrat   

- Les escouades employeuses comptent maintenant **3 à 5 PNJ avant de recruter le joueur**, soit 4 à 6 membres maximum avec lui. Une équipe pleine à 6 ne propose donc pas artificiellement une septième place au personnage.   
- Nouveau `EmployerTeamDisciplineSystem` : chaque contrat suit une confiance professionnelle, des avertissements, missions réussies/ratées, ordres obéis ou désobéis et affectations temporaires en liste d’attente. La confiance commence à 70/100 et une faute isolée ne provoque pas un licenciement immédiat.   
- Les sanctions deviennent progressives : mission ratée ou désobéissances répétées baissent la confiance ; plusieurs incidents peuvent placer le joueur en réserve pour un ou plusieurs combats ; les fautes réellement répétées peuvent rendre le licenciement disciplinaire crédible. Une désobéissance justifiée, notamment pour éviter un dommage grave, n’est pas traitée comme une faute automatique.   
- Un licenciement disciplinaire peut retenir **50 à 75 % du salaire encore dû** selon la gravité, avec plafond dur à 75 %. Le salaire déjà versé n’est jamais repris. Le règlement final garde une trace du montant réellement versé et du montant retenu.   
- Accepter l’offre locale attribue désormais immédiatement un premier ordre de mission : le joueur ne peut plus signer uniquement pour toucher un salaire tout en n’ayant aucune obligation de service. Les échecs synchronisés alimentent la discipline et plusieurs fautes peuvent fermer automatiquement le contrat.   
- La liste d’attente est branchée au PvE standard et à l’exploration : le joueur peut commencer hors des trois places principales, voir réellement le chef et ses collègues combattre, puis être rappelé si une place se libère ou après quelques rounds afin d’éviter une attente frustrante. Si l’escouade gagne avant son entrée, l’affectation est purgée mais aucune XP ni butin personnel de combat n’est créé artificiellement.   
- Le roster employeur est maintenant branché aux **boss** pendant une mission employeur active. Le chef opérationnel ouvre la phase alliée avant le joueur, les autres collègues agissent ensuite, leurs PV/blessures/progression persistent dans le roster employeur et ils restent payés par leur organisation plutôt que par une part prélevée sur le joueur.   
- Si le joueur est actuellement sanctionné en liste d’attente, l’appui de l’employeur n’est pas prêté pour contourner la sanction lors d’un boss personnel. La mise en réserve complète/visible face aux boss reste à approfondir.   
- Nouveaux tests `EmployerTeamDisciplineSystemTest` et `EmployerTeamBossSupportTest`; les tests existants de roster/aftermath ont été adaptés à la nouvelle taille minimale de 3 PNJ. La suite complète passe avant le bump.   
- Aucun changement de dette/prêt ni d’histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.31 — Escouade employeuse persistante et chef toujours premier   

- Nouveau `EmployerTeamRosterSystem` : un contrat externe possède désormais une véritable escouade persistante distincte des recrues personnelles. Le roster crée 4 à 5 collègues PNJ sous le chef du contrat, soit 5 à 6 membres au total avec le joueur, et conserve identité, rôle, niveau, rang, PV, qualité d'équipement, humeur et loyauté de base dans le journal canonique.   
- La règle d'équipe est maintenant explicite : **le chef opérationnel joue toujours en premier**. Dans une équipe employeuse, l'ordre d'ouverture est chef PNJ → joueur → autres collègues actifs ; un chef KO/indisponible ne reçoit jamais de tour fantôme. La règle ne crée aucun tour supplémentaire et ne ressuscite personne.   
- La formation employeuse utilise les trois places principales existantes : avec le joueur présent, 2 collègues PNJ maximum sont actifs et les autres attendent en réserve ; après la chute du joueur, jusqu'à 3 collègues peuvent occuper les places libérées. Le roster reste plafonné à 5 PNJ afin de ne jamais dépasser 6 membres d'équipe.   
- Le menu Équipe expose maintenant l'escouade employeuse réelle : actifs, réserve et indisponibles, ordre d'ouverture, PV, niveau, rôle, humeur et loyauté. Le groupe personnel reste séparé et n'est jamais converti en roster de l'employeur.   
- Pendant une **mission employeur réellement active**, le PvE standard et l'exploration utilisent l'escouade de l'employeur au lieu des recrues personnelles. Le chef agit avant le joueur, les autres collègues agissent ensuite via le `RecruitedAllyActionPlanner` commun, et les collègues salariés ne prélèvent aucune part de recrue sur le butin du joueur.   
- Les collègues employeurs possèdent désormais des PV/blessures et une progression de combat séparés. Un membre mis KO devient indisponible jusqu'à son jour estimé de retour puis revient avec des PV partiels ; sa participation reste enregistrée même s'il termine le combat blessé.   
- Le post-KO utilise maintenant tous les participants réellement impliqués, y compris ceux entrés depuis la réserve. La part économique due au joueur est conservée pour remise au réveil, sans être redistribuée comme salaire de recrue. Le compte rendu conserve la logique RP commune (~85 % modulée) avec catégories dédiées à l'employeur.   
- Un collègue employeur ne peut détourner la part du joueur que si son profil et son état relationnel rendent la trahison crédible ; le détournement est historisé dans le roster employeur et le membre devient indisponible/parti au lieu d'être confondu avec une recrue personnelle.   
- Nouveaux tests `EmployerTeamRosterSystemTest` et `EmployerTeamAftermathSystemTest`, plus gardes de régression dans `test_project.sh` sur le branchement des missions employeur et la priorité de tour du chef.   
- Limites volontaires : les boss n'utilisent pas encore le roster employeur ; les ordres tactiques riches du chef PNJ, les relations évolutives entre collègues, les besoins, les inventaires d'objets complets et les missions employeur spécialisées restent à développer.   
- Aucun changement de dette/prêt ni d'histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.30 — Contrats d’équipe employeuse, salaire et missions du chef   

- Nouveau `TeamEmploymentSystem` : un personnage peut désormais avoir un véritable contrat persistant avec une équipe/organisation externe, distinct de son groupe personnel. Le contrat conserve l'identité du chef PNJ, l'organisation, le rôle, le jour de début, une échéance éventuelle, la périodicité de paie, les périodes déjà réglées et l'historique de fin de contrat.   
- Le salaire externe est réellement acquis avec le temps du monde : seules les périodes complètes écoulées peuvent être encaissées. Plusieurs périodes peuvent s'accumuler ; une démission ou une fin de contrat règle uniquement le salaire déjà acquis et ne crée jamais de paie anticipée par simple clic. Le joueur salarié n'est pas responsable de la masse salariale de l'équipe employeuse.   
- Nouveau `TeamEmploymentMissionSystem` : le chef de l'équipe employeuse peut attribuer une vraie quête de journal liée au contrat actif. Le client et l'origine correspondent au chef et à l'organisation, la progression passe par les combats réels, l'ordre peut expirer/échouer et sa prime reste séparée du salaire contractuel. Les réussites et échecs sont historisés une seule fois.   
- Nouveau `TeamEmploymentOfferSystem` : lorsqu'aucun contrat n'est actif et qu'une ville valide est connue, une escouade locale affiliée à la ville peut proposer un premier contrat jouable de terrain. L'offre expose chef, organisation, rôle, salaire, périodicité et durée avant confirmation ; l'acceptation bascule la hiérarchie vers le vrai chef externe sans effacer le groupe personnel du joueur.   
- Le menu Équipe n'affiche plus le joueur comme chef en dur lorsqu'il travaille pour quelqu'un. L'onglet équipe employeuse permet de consulter le statut, récupérer le salaire dû, recevoir un ordre de mission, revoir les droits de membre et démissionner. Le groupe personnel est explicitement conservé à part pendant le contrat.   
- Une mission employeur encore ouverte est désormais automatiquement interrompue et historisée comme telle si le contrat prend fin (échéance, démission ou licenciement) ; aucun ancien ordre ne reste orphelin après le retour au groupe personnel.   
- Les états de contrat, paiements et missions employeur sont stockés dans le journal canonique persistant sans nouveau schéma de sauvegarde. Les catégories correspondantes sont protégées comme traces narratives.   
- Nouveaux tests `TeamEmploymentSystemTest`, `TeamEmploymentMissionSystemTest` et `TeamEmploymentOfferSystemTest`; la suite complète protège contrat, paie acquise, échéance, démission, mission employeur, historique et offre locale.   
- Limite volontaire de cette passe : les collègues PNJ de l'équipe employeuse ne constituent pas encore un roster persistant complet en combat. Leur progression, humeur, besoins, équipement, rotation/réserve et les ordres tactiques du chef externe restent le prochain chantier ; l'offre locale actuelle sert de première route jouable et non de simulation finale de toutes les organisations.   
- Aucun changement de dette/prêt ni d'histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.29 — Survie létale finale, résistance spirituelle et hiérarchie d'équipe   

- Le sauvetage en mort définitive par l'équipe est recalibré : un allié réellement compétent en soin/réanimation peut désormais offrir **33 à 66 %** de survie selon sa compétence effective, son niveau estimé, son rang, sa maturité et son matériel. Un survivant sans compétence adaptée reste limité à **15 %** de stabilisation/transport d'urgence ; un excellent kit ne transforme toujours pas un non-soigneur en réanimateur.   
- Le sauveteur doit encore être **réellement debout au moment final de la résolution**. En PvE standard/exploration et contre les boss solo, la continuation cachée du combat/extraction est désormais simulée avant le jet de sauvetage létal : un soigneur tombé pendant cette continuation n'accorde donc aucun bonus. Les protections particulières de FireFlight et la seconde vie d'Hitogami restent respectées.   
- Nouveau `LethalSpiritResistanceSystem` : lors de la **première** mort définitive seulement, après échec du secours normal, le personnage peut tenter une ultime résistance spirituelle. La séquence utilise 3 réussites contre 3 échecs, d20 réussi sur 13+, soit environ 31,7 % de survie globale ; réussite ou échec consomme définitivement cette chance. Une réussite rend 1 % des PV sans valider artificiellement un clear.   
- La résistance spirituelle est branchée au PvE standard/exploration, aux boss, au coop et au JcJ létal. En JcJ, survivre par la volonté ne déclenche pas les conséquences destructrices propres à la bénédiction de survie ; la bénédiction rare existante reste un recours séparé seulement après l'échec du secours et de cette première résistance.   
- Nouveau `TeamLeadershipSystem` : le groupe personnel considère le joueur comme chef par défaut, tandis qu'une équipe externe peut déclarer un chef PNJ ou un autre joueur, une organisation et un salaire. Lorsqu'il n'est pas le vrai chef, le joueur ne peut plus réordonner ni commander artificiellement toute l'équipe depuis les menus concernés.   
- La fondation de hiérarchie distingue maintenant clairement le membre salarié du propriétaire de l'équipe : un joueur employé par une organisation externe peut être marqué comme payé et non responsable des coûts de l'équipe. Le versement réel du salaire, les contrats, quêtes du chef et comportements de désobéissance restent à développer dans les prochaines passes.   
- `LethalTeamRescueSystemTest` couvre maintenant le plafond à 66 %, le soigneur mort et le non-soigneur restant à 15 %. Nouveaux tests `LethalSpiritResistanceSystemTest` et `TeamLeadershipSystemTest`, plus gardes de régression sur les continuations létales et les intégrations PvE/boss/JcJ.   
- Aucun changement de dette/prêt ni d'histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.28 — IA alliée commune et sauvetage létal d'équipe   

- Nouveau `RecruitedAllyActionPlanner` : la décision tactique d'une recrue est centralisée entre PvE standard, boss et simulation hors écran. Le planificateur choisit survie personnelle, soin du chef, soin d'un allié, garde, technique ou attaque à partir du rôle, du rang/niveau, de la maturité, de l'équipement, de l'état du groupe, des ordres et du contexte du combat.   
- `MonsterPveMode` et `RecruitedAllyBossSupport` utilisent désormais ce même planificateur. L'affichage et l'exécution restent propres au mode afin de conserver les vraies cibles, techniques et mécaniques de boss, mais la logique de décision n'est plus recopiée entre combat visible, boss et post-KO.   
- Nouveau `LethalTeamRescueSystem` pour la mort définitive en équipe. Un allié vivant avec de vraies capacités de soin/réanimation offre une chance de survie bornée entre **33 et 50 %** selon son profil/compétence, son rang, sa maturité et son matériel.   
- Une équipe encore debout mais dépourvue de véritable sauveteur ne bénéficie que de **15 %** de survie par stabilisation/transport d'urgence. Un kit médical de qualité ne transforme pas à lui seul un vieux guerrier sans formation en réanimateur. Si aucun allié n'est encore debout, le secours d'équipe est impossible.   
- La règle est reliée au PvE standard/exploration, aux boss solo avec recrues, au PvE coop humain et aux boss coop. En coop, les sauveteurs sont figés parmi les personnages réellement debout à la résolution afin d'éviter une chaîne artificielle où le premier réanimé sauverait immédiatement tous les autres.   
- L'ancien jet de survie coop autonome façon « trois verts/trois rouges » est remplacé par le secours réel de l'équipe : la survie vient maintenant des capacités des alliés présents plutôt que d'un jet personnel sans sauveteur. Une bénédiction de survie léthale reste un recours séparé après l'échec du secours normal.   
- Un sauvetage réussi ne compte pas la mort définitive ; un échec laisse la résolution normale se poursuivre. Les tentatives et survies sont enregistrées dans l'historique canonique. FireFlight conserve ses verrous personnels : être sauvé n'accorde ni clear ni test final gratuit.   
- Nouveaux tests `RecruitedAllyActionPlannerTest` et `LethalTeamRescueSystemTest`, plus gardes de régression vérifiant que PvE/boss/coop restent réellement branchés aux systèmes communs. Le script de tests attend désormais explicitement ces tests parallèles avant d'annoncer la réussite.   
- Aucun changement de dette/prêt ni d'histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.27 — Places 3+2, conséquences post-KO RP et équipe contre les boss   

- Les places de combat suivent désormais explicitement la règle **3 places principales partagées + 2 places bonus d'invocation**. Les places principales priorisent joueurs, puis alliés PNJ, puis invocations ; une invocation peut utiliser une place principale restée libre avant d'occuper les deux bonus. Le surplus reste en file/réserve.   
- `CombatSlotRules` centralise les limites 3/2/5 et `CombatGroupBuilder` place les unités prioritaires avant les invocations. Les tests couvrent notamment 1 combattant + 4 invocations et 3 combattants + 2 invocations.   
- Après une chute non définitive en PvE standard/exploration, le résultat hors écran devient un véritable état : jusqu'à 3 recrues peuvent reprendre les places libérées, la réserve entre en jeu, les recrues gagnent progression/parts selon leur activité et la part économique due au joueur est conservée jusqu'au réveil.   
- Nouveau `RecruitedAllyAftermathSystem` : le compte rendu au réveil part d'une base d'environ 85 %, puis dépend de la relation, de l'humeur, des salaires impayés, des soins/sauvetages vécus et du tempérament. Un compagnon peut abrèger/refuser le débrief ; un vol de la part du joueur n'est possible que pour un profil réellement douteux ou un contrat déjà abîmé, avec départ persistant du groupe.   
- Les recrues parties/ayant trahi sont désormais exclues du roster actif/réserve. Un allié loyal avec une relation normale n'est pas transformé arbitrairement en voleur par un simple jet aléatoire.   
- Les boss solo peuvent désormais continuer hors écran après le KO non définitif du joueur : la troisième place principale peut être reprise par la réserve, les alliés se soignent/soutiennent entre eux, le boss conserve sa pression et son état d'ultime, et les blessures/évacuations restent persistantes.   
- Un boss terminé par les recrues pendant l'inconscience du joueur est historisé comme **victoire de l'équipe**, pas comme clear personnel : aucun titre/registre personnel n'est accordé gratuitement. La part économique du joueur peut être remise au réveil selon le même système RP.   
- FireFlight reste un défi personnel : les recrues peuvent sécuriser une extraction mais ne peuvent ni porter son dernier PV à zéro ni valider son test final. Hitogami conserve sa seconde vie même si sa première phase est terminée par les recrues.   
- Les règles futures de hiérarchie d'équipe sont consignées : le chef contrôle l'ordre volontaire ; le joueur dirige normalement son groupe, mais s'il travaille pour une équipe externe il devient membre salarié dépendant de son chef au lieu de contrôler artificiellement cette équipe.   
- Les recrues tombées pendant une simulation cachée restent des participantes du combat : leurs tours, dégâts, soutien, XP et parts ne disparaissent plus simplement parce qu'elles terminent à l'infirmerie. Le même principe couvre PvE standard/exploration et boss. Un combat post-KO sans monnaie peut toujours faire progresser les recrues.   
- Les informations cachées restent réellement cachées : la seconde vie d'Hitogami peut se déclencher pendant l'inconscience sans afficher sa scène au joueur. FireFlight n'enregistre une extraction par les recrues que si quelqu'un a effectivement survécu pour ramener le joueur.   
- Nouveaux tests : `CombatSlotRulesTest`, `RecruitedAllyAftermathSystemTest` et `RecruitedAllyBossPostKoTest`. Suite complète verte avant et après bump.   
- Aucun changement de dette/prêt ni d'histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.26 — Équipe de six et coordination de duo réellement vécue   

- Une équipe de voyage solo compte désormais **6 personnes maximum au total** : le joueur et jusqu’à 5 recrues réellement engagées. Tant que le joueur est conscient, les trois places physiques de combat sont le joueur + 2 recrues (les invocations ne consomment pas ces places). Si le joueur tombe en mort non définitive, sa place se libère et jusqu’à 3 recrues peuvent combattre simultanément ; les autres restent en réserve.   
- Les profils retenus par Torvald sont maintenant explicitement des candidats, pas des membres du groupe. Un simple profil ne consomme aucune place et ne peut plus apparaître dans la réserve de combat ; la limite de cinq recrues s’applique lors de l’engagement réel. Les anciennes sauvegardes avec trop de recrues équipées sont bornées par le roster sans supprimer leur historique.   
- Après une chute non définitive du joueur en PvE standard/exploration, le combat des recrues et de la réserve continue en simulation hors de sa vue. Le personnage inconscient ne voit plus les rounds automatiques qu’il ne peut pas connaître ; après son réveil, une recrue survivante peut éventuellement lui faire un compte rendu bref du résultat.   
- `DuoMasterySystem` suit maintenant une progression réellement vécue du binôme : combinaisons réussies/partielles, journées de combat communes uniques, journées d’ordres coordonnés uniques et relation réelle avec les deux recrues influencent coordination et qualité du lien. Les traces de journée sont uniques par jour du monde afin d’empêcher le farm trivial en boucle.   
- Une technique combinée peut réussir complètement, être rattrapée partiellement ou casser sur un mauvais timing. Une exécution partielle perd les principaux effets tactiques et une partie de sa puissance ; un échec complet consomme quand même les deux tours alliés. Les duos mûrs peuvent atteindre une véritable signature commune, les novices restent nettement moins fiables.   
- L’expérience commune est enregistrée sur la participation réelle même lorsqu’un combat ne rapporte aucune monnaie. La valeur de la récompense ne décide donc plus si le combat a compté dans le vécu du duo.   
- Règle future multijoueur enregistrée : 3 joueurs humains maximum dans une même équipe. Le mélange exact joueurs/recrues du futur réseau reste volontairement non figé.   
- Les règles d’accès croisé restent explicites : quête difficile d’éveil du mana pour un non-mage, entraînement persistant important en mêlée pour un mage pur, voie distance mixte.   
- `RecruitedAllyCombatRosterTest` protège maintenant les limites 6 total / 5 recrues / 2 actives et l’exclusion des simples profils du combat/réserve. `DuoMasterySystemTest` protège les traces quotidiennes anti-farm, la progression relation/lien et les bornes de coordination.   
- Aucun changement dette/prêt ni histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.25 — Recrues actives contre les boss solo   

- Le roster de combat des recrues est extrait de `MonsterPveMode` vers `RecruitedAllyCombatRoster` : engagé, non renvoyé, hors infirmerie/évacuation, équipé, ordre manuel, maximum 2, rang/niveau, PV persistants, équipement et parts de récompense utilisent désormais une base partagée.   
- `MonsterPveMode` consomme ce roster partagé au lieu de reconstruire localement l’état initial des recrues.   
- `BossPveMode::run` charge maintenant les 0 à 2 recrues équipées pour un boss solo. Elles jouent un vrai tour allié après le joueur : soutien/soin pour les profils adaptés, sinon attaque mono-cible du boss.   
- Les boss exercent une pression de zone bornée sur les recrues. Leurs PV sont réels et persistants ; une recrue tombée est marquée à évacuer vers l’infirmerie au lieu d’être remise debout gratuitement.   
- La participation, les dégâts, le soutien, l’XP et la part économique des recrues sont enregistrés après une victoire de boss. Le joueur conserve la plus grosse part économique ; son XP de boss reste entière comme dans le système PvE existant.   
- Les mécaniques uniques de boss restent prioritaires : les recrues ne remplacent pas la règle mortelle de FireFlight, son test final ni les verrous/phases spécifiques.   
- Nouveau `RecruitedAllyCombatRosterTest` : maximum 2 recrues, ordre manuel, exclusion infirmerie et exclusion d’une recrue renvoyée.   
- Règles futures de progression ajoutées à `SYSTEMES_PREVUS.txt` : un non-mage doit éveiller son mana via une quête difficile avant toute magie ; un mage pur doit suivre de vrais entraînements importants avant de devenir convenable au corps-à-corps ; la distance reste la voie mixte à difficulté globalement unifiée.   
- Aucun changement de dette/prêt ni d’histoire. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   


## V3.51.24 — Échanges transactionnels et fin d’audit des transferts d’équipement   

- Les échanges volontaires entre deux personnages passent désormais par `ExchangeTransferSystem` pour l’argent, les armes, les armures, les consommables et les matériaux. Chaque transfert retire d’abord la ressource au donneur avant de l’ajouter au receveur, afin d’éviter les duplications en cas d’échec inattendu.   
- Les armes et armures données conservent leur ID persistant exact. Le donneur reçoit un événement durable `exchange_given` et le receveur `exchange_received`, avec des résumés canoniques `echanges_donnes` / `echanges_recus`. Les consommables utilisent une identité sémantique et les matériaux une identité `id + qualité + quantité`, conformément à leur modèle actuel de sauvegarde.   
- Le menu d’historique du personnage expose maintenant les échanges donnés et reçus. Un échange annulé ou invalide n’affiche plus à tort une sauvegarde réussie.   
- La sauvegarde d’un échange est couplée avec `SaveManager::savePlayerExchangePair` : l’état précédent des deux personnages et de leurs comptes est capturé, puis restauré si l’une des deux sauvegardes normales échoue. Le menu restaure également les deux objets `Player` en mémoire dans ce cas. Cette protection cible les erreurs d’écriture gérables ; elle ne prétend pas fournir une transaction disque atomique face à une coupure brutale du processus ou de la machine.   
- L’audit des autres transferts/pertes d’équipement a confirmé les chemins boutique/revente/rachat, enchanteur, mort non définitive, Grinka, cycle d’équipement et JcJ déjà protégés ou tracés.   
- Le coffre municipal conserve désormais aussi l’identité persistante exacte des armes/armures dans son journal canonique, au lieu de fusionner deux objets portant simplement le même nom. Les matériaux de coffre et transports inter-villes sont distingués par `id + qualité`.   
- `ExchangeTransferSystemTest` couvre argent, arme, armure, consommable, matériau, échec sans mutation et préservation des IDs persistants. `CityVaultIdentityTest` couvre dépôt/retrait d’arme et d’armure avec identité persistante ainsi que la qualité des matériaux. La suite projet vérifie les branchements de ces protections.   
- Aucun changement d’équilibrage, de dette/prêt ou d’histoire dans cette passe. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.23 — Traçabilité des équipements pris en JcJ   

- Les armes et armures réellement récupérées sur un perdant lors d’un combat JcJ mortel sont désormais historisées côté perdant comme `equipment_lost`, avec l’ID persistant exact de l’objet ou un repli type/nom pour les anciennes données.   
- Le vainqueur reçoit en parallèle une provenance canonique `butin_notable` liée à la même identité d’objet, avec le nom du perdant et le contexte du duel. Un même équipement peut donc conserver une trace cohérente lorsqu’il change réellement de propriétaire.   
- Les vols d’équipement porté lors d’un duel à mort définitive distinguent le fait que l’objet était équipé ; les prises dans l’inventaire distinguent le duel définitif du duel mortel non définitif.   
- Aucun historique n’est créé si le jet de pillage échoue, si l’objet de départ est protégé, ou si le transfert ne peut pas être effectué.   
- Les transferts JcJ retirent maintenant l’objet du perdant avant de l’ajouter au vainqueur, ce qui évite une duplication silencieuse si une suppression d’inventaire échouait de manière inattendue.   
- `PvpEquipmentHistoryTest` couvre l’ID persistant, le repli d’ancienne donnée, la perte durable côté perdant et la provenance côté vainqueur. La suite projet vérifie aussi que tous les chemins de vol d’arme/armure de `PvpMode` appellent cette traçabilité.   
- Les probabilités, limites de valeur, récompenses et règles de duel restent inchangées. Schéma de sauvegarde **26** et checkpoint important **V3.51.15** inchangés.   

---   

## V3.51.22 — Confiscations d’équipement par Grinka   

- Les armes et armures réellement confisquées par Grinka après une défaite sont désormais consignées dans l’historique durable (`equipment_lost`), avec ID persistant ou repli type/nom.   
- Il s’agit d’un vol récupérable, et non d’une destruction : les stocks de récupération de Grinka restent inchangés.   
- Aucun événement ajouté si la protection empêche le vol ; chances et équilibrage inchangés.   

---   

Historique détaillé des versions de Dinotofu en français. Le journal anglais équivalent se trouve dans `CHANGELOG.md`.   

## V3.51.21 — Historique des destructions d’équipement chez l’enchanteur   

- Un échec critique d’enchantement détruisant réellement une arme ou une armure laisse désormais un événement durable `equipment_destroyed`.   
- L’événement conserve l’ID persistant de l’équipement, avec repli sur le type et le nom lorsqu’aucun ID n’existe, ainsi que le contexte réel de la destruction.   
- Le sceau anti-casse protège toujours l’équipement : aucune fausse destruction n’est enregistrée quand le sceau intercepte l’échec.   
- Les risques, prix, enchantements, butins et règles de sauvegarde ne sont pas modifiés.   
- La dette/prêt reste en attente de validation, l’histoire reste en pause, et le schéma 26 ainsi que le checkpoint V3.51.15 sont conservés.   

---   

## V3.51.20 — Pertes forcées d’équipement et historique de mort non définitive   

- Les pertes d’équipement provoquées par une mort non définitive laissent maintenant une **trace historique durable** avec l’identité persistante réelle de l’objet concerné.   
- Une arme ou une armure **volée** après la défaite est enregistrée comme équipement perdu ; un objet devenu **irréparable puis détruit** est enregistré comme équipement détruit ; une **casse forcée** qui laisse encore l’objet dans l’inventaire est enregistrée séparément.   
- Le registre **Historique du personnage → Événements durables récents** affiche des libellés lisibles pour ces nouvelles catégories sans inventer de provenance quand aucune vraie perte n’a eu lieu.   
- La logique de casse forcée ne marque plus artificiellement comme « brisé par le choc » un équipement qui était déjà à 0 de durabilité avant le jet : l’événement n’est créé que lorsqu’une casse forcée fait réellement passer un objet encore utilisable à 0.   
- `Random` accepte désormais un seed explicite afin de rendre les scénarios probabilistes reproductibles dans les tests sans modifier le comportement aléatoire normal du jeu.   
- `DeathPenaltyHistoryTest` couvre de manière déterministe le vol d’une arme, la casse forcée réparable et la destruction irréparable d’une armure.   
- Aucun pourcentage d’équilibrage de mort n’a été modifié dans cette passe : la calibration reste réservée au playtest.   
- Le système de dette/prêt reste volontairement **non implémenté** en attente de validation utilisateur.   
- Schéma de sauvegarde inchangé : **26**. Checkpoint important inchangé : **V3.51.15**.   

---   

## V3.51.19 — Ramassage configurable et provenance du butin   

- Le ramassage du butin devient configurable dans les préférences d'inventaire. La valeur par défaut est **Tout récupérer automatiquement**. Deux alternatives existent : **Manuel uniquement**, qui demande une décision pour chaque objet généré, et **Filtrage personnalisé**, qui ramasse tout sauf les exclusions explicites.   
- Le filtrage peut exclure des catégories complètes (matériaux, armes, armures, consommables) ainsi que des matériaux précis déjà connus/possédés. Un objet filtré n'est jamais vendu ni détruit automatiquement : il est simplement laissé sur place et signalé dans le récapitulatif de butin.   
- Les préférences de ramassage sont persistées de manière additive dans les sauvegardes (`lootPickupPolicy`, catégories ignorées et matériaux ignorés). Les anciennes sauvegardes utilisent automatiquement **Tout récupérer**. Le schéma reste 26.   
- Le générateur de butin de combat respecte désormais ces règles pour les matériaux et les équipements récupérés. Les récompenses authored de quête ne passent volontairement pas par ce filtre afin de rester exactes et garanties.   
- L'historique du personnage gagne une section **Butin remarquable**. Les armes/armures récupérées sur les ennemis et les matériaux rares, magiques ou de qualité élevée laissent une trace canonique avec leur provenance quand elle est connue.   
- Les tests de sauvegarde protègent le mode par défaut, les filtres persistants et leur compatibilité ; `test_project.sh` protège aussi le branchement réel du générateur de butin et du registre de provenance.   
- Le système de dette/prêt/sanctions économiques proposé après V3.51.18 reste volontairement **non implémenté** en attente de validation de sa structure détaillée.   
- Schéma de sauvegarde inchangé : **26**. Checkpoint important inchangé : **V3.51.15**.   

---   

## V3.51.18 — Compagnons motivés, amitié et salaires réels   

- Les profils retenus à la guilde sont désormais distincts des recrues réellement engagées. Une fiche conservée n'entre plus automatiquement dans le rang du groupe, le partage des récompenses, les salaires ni l'équipe active. Les anciennes sauvegardes restent compatibles : les anciennes « recrues retenues » sont considérées comme déjà engagées.   
- Les profils pouvant exiger une quête personnelle doivent réellement l'achever avant l'engagement. Une action **Engager un profil retenu** est ajoutée au menu Équipe.   
- Chaque nouvelle recrue reçoit une motivation initiale crédible : **contrat**, **objectif commun** ou **affinité / plaisir d'aventure**. Cette motivation est persistée dans son enregistrement et affichée dans sa fiche.   
- Une valeur d'amitié est dérivée uniquement de faits vécus et persistants : participations, quêtes communes, semaines actives, quête personnelle, soins et sauvetages. Elle ne monte pas arbitrairement au recrutement. Les impayés peuvent la faire baisser.   
- Le salaire hebdomadaire devient une vraie dépense : il est désormais prélevé de la bourse physique du joueur au bilan. Un paiement ou un impayé laisse une trace canonique.   
- Les exceptions demandées sont implémentées : une recrue poursuivant réellement le même objectif peut renoncer au salaire pour la semaine concernée ; une recrue venue par affinité peut finir par voyager gratuitement si une véritable amitié s'est construite ; une relation exceptionnellement forte peut également justifier ce choix.   
- Le partage de récompenses et le rang du groupe ignorent explicitement les simples profils retenus. Les profils non engagés reçoivent 0 %, ne gonflent pas le clan et ne déclenchent pas les systèmes hebdomadaires.   
- L'affichage hebdomadaire est 1-based côté joueur : la première semaine est **Semaine 1**, tout en conservant les clés internes historiques.   
- `SYSTEMES_PREVUS.txt` consigne maintenant les idées à conserver pour plus tard : coop multijoueur par sessions, arène en ligne avec file de combattants, IA RP « Parler avec Dinotofu », FireFlight IA seulement comme piste peu probable/expérimentale, GUI confiée à l'amie de l'utilisateur, pixel-art en dernier et histoire gelée jusqu'à nouvelle demande explicite avec objectif chapitres 1–3 jouables puis message de suite en développement.   
- Les tests projet protègent l'engagement réel, l'amitié, les motivations, les salaires payés/impayés, les objectifs communs et les sauvetages.   
- Schéma de sauvegarde inchangé : **26**. Checkpoint important inchangé : **V3.51.15**.   

---   

Historique détaillé des versions de Dinotofu en français. Le journal anglais équivalent se trouve dans `CHANGELOG.md`.   

## V3.51.17 — Épingles d’inventaire, munitions favorites et historique canonique   

- La préférence générique **Favori** de V3.51.16 est séparée en deux concepts : **Épinglé** contrôle uniquement l’ordre d’affichage de l’inventaire, tandis que **Munition favorite** sert uniquement au mode de sélection automatique des projectiles. Épingler un objet ne peut donc plus provoquer sa consommation automatique.   
- Armes, armures, consommables et matériaux peuvent être **épinglés**. Les épingles remontent en tête des listes interactives et du résumé « Voir tout », avec le tag `[ÉPINGLÉ]`. Verrouillage et suivi restent indépendants.   
- Les munitions favorites sont limitées aux vraies familles de projectiles : flèches, carreaux et couteaux de lancer. Une seule favorite est conservée par famille, ce qui permet par exemple d’avoir simultanément une flèche favorite et un carreau favori sans ambiguïté.   
- Le mode **Favori si disponible** reste disponible uniquement pour les munitions. Les matériaux magiques ne proposent plus de favori automatique : ils restent sur **Toujours demander** par défaut ou **Automatique prudent** si le joueur l’active explicitement.   
- Compatibilité V3.51.16 : les anciens `favoriteItemKeys` sont migrés en **épingles** pour conserver l’intention visuelle. Quand un ancien favori correspond réellement à une munition, il est aussi converti en munition favorite. Les nouvelles sauvegardes utilisent `pinnedItemKeys` et `favoriteAmmunitionIds`. Le schéma reste **26** car la lecture de l’ancien format est conservée.   
- Le squelette `CharacterHistoryMenu` devient un vrai registre accessible depuis **Stats → Historique du personnage**. Il expose les événements historiques durables déjà persistés ainsi que les achats, ventes, rachats et équipements réellement portés.   
- Les transactions de boutique alimentent désormais le journal canonique : achats, ventes par quantité et rachats sont agrégés par entrée. Équiper réellement une arme ou une armure alimente également le registre `equipements_portes`. Aucun événement n’est inventé rétroactivement.   
- Les tests de sauvegarde et d’inventaire protègent la séparation Épinglé/Favori, la valeur par défaut Toujours demander, l’unicité du favori par famille de munitions et la compatibilité des anciennes sauvegardes.   
- Checkpoint important inchangé : **V3.51.15**. Schéma de sauvegarde inchangé : **26**.   

---   

## V3.51.16 — Stabilisation générale, préférences d'inventaire et garde-fous d'équilibrage   

- L'inventaire gagne trois préférences persistantes par objet/matériau : **Favori**, **Verrouillé** et, pour les matériaux, **Suivi**. Les favoris remontent en tête des listes sans modifier les piles physiques ; les verrous ajoutent une confirmation supplémentaire avant les usages volontaires sensibles ; les matériaux suivis affichent une bannière d'obtention fortement mise en évidence en majuscules.   
- Les préférences de sélection des **munitions** et des **supports magiques** deviennent persistantes. La valeur par défaut reste explicitement **Toujours demander**. Deux modes optionnels existent : **Favori si disponible** puis retour à la question, et **Automatique prudent**, qui préfère les munitions ordinaires ou les supports magiques moins rares plutôt que de sacrifier automatiquement les meilleures ressources.   
- Les préférences restent attachées à l'identité logique du matériau même lorsque sa quantité tombe à zéro. Une Poussière arcanique marquée favorite/suivie le reste donc lorsqu'une nouvelle pile est retrouvée plus tard. Les anciennes sauvegardes sans ces champs chargent proprement avec `Toujours demander` et sans favori/verrou/suivi inventé. Le schéma reste **26** car les nouvelles données sont additives et optionnelles.   
- Le verrou est relié aux usages volontaires importants : vente, équipement/consommation depuis l'inventaire, potions de combat, matériaux magiques, craft, remises de quête, échanges de Marques de défi, services runiques de l'enchanteur, preuves/justificatifs d'église, bons et tickets de service, trocs du marché noir, travaux municipaux, outils de coffre, matériaux tactiques et consommables/préparatifs d'expédition. Un refus conserve l'objet et évite qu'un autre objet soit consommé silencieusement à sa place.   
- Le verrou ne protège volontairement **pas** les marqueurs techniques internes ni les pertes forcées issues d'une vraie conséquence de monde/combat/mort : il empêche les erreurs du joueur, pas les conséquences du jeu.   
- L'affichage de temps des services locaux abandonne lui aussi le mot technique `segment` au profit de **moments de la journée**, dans la continuité de l'immersion des expéditions.   
- L'audit de stabilité a révélé un faux crash provoqué par un build incrémental mélangeant d'anciens objets avec la nouvelle disposition de `Player`. Un **clean rebuild complet** a confirmé qu'il ne s'agissait pas d'un bug gameplay ; la base recompilée depuis zéro est saine.   
- L'équilibrage n'est pas modifié arbitrairement sans playtest. En revanche, `MagicResourceBalanceTest` impose maintenant que **Sacrifice matériel** soit toujours plus cher en mana et plus explosif sur le tour, mais moins efficace en puissance/mana sur la durée que la canalisation normale, pour plusieurs raretés et toutes les qualités représentatives. La progression de qualité doit aussi améliorer ou conserver le profil sans inversion absurde.   
- `SYSTEMES_PREVUS.txt` est entièrement réécrit : les systèmes déjà terminés ne sont plus présentés comme des TODO. Le fichier distingue désormais histoire/monde, équilibrage à playtester, économie/traçabilité, combat/clan/compagnons, mort/héritage, serments, registres, GUI et les squelettes techniques qui ne sont **pas** des bugs actifs.   
- Le résumé **Voir tout** respecte désormais lui aussi l'ordre des favoris et affiche clairement `[FAVORI]`, `[VERROUILLÉ]` et `[SUIVI]`, au lieu de ne trier correctement que les sous-menus interactifs.   
- Le suivi d'un matériau ne produit plus de faux bandeau **OBJET SUIVI OBTENU** lorsqu'un minerai perd simplement un palier de qualité. Le chargement d'une sauvegarde neutralise aussi temporairement les anciennes préférences en mémoire avant de reconstruire l'inventaire, afin qu'un rechargement ne ressemble jamais à une nouvelle récolte.   
- Les échanges directs entre personnages respectent maintenant les verrous d'inventaire : transférer une arme, une armure, un consommable ou un matériau verrouillé demande une confirmation exceptionnelle supplémentaire.   
- Le nettoyage RP du temps est étendu à tous les écrans joueur concernés : voyages, auberge, église, réputation, guilde et services locaux parlent désormais de **moments de la journée**. Le mot « segments » reste seulement lorsqu'il décrit réellement l'anatomie d'une créature.   
- La régénération naturelle de mana en combat conserve son rythme initial au début de progression mais est désormais plafonnée à **5 mana par tour**. Sans ce plafond, les réserves niveau 255 pouvaient régénérer plus que le coût de plusieurs sorts réels et rendre la magie catalysée quasi gratuite à très haut niveau.   
- Le checkpoint important reste **V3.51.15** : c'est la vraie frontière de migration vers la magie matérielle moderne. V3.51.12 reste une passe de progression late-game et n'est pas requalifiée rétroactivement.   

---   

## V3.51.15 — Jalon important de transition vers la magie matérielle moderne   

- **V3.51.15 devient le nouveau point de sauvegarde important.** Toute sauvegarde dont la dernière adaptation est antérieure à ce jalon doit passer une fois par le rituel de transition et bénéficie d'abord du backup pré-mise-à-jour non écrasant déjà prévu par Dinotofu.   
- Le **schéma de sauvegarde reste 26** : aucune donnée persistante structurelle nouvelle ne justifie un faux bump. Les nouveaux systèmes réutilisent les IDs de matériaux, quantités, qualités, langues et compétences déjà sauvegardés.   
- La transition V3.51.15 est volontairement **non destructive** : aucune pile de matériau n'est convertie, aucune qualité n'est changée, aucun minerai n'est dissous, aucun grimoire/parchemin n'est supprimé. Les nouvelles règles prennent effet seulement lors des futures canalisations, études ou utilisations.   
- L'adaptation lourde distingue désormais les époques : les réparations historiques de kit/munitions/potion restent réservées aux sauvegardes réellement antérieures au checkpoint V3.50.33. Un personnage récent franchissant uniquement V3.51.15 ne reçoit donc aucun objet gratuit sans rapport avec la mise à jour matériaux.   
- L'écran du jalon explique explicitement la raison de la transition et rappelle avant validation que piles, quantités et qualités physiques seront conservées.   
- Le checkpoint clôt la séquence 3.51.08–3.51.14 : choix des supports, éléments, recettes, dégradation de qualité, Sacrifice matériel, prérequis des ouvrages, qualité exacte des ingrédients, progression tardive 100–255 et journal matériel synchronisé.   
- Les tests de checkpoint/migrations sont renforcés pour garantir le backup non écrasant, l'obligation de transition des versions antérieures à 3.51.15 et l'absence de réparation de kit parasite pour une sauvegarde récente.   

---   

## V3.51.14 — Journal des matériaux synchronisé avec la magie physique   

- Le Journal des matériaux explique désormais les usages magiques réellement observables au lieu de rester limité aux anciennes règles de craft. Pour chaque matériau magique découvert, il affiche le profil de canalisation lié à la qualité observée : coût relatif en mana, stabilité et bonus de puissance.   
- Les carburants minéraux sont identifiés comme tels et le journal rappelle qu'ils perdent un palier de qualité au lieu d'être consommés immédiatement pendant une canalisation normale.   
- Les affinités élémentaires des matériaux découverts sont visibles (Feu, Froid/glace, Terre, Eau, Vent, Foudre, Arcanique), ce qui permet de comprendre en jeu pourquoi certains supports remontent en priorité pour certains sorts.   
- Les recettes multi-matériaux ne sont pas révélées gratuitement par l'interface : une formule n'apparaît dans le journal que lorsque chacun de ses ingrédients a réellement été observé. Le journal indique alors les quantités et si chaque ingrédient est sacrifié ou simplement dégradé.   
- `MagicMaterialRules` expose maintenant l'identification des matériaux magiques connus et leurs affinités sous une forme centralisée afin que combat et journal utilisent les mêmes règles.   
- Les tests et garde-fous protègent la synchronisation du journal, les affinités et l'absence de révélation précoce des recettes. Schéma de sauvegarde et checkpoint important inchangés en préparation de la passe V3.51.15.   

---   

## V3.51.13 — Recettes par qualité et grimoires réellement difficiles   

- Les recettes multi-matériaux ne choisissent plus silencieusement la première qualité disponible. Quand un ingrédient existe sous plusieurs qualités, le joueur choisit désormais la pile exacte pour chaque composant avant la confirmation finale.   
- Le profil combiné de la recette (mana, stabilité, puissance) est recalculé à partir des qualités réellement sélectionnées. Une qualité exceptionnelle peut donc être volontairement investie dans une formule sans que le moteur la remplace par une pile moins bonne.   
- La consommation devient transactionnelle au niveau des piles exactes : toutes les quantités/qualités sont revérifiées avant la première mutation. Les ingrédients sacrifiés retirent la qualité confirmée et les carburants dégradables abaissent précisément la pile choisie.   
- Plusieurs grimoires spécialisés demandent maintenant une véritable lecture de niveau `conversation` : Nain 2 pour Aiguille de givre, Céleste 2 pour Suture de mana et Elfique noir 2 pour Ronces occultes. Les parchemins restent volontairement plus accessibles car leur tracé porte déjà une partie de la formule.   
- La Notation anormale n'est pas artificiellement poussée au niveau 3 : elle reste une écriture instable et fragmentaire, principalement filtrée par niveau, mana, connaissances et reconnaissance de motifs.   
- `MagicResourceBalanceTest`, `MagicStudyRulesTest` et `scripts/test_project.sh` protègent le choix des qualités de recette, la validation transactionnelle et les nouveaux seuils linguistiques. Schéma de sauvegarde et checkpoint important inchangés.   

---   

## V3.51.12 — Maîtrises tardives sur la courbe 1–255   

- La progression générique exploite désormais réellement l'après-niveau 100 sans ajouter une forêt de nouveaux boutons. Les techniques existantes gagnent des **formes de maîtrise** à des paliers tardifs, ce qui conserve leur identité tout en donnant une vraie raison de progresser.   
- Au **niveau 120**, les enchaînements universels obtiennent une quatrième impulsion : Quadruple coup, Quadruple tir, ou **Résonance canalisée** pour les profils magiques. La version magique utilise une base de 40 mana avant application de la voie matériaux/catalyseur/incantation, et les tirs continuent d'exiger une munition réelle par projectile.   
- Au **niveau 150**, le Balayage de ligne/tirs devient maîtrisé : pénalité offensive réduite et récupération ramenée de 4 à 3 tours.   
- Au **niveau 180**, Ralliement prudent devient **Ralliement vétéran** avec garde renforcée et courte régénération, sans devenir un soin complet.   
- Au **niveau 210**, Exécution opportuniste devient **Exécution maîtresse** : la fenêtre de cible blessée s'élargit de 40 % à 50 % des PV et le bonus conditionnel augmente légèrement.   
- Au **niveau 240**, Brise-garde atteint sa forme experte : affaiblissement/vulnérabilité durent un tour de plus et la récupération passe de 3 à 2 tours. Le niveau 255 reste le plafond sans pouvoir artificiel obligatoire.   
- `TechniqueProgressionRulesTest` et `scripts/test_project.sh` protègent les paliers 120/150/180/210/240. Aucun changement du schéma de sauvegarde ni du checkpoint important.   

---   

## V3.51.11 — Qualité exacte en canalisation et routine RP du prologue   

- La voie **minerais / matériaux** distingue maintenant la **qualité exacte** aussi pendant une canalisation normale, pas uniquement pour `Sacrifice matériel`. Si un même support existe en impur, normal, pur ou exceptionnel, chaque pile apparaît séparément avec son stock, son coût de mana, sa stabilité et son bonus de puissance. Le joueur choisit donc réellement quelle qualité consommer ou dégrader.   
- Les carburants minéraux utilisent `degradeMaterialQualityByIdAndQuality(...)` après confirmation : le moteur dégrade précisément l'unité choisie et ne peut plus remplacer silencieusement un support exceptionnel par la pile la moins chère. Les composants consommables utilisent de la même manière `removeMaterialQuantityByIdAndQuality(...)`.   
- Le chemin non interactif conservé pour les tests choisit explicitement la qualité qui aurait été utilisée selon les règles normales, afin de rester déterministe sans diverger du vrai moteur.   
- Le prologue gagne une scène **« Avant la montée »** entre l'introduction et le contrat : rations, sangles, gants, contrôle du matériel, convoyeurs croisés et plaisanteries sur l'auberge du soir renforcent l'impression d'une journée ordinaire vécue par une équipe expérimentée. Aucun texte ne révèle la nature mémorielle/technique de la scène avant le réveil.   
- `MagicResourceBalanceTest`, `StoryPrologueMemoryTest` et `scripts/test_project.sh` protègent le choix exact de qualité et la nouvelle routine pré-chasse. Aucun changement de schéma de sauvegarde ni du checkpoint important.   

---   

## V3.51.10 — Étude magique conditionnelle et sacrifice matériel par qualité   

- Les **livres et grimoires magiques** ne transforment plus automatiquement leur possession en maîtrise. Un nouveau module central `MagicStudyRules` exige selon la formule un niveau minimal, une réserve maximale de mana suffisante, un score de connaissances magiques et, pour certains textes, la compréhension d'une langue précise. Les exigences restent volontairement différentes d'un ouvrage à l'autre.   
- Les grimoires difficiles peuvent maintenant rester dans l'inventaire sans être perdus : une étude trop précoce explique exactement ce qui manque, puis le joueur peut **relire le même ouvrage plus tard** après avoir progressé. Les achats en boutique et les relectures d'inventaire passent par les mêmes règles centrales.   
- Les langues deviennent de vrais prérequis d'étude pour certaines traditions : par exemple Nain pour l'Aiguille de givre, Céleste pour la Suture de mana, Elfique noir pour les Ronces occultes et Notation anormale pour la Faille de résistance. Ces seuils s'appuient sur le système linguistique déjà sauvegardé, sans nouveau champ de sauvegarde.   
- Les **parchemins** appliquent eux aussi des barrières adaptées de niveau, réserve maximale de mana, connaissances et parfois langue. Ils restent plus accessibles qu'un apprentissage permanent parce que leur tracé porte une partie de la formule, mais un personnage incapable de stabiliser le parchemin ne le consomme plus inutilement.   
- Les **connaissances magiques** deviennent une valeur lisible calculée à partir d'Intelligence/Sagesse, de la spécialisation magique, des formules déjà étudiées et de quelques maîtrises pertinentes. Les Stats affichent maintenant le score et son niveau (`rudimentaire`, `notions solides`, `confirmé`, `avancé`, `érudit`). Les grimoires inspectés affichent leurs prérequis avec `[OK]` / `[À ATTEINDRE]` avant même une tentative d'étude.   
- La qualité et la rareté des supports de la voie **minerais / matériaux** influencent désormais réellement trois axes : **coût de mana**, **stabilité** et **puissance canalisée**. Un support rare et de haute qualité est plus efficace et plus stable ; les bonus restent plafonnés afin de conserver un terrain de playtest raisonnable.   
- La nouvelle technique contextuelle **Sacrifice matériel** se débloque au **niveau 60** sur la courbe 1–255. Elle est volontairement moins rentable qu'une canalisation matérielle normale : coût de mana nettement supérieur et destruction immédiate de l'unité physique, mais toute sa rareté et sa qualité restante sont converties en une poussée de puissance sur le même tour.   
- Sacrifice matériel permet maintenant de choisir la **qualité exacte** de l'unité détruite. Si un même minerai existe en impur, normal et exceptionnel, chaque pile apparaît séparément avec stock, mana, stabilité et puissance prévus. Le moteur retire précisément la qualité confirmée ; si elle n'existe plus au moment d'engager la ressource, le mana est rendu et le sacrifice est annulé.   
- Les recettes multi-matériaux conservent leur fonctionnement transactionnel et profitent d'un profil combiné qualité/rareté. La canalisation normale continue de dégrader progressivement les carburants minéraux, tandis que Sacrifice matériel détruit immédiatement une unité choisie : le burst est donc puissant mais volontairement moins économique sur plusieurs tours.   
- Un audit annexe a corrigé trois synergies tactiques (`Voile élémentaire`, `Marque élémentaire`, `Suture de mana`) qui vérifiaient par erreur les formules apprises comme compétences passives. Elles reconnaissent maintenant correctement les compétences actives acquises par grimoire.   
- `MagicStudyRulesTest`, `MagicResourceBalanceTest`, `TechniqueProgressionRulesTest` et `scripts/test_project.sh` protègent les nouveaux prérequis, la relecture, les langues, le choix exact de qualité et les synergies tactiques. Aucun changement du schéma de sauvegarde, des migrations ou du checkpoint important.   

---   

## V3.51.09 — Écho canalisé interactif et recettes incomplètes visibles   

- Le vrai chemin joueur d'**Écho canalisé** utilise désormais le même sélecteur de support physique que les autres sorts de la voie minerais / matériaux. Le joueur voit donc le support, sa quantité, sa qualité et son effet avant confirmation au lieu de laisser Écho choisir silencieusement le premier composant disponible. La variante non interactive de `prepareMagicTechnique` reste disponible pour les tests et les chemins techniques déterministes.   
- L'affinité matérielle d'Écho est déduite de la spécialisation quand elle est claire : Pyromancien → feu, Cryomancien → froid, Électromancien → foudre, Hydromancien → eau, Géomancien → terre, Aéromancien/vent → vent ; les autres profils retombent proprement sur arcanique.   
- Une **recette connue mais incomplète** ne disparaît plus du menu. Elle reste visible sous `Recette dédiée (incomplète)`, verrouillée, avec la liste exacte des ingrédients manquants et leurs quantités. Le joueur peut ainsi apprendre/comprendre progressivement une formule sans pouvoir l'exécuter avant de réunir tous les composants.   
- `MagicMaterialRules::missingSpellMaterialIngredients` centralise le calcul des manques. `MagicResourceBalanceTest` vérifie l'aperçu `[MANQUANT]` et les quantités, tandis que `scripts/test_project.sh` protège le chemin interactif d'Écho et la visibilité des recettes incomplètes.   
- Aucun changement des coûts de mana, des ratios des trois voies, de la puissance, de la qualité des matériaux, du schéma de sauvegarde, des migrations ou du checkpoint important.   

---   

## V3.51.08 — Choix des matériaux magiques, recettes confirmées et grimoires recalés sur 255 niveaux   

- La voie **minerais / matériaux** n'utilise plus silencieusement le premier composant disponible pendant les vrais sorts de combat. Le joueur choisit désormais son support dans un écran dédié avant la canalisation. Les supports accordés à l'élément du sort sont proposés en priorité, tout en conservant les composants génériques comme solution de repli.   
- Le menu de préparation affiche avant toute dépense le **nom réel**, la **quantité disponible**, la **prochaine qualité utilisée** et l'effet prévu sur la ressource : consommation d'une unité ou perte d'un palier de qualité jusqu'à dissolution pour un carburant minéral. Une seconde confirmation explicite est exigée avant d'engager le support.   
- Le joueur peut volontairement **préserver ses matériaux** et forcer l'incantation pure de secours. Cette décision conserve le coût/rythme/risque du fallback déjà défini ; aucun nouveau bonus arbitraire de dégâts n'est ajouté selon la qualité dans cette passe.   
- Les familles élémentaires sont enrichies et réellement reliées aux sorts : feu, froid/glace, terre, eau, vent, **foudre** et arcanique disposent de supports physiques identifiés. Les sorts à choix d'élément (`Marque élémentaire`, `Surcharge contrôlée`, `Faille de résistance`) transmettent maintenant l'élément choisi à la préparation matérielle ; `Aiguille de givre`, `Ronces occultes`, `Suture de mana`, `Voile élémentaire` et `Entrave arcanique` utilisent leur famille correspondante.   
- Les **recettes multi-matériaux** cessent d'être de simples données dormantes. Lorsqu'une recette complète existe, elle apparaît comme option séparée, détaille les ingrédients sacrifiés et ceux seulement dégradés, puis demande confirmation avant exécution. La consommation n'arrive qu'après validation du mana ; si la recette devenait incomplète entre la sélection et la canalisation, le mana engagé est rendu et le sort ne part pas.   
- Les carburants minéraux sont généralisés au-delà du seul fer runique : clou de fer froid, coeurs runiques rares, noyau instable et noyau de fusion élémentaire peuvent eux aussi perdre progressivement leur qualité au lieu d'être détruits immédiatement.   
- L'état magique du personnage compte désormais aussi les supports élémentaires valides, pas uniquement les quatre composants génériques historiques. L'affichage des stocks précise la qualité du prochain exemplaire qui serait utilisé.   
- Un second audit de la progression **1–255** a trouvé un contournement par les grimoires : plusieurs formules avancées pouvaient encore être étudiées dès les niveaux 3 à 9. Les seuils d'étude sont maintenant centralisés dans `TechniqueProgressionRules::magicStudyUnlockLevel` : Marque 8, Entrave 15, Aiguille de givre 20, Voile élémentaire 25, Ronces occultes 25, Suture de mana 30 et Faille de résistance 55. Les livres permettent donc toujours d'apprendre avant le déblocage naturel, sans annuler toute la courbe de progression.   
- Les anciennes sauvegardes qui possèdent déjà un sort appris trop tôt restent compatibles, mais le combat vérifie désormais aussi le niveau d'étude minimum avant d'autoriser la formule. Les boutiques utilisent les mêmes règles centrales et n'ont plus leurs anciens nombres 3/4/5/6/7/9 codés en dur.   
- `MagicResourceBalanceTest`, `TechniqueProgressionRulesTest` et `scripts/test_project.sh` protègent le choix de support, les familles élémentaires, la qualité affichée, l'exécution des recettes, la dégradation des minerais et les minimums de niveau des grimoires.   
- Aucun changement du schéma de sauvegarde, des migrations, du checkpoint important ni des ratios de balance mana. Les valeurs 45 % / 75 % / 120–125 %, Écho canalisé 18/28 et l'épuisement à 0 mana restent des sujets de **playtest réel**.   

---   

## V3.51.07 — Temps d'expédition RP et jour actuel du personnage   

- Les écrans d'**expédition/exploration** n'exposent plus le terme technique « segment ». Le moteur conserve ses unités internes, mais l'interface parle désormais de **moments de la journée** et rappelle explicitement la progression RP : **Matin (1/5) · Midi (2/5) · Après-midi (3/5) · Soir (4/5) · Nuit (5/5)**.   
- `Player::formatWorldDayPartLine()` adopte partout la forme lisible **Matin (1/5)** au lieu de `Matin 1/5`. Les transitions de temps utilisent le même format avant/après.   
- Les menus d'expédition affichent le **moment actuel**, les repères d'une journée, les durées, l'autonomie, les trajets et les effets d'exposition sans réintroduire le vocabulaire « segment ».   
- Le **jour actuel du personnage** est maintenant visible dans le résumé des Stats, les Stats compactes du menu Personnage, l'affichage long et la sélection des personnages sauvegardés. La sélection lit directement `worldDaysElapsed` et `worldDayProgressUnits` déjà présents dans les sauvegardes ; aucun nouveau champ de sauvegarde n'est créé.   
- Les affichages de journal/mémoire et d'infirmerie convertissent correctement les compteurs internes 0-based en jours RP 1-based. Un personnage au début de sa partie voit donc **Jour 1**, jamais « jour 0 », sans modifier les délais ni les comparaisons internes.   
- Un nouveau `WorldTimePresentationTest` protège les cinq moments, le passage Jour 1 → Jour 2 et le format des transitions. `scripts/test_project.sh` interdit aussi le retour du mot « segment » dans `QuestExplorationMenu.cpp` et protège l'affichage du jour courant dans les fiches personnage.   
- Aucun changement du schéma de sauvegarde, des migrations, du checkpoint important, des valeurs d'équilibrage ni des fondations de magie matérielle. Les matériaux avancés restent volontairement en attente comme décidé en V3.51.06.   

---   

## V3.51.06 — Stabilisation hors matériaux avancés : progression tactique, auberge et salves réelles   

- Le développement des **recettes et choix avancés de matériaux magiques est volontairement mis de côté** pour cette passe à la demande utilisateur. Les fondations ajoutées en V3.51.05 restent intactes ; aucun sacrifice automatique de recette supplémentaire n'est activé.   
- L'audit de la progression **1–255** a trouvé un vrai contournement dans `PlayerWaveTacticalActionMenu` : de nombreuses actions pouvaient être utilisées très tôt uniquement grâce à leur affinité/maîtrise. Les **51 actions tactiques** ont maintenant un minimum de niveau centralisé via `TechniqueProgressionRules::tacticalActionUnlockLevel`, en plus de leurs anciennes conditions contextuelles. Les gestes simples restent précoces, tandis que les contrôles avancés sont espacés, par exemple Brise-garde 5, Pas de l'ombre 10, Coup en arc 24, Percer le rempart 35, Rupture de formation 40, Lien d'invocation 45, Danse de lame 50 et Chaîne d'états 55. Une garde côté exécution empêche également un appel détourné de contourner le menu.   
- L'**auberge** n'a plus deux chemins concurrents : le menu monde délègue désormais repas et lit commun au système central `ShopCityServiceMenu::openLodgingServiceMenu`, qui gère correctement bons, abonnements et paiement direct exact. La chambre sûre premium reste une dépense séparée. Les repas et nuits centralisés alimentent aussi de nouveau les événements canoniques de fatigue/repos.   
- Les techniques à distance multi-coups vérifient désormais les **munitions compatibles réelles** avant et pendant la salve. Si le carquois se vide en cours de Double/Triple tir, balayage ou ricochet, la salve s'arrête proprement au lieu de transformer les projectiles restants en coups de crosse. Le coup de secours sans munition reste disponible comme attaque volontaire séparée et conserve sa forte usure.   
- `StoryCampaign.cpp` est nettoyé de ses deux warnings connus : le paramètre inutilisé de `buildLongIntroductionLines` n'est plus nommé dans la définition, et `specialStoryHookLines`, confirmé comme helper mort et jamais appelé, est retiré. Les réactions spéciales réellement actives du prologue restent dans leurs chemins dédiés.   
- L'audit du **prologue** confirme l'ordre voulu : journée/mission normale, combat, après-coup distinct selon victoire/retraite/défaite, puis seulement arrivée de la Brume et réveil. Aucun changement narratif inutile n'est appliqué à cette séquence.   
- L'audit de la **monnaie physique** confirme que les récapitulatifs combat/exploration utilisent encore les piles réelles et leur delta ; les usages restants de `Money::formatCopper` correspondent à des coûts/valeurs comptables et non à une normalisation silencieuse de la bourse.   
- `scripts/test_project.sh` protège maintenant la progression tactique, l'unification de l'auberge, les événements repas/repos, l'arrêt des salves sans munitions et l'absence du helper narratif mort. Un **clean build C++23 complet** a été réalisé sans warning, puis toute la suite de tests a été validée avant versionnement.   
- Aucun changement du schéma de sauvegarde, des migrations, du checkpoint important ni des valeurs d'équilibrage mana n'est effectué dans cette passe. Les questions restantes sur mana, dégâts, durabilité, compréhension des bons et immersion sont désormais des points de **playtest réel**, pas des défauts de code connus.   

---   

## V3.51.05 — Magie matérielle évolutive et progression des techniques sur 255 niveaux   

- La progression des techniques du joueur est désormais pensée sur le vrai plafond de **255 niveaux** au lieu d'être presque entièrement concentrée dans les niveaux 2 à 12. Les gestes simples restent accessibles tôt, mais les techniques plus complexes sont étalées jusqu'aux niveaux **40, 50, 65, 75, 80 et 100** selon leur puissance et leur spécialisation.   
- Les enchaînements génériques sont recalés : Double coup/tir arrive au **niveau 8**, Écho canalisé au **niveau 15**, et les variantes Triple au **niveau 35**. Les techniques génériques suivent maintenant une table centrale `TechniqueProgressionRules`, par exemple Ralliement prudent 3, Brise-garde 5, Coup d'arrêt 12, Signal de focus 18, Balayage 24, Rupture de ligne 40, Suture de fortune 50, Ricochets 65, Exécution opportuniste 80 et Mélange instable 100.   
- Les techniques de classe sont elles aussi espacées : les premières signatures arrivent typiquement vers **8–10**, tandis que les techniques avancées montent jusqu'à **75**. Les compétences identitaires de classe ne sont plus toutes données au niveau 5 ; Assassin et Mage commencent notamment leur signature au niveau 10.   
- L'apprentissage naturel par pratique d'arme conserve ses exigences de combats réussis mais reçoit aussi un **niveau minimum**. Le stand d'entraînement respecte les mêmes minima et ne peut donc plus acheter une technique avant que le personnage ait atteint le palier prévu.   
- La voie **minerais / matériaux** gagne une première vraie notion de **carburant minéral**. L'Éclat de fer runique n'est plus forcément détruit en une utilisation : lorsqu'il alimente une canalisation, sa qualité baisse d'un palier ; arrivé à la qualité la plus faible, il se dissout. L'inventaire sait maintenant dégrader proprement une unité de matériau tout en conservant les piles séparées par qualité.   
- `MagicMaterialRules` connaît désormais des affinités de matériaux **élémentaires** (feu, froid/glace, terre, eau, vent et arcanique) afin que les futurs sorts puissent demander un support cohérent avec leur élément au lieu d'utiliser un composant universel.   
- Une structure de **recettes magiques multi-matériaux** est ajoutée pour les sorts qui pourront sacrifier plusieurs objets/composants, avec distinction entre ingrédients consommés et minerais seulement dégradés. Des recettes de données existent déjà pour *Aiguille de givre* et *Surcharge contrôlée*. Elles ne sont volontairement **pas encore exécutées automatiquement** : aucun objet précieux ne doit être sacrifié sans une vraie interface de choix du joueur.   
- `MagicResourceBalanceTest` couvre les affinités élémentaires, les recettes, la présence des ingrédients et la dégradation/dissolution du carburant minéral. Un nouveau `TechniqueProgressionRulesTest` protège le plafond 255 et les principaux paliers.   
- Le processus de **backup essentiel** est renforcé après détection d'une archive qui avait exclu par erreur les modules de code `include/save` et `src/save`. Un script dédié exclut uniquement les dossiers de données privées `saves/`, `accounts/`, etc. et les sorties générées ; les modules de sauvegarde du code source sont explicitement protégés par la suite de tests.   
- Aucun changement du schéma de sauvegarde, des migrations, du checkpoint important ou des valeurs de balance mana n'est effectué dans cette passe. Les recettes avancées et le choix du composant restent volontairement à terminer après validation de l'UX.   

---   

## V3.51.04 — Ressources d'exploration lisibles et règles centrales des composants magiques   

- Les quatre composants utilisables par la voie **minerais / matériaux** sont désormais définis dans un module central `MagicMaterialRules` au lieu d'être recopiés séparément dans le combat et dans l'affichage des Stats. La priorité existante reste inchangée : **Poussière arcanique → Éclat de fer runique → Sel lunaire → Résine d'écho**.   
- `CombatActions` demande maintenant au module central quel composant réellement disponible doit être préparé ; `Player::getMagicSupportStatusLine()` utilise la même source pour compter et détailler le stock. Une future modification de la liste ne peut donc plus désynchroniser le combat et l'interface.   
- L'audit de disponibilité n'a révélé aucun blocage nécessitant un buff artificiel : la Poussière arcanique existe très tôt en exploration, la Résine d'écho et le Sel lunaire arrivent dans des biomes intermédiaires, et l'Éclat de fer runique reste volontairement plus « atelier », avec notamment une disponibilité garantie chez le forgeron en plus des rotations de vendeurs/enchanteurs. Aucun taux de drop ni prix n'est modifié dans cette passe.   
- Les cartes d'**exploration** n'affichent plus les identifiants internes des ressources (`worn_leather_piece`, `arcane_dust`, etc.) dans le champ Commun/Rare. Elles résolvent maintenant les vrais noms du catalogue, par exemple **Morceau de cuir abîmé** ou **Poussière arcanique**.   
- `MagicResourceBalanceTest` protège la liste centralisée, son ordre de priorité, les noms français et le comptage/formattage des stocks. `scripts/test_project.sh` protège aussi l'utilisation de `MagicMaterialRules` par le combat/Stats et empêche le retour de l'ancienne ligne d'exploration exposant les IDs techniques.   
- Aucun coût de mana, rythme, stabilité, économie, schéma de sauvegarde ou règle de migration n'est modifié. Cette passe est une stabilisation de données/UX avant les vrais playtests d'équilibrage.   

---   

## V3.51.03 — Lisibilité des trois voies magiques et support réel   

- Les messages de canalisation de la voie **minerais / matériaux** n'affichent plus les identifiants techniques (`arcane_dust`, `runic_iron_shard`, etc.) : le joueur voit maintenant le vrai nom français du composant préparé, par exemple **Poussière arcanique**.   
- `Player` expose désormais un résumé lisible et centralisé du **support magique réel**. La voie matériaux indique combien de composants compatibles sont disponibles et lesquels ; la voie catalyseur indique le bâton équipé, sa durabilité et s'il est fonctionnel ou brisé ; l'incantation pure rappelle qu'aucun support n'est obligatoire.   
- Le rythme magique est centralisé via `getMagicRecoveryLabel()` (`rapide`, `standard`, `lent`) afin d'éviter que plusieurs menus reconstruisent chacun leur propre libellé.   
- Les **Stats compactes**, le résumé détaillé des statistiques et l'écran de combat exposent maintenant le support magique réel et le fallback éventuel. Le joueur peut donc voir avant d'agir pourquoi un Mage est rapide avec son bâton, lent sans catalyseur, ou pourquoi un Runiste n'a plus de composant adapté.   
- `MagicResourceBalanceTest` protège les libellés de rythme et les états de support : absence de composant, composant disponible, catalyseur absent, catalyseur fonctionnel, catalyseur brisé et incantation pure. `scripts/test_project.sh` protège aussi l'absence de fuite des IDs internes dans le message de canalisation.   
- Aucun coût de mana, pourcentage de stabilité, rythme mécanique, schéma de sauvegarde ou règle de migration n'est modifié par cette passe : elle rend les compromis déjà présents **visibles** avant le prochain vrai playtest d'équilibrage.   
- Build C++23 complet et `scripts/test_project.sh` validés après versionnement : **Tous les tests Dinotofu sont passés.** Binaire **V3.51.03**, schéma de sauvegarde **26 inchangé**, checkpoint important **V3.50.33 inchangé**.   

---   

## V3.51.02 — Canalisation magique transactionnelle et fallback catalyseur cassé   

- La préparation magique ne consomme plus un **composant** ni un point de **durabilité de catalyseur** avant d'avoir vérifié que la réserve de mana permet réellement de démarrer le sort. Un échec pour mana insuffisant laisse donc l'inventaire et le bâton intacts.   
- Les voies minerais/matériaux continuent de réserver leur composant avant la tentative, mais celui-ci n'est retiré qu'après la dépense de mana réussie ; une canalisation réellement engagée le consomme ensuite même si le jet de stabilité échoue.   
- Les catalyseurs fonctionnels suivent la même logique : l'usure de 1 point arrive seulement une fois la canalisation engagée. Les équipements indestructibles et le cheat correspondant restent respectés.   
- Un **catalyseur brisé** ne conserve plus par erreur le coût réduit à 75 % : il bascule réellement en incantation de secours à **120 % du coût de base**, avec stabilité dégradée et le rythme lent déjà prévu par `getMagicRecoveryAdjustment()`.   
- `MagicResourceBalanceTest` couvre maintenant les cas de mana insuffisant sans perte de ressource, le fallback du catalyseur brisé et la consommation/usure lorsqu'une tentative démarre réellement. `scripts/test_project.sh` protège également les garde-fous de consommation différée.   
- Build C++23 complet et `scripts/test_project.sh` validés avant versionnement : **Tous les tests Dinotofu sont passés.** Schéma de sauvegarde **26 inchangé**, checkpoint important **V3.50.33 inchangé**.   

---   

## V3.51.01 — Stabilisation monnaie physique et voies de mana   

- Le récapitulatif d'exploration est réellement aligné sur la bourse physique : il affiche désormais les **piles exactes avant/après**, leur variation par dénomination (`+1 PE | -4 PC`, etc.) et le total PC uniquement comme valeur comptable. Le moteur ne reconstruit plus une « bourse idéale » visuelle susceptible de faire croire à une conversion automatique.   
- `Money::formatCoinStackDelta(...)` centralise désormais le calcul des variations de piles physiques et est partagé par les récapitulatifs concernés afin d'éviter des représentations monétaires divergentes.   
- L'économie des **bons prépayés d'auberge** est protégée par test : les carnets de cinq utilisations restent légèrement moins chers que cinq services directs, sans devenir une source d'économie disproportionnée.   
- Les classes réellement magiques qui pouvaient encore tomber sur une simple « réserve mineure » sont rattachées aux voies prévues : notamment Druide, Shaman/Chaman, Prêtre, Paladin, Templier, Nécromancien, Pactisant, Électromancien et Miragien.   
- **Écho canalisé** (Double/Triple variante magique) n'utilise plus un retrait forfaitaire de mana indépendant du style de magie. L'enchaînement passe par la vraie préparation magique : composants pour les voies matérielles, durabilité/stabilité du catalyseur, coût et risque de l'incantation pure, ainsi que l'épuisement éventuel à 0 mana.   
- Les trois voies ont maintenant un **rythme de récupération mécanique** en plus de leurs différences de coût : un catalyseur fonctionnel accélère la récupération, l'incantation pure la ralentit, tandis que minerais/matériaux gardent un rythme standard compensé par leur forte économie de mana. Un catalyseur absent ou brisé force une récupération plus lente.   
- La vue Stats indique ce rythme magique (`rapide`, `standard` ou `lent`) à côté de la réserve de mana pour rendre le compromis visible sans exposer de formule interne.   
- `MagicResourceBalanceTest` protège les voies, réserves, progression de mana et rythmes de récupération ; `EconomyScaleTest` protège également la variation physique des pièces et la rentabilité mesurée des bons.   
- Build C++23 complet et `scripts/test_project.sh` validés avant versionnement : **Tous les tests Dinotofu sont passés.** Schéma de sauvegarde **26 inchangé**, checkpoint important **V3.50.33 inchangé**.   

---   

## V3.51.00 — Immersion RP, mana, progression des techniques et UX vécue   

- Les récapitulatifs d'exploration et de combat n'affichent plus une « bourse idéale » reconstruite depuis la valeur totale. Ils capturent désormais les **cinq piles physiques réelles** avant/après et affichent leur variation pièce par pièce ; le total en PC reste disponible uniquement comme comptabilité exacte secondaire. Aucune conversion automatique de monnaie n'est introduite.   
- L'auberge distingue clairement les **services immédiats** des **bons prépayés**. Les anciens articles ambigus deviennent des carnets de cinq bons de repas ou de lit, classés comme bons prépayés, légèrement avantageux par rapport à cinq achats directs et explicitement décrits comme non consommés au moment de l'achat. Le menu « Utiliser l'auberge » affiche les bons en possession et débite les services directs en **PC exacts**.   
- Le temps de jeu devient réellement visible en Terminal : la sélection de personnage affiche le temps réel joué et la valeur totale en PC. Le compte conserve en plus un **temps de jeu historique propre**, cumulant les sessions des personnages y compris après leur mort ou suppression.   
- Le menu Personnage gagne une vue compacte **Stats** : identité, race, classe, niveau, bourse physique, valeur exacte en PC, temps réel joué, PV, mana, dégâts, critique et protection équipée.   
- L'attaque de secours d'un arc/arbalète sans munition reste moins efficace et use désormais fortement l'arme : **5 points de durabilité** au lieu d'une usure normale de 1. L'audit n'applique pas de nerf global aveugle aux dégâts distance : leur avantage offensif reste compensé par les munitions et les profils de PV plus faibles, à réévaluer après playtests.   
- Les douze techniques actives génériques ne sont plus toutes accessibles niveau 1. **Brise-garde** et **Ralliement prudent** arrivent tôt, puis les enchaînements, arrêts, balayages, ruptures et techniques avancées se débloquent progressivement jusqu'aux niveaux supérieurs. Les noms s'adaptent à l'arme : **Double/Triple coup** en mêlée, **Double/Triple tir** à distance, et une variante magique dédiée. Chaque tir continue de consommer sa propre munition.   
- Dinotofu possède désormais une vraie **jauge de mana persistante** (`mana actuel / mana maximum`) sauvegardée avec le personnage, visible en combat et dans les Stats. Le mana se régénère lentement au fil des tours, se restaure avec le repos et peut être récupéré par une nouvelle Potion de mana. Les anciennes sauvegardes sans ces champs chargent avec une réserve pleine.   
- Tomber exactement à **0 mana** peut provoquer un épuisement magique : environ 75 % de risque de perdre connaissance, généralement un tour et parfois deux. Le malaise n'est volontairement pas garanti à 100 %, afin de conserver l'incertitude et les moments de dépassement de soi.   
- Trois voies de magie sont distinguées : **minerais/matériaux** (petite réserve, coût fortement réduit mais composants consommés), **baguette/catalyseur** (coût réduit, meilleure stabilité et durabilité du support), et **incantation pure** (aucun support obligatoire, plus lente/gourmande et plus exposée, mais légèrement plus puissante). Un fallback d'incantation pure existe lorsqu'un composant ou catalyseur manque.   
- Le prologue pré-Brume est réécrit en **RP immersif**. Il commence comme une journée ordinaire entre aventuriers expérimentés : équipement, petites disputes familières, contrat de guilde et départ en mission. Le jeu ne dit jamais au joueur qu'il vit un souvenir, qu'il utilise un build temporaire ou que le combat est hors progression.   
- Les trous de mémoire sont présents sans explication : noms variables (`Sca---`, `--arle--`, `S--rlett`, `Lor---`, etc.), portions de contrat, route, prime ou phrases partiellement effacées. Ils représentent les résidus incomplets dont le personnage dispose aujourd'hui, sans révéler leur nature avant le réveil. La **Brume/fumée blanche n'arrive chronologiquement qu'après** le combat et son après-coup (relâchement/festin ou retraite/défaite), puis l'effacement s'aggrave.   
- Après le réveil, la compréhension reste entièrement diégétique : le personnage peut seulement soupçonner que ce qu'il vient de vivre était un souvenir ancien, sans savoir précisément quand ni avec qui. Les formulations hors-RP du type « souvenir jouable », « build temporaire », « aucun XP/butin » ont été retirées des écrans joueur.   
- Le système de mise à jour, les migrations, le backup pré-migration non écrasant et le checkpoint important **V3.50.33** ont été audités et validés par la suite de tests. Ils n'ont pas été modifiés.   
- Build C++23 complet et `scripts/test_project.sh` validés avant versionnement : **Tous les tests Dinotofu sont passés.** Schéma de sauvegarde **26 inchangé**, checkpoint important **V3.50.33**.   

---   

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
- Lanceurs Windows/Linux : le **Terminal devient le mode par défaut   

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