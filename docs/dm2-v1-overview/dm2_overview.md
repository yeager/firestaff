# DM2 V1 Overview — Source-Locked

## Sources

- SKULL.ASM (522,128 lines IDA disassembly, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject (github.com/gbsphenx/skproject) HEAD a962896
- Original archive: Dungeon-Master-II-Skullkeep_DOS_EN.zip (sha256: d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929)
- firestaff include/dm2_v1_game.h, include/dm2_v1_outdoor_renderer.h

---

## What is Dungeon Master II: The Legend of Skullkeep?

Dungeon Master II (DM2), released 1993 by FTL Games, is the direct sequel to
Dungeon Master (1987). The game is set in the Skullkeep - a mysterious
fortress guarded by a dragon that protects a legendary artifact of great power.

Developer: FTL Games (Christophe Fontanel)
Platform: DOS (primary), also PC-9821 (Japan), Sega CD

Engine: Completely new engine, NOT derived from DM1/CSB codebase.
- skproject/SKWIN/README.md: SKWIN = PC-9821 version, SKWINDOS = PC-DOS version
- Engine variants (v0/v4/v5) reflect platform ports, not version differences

---

## How DM2 Differs from DM1

| Aspect        | DM1 (1987)           | DM2 (1993)                     |
|---------------|----------------------|--------------------------------|
| Engine        | DM1 engine (FTL)     | New Skullkeep engine            |
| Setting       | Chaos Horde dungeon  | Skullkeep fortress + outdoor    |
| World         | Indoor only          | Indoor + outdoor exploration    |
| GRAPHICS.DAT  | 363 KB               | ~8.6 MB                        |
| DUNGEON.DAT   | 33 KB                | 39 KB                          |
| Creatures     | 42 AI slots          | 64 AI slots                    |
| Weapons       | Melee + thrown       | Melee + ranged (crossbow/gun)  |
| Magic         | 16 spells            | 34 original, 255 custom via GDAT |
| NPCs          | None                 | Companions, shops              |
| Weather       | None                 | Rain, fog, storm               |
| Day/night     | None                 | Time-of-day cycle              |
| Multi-floor   | Single dungeon       | Outdoor + buildings + dungeon   |

---

## DM2 Engine Architecture

DM2 is architecturally distinct from DM1. The SKULL.EXE implements:

1. Outdoor renderer - sky, ground, trees, buildings (dm2_v1_outdoor_renderer.h)
2. Indoor dungeon renderer - first-person perspective similar to DM1
3. Combat system - ranged combat, tech weapons (dm2_v1_combat.h)
4. Tech/Magic hybrid system (dm2_v1_tech_magic.h)
5. Companion system (dm2_v1_companion.h)
6. Save/Load (dm2_v1_save_load.h)

Source: include/dm2_v1_game.h:7 - DM2 has a different engine with outdoor areas, shops, NPCs.

---

## Versions

- DM2 DOS EN/Legend - English DOS release (primary reference)
- DM2 Sega CD - Enhanced audio/visuals, DUNGEON.DAT 37,957 bytes
- skproject v4 - PC-9821 (Japan) version source
- skproject v5 - PC-DOS version source
- DM2GDED - graphics editor tool for DM2 GRAPHICS.DAT format

---

## STATUS: SOURCE-LOCKED
