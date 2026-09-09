# Firestaff TODO — Nexus

Reviewed 2026-08-31. Only open work is listed here; completed evidence belongs
in the Nexus capture and reverse-engineering records.

## Available local retail media

The staged JP retail set supplies the CUE, nine BIN tracks, CDDA WAV tracks,
and the original ZIP package.  The disc itself does **not** contain a Saturn
VDP1/VDP2 VRAM dump, CRAM dump, register snapshot, or frame/timing capture.
Firestaff reads the CUE/ISO members directly in memory; it must not manufacture
either a capture or a presentation claim from those disc files.

The local development capture corpus now includes hash-bound, same-session
hardware evidence for the retail Track 01 CD→RAM→VDP2 transfer, including
VDP1/VDP2, VRAM, CRAM, register and timing domains.  The admission tests
`nexus_v1_authenticated_hardware_capture_real`,
`nexus_v1_title_same_session_capture_real`, and
`nexus_v1_title_mapd_vdp2_transfer_real` verify that transport evidence.
Its asset semantics remain explicitly `unassigned`: it does **not** identify
the final NBG0 source, VDP1/VDP2 layer composition, priorities, or interactive
title-menu ownership.  It must therefore not enable the native title renderer.

- Capture one same-revision, post-composition title/menu state that jointly
  binds the active VDP2 source layer, CRAM palette, VDP1/VDP2 layers, priorities and
  timing. The observed NBG0 span remains unowned; do not admit a native title
  renderer or substitute inferred assets until this consumer is identified.
  The local `title-owner-join-complete-20260904` ten-frame witness was
  rechecked on 2026-09-09: every captured frame has `BGON=0`, no enabled
  VDP2 layer, and NBG0 only as an unconsumed bitmap-mode register setting.
  It is transport evidence, not a post-composition title frame; do not target
  that window again for title admission.
  The later `title-render-scout-r39-20260908` post-render frame at absolute
  frame 18000 has `BGON=0x0002`: NBG1 is its sole active VDP2 bitmap layer
  at priority 5, while NBG0 is inactive. That capture binds a raw state and
  matching PPM but has no same-session asset/write trace, so it corrects the
  next capture target without authorizing NBG1 bytes for native presentation.
  A second hash-identical one-frame capture at the same absolute frame on
  2026-09-09 (`runtime-vdp12.raw` SHA-256
  `52b423d1ba65d796508fe36583cf2006faba03ec1345a08107f767ff345b08c1`)
  again produced no VDP2 register writes in that stable frame. Future
  producer capture must span the preceding title-state transition and retain
  a bounded write window; recapturing only frame 18000 cannot establish the
  asset writer or source pointer. A third, longer bounded transition attempt
  (`title-nbg1-transition-owner-20260909b`) reached the same frame from the
  hash-verified JP retail CUE and BIOS and produced the same raw SHA-256;
  its frame-16000..18000 writer traces remained empty. This is useful
  confirmation that the late NBG1 state is stable, not evidence of a writer
  or asset join. Target the earlier transition rather than enlarging this
  late window or treating the repeated state as native-renderer admission.
  The narrowly bounded retail run
  `title-vdp2-owner-transition-13450-20260909` now identifies the immediate
  transition writer: at frame 13459, SH-2 PC `0x060856f0` writes VDP2 BGON
  (`0x180020`) as `0x0002`, enabling NBG1 from the RAM table word at
  `0x06095298`; the frame-13460 witness is hash
  `f700206629eaf6f37dd750271b3a76371d11bd80a2b1abcf79224337f8dbb672`.
  The capture also binds the visible NBG1 name-table and character lanes,
  but the table did not initially establish a final title asset identity.
  Treat `0x060856f0` as an observed generic VDP2-table copier, not as title
  asset ownership.
  That RAM-table provenance trace is now present in
  `title-vdp2-table-provenance-20260909`: the state word at `0x06095298`
  is updated by retail SH-2 code `0x06084784..0x06084956` with source LBA
  `0x17c9` (6089), the terminal sector in the already verified TITLE.BIN
  MAPD plane-transfer range. The separate real-media MAPD verifier confirms
  its five plane transfers at frames 13294, 13334, 13375, 13415 and 13455.
  This establishes a CD→RAM→VDP2 control-table chain for that pre-title
  NBG1 transition, but not a final composed title/menu asset identity:
  retain `asset_consumer_identity=unbound` and trace the later NBG1 bitmap
  state's distinct table/producer before enabling native title presentation.
  The current native integration has an additional fail-closed ownership gap:
  `Nexus_V1_Engine.startup_title_vdp_capture_verified` is consumed by the
  launcher receipts but has no assignment site. Add an authenticated capture
  loader that verifies the selected retail-disc identity, frame/timing,
  VDP1/VDP2, VRAM and CRAM records, then assigns this flag only after the
  compositor's complete receipt succeeds. Do not set it from TITLE.CG,
  manifests, a BIOS, or a host-generated frame.
- Capture the actual interactive title/menu transition and its input contract.
  The JP window at frames 13000–13039 is bit-identical with and without
  Start/A pulses, so it is non-interactive animation rather than the native
  startup menu.
- Resolve the remaining Structure2/VDP1 material, texture, CLUT, raster,
  clipping, animation and composition ownership with real captures. Keep
  unbound bytes and generated fixtures out of production gameplay.
- Implement native Saturn runtime semantics only after each dispatcher,
  material, event, save or audio consumer has a captured, hash-verified
  contract.
