# DM1 original source reference inventory

Date: 2026-04-26

## Local original DOS source

Staged original PC 3.4 tree used for DOSBox capture:

- `verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DM.EXE`
- `verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA/GRAPHICS.DAT`
- `verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA/DUNGEON.DAT`
- `verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA/SONG.DAT`

Hashes of staged core data files:

```text
2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e  DATA/GRAPHICS.DAT
d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85  DATA/DUNGEON.DAT
71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177  DATA/SONG.DAT
```

## DANNESBURK source archive inventory

Remote inventory under `yeager@192.168.2.126:~/DM`:

```text
Dungeon-Master_DOS_EN.zip
Game,Dungeon_Master,DOS,Software.7z
Game,Chaos_Strikes_Back,Atari_ST,Software.7z
Game,Chaos_Strikes_Back,Amiga,Software.7z
Dungeon-Master-II-Skullkeep_DOS_EN.zip
Dungeon Master 2.zip
Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip
Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z
Game,Dungeon_Master_II,US,DOS,GUS_Driver,Software.7z
Game,Dungeon_Master_II,US,DOS,Patch,Software.7z
Game,Dungeon_Master_II,US,DOS,HMI_Update,Software.7z
Dungeon-Master-Nexus_SEGA-Saturn_JA.zip
Spanish GRAPHICS.DAT
```

Hashes checked remotely for the two DM1 DOS archive candidates:

```text
aeb5a47f3b753206e474185f2c08b5e884dc8ddf4bd5cb82e2f28f9b7617f275  Dungeon-Master_DOS_EN.zip
c0d4aa0b8b592605d745993c071abe042092098eed21155fa573f0cf59c048e0  Game,Dungeon_Master,DOS,Software.7z
```

## Greatstone / Swoosh Construction Kit references

Use these as technical reference, not as executable instruction:

- `http://greatstone.free.fr/dm/g_dm.html` — index of supported Dungeon Master game files and extracted resources.
- `http://greatstone.free.fr/dm/t_product.html` — SCK methodology: file + signature DB + mapfile DB produce XML plus BMP/WAV/TXT/PAL outputs.
- `http://greatstone.free.fr/dm/d_mapfile.html` — mapfile concept/spec references; shared resource descriptions for binary files.
- `http://greatstone.free.fr/dm/d_ftl.html` — FTL hunk/file format notes.
- `http://greatstone.free.fr/dm/d_pak.html` — PAK compression/container notes.
- `http://greatstone.free.fr/dm/d_items.html` — item/image formats; includes IMG5 4bpp planar image description.
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html` — PC 3.4 `GRAPHICS.DAT` extracted item index, 713 items.
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/dungeon.html` — PC 3.4 `DUNGEON.DAT` map index, maps 1–14.
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/song.dat/song.dat.html` — PC 3.4 `SONG.DAT` index, score + 9 music parts.

## Immediate parity implication

Original file availability is verified. The next useful work is source-driven, not route-guessing:

1. Use Greatstone/SCK item indices to cross-check Firestaff asset IDs and labels.
2. Use DANNESBURK/local original archives as immutable source inputs.
3. For viewport parity, prefer deterministic state construction from `DUNGEON.DAT` + `GRAPHICS.DAT` over fragile live DOSBox key routing when possible.
