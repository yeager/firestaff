# N2 CSB sample-save search blocker — 2026-04-30

Result: BLOCKED for runtime CSBGAME.DAT compatibility probing — no extracted/curated `CSBGAME*.DAT` or `CSBGAME*.BAK` saved-game sample is present under the approved N2 local original-game roots.

Scope: evidence/blocker inventory only. No Firestaff runtime behavior, binary save compatibility, rendering, gameplay, or generated-save harness behavior is claimed.

## ReDMCSB source-lock compatibility criteria

A usable CSB runtime sample for this lane must satisfy the ReDMCSB source contract, not merely contain the text `CSBGAME.DAT` inside an executable or disk image:

- `DEFS.H:482-498` / `CSB_SAVE_HEADER` and save-header format constants: CSB saves use the CSB header layout and `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK`.
- `DEFS.H:519-523` / dungeon ID constants: CSB game saves must identify `C13_DUNGEON_CSB_GAME`; CSB prison uses `C12_DUNGEON_CSB_PRISON`; DM uses `C10_DUNGEON_DM`.
- `CEDTINCU.C:18-33` / `F7272_IsDungeonValid`: validation reads `DungeonID`, `Platform`, `FormatID`, and `Useless` through the CSB save-header layout when `SaveHeaderFormat` is CSB.
- `CEDTINCU.C:49-58` / `F7272_IsDungeonValid` criteria 2: accepts only `C13_DUNGEON_CSB_GAME` for the CSB game path, plus the media-specific platform gate compiled for that target.
- `CEDTINCH.C:55-58` / `F1996_`: SU1E gate requires `C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK`, `FormatID >= C2_FORMAT_DM_AMIGA_2X_PC98_X68000_FM_TOWNS_CSB_ATARI_ST`, and `DungeonID == C13_DUNGEON_CSB_GAME`.
- `CEDTINC8.C:101-118` / save-file selection block: `C13_DUNGEON_CSB_GAME` and `C12_DUNGEON_CSB_PRISON` route to `M746_FILE_ID_SAVE_CSBGAME_DAT`; `C10_DUNGEON_DM` routes to `M745_FILE_ID_SAVE_DMSAVE_DAT`; unknown dungeon IDs reject with `-5`.
- `LOADSAVE.C:2873-2920` / `F0435_STARTEND_LoadGame` saved-game block: runtime recovers the saved-game `DungeonID` from the save header; the old fallback detector is explicitly documented as not matching actual CSB (`11` maps, seed `13`).

Therefore a compatible sample must be an extracted saved-game file such as `CSBGAME.DAT`, `CSBGAMEF.DAT`, `CSBGAMEG.DAT`, or numbered/backup variants, with a decodable CSB header meeting the gates above. Plain string references inside game/utility disks are not enough.

## N2-local search result

Approved roots searched:

- `~/.openclaw/data/firestaff-original-games/DM/`
- `~/.openclaw/data/firestaff-original-games/DM/_extracted/`

Exact filesystem saved-game candidates:

- No extracted file named `CSBGAME*.DAT` or `CSBGAME*.BAK` was found.
- One save-named disk image exists: `~/.openclaw/data/firestaff-original-games/DM/_extracted/csb-atari/Floppy Disks MSA/Chaos Strikes Back for Atari ST Save Disk.msa` (`730035` bytes). This is a disk image, not an extracted curated `CSBGAME*.DAT` sample.

Archive/string evidence found but not accepted as runtime samples:

- `Dungeon Master 2.zip`, `_canonical/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip`, and `Dungeon-Master-II-Skullkeep_DOS_EN.zip` contain `SKSAVE*.DAT/BAK`; these are DM2/Skullkeep saves, not CSB `CSBGAME*.DAT` samples.
- CSB Atari/Amiga `.msa`, `.st`, and `.adf` images contain strings such as `CSBGAME.DAT`, `CSBGAME.BAK`, `DF0:CSBGAME.DAT`, and localized `CSBGAMEF/G.DAT`. Those strings prove executable/disk routing names exist, not that an extracted saved-game sample is available.
- ReDMCSB also has `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Reference/Original/P31J/CSBGAME` (`sha256 3c7c1cbe86af08dd23d628ac2e23c956369f96c357055949ace75b58de57889b`), but `file` identifies it as an `MS-DOS executable, MZ for MS-DOS, LZEXE v0.91 compressed`; it is not a saved-game sample.

## Verification

Commands run on N2 (`Firestaff-Worker-VM`) in `/home/trv2/work/firestaff`:

```sh
python3 -m py_compile tools/verify_csb_sample_save_search_blocker.py
./tools/verify_csb_sample_save_search_blocker.py
python3 -m json.tool parity-evidence/verification/csb_sample_save_search_blocker.json >/dev/null
```

Output:

```text
PASS csb sample-save inventory: blocker recorded; no exact extracted CSBGAME*.DAT/BAK sample present
wrote parity-evidence/verification/csb_sample_save_search_blocker.json
```

JSON output: `parity-evidence/verification/csb_sample_save_search_blocker.json`

## Non-claims / follow-up

- No deprecated remote source, external capture, network reference, or non-N2 sample was used.
- No Firestaff runtime code was changed.
- Runtime CSB save compatibility remains blocked until a curated extracted `CSBGAME*.DAT/BAK` sample is added under an approved N2-local reference path or an isolated generated-save harness produces one with recorded provenance.
