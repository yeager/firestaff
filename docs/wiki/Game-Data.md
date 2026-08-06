# Game Data

> **Status reviewed 2026-08-06.** Original data is user-supplied, hash-gated
> and never included in Firestaff. The scanner accepts the documented loose
> files and supported archive/disc containers; a recognized file is not by
> itself proof that every later game route is playable.

## Overview

Firestaff requires original game data files to run each game. These files are **not included** in the release and must be obtained from your own copies of the original games. Game data files must never be committed to the repository.

## Required Files by Game

### Dungeon Master (DM1) — PC 3.4

| File | Description |
|------|-------------|
| `GRAPHICS.DAT` | PC 3.4 graphics data (IMG3 format) |
| `DUNGEON.DAT` | PC 3.4 dungeon data |

Files are identified by content hash, not filename. The recursive scanner will find them in any directory structure.

### Chaos Strikes Back (CSB) — PC 3.4

| File | Description |
|------|-------------|
| `GRAPHICS.DAT` | CSB graphics data (**IMG1 nibble-RLE format**, not PC IMG3) |
| `DUNGEON.DAT` | CSB dungeon data |

**Important**: CSB's `GRAPHICS.DAT` uses the Amiga v3.1 IMG1 nibble-RLE container format, not the PC IMG3 format used by DM1. Do not substitute DM1 graphics data for CSB or vice versa.

### Dungeon Master II: Skullkeep (DM2) — PC

| File | Description |
|------|-------------|
| `GRAPHICS.DAT` | DM2 GDAT graphics data |
| `DUNGEON.DAT` | DM2 G1 dungeon data (39,437 bytes for the PC version) |

### Theron's Quest — PC Engine CD

| File | Description |
|------|-------------|
| Track 02 BIN | Raw CD-ROM Track 02 binary (JP or US variant) |

Theron's Quest uses record-based CD access, not an ISO filesystem. Firestaff
requires the authenticated Track 02 raw media path from a CUE/BIN disc image.
JP and US variants are supported with different stage-two record offsets (JP:
0x04df, US: 0x04e0). ISO files can assist inspection, but do not replace the
Track 02 handoff required for launch.

### DM Nexus — Sega Saturn

| File | Description |
|------|-------------|
| `LEV00.DGN` – `LEV15.DGN` | Level geometry files |
| `SN_FLOOR.MNS` | Static floor/ceiling material |
| `SN_WALL.MNS` | Static wall material |
| `MENU.BPK` | Menu assets (PRS3 compressed) |
| `TITLE.CG` | Title screen |
| `SNDLEV00.SAL` – `SNDLEV15.SAL` | Level audio |
| `SNDLEV00.MAP` – `SNDLEV15.MAP` | Audio map selectors |
| `SLEV00.BIN` – `SLEV15.BIN` | Level scripts |
| `FACE.BIN` | Champion portrait data |
| `STABG.BIN` | Status background |

## Data Directory Layout

Place game data in the platform-appropriate directory. The default data directory can be changed in Settings within the launcher.

### macOS / Linux

```
~/.firestaff/data/
```

### Windows

```
%USERPROFILE%\.firestaff\data\
```

### iOS

```
(App Sandbox)/Documents/Firestaff/data/
```

Transfer files using the **Files** app (On My iPhone/iPad > Firestaff) or iTunes File Sharing.

### Android

```
/sdcard/Documents/Firestaff/data/
```

Transfer files using a file manager app or USB connection.

### Directory Layout (all platforms)

```
data/
  dm1/data/
    GRAPHICS.DAT
    DUNGEON.DAT
  csb/data/
    GRAPHICS.DAT
    DUNGEON.DAT
  dm2/data/
    GRAPHICS.DAT
    DUNGEON.DAT
  theron/
    track02-us.bin
    track02-jp.bin
  nexus/
    LEV00.DGN
    ...
    SN_FLOOR.MNS
    SN_WALL.MNS
    MENU.BPK
    TITLE.CG
    SNDLEV00.SAL
    ...
```

On first launch, if no game data is found, Firestaff shows a popup explaining where to place files and how to change the data directory. A progress meter is displayed during scanning.

## Environment Variables

For tests and probes that need game data, set the appropriate environment variable:

| Variable | Game | Example |
|----------|------|---------|
| `FIRESTAFF_DM1_DATA_DIR` | DM1 | `$HOME/.firestaff/data/dm1/data` |
| `FIRESTAFF_CSB_DATA_DIR` | CSB | `$HOME/.firestaff/data/csb/data` |
| `FIRESTAFF_DM2_DATA_DIR` | DM2 | `$HOME/.firestaff/data/dm2/data` |
| `FIRESTAFF_THERON_TRACK02_US_BIN` | Theron's Quest (US) | `/path/to/us-track02.bin` |
| `FIRESTAFF_THERON_TRACK02_JP_BIN` | Theron's Quest (JP) | `/path/to/jp-track02.bin` |
| `FIRESTAFF_NEXUS_DATA_DIR` | DM Nexus | `$HOME/.firestaff/data/nexus` |

## Data Verification

Firestaff verifies all game data by content hash before use. If a file's hash does not match a known original, it is rejected. This ensures only authentic, unmodified game data is used — no synthetic or reconstructed data is accepted.

## Legal Note

Original game data files are copyrighted by their respective rights holders. Firestaff does not distribute any original game data. You must supply your own legally obtained copies.
