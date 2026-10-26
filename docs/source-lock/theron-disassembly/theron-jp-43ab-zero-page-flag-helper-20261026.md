# JP runtime helper at `$43AB`: zero-page bit flag

## Evidence identity

The execution evidence is the private PCE Fast capture
`/home/trv2/firestaff-theron-evidence/capture/theron-a1a9-initializer-20261026/live.trace.a1a9-init`
on TRV2. Its SHA-256 is
`78c31eb35cd64d96e8af5bd8bde93221efe3adca70aedac60fd5c519a20f1550`.
It was captured from an emulator-created Ak-Tu-Ba state using authentic JP
Rev. 1 media. The authentic JP Track 02 MD5 is
`b7afb338ad31be1025b53f9aff12d73a`; the matching US Track 02 MD5 is
`f23601102138f87c33025877767ebf76`.

Across 45 bounded trace windows, the JP code at logical `$43AB–$43B4` ran at
physical `$0D03AB–$0D03B4`. The trace records 45 visits to `$43AB` and `$43AD`,
23 fall-through branches from `$43AD` to `$43B0`, 22 branches from `$43AD` to
`$43B2`, 45 visits to `$43B2`, and 45 returns through `$43B4` to `$A1C1`.

## Decoded routine

The ten-byte sequence is identical at seven source offsets in each authentic
raw Track 02. The new fields in `Theron_V1Huc6280DisassemblyReceipt` expose
those offsets only for verified raw-BIN editions; they do not claim which copy
was loaded. The JP offsets are `0x94f4b`, `0xde74b`, `0x127f4b`, `0x17174b`,
`0x1baf4b`, `0x20474b`, and `0x24df4b`. The US offsets are `0x9587e`,
`0xdf07e`, `0x12887e`, `0x17207e`, `0x1bb87e`, `0x20507e`, and `0x24e87e`.

| Logical address | Bytes | Instruction | Effect |
| --- | --- | --- | --- |
| `$43AB` (JP) / `$43AE` (US) | `A9 00` | `LDA #$00` | Set A to zero. |
| `$43AD` (JP) / `$43B0` (US) | `0F 06 02` | `BBR0 $06,$43B2` | Branch to `$43B2` when bit 0 of zero-page `$06` is clear. |
| `$43B0` (JP) / `$43B3` (US) | `A9 02` | `LDA #$02` | On the fall-through path, set A to two. |
| `$43B2` (JP) / `$43B5` (US) | `85 07` | `STA $07` | Store A in zero-page `$07`. |
| `$43B4` (JP) / `$43B7` (US) | `60` | `RTS` | Return to the caller. |

The HuC6280 opcode table identifies `$0F` as `BBR0` with zero-page-relative
addressing in Mednafen 1.32.1 `src/pce/dis6280.cpp:222`. The CPU implementation
is `HuC6280::BBRi` and `HuC6280::JR` in `src/pce/huc6280.cpp:353–378`; `BBRi`
branches when the selected bit is clear. Therefore this routine leaves and
stores A as zero when bit 0 of `$06` is clear, or as two when it is set.

This decode rules out the helper itself as a writer of the six-rectangle
buffer `$287F–$2896`: its only memory store targets `$07`. It does not identify
what `$06` represents, who consumes `$07`, or whether either value corresponds
to pointer input. The runtime capture also contains no same-session CD read
or authenticated CD-to-RAM receipt. The exact source LBA among seven copies
remains ambiguous, and no native click behavior is established.

## Verification

`theron_v1_huc6280_disassembly` checks the exact ten bytes at all seven JP and
US raw-BIN offsets after validating the full authentic Track 02 identity. On
TRV2, the JP execution trace also matched all 203 dynamically executed callee
rows against each of the seven JP copies in three repeated verifier runs.
That runtime comparison is a byte match, not a load-provenance receipt.
