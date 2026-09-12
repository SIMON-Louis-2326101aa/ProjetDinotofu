# Dinotofu   

Dinotofu is a C++17 RPG / arena game. The terminal version remains the stable foundation, while an experimental HTML/JavaScript graphical interface is being developed for a more comfortable desktop experience.   

## Current version   

- Current version: **V3.49.92**   
- Strongly recommended character recreation baseline: **V3.00.00**   
- Characters created before **V3.00.00** may still work, but recreating them is recommended to use the story foundation cleanly.   

## What is already in the game   

The game already lets players:   

- create, load, and save accounts and characters;   
- choose a race, an exact race-compatible age, visual presentation, final variant, class, difficulty, death rule, and certain protected special identities;   
- fight in AI PvP, two-player PvP, monster PvE, solo boss fights, and cooperative boss fights, with recruited allies, real two-ally combo techniques, and persistent enemy rivals that can return after surviving;   
- manage inventory, weapons, armor, consumables, materials, durability, repairs, and item quality; weapons and armor now have persistent individual identities so specific copies can accumulate their own memories;   
- buy, sell and barter through one unified **Boutiques et comptoirs** menu with quick categories or complete list, visit locations, use the guild, library, field observation post, mercenary counter, training stand, and purchase/upgrade independent municipal vaults per town;   
- explore biomes containing resources, chests, traps, tracks, events, dangerous locations, fused slimes, and a four-stage dividing slime mini-boss;   
- follow guild, client, exploration, combat, delivery, service, bestiary, and main-story quests;   
- accept up to three daily guild challenges, earn Challenge Marks, and unlock titles tied to real combat restrictions;   
- consult three special permanent registers: Bestiary for beings, Encyclopedia for systems/items/areas, and Discovery Notebook for lore, legends, and rumours;   
- use hidden systems such as cheat codes, altered data, special characters, legendary merchants, and selected lore events;   
- launch a story mode built as a progressively restricted guided sandbox; the current playable development scope intentionally ends after the Chapter 3 introduction, followed by a temporary development-end message while world content and systems are expanded.   

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

## Special developer note   

Hello, future adventurer, future hero, future statistically probable corpse.   

Welcome to Dinotofu. Here, you can win with a real strategy, die because you tried to look clever, or discover that a tax-motivated goblin is sometimes more dangerous than a mythical boss. The game is still under construction, but it already exists to be played, broken, criticized, trolled, improved, and tested again.   

I want Dinotofu to remain free: you can optimize, try stupid ideas, die like a legend, or survive when you clearly did not deserve it. When a mechanic frustrates you, a boss feels unfair, a strategy destroys the balance, or an idea makes you want to continue the adventure, say it. The game should grow with player feedback, not only with ideas developed alone in a corner.   

Challenge number one: survive long enough to understand what is happening.   

Challenge number two: defeat a boss without saying the game cheats. Spoiler: sometimes it may cheat, but at least it does it with style.   

Challenge number three: find a strategy that breaks the game, show it to me, and let me pretend it was planned from the beginning.   

Play seriously, but not too seriously. Complain when needed, suggest ideas, insult bosses politely, and above all tell me what makes you want to launch another run. When you find an overpowered combination, I will probably deny panicking at the numbers, then claim it was an advanced balance test and everything was planned. That is false, but it sounds more professional.   

When you become a legend in Dinotofu, congratulations. When you die to the first enemies, congratulations too, but differently. When you lose to a chest, a trap, or your own confidence, remember that the game does not judge you. I do a little, but affectionately.   

## Release notes   

Detailed version history is kept in one dedicated file:   

- Detailed version history: `CHANGELOG.md`.   

README files and design documents are not development logs. They contain only useful player and project information; use `CHANGELOG.md` for version history.   
