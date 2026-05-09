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
| `DUNGEON.DAT` | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | Exact variant: DM PC 3.4 English / I34E. MD5 `766450c940651fc021c92fe5d0d0b3a6`; M10/M11 real dungeon probes consume this file by path, no conflicting English hash found. |
| `GRAPHICS.DAT` | 363417 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` | Exact variant: DM PC 3.4 English / I34E. Matches Daniel registry MD5 `fa6b1aa29e191418713bf2cda93d962e` for `DM PC 3.4 English GRAPHICS.DAT`; MD5 is provenance only, SHA-256 is the gate identity. |
| `SONG.DAT` | 162482 | `71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177` | Matches the DM PC `SONG.DAT` entry in `asset_validator_checksums_m12.json` by file identity and runtime audio assumptions. |

Additional local cross-check:

- `DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT` has the same SHA-256 as English `DUNGEON.DAT`: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` (33357 bytes), MD5 `766450c940651fc021c92fe5d0d0b3a6`.
- `DungeonMasterPC34Multilingual/EUDATA/SONG.DAT` has the same SHA-256 as English `SONG.DAT`.
- `DungeonMasterPC34Multilingual/EUDATA/GRAPHICS.DAT` is different, as expected: SHA-256 `291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc`, 398925 bytes, Daniel registry MD5 `f934d97e43e1ba6e5159839acbcd0611` for `DM PC 3.4 Multilanguage GRAPHICS.DAT`.

Pass446 rule: no Firestaff runtime/parity comparison may identify original dungeon/graphics data by basename alone. Every comparison must name the exact variant and SHA-256; registry MD5s are useful provenance cross-references but are not SHA-256 substitutes.

## Pass445 hash-lock evidence

This pass promotes the PC34 dungeon/graphics source identity into a CTest gate:
`pass445_dm1_v1_pc34_data_hash_lock`.

| Filename | Variant | Bytes | SHA-256 | MD5 cross-ref |
|---|---|---:|---|---|
| `GRAPHICS.DAT` | DM PC 3.4 English / I34E | 363417 | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` | Daniel registry: `FA6B1AA29E191418713BF2CDA93D962E` |
| `DUNGEON.DAT` | DM PC 3.4 English / I34E | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | Local MD5: `766450c940651fc021c92fe5d0d0b3a6` |
| `GRAPHICS.DAT` | DM PC 3.4 Multilanguage / EUDATA cross-reference | 398925 | `291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc` | Daniel registry: `F934D97E43E1BA6E5159839ACBCD0611` |
| `DUNGEON.DAT` | DM PC 3.4 Multilanguage / EUDATA cross-reference; same payload as English | 33357 | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` | Local MD5: `766450c940651fc021c92fe5d0d0b3a6` |

Pass448 adds `pass448_dm1_v1_dungeon_hash_provenance`: DM1 V1 map/start/dimension evidence must first verify the exact `DUNGEON.DAT` identity above, then parse `mapCount=14`, initial party location `0x0861` (`map=0 x=1 y=3 direction=2`), map0 dimensions `18x19`, raw map byte count `12283`, and map0 raw squares `x=1,y=3 -> 0xB0`, `x=1,y=4 -> 0x30`. Local sibling variants `DUNGEONF.DAT` and `DUNGEONG.DAT` are recorded as different hashes and are not accepted for DM1 V1 English comparisons.

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
