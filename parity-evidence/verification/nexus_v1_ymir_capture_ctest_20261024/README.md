# Ymir capture regression checks — 2026-10-24

On TRV2, two focused CTest checks passed against the shared Firestaff Debug
build:

- `nexus_v1_ymir_runtime_capture_authentic` — repacks the hash-pinned Ymir VDP1,
  VDP2, CRAM, and framebuffer dumps and compares the composition to the
  observed RGB5 display framebuffer.
- `nexus_v1_ymir_title_color_calculation_authentic` — checks the capture-backed
  title color-calculation path.

The exact CTest output is in `ctest.log`. `input_sha256.json` and
`state_observation.json` identify the authentic capture inputs and selected
Ymir display framebuffer. The binary dumps and game media remain in the
existing local evidence/data directories and are not duplicated here.

The build used the shared source snapshot based on revision `dd79c4f18`; that
snapshot contained concurrent uncommitted changes. These tests validate
capture-level composition and color calculation. They do not prove production
renderer permission, title-menu selection, or viewport/HUD parity. The Nexus
startup gate remains blocked pending verified retail source ownership and
compositor evidence.
