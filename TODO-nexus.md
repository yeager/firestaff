# Firestaff TODO — Nexus

Reviewed 2026-09-29. Only open work is listed here; completed evidence belongs
in the Nexus capture and reverse-engineering records.

## Available local retail media

The staged JP retail set supplies the CUE, nine BIN tracks, CDDA WAV tracks,
and the original ZIP package.  The disc itself does **not** contain a Saturn
VDP1/VDP2 VRAM dump, CRAM dump, register snapshot, or frame/timing capture.
Firestaff reads the CUE/ISO members directly in memory; it must not manufacture
either a capture or a presentation claim from those disc files.

The native title renderer consumes the retail `TITLE.BIN`/`TITLE.CG` MAPD
planes, selector sequence and BGR555 palettes directly from the selected CUE/
BIN media. `test_nexus_v1_title_mapd_real`, `test_m11_nexus_startup_gate` and
`test_m11_nexus_startup_runtime_handoff` cover this bounded path. The renderer
does not authorise the separate menu, face, HUD or dungeon compositors.

The French `TITLE.BIN` now has a separate exact SHA-256 corpus profile and a
real-CUE regression for its 60 RES* directory entries. The receipt preserves
observed IDs, offsets, record heads and chain bounds only; regional menu
rendering remains blocked.

Four authentic 32 KiB Saturn Backup RAM images with a `DMNEXUS__01` entry
are available in the private real-data corpus. The savegame editor now reads
their block chains and exposes each exact 20,480-byte payload as read-only
hex and can compare same-name entries from another image using payload-relative
byte offsets; it does not assign payload fields or allow rewriting. The BKR
self-test accepts only these four filename/SHA-256-pinned captures, while
`nexus_v1_saturn_bkr_real_data` validates their container structure and skips
when the private sample corpus is absent. Native Saturn save/load compatibility
remains blocked until the payload schema and retail consumer are source-bound.
Across the authentic two-, three-, and four-champion samples, the big-endian
value at payload offset `0x0a` is `0x0274 + champion_count * 0x00e0`, and the
payload tail from that value is zero in all four images. This narrows the
observed count-correlated boundary but does not establish record or field
semantics; trace the retail BUP save/load consumer before decoding or writing it.

An end-to-end check against the installed English Saturn CUE on 2026-09-27
confirms that `--game nexus --platform saturn` opens the original disc and
reaches `phase=nexus-title` with `titleReady=1` after 140 frames. Sending Enter
after 500 title frames still leaves the process at `nexus-title`, with no menu
or level loaded; the startup receipt reports `blocker=faces`. The separate
retail `FACE.BIN` decoder passes for all 20 authenticated portraits, so this
is not missing user media or failed portrait decoding. The remaining join is
PLRD roster row to FACE.BIN ordinal: `nexus_v1_champions.c` deliberately leaves
each `portrait_index` unknown (`-1`) because no Saturn consumer trace proves
the ordinal mapping. CLI launch to the title is therefore verified, but the
interactive Nexus start-menu handoff remains blocked until that real-media
mapping and the corresponding menu consumer are captured.

A same-revision, media-immutable title-session receipt now joins retail CD
FIFO records for LBA 6063--6089 to SH-2 RAM source writes for the same range,
cached SH-2 reads, and frame-stamped VDP2 writes/registers for frames
13294--13455. This closes title CD-to-RAM-to-VDP2 provenance only; it is not
menu, HUD, dungeon, audio or input evidence and does not widen the native
presentation boundary.

- Capture the actual interactive title/menu transition and its input contract,
  including the display consumer that places menu and face material.
  The JP window at frames 13000–13039 is bit-identical with and without
  Start/A pulses, so it is non-interactive animation rather than the native
  startup menu. A later same-session control/Start pair establishes that
  Start is read from the live controller register at frames 18020--18023 by
  SH-2 code, but its 64-frame VDP1/VDP2 output remains byte-identical to the
  no-input control. That reader is therefore not yet an admitted menu
  consumer; find the later transition that changes presentation and bind it
  to the retail menu asset consumer.  A separate post-title control/input
  pair at frames 30000--30119 exercised Start, A, B and C in four distinct
  20-frame pulses.  All eight captured video regions (VDP1 state/VRAM/frame
  buffers/draw selector and VDP2 registers/VRAM/CRAM) were byte-identical to
  the no-input control.  The observed active VDP1 state has two stable
  texture sources for frames 30000--30037, but remains an unbound hardware
  observation rather than proof of a menu, HUD or dungeon consumer.  Search
  beyond this window for the first input-correlated presentation change.
  A further matched JP control/input pair at frames 40000--40079 scheduled the
  remaining Up/Down/Left/Right/X/Y/Z/L/R masks (each for two frames). The
  80-frame raw VDP1/VDP2 capture matched the no-input control byte-for-byte
  (`627a1055274abbd164bc1bab7a9253ae2760e0a1a039ef0c587748ff10083681`),
  and frames 40000 and 40079 rendered identically; the input-event receipt
  confirms only that the external hook scheduled the masks, not that Saturn
  code consumed them. This later window adds no menu/gameplay semantics; the
  first input-correlated presentation change remains open.
  A fresh cold-start matched pair then captured frames 0--279 on the
  authenticated Japanese BIOS/disc: Start was requested at 140--199 and A at
  210--239 in the input run; the control forced no buttons in those windows.
  The SMPC `0x10` read at RPC `0x060103e6` returned `0x10` in the input run
  and `0x00` in control, but all ten sampled rendered frames and the complete
  280-frame raw VDP1/VDP2 stream were identical (both raw SHA-256
  `ea2eb96dc56ce9505d67062b2a5f98141d2a456d413c74c19997c04346e27af3`).
  This confirms delivered input and a live reader, not action semantics or a
  presentation change; startup/menu admission remains blocked. A later
  matched frame-10500 WorkRAMH pair and retail CD-to-RAM source receipt now
  bind the relocated reader PCs to an exact 512-byte `TM.BIN` span at file
  offset `0x4500` (destination `0x06014500..0x060146ff`). The authenticated
  instruction stream reads SMPC OREG and a Start pulse changes six bytes in
  the matched input/control memory snapshots. It still does not map those
  bytes to a Start action or menu transition; see the JP controller-reader
  section in `docs/NEXUS_RUNTIME_CAPTURE.md`.
  A complete 52-frame matched pair then scheduled 13 two-frame button pulses
  and sampled WorkRAMH every four frames. All eight VDP1/VDP2 regions remained
  byte-identical. Two corresponding RAM ranges differed, but their values did
  not uniquely track the 13 masks; this is not a button/action mapping. A later
  195-frame pair used 12-frame holds, three-frame releases and a
  WorkRAMH snapshot every frame. It still produced no VDP1/VDP2 presentation
  change; RAM differences persisted through release frames and did not bind all
  masks to actions. A later independent 60-frame Start-only pair joins the
  mask to transient controller-state bytes (`0x0602c90c`); a frame-stamped
  SH-2 read/write follow-up observes `0x1000f001` versus `0x0000f001` at
  `0x0602c90c` and related byte writes, but the raw VDP streams and all four
  sampled renders remain identical. This is controller-state evidence, not an
  action or menu dispatch; two other RAM bytes remain different through all
  release frames. A widened one-frame pair also reaches TM.BIN state code at
  `0x0601462c`: authentic JP TM.BIN compares the byte read at `0x0602c940`
  with `1`, but that fetched value is identical in Start and control; only a
  sampled register differs, while raw video remains identical. This is not a
  Start-dependent dispatch or menu receipt. Continue tracing the controller
  state into a source-bound event consumer and capture an actually interactive
  title/menu interval before opening menu admission; see
  `docs/NEXUS_RUNTIME_CAPTURE.md`.
- Resolve the remaining Structure2/VDP1 material, texture, CLUT, raster,
  clipping, animation and composition ownership with real captures. Keep
  unbound bytes and generated fixtures out of production gameplay. In the
  retained authentic 701-frame JP Saturn window, frame 700 has 249 linked
  VDP1 records; the current compositor resolves 225 of 242 draw commands and
  correctly keeps semantic admission blocked for the 17 unowned draws. The two
  nonzero mode-1 spans at VRAM offsets `0x58b58` (344x177) and `0x58c58`
  (88x177) have no exact or word-byte-swapped match in the 156 files under the
  TRV2 user's `.firestaff/data/nexus` directory. The Japanese retail
  `MENU.BPK` there matches its verified SHA-256
  `740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636`; the
  real-media surface-class test passes and the PRS3 decoder succeeds on all
  162 PRS3 surfaces, but a capture-CLUT/palette-color remap and 4-bpp repacking
  still produce no exact join for either span. Thus neither source ownership
  nor palette, placement, or the original Saturn consumer is established.
  A frame-by-frame pass finds the `0x58c58` source in 287 command lists and
  the `0x58b58` source in 289, spanning relative frames 97--700; each pixel
  span has exactly one SHA-256 across the window, and both commands share one
  stable captured CLUT hash. Their raw signed command coordinates are
  `(-3701,-3701)` and `(-3733,-3733)`; local-coordinate transforms and actual
  visible placement remain unverified. This proves recurring stable runtime
  bytes only, not retail provenance or composition.
  A separate hash-bound cold-start capture at runtime frames 13197--13897 did
  not reproduce either source offset in any of its 701 frames; frame 700 has
  only one mode-0 source at `0x415a0`. Its ten-million-row VDP1 write-trace
  budget filled before frame 0, so that run cannot bind writes to the captured
  window. Recover or reproduce the exact retained runtime window before using
  a fresh writer trace to claim source upload provenance. The external
  producer now supports inclusive absolute-frame bounds; set them to
  `skip_frames` through `skip_frames + frame_limit - 1` so earlier startup
  writes do not consume the selected capture window's trace budget.
  The bound pre-capture prefix did record one write at each target start from
  PC `0x0601307c`, but the corresponding 96-byte live code window has no exact
  native- or word-swapped match in the authentic 156-file corpus, including
  `DM.BIN` at its verified load base. Trace the helper's SH-2 RAM reads and
  inspect its runtime caller in a frame-aligned session before assigning that
  writer or either span to a retail resource.
  Keep both spans blocked until upload provenance or an exact transformed
  retail-surface join is captured.
- Implement native Saturn runtime semantics only after each dispatcher,
  material, event, save or audio consumer has a captured, hash-verified
  contract.
