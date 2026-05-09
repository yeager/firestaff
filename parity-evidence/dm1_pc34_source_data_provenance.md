# DM1 PC 3.4 source-data provenance

Date: 2026-04-27

## Verdict

Firestaff V1's current DM1 PC 3.4 source-data assumptions match the worker-VM
local canonical archive and local extracted PC34 data set for the three runtime
data files currently used as provenance anchors.
No rendering-code correction was needed.

## Source paths checked

Worker-VM local canonical archive:

- `~/.openclaw/data/firestaff-original-games/DM/Game,Dungeon_Master,DOS,Software.7z`
- Archive SHA-256: `c0d4aa0b8b592605d745993c071abe042092098eed21155fa573f0cf59c048e0`
- Extracted local check directory during validation: temporary `DungeonMasterPC34/DATA/` below `firestaff-pc34-provenance-*`.

Worker-VM local extracted set:

- `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/`

Deprecated reference note:

- Earlier audit text referenced <private-host> / `<deprecated-remote-host>`; that source is deprecated for worker subagents and must not be used. This note and helper now use only the worker-VM local original game data paths above.

## Locked PC34 data-file anchors

| File | Bytes | SHA-256 | Existing Firestaff assumption |
|---|---:|---|---|
| `DUNGEON.DAT` | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | M10/M11 real dungeon probes consume this file by path, no conflicting hash found. |
| `GRAPHICS.DAT` | 363417 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` | Matches `asset_status_m12.c` / `asset_validator_checksums_m12.json` MD5 `fa6b1aa29e191418713bf2cda93d962e` for `dm1 pc34-en`; also the SHA-256 cited in `DM1_ALL_GRAPHICS_FIX_PLAN.md`. |
| `SONG.DAT` | 162482 | `71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177` | Matches the DM PC `SONG.DAT` entry in `asset_validator_checksums_m12.json` by file identity and runtime audio assumptions. |

Additional local cross-check:

- `DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT` has the same SHA-256 as English `DUNGEON.DAT`.
- `DungeonMasterPC34Multilingual/EUDATA/SONG.DAT` has the same SHA-256 as English `SONG.DAT`.
- `DungeonMasterPC34Multilingual/EUDATA/GRAPHICS.DAT` is different, as expected, and matches the Firestaff MD5 assumption for `dm1 pc34-multi` (`f934d97e43e1ba6e5159839acbcd0611`).

## Pass445 hash-lock evidence

This pass promotes the PC34 dungeon/graphics source identity into a CTest gate:
`pass445_dm1_v1_pc34_data_hash_lock`.

| Filename | Variant | Bytes | SHA-256 | MD5 cross-ref |
|---|---|---:|---|---|
| `GRAPHICS.DAT` | DM PC 3.4 English / I34E | 363417 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` | Daniel registry: `FA6B1AA29E191418713BF2CDA93D962E` |
| `DUNGEON.DAT` | DM PC 3.4 English / I34E | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | n/a |
| `GRAPHICS.DAT` | DM PC 3.4 Multilanguage / EUDATA cross-reference | 398925 | `291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc` | Daniel registry: `F934D97E43E1BA6E5159839ACBCD0611` |
| `DUNGEON.DAT` | DM PC 3.4 Multilanguage / EUDATA cross-reference; same payload as English | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | n/a |

ReDMCSB audit anchors used by the gate:

- `FILENAME.C`: `DATA\\DUNGEON.DAT`, `DATA\\GRAPHICS.DAT`, `EUDATA\\DUNGEON~.DAT`, `EUDATA\\GRAPHICS.DAT`.
- `MEMORY.C`: `F0477_MEMORY_OpenGraphicsDat_CPSDF`, `F0770_FILE_Open(G2130_GraphicsDatFileName)`, `F0479_MEMORY_ReadGraphicsDatHeader`.
- `CEDTINCA.C`: `F7063_LoadDungeon` reads dungeon parts via `F7059_ReadDungeonPartWithChecksum`.

## Repro command

Use the committed helper:

```sh
./tools/validate_dm1_pc34_provenance.py
```

It extracts the local `.7z` with `7zz`/`7z`, hashes `DUNGEON.DAT`,
`GRAPHICS.DAT`, and `SONG.DAT`, then hashes the worker-VM local extracted data
set. It performs no SSH or <private-host> access.
