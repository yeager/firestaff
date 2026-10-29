# Standard PCE mouse-script repeat verification

## Change

The reference-only Mednafen mouse injector previously accepted the wrapper's
`dx,dy,button-mask*repeat` grammar for both PCE cores, but only PCE Fast
expanded repeat counts. Standard PCE parsed the first three integers and
silently ignored the trailing `*repeat` text. The instrumenter now shares one
strict parser between standard PCE and PCE Fast, including the 20,000-update
expanded-script limit.

## Verification

- `tests/test_theron_mouse_input_parser.py` compiles the inserted C++ parser
  with warnings as errors. It checks repeat expansion, malformed tokens, and
  the total update bound. It ran three times through
  `tests/test_theron_v1_mednafen_live_capture_script.sh`.
- The full Theron Mednafen patch set applied to three isolated clean source
  copies on TRV2. A serial `-j1` Mednafen build with PCE Fast enabled completed
  and exposed both `pce` and `pce_fast` modules.
- A fresh standard-PCE reference process loaded the authentic USA Ak-Tu-Ba
  state (MD5 `f17f377df210b4a3ae904a13fb85a7f0`), authentic USA Track 02 (MD5
  `f23601102138f87c33025877767ebf76`), and System Card 3.0 (MD5
  `ff1a674273fe3540ccef576376407d1d`). The input script was
  `1,0,0*3;0,0,1;0,0,0`. The bounded device trace recorded three horizontal
  units in latch deltas `-2` and `-1`, followed by Button I. This verifies the
  injector's repeat behavior and device delivery in that capture.

The instrumented binary SHA-256 was
`17b7e776cd58e8ca253913f0953fd7aa66c842420bb49ebb355c10bafda3f26e`; the
mouse trace SHA-256 was
`3495c5d3c9e7dae5a2e5018a4f0bc1a0dbb0f6e4d5960e1044d1fdc0f719d8f0`.

## Limits

The capture replayed a save state and therefore produced no authenticated
CD-to-RAM receipts; its transition receipt says `missing`. It verifies input
tooling only. The device deltas are not bound to game-owned pointer
coordinates, target geometry, command semantics, or a resulting world-state
change. Native Firestaff mouse parity remains open.
