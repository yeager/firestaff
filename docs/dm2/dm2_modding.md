# DM2 V1 — Moddability

## Overview

Dungeon Master II: The Legend of Skullkeep (DM2) is a **highly moddable** game. The game ships with several data files that have been extensively reverse-engineered by the community, and multiple editors exist for creating custom dungeons, graphics, and gameplay modifications.

## Data Files

DM2 stores its game data in the `data/` subdirectory of the install folder. Key files:

| File | Purpose |
|------|---------|
| `dungeon.dat` | Dungeon map data (walls, floors, creatures, items, teleporters, actuators). **The primary modding target.** |
| `graphics.dat` | All sprite/graphic imagery (tiles, creatures, UI elements, portraits) |
| `soundlist.dat` | Maps map numbers to HMP music track numbers. Each byte = HMP track for that map |
| `dmgame.dat` | Per-saved-game runtime state; derived from dungeon.dat at first save |
| `dmsave.dat` | Saved game state (scrambled/encrypted, different key per save) |

**Important:** After the first save, `dmgame.dat` is created from the dungeon.dat that existed at save time. Subsequent edits to dungeon.dat only affect **new games** — not already-saved games.

## File Format Compatibility

The dungeon.dat format is **very similar to Dungeon Master (DM) and Chaos Strikes Back (CSB)** — so much so that the CSBuild tool can open DM2's dungeon.dat. This deep structural similarity enabled the editor ecosystem.

## Editors

### DMDC2 (Dungeon Master II Dungeon Creator)
- The primary map/dungeon editor for DM2 PC
- Can edit dungeon.dat (maps, creatures, items, teleporters, actuators)
- Supports 3072 actuators (extended from original limits)
- Handles dungeon expansion, tile placement, creature spawning
- Notes: Edits to dungeon.dat only appear in new games; existing saves use the dungeon.dat that existed when the save was created

### DM2GDED (Dungeon Master II Graphics Editor)
- Graphics editing: tile sprites, creature images, UI elements, portraits
- Can edit `graphics.dat` imagery
- Can change color palettes (HP/Stamina/Mana bar colors are in the palette, not in graphics.dat)
- Known issue: importing replacement images sometimes loses transparency on non-first images (grey square artifacts)
- Rev.5 can edit creature AI definitions and spell re-definitions

### DMute
- Originally for DM/CSB, but also applicable to DM2
- Can create new dungeon.dat files from scratch

## What Can Be Modified

### Dungeon/Maps
- Tile placement (walls, floors, pits, doors, decorations)
- Creature spawn locations (but creatures not native to an area will die/disappear there)
- Items placement
- Teleporters and ACTUators (actuators handle events: cross-scene transitions, chest opening, Torham revival, etc.)
- **Adding entirely new maps is complex** — the editor can expand existing maps but adding new map numbers (beyond the ~43 existing) is not straightforward. Scene transitions are done by combining teleporters with actuators ("Cross scene" type with an Ornate "Gate")
- Ladders/exits between maps: tied to depth parameters; expanding a map can shift depth references

### Graphics
- Replace any sprite in graphics.dat (8-bit indexed color format)
- Custom champion portraits (Amiga version: replace `Torham.iff` etc. in IFF format; Mac: rename `Ending.Moov`/`Credits.Moov` to `Title.Moov`)
- UI color customization (palette editing via DM2GDED)

### Audio
- `soundlist.dat`: each byte maps a map number to an HMP music track; edit in hex editor to reassign music

### Gameplay
- Shop item lists (via DM2GDED: creature subtree `[22 MERCHANT]`, `[43 BAD MERCHANT]`, `[44 MAGIC MERCHANT]`, Text field like "J6 J8 J56 J26-28 C1")
- Spell re-definition (DM2GDED Rev.5)
- Creature AI re-definition (DM2GDED Rev.5)
- Torham removal/modification: Torham is revived by a resurrection-type actuator at upper-left of Map#0 — remove or modify it to play without Torham

## Community Tools & Resources

- **CSBuild**: open-source tool that can read DM2's dungeon.dat (format similarity to DM/CSB)
- **DMDC2/DM2GDED** bundle (2012-07-30 version): extended features for custom DM2 clones
- **RTC Editor**: built into the Return to Chaos (RTC) reimplementation — visual dungeon editor for DM, DM2, and CSB
- `dmweb.free.fr` — primary community resource (Encyclopedia, editors, forum)
- `dungeon-master.com` forum — editor help, format documentation

## Modding Notes & Quirks

1. **New game required for dungeon.dat changes**: dungeon.dat edits only show up in new games
2. **Depth parameters**: each map has a depth value; expanding maps can misalign depth-based connections (ladders, pits)
3. **ACTUators**: complex event system; cross-scene transitions combine a teleporter destination with an actuator marked "Cross scene" with Ornate "Gate"
4. **Creature permissions**: creatures placed outside their native areas will die/disappear; flying creatures (Attack Minions) and some others can be placed more freely
5. **Transparency in imported graphics**: DM2GDED sometimes loses transparency on non-first imported images — the imported image appears with a grey square instead of transparent background
6. **Palette conflicts**: HP/Stamina/Mana bar colors share palette entries with menu text and cursor colors — changing one affects others

## Source Reference

- SKULL.ASM — IDA-disassembled SKULL.EXE entry stub (522,128 lines), available at canonical path on N2
- skproject source: https://github.com/gbsphenx/skproject (secondary reference)
