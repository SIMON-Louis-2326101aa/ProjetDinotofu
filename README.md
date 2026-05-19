# Dinotofu   
   
Dinotofu is a C++17 terminal RPG / arena game.   
   
The project is currently being rebuilt from an old single-file prototype into a cleaner object-oriented architecture. The game still runs entirely in the terminal for now, but the codebase is prepared for future systems such as campaign saves, class evolution, better inventory management, boss mechanics, enemy waves, coop, allied AI, summons, bestiary progression, account statistics, and JSON saves.   
   
## Current status   
   
Dinotofu is still in development.   
   
The current playable combat modes are:   
   
- PvP: player vs player   
- PvP AI: player vs computer-controlled opponent   
- PvE Monsters: enemy wave combat   
- PvE Boss: boss arena combat   
   
The campaign mode is planned but not fully implemented yet.   
   
## Game concept   
   
Dinotofu is designed as a medieval-fantasy terminal RPG with arena-style combat.   
   
The combat flow is inspired by classic turn-based games: the player chooses actions such as attacking, using potions, checking equipment, opening the inventory, passing a turn, or trying to escape.   
   
The game uses randomness, close to a DND feeling, for several actions such as damage rolls, AI choices, escape attempts, future initiative, and special encounters.   
   
Planned long-term systems include:   
   
- character creation and JSON saves   
- campaign progression   
- unlocked monsters and bosses   
- DND-like attributes: Strength, Dexterity, Constitution, Intelligence, Wisdom, Charisma   
- class evolution based on stats   
- damage types and resistances   
- weapons, armor, materials, durability, special effects, heroic and relic equipment   
- bestiary and material knowledge progression   
- account-wide statistics across all characters   
- Lethal difficulty with permanent character death   
- coop, allied AI, summons, combat groups, and initiative-based turns   
- special semi-human characters with rare appearance chances   
   
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
   
## Make commands   
   
Compile the project:   
   
```bash   
make   
```   
Run the game:   
   
```bash   
make run   
```   
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
make install-desktop   
```   
Remove the Linux desktop launcher:   
   
```bash   
make remove-desktop   
```   
   
## Launching on Linux   
   
Recommended:   
   
```bash   
make run   
```   
Or compile, then run the executable directly:   
   
```bash   
make   
./output/Dinotofu   
```   
   
## Launching on Windows   
   
The Makefile is mainly made for Linux.   
   
Recommended Windows options:   
   
### Option 1: WSL   
   
Install WSL, open the project inside a Linux terminal, then use:   
   
```bash   
make run   
```   
### Option 2: MSYS2 / MinGW   
   
Install a MinGW toolchain and `make`, then run the project from an MSYS2 terminal:   
   
```bash   
make run   
```   
### Option 3: Manual g++ command   
   
If all includes are correctly configured, a manual command can be used, but this is less comfortable than the Makefile:   
   
```bash   
g++ -std=c++17 -Wall -Wextra -Iinclude $(find src -name "*.cpp") -o output/Dinotofu   
```   
This direct command is mostly useful on Linux-like shells. On classic Windows CMD or PowerShell, use WSL or MSYS2 instead.   
   
## Git notes   
   
Generated folders must not be committed:   
   
- `build/`   
- `output/`   
   
The real `Makefile` must stay in Git.   
   
## Code organization   
   
The project uses an `include/` and `src/` architecture.   
   
Headers are placed in `include/`, implementation files are placed in `src/`, and both folders follow almost the same structure.   
   
The code identifiers and folders are written in English. Player-facing terminal text can remain in French.   
   
The preferred naming is `item/`, not `object/`. The project should not contain an old root `action/` folder anymore. The folder `combat/action/` is kept because it contains real combat action logic.   
   
## Current stabilization pass   
   
Checked and stabilized points:   
   
- `make clean` and `make rebuild` work correctly.   
- `build/` and `output/` are ignored by Git.   
- No old root `action/` folder is required anymore.   
- `combat/action/` is kept because it contains real combat action logic.   
- The four current combat modes use the shared combat menu structure.   
- Equipment and inventory menus can be opened without consuming the combat turn.   
- Failed potion actions do not consume the combat turn.   
- Offensive potions use the real potion power instead of a fixed hardcoded value.   
- Monster wave target selection matches the displayed enemy numbers.   
- Boss escape attempts correctly consume the turn while keeping escape impossible.   
- Future save/progression folders are prepared for JSON-based systems.   
- Monster PvE waves are generated from the player level.   
- Monster PvE rewards now track defeated enemies and escaped enemies separately.   
- Player escape in Monster PvE gives 50% of defeated enemy rewards, to avoid being too punitive after resource consumption.   
- Player escape in Monster PvE also gives 25% of the reward value of damaged enemies still alive.   
- Enemy escape in Monster PvE gives 75% of that enemy reward when the combat ends correctly.   
- Future difficulty rules are prepared: Easy can go up to 90% reward recovery, Normal stays balanced, Hard and Lethal are harsher.   
- Future non-lethal death penalties are prepared: gold, experience, consumables, and later materials can be partially lost, while weapons, armors, unique items, and relics stay protected.   
   
## Future systems prepared   
   
The project now contains placeholder folders and files for future systems:   
   
- `character/`: special semi-human characters and random character generation   
- `combat/encounter/`: rare special encounters and encounter generation   
- `combat/group/`: future coop, allied AI, summons, unit slots, turn order, and initiative   
- `progression/`: statistics, bestiary, material knowledge, difficulty mode, identity reveal states, and death penalties   
- `save/`: future JSON save manager and save data models   
- `interface/menu/progression/`: future menus for statistics, bestiary, materials, and account data   
- `assets/saves/accounts/`: future account JSON saves   
- `assets/saves/characters/playable/`: future playable character JSON saves   
- `assets/saves/characters/dead/`: future dead Lethal characters   
- `assets/saves/bestiary/`: future bestiary knowledge saves   
- `assets/saves/materials/`: future material knowledge saves   
   
## Future difficulty and death penalty notes   
   
Difficulty reward recovery should not be all-or-nothing. Easy mode can be generous, but even there 100% recovery is too high. The current prepared idea is closer to this:   
   
```text   
Easy: around 90% reward recovery on PvE escape   
Normal: around 50% reward recovery on defeated enemies, plus 25% on damaged living enemies   
Hard: harsher recovery   
Lethal: very harsh recovery or permanent consequences   
```   
   
In non-lethal difficulty, death should still hurt. The future system is prepared to remove a coherent part of what the player carried or gained:   
   
```text   
10% to 20% of gold, consumables, materials, or current experience can be lost.   
Weapons, armors, very rare items, unique items, heroic items, and relics should not randomly disappear.   
```   
   
## Future Lethal difficulty note   
   
In normal difficulty, character death statistics will display how many times the character has died.   
   
In Lethal difficulty, death statistics should become corrupted instead of showing a normal counter:   
   
```text   
Morts du personnage : [STATISTIQUE CORROMPUE]   
Vous ne deviez pas mourir.   
Statut : personnage supprimé du registre des vivants.   
Connais-tu quelqu’un capable d’échapper à la mort ? Moi non...   
```   
   
Even before dying, the basic statistics display in Lethal difficulty should not show a normal death counter. It should show a corrupted warning instead:   
   
```text   
Morts du personnage : [STATISTIQUE CORROMPUE]   
But de mission : survivre.   
```   
   
## Boss knowledge rule   
   
Meeting a boss is not enough to fully identify it.   
   
A boss name should only be added to the bestiary when the boss says its own name, when a reliable character names it, when the boss reaches a narrative threshold such as 50% health, or when a final line before defeat reveals enough information.   
   
Before that, the bestiary can show unknown, physical descriptions, clues, or suspected identities based on writings and legends already discovered by the player.   
   

## Prepared future systems   
   
This version intentionally contains many future folders and placeholder files. They are not all active yet. Each placeholder file includes:   
   
```text   
- the standard Dinotofu ownership comment   
- a short file description   
- a TODO explaining that the system will be implemented later   
```   
   
Prepared systems include:   
   
```text   
- JSON saves and registries   
- playable/dead character registry   
- account statistics   
- character statistics   
- death records and lethal-mode corruption   
- bestiary knowledge progression   
- boss identity reveal progression   
- material knowledge and experimentation progression   
- special semi-human encounters   
- random character/name generation   
- combat groups, units, slots and initiative   
- summons   
- loot tables   
- future adventure, world and lore systems   
- item rarity, ammunition, durability, repair and equipment materials   
```   
   
## Current architecture   
   
```text   
Dinotofu/   
├── Makefile   
├── README.md   
├── .gitignore   
│   
├── assets/   
│   ├── config/   
│   │   ├── difficulties/   
│   │   ├── encounters/   
│   │   ├── loot/   
│   │   └── pantheon/   
│   ├── saves/   
│   │   ├── accounts/   
│   │   ├── bestiary/   
│   │   ├── campaign/   
│   │   ├── characters/   
│   │   │   ├── graveyard/   
│   │   │   └── playable/   
│   │   ├── encounters/   
│   │   ├── lore/   
│   │   ├── materials/   
│   │   ├── player/   
│   │   └── statistics/   
│   └── texts/   
│       ├── bosses/   
│       ├── descriptions/   
│       ├── dialogues/   
│       ├── lore/   
│       └── special_characters/   
│   
├── include/   
│   ├── adventure/   
│   ├── boss/   
│   ├── character/   
│   ├── class_system/   
│   ├── combat/   
│   │   ├── action/   
│   │   ├── ai/   
│   │   ├── boss/   
│   │   ├── death/   
│   │   ├── encounter/   
│   │   ├── escape/   
│   │   ├── group/   
│   │   ├── initiative/   
│   │   ├── loot/   
│   │   ├── modes/   
│   │   ├── reward/   
│   │   ├── summon/   
│   │   ├── system/   
│   │   ├── turn/   
│   │   ├── unit/   
│   │   └── wave/   
│   ├── core/   
│   ├── economy/   
│   ├── effect/   
│   ├── entity/   
│   ├── interface/   
│   │   └── menu/   
│   │       ├── equipment/   
│   │       ├── inventory/   
│   │       ├── potions/   
│   │       ├── progression/   
│   │       └── save/   
│   ├── item/   
│   │   ├── ammunition/   
│   │   ├── armor/   
│   │   ├── consumable/   
│   │   ├── durability/   
│   │   ├── effect/   
│   │   ├── material/   
│   │   ├── rarity/   
│   │   └── weapon/   
│   ├── lore/   
│   ├── progression/   
│   │   ├── bestiary/   
│   │   ├── death/   
│   │   ├── difficulty/   
│   │   ├── material/   
│   │   └── statistics/   
│   ├── save/   
│   │   ├── json/   
│   │   └── registry/   
│   ├── story/   
│   ├── utils/   
│   └── world/   
│   
└── src/   
    └── same structure as include/, with .cpp implementation files   
```   
   
## Developer note   
   
This project is personal and experimental. The goal is to keep the fun prototype spirit while progressively making the code cleaner, easier to maintain, and ready for bigger systems.   
   
## Note Spéciale   
   
Toi qui lis ça, oui, c'est bien moi, le dev, qui te parle. Ce jeu est encore en chantier, il a sûrement deux-trois boulons qui tremblent, des monstres qui font les malins, et des boss qui pensent vraiment être les personnages principaux. Mais justement : viens tester.   
   
Ton objectif est simple : survivre, comprendre les menus, trouver les meilleures décisions, et surtout prouver que tu n'es pas juste là pour te faire plier par un gobelin random. Si tu perds contre Matt, je ne juge pas... enfin si, un peu quand même.   
   
Défi numéro un : gagner un combat sans paniquer dans l'inventaire comme si tu cherchais tes clés un lundi matin.   
   
Défi numéro deux : battre un boss sans dire que le jeu triche. Spoiler : parfois il triche peut-être, mais avec style.   
   
Défi numéro trois : trouver une stratégie qui casse le jeu, me la montrer, et me laisser faire semblant que c'était prévu depuis le début.   
   
Ce projet existe surtout pour être essayé, critiqué, vanné, amélioré, puis retesté. Donc joue sérieusement, mais pas trop. Râle si tu veux, propose des idées, insulte poliment les boss, et surtout dis-moi ce qui te donne envie de continuer.   
   
Si tu arrives à devenir une légende dans Dinotofu, bravo. Si tu meurs contre les premiers ennemis, bravo aussi, mais différemment.   
   
Allez, entre dans l'arène. On va voir si tu joues vraiment bien, ou si tu parles juste fort sur Discord.   
   
Je tiens à noter que lors de la vraie bêta ouverte, tu auras une chance d'être directement intégré au jeu de base si ta contribution le mérite. Alors prouve-moi que tu en vaux le coup. Bon courage d'ici là.   

## Latest development notes   
   
This version adds a first real difficulty flow. The player now chooses difficulty after entering the name and before choosing the class. This matters because the starter kit can now depend on the difficulty: starter gold, potion counts, and starter equipment durability can change.   
   
Prepared difficulty levels are:   
   
- Easy   
- Normal   
- Hard   
- Nightmare   
- Lethal   
   
Monster PvE escape rewards can now use difficulty-dependent percentages. Easy is more generous, Normal stays balanced, and higher difficulties become harsher. Damaged enemies still alive can still give partial reward value, so escaping after doing most of the work does not feel like a complete punishment.   
   
Non-lethal death penalties now include prepared logic for gold loss, experience loss, consumable loss, equipped weapon durability loss, equipped armor durability loss, break chances, irreparable destruction chances, and future equipment theft conditions. Theft is only meant to be enabled later when the enemy logically has a reason to steal equipment.   
   
Lethal mode keeps its corrupted death identity. A future rare narrative exception is prepared through the survival anomaly idea: surviving at 1 HP after a special blessing-like event, losing blessings and suffering brutal consequences instead of a normal death.   
   
A second project planning file is available: `SYSTEMES_PREVUS.txt`. It contains the longer French roadmap for future systems, death rules, bestiary, statistics, materials, special semi-human characters, JSON saves, and pantheon-linked bosses.   

## Prepared Systems: Special Characters and Cheat Codes   

This version also prepares the next Dinotofu systems without fully activating them yet.   
The goal is to keep the project compiling while making the future architecture visible.   

Prepared special-character systems:   
- protected special identities such as Hazak, Aoi, Trexof, Skuro, Sanctus, Hestia, Fire Flight and Louis;   
- future special-date validation using `DD/MM/YYYY`;   
- native class bonus markers for protected characters;   
- random special-character encounters separated from normal monsters.   

Prepared cheat systems:   
- altered-character state after cheat activation;   
- first-cheat warning before altering a character;   
- reusable and single-use cheat codes;   
- creator secret codes and hidden combat input sequence;   
- future action refund system with limited uses per character.   

A full French design summary is available in `SPECIAL_CHARACTERS_AND_CHEATS.txt`.   

## Prepared local account and save system   

The game now prepares an `assets/saves/` directory automatically.   
At startup, the player can select an existing local account or create/use a new one. If the account name is left empty, the `local` account is used.   
When an existing account is selected, the player can now choose `Log in`, `Delete`, or `Back`. Deleting an account also deletes every linked character save.   
After the account step, the player can select an existing playable character for that account or create a new one.   
When an existing character is selected, the player can now choose `Play as`, `Delete`, or `Back`.   
A JSON snapshot of the character is saved after character creation and after a combat session.   
The current loader restores the main character identity and progression data: name, race, class, difficulty, level, experience, HP, gold, equipped items, weapons, armors, consumables and stackable materials/information items.   

This system prepares the foundation for future account saves, character saves, dead character archives, bestiary progress, material progress, statistics, blessings and cheat states.   

## Current combat targeting note   

A first threat system now exists in `combat/threat`.   
When a character heals, enemies can mark that healer as a priority target for the next attack.   
Tank-like characters can also create provocation, forcing hostile attention back onto them instead of letting enemies freely attack summons.   
Sanctus currently has stronger passive provocation hooks because his identity is built around protection, restraint, and the future Sanctus/Skuro split.   

This is still a first pass: the future version should use full combat slots, better enemy intelligence, active tank skills, healer roles, and boss-specific resistance to provocation.   

## Recent update: threat, healing, and combat slots   

The threat system now separates self-healing from ally-healing.   
Healing yourself does not increase healer threat anymore.   
Healing an ally can later make intelligent enemies prioritize the healer.   

A first combat slot foundation is also ready for future group fights: player, allies, enemies, summons, and bosses.   

## Recent update: visible combat groups and role actions   
   
This version adds a first real bridge between the old combat system and the future slot-based group combat system.   
PvP AI and PvE now display visible combat groups built from the player, enemies and active summons.   
A new `CombatGroupBuilder` prepares the future target selection system without breaking the old menus.   
   
A new `CombatRoleActionSystem` also prepares active role behaviours, especially tank provocation and future threat reduction.   
Special character effects are now also applied when an entity attacks a summon, so summons interact better with characters like Skuro, Louis, Hestia or Fire Flight.   
   
Next logical steps: migrate target selection to combat slots, add a real skill/role menu, create ally-healing actions, then continue toward full inventory saves and real cheat activation.   

## Recent update: role menu and slot target selection   

The combat interface now includes a first active role menu. Tanks can manually use provocation, assassins can reduce their immediate threat, and future ally protection/healing actions are already visible as locked options.   

Target selection in duels against summoned enemies now starts using `CombatGroup` and `CombatUnitSlot`, making the project closer to the future group-combat structure.   

Next steps: migrate PvE wave targeting to slots, add true allies, connect ally healing/protection, then continue with inventory saving, real cheat activation, and the global bestiary.   

## Combat interface

The main combat menu keeps `0 : Interface`.   
This option is used to understand the current situation without necessarily performing an offensive action.   

It can display character stats, quick equipment summary, combat state, role skills, target observation, bestiary lookup, future ally orders and future summon control.   

The `Equipment` menu stays separate because it is meant to manage gear: detailed view, weapon swap, outfit swap, and later more equipment actions.   

## Planned Shops

Shops are now planned before the full global bestiary.  
They will be able to refresh after each fight, even outside story mode.  
Planned shop types include monster materials, materials, plants, armor, weapons, consumables and library knowledge.  
The library will later sell common information and magic knowledge, preparing the future bestiary system.  

## Recent update: hidden altered data system   

Cheat codes are no longer shown openly to a clean character.   
Outside combat, the player can enter a hidden command instead of a numeric menu choice.   
If the command is recognized, the first alteration warning appears and the character becomes permanently `Altered` if the player accepts.   

Once the character is altered, the post-combat menu reveals `6 : Données altérées`.   
This menu lists known alterations, their effects, their active/inactive state, and lets the player enter another command.   
Toggle-based alterations can be entered again to disable them, then entered again to enable them.   
The character remains altered forever, even if every alteration is disabled.   

The post-combat menu now also has:   
- `0 : Continuer`, returning to game mode selection;   
- `4 : Sauvegarde rapide`, saving without leaving;   
- `5 : Sauvegarder et quitter`, saving then ending the current session.   

## Recent update: special dates and persistent bestiary   

Protected special identities can now be validated during character creation with a special date using the `DD/MM/YYYY` format. Matt (PRO) remains non-playable. Other protected characters ask for confirmation and then require the correct special date. When the identity is recognized, the character race is locked by story identity, but the class can still be selected.   

The runtime bestiary is now also saved inside the character snapshot: encounters, kills and bought information can be restored after loading. This is not the final full bestiary yet, but the persistent base is connected.   


## Update - Attribute Progression   

- Level up now grants 2 attribute points.   
- The post-combat menu now includes an attribute menu.   
- Attributes are DND-inspired: Strength, Dexterity, Constitution, Intelligence, Wisdom and Charisma.   
- Strength, Dexterity and Constitution already give small direct combat benefits.   
- Intelligence, Wisdom and Charisma are saved and prepared for future magic, summons, dialogue, pact and class evolution systems.   
- Attribute values and unused points are saved and restored with the character.   
