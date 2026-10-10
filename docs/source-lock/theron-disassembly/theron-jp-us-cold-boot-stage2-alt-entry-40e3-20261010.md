# Theron cold-boot Stage-2 alternate entry at `$40e3`

The byte-locked `$4002` static candidate contains `JSR $40e3` at `$404d`,
after `CPX #$03` and a conditional branch. The target lies 225 bytes after the
candidate start. In the linear listing begun at `$4002`, the byte at `$40e3`
is the final operand byte of a `TIA` instruction begun at `$40dd`. Decoding
again at the direct call target produces a second, overlapping instruction
stream. This is a static edge in the candidate bytes, not evidence that this
candidate supplied the runtime code.

## Authentic media locks

| Edition | Track 02 | Full-image MD5 | Raw offset at `$40e3` | Length | Span SHA-256 |
| --- | --- | --- | ---: | ---: | --- |
| JP Rev. 1 | `TQJP02.bin` | `b7afb338ad31be1025b53f9aff12d73a` | `0x297483` | 201 | `2c41de75cc88b527a3ed8606c24bfe1cfc4e11f39a781714601890b638e5a573` |
| US Rev. 1 | `TQUS02.bin` | `f23601102138f87c33025877767ebf76` | `0x297db3` | 201 | `fa686ced729a6563d0f4d00c6d9e6bdf2f9c3f547d8a21afbd51ee6c26c0cd05` |

Both alternate-entry spans begin with the same 20 bytes at `$40e3` and diverge
at `$40f7`. The following 108 bytes also contain edition-specific operands
and call targets. Keep the two MAME listings separate; neither should be
substituted for the other or treated as proof of a common implementation.

The alternate entry statically calls `$4143`, branches to `$40e9` on carry,
and otherwise returns. The `$4143` path reaches a status test at `$4163` and
returns with carry clear or set; `$40e9` later calls `$4179`. The `$417c`
branch to `$41ab` lands on an `RTS` included at the end of both listings. These
are instruction-level edges only; callees remain unidentified.

The listings cover `$40e3-$41ab` (end-exclusive) and were generated from the exact original
raw Track 02 files with MAME 0.285 `unidasm -arch h6280 -basepc 0x40e3
-count 201`, using raw offsets
`2716803` (JP) and `2719155` (US). The registered static-source test checks
each listing against its complete authenticated image identity, exact raw
offset, span hash and bytes. No names or runtime semantics are assigned to
these call targets or data writes. Dynamic source binding and gameplay remain
unproven.
