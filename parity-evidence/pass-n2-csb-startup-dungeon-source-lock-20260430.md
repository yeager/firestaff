# N2 CSB startup/dungeon source-lock gate — 2026-04-30

Result: PASS — ReDMCSB source sequence and canonical local CSB Atari `DUNGEON.DAT` identity verified.

Scope: evidence gate only. This does **not** claim CSB runtime, rendering, gameplay, or savegame compatibility parity.

## ReDMCSB citations checked

- `STARTUP2.C:1303-1410` / `F0463_START_InitializeGame_CPSADEF`: initializes memory, reads `GRAPHICS.DAT` header, initializes graphics, loads stone floor/wall sets, and initializes text.
- `LOADSAVE.C:1803-1944` / `F0434_STARTEND_IsLoadDungeonSuccessful_CPSC`: distinguishes compressed dungeon headers, records compressed `DungeonID`, decompresses when needed, allocates/reads `DUNGEON_HEADER` with checksum, and seeds initial party map coordinates.
- `LOADSAVE.C:2291-2393` / `F0435_STARTEND_LoadGame` new-game block: starts from `C10_DUNGEON_DM`, requires game disk, opens platform-specific dungeon files including Atari `\\DUNGEON.DAT`, and hard-stops on open failure.
- `LOADSAVE.C:2826-2920` / `F0435_STARTEND_LoadGame` saved-game block: calls the same dungeon loader, recovers CSB dungeon identity from the save header, and documents that the old DM-format fallback does **not** match actual CSB (`11` maps, seed `13`).
- `DEFS.H:519-523` / Dungeon IDs: locks `C10_DUNGEON_DM`, `C12_DUNGEON_CSB_PRISON`, and `C13_DUNGEON_CSB_GAME` as distinct source constants.

## Local N2 original-data anchor

- `_canonical/csb/atari-DUNGEON.DAT`
  - bytes: `2098`
  - sha256: `3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba`
  - note: canonical Atari ST hard-disk extracted CSB dungeon member in `/home/trv2/.openclaw/data/firestaff-original-games/DM/`.

## Verification

- `python3 -m py_compile tools/verify_csb_startup_dungeon_source_lock.py`
- `./tools/verify_csb_startup_dungeon_source_lock.py`
- JSON output: `parity-evidence/verification/csb_startup_dungeon_source_lock.json`

## Non-claims

- No Firestaff runtime code changed.
- No build required.
- No CSB render, gameplay, or save compatibility parity claimed.
- No non-N2 path used.
