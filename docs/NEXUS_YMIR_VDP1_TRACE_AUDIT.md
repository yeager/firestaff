# Nexus Ymir VDP1 Trace Audit

`scripts/audit_nexus_vdp1_ymir_trace.py` validates Ymir `--debug` VDP1 command
traces and optionally compares a frame from an independent raw Saturn runtime
capture. It is a developer diagnostic: a valid command chain proves dispatch
order and observed register state, not that Firestaff renders the retail scene
correctly.

## Ymir trace

The CSV must contain frame and command indices, byte addresses, all sixteen
command words, frame root, and observed LOCAL coordinates. The audit checks
command alignment and bounds, sequential indices, frame grouping, CMDLINK
sequential/link/call/return behavior, loops, END placement, and that each
dispatched frame begins at VDP1 command address zero. `--frame N` adds per-draw
LOCAL and clip state for one frame.

```sh
python3 scripts/audit_nexus_vdp1_ymir_trace.py /path/to/trace.csv --frame 647
```

The optional raw-capture comparison follows the VDP1 command chain rooted at
zero and compares it with Firestaff's VRAM candidate-scan heuristic. COPR is
reported as an independently captured pointer; it is not required to point
inside the completed root-zero chain because capture timing can differ.

```sh
python3 scripts/audit_nexus_vdp1_ymir_trace.py /path/to/trace.csv \
  --frame 670 \
  --runtime-capture /path/to/capture.raw \
  --runtime-capture-frames 411 \
  --runtime-frame 23
```

When both `--frame` and `--runtime-capture` are supplied, the report also
compares Ymir's ordered command records with the root-zero command records in
the explicitly selected runtime-capture frame. The selected indices are an
operator-supplied pairing; equal numeric offsets do not prove the emulators
were at the same game state.

## Evidence and limits

On the hash-locked Japanese retail Ymir trace audited on 2026-10-24, 411 frames
(647–1057) contained 40,164 records and 39,155 draw commands. Every dispatched
frame began at address zero and reported frame root zero. The separate
authentic raw capture comparison found a 227-record root-zero chain beginning
at `0x00000`, while the scan heuristic selected a 236-record chain beginning
at `0x39f40`; the paths and per-draw state differed. This is evidence that the
candidate scan can choose a different chain. It does not by itself prove that
the corresponding production renderer is wrong or authorize scene admission.

DMWeb's Nexus references are used for Nexus retail file formats and decoder
rules. Greatstone is a secondary source for Dungeon Master/CSB comparisons and
related byte-format context. Neither source documents Saturn VDP1 runtime
dispatch; the dispatch claim here is grounded in the Ymir trace and Saturn
hardware command semantics. These references must not be used to infer texture,
palette, camera, or retail scene ownership.

Semantic admission remains blocked until command textures, palettes, source
owners, camera transforms, clipping, and Firestaff output are independently
bound to authentic retail evidence.

An additional Mednafen capture was made with the same hash-verified Japanese
BIOS and CUE, skipping 647 frames and recording the following 411 with no
button input. The raw capture is stored outside the repository with SHA-256
`ef656bc061401cd7eba5e3152ee70a7ee2c2c6ec95491c541ccca954ac69578b`. Pairing
its frame 23 with Ymir frame 670 by the nominal frame offset does not yield
matching command lists: Mednafen's root-zero chain has 3 records while Ymir
dispatched 306. The only exact full-list matches across nominal pairs are
single-record END lists. These runs therefore are not a synchronized runtime
parity witness; absolute emulator frame numbers cannot be used as a
cross-emulator join key without a shared state marker.
