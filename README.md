# Dinotofu   

Dinotofu is a C++17 RPG / arena game. The terminal version remains the stable foundation, while an experimental HTML/JavaScript graphical interface is being developed for a more comfortable desktop experience.   

## Current version   

- Current version: **V3.49.89**   
- Strongly recommended character recreation baseline: **V3.00.00**   
- Characters created before **V3.00.00** may still work, but recreating them is recommended to use the story foundation cleanly.   

## What is already in the game   

The game already lets players:   

- create, load, and save accounts and characters;   
- choose a race, an exact race-compatible age, visual presentation, final variant, class, difficulty, death rule, and certain protected special identities;   
- fight in AI PvP, two-player PvP, monster PvE, solo boss fights, and cooperative boss fights;   
- manage inventory, weapons, armor, consumables, materials, durability, repairs, and item quality;   
- buy, sell and barter through one unified **Boutiques et comptoirs** menu with quick categories or complete list, visit locations, use the guild, library, field observation post, mercenary counter, training stand, and purchase/upgrade independent municipal vaults per town;   
- explore biomes containing resources, chests, traps, tracks, events, dangerous locations, fused slimes, and a four-stage dividing slime mini-boss;   
- follow guild, client, exploration, combat, delivery, service, bestiary, and main-story quests;   
- accept up to three daily guild challenges, earn Challenge Marks, and unlock titles tied to real combat restrictions;   
- consult three special permanent registers: Bestiary for beings, Encyclopedia for systems/items/areas, and Discovery Notebook for lore, legends, and rumours;   
- use hidden systems such as cheat codes, altered data, special characters, legendary merchants, and selected lore events;   
- launch a story mode built as a progressively restricted guided sandbox, with a prologue, playable Chapters 1–3, and a first playable Chapter 4 investigation phase around the village at the wrong date.   

## Story mode   

Story mode begins with deliberate restrictions: few shops, routes, stocks, and non-refusable main quests. It uses the exact same entries as sandbox play — **Combat**, **Exploration**, **Quests**, **Notable NPCs**, and **Notable locations** — without creating a parallel route. The only difference is what truly exists at the current story step: NPCs who have not arrived, destroyed or unfinished buildings, unopened shops, and future areas remain hidden. **Notable locations** is organized everywhere into three sections: **Town**, **Outside**, and **Shops**, with a **Show all** view for players who want the complete list immediately. **Notable NPCs** provides the same full-list shortcut before its categories. Step-based objectives reveal only the current stage and mark completed stages with `[fait]`. In Chapter 1, Mira remains the only mandatory first contact. Orren, Lysa, Bram, and Soryn then become available together; each one immediately grants a separate main quest that can be progressed or completed in any order. After all four conversations, Mira creates a synthesis quest that reads the real state of those four quests, including any objective already turned in earlier. Completed main-story objectives remain available in a dedicated archive.   

The sandbox remains available alongside it for unrestricted testing, exploration, boss fights, and system use without following the narrative order.   

## Boss registry progression   

Boss discovery follows an internal ordered progression without exposing the full list to the player. A first victory can add at most the next unknown presence as `???`. Repeating an already defeated boss reveals nothing new. Exploration can exceptionally reveal an approximate location without a victory, but this event is extremely rare, has a long in-game cooldown, and reveals no identity. FireFlight keeps its separate final requirement.   

## Graphical interface   

The graphical interface remains experimental but playable and mirrors the C++ engine without inventing hidden game logic. V3.49.62 clarifies terminal routing after combat: Continue returns to the **Base menu**, and **World / city** is selected there as a normal explorable activity. The quick menu stays focused on the character, free input, session options and save options. Terminal mode remains the most reliable fallback: gameplay images are disabled and not toggleable there, while launchers/installers may use their dedicated icons.   

## Installing from a GitHub release   

To install Dinotofu without compiling the project manually:   

1. open the GitHub repository page;   
2. open the latest Release shown on the repository page;   
3. download the launcher for your operating system: Windows or Linux;   
4. run the launcher, which opens or repairs the appropriate game version.   

On desktop Windows/Linux builds, the installer or launcher should create two clear entries:   

- **ProjetDinotofu Launcher**: normal / automatic launch through the OS-specific launcher;   
- **ProjetDinotofu Launcher Terminal version**: forces the terminal version through the corresponding Windows/Linux launcher.   

## Quick project structure   

- `src/`: C++ game source code;   
- `include/`: C++ headers;   
- `assets/config/`: reference JSON data;   
- `assets/branding/`: official logo, banner, intro/menu cover and launcher/shortcut icons;   
- `tools/gui/`: experimental graphical interface and local server;   
- `tools/windows/` and `tools/linux/`: launchers/installers;   
- `scripts/`: packaging, versioning, release, and validation scripts;   
- `release/`: release manifest and publication data;   
- `assets/saves/`: local save folders, without private data in release ZIP files.   
- `logs/dinotofu_session_latest.txt`: local beta session log, safe to delete and useful to send to the developer after bugs or strange fights.   

## Release notes   

Detailed release notes are split by language:   

- English: `PATCHNOTE_DINOTOFU.md`;   
- French: `PATCHNOTE_DINOTOFU_FR.md`.   

README files and design documents are not development logs. They contain only useful player and project information.   

### V3.49.89 - Rivals and unstable fate   

V3.49.89 adds the Rival and Unstable Fate church oaths. Escapes, panic reactions and signature skills can leave clearer rival traces, while unstable fate creates small oscillations only when an actual trace already exists: memory, broken oath, rival or marked item.   

## Special developer note   

Hello, future adventurer, future hero, future statistically probable corpse.   

Welcome to Dinotofu. Here, you can win with a real strategy, die because you tried to look clever, or discover that a tax-motivated goblin is sometimes more dangerous than a mythical boss. The game is still under construction, but it already exists to be played, broken, criticized, trolled, improved, and tested again.   

I want Dinotofu to remain free: you can optimize, try stupid ideas, die like a legend, or survive when you clearly did not deserve it. When a mechanic frustrates you, a boss feels unfair, a strategy destroys the balance, or an idea makes you want to continue the adventure, say it. The game should grow with player feedback, not only with ideas developed alone in a corner.   

Challenge number one: survive long enough to understand what is happening.   

Challenge number two: defeat a boss without saying the game cheats. Spoiler: sometimes it may cheat, but at least it does it with style.   

Challenge number three: find a strategy that breaks the game, show it to me, and let me pretend it was planned from the beginning.   

Play seriously, but not too seriously. Complain when needed, suggest ideas, insult bosses politely, and above all tell me what makes you want to launch another run. When you find an overpowered combination, I will probably deny panicking at the numbers, then claim it was an advanced balance test and everything was planned. That is false, but it sounds more professional.   

When you become a legend in Dinotofu, congratulations. When you die to the first enemies, congratulations too, but differently. When you lose to a chest, a trap, or your own confidence, remember that the game does not judge you. I do a little, but affectionately.   

## V3.30.00 note — routes and equipment weight   

- Cities now have distances between each other, distances toward biomes, and access requirements based on level or defeated bosses.   
- Vaults remain independent per city: travel changes the active town but does not merge contents.   
- The textual exploration map prepares future pixel-art backgrounds per biome, with unknown places shown as grey or foggy.   
- Weapons and armor now have light, medium, or heavy weight classes. Bonuses and tradeoffs are applied to combat, damage, and escape, with intentionally moderate penalties.   



## V3.31.00 note — city hubs and canonical journal   

V3.31.00 turns the current city into a more structured hub: local buildings, contacts, lock states and future pixel-art hints now come from world rules instead of one fixed text list. City destinations expose structured GUI metadata for access, distance, travel time and future route costs. A saved canonical journal now records key world events such as places visited, routes taken and vault movements so later Top 3 screens can rely on engine data rather than parsed interface text.   




### V3.49.86 - Loadout-aware tactical mastery   

V3.49.86 ties active mastery more directly to class/loadout coherence. A coherent weapon or armor can lightly support linked tactical actions, while a class malus can make the gesture less clean even with mastery. Class audit now shows survival, crit identity and expected equipment, and observation-style passives reduce surprise against enemy signature skills.   

### V3.49.36 - Creature aiming and more immersive combat text   

V3.49.36 adds an aiming read to enemy profiles: small creatures such as rats, fairies, bats, insects and sneaky profiles are harder to frame, while brutes, constructs, dragons, rooted plants and guardians sometimes leave easier windows to hit. Observation and bestiary entries now expose this aiming read. Several combat text lines were also reworded to stay in the game world instead of sounding like patch notes.   

### V3.49.36 - Enemy variants and fairy affinity   

V3.49.36 expands behavior profiles with more precise variants: bat, rat/pest, massive charger, spider, kobold, archer, alchemist, fairy, specter, construct, dragon and sacred oath. Profiles now have a signature attack, cleaner reactions and a counterplay line. Fairies also gain a real rule: 50% magical resistance, but 50% physical weakness. Offensive scrolls count as magical damage so this weakness/resistance is actually visible.   

### V3.49.34 - Monster profiles and signature attacks   

V3.49.34 adds a first central behavior profile layer for enemies. Slimes, thieves, goblins, brutes, predators, guardians, plants, insectoids, supports and unstable entities gain more specific attack descriptions, strengths, weaknesses and reactions. Active observation and the bestiary now display those profiles to help the player understand why one enemy acts differently from another.   

### V3.49.33 - Enemy formations and formation break   

V3.49.33 adds a new combat layer around enemy formation turns. Some coordinated waves can now spend a turn on coverage, short precision, light warding or defensive posture instead of always making a basic attack. The player gets the answer **Break formation**, a tactical action that perturbs several enemies, can remove defensive postures, and can unlock **Formation breaker** after repeated real use.   

### V3.49.31 - Guard break and support posture   

V3.49.31 added two more tactical actions: **Guard Break**, a short control action with weakening/vulnerability, and **Hold the line / cover**, a support posture with short provocation, elemental guard and precision. These actions started passive progress toward **Guard Breaker** and **Support Rhythm**.   

### V3.49.30 - Delayed Hero Villager rumor and enemy pressure   

V3.49.30 fixes the early Hero Villager rumor: the guild no longer mentions him immediately after registration on day 0. The rumor and the rare road encounter now require real progression first: enough days, level, contracts, story progress, tactical actions or observation. Enemies also gain a small non-basic pressure system: intelligent profiles can feint, wounded or opportunistic creatures can exploit visible openings, and bestial enemies can sometimes create vulnerability instead of always using a plain attack.   

### V3.49.29 - Weapon preparation, rogue reading and linked chests   

V3.49.29 adds the **Coat / quick-fuse the weapon** tactical action, consuming a component to apply a temporary effect on a target: poison, shock, frost, vulnerability, precision or power depending on the material. Daggers receive a special synergy close to the poisoned dagger idea. Active observation can also teach **Lock and Fault Reading** to discreet profiles, then that reading actually helps with suspicious exploration chests.   

### V3.49.28 - Tactical openings and combat crafting   

V3.49.28 expands the **Tactical Actions** menu with **Exploit an opening** and **Improvised artisan trap**. Wounds and statuses can now create useful tactical reactions, and some small materials can be consumed in combat to disrupt the enemy line. Tactical actions also progress passive skills such as **Terrain Reading** and **Combat Improviser**.   

### V3.49.27 - Feedback audit, Dinotofu companion and status combos   

V3.49.27 adds a first **Dinotofu Companion** available from the activity menu and the out-of-combat menu. It gives short advice based on HP, quests, lanterns, skills and the local beta log. Weapon techniques also benefit from status reactions such as burning + frost, poison + bleeding, shock + vulnerability, or weakening + vulnerability. This pass also fixes version-file consistency after V3.49.26.   

### V3.49.26 - Tactical actions, guild board and quick categories   

V3.49.26 adds the **Tactical Actions** menu in wave combat: throw a lantern, throw a lantern on the ground, push back, use dust or actively observe. The guild board now uses a separate offer label instead of active-quest wording, displays rank and clarifies same-location offers. The **Boutiques et comptoirs** menu also gains several useful quick categories.   

### V3.49.25 — More tactical ally directives   

V3.49.25 adds more useful combat directives for recruits: force a ready technique on one recruit, request a coordinated group breakthrough, or spread targets for 1 turn against multiple enemies. These directives do not consume the player turn, but they expire after the ally turn, except focus priority which remains until the target dies or disappears.   

### V3.49.25 — Combat variety and beta log access   

V3.49.25 adds Rupture de ligne, Suture de fortune and Signal de focus, slightly stabilizes normal attacks with very wide damage ranges, and adds a Beta log entry in the post-combat menu to find `logs/dinotofu_session_latest.txt` quickly.   

### V3.49.25 — Recruit personal quests and richer ally contributions   

V3.49.25 adds a first personal-quest loop for recruits. A recruit can now ask for a profile-flavored personal issue to be solved, progress can be recorded across attempts, and completion improves loyalty slightly while adding clan reputation. Ally combat contribution also becomes more readable: support actions and finish blows are tracked, and guard/rogue-style recruits gain more distinct active techniques.   

### V3.49.19 — Blocking infirmary debts   

V3.49.19 makes infirmary debts actually restrictive: as long as any debt remains, paid healing is blocked. If total debt exceeds 100 gold, recovering/reviving a recruit who is ready to leave is also blocked until the debt is reduced.   

### V3.49.17 — Full infirmary service and less overpowered inn healing   

V3.49.17 adds a real infirmary service: heal yourself, heal a team member, or manage admission/recovery for KO recruits. Paid care restores up to 90% HP. Inn healing is rebalanced: common/simple beds cap at 50% HP, while the safer expensive room can reach 90% without giving a free full heal.   

### V3.49.16 — Persistent recruits and infirmary evacuation   

V3.49.16 keeps improving combat feedback: recruited allies now have persistent saved HP instead of temporary combat HP. They can start below full health, keep their HP/potions between fights, and if one falls to 0 HP they must be evacuated to the infirmary rather than disappearing there automatically. If the player also falls, several days can pass depending on severity, and KO recruits are transferred to treatment too.   

### V3.49.11 — Recruited allies in PvE combat   

Equipped recruits now start acting in standard PvE combat with simple support, damage, contextual healing and reward sharing. The player remains first and keeps the biggest share; manual team order stays above the automatic fallback.   




### V3.49.70 - FireFlight group mortal mark   

V3.49.70 changes FireFlight's mortal mark from a single-target panic test into a group-wide mark over all opponents facing him. For 2 boss turns, any marked opponent who falls is treated as a real permanent death, with RP pressure lines making it clear that FireFlight is playing with the whole group rather than only one target.   

### V3.49.69 - FireFlight mortal mark and recruit hesitation   

V3.49.69 turns FireFlight's panic window into a targeted **mortal mark** placed on the player. For 2 boss turns, falling under this mark is treated as permanent death rather than a normal non-lethal defeat. The pass also adds a small low-rank recruit hesitation rule: inexperienced allies may back out of an advanced technique without a clear order and fall back to a simpler action.   

### V3.49.68 - Variable mastery caps and FireFlight mortal window   

V3.49.68 stops treating 10 active levels and 5 passive levels as mandatory caps for every skill. They are now absolute maximums only, while simpler skills can end earlier and display their local cap. FireFlight also gains a rare 2-turn mortal-rules window for extra panic without changing the saved difficulty.   

### V3.49.67 - Active/passive progression readability   

V3.49.67 improves skill progression feedback. Active tactical mastery now explains both the next use threshold and the kind of small effect gained, while mastery passives show their own 5-level progress and light effect hint. Tactical actions also clarify that mastery stays balanced: mostly reliability, precision, breath, small rhythm or control rather than early overpowered scaling. Ally order menus now remind the player that low-rank recruits understand clear orders better than vague group intent.   

### V3.49.66 - Recruited ally combat maturity   

V3.49.66 adds a combat maturity layer to recruited allies. Low-rank recruits now read the field less often, choose simpler targets, use advanced techniques more rarely and follow clear orders better than vague instinct. Higher rank, level and equipment gradually improve reactions, tactical comments, targeting and technique reliability so ally growth is more visible.   

### V3.49.65 - Living enemy and ally reads   

V3.49.65 adds more contextual combat reading without changing the menu structure. Enemy behavior lines now react more clearly to archetype, visible statuses and target danger, while recruited allies can comment on the first enemy profile, player danger, group pressure and their own role or race. **World / city** also gains short ambience and rumor lines based on time of day, health and quest context.   

### V3.49.62 - Clear base menu routing after combat   

V3.49.62 clarifies the menu flow after combat. **Continue** now means returning to the **Base menu**. **World / city** remains a normal base-menu activity for explorable places, shops, guild, NPCs and services, while the quick menu remains a constant character/session/save hub.   

### V3.49.61 - Quick menu correction and explorable city routing   

V3.49.61 corrects the menu refactor: **Character** stays in the quick menu and groups inventory, titles, active/passive skills, accepted quests, statistics, quick equipment, team and exchange. **World / city** is no longer part of the quick menu because it represents explorable places; it stays available from the activity selection. Post-combat now sends the player to **Continue** for city visits, or to the quick menu for character/options/save actions.   

### V3.49.59 - Skill loadout menu and active/passive audit   

V3.49.59 adds the out-of-combat skill loadout menu in the statistics hub. Active skills can be equipped or unequipped, passive skills can be enabled or disabled, and combat-opened statistics stay consult-only to avoid changing a build during a turn. The pass also audits recent tactical actions so active identifiers are no longer treated as passive effects.   

### V3.49.58 - Skill loadout and progressive mastery   

V3.49.58 adds the base distinction between known, equipped and enabled skills. A character can know more skills, but only 10 active skills can be equipped and only 10 passives can be enabled at the same time. Active skills remain chosen actions, while passives remain automatic or semi-automatic effects that may also work outside combat depending on their nature.   

Active skills now gain up to 10 mastery tiers, with spaced thresholds so they do not become too strong too early in a progression designed to reach level 255. Repeated-practice passives must first pass three visible successful trials before becoming real passives.   

### V3.49.57 - Active/passive mastery split   

V3.49.57 clarifies the difference between selectable combat techniques and passive mastery. The affinity techniques remain active choices in the tactical menu, while repeated real uses now unlock separately named mastery passives such as **Elemental Mastery**, **Circular Guard**, **Binding Trait**, **Revigorating Voice**, **Channeled Instinct**, and **Blade Rhythm**. Legacy save identifiers remain recognized, but player-facing names no longer make it look as if **Blade Dance** or the other techniques became automatic passives.   

### V3.49.56 - Varied affinity techniques   

V3.49.56 adds six more affinity techniques so combat does not revolve only around breaks/debuffs: **Elemental Blade**, **Protective Circle**, **Binding Shot**, **Inspiring Chant**, **Beast Instinct** and **Blade Dance**. Elemental, protector, skirmisher, bard/leader, wild and duelist profiles each gain a dedicated option with passive progression after real uses.   

### V3.49.55 - Wider class-affinity techniques   

V3.49.55 adds five more class-affinity techniques: **Mastered Rage**, **Battle Order**, **Breath Totem**, **Workshop Bomb** and **Steel Prayer**. The pass widens specialized class gameplay without making every action universal: frontline, command, nature, workshop and sacred profiles each gain a dedicated option with passive progression after real uses.   

### V3.49.53 - Class-affinity techniques   

V3.49.53 adds a first layer of **class-affinity tactical techniques**: **Shadow step** is no longer universal, and sneaky, support, arcane, rampart and skirmisher profiles each gain a dedicated technique with progression after real uses.   

### V3.49.50 - Reach, bulwarks and anchors   

V3.49.50 adds three tactical actions: **Break reach**, **Pierce bulwark** and **Break occult anchor**. It also adds **Careful reach**, **Necrotic anchor** and **Life drain** profiles, with distance reactions, grave-cold pressure, short recovery and counterplay through broken spacing or broken anchors.   

### V3.49.49 - Status chains, marked prey and enemy profiles   

V3.49.49 adds three tactical actions: **Force a status chain**, **Mark prey** and **Controlled retreat**. It also adds the **Field Healer**, **Shield Bearer**, **Wounded Berserker** and **Curse Bearer** profiles, with their own observation lines, weaknesses, signature attacks and mechanical reactions.   

### V3.49.38 - Size, material and physical durability   

V3.49.38 adds a real body durability read for creatures: small fragile bodies, small protected bodies, large organic masses, large hard masses, poor material bodies, slimes, specters and fairies no longer react the same way to physical damage. Observation and bestiary entries expose that read, and the damage report shows an immersive line when the body changes the impact. Recruited ally techniques also gain gestures tied more closely to their profile.   


### V3.49.74 - Wider mastery effects and sortie preparation   

V3.49.74 continues the six active work tracks at once: several older affinity techniques now receive real mastery scaling, tactical passives remain supporting effects instead of automatic actions, enemy pressure gains more contextual intent lines, recruited allies scale their technique frequency more clearly with maturity, and World / city gains a sortie preparation reader for care, tools, observation, chests, combat crafting and mercenary options.   

### V3.49.73 - Enabled mastery passives and clearer city signals   

V3.49.73 connects enabled mastery passives to more real tactical effects: an unlocked and enabled passive can now lightly support power, secondary chance, duration or rhythm without ever triggering the active action by itself. World / city also shows clearer local signals such as current city, guild registration and travel time.   


### V3.49.81 - Readable synergies and mastery impact   

V3.49.81 adds a global build read to the Character menu and run preparation: weapon, armor, class bonus/malus and overall coherence. Active/passive mastery labels describe impact more clearly, recruited ally techniques scale more with maturity and orders, and enemy signature skills become slightly more present when an enemy is trained, elite or facing an already-open player.   

