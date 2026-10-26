# Dinotofu changelog   

## V3.57.08 — Pre-push pass: typed quest follow-ups, city services and Git safeguards   

- This pass is designed as the final consolidation milestone before the first Git push: finish readable, testable systems instead of opening a large new subsystem immediately before publishing the repository.   
- `QuestSecondPhaseSystem` now truly distinguishes **service/delivery, rescue, investigation/verification, gathering/material, hunt/combat/bestiary and exploration** follow-ups. Each family has several ordinary continuations instead of falling back too often to the same generic surprise.   
- Some follow-ups are explicitly optional or mandatory according to their logic: for example a final favor after a rescue may be declined, while a wrong counter, an item sample requiring verification or essential corroboration may remain contractual. V3.57.07 reward rules remain unchanged: mandatory/inter-city is paid, optional local follow-ups may be non-monetary.   
- The phase-2 journal now exposes **Follow-up type**, **Estimated difficulty** and **Known risks**. Risks only describe what the character can reasonably anticipate and explicitly preserve the rule that rumor/testimony is not proof; genuinely unknown dangers are not spoiled.   
- `CityEconomyDynamicsSystem` now exposes a shared local quality score for **forge/repairs, alchemy, enchanting and transport**, combining structural city specialization, current wealth and economic conditions. The city economy report displays those differences qualitatively and numerically; detailed integration into every artisan service remains progressive work after the first push.   
- New `make pre-push` control: clean tree, source audit, JSON validation, build/tests and Git checks when a repository is present. An essential backup without branding images can be validated as **source ready to push**, while full releases/installers still require official branding assets.   
- New minimal `.github/workflows/ci.yml`: source audit and test suite on push, pull request and manual dispatch. It does not replace the future release workflow.   
- `scripts/release_push.sh` now supports `current` / `--no-bump` to push an already validated version without silently creating another patch version. `PREMIER_PUSH_GIT.md` documents the initial push and this mode.   
- `.gitignore` explicitly excludes local chat handoff files and essential backup archives to prevent accidental publication of handoff artifacts.   
- `QuestSecondPhaseSystemTest` and `CityEconomyDynamicsSystemTest` cover the new families, journal metadata and city service-quality differences across profiles/economic conditions.   
- First-repository simulation in a clean copy: **1003 files** staged, no forbidden private/generated content, clean `git diff --cached --check` outside the Markdown convention, `.gitignore` rules confirmed and a simulated initial commit completed successfully. The check also cleaned 74 historical extra EOF blank lines and 2 PowerShell trailing-whitespace cases; all 76 fixes were proven whitespace-only.   
- Save schema remains **26**. Important checkpoint remains **V3.57.00**. **Titles: no additions, modifications or rebindings**; catalog remains **V3.57.02 / 419 titles**.   
- Final validation: full **V3.57.08 build OK**; global runner **119 launched / 119 awaited / 119 PASS** with static/versioning checks PASS. The `make pre-push` path validated cleanup, source audit and JSON; its full clean recompilation exceeds this environment execution window, but the same build/test steps were validated separately on the final state. A final clean-tree audit and backup re-extraction are required before handoff.   

---   

## V3.57.07 — Stable anti-abuse, community collection and non-monetary optional follow-ups   

- This pass deliberately recenters development on general **B — quests / contracts / travel** and **C — cities / economy / living world** systems: no new bounty or criminal-network feature is added here, keeping D as a subsystem rather than Dinotofu's main mode.   
- New `LodgingRestSystem` for free stable-floor rest: up to **3 free nights within a rolling 5-day window** are tolerated. A further attempt beyond that tolerance causes eviction and a **3-day local ban**. The ban does not affect other cities or paid/private lodging.   
- The lodging menu now exposes the stable's real state, recent free uses and any remaining ban. A refused attempt does not advance time or heal the character.   
- The active regular-event schedule removes the **Garden Festival** and replaces it with a **Community Collection** focused on useful donations, materials, consumables, plants and local services. The Harvest Festival remains the main seasonal harvest event.   
- Community Collection economy/stock effects reuse existing dynamic systems and remain bounded. Participation primarily rewards local recognition and useful service instead of direct cash.   
- **Optional local phase-2 follow-ups** may now primarily reward **local reputation**, an **NPC relationship**, **cross-checked information** or a **future opportunity** rather than always adding extra money. The original contract keeps its normal reward.   
- **Mandatory** phase-2 steps and **inter-city** continuations remain monetarily/logistically compensated, so the feature does not recreate disguised free transport. Non-monetary rewards are recorded idempotently and cannot be claimed twice.   
- The new phase-2 reward fields are optional and backward-compatible; save schema remains **26**. New `LodgingRestSystemTest` plus extensions to `QuestSecondPhaseSystemTest`, `SaveRoundTripTest` and `CityEconomyDynamicsSystemTest`.   
- Final **V3.57.07 build: OK**. Global runner: **119 tests launched / 119 awaited / 119 PASS**; static and versioning checks: PASS.   
- **Titles: no additions, modifications or rebindings in this version.** The catalog remains **V3.57.02**, with **419 titles**. The important checkpoint remains **V3.57.00**.   

---   

## V3.57.06 — Progressive recovery after criminal disruptions   

- Local criminal pressure is no longer frozen: several relatively quiet days progressively reduce the observable pressure on known contacts. Historical rivalries recover much more slowly and do not disappear automatically.   
- New `CrimeLocalStabilizationSystem`: completing and turning in a real quest created from **Local follow-ups** applies a small, bounded reduction to pressure among known contacts in the city and records a persistent local stabilization event.   
- The three follow-up types have different stabilization strengths: checking merchant records, watching relays and cross-checking witnesses do not carry exactly the same weight, but none can instantly reset a city.   
- `CrimeLocalImpactSystem` now separates recent disruption weight from recent stabilization weight. Economic effects therefore fade naturally over time and recover faster when the player actually helps document and stabilize the situation.   
- `CrimeNetworkCompetitionSystem` now accounts for pressure cooling after several quiet days. Rivalry between networks remains a longer-term memory unless later events justify reducing it.   
- Turning in a local consequence quest now calls stabilization through the normal quest turn-in flow; the same follow-up cannot be reused to farm artificial recovery.   
- Added `CrimeLocalStabilizationSystemTest` to the global runner. The runner now has **118 tests launched / 118 awaited / 118 PASS**; static and versioning checks: PASS.   
- **Titles: no additions, modifications or trigger rewiring in this version.** The catalog remains **V3.57.02**, with **419 titles**. Save schema remains **26** and the important checkpoint remains **V3.57.00**.   
- Final **V3.57.06 build: OK**.   

---   

## V3.57.05 — Local crime consequences and small guild follow-ups   

- First deliberate bridge between **D — bounties / crime / networks / rumors**, **C — cities / economy / living world** and **B — quests / contracts / travel**: a documented criminal disruption can now leave a temporary local effect instead of disappearing immediately after an arrest or bounty resolution.   
- New `CrimeLocalImpactSystem`: it evaluates only what the character can observe from already-known contacts and networks, real arrests/unavailability, known competition and recent disruptions. No unknown network is revealed and no minor case is artificially escalated into a citywide crisis.   
- Economic effects are intentionally bounded: depending on local pressure, some services may lose **1 to 5 quality points**, local trade demand may add **1 to 3%** to some sell prices, and indicative logistics risk may rise by **4 to 18%**. Strong pressure can also create a small demand for transport/relay supplies.   
- `CrimeJusticeSystem` now records a local disruption when a durable arrest genuinely reorganizes known relays. `BountySocialConsequenceSystem` may do the same after capture or death only when the character already knew contacts from the affected network.   
- Shops and selected city services consume this pressure through the existing economic system. The local economy report also exposes **observed criminal pressure** when sufficiently documented.   
- Disruptions create a sourced local rumor through `WorldRumorSystem`. That rumor remains explicitly separate from confirmed fact and never becomes judicial evidence by magic.   
- New `CrimeLocalOpportunitySystem`: a sufficiently meaningful disruption can leave a persistent small guild follow-up. Three forms currently exist: **Registres sous tension**, **Relais sous surveillance** and **Après le remous**. When accepted they become real service quests, with modest rewards and no artificial long quest chain after every incident.   
- The guild menu gains **Suivis locaux**, showing observable consequences and available opportunities in the current city. Identical opportunities are anti-spam and have a cooldown before they may recur.   
- Added `CrimeLocalImpactSystemTest` and `CrimeLocalOpportunitySystemTest` to the global runner. A defect in the impact test itself was also corrected: it retained a reference to a `vector` element across an insertion that could reallocate the vector; the engine was not at fault.   
- **Titles: no additions, modifications or rebindings in this version.** The catalog remains **V3.57.02** with **419 titles**. Save schema remains **26** and the important checkpoint remains **V3.57.00**.   
- Final **V3.57.05 build: OK**. Corrected global runner: **117 tests launched / 117 awaited / 117 PASS**; static and versioning checks: PASS.   

---   

## V3.57.04 — Custody interviews and observable criminal competition   

- Third priority pass on **D — bounties / crime / networks / rumors**: a named target captured alive can now leave one optional **custody interview** at the guild. A killed target naturally creates no interview.   
- New `BountyCustodySystem`: the player may calmly debrief the captive, ask about their network or offer formal cooperation. Testimony remains **sourced testimony**, never automatic truth: it can feed rumors, leads and the canonical journal, but still requires corroboration before becoming evidence.   
- Interviews preserve non-omniscience: asking about a network can only expose or affect criminal contacts the player already knew. Unknown contacts stay unknown.   
- New `CrimeNetworkCompetitionSystem`: when the player already knows several networks in one city, their local tension can be assessed. A documented disruption to one network may raise rivalry, pressure and opportunism among **already-known** rivals without spawning convenient new criminals.   
- A durable arrest validated by `CrimeJusticeSystem` can now create that local disruption. The consequence remains sourced by the real arrest and only affects networks already identified by the character.   
- `CrimeContactRelationshipSystem` can select an **already-known replacement relay** when a contact is arrested, detained, fleeing or in rupture. `CrimeContactDialogueSystem` then exposes that alternative explicitly; unknown relays are never revealed for free.   
- The vault-theft menu now displays tension between already-known criminal networks in the current city when that competition is actually observable.   
- Added `BountyCustodySystemTest` and `CrimeNetworkCompetitionSystemTest` to the global runner. Existing justice, criminal-dialogue and bounty-social-consequence coverage remains in place.   
- **Titles: no additions, modifications or trigger rewiring in this version.** The catalog remains **V3.57.02**, with **419 titles**. Save schema remains **26** and the important checkpoint remains **V3.57.00**.   

- Final **V3.57.04 build: OK**. Global runner: **115 tests launched / 115 awaited / 115 PASS**; static and versioning checks: PASS.   

---   

## V3.57.03 — Playable bounty follow-ups, contextual contacts and evidence-based justice   

- Second priority pass on **D — bounties / crime / networks / rumors**: social consequences from named bounties are no longer mere counters. `BountyAftermathSystem` creates persistent follow-ups for the issuer, their organization and, after a credible delay, a close relation of the target.   
- The guild now exposes a **Bounty follow-ups** screen. The player can answer professionally, conciliatorily, firmly, offer help or close the exchange. Offering help records an opportunity; it does not instantly manufacture an artificial quest.   
- Close relations do not teleport in at resolution time: their follow-up only becomes available from the following day. The same outcome cannot be farmed to generate the same reaction repeatedly.   
- New `CrimeContactDialogueSystem`: already-known contacts now speak according to reliability, debt/favor balance, rivalry, pressure, temporary rupture, detention, flight and judicial state. The system never reveals an unknown contact or network.   
- New `CrimeJusticeSystem`: a durable arrest can only be applied after a sufficiently strong case has been built. The current threshold notably requires at least **6 clues**, **2 holder observations**, **2 guard confrontations**, **1 provisional detention**, an officially documented case and a high evidence score.   
- Justice explicitly distinguishes **documented receiving/complicity** from the original theft: a fence never becomes the burglar retroactively merely because they are suspicious or unpleasant.   
- Durable arrests are persistent, idempotent and recorded in history. Vault-theft cases now expose their evidence score and whether the durable-arrest threshold has been reached.   
- New tests: `BountyAftermathSystemTest`, `CrimeContactDialogueSystemTest` and `CrimeJusticeSystemTest`. The existing `BountySocialConsequenceSystemTest` remains green, locking compatibility with the first D pass.   
- **Titles: no additions or modifications in this version.** The catalog remains **V3.57.02**, with **419 titles**. Save schema remains **26** and the important checkpoint remains **V3.57.00**.   
- Final **V3.57.03 build: OK**. Global runner expanded to **113 tests launched / 113 awaited / 113 PASS**; static and versioning checks: PASS.   

---   

## V3.57.02 — Named bounties, persistent criminal networks and distinct marriage titles   

- First priority pass on **D — bounties / crime / networks / rumors**: targeted bounties can now use a persistent roster of named fugitives with home city, network, issuer, organization and a close relation. Social bounty data is saved, and known legacy targets can attach to their profile without rerolling the target.   
- Bounty presentation exposes only legitimate contract information: issuer, organization and whether live capture is preferred. Private relations and hidden criminal affiliations are not revealed for free.   
- Resolving a bounty now creates **persistent social consequences** for the issuer, its organization, a close relation and already-known contacts in the target's network. Live capture and death create different memories, while unknown contacts remain unknown.   
- New `CrimeContactRelationshipSystem`: reliability, favors/debts, rivalry, fulfilled or broken deals and temporary ruptures now shape criminal-contact relationships. Alternate relays can only use contacts the player actually knows and that are currently available. These fields persist without changing save schema 26.   
- Vault-theft investigations now display the persistent relationship state for known contacts, and a successfully honored negotiated return records a reliable deal. `CrimeNetworkSystem` also factors rupture, reliability, rivalry and favors into relay readiness.   
- `WorldRumorCatalog` gains three lore-backed rumor subjects: **Courriers nocturnes**, **Torvald** and **Grinka**. They remain rumors rather than automatic truths.   
- Title changes in V3.57.02 — **ADDED: « Jusqu'à ce que le ping nous sépare »**, unlocked by player-to-player marriage and deliberately excluded from ultimate completion. **MODIFIED: « Jusqu'à ce que la sauvegarde nous sépare »**, now reserved for NPC marriage; it stays outside ultimate completion until the complete marriage/divorce route is playable and audited. Catalog: **418 -> 419 titles**, version **V3.57.02**.   
- Marriage design recorded for future F work: NPC marriage and player-to-player marriage may coexist; while a player marriage is active, NPC-spouse bonuses are suspended by jealousy without removing the NPC marriage. Divorce will have administrative costs and remarriage will require buying a new wedding ring. Full divorce gameplay is not implemented in this pass.   
- Added `CrimeContactRelationshipSystemTest`, `BountyIdentityCatalogTest` and `BountySocialConsequenceSystemTest`; extended `SaveRoundTripTest`, `WorldRumorCatalogTest`, `TitleCatalogUpdateTest` and `TitleProgressSystemTest`.   
- Final **V3.57.02 build: OK**. Final global runner: **110 tests launched / 110 awaited / 110 PASS**; static and versioning checks: PASS.   

---   

## V3.57.01 — Full title audit since V3.51.51   

- Full title-catalog consolidation after auditing systems added since the last major **V3.51.51** title review: **375 -> 418 titles**, adding **43 titles**. Title catalog is now **V3.57.01**; game version is **V3.57.01**, save schema remains **26**, and the important checkpoint remains **V3.57.00**.   
- Titles added in this pass: **“On a un QG !”**; **“Ce n'est plus une cabane”**; **“Ça devient sérieux”**; **“Il reste une chambre ?”**; **“Le tableau travaille aussi”**; **“La caisse du patron”**; **“Une tournée pour le clan !”**; **“On avait dit petite fête”**; **“Comme à la maison”**; **“Ça va forger.”**; **“Forgeron maison”**; **“C'est en forgeant...”**; **“Chacun sa spécialité”**; **“Prix d'ami”**; **“Enfin chez moi”**; **“Le loyer, cette quête hebdomadaire”**; **“Plus de propriétaire”**; **“Mon lit, mes règles”**; **“J'ai dit oui”**; **“Une place pour deux”**; **“Jusqu'à ce que la sauvegarde nous sépare”**; **“Dossier : retrouvée”**; **“Une colocataire de plus”**; **“Elle apprend... doucement”**; **“Un gant un peu trop attachant”**; **“Deux formes valent mieux qu'une”**; **“Elle ne m'a pas laissé mourir”**; **“C'était ma première vraie amie”**; **“Sur la piste”**; **“Viens sans faire d'histoire”**; **“Il fallait vraiment en arriver là ?”**; **“Pas assez loin”**; **“Tu peux courir...”**; **“J'avais gardé la piste”**; **“Évidemment, ce n'était pas fini.”**; **“Puisqu'on y est...”**; **“Facteur officiel”**; **“Ce n'est pas une lettre, ça.”**; **“Avec correspondance”**; **“J'ai touché de l'herbe”**; **“On barricade d'abord”**; **“Le convoi est arrivé... à peu près”**; **“Il manquait quelque chose au chargement”**.   
- **“Rencontre avec notre cobaye”** and **“J'ai lu le manuel”** now count toward ultimate completion. **“Complétionniste de l'impossible”** now evaluates the union of character titles and legitimate account trophies, so tutorial trophies can count without being copied artificially to every character.   
- New **ultimate alternative-group** mechanism supports genuinely incompatible routes. Housing already uses `home_financing_route`: either completing the rental route or buying the home outright validates that branch without requiring both.   
- New title hooks consume real canonical events from clan headquarters, home, Hestia, bounties, phase-2 quests, inter-city contracts, city events and trade incidents. Existing **“Livreur sous tension”** also gains a coherent trade-incident trigger; **“Commis de crise”** accepts canonical economic-help events in addition to its existing quest paths.   
- Post-rescue Hestia titles intentionally remain outside the ultimate requirement for now: in permanent-death mode, AI allies are permanently mortal too, including Hestia. Those titles should only become mandatory once an equivalent memory/legacy route prevents permanent death from unfairly making 100% impossible. **“Dossier : retrouvée”**, earned before those risky branches, does count.   
- Marriage titles remain outside the ultimate requirement until the full route is honestly playable; **“Jusqu'à ce que la sauvegarde nous sépare”** will remain optional because it requires another human player.   
- Added centralized `TitleProgressSystem` plus `TitleProgressSystemTest` and `AccountAwareUltimateTitleTest`; extended `TitleCatalogUpdateTest` and `TutorialSystemTest` to lock catalog count, alternative groups and account-trophy completion.   
- Final **V3.57.01 build: OK**. The suite now contains **107 tests**: all 107 executables were rebuilt and run against the final tree, **107/107 PASS**; static/version checks also pass. The runner now closes test-binary stdin so interactive pause screens cannot block CI runs.   

---   

## V3.57.00 — Jean-Michel tutorial, title consolidation and onboarding checkpoint   

- New **replayable tutorial** exposed as an extra character card in character selection: `Jean-Michel Test`, officially employed as a **Test Subject**. The card is recommended on a fresh account with no character and remains available later for replay.   
- Tutorial progression is fully **isolated from the real save**: temporary dummy, training contract, temporary item, training combat and fictional reward. No inventory, XP, injury, economy, quest or relationship state from Jean-Michel is persisted.   
- Short onboarding route covers city services, reading a contract and its location accessibility, exploration with clues, combat basics and quest turn-in. The instructor explains continuously but never performs the player's actions.   
- Two account trophies accompany the route: **“Rencontre avec notre cobaye”** on first launch and **“J'ai lu le manuel”** on first completion. They are archived on the account with `Jean-Michel Test` as the unlock character and do not count toward the ultimate-title requirement.   
- Existing **“Bienvenue dans Dinotofu”** behavior is preserved: it is still granted only when a real loaded/created character enters the game; the tutorial cannot consume that milestone.   
- Title-catalog consolidation adds the two tutorial trophies and registers **“Ami du petit Dinotofu”** and **“Œil de serrure”**, which were already granted by gameplay but missing from the catalog. The engine now contains **375 titles**; title catalog synchronized to **V3.57.00**.   
- `scripts/bump_version.py` now manages title-catalog versioning independently and asks, immediately after the important-save checkpoint question, whether titles were added or modified. Non-interactive mode also supports `--titles keep|current|X.Y.Z`. Checkpoint-test synchronization was made targeted instead of blindly rewriting historical version strings.   
- New important save checkpoint **V3.57.00**. Save schema remains **26**: no structural migration is required, while the transition backup protects pre-3.57 saves before onboarding/account-progression adaptation.   
- New `TutorialSystemTest` plus extended title-catalog/checkpoint coverage. Final global runner on the exact V3.57.00 tree: **105 launched / 105 awaited / 105 PASS**, followed by a successful full release audit of the essential backup tree.   

---   

## V3.56.00 — Quest-location consolidation and anti-abuse logistics allowances   

- New `QuestLocationAccessSystem` applied through the shared quest-offer screen: an offer can be **Accessible**, **Reduced find chance**, or **Currently inaccessible/unknown** according to the character's real progression and whether the requested place is exact or only a recommended habitat.   
- Quests that strictly require an inaccessible city, biome or building are now blocked at acceptance with Diane explaining why, rather than entering the journal as a frustrating impossible contract.   
- Hunts, exploration and material searches may remain acceptable when the recommended habitat is inaccessible but the target/resource can plausibly appear elsewhere; the UI warns about the **reduced find chance** and asks for a second confirmation.   
- Guild-board entries expose this location state directly while remaining inspectable. Personal NPC requests, quest events and challenges using the shared offer screen inherit the same check.   
- Inter-city contracts no longer imply full travel reimbursement: their supplement is now a small, capped **one-shot logistics premium**. Normal travel taxes, detours, personal return trips and extra travel always remain the player's responsibility.   
- Inter-city phase-2 follow-ups use the same anti-abuse rule: a small one-time service premium, never a free-travel pass.   
- New `QuestLocationAccessSystemTest`; `IntercityQuestSystemTest` extended with logistics-premium caps. Global runner: **104 launched / 104 awaited / all passed**, exit code 0.   
- Intentional milestone jump `3.51.89 -> 3.56.00`: many major systems had accumulated while the project still stayed under `V3.51.xx`, simply because the larger version number had not been raised earlier. **V3.56.00** therefore realigns numbering with the project's actual scope; there are no missing public `V3.52` to `V3.55` releases. Save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.89 — Paid inter-city missions and service/logistics phase-2 follow-ups   

- New `IntercityQuestSystem`: the guild can now offer real **paid inter-city contracts** for transmitting information or transporting materials. Travel is never completed by an abstract button: each step progresses only when the character physically reaches the requested city.   
- Inter-city contracts are not offered at rank F. Current templates begin at **rank D** (one trip) and **rank C** (up to two relay stops), provided those cities are actually accessible to the character. An impossible offer stays on the board rather than creating a blocked quest.   
- Payment now includes dedicated compensation for **travel, distance and relay count**; material transport pays slightly more than simple information delivery. Deadlines are extended according to contractual distance as well.   
- Service phase-2 follow-ups are now more mundane and contextual: `Before leaving...`, wrong office, signed paperwork that must be handed elsewhere, complementary information delivery, a forgotten package, or one last favor requested by someone who was rescued. A follow-up therefore no longer needs to reveal a conspiracy or a greater danger.   
- Some of these follow-ups become **real inter-city travel**: the journal first preserves what phase 1 completed and what caused the follow-up, then names the delivery city. Physically reaching the correct city completes that leg, and distance adds to the phase-2 reward.   
- Beginner protection: a **rank-F quest/follow-up stays local**. If a normally inter-city dynamic phase 2 appears while no other city is legally accessible, it falls back to a local relay instead of soft-locking the quest.   
- `Quest`, `QuestMenu`, `QuestWorldMenuSupport`, `QuestPresentationSupport` and `SaveManager` now preserve route, transport type, payload, completed relays, distance and travel reward supplements, plus phase-2 inter-city destinations.   
- New `IntercityQuestSystemTest`; extended `QuestSecondPhaseSystemTest` and `SaveRoundTripTest`. Targeted `IntercityQuestSystemTest`, `QuestSecondPhaseSystemTest`, `SaveRoundTripTest`, `GuildQuestAccessPolicyTest`, `EconomyScaleTest` and `TeamEmploymentMissionSystemTest`: PASS.   
- The global runner now contains **103 launched / 103 awaited tests**. It was started pre-bump, but recompiling the entire suite exceeds this sandbox execution window before the final verdict; no false 103/103 claim is made.   
- Bump `3.51.88 -> 3.51.89`: save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.88 — Dynamic quest phase 2 and mandatory/optional follow-ups   

- New `QuestSecondPhaseSystem`: some ordinary quests can look fully normal and only reveal a **phase 2** after their original objective is completed, caused by a consequence, late witness, hidden link, hostile reaction or final inconsistency. The reveal is deterministic per quest, so reopening menus never rerolls it.   
- Two follow-up types exist: **mandatory**, which reopens the quest and blocks hand-in until resolved; and **optional**, which may be declined for the normal reward or accepted for a larger payout.   
- The journal explicitly preserves context: `Phase 1 completed` records **what was just finished**, while `Trigger` records **what led to phase 2**, followed by the new objective, mandatory/optional state, progress and bonus.   
- Active follow-ups are playable from the quest hub with a **careful** approach (slower, safer) or **quick** approach (faster, riskier). Failure consumes real time without granting progress or bonus rewards.   
- Completing phase 2 truly increases the quest reward. Follow-up XP is still paid when the original contract had no authored XP; exact physical payouts remain contractual and are never silently replaced.   
- Generic phase 2 is excluded from story quests, bounties, guild challenges, trade-incident missions and employer team orders, because those systems already own their narrative/administrative lifecycle and must not be randomly blocked.   
- `Quest`, `QuestLog`, `QuestMenu` and `SaveManager` persist offer/activation/decline/completion, previous-phase summary, trigger, objective, attempts, progress and bonus without a save-schema bump.   
- New `QuestSecondPhaseSystemTest`; `SaveRoundTripTest` extended. During the global runner, **101 tests** had already completed PASS and `TeamEmploymentMissionSystemTest` exposed a real eligibility regression; it was fixed, then that test, `QuestSecondPhaseSystemTest` and `SaveRoundTripTest` all passed again. The runner now contains **102 launched / 102 awaited tests**.   
- Bump `3.51.87 -> 3.51.88` (V3.51.87 functionality was reconstructed from the latest complete V3.51.86 backup available in this session): save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.87 — Playable trade incidents and investigation-generated bounties   

- New `CityTradeIncidentMissionSystem`: a convoy incident that truly exists in `CityTradeFlowSystem` can become a temporary Local Information Office mission. No mission is invented without an actual route/incident.   
- Interventions use at least two stages: **local preparation** then road action. Careful play costs more time but improves reliability; quick action saves time with more risk. Failure consumes time and gives no free resolution/reward.   
- Incident missions persist, expire according to urgency and obey existing quest-rank rules. Serious thefts/disappearances may reveal a criminal lead and publish a **separate bounty** to the guild board, which the player remains free to accept or ignore.   
- Publication respects normal board capacity and never duplicates the special bounty. Incident city/goods/type/severity/stage/attempt/publication state is saved.   
- New `CityTradeIncidentMissionSystemTest`; functionality reconstructed during this pass because V3.51.86 was the latest complete backup locally available.   
- Save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.86 — Rare trade incidents and road consequences   

- New `CityTradeIncidentSystem`: a genuinely active or already disrupted trade route can now **rarely** produce an additional incident. Incidents are deterministic per route/cycle, so reopening a menu never rerolls the world.   
- Current incidents: **escort request**, **partial cargo theft**, **missing convoy**, **damaged arrival** and **temporary road obstruction**. A simple delay often remains only a delay: normal traffic is not turned into constant disaster.   
- Incidents affect the **real destination stock** when cargo is lost, blocked or damaged. Escort requests do not create artificial shortages. Emergency-priority convoys remain better protected and take slightly softer economic damage.   
- `CityTradeFlowSystem::routeBetween(...)` now exposes incident identity, severity and effects. `pressureFor(...)` reuses the exact same incident instead of independently rerolling economy and information.   
- The local information desk surfaces important incidents among trade flows. Escort requests report a temporary need for guards; theft/missing-convoy incidents report usable merchant/guard testimony for later investigation or bounty work.   
- `BountyTradeTransitSystem` now reacts to this layer: extra guards make a convoy less comfortable for a discreet escape, while a dangerous low-resource target can exploit an already stolen cargo incident at the cost of leaving especially visible evidence.   
- No new save state is required: incidents are stably derived from day, route, goods category and trade cycle.   
- New `CityTradeIncidentSystemTest`; targeted tests `CityTradeIncidentSystemTest`, `CityTradeFlowSystemTest`, `BountyTradeTransitSystemTest`, `BountyRelocationSystemTest`, `CityStockDynamicsSystemTest` and `CityEconomyDynamicsSystemTest`: PASS.   
- The global runner now contains **100 launched / 100 awaited tests**. It was started pre-bump, but full test compilation exceeds this sandbox window before a final verdict; no fake 100/100 PASS is claimed.   
- Bump `3.51.85 -> 3.51.86`: save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged.   

---   

## V3.51.85 — Mobile bounties and real trade convoys   

- New `BountyTradeTransitSystem`: mobile bounty targets can now exploit **trade routes that are actually active on the current world day** between their origin and an accessible destination. The bounty layer queries `CityTradeFlowSystem` instead of inventing a convenient caravan.   
- `CityTradeFlowSystem` now exposes `routeBetween(...)`, describing one explicit route (departure, arrival, disruption, emergency priority and distance) while reusing the same deterministic network as city stocks and imports. Critical exports suspended during a local emergency cannot magically become escape vehicles.   
- A live trade opportunity can slightly influence destination preference and relocation chance, while the overall chance remains capped so a convoy never guarantees escape.   
- Depending on resources, mobility and danger, a target may **hide in a merchant convoy**, **bribe a driver**, exploit a **delayed convoy**, leave **merchant witnesses**, or steal/infiltrate cargo when short on resources.   
- These behaviors create real inter-city trails used by the existing investigation loop. Cargo theft tends to leave stronger evidence, while bribed transport is more discreet. None of these trails grants an immediate interception.   
- Existing relocation rules remain authoritative: destination access gates, anti-ping-pong, security, distance, wealth/support and transport/black-market context still matter. Trade enriches the choice rather than replacing it.   
- New `BountyTradeTransitSystemTest` plus an extended `CityTradeFlowSystemTest`. Targeted trade/bounty/stock/economy tests pass. The global runner now contains **99 started / 99 awaited tests**, but its full recompilation exceeds this sandbox window before a final verdict, so no false 99/99 claim is made.   
- Bump `3.51.84 -> 3.51.85`: save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged.   

---   

## V3.51.84 — Regional trade flows, imports and convoys   

- Added `CityTradeFlowSystem`: city economic specializations now feed real **inter-city trade flows**. A city that lacks a local industry may receive shipments from a specialized producer, while a producer can export part of its public stock.   
- Trade routes are deterministic and account for **distance**, **city security**, and convoy cycles. Shipments can arrive normally or be delayed; a city with no local production feels a missed convoy more strongly than a city that can already supply itself.   
- Imports/exports now affect **real shop stock** through `CityStockDynamicsSystem`, on top of existing specialization and event pressure. Modifiers stay bounded so imports do not erase shortages, while exports only create modest local pressure.   
- Crises now interact with trade: raid mobilization and reconstruction can prioritize critical inbound shipments, while **critical local exports are suspended** so a city does not ship away its last weapons/materials while it needs them itself.   
- The local information desk now exposes up to four useful trade notices: **arrival**, **dispatch**, **delayed convoy**, improved availability, or associated stock tension, without flooding the player with every route.   
- No new save data is required: convoy cycles are stably derived from world day, city pair and goods type.   
- Added `CityTradeFlowSystemTest`. Targeted `CityTradeFlowSystemTest`, `CityStockDynamicsSystemTest`, and `CityEconomyDynamicsSystemTest` pass. The global runner now contains **98 tests**, but full recompilation exceeds this sandbox window before the final verdict, so no false 98/98 claim is made.   
- Bump `3.51.83 -> 3.51.84`: save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.83 — Artisan specializations, clan prices and headquarters phase 3   

- Clan blacksmiths now unlock one permanent specialization at **artisan level 3**: **weapon forging**, **armor/protection**, or **economic maintenance**. This belongs to the NPC and prepares future value for multiple complementary smith stations instead of one universal artisan.   
- Weapon specialization adds **+5 points** to the Forge I weapon repair cap (95% maximum); armor specialization does the same for ordinary armor; maintenance adds **3 discount points** and saves one material on large repairs that would require at least three. Magical armor remains beyond Forge I.   
- High-level clan artisans now reward long-term investment with a small **boss/friend price**: 0% at levels 1–2, then about **4% / 7% / 10%** at levels 3/4/5 on blacksmith labor. The advantage stays deliberately modest so external specialists remain useful.   
- Forge quotes now expose normal labor price, actual clan discount and final cost. Specialization is visible in the artisan menu and persists through `clan_hq_blacksmith_specializations`.   
- Clan headquarters now have an explicit structural cap of **level 5**. This version opens the real progression up to **level 3**; levels 4–5 stay locked until their advanced module prerequisites actually exist.   
- Upgrading **HQ level 2 -> 3** requires more than money: **quest board II**, **at least two real service modules** (cashbox/bar/forge...) and **at least 3 constructed beds**. The provisional project costs **48,000 PC**, lasts **10 days**, and consumes substantially more materials.   
- HQ level 3 raises caps to **8 beds** and **9 module slots**. It still does not grant free bedrooms; beds remain separate construction projects.   
- Construction quotes now use target-phase cost/time/materials while preserving existing level-2 helper compatibility.   
- `ClanBlacksmithSystemTest`, `ClanHeadquartersUpgradeSystemTest`, `SaveRoundTripTest` and directly affected HQ/celebration tests pass. The global runner starts correctly but recompiling all **97 tests** still exceeds this sandbox window; no fake 97/97 claim is made.   
- Bump `3.51.82 -> 3.51.83`: save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.82 — First clan forge and progression-based blacksmith   

- Added a real **Forge Workshop level 1** HQ module: requires HQ level 2, consumes one module slot, currently costs **18,000 PC**, takes **4 world days**, and requires substantial construction materials for the hearth, anvil, tools, bellows and protections.   
- Building the workshop does not summon a free artisan: the player must recruit a **real clan blacksmith** for **3,200 PC**. Their name, contract, experience and completed jobs are persistent.   
- The blacksmith has **5 skill levels** and advances only through real completed work. Current thresholds are 0 / 60 / 160 / 320 / 560 XP.   
- Forge I can actually repair inventory weapons and armor. Each repair consumes appropriate materials and a modest fee, then grants blacksmith experience.   
- Repair limits deliberately stay below specialized master smiths: about **60% max durability at skill 1**, then 68 / 76 / 84 / **90% at skill 5**. The HQ becomes convenient without making specialized cities obsolete.   
- Repair materials depend on the item: metal for most weapons and metal armor, leather for lighter armor and some weapons, rope for bows. Quantities scale with durability actually restored.   
- **Magical armor** is intentionally beyond Forge I and will later require a specialized artisan/module or enchantment service. Indestructible items cannot be repaired.   
- HQ menus now expose the workshop, recruit candidate, hired blacksmith, skill/XP/jobs, repair cap, and weapon/armor repair quotes. Major confirmations use `FORGERON` and `REPARER`.   
- Added protected persistent categories for forge construction, artisan hiring/contracts, blacksmith XP/jobs/payments and milestones.   
- Added `ClanBlacksmithSystemTest`. Targeted forge/blacksmith, HQ upgrade, celebration and save/reload tests pass. The runner now contains **97 launched / 97 awaited tests**; full recompilation exceeds the sandbox window before a final verdict, so no false global `97/97` claim is made.   
- Bump `3.51.81 -> 3.51.82`: save schema **26**, checkpoint **V3.51.15**, and title catalog **V3.51.51** unchanged.   

---   

## V3.51.81 — HQ comfort, decoration and clan celebrations   

- New **Comfort & decoration level 1** HQ project: requires building level 2, provisionally costs **7,500 PC**, takes **2 days**, and consumes clay, metal, leather/bedding and rope.   
- This upgrade improves existing rooms (bedding, furniture, lighting and shared decoration) and **does not consume a module/workshop slot**, so it does not artificially block future forge/alchemy workshops.   
- With this comfort upgrade, the common-room bar heals slightly more: about **10% max HP** instead of roughly 8%, still once per world day and paid. No permanent combat buff is added.   
- **HQ celebrations** are now playable: at least two residents, the player must be in the HQ city, base cost **900 PC**, reduced to **750 PC** when the bar is installed, and at least **14 days** between major parties.   
- A celebration records participants, grants only **1 to 3 clan reputation** depending on attendance, and gives each present resident one small social-morale charge for their next autonomous mission.   
- That morale only adds **+2 percentage points** to the next autonomous mission's XP share, still strictly capped by the global **15–25% of equivalent active-quest XP** rule. It creates neither direct income nor combat bonuses.   
- New protected persistent categories: `clan_hq_comfort_decor_upgrades`, `clan_hq_furnishings`, `clan_hq_festivals`, `clan_hq_festival_participants`, `clan_hq_festive_morale`, and `clan_hq_festive_morale_consumed`.   
- Added `ClanHeadquartersCelebrationSystemTest`; targeted comfort/party, bar, HQ upgrades, rank-policy and save/reload tests pass. The global runner now starts/waits for **96 tests**, but recompiling the whole suite exceeds this sandbox execution window, so this pass does not claim a fake `96/96`.   
- Bump `3.51.80 -> 3.51.81`: save schema **26**, checkpoint **V3.51.15**, and title catalog **V3.51.51** unchanged.   

---   

## V3.51.80 — Five-member rank averaging and the HQ common-room bar   

- Central `GuildQuestAccessPolicy` now separates **1-4 member parties** from larger groups. Up to four members, access stays strict: solo is capped at the character's own rank; in a party, the weakest member caps the mission at at most their rank +1.   
- From **5 members onward**, the maximum mission rank uses the **group average rank**, rounded to the nearest rank. If that average is **within one rank of the strongest member**, the party may attempt **average +1**; otherwise the average itself is the cap. This rewards coherent large teams without letting one or two elite characters drag several beginners far above their level.   
- The same 5+ averaging rule is reused by player group quests and autonomous HQ missions. An integration test specifically verifies **4 rank-E + 1 rank-F -> rank-D cap** at the HQ.   
- Guild/group UI text now explains the small-party vs five-member rule instead of showing the old generic “+1 rank maximum” wording.   
- New real **level-1 bar / common-room module** for the clan HQ: requires a level-2 HQ and a free module slot, currently costs **10,500 PC**, takes **3 days**, and consumes construction/furnishing materials.   
- A bar visit currently costs **35 PC**, restores a small amount of HP, and records a social moment with available residents. It is limited to **one use per world day**, preventing infinite healing or permanent combat buffs.   
- New protected canonical categories: `clan_hq_common_room_bar_upgrades`, `clan_hq_bar_visits`, and `clan_hq_social_moments`. `SaveRoundTripTest` now also checks the bar module after save/reload.   
- Added `ClanHeadquartersCommonRoomSystemTest` and `ClanHeadquartersRankPolicyTest`. Targeted rank-policy, base/upgraded HQ, common room, save/reload and recruit-roster tests pass. The global runner now contains **95 started / 95 awaited tests**, but recompiling the full runner exceeds this sandbox window, so no fake `95/95` claim is made.   
- Bump `3.51.79 -> 3.51.80`: save schema **26**, checkpoint **V3.51.15**, and title catalog **V3.51.51** unchanged.   

---   


## V3.51.79 — Consistent quest ranks, proportional autonomous XP and HQ mission cashbox   

- New central `GuildQuestAccessPolicy`: **solo** players may only accept guild quests up to their own guild rank; with a **real active party**, they may attempt at most **one rank higher**. Orange/red reliability penalties may still lower that ceiling.   
- Group quests no longer rely only on average party rank: **every present member** must individually satisfy the `+1 rank` limit. A rank-F ally can therefore no longer be dragged into a rank-D contract by a much stronger companion.   
- The same policy now applies to HQ residents: a lone recruit stays capped at their own rank; at least two available residents can form a group and attempt at most `+1 rank`, still limited by quest-board level.   
- Autonomous HQ missions no longer grant a fixed 5/7 XP. They now award **15–25% of the estimated XP of an equivalent active quest**, based on mission rank and the recruit's estimated level, remaining clearly slower than adventuring with that recruit directly.   
- First buildable **mission cashbox**: requires **HQ level 2 + quest board level 2 + a free module slot**, provisionally costs **9,000 PC**, takes **2 days**, and consumes materials. Initial storage is capped at **2,500 PC**.   
- Autonomous missions may deposit a small clan share into the cashbox, but **no money is paid directly to the player**. The player must physically return to the HQ city to collect it. Once full, overflow is not stored and adventurers remain free to take guild contracts instead.   
- Canonical persistence added through `clan_hq_mission_chest_upgrades`, `clan_hq_mission_chest_deposits`, and `clan_hq_mission_chest_withdrawals`, now also protected from generic journal cleanup. `SaveRoundTripTest` verifies installation, deposits, withdrawals and reload.   
- New `GuildQuestAccessPolicyTest`; `ClanHeadquartersUpgradeSystemTest` also covers the cashbox, autonomous rank limits and manual collection. Directly affected tests pass. The global runner now starts/waits for **93 tests**, but recompiling the complete suite exceeds this sandbox execution window, so this pass deliberately does not claim a fake `93/93`.   
- Bump `3.51.78 -> 3.51.79`: save schema **26**, checkpoint **V3.51.15**, and title catalog **V3.51.51** unchanged.   

---   

## V3.51.78 — Real clan HQ expansion, bedrooms and level-2 quest board   

- The clan HQ can now be **physically expanded to building level 2** after its initial construction. The provisional project costs **26,000 PC**, takes **7 world days**, and consumes a substantial amount of clay, metal, leather and rope.   
- Level-2 expansion raises the structure to **5 maximum beds** and **6 module slots**, but does not grant free rooms: the two starter beds remain and each additional bedroom must be funded separately.   
- **Extra bedrooms** are now playable one at a time for **4,500 PC**, **2 construction days**, and dedicated materials. A level-2 HQ can therefore progressively grow from 2 to 5 permanent residents.   
- The **level-2 quest board** can now be built for **12,000 PC** and **3 days** after the building expansion. It unlocks autonomous clan contracts up to **E/D ranks** and raises the current autonomous recruit XP from **5 to 7 XP per mission**.   
- Only one HQ upgrade project may run at a time, and construction must be ordered from the **HQ's home city**, keeping the building physically anchored in the world.   
- Autonomous gains remain deliberately limited: **no direct AFK income is paid to the player yet**. A future capped mission cashbox/bank will be the controlled path for recoverable rewards, and the HQ still does not replace the guild.   
- The HQ menu now shows building level, current/max bed capacity, module capacity, board level, active construction, completion day, and full quotes before confirmation with the `TRAVAUX` keyword.   
- Canonical persistence was added for construction jobs through `clan_hq_upgrade_jobs_started` and `clan_hq_upgrade_jobs_completed`, with no save-schema bump. `SaveRoundTripTest` now verifies a level-2 HQ, a third bedroom, a level-2 board, and a following active construction job across save/reload.   
- New `ClanHeadquartersUpgradeSystemTest`. The complete runner now explicitly starts and waits for **92 C++ tests**: **92/92 PASS**, exit code 0, ending with “Tous les tests Dinotofu sont passés.” before the bump.   
- Bump `3.51.77 -> 3.51.78`: save schema **26**, checkpoint **V3.51.15**, and title catalog **V3.51.51** unchanged.   

---   

## V3.51.77 — First clan headquarters, retained recruits and title history   

- New `ClanHeadquartersSystem`: a character can start building their **single clan headquarters** after passing the guild group test and registering a clan name. The project consumes money, materials and several world days.   
- The base HQ deliberately starts small: **counter, level-1 quest board and two recruit beds/rooms**. Its host city persists and a second HQ cannot be started in parallel or after completion.   
- Recruits the player wants to keep can be attached to the HQ without remaining in the active party. They stay in the clan's circle; leaving a room frees capacity without deleting their history.   
- The base quest board processes small autonomous jobs about every three days. A recruit travelling with the player, injured or awaiting evacuation cannot simultaneously farm HQ jobs. Passive progression is deliberately slow and pays no direct AFK income to the player; a future capped quest treasury/bank will govern withdrawable proceeds.   
- Real `xp_recrues` XP now contributes to each recruit's **estimated level**, so HQ activity can genuinely develop them, much more slowly than active adventuring.   
- The team menu now exposes HQ construction, resident slots, passive activity and an explicit future roadmap: building size, beds, upgraded board, bar, treasury, comfort/parties, forge, alchemy and craftsmen. These later modules are not active yet.   
- Newly unlocked account titles now retain their **unlock date/time** and the **character name** that earned them. The account title archive displays the metadata while remaining compatible with legacy name-only entries.   
- HQ canonical-journal categories are protected from narrative cleanup. Added `ClanHeadquartersSystemTest` and extended `AccountTitleProgressTest` plus `RecruitedAllyCombatRosterTest`.   
- Bump `3.51.76 -> 3.51.77`: save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged.   

---   

## V3.51.76 — Real local stocks and inter-city bounty trails   

- New `CityStockDynamicsSystem`: city specializations now change the **actual finite stock offered** by relevant shops, not only prices or abstract service quality. A specialized modest city can carry more of its specialty than a richer generalist city.   
- Local events and preparations can temporarily tighten or broaden stocks. Raid mobilization pressures materials, weapons, armor, smithing and consumables, while fairs and agricultural festivities can increase selected availability.   
- Specialization cushions shortages without erasing them. Unlimited inventory is not artificially turned finite and already sold-out entries are not resurrected for free.   
- Shop screens now expose a short local availability line when a stock profile is active.   
- Bounty targets that truly relocate between cities now leave a persistent **inter-city trail** such as a coach registry, road witness, suspicious purchase, criminal contact or ferryman trace.   
- Experienced targets who know they are being hunted can occasionally plant a **false lead**. Trails have reliability and are one-shot investigation opportunities: they can help rebuild clues but never grant a free interception.   
- Investigation approaches use transit evidence with different effectiveness; poor or planted evidence may add investigation noise. The bounty tracking screen shows when a transit trail remains usable.   
- Save data gains optional `bountyTransitTrailKind`, `bountyTransitTrailFromCityId`, `bountyTransitTrailReliability` and `bountyTransitTrailAvailable` fields with no save-schema bump.   
- New `CityStockDynamicsSystemTest`; `BountyRelocationSystemTest`, `BountyQuestSystemTest` and `SaveRoundTripTest` extended. All four directly affected tests pass after integration. The 90-test global runner was started, but its full recompilation exceeds this environment's execution window; no functional regression appeared in the targeted validation.   
- Bump `3.51.75 -> 3.51.76`: save schema **26**, important checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.75 — Living local events and inter-city fugitives   

- The local-event cycle expands from 8 to **10 recurring highlights** without turning every day into a festival: a community **Garden Festival** and a **mobilization against an announced raid** were added.   
- Garden Festival preparation creates temporary demand for plants, seeds, flowers, decorative crops, light construction materials and botanical ingredients, with small community rewards for helping locally.   
- An upcoming raid now creates real logistical pressure before the attack: strong demand for **iron/barricade materials, weapons, armor, blacksmith work and consumables**. The local information desk exposes these needs during preparation.   
- Existing community-help interactions now recognize the new events where appropriate, without automatically making the player the commander of a raid.   
- New `BountyRelocationSystem`: experienced, dangerous or highly mobile targets may now **leave a city after escaping** and seek shelter in another region.   
- Destination scoring considers local security, black-market/transport specialization, wealth, travel distance and a small deterministic variation. Unstable/chaotic areas are naturally more attractive to fugitives than heavily secured cities.   
- A live bounty can never relocate behind a city that the player cannot currently access because of level, boss or permit gates. Previously visited target cities are remembered to prevent silly back-and-forth ping-pong.   
- Relocating invalidates overly precise preparation from the old area: the player keeps a broad trail but must rebuild local intelligence instead of receiving free omniscience.   
- Bounties accepted outside the original starter area now receive a persistent **current city and current refuge/biome**, so the legacy “Trade Route” fallback no longer pins every contract to Valebrume.   
- Regional target adaptation is reapplied exactly once when entering a new city as an environment delta, preventing security modifiers from stacking forever. Compatibility is preserved for V3.51.74 saves that only contain the legacy applied boolean.   
- Save persistence adds optional `bountyCurrentCityId`, `bountyCurrentBiome`, `bountyVisitedCityIds`, `bountyRelocationCount` and `bountyRegionalProfileCityId` fields with no schema bump.   
- Added `BountyRelocationSystemTest` and extended city-economy, regional-bounty and save-roundtrip coverage. Pre-bump suite: **89/89 C++ tests plus static checks passed**.   
- Bump `3.51.74 -> 3.51.75`: save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged.   

---   

## V3.51.74 — Local economy pulses, market needs and resale channels   

- Added `CityEconomyDynamicsSystem`: a city’s **structural wealth** is now separate from its **effective/temporary wealth**. Fairs, celebrations, preparations and reconstruction can create temporary booms or downturns without rewriting the city’s underlying identity.   
- Temporary pressure naturally returns toward the structural tier when it ends. Future governance rules can extend/repeat these shifts without treating wealth as universal superiority.   
- Weekly events and their preparation windows now create **targeted market demand** for relevant goods, with bounded resale bonuses.   
- Real reconstruction can temporarily reduce felt prosperity and service quality while increasing demand for rebuilding materials, tools, healing plants and supplies.   
- The **local information desk** exposes structural/current wealth, security, specializations, current economic pulse, active needs and remaining crisis/preparation horizon.   
- Materials now receive better resale value from the **right buyer**. Monster components are especially suited to `MonsterMaterial` collection channels.   
- The guild now exposes a **monster-material collection counter**; a dedicated monster-material shop uses the same reference channel. Compatible unrelated merchants can still buy such goods but pay less.   
- Resale percentage rounding was improved so small specialization bonuses do not systematically vanish on low-value items.   
- Inn service quality now reacts to temporary economic conditions in addition to the structural city profile.   
- Added `CityEconomyDynamicsSystemTest`; the runner now explicitly waits for both city-profile and city-economy tests. Pre-bump full suite and static checks pass.   
- Bump `3.51.73 -> 3.51.74`: save schema **26**, important checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   

## V3.51.73 — Living cities: security, wealth, specialization and bounty value   

- New `CityProfileSystem`: every city can now carry an independent **security** tier (`highly secured`, `watched`, `neutral / independent`, `unstable`, `anarchic / chaotic`) and **wealth** tier. The two axes are intentionally separate.   
- Cities now have **economic specializations** per shop/service type with their own strength. Specialization can deliberately outperform raw wealth: **Rocheveille** is only modest, but its weapon/armor/blacksmith expertise beats a rich generalist city for those domains.   
- First playable distribution: Valebrume = crafting/plants/survival; Rocheveille = forge/weapons/armor; Port-Lanterne = transport/imported goods/parallel networks; Lys-de-Givre = enchanting/alchemy/rare healing.   
- Buy prices, resale value and **service quality** now receive city context. The inn is the first complete quality-sensitive service: meal efficiency and common-bed recovery cap vary inside bounded ranges.   
- Local profiles are exposed in city hub/network, shop and bounty screens so wealth, security and specialization are not hidden values.   
- Targeted bounties now receive a persistent one-time value based on **danger**, **notoriety/target importance** and **time at large**. Total bonus is capped to preserve PF/PC purchasing-power checkpoints and prevent extreme targets from breaking the economy.   
- City security also influences fugitives: highly secured areas reduce local support and mobility while raising vigilance; unstable/chaotic areas do the reverse without granting criminal omniscience.   
- Inn voucher bundles remain genuinely cheaper than buying five services separately after local pricing; the warm-meal booklet received a slight adjustment to preserve that rule.   
- Save persistence adds bounty valuation fields (`bountyDangerRating`, `bountyNotorietyRating`, `bountyDaysAtLarge`, `bountyBaseRewardGold`, `bountyRewardProfileApplied`) with no schema change.   
- Added `CityProfileSystemTest` and extended bounty, save-roundtrip and economy-scale coverage. Full suite: **87/87 C++ tests plus static checks passed** before and after bump.   
- Bump `3.51.72 -> 3.51.73`: save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged.   

---   

## V3.51.72 — Targeted bounties: regional memory, live capture and lasting consequences   

- New `BountyRegionalConsequenceSystem`: each region now keeps a real memory of targeted bounties resolved there, without making criminals omniscient.   
- Confrontations now distinguish three outcomes: **surrender/live capture**, **live capture after combat**, or an explicitly chosen **lethal approach**. A refused surrender never silently turns the player lethal; combat remains capture-oriented unless the player explicitly chooses otherwise.   
- The outcome is persisted on the quest (`captured_surrender`, `captured_combat`, `killed_combat`) and protected against duplicate consequence application after save/reload.   
- Live captures, lethal resolutions, very noisy hunts and previous escape attempts are remembered **per city/region** in the canonical journal.   
- Resolving a bounty now affects **local reputation** and can make criminal contacts already known by the player in that city more tense: pressure rises and cooperation drops. Unknown contacts are never revealed or fabricated.   
- Each resolution creates a persistent, credible world rumor about the target's actual fate.   
- Future bounty targets in the same region adapt once to that history: higher awareness, local support that can dry up after clean captures or close ranks after violent/noisy hunts, and bounded changes to resources and mobility based on local precedent.   
- New bounty fields are optional in saves: **save schema 26 remains unchanged** and older saves remain compatible.   
- `BountyRegionalConsequenceSystemTest` covers live capture, known criminal-contact reaction, rumor creation, duplicate prevention, future-target adaptation, lethal/noisy resolution and escape-driven learning. `SaveRoundTripTest` also covers the new persistent fields.   
- Hestia canon received a small non-central addition: after some time discovering the world with the player, she can taste **pammes**, absolutely loves them and gradually develops a harmless comedic obsession with them, with no dependency, maintenance meter, withdrawal or obligation. The universe rule that bulk pammes are slightly cheaper per unit is also recorded in `LORE_HESTIA_DINOTOFU.txt`.   

- C++23 pre/post-bump build: **OK**. Full suite: **86 / 86 C++ tests PASS** through the isolated stdin-closed runner; project static checks: **PASS**.   
- Bump `3.51.71 -> 3.51.72`: **OK**. Save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged.   


## V3.51.71 — Expanded anatomy and genuinely differentiated monster weak points   

- `MonsterAnatomySystem` now goes well beyond the explosive skeleton, ordinary skeleton and slime: the **Target body weakness** tactical action recognizes several major bestiary families from their race and credible anatomy.   
- **Constructions** expose mechanical joints or plate seams; a successful read can reveal stats, slightly weaken the target, prepare the next hit and break an active defensive posture.   
- **Dragons** can be read at the wing root, while **draconids/semi-dragons** expose scale seams. These openings stay brief and tactical rather than turning durable targets fragile.   
- **Plants** expose a root collar, **insectoids** use leg joints or carapace seams depending on protection, and **armored/shelled beasts** expose mobile edges or shell hinges.   
- **Elementals** have a temporarily readable energy focus, while **spirits/angels/demons** rely on a spiritual anchor. Successful reads can reveal the target and apply a small nature-consistent vulnerability.   
- **Arcane anomalies** now have an unstable knot: exploiting it is genuinely useful, but contact also causes backlash damage and a brief hallucination. A weak point is therefore not always a free choice.   
- The **Explosive suicide skeleton** keeps its existing dangerous priority behavior: its trapped skull can still detonate onto the player with poison, burning and hallucination.   
- Anatomy bonuses remain deliberately modest because the tactical action already applies generic effects; the system adds creature identity without doubling the player's power.   
- `MonsterAnatomySystemTest` now covers positive and negative reactions across trapped/ordinary skeletons, standard and armored constructions, dragon, draconid, plant, armored insectoid, armored beast, elemental, spirit, anomaly and a generic humanoid fallback.   

- Post-bump C++23 build: **OK**, confirmed `Dinotofu V3.51.71`. All **85 C++ tests** referenced by `test_project.sh` compile and pass through an isolated runner used to avoid a container-specific job-control issue; the same script's static validation checks also pass completely.   
- Bump `3.51.70 -> 3.51.71`: **OK**. Save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** remain unchanged.   

## V3.51.70 — Combat-ready Hestia, real equipped artifact and tactical anatomy   

- Added `HestiaCompanionSystem`: active Hestia now occupies a **real active ally slot** instead of granting a free third support. She creates no normal recruit reward share or salary state.   
- The player freely chooses Hestia's current class, weapon style and protection. Her real magical specialty is not automatically revealed: she starts poor at everything, including magic, and the gap only emerges through practice, combat and awakenings.   
- Diane now exposes optional Hestia training while Hestia is at the guild or travelling. Practice consumes world time but no upkeep loop is imposed if the player leaves her safely with Diane.   
- Hestia can cause real early combat mishaps: lost actions or accidental ally damage, capped so a direct mishap alone cannot kill the player or a recruit. Practice, experience, trust and awakenings progressively reduce the risk.   
- Her three divine/light awakenings can now occur from magical combat actions once progression conditions are met in normal combat and bosses; the third awakening triggers her actual return to the heavens.   
- `HestiaSoulArtifactSystem` is connected to real equipment: the artifact can be equipped as a genuine indestructible, soul-bound, unique/personal weapon and rests as its skin-bound glove. Equipping a normal weapon rests it.   
- Artifact damage now genuinely follows `3 × level`; materialized bow/crossbow forms use no ordinary ammunition. A soul is credited only when the artifact's **direct impact** delivers the killing blow; poison, burning, secondary ammo and delayed effects do not count.   
- Equipment UI now exposes artifact equip/rest, form switching and the permanent player-chosen second form unlocked at level 16.   
- Added generic `MonsterAnatomySystem` and connected it to **Target body weakness** tactical actions instead of quest-only hard-coding.   
- The **Explosive suicide skeleton** now has a truly trapped skull: successfully striking that weak point detonates it onto the player, destroys the skeleton and can inflict damage, poison, burning and hallucination. Ordinary skeleton and slime anatomy provide reusable foundations.   
- Hestia configuration and artifact-equipped state survive save/reload through existing persistent data without changing save schema 26.   
- Added tests for Hestia companion discovery/loadout/active slot/awakening, real artifact equipment/progression/persistence and generic monster anatomy.   

- Pre-bump C++23 build: **OK**. Full pre-bump suite: **ALL DINOTOFU TESTS PASS (`TEST_EXIT=0`)**.   
- Bump `3.51.69 -> 3.51.70`: **OK**, save schema 26, checkpoint V3.51.15 and title catalog V3.51.51 preserved. Post-bump rebuild: **OK**, `Dinotofu V3.51.70` confirmed. Full post-bump suite: **ALL DINOTOFU TESTS PASS (`POST_BUMP_TEST_EXIT=0`)**.   

## V3.51.69 — Playable Hestia missing-person case, active housing and real sacrifice   

- New `HestiaMissingPersonQuestSystem`: Hestia's encounter appears only after **20 actually completed quests** once story mode has started. Diane presents it only as an old **missing-person case**; no divine identity, light magic or future importance is spoiled.   
- The case is playable from exploration with a hard stealth route or frontal combat and a stable per-character goblin+ninja / skeleton+explosive-suicider defense variant. It has no deadline and never blocks other story chapters if ignored.   
- Hestia can stay with Diane, live at the player's home or travel actively. All residence choices are reversible and there is **no upkeep, trust decay, guilt reminder or progression penalty** for leaving her safely behind.   
- Housing is now usable: purchase costs are actually charged; rental housing suspends free services when rent is due but creates no automatic debt, arrears or eviction. One payment simply restarts seven days of service. Sleeping at an active home is free and restores up to 90% HP plus full mana.   
- Guild and home menus now expose optional Hestia residence/visit controls without forcing her into the active roster.   
- `PlayerDeathResolver` now consumes Hestia's post-awakening sacrifice before normal death protections in standard PvE, exploration and standard boss defeats when she is actually travelling with the player. The player is extracted at 20% HP and Hestia loses her materialization; no remote rescue from Diane/home.   
- New tests cover the 20-quest gate/no-spoiler presentation, rescue defenders, housing payment/no accumulated arrears/no Hestia decay, and the real Hestia death resolver.   

- Pre-bump C++23 build: **OK**. Full pre-bump suite: **ALL DINOTOFU TESTS PASS (`TEST_EXIT=0`)**.   
- Bump `3.51.68 -> 3.51.69`: **OK**, save schema 26 and checkpoint V3.51.15 preserved. Post-bump rebuild: **OK**, `Dinotofu V3.51.69` confirmed. Full post-bump suite: **ALL DINOTOFU TESTS PASS (`POST_BUMP_TEST_EXIT=0`)**.   

## V3.51.68 — Persistent Hestia, housing, soul artifact, Arone and Reaper doctrine   

- Added persistent Hestia story foundations: rescue state, residence at active party / Diane's guild desk / player home, trust visits, small harmless home finds, three divine/light awakenings and the final return to the heavens.   
- Hestia now starts bad at everything, including magic during readaptation; automatic name-based magic/healing power and the obsolete Hazak-rescuer interpretation were removed.   
- Added Hestia's post-first-awakening sacrifice rule, available only while she is genuinely active with the player.   
- Added one-city player-home foundations, spouse cohabitation state and persistent residence data.   
- Added `HestiaSoulArtifactSystem`: skin-bound glove at rest, adaptive main weapon form, `3 × level` base damage, artifact-only killing blows for soul progression, anti-farm per soul family, and a permanent player-chosen second form at level 16 with free switching afterward.   
- Added Arone Aster as a special character and a private Reaper family-secret rule that only Arone can unlock through personal recognition; generic investigation can never reveal the buried name.   
- Updated Reaper doctrine toward verified bounty execution, nuanced blood-debt logic, legitimate-execution merit, anti-oppression principles and non-pacifist use of chaos when the unit is directly threatened.   
- Sanctus and Skuro are now mutually exclusive states of the same person.   
- Save schema remains 26. Full pre-bump build and test suite pass (`TEST_EXIT=0`).   


- Final post-bump validation: C++23 rebuild OK, `Dinotofu V3.51.68` confirmed, full suite `POST_BUMP_TEST_EXIT=0`.   
- Final cleanup: removed legacy name-based Hestia combat bonuses (free dodge and old revive dome), so her power now truly depends on readaptation and awakenings.   

## V3.51.67 — Playable targeted bounties, hunt awareness and credible interception   

- Explicitly separates **being wanted** from **knowing an active hunter is on your trail**. `BountyHuntAwarenessSystem` now classifies a target as `Unaware`, `Suspicious` or `ConfirmedHunt` from observable facts only: investigation noise, exposed informants, detected shadowing, direct warning and previous failed pursuits.   
- A target that does **not know about the hunt prepares no anti-hunter countermeasures**, even when naturally cautious or well funded. Suspicious targets stay limited to cautious reactions; full decoys and refuge changes require a credible reason to believe the hunt is real.   
- The unclear `LayLow` wording is removed in favor of **hide or change refuge** (`HideOrChangeRefuge` / `refugeConcealment`).   
- New `BountyQuestSystem`: first persistent, playable targeted bounty loop with investigation → preparation → pursuit/interception → confrontation. Players can observe quietly, ask faster but noisier questions, scout exits, prepare an interception and attempt to contain the target once enough clues exist.   
- Guild boards can now generate rank E-to-B mobile human bounty targets with persistent name, temperament, resources, mobility, local support, terrain knowledge and awareness. A target's traits are generated once and never rerolled during later investigation steps.   
- Investigation now has a real **information / discretion** tradeoff. Failed interception explicitly informs the target of the hunt, degrades local preparations and expires part of the exact trail. Successful interception leads to surrender demand or real PvE confrontation.   
- Bounty missions are visible as field/combat contracts in the quest journal instead of behaving like a hidden technical objective type.   
- Target identity, clues, investigation noise, hunt awareness, resources, escape attempts and both sides' preparations persist through backward-compatible optional save fields; save schema remains 26. `SaveRoundTripTest` now covers the entire targeted-bounty state explicitly.   
- Lore wording was made weapon-agnostic (`why you fight`, `knowing how to kill`) and Reaper-13's arsenal is documented: the **scythe is the primary artifact**, alongside a sickle, dagger, crossbow and gadgets; selected secondary weapons may use artifact-like mechanisms without replacing the scythe's central role.   
- REAPERS-13 remains an expert user of shared hunting systems: no omniscience, no magical `no escape`, and no target countermeasure when the target has no reason to know it is being hunted.   

- Pre-bump C++23 build: **OK**. Full pre-bump suite: **ALL DINOTOFU TESTS PASS (`TEST_EXIT=0`)**.   

- Bump `3.51.66 -> 3.51.67` with `--no-commit`: **OK**. Post-bump C++23 rebuild: **OK**. Full post-bump suite: **ALL DINOTOFU TESTS PASS (`POST_BUMP_TEST_EXIT=0`)**.   
- Save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged. Loans/debt/overdraft/collections and story work remain paused.   

## V3.51.66 — Update notes   

- Version synchronization placeholder. Replace with detailed release notes before publishing.   

---   

## V3.51.65 — Counter-tracking, inter-city crime networks, recruit recovery and world folklore   

- `BountyPursuitSystem` now has a real shared **preparation / counterplay** layer for all bounty hunters: exit scouting, informants, ambush setup and shadowing for hunters; route changes, decoys, counter-surveillance and hiding or changing refuge for targets.   
- Preparation is bounded and stackable without becoming absolute. Skilled hunters improve containment through real work; prepared targets can reopen routes and reduce extra escape openings. Escape chance remains bounded to at least 5%.   
- REAPERS-13 keeps using the shared system as an expert profile; no magical `no escape` switch was added.   
- New `CrimeNetworkSystem`: already **known** criminal contacts can receive inter-city warnings when two contacts belong to the same plausible network (`grey market`, `night couriers`). Unknown contacts remain completely unknown.   
- `PlayerCrimeContactState` now remembers home city, network, warning count/date/source. These optional fields round-trip without changing save schema 26.   
- Guard confrontations still affect known local contacts and can now send lighter pressure to already-known relays in other cities; a failed confrontation sends a stronger warning than cooperation.   
- `RecruitedAllyConditionSystem` now exposes a lightweight recovery state (`no persistent wound`, `bruised`, `significant wound`, `severe recovery`) based on existing combat facts, without psychiatric diagnosis or arbitrary permanent penalties.   
- The infirmary gains **Recovery follow-up**: physical cost, one world-time unit, reduced fatigue/injury stress, but no magical HP restoration. HP treatment and recovery remain separate systems.   
- New generic `WorldRumorCatalog` with an initial set of rumors around Valebrume, Rocheveille, Port-Lanterne, Lys-de-Givre, the Whistling Mine, statues, mana fairies, Silent Blades and other places. Hearing a rumor never confirms it.   
- Exploration can rarely record a local/non-Reaper biome rumor with its own cooldown. REAPERS-13 is therefore no longer the only subject with folklore flowing through `WorldRumorSystem`.   
- New tests: `CrimeNetworkSystemTest`, `WorldRumorCatalogTest`; `BountyPursuitSystemTest`, `RecruitedAllyConditionSystemTest` and `SaveRoundTripTest` cover this pass.   
- Pre-bump C++23 build: **OK**. Sensitive targeted tests (pursuit, crime network, recruit condition, rumors, save, vault, Reapers): **OK**.   
- Pre-bump full-suite attempts exceeded the execution window without producing `[FAIL]`. After the bump and rebuild, the complete suite ran through to completion: **ALL DINOTOFU TESTS PASS (`POST_BUMP_TEST_EXIT=0`)**.   
- Bump `3.51.64 -> 3.51.65` with `--no-commit`: **OK**. Post-bump C++23 rebuild: **OK**.   
- Save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged. Loan/debt/overdraft collection work and story remain paused.   

## V3.51.64 — Bounty pursuit, credible escape denial, suicidal-order refusal and reactive criminal networks   

- New generic `BountyPursuitSystem` for bounty hunters: tracking, route control, stealth, coordination, target mobility, terrain knowledge, prepared escape routes and observed escape attempts now produce an actual pursuit plan.   
- Pursuit never grants magical no-escape certainty: it can apply a bounded escape-chance penalty (currently at most -30 points) and add up to two required openings when exits were genuinely prepared. Mobile/prepared targets therefore retain a credible escape route.   
- `ReaperDoctrineSystem` now formalizes the user-approved identity: REAPERS-13 is first and foremost a major clan of **shadow bounty hunters**. Their pursuit profile is very strong in tracking/containment/stealth/coordination and prefers capture/neutralization; their escape denial uses the generic system rather than faction immunity.   
- The doctrine/legend catalog adds **“Shadow bounty hunters”** as strong doctrine. Existing rumours stay separate from facts and may still be false or exaggerated.   
- A natural Reaper exploration trace now reads primarily as a prepared hunt: secondary routes closed, retreat angles watched and interception organized, without pretending supernatural target knowledge.   
- New `RecruitedAllyOrderSafetySystem`: a recruit can finally refuse a forced offensive order that is objectively close to suicidal. HP, readiness, morale, enemy count, boss context and relationship shape danger/refusal.   
- Normal or merely risky orders are still followed. Refusal does not steal the recruit's turn: only the catastrophic offensive instruction is ignored, then the shared ally AI chooses a credible survival action.   
- Visible PvE wires this safety without deleting the order for the rest of the group. An observable refusal is recorded in history.   
- Criminal network: a guard confrontation no longer affects only the targeted fence/accomplice. Other **already-known contacts in the same city** gain some pressure/caution; a failed confrontation alerts the network more strongly than a cooperative one. No unknown NPC is invented to simulate the reaction.   
- `CityVaultSecurityLogisticsTest` protects that network reaction. New `BountyPursuitSystemTest` and `RecruitedAllyOrderSafetySystemTest`; `ReaperDoctrineSystemTest` also covers pursuit doctrine. The runner now explicitly waits for `reaper_doctrine`, which was previously launched without a dedicated wait.   
- Pre-bump C++23 build: **OK**. Full pre-bump suite: **ALL DINOTOFU TESTS PASS (`TEST_EXIT=0`)**.   
- Bump `3.51.65 -> 3.51.66` with `--no-commit`: **OK**. Post-bump C++23 rebuild: **OK**. Full post-bump suite: **ALL DINOTOFU TESTS PASS (`POST_BUMP_TEST_EXIT=0`)**.   
- Bump `3.51.63 -> 3.51.64` with `--no-commit`: **OK**. Post-bump C++23 rebuild: **OK**. Full post-bump suite: **ALL DINOTOFU TESTS PASS (`POST_BUMP_TEST_EXIT=0`)**.   
- This complete validation also revalidates the V3.51.63 foundations whose global rebuild had not completed inside the previous environment window.   
- Save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged. Loan/debt/overdraft/collection development and story remain paused.   

## V3.51.63 — Reaper doctrine, legends separated from facts, and lightweight persistent injury strain   

- REAPERS-13 remains a **major but optional faction/clan**: no main-story takeover, no automatic recruitment, no free rank.   
- New `ReaperDoctrineSystem`: an explicit legend catalog adapted from the user-provided Discord corpus. Each story has an id, title, initial reliability and starting credibility; hearing it records a world rumor without making it true.   
- Initial legends include *He was nobody*, *The thirteenth signal*, *Do not say his name*, *The bounty that was never paid*, *The Reapers save nobody*, *Recruitment*, *The Reaper who does not take every bounty*, *The Reaper rule*, contradictory origins and the responsibility/necessity doctrine.   
- Incompatible origin stories stay deliberately **contradictory**; the engine does not arbitrarily choose a canon version.   
- First real hunt-selection logic: bounty value does not influence the decision. Evidence, danger, repeat offenses, active threat and deliberate harm to innocents determine whether a hunt should be accepted.   
- A one-off mistake followed by correction can be refused/de-prioritized. Accepting a target never means automatic execution: capture/neutralization stay preferred when lethal force is unnecessary.   
- Personal recruits gain a lightweight persistent **injury strain** distinct from ordinary fatigue: KO, critical HP and hard boss fights can leave bounded strain that recovers slower and modestly affects morale/readiness.   
- `RecruitedAllyConditionSnapshot` now exposes `injuryStressPercent` and `shaken`; summaries can show that a recruit is still shaken or marked by injuries. This is not a psychiatric diagnosis or automatic permanent debuff.   
- New `ReaperDoctrineSystemTest`; `RecruitedAllyConditionSystemTest` also covers slower injury-strain recovery.   
- Technical validation is limited in this environment: full C++23 rebuilds did not finish inside the available execution windows, although no compiler error was emitted before timeout. New tests are wired into the runner, but this pass does **not claim** a full green suite.   
- Save schema **26**, checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged. Guild-loan/debt/overdraft/collection development and story remain paused.   

## V3.51.62 — Living recovery leads, criminal lifecycle, world rumors, and lightweight recruit condition   

- Battlefield recovery leads no longer remain frozen in one location. After repeated failed searches, an exact protected lot may move from the **field** to a **nearby cache**, be **secured by the guard**, be **picked up by a civilian**, or end up with an **as-yet unidentified opportunistic holder**.   
- Custody/location changes are persistent (`custodyState`, label, movement day/count, known-location flag) and historically recorded. The engine never invents a named thief or rival merely because an item moved.   
- When the guard has physically secured a lot, recovery is truly **100% guaranteed**; generic disturbance penalties can no longer make already protected property randomly vanish.   
- Criminal contacts now have a richer **global lifecycle**: known fence/accomplice role, provisional-detention count, releases, returns after fleeing and their dates. Releases and returns become persistent events rather than silent timers.   
- Leaving provisional detention may reduce pressure and slightly affect cooperation; returning after flight preserves the contact’s prior history. Permanent arrest remains reserved for future explicit evidence/conviction logic.   
- New generic `WorldRumorSystem`: any faction, city, creature or person can carry rumors/legends with source, credibility, evidence level, confirmations and contradictions. Character history now exposes **Rumors & legends** without converting folklore into omniscient truth.   
- Sources are differentiated (tavern, traveler, guild, guard, archive, witness, direct observation, unknown source) and affect credibility. Repeated transmissions can strengthen, confirm or contradict a story.   
- REAPERS-13 merely uses this generic layer: directly observing an operation may confirm the factual core that the faction exists and operates, without validating every story about its methods or leader.   
- Design clarification: **REAPERS-13 is one of the world’s major factions/clans**, autonomous and often perceived from outside as clandestine operatives or masked vigilantes. Its members follow their own principles/codes. The faction may carry substantial mystique, rumors and legends — especially around Reaper-13 — while remaining independent from the main story and never monopolizing the game.   
- V3.51.61 rarity safeguards remain active: natural traces at roughly 2% only in plausible dangerous biomes, at least 12 days between natural observations, and no free recruitment/grade/roster. No major Reaper story arc was developed in this pass.   
- Personal recruits gain a **lightweight persistent condition state**: fatigue, morale and readiness are reconstructed from actual experience. Real combat adds bounded fatigue based on turns taken, remaining HP, KO state and boss pressure; world-day rest naturally removes about 18 points per day.   
- Condition only nudges ally initiative, mood and readiness; it does not create punitive micromanagement, extra turns or override operational-leader priority. The team menu now displays a readable condition summary for each personal recruit.   
- Employer-team coworkers deliberately retain their existing mood/loyalty model rather than being artificially merged with personal-recruit condition.   
- `BattlefieldRecoverySystemTest`, `CrimeContactLifecycleTest`, `WorldRumorSystemTest`, `RecruitedAllyConditionSystemTest`, `ReaperWorldSystemTest` and `SaveRoundTripTest` cover the new behavior/persistence; the global script explicitly waits for them and adds structural regression guards.   
- C++23 build and full suite **before and after bump: OK / every Dinotofu test passes (`TEST_EXIT=0`, then `POST_BUMP_TEST_EXIT=0`)**. Bump `3.51.61 -> 3.51.62` used `--no-commit`. Save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** remain unchanged. Guild-loan/debt/overdraft/collection development and story remain paused.   

---   

## V3.51.61 — Deferred recovery, persistent criminal memory, and deliberately secondary REAPERS-13   

- Exact **Quest / Personal / Unique** items identified but left behind after an ordinary post-KO extraction can now be stored in a persistent `PlayerBattlefieldRecoveryCache` tied to the city, combat context and battlefield label instead of being only narrated as abandoned.   
- A local **Recovery Leads** location is exposed only when an unresolved cache exists in the current city. Returning to the area is a real action rather than automatic loot teleportation.   
- Deferred recovery currently costs 2 world-time units. Base chance is 100% on the same day/J+1, 90% through J+3, 75% through J+7 and 60% afterward. Failed searches create disturbance/relocation state while prior searches provide a small retry knowledge bonus. Values remain subject to playtesting.   
- Protected exact items are never silently deleted just because the lead ages. A future credible NPC/rival/cache handoff must exist before the engine may truly move or lose such property. Ordinary unprotected surplus still remains abandoned so every battlefield does not become permanent storage.   
- Recovery caches preserve the **same objects** across save/load: persistent ID, durability, enchantments, ownership binding and recovery metadata. Resolving a cache cannot duplicate the same item on a second attempt.   
- Receivers/accomplices now have **global per-contact memory** across theft cases: first sighting, interactions, observations, negotiations, guard confrontations, pressure, cooperation, fleeing, guard reporting and detention persist beyond a single case.   
- A contact who fled therefore cannot instantly reset in a new case, and prior pressure/cooperation carries into later cases involving that same world contact.   
- Judicial logic was corrected: a successful guard confrontation may create **time-limited provisional detention**, but no longer marks a fence as permanently arrested by default. `arrested` is reserved for a future truly established long-term arrest; current detention uses persistent `detainedUntilDay`.   
- **REAPERS-13 is now explicitly locked as a secondary, optional world faction.** It is not the main cast, not the main story, and not required for ordinary progression. A normal run should be able to encounter it rarely or not at all.   
- Natural REAPERS-13 sightings are restricted to plausible dangerous biomes, use only about **2%** chance and require at least **12 world days** between natural observations. Safe/ordinary biomes cannot produce natural sightings regardless of RNG.   
- A deliberately transmitted Protocol 13 signal remains a contextual exception with its own response chance, but still never grants free recruitment, grade, roster or party members. Reaper roster/mission expansion is intentionally not prioritized in this pass.   
- New `BattlefieldRecoverySystemTest`; `SaveRoundTripTest` covers exact cache + criminal memory + detention; `ReaperWorldSystemTest` protects safe-biome exclusion, the 12-day natural cooldown and no free recruitment. Structural guards were added to the full test script.   
- C++23 build and complete suites **before and after the bump: OK / all Dinotofu tests pass (`TEST_EXIT=0`, then `POST_BUMP_TEST_EXIT=0`)**. Bump `3.51.60 -> 3.51.61` used `--no-commit`. Save schema **26**, important checkpoint **V3.51.15** and title catalog **V3.51.51** remain unchanged. New finance debt/loan work and story writing remain paused.   

---   


## V3.51.60 — Broad pass: exact post-KO loot, ally initiative, criminal pressure and first REAPERS-13 world presence   

- Ordinary waves completed while the player is KO no longer select abstract enemy bundles before loot generation. For every defeated enemy, the engine now generates the **exact loot** it would have produced in an isolated staging inventory: materials, physical currency, exact weapon and exact armor when their normal rolls succeed.   
- The post-KO transport budget is applied **after** detailed generation. Every exact candidate has a real transport cost and goes through `selectPostKoRecoveryCandidates`: Quest/Personal/Unique and explicit recovery priority stay protected first, then the group favors candidates that actually fit the remaining capacity.   
- Post-KO reporting now distinguishes the exact recovered and left-behind items; canonical event `butin_post_ko_transport` records exact candidate/recovered counts and available capacity.   
- **Intentional remaining limitation:** an important item that cannot be carried is now identified exactly instead of being mixed into generic loot, but ordinary battlefields do not yet provide a persistent container that lets the player return later for it. Battlefield persistence must exist before generic deferred recovery can honestly be promised.   
- New `RecruitedAllyInitiativeSystem`: active recruits now have a minimum initiative order inside their own allied action window. Level/rank/weapon quality matter modestly; scout/fast/agile profiles are favored while tank/heavy/slow profiles are penalized.   
- Ally initiative is used in standard PvE and boss fights without moving the enemy response, adding actions or bypassing reserve-entry delay. Operational leader priority remains explicitly stronger than raw speed.   
- The vault-theft network gains a real **guard confrontation** route for a sufficiently documented human fence/accomplice (at least 4 clues). Guards act on observed facts and never automatically turn the current holder into the original burglar.   
- Theft cases can now persist confrontation attempts, pressure, cooperation, guard reporting and temporary contact closure. A credible confrontation may produce more clues, cooperation and sometimes return the property; failure may alert the contact and temporarily close the local lead. All of this survives save/reload.   
- First real **REAPERS-13 world layer**, independent from cheats: exploration can very rarely expose an unknown disciplined targeted operation. This only creates credible knowledge/observation; it reveals no secret roster, grades or internal objectives and offers no recruitment.   
- A character who legitimately transmitted `DINOTOFU:13-02313` can later, rarely, receive a **physical Echo Unit relay response** during exploration. The relay challenges how the character knows the channel and then leaves; no rank, free teammate, victory or faction access is granted. The response is recorded and the signal is no longer pending.   
- Added `RecruitedAllyInitiativeSystemTest`, `PostKoExactWaveLootTest` and `ReaperWorldSystemTest`. `CityVaultSecurityLogisticsTest` covers guard confrontation; `SaveRoundTripTest` covers the new persistent criminal state. Structural guards also protect exact post-KO loot, initiative wiring, natural Reaper presence and the no-auto-recruitment rule.   
- C++23 build and full suite **before and after bump: OK / all Dinotofu tests pass (`TEST_EXIT=0`, then `POST_BUMP_TEST_EXIT=0`)**. Save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** remain unchanged. Loan/debt/overdraft/collection development and story remain paused.   

---   


## V3.51.59 — Broad pass: priority items, post-KO recovery, interactive crime network and boss rotation   

- All `Item` descendants now support generic **Unique / Quest / Personal** recovery traits plus an explicit **0–100 recovery priority**, intentionally separate from soul binding. Binding controls ownership transfer; recovery traits control how strongly an item should be protected from automatic abandonment.   
- Automatic priority floors are 90 for unique objects, 95 for personal property and 100 for quest objects. Traits can be combined without making the item artificially non-transferable.   
- Recovery metadata persists across inventory, municipal vaults, exact Grinka-stolen equipment and exact equipment stored in burglary cases. Older saves default to ordinary/priority 0 without changing save schema 26.   
- Material stacks with the same ID/quality no longer merge when recovery metadata differs, preventing a protected stack from losing its semantics through stacking.   
- Vault burglary now uses the generic metadata: **Quest** and **Personal** objects are excluded from generic theft; **Unique** or high-priority transferable objects remain theoretically stealable but are made extremely unlikely instead of being confused with soul binding. Legacy text heuristics remain only as compatibility fallback for older content.   
- New generic `selectPostKoRecoveryCandidates` orders exact objects under limited physical extraction capacity: protected objects first, then recovery priority, then transport cost. Non-carried exact objects stay deferred/registered where the owning system supports later recovery rather than silently disappearing.   
- Exact weapons/armor confiscated by Grinka are now stored as **Personal / priority 100**, and post-KO recovery uses the generic selector. If capacity is insufficient, the exact object remains recorded with Grinka for later recovery.   
- Main boss fragments are now real **Unique / priority 100** objects in the engine rather than merely being described as unique in text. The metadata follows them through inventory and saves.   
- The burglary network is now truly **interactive**. When a case leads to a world-anchored fence/accomplice/cache, the vault offers a local-lead action where the player can observe, negotiate a return, or search a cache depending on holder type and evidence.   
- Observation consumes time and adds 1–2 evidence points without omniscient identity reveals; a conspicuous approach can alert the contact. The same case cannot be forced repeatedly during one world day.   
- With at least 3 clues, a fence/accomplice may accept a **negotiated return** for a bounded fraction of estimated property value. Money is charged only on success, and returning property is never treated as an automatic confession of burglary.   
- A sufficiently evidenced cache can be searched. Success returns still-traceable property without fabricating a human culprit; failure can still confirm another lead. Observation/negotiation counts, contact alert state and last interaction day are persistent.   
- **Manual player-leader rotation against bosses** is now available to personal teams. With reserves available, the player may sacrifice their turn to swap 1 or 2 recruits; a double swap is atomic, while a rare critical-member refusal keeps the normal action but locks another rotation attempt until the next player turn.   
- Recruits entering from reserve through this boss rotation use the same no-free-turn delay as other rotation paths. Existing active recruits may still act, then the boss keeps its normal next phase; rotation never moves enemy initiative.   
- `SaveRoundTripTest` covers recovery metadata and crime-contact interaction state; `RecruitedAllyAftermathSystemTest` covers capacity-limited priority selection; `CityVaultSecurityLogisticsTest` covers negotiated return and cache search without fake rivals; `GrinkaEquipmentRecoveryTest` remains green with the generic selector. Structural guards also protect boss rotation and persistence.   
- Known remaining limitation: ordinary post-KO waves still choose **abstract enemy bundles** before all exact drops exist. The exact-object selector is now ready and used wherever persistent objects already exist; migrating every wave to exact candidates remains a separate refactor to avoid destabilizing off-screen loot.   
- C++23 build and the full suite **before and after the bump: OK / all Dinotofu tests pass**. Save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** remain unchanged. New loans/debt/overdraft/collection work and story remain paused.   

---   

## V3.51.58 — Broad pass: fencing network, in-game morphology and logistics titles   

- Stolen property moving through a **receiver, accomplice or cache** is no longer represented only by an abstract holder label. A new `VaultTheftNetworkCatalog` anchors these roles to coherent contacts and locations in each city: Valebrume market/gate, Rocheveille forge/rails, Port-Lanterne harbor/underbridge market, and Lys-de-Givre gate/market/temple routes.   
- Burglary cases now retain a world-contact ID, location ID/label and associated exploration biome. These optional fields survive save/reload without changing save schema 26.   
- Direct recognition of exact stolen equipment now respects the linked place: attempting exact recognition in the wrong biome is downgraded to a weak movement trail. Internal knowledge can no longer make the exact property appear at an impossible location.   
- A located cache keeps a real world location while still never fabricating a human rival. Identified receivers/accomplices can instead use their world-facing name and location as the investigation anchor.   
- First real **in-game morphology progression**: Fine / Standard / Robuste are no longer permanently frozen at character creation. At an inn, the player may voluntarily start a program toward one adjacent tier only (Fine ↔ Standard ↔ Robuste), never a direct Fine ↔ Robuste jump.   
- A program currently requires **8 distinct sessions** and at least **14 world days**. Only one session counts per day; each session represents training plus sustained nutrition, costs 35 copper and consumes 2 world-time units. Values are explicitly meant for playtesting rather than instant transformation.   
- Until completion, the current morphology and all of its effects remain unchanged. Cancelling changes nothing. Final validation applies the new morphology, after which `CarryLoadSystem` immediately uses it for capacity/hitbox/mobility.   
- Current morphology, target, sessions, start day and last session day are persisted through backward-compatible optional fields. Older saves still derive Fine/Standard/Robuste from their creation variant when no explicit in-game morphology state exists.   
- Previously dormant morphology titles now react to real body changes: `Un corps façonné`, `Plus solide qu'avant`, `Taillé pour porter`, `Plus léger qu'avant`, `Affûté`, and after multiple real transformations `Silhouette travaillée`. The title catalog itself is unchanged and remains V3.51.51.   
- Character Statistics now show the current morphology and, when active, the program target and session progress.   
- Four dormant storage/logistics titles are now connected to **real session events** rather than one-frame snapshots: `Grand ménage` for a large real vault unloading session, `Ça respire enfin` for dropping from >150% load to <50%, `Je garde tout` for returning from exploration at 180–199% with full pickup while never crossing the 200% hard block, and `Déménageur` after carrying at least four spare heavy equipment pieces during one expedition without reaching 200%.   
- These title evaluations are centralized on `Player` so they can be tested independently of UI code. `StorageTitleSessionTest` protects the thresholds and specifically prevents exploration titles after a 200%+ peak.   
- `CityVaultSecurityLogisticsTest` now protects receiver/cache world anchoring, wrong-biome exact-recognition rejection, and correct-location identification. `MorphologyProgressionTest` protects adjacent tiers, 8 sessions/14 days, no early transformation, cancellation and titles. `SaveRoundTripTest` covers persistent morphology programs and crime-location fields.   
- `scripts/test_project.sh` explicitly waits for the new tests and adds structural guards for the crime catalog, persistence, session-title hooks and morphology display.   
- C++23 build and complete suite **before and after the bump**: **OK / all Dinotofu tests pass**. Save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** remain unchanged. New loan/debt/overdraft/collection work and story writing remain intentionally paused.   

---   

## V3.51.57 — Burglary leads encountered during exploration   

- An unresolved municipal-vault burglary case can now produce a **credible chance field encounter during exploration** tied to the same city. Investigation is no longer limited to the vault menu.   
- The scene is contextual and uncommon: roughly **28%** in human-travel areas (road, plain, market, district, ruins) and **14%** elsewhere, and only while a local case still has no identified target.   
- Three observation types are supported: **movement trail**, **suspicious exchange**, and the rarer **direct recognition of distinctive stolen equipment**. A trail/exchange gives 1 clue; genuinely recognizing the exact stolen weapon/armor gives 2.   
- Recognizing property never automatically proves that its current carrier committed the burglary. The UI explicitly treats that person as a holder/possible accomplice or fence until accumulated evidence reaches the existing credible identification threshold.   
- Generic currency/materials/consumables are never claimed to be exactly recognized without evidence; those scenes only mention packaging, cargo shape or behavior consistent with the case.   
- A **secondary cache** can now be located without fabricating a human rival. Reaching the clue threshold while the property is cached does not invent a person to hunt; a later real holder or additional trail is required.   
- A persistent per-case cooldown prevents farming the same field lead repeatedly during one in-game day. The scene consumes no extra time unit because it is part of the exploration already underway.   
- `CityVaultSecurityLogisticsTest` covers weak trail/no rival, exact weapon recognition leading to identification only after enough clues, wrong-city rejection, and no fabricated target for a cache. `scripts/test_project.sh` also protects the exploration hookup, cooldown and anti-omniscience rule.   
- C++23 build and full suite: **OK before and after bump**. Save schema **26**, checkpoint **V3.51.15** and title catalog **V3.51.51** unchanged. New loan/debt/overdraft/collection development remains paused; story remains paused.   

---   

## V3.51.56 — Generic persistent ownership binding   

- Every object inheriting from `Item` now carries real generic ownership metadata: **transferable** or **soul-bound**. Transferability no longer depends on names, descriptions or textual enchantment heuristics.   
- `canChangeOwner()` centralizes the rule: a bound item may still move between storage locations owned by the same character, but it cannot be given, stolen, confiscated or transferred to another owner.   
- Character-to-character exchanges now reject bound weapons, armor, consumables and materials, with an engine-level guard even if a UI path attempts to bypass filtering.   
- Vault burglary, theft while escaping, Grinka confiscation and PvP equipment taking all use the same generic rule. A vault therefore no longer creates a false burglary case when no actually transferable property was taken.   
- Transfers between the same character's municipal vaults remain valid for bound items because they are storage moves rather than ownership changes.   
- Bound and unbound material stacks with the same ID/quality remain separate so stacking can never erase or dilute ownership protection.   
- Ownership binding is persisted for inventory, vaults, stolen equipment and burglary cases. Older saves without the optional field remain compatible and load items as transferable by default; save schema remains **26**.   
- `SaveRoundTripTest`, `ExchangeTransferSystemTest` and `CityVaultSecurityLogisticsTest` cover persistence, stack separation, transfer rejection, burglary protection and same-owner vault transport.   
- `scripts/test_project.sh` now structurally protects this metadata, cleanly skips GitHub-workflow checks when `.github` is absent from an essential backup, and explicitly waits for `TitleCatalogUpdateTest` before temporary test files are cleaned.   
- C++23 build and the complete suite pass **both before and after the bump**. No new loan/debt/overdraft development resumed. Title catalog **V3.51.51**, checkpoint **V3.51.15** and save schema **26** remain unchanged.   

---   

## V3.51.55 — Vault defense aligned with active run rules   

- Municipal-vault defense no longer uses the hidden **Normal / non-definitive death** fallback. It now receives the actual active `DifficultyMode` and `DeathRuleMode`.   
- The combat context is propagated from `Game` through locations, the road relay, the city hub and finally the vault, so reaching the vault through a secondary navigation path can no longer silently change combat rules.   
- `MonsterPveMode::runExplorationWave` now receives the active difficulty/death rule directly for vault defense. Existing centralized difficulty multipliers, penalties and lethal/permanent-death behavior therefore remain consistent with other combat paths.   
- `QuestMenu::openLocations`, `QuestWorldMenuSupport::openCityTravelMenu` and `openCityVault` now expose this context explicitly instead of relying on a hidden global state or implicit default.   
- `scripts/test_project.sh` protects the propagation chain and fails if the old `DifficultyMode::Normal / DeathRuleMode::NonDefinitive` pair returns to this path.   
- C++23 build and the complete engine/test suite pass. The supplied essential backup did not contain `.github`, so only the four structural assertions targeting the GitHub workflow were skipped while resuming from that archive; all Dinotofu code tests pass.   
- No new loan/debt/overdraft development resumed. Save schema stays **26**, important checkpoint **V3.51.15**, and title catalog **V3.51.51** unchanged.   

---   

## V3.51.54 — Unknown vault burglars, investigations and stolen-property circulation   

- A vault robbed while the player is away no longer magically creates an identified rival. If the courier deadline is ignored and property is actually stolen, the engine now creates a **persistent burglary case with an unknown culprit**.   
- The case keeps real stolen property: currency, a material stack, a consumable and up to an exact equipment item with its `persistentId`, durability and enchantments. Automated vault theft therefore no longer turns equipment into unrecoverable text.   
- Stolen-property selection is now more nuanced: abundant/common stocks remain strongly favored, but transferable weapons and armor can also be taken; higher value/enchantment sharply lowers their selection weight. Items explicitly detectable as soul-bound/non-transferable are excluded.   
- New **Investigate burglary** option in the affected city's vault. Investigation consumes local time and gathers clues from witnesses, tracks, guards and fences.   
- The culprit is revealed only after enough credible evidence. At 3 clues a target can be identified and only then becomes a persistent rival/trackable target. Before that, no name or identity is exposed to the player.   
- Stolen property can **change hands or caches over time** if the player waits: accomplice, fence or secondary cache. This can reduce certainty and force the investigation to find the current trail again without deleting the property from the world.   
- Defeating the rival currently linked to the case recovers still-traceable property; `Give it back.` / `Rends-moi ça.` remains awardable through this recovery.   
- The engine now clearly distinguishes **opportunistic escape theft** from **vault burglary**: fleeing thieves grab what is quickly accessible while a burglar has time to search and may discover more important property.   
- `SaveRoundTripTest` covers persistence of an unknown theft case and an exact stolen weapon; `CityVaultSecurityLogisticsTest` covers unknown culprit → investigation → identification → recovery.   
- REMAINS: add a real generic `soul-bound/non-transferable` item metadata instead of relying partly on textual signals; allow credible random exploration encounters with an as-yet unidentified thief; later connect fences to real NPCs/locations when that world layer is rich enough.   
- Vault defense still uses the isolated-menu Normal / non-definitive death fallback; this remains a priority for a later pass.   
- No new loan/debt/overdraft development resumed. Save schema stays **26**, important checkpoint **V3.51.15**, title catalog **V3.51.51** unchanged.   

---   


## V3.51.53 — Exposed vaults, lawful defense and shared couriers   

- Daily vault-raid risk now targets a meaningful but still random average frequency: **1.5% per away day** for a non-empty, uncovered vault outside the player's local presence, or roughly **one alert every 67 away days on average**. The courier warning and 7-day response window remain mandatory before consequences.   
- Additional vault purchase prices now follow the agreed cumulative progression: first vault **2 PP**, then successive increments of **+2 PO, +3 PO, +4 PO, +5 PO...**. The first prices are therefore **2 PP; 2 PP 2 PO; 2 PP 5 PO; 2 PP 9 PO; 3 PP 4 PO...**.   
- The mayor discount on the very first vault is substantially stricter: the character must already have at least **1 PP** of total liquid monetary wealth across the physical purse and actually deposited guild balance, then satisfy local service requirements. Roughly **15 ordinary local missions/services** or **4 missions from an important local character** are required in addition to reputation; the discount tiers remain 1.5 PP and 1.25 PP for an exceptional record.   
- Quests actually issued by notable local NPCs are now recorded locally under `quetes_personnage_important`, so the discount is based on credible service rather than only a pile of anonymous small jobs.   
- A successful vault defense is now treated as an intervention under municipal authority: the player receives only about **40% of the lawful reward and ordinary attacker loot**, with the remainder turned over to the authorities. Goods that already belonged to the player and are recovered from thieves do not count toward this limit.   
- A temporary **Municipal Guard** still takes no equipment or items. Because the guard physically participates in combat, their presence only reduces final XP by about **10% more**, without further reducing the lawful material share.   
- Loot generation now supports an authorized recovery percentage: at 40% defense rights only a coherent subset of defeated attackers yields standard loot, and the interface explains that the rest is seized/sealed by the city.   
- When a raid succeeds or its deadline is ignored, thieves prioritize **large stacks of common, low-value materials** plus a small share of physical currency. Quest items, keys, information and notable goods remain protected; rare/artifact-like materials only become candidates as a last resort with a deliberately tiny probability.   
- Thieves that successfully steal while escaping now use the same preference logic: a large common stack is favored over a rare resource. Protected items remain excluded.   
- Weapons/armor are still never silently deleted by automated vault raids: before enabling that case, the engine must be able to attach the exact persistent item to a recoverable thief/rival instead of turning bad RNG into permanent deletion.   
- Inter-city **recall orders** can now share the initial courier fee. Multiple requests on the same route launched within roughly **2 time units out of 5 per day** reuse the courier already sent and pay only a small administrative surcharge; after that window a full new courier must depart.   
- Recall courier state (time, source city and destination city) is persisted. Cost previews state when the courier is shared; save/reload cannot lose or artificially recreate the discount.   
- `CityVaultSecurityLogisticsTest`, `EconomyScaleTest`, `RivalEscapeTheftRecoveryTest` and `SaveRoundTripTest` cover the new prices, discount thresholds, theft priorities, lawful defense and courier sharing.   
- No new loan/debt/overdraft development is resumed. Save schema remains **26**, important checkpoint **V3.51.15**, and the title catalog remains **V3.51.51** because no title definition changes in this pass.   

---   

## V3.51.52 — Patrimonial vaults, local security and inter-city logistics   

- Municipal vaults are now a true **patrimonial investment**: the normal first-vault price is **2 PP** (2000 PF). Every additional vault bought in another city adds at least **+2 PO** to the purchase price: 2 PP, 2 PP 2 PO, 2 PP 4 PO, and so on. Vault purchase price no longer uses the city economic multiplier.   
- The mayor may discount only the character's **first vault**, based on real local history: good reputation plus several completed local missions/services lowers it to **1.5 PP**; exceptional reputation plus a stronger local record lowers it to **1.25 PP**. Once any vault exists, later purchases use the normal network price.   
- Capacity upgrade prices were rebalanced around the higher acquisition cost: N1→N2 **1 PP**, N2→N3 **1.5 PP**, N3→N4 **2.25 PP**, N4→N5 **3.5 PP** before any local upgrade multiplier. `Optimized space` currently costs **1 PP** and `Prepared storage` **2.5 PP**; values remain subject to playtesting.   
- New per-vault **Security and maintenance** service. It buys 7 protected days that are consumed only while the character is away from that vault's city; the counter is therefore **paused while in the city and during local exploration tied to it**. Current cap: 35 protected away-days.   
- Current security price by vault level: **30 / 40 / 50 / 60 / 70 PF per 7 away-days**. It is meant to be a small delegation/guarding expense, not a second acquisition tax.   
- A remote, unprotected, non-empty vault now has a deliberately rare current chance (**0.5% per remote day**) to attract a burglary attempt. Nothing is lost immediately: an **urgent messenger** warns the player and opens a **7-day** response window.   
- Burglary alerts are persistent through save/reload: remaining security days, active alert, deadline and last check are serialized. A pre-bump fix added the missing JSON write path after tests caught that loading already supported the fields while saving did not.   
- Once the player returns to the affected city, a **Defend vault** option becomes available. Encounters are mostly level-scaled humanoid thieves/raiders, with a low current chance of a monster incident because ordinary city security is assumed to handle most monster threats.   
- If the party is not full, a defense can receive temporary help from a **Municipal guard**. The guard fights physically but is never recruited, never persisted as a companion and **takes no reward share**; a dedicated test prevents reward dilution.   
- Winning closes the alert. Losing does not wipe the vault: the first current consequence is a small loss of accessible physical currency (about 5%). Ignoring the full deadline may lose about 10% of stored currency. Unique/quest items and equipment are never silently deleted by this layer until generic item metadata is reliable enough.   
- Buying security while an alert is already active **does not retroactively cancel the attack**; the current alert still needs to be defended, while the coverage protects future days.   
- Inter-city logistics are now bidirectional and general: **Send** from the current vault or **Recall** from a remote vault for weapons, armor, consumables, material stacks and physical currency. Recall costs slightly more and takes longer because a paid messenger must first carry the request before the return convoy.   
- Convoy cost depends on distance and the **real cargo load**: shipping heavy armor costs more than a compact parcel. Physical currency keeps its denominations, while weapons and armor preserve their exact persistent identity.   
- Before transport confirmation, the UI shows **source load before → after** and **destination load before → after**, plus cost and duration. Transfers respect real destination capacity and storage optimizations, and remove the source object before adding it to the destination.   
- `CityVaultSecurityLogisticsTest` covers mayor discounts, +2 PO network progression, local/remote subscription behavior, protected/unprotected alerts, two-way transfer, exact weapon identity and physical coin denominations. `SaveRoundTripTest` now covers security/alert persistence; `RecruitedAllyCombatRosterTest` protects the temporary municipal guard's zero reward share.   
- No new loan/debt/overdraft/collection development resumes. Global economic balance auditing remains active. Save schema **26** and checkpoint **V3.51.15** remain unchanged.   

---   

## V3.51.51 — Full carry-load rules, backpacks, optimized vaults and morphology   

- Carry load is now visible as `INVENTORY [used/capacity]`, with a one-time warning when crossing 85%. The system stays abstract and intentionally avoids kilogram/Tetris micromanagement.   
- Backpack base capacities are **40 / 50 / 60 / 70 / 80**. Race, chosen morphology, Strength and pure-mage profile make modest adjustments; the object-centric Bibliomancer is exempt from the pure-mage carrying penalty.   
- Shared load values now cover weapons, armor, ammunition, consumable stacks, material stacks and physical money. **100 actual coins = 1 load unit**, regardless of denomination value. Equipped weapon/armor are not double-counted inside the bag.   
- Overload thresholds are now meaningful: 101–125% gives `-10%` escape, `-5%` melee accuracy, `+2%` physical protection and `+10%` travel/exploration time; 126–199% gives `-20%`, `-10%`, `+4%` and `+25%`. At **200%+**, movement to a new travel/exploration area is blocked until the character lightens the load, without introducing a third harsher combat tier.   
- Natural racial body size and chosen **Fine / Standard / Robust** morphology are separate layers. Old visual Variant A/B saves remain compatible. Size/morphology now modestly affect carrying capacity and hitbox.   
- Exploration summaries display carry load **before → after** plus the highest load reached during the run. Travel and exploration consume the actual overload-adjusted world time.   
- Municipal vault capacities are now **250 / 300 / 350 / 400 / 500** shared load units. The base Valebrume purchase remains a meaningful medium-term **350 PF** target under the existing local-price multipliers.   
- **Optimized Space** becomes available from vault level 2 and subtracts exactly 10% of base stored load. **Prepared Storage** becomes available at level 5 after it and subtracts another 15 percentage points from the same base, for an exact **25% total reduction** rather than multiplicative stacking.   
- Vaults can persist exact physical coin stacks. Deposit/withdraw preserves denominations and large saved stacks now use 64-bit parsing.   
- Dinotofu can suggest a vault once the bag is at least 85% full and none is owned; if the player is too poor, the advice becomes selling/sorting first.   
- Title catalog version is now **V3.51.51** with the accepted storage/load/backpack/morphology title family, including `Conservateur`, `Réseau de stockage` and `Minimaliste`. Mutually exclusive lifestyle titles do not count toward ultimate completion.   
- `Minimaliste` checks departure below 10%. `L'essentiel seulement`, `Voyageur organisé` and `Préparation impeccable` use the real peak load during the run. `Ça pèse moins comme ça` is connected to a voluntary coin exchange that strongly reduces physical coin bulk.   
- Long, voluntary in-run morphology changes through training/food remain future work. Their titles are catalogued but intentionally dormant until a real opt-in transformation system exists.   
- Fixed a load heuristic where the generic French word `composant` accidentally matched `os` and was treated as dense bone material.   
- Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.50 — Update notes   

- Version synchronization placeholder. Replace with detailed release notes before publishing.   

---   

## V3.51.49 — Simplified post-KO carrying and exact Grinka property recovery   

- Carry weight remains deliberately **outside normal player micromanagement**: no everyday kilogram inventory meter, no permanent encumbrance gauge and no routine movement penalty because a conscious player picked up a few extra items. Carry units remain an internal resolver for automatic post-KO extraction only.   
- The post-KO loot screen no longer exposes raw `x/y carry units` by default. Capacity is summarized qualitatively while the numeric budget stays internal for deterministic resolution and canonical records.   
- Grinka now uses the same physical extraction logic when allies defeat her while the player is unconscious. Previously confiscated personal property is searched before ordinary loot.   
- A recovered Grinka weapon/armor is the **exact persisted object**: same individual persistent ID, durability and enchantments. No equivalent clone is rebuilt.   
- If the survivors cannot carry a confiscated item together with the unconscious player, that item remains registered with Grinka for a later recovery instead of being deleted.   
- Current automatic Grinka extraction priority: confiscated personal equipment -> notable boss fragment -> confiscated money. Money is treated as one compact pouch to avoid coin-by-coin encumbrance simulation.   
- A conscious personal victory stays simple and restores the confiscated property normally without a carrying minigame. The existing **“Rends-moi ça.”** title is now wired to the real recovery of an exactly confiscated weapon or armor. Title catalog version remains V3.51.40 because the title definition itself did not change.   
- New selective `recoverGrinkaStolenWeapon/Armor/Gold` paths allow partial recovery without clearing unrecovered property and prevent duplicate restoration.   
- `GrinkaEquipmentRecoveryTest` covers persistent identity, durability, enchantments, partial recovery, money and duplicate prevention.   
- Zelef/Lyknir audit: their persistent consequences remain separate from Grinka/property recovery and post-KO salvage. No current post-KO path recreates equipment destroyed by another mechanic or silently clears a curse. More detailed equipment-corrosion/curse auditing remains for the future once those states live directly on items.   
- Guild loan/debt development remains paused while global monetary balance is still audited. Save schema **26** and important checkpoint **V3.51.15** remain unchanged.   

---   

## V3.51.48 — Progressive player escape and real post-KO carrying capacity   

- Player escape now follows the same progressive logic already used by monsters: against several blockers, one attempt no longer has to solve every opening in the same turn. A successful but incomplete opening remains acquired for the current fight and currently grants `+18%` per preserved opening, capped at `+36%`, on the next attempt.   
- A player escape attempt remains a real action: partial progress, a blocked opening or complete retreat all consume the turn. Progress is transient and reset when a new combat starts; no save-schema field is required.   
- Player body/race now slightly affects disengagement in addition to class and equipment: naturally small/mobile profiles such as Fairy/Kitsune/Gnome/Halfling gain an advantage, while Orc/Half-Dragon/Dwarf profiles are less mobile. Values remain deliberately modest and need playtesting.   
- `EscapeOppositionPressureTest` now also covers multi-turn player progress and verifies that a Fairy profile receives a better mobility modifier than an Orc profile. Numerical opposition pressure remains shared by both camps.   
- Post-KO loot transport is no longer based only on raw survivor count. Every conscious survivor contributes an **effective carrying capacity** based on health and profile/body size: badly injured or very small allies carry less, while robust/warrior/carrier profiles can carry more.   
- Carrying the unconscious player now consumes units from that same capacity. The remainder becomes one shared transport budget for all loot: compact bundles cost 1 unit, then recovered weapons/armor consume 1/2/3 units according to their real light/medium/heavy weight class.   
- Heavy equipment therefore no longer uses an independent pool that could bypass compact-loot weight. Bringing back heavy armor can genuinely prevent the group from hauling additional remains afterwards.   
- `LootGenerator::givePostKoRecoveredWaveLoot` spends this shared budget as recovery proceeds and reports used/remaining capacity; excess loot stays physically abandoned. V3.51.47 elite/evolved/level prioritization remains in place.   
- `RecruitedAllyAftermathSystemTest` now locks an average carrier, two carriers, a robust Orc/warrior carrier and a badly injured small Fairy. General economic auditing remains active: carrying capacity never creates loot or money that did not already exist.   
- No new loan/debt/overdraft development was resumed. No title changed: title catalog remains **V3.51.40**. Save schema **26** and important checkpoint **V3.51.15** remain unchanged.   

---   

## V3.51.47 — Progressive escape and physical post-KO loot transport   

- Enemy escape is no longer a flat roll that immediately forgets a successful opening. Monsters now keep **temporary disengagement progress** during combat: against several blockers, one successful opening may create distance without removing the creature from combat yet.   
- Each retained opening substantially improves the next committed escape attempt (`+18%` per retained opening, currently capped at `+36%`). A creature that has already started disengaging therefore has a real advantage when it tries again instead of restarting from zero.   
- Creature mobility now affects escape directly: small/mobile races and agile/light profiles receive bonuses, while constructs, dragons, giants, guardians, tanks and massive/heavy profiles are penalized. Values are bounded and remain subject to playtesting.   
- V3.51.46 numerical pressure remains shared: roughly 5 capable blockers require 2 openings. If the opposition shrinks after a real first breakthrough, already-earned progress is not erased artificially.   
- A monster escape attempt is now a **real turn action**. Full escape, partial progress and a blocked opening all consume the turn; a monster can no longer fail an escape attempt and then attack for free in the same phase. Morale-driven flight follows the same rule.   
- `EscapeOppositionPressureTest` now covers multi-turn progress, retry bonuses and the mobility advantage of a small agile profile over a massive/tank profile. The design intentionally follows the same core principles as player/NPC escape: numerical pressure plus profile mobility, with actor-specific modifiers.   
- First layer of **physical post-KO loot transport**: when allies finish a fight while the player is unconscious, recoverable loot now depends on surviving allies who can still carry it instead of assuming that everything teleports into the inventory.   
- Carrying the unconscious player consumes part of that capacity. A lone survivor can bring at most one compact loot bundle in addition to the player; additional survivors increase capacity. Heavy equipment has a separate stricter carrying limit.   
- In standard/exploration PvE waves, only bundles within real carrying capacity are recovered and the remainder is explicitly left behind. Current selection prioritizes elite/evolved and then higher-level defeated enemies so the system does not abandon obviously better salvage first.   
- Materials, small currency and compact rewards remain easier to carry than weapons/armor. Recovered equipment count is limited by remaining heavy-item capacity.   
- Bosses defeated after a player KO also use this transport layer: boss loot is delivered on recovery only when at least one capable survivor can physically bring the bundle while transporting the player. A boss fight only observed from the bench with no personal participation still grants no personal boss loot.   
- `RecruitedAllyAftermathSystemTest` locks capacity for one/two survivors, the cost of carrying the player and exclusion of KO allies. This pass also continues general economic auditing by preventing automatic recovery of every post-KO drop.   
- No new loan/debt/overdraft development was resumed. No title changed: title catalog remains **V3.51.40**. Save schema **26** and important checkpoint **V3.51.15** remain unchanged.   

---   

## V3.51.46 — Observable reserve and symmetric escape pressure   

- The **reserve/waiting line is no longer treated as automatically blind**. In fights where it stays physically beside the scene — current standard PvE, exploration and employer arena/boss contexts — a recruit may directly observe rounds without ever taking a turn. Active participation and observation are now separate concepts.   
- Post-KO knowledge windows now track rounds observed from reserve separately from rounds spent on the active line. A reporter with `0` turns taken may therefore provide a complete direct account if they genuinely witnessed the whole scene.   
- This refines V3.51.45: entering the active line late does **not** necessarily mean learning about the fight late. Credible sideline visibility can provide direct knowledge of earlier rounds; remote/obstructed/absent reserves still keep unknown or second-hand gaps.   
- Observation grants no automatic economic participation: characters with **zero active turns** are excluded from normal combat reward sharing even if they witnessed everything. Observation also creates no artificial clear, kill or personal participation.   
- An employed player who spends an entire boss fight on the bench can now record a real **observed encounter** in bestiary/history when the scene is visible, while still receiving no personal boss clear or reward.   
- Monster escape now uses the same numerical opposition pressure as player escape: the more living opponents can close routes, the more successive openings are required. Monsters no longer keep a flat escape chance independent of pursuer count.   
- Standard PvE/exploration counts a living player, active recruits, physically nearby reserve recruits and active summons as potential escape blockers. Under the current rule, fleeing against 5 opponents requires **2 successive openings**.   
- Enemy morale reactions that choose flight also use this pressure model; failure can leave the creature trapped/panicked instead of removing it from combat for free.   
- `EscapeOppositionPressureTest` locks the scaling with opponent count. Aftermath and boss post-KO tests now also cover a full-time reserve observer with no active participation.   
- No new loan/debt/overdraft development was resumed. Global monetary balance remains audited; this pass specifically prevents spectator-only observers from taking an active combat reward share.   
- No title changed: title catalog remains **V3.51.40**. Save schema **26** and important checkpoint **V3.51.15** remain unchanged.   

---   

## V3.51.45 — Sourced post-KO reports and separate team victories   

- Post-KO reports no longer receive the omniscient result of the whole hidden simulation. Each participant now keeps combat-only observation data: first/last post-KO round actually witnessed, enemies seen falling, whether the final outcome was directly observed, and whether the ally is still available for a debrief.   
- An ally entering late therefore no longer magically knows earlier rounds. If another surviving ally actually witnessed that missing period from round 1, the reporter may identify that ally as a **second-hand source**; otherwise the gap remains explicitly unknown.   
- `RecruitedAllyAftermathSystem::buildKnowledgeReport` centralizes direct observation, sourced second-hand information and unknown gaps. The engine may know the full result without injecting it into character dialogue.   
- Standard/exploration PvE and boss post-KO simulations now populate these observation windows. Fallen allies cannot be selected as reporters, while surviving witnesses can confirm only the ending they actually observed.   
- Multi-phase bosses such as Hitogami keep continuous post-KO round numbering and correctly invalidate the previous “final outcome observed” marker when a new phase begins.   
- RP debriefing and the player’s **economic share** are presented as separate operations. Refusing or limiting a report does not automatically create or erase money owed to the player.   
- New canonical event `team_victories_without_personal_clear` records fights finished by the team while the player is unconscious. This is explicitly separate from a personal clear and grants no free title/clear.   
- `RecruitedAllyAftermathSystemTest` now covers late reporters, round-1 direct sources, missing credible sources and a witness covering the entire hidden phase. `RecruitedAllyBossPostKoTest` also validates observation windows produced by real boss simulation.   
- New loan/debt/overdraft development remains paused. General monetary balance is still audited, especially the separation between team rewards, the player share and no-free-clear outcomes.   
- No title changed: title catalog remains **V3.51.40**. Save schema **26** and important checkpoint **V3.51.15** remain unchanged.   

---   

## V3.51.44 — No-free reserve entry and minimum combined-technique recovery   

- The hard rule **“entering from reserve never grants a free turn”** is no longer boss-only. It is centralized in `RecruitedAllyCombatRoster` through a transient marker consumed exactly once.   
- A player-leader rotation performed before the allied phase marks every incoming reserve member: that ally skips only the immediate action window, then returns to normal pacing on the following turn.   
- Employer-leader rotations in standard PvE follow the same rule. A teammate called in during the leader phase can no longer act again a few moments later during the non-leader phase of the same round.   
- Individual requested/autonomous switches remain distinct: they already consume the outgoing member's own turn, so the replacement is not punished with an artificial second lost turn.   
- Boss combat now reuses the same reserve helper instead of maintaining a parallel rule. `RecruitedAllyCombatRosterTest` locks the exactly-once delay behavior.   
- Combined techniques now have an explicit **combat recovery window** independent from ordinary personal-technique readiness. After every real attempt — full, partial or failed — the immediate enemy response plus one complete intervening round must pass before another attempt.   
- The orders menu displays that recovery and disables “Combined technique” while the pair is still recovering. An ally that just entered from reserve also cannot bypass its initiative delay through a combo.   
- `DuoMasterySystem` exposes the pacing rule as testable helpers: recovery after attempt = 2 enemy responses, then 1, then 0/ready. `DuoMasterySystemTest` covers that progression.   
- This blocks `A -> B -> A -> B` style allied loops before a real enemy answer without arbitrarily moving enemy initiative or deleting enemy turns.   
- No new loan/debt/overdraft development has resumed. **General monetary balance remains continuously audited**, including the economic impact of combat, rewards and group systems.   
- No title changed, so the title catalog remains **V3.51.40**. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.43 — Exact Focus Fire targeting and contextual autonomous-switch review   

- `Focus Fire` now keeps the **exact target designated by the leader** instead of silently falling back to the first remaining enemy. If that target truly leaves combat, the order expires cleanly.   
- Every monster receives a **temporary combat-instance ID** (`enemy_1`, `enemy_2`, ...) when entering an enemy queue. It is not saved, so two enemies with the exact same display name remain distinguishable without changing the save schema.   
- `PlayerWaveCombatTurn` now reports structured action observations: action kind, exact target name/identity when genuinely known, and whether targeting was forced by a combat rule.   
- Targeted attacks and skills can therefore be compared against the live `Focus Fire` target. Correct target means obeyed; freely choosing another target means unjustified disobedience; a different target forced by taunt/combat constraints is justified.   
- Multi-target, random or otherwise non-unique targeting remains **inconclusive** when the engine cannot honestly identify one exact target, so discipline is never invented from missing information.   
- Unauthorized individual switches by **employer NPC teammates** now receive contextual review using witnesses, HP, fear, survival urgency, replacement condition and tactical opportunity. Survival or a real tactical gain can justify the switch; ambiguous fear/survival cases stay inconclusive; no witness remains unobserved.   
- Switch review is recorded in `team_employer_unauthorized_switch_reviews` and does not automatically punish the actor. A future voluntary player-switch path must reuse the same logic before this item is considered fully complete.   
- `EmployerTeamDisciplineSystemTest` now covers exact Focus Fire, forced targeting, expired targets, unknown targets and justified/unjustified/unobserved autonomous switches. `EnemyGroupBehaviorTest` also locks combat-instance uniqueness when names are identical.   
- No new loan/debt/overdraft development is resumed. **General monetary balance remains a permanent cross-system audit constraint**.   
- No title changed, so the title catalog remains **V3.51.40**. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.42 — Employer boss reserve participation and no free entry turns   

- Employer-team boss encounters now support a salaried player **starting genuinely in reserve** instead of always occupying a frontline slot.   
- While the player is benched, enemies act against the NPCs actually present on the frontline; the player is not treated as a hidden active target.   
- The employer leader can spend a real turn to call the player in without waiting for a KO, while a naturally opened slot can also trigger entry.   
- Entering from reserve never grants an immediate bonus turn. Boss-side NPC rotations use the same no-free-action principle.   
- If the employer squad defeats the boss before the player personally enters, the result is recorded as a **squad victory**, not a personal boss clear: no artificial personal boss XP, loot or title is granted. Contract salary remains a separate economic system.   
- Employer-member injuries occurring while the player watches from reserve are persisted normally.   
- Clarification: the guild loan/debt development branch remains paused, but **global monetary balance remains an active permanent constraint** for rewards, salaries, loot, prices and exploit prevention.   
- No title was changed, so the title-catalog version remains **V3.51.40**. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.41 — Observable tactical compliance and leader review   

- The **guild finance / loans / debt branch is intentionally paused** at the user’s request. This release resumes combat/command work without adding new finance mechanics.   
- `EmployerTeamDisciplineSystem` can now assess player actions against an employer tactical order from actually observable facts: active order, action kind, leader presence, credible witnesses, survival state and escape-attempt outcome.   
- New hard rule: **no credible witness means no disciplinary penalty**. If the leader did not directly see the action but a living employer teammate did, the fact may be retained as a team report instead of making the leader omniscient.   
- Verdicts are separated into `unobserved`, `inconclusive`, `obeyed`, `justified disobedience` and `unjustified disobedience`. Ambiguous actions never automatically reduce trust.   
- Turns lost to a forced status (`forced_skip`) are explicitly inconclusive, so the player is not punished for being unable to act.   
- Escaping under an active order is contextual: without an observable survival emergency it may count as unjustified disobedience; at critically low HP / immediate danger it can instead be recognized as justified, especially when extraction succeeds.   
- First safe order/action mappings are live: defense/wait for `Hold Line`, attack/skill for `Controlled Breakthrough`, and team coordination during `Extract Wounded`. `Focus Fire` deliberately remains **inconclusive** until the exact target selected by the player is retained by this tracking layer.   
- `PlayerWaveCombatTurn` now exposes a non-intrusive action observer used by employer PvE. Standard PvE and exploration record actions that actually consume the player turn without breaking existing callers.   
- Discipline is applied once at the **combat review**, preventing trust farming from repeatedly performing compliant actions every turn. The leader can then provide an RP debrief after victory or escape while the player is conscious.   
- Observations and reviews are persisted through `team_employer_order_observations` and `team_employer_order_reviews`, distinguishing direct leader sight, teammate reports and no-witness cases.   
- `EmployerTeamDisciplineSystemTest` now covers obeyed orders, ambiguous actions, no-witness behavior, unjustified escape and justified survival escape.   
- No title was added, so the title-catalog version remains **V3.51.40**. Save schema stays **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.40 — Large title audit, spoiler mode, new cheats and first overdue-loan reminders   

- The title catalog now has an **independent version**: **V3.51.40**. It changes only when titles are actually added, removed or meaningfully modified, making the last title-audit point explicit.   
- A retrospective audit expands the catalog from roughly **181 to 326 titles**, covering finance/guild, companions, command, duo play, survival, mana/hybrid growth, languages, rumors, exploration, rivals, equipment memory, oaths, reputation, material magic, employment, FireFlight and REAPERS-13.   
- `Reaper-13` is deliberately **not** a normal player title while the existing leader lives. The extreme case of killing that leader instead points toward the secret title **“Cible prioritaire”**, reflecting likely clan-wide hostility.   
- New **Hidden titles** setting, OFF by default. Enabling it requires typing `SPOILER` after an explicit warning that names and unlock conditions may reveal story, bosses, factions, systems, outcomes and secrets. The setting is reversible and saved per character.   
- Title screens now display the title-catalog version and honor that setting: normal `????` discovery remains the default; names and conditions are revealed only after explicit consent.   
- Validated cheats `thatsalotofpower`, `icanfeelit`, `bornforit`, `nottoday`, `fearmeinstead`, `trustmebro`, `fillmypocket`, `justanotherday`, `luckytofu`, `onemorechance` and `everybodylovesme` are now live, case-insensitive and tracked in the alteration history.   
- `luckytofu` never grants **“Luck is a skill”**; `fillmypocket` never creates unique/quest/boss items; `onemorechance` re-arms exactly one extra lethal spirit-resistance attempt; `trustmebro` already affects employer-team loyalty and can expand once other persistent loyalty systems are unified.   
- Legitimate Protocol 13 usage can now unlock **“Canal interdit”**. Several finance titles are hooked to real events; **“Bon payeur”** now requires 3 on-time repaid loans instead of one. Titles tied to future systems stay catalogued but are not artificially simulated.   
- First layer of **overdue-loan handling**: an overdue loan suspends all new overdraft/credit until regularized, without erasing or artificially increasing existing debt. Canonical reminders are recorded on the first overdue day and periodically afterward.   
- Financial status now distinguishes probable hardship, partial payment capacity and sufficient available funds. The game **never automatically labels the player as acting in bad faith**; that will require later evidence such as explicit refusals and negotiation history.   
- New `TitleCatalogUpdateTest` plus expanded cheat/finance tests cover catalog version, 320+ titles, spoiler-setting persistence, intentional lack of `Reaper-13`, new cheat effects, overdue credit restriction and reminders.   
- Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.39 — Live guild loans, long-term surcharges and repayment priority   

- **Guild loans are now live**: a sufficiently trusted active member can receive physical capital, with only one active loan at a time and a trust-based principal ceiling up to a theoretical **5 PO**.   
- Available terms are **1 / 2 / 3 / 4 / 6 / 8 weeks**. Base rates are 12 / 14 / 16 / 18 / 21 / 24 %. Longer terms add a separate signing surcharge: **3 weeks +2 %, 4 weeks +2 %, 6 weeks +3 %, 8 weeks +4 %**. Example: 3 weeks is 16 % + 2 % = **18 % signed cost**.   
- Longer terms require more financial trust: current thresholds are 55 / 60 / 65 / 70 / 80 / 90. A new member therefore cannot immediately take the longest or largest loan.   
- Signed cost is **fixed**. Early repayment is allowed but does not retroactively reduce signed interest, and overdue loans do not automatically compound.   
- Repayment mode is chosen at signing: **manual**, or **automatic at maturity** from money actually deposited with the guild. Automatic repayment never teleports coins out of a distant physical wallet.   
- Preventive extensions before maturity are active: +1 week = +3 %, +2 = +5 %, +3 = +7 % of original principal. Total contractual cost is capped at **30 %**; an extension that would exceed the cap is clipped to the remaining room.   
- A loan that reaches maturity with a remaining balance becomes overdue without additional automatic compound interest. Late negotiation and collection escalation remain separate future systems.   
- Loan repayment works from either the wallet or the guild account. Financial trust now rewards on-time completed loans and penalizes an overdue active loan.   
- When **overdraft + loan** coexist, a combined repayment menu lets the player explicitly choose **overdraft first [recommended]**, loan first, or split repayment, from either the wallet or guild-deposited funds. The game does not silently choose which debt to reduce.   
- Overdraft and loan canonical records are protected from journal compaction so persistent debt cannot disappear during a very long playthrough.   
- The **`whatdebt`** cheat is now connected: it artificially clears both guild overdraft and active loan without spending coins, marks the character Altered, and never grants honorable repayment history.   
- The terminal finance UI was extracted from `QuestMenu.cpp` into `GuildFinanceMenu.cpp`, bringing `QuestMenu.cpp` back under its modularity limit and giving future financial features a dedicated module. Structural tests now follow the new location.   
- `GuildFinanceSystemTest` now covers term rates/surcharges, single-loan enforcement, physical capital, extensions, the 30 % cap, early repayment, save/load of an active loan, automatic repayment, overdue behavior without compounding, and multi-debt priority. `CheatLegacyCodesTest` also covers `whatdebt`.   
- Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.38 — Capped guild account and live network overdraft   

- The guild account is no longer unlimited money storage: capacity is **2 PO** for a registered non-member Adventurer and **5 PO** for an active member. These values remain subject to playtest calibration so the personal vault keeps a meaningful role.   
- Existing V3.51.37 saves already above the new cap lose nothing: no funds are confiscated and withdrawals remain available; only new deposits are blocked until the balance returns under the limit.   
- `GuildFinanceSystem` now exposes total account capacity and remaining deposit room. **Money Management** shows `balance / capacity` and clamps deposit quantity to the real remaining space.   
- The **guild overdraft is now live**: persistent debt is reconstructed from `guild_finance_overdraft_draws - guild_finance_overdraft_repayments`, with a ceiling based on active membership and financial trust and a real remaining margin that never makes the physical wallet negative.   
- Overdraft remains **network credit**, never withdrawable cash. An affiliated service first spends the actual wallet and, only after explicit authorization, the guild covers the missing amount and records it as debt.   
- First affiliated integration: the inn's **safe room** can offer guild credit when the wallet is insufficient, showing the physical share, required credit, current debt and remaining margin before confirmation.   
- Overdraft can be repaid manually from the physical wallet or from funds already deposited with the guild. Account repayment reduces both deposited funds and debt without teleporting coins.   
- Expired membership sets new-credit capacity to zero without erasing existing overdraft debt or blocking repayment.   
- `GuildFinanceSystemTest` now covers deposit caps, overflow refusal, mixed wallet/credit partner payment, both repayment paths, save/load persistence and debt survival after membership expiry.   
- `scripts/test_project.sh` now explicitly waits for the parallel guild-finance test before printing the final success message.   
- Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.37 — Money management and guild financial membership   

- The guild **Currency Exchange** entry is replaced by a unified **Money Management** hub covering the physical wallet, PC/PF/PE/PO/PP conversion, guild deposits, membership and previews of future overdraft/loan features.   
- The already-existing `Standard adventurer contribution` is reused as the canonical guild membership instead of creating a competing subscription. Existing saves keep compatibility and existing service benefits.   
- Guild membership is now managed from the guild financial desk rather than being sold again from inn/city subscription lists. The existing 7-day period, cancellation and automatic renewal behavior are preserved.   
- New `GuildFinanceSystem`: registered Adventurers can physically deposit money into a guild balance and withdraw it later. Deposits really remove coins from the wallet; withdrawals return physical money; the wallet never becomes negative.   
- Guild balance is reconstructed from protected canonical `guild_finance_deposits` / `guild_finance_withdrawals` records, preserving history and save/load persistence without changing schema 26.   
- A first financial-trust score uses real guild history, membership and deposited funds to prepare future credit ceilings without creating debt yet.   
- Member previews use the validated unambiguous labels `Overdraft used`, `Overdraft ceiling`, `Remaining margin`; the displayed ceiling is explicitly preparatory and cannot be spent in this version.   
- Loan preview documents the validated rules without creating a live loan contract yet.   
- New `GuildFinanceSystemTest` covers registration gates, deposits/withdrawals, insufficient funds, membership, trust, member-only future ceiling and save/load persistence.   
- Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.36 — Cheat registry, contextual Protocol 13 and legacy code reconnection   

- The special **“On ne dira rien...”** trophy is now synchronized at account level on the first alteration. It is intentionally the only account trophy obtainable through cheating and is explicitly excluded from ultimate-title requirements. Normal titles explicitly granted by future cheats remain character-only.   
- `DINOTOFU:13-02313` is now a **once-per-character** channel opening. Re-entering it never emits another beacon and reports that the protocol was already transmitted or that the channel is no longer reachable.   
- Protocol 13 is contextual: prior credible Reaper knowledge (NPC, rumor or encounter) keeps the use diegetic/non-altering; using the forbidden code with no in-world knowledge is treated as metagaming and marks the character Altered. A blocked lethal-mode metagame attempt also consumes the single opening to prevent spam.   
- Character stats now expose a persistent **used codes / protocols** registry with use counts and meaningful statuses such as Active, Inactive, One-shot or Consumed when applicable.   
- Previously documented legacy commands are reconnected to `CheatManager`: `13023131302313`, `iamnotgoblinfood`, `letmecook`, `whereismyplotarmor`, `talktothedev`, `oopsallbosses` and `randomizemyfate`.   
- `13023131302313` fully heals HP/mana and arms an execution on the next ordinary hostile target actually attacked. Boss-specific mechanics are deliberately not bypassed.   
- `iamnotgoblinfood` now affects goblin morale; `letmecook` temporarily widens the critical window for the current world day; `whereismyplotarmor` grants a real survival blessing; `oopsallbosses` reveals the boss registry; `randomizemyfate` rebuilds class/starter kit while preserving ownership/level; `talktothedev` stays a non-altering secret.   
- New `SystemCallManager` skeleton: `system_call` now really opens only the **SYSTEM CALL** title/prompt, with no help, command list or autocomplete. Hidden `generate_*` subcommands remain to be connected later.   
- Added `CheatLegacyCodesTest` and expanded `AccountTitleProgressTest` for account trophy rules, one-use/contextual Protocol 13, and legacy cheat reconnection.   
- Save schema remains **26** and important checkpoint **V3.51.15** is unchanged.   

---   

## V3.51.35 — Account titles, Altered characters and Protocol 13 channel   

- Legitimately earned titles are now archived at the **account** level in a persistent unlocked-title collection. The archive survives deletion of the character that originally earned a title and has a dedicated account-menu view.   
- A character made **Altered** by an official cheat keeps previously earned titles, but normal new-title grants are blocked and that save can no longer expand the legitimate account trophy archive. Legitimate titles earned before the first alteration are frozen into the account immediately before the first cheat effect is applied.   
- New local alteration title **“On ne dira rien...”** is granted on the first transition to Altered. It belongs only to that character, never enters account trophies and is explicitly excluded from ultimate-title prerequisites. Older already-Altered saves receive the marker on load when missing.   
- New explicit `grantCharacterOnlyTitleFromCheat` path lets future skip/unlock cheats grant a base title to the current character only without contaminating account progression. Audit confirms there is currently no “unlock all titles” cheat; `skipallstory` does not unlock titles.   
- Fixed a historical inconsistency: secret code `1302313`, documented as non-altering, no longer marks the character Altered merely for showing the creator message.   
- Added non-altering diegetic protocol `DINOTOFU:13-02313`: it opens **PROTOCOL 13**, records a REAPERS-13 forbidden-channel beacon in canonical history and grants no recruitment, victory or free spawn. Any future response must come from the real world/faction logic.   
- Altering cheats in permanent-death modes remain punished; explicitly non-altering secret protocols do not trigger that punishment.   
- `CHEATS_DINOTOFU.txt`, `TITRES_DINOTOFU.txt` and `SYSTEMES_PREVUS.txt` now capture the validated design for future SYSTEM CALL/generate commands, always-altering generation, planned save anti-tamper, validated future cheat names, unlocked guild-finance design, REAPERS-13 lore and future FR/EN first-launch language selection.   
- Added `AccountTitleProgressTest` covering legitimate account-title persistence after character deletion, title lockout after alteration, character-only cheat grants, exclusion of the alteration marker from account trophies and non-altering Protocol 13 behavior. Full suite passes before the bump.   
- Save schema remains **26**, important checkpoint remains **V3.51.15**, and story writing remains paused.   

---   

## V3.51.34 — Authorized individual switches, reserve spillover and boss rotations   

- Added `CombatIndividualSwitchSystem`: a member who wants to leave the line on their own normally asks the still-present leader for permission. A denied request leaves the member in place and does not consume their normal action.   
- Fear, survival instinct, panic or an immediate tactical judgment may exceptionally make a member switch without waiting for authorization. This remains deliberately rare (bypass capped at 18%) and is recorded separately so it can later be judged as justified initiative or disobedience.   
- Personal recruits now genuinely ask the player-leader through a dedicated prompt; employer-team colleagues use the NPC leader decision. Non-leader enemies follow the same logic: ask a present leader, potentially get denied, or rarely move on instinct depending on fear, intelligence, discipline and cohesion.   
- Added `CombatReserveSpilloverSystem`: effects that are truly wide enough may spill into reserve at only **5 to 15%** of their main power. Edge reach affects at most 1 waiting member, wide reach 2, and massive reach 3. Effects without sufficient reach never touch the queue.   
- The allied group-heal order is now a real area heal resolved by a capable support. The active line receives the main effect while reserve only receives a 5-15% echo based on reach, rank and healer profile.   
- Experienced enemy healers may also occasionally cast a group heal. Their waiting reinforcements receive the same reduced spillover, keeping the rule symmetrical rather than player-only.   
- Workshop Bomb is the first explicit player explosion wired to enemy reserve: the blast may hit waiting enemies for 5-15%, with affected targets capped by the actual reach. `EnemyCombatQueue` can now damage/heal waiting members without moving them to the front and properly removes a reserve target killed by area damage.   
- Large boss pressure can also lightly reach allied reserve when it represents a genuinely wide zone; local pressure does not automatically cross formation boundaries. A reserve member dropped this way can no longer later enter as a replacement.   
- An employer NPC leader may now sacrifice their turn **during a boss fight** to pull back a wounded colleague and bring a reserve member in without waiting for a KO. Boss phases are preserved and no extra action is created.   
- Added `CombatIndividualSwitchSystemTest` and `CombatReserveSpilloverSystemTest`; `EmployerTeamBossSupportTest` now covers employer-leader boss rotation. `test_project.sh` explicitly waits for these tests before reporting success. Full suite passes before the bump.   
- Intentional limits: not every legacy area effect has been classified by reach yet; disciplinary interpretation of unauthorized switches still needs observable-context evaluation; manual player-leader rotation in boss UI remains to be integrated cleanly.   
- No debt/loan or story changes. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.33 — Leader rotations and first real tactical orders   

- Added `CombatLeaderRotationSystem`: an operational leader may now spend their own turn to exchange **1 or 2 members** between the active line and reserve. A successful rotation consumes exactly the leader turn and never grants an extra action.   
- Player-led parties get a manual leader-rotation action inside team orders. Both exchanges are planned before mutation so a two-member rotation stays atomic. A critically weakened member may rarely refuse; the leader keeps the normal turn in that case, while Rotation is locked until the next leader turn to prevent retry spam.   
- An employer NPC leader may now reorganize colleagues or **call the player in from reserve without waiting for someone to fall**. This is not automatic and depends on combat context; a successful rotation replaces the leader's normal action.   
- Enemy waves follow the same rule when a genuine group leader is present. The leader is kept first in the enemy action order and may sacrifice the turn to pull one or two wounded compatible allies back for reserve members. Intelligence, cohesion, solidarity and wounded-abandonment behavior affect the decision; groups that do not care for wounded allies rotate far less often. The old free enemy tactical switch is removed.   
- `EnemyCombatQueue` can now move an active leader to the front without creating/removing turns. Enemy rotations preserve headcount and require a real compatible reserve.   
- Added `EmployerTacticalOrderSystem`: an employer leader may issue a credible engagement order such as protect support, focus fire, hold the line, extract wounded, or controlled breakthrough. NPC colleagues actually consume that order through the shared `RecruitedAllyActionPlanner` and existing ally-order layer.   
- A salaried player still controls their own character. Discipline does not yet auto-interpret every combat action: only sufficiently observable disobedience should be penalized in a later pass, avoiding arbitrary punishment for ambiguous choices.   
- Added `CombatLeaderRotationSystemTest` and `EmployerTacticalOrderSystemTest` and wired them into `test_project.sh`; the full suite passes before the version bump.   
- Intentional limits: generic leader rotation is not yet wired into every special boss formation, and automatic player obedience/disobedience classification is still partial.   
- No debt/loan or story changes. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.32 — Employer-team discipline, reserve duty and contracted boss support   

- Employer squads now contain **3 to 5 NPCs before recruiting the player**, for a final 4-to-6 member team. A full six-member squad therefore never invents a seventh slot for the player.   
- New `EmployerTeamDisciplineSystem`: each contract tracks professional trust, warnings, completed/failed missions, obeyed/disobeyed orders and temporary reserve-duty assignments. Trust starts at 70/100 and one mistake never causes instant dismissal.   
- Discipline escalates gradually: failed missions or repeated unjustified disobedience lower trust; repeated incidents may bench the player for one or more service fights; genuinely repeated misconduct may recommend disciplinary dismissal. Justified disobedience, especially when it prevents serious harm, is not treated as an automatic fault.   
- Disciplinary dismissal may retain **50 to 75% of salary still owed**, with a hard 75% cap. Already-paid salary is never taken back. Final settlement records both the amount paid and the amount retained.   
- Accepting the local employment offer now immediately assigns a first real employer mission, preventing a player from signing only to collect salary without any service obligation. Synchronized mission failures feed discipline and repeated failures can automatically end the contract.   
- Reserve duty is wired into standard PvE and exploration: the player may start outside the three main slots, visibly watch the leader and colleagues fight, then be called in when a slot opens or after a few rounds to avoid frustrating spectator-only combat. If the squad wins before entry, reserve duty is served but no personal combat XP or loot is invented.   
- The employer roster is now connected to **boss fights** while a real employer mission is active. The operational leader opens the allied phase before the player, other colleagues act afterward, their HP/injuries/progression persist in the employer roster, and they remain paid by their organization instead of taking a mercenary share from the player.   
- If the player is currently benched by the leader, employer support is not loaned out to bypass that sanction on a personal boss fight. Full visible reserve handling during bosses remains future work.   
- Added `EmployerTeamDisciplineSystemTest` and `EmployerTeamBossSupportTest`; existing roster/aftermath tests now accept the new three-NPC minimum. Full suite passes before the bump.   
- No debt/loan or story changes. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.31 — Persistent employer squad and leader-first turn rule   

- New `EmployerTeamRosterSystem`: an external contract now owns a real persistent squad distinct from the player's personal recruits. The roster creates 4 to 5 NPC colleagues under the contract leader (5 to 6 total members including the player) and keeps identity, role, level, rank, HP, equipment quality, baseline mood and loyalty in the canonical journal.   
- Team order now has a hard rule: **the operational team leader always acts first**. In an employer squad the opening order is NPC leader → player → other active colleagues; a KO/unavailable leader never receives a ghost turn. This priority grants no extra turns and revives nobody.   
- Employer formation uses the existing three main combat slots: with the player present, at most 2 NPC colleagues are active and the rest wait in reserve; after the player falls, up to 3 colleagues may occupy the freed main slots. The roster is capped at 5 NPCs so the team never exceeds 6 members.   
- The Team menu now exposes the real employer squad: active, reserve and unavailable members, opening order, HP, level, role, mood and loyalty. The player's personal group remains separate and is never converted into employer personnel.   
- During a **real active employer mission**, standard PvE and exploration use the employer squad instead of personal recruits. The leader acts before the player, other colleagues then use the shared `RecruitedAllyActionPlanner`, and employer-paid colleagues never take personal-recruit shares from the player's loot.   
- Employer colleagues now keep separate combat HP/injuries and progression. A KO member stays unavailable until their estimated recovery day and then returns with partial HP; participation is still recorded even when the member finishes the encounter wounded.   
- Post-KO handling now uses every participant who actually took part, including reserve members who rotated in. The player's owed economy share is preserved for delivery after waking rather than redistributed as recruit salary. After-action reports reuse the RP-aware ~85% baseline with employer-specific persistent categories.   
- An employer colleague may divert the player's share only when profile and relationship state make betrayal credible; the theft is stored in employer history and that member becomes unavailable/departed instead of being confused with a personal recruit.   
- Added `EmployerTeamRosterSystemTest` and `EmployerTeamAftermathSystemTest`, plus `test_project.sh` regression guards for employer-mission combat wiring and leader-first behavior.   
- Intentional limits: boss combat does not yet use the employer roster; rich NPC-leader tactical orders, evolving colleague-to-colleague relationships, needs, full item inventories and specialized employer missions remain future work.   
- No debt/loan or story changes. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.30 — Employer-team contracts, salary and leader-issued missions   

- New `TeamEmploymentSystem`: a character can now hold a persistent contract with an external team/organization, separate from their personal group. The contract stores the NPC leader, organization, role, start day, optional end day, pay interval, settled pay periods and termination history.   
- External salary is earned from actual world time: only fully elapsed pay periods can be collected. Several periods may accumulate; resignation or normal contract completion settles only salary already earned and never creates advance pay from menu clicks. A salaried player does not pay the employer team's payroll.   
- New `TeamEmploymentMissionSystem`: the employer-team leader can issue a real journal quest tied to the active contract. Client and origin match the actual leader and organization, progress uses real combat activity, orders can expire/fail, and mission bonuses stay separate from contractual salary. Completion/failure history is recorded once.   
- New `TeamEmploymentOfferSystem`: when no contract is active and the player is in a valid city, a local field squad affiliated with that city can provide an initial playable employment offer. Leader, organization, role, salary, interval and duration are shown before confirmation; accepting switches leadership to the external leader without deleting the player's personal group.   
- The Team menu no longer hard-codes the player as leader while employed by someone else. The employer-team screen now exposes status, due salary, leader-issued missions, member rights and resignation while explicitly preserving the personal group separately.   
- Any still-open employer mission is now automatically interrupted and recorded when the contract ends (expiry, resignation or dismissal), so no former leader order remains orphaned after returning to the personal group.   
- Contract state, salary payments and employer-mission traces use the persistent canonical journal without changing the save schema; their categories are protected as narrative state.   
- Added `TeamEmploymentSystemTest`, `TeamEmploymentMissionSystemTest` and `TeamEmploymentOfferSystemTest`; the full suite protects contract state, earned pay, expiry, resignation, employer missions, history and local offers.   
- Intentional limitation for this pass: employer-team NPC colleagues are not yet a complete persistent combat roster. Their personal progression, mood, needs, equipment, reserve rotation and external-leader tactical orders remain the next major step; the current local offer is an initial playable route rather than the final simulation of every organization.   
- No debt/loan or story changes. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.29 — Final-survivor lethal rescue, spirit resistance and team leadership   

- Team rescue from permanent death is recalibrated: an ally with genuine healing/revival ability can now provide **33 to 66%** survival depending on effective capability, estimated level, rank, maturity and equipment. A surviving non-healer remains limited to **15%** emergency stabilization/transport; a strong medical kit still does not turn an untrained fighter into a reviver.   
- The rescuer must still be **actually standing at final resolution time**. In standard/exploration PvE and solo boss fights, hidden combat/extraction continuation now resolves before the lethal rescue roll, so a healer who falls during that continuation provides no bonus. FireFlight's personal safeguards and Hitogami's second life remain intact.   
- New `LethalSpiritResistanceSystem`: on the **first** permanent-death event only, after normal rescue fails, the character may make one final spiritual refusal to let go. The sequence uses 3 successes versus 3 failures, d20 success on 13+, for about 31.7% overall survival; success or failure consumes this chance forever. Success restores 1% HP without granting an artificial clear.   
- Spirit resistance is wired into standard/exploration PvE, bosses, coop and lethal PvP. In PvP, surviving by willpower does not invoke the destructive inventory consequences specific to the survival blessing. The existing rare lethal-survival blessing remains a separate fallback only after team rescue and first spirit resistance have failed.   
- New `TeamLeadershipSystem`: the personal party treats the player as leader by default, while an external team may declare an NPC leader or another player, an organization and a salary. When the player is not the actual leader, relevant menus no longer let them freely reorder or command the entire team.   
- The leadership foundation now clearly distinguishes a salaried member from the owner of the team: a player employed by an external organization can be marked as paid and not responsible for team costs. Actual salary payment, contracts, leader-issued quests and disobedience behavior remain future work.   
- `LethalTeamRescueSystemTest` now covers the 66% cap, a dead healer and a surviving non-healer staying at 15%. Added `LethalSpiritResistanceSystemTest` and `TeamLeadershipSystemTest`, plus regression guards for lethal continuation and PvE/boss/PvP integrations.   
- No debt/loan or story changes. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.28 — Shared ally AI and lethal team rescue   

- Added `RecruitedAllyActionPlanner`: recruited allies now share one tactical decision layer across standard PvE, bosses, and hidden post-KO simulation. It chooses self-preservation, leader healing, ally healing, guard, technique, or attack from role, rank/level, maturity, equipment, party state, orders, and combat context.   
- `MonsterPveMode` and `RecruitedAllyBossSupport` now consume that same planner. Rendering and concrete execution remain mode-specific so real targets, techniques, and boss mechanics are preserved, while decision logic is no longer duplicated between visible combat, boss combat, and post-KO simulation.   
- Added `LethalTeamRescueSystem` for permanent-death team play. A living ally with genuine medical/healing/revival capability grants a survival chance bounded between **33 and 50%**, influenced by capability, rank, maturity, and equipment.   
- A surviving team with no genuinely capable rescuer only provides a **15%** emergency stabilization/transport chance. A high-quality kit alone does not turn an untrained old warrior into a reviver. If nobody is still standing, team rescue is impossible.   
- The rule is wired into standard/exploration PvE, solo bosses with recruits, human cooperative PvE, and cooperative bosses. In human coop, eligible rescuers are snapshotted from characters who were actually standing at resolution time, preventing artificial chain revivals.   
- The previous self-contained coop three-green/three-red death-save sequence is replaced by actual team rescue: survival now comes from allies who can credibly intervene instead of a personal roll with no rescuer. Lethal-survival blessings remain a separate fallback after normal rescue fails.   
- Successful rescue avoids the permanent-death record; failed rescue falls through to the normal lethal resolution. Attempts and successful rescues are recorded canonically. FireFlight keeps its personal challenge locks: being rescued never grants its clear or final test.   
- Added `RecruitedAllyActionPlannerTest` and `LethalTeamRescueSystemTest`, plus regression guards ensuring PvE/boss/coop paths stay connected to the shared systems. The project test script now explicitly waits for these parallel tests before reporting success.   
- No debt/loan or story changes. Save schema remains **26** and the important save checkpoint remains **V3.51.15**.   

---   

## V3.51.27 — 3+2 slots, RP post-KO aftermath and team boss continuation   

- Combat slots now explicitly follow **3 shared main slots + 2 summon-only bonus slots**. Main slots prioritize human players, then NPC allies, then summons; a summon may use a free shared slot before using the two bonus slots. Overflow stays queued/reserved.   
- `CombatSlotRules` centralizes the 3/2/5 limits and `CombatGroupBuilder` places priority units before summons. Tests cover 1 fighter + 4 summons and 3 fighters + 2 summons.   
- After a non-permanent KO in standard/exploration PvE, the hidden continuation now has a real outcome: up to 3 recruits can occupy freed main slots, reserves enter, recruits progress/receive shares according to actual activity, and the player's owed economy share is held until wake-up.   
- New `RecruitedAllyAftermathSystem`: wake-up reports start around an 85% baseline and are modified by relationship, mood, unpaid wages, lived rescues/healing and personality. A companion may shorten/refuse the debrief; stealing the player's share requires an actually dubious profile or a damaged contract and persists as a departure from the group.   
- Recruits who left/betrayed are excluded from active/reserve rosters. A normally loyal companion cannot become a thief through arbitrary RNG alone.   
- Solo boss fights can now continue off-screen after a non-permanent player KO: reserve members can fill the third main slot, allies heal/support each other, boss pressure/ultimate state continues, and injuries/evacuations remain persistent.   
- A boss finished by recruits while the player is unconscious is recorded as a **team victory**, not a personal clear: personal titles/registry progression are not granted for free. The player's economy share can be handed over on wake-up through the same RP aftermath system.   
- FireFlight remains personal: recruits may secure extraction but cannot deal the validating last hit or pass the final test. Hitogami still gets his second life even if recruits finish the first phase.   
- Future team hierarchy rules are recorded: the actual team leader controls voluntary order; the player normally leads their own group, but when employed by an external team they are a salaried member under that leader rather than magically controlling the group.   
- Recruits who fall during an off-screen simulation remain real combat participants: their turns, damage, support, XP and shares no longer disappear merely because they end the fight headed to the infirmary. This applies to standard/exploration PvE and bosses. A post-KO fight with no money can still progress recruits.   
- Hidden information stays hidden: Hitogami can trigger his second life while the player is unconscious without displaying the revive scene. FireFlight only records recruit extraction when someone actually survived to recover the player.   
- Added `CombatSlotRulesTest`, `RecruitedAllyAftermathSystemTest`, and `RecruitedAllyBossPostKoTest`. Full suite passes before and after bump.   
- No debt/loan or story changes. Save schema remains **26** and important checkpoint remains **V3.51.15**.   

---   

## V3.51.26 — Six-person recruit team and lived duo coordination   

- Solo travel teams are now defined as **6 people maximum total**: the player plus up to 5 actually engaged recruits. While the player is conscious, the three physical combat places are the player + 2 recruits (summons remain outside that recruit-slot limit). If the player falls non-permanently, that physical place is freed and up to 3 recruits can fight simultaneously; the others remain reserve.   
- Retained Torvald profiles are explicitly candidates, not team members. A profile-only candidate no longer consumes a team slot and can never leak into the combat reserve; the five-recruit cap is enforced when a recruit is actually engaged. Legacy saves with too many equipped recruits are bounded by the shared roster without deleting history.   
- After a non-permanent player knock-out in standard/exploration PvE, recruit/reserve combat continues as an off-screen simulation. The unconscious player no longer sees automatic round-by-round information they could not know; a surviving recruit may later provide a brief result report after recovery.   
- `DuoMasterySystem` now tracks lived pair progression: successful/partial combinations, unique shared-combat days, unique coordinated-order days and the real relationship scores of both recruits contribute to coordination and bond quality. Shared-day/order traces are unique per world day to prevent trivial same-day farming.   
- Combined recruit techniques can now fully succeed, be partially recovered, or fail from bad timing. Partial execution loses major tactical effects and power; complete failure still consumes both allied turns. Mature pairs can reach a named common signature while novice pairs are meaningfully less reliable.   
- Shared duo experience is recorded from real participation even when a combat has no monetary reward. Reward value therefore no longer decides whether a fight counted as shared experience.   
- Future multiplayer rule recorded: at most 3 human players in one team. The exact future online mixture of human players and recruits is deliberately not frozen yet.   
- Cross-training rules remain explicit: non-mages must complete a difficult mana-awakening quest before magic; pure mages require substantial persistent melee training; ranged combat remains the mixed path.   
- `RecruitedAllyCombatRosterTest` now protects 6-total / 5-recruit / 2-active limits and excludes profile-only candidates from active/reserve combat. `DuoMasterySystemTest` protects daily anti-farm traces, relationship/bond progression and coordination bounds.   
- No debt/loan or story changes. Save schema remains **26** and important checkpoint remains **V3.51.15**.   

---   

## V3.51.25 — Active recruits in solo boss fights   

- Recruit combat roster construction is extracted from `MonsterPveMode` into shared `RecruitedAllyCombatRoster`: retained, not dismissed, not in infirmary/pending evacuation, equipped, manual order, max 2, rank/level, persistent HP, equipment and reward shares now use one shared source.   
- `MonsterPveMode` now consumes that shared roster instead of rebuilding initial recruit state locally.   
- `BossPveMode::run` now brings 0–2 equipped recruits into solo boss fights. They take a real allied turn after the player: support/healing for suitable profiles, otherwise a single-target attack on the boss.   
- Bosses apply bounded area pressure to recruits. Recruit HP is real and persistent; a fallen recruit is marked for infirmary evacuation instead of being restored for free.   
- Recruit participation, damage, support, XP and economy share are recorded after boss victory. The player keeps the largest economy share while boss XP remains full, matching the existing PvE recruit split model.   
- Unique boss mechanics remain authoritative: recruits cannot replace FireFlight's mortal rule, final test, phase gates or unique checks.   
- Added `RecruitedAllyCombatRosterTest` covering max 2, manual ordering, infirmary exclusion and dismissed recruit exclusion.   
- Future progression rules were added to `SYSTEMES_PREVUS.txt`: non-mages must awaken mana through a difficult quest before using magic; pure mages need substantial real training before becoming competent in melee; ranged combat remains the mixed path with broadly unified difficulty.   
- No debt/loan or story changes. Save schema remains **26** and important checkpoint remains **V3.51.15**.   

---   


## V3.51.24 — Transactional exchanges and equipment-transfer audit completion   

- Voluntary exchanges between two characters now go through `ExchangeTransferSystem` for money, weapons, armor, consumables and materials. Each transfer removes the resource from the giver before adding it to the receiver, preventing duplication if an unexpected removal failure occurs.   
- Given weapons and armor preserve their exact persistent ID. The giver receives durable `exchange_given` history and the receiver `exchange_received`, alongside canonical `echanges_donnes` / `echanges_recus` summaries. Consumables use semantic identity, while materials use `id + quality + quantity`, matching their current save model.   
- Character History now exposes given and received exchanges. Cancelled or invalid exchanges no longer falsely report a successful save.   
- Exchange persistence is coupled through `SaveManager::savePlayerExchangePair`: the previous state of both character/account files is captured and restored when either normal save fails. The exchange menu also restores both in-memory `Player` objects on such a failure. This protects recoverable write failures; it does not claim disk-level atomicity against a hard process or machine crash.   
- The remaining equipment transfer/loss audit confirmed shop sale/buyback, enchanter destruction, non-definitive death, Grinka, equipment lifecycle and PvP paths are already protected or traced.   
- City vault history now preserves the exact persistent identity of weapons/armor instead of merging same-name items. Vault materials and inter-city material transports are distinguished by `id + quality`.   
- `ExchangeTransferSystemTest` covers money, weapon, armor, consumable, material, no-mutation failure and persistent-ID preservation. `CityVaultIdentityTest` covers weapon/armor deposit-withdraw identity and material quality. Project guards verify the new wiring.   
- No balance, debt/loan or story changes in this pass. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.23 — PvP equipment transfer history   

- Weapons and armor actually taken from a loser during a deadly PvP duel now create durable `equipment_lost` history for the loser using the exact persistent item ID, with a type/name fallback for legacy data.   
- The winner simultaneously receives canonical `butin_notable` provenance tied to the same item identity, including the loser and duel context, so a physical item can keep a coherent trace when ownership really changes.   
- Equipped gear taken during definitive-death PvP is explicitly identified as equipped, while inventory theft distinguishes definitive death from non-definitive deadly duels.   
- No history is created when the loot roll fails, starter gear is protected, or the actual transfer cannot be completed.   
- PvP transfers now remove the item from the loser before adding it to the winner, preventing silent duplication if an inventory removal unexpectedly fails.   
- `PvpEquipmentHistoryTest` covers persistent IDs, legacy fallback, durable loser history and winner-side provenance. The project suite also checks that every weapon/armor theft path in `PvpMode` is wired to the traceability module.   
- Theft chances, value caps, rewards and duel rules are unchanged. Save schema remains **26** and the important checkpoint remains **V3.51.15**.   

---   

## V3.51.22 — Grinka equipment confiscation history   

- Weapons and armor actually confiscated by Grinka after defeat now produce durable `equipment_lost` events with the persistent item ID or type/name fallback.   
- These remain recoverable thefts, not destruction; the existing Grinka retrieval storage is unchanged.   
- No event when protection prevents theft; probabilities and balance are unchanged.   

---   

## V3.51.21 — Enchanter equipment destruction history   

- Critical enchanting failures that genuinely destroy a weapon or armor now write a durable `equipment_destroyed` historical event.   
- The event preserves the item's persistent ID, falling back to type and name where no ID exists, and describes the actual reason for destruction.   
- Runic safety seals still prevent destruction, with no false history event when a seal saves the equipment.   
- Enchantment risks, prices, loot, and save behavior are unchanged.   
- The debt/loan system still awaits approval and story work remains paused. Save schema stays 26; important checkpoint stays V3.51.15.   

---   

## V3.51.20 — Forced equipment losses and non-lethal death history   

- Equipment losses caused by a non-lethal death now leave a **durable historical trace** tied to the actual persistent identity of the affected item.   
- Weapons or armor **stolen** after defeat are recorded as lost equipment; gear made **irreparable and destroyed** is recorded as destroyed equipment; a **forced break** that leaves the item in inventory is recorded separately.   
- **Character history → Recent durable events** now renders readable labels for these new categories without inventing provenance when no real loss occurred.   
- Forced-break logic no longer reports an already-broken item as newly “broken by the impact”: the event is created only when the forced break actually takes usable gear down to zero durability.   
- `Random` now accepts an explicit seed so probabilistic scenarios can be reproduced deterministically in tests while normal gameplay keeps using `std::random_device`.   
- `DeathPenaltyHistoryTest` deterministically covers weapon theft, a repairable forced break and irreparable armor destruction.   
- No death-balance percentages were changed in this pass; calibration remains a playtest task.   
- The proposed debt/loan system remains deliberately **unimplemented** until the user validates its structure.   
- Save schema remains **26**. Important checkpoint remains **V3.51.15**.   

---   

Detailed English version history for Dinotofu. The matching French history is stored in `CHANGELOG_FR.md`. README files remain focused on installation and useful player information.   

## V3.51.19 — Configurable loot pickup and loot provenance   

- Loot pickup is now configurable from inventory preferences. The default is **Pick up everything automatically**. Alternatives are **Manual only**, which asks for every generated item, and **Custom filtering**, which collects everything except explicit exclusions.   
- Filtering can exclude whole categories (materials, weapons, armor, consumables) and specific known/owned material IDs. Filtered loot is never auto-sold or destroyed: it is simply left behind and explicitly reported.   
- Loot preferences are persisted additively (`lootPickupPolicy`, ignored categories and ignored material IDs). Old saves default to **Pick up everything**. Save schema remains 26.   
- Combat loot generation now respects these rules for materials and recovered equipment. Authored quest rewards intentionally bypass the filter so exact promised rewards remain guaranteed.   
- Character History gains a **Notable loot** section. Recovered weapons/armor and rare, magical or high-quality materials leave canonical provenance records when a source is known.   
- Save round-trip tests protect the default mode, persistent filters and backward compatibility; project guards also verify real loot-generator and history integration.   
- The proposed debt/loan/economic-sanction system after V3.51.18 remains intentionally **unimplemented** pending approval of its detailed structure.   
- Save schema unchanged: **26**. Important checkpoint unchanged: **V3.51.15**.   

---   

## V3.51.18 — Motivated companions, friendship and real wages   

- Retained guild profiles are now distinct from actually hired recruits. Keeping a candidate sheet no longer automatically affects team rank, reward shares, wages or active-party state. Old saves remain compatible: pre-3.51.18 retained recruits are treated as already hired.   
- Candidates with a mandatory personal quest must actually complete it before hiring. The Team menu gains an explicit **Hire a retained profile** action.   
- New recruits receive a persistent initial motivation: **contract**, **shared objective**, or **affinity / enjoyment of adventuring**.   
- Friendship is derived only from persistent lived facts: participation, shared quests, active weeks, personal quests, healing and rescues. It is never granted randomly on first meeting; unpaid wages can reduce it.   
- Weekly wages become real expenses and are deducted from the player's physical purse. Paid and unpaid wages leave canonical records.   
- Requested exceptions are implemented: a recruit actively pursuing the same objective may waive that week's wage; an affinity-driven recruit may eventually travel for free after genuine friendship is built; exceptionally strong relationships may also justify waiving wages.   
- Reward normalization and team-rank computation explicitly ignore retained-only profiles. Unhired profiles receive 0%, do not inflate clan rank and do not trigger weekly recruit economy.   
- Player-facing weekly numbering is now 1-based while historical internal week keys remain unchanged.   
- `SYSTEMES_PREVUS.txt` now preserves future ideas separately: session-based online co-op, arena matchmaking, strictly in-world RP AI conversations with Dinotofu, FireFlight AI only as an unlikely/experimental idea, GUI handled by the user's friend, pixel art last, and story paused until explicitly requested with chapters 1–3 as the minimum playable target followed by a development-in-progress message.   
- Project tests protect real hiring, friendship, motivations, paid/unpaid wages, shared objectives and rescue traces.   
- Save schema remains **26**. Important checkpoint remains **V3.51.15**.   

---   

## V3.51.17 — Inventory pins, favorite ammunition and canonical history   

- V3.51.16’s generic **Favorite** preference is split into two concepts: **Pinned** only controls inventory ordering, while **Favorite ammunition** is reserved for automatic projectile selection. Pinning an item can therefore never make it auto-consumed.   
- Weapons, armor, consumables and materials can be **pinned**. Pins sort to the top in interactive lists and in the View all summary, using the `[PINNED]` state internally represented by the French terminal label `[ÉPINGLÉ]`. Locking and tracking remain independent.   
- Ammunition favorites are restricted to actual projectile families: arrows, bolts and throwing knives. Only one favorite is kept per family, so an arrow favorite and a bolt favorite can coexist without ambiguity.   
- **Favorite if available** remains an ammunition-only selection mode. Magic materials no longer expose favorite automation: they stay on **Always ask** by default or **Cautious automatic** when explicitly enabled.   
- V3.51.16 compatibility: legacy `favoriteItemKeys` migrate to **pins** to preserve their visual-order intent. When a legacy favorite is real ammunition, it also migrates to an ammunition favorite. New saves write `pinnedItemKeys` and `favoriteAmmunitionIds`. Save schema remains **26** because the old format is still read.   
- The `CharacterHistoryMenu` skeleton becomes an actual **Stats → Character history** register. It exposes already-persisted durable events plus real purchases, sales, buybacks and equipped gear activity.   
- Shop transactions now feed the canonical journal: purchases, quantity sales and buybacks are aggregated by entry. Actually equipping a weapon or armor also feeds the `equipements_portes` record. No retroactive event is invented.   
- Save and inventory tests guard Pin/Favorite separation, Always ask defaults, one favorite per ammunition family and backward compatibility.   
- Important checkpoint remains **V3.51.15**. Save schema remains **26**.   

---   

## V3.51.16 — General stabilization, inventory preferences and balance guardrails   

- Inventory gains three persistent preferences per item/material: **Favorite**, **Locked** and, for materials, **Tracked**. Favorites are sorted to the top without altering physical stacks; locks add an extra confirmation before sensitive voluntary uses; tracked materials print a strongly highlighted uppercase acquisition banner.   
- **Ammunition** and **magic-material** selection policies are now persistent. The explicit default remains **Always ask**. Optional modes are **Favorite if available** with prompt fallback, and **Cautious automatic**, which favors ordinary ammunition or less-rare magic supports instead of silently burning premium resources.   
- Material preferences survive stock reaching zero. Old saves without the new optional fields load with Always ask and no invented favorite/lock/tracking state. Save schema stays **26** because the additions are optional and backward-compatible.   
- Inventory locks now cover the important voluntary consumption paths: selling, inventory equip/use, combat potions, magic materials, crafting, quest deliveries, challenge-mark exchanges, enchanter runic services, church proofs/service vouchers, black-market barter, municipal repair requests, chest tools, tactical materials and expedition rations/travel/survival preparations. Refusing a locked item keeps it and prevents silent substitution with the next consumable.   
- Locks deliberately do **not** make internal state markers or items lost through forced world/combat/death consequences invulnerable. They prevent player mistakes, not game consequences.   
- Local-service time text also replaces the technical word `segment` with **day moments**, matching the expedition RP presentation.   
- Stability auditing exposed a false crash caused by an incremental build mixing stale objects with the changed `Player` layout. A **full clean rebuild** confirmed this was not a gameplay defect and the rebuilt codebase is healthy.   
- Balance numbers are not changed blindly without playtesting. Instead, `MagicResourceBalanceTest` now enforces the design invariant that **Material Sacrifice** costs more mana and bursts harder for one turn while remaining less power-per-mana efficient over time than normal material channeling across representative rarities and qualities. Better quality must not invert into a worse profile.   
- `SYSTEMES_PREVUS.txt` is fully rewritten so completed systems no longer masquerade as TODOs. It now separates story/world work, playtest balance, economy/traceability, combat/clan/companions, death/inheritance, oaths, records, GUI and technical skeletons that are **not** active bugs.   
- The **View all** inventory summary now follows favorite ordering too and clearly exposes `[FAVORITE]`/lock/tracking state through the same underlying preferences instead of leaving the global summary out of sync with interactive submenus.   
- Tracking no longer prints a false **TRACKED ITEM ACQUIRED** banner when an already-owned mineral merely loses one quality tier. Save loading also clears in-memory preferences before reconstructing possessions, preventing reloads from looking like fresh tracked loot.   
- Direct character-to-character exchanges now respect inventory locks: transferring a locked weapon, armor, consumable or material requires an extra one-time confirmation.   
- RP time wording is normalized across player-facing travel, lodging, church, reputation, guild and local-service screens: they now speak in **moments of the day** rather than the technical word `segment`. The word remains only for actual creature anatomy.   
- Natural combat mana regeneration keeps its original early-game rhythm but is now capped at **5 mana per turn**. Without that ceiling, level-255 pools could regenerate more mana than several real spell costs and make catalyst casting effectively free at the top end.   
- Important checkpoint remains **V3.51.15**, the actual compatibility boundary for modern material magic. V3.51.12 remains a late-game progression pass and is not retroactively relabeled.   

---   

## V3.51.15 — Important transition checkpoint for modern material magic   

- **V3.51.15 becomes the new important save checkpoint.** Any character last adapted before this milestone must complete the transition ritual once, with Dinotofu first creating the existing non-overwriting pre-update backup.   
- The **save schema remains 26**: no new structural persisted field justifies a fake schema bump. The new systems reuse already saved material IDs, quantities, qualities, languages and skills.   
- The V3.51.15 transition is deliberately **non-destructive**: no material stack is converted, no quality is changed, no ore is dissolved and no grimoire/scroll is removed. New rules only apply to future channeling, study and use.   
- Heavy adaptation now distinguishes eras: historical starter-kit/ammunition/potion repairs remain limited to saves truly older than the V3.50.33 checkpoint. A recent character merely crossing V3.51.15 therefore receives no unrelated free item.   
- The checkpoint UI explicitly explains why the transition exists and states before confirmation that physical stacks, quantities and qualities will be preserved.   
- This checkpoint closes the 3.51.08–3.51.14 sequence: support selection, elements, recipes, quality degradation, Material Sacrifice, study prerequisites, exact recipe qualities, late 100–255 progression and synchronized material journal.   
- Checkpoint/migration tests are strengthened to guard the non-overwriting backup, mandatory transition for pre-3.51.15 saves and absence of unrelated starter-kit repairs for a recent save.   

---   

## V3.51.14 — Material journal synchronized with physical magic   

- The Material Journal now explains actually observed magical uses instead of remaining limited to older crafting rules. For every discovered magical material it shows the channeling profile tied to the observed quality: relative mana cost, stability and power bonus.   
- Mineral fuels are identified explicitly and the journal explains that normal channeling lowers their quality by one tier rather than immediately consuming them.   
- Elemental affinities of discovered materials are visible (Fire, Cold/Ice, Earth, Water, Wind, Lightning and Arcane), making the support priority used by spells understandable from inside the game.   
- Multi-material recipes are not revealed for free: a formula appears in the journal only after every ingredient has actually been observed. The journal then shows quantities and whether each ingredient is sacrificed or merely degraded.   
- `MagicMaterialRules` now exposes known magical-material identification and elemental affinities centrally so combat and journal share the exact same rules.   
- Tests and source guards protect journal synchronization, affinities and the absence of premature recipe reveals. Save schema and important checkpoint remain unchanged while preparing V3.51.15.   

---   

## V3.51.13 — Exact-quality recipes and genuinely difficult grimoires   

- Multi-material recipes no longer silently pick the first available quality. When an ingredient exists in several quality stacks, the player now selects the exact stack for every component before final confirmation.   
- The combined recipe profile (mana, stability and power) is computed from the qualities actually selected, so exceptional material can be deliberately invested without being silently replaced by a weaker stack.   
- Consumption is transactional at exact-stack level: every quantity/quality is revalidated before the first mutation. Sacrificed ingredients remove the confirmed quality while degradable fuels lower the exact selected stack.   
- Several specialized grimoires now demand genuine `conversation`-level reading: Dwarven 2 for Frost Needle, Celestial 2 for Mana Suture and Dark Elven 2 for Occult Brambles. Scrolls intentionally remain easier because their prepared pattern already carries part of the formula.   
- Abnormal Notation is not artificially pushed to level 3: it remains an unstable fragmented script, mainly gated by level, mana, magical knowledge and motif recognition.   
- `MagicResourceBalanceTest`, `MagicStudyRulesTest` and `scripts/test_project.sh` guard recipe quality selection, transactional validation and the stricter language thresholds. Save schema and important checkpoint remain unchanged.   

---   

## V3.51.12 — Late masteries across the 1–255 curve   

- Generic progression now genuinely uses levels above 100 without flooding combat with unrelated buttons. Existing techniques gain **late mastery forms**, preserving their identity while creating meaningful long-term milestones.   
- At **level 120**, universal chains gain a fourth impact: Quadruple Strike, Quadruple Shot, or **Channeled Resonance** for magic profiles. The magic form uses a 40-mana base before material/catalyst/incantation adaptation, while ranged chains still require one real projectile per shot.   
- At **level 150**, line sweep reaches a mastered form with a smaller offensive penalty and 3-turn recovery instead of 4.   
- At **level 180**, cautious rally becomes **Veteran Rally**, adding stronger guard plus short regeneration without turning into a full heal.   
- At **level 210**, opportunistic execution becomes **Master Execution**, widening the wounded-target window from 40% to 50% HP and slightly improving its conditional bonus.   
- At **level 240**, Guard Break reaches its expert form: weakening/vulnerability last one extra turn and recovery falls from 3 to 2 turns. Level 255 remains the cap without an artificial mandatory capstone.   
- `TechniqueProgressionRulesTest` and `scripts/test_project.sh` guard milestones 120/150/180/210/240. Save schema and important checkpoint remain unchanged.   

---   

## V3.51.11 — Exact-quality channeling and ordinary prologue routine   

- The **minerals/materials** path now distinguishes the **exact quality stack** during normal channeling as well as Material Sacrifice. Impure, normal, pure/high and exceptional stacks are listed separately with stock, mana cost, stability and power preview so the player genuinely chooses which quality to consume or degrade.   
- Mineral fuels now call `degradeMaterialQualityByIdAndQuality(...)` after confirmation, while disposable components use `removeMaterialQuantityByIdAndQuality(...)`. A high-grade support can no longer be silently replaced by whichever stack the engine considered first.   
- The deterministic non-interactive casting path records the exact quality that normal rules would select, keeping tests aligned with the player-facing engine.   
- The prologue gains an **ordinary pre-climb routine** between the opening and the contract: rations, straps, gloves, equipment checks, passing haulers and jokes about the evening inn reinforce the feeling of a normal workday shared by an experienced party. No player-facing text reveals the memory/technical nature of the sequence before awakening.   
- `MagicResourceBalanceTest`, `StoryPrologueMemoryTest` and `scripts/test_project.sh` guard the new behavior. Save schema and important checkpoint remain unchanged.   

---   

## V3.51.10 — Conditional magic study and quality-targeted material sacrifice   

- **Magic books and grimoires** no longer turn ownership directly into mastery. A new central `MagicStudyRules` module can require minimum character level, sufficient maximum mana reserve, magical-knowledge score and, for selected texts, understanding of a specific language. Requirements intentionally vary between formulae.   
- Difficult grimoires now remain useful when acquired too early: a blocked study attempt explains every missing prerequisite and the same physical book can be **read again later** after progression. Shop acquisition and inventory rereading share the same centralized rules.   
- Languages become real study prerequisites for some traditions, including Dwarven for Frost Needle, Celestial for Mana Suture, Dark Elven for Occult Brambles and Abnormal Notation for Resistance Rift. Existing persisted language knowledge is reused; no save field is added.   
- **Spell scrolls** now use lighter but real level, maximum-mana, knowledge and optional-language requirements. Their sealed trace still makes them easier than permanent study, but an unusable scroll is no longer consumed merely because the character tried too early.   
- **Magical knowledge** is now a readable score derived from Intelligence/Wisdom, magic specialization, already studied formulae and a few relevant masteries. Character Stats show both the score and its label (`rudimentary`, `solid notions`, `confirmed`, `advanced`, `scholar` in the French UI). Inspecting a magic book shows each prerequisite with `[OK]` / `[À ATTEINDRE]` before study.   
- Material-path support quality and intrinsic rarity now genuinely affect **mana cost**, **stability** and **channeled power**. Better and rarer supports are more efficient/stable, with capped bonuses kept intentionally conservative for later playtesting.   
- The contextual **Material Sacrifice** technique unlocks at **level 60** on the 1–255 progression curve. It is intentionally less efficient over time than normal material channeling: much higher mana cost plus immediate destruction of one physical unit, in exchange for converting all remaining quality and rarity into a large one-turn burst.   
- Material Sacrifice now targets the **exact quality stack** chosen by the player. If the same ore exists as impure, normal and exceptional stacks, each sacrifice option exposes stock, mana, stability and power separately. The engine removes exactly the confirmed quality and refunds mana if that exact unit disappears before commitment.   
- Multi-material recipes keep their transactional behavior and now benefit from a combined quality/rarity profile. Normal mineral channeling still degrades fuel one quality step at a time while Material Sacrifice destroys one selected unit immediately, preserving the intended burst-versus-efficiency tradeoff.   
- A side audit fixed three tactical synergies (`Elemental Ward`, `Elemental Mark`, `Mana Suture`) that incorrectly queried grimoire-learned formulae as passive skills. They now recognize the active skills actually granted by study.   
- `MagicStudyRulesTest`, `MagicResourceBalanceTest`, `TechniqueProgressionRulesTest` and `scripts/test_project.sh` guard prerequisites, rereading, languages, exact-quality sacrifice and tactical grimoire synergies. Save schema, migrations and the important checkpoint remain unchanged.   

---   

## V3.51.09 — Interactive Channeled Echo and visible incomplete recipes   

- The real player-facing **Channeled Echo** path now uses the same physical-support selector as other minerals/materials spells. Players can inspect support quantity, quality and the exact effect before confirmation instead of letting Echo silently consume the first available component. The non-interactive `prepareMagicTechnique` path remains available for deterministic tests and technical callers.   
- Echo's material affinity is inferred from an obvious specialization when possible: Pyromancer → fire, Cryomancer → cold, Electromancer → lightning, Hydromancer → water, Geomancer → earth and Aeromancer/wind → wind; other profiles safely fall back to arcane.   
- A **known but incomplete recipe** no longer disappears from the support menu. It remains visible as a disabled `Recette dédiée (incomplète)` entry with the exact missing ingredients and quantities, letting players understand a formula before they can execute it.   
- `MagicMaterialRules::missingSpellMaterialIngredients` centralizes missing-ingredient calculation. `MagicResourceBalanceTest` checks missing previews/quantities and `scripts/test_project.sh` guards Channeled Echo's interactive path plus incomplete-recipe visibility.   
- Mana costs, three-path ratios, spell power, material-quality balance, save schema, migrations and the important checkpoint are unchanged.   

---   

## V3.51.08 — Player-selected magic materials, confirmed recipes and 255-level grimoire progression   

- The **minerals/materials** casting path no longer silently consumes the first available component during real combat spells. Players now choose their physical support in a dedicated preparation screen. Element-matched supports are listed first while generic channeling materials remain valid fallback choices.   
- Before any resource is spent, preparation displays the real material name, available quantity, the **next quality tier that will be used**, and whether the action consumes one unit or degrades a mineral fuel by one quality step toward dissolution. A separate explicit confirmation is required before committing the support.   
- Players may deliberately **preserve their materials** and force the existing pure-incantation fallback instead. This keeps the already-defined fallback mana/stability/recovery trade-off; this pass does not invent quality-based damage bonuses.   
- Elemental material families are expanded and connected to actual spell preparation: fire, cold/ice, earth, water, wind, **lightning** and arcane supports are recognized. Element-selecting spells now pass their chosen element into material preparation, while fixed spells use their own matching family.   
- **Multi-material spell recipes** are no longer data-only. When all ingredients are present, a recipe appears as its own choice, previews sacrificed versus degraded ingredients, and requires confirmation before execution. Resources are committed only after mana validation; if the recipe somehow becomes incomplete between selection and channeling, the mana is refunded and the spell is cancelled safely.   
- Mineral-fuel degradation is generalized beyond Runic Iron Shard to Cold Iron Nail, rare rune cores, Unstable Core and Elemental Fusion Core.   
- The character's magic-support summary now counts usable elemental supports as well as the four historical generic components and reports the next quality tier that would be used.   
- A second audit of the **1–255 progression** found that grimoires still bypassed the new curve by teaching advanced formulas around levels 3–9. Study thresholds are now centralized through `TechniqueProgressionRules::magicStudyUnlockLevel`: Mark 8, Binding 15, Frost Needle 20, Elemental Ward 25, Occult Brambles 25, Mana Suture 30 and Resistance Rift 55. Books can still teach earlier than natural class progression without erasing the progression curve.   
- Older saves that already learned a spell too early remain compatible, but combat now also enforces the study-level threshold before the learned formula becomes usable. Shops use the same centralized rules instead of hard-coded 3/4/5/6/7/9 checks.   
- `MagicResourceBalanceTest`, `TechniqueProgressionRulesTest` and `scripts/test_project.sh` protect support selection, elemental families, quality previews, recipe execution, mineral degradation and grimoire level gates.   
- No save-schema, migration, important-checkpoint or mana-balance changes. The 45% / 75% / 120–125% ratios, Channeled Echo 18/28 costs and zero-mana exhaustion remain **live-playtest** topics.   

---   

## V3.51.07 — RP expedition time and visible current character day   

- **Exploration/expedition** screens no longer expose the technical word “segment”. Internal time units remain unchanged, while player-facing text now talks about **day moments** and explicitly presents the RP cycle: **Matin (1/5) · Midi (2/5) · Après-midi (3/5) · Soir (4/5) · Nuit (5/5)**.   
- `Player::formatWorldDayPartLine()` now consistently renders **Matin (1/5)** instead of `Matin 1/5`, including before/after time transitions.   
- Expedition menus show the **current moment**, the five daily landmarks, travel/duration/autonomy and exposure timing without leaking the old technical terminology.   
- The character's **current in-world day** is now visible in the Stats summary, compact Character Stats, long-form stats and saved-character selection. Selection reads the already-persisted `worldDaysElapsed` and `worldDayProgressUnits`; no new save field is introduced.   
- Journal/history and recruit infirmary displays now convert internal 0-based elapsed-day indexes into 1-based RP day numbers, preventing player-facing “day 0” while preserving all internal deadlines and comparisons.   
- A new `WorldTimePresentationTest` protects all five moments, the Day 1 → Day 2 transition and time-change formatting. `scripts/test_project.sh` also prevents the word “segment” from returning to `QuestExplorationMenu.cpp` and protects visible current-day information on character sheets.   
- No save-schema, migration, important-checkpoint, balance-value or advanced material-magic changes in this pass. Advanced material recipes/selection remain intentionally deferred as decided in V3.51.06.   

---   

## V3.51.06 — Non-material stabilization: tactical progression, unified inn and real-ammo salvos   

- Development of **advanced magic-material selection and recipe execution is intentionally parked** for this pass at the user's request. The V3.51.05 foundations stay intact and no additional recipe sacrifice is enabled automatically.   
- The **1–255 progression** audit found a real bypass in `PlayerWaveTacticalActionMenu`: many actions could be used very early from affinity/mastery alone. All **51 tactical actions** now have a centralized minimum level through `TechniqueProgressionRules::tacticalActionUnlockLevel`, on top of their previous contextual requirements. Basic gestures remain early while advanced control is spread out, e.g. Guard Break 5, Shadow Step 10, Arc Strike 24, Pierce Rampart 35, Formation Break 40, Summon Link 45, Blade Dance 50 and State Chain 55. An execution-side guard also prevents bypassing the menu.   
- The **inn** no longer has two competing service paths: the world menu delegates meals and common beds to `ShopCityServiceMenu::openLodgingServiceMenu`, which correctly handles vouchers, subscriptions and exact direct payment. The premium safe room remains a separate direct expense. Central meals and nights also feed the canonical fatigue/rest journal again.   
- Multi-hit ranged techniques now check **real compatible ammunition** before and during a salvo. If ammunition runs out during Double/Triple Shot, sweep or ricochet, the salvo stops instead of turning the remaining projectiles into emergency bow/crossbow melee hits. The intentional no-ammo fallback attack remains available separately and keeps its heavy durability wear.   
- `StoryCampaign.cpp` is cleaned of its two known warnings: the unused `buildLongIntroductionLines` parameter is unnamed in the definition and `specialStoryHookLines`, confirmed dead and never called, is removed. Active special prologue reactions remain in their dedicated paths.   
- The **prologue** audit confirms the intended ordering: normal day/mission, combat, distinct aftermath for victory/retreat/defeat, then the Mist and only afterwards waking. No unnecessary narrative rewrite is made.   
- The **physical-money** audit confirms combat/exploration recaps still use real coin stacks and stack deltas; remaining `Money::formatCopper` calls represent costs/accounting values rather than silently normalizing the purse.   
- `scripts/test_project.sh` now guards tactical progression, the unified inn path, meal/rest journal events, real-ammunition salvo interruption and removal of the dead narrative helper. A full **clean C++23 build** completed without warnings, followed by the complete test suite before versioning.   
- Save schema, migrations, important checkpoint and mana balance values are unchanged. Remaining questions about mana feel, real DPS/durability, voucher comprehension and prologue immersion are now **playtest questions**, not known code defects.   

---   

## V3.51.05 — Evolving material magic and 1–255 technique pacing   

- Player technique progression now respects the real **level 255 cap** instead of concentrating almost everything between levels 2 and 12. Simple survival gestures remain early, while stronger or more specialized techniques are spread through **40, 50, 65, 75, 80 and 100** where appropriate.   
- Generic chains are rescaled: Double Strike/Shot unlocks at **level 8**, Channeled Echo at **level 15**, and Triple variants at **level 35**. Generic active techniques now use centralized `TechniqueProgressionRules`, from cautious rally at 3 and Guard Break at 5 up to line breach 40, field suture 50, ricochet 65, opportunistic execution 80 and unstable mixture 100.   
- Class techniques are spread as well: first class signatures generally begin around **8–10**, while advanced techniques reach **75**. Class identity skills are no longer all granted at level 5; Assassin and Mage signatures, for example, begin at level 10.   
- Natural weapon-practice unlocks still require their real combat-use milestones but now also require a **minimum character level**. Training grounds enforce the same floors, so money can no longer bypass the intended progression gate.   
- The **minerals/materials** casting path gains a first real **mineral-fuel** behavior. Runic Iron Shards no longer have to vanish in a single use: channeling degrades their material quality by one step, and the lowest-quality unit dissolves. Inventory now supports degrading one physical material unit while preserving quality-separated stacks.   
- `MagicMaterialRules` now maps materials to **elemental affinities** (fire, cold/ice, earth, water, wind and arcane), preparing elemental spells to require coherent physical supports instead of one universal component.   
- A **multi-material spell recipe** structure now distinguishes consumed ingredients from mineral fuel that only degrades. Data recipes already exist for *Frost Needle* (`Aiguille de givre`) and *Controlled Overload* (`Surcharge contrôlée`). They are deliberately **not auto-executed yet**: valuable items must not be sacrificed before the player gets a real material/recipe choice UI.   
- `MagicResourceBalanceTest` now covers elemental affinity, recipes, ingredient availability and mineral degradation/dissolution. A new `TechniqueProgressionRulesTest` protects the 255 cap and key progression milestones.   
- The **essential backup** process is hardened after detecting an archive that accidentally omitted source modules under `include/save` and `src/save`. A dedicated script excludes only private runtime data folders such as `saves/` and generated outputs; the regression suite explicitly protects the source save modules.   
- Save schema, migrations, the important checkpoint and mana-balance values are unchanged. Advanced recipe execution and player material choice intentionally remain future work pending UX validation.   

---   

## V3.51.04 — Readable exploration resources and centralized magic-material rules   

- The four physical components usable by the **minerals/materials** casting path are now defined by a central `MagicMaterialRules` module instead of being duplicated between combat and the Stats display. The existing priority remains unchanged: **Arcane Dust → Runic Iron Shard → Moonlit Salt → Echoing Resin**.   
- `CombatActions` now asks the central rules for the first actually available component, while `Player::getMagicSupportStatusLine()` uses the same source to count and format compatible stock. Future list changes can therefore no longer silently desynchronize combat behavior and UI reporting.   
- The availability audit found no progression blocker requiring an arbitrary drop buff: Arcane Dust appears very early in exploration, Echoing Resin and Moonlit Salt arrive in intermediate biomes, while Runic Iron Shard intentionally has a more workshop-oriented source profile and is always stocked by blacksmiths in addition to vendor/enchanter rotations. No drop chance or price is changed in this pass.   
- **Exploration** cards no longer expose internal material IDs such as `worn_leather_piece` or `arcane_dust` in the Common/Rare field. They now resolve player-facing catalog names instead.   
- `MagicResourceBalanceTest` protects the centralized list, priority order, localized names and stock counting/formatting. `scripts/test_project.sh` also protects the combat/Stats integration and rejects the previous exploration line that exposed raw material IDs.   
- No mana cost, recovery pace, stability, economy, save schema or migration rule changes. This is a data/UX stabilization pass before live balance playtests.   

---   

## V3.51.03 — Three-path magic readability and real support status   

- **Mineral / material** channeling no longer leaks technical item IDs (`arcane_dust`, `runic_iron_shard`, etc.) into player-facing messages. Prepared components now use their actual localized material names.   
- `Player` now exposes one centralized, readable summary of the **real magical support state**. Material casters see how many compatible components remain and which ones; catalyst users see the equipped staff, durability and whether it is functional or broken; pure incantation explicitly shows that no external support is required.   
- Magical recovery wording is centralized through `getMagicRecoveryLabel()` (`fast`, `standard`, `slow` conceptually; French runtime labels remain player-facing) instead of rebuilding the same condition in multiple menus.   
- Compact **Stats**, the detailed statistics summary and the combat screen now expose the actual magical support/fallback state, making the existing trade-offs visible before the player commits to an action.   
- `MagicResourceBalanceTest` protects recovery labels and support states for missing/available materials, missing/functional/broken catalysts and pure incantation. `scripts/test_project.sh` also protects the localized component display and the new support-status integration.   
- This pass intentionally changes no mana cost, stability percentage, mechanical recovery pace, save schema or migration rule; it makes the already-existing trade-offs **observable** before the next balance playtest.   
- Full C++23 build and `scripts/test_project.sh` passed after versioning: **all Dinotofu tests pass.** Binary is **V3.51.03**, save schema remains **26**, and the important checkpoint remains **V3.50.33**.   

---   

## V3.51.02 — Transactional magic channeling and broken-catalyst fallback   

- Magical preparation no longer consumes a **component** or one point of **catalyst durability** before confirming that the current mana reserve can actually start the spell. A mana-insufficient attempt now leaves inventory and staff durability untouched.   
- Mineral/material casters still reserve the selected component during preparation, but it is removed only after mana spending succeeds; once channeling really starts, the component is consumed even if the later stability roll fails.   
- Functional catalysts follow the same rule: the 1-point wear is applied only after channeling has begun. Indestructible equipment and the related cheat remain respected.   
- A **broken catalyst** no longer incorrectly keeps the normal 75% mana cost. It now truly falls back to direct incantation at **120% of base mana cost**, with degraded stability and the already-defined slow recovery from `getMagicRecoveryAdjustment()`.   
- `MagicResourceBalanceTest` now covers insufficient-mana attempts without resource loss, broken-catalyst fallback, and normal consumption/wear once an attempt starts. `scripts/test_project.sh` also protects the delayed-consumption guards.   
- Full C++23 build and `scripts/test_project.sh` passed before versioning: **all Dinotofu tests pass.** Save schema remains **26** and the important checkpoint remains **V3.50.33**.   

---   

## V3.51.01 — Physical currency and mana-path stabilization   

- Exploration recaps now truly match the physical wallet: they show **exact stacks before/after**, denomination-level changes (`+1 PE | -4 PC`, etc.) and total PC only as an accounting value. The UI no longer rebuilds an “ideal wallet” that could be mistaken for automatic conversion.   
- `Money::formatCoinStackDelta(...)` now centralizes physical-stack differences and is shared by the relevant recaps to prevent diverging currency representations.   
- **Prepaid inn voucher** economics are regression-tested: five-use booklets remain slightly cheaper than five direct services without becoming an excessive discount exploit.   
- Magic-capable classes that could still fall back to a minor reserve are now assigned to their intended casting paths, including Druid, Shaman, Priest, Paladin, Templar, Necromancer, Pact-bound, Electromancer and Mirage-oriented profiles.   
- **Channeled Echo** (the magical Double/Triple chain) no longer uses a flat mana subtraction independent of casting style. It goes through real magical preparation: materials/components, catalyst durability/stability, pure-incantation cost/risk, and possible zero-mana exhaustion.   
- The three paths now have a **mechanical recovery pace** in addition to cost differences: a functional catalyst speeds recovery, pure incantation slows it, while mineral/material casting keeps standard recovery and relies on strong mana efficiency. Missing or broken catalyst support causes slower fallback recovery.   
- The Stats screen exposes that magical pace (`fast`, `standard` or `slow`) beside the mana pool without exposing internal formulas.   
- `MagicResourceBalanceTest` protects path assignment, reserves, mana scaling and recovery pace; `EconomyScaleTest` also protects physical coin deltas and measured voucher value.   
- Full C++23 build and `scripts/test_project.sh` passed before versioning: **all Dinotofu tests pass.** Save schema remains **26** and the important checkpoint remains **V3.50.33**.   

---   

## V3.51.00 — RP immersion, mana, skill progression and lived-world UX   

- Exploration and combat recaps no longer display an “ideal wallet” reconstructed from total value. They now snapshot the **five actual physical coin stacks** before/after and show denomination-by-denomination changes; total PC remains only as a secondary exact accounting value. No automatic currency conversion was introduced.   
- Inns now clearly separate **immediate services** from **prepaid vouchers**. Formerly ambiguous meal/bed shop items are five-use voucher booklets, categorized as prepaid vouchers, slightly cheaper than five direct purchases and explicitly described as not consuming the service on purchase. The inn-use menu shows owned vouchers and direct services charge **exact PC**.   
- Play time is visibly exposed in Terminal character selection together with exact total PC value. Accounts also persist their own **historical total play time**, accumulating character sessions even after death or deletion.   
- The Character menu gains a compact **Stats** screen with identity, race, class, level, physical wallet, exact PC value, real play time, HP, mana, damage, critical rate and equipped protection.   
- Emergency bow/crossbow melee attacks without ammunition now damage the weapon heavily: **5 durability** instead of the normal 1. No blind global ranged-damage nerf was applied after audit; higher ranged offense remains paired with ammunition pressure and lower-HP profiles pending live playtests.   
- The twelve generic active techniques are no longer all available at level 1. **Guard Break** and cautious rally unlock early, followed progressively by chains, stops, sweeps, breaches and advanced techniques. Names adapt to weapon style: melee Double/Triple Strike, ranged Double/Triple Shot, and a dedicated magical variant. Every ranged hit still consumes its own ammunition.   
- Dinotofu now has a real persistent **mana bar** (`current / maximum`), saved with the character and shown in combat/Stats. Mana regenerates slowly over turns, recovers through rest and can be restored by a new mana potion. Legacy saves without mana fields safely load at full reserve.   
- Reaching exactly **0 mana** can trigger magical exhaustion: roughly a 75% chance to faint, usually for one turn and sometimes two. Exhaustion is intentionally not guaranteed, preserving uncertainty and rare overexertion moments.   
- Three casting paths are distinguished: **minerals/materials** (small pool, strongly reduced mana costs but consumed components), **wand/catalyst** (reduced costs, improved stability and support durability), and **pure incantation** (no required support, slower/more expensive and exposed, but slightly stronger). Missing materials/catalysts can fall back to pure incantation.   
- The pre-Mist prologue is rewritten for **full in-world RP**. It begins as an ordinary day among experienced adventurers—gear, familiar banter, guild contract and routine departure. The player is never told they are in a memory, using a temporary build or playing a progression-free encounter.   
- Memory gaps exist without explanation: variable name fragments (`Sca---`, `--arle--`, `S--rlett`, `Lor---`, etc.) and partially erased contract/route/reward/dialogue text. They represent present-day residual recall without exposing their nature before awakening. The **white Mist/smoke only arrives chronologically after** the fight and its aftermath, then the erasure intensifies.   
- After awakening, interpretation remains diegetic: the character may only suspect the experience was an old memory, without knowing exactly when or who accompanied them. Player-facing meta wording such as “playable memory”, “temporary build” or “no XP/loot” has been removed.   
- The updater, migrations, non-overwriting pre-migration backup and important checkpoint **V3.50.33** were audited and validated by the regression suite and intentionally left unchanged.   
- Full C++23 build and `scripts/test_project.sh` passed before versioning: **all Dinotofu tests pass.** Save schema remains **26** and the important checkpoint remains **V3.50.33**.   

---   

## V3.50.36 — Update notes   

- Fixed Update script.   
- Better compiling scripts.   
- Faster compiling speed.   

---   

## V3.50.35 — Update notes   

- Giving people back the display mode of the game.   
- Fixing a bug when executing the GUI mode.   

---   

## V3.50.34 — Prologue stabilization and important V3.50.33 checkpoint   

- **V3.50.33 officially becomes the new important save checkpoint.** Characters last adapted before V3.50.33 go through the existing non-overwriting pre-update backup and transition ritual; saves already adapted on V3.50.33 or later are not prompted again. Save schema remains **26**.   
- The memory battle now applies the game's official difficulty multipliers to **pack HP and damage**: Easy genuinely softens the encounter, while Hard/Nightmare/Lethal progressively strengthen the pack leader and frost fangs. Difficulty therefore no longer affects almost only escape odds in this special prologue battle.   
- **Victory / retreat / defeat** now receive more distinct emotional transitions before converging into the Mist: retreat reads as an experienced team decision, while defeat emphasizes that a companion physically comes back for the player instead of abandoning them.   
- The new erasure sequence now consistently uses **White Mist** wording, and the level-42-to-level-1 contrast is reinforced by the body instinctively searching for a weapon and strength that are no longer there.   
- `StoryPrologueMemoryTest` now protects actual difficulty scaling and three distinct outcome transitions in addition to the existing temporary-build, equipment and erased-name guards.   

---   

## V3.50.33 — The last hunt before the White Mist   

- The opening prologue is fully rebuilt as a **playable memory of the character's last day before erasure**, not as a computer interface malfunction. Broken text, truncated names and missing information are memory holes already being eaten by the White Mist.   
- The memory uses an **advanced temporary copy** of the created character: same name, class and identity, memory level 42, reinforced combat stats, class-appropriate high-end gear, consumables and already-unlocked skills. This copy is isolated and discarded after the memory; it never mutates the real saved character.   
- Skill refresh now supports a silent mode reserved for temporary reconstructions: the memory build gets its techniques without fake “NEW SKILL” popups, while normal player progression keeps the existing visible unlock behavior.   
- Memory equipment now distinguishes the major playable families: crossbow users receive an actual crossbow with frozen bolts, ranged archers receive bow/ammunition, magic/invocation/healing profiles receive staff/robe gear, assassins use daggers, spear users keep spears, heavy profiles receive a hammer, and remaining martial cases use the runic fallback. Representative tests prevent obviously mismatched memory loadouts from returning.   
- The last mission takes place in an already-existing dangerous zone, the **Glacier des Serments froids**, hunting a **Chef de meute du Serment froid** with two **Garde-crocs givrés**. The encounter uses the real turn/wave combat engine rather than a text-only cutscene.   
- Three pre-fight approaches — tight formation, fast hunt or normal rhythm — apply small different advantages so the travel choice matters without deciding the fight automatically.   
- The two experienced allies have fixed internal design identities, but their real names are deliberately absent from runtime prologue files. The memory only displays eroded variants such as “Sca—”, “S…lett?”, “Lor—”, “L?ren…”, “[nom arraché]” or “[nom perdu]”. `scripts/test_project.sh` explicitly rejects any leak of their real names into runtime prologue files.   
- During combat, the player can issue **team orders**: focus the pack leader, finish the weakest target, request cover, or let the allies act freely. Companions attack, heal, apply vulnerability/ward/regeneration and periodically perform coordinated actions so the party feels experienced and familiar with each other.   
- **Victory, voluntary retreat and defeat are all valid outcomes.** Victory gives a short moment of relief; retreat/defeat forces the group to disengage. All three branches then converge into the same rising Mist, erasing objective, route, gear, skills and finally identities before a full white-out.   
- The memory battle is deliberately **outside persistent progression**: it grants no XP, money, loot, bestiary entry or durable stat to the present-day character. Existing special-identity reactions to the Mist are preserved.   
- After erasure, the existing forest survival sequence resumes with the **real unequipped beginner character**. The later corpse kit remains the generic class starter kit and is not presented as the advanced gear used in the memory.   
- `HISTOIRE_DINOTOFU.txt` now explicitly documents the hunt, progressive erasure, the companions' internal design identities, all three accepted outcomes, and the strict separation between the memory build and real progression.   
- Added the `StoryPrologueMemory` module and `StoryPrologueMemoryTest`; temporary-player creation, the pack, ally-name masking, mission fragments and Mist transition are regression-tested. The complete `scripts/test_project.sh` suite passes through its final **“Tous les tests Dinotofu sont passés.”** message before versioning.   
- Save schema remains **26** and the important checkpoint remains **V3.50.30**.   

---   

## V3.50.32 — Exact rewards and voluntary currency exchange   

- Quests can now define an **exact physical coin-stack reward** through `Quest::rewardCoins`. If a quest promises **17 PC + 2 PF + 3 PO**, those exact denominations are added unchanged to the wallet; no silent normalization or conversion occurs. Legacy rewards authored as abstract economy value keep their separate compact-payout behavior.   
- Exact quest payouts are serialized with the quest (`rewardCoinCopper`, `rewardCoinIron`, `rewardCoinElectrum`, `rewardCoinGold`, `rewardCoinPlatinum`) and survive a save/load round trip without denomination changes. These fields are optional, so save schema remains **26**.   
- The guild **Currency Exchange Desk** now asks for one of three intentions before selecting denominations: **Custom conversion**, **Everything toward highest denominations**, or **Everything toward the lowest denomination**. The former “ideal wallet” therefore remains available only when the player explicitly requests it.   
- Custom conversion lets the player choose the **source denomination**, **target denomination**, then the **number of conversion lots**. The menu displays the exact input/output ratio and the **maximum possible** before quantity entry; conversions may skip tiers directly (for example PC -> PO or PO -> PF) without value loss.   
- **Everything toward highest denominations** voluntarily compacts wealth into PP/PO/PE/PF plus the exact PC remainder; **Everything toward the lowest denomination** voluntarily flattens the whole wallet into PC. Both modes verify exact copper value before and after the operation.   
- The **“Premier éclat de platine”** milestone no longer waits for an unrelated skill refresh: monetary gains, sales, loot, rewards and exchange operations now trigger a dedicated economy-title refresh. Receiving or creating the first PP therefore grants the milestone immediately.   
- `EconomyScaleTest`, `SaveRoundTripTest` and `scripts/test_project.sh` now guard arbitrary conversions, both voluntary normalization modes, exact quest denominations, their persistence and the platinum-title refresh.   
- Full C++23 build validated; targeted economy/save tests pass, and the complete `scripts/test_project.sh` suite was validated through its final **“Tous les tests Dinotofu sont passés.”** message (split into two executions because of the environment time limit).   
- Save schema remains **26** and the important checkpoint remains **V3.50.30**. No main-story progression is added: **V3.50.33 remains reserved for the major update / pre-Mist prologue**.   

---   

## V3.50.31 — Physical money, social status and a modest start   

- Normal starting money is no longer 50 abstract PF: the purse physically contains **5 PF + 15 PC**, exactly **65 PC**. Other difficulties also use physical starter stacks.   
- Legacy economy-unit rewards are now paid in **compact physical denominations** without normalizing stacks already owned: 13 PF of value becomes 1 PE + 3 PF, while 100 PF becomes 1 PO.   
- Coin denominations now carry social meaning: copper = modest means, iron = ordinary money, electrum = visible comfort, gold = notable/noble-like wealth, platinum = exceptional fortune. Shops and inventory expose this perception from the real stacks held.   
- Added the secret title **“Premier éclat de platine”** for first possession of a platinum coin.   
- Current-wallet screens now show the **actual physical stacks** instead of recomputing an ideal denomination split from total value.   
- Character selection shows **total real play time and exact wealth in PC side by side**; `CharacterSaveSummary` now reads `totalCopperCurrency`.   
- Small living-cost references were tightened toward the intended medieval-fantasy scale: Valebrume level-1 warm meal **50 PC**, common bed **100 PC**, safe room **192 PC**. A 65-PC starter can eat, but must earn money before comfortably paying for lodging.   
- `EconomyScaleTest` now protects physical starter stacks, social tiers, natural PE/PO payouts and exact-copper purchasing power.   
- Save schema remains **26** and the important checkpoint remains **V3.50.30**. **V3.50.32** is reserved as stabilization margin and **V3.50.33** for the major pre-Mist prologue pass.   

---   

## V3.50.30 — Major economy checkpoint: purchasing power, units and regression guards   

- **Completed a cross-system economy audit** covering guild quests, ordinary combat, bosses, PvP, exploration, sales/buyback, city and church services, training, reputation repair, death penalties, travel, inns and municipal vaults. The current economy is broadly coherent on the authored **PF-unit scale**, so no destructive global x10/x100 rescale is applied merely to chase isolated-looking values.   
- Real purchasing power is now protected at **levels 1, 5, 10, 25 and 50**. `EconomyScaleTest` samples guild boards, checks average/maximum payouts and verifies that a representative wallet unlocks progressively more shop listings without making the whole catalogue trivial.   
- The **municipal vault** remains a medium-term goal: it stays above a representative level-10 wallet after a few contracts, while becoming realistically reachable later in progression.   
- **Combat income** is covered by the same audit. Ordinary humanoid rewards remain level-scaled pocket money, while a benchmark boss pays materially more without immediately funding major structural purchases. Friendly PvP remains symbolic and dangerous loot duels transfer wealth between players instead of minting new currency.   
- Exploration jackpots and carried coin purses were reviewed with their current probabilities and soft caps. They remain intentionally exciting without requiring a global scale change; exploration overflow compression continues to absorb extreme rolls.   
- Several **misleading currency displays** were fixed: training, equipment lists, inventory selections, potions, merchant buyback and the money cheat no longer display raw PF-scale values as literal `PO`/“gold”. They now use `Money::formatEconomyUnits`.   
- Ambiguous internal names were cleaned up before freezing the checkpoint: `starterGold` becomes `starterEconomyUnits`, `getStarterGold` becomes `getStarterEconomyUnits`, `fineCostGold` becomes `fineCostEconomyUnits`, `CombatReward` explicitly stores/exposes `economyUnits`, difficulty percentages refer to economy reward/loss, and `DeathPenaltyResult` exposes `lostEconomyUnits`. The persisted historical `Quest::rewardGold` field is intentionally retained for save compatibility and remains documented as PF units rather than literal PO.   
- The denomination ladder is regression-tested through large values: **1 economy unit = 1 PF = 10 PC**, **100 units = 1 PO**, **1000 units = 1 PP**. Systems authored in exact copper continue to stay exact-PC systems.   
- `scripts/test_project.sh` now rejects the reintroduction of several ambiguous economy identifiers and raw `PO/or` formatting in the audited menus. It also verifies that README current-status checkpoint lines match the actual important checkpoint.   
- `bump_version.py` now synchronizes README checkpoint status lines by pattern even if a README had drifted to an older checkpoint; historical changelog entries are never rewritten.   
- Full C++23 build and `scripts/test_project.sh`: **all tests pass before the 3.50.30 version bump**. A second post-version validation protects the new checkpoint.   
- Save schema remains **26**. **V3.50.30 becomes the new mandatory important-save checkpoint**, using the existing non-overwriting pre-update backup and transition ritual for older characters.   
- No main-story progression was added in this pass. The **pre-Mist prologue remains reserved for V3.50.31** as planned.   

---   

## V3.50.29 — Pre-checkpoint: special characters, navigation, combat readability and explicit units   

- **Willow**, **Dwarf** and **Badr** join the protected special-character catalog with the shared identity date **15/12/2025**. Willow is a 20-year-old human archer who travels alone and complains easily; Dwarf is a 24-year-old dwarf warrior and provocative joker whose flaws are not treated as virtues; Badr is a 48-year-old semi-human cleric with an undead-like body and the organic companion **Second**.   
- `SpecialCharacter` can now store known age/gender information without forcing legacy characters to define it. The new identities are wired through name protection, date validation, native-class bonuses, dialogue and progressive bestiary data.   
- The **Willow / Dwarf / Badr** trio plus two coherent sub-groups join special adventurer encounters. Group dialogue covers entrance, victory and defeat. Modest combat synergies make the team meaningful while **"En avant Second"** remains deliberately deferred.   
- Terminal selection is more forgiving: both menu readers now understand `help`/`aide`/`?` and `back`/`retour`/`r` when zero is a valid exit, report valid ranges/choices after invalid input and can redisplay menu descriptions. Legacy character/account menus benefit as well.   
- Combat turn screens now summarize active player statuses and explicitly show a pending defense posture, including its reduction/counter values and whether selecting defense again would replace it.   
- V3.50.30 economy-checkpoint preparation: helpers returning **exact copper (PC)** now use explicit `Copper` suffixes (`innCommonBedCostCopper`, `innSafeRoomCostCopper`, `innWarmMealCostCopper`, `cityVaultMaterialTransferCostCopper`, `routeRewardBudgetCopperForDistance`). Vault purchase/upgrade values remain documented PF economy units.   
- `EconomyScaleTest` now protects concrete starter purchasing-power landmarks and the separation between exact-PC costs and PF economy-unit prices.   
- Removed an accidental duplicate Sanctus combat condition found during the audit.   
- Regression guards cover Terminal navigation, combat-state readability, explicit economy-unit naming and the new special trio.   
- Full C++23 build and `scripts/test_project.sh`: **all Dinotofu tests pass** before versioning.   
- Save schema remains **26** and the important checkpoint remains **V3.50.20**. **V3.50.30 remains reserved for the major economy audit/checkpoint.**   

---   

## V3.50.28 — Physical currency, proportionate memory, visible crafting and prepared-attack defense   

- Non-combat interactions are no longer all promoted to permanent character history. The local/anti-farm journal still tracks resolved interactions, but only genuinely unusual discoveries are flagged as `notableForLongTermHistory`. A mundane, unseen action therefore no longer becomes an artificial world-defining event.   
- The guild gains a **Currency Exchange Desk** that rearranges physical coin stacks without creating or destroying value: one coin can be broken into ten coins of the next lower denomination, or ten lower coins can be combined into one higher coin. Total copper value is checked before and after each operation.   
- The full currency legend is no longer repeated throughout the UI. It is centralized at the exchange desk: **Copper Coin (PC), Iron Coin (PF), Electrum Coin (PE), Gold Coin (PO), Platinum Coin (PP)**, with the rule **10 coins of one tier = 1 coin of the next tier**. Other screens may naturally use abbreviations or full names depending on context.   
- `Money` and `Inventory` now expose an explicit physical-denomination API (`CoinType`, stack counting, lower/higher tier lookup, breaking/combining), giving future economy systems one consistent conversion path.   
- A structured-inventory economy remnant was corrected: weapon, armor, consumable and material values are no longer exposed as raw “gold” when the stored number actually represents historical economy units. They now use the same economy formatting as the Terminal UI.   
- **Crafting** is discoverable without first guessing it exists inside the inventory: the Character menu now has a direct “Crafting” entry, navigation descriptions mention it, and the quick menu has continuous numbering instead of artificial 8/9 jumps.   
- In combat, **defensive posture** now also mitigates prepared/telegraphed enemy abilities (charges, breaths, heavy shots, rituals, etc.). Their damage passes through `DefensePostureSystem`; ability-specific side effects may still occur, so defending mitigates the hit without deleting the mechanic.   
- Tests now protect exact value conservation during physical coin exchange, denomination names/abbreviations, one player-facing full currency legend, crafting discoverability, local-vs-durable exploration memory and real mitigation of prepared attacks under defensive posture.   
- Full C++23 build and `scripts/test_project.sh` validated.   
- Save schema remains **26** and the important checkpoint remains **V3.50.20**. **V3.50.30** is intentionally reserved for a future major economy audit/checkpoint, with V3.50.29 available as a preparation pass if needed.   

---   

## V3.50.27 — Weeping statue garden, guild variety and exploration traces   

- The **Garden of Weeping Statues** now has a signature location mechanic: some statues can change position only between two observations, without leaving movement traces in the gravel.   
- A dedicated Garden event lets the player physically mark statue positions, collect a mineral tear without touching the statues, approach the stone circle directly, or leave carefully. Outcomes can produce observations, resources, quest progress or combat.   
- Repeated Garden observations persist in character history. After enough comparisons the game can confirm an impossible displacement instead of treating every visit as an isolated scene.   
- Dangerous-site name, warning, boss trace, environmental hazard and field observation text for the Garden are now location-specific.   
- A new non-combat interaction, **Bouquet before the angel**, adds recent human traces to the mystery through footprints, an altered offering and cautious observation.   
- Six additional biomes receive dedicated non-combat interactions: **Gelatinous Pools** (neutral slime migration), **Cold Mountain** (collapsed cairn), **Collapsed Ruins** (mosaic under unstable structure), **Blue Mist Canals** (boat without a ferryman), **Abandoned Fair** (one-turn carousel) and **White Bone Quarry** (recent tool mark).   
- Resolved biome non-combat interactions now also leave a character-history trace in addition to the canonical anti-farm journal entry.   
- Guild boards now maximize objective-family variety among the templates actually available. With at least three visible offers, the board tries to mix at least three types among combat, exploration, service and bestiary work.   
- Tiny three-offer boards are no longer forced to contain three field jobs. From level 7 onward they preserve at least two combat/exploration offers while leaving room for service or bestiary work; larger boards keep the existing field-work minimum.   
- Added/expanded regression coverage for Garden identity, Garden non-combat content, guild-board diversity across several levels and field-work minimums.   
- Full C++23 build validated. `scripts/test_project.sh` was executed in chunks because of the environment execution limit and every section passed.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.26 — Social quest memory   

- Expired quests are no longer socially forgotten after archival: the relevant client now stores a local `quest_failed` fact.   
- Deadline failures are also recorded in the local canonical journal through `quetes_echouees`, with idempotent synchronization to prevent duplicate counting.   
- Turned-in quests now explicitly record `quetes_reussies`, alongside the existing `quest_completed` NPC memory.   
- NPC reactions to failure depend on profile and profession. Failure does not automatically become hostility or an arbitrary reputation penalty.   
- Declining a personal request now creates a `quest_declined` memory. Contacts can remember that the player declined, while clearly distinguishing it from a broken promise.   
- Appropriate local information networks may relay quest failures. Propagation remains sourced, local and confidence-decaying; no omniscience is introduced.   
- Main time-advancing paths that can expire quests now synchronize these social consequences.   
- Added regression coverage for expiry-to-client memory, idempotent local journaling, NPC reactions and `quest_failed` relay rules.   
- Full C++23 build and `scripts/test_project.sh` passed.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.25 — Local guild-board prioritization   

- The guild board now goes beyond checking whether a location matches the player level: already generated offers are ordered by their **coherence with the current city**.   
- Counter service contracts remain naturally local even when paperwork mentions an item or document originating from a remote zone. Generic village, road and creature-family jobs also remain well placed when no precise biome requires a long trip.   
- When a contract names a concrete biome, the system checks the city’s regional distance data: nearby, initially known biomes are shown before more distant or less familiar regions. A remote contract remains possible; it is merely deprioritized rather than artificially deleted.   
- This weighting complements V3.50.21’s real minimum-level filter, so beginners avoid both over-level locations and unnecessarily remote offers among the places they can legitimately visit.   
- Ordering is recalculated when the board is opened from the current city, allowing the same stored offer set to become more sensible after travel without changing the save format.   
- Added coverage asserting that, from Valebrume, a local Plains mission and a counter service rank ahead of an exploration contract for the Grey Drake Cliffs.   
- Full C++23 build and test suite were validated before versioning.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.24 — Minimal graphical welcome screen and clean Terminal handoff   

- The previous large experimental HTML interface has been removed from the temporary graphical entry point. The graphical page now keeps only the official visual identity, banner, logo and **Play** button.   
- **Play** no longer opens a pseudo-playable web UI. It clearly displays **Graphical interface in production** and then offers **Stop and switch to Terminal mode**.   
- The graphical placeholder dropped from roughly 6,300 lines of HTML/JavaScript to a lightweight page of roughly 140 lines, without combat, inventory, quest panels or snapshot polling while the real interface is being rebuilt.   
- A local `/gui/switch-terminal` endpoint writes a handoff request, clears legacy GUI inputs and cleanly shuts down the small local server after replying to the browser.   
- Linux and Windows launchers watch this request, stop the placeholder and launch the real Terminal version. The C++ game engine is no longer started hidden in the background merely to display the graphical welcome screen.   
- Terminal remains the default mode. The server temporarily keeps its older debug endpoints for development tools, but the public placeholder no longer uses them.   
- README files and the graphical-interface guide now distinguish the minimal current state from the future target: a real desktop application rather than a second large web interface maintained in parallel.   
- Regression guards protect the GUI-to-Terminal handoff and prevent the hidden engine from returning behind the placeholder. The endpoint was also exercised with a real marker write and server shutdown.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.23 — Context categories for service challenges   

- Intellectual checks inside service contracts now show an explicit **service category** tied to the task actually being performed: transport and logistics, appraisal and negotiation, records and administration, commercial arithmetic, French and writing, equipment and morphology, inventory and logistics, guild procedure, and similar contexts.   
- Internal families already used by merchant paperwork are now visible to the player rather than remaining hidden selection metadata. Other challenges infer their category from their title and question.   
- The challenge screen also states that the question belongs to the concrete step of the active contract, reducing the feeling of an unrelated random quiz.   
- Generic service challenges now receive a stable title-derived ID and use `serviceChallengeHistory`. A question already seen in that contract is avoided while unseen questions remain in the relevant pool.   
- Once every appropriate challenge has been seen, the local history can restart cleanly instead of blocking progress. The existing merchant-specific selection keeps its specialized families and unusual/confidential document handling.   
- `scripts/test_project.sh` protects both the player-facing service category and the `serviceChallengeCategory` logic.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.22 — RP pricing, consistent money display and anti-inflation guards   

- Audited early-game baskets after the V3.50.19/V3.50.20 normalization. Core prices deliberately stay on the current PF scale: the Training Bow remains around 85 PF, the Hunting Bow around 205 PF, Survival Rations around 15–17 PF and minor potions around 12–13 PF. No second global division was applied.   
- Beginner quests now have tighter regression guards: level-1 F contracts stay below 20 PF, small F services below 5 PF, and E contracts offered at level 2 below 30 PF. This prevents trivial local jobs from drifting back into literal PO/PP payouts.   
- Weapon shops, armor shops and blacksmiths now use **craftsman quotes**. After race, class, city, crisis and promotion modifiers, the actually charged price may be rounded to a clean coin denomination when the difference stays within roughly 12%. For example, 205 PF can become exactly 2 PO.   
- Other counters keep an **exact quote** and make change when necessary. Shop screens state which pricing style is in use so players can tell whether a merchant quotes a round sum or counts exact change.   
- Merchant rounding changes the amount actually charged rather than merely prettifying the label. Regression tests also ensure the rounded buy price cannot create buy/resell arbitrage under the main race/class modifiers.   
- Removed ambiguous legacy `X coins` displays from player-facing economy values. Item sheets, weapons, armor, consumables, materials, material journal entries, inventory inspection, character exchanges, monetary loot, PvP valuation, city vault prices and rune transfer costs now use `Money::formatEconomyUnits(...)` when the underlying value is an economy-unit amount.   
- Genuine lore references to generic physical pieces remain untouched. Cheats that explicitly grant historical gold coins also remain explicitly PO-based.   
- Added regression caps for representative starter goods: Training Bow <= 100 PF, Rusty Sword <= 80 PF, Worn Leather Armor <= 100 PF, Minor Healing Potion <= 20 PF and Survival Ration <= 20 PF.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.21 — Coherent quests, challenge variety and exploration supplies   

- Guild contracts now account for the **actual minimum level of explicitly named locations**, in addition to quest rank and template level. F/E offers can therefore no longer send a beginner directly into a biome intended for much later progression. City-side service requests remain exempt when they merely handle an item originating from a dangerous area.   
- Suggested quest locations are more precise: an explicitly named exploration biome wins over generic inference, while route, relay and delivery wording is no longer accidentally mapped to ruins.   
- A **search/exploration quest that names a precise biome** can no longer progress in another biome merely because generic words such as plants, tracks or materials happen to match. Hunt quests intentionally remain target-family based, so defeating the correct enemy type elsewhere can still count when that makes sense.   
- Exploration micro-challenges now carry a **context category** such as orientation, observation, survival, cartography, weather, first aid, logistics or tracking. Twelve new generic challenges were added, and selection avoids recently seen categories as well as duplicate questions.   
- Recent challenge history grows from 4 to **10 questions**, while existing cooldowns remain active. Biome-specific challenges are tagged as local-terrain questions so the screen explains why the question belongs to the current activity.   
- Exploration endurance is now checked **before each new field run**. The intensity screen shows remaining autonomous segments and Survival Ration count; when the planned run would exceed the reserve, the required ration is consumed before departure or the run is refused without advancing world time.   
- Continue/return summaries clearly show remaining endurance and rations. An expedition with no endurance and no ration must return instead of silently overshooting the limit and only discovering it afterward.   
- Added regression coverage that generates guild boards across levels 1–12 to reject over-level locations, checks the longer anti-repeat history, and protects the new quest/exploration coherence guards.   
- No save-schema change (**26**) and no mandatory checkpoint change (**V3.50.20**).   

---   

## V3.50.20 — Physical wallet and persistent coin stacks   

- The wallet no longer auto-normalizes coins into larger denominations. A stack of **51 PO remains 51 PO** instead of silently becoming 5 PP + 1 PO.   
- `Inventory` now preserves five independent physical stacks: PC, PF, PE, PO and PP. Historical economy-unit income adds PF, explicit copper income adds PC, and legacy explicit gold income adds PO.   
- Payments spend available coins without rewriting unrelated stacks. When a larger coin must be broken, ordinary lower-denomination change is returned while preserving the exact total value.   
- Exact total-PC value remains available for balance checks and compatibility; only the physical wallet representation becomes more faithful to the world.   
- Save schema advances **25 -> 26**. New saves persist each denomination stack. Older saves, which only knew a total value, retain that exact value and receive an initial denomination breakdown during migration.   
- The mandatory checkpoint advances to **V3.50.20** to protect the new persisted wallet representation before adapting older saves.   
- Economy regression coverage now explicitly checks a 51-PO stack, change-making from an oversized coin, the 50-PF starter wallet, and exact five-stack save/load persistence.   

---   

## V3.50.19 — Economy normalization and save-safe currency migration   

- Rebased the historical gameplay economy around the intended denomination scale: **1 authored economy unit = 1 PF = 10 PC**. Existing shop prices, quest rewards, combat rewards and service values that historically used small integer “gold” values now keep their relative balance without being interpreted as full PO.   
- Normal starting money remains `50` authored units but now means **50 PF = 5 PE = 500 PC**, instead of 50 PO.   
- Shop transactions, quest payouts, combat rewards, exploration rewards, PvP/boss money transfers, training, reputation repair and other historical `earnGold/spendGold` paths now use economy units. Explicit micro-prices already authored in copper (inns, meals, travel taxes, infirmary, delegated missions and similar systems) stay exact-PC values.   
- Added explicit `Money`/`Inventory` APIs for economy units while preserving legacy PO helpers only for backward compatibility. Player-facing wallet totals no longer display confusing decimal PO equivalents: exact PC totals and denomination breakdowns are used instead.   
- Save schema advances **24 -> 25**. Saves from schema <25 receive a one-time wallet normalization so wealth created by the old PO interpretation is converted to the new PF-scale economy. The migration cannot be applied twice once the save is rewritten as schema 25.   
- Because wallet normalization changes persisted monetary value, the mandatory important-save checkpoint advances to **V3.50.19**, creating the pre-update backup/transition protection before migration.   
- Exact-copper preservation was fixed in combat recap, PvP snapshots, cheat class switching, death penalties and exploration summaries so sub-PO money is never silently rounded away.   
- Added economy regression coverage for PF/PC conversion, the normal starter wallet, shop/quest/combat payout paths, legacy-save migration and guards preventing gameplay code from reintroducing `earnGold/spendGold`.   
- Continued RP-facing wording cleanup around money: generic rewards use “argent” where appropriate instead of pretending every payment is literal gold; true coin/lore references keep their denomination names.   

---   


## V3.50.18 — RP text cleanup, class-skill pacing and elite density   

- Cleaned player-facing text that read like patch notes or development commentary: `Future IG`, future asset mentions, development priorities, meta-lore wording, announced future functionality, and unnecessary comparisons with previous behavior were removed or rewritten in-world. Real progression feedback (unlocks, quest progress, actual state changes) remains visible. Necessary save-migration messages also remain explicit.   
- The current story boundary after the Chapter 3 introduction is presented as **story continuation unavailable** instead of exposing internal refactor notes, disabled source scenes or development priorities. Free-world play remains available.   
- Class-skill progression is spread out: most first class techniques now start around **level 5**, summoning at level 6, and later techniques are distributed across wider milestones. The arcane branch now reaches levels 14–17 for advanced techniques. Spells genuinely learned from grimoires keep their specific unlock bypasses.   
- `PlayerSkills` class-identity unlocks move from level 4 to level 5, and AI class-skill requirements are aligned so ordinary enemies do not systematically access their class techniques before the player.   
- Ambient elite density is capped at selection time: no random elites at levels 1–2, roughly 5% at levels 3–4, then a gradual increase with level. Very high-level regions made entirely of elites can still produce them, and explicit scripted elites/minibosses are unchanged.   
- Normal waves can no longer apply an evolved variant on top of a monster that is already elite, preventing accidental catalogue-elite + evolution stacking.   
- Added a statistical early-game elite-density regression test plus invariants for class-skill milestones and the main RP-text cleanup rules.   
- No save-schema change (**24**) and no mandatory checkpoint change (**V3.50.12**). Economy and full low-level quest filtering remain separate passes to avoid mixing a risky currency rebalance into this gameplay cleanup.   

---   

## V3.50.17 — Installer stabilization, C++23 and tooling clarity   

- Migrated the project toolchain to **C++23**. The Makefile, tests and Windows/Linux builds now share a detector that uses `-std=c++23` when available and falls back to the historical `-std=c++2b` alias on older GCC/MinGW toolchains that expose C++23 under that spelling. Save schema stays at **24** and the mandatory checkpoint remains **V3.50.12**.   
- Important Windows fix: both the installer and launcher could still repair `Lancer-Dinotofu.cmd` in `Auto` mode, which could re-enable the GUI chooser. The main launcher is now always rebuilt in **Terminal** mode; `Auto`/`Gui` remain opt-in only.   
- Fixed the Windows installer default `AssetPattern`: its fallback incorrectly targeted `Dinotofu-Windows-v*.7z` even though the Windows release payload is ZIP. It now targets `Dinotofu-Windows-v*.zip`.   
- Clearer GitHub Releases: player downloads now use the highly visible **`INSTALLER-DINOTOFU-...`** prefix. Large updater files remain necessary but now end in **`-TECHNICAL-PAYLOAD`** so players do not mistake them for the installer. Their historical `Dinotofu-<OS>-v*` prefix is intentionally preserved so legacy wildcard-based installers can still find them.   
- Player installer bundles still contain exactly **one `INSTALLER-DINOTOFU` file plus `Documentation/`**, with `.txt` files only inside Documentation. The full game is not bundled in the player installer download.   
- `scripts/bump_version.py` still asks before creating the final Git commit, but **Yes is now the interactive default**. Non-interactive use remains safe and never commits without `--commit`; development/ChatGPT passes can explicitly use `--no-commit`.   
- Added `tools/windows/README_TOOLS.txt` and `tools/linux/README_TOOLS.txt` to explain the clickable wrappers (`Installer/Lancer`) versus the internal engines (`DinotofuInstaller/DinotofuLauncher`). Legacy Terminal aliases remain only for compatibility and are not separate game versions.   
- The large GUI rewrite, economy pass, progressive class-skill unlocks, elite density and full low-level quest filtering remain intentionally separate from this technical pass.   

---   

## V3.50.16 — Save, quest and distribution stabilization   

- Character saves now persist `realPlayTimeSeconds` and `lastSavedAt`, while remaining compatible with older saves. Character selection now displays creation date, real play time and last-save time. Save schema moves to **24** while the mandatory checkpoint stays at **V3.50.12**, because this is an additive migration that does not require a destructive transition ritual.   
- The save round-trip test now explicitly verifies **level**, experience, the **exact copper total**, and real play time after save/reload. This directly covers the reported regression where level or money could appear to fall back after loading.   
- Guild board: accepting a quest now truly leaves its slot empty until the **next in-game day**. `ensureGuildBoardReady()` accounts for deferred replacements instead of immediately filling the gap on the same day. A regression test covers this behavior.   
- Inn: added a free **stable-floor** rest option. It advances one day but only restores health up to 25% of maximum HP so paid beds remain useful.   
- Windows/Linux launchers now use **Terminal as the default mode**. The previous graphical interface remains explicitly available for development but is marked as being rebuilt; the large GUI rewrite is deliberately deferred to a dedicated pass.   
- Distribution restores **four release assets**. Players get `Installer-Dinotofu-Windows-v*.zip` or `Installer-Dinotofu-Linux-v*.7z`, whose root contains only the installer plus `.txt` files under `Documentation/`. Full `Dinotofu-Windows-v*.zip` and `Dinotofu-Linux-v*.7z` archives remain technical payloads so existing launchers/updaters are not broken.   
- `scripts/bump_version.py` no longer commits implicitly in interactive mode: after all file edits it explicitly asks whether to create a Git commit, with **No as the default**. Non-interactive mode never commits unless `--commit` is supplied.   
- Release documentation and validation are aligned with Terminal-by-default and the 2 installer + 2 payload model.   
- This pass intentionally does not yet rebalance starting money, class-skill unlock pacing, elite frequency, or the full low-level quest-distance filter; those balancing topics remain separate from technical stabilization.   

---   

## V3.50.15 — Update notes   

- Fixed installation scripts by correctly filtering running programs (was killing the script itself).   
- Staticly linked some library and downgraded the OS version to ubuntu-22.04 for some library compatibility.   

---   

## V3.50.14 — Update notes   

- Fixed Linux launcher self-termination bug in `tools/linux/DinotofuLauncher.sh`.   
- In `stop_dinotofu_background_processes`, replaced broad `pkill -f "${target_dir}/(output/)?Dinotofu"` with precise PID checks (`pgrep` with process name anchoring and self/parent/script exclusions) so launching the graphical interface does not terminate the launcher script when installed in a path containing `Dinotofu`.   
- Verified full test suite and packaging validation.   
- No change to the save schema (`saveVersion = 23`) or mandatory save checkpoint (`V3.50.12`).   

---   

## V3.50.13 — Update notes   

- Bug fixes and stabilization for Windows and Linux installers and launchers.   
- Windows: unconditional synchronization of launcher scripts (`DinotofuLauncher.ps1`, `Lancer-Dinotofu.cmd`, `Lancer-Dinotofu-Terminal.cmd`), GUI tools, and assets during installation to overwrite stale files extracted from downloaded release archives.   
- Windows: robust process termination in `Stop-DinotofuBackgroundProcesses` (PID files, WMI/CIM, .NET process filtering, and `taskkill` safety net) to prevent file locking during `Copy-Item`. Added retries with backoff during installation and update extraction.   
- Windows: resolved relative path issue that created a stray `C\` directory in the repository, adding automatic cleanup of orphaned shortcuts/folders and enforcing rooted paths (`IsPathRooted`).   
- Windows & Linux: interactive launcher menu at startup (choice between Graphical Browser Interface and Classic Terminal Mode), with interactive console session management to cleanly stop background processes via Enter or Ctrl+C.   
- No change to the save schema (`saveVersion = 23`) or mandatory save checkpoint (`V3.50.12`).   

---   

## V3.50.12 — Update notes   

- Version synchronization placeholder. Replace with detailed release notes before publishing.   

---   

## V3.50.11 — Update notes   

- Reworked `scripts/bump_version.py` so the game version, internal save schema (`saveVersion`) and mandatory important-save checkpoint are three independent decisions. Interactive runs now explicitly ask whether the save schema should change and whether the new game version should become a new backup + transition checkpoint. `--save-schema`, `--checkpoint` and `--non-interactive` support automated usage.   
- Centralized the save schema in `include/save/SaveSchemaVersion.hpp`. `SaveManager` now writes `SaveSchemaVersion::Current` instead of duplicating a numeric literal, and round-trip tests use the same constant. V3.50.11 intentionally keeps schema **23** and the important checkpoint at **V3.50.09**.   
- Hardened GitHub publishing: the workflow no longer pushes a Git tag before compilation. Windows/Linux jobs build the commit that triggered the workflow; the tag/release is created only after successful builds through `gh release create --target $GITHUB_SHA`. This avoids empty tags and reduces failures caused by tag-protection rules.   
- Automatic publishing still triggers on pushes to `main`/`master`, with `workflow_dispatch` kept for manual repair. Added `scripts/trigger_release.sh` and `make release-trigger` to rebuild/repair a release through GitHub CLI with `force_release=true`. Documentation now makes it explicit that pushing without changing the game version intentionally produces no new release.   
- Added clean player-facing installer bundles: `Installer-Dinotofu-<Platform>-vX.YY.ZZ.7z` contains exactly one installer file plus `Documentation/`, and that folder only contains `.txt` files. Windows uses a single CMD bootstrap that downloads the PowerShell installer from the release tag (falling back to `main`) and then fetches the technical payload; Linux ships the standalone shell installer.   
- Historical `Dinotofu-Windows-vX.YY.ZZ.7z` and `Dinotofu-Linux-vX.YY.ZZ.7z` archives remain published as updater payloads so old launchers are not broken. Their player/developer text files are now grouped under `Documentation/` instead of cluttering the payload root.   
- GitHub Releases now consider all four assets (two installer bundles + two updater payloads) required for completeness and can rebuild them through `force_release=true`. Validation protects the clean installer layout, release helpers and the independence between game version/save schema/checkpoint.   
- No main-story progression and no new serialized player field in this tooling pass; the story remains locked immediately after the Chapter 3 introduction.   

---   

## V3.50.10 — Merge stabilization, legacy updater recovery and forced old-save migration   
- Audited the user-merged V3.50.09 against the previously delivered V3.50.09 byte-for-byte. Gameplay/content modules were not overwritten: the merge added 3 files and modified 14 source/tooling files, almost entirely around CI, Makefile, installers, launchers, packaging and `main.cpp`. The new `.gitattributes`, CI workflow, multilingual desktop-directory handling, default repository wiring and `data/assets` runtime fallback were retained where safe.   
- Fixed a legacy updater regression caused by historical configs containing the literal placeholder `TON_COMPTE/TON_REPO`. New Linux and Windows launchers/installers now recognize that placeholder (and malformed repo values) as unconfigured and automatically fall back to `SIMON-Louis-2326101aa/ProjetDinotofu`, allowing old installations that receive the new bootstrap scripts to find GitHub Releases again.   
- Linux update comparison now uses semantic version ordering instead of “any different version”, preventing an accidental downgrade when a local development build is newer than the published release. Windows already used semantic comparison and keeps that behavior.   
- Restored GitHub release self-repair: if a version tag exists but the GitHub Release or required Windows/Linux `.7z` asset is missing, the release workflow rebuilds/publishes it instead of declaring that nothing needs to be done. Build jobs again checkout the release tag so generated artifacts exactly match the tagged source.   
- Repaired merge bugs in packaging. `package_linux_release.sh` previously referenced an undefined `REPO_NAME` under `set -u`; both Linux and Windows packagers now resolve the repository through the same `detect_repo_name()` fallback. The historic root `assets/` release layout is deliberately preserved for this stabilization release so old `assets/saves` installations do not silently move player data to another path. Runtime/installers still understand the future `data/assets` layout, but no save-location migration is attempted implicitly. GUI tools are included again in release packages and local saves are explicitly removed from staging.   
- Hardened the mandatory V3.50.09 save checkpoint. `VersionInfo::requiresImportantSaveUpdate()` now treats unknown/unversioned metadata as legacy instead of returning false. Pre-V3.00 saves and saves missing `lastAdaptedVersion` can no longer bypass the checkpoint via the old recreation-warning branch: the dedicated non-overwriting checkpoint backup + heavy transition ritual takes priority before normal play.   
- Older save metadata now falls back from `lastAdaptedVersion` → `createdForVersion` → legacy `gameVersion`. This lets saves created before the newer metadata fields still identify their real origin when possible. Added `LegacySaveMigrationTest.cpp` and expanded `ImportantSaveCheckpointTest.cpp` to cover 1.x/2.x saves, unknown metadata, fallback from `gameVersion`, checkpoint backup creation, heavy adaptation and final current-version marking. Save schema remains **23**.   
- Fixed the merged `bump_version.py`: version bumps now update only current-version fields instead of blindly replacing historical references. The important checkpoint therefore remains **V3.50.09** when the application moves to V3.50.10+, changelog history is preserved, both bilingual changelogs stay synchronized, and manifest free-text notes are no longer rewritten by accident.   
- Kept the safe merge improvements: automatic parallel Makefile targets/check/help, `.gitattributes`, cross-platform CI, improved desktop-folder discovery, explicit GitHub repository defaults, release-asset upload failures, and support in `main.cpp`/launchers for both legacy `assets/` and future `data/assets` layouts.   
- Main-story progression is unchanged and remains locked immediately after the Chapter 3 introduction. This release is a merge/update/migration stabilization pass, not a story-content expansion.   

## V3.50.09 — Mandatory save checkpoint and major exploration split   
- **V3.50.09 is a new important save checkpoint.** `VersionInfo::importantSaveUpdateVersion()` now points to 3.50.09. A character whose `lastAdaptedVersion` predates this checkpoint cannot silently resume normal play: the character menu explicitly marks it as requiring an important save update and offers the known heavy transition ritual.   
- Before the transition is allowed, `SaveManager::createImportantUpdateBackup()` creates a dedicated checkpoint under `assets/saves/update_backups/V3.50.09/<account>/`, including the character save, account save and a small manifest. The pre-update character/account copies use fixed `__before_V3.50.09` names and are deliberately **never overwritten** by later attempts or ordinary `.bak` rotation. If the checkpoint backup cannot be created, Dinotofu refuses to adapt/recreate the save instead of risking a destructive migration.   
- Completing the heavy transition marks the character as adapted to the current version, so the ritual is requested once for this checkpoint rather than at every launch. Existing pre-V3.00.00 legacy compatibility/recreation rules remain separate. Save schema remains **23** because this checkpoint protects/migrates existing state without adding a new serialized field.   
- Added `ImportantSaveCheckpointTest.cpp`, covering checkpoint detection, dedicated character/account backup creation, manifest policy, and the non-overwrite guarantee after the live save changes. `make test` also checks that the mandatory checkpoint wording/wiring cannot disappear silently.   
- Continued the no-`.inc` modularization with the largest `QuestMenu` split so far. The exploration engine — biome setup, distance/intensity, travel/night/temperature risk, quest-search clues, combat/event flow, micro-challenges, chests, mini-bosses, dangerous sites, discoveries, rewards and the exploration menus — now lives in the real `QuestExplorationMenu.cpp` module.   
- `QuestMenu.cpp` falls from roughly **12,334 to ~6,176 lines**. `QuestExplorationMenu.cpp` contains ~6,347 lines and is connected through `QuestExplorationSupport.hpp` plus a narrow `QuestMenuInternalSupport.hpp` bridge for the few helpers still shared with guild/client flows. Shared behavior was exposed through interfaces instead of copied, and no `.inc` fragment was introduced.   
- Structural tests now require the exploration module/support files and enforce a `< 7,000` line guard on `QuestMenu.cpp`, preventing the extracted engine from quietly drifting back into the monolith. Existing biome living-content, ambient-event, multilingual trace and non-combat-interaction checks now target the exploration module where those responsibilities actually live.   
- Full C++17 build/link and the complete `make test` suite pass with `-Wall -Wextra`. Main-story progression remains locked immediately after the Chapter 3 introduction; this pass changes save safety and architecture only, not story reach.   

## V3.50.08 — Story/runtime separation, tactical support modules and richer biome choices   
- Continued the no-`.inc` structural pass with real `.cpp/.hpp` boundaries. The already-existing story runtime moved out of `Game.cpp` into `GameStory.cpp`; no new main-story scene was added and the Chapter 3 development lock remains unchanged. `Game.cpp` falls from roughly 6,225 to **3,250 lines**, while `GameStory.cpp` contains the isolated existing story runtime.   
- `QuestMenu.cpp` falls again from roughly 14,100 to **12,334 lines**. Main-story synchronization/browsing now lives in `QuestStoryMenu.cpp`, story-only helpers in `QuestStorySupport.cpp/.hpp`, and shared quest-kind/progress presentation logic in `QuestPresentationSupport.cpp/.hpp`. The split exposed hidden cross-file dependencies, which were centralized instead of duplicated.   
- `PlayerWaveTacticalActionMenu.cpp` falls from roughly 4,410 to **2,901 lines**. Seventy-four helpers for affinity, targeting, tactical mastery, equipment synergy, formations, coatings, lanterns, traps and body/status selection now live in `PlayerWaveTacticalSupport.cpp/.hpp`; the action module keeps execution responsibilities instead of low-level shared helpers.   
- Added **10 further biome-specific non-combat interactions**: overturned wagon in the Wild Plain, extinguished lantern in the Lantern Hedgerow, disputed well markers in the Red Clay Desert, marked window in the Abandoned Quarter, cracked support in the Whistling Mine, magnetized tool in the Iron Firefly Orchard, fresh black bark in the Corruption Woods, offering circle in the Mana Fairy Grove, nine-version ribbons at the Kitsune shrine and a drifting mooring in the Floating Isles. Each provides local choices, bounded exploration consequences, history traces and same-day anti-farming.   
- Prepared enemy signatures gain three additional mechanical families: **elemental breath**, **aerial dive** and **prepared venom**. Each has its own visible telegraph, interrupt threshold and resolution profile. Elemental breath weakens and opens the target, aerial dive trades commitment for heavier damage and forced repositioning, and prepared venom emphasizes poison over immediate damage.   
- Expanded targeted C++ coverage for the new biome interactions and prepared-action families. Structural tests now protect the new `GameStory`, quest-story/presentation and tactical-support modules and enforce tighter size ceilings on the remaining orchestrators.   
- Full C++17 build/link and the complete `make test` suite pass with `-Wall -Wextra` before the release bump. Save schema remains **23** because this pass introduces no new persistent save fields.   
- Main story progression remains locked immediately after the Chapter 3 introduction. Current priority remains living-world depth, meaningful content, combat variety, stability and continued decomposition of oversized files.   

## V3.50.07 — Modularized player turns, biome interactions and richer enemy tactics   
- Major combat-structure pass: `PlayerWaveCombatTurn.cpp` drops from roughly 4,769 to **394 lines**. Thousands of player tactical-action lines now live in the real `PlayerWaveTacticalActionMenu.cpp/.hpp` module (~4,410 lines), while the main turn file returns to being a readable orchestrator. No `.inc` fragments were introduced, and project invariants now enforce this split.   
- Added and wired `BiomeNonCombatInteractionSystem` into exploration. Several biomes can now surface small contextual local choices — conflicting road markers, a forgotten snare, a damaged marsh footbridge, a tended grave, a drowned register page, rope near a drake nest, a disputed debt or an oath bell — with exploration consequences, history traces and occasional quest progress. The same interaction cannot be farmed by reopening the menu on the same day.   
- Prepared enemy signatures now have observable mechanical families: heavy charge, heavy shot, massive restraint, pack cry, ritual and committed strike. Each family has its own damage-interruption threshold, effect and telegraph; shock, frost and entanglement remain valid hard interrupts.   
- Compatible protectors/coordinated allies may now sacrifice their own turn to **cover an enemy preparing a signature**. Cover grants temporary defense/ward rather than immunity, consumes the protector's turn and creates an observable combat reaction.   
- Added **18 new creatures/variants** tied to biomes and the new mechanics, including relay crossbowmen, ancient-root weavers, long-breath mire shamans, drowned-seal scribes, grey diving drakes, great-oath bell ringers and rare contradictory-rumor themed enemies.   
- Added `NpcRelationshipSystem`: named NPC pairs can have explicit reciprocal relationships such as professional trust, local coordination, scholarly links, partnership, caution or useful distrust. Relationships alter the priority of relaying an already-known fact without creating information or bypassing source requirements.   
- More named NPCs now use explicit living profiles instead of generic inference: Mira, Orren, Lysa, Bram, Soryn, Eda, Nell, Meron, Prunigil, Bob and Maurice. Stewardship, healer, logistics, messenger and travelling-merchant professions also have dedicated relay rules/channels instead of falling back to generic word of mouth.   
- Tests now cover biome interactions and same-day anti-farming, NPC relationships and named profiles, the 18 monster additions, prepared-action variants and group protection of a preparer. Full C++17 build/link and `make test` pass with `-Wall -Wextra`.   
- Save schema remains **23**: these systems reuse existing persistent NPC facts and canonical history, while prepared combat actions remain transient combat state.   
- Main story progression remains locked immediately after the Chapter 3 introduction. Development continues to prioritize a living world, variety, consequences, systems and progressive reduction of oversized files.   

## V3.50.06 — Inter-city knowledge, interruptible threats and deeper modularization   
- Continued the no-`.inc` structural pass with real `.cpp/.hpp` boundaries. Character creation, difficulty/death-rule selection, race/class/appearance choices and local party setup moved from `Game.cpp` into `GameSetup.cpp`; `Game.cpp` falls from roughly 7,642 to 6,225 lines.   
- `QuestMenu.cpp` falls again from roughly 14,989 to 14,017 lines. Location navigation and notable-NPC browsing now live in `QuestLocationNpcMenu.cpp`, while shared client status, ready-to-turn-in rules, material-delivery checks and recommended-client navigation live in `QuestClientNavigationSupport.cpp`. Regression tests now protect the new file-size boundaries.   
- NPC knowledge can now travel between real cities without teleportation. Inter-city propagation uses `CityTravelRules` distances and a network-specific carrier/delay such as guard messenger, guild courier, merchant caravan, pilgrim, relay traveler, archive copy or artisan convoy. Fresh information therefore cannot appear in another city before enough world days have elapsed.   
- Inter-city information preserves source identity and contradictory claim variants while losing confidence according to distance/network. Ordinary rumors cannot become hard proof merely by traveling; stronger scholar/guild reports can preserve limited evidence when their original source actually carried it.   
- Added `MonsterPreparedActionSystem`: selected heavy enemy signatures can be visibly prepared for one enemy turn instead of resolving as a hidden instant buff. The preparation can be interrupted by shock, frost, entanglement or enough immediate damage. Successful interruption consumes the enemy's prepared move and briefly exposes it.   
- A prepared signature that resolves deals its committed hit and causes an abstract forced reposition/opening without inventing a grid system the game does not have. The UI only announces preparation the player has actually observed; it still never displays hidden future AI intentions.   
- Added lightweight C++ coverage for inter-city knowledge delays/carriers and prepared-action resolution/interruption, and wired those tests into `make test`. Existing architecture checks were updated so moving responsibilities out of monolithic files is treated as the expected design rather than a failure.   
- Full C++17 build/link passes with `-Wall -Wextra`, and the full `make test` suite passes. Save schema remains 23 because the new inter-city propagation reuses already-persisted NPC fact metadata and prepared combat actions are transient combat state.   
- Main story progression remains locked immediately after the Chapter 3 introduction. Development priority stays on world life, systems, content, stability and continued decomposition of oversized files.   

## V3.50.05 — Information networks, group tactics and further modularization   
- Continued structural cleanup without `.inc` fragments: the city/travel/inn/municipal-vault/route-discovery block moved out of `QuestMenu.cpp` into `QuestWorldMenuSupport.cpp`, while deadline expiration is centralized in `QuestDeadlineSupport.cpp`. `QuestMenu.cpp` falls from roughly 16,457 to 14,989 lines.   
- `ShopMenu.cpp` falls from roughly 6,123 to 4,075 lines. City services, subscriptions, local events, lodging, transport and municipal work now live in `ShopCityServiceMenu.cpp`, reusing `ShopServiceSupport` instead of duplicating payment/display helpers.   
- Living NPC profiles now carry a structured information network: guard, trade, guild, scholar, contacts, inns, artisans, temple or neighborhood. Relay priority considers profession, source/recipient network, evidence strength and the actual age of the fact.   
- Networks now have different relay delays: guard/guild/contact information can move immediately, trade/scholar/inn networks usually need one day, and artisan/temple channels are slower. Fresh rumors therefore remain local instead of teleporting instantly.   
- Strong local proof can weaken weaker contradictory variants known by THAT NPC without deleting the older account or globally correcting everyone. Contradictions remain inspectable with their original source and confidence.   
- Enemy group profiles gain `coversRetreat` and `controlsTerrain`. Some ranged/protector units can briefly cover an ally's escape with a real cost to the covering unit rather than a free defensive reaction.   
- Web, root, thorn, spore, mud, ice and trap-oriented creatures can spend a formation turn controlling local terrain, briefly weakening the player and creating an opening instead of making a direct attack.   
- `EnemyGroupBehaviorTest` covers the new roles, while `NpcInformationPropagationSystemTest` verifies network delays, contradictory accounts, no inter-city teleportation and local proof-based correction.   
- Full C++17 build/link passes with `-Wall -Wextra`, and the complete `make test` suite passes after updating expectations for real relay delays.   
- Main story progression still stops after the Chapter 3 introduction. Current priority remains living systems, free content, consistency, stability and splitting oversized files.   


## V3.50.04 — Major modularization, conflicting rumors and living world   
- Major structural pass with no `.inc` fragments: oversized files are split only into real `.cpp/.hpp` modules with explicit responsibilities and independently compilable boundaries.   
- `Player.cpp` drops from roughly 8,394 to 4,507 lines. Skills, cheats, challenge tracking, municipal storage, equipment lifecycle and attack resolution now live in dedicated player modules.   
- `MonsterPveMode.cpp` drops from roughly 4,917 to 3,608 lines. Cooperative PvE now lives in `MonsterPveCoopMode.cpp`, while wave bestiary/journal/dialogue support lives in `PveWaveNarrativeSupport.cpp`.   
- `ShopMenu.cpp` drops from roughly 8,800 to 6,123 lines. Church and enchanter services moved into dedicated modules with a shared `ShopServiceSupport.cpp`; oath tests now follow the new architecture rather than the old monolith.   
- `QuestMenu.cpp` drops from roughly 21,500 to 16,457 lines. Contractor/delegated mission/guild sanction logic moved to `QuestContractorMenu.cpp`, while team/recruit/clan/infirmary/Torvald/order/group mission management moved to `QuestTeamMenu.cpp`. Shared hooks are exposed through normal headers.   
- NPC memory can now keep several contradictory variants of the same subject instead of silently overwriting the previous one. Each version keeps its source, confidence, evidence level and transmission channel such as guard post, market, inn, guild, library, temple or word of mouth. The game may therefore mark a claim as disputed without magically knowing which version is true.   
- Save schema moves to 23 to persist NPC knowledge variants/channels. Older memories remain compatible and receive a safe default variant without inventing new knowledge.   
- Living biome content now covers many early and mid-game areas as well, adding stronger visual identity, hazards, resources, neutral life, social traces and unusual details instead of reserving the system for advanced zones.   
- Structured enemy-group behavior gains leader protection for compatible formations, extending the existing surrender, panic and retreat logic.   
- Post-extraction cleanup removed obsolete helpers and updated project invariants to target the new modules. Full compilation/linking was validated with `-Wall -Wextra`.   
- Main-story progression still does not advance beyond the Chapter 3 introduction; current priority remains world life, systems, free content, stability and modularization.   

## V3.50.03 — Local rumors, biome micro-events and item memory   
- Added sourced local NPC-to-NPC information propagation. An NPC only relays facts they actually know, within the same city, with confidence loss at every relay and preservation of the previous source identity. Professions prioritize plausible subjects: guards favor threats/attacks/rivals, scholars favor writings/unusual creatures, guild contacts favor quests and danger, and so on.   
- Relays do not create omniscience: information does not teleport between cities and a relayed statement becomes an explicit `rumeur_locale`. Re-learning the exact same relay from the same source is blocked so reopening a menu cannot manufacture certainty.   
- NPCs capable of initiative can now occasionally start a conversation themselves. Their opening line is driven by profession, profile and memories they actually hold, keeping a clear distinction between what the NPC believes and invisible global world state.   
- Added biome ambient micro-events such as recent passage, shifted terrain, active wildlife, exposed resources and unusual details. They only describe currently observable conditions, never future outcomes, and can slightly alter that exploration. A biome/day event is journaled after use so repeatedly reopening the menu cannot farm its effect.   
- Expanded creature lifecycle flavor with family/archetype-specific death, escape and surrender descriptions. Constructs no longer “die” like humans, undead do not flee with the same body language as bandits, and surrender keeps its own narrative identity.   
- Equipment sale and buyback now fully use exact persistent instance identity. A sold then repurchased weapon/armor keeps its `persistentId`, records transfer and recovery memories, and can earn the emergent `la Revenante` / `la Revenue` nickname without renaming every copy of the item type.   
- Fixed a defect found while testing NPC propagation: retaining a pointer into the fact vector and then extending NPC memory could invalidate that pointer. Propagation now copies the selected fact before mutating memory, removing the crash risk.   
- Added lightweight C++ tests for local knowledge propagation, no cross-city teleportation, death/escape/surrender flavor and ambient biome events. Project invariants also check that the systems are actually wired into gameplay and that equipment sale/recovery memory hooks remain present.   
- Final portability validation now covers every shell script in the project; a remaining CRLF-formatted `tools/linux/DinotofuLauncher.sh` was normalized to LF to prevent a Linux shell syntax failure.   
- Main story progression still does not advance beyond the Chapter 3 introduction. Development remains focused on systems, consistency, world life, variety and free content.   

## V3.50.02 — NPC memory, group morale and local repair   
- Added persistent NPC factual memory. A named NPC can now remember a fact with source, subject, place, first/last reinforcement day, confidence, evidence level and number of times heard. NPC memory is saved in schema 22 and old saves safely start with no invented memories.   
- NPC knowledge remains deliberately non-omniscient. Player testimony, direct observation, proof, registers and rumors are distinguished, and repeated hearing from the exact same source cannot magically manufacture certainty.   
- Added memory aging: proofs and direct interactions remain reliable much longer, while unsupported testimony and rumors lose effective confidence over time. The original stored source is preserved instead of silently rewriting history.   
- Quest clients and shop contacts now expose fact-based memory reactions. Accepting or completing a client quest can become a direct remembered interaction, and the player can deliberately share a recent witnessed fact with a named NPC.   
- Fixed language `studyProgress` being serialized but discarded on load. Conversational languages can now advance toward fluent level through guided library practice; level 3 requires repeated practice and time rather than one instant purchase. Anomaly notation still cannot become normal fluent speech.   
- Extracted allied duo mastery into the real `DuoMasterySystem` module. Added role-based pair plans including Tank+Tank `Mur en mouvement`, Support+Support `Relais vital`, ranged crossfire and guarded assault, while preserving real two-turn consumption and coordination failure.   
- Added structured enemy group roles (leader, protector, coordinated member, surrender-capable, wounded-abandoning). Morale can now cause a true surrender state distinct from death and escape; surrendered enemies remain alive, grant only reduced experience and no death loot, and never become rivals merely because they yielded.   
- Added group shock after the actual death of a compatible leader. Some enemies panic, weaken or break formation, while cowardly groups may retreat without being promoted into rivals. Fixed combat iteration so removing a surrendering/fleeing enemy no longer skips the next shifted enemy's turn.   
- Added richer living-biome observations with visual identity, terrain hazard, visible resources, neutral life, social traces and unusual signs. Observations rotate with the current day without predicting future events. Advanced locations such as assassin rooftops, draconic nests, black lava flows, the mana-fey grove and oath glaciers now have their own environmental identity.   
- Expanded seven previously sparse common monster pools with additional humans, goblins, beasts and corrupted creatures whose language, morale, group behavior and flavor systems apply automatically.   
- Added real local-reputation rehabilitation. A municipal mediation office now offers proportional reparative fines and once-per-day community service that consumes world time, so negative reputation has meaningful recovery paths rather than becoming only a permanent surcharge.   
- Extended save round-trip tests to cover NPC facts, language study progress, a full rival lifecycle, duo mastery and exact per-instance equipment memory. Added dedicated tests for duo mastery, enemy surrender/group profiles and local-reputation repair.   
- Continued modularization with real `.cpp/.hpp` modules only; no `.inc` fragments were introduced. Main story progression remains capped immediately after the Chapter 3 introduction.   

## V3.50.01   
- Added persistent multilingual exploration traces for advanced biomes. Written clues now use the player's actual language knowledge and can remain unreadable, partially understood, or fully translated.   
- Added a structured living NPC profile foundation with profession, temperament, native language, conversation capability, local memory intent and fact-based reactions.   
- Wired multilingual traces into careful exploration observations and world history without granting omniscient knowledge.   
- Expanded save round-trip coverage to include a complete rival lifecycle (escape, wounds, visible mark, return) and persistent allied-duo mastery records.   
- Added lightweight C++ tests for living-world content and language traces.   
- Story progression remains capped after the chapter 3 introduction.   

## V3.50.00 — Languages, living encounters and exploration identity   

- Added a persistent language system with Common plus racial native languages at character start. Current characters always know Common as requested; race-specific native speech is added without making the spawn unplayable. Knowledge has unknown, notions, conversational and fluent/native levels.   

- The library now sells introductory and advanced language material for goblin, orcish, infernal, draconic, elven, dark-elven, celestial, fey, kitsune, dwarven, gnomish, halfling, vampiric and spirit speech. Anomaly notation can only be recognized in fragments rather than spoken normally.   

- Conscious monster/NPC-like encounter speakers can now introduce themselves or react before combat with a probability based on importance. Their speech uses their actual racial language; unknown languages stay foreign, partial knowledge gives fragments, and sufficient knowledge reveals meaning. Mindless creatures do not suddenly talk.   

- Guild contracts can occasionally contain a thematically appropriate foreign-language annex. The guild refuses to make the player sign an annex they cannot verify, so learning a language at the library has a concrete quest purpose. Main-story quests are not blocked by this system.   

- Language knowledge and foreign quest metadata are persisted in save files. Save schema moved to 21 while older saves default safely to Common plus the character's native racial language. Known languages are visible in the progression/statistics screen.   

- Added `MonsterFlavorCatalog` so named monsters, species and attacks gain stable appearance, posture, motion and impact texture instead of sharing one generic description. The flavor is deterministic for a given identity and still respects race, type, elite/evolved state and visible injury.   

- Expanded low- and mid-tier monster variety with new goblins, beasts, plants, spirits, kobolds, hobgoblins, fey, orcs and constructs.   

- Expanded the world map and city biome routes to expose many combat biomes that previously existed mostly in backend pools: troubled marsh, forgotten cemetery, drowned archives, abandoned fair, weeping-statue garden, kitsune sanctuary, pure-mana confluence, floating islands and others. Each received its own place text and exploration identity.   

- Extracted exploration biome flavor out of the oversized `QuestMenu.cpp` into a real `.cpp/.hpp` module, and extracted encounter dialogue/language behavior out of `MonsterPveMode.cpp`. No `.inc` fragments were reintroduced.   

- Added lightweight runtime tests for language mappings and thematic foreign-quest assignment, plus project invariants covering save schema, library courses, multilingual encounters and quest gating.   

- Story progression remains intentionally capped after the Chapter 3 introduction. This pass develops systems, free exploration and content rather than advancing the main plot.   

---   

## V3.49.93 — Living rivals, social consequences and persistent mastery   

- Enemy escape and rival creation are now separate outcomes. An ordinary escape records survival without promising a return, while a persistent rival requires a dedicated emergence roll based on real traces, elite/evolved status, witnessed memory and relevant oaths.   

- Even the strongest combination of rival conditions is capped at a 65% emergence chance. The Rival Oath increases the chance but never turns every fleeing enemy into a recurring mini-boss.   

- Enemy morale eligibility now comes from the centralized behavior profile instead of being re-guessed inside the combat turn. Mindless undead, anomalies, constructs, spectres and twisted oath entities do not suddenly gain human fear behavior.   

- Persistent rivals now save temperament, visible marks, last known outcome, emergence strength and notoriety. Wounded rivals keep scars, returning rivals become more recognizable, and defeated rivals remain permanently dead.   

- Rival returns now respect an unseen minimum delay and decreasing return chance. The memory screen only reports known facts and never displays the next return date or hidden intention.   

- Allied combination techniques now use persistent duo experience. Coordination can fail while still consuming both ally turns, the Bonds Oath helps without guaranteeing success, and successful duos unlock shared mastery milestones at 3, 7 and 12 executions.   

- Exact equipment instances now remember Bound Forge strikes and enemy signature impacts through their persistent IDs. Repeated repairs, impacts and linked strikes can produce an emergent reputation and nickname during inspection without renaming every copy of the same item.   

- Negative local reputation now produces visible social reactions, service surcharges, smaller price pressure in ordinary shops and refusal of expensive trusted sales when the player is locally undesirable. Positive and negative consequences use the same centralized reputation result.   

- Added a real lightweight C++ test for rival emergence and return-delay rules. Project checks also verify ordinary escapes, dual changelogs, duo mastery, equipment-instance memory and negative local-reputation effects.   

- Fixed the ignored `system()` result warning in the console clear path and strengthened clean-build validation.   

- Normalized every shell script to Unix line endings and added a regression check, so Linux test and release commands no longer fail on a stray carriage return.   

---   

## V3.49.92 — Persistent consolidation, contracts and living content   

- Rebuilt and consolidated the working branch on a compilable base while preserving persistent-memory additions and keeping the obsolete PATCHNOTE files removed.   

- Church oaths became persistent contracts outside the passive loadout: they consume no passive slot, cannot be toggled from the skill menu, and a broken oath cannot be restored for free.   

- The canonical journal now keeps local actions attached to their actual city, while important narrative categories remain protected during journal cleanup.   

- Local reputation uses the centralized `LocalReputationSystem` for both city and shop rules.   

- Rivals gained individual persistent identity, escape/return/wound history and permanent death state.   

- The statistics menu gained a World Memory / Rivals view that reports experienced facts without predicting future actions.   

- Weapons and armor gained per-instance persistent IDs and early equipment-memory events.   

- Added the first two-recruit combined techniques, consuming both ally turns and placing both personal techniques on cooldown.   

- Normal story progression is intentionally capped after the chapter 3 introduction. Later draft scenes remain in code for future rework but are not injected into a normal playthrough.   

- Added a lightweight `make test` target covering the main project invariants.   

---   

## V3.49.91 — Persistent memory, true rivals and equipment identity   

- Separated aggregate canonical statistics from individual historical events with unique ID, subject, location, day and resolution state.   

- Added persistent rival records, save migration, rival return handling, permanent rival death, world-memory inspection, per-instance equipment IDs and the first real ally combination action.   

- Removed temporary `.inc` fragments. Future modularization must use real `.cpp/.hpp` modules only.   

---   

## V3.49.90 — Technical and documentation consolidation   

- Moved detailed version history out of README files and replaced obsolete PATCHNOTE documents with the changelog.   

- Consolidated oath contracts, canonical journal localization, centralized local reputation and a lightweight invariant test target.   

---   

## V3.49.89 — Rivals and unstable fate   

- V3.49.89 adds the Rival and Unstable Fate church oaths. Escapes, panic reactions and signature skills can leave clearer rival traces, while unstable fate creates small oscillations only when an actual trace already exists: memory, broken oath, rival or marked item.   


## V3.30.00 — note — routes and equipment weight   

- Cities now have distances between each other, distances toward biomes, and access requirements based on level or defeated bosses.   

- Vaults remain independent per city: travel changes the active town but does not merge contents.   

- The textual exploration map prepares future pixel-art backgrounds per biome, with unknown places shown as grey or foggy.   

- Weapons and armor now have light, medium, or heavy weight classes. Bonuses and tradeoffs are applied to combat, damage, and escape, with intentionally moderate penalties.   

## V3.31.00 — note — city hubs and canonical journal   

- V3.31.00 turns the current city into a more structured hub: local buildings, contacts, lock states and future pixel-art hints now come from world rules instead of one fixed text list. City destinations expose structured GUI metadata for access, distance, travel time and future route costs. A saved canonical journal now records key world events such as places visited, routes taken and vault movements so later Top 3 screens can rely on engine data rather than parsed interface text.   

## V3.49.86 — Loadout-aware tactical mastery   

- V3.49.86 ties active mastery more directly to class/loadout coherence. A coherent weapon or armor can lightly support linked tactical actions, while a class malus can make the gesture less clean even with mastery. Class audit now shows survival, crit identity and expected equipment, and observation-style passives reduce surprise against enemy signature skills.   

## V3.49.36 — Creature aiming and more immersive combat text   

- V3.49.36 adds an aiming read to enemy profiles: small creatures such as rats, fairies, bats, insects and sneaky profiles are harder to frame, while brutes, constructs, dragons, rooted plants and guardians sometimes leave easier windows to hit. Observation and bestiary entries now expose this aiming read. Several combat text lines were also reworded to stay in the game world instead of sounding like patch notes.   

## V3.49.36 — Enemy variants and fairy affinity   

- V3.49.36 expands behavior profiles with more precise variants: bat, rat/pest, massive charger, spider, kobold, archer, alchemist, fairy, specter, construct, dragon and sacred oath. Profiles now have a signature attack, cleaner reactions and a counterplay line. Fairies also gain a real rule: 50% magical resistance, but 50% physical weakness. Offensive scrolls count as magical damage so this weakness/resistance is actually visible.   

## V3.49.34 — Monster profiles and signature attacks   

- V3.49.34 adds a first central behavior profile layer for enemies. Slimes, thieves, goblins, brutes, predators, guardians, plants, insectoids, supports and unstable entities gain more specific attack descriptions, strengths, weaknesses and reactions. Active observation and the bestiary now display those profiles to help the player understand why one enemy acts differently from another.   

## V3.49.33 — Enemy formations and formation break   

- V3.49.33 adds a new combat layer around enemy formation turns. Some coordinated waves can now spend a turn on coverage, short precision, light warding or defensive posture instead of always making a basic attack. The player gets the answer **Break formation**, a tactical action that perturbs several enemies, can remove defensive postures, and can unlock **Formation breaker** after repeated real use.   

## V3.49.31 — Guard break and support posture   

- V3.49.31 added two more tactical actions: **Guard Break**, a short control action with weakening/vulnerability, and **Hold the line / cover**, a support posture with short provocation, elemental guard and precision. These actions started passive progress toward **Guard Breaker** and **Support Rhythm**.   

## V3.49.30 — Delayed Hero Villager rumor and enemy pressure   

- V3.49.30 fixes the early Hero Villager rumor: the guild no longer mentions him immediately after registration on day 0. The rumor and the rare road encounter now require real progression first: enough days, level, contracts, story progress, tactical actions or observation. Enemies also gain a small non-basic pressure system: intelligent profiles can feint, wounded or opportunistic creatures can exploit visible openings, and bestial enemies can sometimes create vulnerability instead of always using a plain attack.   

## V3.49.29 — Weapon preparation, rogue reading and linked chests   

- V3.49.29 adds the **Coat / quick-fuse the weapon** tactical action, consuming a component to apply a temporary effect on a target: poison, shock, frost, vulnerability, precision or power depending on the material. Daggers receive a special synergy close to the poisoned dagger idea. Active observation can also teach **Lock and Fault Reading** to discreet profiles, then that reading actually helps with suspicious exploration chests.   

## V3.49.28 — Tactical openings and combat crafting   

- V3.49.28 expands the **Tactical Actions** menu with **Exploit an opening** and **Improvised artisan trap**. Wounds and statuses can now create useful tactical reactions, and some small materials can be consumed in combat to disrupt the enemy line. Tactical actions also progress passive skills such as **Terrain Reading** and **Combat Improviser**.   

## V3.49.27 — Feedback audit, Dinotofu companion and status combos   

- V3.49.27 adds a first **Dinotofu Companion** available from the activity menu and the out-of-combat menu. It gives short advice based on HP, quests, lanterns, skills and the local beta log. Weapon techniques also benefit from status reactions such as burning + frost, poison + bleeding, shock + vulnerability, or weakening + vulnerability. This pass also fixes version-file consistency after V3.49.26.   

## V3.49.26 — Tactical actions, guild board and quick categories   

- V3.49.26 adds the **Tactical Actions** menu in wave combat: throw a lantern, throw a lantern on the ground, push back, use dust or actively observe. The guild board now uses a separate offer label instead of active-quest wording, displays rank and clarifies same-location offers. The **Boutiques et comptoirs** menu also gains several useful quick categories.   

## V3.49.25 — More tactical ally directives   

- V3.49.25 adds more useful combat directives for recruits: force a ready technique on one recruit, request a coordinated group breakthrough, or spread targets for 1 turn against multiple enemies. These directives do not consume the player turn, but they expire after the ally turn, except focus priority which remains until the target dies or disappears.   

## V3.49.25 — Combat variety and beta log access   

- V3.49.25 adds Rupture de ligne, Suture de fortune and Signal de focus, slightly stabilizes normal attacks with very wide damage ranges, and adds a Beta log entry in the post-combat menu to find `logs/dinotofu_session_latest.txt` quickly.   

## V3.49.25 — Recruit personal quests and richer ally contributions   

- V3.49.25 adds a first personal-quest loop for recruits. A recruit can now ask for a profile-flavored personal issue to be solved, progress can be recorded across attempts, and completion improves loyalty slightly while adding clan reputation. Ally combat contribution also becomes more readable: support actions and finish blows are tracked, and guard/rogue-style recruits gain more distinct active techniques.   

## V3.49.19 — Blocking infirmary debts   

- V3.49.19 makes infirmary debts actually restrictive: as long as any debt remains, paid healing is blocked. If total debt exceeds 100 gold, recovering/reviving a recruit who is ready to leave is also blocked until the debt is reduced.   

## V3.49.17 — Full infirmary service and less overpowered inn healing   

- V3.49.17 adds a real infirmary service: heal yourself, heal a team member, or manage admission/recovery for KO recruits. Paid care restores up to 90% HP. Inn healing is rebalanced: common/simple beds cap at 50% HP, while the safer expensive room can reach 90% without giving a free full heal.   

## V3.49.16 — Persistent recruits and infirmary evacuation   

- V3.49.16 keeps improving combat feedback: recruited allies now have persistent saved HP instead of temporary combat HP. They can start below full health, keep their HP/potions between fights, and if one falls to 0 HP they must be evacuated to the infirmary rather than disappearing there automatically. If the player also falls, several days can pass depending on severity, and KO recruits are transferred to treatment too.   

## V3.49.11 — Recruited allies in PvE combat   

- Equipped recruits now start acting in standard PvE combat with simple support, damage, contextual healing and reward sharing. The player remains first and keeps the biggest share; manual team order stays above the automatic fallback.   

## V3.49.70 — FireFlight group mortal mark   

- V3.49.70 changes FireFlight's mortal mark from a single-target panic test into a group-wide mark over all opponents facing him. For 2 boss turns, any marked opponent who falls is treated as a real permanent death, with RP pressure lines making it clear that FireFlight is playing with the whole group rather than only one target.   

## V3.49.69 — FireFlight mortal mark and recruit hesitation   

- V3.49.69 turns FireFlight's panic window into a targeted **mortal mark** placed on the player. For 2 boss turns, falling under this mark is treated as permanent death rather than a normal non-lethal defeat. The pass also adds a small low-rank recruit hesitation rule: inexperienced allies may back out of an advanced technique without a clear order and fall back to a simpler action.   

## V3.49.68 — Variable mastery caps and FireFlight mortal window   

- V3.49.68 stops treating 10 active levels and 5 passive levels as mandatory caps for every skill. They are now absolute maximums only, while simpler skills can end earlier and display their local cap. FireFlight also gains a rare 2-turn mortal-rules window for extra panic without changing the saved difficulty.   

## V3.49.67 — Active/passive progression readability   

- V3.49.67 improves skill progression feedback. Active tactical mastery now explains both the next use threshold and the kind of small effect gained, while mastery passives show their own 5-level progress and light effect hint. Tactical actions also clarify that mastery stays balanced: mostly reliability, precision, breath, small rhythm or control rather than early overpowered scaling. Ally order menus now remind the player that low-rank recruits understand clear orders better than vague group intent.   

## V3.49.66 — Recruited ally combat maturity   

- V3.49.66 adds a combat maturity layer to recruited allies. Low-rank recruits now read the field less often, choose simpler targets, use advanced techniques more rarely and follow clear orders better than vague instinct. Higher rank, level and equipment gradually improve reactions, tactical comments, targeting and technique reliability so ally growth is more visible.   

## V3.49.65 — Living enemy and ally reads   

- V3.49.65 adds more contextual combat reading without changing the menu structure. Enemy behavior lines now react more clearly to archetype, visible statuses and target danger, while recruited allies can comment on the first enemy profile, player danger, group pressure and their own role or race. **World / city** also gains short ambience and rumor lines based on time of day, health and quest context.   

## V3.49.62 — Clear base menu routing after combat   

- V3.49.62 clarifies the menu flow after combat. **Continue** now means returning to the **Base menu**. **World / city** remains a normal base-menu activity for explorable places, shops, guild, NPCs and services, while the quick menu remains a constant character/session/save hub.   

## V3.49.61 — Quick menu correction and explorable city routing   

- V3.49.61 corrects the menu refactor: **Character** stays in the quick menu and groups inventory, titles, active/passive skills, accepted quests, statistics, quick equipment, team and exchange. **World / city** is no longer part of the quick menu because it represents explorable places; it stays available from the activity selection. Post-combat now sends the player to **Continue** for city visits, or to the quick menu for character/options/save actions.   

## V3.49.59 — Skill loadout menu and active/passive audit   

- V3.49.59 adds the out-of-combat skill loadout menu in the statistics hub. Active skills can be equipped or unequipped, passive skills can be enabled or disabled, and combat-opened statistics stay consult-only to avoid changing a build during a turn. The pass also audits recent tactical actions so active identifiers are no longer treated as passive effects.   

## V3.49.58 — Skill loadout and progressive mastery   

- V3.49.58 adds the base distinction between known, equipped and enabled skills. A character can know more skills, but only 10 active skills can be equipped and only 10 passives can be enabled at the same time. Active skills remain chosen actions, while passives remain automatic or semi-automatic effects that may also work outside combat depending on their nature.   

- Active skills now gain up to 10 mastery tiers, with spaced thresholds so they do not become too strong too early in a progression designed to reach level 255. Repeated-practice passives must first pass three visible successful trials before becoming real passives.   

## V3.49.57 — Active/passive mastery split   

- V3.49.57 clarifies the difference between selectable combat techniques and passive mastery. The affinity techniques remain active choices in the tactical menu, while repeated real uses now unlock separately named mastery passives such as **Elemental Mastery**, **Circular Guard**, **Binding Trait**, **Revigorating Voice**, **Channeled Instinct**, and **Blade Rhythm**. Legacy save identifiers remain recognized, but player-facing names no longer make it look as if **Blade Dance** or the other techniques became automatic passives.   

## V3.49.56 — Varied affinity techniques   

- V3.49.56 adds six more affinity techniques so combat does not revolve only around breaks/debuffs: **Elemental Blade**, **Protective Circle**, **Binding Shot**, **Inspiring Chant**, **Beast Instinct** and **Blade Dance**. Elemental, protector, skirmisher, bard/leader, wild and duelist profiles each gain a dedicated option with passive progression after real uses.   

## V3.49.55 — Wider class-affinity techniques   

- V3.49.55 adds five more class-affinity techniques: **Mastered Rage**, **Battle Order**, **Breath Totem**, **Workshop Bomb** and **Steel Prayer**. The pass widens specialized class gameplay without making every action universal: frontline, command, nature, workshop and sacred profiles each gain a dedicated option with passive progression after real uses.   

## V3.49.53 — Class-affinity techniques   

- V3.49.53 adds a first layer of **class-affinity tactical techniques**: **Shadow step** is no longer universal, and sneaky, support, arcane, rampart and skirmisher profiles each gain a dedicated technique with progression after real uses.   

## V3.49.50 — Reach, bulwarks and anchors   

- V3.49.50 adds three tactical actions: **Break reach**, **Pierce bulwark** and **Break occult anchor**. It also adds **Careful reach**, **Necrotic anchor** and **Life drain** profiles, with distance reactions, grave-cold pressure, short recovery and counterplay through broken spacing or broken anchors.   

## V3.49.49 — Status chains, marked prey and enemy profiles   

- V3.49.49 adds three tactical actions: **Force a status chain**, **Mark prey** and **Controlled retreat**. It also adds the **Field Healer**, **Shield Bearer**, **Wounded Berserker** and **Curse Bearer** profiles, with their own observation lines, weaknesses, signature attacks and mechanical reactions.   

## V3.49.38 — Size, material and physical durability   

- V3.49.38 adds a real body durability read for creatures: small fragile bodies, small protected bodies, large organic masses, large hard masses, poor material bodies, slimes, specters and fairies no longer react the same way to physical damage. Observation and bestiary entries expose that read, and the damage report shows an immersive line when the body changes the impact. Recruited ally techniques also gain gestures tied more closely to their profile.   

## V3.49.74 — Wider mastery effects and sortie preparation   

- V3.49.74 continues the six active work tracks at once: several older affinity techniques now receive real mastery scaling, tactical passives remain supporting effects instead of automatic actions, enemy pressure gains more contextual intent lines, recruited allies scale their technique frequency more clearly with maturity, and World / city gains a sortie preparation reader for care, tools, observation, chests, combat crafting and mercenary options.   

## V3.49.73 — Enabled mastery passives and clearer city signals   

- V3.49.73 connects enabled mastery passives to more real tactical effects: an unlocked and enabled passive can now lightly support power, secondary chance, duration or rhythm without ever triggering the active action by itself. World / city also shows clearer local signals such as current city, guild registration and travel time.   

## V3.49.81 — Readable synergies and mastery impact   

- V3.49.81 adds a global build read to the Character menu and run preparation: weapon, armor, class bonus/malus and overall coherence. Active/passive mastery labels describe impact more clearly, recruited ally techniques scale more with maturity and orders, and enemy signature skills become slightly more present when an enemy is trained, elite or facing an already-open player.   
