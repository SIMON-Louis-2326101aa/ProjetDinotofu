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
   
## Future systems prepared   
   
The project now contains placeholder folders and files for future systems:   
   
- `character/`: special semi-human characters and random character generation   
- `combat/encounter/`: rare special encounters and encounter generation   
- `combat/group/`: future coop, allied AI, summons, unit slots, turn order, and initiative   
- `progression/`: statistics, bestiary, material knowledge, difficulty mode, and identity reveal states   
- `save/`: future JSON save manager and save data models   
- `interface/menu/progression/`: future menus for statistics, bestiary, materials, and account data   
- `assets/saves/accounts/`: future account JSON saves   
- `assets/saves/characters/playable/`: future playable character JSON saves   
- `assets/saves/characters/graveyard/`: future dead Lethal characters   
- `assets/saves/bestiary/`: future bestiary knowledge saves   
- `assets/saves/materials/`: future material knowledge saves   
   
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
   
## Current architecture   
   
```text   
Dinotofu/   
├── Makefile   
├── README.md   
├── .gitignore   
│   
├── assets/   
│   ├── config/   
│   ├── saves/   
│   │   ├── accounts/   
│   │   ├── bestiary/   
│   │   ├── campaign/   
│   │   ├── characters/   
│   │   │   ├── graveyard/   
│   │   │   └── playable/   
│   │   ├── materials/   
│   │   └── player/   
│   └── texts/   
│   
├── include/   
│   ├── boss/   
│   ├── character/   
│   ├── class_system/   
│   ├── combat/   
│   │   ├── action/   
│   │   ├── ai/   
│   │   ├── boss/   
│   │   ├── encounter/   
│   │   ├── group/   
│   │   ├── modes/   
│   │   ├── system/   
│   │   └── turn/   
│   ├── core/   
│   ├── economy/   
│   ├── effect/   
│   ├── entity/   
│   ├── interface/   
│   │   └── menu/   
│   │       ├── equipment/   
│   │       ├── inventory/   
│   │       ├── potions/   
│   │       └── progression/   
│   ├── item/   
│   ├── progression/   
│   ├── save/   
│   ├── story/   
│   └── utils/   
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
