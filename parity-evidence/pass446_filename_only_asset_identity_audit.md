# Pass446 — filename-only original asset identity audit

Date: 2026-05-09

## Verdict

Filename-only assumptions fixed or gated. Firestaff may cite original filenames as loader paths because ReDMCSB does, but runtime/parity comparisons must identify every dungeon/graphics input by exact media variant and SHA-256. Daniel's graphics registry MD5 values are MD5 provenance, not SHA-256 substitute.

## Locked DM1 PC34 identities

| Variant | File | Bytes | SHA-256 | MD5 provenance |
|---|---|---:|---|---|
| DM PC 3.4 English / I34E | `GRAPHICS.DAT` | 363417 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` | `fa6b1aa29e191418713bf2cda93d962e` (`DM PC 3.4 English GRAPHICS.DAT`) |
| DM PC 3.4 English / I34E | `DUNGEON.DAT` | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | n/a |
| DM PC 3.4 Multilanguage / EUDATA | `GRAPHICS.DAT` | 398925 | `291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc` | `f934d97e43e1ba6e5159839acbcd0611` (`DM PC 3.4 Multilanguage GRAPHICS.DAT`) |
| DM PC 3.4 Multilanguage / EUDATA; same dungeon payload as English | `DUNGEON.DAT` | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | n/a |

## ReDMCSB loader/decode anchors

- `FILENAME.C:6` defines I34E `G1059_pc_DungeonFileName = "DATA\\DUNGEON.DAT"`.
- `FILENAME.C:9` defines I34E `G2130_GraphicsDatFileName = "DATA\\GRAPHICS.DAT"`.
- `FILENAME.C:45` defines I34M `G2175_ac_DungeonFileName[] = "EUDATA\\DUNGEON~.DAT"`.
- `FILENAME.C:51` defines I34M `G2130_GraphicsDatFileName = "EUDATA\\GRAPHICS.DAT"`.
- `MEMORY.C:1212` starts `F0477_MEMORY_OpenGraphicsDat_CPSDF`; `MEMORY.C:1266` and `MEMORY.C:1269` open `G2130_GraphicsDatFileName`; `MEMORY.C:1330` starts `F0479_MEMORY_ReadGraphicsDatHeader`.
- `CEDTINCA.C:5` starts `F7063_LoadDungeon`; `CEDTINCA.C:18` reads the dungeon header with checksum; `CEDTINCA.C:110` reads raw map data with checksum.
- `DM.C:462`, `DM.C:474`, `DM.C:556`, `DM.C:564`, and `DM.C:590` cover VGA/EGA detection and DOS file-open/existence helpers (`F8076_IsVGAGraphicsDetected`, `F8077_IsEGAGraphicsDetected`, `F8082_ExitBecauseFileNotFound`, `F8083_DOS_OpenFile`, `F8085_FileExists`).

These source lines explain historical loader filenames. They do not prove asset identity without hashing the local payload.

## Unsafe assumptions fixed or gated

- `docs/plans/M12_PLAN.md` no longer claims arbitrary renamed files work today. It now states current M12 startup status tries bounded expected filenames and then validates MD5; hash-first recursive discovery remains design intent.
- `docs/plans/M12_PLAN.md` replaces placeholder DM1 PC34 MD5/size values with exact I34E SHA-256 identities and labels MD5 as provenance where present.
- `docs/design/CSB_V1_BOOTSTRAP_SCOUT.md` now says CSB catalogue hashes are status evidence only and that CSB runtime identity needs exact variant + SHA-256 for both graphics and dungeon/runtime data.
- `parity-evidence/dm1_pc34_source_data_provenance.md` now records the DM PC 3.4 Multilanguage GRAPHICS.DAT SHA-256 and repeats the pass446 basename rule.
- `tools/verify_pass446_filename_only_asset_identity_audit.py` gates the above docs, local hashes, ReDMCSB anchors, and CMake registration.

## Gate

Run:

```sh
python3 tools/verify_pass446_filename_only_asset_identity_audit.py
ctest --test-dir build -R 'pass44[56]_dm1_v1_pc34_data_hash_lock|pass446_filename_only_asset_identity_audit' --output-on-failure
```

Evidence JSON: `parity-evidence/verification/pass446_filename_only_asset_identity_audit.json`.
