# DM2 SKULL.EXE LE object/page/fixup table probe

Result: **PASS**.

This is a conservative provenance/source audit and layout probe for canonical DM2 `SKULL.EXE`. It parses LE object/page/fixup tables and maps object-relative offsets to canonical file offsets. It does **not** claim a protected-mode call graph or DM2 runtime source correspondence.

## Provenance cited

- Canonical README: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/README.md`.
- EN archive `Dungeon-Master-II-Skullkeep_DOS_EN.zip` sha256 `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929`; member `skull.exe` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- CD/layout archive `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` sha256 `a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228`; member `dumast2/SKULL.EXE` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- `SKULL.ASM` sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`; parsed exact labels: 156; max exact label file offset: `0x188D`.
- EN/CD `SKULL.EXE` members identical: `True`.

## MZ/LE layout

- MZ declared image ends at `0x2E0A`; LE header at `0x2E10`.
- LE data pages: `0x16C00`..`0x7F98D` (429453 bytes), page size `0x1000`, module pages 105.
- LE tables: object table rel `0xC4`, page table rel `0xF4`, fixup page table rel `0x2A5`, fixup record table rel `0x44D`.
- Start: object 1 offset `0x52284` => file `0x68E84`.
- Stack: object 2 ESP `0x128F0`; top-minus-one file offset `None`.

| object | virtual size | base | flags | pages | file span | object-relative span |
| ---: | ---: | ---: | ---: | --- | --- | --- |
| 1 | `0x595AC` | `0x10000` | `0x2005` | 1..90 | `0x16C00`..`0x701AC` | `0x0`..`0x595AC` |
| 2 | `0x128F0` | `0x70000` | `0x2003` | 91..105 | `0x70C00`..`0x7F98D` | `0x0`..`0x128F0` |

## Fixup table parse

- Fixup page table absolute offset `0x30B5`.
- Fixup record table absolute offset `0x325D`; bytes 80125.
- Pages with fixups: 99 / 105.
- Parsed records: 11013.
- Parse failures: [].

| page | record-table bytes |
| ---: | ---: |
| 1 | `0x0`..`0x52A` (1322) |
| 2 | `0x52A`..`0x8E7` (957) |
| 3 | `0x8E7`..`0xB87` (672) |
| 4 | `0xB87`..`0xE11` (650) |
| 5 | `0xE11`..`0x1162` (849) |
| 6 | `0x1162`..`0x13B5` (595) |
| 7 | `0x13B5`..`0x15C9` (532) |
| 8 | `0x15C9`..`0x172A` (353) |
| 9 | `0x172A`..`0x188B` (353) |
| 10 | `0x188B`..`0x1A7C` (497) |
| 11 | `0x1A7C`..`0x1C5F` (483) |
| 12 | `0x1C5F`..`0x1CB3` (84) |
| 13 | `0x1CB3`..`0x1EFF` (588) |
| 14 | `0x1EFF`..`0x20DB` (476) |
| 15 | `0x20DB`..`0x2255` (378) |
| 16 | `0x2255`..`0x23B9` (356) |
| 17 | `0x23B9`..`0x287B` (1218) |
| 18 | `0x287B`..`0x2DEA` (1391) |
| 19 | `0x2DEA`..`0x3206` (1052) |
| 20 | `0x3206`..`0x35DA` (980) |
| 21 | `0x35DA`..`0x3A7B` (1185) |
| 22 | `0x3A7B`..`0x3DFB` (896) |
| 23 | `0x3DFB`..`0x420F` (1044) |
| 24 | `0x420F`..`0x4634` (1061) |
| 25 | `0x4634`..`0x494D` (793) |
| 26 | `0x494D`..`0x4DDD` (1168) |
| 27 | `0x4DDD`..`0x5226` (1097) |
| 28 | `0x5226`..`0x54E9` (707) |
| 29 | `0x54E9`..`0x571D` (564) |
| 30 | `0x571D`..`0x5A20` (771) |
| 31 | `0x5A20`..`0x5D68` (840) |
| 32 | `0x5D68`..`0x60B9` (849) |
| 33 | `0x60B9`..`0x62BF` (518) |
| 34 | `0x62BF`..`0x668C` (973) |
| 35 | `0x668C`..`0x69DC` (848) |
| 36 | `0x69DC`..`0x6C67` (651) |
| 37 | `0x6C67`..`0x6F8D` (806) |
| 38 | `0x6F8D`..`0x7429` (1180) |
| 39 | `0x7429`..`0x76F9` (720) |
| 40 | `0x76F9`..`0x7C9D` (1444) |
| 41 | `0x7C9D`..`0x7F79` (732) |
| 42 | `0x7F79`..`0x816D` (500) |
| 43 | `0x816D`..`0x85C6` (1113) |
| 44 | `0x85C6`..`0x8984` (958) |
| 45 | `0x8984`..`0x8CB9` (821) |
| 46 | `0x8CB9`..`0x90B4` (1019) |
| 47 | `0x90B4`..`0x94B9` (1029) |
| 48 | `0x94B9`..`0x97DD` (804) |
| 49 | `0x97DD`..`0x9C05` (1064) |
| 50 | `0x9C05`..`0xA012` (1037) |
| 51 | `0xA012`..`0xA355` (835) |
| 52 | `0xA355`..`0xA630` (731) |
| 53 | `0xA630`..`0xA8C4` (660) |
| 54 | `0xA8C4`..`0xAB90` (716) |
| 55 | `0xAB90`..`0xAD6F` (479) |
| 56 | `0xAD6F`..`0xB020` (689) |
| 57 | `0xB020`..`0xB338` (792) |
| 58 | `0xB338`..`0xB661` (809) |
| 59 | `0xB661`..`0xB865` (516) |
| 60 | `0xB865`..`0xBA77` (530) |
| 61 | `0xBA77`..`0xBB9F` (296) |
| 62 | `0xBB9F`..`0xBDA9` (522) |
| 63 | `0xBDA9`..`0xBFF1` (584) |
| 64 | `0xBFF1`..`0xC10A` (281) |
| 65 | `0xC10A`..`0xC39A` (656) |
| 66 | `0xC39A`..`0xC672` (728) |
| 67 | `0xC672`..`0xC9B8` (838) |
| 68 | `0xC9B8`..`0xCC15` (605) |
| 69 | `0xCC15`..`0xCFE3` (974) |
| 70 | `0xCFE3`..`0xD4D8` (1269) |
| 71 | `0xD4D8`..`0xD7B0` (728) |
| 72 | `0xD7B0`..`0xD87B` (203) |
| 73 | `0xD87B`..`0xDBA9` (814) |
| 74 | `0xDBA9`..`0xDDFC` (595) |
| 75 | `0xDDFC`..`0xE3E9` (1517) |
| 76 | `0xE3E9`..`0xEA12` (1577) |
| 77 | `0xEA12`..`0xEF0C` (1274) |
| 78 | `0xEF0C`..`0xF6B8` (1964) |
| 79 | `0xF6B8`..`0xFB26` (1134) |
| 80 | `0xFB26`..`0xFFDB` (1205) |
| 81 | `0xFFDB`..`0x103CB` (1008) |
| 82 | `0x103CB`..`0x1073D` (882) |
| 83 | `0x1073D`..`0x10A08` (715) |
| 84 | `0x10A08`..`0x10C4D` (581) |
| 85 | `0x10C4D`..`0x10F3C` (751) |
| 86 | `0x10F3C`..`0x1147C` (1344) |
| 87 | `0x1147C`..`0x11BD5` (1881) |
| 88 | `0x11BD5`..`0x11FFA` (1061) |
| 89 | `0x11FFA`..`0x12271` (631) |
| 90 | `0x12271`..`0x1236D` (252) |
| 91 | `0x1236D`..`0x1239E` (49) |
| 92 | `0x1239E`..`0x123B3` (21) |
| 93 | `0x123B3`..`0x1241F` (108) |
| 94 | `0x1241F`..`0x12A23` (1540) |
| 95 | `0x12A23`..`0x13487` (2660) |
| 96 | `0x13487`..`0x137F2` (875) |
| 97 | `0x137F2`..`0x13822` (48) |
| 99 | `0x13822`..`0x138AE` (140) |
| 105 | `0x138AE`..`0x138FD` (79) |

## Exact SKULL.ASM label mappings

- none; no exact `SKULL.ASM` labels fall inside the sampled/proven protected-mode object points.

## Conservative conclusion

The LE object and fixup tables are mechanically parseable for the canonical binary. Object-relative offsets can be converted to canonical file offsets through the proven contiguous page map. This pass found no mechanically provable protected-mode `SKULL.ASM` label correspondence beyond exact labels listed above.

## Limitations / unknowns

- No SKULL.ASM labels were found in the protected-mode object file-offset spans unless skull_asm_exact_label_mappings is non-empty; this file is mostly flat db data after the real-mode stub.
- Internal fixup records are parsed mechanically for this canonical LE layout; imported/non-internal forms would be reported as unsupported instead of guessed.
- Object-relative to file-offset mappings depend on the contiguous canonical page map proven here and are not runtime linear addresses after relocation/loading.
- This probe does not disassemble the protected-mode payload, emulate it, or infer a call graph.
- SKULL.ASM is treated as IDA-generated disassembly evidence, not authoritative handwritten DM2 source.
- ReDMCSB source is intentionally not cited for DM2 runtime correspondence in this pass.
