# Authentic Nexus and Ymir capture checks — 2026-10-24

On TRV2, seven focused tests passed against the shared Firestaff Debug build.
Four exercise authentic Japanese retail title resources from
`FIRESTAFF_NEXUS_CUE=/home/trv2/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue`:

- `nexus_v1_title_res_corpus_receipt_real`
- `nexus_v1_title_titl_pp_payload_admission_real`
- `nexus_v1_title_dgt2_pp_payload_admission_real`
- `nexus_v1_title_mapd_tibg_admission_real`

The other three passing checks exercise authentic Ymir capture data:

- `nexus_v1_ymir_priority_compositor_authentic`, using real viewport layer
  captures and the verified retail `STABG.BIN` hash.
- `nexus_v1_ymir_runtime_capture_authentic`, repacking hash-pinned Ymir VDP1,
  VDP2, CRAM, and framebuffer dumps and comparing composition to the observed
  RGB5 display framebuffer.
- `nexus_v1_ymir_title_color_calculation_authentic`.

`ctest.log` and `title-resource-ctest.log` preserve the pass results;
`title-resource-ctest-verbose.log` records the CUE path each retail test used.
The real data folder `/home/trv2/.firestaff/data` was symlinked into the test
snapshot; no game data was copied. `STABG.BIN` SHA-256:
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
snapshot contained concurrent uncommitted changes. Passing these tests
validates authentic title-resource admissions and capture-level composition.
It does not prove production renderer permission, title-menu selection, or
viewport/HUD parity. The Nexus startup gate remains blocked pending verified
retail source ownership and compositor evidence.
