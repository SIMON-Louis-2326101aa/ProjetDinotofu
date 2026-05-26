# Dinotofu

Dinotofu is a C++17 terminal RPG and arena game built around character progression, tactical combat, exploration, quests, crafting, shops, boss encounters and long-term account/character saves.

The project started as a large single-file prototype and is being rebuilt into a cleaner object-oriented architecture. The terminal version now acts as the stable gameplay base while the project prepares the graphical interface layer. The goal is to keep the existing systems playable, readable and reusable before building the full story mode.

## What you can do in the game

Dinotofu already offers several playable or partially connected activities:

- create, load and save accounts and characters;
- choose a race, class, difficulty and protected special identities when allowed;
- fight in PvP AI, two-player PvP, PvE monster waves and boss arenas;
- explore biomes with resources, events, chests, traps, dangerous places, mini-bosses and rare traces;
- accept and follow guild, client, exploration, combat, delivery and bestiary-style quests;
- visit shops and locations such as forge, herb shop, monster material shop, equipment shops, consumables and library-style knowledge hooks;
- manage inventory, weapons, armor, consumables, materials, repair kits, quality tiers and durability;
- craft items from recipes and material qualities;
- consult the bestiary, material journal, summon journal, rare object entries and lore categories;
- use or face special characters, special groups and hidden altered-data systems;
- unlock or progress durable skills through gameplay actions, class/race hooks and weapon usage;
- track character statistics, equipment usage, combat results, boss progress and special states.

The story mode is visible but still intentionally unfinished. The real story is planned later, after the terminal systems and the future graphical interface are ready.

## Main activities

The main menu is organized around activities:

1. Story
2. Combats
3. Exploration
4. Quests
5. Shops / visitable places
6. Notable NPCs
7. Trade / donation
8. Information about all options
0. Save and quit

The information menu explains what each activity currently does and what is planned for later.

## Combat

Combat is turn-based. Depending on the mode, the player can attack, use potions, manage equipment, use defense posture, provoke as a tank, open inventory or bestiary tools, control summons, use role/interface actions, attempt escape when allowed, or pass the turn.

Boss fights are special: escape is impossible in boss arenas and this must be explained through lore rather than a plain technical message. Boss ultimates unlock only after the boss has crossed the 50% HP threshold at least once.

## Boss system

Dinotofu contains a large validated boss roster. The first three bosses are visible by name at the start, while later entities are usually hidden as unknown abnormal energy variations until discovered.

FireFlight is treated as the final boss. His entry stays locked until the character has proven enough value through the boss progression. He has special meta/dev-style dialogue, reacts to altered characters, and has special lines for protected special characters.

## Exploration

Exploration is a main activity, separate from voluntary combat. The player chooses a biome and can find plants, materials, treasure, events, traces, NPCs, mini-bosses or dangerous places. Some events can lead to combat, but exploration itself is meant to feel like passive searching rather than simply selecting an enemy.

Known biome directions include ancient forest, cold mountain, troubled marsh, commercial road, collapsed ruins and wild plains.

## Quests

Quests are split between guild quests and personal/client/event quests.

Guild quests are limited and are handled through the guild. Personal or event quests can come from clients, NPCs, exploration, shops or combat events. Quest objectives can involve combat, exploration, delivery, resources or bestiary knowledge.

## Inventory, crafting and economy

The inventory handles weapons, armor, consumables, materials, repair kits, special items, craft recipes and bestiary access. Materials and plants can have qualities such as low quality, normal, high quality, impure, pure or exceptional. Different qualities do not stack together.

Crafting supports recipes, material requirements, quality influence and early profession bonuses. Exceptional materials can later create special item particularities.

Shops and visitable places are designed to support the long-term economy: buying, selling, rotating stock, rare resources, repair logic, knowledge purchases and quest hooks.

## Bestiary and knowledge

The bestiary is not only a monster list. It is meant to track enemies, boss information, invocations, materials, plants, races, special characters, rare objects, divinities and lore.

Knowledge is progressive: the player can meet entities, defeat them, buy information, read lore, discover rare traces or unlock deeper entries through exploration and combat.

## Summons and allies

Summons are connected to combat through slots and a summon-link resource. Some can be manually maintained or sacrificed. This is still a foundation for future mana, upkeep, sacrifices, allied AI, Hazak-related shadows, group combat and advanced summon evolution.

## Durable skills

Dinotofu distinguishes passive skills and active skills.

Passive skills improve or unlock naturally through the way the player plays. For example, repeated dagger kills can progress toward an active skill such as Chain Execution, while bow kills can progress toward Ranger Eye. Bare-hand kills, staff usage, class/race identity and character-specific hooks can also influence skill progression.

Active skills are meant to have cooldowns and combat actions. The current system already tracks first unlocks and progress, but the full combat skill menu and cooldown logic still need to be expanded.

Future skills can come from gameplay habits, level-up, race, character, class, weapons used most often and specific challenges or hidden conditions.

## Special characters

Protected special characters exist with special identity validation, native races, special behavior, group relationships and unique combat moments. Some are tied to the creator's DND characters or friends, and FireFlight has special dialogue for them.

Matt (PRO) remains a special opponent and is not meant to be a normal playable identity.

## Cheats and altered data

Hidden commands can mark a character as Altered. Altered characters keep that status even if cheat effects are later disabled.

Lethal difficulty blocks cheat activation. A failed attempt triggers a lore event, temporary penalties and entity reactions from beings connected to justice, destiny, anomaly, origin or the developer side of the world.

Some cheats are meant for testing, anti-grind access or hidden interactions, but using them changes the identity of the character.

## Difficulties

The game supports several difficulty intentions: Easy, Normal, Hard, Nightmare and Lethal.

Difficulty can influence starter resources, rewards, durability, escape, death, respawn, loot and shop behavior. Lethal is treated as the serious mode: death is meant to be definitive, with rare future exceptions only through very special narrative systems.

## Saves and Git hygiene

The project uses JSON saves for account and character data. Personal saves should not be committed.

The save folders remain present through `.gitkeep` placeholders, while generated save files are ignored by Git.

## Build requirements

Required on Linux:

- `g++`
- `make`
- C++17 support

Example install command on Debian / Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential make g++
```

WSL installation for Windows:

```bash
wsl --install
# If Windows asks for a distribution, choose Ubuntu.
wsl
sudo apt update
sudo apt install build-essential
sudo apt install cmake
```

## Commands

Compile the project:

```bash
make
```

Run the game directly after compilation:

```bash
make run
```

Run with the project launcher target:

```bash
make launch
```

On Windows, double-click:

```bash
Installer-Dinotofu.cmd / Lancer-Dinotofu.cmd
```

The release launcher checks GitHub updates, downloads a newer release when available, then starts the game.

Clean generated files:

```bash
make clean
```

Rebuild from zero:

```bash
make rebuild
```

Create a clickable Linux desktop launcher:

```bash
make desktop
# or
make install-desktop
```

The smart launcher also creates a desktop shortcut when possible.

## Project structure

The project is organized around folders such as:

- `src/core/`
- `src/combat/`
- `src/combat/modes/`
- `src/combat/boss/`
- `src/character/`
- `src/entity/`
- `src/item/`
- `src/weapon/`
- `src/armor/`
- `src/consumable/`
- `src/material/`
- `src/interface/`
- `src/menu/`
- `src/progression/`
- `src/death/`
- `src/bestiary/`
- `src/attribute/`
- `src/economy/`
- `src/shop/`
- `src/save/`
- `src/cheat/`
- `assets/config/`
- `assets/saves/`

## Development direction

Current priorities are:

1. Prepare the graphical interface layer from the current terminal systems.
2. Keep menus, paginated lists and navigation consistent so they can be reused cleanly.
3. Continue durable skills and class identity without breaking existing saves.
4. Add content progressively when a system needs it: monsters, bestiary entries, recipes, shops and events.
5. Complete DND-inspired attributes after the interface base is stable.
6. Build the real story mode after the graphical interface.

## Personal note

Challenge one: survive long enough to understand what is happening.

Challenge two: beat a boss without saying the game cheats. Spoiler: sometimes it might cheat, but at least it does it with style.

Challenge three: find a strategy that breaks the game, show it to me, and let me pretend it was planned from the beginning.

This project exists to be played, criticized, mocked, improved, and tested again. Play seriously, but not too seriously. Complain if needed, suggest ideas, politely insult the bosses, and most importantly tell me what makes you want to continue.

If you manage to become a legend in Dinotofu, congratulations. If you die against the first enemies, congratulations too, just differently.

Step into the arena. We will see whether you truly play well, or whether you only talk loudly on Discord.

During the real open beta, you may even have a chance to be directly integrated into the base game if your contribution deserves it. Prove that you are worth it. Good luck until then.

Update 1.32.28:
- Added Mana Suture: a short persistent regeneration status, displayed in active states and resolved at turn start.
- Added the Mana Suture Grimoire, a learnable spell without a common scroll equivalent, limited to coherent caster profiles with level and catalyst constraints.
- Added the Minor Purification Scroll, a one-use consumable usable by both mages and non-mages to remove simple combat afflictions.
- Expanded durable magic-study compatibility to coherent mystical, sacred, summoning and hybrid classes without giving permanent magic to everyone.
- Added bestiary/library notes about minor purification, mana suture and non-equivalent magical supports.

Ajout 1.32.30 :
- Grosse passe bestiaire : les fiches affichent maintenant danger estimé, habitat/origine, faiblesses, résistances, butin/ressource liée et conseil de chasse selon le niveau de connaissance.
- Ajout d'une catégorie dédiée aux effets et altérations : brûlure, poison, givre, choc, saignement, affaiblissement, faille ouverte, voile élémentaire et suture de mana.
- Ajout de nouvelles entrées de familles/variantes : slimes colorés, racine étrangleuse, squelette archer rouillé, oracle fissuré et bandit apothicaire.
- Ajout d'un index tactique dans le bestiaire pour lire rapidement les grandes familles, leurs contres et les pièges d'interprétation.
- Les entrées découvertes pendant la partie stockent aussi des indices persistants : danger, habitat, faiblesses, résistances, butin et stratégie.

## Version 1.32.31

- Expanded the bestiary with zone/habitat entries, many more hostile creature sheets, knowledge progression hints, a bestiary summary screen, and richer tactical notes for constructs, dragons, anomalies and biomes.


### V1.32.33
- Ajout d'une première infrastructure de publication GitHub : scripts de packaging, workflow GitHub Actions, guide d'installation et base installer/launcher PowerShell.
- Ajout de scripts pour créer une archive source sans exécutable et une release Linux compilée.
- Préparation d'un launcher Windows capable de vérifier les releases GitHub, télécharger une mise à jour avec progression et lancer le jeu quand une release Windows précompilée sera disponible.

## Version 1.32.40

- Inventory weapon, armor, consumable and material lists now use the shared terminal pagination helper instead of dumping very long lists.
- The quest journal now opens on active quests by default, keeps completed quests in a separate view, and supports page navigation.
- A few meta-style player-facing messages were replaced with more immersive text.


## Version 1.32.41
- Shop item lists now use paginated terminal views.
- Source packaging checks remain focused on game files and generated/private artifacts.

## Version 1.35.06
- Project version aligned to V1.35.06 after the interface-preparation branch became more than simple patch work.
- Priority list updated: graphical interface preparation is now the visible direction; balancing continues as normal background work, not as a repeated task.
- Empty reserve JSON files are now valid empty arrays, special-character dialogues include Trexof, cheat-code archives are synchronized, and console/input safety is improved.

## Version 1.35.07
- Added display-neutral `MenuScreen` / `MenuOption` models to centralize menu data before rendering.
- Terminal menus can now render shared screen models through `TerminalInterface`, preparing the future graphical interface.
- Combat turn, target choice, group target choice and post-combat menus were migrated to the shared screen model.


## Version 1.35.08
- Continued GUI preparation by moving equipment, inventory selection, potion, combat role and statistics hub screens to shared `MenuScreen` data.
- More menus now expose stable `screenId` and `actionId` values, so the future graphical interface can consume the same actions as the terminal.
- Terminal rendering remains active through `TerminalInterface`, keeping the game playable during the transition.


## Version 1.35.09
- Continued GUI preparation by migrating shop hub, shop stock, shop item inspection, bestiary hub, bestiary entry lists/details, quest hub, guild, locations, notable NPC and exploration selection screens to shared `MenuScreen` data.
- Shop, bestiary, quest and exploration screens now expose more stable `screenId` / `actionId` values for the future graphical interface.
- Terminal rendering remains unchanged in behavior, but more displayed menus now pass through `TerminalInterface`.


## Version 1.35.10
- Added `GuiMenuSnapshot` / `GuiMenuActionSnapshot` so the future graphical interface can read clean menu data without parsing terminal output.
- Added `TerminalInterface::askMenuChoice` to centralize migrated menu rendering and input handling.
- Saved account and character menus now use `MenuScreen` more directly with stable `screenId` / `actionId` values.
- Several exploration event choices now pass through a shared screen helper, with one overly meta wave message replaced by an in-world line.

## Version 1.35.11
- Continued GUI preparation on paginated inventory category screens: weapons, armors, consumables and materials now use shared `MenuScreen` data for their page rendering.
- Added pagination helpers that can populate `MenuScreen` navigation options directly, reducing terminal-only pagination code.
- Inventory page entries now expose stable action ids such as `inventory.weapon.select`, `inventory.armor.select`, `inventory.consumable.select` and `inventory.material.select`.
- Removed one obsolete terminal-only repair-kit display helper after moving consumable page rendering closer to the shared interface model.



## Version 1.35.12
- Continued GUI preparation by centralizing difficulty selection, session selection, activity selection, selected activity confirmation and activity information through shared `MenuScreen` data.
- Coop secondary-account and secondary-character selection now use paginated `MenuScreen` screens instead of raw terminal lists.
- Craft screens now use shared menu data, local page choices, 98/99 navigation, detailed recipe screens and confirmation screens.
- Exchange/donation account, character and action screens now expose `screenId` / `actionId` values for the future graphical renderer.
- `MenuScreen` can now export a `GuiMenuSnapshot`, and `TerminalInterface` can ask for one of the real enabled options of a screen.

## Version 1.35.13
- Character creation race and class choices now use structured MenuScreen screens instead of raw terminal-only lists.
- Class catalog exposes ClassOptionInfo so GUI preparation can read class names, roles and starting numbers without scraping console output.
- Exchange item selection now uses paginated structured menus for weapons, armors, consumables and materials.
- Equipped weapon/armor transfer is blocked directly through disabled menu choices.


## Version 1.35.14
- Added a reusable `MessageScreen` helper for information screens, text input screens and keyword-confirmation screens.
- Local account import, creation, login, export and deletion flows now use more structured shared interface screens.
- Character name input, version warnings, legacy adaptation decisions, ownership refusal, transfer, export, clone and deletion flows now use more structured shared screens.
- Dangerous SUPPRIMER / TRANSFERER confirmations are now closer to the shared interface model, preparing the future graphical renderer.

## Version 1.35.15
- Continued GUI preparation on boss flows: solo/coop appearance choice, manual selection, power analysis and confirmation now use more shared `MenuScreen` data.
- Boss selection now exposes structured options with `screenId` / `actionId` values instead of relying on raw console lists.
- Coop boss support healing now uses shared screens for action, target and potion selection.
- `BossCatalog` exposes registry names and hints without forcing terminal output, preparing the future graphical renderer.

## Version 1.35.16
- Continued GUI preparation on combat outcome screens: PvE encounter choice, escape, victory, defeat and revive screens now use shared screen helpers.
- Reward and partial-reward displays are now rendered through shared interface data instead of hard-coded terminal frames.
- Non-lethal death penalties, lethal corruption and survival anomaly messages are now centralized in reusable screens.
- Several boss result and coop reward headers now use shared message screens, keeping the terminal playable while preparing a graphical renderer.

## Version 1.35.17
- Added a GUI-ready combat state snapshot model for duels, groups, summons, HP, statuses and targetability.
- Reworked more PvP/PvE AI, exploration and special dialogue screens through shared MenuScreen/MessageScreen models.
- Boss ultimate start/damage feedback now goes through shared message screens.

## Version 1.35.19
- Windows installer/launcher pass: ASCII-only Windows scripts to avoid broken console accents.
- Default installation folder is now `Downloads/ProjetDinotofu`; the player can choose another parent folder, but the final folder is always `ProjetDinotofu`.
- Windows launcher does not use WSL; Windows releases must include `Dinotofu.exe`.
- Linux installer/launcher now follows the same `ProjetDinotofu` installation rule.
