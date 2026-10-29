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

Capture writes from the refill routine and its callers with logical and
physical destinations, active MPR values, instruction bytes, and bounded
register state. Join those writes to the decoded `$0D0000–$0D13C7` input and a
later authentic game state before assigning semantics or enabling a native
renderer route. Keep the capture gate explicit: the subsequent r4 attempt did
not witness the final scripted Button I mask, and its raw traces were removed
during TRV2 cleanup.
