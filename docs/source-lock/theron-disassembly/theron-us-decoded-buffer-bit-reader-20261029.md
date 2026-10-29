# US decoded-buffer bit-reader observation

Date: 2026-10-29

## Source and runtime evidence

The observation comes from the cold authentic USA PCE Fast capture
`theron-us-decoder-consumer-20261029-r3` on TRV2. The receipt identifies
Track 02 MD5 `f23601102138f87c33025877767ebf76`, headered System Card 3.0 MD5
`ff1a674273fe3540ccef576376407d1d`, and the authentic USA Backup RAM. No state
was autoloaded. The `$23AD` decoder wrote 5,064 ordered bytes at physical
`$0D0000–$0D13C7`, with output SHA-256
`9adf13522e818ee49de16ac1020a7697c3416877e6bc6baa918840d7e963f8ee`.

The shared-sequence consumer trace recorded 7,082 reads by physical PC
`$1F1C3E` in the observed output range. A read at that PC executed
`B2 2E 85 0E E6 2E D0 02`, matched at PCE Fast RAM snapshot offset `$1C3E` and
authentic Track 02 LBA 4,230, user offset 1,086. The adjacent 16-byte runtime
window also matched the bytes read from that same Track 02 sector. The reader
source and capture join are documented in [TODO-theron.md](../../../TODO-theron.md).

The final 8 KiB PCE Fast BaseRAM snapshot was disassembled on TRV2 with MAME's
`unidasm -arch h6280`, using PC `$1F1C3E`. The adjacent bit-extraction loop and
refill helper disassemble as follows:

```text
$1F1C2A  STZ $10
$1F1C2C  LDY $14
$1F1C2E  LDX #$08
$1F1C30  DEC $0F
$1F1C32  BEQ $1C3E
$1F1C34  ROL $0E
$1F1C36  ROL $10
$1F1C38  ROL $11
$1F1C3A  DEY
$1F1C3B  BNE $1C30
$1F1C3D  RTS
$1F1C3E  LDA ($2E)
$1F1C40  STA $0E
$1F1C42  INC $2E
$1F1C44  BNE $1C48
$1F1C46  INC $2F
$1F1C48  STX $0F
$1F1C4A  LDA $12
$1F1C4C  BNE $1C50
$1F1C4E  DEC $13
$1F1C50  DEC $12
$1F1C52  BNE $1C34
$1F1C54  LDA $13
$1F1C56  BNE $1C34
$1F1C58  RTS
```

This establishes that the observed reads feed a pointer-advancing byte-refill
path adjacent to a bit-rotation loop. It does not establish what the extracted
bits encode, where a caller stores decoded output, or whether the data is a
map, tile, palette, or other resource. The r3 cold run also ended with
`transition=missing`; no gameplay or rendering transition is claimed.

## Next evidence

Follow the verified byte-copy destination from `$47E0` through the bit-reader's
later consumers and join it to an authentic game-state or rendering effect
before assigning structure semantics or enabling a native renderer route. The
cold capture still did not witness its final scripted Button I mask and
reported `transition=missing`.

## Shared-sequence read candidates

The bounded shared-sequence consumer trace recorded 29,618 reads from the
decoded output span after its final write. The reads touched all 5,064 output
addresses; 6,111 matched the decoded byte across 3,130 addresses. The strongest
byte-equal reader groups were physical PCs `$1042AB` and `$1042B2` (2,012
reads each), `$104287` and `$10428C` (928 each), and `$1F1C3E` (130).
`$1042AB` executes `LDA ($00),Y; STA $47E0,X; INY; INX`; its source bytes match
authentic Track 02 LBA 4,241, user offset 683. The earlier consumer-only
capture did not show the store destination. A follow-up added an opt-in
physical-write hook and narrowed it to `$1047E0–$1048DF` after the broad
two-bank window filled its 65,536-row cap. The narrow cold capture recorded
12,928 writes without truncation. Of 2,012 byte-equal post-decode reads at
`$1042AB`, 1,972 were followed by a `$1042AD` write of the same byte at the
next shared event sequence. That write's fetched code bytes are `9D E0 47`,
and MPR2=`$82` maps its observed logical destinations to physical
`$1047E0–$10485E` (64 unique addresses). This proves the observed copy
operation from the decoded buffer; it does not prove what the copied bytes
represent or that they affect rendering or gameplay.

The expanded code-window write trace now covers `$1F1C00–$1F1DFF`. A separate
verifier matched all 5,064 ordered `$23AD` output writes to exact address and
value records one shared-sequence step later: 1,840 at `$1F1C8D`, 1,432 at
`$1F1CE6`, and 1,792 at `$1F1D1D`. The previous filter ended at `$1F1CFF` and
missed the last group. The capture contained 10,128 code-window writes in all;
the remaining 5,064 occurred after decoder return and are not attributed. The
capture receipt reports no CD IRQ, game `$E009` dispatch, or gameplay
transition, and its final scripted input still lacked a poll witness. Raw
traces remain private on TRV2 until their verifier summaries are recorded; no
rendered effect or full gameplay parity is claimed.
