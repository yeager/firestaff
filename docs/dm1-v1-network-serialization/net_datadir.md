# DM1 V1 — Data Directory Structure

## Source-locked
ReDMCSB: MEMORY.C (lines 1230–1283), FILENAME.C, LOADSAVE.C (lines 2337–2373), FLOPPY.C
Firestaff: src/engine/firestaff_asset_pipeline.c, src/engine/firestaff_graphics_dat_reader.c

## Data File Locations (MEMORY.C:1230–1260)

GRAPHICS.DAT search order (Atari ST TOS):
1. "A:\\GRAPHICS.DAT" — root of drive A (game disk)
2. "\\JDATA\\GRAPHICS.DAT"
3. "JDATA\\GRAPHICS.DAT"
4. "Q:\\DATA\\GRAPHICS.DAT"
5. "Q:\\JDATA\\GRAPHICS.DAT"
6. "CDATA\\GRAPHICS.DAT"
7. "CJDATA\\GRAPHICS.DAT"
8. "A:\\GRAPHICS.DAT" (fallback)

Alternative path for EU DATA: "EUDATA\\GRAPHICS.DAT" (FILENAME.C:51)

DUNGEON.DAT search order (LOADSAVE.C:2337–2373):
1. "\\DUNGEON.DAT" — root
2. "\\JDATA\\DUNGEON.DAT"
3. "JDATA\\DUNGEON.DAT"
4. "Q:\\DATA\\DUNGEON.DAT"
5. "Q:\\JDATA\\DUNGEON.DAT"
6. "CDATA\\DUNGEON.DAT"
7. "CJDATA\\DUNGEON.DAT"

## Drive / Path Resolution (SAVEPATH.C, FLOPPY.C)

DM1 uses ~ substitution in file paths. ~ expands to:
- Game disk drive (G0571_puc_GameDiskDriveName) for game data
- Save disk drive (G0572_puc_SavedGameDiskDriveName) for save games

Example: "DF0:DMGAME.DAT" — saved game on floppy drive DF0:
(FILENAME.C, CEDT005.C, CEDT101.C)

## Data Files

GRAPHICS.DAT — contains all visual assets (sprites, tiles, UI elements)
DUNGEON.DAT — contains dungeon map data, thing definitions, text
DMGAME.DAT — saved game file (on floppy: "A:\\DMGAME.DAT" or "B:\\DMGAME.DAT")
  Also: "2:\\DMGAME.DAT" in CedtData (CEDTDATA.C:381) — save slot path
  And: "DF0:DMGAME.DAT" for Amiga (CEDT101.C:5)

## Firestaff Data Layout
data/ — game data directory
saves/ — save game directory (fs_save_slot_path generates saves/dm1_slot0.sav etc.)
assets/ — extracted/processed game assets
assets-v2/ — v2 extended assets

## File Handle Management
FIO1_FILE struct (FIO1.C) — wraps file handle with disk change tracking.
Signature 0x52A3 on open, 0x52A4 on successfully created.
