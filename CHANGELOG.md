# Dinotofu Changelog   

Detailed English version history for Dinotofu. The matching French history is stored in `CHANGELOG_FR.md`. README files remain focused on installation and useful player information.   

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
