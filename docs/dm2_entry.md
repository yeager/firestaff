# DM2 SKULL.EXE LE entry/fixup target disassembly-aware probe

Result: **PASS**.

This pass starts from the existing DM2 provenance/source-lock gate, then decodes a tiny bounded byte window around the protected-mode LE entry and a deterministic sample of fixup target offsets. Unknown opcodes remain byte rows; no emulator guesses or source correspondence are inferred.

## Provenance cited

- Canonical README: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/README.md`.
- EN archive sha256 `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929`; member `skull.exe` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- CD/layout archive sha256 `a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228`; member `dumast2/SKULL.EXE` sha256 `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35`.
- `SKULL.ASM` sha256 `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099`.
- Source/layout gate `firestaff.dm2_skull_exe_le_objects_fixups.v1` pass: `True`.

## LE entry

- Entry object 1 offset `0x52284` => file `0x68E84`.
- LE payload `0x16C00`..`0x7F98D`; fixup records: 11013.

## Decoded windows

### le_entry @ `0x68E84`

- object: 1 object offset `0x52284`; window `0x68E84`..`0x68EA4`.
- bytes: `eb 76 57 41 54 43 4f 4d 20 43 2f 43 2b 2b 33 32 20 52 75 6e 2d 54 69 6d 65 20 73 79 73 74 65 6d`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x68E84` | `eb 76` | `jmp 0x68EFC` |
| `0x68E86` | `57` | `push edi` |
| `0x68E87` | `41` | `db 0x41 (byte-unclassified)` |
| `0x68E88` | `54` | `push esp` |
| `0x68E89` | `43` | `db 0x43 (byte-unclassified)` |
| `0x68E8A` | `4f` | `db 0x4F (byte-unclassified)` |
| `0x68E8B` | `4d` | `db 0x4D (byte-unclassified)` |
| `0x68E8C` | `20` | `db 0x20 (byte-unclassified)` |
| `0x68E8D` | `43` | `db 0x43 (byte-unclassified)` |
| `0x68E8E` | `2f` | `db 0x2F (byte-unclassified)` |
| `0x68E8F` | `43` | `db 0x43 (byte-unclassified)` |
| `0x68E90` | `2b` | `db 0x2B (byte-unclassified)` |
| `0x68E91` | `2b` | `db 0x2B (byte-unclassified)` |
| `0x68E92` | `33` | `db 0x33 (byte-unclassified)` |
| `0x68E93` | `32` | `db 0x32 (byte-unclassified)` |
| `0x68E94` | `20` | `db 0x20 (byte-unclassified)` |

### fixup_target_sample_1 @ `0x710C6`

- object: 2 object offset `0x4C6`; window `0x710BE`..`0x710DE`.
- bytes: `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 00 00`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x710C6` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C7` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C8` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C9` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CA` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CB` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CC` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CD` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CE` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CF` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D0` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D1` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D2` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D3` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D4` | `ff` | `db 0xFF (byte-unclassified)` |
| `0x710D5` | `ff` | `db 0xFF (byte-unclassified)` |

Relocation context:
- record `0x0` source kind `32-bit-offset` source file offsets 0x17752; raw `07 00 52 0b 02 c6 04`
- record `0xE` source kind `32-bit-offset` source file offsets 0x17741; raw `07 00 41 0b 02 c6 04`
- record `0x3F` source kind `32-bit-offset` source file offsets 0x176EF; raw `07 00 ef 0a 02 c6 04`

### fixup_target_sample_2 @ `0x710C4`

- object: 2 object offset `0x4C4`; window `0x710BC`..`0x710DC`.
- bytes: `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00 00 00 00`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x710C4` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C5` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C6` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C7` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C8` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C9` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CA` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CB` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CC` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CD` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CE` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CF` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D0` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D1` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D2` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D3` | `00` | `db 0x0 (byte-unclassified)` |

Relocation context:
- record `0x15` source kind `32-bit-offset` source file offsets 0x17731; raw `07 00 31 0b 02 c4 04`

### fixup_target_sample_3 @ `0x710C8`

- object: 2 object offset `0x4C8`; window `0x710C0`..`0x710E0`.
- bytes: `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 00 00 a0 25`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x710C8` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C9` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CA` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CB` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CC` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CD` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CE` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CF` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D0` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D1` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D2` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D3` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D4` | `ff` | `db 0xFF (byte-unclassified)` |
| `0x710D5` | `ff` | `db 0xFF (byte-unclassified)` |
| `0x710D6` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D7` | `00` | `db 0x0 (byte-unclassified)` |

Relocation context:
- record `0x1C` source kind `32-bit-offset` source file offsets 0x1772C; raw `07 00 2c 0b 02 c8 04`
- record `0x2A` source kind `32-bit-offset` source file offsets 0x17717; raw `07 00 17 0b 02 c8 04`

### fixup_target_sample_4 @ `0x710D6`

- object: 2 object offset `0x4D6`; window `0x710CE`..`0x710EE`.
- bytes: `00 00 00 00 00 00 ff ff 00 00 00 00 00 00 00 00 a0 25 f0 11 00 0b 30 07 90 04 90 02 d0 00 00 00`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x710D6` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D7` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D8` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D9` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710DA` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710DB` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710DC` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710DD` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710DE` | `a0` | `db 0xA0 (byte-unclassified)` |
| `0x710DF` | `25` | `db 0x25 (byte-unclassified)` |
| `0x710E0` | `f0` | `db 0xF0 (byte-unclassified)` |
| `0x710E1` | `11` | `db 0x11 (byte-unclassified)` |
| `0x710E2` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710E3` | `0b` | `db 0xB (byte-unclassified)` |
| `0x710E4` | `30` | `db 0x30 (byte-unclassified)` |
| `0x710E5` | `07` | `db 0x7 (byte-unclassified)` |

Relocation context:
- record `0x31` source kind `32-bit-offset` source file offsets 0x1770A; raw `07 00 0a 0b 02 d6 04`

### fixup_target_sample_5 @ `0x710C2`

- object: 2 object offset `0x4C2`; window `0x710BA`..`0x710DA`.
- bytes: `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00 00`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x710C2` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C3` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C4` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C5` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C6` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C7` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C8` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710C9` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CA` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CB` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CC` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CD` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CE` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710CF` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D0` | `00` | `db 0x0 (byte-unclassified)` |
| `0x710D1` | `00` | `db 0x0 (byte-unclassified)` |

Relocation context:
- record `0x38` source kind `32-bit-offset` source file offsets 0x176FD; raw `07 00 fd 0a 02 c2 04`

### fixup_target_sample_6 @ `0x7110E`

- object: 2 object offset `0x50E`; window `0x71106`..`0x71126`.
- bytes: `00 cb 00 da 00 e9 00 f8 ff 00 00 00 00 00 00 00 00 00 00 00 00 00 2c 00 00 00 34 00 00 00 3c 00`

| file offset | bytes | decode/classification |
| ---: | --- | --- |
| `0x7110E` | `ff` | `db 0xFF (byte-unclassified)` |
| `0x7110F` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71110` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71111` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71112` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71113` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71114` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71115` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71116` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71117` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71118` | `00` | `db 0x0 (byte-unclassified)` |
| `0x71119` | `00` | `db 0x0 (byte-unclassified)` |
| `0x7111A` | `00` | `db 0x0 (byte-unclassified)` |
| `0x7111B` | `00` | `db 0x0 (byte-unclassified)` |
| `0x7111C` | `2c` | `db 0x2C (byte-unclassified)` |
| `0x7111D` | `00` | `db 0x0 (byte-unclassified)` |

Relocation context:
- record `0x46` source kind `32-bit-offset` source file offsets 0x177CB; raw `07 00 cb 0b 02 0e 05`

## Conservative conclusion

- Proven SKULL.ASM label/control-flow mappings: 0.
- The decoder produced local byte/instruction classifications only. Any relative call/jump decoded here is not promoted to source/control-flow evidence without relocation plus source/label proof.

## Limitations

- No SKULL.ASM label or source-level control-flow mapping is claimed; mapping_count is intentionally 0.
- Decoded instructions are local byte-decoder facts only, not function boundaries or a call graph.
- Fixup target samples come from the existing mechanically parsed LE fixup-record samples, not the full 11013-record table.
- ReDMCSB is not used to infer DM2 runtime correspondence.
