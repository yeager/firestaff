# Theron's Quest JP `$C414` static call contract

## Evidence boundary

This note reads only the authenticated JP Rev. 1 Track 02 source windows
already recorded in
[`theron-jp-c3a0-record-consumer.asm`](theron-jp-c3a0-record-consumer.asm),
[`theron-jp-c95d-spawn-target-candidate-20261009.asm`](theron-jp-c95d-spawn-target-candidate-20261009.asm),
and
[`theron-jp-cc3e-spawn-target-candidate-20261009.asm`](theron-jp-cc3e-spawn-target-candidate-20261009.asm).
The caller listing's receipt identifies the authentic JP BIN, raw offset,
length, and hash; the two candidate listings record their own raw offsets,
lengths, FNV values, and repeated MAME disassembly hashes.

This is a static register/control-flow observation. It does not establish
that either candidate window is mapped at its displayed CPU address in a live
bank, that this caller executes, or what any pointer/table represents.

## Caller sequence

At `$C41E`, the caller loads A from `$AB` and Y from `$AC`. It calls `$C95D`
at `$C422`, then tests the returned A with `BNE $C429`. If A is zero, the
fall-through at `$C427` shifts `$B4` right once. Both paths continue at
`$C429`, load X from `$BB`, and call `$CC3E` at `$C42B`. The caller returns at
`$C42E`.

The candidate `$C95D` listing contains an `RTS` at `$CA1B`; its bytes after
that return are not included in this caller contract. The `$CC3E` listing
contains an `RTS` at `$CC5C`; later bytes are likewise excluded. These
bounded return sites are consistent with the two displayed call operands,
but static adjacency and matching operands do not prove runtime reachability
or same-bank mapping.

## Remaining proof needed

A single authenticated JP runtime capture must bind the executing `$C414`
caller and both return edges to their mapped Track 02 bytes, including the
observed A/X/Y values. Until then this evidence cannot identify spawn data,
RNG effects, records, or gameplay semantics, and must not enable creature or
combat behavior.
