# Exact-LBA capture filter for the JP Akutuba source sector

**Status: instrumentation implemented; authentic capture evidence remains open.**

The identity-checked JP Rev. 1 planner maps the Akutuba descriptor and all
four map arrays to Track 02 file sector 308, Track 02 relative LBA 84, and disc
LBA 3674. This note documents a capture filter for that source location. It
does not attribute the data to the game's selected map or renderer.

## Capture interface

Set `THERON_CAPTURE_PCE_FAST_EXACT_SOURCE_LBA=3674` when invoking
`scripts/capture_theron_mednafen_live_trace.sh`. The wrapper accepts decimal
values from 0 through 4294967295, canonicalizes leading zeroes, and requires
the `pce_fast` Mednafen module. It passes the value to the reference core and
records `exact_source_lba_filter=3674` in the capture summary. An unset value
is recorded as `none` and preserves the prior unfiltered behavior.

The PCE Fast CD data-port trace applies the exact-LBA gate before consuming its
row quota. FIFO-origin RAM receipts apply the same gate after reading each
byte's source LBA and before consuming their receipt quota. The CD FIFO origin
queue still tracks every sector and byte; filtering the queue itself would
shift provenance for intervening reads. Invalid values fail closed in the
instrumentation helper, in addition to the wrapper's input validation.

## Evidence boundary

The filter narrows two diagnostic sidecars. It does not authenticate the
observed byte against the retail BIN, prove a game-owned consumer, identify an
active-map selector or party pose, or show a VDC/VCE/VRAM update. A cold
authentic capture must still pass the existing media identity, raw-sector,
source-offset/value, RAM destination, event-order and completeness checks. The
same-session consumer and renderer chain remains open in
[`TODO-theron.md`](../../TODO-theron.md).

## Verification

The capture-wrapper regression covers valid LBA 3674 (including zero-padded
input), out-of-range input, and rejection under the standard PCE module. The
instrumentation patch regression requires the CD data-port and FIFO-origin
gates and verifies that the complete Mednafen 1.32.1 Theron patch set applies
to a disposable source copy. Results from the current run are recorded in
`DONE-theron.md`; passing these tooling checks is not a captured-game result.
