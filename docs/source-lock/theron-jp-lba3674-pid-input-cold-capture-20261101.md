# JP LBA 3674 PID-input cold capture (2026-11-01)

**Result: authentic host input reached the System Card, but this capture did not
verify a game handoff or exercise the exact-LBA source read.**

## Inputs and isolation

- Authentic JP Rev. 1 Track 02 MODE1/2048 image, MD5
  `62a39bbf43415c9739c41c2481080a49`, launched from its original CUE.
- Authentic Japanese System Card 3.0, MD5
  `ff1a674273fe3540ccef576376407d1d`.
- Instrumented Mednafen 1.32.1 PCE Fast binary, SHA-256
  `0280a8e9dcce6ca55b43a29d2f8549fd30ad62c339bca940a2717b58cbca81f6`,
  built against standalone SDL 2.32.10.
- Cold start with no state autoload. Save, state and trace outputs used a new
  private profile on the external disk. PID-bound Quartz events targeted only
  this capture's Mednafen process; the existing user emulator was not used.
- Capture ID: `61ac56196710487897df4af75dfb4cb9`.

## Input and observations

The capture sent 15 PCE button presses through the host event path: one
four-second RUN hold, two four-second Button I holds, then twelve one-second
Button I holds scheduled two seconds apart. Mednafen recorded the host SDL
key events and its PCE input trace observed active-low masks `0008` (RUN) and
`0001` (Button I) at the System Card poll `$E4C5`. This proves delivery to the
BIOS poll, not a title-menu or game-owned input receipt.

The independent decision trace passed with 88 rows at six selected BIOS PCs
and no decision-trace truncation. The separate System Card status trace
reached its 65,536-row cap and emitted its truncation marker. The dispatcher
sidecar contains `$4000` entries classified as System Card path; the required
`$E11D` bootstrap carry branch and a game-owned input poll were not verified.
The wrapper therefore did not accept a game handoff. It recorded zero CD IRQs
and zero authenticated CD-to-RAM receipts.

The exact-LBA CD data-port sidecar contains only its header: no row for LBA
3674 was observed. This failed route does not show that the sector is absent
from a successful gameplay load. The process ended at its 140-second timeout
(exit 124), and the wrapper rejected the capture.

## Tooling correction

Before this attempt, host-input scheduling waited for the input trace header
to equal `source=mednafen-pce-fast-instrumented-input` as a whole line. The
actual versioned header appends `event_sequence_source=...`, so the gate timed
out before it sent any keys. The gate now matches the exact source token at
the start of the header and requires a token boundary. Its regression accepts
the metadata-bearing PCE Fast header and rejects a different source marker;
the complete live-capture wrapper regression passed three consecutive loops.

The earlier headless attempt did hit the independent decision trace's
2,048-row cap. That is a separate failed capture. Neither attempt exercised
LBA 3674 during a verified game load. Keep the cold input sequence aligned to
observed menu readiness before retrying; see the
[capture-readiness summary](../theron/THERON_CAPTURE_READINESS.md) and the
[exact-LBA filter note](theron-jp-akutuba-exact-lba-capture-tool-20261101.md).

## Visual follow-up with a PID-targeted RUN attempt

A later cold diagnostic (capture ID
`d841429462394da38f1de5259d0f276a`) opened the same authentic JP route and
recorded a screenshot of the PC Engine Super CD-ROM System splash. A
PID-targeted four-second RUN keypair was sent, but the PCE input trace contains
only released-button samples (`value=3f`) and no RUN/I mask. The post-input
window remained at the splash. The dispatcher sidecar recorded `$E0F3`,
`$E109`, and `$C860` only; decision/status sidecars had headers but no rows,
and the exact-LBA data-port sidecar did not produce evidence. The wrapper
rejected this attempt with `status trace contains no observed $1800 reads`.
This attempt therefore provides no evidence of accepted RUN input, title-menu
readiness, game handoff, or LBA 3674 use. Keep it separate from the earlier
15-press capture above, which did record RUN/I at `$E4C5`.

Raw traces and emulator outputs remain outside Git. No media bytes or
emulator-produced data were added to the repository.
