# Theron's Quest V1 — later-level resource-chain boundary

## Result

The authentic US and JP Track 02 later-level spans are admitted as
hash-verified byte windows and their six-byte resource framing is retained.
They are not yet decoded into a bitmap, tile atlas, dungeon grid, palette, or
object table.

The first direct full-frame experiment used authentic US level 1 data at user
data offset `$09F000`. Treating the shared `$E8`-byte prologue as a caller
pointer-table seed and passing the frame to the bounded C lift stopped with
`THERON_HUC6280_DECODE_POINTER_TABLE`. That result is a negative diagnostic,
not permission to invent a table or to discard the real bytes.

## Why the flat lift is insufficient

The byte-backed bank-$1f listing in
`docs/source-lock/theron-disassembly/theron-us-bank1f-consumer.asm` shows:

- `$23DC` re-enters `$23AD` after advancing the source by six bytes;
- `$23DE` writes destination pointers before token consumption;
- `$2459` and `$246E` load MPR values from `$3B7E-$3B85`;
- `$2496-$252A` consumes the pointer table for back-references.

Therefore the current host API, which accepts one flattened six-byte frame,
one destination window and a caller-supplied table, is only a byte-level
algorithm lift. It cannot authenticate the real output without the same
runtime's frame-chain termination, destination pointer, MPR state and
post-CD consumer.

## Admission rule

Until one authenticated capture binds those values to a raw-sector span and
the executing `$2600` RAM consumer, production must retain the later-level
receipt as opaque compressed source data. No square-to-tile mapping,
perspective material, VCE palette, atlas, monster record, object record or
bitmap is derived from this boundary.

The focused real-media receipt test still verifies all seven US/JP BIN and
ISO block offsets, prologues, metadata, lengths and hashes. The existing
synthetic decoder fixture remains algorithm-only and is not game data.
