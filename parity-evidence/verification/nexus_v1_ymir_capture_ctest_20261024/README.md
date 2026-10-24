# Ymir capture regression checks — 2026-10-24

On TRV2, two focused CTest checks passed against the shared Firestaff Debug
build:

- `nexus_v1_ymir_runtime_capture_authentic` repacks the hash-pinned Ymir VDP1,
  VDP2, CRAM, and framebuffer dumps and compares the composition to the
  observed RGB5 display framebuffer.
- `nexus_v1_ymir_title_color_calculation_authentic` checks the capture-backed
  title color-calculation path.

`ctest.log` records both passes and the suite result. The separate
`nexus_v1_title_same_session_capture_real` gate was skipped. Its verbose log
states that there is no complete hash-bound same-session Nexus title capture
in TRV2's capture root. It needs a real same-session trace binding CD FIFO
reads, SH-2 source writes and instruction reads, VDP2 writes/registers, and the
full VDP1/VDP2 state. The standard root
`/home/trv2/.firestaff/external/nexus-capture` currently has no capture
subdirectories. This is the next concrete evidence gap for admitting the
retail title source path.

`input_sha256.json` and `state_observation.json` identify the authentic Ymir
capture inputs and selected display framebuffer. Binary dumps and game media
remain in the existing local evidence/data directories and are not duplicated
here.

The build used the shared source snapshot based on revision `dd79c4f18`; that
snapshot contained concurrent uncommitted changes. Passing these tests
validates capture-level composition and color calculation only. It does not
prove production renderer permission, title-menu selection, or viewport/HUD
parity. The Nexus startup gate remains blocked pending verified retail source
ownership and compositor evidence.
