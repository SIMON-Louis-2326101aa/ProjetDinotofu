# Dinotofu

Dinotofu is a C++17 terminal RPG and arena game built around character progression, tactical combat, exploration, quests, crafting, shops, boss encounters and long-term account/character saves.

The project started as a large single-file prototype and is being rebuilt into a cleaner object-oriented architecture. The game currently stays in the terminal on purpose: the priority is to stabilize the RPG systems first, then prepare DND-inspired attributes and durable skills, then move toward a graphical interface, and only after that build the full story mode.

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

Run with automatic version/build check:

```bash
./run_dinotofu.sh
# or
make launch
```

On Windows, double-click:

```bash
run_dinotofu_windows.bat
```

This launcher calls WSL, checks whether the build version changed, rebuilds when needed, clears the screen, then starts the game.

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

1. Expand durable skills with passive progression, active skills and cooldowns.
2. Continue terminal interface cleanup without hiding useful options.
3. Polish crafting and economy balance.
4. Complete DND-inspired attributes once the surrounding systems are stable.
5. Move toward a graphical interface.
6. Build the real story mode after the graphical interface.

## Personal note

Challenge one: survive long enough to understand what is happening.

Challenge two: beat a boss without saying the game cheats. Spoiler: sometimes it might cheat, but at least it does it with style.

Challenge three: find a strategy that breaks the game, show it to me, and let me pretend it was planned from the beginning.

This project exists to be played, criticized, mocked, improved, and tested again. Play seriously, but not too seriously. Complain if needed, suggest ideas, politely insult the bosses, and most importantly tell me what makes you want to continue.

If you manage to become a legend in Dinotofu, congratulations. If you die against the first enemies, congratulations too, just differently.

Step into the arena. We will see whether you truly play well, or whether you only talk loudly on Discord.

During the real open beta, you may even have a chance to be directly integrated into the base game if your contribution deserves it. Prove that you are worth it. Good luck until then.
