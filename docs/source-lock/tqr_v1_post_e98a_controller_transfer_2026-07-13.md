# Theron's Quest post-E98A controller transfer receipt

## Scope

This receipt records the first executed HuC6280 control transfer after the
already observed System Card `$e98a` instruction. It is a live Mednafen PCE
observation gate, not a decoder and not a game-data classification.

## Capture contract

The instrumented Mednafen 1.32.1 patch begins this narrow capture directly
after the real post-latch flow has executed `$e98a` (`LDA $22A4`). The live
program counter is the gate, so earlier diagnostic markers cannot hide an
executed instruction. It records the
source PC, physical source PC, disassembly text, next logical PC, and next
physical PC for the first branch, call, return, or indirect transfer reached
inside `$e98a..$ea3f`.

`scripts/verify_theron_post_e98a_controller_transfer_trace.sh` accepts only a
trace with the original `$e98a` receipt before that captured transfer. It does
not accept a static disassembly, fixture, or a trace that merely reaches the
surrounding controller loop.

## Boundary

The captured source and target are control-flow facts only. No CD command,
CD_READ, sector, record, destination, payload, bitmap, palette, object, or
level meaning is assigned until an independent original observation proves it.

## Runtime-handoff consumer

`scripts/verify_theron_post_e98a_track02_runtime_handoff_trace.sh` is the
single fail-closed consumer for a combined live capture. It first requires the
post-`$e98a` transfer receipt above, then requires exactly one separately
captured stage-two transaction at `$4090 -> $4093`: one sector, local-RAM
destination `$3800`, and all live `CL/DL/CH` record registers. It also requires
the original System Card 3.0 IRQ2 state receipt at `$e74c`, including the
observed `$f5`, `$1802`, `$1803`, and `$f2` merge.

The record is never derived from controller flow. The consumer accepts only a
captured JP `0004df` or US `0004e0` value paired with its matching variant;
these are the existing `$4090` runtime-handoff records established by the real
HuC6280 stage-two loader (`theron-us-stage2-huc6280.asm:163-187`) and the
Track 02 IPL receipt. A missing, duplicated, reordered, or mismatched capture
blocks the handoff. This binds provenance only: the loaded manifest remains
semantically unbound.
