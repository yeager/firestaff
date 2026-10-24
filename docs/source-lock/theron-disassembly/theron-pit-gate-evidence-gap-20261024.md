# Theron Track 02 Pit Gate Evidence Gap

**Status:** UNRESOLVED — source-backed pit tiles fail closed.

## Proven data boundary

The authenticated Track 02 data-format inventory identifies type bits `$40`
as the pit family in the one-byte map tile. This is a media-format
classification only; it does not establish movement, levitation, damage, or
landing semantics. See
[`tqr_v1_phase2_data_formats_H2339.md`](../tqr_v1_phase2_data_formats_H2339.md#23-square-type-table)
and `theron_tile_type` in `include/theron_v1_track02_dungeon_map.h`.

## Withdrawn source claim

The earlier claim that original Theron code at `$C332-$C347` applies the
`OPEN` (`$08`), `IMAGINARY` (`$01`), and party-levitation gate is not supported
by checked-in evidence. `theron-us-stage2-huc6280.asm` is a disassembly of an
authenticated Track 02 Stage-2 payload, but its presence does not bind a
logical CPU address to the gameplay bank active during a pit action. No
Theron code window, raw-source offset, mapped-bank receipt, or runtime
instruction trace in the repository establishes that `$C332-$C347` is the pit
consumer. Similar conditions in DM1/CSB-oriented probe code are not Theron
evidence.

The former claim was removed from
`src/theron/theron_v1_mechanics.c` and `DONE-theron.md`. The real-media pit
census remains useful as a count of source tile encodings only; it does not
verify a gameplay gate.

## Runtime behavior while unresolved

Both read-only movement queries and party movement block pit tiles whenever
the current level is source-authenticated, independent of the raw attribute
bits and levitation. No party position, damage, or transition state is
committed. Source-less fixture probes retain their separate legacy behavior.
Source-backed floor-actuator events targeting pits remain queued without
changing the runtime tile; the decoded record and raw attribute bits are
retained, but the original actuator consumer is not inferred.
The real-media loader test emits one first floor-adjacent pit coordinate per
edition and dungeon, including the raw tile byte, level-array slot, and opaque
header-level value. These coordinates help target emulator capture only; they
do not prove which map or tile the original runtime displays at a given state.

## Evidence required to promote behavior

For each supported US and JP runtime, capture a same-session original-emulator
receipt that:

1. Identifies the authentic Track 02 image by hash and records the active
   logical PC, physical bank, MPR mapping, and instruction bytes for the pit
   query and its callers.
2. Joins the executing code to an authenticated source byte window or records
   the loaded overlay's byte-exact origin from the retail disc.
3. Exercises an original open/closed pit with levitation state recorded and
   captures the resulting party, HP/stamina, level, and destination square.

Until those receipts exist, do not restore the OPEN/IMAGINARY/levitation gate,
apply fixture damage, choose a destination level, or claim pit parity.
