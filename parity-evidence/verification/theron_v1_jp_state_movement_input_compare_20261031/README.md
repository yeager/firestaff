# Theron JP state-only input/position comparison

Three PCE Fast reference runs replayed the same directional sequence from an
authentic Japanese Akutuba F5 emulator state. A matched three-run control kept
the same Button II holds and right turns but omitted later UP inputs at frames
461, 731, and 1001.
The authentic guest input reads and movement-context traces repeat the same
ordering in every run.

In the movement arm, Button II+UP (`raw=0012`, active-low port value `3e`) was
read at `$44C1` in frame 461, shared event 9565. The original then copied two
bytes from `$20B4/$20B5` to `$2040/$2041` through the observed `$C1FC` TII in
frame 469, events 9678–9681, writing `01/03`. The same sequence repeated at
frame 731/event 11298 and frame 734/events 11390–11393, writing `01/02`.
The final snapshots in all three runs contain `$2040/$2041 = 01/02`.

The control arm reads Button II alone (`raw=0002`, active-low value `3f`) at
the corresponding frames. It records no `$C1FC` transfer or write to
`$2040/$2041`; all three final snapshots retain `02/03`. The bounded movement
observer and final candidate bytes are stable across the repetitions. Other
RAM bytes vary in one run, so the full RAM hashes are not treated as identical.

This is a repeatable input-conditioned state-transfer observation from an
authentic save state. The bytes remain candidate JP position fields: the
capture does not identify the active dungeon map or prove tile passability,
the field semantics in this region, a rendered response, a cold CD-to-RAM
handoff, or Firestaff gameplay parity. The PCE Fast close-time VRAM snapshots
also vary between two movement repetitions and are not used as visual evidence.
All six transition receipts report `transition=missing` and zero authenticated
CD-to-RAM receipts. Raw traces and snapshots are retained outside Git; the
manifest records their SHA-256 values.

There is a concrete alternative to interpreting this transfer as player
movement: Firestaff's source-locked Track 02 teleporter analysis associates
the same `$C1FC` `TII $20B4,$2040,$0002` instruction with copying a packed
teleporter destination before the destination-tile check. These traces do not
include a call-stack/caller receipt or the active map's source object row, so
they cannot distinguish a movement path from teleporter resolution. See the
[teleporter handoff analysis](../../../docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md#2026-08-09-authenticated-track-02-teleporter-handoff-correction)
and [native implementation boundary](../../../src/theron/theron_v1_mechanics.c).

## Provenance

The captures used authentic Japanese Rev. 1 Track 02 (CUE SHA-256
`7bbbe8b077f35b2e9b05c68848f8cb86ea9aa74021a076e882eee8841bc56545`, Track 02
MD5 `b7afb338ad31be1025b53f9aff12d73a`), System Card 3.0 (SHA-256
`df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1`), and an
authentic JP Akutuba F5 `.mc0` state (SHA-256
`b4f2a06788e4723110b19892f93e8ac4aef1b5c18c6358a9fe36f1baf747b068`). The
state is an emulator snapshot, not in-game BRAM. The isolated instrumented
Mednafen 1.32.1 PCE Fast binary has SHA-256
`d673c3bbd7d8f27fe6b717b29af2ffaaa68d2c74e93b27abe6609a68cb019e56`.

See `manifest.json` for the per-run input, movement, RAM, and transition trace
hashes. The raw evidence is not committed.
