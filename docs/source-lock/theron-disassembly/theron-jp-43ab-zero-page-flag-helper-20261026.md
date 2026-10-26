# JP runtime helper at `$43AB`: zero-page bit flag

## Evidence identity

The execution evidence is the private PCE Fast capture
`/home/trv2/firestaff-theron-evidence/capture/theron-a1a9-register-local-20261027/live.trace.a1a9-init`
on TRV2. Its SHA-256 is
`80be0ef30865051216ecc341a7f3b936e030221d4fed4fcc7238e58d1a09fb32`.
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

## Dynamic caller context

The same private trace records 45 calls at logical `$A1BE`, physical
`$0E01BE`, with instruction bytes `20 AB 43` (`JSR $43AB`). Every observed
return reaches `$A1C1`, then executes `STZ $271F` at `$A1C1`, `STZ $271D` at
`$A1C4`, and `LDA #$DF` at `$A1C7`. The caller does not consume the helper's
returned A value before replacing it, and this immediate sequence does not
read zero-page `$07`. This narrows the observed call path but does not identify
a later consumer or assign meaning to the written `$27xx` registers.

## First observed zero-page consumer

The same 128-instruction windows continue through a trace row at logical
`$C292` / physical `$0D2292` whose sampled bytes are `A5 07` (`LDA $07`). This
row occurs in all 45 windows. At the next row, `$C294`, A is `$02` in 23
windows and `$00` in 22 windows, matching the values stored by `$43B2` on the
preceding branch paths. The sampled bytes at `$C294–$C296` are three `ASL A`
instructions, and the row at `$C297` records a store to `$279F` with A=`$10`
in the 23 `$02` cases and A=`$00` in the 22 `$00` cases. This establishes a
trace correlation between the sampled `$07` load bytes, following A values,
and `$279F` store bytes for this JP state; it does not establish a
fetched-opcode-backed read or the semantic name or use of `$279F`.

The next sampled byte windows are `TAX` at `$C29A` and `LDA $4842,X` at
`$C29B`. Mednafen 1.32.1
`src/pce_fast/huc6280.cpp:29–60` maps `HU_X`, `HU_Y`, and `HU_P` to local
variables inside `HuC6280_Run`; `SAVE_LOCALS` writes them back to `HuCPU` only
when that run call exits. The old trace sampled `HuCPU.X`, `HuCPU.Y`, and
`HuCPU.P`, so it recorded stale struct values while the CPU was running. The
trace patch now samples `X_local`, `Y_local`, and `P_local`. The corrected
capture records X=`$10` in all 23 `$02` selector cases and X=`$00` in all 22
`$00` cases at `$C29B`, matching the value previously stored to `$279F`.
This verifies the bounded live-register correlation for this state. The byte
windows still come from Mednafen's mapped code page rather than the opcode
fetch. The adjacent helpers that read `$27A1` or increment `$05/$06` also
still have unbound callers and effects.

`scripts/verify_theron_pce_fast_a1a9_zero_page_consumer_trace.py` checks this
bounded trace correlation and requires the live `$C29B` X value to match the
shifted `$279F` value. The corrected private trace has 5,760 rows, 45 validated
windows, and 23/22 branch counts. The full capture's broader dynamic-evidence
gate did not obtain host-key, IRQ, or authenticated CD-to-RAM receipts; this
sidecar therefore supports only the stated instruction-window correlation.
The raw trace remains private.

## Adjacent authentic helper family

The 23 bytes immediately following the traced routine are also identical at
all seven JP and all seven US Track 02 candidates. Regional disassembly places
them at JP `$43B5–$43CB` and US `$43B8–$43CE`:

| Region address | Bytes | Decode |
| --- | --- | --- |
| JP `$43B5` / US `$43B8` | `A9 02 0F 06 02 A9 00 85 07 60` | Test bit 0 of `$06`; store 2 when clear, 0 when set. |
| JP `$43BF` / US `$43C2` | `AD A1 27 85 07 60` | Load `$27A1`, store to `$07`, return. |
| JP `$43C5` / US `$43C8` | `78 E6 05 E6 06 58 60` | Mask interrupts, increment `$05` and `$06`, restore interrupts, return. |

The static receipt now verifies this adjacent byte span at every admitted
raw-BIN candidate. These decodes narrow the next dynamic tracing task to
opcode-fetch provenance and the callers of the `$27A1` and `$05/$06` helpers;
they still do not identify the routines' gameplay roles, caller path, or
loaded CD source.

## Verification

`theron_v1_huc6280_disassembly` checks the exact ten-byte callee and its
23-byte adjacent helper span at all seven JP and US raw-BIN offsets after
validating the full authentic Track 02 identity. On TRV2, the JP execution
trace also matched all 203 dynamically executed callee rows against each of
the seven JP copies in three repeated verifier runs. That runtime comparison
is a byte match, not a load-provenance receipt.
