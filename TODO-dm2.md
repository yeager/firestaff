# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete native Resume for Macintosh, Amiga and FM Towns. The DOS `SKSave`
  importer is offered only when M12 selects the PC edition; a selected or
  AUTO-resolved non-DOS platform no longer receives a DOS save through Quick
  Resume or explicit `--save`. Mac header/prefix admission still does not
  implement SKProject `DM2_GAME_LOAD` record and possession reconstruction.
  Obtain authentic platform save corpora and traces, implement each native
  load owner, and verify saved party poses through normal M12 Resume. Do not
  synthesize saves to close these gaps.

- Verify the live macOS Retina dungeon viewport with a real runtime capture.
  The direct `--game dm2` AUTO route now binds authenticated Macintosh retail
  media when present, eliminating the DOS asset set previously selected on
  macOS. Authenticated Mac retail owns RECT_7=(0,40,224,136), but the runtime
  had excluded Macintosh from the RECT_7 pass and drawn its 224x136 scene at
  the 320x200 framebuffer origin, 40 pixels above the source aperture. Mac now
  uses the receipt-backed aperture. A real Mac capture then exposed a second
  bug: floor IMG9 pixels were translated through the final 16 image bytes as
  if they were a C4 local palette. Mac's source floor is IMG9; its ceiling is
  IMG3. Format-aware palette selection now keeps IMG9/U8 indices unchanged and
  retains the IMG3 local palette. A second capture exposed a geometry error:
  Mac RECT_700 rows carry top anchor 11 while RECT_701 rows carry bottom anchor
  14, but the big-endian path had forced both destinations to y=0. The Mac
  floor now uses y=136-height and ceiling y=0. The real-media M11 test checks
  those source anchors, palette domains, floor/ceiling receipts and movement.
  A fresh Mac CLI capture from the authenticated retail BIN/CUE now shows the
  lower floor band, which was missing from the earlier origin-aligned frame.
  The Mac M11 test also requires its authentic IMG9 floor and IMG3 ceiling;
  the native CLI screenshot gate checks the lower 39 pixels of RECT_7 so a
  blank floor band cannot pass on HUD occupancy alone. A paired Mac/DOS runtime
  capture at the same map, party position and graphics set showed the Mac wall
  at the wrong depth. The renderer had assigned one GRAPHICSSET field per cell
  through cell 22. SKULLWIN's DM2_DRAW_WALL instead uses fields cell+0x22 for
  cells 0..15 and reuses field 0x32 for cells 16..22; the scheduler visits cells
  19..2. Terrain projection and wall commands now use the source's exact
  table1d6ad0 coordinates and GDAT-field rule. The authentic Mac New Game test
  checks all four directions, requires visible GDAT walls in the east view,
  distinct direction frames and no fallback. The DOS wall-plan, item-placement,
  door-side and table tests pass. A source audit then found that
  SET_GRAPHICS_FLIP_FROM_POSITION uses the map descriptor's `w8 & 0x3f`, not
  the map number. That seed is now retained by PC and big-endian Mac dungeon
  loaders, included in floor/ceiling parity and used to rebuild the wall plan
  when party position or direction changes. `DM2_DRAW_WALL`'s `table1d6b2c`
  alternate fields and missing-asset fallback are now exercised against the
  authenticated Mac retail archive. The real-media Mac viewport tests pass
  each direction without material fallbacks.
  SDL captures do not
  establish visual parity or verify a physical Retina drawable. Verify HiDPI
  on a native display. The Mac HUD no longer routes the generic
  INTERFACE_GENERAL/4/1 (97x62) starburst into a full-height portrait panel.
  M11 binds authentic CHAMPIONS portraits and status bars to Mac's RAW4
  portrait destinations and retains all six Mac movement images in one
  consumed HUD plan. The real-media test requires top-row pixels and complete
  eight-command consumption for its two-member starting party. The Mac RAW4
  portrait rows decode to x=8/89 for a two-member party after the source
  640x400 to 320x200 scale. The Mac archive contains movement
  arrow images in INTERFACE_GENERAL/3 and RAW4 rows 40..45; SKProject's
  DRAW_ARROW_PANEL uses fields 2,4,6,8,10,12 with those rectangles. The full
  QUERY_BLIT_RECT traversal now follows that retail Mac graph; the real-media
  startup test locks all six source placements at x=229/260/291, y=129/153,
  each 29x23. SKProject stops walking when a linked rectangle is absent and
  still computes a clipped blit; Firestaff had rejected the whole query. M11
  now draws the six exact Mac images at those positions, using their image-local
  palettes, and draws only Mac-owned portrait/status and movement material.
  A real-media M11 test now clicks the top-center forward button and verifies
  that the normal DM2 movement pipeline advances the party north one tile.
  The buttons are outside the viewport's C080 hit list, so the Mac input route
  resolves their authentic RAW4 rectangles before routing to keyboard-equivalent
  movement actions.
  The native CLI capture verifies visible pixels in all six arrow rectangles,
  the RECT_7 dungeon viewport, and its lower floor band. The runtime capture
  still needs a same-state original comparison to diagnose the reported
  dungeon view. The Mac M11 real-media gate now also checks the retail start
  pose from File_header::w8 (map 0, x=1, y=8, north) and queries the authentic
  map two tiles ahead with c_map's tile-coordinate contract; that cell is a
  wall, and the centered wall appears in the captured runtime frame. The
  224x136 view at (0,40) plus the six RAW4 arrow rectangles at x=229..320
  accounts for the black right-side space in the 320x200 source page. This
  establishes source ownership for the close wall and layout, but does not
  resolve the user's report that the view is unusable or prove visual quality.
  Rebuilt the local app and repeated both native Mac startup routes after that
  viewport work: direct `--game dm2` AUTO and M12 → Mac → Title.MooV → New
  Game both reach the accepted real-asset frame. A separate deterministic
  boot-probe step reaches map 0 at (1,7), north, and produces a different
  presented view from the initial (1,8) pose. The frame still looks like a
  close wall; those movement and asset receipts do not establish usable wall
  selection or perspective. Keep this Mac viewport issue open.
  A local dummy-video capture of the scripted Mac menu route after turning
  east and moving twice ends at (3,7); the authenticated map has floor one
  square ahead, a wall two squares ahead, and no live DB4 creature within one
  square. The runtime scene's large centered wall therefore matches this
  route's source map and is not evidence of an adjacent creature. It still
  does not establish that the reported M5 screen or physical key input behaves
  the same way.
  The held-key sampler had a second, conflicting generic mapping for Mac:
  W/E (retail wall buttons) could be treated as forward/right-turn motion
  while held. It now resolves held scancodes through the same authenticated
  Mac key table as key-down events; the real-media CLI startup/movement test
  now drives one forward step with the source Mac `S` key and another with Up
  through the production SDL event loop, then checks the resulting party pose;
  the end-to-end test passes against authentic retail media. The Mac input unit
  test also locks arrows, A/S/D, lateral movement and the W/Q/E wall controls.
  This does not establish M5 keyboard feel, audible
  sound or viewport quality. The user reports that the latest release has no
  audible sound, an unusable dungeon view, no movement and a monster seemingly
  adjacent to the party on a MacBook Pro M5. On the local Mac M4 build host,
  `system_profiler` reports no audio devices. With the authentic Mac retail ZIP,
  the SDL dummy-device SFX test passes its decoded/mixed-sample checks, while
  the native-output check skips after SDL reports `No default audio device
  available`. Authentic Mac MIDI parsing and scheduling find 4,328 events.
  The Mac M11 New Game real-media test now also requires the selected MIDI cue
  to advance at the first gameplay tick; that source-timeline assertion passes
  even on the no-output host. M11 now calls SKProject's stop-music owner when a
  DM2 session shuts down, and the same real-media test verifies that no MIDI
  schedule remains afterward. The native backend remains unavailable here and
  delivers zero events. The MIDI test reports that case as skipped rather than
  green; a separate native SDL audio
  test also skips when the required output device is absent. Run both checks on
  a Mac with an available output, then verify audible output on the M5. The
  CoreMIDI fallback now rejects offline endpoints and only accepts destinations
  advertising General MIDI or sampler output, avoiding false success on a
  silent virtual port when Apple's DLS output is unavailable. This filter
  compiles and source/scheduling tests pass, but no physical output or external
  synth endpoint is available here to exercise it. These host results do not
  explain or resolve the M5 report.
  The M11 runtime JSON probe now records SDL logical-window and renderer
  drawable dimensions separately; the dummy-driver Mac test checks both are
  valid, but its equal 1920x1080 values do not exercise Retina scaling. An
  attempted native SDL run on this build host exited before game startup with
  `The video driver did not add any displays`, so paired display evidence still
  requires a Mac session with an accessible display.
  On 2026-10-01, the shared M11 SDL path was also fixed so a failed attempt to
  open its playback stream neither reinitializes an already-active audio
  subsystem nor shuts that process-wide subsystem down. A dummy-device
  lifecycle regression keeps another owner's live stream usable across the
  failure; authentic DM1, CSB Atari, and DM2 Mac startup checks pass with the
  change. This closes the shared-owner failure path, but it does not prove
  native output or resolve the M5 audio report.
  Downloaded and launched the published v3.0.352 arm64 app bundle with direct
  `--game dm2`; it reports the PC-DOS media hash
  `25247ede4dabb6a71e5dabdfbcd5907d`. The rebuilt macOS AUTO route selects the
  authentic Mac retail hash `5cab25f6b975957eae4a203174e7f2a6`. Its real-media
  CLI CTest passes the standard movie-to-runtime path and one scripted native
  SDL keydown north move from (1,8) to (1,7). The real-media M11 New Game test
  now checks a second forward move through the retail Mac action table and a
  one-minute stationary New Game interval (3,600 source ticks), sampling live
  DB4 chains every ten ticks and requiring stable party coordinates. After the
  real turn-and-move sequence to (3,7), it now samples live DB4 occupancy every
  ten ticks for another source minute without teleporting the party. It also
  checks the first forward move. These passed against the authentic Mac retail
  archive; no live DB4 creature approached within one tile during either
  sampled minute. This improves opening-route evidence but does not prove monster
  behavior during a longer campaign. These checks do not establish physical M5 key repeat
  or Retina presentation. A fresh logical 320x200 screenshot still shows a
  dominant close wall; visual parity and
  HiDPI remain unverified.
  The native Mac PCM test previously proved only SDL stream creation and queue
  admission. It now also requires an active output device when native audio is
  requested, confirms it is not paused, and waits for at least 99% of the
  authentic sample to drain, allowing SDL's short resampler tail. The dummy
  device playback test passes this consumption gate; the native-device test
  skips on this host because SDL reports no default output device. This still
  cannot establish audible output on the reported MacBook Pro M5.
  Rechecked the configured data root containing both similarly named Mac
  archives: the 18 MB `The First Chapter` demo is rejected by CLI, while
  ordinary `--game dm2` AUTO selects the 759 MB full retail BIN/CUE with
  `assetMd5=5cab25f6b975957eae4a203174e7f2a6`, Mac platform and map-0
  GRAPHICSSET 2. The dedicated demo-rejection CTest passes, so this naming
  collision does not explain the reported wrong gameplay assets.
  These checks do not establish physical Retina output.
  On 2026-09-30, downloaded and exercised the exact published v3.0.353
  macOS arm64 app bundle against the authenticated full Mac retail ZIP. Both
  direct `--game dm2` startup and the M12 → Mac → title movie → New Game route
  reached the active runtime; scripted north movement changed the pose from
  (1,8) to (1,7). The M11 test checks the spawn through a one-minute stationary
  interval with live DB4 sampling every ten ticks, and again after that first
  step. These checks run on the local Mac build host and capture logical
  320x200 output; they do not
  establish physical M5 keyboard input, HiDPI presentation, visual quality or
  audible sound. The source audit found no concrete Retina coordinate defect,
  but the actual reported M5 session still needs paired runtime evidence:
  pose and live creature coordinates, SDL logical/drawable dimensions, an
  observed key event, and native SFX/MIDI output.
  The authenticated Mac start corridor is a floor at (1,7) and a wall at
  (1,6); after one north step, another north step is correctly blocked. The
  M11 real-media test already proves that a right turn followed by two forward
  steps reaches (3,8) facing east. The normal SDL startup test now exercises
  that same three-key sequence and requires the final pose, so a single
  successful north key can no longer stand in for continued movement. This
  explains the close wall when continuing straight, but does not establish
  whether the reported M5 keyboard, visible monster, Retina presentation or
  native audio issue is the same behavior. The Codex Mac remains locked, so
  those hardware observations are still unavailable here.
  On 2026-10-01, reran the exact staged macOS arm64 3.0.353 application bundle
  (`Firestaff.app/Contents/MacOS/Firestaff`) with the authenticated Mac retail
  ZIP. Its ordinary CLI path turned east and moved twice to (3,8); the scaled
  M12 Mac platform → game → retail title → New Game path also reached and
  presented an active runtime after movement. The archive hash gate and
  `--game dm2` AUTO selection passed. The test used SDL's dummy audio device,
  so it validates neither audible playback nor the reported M5's native
  keyboard/audio devices. The M11 real-media test now samples live creature
  chains during one stationary game minute and found none adjacent to the
  retail spawn. The initial failed invocation targeted the bundled
  Dungeon Studio UI executable, not `Firestaff.app`, and is not game-engine
  evidence.
  The M12 part of this regression now sends a right turn and two forward
  commands after the mirror handoff and requires map-0 pose (3,7), facing
  east. It also uses `FIRESTAFF_DATA` for the shared data root; passing the ZIP
  itself as `--data-dir` had produced a false “Data missing” menu. The updated
  script passed against the staged bundle and authentic ZIP. This confirms
  software input and source runtime movement after both CLI and M12 starts,
  but does not verify physical M5 input or audio output.
  The corresponding CTest row now passes in 178 seconds against its configured
  authenticated ZIP. The M12 sessions scan a temporary two-package install
  containing the original Mac ZIP and, when staged, the original DOS ZIP;
  AUTO reaches Mac gameplay within 500 probe frames. Its CTest timeout is 240
  seconds because the full host install scan plus source-movie checks exceeded
  the previous 180-second limit.
  On 2026-10-01, an isolated normal-loop run against the same authenticated
  retail ZIP completed Title.MooV with `movieActive=0`, `movieComplete=1`, and
  `movieRejected=0`, then reached the first source-owned runtime frame and
  accepted a turn plus two forward moves at the exact 320x200 test geometry.
  The standalone 45-second run took 47.57 seconds including media admission;
  its final party pose was map 0 `(3,8)`, east, with two champions and no core
  fallback draws. This rules out a repeatable local decoder hang for that
  route, but does not establish physical M5 playback cadence, audio, HiDPI or
  explain a hardware-specific stall.
- Complete the source `DM2_DISPLAY_VIEWPORT` pass ordering inside the original
  224x136 backbuffer and `RECT_7` presentation route. The native indoor
  runtime now owns a separate backbuffer, copies the retail RAW4 `RECT_7`
  portion unscaled, then draws the source HUD on the 320x200 screen. The
  source-plane, creature, item, projectile and wall-ornament routes now clip
  to that 224x136 pass; canonical retail GDAT regression tests compare every
  plane byte there and guard both buffer boundaries. The private allocation
  remains full logical size as a defensive guard while remaining special
  passes are audited. Native real-media New Game routes are covered for
  PC-DOS, Amiga, FM Towns, and Mac. The PC-DOS start is map 0's indoor Skullkeep
  cave: SKProject `IS_MAP_INSIDE` reads the active map's GRAPHICSSET scene flag
  (map 0 uses graphics set 2 with flags `0x000b`), rather than treating map 0
  as outdoor. A previous real-media test incorrectly required the T600 outdoor
  route at this start and has been corrected. The PC-DOS launch and indoor
  runtime frame are admitted with real assets and zero fallback draws; this
  establishes runtime admission, not original-vs-Firestaff pixel parity.
  PC-9821 support is intentionally excluded. Keep PC-9801 and IBM PS/V
  media preservation-only.
  Remaining work is outdoor's distinct composition, transition stretching
  and same-tuple original-capture comparison; retain only GDAT-owned pixels.
- Pair the newly captured, labelled PC 1.0 EN original New Game route with
  Firestaff at the same game state. The retired H2313 crops remain
  non-promotable because they are byte-identical and lack route labels; they
  must not be relabelled. The new original route establishes menu → New Game
  → real runtime with matching `320x200` frames, `RECT_7` crops, input
  transcript and source-data hashes. Its final two runtime captures are
  byte-identical, so it proves one stable runtime pose only. Capture the same
  labelled pose from Firestaff, then add a same-state pair manifest and diff;
  do not promote either side as pixel parity before that comparison exists.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- For the Japanese FM Towns edition, pair one original-emulator session with
  Firestaff at the same startup checkpoints. The retained original trace
  proves pre-title → FTL → castle title → emulator-directed input → first
  dungeon, but its nominal menu checkpoint was still title animation and it
  has no valid audio receipt. Obtain a source-owned menu/loading/HUD trace,
  VRTC/CRTC palette/register receipts, and a same-state Firestaff comparison
  before correcting or claiming parity for the reported palette, missing-menu
  and post-New-Game dungeon defects. FM Towns U4 plane indices now retain their
  authenticated physical-palette binding; the local gameplay CTest passes its
  active palette, frame-admission and HUD-plan checks. Do not use Firestaff's
  own decoder output as the expected original framebuffer.
- Capture an original PC-DOS `SKSAVE1` WIELD input-to-CD/RAM trace with a
  valid encounter, weapon choice, command arguments and RNG timing, then
  bind the remaining WIELD fallback/luck and creature-drop route to that
  trace. Do not replace it with a generated weapon, creature, save or combat
  result; keep the creature-drop gate closed until the original interaction
  identifies a valid route.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
