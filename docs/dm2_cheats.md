# DM2 V1 — Cheats & Debug Modes

## Overview

DM2 does not have an official debug mode or developer console, but the community has discovered numerous cheat codes, exploit-based hacks, and trick mechanics that serve as informal cheats. These range from in-game exploits (duplication bugs, item cloning) to patched EXE modifications (unlimited mana).

**No INT 3 / debug trap found in SKULL.EXE** — the only `int 3` in the disassembly is at offset line 175 (the entry stub's abort/breakpoint handler), not a debug feature.

## In-Game Exploits (No Patching Required)

### Item/Champion Cloning
- **Stealing items from champion mirrors**: Click a champion's chamber → click item → press Space bar. The game may hang if you steal the same item multiple times or put them on the floor.
- **Cloning champions**: Click champion's chamber → press Space bar twice. This adds the champion to your party without emptying their sleeping chamber. Repeat for additional clones. **Warning**: Do not drop the same cloned item from two cloned champions on the same square — the game hangs.
- **Invisible items in champion mirrors**: Right-click a champion portrait after cloning. You see an empty inventory with weird invisible items (Green face, Eye of Time, Spiral Staff, Cape, Cloak of Night, Ra Sar Shield, Vi Potion, Bones, Gold coin, Ya Key, Dead Thorn Demon). The Dead Thorn Demon, when pointed at Torham's eye, shows an image of a Thorn Demon; dropping it on the floor gives steaks (like a killed Thorn Demon). This can be repeated for infinite steaks — but never drop two Dead Thorn Demons on the same square.

### Money Duplication
- **Money box duplication** (PC version only):
  1. Start new game, open Equus's inventory
  2. Pick up money box with left click, press Space bar → goes to Torham's inventory
  3. Put money box in Torham's empty hand, exit inventory
  4. Open Torham's inventory, open the money box, put all of Torham's money inside
  5. Check Equus's money box (eye icon) — changes in Torham's box are reflected in Equus's box (they are linked)
  6. Save game, restart, resume — this severs the link between the two money boxes
  7. Repeat steps 2–5 (put duplicate in Torham's dagger hand instead)
  8. Move all money from old box to new box → each round doubles your money
  9. Final save/restart to unlink the boxes

Note: Duplicating armor or weapons can corrupt saved games, making them unusable.

## EXE Patch Cheats

### Free Mana (PC)
Open SKULL.EXE with a hex editor. Find the byte sequence:
```
01 02 03 04 05 06 02 03 04 05 06 07 04 05 06 07 07 09 02 02 03 04 06 07
```
Replace all those bytes with `00`. Result: mana never decreases when casting spells.

## Cheat Codes / Secrets (Platform-Specific)

### Macintosh
- **Designer portrait Easter egg**: Name your champions after the designers — Wayne Holder, Doug Bell, Bill Kelly, Kirk Baker, Nancy Holder — and their faces replace your champions' faces in the game.

### Amiga
- **Custom champion portraits**: The game contains a file named `sample_palette.IFF`. Following the instructions in that file (16-color palette, 32x29 pixel portraits), you can create custom portraits for any champion. Save as `Torham.iff` (or any champion name) in the same folder as `sample_palette.IFF`.

### PC/Mac/Sega CD
No traditional cheat-code entry system (like pressing keys on the title screen) is known to exist in DM2.

## Watching Cutscenes Without Playing

- **PC**: Rename `FTL` to `FTL.BAK`, rename `END` to `FTL` to play the ending animation at the title screen. To replace the intro with credits: rename `INTRO` to `INTRO.BAK`, rename `CREDITS` to `INTRO`.
- **Amiga**: Rename `SWSH.DAT` to `SWSH.BAK`, rename `ENDA.DAT` to `SWSH.DAT`.
- **Macintosh**: Rename `Title.Moov` to `TitleBak.Moov`. Then rename `Ending.Moov` or `Credits.Moov` to `Title.Moov`.

## Hacks & Modified Dungeons

### Play Without Torham
Community members have created modified `dungeon.dat` files that remove or relocate Torham so the player starts without him. These are available from community shared drives.

### Super Characters Saved Games
Pre-made saved games with maxed-out champion stats are available for both PC and Amiga versions. These are not cheat codes per se, but saved game hacks.

## Game Mechanics Exploits

- **Accelerate training**: Stand in a hallway and repeat an action (slash/bash for fighters, weapon throwing for ninjas, spell casting in the air for wizards) to gain experience quickly without combat risk.
- **See Through Walls**: At the dungeon entrance, face the door, cast OH EW RA.
- **Magic Footprints**: YA BRO ROS — reveals invisible creatures' movement paths.
- **Avoiding moving pits and teleporters**: Use Scout Map to invoke a minion and tell it to go to your destination, then stay in the same square as the minion. The engine prevents two creatures occupying the same square, so the minion blocks the invisible creature that drives the moving effect. Alternatively, use a Magical Box to Freeze life — this stops both the invisible creature and its pits/teleporters.
- **Guard Minion farming**: Place Guard Minions around creature spawn squares (not on the spawn square itself) — each generated creature is immediately killed by the minions, and items accumulate. Works for Thicket Thief, Axemen, and Cavern Bat spawn points.

## No Formal Debug Mode

DM2 has no documented debug menu, developer console, or cheat code entry screen. All "cheats" are either in-game exploits, hex-edited EXE patches, or saved game/saved data modifications. The SKULL.EXE disassembly contains no debug hook (single `int 3` at entry stub is a breakpoint trap, not a feature).
