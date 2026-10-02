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
  Mac Title.MooV-to-menu route now queues the authentic Midi(1000) cue after
  the movie's PCM drain. The original-media movie test requires a due MIDI
  event before Credits and New Game. This closes the missing source-schedule
  handoff but does not establish audible output here. The
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
  DM2's GDAT and DOS MVE streams now share that macOS playback preparation;
  MVE no longer repeats SDL audio subsystem initialization while it is live.
  Authentic PCM and lifecycle checks pass with the dummy device, but this host
  still reports no CoreAudio output and cannot establish audible M5 behavior.
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
  On 2026-10-02, the Mac held-key sampler gained the remaining source movement
  keys J/K/L/M/comma/period, and CLI scripts gained those key names. The
  authenticated Mac retail New Game route now accepts `key:k` and advances
  from (1,8) to (1,7), whereas the previous CLI parser silently dropped K.
  This proves one scripted keydown and the movement handoff. A physical held
  key on the reported M5 still needs observation to confirm repeat cadence.
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
- Complete native Macintosh CHARSHEET pointer item transactions. The current
  `M11_GameView_HandlePointerButton` branch resolves source object events
  32..57 to champion inventory slots 4..29; the other slot owners remain open.
  Retail `CODE(0)` maps `A5+0x222` to control insert
  (`CODE(3)+0x0b1a`), `A5+0x22a` to removal (`+0x0bd6`), and `A5+0x212` to
  Rect allocation (`+0x0ae0`); `CODE(3)+0x0170` hit-tests linked Rects.
  `CODE(10)+0x166e` selects CHARSHEET view 8; `CODE(8)+0x0674` toggles
  controls from the view tree, and `+0x0446` resolves its first matching
  object. `CODE(1)+0x006e` expands retail `DATA(0)`/`ZERO(0)` to `0x681e`
  A5 bytes. View-8 leaf 24 starts at six-byte object record 55: event 0x20,
  Rect 0x81ff, flags 0x0002. `CODE(8)+0x025c` masks that Rect ID with
  `0x3fff`; Mac RAW4 Rect 0x01ff is (68,138,16,16). `CODE(8)+0x1ad8`
  dispatches event 0x14..0x41 as event minus 0x14 to `A5+0xb42`, which
  maps to `CODE(10)+0x1e3c`. That function subtracts eight for champion
  slots, so event 0x20 selects slot 4, not slot 12. Mac
  `CODE(10)+0x1538..0x162e` binds the one-based CHARSHEET owner at
  `A5-0x21f6`; F1–F4 now bind the matching source runtime owner rather than
  using the unrelated attack-hero selector. The original-media test verifies
  empty slot 4 selection with both F1 ownership and ownerless inventory,
  plus an outside click. Both new-game
  champions have empty inventories. New Game starts on map 0 at (1,8),
  layer 7. A later direct File_header chain census found DB5–DB10 records
  on map 0, including a mirror/text-prefixed DB6/DB10 chain at (4,7);
  the earlier claim that maps 0–8 had no DB5–15 records was false.
  Original-media rendering now admits DB6 from the reachable corridor.
  A diagnostic pose at map 0 (3,7) facing east also proves an opaque
  pointer pickup of exact DB6 `0x18a9`, source-chain splice after the
  mirror/text prefix, placement, redraw, and repick. The original-media
  test now reaches this pose continuously from Title → New Game through
  ordinary M11 forward, turn, and forward inputs; no diagnostic position
  setter is used before the click.
  A separate DB10 pickup diagnostic uses handle 0x2831 on map 9 at (1,0),
  layer 5; map 16 also has items, including DB10 at (4,7). The current Mac C080
  New Game movement reaches map 0 (4,7) through ordinary M11 commands
  (`UP`, `TURN_RIGHT`, `UP` three times), but the next east move to (5,7)
  is blocked. Retail map 0 contains 33 floor, 35 wall and two pit tiles;
  all floor cells are connected to the first north-step cell, with no
  door, stair or teleporter tile. New Game (1,8) itself is raw wall class 0.
  This is the retail Mac pose, not a Firestaff endian or spawn fallback:
  `CODE(16)+0x21a8..0x21ce` reads BE File_header word 8 from `$8(a0)`,
  masks its low five bits into party x (`A5-0x6684`), the next five into
  party y (`A5-0x6682`), the next two into facing (`A5-0x6686`), and clears
  map (`A5-0x6680`). It yields map 0 (1,8), facing north, and ordinary M11
  movement leaves that entrance wall cell for floor (1,7). The subsequent
  `CODE(16)+0x21d2..0x25de` loads map columns, records and level tables;
  it does not by itself prove a later scripted transition or exit. Trace
  the live post-selection event and timer path before changing the spawn.
  A previous little-endian census mislabelled the DB3 record at wall (5,7)
  as subtype 0x05. Its Mac big-endian word is 0x057e: subtype 0x7e, a
  champion mirror. All map-0 DB3 chains inspected so far contain mirror
  (0x7e) or ornate animator (0x2c) records; no exit actuator is established.
  The production Mac wall-control fallback had the same endian mistake when
  reading live DB3 words; it now uses the record pool's source byte order.
  The original-media test renders the map-0 mirror from the normal New Game
  pose and verifies that a center wall-button action is not accepted. That
  negative click also passed with the old decoder, so it is not a
  red-before-green proof of the BE fix. A direct File_header source receipt
  additionally locks map-2 wall (7,1), DB3 0x8c72, to BE attributes
  0x1888/subtype 0x08; PC-order decoding would incorrectly call it a 0x18
  wall switch. A
  positive switch/keyhole pointer transaction remains open: their original
  records occur on later maps, and no normal New Game route to one is yet
  verified. A retail map-5 diagnostic pose at (1,3) facing north proved that
  the drawn wall target for DB3 `0x4fa3` was left in local 224x136 coordinates
  while RECT_7 presents the scene at y=40. The runtime now translates all
  source click targets through RECT_7, and the original-media hit test accepts
  the visible lower wall pixel while rejecting a pixel above the viewport.
  This alignment check does not establish a normal route to map 5 or a
  successful switch transaction.
  Same-layer map 12 touches map 0 only at map 0
  (6,0), a wall tile. The verified map16→map0 stair does not establish a
  reverse route. Both map-0 pits are enclosed and cannot be entered from
  its 33 connected floor cells; ordinary edge movement also meets source
  wall tiles. Trace the original post-selection event and timer path for an
  actual map change or tile mutation. Resolve the Mac New Game transition before
  assuming the map 0 boundary can be crossed.
  A 2026-10-02 read of retail Mac CODE resources narrows that trace:
  `CODE(8)+0x1aa8..0x1ad4` sends events 1–2 to turn at `+0x2afc` and
  events 3–6 to movement at `+0x2ca0`; events 7–11 are hero actions.
  `CODE(8)+0x1f2e..0x1f5a` sets or clears the New Game flag for events
  `0xd7..0xd9`, without changing the map. The map setter is
  `CODE(15)+0x20bc`, writing `A5-0x6684`/`-0x6682`/`-0x6680` (x/y/map),
  `CODE(7)+0x35dc` calls the tile callback at `A5+0x8ca`, then compares
  current and target map/position words; it does not directly call the
  setter. `CODE(7)+0x0f06` moves the party, calling the setter at
  `+0x10a4..+0x10c4` when the destination map differs. The movement path calls
  `CODE(15)+0x2806`. Its successful-step path calls `A5+0x72a`, mapped
  through the retail jump table to `CODE(15)+0x2482`. That routine uses
  same-level map descriptors and global coordinates to normalize a moved
  position into an overlapping map, rejecting candidate class 7 tiles.
  Retail map 0 occupies global `(21,36)` at size 7x10; map 12 shares its
  level and occupies `(27,27)` at size 26x10. Their only near boundary has
  map-0 wall `(6,0)` and map-12 wall `(0,8)`; the east candidate on map 12
  is class 7. This normalizer therefore does not establish a walkable Hall
  exit. Firestaff now applies the source's ascending map membership order
  to Mac movement. A retail-media M11 test steps from map 2 `(12,15)` south
  into map 3 `(12,5)` and returns north through the overlapping cell, using
  ordinary move input from diagnostic starting poses. It also enters a
  class-5 tile whose first source record has a clear byte-five bit zero.
  Class-5 tiles with records shorter than six bytes remain blocked because
  the original reads beyond the logical record; an authenticated ownership
  trace is needed before admitting those cases. Mac French has the same
  source code path but lacks an independent original-media test.
  `CODE(16)+0x21a8..+0x21ce` reads the startup pose from the sole retail
  `Dungeon.dat` File_header and sets map 0; its save/load branch at
  `+0x29e6..+0x29f8` can instead restore the map from a save buffer.
  `CODE(4)+0x02f4..+0x034a` consumes pending map `A5-0x66b2` through
  `CODE(4)+0x029a`, the map setter and the party mover, then clears it.
  A chunked read of the original Mac code corrected an earlier incomplete
  disassembly: `CODE(7)+0x1212` and `+0x1314` do write the mover's resolved
  destination map to `A5-0x66b2`; `+0x124c` clears it on same-map
  restoration. `CODE(4)+0x0314..+0x034a` consumes that pending map.
  `CODE(8)+0x246c..+0x298a` handles viewport clicks. Its mirror/wall
  branch reaches `A5+$a32` at `+0x27e6`; this is `CODE(7)+0x0006`, which
  only rotates linked record orientation bits and does not move the party.
  Another callback, `A5+$aca`, is `CODE(11)+0x0308`. Its class check at
  `+0x0396` skips directly to the next record for DB0–DB4, so a DB3
  subtype-`0x7e` mirror cannot reach that callback's action body. The
  retail HFS catalog has one
  `Dungeon.dat` and no bundled save, so a separate post-selection dungeon
  file is unsupported. No mirror-selection-to-party-map-change edge has
  been established through these callbacks. Find the mirror's actual event
  path and trace its party mover call before changing the spawn.
  The Mac C080 production
  pointer route now accepts source-admitted DB10 floor items through opaque
  pixels, including a linked record. Mac linked DB5–DB9 rendering now uses
  the live File_header chain and original category/type fields; positive
  retail-media render receipts cover each category. A normally reachable
  map-0 corridor pose renders and admits DB6 for a pointer pickup and
  placement round trip. A retail map-11 diagnostic pose now also verifies
  DB5 weapon `0xd407` pickup, source tile splice, cell-2 placement as
  `0x9407`, repick, and replacement through opaque viewport pixels.
  The same authentic archive now positively verifies opaque pointer pickup,
  source-chain removal, placement, repick, and replacement for DB7 `0x5c01`
  on map 7, DB8 `0xa037` on map 17, and DB9 `0x240e` on map 14. These use
  diagnostic source poses; normal gameplay access to the later maps remains
  open. Their placed handles are `0x1c01`, `0x2037`, and `0x240e` respectively.
  Mac `CODE(8)+0x1d7e` dispatches event 0x50 to `+0x246c`, which
  searches live 12-byte viewport targets at `A5-0x2f72` and branches on
  target kind 1–3 when the hand is empty. SKProject `c_gui_vp.cpp:3816`
  builds item zones from the drawn `dm2_image2.rect`, and
  `c_events.cpp:973` removes the chosen tile record before taking it into
  the hand. Retail Mac `CODE(8)+0x2838` handles an occupied hand through
  placement rect IDs `0x2f8`–`0x2fb`; `CODE(9)+0x02d8` resolves them from
  a dynamic tree at `A5-0x662`. The Mac RAW4 FC0D graph now expands their
  ordinary viewport-local boxes to `(24,115,88,21)`, `(112,115,88,21)`,
  `(112,89,72,26)`, and `(40,89,72,26)`. A retail-media diagnostic at
  map 9 (1,1) facing north verifies DB10 `0x2831` pickup, placement through
  Rect `0x2fa` onto the front tile as `0xa831`, redraw, and opaque repick.
  This proves the local item transaction; reach an item through normal New
  Game movement before calling the gameplay exchange complete. Other live
  Mac clipping states also need source receipts before using these boxes.
  An original-media diagnostic pose on map 9 at (1,1) facing north found DB10
  `0x2831` on the floor directly ahead at (1,0). Its original category
  `0x15`, type `0x2c`, field-0 image now reaches the M11 frame as one drawn
  item through source-gated placement. An opaque pointer click now moves that
  exact square-root record into the hand. The original Mac graph contains
  linked DB10 records, proven at map 10 (4,0) and map 15 (10,6). The indoor
  map-10 viewport now draws all five linked DB10 records with source draw
  slots. The outdoor map-15 viewport also draws both linked DB10 records.
  A pointer click at the map-10 diagnostic pose can take linked `0xe813`
  while preserving the remaining chain. Prove pointer pickup and placement
  for linked DB5–DB9, and the normal gameplay route to the later-map items.
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
