# Japanese cold PCE mouse-mode Run trace

Date: 2026-10-29

## Setup

The capture used authentic Japan Rev. 1 Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`) and System Card 3.0 (MD5
`ff1a674273fe3540ccef576376407d1d`; headerless runtime body MD5
`38179df8f4ac870017db21ebcbf53114`). It ran Mednafen 1.32.1 `pce_fast`
with port 1 set to its mouse device, no save-state, no autosave, and the
capture's isolated profile. The instrumented binary SHA-256 is
`b89d44d8a77f0f70abb21054e9db5eb428bf63b9ad3c76d00de83b7f475c3dfa`.
Mednafen logged the Japanese SBI sidecar as missing; the capture does not
establish that this caused the absent game handoff.

The cold input plan used the supported compressed sample form
`dx,dy,button-mask*repeat`. It waited 9,600 mouse updates, held the Run bit
(`08`) for 240 updates, then sent a three-second Button I hold (`01`) and six
short Button I presses. The late input log was bounded to start at frame 9,300
so early System Card polling could not exhaust the trace limit.

## Result

- The native PCE Fast input trace recorded System Card polls at `$E4B4/$E4C5`.
  During the Run hold, the `$E4C5` poll returned active-low `$37`; during the
  Button I hold it returned `$3E`. The raw mouse trace records the corresponding
  scripted `08` and `01` button masks.
- The CPU remained in the System Card input poll through frame 10,384. The
  capture had zero CD IRQ callbacks, zero raw-sector spans, zero authenticated
  CD-to-RAM receipts, zero game `$E009` dispatches, and no `$C1DF` pointer
  trace entries. Its transition receipt is `missing`.
- This is evidence that the selected mouse button bits reached the System Card
  input port. It does not identify why the System Card did not hand off to the
  game, prove a Theron menu action, or provide pointer/click behavior.

The private capture and trace files are retained at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-cold-mouse-trace-20261029-r3/`.
They contain no copy of the game media, firmware, or save-state in Git.

## Capture-tool change

The mouse-script parser now accepts optional bounded `*repeat` counts. A
single compressed token can represent a long cold-start wait without
exceeding the 8,191-byte environment-script limit. Each repeat count and the
expanded total are capped at 20,000 input updates, matching the bounded mouse
trace capacity. The C++ instrumenter
patch applied to a clean Mednafen source copy; the incremental build compiled
the new parser. Three loops of valid cases and five malformed/out-of-range
cases passed the shell validator.
