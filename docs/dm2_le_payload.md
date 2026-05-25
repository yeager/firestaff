# DM2 SKULL.EXE DOS/4G LE payload boundary probe

This is a source/provenance-locked layout probe for canonical DM2 `SKULL.EXE`. It starts from the canonical N2 DM2 anchors and records what can be mechanically located in the DOS extender executable. It does **not** claim a runtime call graph or Firestaff source correspondence.

Result: PASS.

## Provenance identities

- EN archive: `Dungeon-Master-II-Skullkeep_DOS_EN.zip` sha256 `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929`; member `skull.exe` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- CD/layout archive: `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` sha256 `a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228`; member `dumast2/SKULL.EXE` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- `SKULL.ASM`: sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`.
- Canonical README cited: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/README.md`.
- EN/CD `SKULL.EXE` members identical: `True`.

## MZ stub and DOS extender markers

- File bytes: 522637.
- MZ declared image: `0x2E0A` (11786 bytes); header bytes `0x60`; entry CS:IP `0000:04F6` => file offset `0x556`.
- Bytes after declared MZ image: 510851.
- LE header offset: `0x2E10`; padding between declared MZ end and LE header: 6 bytes.

Markers found:
- `DOS4G`: 0x2A52, 0x2A71
- `WATCOM`: 0x559, 0x5AF, 0x68E86, 0x68EB7
- `LE`: 0x2E10
- `LX`: 0x64576

## LE protected-mode payload boundaries

- LE pages: 105; page size `0x1000`; last page bytes `0xD8D`.
- Object table: rel `0xC4`, count 2.
- Object page map: rel `0xF4`; contiguous 1-based page map: `True`.
- Data pages start: `0x16C00`; page payload end: `0x7F98D`.
- Start object/EIP: object 1, EIP `0x52284` => canonical file offset `0x68E84` for this layout.

| object | virtual size | base | flags | page index | pages |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `0x595AC` | `0x10000` | `0x2005` | 1 | 90 |
| 2 | `0x128F0` | `0x70000` | `0x2003` | 91 | 15 |

| range | start | end | bytes | sha256 |
| --- | ---: | ---: | ---: | --- |
| `mz_declared_image` | `0x0` | `0x2E0A` | 11786 | `` |
| `mz_overlay_containing_le` | `0x2E0A` | `0x7F98D` | 510851 | `` |
| `le_header_and_tables` | `0x2E10` | `0x16C00` | 81392 | `` |
| `le_page_payload` | `0x16C00` | `0x7F98D` | 429453 | `b89d5c60f11cb92c32dc983e4b6db55308d4ba914fd2511dcabc7361b5b51e34` |
| `le_image_from_header_to_eof` | `0x2E10` | `0x7F98D` | 510845 | `20ae85e3fdb9da0080c3bbed54b92b7a9f3ad718dc3f7b8a6e09c58442c61477` |
| `extra_overlay_after_le_page_payload` | `0x7F98D` | `0x7F98D` | 0 | `` |

## Conclusion

The protected-mode payload is mechanically locatable: canonical `SKULL.EXE` has an MZ stub followed by a Linear Executable (`LE`) image at `0x2E10`, and the LE page payload spans `0x16C00`..`0x7F98D`. The computed LE page payload ends exactly at EOF, so this pass finds no additional overlay bytes after the LE payload.

## Limitations

- This locates and bounds the LE protected-mode payload; it does not disassemble, decompress, or emulate it.
- No runtime call graph or Firestaff source correspondence is claimed from these ranges.
- SKULL.ASM remains IDA-generated disassembly evidence, not authoritative handwritten source.
- The conversion of LE object-relative addresses to file offsets is only recorded for the canonical binary layout proven here.
