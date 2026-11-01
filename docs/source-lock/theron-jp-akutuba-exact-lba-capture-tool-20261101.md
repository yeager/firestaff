# Exact-LBA capture filter for the JP Akutuba source sector

**Status: instrumentation implemented and reference binary rebuilt; authentic
capture evidence remains open.**

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
to a disposable source copy. The follow-up indirect-jump trace patch now arms
its bounded fetch window from the executed dispatch and places the fetch hook
at the HuC6280 opcode-fetch point; its patch-application regression passed
three consecutive loops. The complete r4 binary was built against standalone
SDL 2.32.10 and passed its isolated startup/linkage check.

The earlier headless JP attempt stayed in the System Card and hit the
2,048-row decision-trace cap. A later PID-bound cold attempt delivered all 15
requested host key presses and recorded active-low RUN/I values at BIOS poll
`$E4C5`, but did not verify the `$E11D` bootstrap branch or a game-owned poll.
Its dispatcher `$4000` entries remain classified as System Card path. The
wrapper recorded zero CD IRQs, zero authenticated CD-to-RAM receipts, and only
the exact-LBA sidecar header (no LBA 3674 rows) before timing out. Its decision
trace passed with 88 rows; the separate status trace reached its 65,536-row
cap. Neither attempt shows that the sector is absent from a successful game
load. See the [PID-input capture report](theron-jp-lba3674-pid-input-cold-capture-20261101.md)
and [capture-readiness summary](../theron/THERON_CAPTURE_READINESS.md).

A later visual follow-up attempted a four-second RUN press after inspecting
the cold-start window. The input trace still contained only released-button
samples, the screen remained at the PC Engine Super CD-ROM System splash, and
the wrapper rejected the capture because the status trace had no `$1800` reads.
It did not exercise the exact-LBA filter or verify a game handoff; see the
[visual follow-up record](theron-jp-lba3674-pid-input-cold-capture-20261101.md#visual-follow-up-with-a-pid-targeted-run-attempt).
