# Ymir capture regression checks — 2026-10-24

On TRV2, three focused CTest checks passed against the shared Firestaff Debug
build:

- `nexus_v1_ymir_priority_compositor_authentic` composes authentic Ymir
  viewport layer captures with the verified retail `STABG.BIN` hash.
- `nexus_v1_ymir_runtime_capture_authentic` repacks hash-pinned Ymir VDP1,
  VDP2, CRAM, and framebuffer dumps and compares the composition to the
  observed RGB5 display framebuffer.
- `nexus_v1_ymir_title_color_calculation_authentic` checks the capture-backed
  title color-calculation path.

`ctest.log` records these three passes and one skipped gate. The real data
folder `/home/trv2/.firestaff/data` was symlinked into the test snapshot; no
game data was copied. The retail `STABG.BIN` SHA-256 was
`7b8e44ffd1249175da1c407993b983a26bc180204e63f9b69274014b336c6913`.
`input_sha256.json` and `state_observation.json` identify the authentic Ymir
capture inputs and selected display framebuffer. Binary dumps and game media
remain in the existing local evidence/data directories and are not duplicated
here.

The separate `nexus_v1_title_same_session_capture_real` gate was skipped. Its
verbose log states that TRV2 has no complete hash-bound same-session Nexus
title capture. The capture must bind real CD FIFO reads, SH-2 source writes and
instruction reads, VDP2 writes/registers, and the full VDP1/VDP2 state. The
standard root `/home/trv2/.firestaff/external/nexus-capture` currently has no
capture subdirectories. Producing that trace is the next concrete evidence
gap for admitting the retail title source path.

The build used the shared source snapshot based on revision `dd79c4f18`; that
snapshot contained concurrent uncommitted changes. These tests validate
capture-level composition and color calculation only. They do not prove
production renderer permission, title-menu selection, or viewport/HUD parity.
The Nexus startup gate remains blocked pending verified retail source
ownership and compositor evidence.
