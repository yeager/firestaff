# Theron US saved-state scripted-input boundary — 2026-10-31

## Setup

This diagnostic reference capture used the PCE Fast module, the original US
CUE, and an existing Mednafen state file. The state-load message appears in
both control and input-run logs. The capture recorded these input hashes:

| Artifact | MD5 |
|---|---|
| US CUE | `63dbd2fab613b2e8030ff4e44b978a39` |
| US Track 02 BIN | `f23601102138f87c33025877767ebf76` |
| Existing US state file | `f17f377df210b4a3ae904a13fb85a7f0` |
| System Card 3.0 | `ff1a674273fe3540ccef576376407d1d` |
| Instrumented Mednafen binary | `cdef1c8ebe6ec3248ea19559cdd13fe3` |

The state file is retained outside the repository at
`/home/trv2/firestaff-theron-evidence/state/Dungeon Master - Theron's Quest (USA).bee0988239a817f20a64cd38fc8caeac.mc0`.
Capture outputs are under
`/home/trv2/firestaff-theron-evidence/capture/theron-stair-route-pce-20261031/`.
The input run scripted Right at frame 1 for 140 frames, Down at frame 2 for 33
frames, and Button I at frame 145 for 5 frames.

## Observations

The run logged 1,354 guest input reads and corresponding result records at
`$44CA` and `$44DB` (677 at each PC, frames 2–1354). Every recorded `raw` field
was `0000`, including reads during the scripted events. The script log confirms
that masks were applied by the instrumentation, but the guest-read records do
not show those masks. This capture therefore does not prove that the game
received any of the scripted actions or identify the semantics of either PC.

The close-time control/input snapshots differ by 570 bytes in main RAM, 17,385
bytes in VRAM, and 3 bytes in SAT. VCE (1,024 bytes) and the 116-byte VDC-state
snapshot are identical. The artifacts do not bind those differences to a
particular guest action, party pose, map cell, or game-owned display consumer.
They cannot be used to derive a map-to-BAT/tile/palette mapping.

## Boundary and next step

This is useful evidence that the authentic-media state can enter a code path
outside the previously observed System Card input PCs, but neither the state
nor the `$44CA/$44DB` routines have been joined to authenticated runtime
Track 02 provenance. The instrumentation discrepancy must be resolved before
using this route to infer input behavior. A follow-up needs reads that expose
the applied controller value, a source-backed guest state change, and a
same-session join from that state to VDC/VRAM output.

The paired capture is not checked into Git; its paths and input hashes are
recorded above. A separate standard-PCE artifact with the same state file
reported a state-format/load failure and is excluded from these observations.
SHA-256 values for the control and input-run RAM, VRAM, SAT, VCE, and VDC
snapshots are recorded in the capture's external `artifact-sha256.txt` ledger.
