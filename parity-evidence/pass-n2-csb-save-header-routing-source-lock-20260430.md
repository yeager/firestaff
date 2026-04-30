# N2 CSB save-header/routing source-lock gate — 2026-04-30

Result: PASS — ReDMCSB source lineage for CSB save-header identity and DM-vs-CSB save-file routing is locked.

Scope: evidence gate only. This does **not** claim Firestaff runtime behavior, binary save compatibility, rendering, gameplay, or CSBGAME.DAT sample-save coverage.

## ReDMCSB citations checked

- `DEFS.H:468-498` / `DM_SAVE_HEADER` and `CSB_SAVE_HEADER`: distinct header structures and explicit format IDs `C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER` and `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK`.
- `DEFS.H:519-523` / dungeon ID constants: `C10_DUNGEON_DM`, `C12_DUNGEON_CSB_PRISON`, and `C13_DUNGEON_CSB_GAME` are distinct source constants.
- `DEFS.H:5001-5019` / file ID namespace: load/save IDs separate `DMSAVE.DAT` from `CSBGAME.DAT` (`M741`/`M745` vs `M742`/`M746`).
- `CEDTINCU.C:18-33` / `F7272_IsDungeonValid`: selects the DM or CSB save-header layout by `SaveHeaderFormat`; for CSB it casts to `CSB_SAVE_HEADER` and reads `DungeonID`, `Platform`, `FormatID`, and `Useless` from that layout.
- `CEDTINCU.C:36-72` / `F7272_IsDungeonValid`: criteria 2 accepts `C13_DUNGEON_CSB_GAME`; criteria 3 accepts `C12_DUNGEON_CSB_PRISON` or `C10_DUNGEON_DM`, not CSB game.
- `CEDTINCH.C:38-45` / `F7086_IsReadyToMakeNewAdventure`: New Adventure creation uses `F7272_IsDungeonValid(..., 3)` before allowing unique champion names.
- `CEDTINCH.C:49-64` / `F1996_`: SU1E-specific gate requires `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK` and `C13_DUNGEON_CSB_GAME`.
- `CEDTINC8.C:101-118` / save-file selection block: source routes `C10_DUNGEON_DM` to `M745_FILE_ID_SAVE_DMSAVE_DAT`; routes `C13_DUNGEON_CSB_GAME` or `C12_DUNGEON_CSB_PRISON` to `M746_FILE_ID_SAVE_CSBGAME_DAT`; rejects unknown dungeon IDs with `-5`.
- `LOADSAVE.C:2873-2920` / `F0435_STARTEND_LoadGame` saved-game block: recovers `DungeonID` from save header and documents that the old fallback detector does not match actual CSB (`11` maps, seed `13`).

## Verification

Commands run on N2 (`firestaff-worker`) in `/home/trv2/work/firestaff`:

```sh
python3 -m py_compile tools/verify_csb_save_header_routing_source_lock.py
./tools/verify_csb_save_header_routing_source_lock.py
python3 -m json.tool parity-evidence/verification/csb_save_header_routing_source_lock.json >/dev/null
```

Output:

```text
PASS csb save-header routing source lock: 9 source checks
```

JSON output: `parity-evidence/verification/csb_save_header_routing_source_lock.json`

## Non-claims / blocker carried forward

- No Firestaff runtime code changed.
- No local CSBGAME.DAT sample save was found or required for this source-lineage gate.
- This is a source-lock gate only; runtime save compatibility remains a separate follow-up that needs curated CSB save samples or an isolated generated-save harness.
- No DANNESBURK, external capture, or non-N2 reference was used.
