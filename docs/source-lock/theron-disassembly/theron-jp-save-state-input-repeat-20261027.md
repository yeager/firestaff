# Theron JP save-state input repeat capture (2026-10-27)

## Scope

This note records repeated, authentic PCE Fast observations from one
user-created Japanese Akutuba in-dungeon save state. It tests whether bounded
directional input produces repeatable close-time RAM and VDC snapshots. It
does not establish a source-media-to-runtime transfer, party movement, a
coordinate-to-screen mapping, or a production renderer contract.

## Inputs and method

- Track 02: authentic Japanese Rev. 1, MD5
  `b7afb338ad31be1025b53f9aff12d73a`.
- Save state: private Mednafen `.mc0`, MD5
  `10a929bd1ef15462a7bc59926cb960dd`.
- Reference binary: instrumented PCE Fast Mednafen, SHA-256
  `19daccfac19e4cc847e6f4b6e0e0045a24e6523bba73676e09807b90caeb288`.
- Three isolated 12-second captures were made for each condition: no input,
  `up@120:60`, and `right@120:60`. Every run loaded the same save state.
- The scripted-input verifier passed for all six directional runs. Each
  event was applied during its hold and read at the non-System-Card poll
  `$44D2` (physical PC `$0D04D2`); each capture contained 1,432 controller
  reads. These are input-delivery and poll receipts only.

## Observations

- The three no-input close-time snapshots were byte-identical across the
  8-KiB PCE Fast RAM, 64-KiB VRAM, VCE, SAT, and VDC-state files.
- For each direction, the three final VRAM and SAT snapshots were byte-
  identical to one another. Compared with the paired no-input run, `up`
  changed 2 VRAM bytes and 2 SAT bytes; `right` changed 90 VRAM bytes and
  3 SAT bytes. VCE and VDC-state snapshots were identical to the controls.
- PCE Fast RAM differed from its paired control by 55, 63, and 63 bytes for
  `up`, and by 47, 55, and 47 bytes for `right`. Some general RAM bytes also
  varied between repeated captures of the same direction, so those deltas
  remain opaque.
- Every final RAM snapshot reported `$2031=02`, `$203F=01`, `$2040=02`,
  `$2041=03`, `$20DA=01`, and `$20DB=00`. The party-RAM sidecar stream was
  byte-identical across the nine runs; it has no replay-frame field and does
  not identify a command consumer or movement transaction.

## Limits and next evidence

Every wrapper run ended with `transition=missing`, zero CD IRQs, zero raw
sector bindings, and zero authenticated CD-to-RAM receipts. Loading the save
state bypasses the original Track 02 transfer. The directional events are
therefore correlated with repeatable screen-memory deltas at this saved
boundary, but not with a proven movement, pose, or source-owned renderer
state. The captures and all media remain private under the task-specific
`/dev/shm/firestaff-theron-native-pair-loop-20261027-7ae2/` directory on
`trv2`; no binary, state, screenshot, or raw trace was added to Git.

The next useful capture must start from the original System Card path and
produce a same-session authenticated Track 02-to-RAM receipt, game-owned input
consumer, party/world-state result, and VDC output. Keep production rendering
and free-form world clicks gated until those bindings exist.
