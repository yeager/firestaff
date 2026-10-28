# Japanese Theron authentic-save mouse input trace

Date: 2026-10-28

Purpose: Check what a bounded PCE Fast mouse input sample changes while the
authentic Japanese first-dungeon save is running. This is reference-emulator
evidence only; Mednafen is not part of Firestaff's runtime.

## Inputs and method

- Authentic Japan Rev. 1 Track 02: MD5
  `b7afb338ad31be1025b53f9aff12d73a`.
- Existing authentic in-dungeon Mednafen state: MD5
  `14dec90b96ec3e14622ec0fab535a92f`.
- Authentic System Card 3.0: MD5
  `ff1a674273fe3540ccef576376407d1d`; the capture normalized its 512-byte
  header to runtime MD5 `38179df8f4ac870017db21ebcbf53114`.
- Instrumented Mednafen 1.32.1 build: SHA-256
  `a62baa8b6b7832ad1fe7b6a9a46a740935845962a9c47d38b84521668dd7ce65`.

Six isolated three-second captures used `pce_fast`, the same authentic CUE,
state and System Card, with three no-script controls and three scripted mouse
runs. Each mouse run injected
`127,0,0;0,0,1;0,0,0;0,0,0`: one positive horizontal delta, one one-update
button-mask `01` sample, then two zero samples. The three control mouse-trace
files have identical SHA-256
`d8a2cb6d53dae07d15bb087d1a58c970b536d5daf772483da5c40913de07d5b4`; the
three scripted traces have identical SHA-256
`bc5d57d63b03d8c7472a7390d2d2c174e5d9e78714fdc180ac16b5b0958b222e`.
The private trace files remain outside Git in the task-specific TRV2
`/dev/shm/firestaff-theron-reference-capture-20261028` directory.

## Observations

- All three scripted captures recorded the four expected `device_script`
  samples. The control captures recorded none. The input receipt contained
  356 or 358 transactions, depending on shutdown timing.
- The original code wrote `$00` and `$50` to `$2911` from `$D26D` and `$D282`
  in both control and scripted captures. These writes therefore cannot be
  attributed to the scripted mouse sample or assigned hover meaning from
  this trace.
- No capture recorded a guest write to `$2905`; all six transition receipts
  reported `command_ram_writes=0`, `cd_irq_callbacks=0`,
  `authenticated_cd_ram_receipts=0`, and `transition=missing`.
- The same 356-transaction control and scripted pair had equal close-time
  VRAM hashes. Other captures with 358 transactions did not share the same
  VRAM hash across conditions. Shutdown-time memory differences are therefore
  not a stable mouse-caused graphics or gameplay result.
- Both conditions entered the `$C1DF` trace from `$A1D1` and `$A1D8`; the
  save-state load did not provide a Track 02 sector-to-bank receipt for those
  executing bytes. It also bypassed the cold-start CD transfer path.

## Limits and next evidence

This confirms that the instrumented emulator applied the requested device
samples. It does not show that the retail game consumes them as a pointer
command, identify a mouse target, or establish a native Firestaff interaction.
Do not use `$2911` as mouse-hit evidence from these captures. A direct viewport
mouse action still needs a source-bound command consumer and a demonstrated
source-owned game-state transition; a save-state-only trace cannot supply the
missing CD-to-RAM provenance.

No game media, save-state, firmware, or raw memory snapshot was added to the
repository.
