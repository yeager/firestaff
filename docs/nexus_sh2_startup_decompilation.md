# Nexus SH-2 startup decompilation

The retail Saturn startup dispatcher is recovered from an instrumented
Mednafen work-RAM capture, not from a guessed translation of menu labels.
The relevant capture was exported by Ghidra as SuperH big-endian SH-2 code.
Every export records the SHA-256 of its input capture.

`scripts/verify_nexus_sh2_decompilation.py` validates that a set of function
and disassembly exports share exactly one input hash, use the same language,
and contain only complete function exports.  It emits a JSON provenance
receipt and does not grant any drawing or startup admission by itself.

For the retained retail trace on TRV2, validate the dispatcher and its screen
callback together:

```sh
python3 scripts/verify_nexus_sh2_decompilation.py \
  /home/trv2/work/firestaff-nexus-decompile-20261005/event38-screen-route-output/functions.c \
  /home/trv2/work/firestaff-nexus-decompile-20261005/object-builder-followup-20261005/out/functions.c \
  --require 06026cde --require 06027020
```

That evidence establishes the following bounded facts:

* `0x06026cde` dispatches distinct modes, including mode values `2`, `4` and
  `6`, and installs a callback when the relevant runtime state requires it.
* `0x06027020` selects one of two continuation paths from a byte in the
  runtime object's 0x40-byte row.
* The dispatcher is stateful and callback-driven.  It is not evidence that
  the current host-side title, save, or champion presentation has the same
  pixels, timing or input semantics as the Saturn UI.

The next admissible implementation step is to capture the same session's
VDP1/VDP2/CRAM state and the displayed frame around this dispatcher.  Only
that join may unblock the existing real title or `MENU.BPK` render gates.

## Rejected startup candidate

The retained Mednafen capture from the `nexus-save-input-sweep-20261008`
session is authentic but is not a title/menu render candidate:

* SHA-256: `d5d9e190e7b3ee2812206b0c7da3277690490fb45c9498f0297d2e4f6d34d36e`
* 300 raw VDP1/VDP2 frames, with active VDP1 state throughout.
* The raw NBG1 tilemap decoder accepts sampled frames as capture-only data.
* VDP1 source joins resolve to `LEV00.DGN`, but the sequence join, camera
  transform, display origin and VDP2 composition remain unproven.

It must therefore remain excluded from title and menu admission.  It is useful
as a real-data regression input for the capture-only decoders, not as a
substitute for the missing same-session title/menu capture.
