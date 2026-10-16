- 2026-10-15: Reproduced offline selected roots being silently replaced in
  saved settings. Preserve the durable selection separately from admitted
  runtime roots; default recovery no longer overwrites it. Saved /dm1 leaves
  can recover original default media, and return-menu rescan retries a
  reconnected selection, including genuine PC34 ZIP::DATA locators.
  Original FM Towns/PC34 tests pass for display-token repair, unrelated setting
  saves, physical/virtual reconnect and unchanged permission recovery. Desktop
  absence/reconnect tests also pass. No readiness or hash gate is relaxed.

- 2026-10-15: Reproduced the launcher recreating a missing selected data root
  while showing its no-data message. Restricted bootstrap to the built-in
  default root, without a FIRESTAFF_ORIGINALS_DIR override, and normalized
  separators for Windows. Desktop CLI/CTest regression passes for missing
  selected directories, archive paths and process overrides, while ordinary
  default-directory creation still works. No synthetic game media is used:
  these are filesystem absence checks. Catalog validation also passes.

- 2026-10-15: Deferred startup/return-menu scans now publish access-denied
  and no-data explanations on the UI thread after completion/join, reusing
  the existing startup policy. Incomplete/cancelled scans stay quiet.
  Original DM1 archive tests pass for denial beside playable media, granting
  access, renewed denial and removing the last staged archive link. The
  regression also verifies that RunDeferredScan itself does not open a modal.
  Original-PC keyboard and mouse return/close during rescan still pass.
  Catalog validation passes; refreshed catalogs change reference lines only.

- 2026-10-15: Bounded the uppercase DM1 menu-leaf regression to selected
  authentic DM1/CSB/DM2 archives instead of every installed edition. The
  complete original-media startup diagnostic suite passes in 167.34 seconds;
  the isolated optional-media case also finds DM2 beside an empty DM1 leaf.
  Increased the suite timeout from 180 to 240 seconds to allow slower media
  I/O without weakening its timed-intro or game-selection assertions.

- 2026-10-15: Refreshed startup-menu catalog source references after the
  diagnostic/runtime edits. Verified all 21 changed POT/PO files differ only
  in source-reference lines; translation text is unchanged. po/update.sh
  --check passes, addressing CI run 38798285559's stale-catalog failure.

- 2026-10-15: Added opt-in --debug DM1 source-outcome/Draw/Present-return
  stage diagnostics. Fresh outcome identity prevents stale pipeline results
  from masquerading as movement; accepted moves, turns and blocked commands
  remain distinct. No timestamp is captured when diagnostics are disabled.
  Original PC34/A20/demo/ST material tests pass, including the new outcome
  serial/timestamp assertions. The original-media CLI regression also passes:
  repeated full redraws preserve unique movement/turn/block outcome reports,
  stage durations add up, and ordinary runs emit no motion diagnostics.
  Physical input/FIFO/scanout latency is unmeasured.

- 2026-10-15: Removed per-frame diagnostic RGBA hashing from ordinary DM1
  presentation and startup tracing. Full capture requests remain fresh;
  lightweight/full receipt regression and original-media RGBA oracles pass.
  DM1 launcher capture clear/reimport and original-PC keyboard/mouse return
  during the queued originals rescan pass; close completes in both cases.
  The complete original-media startup diagnostics suite passes for DM1, CSB
  and DM2, including FM Towns defaults and uppercase menu data leaves.
  Native Metal CPU fell from about 90.5 to 31.7-32.1 percent of one core
  at 62.5 fps. This does not establish physical Retina or visual parity.

- 2026-10-15: Verbose/debug startup diagnostics now query the actual SDL
  renderer name, VSync interval and drawable/window scale after saved settings
  and CLI overrides are applied. Native Metal checks with isolated settings
  report VSync 1, then 0 with --no-vsync, and a 1x drawable. The complete
  original-media startup diagnostics suite passes for DM1, CSB and DM2;
  its ordinary PC route also requires the renderer diagnostic fields.
  Renderer identity does not measure GPU utilization or physical Retina
  behavior, and the existing viewport-dirty counters do not measure latency.

- 2026-10-15: Reuse the completed RGBA row when nearest CPU upscaling
  repeats a source row. This is an intra-call optimization; palette state,
  sampling, alpha, output dimensions and screenshot contracts are unchanged.
  Authentic PC34 HoC frames pass every RGBA byte at 640x400, 1920x1080,
  853x479, 157x93 and 3840x2160 against independent floor-sampling/palette
  oracles. The full original PC/early Amiga/Atari suite also passes without
  skips. A matched Cocoa full-redraw run at the actual 1920x1018 drawable
  reduces CPU from 96.6-96.8 to 90.5-90.6 percent of one core, with mean
  compose/present cost reduced from 15.09-15.15 to 13.97 ms. The 4K buffer
  check is not physical HiDPI evidence; GPU load remains unmeasured.

- 2026-10-15: Correct host-loop pacing on SDL 3.2+ by measuring the
  remaining frame interval in nanoseconds and using SDL_DelayPrecise.
  The original source tick and startup VBlank policies are unchanged.
  A matched original PC34 HoC full-redraw run at 640x400 with SDL dummy
  output improves from 54.29 to 62.50 rendered/presented frames per second
  against the unchanged 62.5 target; CPU rises from 28.3 to 30.9 percent
  of one core. The five-second corrected window has zero over-budget loops.
  This proves host-loop pacing, not physical HiDPI/GPU scanout or complete
  input-to-display latency. Older SDL headers retain the existing wait path.

- 2026-10-15: Inspected ScummVM's current DM engine at pinned revision
  5eefba1a2cd412016e83a89eaae31376a95c9367 as a secondary reference.
  Detection, graphics-format endian/version branches, separate material
  banks, dungeon decompression and per-square D0 drawing corroborate the
  corresponding original-source rules. Recorded targeted comparison work
  and known non-original approximations in TODO-dm1.md. This was a source
  audit, not a ScummVM runtime or cross-engine parity test; no third-party
  implementation was added to Firestaff.

- 2026-10-15: The PC C127 front-mirror backing now follows the same
  selected C696/F0635/F0791 material path as ordinary wall ornaments.
  F0107's D1C branch uses native C346 at 48x43, centered at (88,29),
  without the host's former 64x43 stretch or invented D2 palette remap.
  The independent original-data test follows real inputs to (14,3,N),
  selects original C127 ordinal 5 and checks the native frame and C026
  portrait at (96,35), including VGA-bank ownership. Other editions'
  front-mirror paths and the legacy shared plan/query API remain separate
  work; this does not establish complete HoC or emulator parity.

- 2026-10-15: Correct F0107 wall-ornament material selection: G0190
  addresses derived cache entries, not native side/front graphics; D1 uses
  the unscaled bitmap. PC34 resolves all nine coordinate sets through the
  selected original C696 records and F0635/F0791 clipping. Early A20E/ST12
  use their native 121-based bank, selected G0194/G0205 frames and palettes,
  with the original padded-row shrink/flip rules. Independent original-data
  raster checks pass 120 PC and 104 cases per early native edition. A real
  recruitment/pickup/throw replay at HoC (4,15,N), tick 301, verifies PC
  graphic 323 at 16x19 and native graphic 197 at 32x48, including every
  visible opaque source pixel and original program RGB. This is original
  media/source evidence, not frame-aligned emulator or full viewport parity.
  The explicit PC inscription test also resolves selected C696 geometry
  for original map 1 text at wall (30,2): view 3 draws graphic 260 at
  (95,71), 39x13. Preserve side/depth receipts through the final readable
  text repaint; frame entry still clears both surfaces before every turn.

- 2026-10-15: F20 wall materials now use native G2002 aliases and G3019
  zones, mirroring the complete bitmap before selecting source columns.
  Remove the nonexistent F20 D2-outer fallback. C255 follows its nine-wall
  F0128 order, including D0/D3 outer panels, opaque floor and the source's
  even-parity mirrored ceiling. Both authenticated EDM/JDM owners pass all
  30,464 C255 indexed pixels against an independent source oracle. Full
  gameplay Draw at the real HoC start and one accepted step verifies D0's
  full 136 rows, exposed D2L aliases, both parities and original program RGB.
  Native JDM C007 is presented at y31, ending before C015 at y167; EDM
  retains y33. JDM viewport clicks alone translate to the internal working
  page: original boundary rows, a real C127 mirror's top-edge click and the
  debug control strip pass; native pointer/HUD coordinates stay intact.
  Other gameplay materials, inventory, native F20 throw-zone selection and
  original-emulator alignment still need separate verification.

- 2026-10-15: Early Amiga/Atari F0108 floor ornaments now use native
  M616=247, the selected graphic 558's coordinate/frame selectors and
  original padded-row blits. Expand compressed dungeon metadata in bounded
  RAM before reading ornament lists; compressed bytes previously selected
  unrelated graphics, producing the cyan/orange floor rectangle. Each
  original A20E/ST12 passes 180 independent full-raster ornament cases;
  actual recruitment/pickup/throw replays verify all 217 opaque native
  puddle pixels in the affected rectangle. Previous object/projectile,
  door/stair, owner-rebinding, Hall-walk, demo and PC wall checks pass.
  Native wall/door ornament banks and complete viewport parity remain open.

- 2026-10-15: A36 C255 now follows the original F0797 map and F0128
  wall order, including outer panels and terminal D0L/D0R, without the PC
  diagnostic walker's global bitmap dependency. Floor/ceiling are opaque;
  walls retain C10 transparency. The original-media regression compares all
  30,464 indexed pixels and the admitted native record mask 0x1eb78003.
  Archive CLI, installed-directory CLI and M12 checks complete the 64 door
  steps and fifteen original driver fades. This verifies the entrance scene;
  original-emulator frame alignment and full gameplay parity remain open.

- 2026-10-15: Early Amiga/Atari positive F0142 thrown-object aspects
  now bind to their selected owner's G0209/M612=360 material at admission,
  use G0218 set0 X visibility, and render through the shared native F0115
  object raster with Y47. Projectiles apply no floor pile shifts and publish
  neither floor/alcove receipts nor clickable floor targets. Each original
  A20E/ST12 suite now checks 900 full-raster floor/alcove/projectile cases
  (including source scaling, flip, palette changes and C10), plus same-state
  owner rebinding. Actual original-world replays recruit champion ordinal5,
  pick up waterskin Thing0x280b, and throw it into live C14 slot0 at (4,14),
  cell3, tick301. The resulting native graphic367 is 48x13 at (53,68), with
  exact source pixels and original RGB4/RGB3 row0 colours. Final bank1 is
  host compatibility transport, not an Atari hardware-bank claim. Previous
  native door/stair, real Hall-walk and PC wall checks remain green. This
  verifies positive object aspects; the separate native G0210 projectile
  family, energy-dependent scaling and original-emulator frame parity remain
  open. A separate cyan/orange lower-floor rectangle remains in the actual
  Amiga cast capture outside the verified projectile. No party/map fixtures
  or projectile-physics changes were used.

- 2026-10-15: Replace PC34 TITLE's borrowed PC98/Towns five-unit minimum
  and 70 Hz scheduler with its default IBMIO contract. ANIM assigns each
  raw EN/DL counter before decode; IBMIO subtracts three on the BIOS/PIT
  cadence (65536/1193182 seconds). The bounded rational deadline helper
  resets each frame, includes decode/presentation overrun, and uses the
  focus-paused active clock without accumulating negative counter debt.
  The authentic TITLE has all 53 raw attributes equal to one. Original
  TITLE/IBMIO CTest, zero/max-counter, IRQ-boundary, overrun, active-clock
  pause and overflow checks pass. A completed original CLI startup presents
  all 53 frames in 2890 ms, keeps the exact previous final RGB image and
  retained palette, and reaches SELECTOR/runtime. Historical initial IRQ
  phase and original CPU decode costs remain unmeasured; this is default
  driver scheduling fidelity, not frame-aligned emulator timing parity.
  Original driver -D polling behavior is not implemented by this scheduler.

- 2026-10-15: Correct PC34 TITLE's `-ah -as` lifecycle. The original
  IBMIO launch table selects a start erase and retained final palette;
  the previous terminal wipe incorrectly treated S as E. Run the source
  LFSR erase before TITLE and retain its final presentation until SELECTOR.
  The authentic TITLE/IBMIO regression checks 53 frames, exact program
  identity and both original TITLE/END flag strings. Presented CLI captures
  compare the final and retained TITLE image byte for byte. This does not
  establish PC34 callback/decode timing: the existing pacing remains a
  documented host model pending original-program calibration.

- 2026-10-15: Early Amiga/Atari F0115 floor and alcove objects now use
  their original M612=360 bank, six-byte G0209 aspects, G0218 coordinates
  and G0213/14 palette changes from the selected owner's graphic 558.
  Native F0132 uses original byte widths, centre sampling, mirrored padding,
  cell-specific pile shifts, clipping and C10 transparency. Metadata is
  invalidated on every new launch; original A20E -> S12E -> A20E starts
  reuse one state and recheck its native tables. Each retail original passes
  500 isolated full-raster object cases across five object aspects, ten
  view squares, floor/alcove cells and two pile phases. The shared G0237
  object-aspect lookup was compared with all 180 ReDMCSB source entries.
  Real 35-step walks from both original starts reach (4,15,W): the floor
  waterskin is now graphic 367 (48x13), drawn at (137,149), replacing the
  unrelated creature 505 (64x89) previously projected as a cyan rectangle.
  Original hashes and prior native door/stair, PC wall and demo controls
  remain intact. These material and actual-world checks do not establish
  full viewport parity. The positive thrown-object route was subsequently
  verified as described above; the native G0210 projectile family remains open.

- 2026-10-15: Apply native PC34 C710/C711 D2 geometry to the real
  catalogue startup route as well as direct-dungeon starts. The previous
  format guard only covered the latter; its isolated endpoint test missed
  the real catalogue route's old 75x71 crop. Original full Draw now verifies
  the exposed D2 wall strips at the real start and after accepted movement,
  through both launch routes, with native 78x74 material, y19 and VGA bank
  ownership. The existing endpoint test covers both C710/C711 parities;
  the initial D2R square is a corridor, so it is not falsely asserted as a
  wall. A20E/Atari original regressions pass. The CLI start viewport now
  matches all 30,464 RGB pixels of emulator image0004, whose pose remains
  unproven; this is diagnostic image equality, not complete runtime parity.

- 2026-10-15: Restore F0125/F0126's terminal D0L/D0R wall material
  callbacks in their source scheduler transactions. The original PC34 Hall
  start (1,3,S) and one accepted forward step (1,4,S) each draw both native
  C716/C717 rectangles, including all opaque pixels, both wall parities,
  VGA bank 16 and the current source G9010 RGB palette. Remove the unrelated
  host poison-label fallback clear that erased viewport rows
  33..43. CLI captures confirm both real poses. Original A20E/Atari 1.2
  material and interaction regressions also pass. The old emulator image0004
  has an unproven pose: diagnostic RGB overlap is now 29,805/30,464 viewport
  pixels, with the 659 differences confined to x60..77,y52..123. This is not
  a parity claim. New Towns/A36 live D0 execution evidence remains open.

- 2026-10-15: Desktop A36 readiness and preparation now require the
  authenticated VDEO.FTL driver beside the selected graphics and dungeon,
  preserving the exact loose directory or nested virtual disk. Missing or
  wrong drivers cannot borrow an original from another installation; an
  incomplete first graphics copy cannot hide a later complete copy. Original
  media tests verify scoped/unscoped inventory, exact selected-file rejection,
  duplicate-owner rescue, all three original English/French/German dungeon
  payloads in the existing DUNGEON.DAT role, and missing default files beside
  DungeonF/G.dat. The native receipt test and CLI archive, CLI directory and
  M12 startup routes pass after the final deadline fixes. Low-level media
  APIs retain their documented contracts. Native source-filename language
  switching remains open; localized filenames alone are not marked ready.

- 2026-10-15: Early Amiga/Atari production stairs, door frames and door
  panels now use MEDIA009/020 native records and original G0110..G0187
  frames, padded strides, crops and transparency. Wall sets use their
  separate 13-record wall and 18-record stair families; active map DoorSet0/1
  selects the 108-based three-record door family. Native F0111 composes
  ornaments and masks before state clipping, retains native G0196 lock
  coordinates, and consumes the two original Random(2) animation decisions.
  A20E retail and Atari ST 1.2 each pass 484 isolated full-raster original-media
  cases covering stairs, frames, all four map-owned door sets, states 0..5,
  both opening directions and both D0 axes. Original decoded pixels and
  graphics hashes remain unchanged. The shared DM1 scheduler now reads the
  raw square's facing axis instead of Door.Vertical, which controls opening
  animation. Original PC34/A20E/ST12 worlds each reproduce the mismatched
  axes behind the initial position; two real turns now invoke the expected
  F0111/frame callbacks. Separate A20E/ST12 production-world command replays
  walk 42 original squares, capture the stair view and open the actual button
  door at (1,14) by clicking from (1,15,N), changing its state from 4 to 0.
  These are targeted material and interaction checks, not complete viewport
  parity. The cyan rectangle then observed in the MEDIA009 stair view was
  subsequently identified as a floor-object bank error and corrected above.

- 2026-10-15: A36M entrance now consumes its authenticated C696 layout
  and same-owner VDEO.FTL DATA palette increments. Native 105x161 / 128x161
  doors open in 64 two-pixel steps, with two VBlanks between presentations;
  the older A20 31-step schedule is not used. C011 uses C435/C436, and the
  dedicated F0797 map uses original A36 floor, ceiling and wall pixels,
  independently of the loaded gameplay dungeon. Fifteen-step driver fades
  and exact RGB4 presentation preserve the source colours. The original
  regression verifies exposed native C255 scenery and all 65 closed/opening
  pages against an independent crop oracle. CLI archive, CLI directory and
  M12 routes pass; all twelve captured phase images agree across routes.
  The original archive SHA256 is unchanged. Frame-aligned original-emulator
  comparison, native audible output, complete runtime palette ownership and
  full dungeon viewport parity remain open.

- 2026-10-15: Early Amiga runtime presentation now uses authentic RGB4
  palettes instead of VGA fallback. One shared source table preserves CSB's
  existing values; UI bank zero remains immutable and dungeon banks 1..6
  own the six G0021 rows. The full-width copper middle strip (31..169) is
  tagged before source-frame publication; inventory/champion views use the
  UI palette. Original demo executable bytes verify every RGB8 bank entry,
  all light rows, band boundaries and inventory switching. All activated
  original-media wall/background/control/PC regressions pass. A completed
  same-pose demo capture preserves geometry while correcting 26,342 RGB
  pixels. A36 driver-band ownership, creature replacement colours and full
  palette lifecycle parity remain separate open work.

- 2026-10-15: The entrance HiDPI regression now retains drawable dimensions
  through production mouse mapping, Original 320x200 conversion, hit-testing
  and command dispatch for all five entrance actions. Existing display and
  entrance tests pass at logical 1512x982 -> drawable 3024x1964, plus separate
  X2/Y1.5 scaling. These are geometry tests, not a physical Retina capture.

- 2026-10-15: Early Amiga/Atari production wall endpoints now consume
  native MEDIA009 compounds and G0163 pixel strides, not separate PC wall
  records. F0096-derived buffers preserve shifted D3/D2 flips, D1 flips,
  D0 source swaps, C10 side transparency and opaque center walls. A bounded
  host-owned 53,872-pixel allocation admits cache capability; allocation
  failure consistently retains native walls, without claiming original
  hardware heap accounting. Shutdown and fresh launch release ownership.
  Original demo and Atari 1.2 regressions pass eleven complete wall rasters,
  both parities and both capability states, with immutable source hashes.
  Their completed dungeon captures contain real walls, background and
  controls. Native 28-pixel turn-button dead strips are also corrected;
  original center/outer-strip clicks pass on both editions. Other editions,
  native palettes and complete frame parity remain open.

- 2026-10-15: Restore early Amiga F0098 floor/ceiling records 75/76 and
  their native 224x70 / 224x29 parity regions; A36 keeps its later order.
  Restore the MEDIA009 C013 movement panel at (224,124), 96x45, for early
  Amiga and Atari instead of rejecting it against PC's 87x45 dimensions.
  The production original-data test passes both complete A20E background
  rasters, every Amiga-demo and Atari 1.2 arrow pixel, and the existing PC34
  D2/F0107 checks. No generated game raster or fixture map is used. Legacy
  wall-compound composition and native Amiga runtime palettes remain open.
  A completed original-demo capture at the same (7,1,S), tick 120 changes
  from a black page to visible background and controls (17,790 nonblack
  viewport pixels). This does not verify missing wall layers.

- 2026-10-15: `--debug-full-redraw` now performs source Draw work each host
  frame, rather than presenting a retained page. Debug telemetry separately
  counts source draws and successful presents, and compose/present timing
  includes preceding source composition. The existing original PC34 perf
  block passes: dummy SDL samples contain 275/276 real draws and equal
  presents per five-second window (54.94/55.08 draws per second), with
  source ticks near 5/s. This verifies instrumentation, not physical-display
  60 FPS, GPU utilization or full gameplay performance parity.
  A subsequent native Cocoa run on Apple M4 passes three stable 60.00
  source-draw/present FPS windows with original PC34 media, including
  301 draws and 301 presents over 5.017 s. Its actual 1920x1018 drawable
  uses density 1, so this does not verify HiDPI 2x or GPU utilization.

- 2026-10-15: Fresh accepted DM1 keyboard steps can close the source
  input wait immediately instead of waiting for its idle deadline. A
  read-only preview uses copied command-core state, preserving blocked
  steps, cooldowns, existing pending commands and held-repeat scheduling.
  The original PC34 launcher regression passes 343 checks with zero skips,
  including unchanged party/pipeline/world-tick snapshots after preview.
  Normal-loop keyboard turn-to-step presentation improved from 217 ms to
  16–20 ms on the same original map0 (1,3,S) -> (1,4,S) route. This measures
  command admission and presentation, not GPU utilization or full-render FPS.
  All 20 existing original PC34 startup/ownership cases pass; the corrected
  320x200 motion tail passes keyboard 20 ms and native-pointer 19 ms, with
  original installation hashes unchanged. Atari original startup/forward
  captures remain visible; the Amiga-demo visual limitation is open in TODO.

- 2026-10-15: Repeated loose graphics copies no longer hide a complete
  original installation behind an earlier orphan. Bounded ordinary-file
  enumeration selects a complete same-directory owner before publishing
  DM1 readiness; selected loose files remain terminal even beside canonical
  original ZIPs. Atari and Amiga retail candidates use the existing original
  dungeon catalog within that parent, with the original Amiga SWSH pairing
  (and A22G four-file tuple); demo and PC34 English retain exact dungeon
  hashes. PC34 also retains uniquely hash-identified renamed dungeons in the
  same directory, without recursive neighbor fallback. Original Amiga demo,
  Atari 1.2 English and PC34 English regressions pass CLI/M12 duplicate-owner
  and explicit-orphan cases. The PC34 case-sensitive matrix now passes
  twenty routes, including renamed-dungeon positives, duplicate renamed
  payloads, invalid native filenames and subdirectory rejection. Existing
  Amiga case/ambiguity and Atari unscoped full-startup checks still pass;
  final demo/Atari runtime probes also pass on the latest app. Original
  archive and installation hashes remain unchanged. This verifies media
  ownership and startup, not display/audio parity across platforms.

- 2026-10-15: Correct PC34 D2 side-wall material placement using authentic
  I34E C696 zones C710/C711 and ReDMCSB F0635/F0104/F0105: full 78x74
  images at (0,19)/(146,19), including 77-x horizontal mirroring. The old
  75x71/y20 Atari frame geometry remains on legacy paths. An original-data
  production-endpoint regression checks every framebuffer pixel for both
  sides and parities, including 5,523 opaque pixels and 249 C10 holes per
  image. All four cases pass. This is a specific wall-material correction,
  not a claim of complete dungeon viewport parity.

- 2026-10-15: Restore authentic PC34 TITLE/SELECTOR startup for loose
  installations whose DATA directory or program filenames use other case.
  Before the fix, a byte-identical `data` copy skipped native TITLE and then
  SELECTOR while still reaching runtime; the canonical DATA baseline played
  53 TITLE frames and 32 SELECTOR frames. One selected-graphics companion
  resolver now admits unique actual-case names in that directory or its
  DOS DATA parent. Invalid or ambiguous local programs never fall through to
  another directory, and arbitrary graphics folders cannot borrow a parent's
  SELECTOR. Ten original-media HFSX cases pass canonical/lowercase and direct
  selected-file CLI/M12 routes plus wrong-program, neighbor and ambiguity
  rejection. The original installation's SHA-256 manifest is unchanged.
  The existing uppercase/wrapped archive regression also passes all four
  CLI/M12 routes after the loose-source change.
  These are functional source-startup checks with SDL dummy drivers, not
  native display/audio or timing-parity evidence.

- 2026-10-15: Preserve the unscoped Atari ST 1.2 start-menu route as an
  original-media regression. With no `--game` or `--platform`, isolated
  configuration and unchanged lowercase loose graphics/dungeon files, the
  normal title and entrance finish before active runtime: the source handoff,
  full graphics and first Hall receipts are consumed at map 0 (1,3), with
  positive gameplay ticks and drained scripted input. The complete existing
  case-sensitive HFSX regression passes uppercase/lowercase CLI admission,
  lowercase M12 handoff, denied incidental DM2 inventory, unscoped continued
  runtime and ambiguous-dungeon rejection. SDL dummy drivers establish
  functional startup only. The temporary image was detached and removed;
  the earlier two-second launch failure has no proven cause.

- 2026-10-15: Admit renamed loose Amiga graphics by their recorded original
  identity, rechecking the exact selected file before same-parent companion
  resolution. Archive member naming policy remains unchanged. The authentic
  demo named Maze-art.dat failed M12 before the fix; its new original-media
  matrix now passes directory/file selection through CLI runtime and M12
  SWSH/title/entrance, retaining exact alias ownership and unchanged original
  ZIP/staged bytes. Original A22G renamed German-art.dat also passes four-file
  identity and selected-file M12 into loaded runtime. The A22G archive CLI/menu
  regression passes in 7.10 seconds. Locale catalogs are current. These are
  functional checks with SDL dummy drivers on one Mac, not native platform
  or visual/audio parity proof.

- 2026-10-15: Reuse one private same-directory case resolver for Amiga
  startup companions and Atari/Amiga loose dungeons. Before the fix, unchanged
  original Atari 1.2 DUNGEON.DAT booted but dungeon.dat failed on case-sensitive
  HFSX. The authentic regression now passes both CLI spellings, lowercase M12
  handoff, ambiguous-filename rejection and unchanged input SHA-256s. It also
  verifies that a denied incidental DM2 directory does not hide DM1 in the
  production scan. Original A22G files with GRAPHICS.DAT/Dungeon.dat/DM/Swoosh
  pass four-file identity, source title/entrance and loaded runtime; original
  demo mixed/ambiguous Swoosh regression and A22G archive CLI/menu regression
  pass after the shared refactor (archive CTest: 7.12 seconds). The temporary
  HFSX volume was removed. Other original editions and native Windows/Linux
  runtime, physical input and display/audio parity remain unverified.

- 2026-10-15: Resolve loose Amiga startup companions by their actual
  filename case within the admitted graphics parent. The authenticated demo
  with Swoosh failed to launch on a case-sensitive HFSX volume before the fix;
  it now completes source SWSH/title/entrance through M12. Adding the identical
  original as both Swoosh and swoosh rejects the ambiguous owner and --debug
  reports wanted/parent/reason=ambiguous. The two-case original-media regression
  passes. Archive paths retain their existing readers; other retail loose-file
  tuples and native Linux/Windows runtime remain unverified. The temporary
  test volume was detached and removed after retaining diagnostic logs.

- 2026-10-15: Shared explicit-file scanner regression: unscoped M12 with
  the original nested Amiga 2.0 ZIP keeps its selected outer archive even
  beside other DM1 editions. The source SWSH, all 18 F0437 title frames and
  31-step F0441 entrance complete before the Amiga runtime handoff. The
  ownership filter retains all matching editions from that ZIP and discards
  sibling-package rows. This SDL-dummy check does not establish native
  presentation/input or visual parity.

- 2026-10-15: Fixed loose original Amiga startup: scanning admitted the demo
  graphics/dungeon, but M11 bypassed compressed Motorola dungeon intake and
  required virtual paths for SWSH/title/entrance. Loose admitted media now
  uses the existing bounded memory loader, and runtime preserves the exact
  hash-verified graphics filename. The original demo passes CLI and M12 from
  both ZIP and loose files; M12 requires authenticated SWSH, F0437, F0441 and
  runtime handoff. The regression compares every staged original file against
  its archive after launch. SDL dummy drivers on one Mac do not establish
  native display, audio or cross-platform filename behavior. Existing A22G,
  A36 and PC34 selector original-media regressions passed 3/3 (48.93 seconds).

- 2026-10-15: Admitted the original A22G startup through an explicit four-file
  identity: graphics, dungeon, dm and swoosh from the selected volume. The
  legacy pair gate remains strict. Same-container sibling lookup loads the
  additional files only for the explicit path; debug records A22G tuple
  admission. Original UAE-1ADF tests validate title/entrance and rejection of
  changed dm, missing dungeon and insufficient pair provenance. New direct
  CLI and M12 tests preserve the complete original disk in ZIP transport,
  run F0437/F0441 and reach runtime. Existing original Amiga title/entrance
  checks also pass (four tests, 7.83 seconds total on the Mac host with SDL
  dummy drivers). No game files are replaced or tracked; visual/audio timing
  and original executable state comparisons remain open.

- 2026-10-15: Fixed I34E selector lookup using a case-sensitive .zip search
  and discarding the authenticated member's wrapper directory. An unchanged
  original archive named DM1.ZIP reproduced a selector-skip receipt before the
  fix despite reaching runtime. Lookup now retains the exact graphics locator
  and its final container/member directory. The new real-media regression
  passes uppercase flat ZIP direct/menu and wrapped-folder ZIP direct/menu,
  requiring the 32-frame selector Enter completion and loaded runtime. Original
  member payloads are unchanged; wrappers stay untracked. This is startup
  routing verification on SDL dummy video/audio, not visual/audio parity.

- 2026-10-15: Reverified the original A22G UAE-1ADF file reader. It visits
  12 OFS files and authenticates the same-volume graphics, dungeon, dm and
  swoosh identities. Updated the stale reader-blocked TODO: title/entrance
  admission still needs an edition-bound complete startup tuple. No A22G
  support is claimed merely from successful file extraction.

- 2026-10-15: Fixed I34E title lookup in the authentic nested DM1 DOS ZIP.

- 2026-10-15: Removed duplicate FM Towns entrance wall-source and zone
  tables. The C255 entrance now uses the same source/zone lookup as the
  dungeon host path, preserving its existing floor/ceiling and crop handling
  while removing 34 net source lines. Both entrance-viewport and layout-crop
  tests pass; a rebuilt binary reaches authenticated EDM runtime using the
  original FM Towns archive. No visual parity claim is implied.

- 2026-10-15: Reverified direct `--game` exit on source `8939e58fe` with
  authentic FM Towns media and an isolated configuration. DM1, CSB and DM2
  each reached runtime and exited after Return to Menu without reopening
  the launcher. The shared-library M12 session also launched DM1, rescanned,
  launched CSB, returned and rescanned again; both scans rediscovered DM1,
  CSB and DM2. These SDL dummy-driver checks verify launch/exit/rescan
  behavior on the Mac host, not native display, audio or visual parity.

- 2026-10-15: Restored F20 entrance-door waits to one 60 Hz VBlank per
  step, matching ReDMCSB ENTRANCE.C F0438:230-240. The 31-step unit
  contract is 517 ms. Both English and Japanese original-media startup
  timing probes pass; the complete CLI/menu, EDM/JDM input matrices and
  Hall-of-Champions regression also pass. Scheduler tolerance is distinct
  from source timing.
  Front and side wall host handoffs now use the native F20 layout-696
  route/crop instead of PC34 rescaling. Route/crop tests pass, the app
  builds, and an authentic 720-frame boot reaches a loaded runtime and
  a nonblank presented frame. Remaining wall routes and visual parity
  are tracked in TODO-dm1.md.
  Firestaff now derives the sibling `dungeon-master/dmaster/TITLE` archive
  member from the authenticated `GRAPHICS.DAT` path instead of checking only
  the ZIP root. The original 12,002-byte title played all 53 frames and 265
  source VBlanks in 3,786 ms; the nested-ZIP regression passed. This verifies
  this archive layout's title handoff, not full platform intro or viewport
  parity. See `TODO-dm1.md`.

- 2026-10-15: Fixed PC entrance-door source sampling. ReDMCSB prepares shifted
  C002 left-door frames but cancels the C003 right-door source offset;
  Firestaff samples the raw source images. The raw-frame mapping now uses
  `4 * animationStep` for C002 and zero for C003. The focused composite
  regression passed, including the step-5 source offsets. This closes the
  identified frame-strip coordinate mismatch, not native presentation cadence
  or full title/entrance visual parity; see `TODO-dm1.md`.

- 2026-10-15: Corrected the authentic mixed-library platform test contract for
  PC 3.4. Its I34E DOS route uses SWOOSH/TITLE/SELECTOR and reaches FIRES; it
  does not emit the shared F0437/F0441 Hall-of-Champions receipt. The boot
  probe intentionally skips I34E's visible startup chain, so this platform
  matrix now checks the selected PC34 source identity and loaded runtime while
  retaining the Hall receipt checks for Atari ST and Amiga. The full I34E
  title/selector path remains independently covered by
  `dm1_v1_pc34_native_cli_boot`. The mixed AUTO/CLI/menu matrix passed with
  authentic FM Towns, PC34, Atari ST, and Amiga media (93.61 s). This fixes a
  false test failure. The remaining I34E items are legacy selector interaction
  and first-runtime visual comparison, as tracked in `TODO-dm1.md`.

- 2026-10-14: Restored the I34E PC 3.4 two-bank VGA viewport presentation.
  ReDMCSB `DRAWVIEW.C:F0097` selects the active dungeon palette for VGA
  entries 16–31, and `VIDEODRV.C:F8161` ORs `0x10` into each 4-bit C007 pixel
  while leaving the surrounding screen on entries 0–15. Firestaff had copied
  host brightness bits as if they were VGA color indices and resolved the
  viewport and HUD through one 16-color bank. The presenter now writes the
  source color nibble into bank 16–31, and a tested palette builder maps that
  bank to the selected `G8151`–`G8156` row while preserving the screen bank.
  The F0097 regression, authentic `dm1_v1_pc34_native_cli_boot` (365.44 s),
  `dm1_macos_real_data_boot`, and PO catalog validation pass. These checks
  establish the code path and original-media boot, not a native Retina color
  capture or geometric viewport parity; those remain open in `TODO-dm1.md`.

- 2026-10-13: Closed a PC 3.4 startup-media mismatch in the M12 catalogue.
  The launcher previously admitted original GRAPHICS.DAT and DUNGEON.DAT as
  a playable PC release even when its required SWSH intro file was absent;
  M11 then rejected the launch later. ReDMCSB `APPA.C:51-68` loads SWOOSH
  before TITLE. The scanner now exposes PC 3.4 English and Multilanguage only
  when the authentic SWOOSH hash is found in the same media tree. A negative
  case derived from the retail ZIP removes only SWOOSH and confirms that PC
  3.4 is not matched and direct launch is refused before runtime. The complete
  original-media `dm1_v1_pc34_native_cli_boot` regression passes, including
  loose/default-root startup, M12 launch, all 23 C001 title steps, Entrance,
  Hall recruitment and the native input matrix. No altered or synthetic media
  was used as positive startup evidence.

- 2026-10-13: Re-ran the authentic French Atari ST 1.3
  `dm1_v1_atari_st_fr_archive_cli_boot` CTest four times (65.26, 65.23,
  65.18 and 65.19 seconds). All four passed through direct CLI boot, explicit
  M12 selection, C001 quit, Hall recruitment and first gameplay movement.
  Current source initializes `menuRequested` to zero and documents the
  fallback caused by an uninitialized value, a plausible explanation for an
  earlier no-output incident. The first repository commit is an imported
  source snapshot, so the historical corrective change cannot be verified;
  the incident remains open in `TODO-dm1.md`. SDL dummy drivers do not verify
  physical Atari input or pixel parity. The separate French Atari SND1
  source-index-1 audio rejection also remains open there.

- 2026-10-13: After the shared M11 frame-pacing change, the authentic FM Towns
  test `dm1_v1_fmtowns_archive_cli_boot` passed on the Mac mini M4 in 240.77
  seconds. It exercises original-media CLI startup, title and entrance flow,
  menu selection, and
  movement assertions; this headless CTest result does not verify physical
  display output.

- 2026-10-11: Made the issue #13 mixed-edition case unconditional in the
  authentic PC 3.4 CTest path when the Atari ST v1.2 archive is staged. The
  existing STX reader exports the original Atari `DUNGEON.DAT` for the
  regression, verifies its known hash (`050fb2cfded1b502ec2c53956b94c5bd`),
  and places it before the matching PC dungeon while the PC graphics are at
  the mixed root. The full `test_dm1_v1_pc34_native_cli_boot.sh` passed with
  original PC34 and Atari media, including this selection check and the CLI,
  start-menu, title, entrance, Hall and input routes.

- 2026-10-11: The published v3.0.376 Linux x86_64 AppImage passed direct
  `--game dm1 --platform pc` and M12 New Game against the authentic PC 3.4 EN
  ZIP in a Linux/amd64 userspace under QEMU on an ARM64 Linux VM. Both routes
  loaded the original `GRAPHICS.DAT`/`DUNGEON.DAT` pair and reached runtime; the
  M12 receipt confirms `levelLoaded=1`, `dm1StartupHandoffExecuted=1`, and
  `dm1StartupHoCFirstFrameReady=1`. The v3.0.376 tag predates the later mixed-
  edition fix, so this run does not verify the issue #13 fix. It does not
  establish native x86_64 hardware or physical mouse behavior.

- 2026-10-11: Rebuilt the current macOS source and passed the full
  `test_dm1_v1_pc34_native_cli_boot.sh` regression against the authentic DOS
  3.4 archive. It covered installed loose files, missing-SWSH rejection, bare
  CLI AUTO selection, M12 launch, all 23 C001 title steps, distinct C001 zoom
  frames, C004 Entrance opening, Hall input/C127, C040 and C007 presentation,
  1920x1080 rendering, and the native input matrix. The physical-card route
  reached `DM1 READY`. SDL dummy input verifies the scripted event path, not
  physical mouse behavior or Linux x86_64 issue #13.

- 2026-10-11: Fixed the mixed-installation path behind issue #13. M12 could
  select PC 3.4 English `GRAPHICS.DAT` while M11 independently chose an Atari
  ST `DUNGEON.DAT` found earlier in the same root. M11 now restricts dungeon
  resolution to the PC 3.4 EN hash for that verified graphics edition and
  binds the verified graphics path even when the matching pair is in separate
  directories. The regression uses authentic PC34 and Atari files, removes
  the nested PC34 graphics copy, and confirms both selected paths and a loaded
  runtime. The full `test_dm1_v1_pc34_native_cli_boot.sh` suite passes.

# Firestaff DONE — DM1

- 2026-10-13: Corrected the English/Japanese FM Towns title zoom palette from
  ReDMCSB `TITLE.C:362-367`: the original installs both C13_DUNGEON and
  C14_MASTER before drawing any zoom frame. Firestaff had left color index 15
  at C12's white value during zoom; the source-combined zoom/master palette
  now makes it red from the first zoom frame. The authenticated English and
  Japanese FM Towns startup/palette tests and the title-frame selection test
  pass with original media. A native Cocoa capture on the Mac mini M4 froze
  original-media zoom frame 10/20 after its debug receipt; the corrected red
  title details are visible. The connected display reports 1920x1080, so this
  does not verify Retina/HiDPI output. See
  `parity-evidence/dm1_fmtowns_title_zoom_m4_20261013.md` and its capture.

- 2026-10-13: On a Mac mini M4, a native Cocoa run of the current main build
  with bare `--game dm1 --debug` selected the hash-matched English FM Towns
  `GRAPHICS.DAT` by default. The trace presented all 20 original title frames,
  held the final title for 1,200 ms, and completed the title phase in 1,616 ms.
  After a fresh Return input, all 31 entrance-door frames were presented at
  16–18 ms intervals and startup reached `dm1-runtime`. This verifies the
  title and door pacing change in `907363f93` on M4 and the default FM Towns
  CLI route. The trace records palette stages but does not prove the displayed
  RGB colors or physical HiDPI behavior.

- 2026-10-12: Rebuilt current `main` and passed
  `dm1_v1_pc34_native_cli_boot` in 252.18 seconds with original PC 3.4 English
  and Atari ST 1.2 data. Coverage includes the mixed-root wrong-edition
  dungeon case, CLI and M12 startup, title/entrance/Hall flow, input, and
  1920x1080 presentation. Fix `1cf2b7565` is included in v3.0.377. This local
  macOS SDL-dummy run does not establish behavior on the reporter's Linux host
  or physical input/display parity.

- 2026-10-12: Extended the authentic mixed-media M12 platform regression to
  start with AUTO and select PC 3.4 using keyboard navigation without a
  `--platform` override. The test confirms the PC34 graphics identity and
  reaches the source-owned first runtime frame. The full authentic mixed-root
  matrix passes with FM Towns, PC 3.4, Atari ST 1.2 and Amiga 2.0 archives.
  Re-ran the issue #13 PC 3.4 regression on the current main binary; loose-file,
  archive, CLI, M12 and native-input startup checks all pass. These are local
  SDL-dummy startup checks, not native M5 Retina or physical-input validation.

- 2026-10-11: Added authentic DM1 Amiga v2.0 German startup support. The
  A20G SWSH hash uses the independently verified 5088-byte HUNK_DATA layout;
  the original German GRAPHICS.DAT/SWSH pair now passes the F0437 title and
  F0441 entrance receipts. The source route recognizes the German A20 edition
  alongside English/French. The real-media regression streams the original
  German ADF from the supplied 7z, verifies GRAPHICS.DAT, SWSH and DUNGEON.DAT
  identities, and reaches runtime through direct CLI and the M12 start menu.
  `dm1_v1_amiga20_german_archive_cli_boot` passes. This verifies startup
  acceptance and source-owned choreography; it does not establish visual
  parity or native-display/input behavior.

- 2026-10-11: Re-ran `test_dm1_v1_amiga_v20_archive_cli_boot.sh` against the
  authentic English A20 game and save archives using both the local v3.0.375
  build and packaged v3.0.376 app. Both runs passed the complete CLI, M12 New
  Game, M12 Quick Resume, and movement sequence. Direct and menu resume restore
  the original F0435 state at map 0, party `(4,15)`, direction 2, four
  champions, tick 292. The earlier local failure was not reproduced. Headless
  runtime evidence does not verify native M5 display/input or visual parity.

- 2026-10-10: Added authentic DM1 Amiga 2.0 French launch support. M12 now
  admits the original A20F dungeon checksum, and the runtime preserves the
  complete FTL-decoded map body before adding its parser checksum. The French
  SWSH, F0437 title and F0441 entrance assets are paired by their original
  hashes. Real-media checks prove the French dungeon initializes a playable
  world, crossed/modified media are rejected, and direct CLI plus M12 New Game
  reach the first runtime frame. The English A20 route and Atari ST v1.2 full
  M12/input matrix also pass after the size fix. This is startup/functionality
  evidence; physical HiDPI presentation and pixel parity remain unverified.

- 2026-10-10: The authentic return-to-menu regression now runs normal game
  returns separately from live media mutations. DM1 PC and FM Towns both
  passed their return scans alongside the CSB and DM2 routes; the mutation
  suite passed removal and addition of authentic CSB while DM1 was running.
  Test scratch now remains in the external build directory instead of the
  user's game-data parent.

- 2026-10-10: `dm1_v1_dos_fr_zip_cli_boot` and
  `dm1_v1_dos_en_archive_cli_boot` both passed against their installed
  authentic DOS archives. These regressions verify CLI and start-menu launch
  paths; they do not claim cross-desktop rendering parity.

- 2026-10-08: Rebuilt the local launcher binary, then
  `dm1_v1_amiga_v20_archive_cli_boot` passed in 50.49 seconds against the
  authentic Amiga v2.0 ZIP -> ZIP -> ADF archive. It verified direct CLI,
  M12 New Game, authenticated SWSH/title/F0441 entrance, native save resume,
  and movement. An initial run against a stale local binary lacked the current
  entrance receipt; the rebuilt-binary run passed.

- 2026-10-08: With the current launcher freshly linked,
  `dm1_v1_pc34_native_cli_boot` passed in 248.73 seconds against the authentic
  PC 3.4 ZIP. It covered the real title sequence, M12 AUTO menu discovery, the
  first runtime frame, and rejection of a private archive copy with its
  required SWSH prelude removed. This is headless startup evidence; it does not
  establish physical M5 rendering or visual parity.

- 2026-10-08: The authentic Atari ST 1.1 `.7z` startup test passed in 63.42
  seconds. It verified direct CLI media selection, M12 handoff, the first
  runtime state and the C127 Champion Hall route using the original archive.

- 2026-10-07: All 14 installed authentic DM1 CLI-start CTests passed in the
  multi-edition startup matrix, including DOS English/French, Atari ST
  English 1.0a/1.0b/1.1/1.2, German 1.2, French 1.3, Amiga HD/v2.0 and
  FM Towns. Each available route passed its registered startup/menu assertions;
  this does not establish physical M5 rendering or visual parity.

- 2026-10-07: Extended the authentic return-to-menu rescan regression with
  DM1 FM Towns. The M12 route reaches the first original HoC runtime frame,
  returns to the launcher, and confirms that installed DM1, CSB, and DM2 media
  are rediscovered. The combined DM1/CSB/DM2 CTest passes against original data.

- 2026-10-07: Corrected the M12 no-data regression fixture to use the English
  PC 3.4 identity, whose required startup pair matches the files the fixture
  provides. Its authentic CSB Atari companion test now verifies native bundled
  7z discovery instead of expecting an external-tool prompt. Both focused M12
  CTests pass, including the authentic CSB preservation archive.

- 2026-10-07: Added an authentic multi-platform M12 AUTO regression with the
  PC 3.4 and FM Towns archives together in one installed data root. The test
  confirms that both editions are discovered, AUTO chooses the documented FM
  Towns default, EDM.EXP and its source hash are selected, and the first
  runtime frame starts at the authentic initial pose. The focused CTest passes
  against the supplied original archives; this verifies menu routing, not
  physical-display parity.

- 2026-10-07: Ran the full authentic English Atari ST v1.2 CLI/M12 startup
  and input matrix against the installed
  `Dungeon-Master_Atari-ST_EN_Version-12.zip`. It contains STX directly, so
  its route is ZIP→STX; the historical CTest name incorrectly called it
  nested. Both direct CLI startup and the M12 path reach the real Hall,
  confirm C127 ordinal 14, and accept the
  native input matrix without external archive tools. The CTest case and script
  now identify it as an Atari ST preservation archive rather than a nested ZIP.

- 2026-10-07: M12 Atari ST discovery now continues through all candidate
  preservation archives after admitting an edition. A persisted root that
  contains both the nested v1.1 archive and the authentic v1.2 ZIP-to-STX
  archive now exposes and launches both; direct v1.2 archive admission and
  the persisted-root M12-to-M11 handoff pass against the original media.

- 2026-10-08: Restored the isolated DM1 Amiga v2.0 ZIP→ZIP→ADF New Game route.
  The selected `swoosh` Hunk executable is admitted against four exact A20
  hashes; its source-owned four-plane logo, 27 palette events, and 9,078-byte
  PCM sample are read from the selected ADF in memory. A direct `--debug`
  M12 launch records the A20E profile, all 10 SWSH frames and 30 source
  VBlanks, the selected ADF path, all 23 C001 title steps, and the complete
  DM1 startup-phase receipt. The isolated authentic-media script passes the
  M12 no-save first-Hall handoff, Quick Resume and movement checks. This
  corrects the earlier failure report: it predated the native SWSH route and
  is no longer current. SDL dummy-driver evidence verifies startup state, not
  physical M5 rendering or visual parity.

- 2026-10-06: Fixed DM1 Amiga ADF admission by keeping the Amiga graphics
  decoder's big-endian legacy-layout flag separate from the little-endian F0434
  `DUNGEON.DAT` fields and checksum. The installed `[HD]` image uses the
  direct IMG2 runtime route. The Amiga HD, DOS English, Atari ST v1.2, and FM
  Towns JA/EN original-media checks listed at the time remain separate
  evidence; SDL dummy-driver evidence does not establish physical M5 rendering
  or visual parity.

- 2026-10-06 (corrected 2026-10-14): The earlier rule that blocked PC 3.4
  English when `SWOOSH` was missing was incorrect. ReDMCSB `APPA.C:51-68`
  ignores the `FTL_SWSH` load result and continues to `FTL_ANIM`; Firestaff now
  follows that behavior, logs the skipped optional prelude under `--debug`,
  and loads the original graphics/dungeon pair. The authentic startup
  regression now covers this case. The separate French multilingual EUDATA
  row still requires its English startup-media companion.

- 2026-10-06: Re-ran the authenticated FM Towns English/Japanese startup and
  launcher matrix against the local bilingual ZIP. Direct launch, default
  `--game dm1` platform selection, version selection, normal M12 title/Entrance
  handoff, first runtime frame and initial party are verified. A natural
  original-media `--debug` run now logs all 20 FM Towns title frames, the 21
  source VBlanks and total elapsed time (about 480 ms on this local host).
  A second natural launch against the original PC34 DOS archive logs all 17
  SWSH palette frames, 30 source VBlanks and its elapsed time (about 3.6 s).
  These SDL dummy video/audio checks do not verify physical Mac M5 HiDPI
  output, window/compositor timing or visual parity.

- 2026-10-05: Corrected the FM Towns text-rasteriser evidence against the
  executable's SYM1 names and opcodes: `TEXT_SIZE` is the measurement routine,
  `TEXT_PIC` at `0x29344` owns the raw font pointer, and the `CHAR_*` fields had
  been mislabeled in the prior report. The standalone text-geometry helpers
  now exclude the final horizontal/vertical spacing pixel, matching the
  source metric pitches; overflow returns zero. This is a source-bound helper
  correction and does not by itself claim a new menu-rendering or visual-parity
  result.

- 2026-10-04: M10 F0207 now emits a source-positioned attack sound request
  after a committed melee or projectile attack, including melee misses.
  This retains the event's map/coordinates at dispatch instead of looking
  up a possibly moved C04 group after the tick. F20 M11 folds the request
  into F0065's existing per-sample PCM queue and applies F0064's distance
  volume. English/Japanese original media checks PCM for a real C04 type
  and square using constructed F0064 emission receipts; a live F0207 attack
  through the full original-dungeon tick remains unverified. The generic
  F20 movement cue no longer bypasses source-issued sound requests.

- 2026-10-04: FM Towns F20 M10 sound emissions now use the authenticated
  source coordinates and DATA.C loud/soft distances. SOUND.C F0064/F0065's
  integer volume steps and loudest request per native sample index are
  applied at each tick flush. Original English/Japanese media passes the
  local, distant, cross-map and duplicate-sample regressions. Physical M5
  output and non-emission sound callers remain to be checked.

- 2026-10-04: M10 gameplay tick sound requests now use retained FM Towns F20
  PCM instead of PC SND3 when the authenticated FM Towns runtime is active.
  An original-media test passes for English and Japanese F20 with an
  EMIT_SOUND_REQUEST through M11 and no loaded SND3 bank. It verifies the
  source PCM handoff, not distance arbitration or audible M5 output.

- 2026-10-03: Bare `--game dm1` with a persisted collection root now scans
  its DM1 leaf directly while retaining the root for the game menu. The
  original-media scan selects the authenticated FM Towns edition by default.
  This check covers selection and startup scanning, not physical audio or
  dungeon presentation.

- 2026-10-03: DM1 Atari ST sound events now use authenticated GRAPHICS.DAT
  retained after startup. ReDMCSB SOUND.C F0060 consumes resident graphics;
  reopening nested ZIP/STX media on every effect diverged from that route.
  Original English 1.2, German 1.2 and French 1.3 media passed the SND1
  source-byte and SDL dummy-queue checks. The live scroll event still selects
  its source sound when the archive path is blank after startup. A subsequent
  original-media check in all three editions measures the SDL queue directly
  before and after an in-game food swallow and requires one queued effect.
  This does not verify audible M5 playback, overlapping-event arbitration or PSG
  waveform parity.

- 2026-10-03: The original-media FM Towns CLI regression now also launches
  bare `--game dm1` with no platform override. The published macOS arm64
  v3.0.355 binary selected the authenticated EDM program and reached the
  first runnable Hall frame; the complete English/Japanese ZIP startup script
  passed. This does not prove physical M5 HiDPI or audible playback.

- 2026-10-02: Restored the F0345 FOOD/WATER inventory page after releasing
  Eye with an empty or ordinary action hand. PANEL.C F0353 calls F0347 for
  that selection; the native held-scroll path had cleared the page and left
  1,523 C101 pixels changed after release. An authentic PC 3.4 Hall test now
  captures the panel before, during and after Eye, checks C023 source pixels
  and M653 text ink, and requires exact panel restoration. The original-media
  test and 140 champion-panel assertions pass locally.

- 2026-10-02: The original PC 3.4 entrance observer now verifies a fresh
  C407 click, the initial door image and all 31 distinct opening steps,
  the Hall handoff to map 0 (1,3) facing south, and its first nonblank
  presented frame. The real-media test passes with SDL dummy video, both
  with and without an audio device. Native Mac M5 input and HiDPI output
  remain unverified.

- 2026-10-02: Preserved each decoded PC 3.4 GRAPHICS.DAT SND3 effect when an
  optional sound pack is empty or replaces only selected events. An incomplete
  original bank no longer loses valid samples during pack discovery. A
  real-media DM1 regression loads the retail SND3 bank, simulates one missing
  event, applies an empty pack and a single source-derived WAV, and confirms
  every unaffected original buffer remains bound. Shared SDL audio lifecycle,
  DM1 object-name audio, and CSB source-audio tests also pass locally.

- 2026-10-02: Re-read the authenticated start square in all six Atari ST
  editions in Motorola source order. Every map 0 (1,3) square has an inactive
  teleporter followed by the same two item-27 party-possession sensors; an
  empty fresh party triggers none. This corrects a prior C04 claim in TODO
  without changing runtime behavior.

- 2026-10-02: Restored the authentic PC 3.4 D0C floor-item route. ReDMCSB
  DUNVIEW.C F0127/F0115 uses C2500 cells 0/1 for objects on the party square;
  the materialization gate had required cell 2 and withheld the rendered
  C080 pickup target. The original-media HoC regression now reaches map 0
  `(4,15)` from the archive bootstrap, clicks the rendered WATER `0x280b`,
  transfers it to backpack slot 1, clicks the exposed scroll 0, and reads
  its source text 33 while holding Eye. The ordinary source mouse route is
  used for both pickups, with no save or pose override.

- 2026-10-02: Added an authentic PC 3.4 CLI mouse-movement assertion. A click
  in ReDMCSB COMMAND.C's C070 forward-arrow box at source point `(275,135)`
  advances the retail ZIP's fresh party from map 0 `(1,3)`, facing 2, to
  `(1,4)`, facing 2. The original-media run uses a native 320x200 window so
  the scripted click follows the production pointer hit route. The same
  original-media test now replays the Hall path with 28 movement-arrow clicks,
  opens C127 ordinal 5 by clicking its portrait, and resurrects that champion
  through C040. It verifies map 0 `(14,3)`, facing 0, one champion, and a
  closed candidate panel without keyboard input, a save, or a pose override.

- 2026-10-01: Extended the authentic PC 3.4 HoC source-placement regression
  through a live production-input route. Starting from the archive bootstrap,
  it replays keypad movement, clicks C127 ordinal 5, confirms the candidate
  through C040, and walks to map 0 `(4,15)`. It verifies the source pile's
  closed scroll 0 -> text 33, picks up the pile's top WATER (`0x280b`) through
  its rendered C080 mouse target, opens C007 and places WATER in backpack slot 1.
  The test then picks up scroll 0 through C080, holds Eye with a separate
  production press,
  confirms the source panel decoder returns the same text as scroll 0/text 33,
  and releases Eye to verify panel cleanup while retaining the scroll. The
  test passes against the original PC 3.4 ZIP without a save or pose override.
  This proves runtime selection and decoding of source text 33; an eye-held
  presented-frame capture and authentic DOS pixel parity remain open.

- 2026-10-01: Strengthened the authentic PC 3.4 normal M12-to-M11 startup
  regression with the retained source-owned handoff receipt. The runtime probe
  now reports whether SWSH, C001 TITLE, ENTRANCE, and the complete graphics
  sequence were consumed; both the public `--game dm1` CLI route and the
  normal M12 menu route require all four plus the first HoC runtime frame.
  The opt-in startup capture also stores the actual post-present C001 first,
  mid-zoom and full-zoom frames, closed C004 Entrance and first door-opening
  frame from original media. Checks require visible non-flat frames and frame
  changes, without pixel-parity claims. This does not verify Mac M5 HiDPI
  presentation.

- 2026-10-01: Extended the authentic FM Towns M12 test to use its Version tile
  to select Japanese data without `--dm1-fmtowns-ja`. The runtime probe now
  reports the bound GRAPHICS.DAT and source-owned `JDM.EXP` hashes; the real
  ZIP test asserts both and the first live dungeon frame. The complete
  `dm1_v1_fmtowns_archive_cli_boot` CTest row passed with separate expected
  Japanese graphics and executable hashes. This proves the tested menu route,
  not Japanese visual, sound or broader gameplay parity.

- 2026-10-01: Added an autotest-only host-quit injection after the first
  rendered Atari ST C001 title frame and exercised it with the authenticated
  French v1.3 archive through the normal M12 launcher. The check requires the
  injected quit receipt, a clean process exit, and no launch-failure message;
  the same full original-media test then verifies ordinary M12 startup,
  C127 recruitment and live movement. The complete CTest row passed in 65.57
  seconds. This covers the Atari close-event path, not PC/F20 event timing or
  native HiDPI behavior.

- 2026-10-01: Re-ran the production CLI boot probe against the authenticated
  PC 3.4 archive on the macOS host. The source title completed all 23 steps,
  loaded the dungeon, and published `phase=dm1-runtime` after 120 runtime
  ticks. The same probe reached runtime from authentic Atari ST 1.2, Amiga
  2.0 and FM Towns Japanese/English archives. FM Towns selected its source
  TMENU `EDM.EXP` program and published the CDDA track-5 receipt. These are
  direct CLI boot checks; they do not verify physical-device sound or visual
  parity on MacBook hardware.

- 2026-09-29: Corrected the live PC/F20 M11 title selection to use the
  ReDMCSB `TITLE.C` F0437 `GRAPHICS.DAT` C001 sequence even when the separate
  canonical loose `TITLE` animation is installed. The authentic PC3.4 normal
  launcher regression now asserts that C001 was selected and all 23 source
  steps completed before the runtime handoff.

- 2026-09-29: PC/F20 startup now marks the title handoff complete only after
  all 23 C001 source steps (or all 53 TITLE.DAT frames plus the final guard)
  finish presenting. Decode, presentation and event-pump interruptions no
  longer count as a completed intro or allow the Entrance handoff. The
  post-launch contract regression confirms that partial title playback closes
  its receipt and stops before Entrance; authentic-media startup coverage is
  still needed to prove the full sequence on each PC/F20 host.

- 2026-09-29: PC34 DM1 Credits now waits for input without inheriting
  the ENTRANCE.C 1800-tick timeout, matching SELECTOR.C:1002-1004. Other
  entrance media keep their existing timed wait. Authentic runtime coverage
  holds Credits for over 37 seconds before sending Return, then checks the
  same score owner and a prompt return to the entrance. The long scenario
  and both normal/no-device selector scenarios pass through CTest.

- 2026-09-29: Restored PC34 selector-owned SONG playback after 60 VGA
  retraces, with bounded refill, source-requested 11126 Hz and the authentic
  sequence-index-1 loop. Credits preserves the stream; exits stop it.
  Fixed a nullable Credits result write and a total-timeout/per-tick mixup
  that blocked input and refill for 36 seconds. Authentic selector tests
  pass Credits/Quit, Enter, Resume and early Quit with and without a device.
  DM1 transport checks pass 332 assertions, including PCM across two loop
  seams; CSB Amiga passes 48 and DM2 DOS passes 60 launcher assertions.
  The authentic live SONG probe passes 11 invariants. Dummy SDL verifies
  transport and event flow, not physical sound or Retina presentation.

## 2026-09-29 — Preserve platform selection when returning from presentation

- Returning from presentation cards restores the selected platform card.
- Verified the back-and-reselect path using the authentic Amiga v2.0 ZIP
  package in `test_dm1_v1_amiga_v20_archive_cli_boot.sh`. The complete script
  passes, including native menu handoff and the first runtime frame.
- Corrected the required-media launcher test to compare its ready message
  in the selected UI language; the test passes on the local macOS host.
- Removed the new synthetic Amiga fixture extension in favor of this
  original-media regression. Refreshed startup-menu catalog source references.

## 2026-09-28 — Launch authentic Atari ST 1.1 directly from the DMWeb 7z

- Firestaff now reads the selected original STX and its required files from the
  multi-member DMWeb archive in bounded memory. When that collection also
  exposes an earlier cracked MSA image, Atari ST fallback selects the
  authenticated English 1.1 STX instead of the neighboring image.
- Authentic-media verification confirms the CLI boot reaches
  `dm1-runtime`, loads the level and reports graphics MD5
  `5095a13692702235d2e74f6b2b1367a9`. The ordinary M12 start-menu handoff
  reaches the Atari ST DMCSB1 runtime using the original archive path. The
  complete authentic 1.1 menu/startup/Hall/gameplay route and virtual-member
  selection regression pass; this does not claim full campaign or visual
  parity.

## 2026-09-28 — Resolve party-possession floor sensors with live inventory

- Connected ReDMCSB MOVESENS.C F0276 C008 to the live party-enter paths.
  The runtime now checks living champions' inventory, closed chests through
  F0274, and M11's separate G4055 leader-hand object before emitting the
  remote effect. The legacy context-free wrapper safely skips C008.
- Regression coverage verifies absent and carried icons, HOLD set/clear, and
  the separate leader hand. The focused sensor test and native program build
  pass; the full PC-34 scripted input matrix remains unverified because its
  later `down` movement probe did not move the party in this run.

## 2026-09-28 — Ignore creature-only floor sensors on party entry

- Aligned party enter/leave processing with ReDMCSB MOVESENS.C F0276
  (C007, lines 1703-1705): the floor-creature sensor does not trigger for the
  party. The focused runtime regression passes. Group movement remains handled
  by its separate F0267 path.

## 2026-09-28 — Do not fire floor group generators on party entry

- Aligned `F0710_SENSOR_Execute_Compat` with ReDMCSB MOVESENS.C F0276:1704-1705:
  C006 group-generator sensors are skipped by the floor addition/removal path.
  Their timed generation path remains independent. The focused regression now
  guards against spawning a group merely because the party enters a square.
  Initial-campaign sensor coverage remains open in TODO-dm1.md.

## 2026-09-28 — PC-34 AUTO start-menu route

- The ordinary DM1 start-menu AUTO route discovers authentic PC-34 media in
  the installed data root with a clean configuration and no `--platform`
  override. It reports `handoff=pc-img3` and reaches the first source-owned
  runtime frame with the expected zero-champion Hall-entry party state. The
  DM1 PC-34 real-media CLI test now asserts this discovery and runtime receipt
  in addition to its platform-forced route.

- 2026-09-27: The fresh Atari startup-party handoff now preserves ReDMCSB
  MOVESENS.C:F0276's C003 guards: an empty party does not fire a floor-party
  sensor, and a data-coded sensor fires only when its facing matches the
  party's direction. Focused fixtures cover empty, mismatched and matching
  cases; the rebuilt game also passes the authentic English Atari ST v1.2
  M12 first-Hall route. That media route confirms startup only; it does not
  claim that its initial square contains C003 or prove the full campaign
  sensor sequence.

- 2026-09-27: Added an authentic first-runtime assertion for the fresh-game
  Atari ST party handoff and ran it against English v1.0a, v1.0b, v1.1 and
  v1.2, German v1.2, and French v1.3 original media. All six full CLI/menu
  regressions pass with the first Hall frame at map 0 (1,3), facing south,
  zero champions, and `dm1StartupHandoffExecuted=1`, before their separate
  Hall-selection and gameplay-input checks. The campaign-square sensor effects
  remain open in TODO-dm1.md.

- 2026-09-27: Corrected stale downward-movement expectations in the authentic
  Amiga, nested Atari ST, and FM Towns English/Japanese launch regressions.
  Each original-media route moves from the initial party coordinate (1,3) to
  (1,2) on `down`; the tests had incorrectly expected no movement. The complete
  Amiga, Atari ST, and FM Towns CLI/start-menu matrices now pass against the
  original archives. No runtime or visual-parity claim changed.

- 2026-09-26: Extended the authentic Amiga v2.0 M12 New Game mouse route
  through its normal IMG2 handoff to the first Hall runtime frame. The probe
  requires the source startup handoff, a loaded runtime level, and the original
  zero-champion pose at map 0 (1,3), facing south. Together with the separate
  authentic Amiga M12 Quick Resume assertion, this covers both New Game and
  saved-game start-menu paths. A playable campaign party remains unproven.

- 2026-09-26: Extended the authentic FM Towns M12 mouse-launch regression
  beyond its launch receipt. Both English EDM and Japanese JDM now complete
  their original title and Entrance sequence through the ordinary launcher
  and reach `dm1-runtime` with the first Hall frame ready. The original ZIP
  supplies the selected executable and graphics; only the headless Entrance
  input is bounded by the test harness. The source start has zero champions,
  so this confirms the menu-to-runtime route, not a playable campaign party.

- 2026-09-26: Extended the authentic Amiga v2.0 save-disk regression through
  M12 Quick Resume. The ZIP → ZIP → ADF `DMGAMEG.DAT` member now reaches the
  same `dm1-runtime` receipt from the actual launcher row as from direct CLI,
  restoring map 0 at (4,15), facing south, with four champions and game tick
  292. The check exits at launch so elapsed host time cannot change the saved
  tick; the original archive stays unmodified and no save is extracted. This
  supplied save has zero active groups, so the route does not prove a living
  Amiga group or complete save parity.

- 2026-09-26: Corrected Atari Hall regression tests that treated opening a
  C127 candidate panel and provisionally incrementing `championCount` as a
  completed recruitment. The normal M12 route now clicks the source C040
  confirmation, asserts that the candidate panel closes, and sends a keypad
  turn to verify gameplay input in the same session. Authentic media tests
  pass for English ST 1.0a, 1.0b, 1.1 and 1.2, German ST 1.2, and French ST
  1.3. The separate German CLI check also confirms C040 and the subsequent
  turn. This proves Hall choice and immediate runtime input on these editions;
  it does not claim the complete F0441/F0435/F0462 campaign-start transition
  or visual parity.

- 2026-09-26: Fixed scripted keypad keydown events to include their SDL
  scancode. The live PC-34 input router selects movement commands by scancode,
  while direct boot-probe scripts translate key names themselves; scripted
  `key:kp*` events therefore failed to exercise the ordinary M12-to-M11 input
  path. Added an authentic PC 3.4 regression that starts from the M12 menu,
  follows the Hall route, confirms C127 ordinal 5 through the C040 choice, and
  sends a keypad turn in the same M12 session. The test asserts the candidate
  panel closes, the party count is one, and facing changes from north to east.
  The full PC-34 native CLI test passes, including the C040/C007 presentation
  checks and input matrix. This proves the tested PC 3.4 route, not campaign
  start, visual parity or other DM1 editions.

- 2026-09-26: Fixed the fresh-start native save rejection for F0195/F0180's
  initial C37. When there is no imported C3/C4 save receipt, export now accepts
  only the source-shaped `C37`, `C.Ticks=0` event whose group resolves exactly
  once through the original square Thing chain. Other generated C29-C41 events
  remain fail-closed without their save receipt. Corrected the regression's
  dungeon column/SFT base setup, then verified G0377 plus GLOBAL_DATA and
  ACTIVE_GROUP round-trip, negative C36 rejection, and the broader original
  save handoff test. The local original-data directory has no staged
  `DMSAVE.DAT`/`DMSAVE.BAK`, so no authentic save/resume claim is made. The
  authentic English DOS archive CLI/menu and first-input regression also
  passes with the changed build.

- 2026-09-25: Extended the authentic German Atari ST 1.2 CLI regression to
  follow the source map to C127 ordinal 14, click it through the normal host
  pointer-coordinate mapper, and assert that the candidate panel opens and the
  champion joins the party. The probe uses a clean default window profile so
  the pointer location is stable across developer settings. Corrected the
  campaign-start notes: ReDMCSB LOADSAVE.C uses `DUNGEON.FTL` for an optional
  custom dungeon; it is not required to start the built-in Atari campaign.
  The same authentic C127 ordinal 14 recruitment now passes through normal
  M12 startup for English and German v1.2, French v1.3, and English v1.0a,
  v1.0b and v1.1. The nested English ZIP→STX test also retains its complete
  native input matrix. Repeat on any other admitted Atari profile.

- 2026-09-25: Re-ran authentic Atari ST archive launch tests for English
  1.0a/1.0b/1.1, German 1.2 and French 1.3. All five passed their CLI and M12
  start-menu checks against original STX files. The receipts still start
  without a recruited champion; this does not close the documented playable
  Atari campaign-start gap.

- 2026-09-25: Fixed the authentic Amiga HD regression's command matrix. It
  previously requested `--boot-probe` together with `--menu`, a combination
  that the CLI rejects, and expanded an empty Bash array under `set -u` on the
  supported system shell. The menu launch is now checked in its normal M12
  route, while all three presentation probes use direct CLI launch. Both the
  Amiga HD and nested Amiga v2.0 original-media tests pass.

- 2026-09-25: Completed authentic Atari ST 1.0a/1.0b/1.1 `DUNGEON.DAT`
  admission. ST 1.0b's missing MD5/SHA256 identity was authenticated from its
  original 1987-12-11 STX; ST 1.0a's already admitted M12 profile also needed
  its dungeon MD5 added to M11. All three original-media regressions reach
  `dm1-runtime` through the normal launcher route. They currently start with
  zero champions. The clean STX
  roots for v1.0a/v1.0b and v1.1/v1.2/v1.3 lack `DUNGEON.FTL`; v1.0a/v1.0b
  STX roots also lack `DMGAME.DAT`. This closes data/runtime recognition for
  two editions, not playable Atari campaign startup.

- 2026-09-24: Fixed the all-black Atari ST runtime presentation by binding
  MEDIA020 floor/ceiling graphics 75/76 and the six Atari RGB3 dungeon
  palettes from ReDMCSB `DEFS.H` and `DATA.C` to M11. The authentic German
  Atari ST v1.2 menu route now presents 17,773 nonblack pixels across six
  colours in its 320x200 first runtime capture. The integration test checks
  only for a visible multicolour frame, not visual parity. The campaign still
  reaches M11 with no recruited champion (`championCount=0`); the Atari
  Champion Hall/new-game flow remains open in `TODO-dm1.md`.

- 2026-09-24: Fixed the Atari ST startup handoff so the authentic C001 title
  frame and source zoom sequence execute before the entrance sequence in both
  direct CLI and M12 launches. The runtime receipt now records title
  consumption only after an original title frame was presented. All six local
  `dm1_v1_atari` CTest cases passed with authentic DE/FR Atari media and the
  nested English archive. Champion selection and Atari pixel parity remain
  open.

- 2026-09-24: The authentic English Atari ST v1.2 preservation archive now has a
  normal M12 start-menu regression that continues through the source-owned
  title/entrance input and asserts the first `dm1-runtime` frame. This closes
  the previous gap where the menu test stopped at the M12→M11 handoff and
  only a separate `--boot-probe` reached runtime. Verification used the
  supplied ZIP→STX original media; broader Atari editions and pixel parity
  remain open.

- 2026-09-24: Authentic German Atari ST v1.2 and French Atari ST v1.3 now
  follow their normal M12 start-menu selection, source-owned title/entrance
  input and M11 runtime handoff. Their receipts bind the selected original
  STX and report the source start pose `(map=0,x=1,y=3,direction=2)` with no
  selected champions. This verifies edition-specific runtime admission, not
  a playable party; continue with the source Champion Hall flow before
  claiming that boundary complete.

- 2026-09-24: Extended the authentic PC 3.4 launcher regression to inspect
  M11's source-owned startup receipt after the actual game-card → PC → Original
  menu route. It now proves that launch handed off to `dm1-runtime` with the
  game active and level loaded, instead of stopping at the menu's `DM1 READY`
  diagnostic. The runtime-probe JSON exposes the same startup receipt for
  local autotests; no gameplay or menu behavior changed. This covers the PC
  3.4 route only; other DM1 editions and all other titles remain open in
  `TODO.md`.

- 2026-09-18: Re-inspected a private DOSBox-X run of unmodified retail PC
  3.4 data after the HoC resurrection route. Three distinct 320×200 frames
  show the source FOOD/WATER inventory panel, an object in the action hand,
  and that object placed in a backpack slot. The raw frames and game media
  remain private and are not repository artifacts. This confirms the visual
  reference set for those poses only; it does not claim a floor pickup,
  object-bearing Eye-scroll read, cadence parity, or full HoC parity.

- 2026-09-16: Revalidated the authentic PC 3.4 Entrance-to-Hall checkpoint
  with DOSBox-X and the original game data. The two-frame route produced a
  healthy, non-duplicate 320x200 Entrance frame followed by a healthy,
  non-duplicate dungeon frame; the semantic classifier identified them as
  `entrance_menu` and `dungeon_gameplay`, respectively. This closes the
  capture-harness handoff check only. It does not establish a recruited C127
  pose, inventory/pickup behavior, Eye-scroll behavior, or pixel parity, all
  of which remain explicitly open in `TODO-dm1.md`. Original captures remain
  operator-local and are not part of the repository.

- 2026-09-08: Corrected the source binding for the authenticated PC 3.4 Hall
  C127 route. `MOVESENS.C` supplies C127's C026 `SensorData` solely as the
  portrait selector; `REVIVE.C F0280` then walks the party's current Hall
  square for its `TEXTSTRING`, which owns the candidate identity and encoded
  values. Firestaff had incorrectly treated the portrait selector as a global
  TextString catalogue ordinal, allowing the wrong champion to be recruited.
  The native C127 → C040 → C160 → C007 route now recruits `CHANT SAYYADINA
  STHAYA` with the source record's C007 name/title and vital values. The
  archive-backed CLI regression passes in Original, scaled Original and
  Modern; the local source-derived C007 frame is deliberately not committed.
  This fixes the documented Hall-open/recruit route, while item transactions
  and full same-state pixel/cadence comparison remain open.

- 2026-09-08: Completed the next source-owned C007 paint phase on the
  authenticated PC 3.4 HoC route. `PANEL.C F0355` now supplies its fixed
  HEALTH/STAMINA/MANA labels and `CHAMDRAW.C F0292/F0290` supplies the
  recruited champion's M653 name/title, health/stamina/mana and load values
  after C160 → C007. The renderer uses the active archive's M653 font and
  fails closed without it; it neither substitutes host typography nor an
  invented panel. The real-media CLI regression passes through
  C127 → C040 → C160 → C007 in 320x200, Original 16:9 and Modern 16:9.
  A same-state original C007 pixel/cadence comparison and a real item
  transaction remain open.

- 2026-09-08: Corrected the real PC 3.4 HoC C160 → C007 continuation. A
  champion with an empty action hand now immediately selects PANEL.C F0347's
  F0345 FOOD/WATER page (C020/C030/C031), instead of exposing a bare C017
  background until a later mouth click. The archive-backed CLI route now
  requires both the C007 inventory and its source FOOD/WATER panel. The full
  976-record PC3.4 object/slot corpus still passes in Original and Modern.
  This restores the first post-resurrection inventory panel; same-state
  pickup, scroll and complete C007 pixel/cadence parity remain open.

- 2026-09-08: Compared the valid local original PC 3.4 C127/C040 capture
  against a native 320x200 frame driven by the same archive, route and C127
  click. The C040 rectangle at `(80,85,144x73)` matched 10,409 of 10,512 RGB
  pixels (99.02%). The main non-source difference is the captured original
  mouse cursor at the C127 click location; the remaining small set is limited
  to C040 transparency over the C017 backing panel. An experimental direct
  Hall backing reduced the match to 91.65%, confirming that this PC3.4 C040
  path retains C017 rather than permitting a guessed Hall-underlay renderer.
  The images remain operator-local under `.codex-scratch`; this is a
  same-state composition measurement, not a claim of full HoC pixel/cadence
  or inventory-interaction parity.

- 2026-09-08 correction: the operator-local images in
  `dm1-original-hoc-c040-20260908` labelled C040 are three bit-identical Hall
  frames, not validated source-modal frames. That set must not be used as
  C040 pixel/cadence evidence. This does not invalidate the separate
  `dm1-original-hoc-recruit-inventory-20260908` route, which visibly records
  C040 followed by C007; the outstanding work is an explicit same-state
  native comparison. The native C127/C040 input regression remains valid.

- 2026-09-08: Extended the authenticated PC 3.4 Hall capture through a real
  C040 resurrection and C007 inventory open. After the source C127/C040 route,
  C160 at `(130,115)` recruits Chani Sayyadina Shaya and C007 at `(54,14)`
  opens the retail FOOD/WATER inventory panel. The source frame remains local
  under `.codex-scratch`; it establishes the correct state and hit geometry
  for later pickup/eye comparisons, not item-transaction parity by itself.

- 2026-09-08: Captured the authentic retail PC 3.4 Hall C127/C040 route with
  DOSBox-X/Xvfb using the existing read-only local original stage. The working
  input sequence is mouse-mode launch (`DM -vv -sn -pm`), Entrance click
  `(260,84)`, keypad-Enter (the PC 3.4 `0x001c` command), approximately eight
  seconds for the door animation, the documented 28 keypad movements, and
  C127 click `(112,83)`. The final original C040 frame is stable across three
  captures (SHA-256 `b534f397…b04856`) and remains only in `.codex-scratch`.
  This establishes route state, not a copyrighted-pixel comparison or full
  inventory parity.

- 2026-09-08: Locked the archive-derived PC 3.4 Hall route into the native
  CLI regression for both Original and Modern. In Firestaff, the sequence
  reaches `(14,3,N)` and physical `(744,464)` in a 1920x1080 16:9 window maps
  to C127 at source `(112,83)`, opening C040 for champion ordinal 5. The test
  uses the supplied archive in place, makes no save or graphics substitute,
  and verifies the Original-mode C040 raster is materially visible before the
  normal RESURRECT route consumes the candidate. This is a native regression,
  not an original-emulator route claim; the separate retail mouse route remains
  open until its DOSBox-X capture reaches the same C127 pose.

- 2026-09-07: Implemented the source-locked PC 3.4 Hall C127 mirror path in
  the native runtime. Candidate selection reaches F0280; a rendered C026
  portrait click opens C040, C160 resurrects, and C161 rename/reincarnate
  applies the original vital-halving rule. The recruited source portraits and
  rename survive the native quick-save/load boundary.
  `m11_dm1_hoc_c127_resurrect_reincarnate_full_pc34` and
  `m11_dm1_hoc_real_mirror_viewport_material` pass against the supplied media.
  This closes the native champion-mirror implementation only. Original
  emulator capture, composite viewport, inventory, and natural-play work stay
  in `TODO-dm1.md`.

- 2026-09-07: F0179 now records the source-gated I34 sound request when an
  idle Couatl flips a frame (movement sound) or an already-attacking Animated
  Armour flips a frame (combat sound), following GROUP.C:235-243 and 267-281.
  C32-C41 reaction and staged attack-entry dispatch publish the receipt only
  after their timeline/group transaction commits; Couatl requests retain
  F0514's rest suppression. Focused F0179 and packed-direction runtime tests
  pass. F0183 map-entry initialization and original live arbitration traces
  remain explicitly open.

- 2026-09-07: F0209 attack-entry fanout now preserves the popped source event
  when its private C38-C41 queue transaction cannot fit. It restores the
  pre-decision RNG and requeues the original record on the next game tick,
  avoiding both partial publication and same-tick re-admission loops. The
  full-capacity regression proves retained group state, RNG and source-event
  identity. This does not establish the remaining C32-C36 branch coverage or
  original-emulator timing.

- Implemented F0205's G0395/G0396 half-square owner as a transient stable
  ACTIVE_GROUP source-slot identity instead of the compacted host AI row or
  C04 group index. Admission chooses the first unused source slot, compaction
  carries the slot with its owner, and F0194/F0195 map rebuilding retains the
  last-owner memo while repopulating slots. C38 retry, F0206 and attack-entry
  fanout share the transactional memo. ReDMCSB GROUP.C:414-435, 1602-1636
  and 1954 are the source reference. Six focused DM1 tests pass, including a
  same-tick cross-dispatch regression and one real DOS-media test. This is
  source-derived native evidence; an original emulator trace remains open.

- Authenticated the local English DOS ZIP against the creature-capture
  contract directly through streamed archive members: GRAPHICS.DAT SHA-256
  2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e and
  DUNGEON.DAT d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85.
  Prepared an unchanged, untracked emulator-only copy under
  .codex-scratch/dm1-original-en-capture. The older dos-raster stage has a
  different graphics hash and cannot support this revision's capture claim.
  No original screenshots or new parity claims result from staging alone.

- Removed the viewport capture launcher's implicit .firestaff game-stage
  selection and updated its help/printed HoC diagnostic. The explicit
  DM1_ORIGINAL_STAGE_DIR override and existing repository-local fallback
  remain. Shell syntax, help and diagnostic printing checks pass. No
  emulator was launched or capture evidence generated by this path cleanup.

- Removed obsolete .firestaff/external-volume source lookup from the
  creature-chain original-capture gate. It now discovers the repository
  ReDMCSB copy first, retains the explicit environment override, and supports
  a .firestaff/reference fallback. Source discovery/audit and check-only
  validation pass. The gate correctly remains blocked on original images;
  no captures or parity claims were fabricated and no reports regenerated.

- Strengthened the 128-seed half-pair attack-entry test with independent
  F0209/F0205 RNG accounting and exact packed direction comparison. It
  confirms turn-delay draws still occur after a suppressed pair turn and
  includes the original I34 type-five F0179 draw count (GI=0x539D).
  The runtime target passes; expectations do not invoke production F0179.
  This remains bounded source-derived evidence, not an emulator trace.

- Prevented a half-square pair from turning twice inside one attack-entry
  fanout, following F0205's same-time suppression (GROUP.C:1606-1620).
  The caller still draws its F0209 turn delay after a suppressed turn.
  An I34 type-five two-creature regression across 128 seeds fails before
  the guard and passes afterward; the first eight seeds alone missed it.
  Verified in the packed-direction runtime target. Cross-event same-tick
  suppression remains separate from this within-fanout fix.

- Added four seeded mixed-direction C37 attack-entry regressions. A bounded
  I34 type-zero fixture starts one creature facing the party and three
  opposite. Independent source-ordered RNG accounting checks packed turns,
  each behavior deadline and remaining aspect ticks (GROUP.C:2114-2128).
  The packed-direction runtime target passes all four seeds. This extends
  already-facing coverage; it is not emulator evidence or half-square pair
  synchronization verification.

- Rechecked the rebuilt public executable (runtime 537ca520c) against
  authentic Atari ST nested ZIP/STX, Amiga nested ZIP/ADF and FM Towns
  Japanese/English archives. All three CLI/menu/input scripts pass without
  missing-media skips; FM Towns covers both EDM and JDM handoffs. Extended
  Amiga with six explicit Original/Filtered/Upscaled CLI/menu checks, all
  passing with the expected original graphics hash. These remain headless
  startup/input tests, not original pixel/audio or campaign parity evidence.

- Post-attack-entry integration recheck at runtime revision 537ca520c:
  rebuilt the public executable and passed the authentic English DOS ZIP
  CLI/menu test in Original, Filtered and Upscaled modes, including native
  movement and original IMG3 graphics hash binding. The test explicitly
  disables external archive tools. Also rebuilt/passed F0182 stop-attacking,
  group-map lifecycle, LoS move rollback and C38 projectile-aftermath tests.
  These bounded checks add startup/regression evidence; they do not prove
  campaign, rendered-pixel, audio or original-emulator timing parity.

- Integrated C31 attack entry with staged F0181 removal of the source
  square's C29-C41 events (GROUP.C:2007-2013,340-371). The queue-removal
  primitive is shared with existing world callers without staging an entire
  world. A regression failed before integration and now verifies four
  continuations, exact already-facing RNG, replacement of old C37 and
  retention of a non-group event. All 21 selected runtime/source tests plus
  the F0182/F0195/F0196 lifecycle helper test pass. Five use original DOS
  media; this is not original capture or full attack-sequence proof.

- C37 attack entry now uses the staged per-creature fanout too. Unlike
  C32-C36, GROUP.C:2094,2123 omits the incoming-event delay draw/addition.
  The already-facing four-creature regression failed before integration;
  it now verifies C41-C38 at now+1 and exactly 25 RNG draws, versus 29 for
  the tested aspect-entry path. Added C32 entry and C37 partial-capacity
  coverage; all 21 selected tests pass, including five original-DOS tests.
  Turning, C31 entry and failure retry remain distinct open verification.

- Added C32-C36 attack-entry fanout from GROUP.C T0209044: descending
  per-creature F0205/delay/F0179/F0208 processing, without immediate F0207
  damage. Queue/RNG/group publication is staged together. Four-slot runtime
  tests cover each incoming C33-C36 type, all continuation slots, exact
  29-draw already-facing I34 sequence, and no partial publication when only
  one queue slot remains. All 21 selected tests pass, including five original
  DOS-media tests. The outer dispatcher still consumes scheduling failure;
  C31/C37 entry, turning pairs and original traces remain separate work.

- Consolidated C32-C36 behavior deadlines in the pure F0209 decision:
  incoming C.Ticks is retained even when zero, and approach entry adds one
  before retaining that delay (T0209054/T0209136). Removed the duplicate M10
  override. A 20-case event/ticks/transition matrix exposed ten failures
  before the fix and passes afterward. All 21 selected tests pass after
  relinking, including five original-DOS-media tests. This does not complete
  the separate attack-entry fanout or original-capture verification.

- Reordered C32-C36 continuation animation after the visibility decision
  (GROUP.C F0209:2051-2088,2452-2463). Existing attack updates its selected
  slot; near nonattack updates the whole group; distant nonattack skips
  F0179 and uses the fixed cadence. Actual aspect and behavior deadlines
  reach F0208 separately, and a zero-delay event is no longer discarded.
  Four-slot runtime checks verify whole-group versus selected scope and
  exact distant RNG (one F0228 draw, no F0179 draws). I34 type-zero timing
  verifies +5 animation and 15 remaining ticks from a +20 behavior deadline.
  All 21 selected tests pass, including five original-DOS-media tests.
  Earlier nonattack sibling-preservation checks have corrected attack-mode
  preconditions: they were byte-conversion evidence, not source branch parity.
  Attack-entry fanout, full transaction rollback and original captures remain
  outside this completed continuation change.

- Restored I34 AttackTicks=255 admission guards for C31, C32-C36 adjacency,
  and the shared wander/approach range decision (GROUP.C F0209 MEDIA720,
  lines 2009, 2057, 2104, 2240). Fourteen boundary cases cover 254 versus
  255 across C31-C37; seven failed before the fix and all pass afterward.
  All 21 selected tests pass after rebuilding runtime targets, including
  five original-DOS-media tests. This is not proof of attack-entry fanout,
  blocked-movement attack admission or pre-MEDIA720 edition behavior.

- C33-C36 aspect updates now retain the selected creature's attack flag
  when its group is already attacking (GROUP.C F0209:2070-2073), instead
  of unconditionally applying the non-attack animation branch. Regression
  fails before the fix and passes for all four slots afterward. Rebuilt
  runtime-linked targets and passed all 20 selected tests, including five
  original-DOS-media tests. This does not prove the entire F0209 visibility,
  distance, animation timing or sound decision tree.

- Expanded the bounded Freeze Life regression to every C32-C41 timed
  aspect/behavior event, following GROUP.C F0209:1962-1968. Frozen groups
  preserve their complete active/dungeon group structures, RNG, and nonzero
  source timing payload while retrying four ticks later. All 20 selected
  tests pass, including five original-DOS-media tests. The new event matrix
  is a constructed runtime fixture, not an original emulator capture or
  proof of cross-edition parity; C29-C31 admission remains separate work.

- Added the source F0209 victory gate for Lord Chaos before aspect/RNG work
  (GROUP.C:1946-1949). Timed events are consumed without mutation or retry.
  The C33-C36 regression fails before the guard and passes after it for all
  four slots; the ordinary/frozen/immune/victorious matrix is now 16 cases.
  All 20 selected tests pass, including five original-DOS-media tests. This
  is a bounded event gate, not proof of the complete fusion/ending sequence.

- Extended C33-C36 Freeze Life checks with original Lord Chaos attributes:
  all four selected slots continue updating and consuming RNG while ordinary
  groups defer. The 12-case ordinary/frozen/immune matrix passes. Read-only
  audit also confirms the outer LoS admission previews a copied RNG rather
  than changing the caller's stream; no speculative change was made there.

- M10 timed reaction handling now applies the PC34 Freeze Life gate before
  visibility RNG and F0179 aspect work (GROUP.C:1962-1968). Non-archenemy
  events retain their payload and retry at gameTick+4. All four C33-C36
  runtime regressions preserve selected/sibling aspects and exact RNG while
  frozen, and update normally when unfrozen. Outer C29-C31 admission and
  edition-specific original traces remain separate verification work.

- Public native Amiga startup/input recheck passes for both
  Dungeon-Master_Amiga_EN.zip and Dungeon-Master_Amiga_EN_Version-20.zip.
  Ran `test_dm1_v1_amiga_hd_archive_cli_boot.sh` against each original archive
  (second via FIRESTAFF_DM1_AMIGA_HD_ARCHIVE override), covering CLI, menu,
  IMG2 graphics hash, first movement and fresh-launch seven-command matrix.
  No extraction or external archive helper is enabled. The separate v2.0
  save-resume suite was not run; savegame work remains deferred. These headless
  checks do not establish original pixel, sound or simulation timing parity.

- Current public executable (runtime code at a8ada3b50) passes authentic
  `test_dm1_v1_fmtowns_archive_cli_boot.sh`: CLI/menu EDM and JDM paths bind
  distinct original program/graphics hashes, pointer-only card selection works,
  and both seven-command input matrices reach expected positions. CDDA track-2
  playback state is asserted, not independently audible playback parity.
  `test_dm1_v1_atari_st_archive_cli_boot.sh` also passes original English
  v1.2 ZIP->STX CLI/menu, pointer navigation and the complete initial
  input matrix. Both run natively in memory with external archive tools disabled.
  Headless probes do not establish pixel, timing, audio or all-edition parity.

- Rebuilt the public firestaff executable at 6224744ad and reran
  `test_dm1_v1_dos_en_archive_cli_boot.sh` against the authentic English DOS
  distribution ZIP: passed CLI/menu launch in Original, Filtered and Upscaled
  modes, native IMG3 fingerprint binding and the expected first dungeon move.
  External archive tools were disabled; the archive stayed in memory. These
  headless boot/input checks do not prove visual, audio or full gameplay parity.

- Expanded post-admission regression run: F0195 add-all, group-state bundle,
  group-sensor bundle, F0182/F0195/F0196 lifecycle, LoS move rollback and
  C38 projectile aftermath all pass after rebuilding against a8ada3b50.
  The seventh selected test (G0377) fails at deferred save export and is
  recorded in TODO-dm1.md. This batch is explicitly 6/7, not a green suite.

- Runtime F0180 resets nonpersistent C04 behaviors (>=4) to wander after
  successful event publication, retaining valid attack/approach/flee states.
  Map-transition staging detaches shared raw C04 storage and publishes decoded
  C04 changes to raw bytes only after all destination groups are admitted.
  Lifecycle tests cover decoded/raw reset and failure after a first group's
  staged reset, with no leaked raw writes. All 21 selected tests pass,
  including five original-DOS-media cases; original traces remain required.

- Expanded the aspect adapter regression across all four original C33-C36
  selected-creature events in a four-creature group. Each event changes its
  selected aspect and preserves the other three byte values. Reintroducing
  the old int-sized memcpy import also fails sibling preservation; restored
  per-slot import passes. Together with the earlier writeback mutation this
  detects both transfer directions. Negative-control changes were removed.

- Strengthened aspect-width regression with a real C33 dispatch through the
  runtime adapter: selected creature changes while three sibling slots retain
  their byte values. Reintroducing the old memcpy writeback fails specifically
  on sibling preservation; restoring per-slot conversion passes. The earlier
  misfacing-C38 test alone did not detect that mutation, so it must not be
  cited as proof of the writeback fix. Negative-control code was removed.

- Replaced five M10 aspect transfers between uint8_t[4] runtime storage and
  int[4] behavior adapters with explicit per-creature conversion. The old
  memcpy sizes could combine slots, erase sibling aspects or read adjacent
  runtime fields. This covers map staging, reaction import, post-compaction
  reload, continuation publication and final writeback. The C38 regression
  pins three untouched sibling aspect bytes; 20 selected tests pass, including
  five original-DOS-media tests. Full visual parity still needs original traces.

- Aligned standalone F0180 and generated-placement event receipts with runtime
  C37 publication. The runtime now consumes the corrected plan event type;
  helper/runtime assertions distinguish original event type from AI state.
  Corrected a stale blocked-move retry assertion to retain source Y=5, not
  destination Y=4, consistent with GROUP.C F0209:2174-2185. All 24 selected
  admission, movement, explosion and original-DOS-media tests pass.

- Runtime F0180 publication now emits CREATURE_REACTION/C37 with explicit
  C.Ticks=0 (GROUP.C:332-334), rather than an AI-state-tagged legacy movement
  tick. The natural first-event regression consumes the event published by
  F0195 without replacing it or reseeding RNG, and verifies F0209 consumes RNG.
  Map-lifecycle tests pin the event fields; 21 selected regressions pass,
  including five original-DOS-media tests. Exact natural C37 outcome traces
  and original priority ordering remain unverified.

- Corrected the standalone F0183 helper's LastMoveTime to byte-wrapped game
  time minus 127 (GROUP.C:438), matching runtime admission. The registered
  active-state test pins stored value and receipt to 0xb5 at time 0x1234 and
  passes. This helper currently has test callers, not a newly proven live path.

- Extended the F0194/F0195 runtime regression to reject a second-column owner
  after the first group has consumed admission RNG and scheduled its event.
  The caller world (including RNG/aspects/timeline/history) remains byte-for-byte
  unchanged. Correcting that owner admits group 1 before group 0 in column order:
  original I34 Rockpile/Skeleton metadata yields aspects 0/1 and final RNG
  3861356397 from seed 7. The registered lifecycle test passes. This constructed
  topology verifies runtime ownership/order, not an original gameplay capture.

- F0183 admission now runs F0179 with original I34 creature metadata and the
  shared RNG, publishing per-creature aspects. Successful staged map entry
  publishes the advanced RNG; failure leaves the caller's stream unchanged.
  The Skeleton regression pins aspect 7 and RNG 3257846826 from seed 7
  (five original draws), while the first behavior deadline remains tick+1.
  All 21 selected tests pass, including five original-DOS-media tests.
  Multi-group ordering, later staged failures and all-edition original traces
  remain open; this bounded evidence is not complete admission parity.

- F0183 runtime admission now preserves C04 attack/approach/flee behavior in
  the active AI owner instead of unconditionally assigning wander. GROUP.C
  F0180:328-330 only resets behavior values >=4. The two-move C29/C37 test
  now also runs from F0195-published state, without manual sidecar receipts;
  its behavior assertion failed before the fix. Both selected events remain
  explicitly scheduled and RNG is reset before them, so this is not proof
  of the complete natural admission RNG/timeline sequence.

- Fixed F0183 runtime admission to publish packed directions and home coordinates
  together with movement history, extending contiguous source-row coverage.
  Previously F0195 map entry left SourceCount zero, causing authenticated
  reaction consumers to reject newly admitted groups. The map-lifecycle
  regression failed before the fix and passes after it, using a nonzero home
  position. All 16 selected lifecycle, packed-direction and explosion-group
  tests and five original-DOS-media tests pass. Existing source capacity is
  preserved; unadmitted legacy gaps are not promoted to authenticated rows.

- Registered and repaired the previously unregistered F0194/F0195 map-lifecycle
  regression. It verifies rejection of an invalid outgoing owner, byte-for-byte
  preservation of world/history/C04 after failed destination admission (including
  staged F0184 cell changes), and fresh F0183 position/time on successful entry.
  `dm1_v1_group_map_lifecycle_pc34_compat` passes. This is bounded constructed
  state, not an original-media or emulator parity capture.

- 2026-09-06: Direct M10 F0412 practice checks stop consuming RNG at the
  first failure and support nine missing levels. Counted receipt APIs retain
  the legacy eight-probe interfaces without widening their buffer reads.
  Potion receipts use the same probes and do not draw potion power after a
  practice failure. The runtime regression pins first- and ninth-probe failure
  RNG states, no projectile/event creation, and the failed-potion ordering.
  Reintroducing eager probe consumption fails the exact-RNG assertion;
  restoring the early stop passes again.
  Nine focused spell/potion and DOS 3.4 media tests pass; these bounded checks
  do not replace original emulator failure captures.

- 2026-09-06: Live M10/M11 skill-level queries now share defined early/late
  accumulator arithmetic. F0849 permanent XP wraps instead of saturating;
  before/after level queries use the world edition policy. Exact known graphics
  hashes bind PC34 and Amiga 3.6 late semantics; direct archive startup hashes
  its actual graphics member when launcher metadata is absent. Boundary and
  live-world tests pass, including UBSan reproduction of the original overflow.
  Five DOS 3.4 media cases and six Atari/Amiga/F20 media cases pass with policy
  assertions (Atari EN/DE/FR, Amiga 2.0, F20 EN/JP). The Amiga HD-named test
  also selects 2.0, so it does not prove 3.6. See
  `docs/parity/DM1_SKILL_ACCUMULATOR_AUDIT.md` for remaining scope.

- 2026-09-06: F0186 fixed-drop sound selection no longer depends on a
  successful allocation. Exhausted dragon junk and Trolin weapon pools retain
  the source wooden/metallic sound respectively (GROUP.C:644), with no pool
  growth. The M11 runtime regression passes both cases.

- 2026-09-06: Fresh F0186 drops now have a three-case pit regression:
  open pits transfer to the lower map, while closed and imaginary pits retain
  the item. Exact cell, existing square ownership and raw/decoded compact-list
  metadata are checked. The five original DOS 3.4 projectile/media cases also
  invoke M11 F0186 on an unchanged original empty corridor and verify one
  correctly typed raw/decoded Trolin club. All six tests pass. This proves
  bounded pit behavior and original-pool integration, not emulator death parity.

- 2026-09-06: M11 fixed-possession materialization now delegates to the same
  allocation-owned F0186/F0267 operation as M10, replacing direct floor linking.
  Compact-list regressions follow ReDMCSB F0163 tail append order, preserving
  existing floor objects before generated and subsequently carried drops.
  Eighteen relevant tests pass, including five original DOS 3.4 archive tests.
  The original-media cases cover projectile runtime regressions, not emulator
  captures of fixed-drop consequences or complete cross-platform parity.

- 2026-09-06: The bounded M10 fixed-drop regression now verifies that a newly
  allocated worm round passes through an object-only teleporter onto a floor
  plate, retaining matching raw/decoded ownership and queuing exactly one
  PIT/SET event for tick 43 at (2,0). Normal timeline dispatch opens the pit
  at tick 43, consumes the event and retains the drop's raw/decoded ownership.
  Missing raw C10 backing is rejected before any sensor effect is published.
  All 13 explosion-group cases and the existing M11 fixed-possession runtime
  gate pass. This is a source-shaped RAM fixture, not an original emulator
  capture.

- 2026-09-06: DM1 M10/M11 fixed drops now reserve before cell RNG and
  publish before the next optional decision (ReDMCSB GROUP.C F0186:610-643).
  The bounded helper matrix covers 256 seed/capacity/cell cases; the behavior
  gate passes 2,489 checks, eight explosion cases pass (including zero/one-slot
  ownership), and the M11 runtime gate
  passes 138 checks including zero/one free-slot exhaustion. Evidence scope:
  `docs/parity/DM1_FIXED_DROP_ALLOCATION_ORDER.md`.

- 2026-09-06: M10 explosion death now has direct fixed-possession coverage,
  separate from the M11 helper gate. A two-worm group with six free C10
  records produces 2-6 worm rounds (DUNGEON.C G0252:543-547), with matching
  raw/decoded links and every allocated item reachable exactly once from
  the floor chain. The source-shaped fixture passes without a runtime fix.
  Exact optional-drop RNG, cell distribution and original-media pickup
  rendering remain open; this test asserts ownership and type/count bounds.

- 2026-09-06: Whole-group explosion death with a carried sharp dagger now
  has an ownership regression: the freed group has no possession head,
  the destination floor chain contains the dagger exactly once and its
  raw/decoded Next values agree. The bounded source-format fixture passes
  without a runtime change. Generated fixed possessions and real-media
  pickup/render pairing remain separate open work.

- 2026-09-06: Exhausted C15 smoke fixtures verify F0213:129-130 admission:
  when the fireball occupies the only source slot, lethal damage creates
  neither a host smoke entry nor a smoke event and preserves that fireball.
  Both one-survivor and all-dead cases pass, alongside the available-pool
  smoke lifecycle and nonlethal RNG tests (four tests total).

- 2026-09-06: C25 continuation skips unrelated co-located C15 records
  instead of rejecting the first different type/attack/fingerprint. The
  death-smoke lifecycle fixture failed before this correction and now
  verifies 190 -> 150 -> 110 -> 70 -> 30 -> freed, including raw attack
  bytes and Next=FFFF at retirement (PROJEXPL.C F0220:853-877).
  Nine tests pass, including five original DOS spell cases; executable built.
  This remains source-shaped lifecycle evidence, not original pixel capture.

- 2026-09-06: Shared F0191 death smoke uses source-first F0213 allocation
  (GROUP.C:916) instead of unconditionally creating a host-only effect.
  A three-slot C15 fixture failed before the fix and now verifies raw smoke
  type 40, half-square strength 190 and one fingerprinted C25 at tick+1.
  Nine tests pass, including five original DOS spells; executable rebuilt.
  This proves publication, not original smoke pixels or full expiration.

- 2026-09-06: A two-creature lethal explosion fixture verifies that F0191
  frees the raw/decoded C04 group, removes its active AI entry and unlinks
  the group from its square. This passes without another runtime change.
  The fixture does not provide free possession pools or source death-smoke
  capacity; drops and C15 smoke remain separate open verification work.

- 2026-09-06: Source explosion group damage now uses the shared world F0191
  transaction: descending slots, individual randomized damage and raw C04
  writeback. Its surviving active cells, directions and aspects are compacted
  with the live group. A two-worm fixture verifies exact HP and final RNG;
  a lethal-first-slot variant verifies the survivor. Raw HP and active cells
  failed before their respective corrections. Fifteen focused/regression
  tests pass, including five original DOS spells; executable rebuilt.
  Complete group death, drops and source death-smoke allocation remain open.

- 2026-09-06: Lethal half-square fixtures now assert exact surviving cell
  bytes in both decoded and raw C04: 08 when removing slot one, 0A when
  shifting slot one over slot zero. Both pass. F0190:892-904 preserves unused
  lower packed fields and does not automatically center a lone survivor;
  equality between two mirrors alone was insufficient to prove this rule.

- 2026-09-06: Projectile F0190 survivor removal shifts active packed
  directions and aspect bytes together with HP/cells (GROUP.C:892-899),
  before scheduling later behavior. The slot-zero lethal fixture now gives
  the survivor a distinct south-facing direction and aspect byte; it failed
  before the correction. Seven impact and seven broader tests pass, including
  five original DOS spell cases. Executable rebuilt. Other damage consumers,
  original ACTIVE_GROUP raw representation and capture pairing remain open.

- 2026-09-06: Projectile survivor compaction now synchronizes the normalized
  active AI cell mirror after F0190 damage. A lethal hit to slot zero in a
  two-worm group reproduced stale ai.groupCells before the fix; it now agrees
  with the surviving decoded group. Killing slot one alone did not expose
  the mismatch. Seven focused impact tests and seven broader tests (five
  original DOS spell cases plus two lifecycle checks) pass; executable built.
  Packed-direction compaction and original raw active-index parity remain open.

- 2026-09-06: Source C14 flight no longer treats C15 Fluxcage as a solid
  blocker. F0219 (PROJEXPL.C:687-764) has no F0221 call; F0221's callers
  at 997-1082 serve Lord Chaos/fusion. A raw C14/C15 Type=50 RAM crossing
  failed before this change and passes afterward. Twenty focused tests
  include five authentic DOS spells; F0221 lookup and source-owner metadata
  regressions also pass. Executable rebuilt. Those latter checks are not
  end-to-end fusion tests; original emulator capture pairing remains open.

- 2026-09-06: Extended the half-square projectile fixture to a lethal hit
  on the second creature. The survivor keeps its 1000 HP, raw and decoded
  group counts agree after removal, and the C14 carrier is freed. This
  bounded F0176/F0190 integration passes without a production change;
  GROUP.C:831-904 owns lethal removal and count reduction. It is not proof
  of original death-smoke pixels, possession drops or all platform deaths.

- 2026-09-06: Added source-shaped F0219 stair-boundary integration checks:
  corridor-to-stairs preserves the flying C14 on the same map, whereas
  stairs-to-stairs retires it without damaging the destination group.
  Both match PROJEXPL.C:723; no production correction was needed. All
  thirteen ownership/occupancy/geometry fixture cases pass. These bounded
  checks do not establish original-media or emulator stair capture parity.

- 2026-09-06: Native projectile destination digests preserve imaginary/open
  fake-wall bits (01/04), following PROJEXPL.C F0219:721-724. The omitted
  flags previously made open fake walls stop projectiles. A source-shaped
  RAM regression failed before the fix; open, imaginary and closed cases
  now pass. Sixteen focused tests include five authentic DOS spell tests;
  five shared movement/teleporter regressions also pass. Executable rebuilt.
  Original emulator comparisons and wider platform parity remain open.

- 2026-09-06: Source-backed C14 projectiles no longer collide with another
  projectile. PROJEXPL.C F0219:687-764 checks party, groups and geometry,
  not peer C14 records. A bounded two-record RAM fixture reproduced the
  erroneous despawn before the fix and now preserves both carriers while
  the first moves. Thirteen focused tests (five original DOS spell cases)
  and five shared movement/teleporter regressions pass; executable rebuilt.
  This is not an original emulator timing capture.

- 2026-09-06: Source projectile champion damage resolves the living roster
  member occupying the impact cell, following PROJEXPL.C F0217:509-514 and
  CHAMPION.C F0285, instead of treating the cell number as roster index.
  The party-landing regression failed before this correction and now proves
  unchanged HP on crossing followed by damage to champion zero in cell 3.
  Twelve focused tests pass, including five original DOS spell tests.

- 2026-09-06: Source C14 landing no longer resolves party/group collisions
  one event early. ReDMCSB PROJEXPL.C F0219:687-697,721-739 checks current
  occupants, moves and reschedules; the next event checks the landing cell.
  The native ownership fixture now asserts undamaged group and live C14
  after crossing, then removes its AI row and verifies the next-event hit
  and retained-weapon cleanup. Eleven focused tests pass, including five
  authentic DOS spell tests. Emulator timing comparison remains open.

- 2026-09-06: Projectile target selection now reuses the F0176 creature
  footprint rule (GROUP.C:88-103): only cells FF means centered, half-square
  creatures occupy facing-dependent pairs, and ordinals search last-first.
  Source-cell and movement preflight use the same selector. Four directions
  by four cells, single quarter-square and duplicate-cell cases pass; a
  bounded RAM integration hits the second worm without an active AI row.
  Fourteen distinct focused tests pass, including five authentic DOS spell
  tests and two shared CSB movement tests. These are source and native-media
  checks, not emulator capture or complete platform parity.

- 2026-09-06: Added native MEDIA009 spell composition for Atari/early
  Amiga, separate from the I34 C009-only route. Original C009 96x33 and
  C011 96x36 are required; source row offsets are 12/24, with Atari's
  interior-row copies preserved. Controls retain the selected caster and
  living tabs; legacy selected text stops at NUL. Closed panels clear.
  Five original-media tests passed (170.74 s, no skips): Atari EN/DE/FR,
  Amiga disk/HD, each with 24 independent whole-panel pixel comparisons
  across Original/V2.0/V2.1 plus closed-panel clearing. Existing source
  name/audio/scroll/object checks also passed. PC34 capture receipts no
  longer mislabel these legacy surfaces as I34 material. This is bounded
  source/data framebuffer evidence, not emulator or complete game parity.

- 2026-09-06: Rebuilt the executable and broader inventory/action/original
  object targets after restoring source HUD ownership in V2.0/V2.1. All
  three regressions pass (101.88 s, no skips), including original DOS
  inventory roundtrips and food-panel/C08 checks. This predates the separate
  in-progress bottom-anchored C009 background correction.

- 2026-09-06: Corrected caster-dependent spell tabs, living-slot iteration,
  selected-caster name drawing and source black-clear/XOR4 highlighting
  (SPELDRAW.C:87-94, VIDEODRV.C:3233-3237). Validated authentic item-696
  records against the retained layout reconstruction; name zones resolve
  to x235+14*caster, baseline48, not horizontal centering. DM1 V2.0/V2.1
  no longer select the generated V2.2 HUD. Independent original M653-bit
  comparisons pass all 24 Original/V2.0/V2.1 caster/sparse-party cases,
  plus the direct plan suite (0.21 s, no skips). Names and party state are
  bounded test fixtures. This does not prove full C009/C011 background,
  other-edition or emulator/display parity; broader regressions remain open.

- 2026-09-06: Rebuilt and reran all six selected original-media startup/
  object regressions after the shared XP and food-state changes: Atari
  English/German/French, Amiga disk/HD, and FM Towns (Japanese/English
  restart within its test). All pass in 165.76 s, with no failures or skips.
  This does not establish full startup animation or gameplay parity.

- 2026-09-06: Rebuilt the main executable and affected food/XP/melee/
  creature-projectile targets together. All eight selected regressions pass
  (8.25 s, zero failures or skips), including original package admission
  and original-dungeon Mon Light XP scaling. This is scoped regression
  evidence, not full game or all-platform parity.

- 2026-09-06: Added `m11_dm1_xp_real`: canonical original archive identity,
  fourteen map difficulties and native Mon Light rune/cast integration on
  difficulty 6. Independent source-derived expectations verify 2,556 XP
  normally and 5,112 after recent combat for Air and Wizard, a light effect
  and no duplicate mana debit (0.17 s, no skip). Party/RNG/timestamps are
  bounded RAM fixtures; no emulator or original-input capture is claimed.

- 2026-09-06: Audited ordinary defined PC 3.4 award reachability against
  ReDMCSB and independently streamed the original EN dungeon's fourteen
  map descriptors (maximum difficulty 6). Conservative melee/spell/throw/
  fear/action/heal/parry award bounds remain below 16-bit overflow even
  at format-maximum difficulty 15 with recent-combat doubling. See
  `parity-evidence/dm1-pc34-xp-award-reachability.md`. This is source and
  original-data evidence, not a fight capture; BUG0_81 and cumulative
  32-bit XP behavior remain outside the proven bounds.

- 2026-09-06: Preserved the active food panel and its pre-consumption
  displayed food during the admitted I34E pending command. Simulation food
  still changes immediately (PANEL.C F0349:1918); panel pixels update only
  after completion (1944-1949). Original-graphics/font regressions cover
  Original and Modern, mouth and UseItem, all four pending frames and
  cross-champion isolation (428 assertions, zero failures; CTest 0.58 s).
  These are source-backed framebuffer checks, not emulator captures.
  The rebuilt original DOS object corpus also passes (104.21 s, no skip):
  an allocated original food record preserves all four pending panel images,
  visibly updates the completed panel and queues exactly the original C08
  sample buffer after the final delay. The main executable was rebuilt.

- 2026-09-06: Blocked direct F5/F9 quicksave/load APIs while a food command
  is pending, preserving PANEL.C F0349 command ownership without changing
  save formats. Both consumption routes verify rejection and preservation
  of the pending command and committed food effect; the mouth regression
  passes (0.34 seconds). Before this guard, the rebuilt executable and six
  focused food/package/XP/melee tests passed (7.79 seconds, no skips).

- 2026-09-06: Verified automatic food-clock binding for the original
  top-level I34E ZIP after requiring matching GRAPHICS.DAT, DM.EXE and VGA
  identities/layout. Original object/C08 corpus passes (99.72 seconds).
  Standard VGA cadence is supported by original-driver disassembly and
  the observed mode13 emulator rate. Four original-byte package cases pass
  (0.91 seconds, no skips), including rejection of missing VGA, original EGA
  substituted for VGA, and sibling-directory executable/driver ownership.
  Complete audible/raster-phase parity remains separate verification work.

- 2026-09-06: Original DOS object corpus passes with deferred food/C08
  transport verification (109.10 seconds, no skip). The test consumes a
  real allocated food record, explicitly acknowledges each presented test
  frame, and checks zero early queueing, exactly one source-buffer enqueue
  after 36 edges and no duplicate enqueue. Startup rebinds SND3 to the same
  archive GRAPHICS.DAT used for graphics. This is bounded command-order and
  source-audio queue proof, not live raster timing or audible waveform proof.

- 2026-09-06: Built the main firestaff executable and rebuilt/reran mouth
  visual and action-runtime tests after adding opt-in food-command ordering:
  both pass (0.35/7.77 seconds). Source-edge fixtures verify pending-command
  sequencing, not live clock authentication or successful original C08
  playback. No normal launch enables the new clock; remaining integration
  is tracked in parity-evidence/dm1-consumption-timing-audit.md.

- 2026-09-06: Rebuilt the DOS PC3.4 original-object corpus test after the
  XP word-width and computed-level changes. It passes without skipping
  (101.01 seconds), reading original media in memory. This is object/runtime
  regression coverage, not a complete dungeon playthrough or timing capture.

- 2026-09-06: Extended direct F0303 threshold round trips through level 24
  (the positive signed-32-bit threshold range), plus M10/M11/F0848 runtime
  checks for levels 17-24. Added threshold-minus-one plus temporary XP=1
  cases, distinguishing ignored temporary XP from live queries. Both rebuilt
  skill suites pass. A bounded read-only caller audit found no newly unsafe
  array indexing or shifts from removing the caps; this is not an exhaustive
  safety claim or evidence for edition-specific overflow behavior.

- 2026-09-06: Removed artificial F0303 computed-level caps (live helper
  capped at 16, lifecycle at 17), following CHAMPION.C:765-770,822.
  Added non-overflow XP vectors 16,384,000 and 32,768,000, verifying levels
  17/18 through F0848, M11 and M10 F0888. The regression failed before
  the fix and passes afterward; the full action runtime test also passes.
  Display-name bounds remain separate. Edition-specific signedness and
  original-media high-XP reachability remain open verification work.

- 2026-09-06: Rebuilt original-media name/runtime tests after the F0304
  word-width correction. All six CTest cases pass without media skips:
  Atari EN/DE/FR, Amiga, Amiga HD and FM Towns (JP plus EN restart).
  Runtime: 179.81 seconds with two test workers. This establishes regression
  coverage for their asserted startup/object paths, not full visual parity
  or natural gameplay overflow reachability.

- 2026-09-06: Corrected F0849 award arithmetic to the unsigned 16-bit
  parameter/assignments in ReDMCSB CHAMPION.C F0304:834,866-889. Six
  explicit boundary cases cover input narrowing, multiplier wrap, recent
  doubling and temporary XP after a nonzero award wraps to zero. The new
  regression failed before the fix and passes afterward. These are source
  contract fixtures; natural gameplay reachability remains unproven.

- 2026-09-06: Rebuilt and reran the M11 skill-query, creature-projectile
  and Giggler-steal runtime suites after removing the kill-XP bridge: all
  three pass. These are scoped runtime regressions, not original-media
  captures or proof of full XP arithmetic parity.

- 2026-09-06: Removed the invented base-health/2 XP request from the DM1
  kill notification plan. PROJEXPL.C F0231:1533-1535 already awards
  damage-derived skill XP, with no second kill reward. Regression checks
  all 27 creature types with a living active champion: defeat logging stays
  enabled while bonus XP stays zero. Removed the obsolete M11 bonus helper
  and its notification call. The paired fatal/nonfatal action regression
  checks equal calculated damage and Fighter XP, zero remaining health and
  an actual EMIT_KILL_NOTIFY on the fatal route. Rebuilt melee-plan and full
  action tests pass. This is a bounded RAM fixture, not an emulator capture.

- 2026-09-06: Extended the C080 leader-hand dagger throw integration case
  to map ordinal zero with difficulty three. XP is independently derived
  as ((8 + 4 + (19 >> 2)) >> 1) * 3 from CHAMPION.C F0328/F0304;
  projectile energy/direction, inventory restoration, consumed tick and
  action-disable assertions remain enabled. The full action suite passes.

- 2026-09-06: FM Towns original-media startup checks now assert the
  PROJEXPL.C:5 attack-time sentinel in both Japanese startup and subsequent
  English restart. The names/font/audio test passes with both assertions.
  DOS PC 3.4 startup also asserts the sentinel, and its original-object
  corpus test passes (94 seconds); missing media was not skipped.

- 2026-09-06: Authentic Atari EN/DE/FR and Amiga/Amiga HD startup tests
  now assert the -200 attack-time sentinel immediately after M11 Start.
  All twenty death/leader/living-caster combinations pass with this check,
  proving propagation beyond default-world initialization for those media
  paths. This does not cover DOS/FM Towns startup or imported saves.

- 2026-09-06: Reproduced the zero initial attack clock and restored
  PROJEXPL.C:5's signed -200 bit pattern in F0859 initialization. Both
  direct no-party initialization and F0881 default-world initialization
  assert the sentinel. No serialized layout or save import was changed.

- 2026-09-06: Reproduced Giggler's live empty-inventory attack leaving the
  XP timestamp unchanged. Its F0207 entry now records the attempt before
  F0193, and cannot fall through to ordinary F0230 damage (GROUP.C:1691,
  1790-1793). Live creature checks pass 69/69 and Giggler checks 18/18.
  This does not establish full Giggler cadence/target-selection parity.

- 2026-09-06: Reproduced ordinary M11 melee leaving the XP attack clock
  at zero on tick 28. The live-creature path now records F0207 entry before
  target selection/damage (GROUP.C:1691), matching ranged attack timing.
  Creature runtime and the 1390-assertion action suite both pass after the
  fix. Initial state and Giggler-specific handling remain separately open.

- 2026-09-06: Corrected the Lord Chaos adjacent-retry test oracle from
  east to south. ReDMCSB BASE.C F0029:1765 advances seed 2 to 1988217957
  and 1387506636, giving two-bit values 0 and 3; TIMELINE.C F0252 maps
  the latter to Y+1. Active-state position and C37 destination now pass.
  No gameplay change was needed. Later cross-map teleporter cases fail,
  so the encompassing C006 suite is not yet verified.

- 2026-09-06: M10 C38 projectile test now supplies the complete bounded
  C04/ACTIVE_GROUP/free-C14 fixture and verifies F0207 attack time at tick
  300 (before the orchestrator's final increment). Its projectile receipt
  and timestamp checks pass. The enclosing C006 suite remains failing on
  two later Lord Chaos random-adjacent movement assertions; not a suite PASS.

- 2026-09-06: Reproduced missing live ranged attack timestamp (0 instead
  of tick 28) and connected the M11 F0207 entry to lifecycle XP time per
  GROUP.C:1691. Tests cover successful launch, failed scheduling retaining
  the timestamp, and rejected C04/position/sight routes leaving it unchanged.
  This is ranged M11 evidence; initial state and melee/M10 paths remain open.

- 2026-09-06: Restored the standalone lifecycle probe's repository-relative
  source/include paths and decompressor/FM Towns/reincarnation link inputs.
  Its executable and reports now stay in the selected output directory
  (default .codex-scratch/champion-lifecycle). Missing dungeon media now
  fails K3 instead of passing, and K3 explicitly distinguishes authentic
  header loading from its RAM party fixture. Verified authentic French DOS
  header acceptance and missing-file rejection; three separate legacy
  size/serialization assertions remain failing and are tracked in TODO.

- 2026-09-06: Reviewed F0304-dependent action XP assertions against ReDMCSB
  CHAMPION.C:866-895 and F0328:2170-2183, plus MENU.C F0407/F0412.
  The local action suite passes all 1390 assertions: hidden magic skills
  receive the recent-attack bonus; startup unsigned stale halving precedes
  recent doubling; separate throw awards round independently; parent skills
  receive no temporary XP. The throw-level fixture now starts at 492 so
  the inner eight-point compass award itself reaches 500 before the common
  action tail. This verifies the reviewed cases, not full game parity or
  live attack-timestamp propagation; remaining XP integration work is open.

- 2026-09-06: Original-media restorative casts now verify the exact F0412
  base-XP formula from the independently advanced first RNG sample, and
  exactly one EMIT_SPELL_EFFECT receipt for the correct caster/potion.
  All twenty high/low-skill cases pass (360 casts). This proves base XP
  handoff, not final F0304 map/freshness scaling; those gaps are tracked.

- 2026-09-06: Removed duplicate practice RNG consumption after the M11
  validated-cast/XP handoff. The original Atari test reproduced potion
  power 44 instead of source-expected 50 before the fix. A bounded RAM seed
  search now exercises successful skill-1 casts independently against
  F0412's wisdom threshold, practice draw count and potion power sample.
  All twenty forced-low-skill original-media cases pass; the default twenty
  cases now mix high/low skill and pass alongside three runtime regressions
  (23 tests). Media and flask contents are authentic; skill/RNG fixtures are
  RAM-only. Exact earned XP and the full failure matrix remain open.

- 2026-09-06: Replaced restorative CastSpell API calls with original C108
  mouse press/release. The independent F0027/F0412 oracle exposed an extra
  M10 XP random draw after M11 had already supplied XP; dispatch now skips
  that duplicate. Exact potion power, paid mana and retained spell controls
  pass in twenty original Atari/Amiga cases (360 casts), repeated with V2.1
  selected (720 total). Open Door and action-runtime regressions also pass.
  This covers high-skill input/state, not low-skill double validation or
  final Modern framebuffer parity; those remain open.

- 2026-09-06: DM1 top-row inventory open/switch/close now preserves the
  spell controls, selected caster and paid rune chain (PANEL.C F0355).
  Twenty original Atari/Amiga cases verify five inventory transitions,
  including both press and release, without mana or symbol mutation.
- 2026-09-06: Broader verification at b0043799a passed all thirteen tests:
  original DOS/Atari/Amiga object coverage, DOS PC3.4 native startup, and
  thirty-six CLI/menu launches across six original-media profiles and
  V1/V2.0/V2.1. Both FM Towns languages pass, with JDATA/JDM.EXP provenance
  checked for Japanese. These probes prove loaded-level/mode selection,
  not emulator-equivalent palette, cadence or final pixels.

- 2026-09-06: Corrected F0370/F0408 cast UI lifecycle. A lone power rune
  now reaches F0410's meaningless-spell feedback through mouse, keyboard
  and API, clearing symbols without a refund or practice RNG draw. Successful
  and source-feedback failed DM1 casts retain the caster's spell controls.
  Twenty original-media cases and four focused runtime tests pass (24 total).
  This is input/state evidence, not a final-frame visual parity claim.

- 2026-09-06: Corrected native DM1 F0399 rune charging and modulo-four
  SymbolStep, including fifth-rune replacement. F0412 now uses a separate
  paid-rune validation entry point, retaining legacy callers and serialized
  request layout. Twenty Atari/Amiga original-media cases enforce exact
  rune costs and selected-caster ownership for 360 restorative casts.
  Focused tests cover insufficient-mana rejection, no refund on recant/
  practice failure, and successful casting at zero remaining mana.
  Five additional runtime/legacy magic regressions pass; no emulator or
  final spell-panel pixel parity is claimed.

- 2026-09-06: Restored DM1 spell mouse routing with inventory open.
  PANEL.C:2445 changes the secondary list, not COMMAND.C's primary C100.
  Twenty original-media Atari/Amiga cases now create MON/EE/VI at all six
  powers through rune mouse press/release (360 casts); symbol ownership and
  unchanged non-caster mana are checked. Cast itself still uses the API.
  The strict opt-in rune mana/step oracle exposed separate failures tracked
  in TODO; this result is not mana timing or rendered spell-panel parity.

- 2026-09-06: Audited the compact spell workbench reachability and removed
  the ineffective DM1 caster change and its release claim. The enclosing
  draw guard excludes both DM1 and CSB. Passing inventory/skill tests did
  not prove a DM1 visual fix; the actual source spell renderer needs its
  own rendered evidence.

- 2026-09-06: Reran all twenty original-media inventory/death/consumption
  cases with presentationMode V2.1 using the explicit
  FIRESTAFF_VERIFY_UPSCALED_INVENTORY=1 test option. All pass. This exercises
  the source-layout input/state paths with that mode selected, not the
  separately composed Modern HUD or final upscaled framebuffer parity.

- 2026-09-06: Verified VI's ten-retry exhaustion using source RNG seed 321
  and wound bit 1 with a runtime-created Lo VI in an original flask. The
  wound remains and the final seed matches ten draws; all twenty cases pass.
  RNG state is controlled in RAM, not substituted potion data.

- 2026-09-06: Added failure-only Atari sound-load diagnostics recording
  errno and an independent read from resident original graphics bytes.
  Forty French Atari test repetitions passed without reproducing the
  intermittent failure. This is diagnostic coverage, not a stability fix.

- 2026-09-06: Extended VI's independent wound/final-seed checks to six
  individual wound bits through alternate UseItem; combined wounds retain
  mouth coverage. MON/EE also exercise both routes. Full twenty-case rerun
  passes; an earlier French original-audio setup failure remains tracked.

- 2026-09-06: Fixed VI's premature generation of unused wound-retry random
  values. Both callers now stop generating masks when F0349 would stop.
  Original-flask VI consumption reproduces the old seed mismatch and now
  matches independently computed wound bits and final RNG seed across six
  powers and twenty Atari/Amiga cases. MON/EE do not advance RNG on drinking.

- 2026-09-06: Added spell-created VI health recovery at all six power runes
  with wounds disabled, covering deficit and maximum clamp. The complete
  twenty-case MON/EE/VI matrix passes (360 cast/consume sequences). One
  earlier French Atari run exited without output; three isolated repetitions
  and the full rerun passed. The unexplained early exit remains open.

- 2026-09-06: All 240 MON/EE cast sequences additionally satisfy the
  independent MENU.C F0412 power bound: ordinal*40 through ordinal*40+15.
  This verifies scaling/range, not exact RNG draw identity or ordering.

- 2026-09-06: Extended original-flask spell/consumption coverage to MON
  and EE at all six power runes: 240 creation/consumption sequences across
  twenty Atari/Amiga cases pass. Checks include raw subtype conversion,
  stamina clamp, EE over-maximum diminishing gains and the 900 mana bound
  before reduction. Formula checks use runtime-generated potion power;
  independent RNG/power provenance remains open.

- 2026-09-06: Created MON through OpenSpellPanel/EnterRune/CastSpell (Lo Ya)
  in an original flask, then consumed it through mouth input. Twenty
  Atari/Amiga cases pass subtype creation, source-derived stamina recovery,
  maximum clamp and empty-flask conversion. Party skill/mana/placement are
  controlled RAM fixtures; potion type/power are produced only by runtime
  spell execution. This is not emulator or complete spell parity.

- 2026-09-06: Extended original antivenin tests to require accepted C08
  swallow transport and no generated-marker fallback for both consumption
  routes. All twenty Atari/Amiga cases pass; waveform/timing remains unproven.

- 2026-09-06: Rebuilt and reran the broader thirteen-test original-media
  suite after the consumable changes: PC3.4 native boot, six edition launch
  profiles through CLI/menu in v1/v20/v21, and six original object corpora
  all pass. No missing-media skips. These checks do not prove emulator
  pixel/timing parity or complete gameplay.

- 2026-09-06: Connected successful mouth and alternative consumption to the
  edition-owned C08 swallow sound transport. Twenty original Atari/Amiga
  cases pass, asserting accepted C08 for food and alternative water use and
  no sound request for an empty waterskin. This is transport acceptance,
  not emulator waveform/timing parity. Rebuilt after the RAM build was lost.

- 2026-09-06: Removed non-source skill XP from DM1 alternative potion use
  per PANEL.C F0349. Original antivenin reproduced the XP mutation before
  the fix; twenty Atari/Amiga cases now preserve all recipient lifecycle
  skill records through both consumption routes. Other games are unchanged.

- 2026-09-06: Verified alternative UseItem with original charged waterskins:
  decremented raw charges, water cap, empty rejection, recipient isolation
  and retained equipment object across five Atari/Amiga editions. Twenty
  original-media cases and the live-transaction unit test pass. Controlled
  equipment placement does not prove pickup/load or swallow-sound parity.

- 2026-09-06: Routed DM1 alternative junk consumption through the original
  inventory transaction instead of legacy placeholder amounts. Original-food
  tests also exposed missing raw-record release; both consumption routes now
  mark consumed food free per F0349. Twenty Atari/Amiga cases pass, including
  alternate food amount, recipient isolation, slot removal and raw release.
  Sound and full input/load coverage remain separate open work.

- 2026-09-06: Fixed alternative UseItem antivenin consumption to cancel
  the recipient's pending poison events and counter via F0323. Original
  Atari media reproduced the retained-counter failure before the fix;
  twenty Atari/Amiga cases now cover both mouth and UseItem routes while
  preserving another champion's poison event.

- 2026-09-06: Fixed ROS/KU/DANE/NETA statistic gains to use F0348's
  PC3.4/S1.2+ diminishing returns above 120 and 150 and cap at 170.
  Twenty-four explicit potion/boundary cases pass alongside existing
  consumable tests and twenty original-media regressions. Original
  ROS/KU/DANE/NETA records from five Atari/Amiga editions also pass mouse
  press/release consumption at statistics 151 and 169: reduced gain,
  cap, recipient isolation, empty-flask conversion and retained hand object.
  These RAM-controlled scenarios do not establish emulator or full-game parity.

- 2026-09-06: Expanded the existing live M11 lightning impact regression
  to both party and individual recipient shield. Both reach projectile
  consumption and the expected 86 HP through AdvanceProjectilesOnce;
  the full action/stamina runtime suite passes. This controlled corridor
  fixture does not replace original-media fire-impact parity evidence.

- 2026-09-06: Added independent F0307/F0313/F0321 damage expectations
  to shield-layer tests: fire 202 with body protection versus 239 without,
  magic 118 (body-scale bypass), normal 128. The RNG fire path consumes
  six slot-defense draws. New and two existing projectile tests pass;
  live original-media builder/impact verification remains open.

- 2026-09-06: M10/M11 combat snapshots now retain separate runtime
  body-shield and elemental-subtraction layers. F0733/F0733b use the body
  layer; legacy 76-byte serialization rejects unrepresentable split
  snapshots without writing. Integrated rebuild and 21 targeted tests
  pass. Actual projectile damage comparison remains open.

- 2026-09-06: Completed a static trace of YA shield into both combat
  snapshot builders and documented their conflated body/spell shield
  representation. The repair and damage proof remain open in TODO-dm1;
  this audit does not claim the combat defect is fixed.

- 2026-09-06: Original YA tests now advance every ordinary idle tick
  through C72 expiry while inventory remains open. All twenty cases
  preserve the shield until the due processing tick, then subtract only
  the potion delta from its recipient. Leader and party shields remain
  unchanged in both normal and above-50 diminishing-return cases.

- 2026-09-06: Fixed YA potion ownership: both M11 consumable bridges
  now read/write the recipient's champion shield, not party shield, and
  C72 records that recipient as Priority. Five original editions reproduced
  the wrong owner. Twenty cases now pass, verifying source power/duration,
  reduced gain above 50 defense, unchanged leader/party shields and C72
  owner. Actual expiry and damage-defense integration remain to be tested.

- 2026-09-06: Fixed original C10 antivenin leaving pending poison and
  its counter active after drinking. PANEL.C F0346 now reaches shared
  F0323 cancellation for the displayed inventory owner. All twenty
  original-media cases pass, preserving the other champion's dose,
  counter and pending event byte-for-byte. Events are controlled RAM
  fixtures; the antivenin Things are allocated original media records.

- 2026-09-06: Original allocated C15 water flasks now pass cross-owner
  mouse consumption on five Atari/Amiga editions. All twenty cases prove
  +1600 water to the displayed champion, unchanged leader water, same
  held Thing transformed to C20 empty flask and leader-owned final weight.
  Weight comparison shares F0140; potion status effects remain separate.

- 2026-09-06: Extended original waterskin cases to uncapped +800 water
  from -1024 and rejection of an empty skin while dehydrated. This removes
  the prior capped-only blind spot, where an accidental drink on empty
  could be hidden by the 2048 cap. All twenty cases pass.

- 2026-09-06: Twenty original-media cases now drain a genuinely allocated,
  charged waterskin through mouse press/release into the non-leader's
  inventory mouth. Verified the 2048 water cap, unchanged leader water,
  one charge per click, empty-skin rejection, retained held Thing and
  leader-owned changing weight. Expected weight still shares F0140;
  the capped fixture does not independently prove the uncapped +800 amount.

- 2026-09-06: Final champion death now runs through an actual due C75
  and AdvanceIdleTick in all twenty original-media regressions, rather
  than setting zero HP and calling the death probe. Verified zero HP,
  both party-dead flags, closed inventory, cleared caster input and removal
  of the old and newly rescheduled poison chains. The controlled event
  and one-HP setup are RAM-only; full emulator/death-screen parity remains
  unproven.

- 2026-09-06: The inventory-open regression now injects a C75 into the
  real-media world's timeline and uses AdvanceIdleTick, not direct damage.
  All twenty cases verify attack 128 deals two HP only to its owner,
  keeps inventory open, and queues attack 127 for the processing tick +36
  with the original C75 tag, champion and current map. The later direct
  death dispatch also removes that actual rescheduled event. This does
  not yet prove lethal poison dispatch or emulator visual parity.

- 2026-09-06: Fixed the DM1 inventory overlay incorrectly stopping idle
  simulation. GAMELOOP.C:81-128 changes rendering while inventory is open
  but still applies damage and advances game time. Five original-media
  cases reproduced the pause; all twenty death-owner regressions now pass
  with an ordinary idle tick advancing while the owner panel stays open.
  Other games' overlay policy is unchanged; full damage/timeline and
  Modern presentation sequences still need broader verification.

- 2026-09-06: Reproduced retained C75 events after death on all five
  Atari/Amiga editions. F0319 now removes the dead champion's poison
  events using stable queue compaction, following F0323. All twenty
  death-owner tests pass, checking unrelated payloads and equal-time
  ordering as well as the cleared poison counter. Pending events are
  controlled RAM-only fixtures on original game media, not emulator traces.

- 2026-09-06: Registered twenty original-media death tests covering
  independent leader/caster ownership on five Atari/Amiga editions. All
  pass, including preserved living-caster input and final-death spell
  clearance. Final death now preserves caster selection as F0319 does.
  The preceding 13-test full object/startup regression also passed before
  this final-death-only adjustment; the rebuilt adjustment passed the
  twenty targeted cases. No emulator pixel-parity claim is implied.

- 2026-09-06: F0319 now clears the dead champion's spell input and
  independently transfers magic-caster selection to a survivor, restoring
  that survivor's pending runes instead of retaining the dead UI buffer.
  Five original Atari/Amiga media probes pass in both leader-death and
  non-leader-death variants. This is a direct death-dispatch regression,
  not an emulator or full-timeline parity claim.

- 2026-09-06: Leader-death probes now retain a third distinct original
  weapon in the mouse hand and verify its identity/weight pass to the
  survivor while the dead champion's load becomes zero. All five editions
  pass both leader and non-leader death variants; weight values share F0140.

- 2026-09-06: Reproduced missing survivor direction alignment in five
  original-media leader-death probes. F0319 now applies F0368's direction
  rule to the replacement leader. Both default non-leader death and
  FIRESTAFF_VERIFY_LEADER_DEATH=1 variants pass for all five editions.
  Held-item transfer and full tick scheduling are not covered by this case.

- 2026-09-06: Reproduced stale load 2 after two-object death drops in all
  five Atari/Amiga editions. F0319 now refreshes carried load/hash after
  drop and leader fallback. All five regressions now reach load 0 and
  retain repeat-call stability. Dying-leader held-item transfer remains
  separate from this non-leader death test.

- 2026-09-06: Five original Atari/Amiga death tests now drop two distinct
  original weapons from source backpack 13 and ready hand 0. Floor-chain
  traversal verifies each exactly once and backpack before hand, matching
  G0057/F0318 and F0163's append semantics. All 30 slots and non-compact
  diagnostic layouts remain outside this ordering proof.

- 2026-09-06: Corrected F0318's source-slot/host-slot mismatch: translate
  G0057 C00..C29 through the inventory mapping before reading host slots.
  Build and five original-media death/ownership tests pass. Those currently
  drop one weapon; multi-item list ordering still needs dynamic proof.

- 2026-09-06: Five original Atari/Amiga death probes traverse the current
  square's Thing chain and find the dropped original weapon exactly once
  with the dead champion's cell bits. Bounded traversal rejects cycles.
  Head/link decoding shares the engine helpers; full inventory drop order
  and independent original-runtime comparison remain unverified.

- 2026-09-06: Five original-media death probes verify that the dead owner's
  ready hand is emptied and a second death-handler call leaves the world
  hash and dropped original weapon's next link unchanged. This checks
  repeat-call stability, not independent full floor-chain reachability.

- 2026-09-06: Five original-media tests now invoke the runtime death
  handler after setting the separate inventory owner to zero health.
  Panel closure, cleared owner, handled-death flag and unchanged living
  leader pass. One idle tick did not dispatch death in this fixture;
  the test uses ProbeCheckPartyDeath and does not prove tick timing,
  dropped-item chain integrity or resurrection behavior.

- 2026-09-06: Corrected F0319 inventory-close selection to use G0423's
  owner rather than the leader. Close the chest cache before dropping the
  dead owner's inventory and clear transient eye panels. Build and five
  existing ownership regressions pass; actual death/drop sequence coverage
  is still open and is not proved by these tests.

- 2026-09-06: Corrected the remaining inventory damage-overlay lookup to
  use the displayed inventory owner. Five original Atari/Amiga viewport
  comparisons pass with different leader/inventory damage timers. This
  does not establish independent C016 visual parity or damage timing.

- 2026-09-06: After enabling inventory/leader separation, all 13 broad
  regressions pass without skips: six original object/scroll corpora,
  PC3.4 application startup and six legacy presentation suites covering
  36 CLI/menu launches. This is regression coverage, not full-game parity.

- 2026-09-06: Promoted inventory/leader isolation into the default five
  original-media ownership tests. Normal mouse input opens, switches,
  closes and reopens both inventory owners; every press/release preserves
  the leader and held Thing. All five pass; no opt-in isolation flag remains.

- 2026-09-06: Enabled separate DM1 inventory selection in normal panel
  input. All five previously failing inventory/leader isolation probes now
  pass, as do the default five ownership regressions. Panel close clears
  selection; owner switching closes the old chest before assigning G0423.
  Broader startup/corpus and multi-owner close/reopen checks are pending.

- 2026-09-06: Migrated DM1 top-row redraw/ownership input records to the
  explicit inventory owner; leader name-color selection remains leader-owned.
  Five original-media regressions pass. Independent top-row highlight
  pixel evidence is still required when activating normal panel input.

- 2026-09-06: Five original Atari/Amiga tests now feed an allocated original
  food item to inventory champion 1 while champion 0 leads. Source G0242
  nutrition reaches only the recipient; leader identity, remaining weapon,
  empty mouse hand and final loads are checked after release. Water and
  potion-specific recipient/effect coverage remain open.

- 2026-09-06: Migrated mouth consumption's champion lookup to the explicit
  inventory owner, preserving leader-hand object selection. Build and five
  existing original-media ownership regressions pass. These tests do not
  exercise cross-owner consumption; that verification remains open.

- 2026-09-06: Migrated inventory main rendering and food/water rendering
  to the explicit owner. Five Atari/Amiga tests compare the 224x136 C017
  viewport for the same inventory owner with different leaders. All pass;
  the comparison shares the renderer and is not an emulator pixel oracle.

- 2026-09-06: Migrated empty-hand eye selection and statistics rendering
  to the explicit inventory owner. Five original Atari/Amiga tests pass
  cross-owner eye press/release and invalid-owner rejection. This verifies
  panel activation, not independent pixel comparison of another owner's
  statistics. Normal inventory opening remains on the migration backlog.

- 2026-09-06: Migrated action-hand scroll text selection to the explicit
  inventory owner while retaining leader-hand eye inspection. Five original
  Atari/Amiga tests decode all original scrolls from champion 1's action
  hand with champion 0 still leading. Expected text shares F0509: this
  verifies owner selection, not an independent text-decoder oracle.

- 2026-09-06: Action-hand chest opening and eye-release action-hand lookup
  now use the explicit inventory owner. Five original Atari/Amiga tests
  open a real chest on champion 1 without changing leader 0. The probe
  checks opening only; cross-owner eye release and close-time chain changes
  still need dedicated coverage before activating normal-input ownership.

- 2026-09-06: Migrated inventory slot transactions to the explicit owner
  accessor. Five Atari/Amiga tests exchange distinct original weapons
  between leader hand and another champion's ready hand in both directions,
  checking identities and separate loads on press/release. The owner is
  selected directly by the test; normal panel opening is not migrated yet.

- 2026-09-06: Added a separately validated DM1 inventory-owner ordinal and
  migrated inventory icon lookup. Five original-media tests verify another
  champion's item without changing the leader, invalid-owner rejection,
  and the explicit legacy fallback. Normal input does not set the ordinal
  yet; this is migration infrastructure, not the inventory-isolation fix.

- 2026-09-06: Added an opt-in original-media diagnostic for inventory/leader
  isolation and reproduced the index conflation on all five Atari/Amiga
  editions. Diagnosis is complete; the runtime correction remains in TODO-dm1.

- 2026-09-06: Rebuilt the application after leader fixes. PC3.4 native
  startup passes, plus 36 CLI/menu launches across Atari EN/DE/FR,
  Amiga EN and FM Towns EN/JP in Original/V2.0/V2.1. These establish
  authenticated dungeon handoff and retained presentation mode, not
  complete visual or emulator parity.

- 2026-09-06: Fixed missing direction alignment when selecting a different
  DM1 leader (CLIKCHAM.C F0368:67). All five Atari/Amiga regressions failed
  with deliberately divergent champion facing before the fix, then passed
  two-way keyboard and original-layout mouse selection afterward.

- 2026-09-06: After the leader-selection fixes, all six complete original
  object/scroll corpus tests pass: PC3.4, Atari EN/DE/FR and Amiga EN/HD.
  No media skips; this supplements the five targeted load regressions,
  not full gameplay or emulator parity.

- 2026-09-06: Fixed DM1 leader selection admitting zero-health champions,
  following CLIKCHAM.C F0368:55. Five original Atari/Amiga tests reproduced
  the keyboard defect before the fix and now verify both keyboard and
  mouse rejection without transferring held weight. Health is controlled
  in RAM; this does not verify the complete death/resurrection lifecycle.

- 2026-09-06: Fixed DM1 source-layout champion name clicks opening inventory
  instead of selecting the leader (CLIKCHAM.C F0367). Five Atari/Amiga
  original-media regressions now pass two-way mouse selection, checking
  immediate load transfer on press and stability on release, without
  opening inventory. Modern composed-HUD geometry remains separate.

- 2026-09-06: Reproduced stale held-object load after keyboard leader
  cycling in all five Atari/Amiga editions. Leader selection now publishes
  a refreshed load/hash immediately, following CLIKCHAM.C F0368. All five
  original-media tests pass a two-way keyboard leader switch before the
  floor drop, preserving the second champion's separate inventory weight.
  Direct mouse selection uses the same refresh but needs input-level coverage.

- 2026-09-06: Five original Atari/Amiga floor-drop load regressions now
  retain a distinct original weapon on a second champion. Normal leader
  pickup/drop/release preserves that champion's ready hand and load.
  This checks isolation, not leader switching or cross-champion exchanges.

- 2026-09-06: All five original Atari/Amiga floor-drop load tests now
  verify the explicit release event as well: load remains zero and the
  dropped Thing does not return to the hand. All five tests pass.

## 2026-09-06 — Carried-load transaction refresh

- Before publishing a DM1 transaction's world hash, recompute carried load
  from inventory and the active leader's held Thing using F0140. Open
  chests use live G0425 slots rather than stale linked contents. CSB's
  separately owned runtime is excluded. Invalid weight data prevents a
  partial load publication.
- The previously failing original Atari drop now reaches load 0. Five
  named `dm1_*_floor_drop_load_real` tests pass for Atari EN/DE/FR and
  Amiga EN2.0/HD, checking conserved slot-to-hand weight and its removal
  after a successful normal-input floor drop.
- Multi-champion, full weight-table and every-mutation coverage remain
  open; this corrects the reproduced defect, not all encumbrance parity.

- 2026-09-06: Added an opt-in original-media load diagnostic and reproduced
  stale load after normal Atari floor-drop input. This is diagnosis only,
  not a completed fix; reproduction and remaining work are in TODO-dm1.

- 2026-09-06: Audited original hand/slot load-update ownership against
  M11's direct hand bridge and recorded the missing dynamic verification
  in TODO-dm1. Existing identity tests do not establish load correctness.

- 2026-09-06: Documented the exact local legacy inventory test selection,
  media/skip requirements, oracle dependencies and public CI exclusion in
  `parity-evidence/dm1-legacy-inventory-verification-scope.md`.

## 2026-09-06 — Legacy occupied action-hand exchanges

- Each of the 606 allocated original objects now exchanges with a distinct
  allocated original weapon in the action hand, then exchanges back.
  Tests check both held and slot Thing identity after press and release.
- All five Atari/Amiga editions pass in Original/V2.1 together with the
  full existing all-slot corpus. Controlled resident placement is in RAM;
  no original archive is modified and no substitute objects are generated.
- Scope is occupied action-hand ownership, not occupied equipment-mask
  rejection, full drag gestures, rendering, load accounting or save parity.

## 2026-09-06 — Independent F0141 equipment-oracle indices

- The Atari/Amiga equipment oracle now decodes each object-info index
  directly from the normalized Thing bytes using DUNGEON.C F0141's
  category offsets, rather than calling the runtime index/subtype helper
  to choose the expected G0237 row. The runtime index is also compared
  explicitly to that independently computed value.
- All five original-media tests pass, including the complete all-slot
  placement/rejection corpus. Archive ingestion, dungeon normalization
  and slot coordinates remain shared dependencies; this does not prove
  complete original-runtime parity.

## 2026-09-06 — Legacy equipment-slot acceptance and rejection

- All 606 allocated original object records now visit all 30 inventory
  slots in Original/V2.1. Acceptance is computed from the selected
  archive's G0237 AllowedSlots word and DATA.C G0038:320–350 slot masks,
  independently of the runtime admission helper.
- Accepted placements are picked back up. Rejected placements must leave
  the original Thing in hand and the slot empty. Identity is checked after
  the press and after explicit release in both cases.
- All five Atari/Amiga editions pass: 61,580 input transactions each,
  307,900 total including rejected placements. Source geometry and
  object-to-info indexing remain shared; occupied swaps, drag sequences
  and complete original-runtime parity remain unproven.

## 2026-09-06 — Original legacy equipment-mask verification

- Read G0237 from each selected Atari/Amiga archive's original graphic
  559 into memory. Require exactly one complete table matching the
  source-defined Scroll/Chest/Mon Potion prefix; missing or ambiguous
  matches fail rather than selecting an arbitrary byte sequence.
- DEFS.H:1683–1688 defines the six-byte OBJECT_INFO layout. Its big-endian
  AllowedSlots word is compared against the runtime mask for each of the
  606 allocated original object records per edition. All five editions pass,
  together with the full hand/backpack transfer corpus.
- Object-to-info indexing remains shared. This verifies original mask
  values, not independent type decoding or body-slot input rejection.

## 2026-09-06 — Real-media read-error reporting

- Atari/Amiga media tests no longer convert every fopen failure into a
  missing-data skip. Only an unset/empty path or ENOENT returns 77;
  other open errors report failure and return 1.
- Rebuilt both binaries and verified missing-file and ENOTDIR paths.
  No game media was created, extracted or modified for these checks.

## 2026-09-06 — Legacy original-object hand/backpack corpus

- Extended the five original-media tests beyond scrolls to every allocated
  weapon, armour, scroll, potion, container and junk record (606 per
  edition). Unused records marked by a 0xffff next pointer are excluded.
- Each record visits the two hands and all 17 backpack slots in Original
  and V2.1 through pointer input. Held and resident Thing identity is
  checked after each placement/pickup press and explicit mouse release.
- Atari EN1.2/DE1.2/FR1.3 and Amiga EN2.0/HD each pass 46,056 transfer
  transactions: 230,280 total. Original archives remain unchanged.
- Scope: empty-slot transfers and release idempotence. This does not
  establish body-slot restrictions, occupied swaps, independent slot
  geometry, item pixel parity, save persistence or full gameplay parity.

## 2026-09-06 — Legacy scroll action-hand ownership

- Extended the five Atari/Amiga original-media tests to place and pick up
  each of the 35 original scrolls through action-hand pointer input before
  eye inspection. Both the held Thing and action-hand slot are checked
  after press and after the explicit left-button release API.
- All editions pass in Original/V2.1. This covers 700 transfer transactions
  and their releases without modifying the original archive. Other item
  classes, occupied exchanges and other slots still need equivalent
  original-media coverage on these platforms.

## 2026-09-06 — Legacy inventory border regression coverage

- The real-media Atari/Amiga checks now compare all 30 inventory slot
  borders against the original C033 32-pixel-stride source, cropped to
  18x18 as specified by CHAMDRAW.C F0291. Nontransparent border pixels
  are checked separately from the 16x16 resident icons.
- All five edition tests pass in Original/V2.1. The coordinate resolver
  remains shared; this checks source pixels/cropping, not independent
  layout geometry, transparent backdrop composition or whole-HUD parity.

## 2026-09-06 — Atari/Amiga inventory and scroll rendering

- Original C033 is stored as 32x18 pixels, whereas its visible box is
  18x18. The PC-only size gate had suppressed the inventory overlay.
  CHAMDRAW.C F0291:558–559,655–658 supplies the crop, source stride and
  C12 transparency. The legacy adapter now binds that original crop;
  the PC material gate remains strict and unchanged.
- Legacy scroll Y is a baseline, not a raster top. TEXT.C F0040:413,714
  subtracts four pixels before drawing, now reflected in the renderer.
- Tests initially failed at the undrawn panel on all five original-media
  editions. After correction, Atari EN1.2/DE1.2/FR1.3 and Amiga EN2.0/HD
  each pass 35 scrolls in Original/V2.1 (350 raster checks in total).
- The oracle uses original panel/font assets and independent baseline
  arithmetic. Line counting/font decoding are shared and transparent
  background pixels are retained from the actual frame. This is not a
  substitute for full same-state emulator comparison or full HUD parity.

## 2026-09-06 — PC3.4 scroll text anchor and baseline

- Read the original archive's C696 layout in memory and verified its CRC
  and GRAPHICS.DAT SHA-256 against the recorded source table.
- C560 resolves to (163,86); F0341/F0644 place the first raster row at
  89-floor(7*n/2). The old renderer was one pixel left and three pixels low.
- The independent coordinate oracle failed on all 35 original scrolls
  before the correction. Afterward the entire real-object corpus passes
  in Original/V2.1, together with the scroll-material gate.
- See `parity-evidence/dm1-pc34-scroll-text-position.md`. This proves the
  source/data calculation, not emulator-capture or other-platform parity.

## 2026-09-06 — Source-owned scroll cell background

- Corrected scroll rendering to copy six columns with white background,
  following PANEL.C F0340 and TEXT2.C F0644:133–143. The previous five-column
  transparent renderer contradicted the original PC3.4 no-transparency blit.
- A source-corrected original-media oracle failed on scrolls 10, 22 and 27
  before the fix. The full PC3.4 Original/V2.1 object corpus and targeted
  scroll-material gate pass afterward. Vertical layout and emulator parity
  remain separate verification requirements.

## 2026-09-06 — Non-vacuous scroll font check

- The real-scroll raster test now requires a loaded original font and a
  nonzero ink-pixel count for every tested scroll. Original/V2.1 and the
  complete original-object corpus pass with these guards.

## 2026-09-06 — Original scroll text raster placement

- The PC3.4 scroll eye corpus reconstructs the C023 panel and scroll-font
  ink with a separate character-remapping and pixel-placement loop. Its
  panel-area comparisons pass in Original and V2.1 alongside the full
  object corpus.
- Text layout and M653 pixel decoding remain shared with production. The
  comparison preserves actual pixels behind transparency, so it does not
  independently prove those pixels, font decoding or full emulator parity.

## 2026-09-06 — Original scroll panel border

- Every PC3.4 original scroll eye transaction now compares the visible
  two-pixel C023 outer border with the loaded original panel at its native
  destination. Original and V2.1 pass with the full object corpus.
- This is border/material evidence only: transparent pixels and the inner
  text area are excluded, and glyph parity remains open.

## 2026-09-06 — Original scroll text ownership

- Every original PC3.4 scroll tested through the eye now requires M11's
  decoded source text to match that scroll's own C02 reference with F0341's
  SCROLL/DECODE_EVEN_IF_INVISIBLE flags. Original and V2.1 pass with the
  full original-object corpus.
- This verifies owner/reference selection using the shared text decoder;
  it does not independently validate decompression, localization or glyph
  pixel parity.

## 2026-09-05 — Three-mode legacy launch matrix

- Added V2.0 to the existing Original/V2.1 original-media CLI/menu matrix.
  All 36 launches pass across Atari EN/DE/FR, Amiga EN and FM Towns EN/JP,
  retaining the requested presentation and reaching a loaded dungeon.
- Japanese FM Towns also asserts its own program/graphics fingerprints.
  These checks do not establish full gameplay, animation timing or pixel
  parity, and do not cover every historical edition of these platforms.

## 2026-09-05 — Japanese title media failure classification

- Missing archives skip explicitly, but unreadable existing archives or
  archives without a readable BIN now fail the JDM title test. Verified
  original media passes, an existing non-archive returns 1, and a missing
  path returns 77. No new game data or extracted files were created.

## 2026-09-05 — FM Towns launch and Japanese title evidence

- Rebuilt Firestaff passes the original ZIP CLI/menu English and Japanese
  launch/input matrix, including selected program/graphics hashes and CDDA
  track status. This is not an emulator timing or full-frame comparison.
- The original JDM title-receipt test passes. Its assertions now stay active
  under NDEBUG, and unavailable media returns CTest's skip code 77 instead
  of a false pass. CMake recognizes that skip explicitly.

## 2026-09-05 — Original chest owner transitions

- The original PC3.4 corpus now refreshes the same open chest after each
  pickup, requiring holes to survive, and switches to another chest while
  holding the first resident. It verifies the held Thing, old owner's full
  remaining chain and unchanged next owner's head after close.
- Original and V2.1 pass together with the complete original-object corpus.
  Test placements are restored in memory; save/resume remains out of scope.

- Bound the stripped Japanese FM Towns `JDM.EXP` title path to its unique
  disassembly fingerprints at `0xc428`, recovered rectangle/data owners, and
  exact C12/C13/C14 RGB6 records. The native title presenter now accepts the
  Japanese title only through the real in-memory ZIP/BIN/ISO receipt; it has
  no default/VGA palette fallback. See
  `parity-evidence/dm1_fmtowns_jdm_title_palette_binding.md`.

- Fixed the native English FM Towns title palette handoff. The authenticated
  EDM.EXP P3 load image now supplies the exact C12 PRESENTS and combined C13
  DUNGEON/C14 MASTER RGB6 transactions used at their original title-frame
  boundaries; stale host/PC palettes are no longer allowed. See
  `parity-evidence/dm1_v1_fmtowns_title_palette_transactions.md`.

## 2026-09-04 — F0125–F0127 targeted D0 transactions

- Replaced the class-wide terminal D0 callback pass with three explicitly
  targeted scheduler transactions in ReDMCSB order: D0L, D0R, then D0C.
  Each side square now completes its F0104/F0115/F0113 tail before the next
  square begins; D0C retains its split around F0115 and the final F0113 field.
- The implementation continues to use the mounted PC 3.4 GRAPHICS.DAT
  consumers. No generated bitmap, extracted runtime asset, or second replay
  was introduced.

## 2026-09-04 — Removed broad post-scheduler stairs ownership

- Deleted the remaining broad D3--D1 stairs fallback after square replay.
  The authenticated-plan failure path already returns before viewport
  background composition, so that branch was unreachable; retaining it gave
  F0104 stairs a second, source-invalid ownership path.
- D3--D1 stairs are now exclusively consumed from each owning square's
  authenticated foreground callback using mounted retail GRAPHICS.DAT data.

## 2026-09-04 — F0128-owned F0104 door frames

- Split native centre and side door-frame material out of F0111 into the
  scheduler's explicit F0104 door-frame callback phase. The live order is now
  F0108, DOORPASS1, frame, optional F0110, F0111 panel, DOORPASS2.
- F0111 no longer redraws frame pixels. Open doors retain their frames, and
  the same source plan covers normal side, centre, and D3L2/D3R2 routes. The
  real PC 3.4 HoC probe requires separate frame and panel receipts.

- Modern temporal post-processing now applies movement blur, phosphor
  persistence, and the next-frame history snapshot in one full-resolution
  traversal. The operation order and idle-frame history semantics are
  unchanged, while the all-effects path no longer performs three separate
  high-resolution memory passes per presented frame. Original mode never
  enters this optional filter path.

## 2026-09-04 — F0128-owned F0110 door-button routes

- Added an explicit F0110 scheduler step only to the four ReDMCSB call sites:
  F0117 D3R and the centre F0118/F0121/F0124 routes. The callback consumes it
  between the plan's frame and F0111 steps; D3L, D2L/D2R and D1L/D1R remain
  source-authentic no-button-call routes.
- Deleted the direct D3R and centre button replay. The real PC 3.4 HoC probe
  finds retail map 0 door `(1,2)` from party `(0,2)` facing east and proves
  one callback-owned F0108, DOORPASS1, F0110 and F0111 step in that order.

## 2026-09-04 — Door-front F0108 source boundary

- Split door-front floor ornaments out of the generic post-door foreground
  phase. Their authenticated scheduler step now rasterizes before
  `DOORPASS1`, followed by the source frame/F0111/DOORPASS2 transaction.
  Corridor, pit and stair F0108 steps remain in their ordinary plan-ordered
  foreground path; no primitive-class batch or second ornament draw exists.
- The real PC 3.4 HoC test finds an unmodified retail door and requires one
  pre-door F0108 callback receipt before accepting its DOORPASS1 and F0111
  receipts. F0113 remains owned by the same square after its final F0115
  consumer.

## 2026-09-04 — F0128-owned F0107 ornament projections

- Moved the authenticated 13-row D3--D1 wall-ornament family into the
  per-square callback. D3/D2 side routes consume their side-facing F0107
  projection followed by their front-facing projection; centre and D1 routes
  consume their sole projection. The three broad hand-written ornament
  replay calls are gone.
- Corrected the scheduler's F0107 cardinality from an unconditional pair to
  the exact ReDMCSB function bodies: two calls only for D3/D2 normal side
  squares, one for outer, centre, and D1 walls. The real PC 3.4 HoC gate now
  requires callback-owned F0107 material alongside wall, D0, and foreground
  work. F0676/F0677 outer walls and F0122/F0123 D1 side walls now also ignore
  the F0107 return exactly as source does, preventing a false alcove Thing
  pass on routes that immediately return. MEDIA720 D3L2/D3R2 now use the
  separately authenticated item-696 C1004 layout anchors; they do not index
  or infer nonexistent rows from the legacy 13-row G0205 source table.

## 2026-09-04 — F0128-owned F0111 door transactions

- Moved every D3--D1 F0111 door transaction from the hand-written viewport
  replay into the authenticated per-square scheduler callback. Center, side,
  and exceptional D3L2/D3R2 routes now consume their native frame, panel,
  ornament, mask, flip and opening-state material only when the owning plan
  span emits `F0111_DOOR`; no duplicate F0111 raster path remains.
- Preserved F0110 exactly where ReDMCSB places it before F0111, including the
  exceptional D3R button; it is now owned by its explicit scheduler step.
  The real PC 3.4 HoC runtime test searches the
  unmodified mounted dungeon for a door and now requires both `DOORPASS1` and
  F0111 callback receipts from that original square.

## 2026-09-04 — PC 3.4 SND3 event-table source closure

- Replaced the guessed priority, distance and period fields in the 35-entry
  DM1 sound table with the exact ReDMCSB I34E `DATA.C` MEDIA719/MEDIA712
  rows. This includes the distinct period for entrance-door event 3 and the
  source periods for party damage, War Cry, Blow Horn and movement sounds.
- The real-media gate reads `DATA/GRAPHICS.DAT` directly inside the supplied
  PC 3.4 ZIP, proves every event maps to its source `SND3` item, and decodes
  all 35 event aliases from the authentic 33-sample bank at 6000 Hz. Invalid
  host emission indices now fail closed instead of entering the audio route.
  No game-data member is extracted and no procedural sound is accepted as
  proof.

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native direct-CLI and start-menu matrix against the staged
  original media. PC DOS 3.4; DOS English and French; Atari ST English,
  German, French and nested archive routes; Amiga HD and v2.0; and FM Towns
  all reached their intended native handoff in memory. The French RAR 2.0
  package retained its explicit unsupported-format diagnostic rather than a
  false missing-media result.

## 2026-08-30 — Amiga v2.0 original save-disk provenance

- The startup menu's Continue entry and direct CLI `--save` now accept the
  authenticated `ZIP → ZIP → ADF → DMGAMEG.DAT` path. M12 validates the
  actual Amiga F0435 envelope in RAM, and M11 routes the virtual path straight
  to the format-specific loader instead of attempting `fopen`. The direct
  Amiga boot probe resumes the real save at tick 292 with no media extraction.
- Added an in-memory receipt for the ordinary save disk retained inside the
  supplied Amiga v2.0 preservation ZIP. The selected ZIP → ZIP → ADF is
  SHA-256 `5679f789655ba3f53f6275137fc80f59eb798b03b88f801e260aed352b6709c9`
  and contains the original `DMGAMEG.DAT` and `DMGAMEG.BAK`, each 49,002
  bytes; no member is written to disk.
- Its primary header verifies as a ReDMCSB-compatible original save family
  (format 5, platform 3, dungeon 10) with a valid header checksum. This
  proves real saved-session material is present, while correctly keeping
  framebuffer/Copper-palette evidence capture-gated.

## 2026-08-30 — Native Amiga RGB4 palette producer

- Recovered the exact dynamic producer from the supplied English Amiga v2.0
  `dm` program without extracting the ADF: the real executable has one
  producer at `0x14306`, copies the caller source table to its work table,
  changes each RGB4 component by one or two, and invokes the Copper-list
  builder through the original `0x14434 → 0x14140` call.
- Added a native, source-gated eight-frame implementation. It accepts only
  verified caller-owned 16-word Amiga RGB4 tables, never invents a palette,
  and is regression checked against the original executable's in-memory
  control flow.

Reviewed 2026-08-29. Completed work only.

- The supplied Amiga 2.0 ZIP→ADF `graphics.dat` receipt now decodes an
  authenticated big-endian IMG1 record into original 4-bit palette indices
  in memory. The Amiga wrapper selects the legacy decoder's big-endian path
  explicitly, preventing an FM Towns/PC byte-order fallback.
- With that original ZIP→ZIP→ADF source present, the Amiga graphics test no
  longer builds a structurally valid replacement `GRAPHICS.DAT`. Positive
  decode and format coverage comes only from the authenticated ADF member;
  compact malformed-header checks remain solely as rejection boundaries.
- The supplied FM Towns ZIP now has a RAM-only CDDA payload receipt. It
  follows the source CUE's first audio index from the MODE1/2048 data region
  into the shared raw-audio BIN, validates every track interval, observes
  PCM in music tracks, and proves documented track 20 silence.
- The supplied FM Towns ZIP also supplies the TMenu input receipt directly:
  Firestaff follows ZIP→CUE→BIN→ISO9660 to `TMENU.EXP` in RAM and validates
  the Phar Lap header plus original poll, initialization, and TBIOS entry
  bytes used by the native input schema.
- The supplied DM1 Amiga 2.0 preservation chain now has a direct real-media
  graphics-format receipt. `test_dm1_v1_amiga_graphics_dat` reads the
  selected ZIP → ZIP → ADF `graphics.dat` member in RAM through the native
  AmigaDOS OFS reader, validates the actual 575-entry Amiga layout, and
  identifies it as the known English 2.0 format. It never copies game data
  to disk. Rendering/pixel comparison remains separate active work.
- The real Amiga ZIP → ZIP → ADF and Atari ZIP → ZIP → STX start-menu paths
  now publish their admitted source decoder (`IMG2` and `DMCSB1` respectively)
  only after the original graphics and dungeon pair have both bound.
- The authentic PC DOS 3.4 ZIP start-menu path likewise publishes its admitted
  `IMG3` source decoder only after its original graphics and dungeon pair bind.
- The supplied DOS-EN archive's nested lowercase `dungeon-master/dmaster/DATA`
  layout is now covered independently.  It binds the same authenticated PC
  3.4 graphics/dungeon pair and reaches native CLI, menu, and movement without
  unpacking the original archive.
- The manually unpacked authentic French PC DOS `EUDATA` route now receives the
  same direct start-menu `IMG3` handoff check; its unsupported RAR 2.0 wrapper
  remains a separate diagnostic boundary.
- The authentic German Atari ST 1.2 and French Atari ST 1.3 packages each now
  require their direct start-menu `DMCSB1` handoff before their existing native
  movement assertions run.
- The Amiga 2.0 preservation package now has its own real-media CTest. It
  verifies the exact ZIP → original ZIP → ADF selection, the `IMG2` handoff,
  and a post-menu native movement result rather than relying on the separate
  HD package's coverage.
- The supplied Atari preservation collection now selects only its `[!]`
  original member (`ZIP → ZIP → STX`) and reads its `GRAPHICS.DAT` and the
  release-specific `DUNGEON.DAT` identity entirely in RAM. Direct CLI, the
  startup menu and a native movement probe are covered by real-media CTests;
  cracked sibling images are never admitted.

- PC DOS 3.4 authentic archive startup reaches native DM1 runtime with a
  hash-verified real-media regression.
- M12 resolves the authenticated PC 3.4 data owner and production retains
  fail-closed behavior when required source data is absent.
- DM1 Amiga 2.0 English supplied as ZIP → ZIP → ADF is hash-verified and
  read entirely in RAM. Both direct CLI launch and the startup-menu route
  reach the bounded native runtime without extracting game data to disk.
- The supplied Amiga HD package (preservation ZIP → original HD ZIP → ADF)
  is admitted using the same authenticated Amiga 2.0 graphics/dungeon pair
  and reaches native runtime through direct CLI and the startup menu entirely
  in memory.
- Atari ST original media is source-locked and read in RAM: the supplied
  English 1.0a/1.2, German 1.2 and French 1.3 variants reach native runtime
  through direct CLI and start menu. The German 1.2 protected STX uses the
  verified image identity `0eff1c902ea155f19e4a177bb2ccac7d`, graphics hash
  `2bdc5f431f84c0ece738f54dbd787c3b` and dungeon hash
  `cea11d6e9f7e1698fc95329fe3fb0899`.
- The supplied FM Towns JA/EN archive is verified through both direct CLI
  and start menu, reaching `dm1-runtime` without media extraction.  The
  boot receipt requires the source-bound `TMENU.INF` selection and selected
  `EDM.EXP`/`JDM.EXP` MD5, rather than promoting generic DM1 movement as a
  native FM Towns handoff.
- The same original FM Towns ZIP now verifies the complete public input
  matrix from independent native sessions: forward, backward, turn left,
  turn right, both strafes, and action.  Every check follows
  ZIP → CUE → BIN → ISO9660 → TMENU → `EDM.EXP` in memory and asserts the
  observed initial runtime position, level ownership, and CDDA title track.
- The DM1 V2 movement/viewport regression now reads the canonical PC 3.4
  `DATA/DUNGEON.DAT` directly from its original ZIP in RAM. It no longer
  depends on an extracted corpus for its positive dungeon decode path, and
  the verified raw-map composition is used as the real-data evidence.
- The supplied Amiga HD preservation ZIP now verifies the complete first
  input matrix from independent original ZIP → ZIP → ADF sessions. Forward,
  backward, both turns, both strafes and action all retain the native runtime
  and observed source party state without a generated save or map.
- The English Atari ST v1.2 preservation chain now verifies the same complete
  first input matrix through fresh native ZIP → ZIP → STX sessions. The
  expected party states are observed from original media; no disk image or
  dungeon replacement is synthesized.
- The canonical PC DOS 3.4 ZIP now also verifies the complete initial input
  matrix through fresh native archive sessions. Its authenticated IMG3 title,
  dungeon and party state are retained in memory for every direction, strafe
  and action check; no extracted fixture or generated save is used.
- Corrected the four authentic PC34, Amiga HD, Atari ST and FM Towns input
  matrices: DOWN at the initial party position leaves the party in place, and
  the earlier expectation of entering (1,2) was false. PC34's original
  DUNGEON.DAT identifies (1,2) as a door. Added an UP-then-DOWN sequence to
  prove backward movement from the open neighboring tile. All four complete
  original-media CLI/start-menu tests pass, including FM Towns English and
  Japanese routes.
- The complete PC34 `F0381_MENUS_PrintMessageAfterReplacements` producer set is
  now closed. ReDMCSB `MENU.C` has exactly two calls for `C005_ACTION_FLIP`;
  the bounded F0407 plan owns their exact HEADS/TAILS source strings, rejects
  values outside `M005_RANDOM(2)`, and passes the selected msgid through the
  DM1 translation domain into the cyan C015 message area with the original
  70-tick lifetime. Both outcomes, Swedish translations, the live M11 action
  tail, and the original PC 3.4 in-memory media identity are regression-gated.
- Original-mode C015 presentation is Unicode-capable without replacing its
  preservation font. ASCII cells still come from hash-identified PC34 M653;
  translated Latin glyphs are decoded once per UTF-8 codepoint into the same
  six-pixel TEXT.C advance. A real in-memory PC 3.4 ZIP regression combines
  the retail GRAPHICS.DAT/M653 owner with Swedish U+00C5 and proves its exact
  raster and single-cell placement on the bottom C015 row.
- Original-mode wall inscriptions now have the same bounded final-presentation
  boundary. F0168's packed TextString span, decoded glyph bytes, hashes and
  M648 raster receipt remain untouched and are always the fallback. A
  media-and-index-qualified gettext key may replace only the final readable
  text; supported UTF-8 Latin glyphs occupy centered 8x8 cells while ASCII
  continues to come from retail M648, using M648's own opaque palette index.
  The canonical PC 3.4 ZIP test visits all 56 visible inscriptions, then proves
  TextString 4 `HALL OF / CHAMPIONS` as Swedish `HJÄLTARNAS / SAL`, including
  the exact U+00C4 raster and retained original material receipt.
- The DM1 V2 side-by-side viewport comparison now accepts the decoded
  original PC 3.4 dungeon state and renders its actual initial composition
  into both lanes. The real-data viewport regression no longer promotes the
  old hard-coded entry fixture as positive composition evidence.
- `dm1_v2_side_by_side_seed_pc34` now uses that same native in-memory ZIP
  path for every positive lane, layout, RGBA export, and region comparison.
  It skips when the optional local PC 3.4 archive is absent instead of
  silently falling back to a hard-coded dungeon fixture.
- `dm1_v2_v1_v2_side_by_side_seed_pc34` now independently reads the original
  PC 3.4 ZIP member in RAM and verifies V1/V2 disabled-presentation framebuffer
  parity at the authentic entry coordinates in all four directions. It no
  longer uses the legacy entry-state fixture for positive pixel evidence. The
  fixture implementation and its data-free V2 screenshot probes were removed,
  so this family has no synthetic-dungeon fallback.
- The supplied authentic French DOS `DMSAVE.DAT` and `DMSAVE.BAK` (48,561
  bytes each; SHA-256 `494d081ee5175b2dccc900d5ea3f25230c8bb3b0f20828d311b8fc5bdfb82d21`
  and `a760234408bf27946b1586ecf396be72e648bd8f3d18abee90a18c2c7e94421f`)
  now pass a native backed-save roundtrip. Each source save is loaded through
  F0435 against its supplied original French `EUDATA`, staged through F0433,
  exported, reloaded through F0435, and checked for party, C03/C04 timeline,
  active-group and dungeon ownership preservation. No save fixture is used.
- DM1 PC 3.4 F0446 endgame cadence now follows `ENDGAME.C`: every ordered
  victory-message F0445 redraw owns its immediately following 780-tick wait,
  and the final 600-tick wait runs only after the last message. These waits
  freeze gameplay time. Victory music fails closed unless the selected
  installation's authentic `SONG.DAT` is admitted, and the victory path keeps
  restart disabled exactly as F0446 requires. A focused runtime test locks the
  event/delay order and the 780 + 780 + 600 total.
- The authenticated PC 3.4 F0444 victory presentation now has distinct
  source-ordered phases: champion mirrors/portraits wait for input, accepted
  input starts authentic SONG.DAT C3 and C006 `THE END`, the 300-tick hold
  freezes gameplay time, and the victory route then presents authentic C005
  credits. All phases own the full logical screen, and missing F0444 material
  or SONG.DAT fails closed instead of selecting synthetic presentation.
- The separate PC 3.4 F0444 restart request no longer escapes to a generic
  host relaunch. It invokes the native F0435-compatible quick-load consumer
  in place: a valid admitted save clears the terminal/death state and resumes
  the restored world, while a failed load clears the request and returns to
  C3/C006 `THE END` for the source 300-tick hold using retained authenticated
  endgame material. If that material is unavailable the failure path stops
  closed. Focused decisions cover load success, load failure/re-entry, and
  missing-material rejection.
- M11 DM1 status/readout localization now has an explicit extraction
  boundary. Six live pickup/drop/spell literals are marked as DM1-owned,
  included in the 578-entry template, and translated into Swedish. The CI
  gate checks catalog presence, nonblank Swedish coverage, and active-game
  domain dispatch so shared CSB/DM2/Nexus/Theron literals cannot inflate the
  DM1 completion number.
- The next explicit M11 localization group covers the source-locked DM1
  CLIKVIEW.C fountain path: `FOUNTAIN`, `DRANK WATER`, `CONTAINER FILLED`,
  and `FLASK FILLED WITH WATER`. All four runtime literals are marker-gated,
  Swedish-translated, and included in the now 581-entry DM1 template without
  importing shared-file strings owned only by another game.
- The shared required-file catalog retains the verified Amiga 2.0
  `DUNGEON.DAT` identity alongside the Atari additions.  The post-scan
  recovery now republishes a selected ZIP → ZIP → ADF source owner before
  calculating availability, so an Atari preservation scan cannot make a
  valid Amiga launch unavailable.
## DM1 PC 3.4 MEDIA720 outer-D3 wall ornaments

- Recovered all sixteen authentic D3L2/D3R2 layout records directly from
  real `GRAPHICS.DAT` item 696 and wired them into F0128's per-square F0107
  callbacks. The native route preserves record alignment types, C30/C14
  scaling, D3 palette mapping, C10 transparency, and D3R2 horizontal flip.
- Ruled out item 558 rather than guessing from D3L/D3R: the admitted PC 3.4
  file has SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`;
  entry 558 starts at `0x25eed`, is 38 bytes compressed/decompressed, has a
  16x7 header, and hashes to
  `df28c5d26e9a7b87903ac817a21675c65ad52216e5deede4148f5286f4223e23`.
- Corrected Hall of Champions mirror admission for the source-owned outer
  D3 records. Internal view indices 13/14 now admit authentic C127 mirrors
  and consume base ornament C345 at the item-696 anchors; they are no longer
  suppressed as invalid aliases of G0205 rows 0/1. The real PC 3.4 archive
  regression covers all 15 matching mirror placements without fixtures.

## DM1 FM Towns Japanese native ZIP start

- Added an explicit `--dm1-fmtowns-ja` selection backed by the canonical
  FM Towns retail disc identities. The native in-memory route now selects
  TMENU -> `JDM.EXP`, reports its exact executable and Japanese GRAPHICS MD5,
  applies the recovered JDM title/palette owners, and reaches the runtime.
- Added the source-specific F20J `JDATA/DUNGEON.DAT` reader. The authentic
  33,931-byte Japanese file ends exactly after raw map data and has no F0434
  checksum trailer; all ordinary dungeon and save readers remain strictly
  checksum-validating. Real-media tests prove both the rejection boundary
  and successful 14-map Japanese materialization without extracting to disk.
# Local PC34 spell-panel background evidence (2026-09-06)

- Original Atari English, German and French archive tests passed with new
  source geometry assertions: C009 96x33 and C011 96x36. These establish
  required legacy bitmap dimensions, not the pending legacy compositor's
  pixel parity. Existing names, source sounds, scroll and object interaction
  checks in those three tests also passed without skips (111.14 seconds).
- Expanded successful-cast pixel/XP checks to six cases across Original,
  V2.0 and V2.1. The 38-test action/spell regression selection passed,
  including action stamina and source-route checks; two obsolete textual
  assertions were corrected to match I34 C009-only painting and VGA XOR4.
- The F0344/F0658 aggregate describes source-font rune zones rather than
  C011 bitmap copies. Its direct positive/negative test and original I34E
  archive gate both passed. This aggregate is not a full paint sequence
  and currently has no live production caller.
- The live effect, command ordering, input admission and capture lifecycle
  now bind C009 plus the original font, without a C011 companion. Original
  C101-C108 click-zone identities are unchanged. Legacy companion/count
  proofs are rejected in the focused negative tests.
- A successful Mon Oh Ir Ra cast now renders the same C013 pixels as the
  original-data panel at identical state. The original I34E test covers
  both recent-attack XP branches on separate source ticks. Its new render
  assertion exposed a cooldown mistakenly selecting an unopened action
  menu; MENU.C:778-797,2036-2039 supports retaining the icon path when
  G0506 is zero. Fourteen focused tests passed, including both original-media
  tests, without skips. This does not establish every cast/failure path.
- ReDMCSB CASTER.C:75-98 (I34) draws C009 without legacy C011 copies.
  The native painter now places the authentic 87x25 image at (233,50),
  retaining the full C013 click rectangle above it.
- `m11_dm1_spell_panel_real` passed against the original DOS 3.4 English
  archive: all pixels in C013 (87x33), four casters, sparse living slots,
  and Original/V2.0/V2.1 modes (24 cases), with original bitmap/font data.
  This proves the tested static panel, not spell-effect receipt integration,
  other editions, emulator parity or full-game completion. Publication is
  pending the integration and edition-routing work recorded in TODO-dm1.md.
# F0412 potion practice boundary regression (2026-09-06)

## F0209 wander movement gate

- Added eight blocked-move cases (four directions, terrain versus occupied
  destination) asserting no unlink/link and preserved active/event source
  coordinates. The teleporter/pit test passes with this added matrix.

- Added nonserialized group prior-square/time history shared by M10/M11,
  initialized at admission and source-row publication, retained through
  compaction and cleared at retirement/staged map replacement. History reads
  preserve unrelated context. Invalid coordinates fail before publication.
  Re-admission replaces old history for the same C04. Two dispatched C29/C37
  moves verify successive prior coordinates and timestamps in bounded RAM.
- The two-move regression exposed two additional defects: physical movement
  used creature facing instead of the chosen direction, and blocked retries
  published the rejected destination as active/event position. Corrected
  both. Preserved SourceCount as storage capacity, correcting the earlier
  retirement patch's live-count interpretation. Teleporter/pit 19-case gate,
  packed-direction runtime test, group bundle and 19 explosion/original-media
  CTests pass. This is not full original-emulator movement timing parity.

- Fixed active-row retirement to compact direction/home sidecars together
  with creatureAI, clear retired tail data and update the admitted source
  count. The last-group death regression failed before the fix. Added a
  neighboring-row sentinel regression through the real explosion death
  path; it verifies retained identity, direction/home data and cleared tail.
  All 14 explosion/group CTests pass. The sentinel is a bounded ownership
  fixture, not a second naturally loaded creature or emulator capture.

- Reconciled stale packed-direction runtime fixtures with PROJEXPL.C
  F0219:720-739: source C14 projectiles pass Fluxcage, and wall-impact
  explosions are published on the source square, not inside the wall.
  Failed explosion publication compacts the remaining square-head array;
  its surviving head is index zero. Corrected these assertions without
  changing production collision logic. Both packed-direction and C38
  projectile-aftermath runtime CTests pass, including drift/full-pool cases.

- Verified the prior-square admission rule with seeds 6 and 7: both select
  west, but only the zero two-bit prior-square sample admits returning
  west; nonzero falls back north. Both consume exactly one additional
  admission draw (GROUP.C:2164-2171). AI test: 2520 passed checks. Confirmed
  production dispatcher callers in the M10 orchestrator and M11 game view;
  this caller audit is not a full runtime movement-capture assertion.

- Corrected the ordinary C37 stationary look-around path to consume the
  admission draw before the smell-distance test, then a separate direction
  draw when last-move timing permits (GROUP.C:2217-2222). The seed-1
  regression previously failed facing and final-state checks; the rebuilt
  AI test passes all 2512 checks after correction. Negative reaction events
  and original emulator traces are not covered by this C37 fixture.

- Added four blocked-direction regressions for zero through three walls.
  A fixed source seed chooses west; fallback visits north, east and south
  in that order without extra direction draws (GROUP.C:2155,2199). All
  four cases retain the same final stream state. The rebuilt AI test now
  reports 2509 passed checks. Prior-square rejection, fully blocked groups
  and original-capture movement timing are outside these four cases.

- Corrected the inverted random-movement condition in the DM1 behavior
  dispatcher: GROUP.C:2153 moves on a nonzero one-bit sample. Two source-seed
  regressions (1: stay, 6: move) both fail before the correction and pass
  afterward. The complete creature behavior test reports 2493 passed checks,
  zero failures. This covers the decision in bounded RAM, not full C37
  scheduling, look-around RNG, CSB bridge behavior or emulator parity.

- Expanded post-fix verification: spell lookup, spell duration, potion
  power, spell-casting receipt and tick-source-ownership tests all pass
  after rebuilding their five targets. Added literal BASE.C seed-1 sample
  and second-state anchors to avoid relying solely on the runtime RNG helper
  when calculating the failure-path oracle.

- M10 now rejects unknown spell-table indices before consuming the XP RNG
  sample, following MENU.C F0412:1816-1826. Added a seed-preservation
  regression: it fails on the previous implementation and passes after
  moving the sample after lookup. Both focused F0412 tests and five DOS
  original-media C15 integration tests pass; unknown-command presentation
  is not claimed as original UI parity by this fixture.

- Audited M11_GameView_CastSpell: XP uses F0732(8), and paid validation
  uses F0732(128) with immediate first-failure return before flask admission.
  Added a low-skill M10 handoff regression with HAS_SPELL_XP: exactly one
  potion-power draw, no repeated XP/practice samples, unchanged supplied XP
  (420), and matching decoded/raw C08 mutation. Focused CTest passes.
  This is a source audit plus command-boundary fixture, not an original
  UI/input capture of the complete M11 path.

- Corrected M10 F0412 XP/practice samples from raw state to `state >> 8`,
  matching BASE.C F0027:1688-1695 and DEFS.H M003. The existing multiplier
  0xBB40E62D is correct for that game routine; CEDT002 is not its replacement.
  Both focused F0412 tests and all five original DOS C15 integration cases
  pass after rebuilding their targets. Updated stream expectations retain
  first/ninth failure, all-nine-pass flask gating, and raw potion mutation.

- Extended the runtime fixture with nine passing practice draws: absent
  flask consumes no potion sample or XP; a C08 flask in hand consumes one
  additional sample and updates decoded and raw potion type/power. Focused
  CTest passes. This verifies current-stream ordering and RAM mutation only;
  original execution trace comparisons remain in TODO.

- Corrected the failure-XP fixture's Wisdom Potion table index from 16
  (Stamina Potion) to 17. Added source-shaped receipt checks for nine
  successful threshold-equality probes (115), independent potion power
  masking (0xffff produces Mon power 255), and ninth-probe rejection at 116.
  Reference: ReDMCSB MENU.C F0412:1837,1853. The focused
  `test_dm1_v1_f0412_failure_xp_pc34_compat` passes. This is bounded
  source-contract evidence, not original emulator or complete RNG-stream parity.

- Fixed a normal M12 → M11 DM1 Atari ST launch stall. The Atari title path
  already renders authenticated GRAPHICS.DAT C001, but then recursively
  searched for optional PC/F20 TITLE.DAT provenance. Skipping that irrelevant
  scan for Atari follows ReDMCSB STARTUP1.C:160-170 and TITLE.C:309-409.
  A real Atari STX title reaches `dm1-runtime` in the normal start-menu path,
  loads level 0 and accepts turn/movement input; a 60-second menu launch
  completes in under seven seconds. This verifies that authentic launch
  route and its gameplay receipt, not all Atari campaigns or editions.

- Preserved the selected DM1 data root in M12→M11 launch handoff when the
  authenticated files use the canonical `<data-root>/dm1/GRAPHICS.DAT` layout.
  The version scanner had replaced the root with the inner `dm1` directory,
  causing four data-directory cache-invalidation contract assertions to fail,
  including saved game-leaf promotion and symlink roots. A narrow normalization
  now applies only when the matched asset directory is exactly one level below
  the selected root; explicit DM1 leaf paths, other layouts, and virtual
  archives retain their own owner. The focused cache-invalidation regression
  and the `firestaff` target build pass. Its synthetic hash fixtures exercise
  path ownership only; original-media runtime behavior was not exercised.
  Follow-up original-media verification: `dm1_v1_pc34_native_cli_boot` passes
  with the retail DOS EN 3.4 ZIP through CLI, start-menu launch, and native
  runtime input. The same shared-path change leaves the CSB FM Towns RAR and
  DM2 Mac retail CLI/start-menu regressions passing.

- Added a PC 3.4 original-media combat regression from Hall of Champions
  recruitment through the first door. Production inputs reach map 1 at
  `(6,1)` facing south with champion HP 47 and original group thing `0x10a9`
  at `(6,2)`; the occupied square blocks forward movement. Four action-row
  attacks use original action indices `6,7,8`. Group `0x10a9` remains after
  the first three rounds and disappears after the fourth. Forward movement
  then reaches `(6,2)` and `(6,3)`; the next wall blocks it. The group's
  original possession slot is `0xfffe`, so this test makes no loot claim.
  Focused real-media CTest `m11_dm1_pc34_first_monster_combat_real` passes.
  This verifies the Firestaff runtime route with retail media, not XP,
  attack-roll parity, or an original-executable capture.
# 2026-10-10 — Amiga 3.6 DM1 direct runtime route

- ReDMCSB `STARTUP1.C` calls F0437 before the platform startup path, while the
  available F0437 title and F0441 entrance receipts are A20/PC34-specific.
  Previously the M12 selected-launch router sent authentic Amiga 3.6 media
  through the PC34 transaction, which rejected its native virtual ADF.
- The selected DM1 Amiga 3.6 edition now uses the authenticated generic
  runtime route. The boot-probe exception is limited to the original
  `GRAPHICS.DAT` MD5 `7f9458e4a3972d06e649a6fa85a7f34b`; other DM1 editions
  retain the existing source-visible intro requirement. The receipt reports
  `dm1-runtime-direct` and marks the intro as bypassed.
- Repacked only the original ADF from the existing archival 7z into
  `Dungeon-Master_Amiga_36.zip` under the external user data directory; no game
  data was added to the repository. `dm1_v1_amiga36_original_media_cli_boot`
  passes for both `--game dm1 --platform amiga` and the start-menu route with
  that ZIP. Both reach a loaded runtime and report the authentic asset hash.
- This does not implement or verify Amiga 3.6 F0437 title playback, F0441
  entrance playback, viewport parity, or native HiDPI behavior. Those remain
  open in TODO.

- 2026-10-15: Final offline-selection verification passed on the exact built binary: full original-media CLI startup suite (214.31 s), both targeted CTest cases (26.02 s), and translation catalog consistency check. Physical volume unmount and macOS permission-dialog interaction remain separate manual checks.

- 2026-10-15: Fixed implicit `--boot-probe` ZIP admission. Reproduced rejection of authentic PC 3.4 default-root media before the fix; both offline preference cases now prove the exact original graphics MD5/member owner and loaded runtime, plus ordinary accepted movement. Full original-media startup diagnostics suite passed (213.97 s), including DM1/CSB/DM2 defaults, mixed DM2 platform selections and unsupported PC-98 rejection. Incremental build, shell syntax, catalog consistency and redacted Gitleaks checks passed. No visual-parity claim.

- 2026-10-15: Reproduced and fixed installation guidance overwritten after empty folder selection, in both synchronous and asynchronous completion. Original-media dialog regression now passes both empty cases followed by authenticated PC34 selection/persistence. Access-denied and missing-selected-root CTest cases passed (2/2, 25.70 s); catalog consistency and incremental build passed. Callback delivery is tested; native OS dialog presentation is not claimed.

- 2026-10-15: Fixed late cancellation of a completed-but-unpublished folder scan. A semaphore-gated real worker regression reproduced five failures before the fix (cancellation result, runtime root, selected directory and persisted configuration); it now passes, including an optional authenticated PC34 original-archive initial collection and runtime-owner preservation. Empty-folder guidance expectation is updated. The separate original-media dialog test passes. Two full five-game corpus cases were intentionally not run; the deterministic SDL interposition test is skipped on Windows, whose late-cancel execution remains unverified.

- 2026-10-15: Investigated Actions run 38804393070 Windows startup-contract failure: the only failing gate could not create its hardcoded `/tmp` scratch fixture. Its directory now uses a recursively created `.codex-scratch` under the test working directory. Targeted incremental build and CTest pass locally; this verifies the test harness path, not original-media startup or native Windows execution. Windows confirmation awaits Actions.

- 2026-10-15: Closed a DM1 virtual-graphics stale-identity gap: the loader now verifies the exact retained GRAPHICS.DAT buffer against the selected source MD5 before decoding. An original-only regression proved unchanged English startup succeeds, while the old code also launched after replacement with authentic multilingual graphics; the fixed code rejects that replacement. Repacking changes ZIP framing only, never original game bytes. Targeted real-media CTest passed (0.71 s); original Amiga 2.0 and Atari ST 1.2 probes retained their expected graphics hashes and loaded levels. Build, catalog consistency and independent source review passed. No full platform/visual parity claim; Windows symlink regression is explicitly skipped.

- 2026-10-15: Original-media version-row inputs reproduced silent substitution of missing PC Multilanguage with installed English. DM1/CSB/DM2 now preserve explicit same-platform edition choice at the launch gate/intent; AUTO and platform/version mismatch recovery remain. Completed scans recover stale saved rows before interaction, with deferred/cancelled scans excluded until inventory is known. Authentic mixed-EN/ML deferred, EN-only deferred and ordinary initialization cases pass, as do English startup and stale ZIP rejection (0.74 s). Broader original-media CLI startup suite passed (214.69 s); its ordinary noncancelled route preceded the final deferred/cancel guard refinement, separately covered by the exact final-source original-media test. Equivalent manual edition-row evidence for CSB/DM2 remains open.

Final-source verification for that batch: rebuilt app and cancellation test; selected-source/edition and missing-root CTest cases passed 2/2 (12.62 s). Deterministic cancellation with authentic PC34 initial media passed; its two full five-game corpus cases remained skipped. Catalog consistency passed; native Windows confirmation awaits the next Actions run.

- 2026-10-15: Original-data tests reproduced loose DM1 graphics replacement being loaded under the old English identity, then reproduced a rejected load leaving the partial world active. The binder now reads/authenticates/decodes the same retained bytes, keeps the exact selected pathname authoritative, and reuses previously authenticated native buffers. Canonical replacement and renamed-selected-file removal cases both reject without an active world; unchanged sources still launch. Final focused CTest passed (0.89 s). Original FM Towns English EDM handoff, Amiga 2.0 and Atari ST 1.2 loaded probes retained expected graphics hashes. Source review and catalog check passed. Windows original-media staging and full visual parity remain separate verification.

The final-binary original-media startup suite passed (213.43 s), including DM1/CSB/DM2 default discovery, mixed DM2 platform choices, unsupported PC-98 rejection and both offline remembered-root fallback cases. This remains startup/admission evidence, not full gameplay or display parity.

- 2026-10-15: Reproduced late FM Towns native-launcher rejection retaining an active world. Staged seven individual symlinks to original startup/data files, verified a complete English EDM/TMENU handoff, removed only the task TMENU.EXP link after scanning, and observed the inactive-state assertion fail before the fix. Native receipt rejection now shuts down the partial session and reports root/program; PC fresh-pool allocation failure uses the same teardown (allocation-failure branch not fault-injected). Both original-media lifecycle/selection CTest cases passed 2/2 (1.52 s), with no skips. Incremental build, catalog check and independent review passed. Native Windows staging and full display/gameplay parity remain open.

- 2026-10-15: Fixed loose FM Towns inventory advertising graphics/dungeon-only packages as ready. Existing native source receipts now filter each loose English/Japanese edition at scan time; virtual archives retain their existing intake. A same-handle 1 MiB startup-file bound precedes allocation. Reproduced missing-TMENU fresh-scan failure before the fix; original-data tests now reject both languages for missing TMENU, preserve Japanese when only EDM is absent, retain a valid PC sibling and recover restored original links. Required launcher filenames appear only when no playable edition survives. Two focused original-media CTest cases passed (1.72 s); verbose rerun had no missing-media skips. Menu cancellation and access tests passed; full five-game corpus checks in the cancellation executable remained skipped. No per-frame program hashing or original-media changes.

Final verification for loose FM Towns admission: the exact final binary passed the complete original-media CLI startup diagnostics suite (214.11 s), including both offline remembered-root cases. Actions run 38808058251 completed successfully across Windows, macOS and Linux for the preceding published batch. This does not establish complete visual or gameplay parity.

- 2026-10-15: Reproduced an incomplete first loose FM Towns launcher hiding a complete second installation of the same English edition. The scanner now considers up to 256 original hash-matched loose owners, retains ready-root order, and preserves missing-launcher diagnostics when none is ready. Shared root derivation keeps late startup checks consistent. Authentic symlink regression went red before the change and green afterward, including a loaded native menu handoff to the second owner, independent Japanese unavailability, and stale selected-owner rejection without an active world. The final expanded regression also preserves missing-launcher feedback when both copies are incomplete. The two focused original-media tests passed (1.82 s), with no skips; catalog consistency and independent review passed. Cross-root ordering is source-reviewed only; no visual parity claim.

The final duplicate-launcher binary also passed the complete original-media CLI startup suite (213.80 s). This verifies admission/default selection and diagnostic behavior, not visual parity.

- 2026-10-15: Authentic Japanese loose FM Towns startup reproduced an invalid `JDATA::JDATA/DUNGEON.DAT` handoff. Selected loose graphics now bind a case-insensitive dungeon sibling from that exact owner, and both loose/archive Towns dungeon buffers are authenticated against the selected language before dispatch to the English or Japanese decoder. ReDMCSB LOADSAVE.C F0435:2363-2367 distinguishes the original DATA/JDATA owners. Original-data tests now start both EDM/JDM worlds, reject English with authentic Japanese dungeon bytes after a stale menu scan, reject missing/wrong-language loose owners during inventory, preserve a playable Japanese sibling, reject both missing dungeons, and select a complete duplicate over a launcher-ready owner lacking its dungeon. Two final focused CTest cases passed (2.55 s), with no missing-media skips. Independent source review passed. Windows symlink staging and full gameplay/display parity remain open.

Final Towns language-owner verification: complete original-media CLI startup suite passed (214.48 s), including both offline remembered-root cases. Rebuilt menu access-denied test passed (14.08 s); deterministic cancellation passed with authentic PC34 initial media, while its two full five-game corpus checks remained intentionally skipped. Catalog consistency and redacted staged Gitleaks checks passed.

- 2026-10-15: Reproduced explicit loose FM Towns GRAPHICS.DAT selection silently changing to a complete nested duplicate when the requested owner lacked its dungeon. Both authenticated Towns languages now participate in the existing terminal loose-owner rule; native programs and language dungeon must belong to that exact file. Later Amiga/Atari publishers respect the same guard. Authentic tests cover exact-file scan/start, stale handoff rejection, and fresh incomplete-file rejection beside a complete duplicate. Two final original-media CTest cases passed (2.66 s), with no missing-media skips; independent source review passed. More specific missing-companion feedback for explicitly rejected owners remains open.

Final explicit Towns-owner binary passed the complete original-media CLI startup diagnostics suite (213.73 s), including both offline remembered-root cases. Catalog consistency and the exact final original-media tests passed. No original data bytes were modified or committed.

- 2026-10-16: Reproduced mixed loose FM Towns/PC required-file reports pairing Towns graphics with another owner's dungeon, and a second stale Towns dungeon receipt after fallback to PC. Reports now republish surviving graphics provenance and authenticated same-owner Towns or PC34 English dungeon provenance after native admission. The final validation clears a failed companion receipt instead of retaining a previous owner, and only decreases availability on missing required rows. Native PC handling is keyed to its original graphics hash; unrelated catalogue test hooks retain their existing behavior. Original-data EN→JP→PC report/availability assertions pass through both full and game-specific inventory scans. Two focused CTest cases passed (3.34 s), with no missing-media skips. Menu cancellation, access-denied and catalog consistency checks passed; two full five-game corpus cancellation cases remained skipped. Independent source review passed. Other edition report ownership and native Windows original-media staging remain open.

Final required-file report binary passed the complete original-media CLI startup diagnostics suite (213.65 s), including both offline remembered-root cases. Rebuilt access-denied CTest passed (14.12 s); deterministic cancellation passed with authentic PC34 initial media and its existing catalogue-hook cases. Two full five-game cancellation corpus cases remained skipped.

- 2026-10-16: Reproduced an explicit incomplete Towns installation reporting its valid GRAPHICS.DAT as missing. The selector now retains that exact authenticated candidate for the existing final native admission gate, which still clears rejected versions before materialization while preserving required-file evidence. Original-data tests cover missing and authentic wrong-language dungeons, missing TMENU, full/game-specific scans, and ordinary two-Accept menu input: only the affected companions are listed, no duplicate installation is substituted, and no runtime launch is requested. Two final original-media CTest cases passed (3.91 s), without missing-media skips; deterministic cancellation and independent source review passed. Broader per-selected-edition feedback and native Windows original-media staging remain open.

Final missing-companion feedback binary passed the complete original-media CLI startup diagnostics suite (214.36 s), including both offline remembered-root cases. Access-denied CTest passed (13.68 s); catalog consistency and deterministic cancellation passed. The cancellation executable's two full five-game corpus cases remained skipped. Redacted staged Gitleaks found no leaks.

- 2026-10-16: Reproduced three original-media PC34 multilingual owner bugs:
  explicit loose graphics admitted a neighboring English edition; stale starts
  borrowed its dungeon; virtual PC graphics overrode explicit dungeon paths.
  Scanner publication and runtime now retain the selected multilingual owner,
  authenticate the consumed default dungeon buffer, and honor explicit PC
  dungeon paths. The test independently compares the unchanged English and
  EUDATA standard dungeon bytes before staging them. Both scan APIs report the
  selected owner, missing/replaced standard worlds reject, renamed same-owner
  originals start, and explicit French originals start with loose/virtual
  graphics. Final targeted original-media tests passed (2/2, 3.96 s), including
  existing Towns lifecycle/feedback cases. No synthetic media or game data is
  committed; native Windows symlink staging remains skipped. Automatic language
  routing and full platform/visual parity remain unproven.

- 2026-10-16: Refreshed the read-only ScummVM DM reference audit at
  a3cf3746e44070528badfbdbb292edd9dd4e6340; gfx.cpp is byte-identical to the
  previous pinned audit. Recorded a potential unpopulated DOS D3R2 source buffer
  and a DOS creature-palette approximation as comparison limitations. Native
  original wall mapping remains intact; no speculative engine change or
  third-party implementation was introduced.

Final multilingual-owner verification: the rebuilt application passed the
complete original-media CLI startup diagnostics suite (215.37 s), including
both offline remembered-root cases. Access-denied CTest passed (13.79 s);
deterministic cancellation and catalog consistency passed. The cancellation
executable's two full five-game corpus checks remained intentionally skipped.
Independent source review found no remaining blocker in the PC path changes.
Redacted patch Gitleaks found no leaks.

- 2026-10-16: Reproduced a mixed DM1 collection reporting the rejected Towns
  dungeon after selecting a surviving loose ST12 installation. Final native
  publication now refreshes the complete same-owner dungeon path and observed
  supported hash for loose native owners, clearing stale receipts on failure.
  Reused the existing admission helper and retained its compatibility policy.
  Original PC/ST12/A20 English sibling cases pass both scan APIs through
  EN Towns -> JP Towns -> surviving sibling selection. A20 staging reads original
  nested ZIP/ZIP/ADF members in bounded memory and verifies sizes/hashes before
  writing task-owned copies; its SWSH remains required. Targeted original-media
  tests passed (2/2, 4.79 s), without missing-media skips. Independent review
  found no blocker, including fake-catalog and platform boundaries. This proves
  collection admission/report provenance, not native Windows or visual parity.

Final native surviving-owner binary passed the complete original-media CLI
startup diagnostics suite (214.55 s), including both offline remembered-root
cases. Access-denied CTest passed (14.33 s); deterministic cancellation and
catalog consistency passed. The cancellation executable's two full five-game
corpus cases remained intentionally skipped. Redacted patch Gitleaks found no
leaks. Preceding GitHub Actions run 38816079690 completed successfully before
publication of this batch.

- 2026-10-16: Reproduced ST12 graphics with unchanged PC34 dungeon bytes being
  admitted by the launcher despite incompatible native loading. Added one shared
  identity boundary for the three authenticated little-endian PC/Towns original
  dungeons. Native loose-owner validation and final report revalidation reject
  these pairings; M11 checks its retained source buffer before decompression or
  endian conversion, including stale scan receipts. Other native compatibility
  rules and explicit PC custom paths remain intact. Reused the existing original
  media test loop for ST12 and A20: both scan APIs reject each foreign original,
  runtime rejects it, and restored original worlds load again. Focused tests
  passed (2/2, 7.48 s), without missing-media skips, including the existing PC
  custom loose/virtual cases. Independent source review found no blocker. This
  is a known-original compatibility check, not a general custom-format validator
  or complete edition/platform/visual parity proof.

Final native-compatibility binary passed the complete original-media CLI startup
diagnostics suite (214.40 s), including both offline remembered-root cases.
Access-denied CTest passed (14.11 s); deterministic cancellation and catalog
consistency passed. The cancellation executable's two full five-game corpus
cases remained intentionally skipped. Runtime diagnostics explicitly confirmed
all six ST12/A20 foreign-original rejections in the focused test log.

- 2026-10-16: Reproduced a selected ST12 ZIP advertising availability after its
  original dungeon was replaced with unchanged PC34 bytes, even though M11
  rejected the consumed buffer. Also reproduced the invalid native edition
  remaining matched beside a valid PC ZIP. Reused one exact default-dungeon
  validator for final native/PC reports and each virtual edition's admission;
  native package readers apply the same original compatibility rule. Own,
  foreign and restored ST12 ZIP phases now pass both scans and runtime; mixed
  collections retain the valid PC edition and its exact graphics/world members.
  Only ZIP framing is generated; all stored game bytes are unchanged originals.

- 2026-10-16: Reproduced late native recovery substituting a neighboring Amiga
  preservation set for an explicitly selected incompatible ZIP through the
  preservation-name parent-scan branch. Scope DM1 archive rows before role
  discovery and limit late native candidate enumeration to the selected physical
  archive. Directories and nonexistent/offline selections retain their existing
  root policy. The regression deliberately varies the container name while
  retaining authentic ST12 bytes: hashes still determine edition. Explicit bad
  selections reject beside complete PC/Amiga archives; restoring ST12 recovers.
  Focused original-media tests passed (2/2, 7.94 s), without skips. Independent
  source review found no remaining blocker after the late-reader boundary fix.

- 2026-10-16: The final native archive binary also loaded original Amiga demo,
  A20 and A36 worlds through CLI while reporting each selected archive's own
  dungeon. A20 also passed a relative archive path. The demo repack's original
  graphics, DemoDun.dat and SWSH were independently authenticated before use;
  installed A20/A36 preservation archives were read directly. SHA-256 before and
  after every probe confirms unchanged source archives. These are headless
  startup/ownership checks, not original-display or physical HiDPI proof.

Final virtual-owner binary passed the complete original-media CLI startup
diagnostics suite (214.90 s), including both offline remembered-root cases.
Access-denied CTest passed (14.50 s); deterministic cancellation and catalog
consistency passed. The cancellation executable's two full five-game corpus
cases remained intentionally skipped. Redacted patch Gitleaks found no leaks.
Preceding GitHub Actions run 38818707400 completed successfully before publication.

- 2026-10-16: Fixed incomplete virtual DM1 copies hiding complete duplicate
  originals during directory discovery. Reused existing readers with a bounded,
  thread-local exact-path collection mode, preserving normal best-match APIs.
  Candidate output is sorted and distinct; capacity and path overflow fail
  without partial publication. Complete initial owners avoid extra enumeration.
  Unified loose/virtual completeness preserves Amiga SWSH, A22G tuple, A36
  driver and PC multilingual startup-media requirements. Final native publication
  reapplies completeness so late admission cannot restore missing companions.
  Original ST12 ZIP regression covers both scans, same-owner file receipts,
  runtime level loading, explicit bad-source rejection, collector overflow and
  duplicate members in one archive. Original A20 missing-SWSH admission is
  rejected. Focused final CTests: 2/2, 8.53 s, no missing-media skips. Full
  A20/A36/demo CLI probes pass with levelLoaded and unchanged source SHA-256.
  These are source/startup checks, not display or full platform parity proof.

Final duplicate-owner verification: rebuilt original-media CLI startup diagnostics
passed (215.49 s), including both unavailable remembered-root cases. Rebuilt
access-denied test passed (14.30 s); rebuilt deterministic cancellation passed
with its two full five-game corpus cases intentionally skipped. Localization
catalog checks passed. Independent source review resolved directory path-overflow
handling and found no remaining blocker. Redacted patch Gitleaks found no leaks.

- 2026-10-16: Reproduced and fixed late canonical Amiga admission overwriting
  a complete duplicate with an incomplete original demo package. The authentic
  regression failed at "late canonical admission cannot replace complete demo
  with missing-SWSH duplicate" before the fix. The complete-owner gate now runs
  before any native version/required-file publication, retaining the original
  A36 driver and other startup gates. Both scan APIs preserve the complete demo,
  actual runtime loads its original dungeon, and explicit incomplete selection
  remains unavailable. Demo/A20 subcases run independently of Atari availability.
  Full A20/A36/demo CLI probes still load worlds; source SHA-256 values remain
  unchanged. Independent source review found no blocker. Visual parity remains
  outside this startup verification.

Final late-admission CTests: 2/2 passed in 8.41 s with the authentic demo path
supplied, no missing-media skips. The wider 215.49 s CLI suite belongs to the
preceding duplicate-collector commit; this native-admission-only follow-up was
checked with both focused tests and all three original Amiga CLI probes.
Redacted patch Gitleaks found no leaks.

- 2026-10-16: Removed the outer-filename dependency for nested original Amiga
  A20 discovery. The original archive alias failed CLI startup and the focused
  test failed at "renamed nested original A20 is discovered by file and directory
  scans" before the fix. Added ADF dispatch to the existing nested ZIP disk
  visitor; no new parser or archive-name table. Both scan APIs × file/directory
  selection now pass with exact alias/hash ownership and actual world loading.
  Final focused tests: 2/2, 10.15 s, no missing-media skips. Source review found
  no blocker and confirms exact outer/inner/disk/member provenance. This proves
  supported OFS-ADF startup, not every Amiga disk representation or visual parity.

Final renamed-A20 verification: extended collector provenance checks authenticate
both original-graphics disk member paths in the preservation set. Focused tests
passed 2/2 (9.57 s), no media skips. Original renamed-archive CLI startup passed
with expected graphics/levelLoaded and unchanged source SHA-256. The rebuilt
original-media CLI suite passed (218.48 s), including both offline remembered-root
cases. Rebuilt access-denied test passed (14.16 s) and cancellation passed with
two full five-game corpus cases intentionally skipped. No full platform or
visual parity claim follows from these headless checks.

Additional current-build original-media CLI checks passed for Atari ST English
1.2, German 1.2 and French 1.3. Each required its catalog graphics MD5 and a
loaded world; each complete source archive retained its SHA-256. These are
headless launch checks, not rendering or native Windows staging evidence.

- 2026-10-16: Reproduced false missing-GRAPHICS.DAT feedback for explicit
  original ST12/A20 files after removing only their staged dungeon. A report-only
  final step now retains the exact original graphics path/hash while leaving
  version matches and availability unchanged. Focused tests cover both scans,
  normal menu refusal with DUNGEON.DAT feedback, direct runtime rejection and
  subsequent restoration of authentic bytes. Final focused CTests passed 2/2
  (9.99 s), with supplied original demo and no missing-media skips. Source review
  found no blocker. This fixes missing-world feedback, not every companion
  diagnostic or physical native-dialog behavior.

Final missing-world report verification: rebuilt original-media CLI suite passed
(217.17 s), including both offline remembered-root cases. Rebuilt access-denied
CTest passed (14.71 s); deterministic cancellation and catalog consistency
passed. Two full five-game corpus cancellation cases remained intentionally
skipped. Redacted patch Gitleaks found no leaks. This report change adds no
per-frame I/O and does not weaken native startup admission.

- 2026-10-16: Reproduced native wrong-world receipts accepting a game-wide
  PC/Towns hash despite correct native launch rejection. Reused the former
  same-owner lookup/hash/compatibility logic as default_dungeon_identity, shared
  by complete-owner admission and unavailable explicit-file reporting. Invalid
  worlds are no longer marked verified. Separately reproduced missing A20 SWSH
  having no startup role. A shared missing_startup_companion check retains the
  original table and A22G exact alternate tuple, A36 driver gate and PC ML
  parent-tree policy; it supplies precise diagnostic filenames without changing
  availability. Added authentic A36 graphics/world/VDEO staging to the existing
  real-media matrix; no synthetic payload or parser was introduced. Final focused
  CTests passed 2/2 (14.04 s), no missing-media skips, covering both scans,
  normal menu messages, incompatible runtime rejection, restoration and mixed
  surviving-owner provenance. Original A20/A36/demo CLI probes load worlds and
  preserve source SHA-256. Source review found no blocker; A22G runtime and
  virtual diagnostic expansion remain separate gaps.

Final companion-report verification: rebuilt original-media CLI startup suite
passed (217.37 s), including both unavailable remembered-root cases. Rebuilt
access-denied CTest passed (14.20 s); deterministic cancellation and PO/catalog
consistency passed. The two full five-game corpus cancellation cases remained
intentionally skipped. Positive A20/A36/demo CLI sources retained SHA-256.
Redacted patch Gitleaks found no leaks; no per-frame I/O was added.

- 2026-10-16: Reproduced explicit original A20 ZIP reporting no precise SWSH
  failure despite verified graphics/world. Added one bounded original-catalog
  lookup confined to the explicitly selected physical archive, with observed
  MD5 verification, then reused existing world/startup diagnostic policy.
  Towns archive language routing remains separate. A further full-Scan red case
  showed the generic ZIP fallback discarding DM1 failure evidence after its
  game-specific scan; a bounded four-row snapshot now retains only that report
  if no game can launch, leaving versions, availability and runtime roots alone.
  Both source changes passed independent review. Final focused CTests: 2/2,
  13.97 s, no missing-media skips with authentic demo supplied. Original A20 ZIP
  missing-SWSH receipts and normal menu input identify only SWSH; ST12 incompatible
  world ZIP retains its own graphics and rejects its world receipt. Positive
  native, duplicate, stale-source and mixed-owner tests remain covered. These
  are headless intake checks, not all archive/edition or display parity proof.

Final explicit-archive report verification: rebuilt original-media CLI startup
suite passed (217.00 s), including both offline remembered-root cases. Rebuilt
access-denied test passed (14.64 s); deterministic cancellation and PO/catalog
consistency passed. The two full five-game corpus cancellation cases remained
intentionally skipped. Redacted patch Gitleaks found no leaks. The earlier
216.16 s intermediate CLI run preceded the final diagnostic handoff fix and is
not used as its final verification.

- 2026-10-16: Reproduced direct selection of an unchanged original Amiga 3.6
  ADF starting a neighboring canonical Amiga 2.0 ZIP. DM1 now keeps the
  selected disk as the exact hash-search and admission owner; full inventory
  uses source-owned per-game file discovery without changing CSB/DM2's shared
  disk classification. Original-data regression covers same-edition and
  different-edition neighbors, both inventory APIs, graphics/world receipts,
  actual A36 runtime loading, unsupported original VDEO bytes named as an ADF,
  and collection directories with an ADF suffix. No original media is committed.
  Direct CLI confirms the selected A36 graphics and loaded original level;
  actual launcher input also hands off the selected A36 disk. The same-edition
  neighbor is an independent byte-for-byte disk copy, not a symlink alias.
  An authentic CSB 3.3 raw ADF still reaches its title, Original/Modern runtime
  movement, runtime input matrix and launcher handoff. Its separate adjacent
  AUTO/FTL-archive comparison is skipped when that archive is absent.

  Final focused original-media tests passed (2/2, 12.33 s), with no media
  skips. Access-denied verification passed (14.23 s); cancellation and catalog
  checks passed, with the cancellation test's two full five-game corpus cases
  intentionally skipped. The DM1/CSB/DM2 original-media CLI startup diagnostics
  suite passed (218.19 s). Independent source review found no blocker. These
  startup checks do not establish full platform or visual parity.

- 2026-10-16: Reproduced French PC34 multilingual menu selection silently
  loading the English standard world. Runtime now selects the original
  DUNGEONF.DAT/DUNGEONG.DAT by language within its authenticated graphics
  owner, for loose files and virtual ZIP members. The original French body
  is 33,687 bytes (82c7802122e9cfb2dc117bc9d197f457); German is 33,705 bytes
  (b9487db563ff9e89605c77ee44d17d14). Both end at raw map data without a
  checksum trailer, which ReDMCSB LOADSAVE.C F0434:2087-2088 permits.
  A separate bounded original-body reader consumes those exact retained,
  hash-verified bytes; generic/save checksum validation remains mandatory.
  The actual CLI and launcher start FR/DE worlds from the selected original
  ZIP, and DM1 READY now includes the chosen dungeon path. Original-media
  tests also cover wrong originals under expected language filenames, missing
  language worlds beside complete English/ZIP siblings, truncated buffers,
  unchanged strict save validation, renamed language files, and Swedish's
  English fallback. Explicit custom-dungeon behavior remains covered.

  Final original-media regression passed (3/3, 13.37 s), including the Towns
  Japanese checksumless-reader boundary, with no missing-media skips. The
  full scoped DM1/CSB/DM2 CLI startup diagnostics suite passed (217.10 s),
  as did all four FR/DE direct/menu CLI checks and catalog validation.
  Independent source review found no blocker. These checks establish the
  tested startup/data-routing paths, not complete visual/platform parity.

- 2026-10-16: Reproduced the ordinary PC ML menu requesting launch even when
  the selected French/German world was absent or held a different original.
  Input now blocks that request with the existing localized missing-data
  dialog and the precise original filename. The final launch intent separately
  rechecks the resolved owner; custom sources and other games retain their
  paths. Shared pure filename/hash helpers keep M11 and M12 in agreement.
  AUTO resolves its actual admitted edition, including stale saved English
  version rows. No language-file hashing was added to rendered launch gates.
  Original-data tests cover missing/incorrect FR/DE files through both PC and
  AUTO menu input, direct intent rejection and recovery using renamed originals.
  Final focused checks passed (3/3, 26.92 s), including access denial and BACK
  returning to the game options with language preserved. Catalog refresh changes
  source references only in 21 startup-menu catalogs.

  The scoped DM1/CSB/DM2 original-media CLI startup suite also passed
  (217.35 s), as did four actual FR/DE direct/menu CLI launches, deterministic
  cancellation and catalog validation. The cancellation test's two full
  five-game corpus cases remained intentionally skipped. Independent source
  review found no blocker; no original media or synthetic replacement artwork
  is committed. This is startup/feedback evidence, not full platform parity.

- 2026-10-16: Reproduced --lang fr on an original Amiga 3.6 ADF loading the
  English world. Shared filename selection now follows A36 FILENAME.C F0745;
  full original French/German compressed identities are checked before native
  decompression. Their declared body lengths are retained, matching the same
  originals in earlier French/German editions. Original-data tests cover both
  languages on direct ADFs, ZIP members and loose files, plus menu handoff,
  direct-runtime rejection of wrong/missing language files, restoration and
  precise missing-language feedback with BACK preserving the selection.
  Final expanded focused checks passed (3/3, 28.53 s), including access denial.
  This proves these world-loading paths, not full language-graphics parity.

  Eight actual A36 CLI launches passed (FR/DE, direct/menu, ZIP/ADF), as did
  the scoped DM1/CSB/DM2 startup suite (217.54 s). Additional authentic A20
  French/German CLI/title/entrance/menu regressions passed (2/2, 21.73 s).
  Their German test now expects the canonical DUNGEON.DAT report spelling and
  separately asserts the runtime dungeon path, retaining the exact archive
  prefix. The initial failure was that spelling expectation, not failed
  runtime loading; all later German steps were rerun successfully afterward.
  Catalog validation and cancellation passed; the cancellation test's two
  full five-game corpus cases remained intentionally skipped. Refreshed
  startup-menu catalogs change source references only. Independent review
  found no blocker. No original media is committed.

- 2026-10-16: Added direct original STX selection regression coverage for
  Atari ST 1.0a/1.0b/1.1. Both CLI and M12 loading retain the explicitly chosen
  uppercase-extension disk's graphics/world paths beside an independent
  identical copy and its wrapped archive. CLI also checks the edition's
  authenticated graphics hash and loaded runtime world. Final serial CTest
  passed all three editions, including the existing wrapped-media interactive
  Hall/C127 confirmation/movement route; no original-media skips occurred.
  The separate five-edition original-media Atari startup run passed 5/5
  (336.53 s), including German 1.2 and French 1.3. Direct raw-STX interactive
  entrance/Hall completion is not established by the loading-only checks.
  Independent read-only review found no blocker. No runtime change was needed.

- 2026-10-16: Reproduced the remaining PCML language-only discovery failure
  with unchanged original files and isolated CLI configurations: FR passes
  with the standard world, fails after removing only that world, and passes
  after restoration. The ordinary FR launcher also fails without the standard
  world. ReDMCSB FILENAME.C:44-52,84-104 explicitly selects DUNGEONF.DAT.
  This is diagnostic evidence for the open scan-policy fix, not a fixed bug
  or a claim of language-only support.

- 2026-10-16: Fixed the reproduced PCML/A36 language-only admission gap.
  Explicit scan options and immutable result snapshots carry the chosen
  EN/FR/DE world policy through same-owner authentication, file reports and
  runtime preparation. CLI language overrides apply before initial scanning.
  Menu language changes rescan asynchronously; pending or stale-language
  snapshots cannot launch, and outdated worker results are discarded. AUTO
  resolves consistently for scanning, final file checks and runtime intents.
  ReDMCSB FILENAME.C:44-52,84-104 defines the language-specific world names.
  Original-media focused CTest passed 3/3 (41.79 s, no missing-media skips),
  covering PCML/A36 FR/DE without English, missing/wrong worlds, exact owner
  receipts, ordinary menu input, asynchronous language switching and stale
  worker rejection. Actual CLI boot and menu launches start the original PCML
  FR/DE worlds without English; the menu checks require runtime receipts and
  exact world paths, not exit status alone. English still rejects when absent.
  The scoped DM1/CSB/DM2 startup diagnostics suite passed (222.02 s).
  Cancellation passed; two full five-game corpus checks intentionally skipped.
  Catalog validation passed; the 21 catalog changes contain source references
  only. Independent scanner/menu review found no remaining blocker after the
  AUTO export correction. Complete language/platform support remains open.

- 2026-10-16: Fixed recovery after cancelling a DM1 language scan. The
  original-media regression failed against the previous menu implementation
  at its incorrect return view (5.15 s), then passed with explicit retry.
  Cancellation retains the previous inventory and requested language; BACK
  returns to options without restarting work. Start explicitly retries the
  scan and a later Start loads the exact original French world. Resume/Load
  retries retain their original views, and catalogue-language policy does not
  affect custom DM1 entries. Independent review found no remaining blocker.
  Focused original-media CTest passed 3/3 (43.79 s, no media skips), including
  cancellation/retry, stale worker rejection, PCML/A36 worlds and access denial.
  Generic cancellation passed; two full five-game corpus checks remain skipped.
  Amiga 2.1/2.2 English and 2.2 German CLI/menu startup tests passed 2/2.
  Updated the English test's obsolete Dungeon.dat diagnostic expectation to
  the canonical DUNGEON.DAT receipt and added the exact runtime world path;
  no engine change was needed for those editions. Startup smoke investigation
  established that accepted/aborted intro transactions can set launchedEver
  without loading a world; scan-time interruptions already fail correctly.
  The suspected cleanup-guard defect was not established and no guard was
  changed. Catalog messages/translations are unchanged; source references and
  their entry ordering were refreshed.

  Final scoped DM1/CSB/DM2 original-media startup diagnostics passed again
  (220.66 s), including both disconnected remembered-root recovery cases.
  Catalog consistency and redacted patch/staged Gitleaks checks passed.

- 2026-10-16: Connected the existing authenticated Amiga format-5 save reader
  to native DMGAME/DMGAMEF/DMGAMEG DAT/BAK discovery, loose Continue validation
  and physical M11 resume. Original A20 save bytes resumed through direct CLI
  and actual Load menu for DAT and explicit BAK (four cases), preserving hashes,
  map 0, position (4,15), direction 2 and four champions. The expanded real-media
  preference test covers language-scan return views, selected physical paths,
  remembered Continue and rejection of unrelated original bytes. Automatic
  native backup selection/promotion and other save formats remain deferred.
- 2026-10-16: Reproduced and fixed missing root SWOOSH admission for authentic
  PC multilingual EUDATA installations, following FILENAME.C:45-51 and IBMIO.C
  G8102:2411-2413. Both scan APIs accept only the named layout and authenticated
  unique sibling; wrong bytes, missing media and arbitrary child names reject.
  Original French DMSAVE.DAT/BAK resumed through CLI and Quick Resume at map 5,
  position (4,18), direction 2, four champions and original tick 195221.
  Final focused tests passed 4/4 (61.90 s), without missing-media skips.
  Deterministic cancellation passed; its two optional five-game corpus checks
  were intentionally skipped. Catalog update/check passed; catalog differences
  only refresh source references. These results establish launcher integration,
  not visual or all-platform parity.
  The final rebuilt binary also passed the complete original-media DM1/CSB/DM2
  CLI startup diagnostics suite (219.61 s), including both offline remembered
  root cases. Redacted patch Gitleaks found no leaks. Independent source review
  found no blocker; the existing strict loaders and source ownership guards
  remain authoritative.

- 2026-10-16: Diagnosed Actions 38834758390's identical launcher-options
  failure on Linux, macOS and Windows. The metadata-only handoff seed selected
  German while retaining an English inventory snapshot. Rebuilt locally and
  reproduced the failure, then corrected only the test: assert stale English
  rejection before admitting a completed German snapshot and checking the
  existing runtime options. Handoff and font/artpack settings tests passed
  2/2 (0.70 s). Independent review found no blocker; production admission is
  unchanged. Original-media startup results above remain separate evidence.
  The separate cancellation CTest also passed (45.09 s) with the locally
  configured corpus. All three launcher-settings tests from the failing CI
  step now pass locally. Redacted patch Gitleaks found no leaks.

- 2026-10-16: Extended execution evidence beyond direct-STX loading for original
  Atari ST 1.0a, 1.0b and 1.1. Ran verify_dm1_atari_first_hall_handoff.sh against
  each explicitly selected uppercase-extension STX beside an independent
  byte-identical disk. All three M12 sessions reached active DM1 runtime with a
  loaded world, executed startup handoff, first Hall frame and F0267 placement
  at map 0 (1,3), direction 2, zero champions and zero sensor effects. Compared
  all six staged disks byte for byte with unchanged original preservation
  archive members. Earlier direct-path provenance checks remain separate;
  this run proves initial Hall completion, not champion confirmation, continued
  raw-disk movement or visual parity. Task logs and media remain ignored under
  .codex-scratch/dm1-direct-stx-hall-20261016.

- 2026-10-16: Verified direct original STX 1.0a, 1.0b and 1.1 interactive M12
  startup, C127 ordinal-14 confirmation and subsequent movement. Separate
  original-media sessions retained each explicitly selected disk for GRAPHICS
  and DUNGEON beside an identical neighboring disk; all reached map 0 (10,4),
  direction 1, one champion and a closed candidate panel in active runtime.
  Original disk hashes remained unchanged. Extended the existing regression
  with a two-owner loop, separate HOME/config/probe/log paths, full original
  graphics-hash assertions, exact raw-disk provenance and unchanged-copy checks.
  Unset the early-exit harness flag so inherited settings cannot replace the
  interactive run. Only the three affected test timeouts increased to 150 s.
  Independent source review found no blocker; no game runtime behavior changed.
  Other champions, routes, platforms and visual parity remain unproven.
  Final rebuilt-binary regressions passed for all three editions without media
  skips: 1.1 (111.76 s), 1.0b (107.27 s) and 1.0a (107.03 s).
  Each regression executes both archive and direct-disk interactive routes.
  Bash syntax, diff whitespace and redacted patch Gitleaks checks passed.

- 2026-10-16: Reproduced a full-inventory DM1 rejection for a selected original
  Z-selected.StX beside an identical earlier-sorted disk. The uppercase STX
  control and both selected-game CLI cases worked; full Scan alone dropped
  the mixed-case owner. The real-media regression reproduced phase 1/full Scan
  unavailable with no retained owner before the fix. DM1's explicit Atari
  admission now uses the existing case-insensitive extension predicate while
  retaining complete-file authentication and exact selected-file ownership.
  Rebuilt original-media test passed (10.33 s), with the original Amiga demo
  supplied and no missing-media skips. Both scan APIs and ordinary M12 full
  initialization retain exact original ST12 graphics/dungeon paths and hashes.
  An actual ordinary menu session without --game also reached active original
  ST11 Hall runtime at map 0 (1,3), direction 2, no champions, using the selected
  mixed-case disk for both required files. Copies contain unchanged original
  disk bytes; no synthetic game payloads or runtime disk extraction were added.
  Independent review found no blocker. Localization consistency and diff
  whitespace checks passed; redacted patch Gitleaks found no leaks.

- 2026-10-16: Strengthened the original Amiga 2.2 German startup regression.
  Its previous menu branch stopped at an accepted launch and checked source
  preflight logs. Actual ZIP and direct UAE-1ADF menu sessions now require an
  active DM1 runtime, authenticated original graphics, a loaded world, completed
  startup handoff and first Hall frame at map 0 (1,3), direction 2, no champions.
  Both routes retain the exact selected owner's graphics and dungeon paths.
  Each route has isolated HOME/config/probe paths. The original 910058-byte
  disk's SHA-256 is checked before staging and again for direct and wrapped
  copies after execution; runtime continues to read media in bounded memory.
  The existing direct CLI boot assertions remain in the same test. Independent
  review found no blocker. This proves startup integration, not champion choice,
  movement, native display/audio timing or visual parity for A22G.
  Final original-media CTest passed (32.78 s), without a missing-media skip.
  Bash syntax, diff whitespace and redacted patch Gitleaks checks passed.

- 2026-10-16: Added native read-only ProDOS intake for the authentic Apple IIGS
  2.0/2.1 logical-block images. Seedling/sapling/tree files, subdirectories,
  source-defined sparse holes, bitmap/block ownership and directory links are
  validated within explicit image/EOF/entry/path/depth limits before callbacks.
  One transient delivery buffer is allocated after preflight. An independent
  review exposed inactive directory-slot handling; an original-derived metadata
  probe reproduced the failure before the storage-nibble correction.
  Public single/batch hash discovery, distinct-path collection and memory reads
  now cover raw PO, ZIP->PO, ZIP->ZIP->PO and existing native 7z transport.
  ZIP magic retains precedence over a misleading PO suffix. The older physical
  materialization route rejects ProDOS; no runtime extractor/cache was added.
  Full original GRAPHICS.GAME, DUNGEON.GAME, SYSTEM/START and both programs are
  authenticated. Both programs match ReDMCSB Reference/Original/G20E/G21E.
  The six shared graphics/world paths in the source archive are preserved as
  distinct byte-hash results; modified historical disk labels are not new game
  admission. The two original executable/START owners remain independently
  identified. Malformed original-derived metadata, truncated raw/ZIP inputs,
  collection overflow and visitor stop are covered, with source hashes retained.
  Final public-intake CTest passed (1.06 s), without a media skip or enabled
  external runtime tool. Independent parser/integration reviews found no blocker.
  This is file intake only: catalogue admission, native graphics/palettes/world
  adaptation, audio and playable IIGS startup remain open in TODO-dm1.md.
  Final rebuilt application passed the complete original-media DM1/CSB/DM2
  startup diagnostics suite (217.15 s) and all three selected existing archive
  regressions (148.09 s combined), without missing-media skips. Localization
  consistency and diff whitespace checks passed. Redacted staged Gitleaks found
  no leaks. Native Windows/Linux original-media execution and cancellation or
  allocator-failure injection remain separate verification requirements.

- 2026-10-16: Implemented a separate Apple IIGS GRAPHICS.GAME record reader
  and indexed-pixel decoder from ReDMCSB EXPAND.C F0466:324-845. It reuses
  only the bounded 575-record little-endian table/raw access, following
  MEMORY.C F0479:1330/1361-1381. Native short/extended fills, literal pairs
  and overlapping previous-row copies retain their GS semantics; A/E commands
  consume their counts without output. Raster record 21 remains usable while
  unused font 12 and sound/text/global records are excluded from bitmap APIs.
  Output stays palette indices; no platform admission or fallback was added.
  Independent source review found no correctness/bounds blocker. A strict
  C11 warning build and direct calls against both unchanged original banks
  matched an independently derived assembly-oracle aggregate over all 532
  rasters (1,692,424 framed bytes, MD5 cf735a173ec8294f393a07c540595d6a).
  Original streams do not cover A/E commands. Palette rendering, native
  startup and runtime platform execution remain open.
  CMake-built original-media CTest passed both IIGS graphics (0.28 s) and
  existing ProDOS intake (0.89 s), with no missing-media skip. All 575 raw
  records preserve their bytes; every native raster also rejects insufficient
  output capacity and a truncated original command stream with cleared output
  dimensions and intact boundary canaries. Both source disks and their archive
  retain their original identities. Native Windows/Linux media execution is
  unverified; the module is not connected to playable runtime admission.

- 2026-10-16: Added bounded native Apple IIGS dungeon intake for original
  GAME/BONUS files. The LE 0x8104 wrapper is validated before writing caller
  output, with 1 MiB input/output limits and overlapping buffers rejected.
  Reused the unchanged MSB-first FTL decompressor and existing full LE world
  parser rather than duplicating section/Thing decoding. The native checksum
  trailer is retained and required by this original-media intake; generic
  save/dungeon checksum policies are unchanged. ReDMCSB LOADSAVE.C F0434:
  1853-1923/1937-2088, DECOMPDU.C F0455:258-392, READWRIT.C F0421:265-293,
  and DEFS.H MAP:1046-1116 establish wrapper, stream, sections and byte order.
  Both original G20E/G21E disks pass GAME/BONUS body hashes derived by an
  independent bit-at-a-time source oracle, checksum validation, full shared
  parsing of every map/Thing count, and source-owned starting coordinates.
  Main expands to 33444 bytes, 14 maps, starting (1,3), south; bonus expands
  to 6562 bytes, 3 maps, starting (4,1), south. Original-derived truncated
  streams, checksum-only bit changes, trailer omission, foreign byte order,
  oversized declared lengths and insufficient capacity reject with cleared
  outputs and intact boundary canaries. Independent source review found no
  blocker. Final CMake original-media tests passed dungeon (0.26 s), graphics
  (0.10 s) and ProDOS intake (0.88 s), with no media skip. An initial test
  incorrectly expected the world parser to retain a save-specific tail copy;
  source inspection corrected that assertion to the actual loaded extent. The
  native decode output itself remains complete and byte-hash verified.
  Runtime admission, native startup, audio and Windows/Linux original-media
  execution remain unverified. No playable IIGS support is advertised.
  The rebuilt Firestaff application linked successfully; the new reader remains
  outside runtime platform admission.

- 2026-10-16: Connected the verified native Apple IIGS bitmap decoder to an
  explicit M11 asset profile. Buffer/file initialization requires the exact
  shared G20/G21 graphics bank size and full MD5, then retains a private copy
  capped at 1 MiB. Native query/load dispatch is separate from the generic LE
  transport and PC/Amiga/Towns decoders. Record indexes are checked before
  narrowing; record 21 uses GS raster semantics and font/sound records reject.
  Loads allocate only the queried native pixel count before installing the
  existing stable cache; shutdown releases source and all cached pixels.
  Native source and independent ownership/bounds reviews found no blocker.
  Both original PO editions pass three buffer/file/reinitialization cycles
  through actual M11 query/load/cache, all 532 rasters matching the independent
  assembly oracle. Caller buffers are overwritten/freed after initialization;
  native virtual-member file reads, cache pointer stability, double shutdown,
  wide indexes and structurally valid altered-bank rejection are covered.
  Six original-media CTests passed (2.43 s): the M11 profile plus native
  dungeon/graphics/ProDOS tests, the complete PC34 713-record asset audit,
  and the FM Towns English/Japanese plus Amiga 2.0 raster corpus. No media
  skip was observed. The rebuilt application linked successfully. This
  provides the source profile only; native executable metadata/font/audio,
  edition ownership, catalogue selection and playable IIGS startup remain open.
  The complete DM1/CSB/DM2 original-media startup suite also passed with
  both original PC ZIP layouts: nested-member CTest (221.20 s) and root-DATA
  direct-script execution. The harness now binds the exact archive and full
  graphics hash while allowing the authentic member directory prefix, and
  explicitly isolates the default-HOME configuration from an inherited
  FIRESTAFF_CONFIG_PATH/USERPROFILE. Initial failures exposed those test
  assumptions, not an incorrect runtime world/source. Independent review
  confirmed the owner/hash assertions remain intact. Localization consistency,
  shell syntax and diff whitespace checks passed.

- 2026-10-16: Added original-executable-owned IIGS startup metadata and native
  interface text primitives. Exact G20E/G21E program identities gate bounded
  LE/byte reads into fixed copied metadata; no original input pointers,
  runtime extraction or peer-platform tables are retained. All 24 serialized
  fields (2493 bytes) match independently source-located originals, including
  RGB4 startup/six-light palettes, 17 UI boxes, 70 mandatory indices, object
  aspects/coordinates, native 0..15 palette remaps and 147 selected frames.
  The hypothesized Amiga M558 base was disproved by source/binary comparison;
  the reader uses actual GS program offsets. Source: DATA.C:119-244 and
  DUNVIEW.C:780-930/1042-1302/1534-1535/1593-1623.
  Native TEXT.C F0040:1094-1207 rendering uses authenticated bank record 557
  through caller-owned source authentication, six rows, five foreground bits
  plus background spacing, baseline-4 and advance 6. Explicit binding and a
  native font flag preserve generic font policies; clipping uses wide coordinate
  math and unsupported strings/colors reject before painting. LF remains glyph
  10 in this primitive. All 128 original glyphs, copied-font lifetime, odd-X
  placement, extreme/clipped coordinates and canaries pass the independent
  TEXT.C raster oracle. Generic PC/CEDT math probes use the same GS bytes
  only to check helper isolation, not to claim those platforms' media parity.
  Independent metadata/font source/ownership reviews found no blocker. Six
  original-media CTests passed (2.16 s), including all previous IIGS intake
  layers and an authentic PC34 C027 text-source gate. No media skip occurred;
  original program/bank/disk identities remain unchanged. The rebuilt app
  links successfully. Creature/projectile metadata, native audio, complete
  source-owner admission and actual GS CLI/M12 startup remain open.
  Existing DM1/CSB/DM2 original-media CLI startup diagnostics also passed
  (215.00 s). Localization consistency, shell syntax, whitespace checks and
  redacted staged Gitleaks passed. Windows/Linux original-media execution and
  complete IIGS runtime startup remain unverified.

- 2026-10-16: Added native IIGS sound-data primitives and complete installation
  ownership discovery. Both G20/G21 original programs supply 34 event specs
  and the native stereo distance table; sound records use BE sample counts,
  signed-to-DOC conversion, zero remapping and a terminal stop byte. Native
  master quantization and pending-event rules follow SOUND.C. Installation
  selection collects all graphics candidates before requiring authenticated
  program, START, graphics and main world from the exact same PO owner.
  Seven original-media CTests passed without skips (1.70 s), including both
  new tests and prior ProDOS/graphics/world/font/cache checks. The sound oracle
  covers all 34 events and 22 original zero-remap samples; installation tests
  reject missing companions and BONUS substituted for the main world beside
  complete original installations. Independent source reviews found no blocker.
  This does not provide a mixer, establish audible timing, or admit IIGS launch.

- 2026-10-16: Bound the remaining native GS creature/projectile/explosion
  metadata from exact source-matched spans in both original G20/G21 programs.
  Explicit types preserve 27 creature aspects, 14 projectile aspects, 13 RGB4
  replacement sets, native remaps and signed coordinate/shift words. Explosion
  tables have fifteen X/Y views, not ten XYZ rows despite equal byte counts.
  Native map-order palette replacement restores defaults then applies nonzero
  ordinals minus one, with the last request winning independently per color.
  The separate 1332-byte original-program oracle passes alongside the existing
  2493-byte startup oracle; all 27 authentic creature requests, reversed map
  order, negative/over-255 coordinates and invalid requests are covered.
  Seven original-media tests pass without skips. Main and bonus dungeon tests
  now also invoke the actual WORLD constructor and check native initial party
  state. This does not prove active input, native creature gameplay profiles,
  CLI/M12 startup or full rendering; those remain open in TODO-dm1.md.

- 2026-10-16: Read the native GS G0243 CREATURE_INFO table (27x26 bytes)
  from authenticated G20/G21 programs and bind copied rows/derived behavior
  profiles to each WORLD before initial active-group activation. WORLD and
  M11 consumers now select their own world profile; default PC/CSB accessors
  retain their prior values. Reset/free discard the profile, clone copies it,
  and GS-only canonical hash fields avoid changing existing default hashes.
  Original-media tests cover all native creature fields, interleaved default
  worlds, source-buffer lifetime and clone profile lifetime for both editions'
  main/bonus worlds. A separate 702-byte source oracle passes alongside the
  earlier metadata oracles. Seven original-media tests and two existing PC
  creature-info regression tests passed (1.47 s total); rebuilt CLI probes
  reach DM1 PC34 and FM Towns first worlds, while CSB FM Towns remains in its
  expected active title phase after two probe frames. An initial CSB probe
  wrongly expected gameplay after two title frames; the corrected startup
  assertion passes and does not claim Prison runtime coverage.
  Source comparison proves all 27 remaining defense, size and poison helper
  inputs equal the original GS values, so those helpers need no duplicate API.
  This does not yet connect IIGS to CLI/M12 or prove complete gameplay.

- 2026-10-16: Connected authentic IIGS 2.0/2.1 installations to actual M11
  first-world startup through CLI and M12. Both scanners require the same
  authenticated four-file ProDOS owner; explicit virtual PO/graphics locators
  retain that owner and cannot borrow another executable edition. Runtime
  preparation revalidates the selected edition without extraction/cache.
  Native startup binds the complete LE world, copied gameplay profile, fresh
  pools, graphics, font and program metadata. Native keyboard turns/steps and
  idle use WORLD; native base-wall/HUD presentation avoids PC metadata.
  Boot diagnostics report dm1-iigs-runtime with no invented title frames.
  The separate source gate requires native world/assets/font/draw ownership;
  existing PC startup validators remain intact. Eleven targeted tests pass
  without skips (13.49 s): nine original-media tests plus two existing PC
  creature-info tests. Both original editions pass actual CLI rotation and
  menu launch; M11 additionally verifies HoC forward/back steps, restart and
  double shutdown. Both public scan APIs verify exact file provenance and
  reject a missing-program disk beside complete originals. FM Towns keeps
  AUTO priority with its canonical original archive beside IIGS; a rebuilt
  PC34 CLI also reaches the first world. Catalog checks pass; 21 catalogs
  change source-reference lines only. This is basic native first-world
  functionality, not full IIGS support: intro, DOC audio, champion/UI actions,
  mouse input and remaining scene consumers are still unfinished.

- 2026-10-16: Connected native IIGS C127 mirror selection, cancel and
  resurrection to original party-square TEXTSTRING data, C026 portraits,
  C040 panel and native input zones. Hidden skill experience lives in the
  twenty-skill lifecycle owner; only four base skills occupy ChampionState.
  Native hexadecimal statistics, Luck minimum, food/water RNG and vitals
  are retained. Equipment follows original Thing order and native slot
  selection; cancellation preserves the wall chain and acceptance unlinks
  possessions and disables the source sensor. Original-media tests follow
  22 normal corridor steps to ELIJA in both editions, through raw PO and
  native 7z, and verify identity, attributes, skills and all four original
  possessions across selection/cancel/accept. This does not prove all heroes,
  equipment modifier side effects, reincarnation or full IIGS gameplay.
  Actual native modal and post-accept Draw dispatch, source pointer selection
  and pointer cancellation are also exercised. Eleven targeted tests pass
  without skips (12.77 s), including nine original-media tests and two PC
  creature-info regressions. The rebuilt PC34 CLI first-world/turn probe,
  localization catalog check and hash harmonization check also pass.
- 2026-10-16: Fixed discovery of the unchanged original FM Towns ZIP after
  renaming. A bounded physical MD5 search and explicit large-original-file
  admission locate it; existing inner media hashes still authenticate launch.
  The real mixed `towns.zip`/IIGS CLI test retains FM Towns AUTO priority.

- 2026-10-16: Applied native IIGS F0299 statistic/maximum-mana modifiers
  and F0301 equipment activation during candidate selection, in original
  Thing order. Lifecycle and visible champion fields retain modifier values;
  torch/scroll/junk activation synchronizes both raw and decoded objects.
  Original cancellation does not undo object activation. Real corridor walks
  recruit ZED, CHANI and BORIS after ELIJA, verifying the original torch,
  Moonstone and Rabbit's Foot across cancel/reselect/accept and later ticks.
  Both original editions and raw PO/native 7z transports retain coverage.
  This proves those original equipment cases, not every modifier or hero.
  Twelve targeted tests pass without skips (13.59 s), including nine
  original-media tests and three existing PC regressions. The rebuilt PC34
  CLI first-world/turn probe and localization catalog check also pass.
- 2026-10-16: Fixed the shared LE scroll icon getter to read Closed from
  property bits 10..15 instead of text-reference bits. A real IIGS scroll
  (Thing 0x1c00, text reference 33) reproduces the old wrong icon after the
  original opening operation; the corrected getter returns open icon 30
  while preserving its text reference. No replacement game payload is used.

- 2026-10-16: Connected native IIGS movement and idle to the live source
  queue instead of the observation-only generic command path. The original
  pressure plate at (6,9) now schedules its real door event, opens (5,9),
  and permits ordinary corridor movement to the level-one stairs. Native
  map changes use the existing transactional F0194/F0195 handoff without an
  extra tick; all fifteen original destination groups retain source IDs,
  positions, types and GS profiles on arrival. New C37 events run on a later
  tick rather than moving those groups immediately during arrival.
- 2026-10-16: Restored native periodic torch decay and selected the actual
  GS dungeon palette from an admitted F0337 light result. Both original
  editions, raw PO/native 7z and restarts verify first descent, torch fuel,
  a genuinely dimmer palette row, all sixteen published dungeon colors and
  all sixteen independent interface colors through the existing RGB6 getter.
  Shared F0337 admission preserves corrupt/duplicate torch rejection while
  ordinary bounded hand objects contribute zero; original unlit charged
  torch data verifies the source's charge-only calculation. Native audio
  requests explicitly remain silent until the DOC path is implemented.
  This is functional progression/light coverage, not full campaign support
  or original-screen visual parity.
  Fifteen targeted tests pass without skips (16.30 s): nine original-media
  tests and six existing PC regressions, including movement, F0195 and light
  admission. The rebuilt PC34 CLI first-world/turn probe and localization
  catalog check also pass. Build output contains no C compiler warnings;
  existing local SDL deployment/linker warnings remain environmental.

- 2026-10-16: Connected ordinary native IIGS inventory opening/switching and
  all thirty body/backpack slots to the original G0030/G0449 coordinates,
  G0038 allowed-slot masks and C042..C048 icon banks. Both executable editions
  verify complete original table spans and independently pinned digests.
  Source F0300/F0301 removal/addition preserves typed/raw activation and
  lifecycle modifiers while inventory selection remains separate from leader.
  Real ZED torch, CHANI Moonstone and BORIS Rabbit's Foot checks cover pickup,
  rejected head placement, re-equipping and occupied cross-champion exchanges;
  exact Thing ownership, carried loads, fuel and current mana are preserved.
  Held pointer movement uses the native icon; the generic PC cursor cannot
  overdraw it. Native Draw is exercised after swaps. Seventeen targeted tests
  pass without skips (16.85 s), including nine original-media tests and eight
  existing PC regressions. This does not complete inventory/gameplay: chest,
  eye/mouth, consumption, names, reincarnation and derived maximum-load refresh
  remain open, along with the previously documented native audio/scene gaps.

- 2026-10-16: Replaced the duplicated initial champion capacity formula
  with the existing complete F0309 calculation and refreshed native IIGS
  capacity after equipment/periodic effects and before movement. Authentic
  LINFLAS recruitment in both original editions reproduces Elven Boots
  capacity 490, removal 460 and re-equipment 490 through ordinary inventory
  clicks, preserving unique Thing ownership, carried weight and original
  strength/stamina. Eight later idle ticks preserve the equipped result.
  Three original-media tests pass without skips (17.55 s); four rebuilt
  PC source/inventory/movement regression tests and the rebuilt PC34 CLI
  first-world/turn probe pass. Original runtime stamina-depletion/wound
  capacity scenarios and complete native interaction remain open.

- 2026-10-16: Connected native IIGS food consumption through its original
  G0449 mouth command, G0242 nutrition table and C048 mouth icons. The bounded
  existing LE transaction retires the actual food record and updates the
  selected inventory champion, independently of the leader's carried weight.
  Both original editions recruit ELIJA and SYRA through actual corridors;
  ELIJA's BlueBox is rejected and SYRA's original Apple is consumed once.
  Food/lifecycle values, free raw/typed record, ownership and leader-load
  changes are checked. Four actual native Draw frames alternate 206/205 and
  require matching presentation acknowledgements and eight source edges each;
  input and idle cannot advance the world during those waits. Edition 2.1
  also exercises the elapsed-ms host adapter at the final edge of each frame.
  The host profile explicitly uses nominal NTSC 60 Hz. Original SetHeartBeat
  follows the machine video mode; PAL 50 Hz selection and exact hardware-rate
  capture remain open. Retained drinks, potions and the empty-hand food/water
  panel are not enabled by this food-only change. This is not complete native
  inventory or game support.
  Nineteen targeted tests pass without skips (166.55 s), including ten
  original-media tests and the rebuilt PC object corpus with deferred food
  completion. The rebuilt PC34 CLI first-world/turn probe, localization
  catalog check and staged Gitleaks scan also pass. Source review found no
  blockers; this does not establish original hardware timing or visual parity.
