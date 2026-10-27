# DM2 SKULL.EXE / SKULL.ASM offset probe

Result: **PASS**

## Provenance/source lock cited

- Canonical README: `_canonical/dm2/README.md` records `Dungeon-Master-II-Skullkeep_DOS_EN.zip`, `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip`, and `SKULL.ASM` SHA256 anchors.
- Runtime archive member: `Dungeon-Master-II-Skullkeep_DOS_EN.zip::skull.exe` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- Matching runtime archive member: `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip::dumast2/SKULL.EXE` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- Source/disassembly archive member: `Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z::SKULL.ASM` sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`; canonical extracted `SKULL.ASM` matches archive member: `True`.

## MZ header facts

- Signature: `MZ`; header bytes: `96`; declared MZ file bytes: `11786`; actual file bytes: `522637`; overlay bytes after declared image: `510851`.
- Entry CS:IP `0000:04F6` => file offset `0x556` when using header + CS*16 + IP.

## Limited line/offset mappings

### Header/db bytes

| SKULL.EXE offset | SKULL.ASM line | ASM byte | EXE byte | ok |
| ---: | ---: | ---: | ---: | --- |
| `0x0` | 78 | `4D` | `4D` | `True` |
| `0x1` | 79 | `5A` | `5A` | `True` |
| `0x18` | 102 | `40` | `40` | `True` |
| `0x19` | 103 | `00` | `00` | `True` |

### First decoded proc labels

| proc | proc line | file offset | checked instruction lines | ok |
| --- | ---: | ---: | --- | --- |
| `sub_100_11B` | 333 | `0x11B` | 334, 335, 336, 337, 338, 339 | `True` |
| `sub_100_169` | 397 | `0x169` | 398, 399 | `True` |
| `sub_100_187` | 430 | `0x187` | 431, 432, 433, 434, 435, 436 | `True` |
| `sub_100_205` | 514 | `0x205` | 515, 516, 517, 518 | `True` |
| `sub_100_51D` | 1230 | `0x51D` | 1231, 1232 | `True` |
| `sub_100_7E7` | 1951 | `0x7E7` | 1952, 1953 | `True` |
| `sub_100_832` | 2020 | `0x832` | 2021 | `True` |
| `sub_100_8A0` | 2122 | `0x8A0` | 2123, 2124, 2125, 2126 | `True` |

## Limitations

- Offsets are SKULL.EXE file offsets because SKULL.ASM includes the MZ bytes beginning at seg000 line 78; this is not a full segmented runtime address map.
- The MZ header declares a 11786-byte DOS load image; the 522637-byte file has a large overlay/packed payload after that image.
- The probe does not unpack, emulate, or map protected-mode/DOS extender/overlay code; it checks only header/db bytes and a few mechanically encoded 16-bit real-mode proc starts.
- SKULL.ASM is IDA-generated disassembly evidence, not authoritative hand-written source.
- Entry file offset 0x556 is derived from the MZ header (header 0x60 + CS:IP 0000:04F6); no SKULL.ASM line is claimed for that offset in this narrow probe because decoded instruction spans interrupt simple db-line counting before that point.
