# Dinotofu   

Dinotofu is a C++17 terminal RPG / arena game.   

The project is currently being rebuilt from an old single-file prototype into a cleaner object-oriented architecture. The game still runs entirely in the terminal for now, but the codebase is prepared for future systems such as campaign saves, class evolution, better inventory management, boss mechanics, enemy waves, and later coop / allied AI / summons.   

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

The game uses randomness, close to a DND feeling, for several actions such as damage rolls, turn order, AI choices, and escape attempts.   

Planned long-term systems include:   

- character creation and saves   
- campaign progression   
- unlocked monsters and bosses   
- DND-like attributes: Strength, Dexterity, Constitution, Intelligence, Wisdom, Charisma   
- class evolution based on stats   
- damage types and resistances   
- weapons, armor, materials, durability, special effects, heroic and relic equipment   
- coop, allied AI, summons, and initiative-based group turns   

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

The code identifiers are now mostly written in English. Player-facing terminal text can remain in French.   

## Current architecture   

```text   
Dinotofu/   
├── Makefile   
├── README.md   
├── .gitignore   
│   
├── assets/   
│   ├── config/   
│   ├── sauvegardes/   
│   └── textes/   
│   
├── include/   
│   ├── boss/   
│   │   └── BossCatalog.hpp   
│   │   
│   ├── class_system/   
│   │   ├── ClassCatalog.hpp   
│   │   ├── ClassEvolution.hpp   
│   │   └── PlayerClass.hpp   
│   │   
│   ├── combat/   
│   │   ├── ai/   
│   │   │   ├── AIAction.hpp   
│   │   │   └── CombatAI.hpp   
│   │   ├── action/   
│   │   │   ├── CombatAttack.hpp   
│   │   │   ├── CombatPotion.hpp   
│   │   │   └── SpecialCombatEffects.hpp   
│   │   ├── boss/   
│   │   │   ├── BossDecryption.hpp   
│   │   │   ├── BossEndTurn.hpp   
│   │   │   └── BossUltimate.hpp   
│   │   ├── modes/   
│   │   │   ├── boss/   
│   │   │   │   └── BossPveMode.hpp   
│   │   │   ├── pve/   
│   │   │   │   └── MonsterPveMode.hpp   
│   │   │   └── pvp/   
│   │   │       ├── AiPvpMode.hpp   
│   │   │       └── PvpMode.hpp   
│   │   ├── system/   
│   │   │   ├── CombatClassSystem.hpp   
│   │   │   ├── DamageSystem.hpp   
│   │   │   ├── EscapeSystem.hpp   
│   │   │   ├── ObservationSystem.hpp   
│   │   │   └── WaveCombatSystem.hpp   
│   │   ├── turn/   
│   │   │   ├── wave/   
│   │   │   │   ├── MonsterWaveCombatTurn.hpp   
│   │   │   │   └── PlayerWaveCombatTurn.hpp   
│   │   │   ├── AICombatTurn.hpp   
│   │   │   ├── BossCombatTurn.hpp   
│   │   │   └── HumanCombatTurn.hpp   
│   │   ├── BossCombat.hpp   
│   │   ├── Combat.hpp   
│   │   ├── CombatActions.hpp   
│   │   ├── CombatTurn.hpp   
│   │   ├── DamageReport.hpp   
│   │   ├── EnemyCombatQueue.hpp   
│   │   └── TurnManager.hpp   
│   │   
│   ├── core/   
│   │   ├── Config.hpp   
│   │   ├── Console.hpp   
│   │   ├── Game.hpp   
│   │   ├── GameMode.hpp   
│   │   └── Random.hpp   
│   │   
│   ├── economy/   
│   │   ├── Money.hpp   
│   │   └── Shop.hpp   
│   │   
│   ├── effect/   
│   │   ├── Bleeding.hpp   
│   │   ├── Corrosion.hpp   
│   │   ├── DamageBuff.hpp   
│   │   ├── Effect.hpp   
│   │   ├── Immobilization.hpp   
│   │   ├── Regeneration.hpp   
│   │   └── SpecialArmor.hpp   
│   │   
│   ├── entity/   
│   │   ├── Boss.hpp   
│   │   ├── Entity.hpp   
│   │   ├── Monster.hpp   
│   │   ├── MonsterCatalog.hpp   
│   │   ├── NPC.hpp   
│   │   ├── Player.hpp   
│   │   └── Race.hpp   
│   │   
│   ├── interface/   
│   │   ├── menu/   
│   │   │   ├── equipment/   
│   │   │   │   ├── EquipmentComparison.hpp   
│   │   │   │   └── EquipmentDisplay.hpp   
│   │   │   ├── inventory/   
│   │   │   │   ├── InventoryDisplay.hpp   
│   │   │   │   ├── InventorySelection.hpp   
│   │   │   │   └── InventoryUtils.hpp   
│   │   │   ├── potions/   
│   │   │   │   ├── CombatPotionDisplay.hpp   
│   │   │   │   ├── CombatPotionUse.hpp   
│   │   │   │   └── CombatPotionUtils.hpp   
│   │   │   ├── CombatMenu.hpp   
│   │   │   ├── CombatPotionMenu.hpp   
│   │   │   ├── CombatTargetMenu.hpp   
│   │   │   ├── EquipmentMenu.hpp   
│   │   │   └── InventoryMenu.hpp   
│   │   ├── CombatDisplay.hpp   
│   │   ├── GraphicalInterface.hpp   
│   │   ├── Interface.hpp   
│   │   └── TerminalInterface.hpp   
│   │   
│   ├── item/   
│   │   ├── armor/   
│   │   │   ├── Armor.hpp   
│   │   │   ├── ArmorCatalog.hpp   
│   │   │   └── ArmorType.hpp   
│   │   ├── consumable/   
│   │   │   ├── Consumable.hpp   
│   │   │   ├── ConsumableCatalog.hpp   
│   │   │   └── ConsumableType.hpp   
│   │   ├── material/   
│   │   │   ├── Material.hpp   
│   │   │   └── MaterialCatalog.hpp   
│   │   ├── weapon/   
│   │   │   ├── Weapon.hpp   
│   │   │   ├── WeaponCatalog.hpp   
│   │   │   └── WeaponType.hpp   
│   │   ├── Inventory.hpp   
│   │   └── Item.hpp   
│   │   
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
