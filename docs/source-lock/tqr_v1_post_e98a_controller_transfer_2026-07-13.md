# Theron's Quest post-E98A controller transfer receipt

## Scope

This receipt records the first executed HuC6280 control transfer after the
already observed System Card `$e98a` instruction. It is a live Mednafen PCE
observation gate, not a decoder and not a game-data classification.

## Capture contract

The instrumented Mednafen 1.32.1 patch begins this narrow capture only after
the real post-latch flow has executed `$e98a` (`LDA $22A4`). It records the
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
