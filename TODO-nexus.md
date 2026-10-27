# Firestaff TODO — Nexus

Reviewed 2026-10-27. The items below track open work and include completed
diagnostics as context. Verified evidence is also recorded in the Nexus capture
and reverse-engineering records and in [DONE-nexus.md](DONE-nexus.md).

The C Saturn BKR container parser is now verified against all four
hash-pinned samples reported as retail-written; see DONE-nexus.md. Remaining Continue work is the
source-bound payload schema and retail save-selection/load consumer. Do not
promote the parser's opaque payload bytes into runtime state without joining
those bytes to the authentic Saturn save/load implementation.

## Current evidence boundary

The current workstation's authentic `~/.firestaff/data/nexus/MENU.BPK` has
SHA-256 `f2f78dddfe37a5ff414775ae888f164624e987059934b034ba36299cc769d2ca`,
the English retail revision admitted by
`scripts/analyze_nexus_vdp1_source_join.py`. The deterministic Ymir title-menu
capture below uses the Japanese disc and Japanese BIOS. Its VDP1-to-resource
join therefore cannot establish source ownership for the installed English
menu archive. A separate 2026-10-27 Ymir capture now uses the authentic English
retail disc and USA 1.01a BIOS. Ymir reads `0DMSTRT.BIN`, then logs a CD Block
soft reset; the run has no `DM.BIN` read. Its frame-12467 VDP1 snapshot contains
168 command rows, but the last command's CMDLINK target is absent and the list
has no END command. Against the installed English `MENU.BPK`, 86 of 162 PRS3
surfaces have direct byte matches in VDP1 VRAM; none of 132 textured command
source starts falls within those ranges. This proves residency only, not that
the surfaces are drawn or that the title menu is displayed. The VDP2 write
trace dropped 1,376,570 rows, and SCU DMA overflow is unknown, so neither gives
complete transfer provenance. The English renderer gate remains closed. The
capture metadata and reproduction details are in
[`docs/nexus_startup.md`](docs/nexus_startup.md#english-retail-boot-attempt-and-menu-asset-residency-2026-10-27);
raw capture files remain on the external evidence volume. Keep the Japanese
capture as separate regional evidence.

2026-10-27 TRV2 Firestaff v3.0.415 direct CLI probe against the authentic
Japanese Saturn corpus selected `DM.BIN` (MD5
`e88d60859f65f08fa622e1992b02280f`). After 110 boot-probe frames and one
scripted Enter, the receipt reported `titleReady=1`, `levelLoaded=0`,
`menu=0`, and `menu-bpk-prs3-capture-required`. The real-data
`test_nexus_v1_native_menu_mouse_title.sh` also passed through the visible M12
mouse flow and stopped at the same title gate. These SDL dummy runs prove
authentic data selection and title handoff only; they do not prove an in-game
Nexus menu, save load, or playable level.

An older 600-frame Mednafen capture on the authentic English merged disc
(`run-authentic-merged-menu-20260811`, disc SHA-256
`5ab2fbd572d454e4a9df024e2bb6b3cd30f84018c3e650bd945bd52742edd83d`) does
not close that gap. The source-join analyzer found one VDP1 direct-colour draw
and no exact match against the 126 hash-verified retail files at each sampled
frame 299, 300, 301, 320, 360, 400, 500 and 599; the source remained unbound.
The separate authentic English-disc capture
(`run-authentic-english-menu-20260811`) records manifest disc SHA-256
`f3575af985cadbecc74edda0c51451ffeea775054ec5fcdd7c4f960dcdc0cc17`. This
also does not identify a retail source for its sampled direct-colour draw.
Against the installed English Nexus data, frames 0, 299, 300, 301, 320, 360,
400, 500 and 599 each produced one unbound draw and zero exact matches across
126 hash-verified files. The manifest skips 1,200 emulation frames, captures
600 frames, and schedules Start at emulation frame 1,500; the input patch's
frame counter is not reset by the capture skip, so the press falls within the
capture window, around capture frame 300. The raw VDP1/source join still does not
identify the retail source or establish whether the input caused a visible
title-to-menu transition. Keep this separate from the English merged-disc
negative result; neither capture authorizes renderer admission.
An earlier cold-start capture requested 400 frames but retained only 48 before
its 45-second timeout, before its scheduled Start input at capture frame 300.
These are incomplete or negative provenance observations, not menu or
renderer evidence. Keep the Nexus menu gate closed until a complete,
session-bound English capture identifies the retail VDP1/CLUT composition and
interactive title-to-menu transition.

PR #19 merged the corrected NBG1 TileFetcher address path and a scroll-offset
regression. Its focused test verifies the bounded name-table fetch behavior;
this does not bind the
NBG1 data to FONT256, MENU.BPK or a retail text/display consumer, and does not
change menu, HUD, viewport or production-readiness gates. Keep the next work
focused on a same-session retail source-to-display join and the interactive
title-to-menu transition.

The 2026-10-26 Mednafen `.mca` startup capture is not evidence of a loaded
authentic backup card. Mednafen 1.32.1 uses the `mca` suffix for its autosave
state via `MDFNMKF_STATE`/`filesys.path_state`; `filesys.path_savbackup` is a
separate backup-file path. The different capture hash and `Cart: Backup
Memory` line do not identify loaded card bytes. Re-run with the authentic
Mednafen save-memory file under verified `filesys.path_sav`, and retain its
digest, effective settings and a same-session load/read receipt. The local
Firestaff `.bkr` is a different format and cannot substitute.

A retained Ymir v0.4.0 Japanese title-menu frame received a read-only source
join on TRV2 against the authentic Japanese CUE/ISO members. Its hash-pinned
VDP1 VRAM (`0f484877f5c5ef300fcbcaec6948b50c8b7aed7945e448a9f8281b2da46a2f21`)
contained 137 active texture draws: 124 indexed image/CLUT pairs from
`LEV00.DGN`, eight direct-color spans from `LEV00.DGN`, and five `TITLE.BIN`
spans, with zero active draws left byte-unbound. Of the 124 indexed draws, 108
matched a static FACE selector and 16 did not. This narrows authentic source
bytes for one emulator frame; it does not establish SH-2 upload ownership,
semantic use of the 16 selector-free draws, input progression, full display
composition, or production renderer permission. Preserve the current startup
gate. The analysis ran against the authentic media and raw capture retained on
TRV2; the VDP1 dump is intentionally not copied into this repository.

The 2026-10-24 direct-CLI check against authenticated Japanese retail
`DM.BIN` confirms the same remaining start gap: the title reaches its ready
frame, but no menu or level is loaded and the startup receipt remains blocked
on the PRS3 menu capture. See
[`parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md`](parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md).

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
  observation rather than proof of a menu, HUD or dungeon consumer. Search
  beyond this window for the first input-correlated presentation change. A
  separate interactive Ymir run on authentic Japanese retail media shows
  F/Start opening the title menu, S selecting Continue, and J/A confirming
  New Game into the dungeon. A separate ShuttleMouse run shows a button event
  advancing the title into the intro. These establish retail input behavior,
  not Firestaff startup parity or the menu/face display consumer; see
  `parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`.
  A further matched JP control/input pair at frames 40000--40079 scheduled the
  remaining Up/Down/Left/Right/X/Y/Z/L/R masks (each for two frames). The
  80-frame raw VDP1/VDP2 capture matched the no-input control byte-for-byte
  (`627a1055274abbd164bc1bab7a9253ae2760e0a1a039ef0c587748ff10083681`),
  and frames 40000 and 40079 rendered identically; the input-event receipt
  confirms only that the external hook scheduled the masks, not that Saturn
  code consumed them. This later window adds no menu/gameplay semantics; the
  first input-correlated presentation change within these raw capture windows
  remains open.
  A fresh cold-start matched pair then captured frames 0--279 on the
  authenticated Japanese BIOS/disc: Start was requested at 140--199 and A at
  210--239 in the input run; the control forced no buttons in those windows.
  The SMPC `0x10` read at RPC `0x060103e6` returned `0x10` in the input run
  and `0x00` in control, but all ten sampled rendered frames and the complete
  280-frame raw VDP1/VDP2 stream were identical (both raw SHA-256
  `ea2eb96dc56ce9505d67062b2a5f98141d2a456d413c74c19997c04346e27af3`).
  This confirms delivered input and a live reader, not action semantics or a
  presentation change in this matched cold-start pair; startup/menu admission
  remains blocked. A later
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

## Input parity

- Finish Nexus keyboard and mouse gameplay routing. Retail Ymir captures now
  confirm forward movement, left/right turns, Start-to-pause, and the Z cursor
  mode. Firestaff maps A/D to Nexus turns while preserving DM1's A/D strafe
  behavior. Tab cycles the loaded party leader forward and Shift+Tab cycles
  backward in Firestaff's desktop controls. The reverse direction is a host
  keyboard convenience; the native cursor action contract and attack,
  inventory, and spell semantics remain open.
  An independent 2026-10-25 Ymir `--debug` comparison loaded a hash-verified
  authentic JP gameplay savestate into a private profile: the no-input reload
  changed 0.00175% of pixels, W-forward changed 3.08408%, and A-left changed
  5.08610%. The three real captures and source hashes are recorded in
  `parity-evidence/verification/nexus-savestate-input-20261025/README.md`.
  This validates the retail ControlPad response from a stable gameplay state,
  not Firestaff's desktop route. The isolated Xvfb exposed no nonzero SDL
  mouse for Ymir ShuttleMouse, so mouse gameplay remains unverified there.
  Static SH-2 triage is now reproducible with
  `scripts/analyze_nexus_mouse_event_crossrefs.py`: the hash-verified English
  DM.BIN has no direct PC-relative literal references to the 60 event-name
  strings, while its HUD table's subtable at `0x38040` is referenced. This
  scan is supplemented by a verified SH-2 path that registers the hit-region
  table with a callback and later dispatches indirectly through that callback
  slot. A second retail callback trace verifies object fields 100/104 and
  queues numeric events 13/44 to `0x06024288` in two modes. The retail
  pointer table names these `EV_EXIT_INV` and `EV_INV_EQUIP`. The event-44
  path is now traced through its context resolver, slot-pair reads/clears and
  compatible destination writes: it uses context stride `0x13c`, slot-pair
  base `0x0607fba0 + context*0x13c + 0x110 + slot*4`, and 40-byte item
  metadata records. Slot 7 has a separate path. Firestaff only has a partial
  item-ID fit: it lacks the opaque second word, and context-to-champion mapping
  is unproven. The hash-pinned Japanese RLOWFIX scan finds 20 PLRD rows with
  ten pairs each: 64 nonempty item ordinals, 136 empty pairs, and zero in all
  second words. This does not identify auxiliary data for live items. The
  helper that transfers the held pair through global UI state and the visible
  post-click result also remain unverified. The eight authentic action rectangles at
  `DM.BIN:0x3850c` are verified to queue `EV_INV_EQUIP` with slot arguments
  0--7 through the retail bounds scanner at `0x24ed6`. Bind that event's
  actual retail state owner to the authentic champion/inventory model before
  opening the production equipment-click route. The verified consumer at `0x21c84`
  special-cases only IDs 38, 39, 40, 49, 52, 56, and 57; IDs 13 and 44 reach
  its generic `bad event data` diagnostic. The separate mouse poller filters
  for event 46 before it invokes the hit scanner; it does not separately
  consume event 44. Continue tracing the held-item helper and capture a
  controlled click's guest RAM and visible result before enabling pointer
  actions.
  See `parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`.
  See also `parity-evidence/verification/nexus-mouse-callback-dispatch-20261024/README.md`.
  Firestaff now has a host Z toggle, logical pointer motion through keyboard
  WASD/arrows or mouse movement, including captured diagonal WASD movement,
  and the authentic retail hand sprite joined from
  ITEM.IBS image 65/palette 6 against Ymir VDP1 capture. Pointer click actions
  remain open until their retail source joins are captured. All host
  pointer-button routes now use the same parsed retail movement-pad rectangle
  and six-cell desktop mapping; this does not claim Saturn ShuttleMouse
  parity. The authentic callback's click-action semantics remain unresolved.

- Input parity follow-up: Firestaff routes Tab and Shift+Tab to cycle the
  loaded Nexus party leader forward and backward. Authentic Ymir `--debug`
  captures confirm retail L/R changes the visible leader marker and portrait;
  the desktop reverse binding is a host convenience, and the captures do not
  bind the guest RAM owner.
  Retail PLRD has no live/dead field, so Tab treats an available roster row
  with positive health as selectable even while `alive` is unknown, and skips
  zero-health members.
  Keyboard/mouse movement, pointer motion, and the desktop movement pad remain
  available. The hand pointer now travels across the authentic full-game-screen
  hit rectangle, including the HUD, clipped to Firestaff's 320x200 framebuffer.
  Mouse clicks on the hand pointer and non-movement actions remain blocked
  until the opaque item data, champion context mapping and runtime click effects
  are established. See `docs/nexus_input.md` and
  `parity-evidence/verification/nexus-leader-cycle-debug-20261025/README.md`.
