# Firestaff DONE — DM2

- 2026-10-10: Built and ran `dm2_v1_four_platform_catalog_real_media` against
  the shared installed data root. The test passed and confirmed that M12
  recognizes DOS, Amiga, FM Towns and Mac at once, and resolves each edition
  to its own original archive instead of the scan's first match. PC-9821 is
  not included in the supported set.

- 2026-10-10: Re-ran `dm2_v1_amiga_native_cli_boot` against the installed
  authentic Amiga ZIP. It passed the M12 New Game route, required an accepted
  runtime frame with real assets and zero fallbacks, checked the presented
  320x200 screenshot, and exercised the native input matrix. This supersedes
  the stale note below that said the initial frame was rejected. Dummy-video
  evidence does not verify physical M5 rendering or visual parity.

- 2026-10-08: Rebuilt the current launcher and reran
  `dm2_v1_fmtowns_native_cli_boot`; it passed in 189.50 seconds against the
  authentic FM Towns ZIP. With DOS English/French, Macintosh and Amiga editions
  also installed, bare `--game dm2` selected FM Towns through AUTO; the test
  completed the 225-frame title, M12 New Game and first-champion route, then
  captured one accepted real-asset runtime frame with no core fallbacks. The
  presented source frame is 320x200 under SDL's dummy video driver, so this
  does not verify the reported M5 Retina viewport or input. The shared
  `m11_display_aspect_present_rect` HiDPI transform test also passed; it
  verifies geometry and mouse mapping helpers, not native SDL presentation on
  the M5.

- 2026-10-08: With the current launcher freshly linked,
  `dm2_v1_mac_native_cli_boot` passed in 138.40 seconds against the authentic
  Macintosh retail ZIP. With no `TMPDIR` set, the script keeps its temporary
  menu root, runtime probe and capture under an isolated `.codex-scratch`
  directory instead of `/tmp` or beside the executable. The test verifies the
  normal Macintosh title, New Game, mirror and movement routes under SDL's
  dummy video driver; it does not verify physical M5 Retina display or input
  behavior.

- 2026-10-07: All five installed authentic DM2 CLI-start CTests passed in the
  multi-edition startup matrix, covering DOS English/French, Macintosh, Amiga
  and FM Towns.
  A native Cocoa run of the v3.0.368 binary with bare `--game dm2` selected the
  original FM Towns ZIP from five installed editions and completed its 225-frame
  title into `dm2-startup-menu` in 26.2 seconds. This host reported a 1900x998
  window and equal-sized drawable, so it does not verify Retina behavior or the
  MacBook Pro M5 report.

- 2026-10-07: Rebuilt and re-ran `dm2_fmtowns_m11_gameplay_real_media` with
  the authenticated FM Towns ZIP; it passed in 2.99 seconds. The assertions
  cover the source squad fill, three-command HUD plan, authentic spell/status
  image fields, transparent index handling, first GAME_LOAD frame and zero
  core fallback draws. This verifies data-bound gameplay presentation, not
  exact original-frame parity or physical M5 HiDPI behavior.

- 2026-10-07: `dm2_v1_dos_native_cli_boot` passed in 75.87 seconds against
  the installed original DOS English ZIP, exercising the native CLI startup
  route. The test result covers startup behavior and does not establish
  physical M5 presentation or dungeon visual parity.

- 2026-10-07: Restored the source-owned FM Towns squad backdrop from
  `GRAPHICSSET/<map style>/0xF5` at expanded RAW4 rectangle 47 and bound each
  live hero's hand/status materials to the original `INTERFACE_GENERAL/4`
  images and rectangles. The native CLI regression now captures the normal
  M12 → New Game → first-champion runtime at 320x200 and requires visible
  dungeon and right-panel pixels, an accepted authentic-asset frame, and zero
  core fallbacks. `dm2_v1_fmtowns_native_cli_boot`, the real-media Towns M11
  gameplay/title checks, and all 12 selected startup, data-directory and
  viewport tests pass. This proves source-owned runtime content is present;
  exact original-frame parity and physical MacBook Pro M5 HiDPI/input behavior
  remain open in `TODO-dm2.md`.

- 2026-10-06: The direct `--debug` regression now checks the normal bare
  `--game dm2` FM Towns route against all 225 source title records: it traces
  every timed frame, rejects any title-media failure, and records completion
  at about 26 seconds. The independent M11 original-media test confirms TITLE
  frame 0 has zero Timer-A ticks. This uses SDL dummy output and does not prove
  physical Mac M5 HiDPI presentation.

- 2026-10-06: Re-ran the original FM Towns startup matrix against the
  authenticated Japanese ZIP. The bare direct `--game dm2` route reaches the
  225-frame New Game menu within 40 seconds, validates its presented source
  pixels, and reaches the original first party on map 0 at (8,0); the M12
  route and 1920x1080 launcher input path also pass. SDL dummy video/audio
  does not verify physical Mac M5 HiDPI viewport/HUD output or audible playback.

- 2026-10-04: A source 0x3c timer now relocates an authenticated FM Towns DB4
  between maps without changing the player's GAME_LOAD map, outdoor state or
  viewport light-map identity. The original-media regression moves the live
  CAII-bound creature from map 1 (2,6) to map 3 (8,4) while the party stays
  on map 0 (2,8). With the prior runtime map write restored temporarily, the
  same test fails after the record move because the party map becomes 3.
  This verifies the timer/record transaction, not creature path selection.

- 2026-10-03: Bare `--game dm2` with a persisted collection root now scans
  its DM2 leaf directly and still retains the collection root for reopening
  the game menu. An original-media test verifies the authenticated FM Towns
  edition, and a native macOS capture shows New, Resume and Quit after the
  title sequence. The local capture is not a physical MacBook Pro M5 test.

- 2026-10-03: The opt-in FM Towns native CLI test now captures the presented
  320x200 menu after bare `--game dm2` and compares every RGB channel with
  TITLE/0/4 and its palette from the authenticated retail ZIP. The full
  original-media script passes on the macOS Actions-built binary, including
  New Game, the first champion and normal-loop runtime. An isolated default
  config makes the source-pixel check independent of saved brightness. The
  published v3.0.358 binary also presents the menu with no platform or data
  directory CLI arguments under SDL dummy video. Physical M5 HiDPI window
  output and native mouse input remain unverified.

- 2026-10-03: The original-media FM Towns CLI regression now checks the
  unqualified `--menu --game dm2` route as well as direct `--game dm2`.
  The published macOS arm64 v3.0.355 binary selected `fmtowns-ja`, completed
  the source title, opened New Game, and reached the first champion in the
  normal runtime loop. The complete ZIP startup script passed. Physical M5
  HiDPI presentation remains unverified.

- 2026-10-02: Re-admitted the authentic Macintosh retail dungeon's declared
  12,603-byte map-data span. The map starts at byte 26,806; the previous
  descriptor-maximum calculation started it nine bytes late and shifted tile
  flags, column prefixes and linked records. All 724 comparable column
  prefixes now match the original data. Earlier diagnostic coordinates and
  close-wall claims based on the late map start are superseded below.

- 2026-10-02: The current DM2 AUTO policy prefers authenticated FM Towns
  media when installed, regardless of host. The older macOS Mac-preference
  section below records the previous policy and is superseded. Use
  `--platform mac` for Mac-specific regression work.

- 2026-10-02: Corrected original-media receipts now verify DB8 `0xa037`
  on map 17 (3,8) and DB9 `0x240e` on map 14 (6,13): both render,
  accept opaque pickup, leave the source tile, and survive placement,
  repick and replacement. These use diagnostic source poses; normal
  gameplay access remains open. DB7 `0x5c01` lies on a wall at map 7
  (20,8), with no DB7 record on an authenticated floor tile, so its old
  pickup receipt remains invalid.

- 2026-10-02: Corrected original-media tests verify DB5 weapon `0xd407`
  at map 11 (10,3), visible from (11,3) facing west. The diagnostic
  pointer path picks it up, removes it from its source tile, places it,
  repicks it and replaces it. The formerly described reachable DB6 map-0
  corridor was phantom floor; its earlier pickup receipt remains invalid.

- 2026-10-02: Corrected original-media tests verify the five-record linked
  DB10 chain at map 10 (4,9). From an authenticated floor pose at (4,8)
  facing south, an opaque click takes tail `0xe813`, preserves head
  `0xe80f` and splices predecessor `0xa812` to the end marker. Placement
  and a normal New Game route to this tile remain open.

- 2026-10-02: The Mac outdoor viewport has a source-admitted DB10
  ground-layer pass following SKProject's outdoor tile/static-object pass.
  Corrected original-media tests draw both linked records `0x2848` and
  `0x6849` at map 15 (10,6) from (10,7) facing north. Other outdoor
  item categories remain open.

- 2026-10-02: The indoor Mac viewport walks DB10 tile chains and carries
  each record's source draw slot into placement. Corrected original-media
  capture draws all five DB10 records on map 10 (4,9) from (3,9) east;
  the tail `0xe813` uses slot 1, ordinal 5. Linked DB5–DB9 rendering
  remains open.

- 2026-10-02: The original Mac retail File_header record graph now follows
  big-endian `w0` links. Real-media tests confirm DB10 chains on map 10
  (4,9) and map 15 (10,6). The source mirror gate also compares links in
  the dungeon's byte order.

- 2026-10-02: Corrected original-media tests verify DB10 `0x2831` at
  map 9 (1,6), one step ahead of diagnostic pose (1,7,N). Its drawn opaque
  pixels admit pickup; a transparent click leaves it untouched. Placement
  through the source rectangle, redraw and repick also pass. Normal New
  Game access to map 9 remains open.

- 2026-10-02: The Mac DB10 static-object render path resolves `0x2831`
  to category `0x15`, type `0x2c`, and its original 34x13 image. A
  corrected original-media capture at map 9 (1,7,N) verifies its scene
  placement.

- 2026-10-02: Viewport item and projectile virtual image addresses no longer
  overlap. The authentic Mac DB10 `0x2831` on map 9 selects category `0x15`,
  type `0x2c`, image field 0; a real-media red/green test now fetches its
  original 34x13 IMG9 image through the production viewport provider. This
  proves image address resolution, not DB10 scene placement or pickup.

- 2026-10-02: Macintosh wall-control list rotation now writes record links
  in the authenticated dungeon's byte order. The rotation callback previously
  wrote little-endian bytes even for Mac big-endian records. The existing
  source-order writer is reused; no reachable local-action switch was found
  in the supplied Mac retail actuator census, so a live rotation remains
  unverified.

- 2026-10-02: Viewport click rectangles now follow the same authenticated
  RECT_7 placement as the rendered 224x136 scene. A Mac retail map-5
  diagnostic pose proved that a wall target had remained 40 pixels above its
  visible location; the original-media hit test now resolves the displayed
  target and rejects a click above it. Live switch action and a normal route
  to that map remain unverified.

- 2026-10-02: Mac wall-control fallback now reads DB3 actuator words in the
  source byte order. Retail record `0x8c72` has BE word `0x1888` and subtype
  `0x08`; a PC-order read would misclassify it as switch subtype `0x18`.
  The original-media census and New Game mirror rejection pass. A live
  pointer transaction at that later-map record remains unverified.
- 2026-10-02: Macintosh CHARSHEET pointer dispatch now reads retail view-8
  object records and masked RAW4 rectangles for champion slots 4–29.
  CODE(8) event dispatch and CODE(10) slot conversion determine the mapping;
  F1–F4 selection now binds the source CHARSHEET champion owner. The retail
  test verifies empty slot 4 selection with and without an owner and a click
  outside the source rectangles. Pickup and placement of an original item by
  pointer remain unverified.
- 2026-10-02: Macintosh Title.MooV now hands off its authentic Midi(1000)
  menu cue after the final movie PCM drain. SKProject calls
  `DM2_PLAY_MUSIC(0, true)` before `SHOW_MENU_SCREEN`; the Mac retail resource
  parses to 4,328 scheduled MIDI events. The original-media M12/M11 movie
  regression completed all 210 title frames, required a due menu MIDI event,
  then passed Credits cancellation and New Game. SDL dummy audio verified
  source scheduling only; no native MIDI backend or audible output is
  available on this host.
- 2026-10-02: The Macintosh held-key sampler now includes the retail
  J/K/L/M/comma/period movement keys. SDL key repeats are discarded at the
  input boundary, so these keys must be sampled while held for another
  source-tick command. CLI scripts now accept `key:j`, `key:k`, `key:l`,
  `key:m`, `key:comma`, and `key:period`. The source-table test covers all six
  mappings. An authenticated Mac retail run of `--script key:k` completed
  Title.MooV, New Game and mirror selection, then moved the party from (1,8)
  to (1,7) with two champions and no core fallback draws. The first run before
  the CLI parser fix reached runtime but stayed at (1,8). Build and PO update/
  check passed. Scripted keydown verifies the CLI path; physical held-key
  repetition remains unverified on this host.
- 2026-10-01: DM2 GDAT effects and DOS MVE audio now use the same macOS
  playback preparation as the shared DM1/CSB SDL path. MVE startup no longer
  takes an additional SDL audio subsystem reference on every opening. The
  shared-owner lifecycle check and authentic DOS GRAPHICS.DAT/MVE PCM tests
  pass with an SDL dummy device. Physical CoreAudio output remains unverified.
- 2026-10-01: The authentic-media CLI startup matrix passed for all five
  registered DM2 editions: DOS English, DOS French, Macintosh retail, Amiga,
  and FM Towns. The four non-Mac tests passed in 442.74 seconds total; the Mac
  normal CLI/M12 route passed separately in 190.37 seconds. These cover each
  edition's own title/start path and scripted movement. They do not establish
  complete platform parity or the reported M5's physical input, Retina output,
  or audible playback.
- 2026-10-01: Re-ran the authenticated Macintosh retail regressions from the
  current checkout. `dm2_v1_mac_native_cli_boot` passed in 190.37 seconds and
  `test_dm2_v1_mac_m11_new_game_real_media` passed in 2.38 seconds. These
  verify source startup, simulated keyboard/menu input, movement, real-media
  viewport receipt and basic visible-pixel gates. SDL dummy presentation does
  not prove that the reported M5 shows a usable view, receives physical input,
  presents correctly on Retina, or plays audible audio.
- The M11 runtime diagnostic now records SDL logical-window and physical
  drawable dimensions separately. The Mac dummy-video regression requires
  valid, nonzero values for both, making presentation geometry available for
  a paired hardware report; dummy output is not Retina evidence.
- 2026-10-01: Re-ran the authenticated Macintosh retail startup checks on the
  macOS host. The direct M11 New Game route reached active runtime, moved, and
  reported no living creature adjacent to the initial party pose; the native
  CLI suite passed title-loop movement, scaled M12 menu movement, AUTO Mac
  selection, and post-launch movement. Tests used SDL dummy presentation, so
  physical Retina rendering and native-device audio remain unverified.

## 2026-10-01 — Reject DM2 Resume on the incomplete Mac load path

- The launcher no longer offers a DOS SKSave as Quick Resume when the
  selected DM2 edition is Macintosh. Explicit `--save` requests are rejected
  on that path with a clear CLI error instead of failing later as a generic
  launch error. Selecting DOS still retains authentic Resume.
- A real-media M12 regression checks the DOS-versus-Mac platform gate using
  the original DOS save corpus. A separate Mac+DOS media regression verifies
  the explicit CLI rejection. Full Macintosh `DM2_GAME_LOAD` reconstruction
  remains open in TODO-dm2.md; no synthetic save was added.

## 2026-09-29 — Prefer authentic Macintosh DM2 media on macOS AUTO launch

- A normal `--game dm2` launch on macOS now selects the authenticated
  Macintosh retail edition before DOS when both are installed; DOS remains
  the fallback when Mac retail is absent. Other hosts keep the existing
  PC-first policy, and explicit platform selection is unchanged.
- The focused AUTO policy test passes on macOS for Mac preference and PC
  fallback. A CLI boot probe using the installed mixed media root selects
  the retail Mac asset hash (`5cab25f6b975957eae4a203174e7f2a6`); before the
  change the same command selected DOS (`25247ede4dabb6a71e5dabdfbcd5907d`).
- The plain `--game dm2` probe then completes the Mac title/menu, New Game,
  mirror selection and first dungeon step. Its receipt confirms two
  champions, the Mac asset hash, `dm2RealAssets=1`, and zero fallback draws.
  SDL dummy output proves selection and the source startup route, not native
  Retina viewport placement or visual quality.

## 2026-09-29 — Verify the complete Mac title film on its source clock

- The authentic retail Mac `Title.MooV` runtime regression now keeps its
  original QuickTime clock through all 210 video samples and requires the
  startup menu to be ready at the natural movie/audio handoff. The expected
  duration is computed from the original per-sample timing table, so the test
  detects premature playback, a stalled frame sequence and a late menu.
- The focused test passes against the supplied retail ZIP. It checks source
  timing and menu state with SDL dummy output, not native-window presentation,
  audible playback or Retina interaction.

## 2026-09-29 — Remove PC-9821 support

- Removed the PC-9821 edition from runtime admission, launcher selection and
  CLI platform aliases at the user’s request. Its dedicated archive reader,
  audio/startup branches and positive support tests are removed.
- Historical source references do not grant runtime support. DOS, Amiga,
  Macintosh and FM Towns remain in scope.
- The explicit-alias/archive rejection, supported-catalog, music-route,
  fingerprint integrity and DOS HUD regressions pass. Original-media
  Macintosh, Amiga and FM Towns normal startup routes pass, and the shared launcher
  handoff checks pass with DM1 (332 assertions), CSB Amiga (55) and DM2 DOS
  (60). SDL dummy runs do not prove physical audio or Retina behavior.

## 2026-09-29 — Amiga startup regression rerun

- After rebuilding the application with the latest launcher input and music
  changes, the authentic Amiga ZIP passes keyboard Back/reselect and pointer
  launch routes. The normal loop completes SWSH/TITL, accepts New Game and
  presents an original-asset runtime frame with no core fallback draws.
  Separate CLI probes pass forward/backward, turns, strafes and action with
  the expected party poses, and the archive hash remains unchanged. These
  SDL dummy checks do not prove physical sound or original visual parity.

## 2026-09-29 — FM Towns startup regression rerun

- The authentic Japanese ZIP passes M12 keyboard and platform-card pointer
  routes, CLI New Game/mirror input, and the uninterrupted normal-loop route
  through all 225 TWANIM startup frames to the first champion. The runtime
  receipt requires map 0 at (1,8,0), one champion, completed title animation,
  an empty script queue and an accepted original-asset frame without core
  fallback draws. The archive hash remains unchanged. SDL dummy output does
  not establish audible playback or original-emulator visual parity.

## 2026-09-29 — Retain Amiga selection after presentation Back

- Verified the menu Back/reselect sequence with the authentic DM2 Amiga
  archive and the existing menu launch gate checks. Added the sequence
  to the original-media startup script. This verifies launcher navigation;
  it does not establish complete game parity.

## 2026-09-29 — Macintosh title movie advances in the normal loop

- The ordinary M11 idle loop now continues requesting presentation while the
  authentic Mac `Title.MooV` owns startup. Without those redraw requests the
  decoder presented its first frame once, then stopped before the source menu
  could accept New Game. A new authentic-media CTest exercises the normal
  `--game dm2 --platform mac` loop through movie completion, New Game and the
  selected mirror into loaded runtime. It now also starts at M12, uses pointer
  input at 1920x1080 to select the Mac platform, and continues through the
  movie and mirror into a presented runtime frame with an empty input queue
  and no core fallbacks. The authentic Mac CLI CTest passes in 160.94 seconds;
  the focused M11 movie-runtime test passes in 2.30 seconds with the supplied
  retail ZIP. SDL dummy output verifies source state and a visible 320x200
  frame, not native macOS video, Retina backing-scale input or audible playback.

## 2026-09-28 — Mac retail archive selection in native CLI test

- The supplied data root contains two files named as DM2 Macintosh English
  archives; the smaller “First Chapter” BIN does not contain admitted retail
  game data, while the second ZIP contains the complete authentic HFS retail
  image. The Mac CLI regression now probes candidate archives through the
  production boot scanner and selects only the one matching the retail
  `GRAPHICS.DAT` hash. The full title/movie, mirror selection, start-menu and
  gameplay input matrix passes with that authentic archive and also checks
  discovery from the shared data root.

## 2026-09-28 — DM2 AUTO start-menu route

- The ordinary DM2 start-menu AUTO route discovers the authentic DOS edition
  from the installed data root without a `--platform` override, follows the
  retail MVE → SKULL → New Game sequence, and reaches the first loaded runtime
  frame at map 0, position (1,8), facing south, with one champion. The local
  receipt and visible 320x200 presentation were verified from the supplied
  retail data. The DOS integration script now preserves this as an explicit
  regression; its full long-running suite still requires verification on the
  dedicated test host.

## 2026-09-27 — Macintosh retail discovery from the shared data root

- Fixed DM2 Macintosh retail admission for the documented
  `~/.firestaff/data/dm2/` layout. The explicit HFS archive check now searches
  the `dm2/` child of the shared data root and checks the common downloader
  duplicate name `Dungeon-Master-II-Skullkeep_Mac_EN (1).zip`; the DM2 boot
  scanner still verifies the original retail contents before admitting it.
- The authentic four-platform catalog test now scans the shared
  `~/.firestaff/data/` root and passes for DOS, Amiga, FM Towns and Macintosh.
  The normal no-`--data-dir` Mac CLI route also reached map-zero runtime and
  consumed its first UP input from the supplied original archive. This closes
  a source-discovery gap only; visual parity remains open.

## 2026-09-26 — Native CLI/start-menu matrix revalidation

- Re-ran all five authenticated native CLI routes against the installed
  original DOS English, DOS French, Macintosh, Amiga and FM Towns media. All
  five passed; the normal M12/startup checks remain distinct from the direct
  CLI probes. The all-games M12→M11 launch-preparation gate also passed
  (207 passed, 0 failed, 3 skipped). This revalidates the exercised launch
  paths and does not close the original-frame or full-gameplay gaps below.

## 2026-09-26 — DOS map-zero ownership and M12/M11 launch coverage

- Corrected the authenticated DOS New-Game regression: SKProject's
  `IS_MAP_INSIDE` derives scene ownership from the active GRAPHICSSET flags;
  the initial map-zero Skullkeep cave is indoor (graphics set 2, flags
  `0x000b`). The stale expectation that it must use T600 outdoor rendering
  was wrong. The corrected test passes against the supplied DOS ZIP and admits
  the map-zero runtime frame with real assets and zero core fallback draws.
- Re-ran the M12→M11 direct-launch boundary with an isolated data root that
  exposed only the authentic DM2 media. Its DM2 route passed, retaining the
  hash-matched data owner and the source startup-menu boundary; other games
  were unavailable and skipped. The overall boundary reported 86 passed,
  zero failed, and five skipped. Repeated it with an Amiga-only data root; M12
  selected the verified Amiga archive hash and the M11 handoff passed with the
  source startup menu active.
- The authentic Amiga M11 route passed SWSH/TITL, selected the original
  New-Game rectangle, started its source party and opened the native inventory
  panel. FM Towns and Macintosh M11 New-Game routes also passed through their
  source-owned mirror selections into map-zero runtime. The Mac test used the
  full Mac archive, not the similarly named Mega CD ZIP.

- 2026-09-25: Re-ran the authentic-media native launch matrix against the
  supplied DOS English, DOS French, Macintosh, Amiga and FM Towns archives.
  All five CTest routes passed. The DOS and Amiga start-menu tests reached a
  presented runtime frame; the Macintosh, French DOS and FM Towns tests
  passed their existing CLI and launcher route checks. This verifies the
  bounded start paths exercised by those tests, not the open original-frame
  parity or full gameplay items in `TODO-dm2.md`.

## 2026-09-25 — French DOS runtime regression timeout

- Increased the French DOS CTest allowance to cover both fresh sessions in the
  regression: the M12 source-menu runtime handoff and the direct CLI movement
  probe. With the original French ZIP supplied, the menu reaches the native
  runtime receipt and the CLI reaches `party=1,7,0`; the shell regression
  passes when allowed to finish. The previous 45-second CTest cap expired
  before that second result, despite no functional failure. The Amiga
  ZIP-to-ADF CLI/menu matrix also passes on authentic media; its observed
  102-second parallel run motivated a larger CTest margin.

## 2026-09-24 — Original DOS save Quick Resume

- Fixed the explicit `--menu --game dm2 --save <path>` route: the M12 row now
  selects Quick Resume rather than consuming Enter as a fresh-game selection
  and dropping the supplied save. The real DOS archive regression passes both
  direct `--boot-probe` resume and ordinary M12 Quick Resume for all eight
  `SKSAVE0–3` primary/backup files, checking each saved map and party pose.
  The archive remains unchanged; full save/write ownership remains open.

## 2026-09-24 — DOS start-menu runtime handoff

- The authentic DOS English ZIP now has a normal M12→M11 regression through
  the source MVE intro and New-Game event. It checks the first source-owned
  runtime receipt for the retail initial party and requires a visible 320×200
  presented frame. The wait keeps the source menu keys from being consumed by
  the movie owner. Direct `--boot-probe` coverage remains a separate path.
  The French DOS route is recorded below; remaining DM2 editions/platforms
  still need equivalent first-runtime evidence.

## 2026-09-24 — French DOS start-menu runtime

- The authentic French DOS ZIP now follows the ordinary M12 → M11 path through
  the retail MVE and New-Game inputs to the native runtime receipt at the
  original party pose `(map=0,x=1,y=8,direction=0,count=1)`. The focused test
  keeps direct boot-probe movement separate and confirms the French GDAT
  identity there. This extends the DOS startup evidence only; it does not
  claim French DOS presentation parity.

## 2026-09-24 — Amiga start-menu runtime handoff

- The authentic Amiga CLI regression now advances the original SWSH.DAT and
  TITL.DAT streams through their 1,345 source-owned 50 Hz ticks, then clicks
  the original GDAT New Game rectangle through the normal SDL input mapper.
  It checks the loaded retail party at `(map=0,x=1,y=8,direction=0,count=1)`
  and requires a visible 320×200 runtime capture. The direct boot-probe input
  matrix remains a separate check. The Amiga big-endian RAW4 decoding fix
  restores source wall-rectangle geometry for GDAT v5, allowing the initial
  indoor frame and authentic movement matrix to use real assets without
  fallback draws. Same-state comparison with the original Amiga renderer and
  broader gameplay/render parity remain open in `TODO-dm2.md`.

## 2026-10-02 — Original DB1 teleporter map-edge input

- Unified live party movement and future cross-map path traversal on one
  source-record DB1 transition decoder. It reads the enabled square's first
  original record, applies the existing scope and bounds gate, and exposes
  destination and rotation without inferring reverse edges. A focused test
  against the original FM Towns DUNGEON.DAT proves map 3 (13,11) → map 38
  (6,6) and map 38 (6,4) → map 3 (13,9). This is a pathfinding input, not a
  completed `DM2_FIND_WALK_PATH` mode 8 implementation.

## 2026-09-16 — FM Towns New Game real-media revalidation

- Rebuilt and ran the full M11 FM Towns real-media gameplay regression against
  the admitted HME-242 ZIP. It reaches the source SKULL menu after the title,
  dispatches New Game through the decoded source rectangle, confirms the
  prepared champion through the source mirror route, and commits the runtime
  session without a PC-English companion archive. The same run verifies
  authentic pit, stairs and DB1 transitions plus the real DB4/F9 creature
  timer path. FM Towns does not borrow the DOS Enter shortcut at this title;
  its source-owned New Game pointer route is required. All game data remains
  archive-backed and in memory.

## 2026-09-25 — FM Towns title-to-map CLI regression

- Added an M11 regression against the authentic HME-242 ZIP that advances the
  source Timer-A title sequence to SKULL, selects New Game from the decoded
  GDAT rectangle, then commits the preselected mirror into the map-0 runtime.
  It requires a real-asset frame with zero core fallback draws.
- Extended `dm2_v1_fmtowns_native_cli_boot` to cover the direct CLI route with
  the same source-coordinate New Game and mirror clicks at 320×200. The
  regression requires a loaded map-0 party and a receipt showing real assets
  with zero fallback draws. The registered CTest passed with the installed
  original media. This proves the CLI start path, not original-renderer pixel
  parity.

## 2026-09-08 — FM Towns IMG2/IMG6 palette binding

- Corrected the FM Towns HME-242 image-palette route. Its `0x8004` GDAT
  stores variable-length IMG2/IMG6 C4 streams, not PC IMG3 records with a
  trailing `QUERY_GDAT_IMAGE_LOCALPAL` table. The old generic decoder used
  the final compressed bytes as palette values and emitted invalid physical
  indices such as 179 and 181 into the 16-colour dungeon framebuffer. FM
  Towns image pixels now keep their source-owned direct `0..15` physical
  indices; the active GRAPHICSSET palette remains the RGB owner. The genuine
  zipped HME-242 New-Game regression verifies a fully admitted source frame
  contains no index above 15. No game media is extracted, generated, or
  substituted.
- Corrected the matching FM Towns CHARSHEET admission and presentation
  gates. They had required the PC 255-colour summary even though the selected
  original panel is an IMG2/IMG6 four-bit surface. The live source route now
  opens, draws, accepts authenticated pointer contexts, and closes the real
  inventory page after New Game.

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native real-media startup matrix for DOS English and French,
  Macintosh, Amiga and FM Towns. Every archived DOS `SKSAVE` primary and
  backup slot resumed through direct CLI and the start menu in RAM; all four
  platform owners selected their own retail source rather than a fallback.

Reviewed 2026-08-29. Completed work only.

- FM Towns and Amiga startup menu/credits palettes now preserve the exact
  eight-bit RGB components in their authenticated GDAT `dtPalIRGB` rows.
  The DOS-only `DM2_CONVERT_DRIVERPALETTE` six-bit VGA conversion is no
  longer applied to those platform routes. Real-media tests compare all 256
  presented entries against each selected ZIP's raw palette and prove the
  old conversion changed at least one source component.

- Macintosh retail `Cmd-O` now reaches a native **Open Game** owner rather
  than being dropped or treated as Back/quickload. The modal lists only
  complete original `SKSave` candidates, two per page, from the configured
  original-save source. Before display and again before loading it performs a
  read-only ordered corpus census; the selected candidate's source path,
  byte order, payload and state hashes must still match before the runtime
  imports it. The chosen member stays in RAM and enters the existing
  source-owned `GAME_LOAD` import path. A missing, incomplete or changed
  corpus fails closed; no default slot, generated save, extraction or fallback
  candidate is used.

- The DOS, Amiga, FM Towns, and Macintosh retail start-menu routes each now
  create a fresh native New-Game session for every observed input command:
  forward/backward movement, left/right turn, both strafes and action.  Every
  path retains its original archive in RAM and checks the resulting party
  pose, accepted real GDAT frame and zero core fallback draws.  FM Towns and
  Macintosh keep their source-specific title and mirror-confirmation order.

- The packed PC-DOS `SKSAVE1` M11 spell regression now uses the retail ZIP
  directly in RAM. It proves that an authentic rejected `YA FUL IR` Fireball
  is consumed with the original rune-tail rule and that a source-admitted
  spell can then commit through M11. Read-only hero mana/wizardry receipts
  select the real cast; no save bytes, stats, or runes are fabricated.

- The supplied DOS and Amiga retail ZIPs are now injected into the native
  CTest title, asset and New-Game receipts. The Amiga six-disk receipt walks
  ZIP→ADF→`dm2_arcsplit1`…`6`→LZX entirely in RAM; the Amiga M11/M12 title
  and New-Game paths and DOS M11 New-Game path cannot silently skip because
  their archive environment was unset.
- The supplied DOS retail ZIP now also drives the SKSAVE corpus regression:
  all four primary and four backup members retain virtual archive provenance,
  pass the original-state census, and are reread through receipt-bound RAM
  APIs rather than an extracted save directory.
- The supplied Macintosh retail BIN/CUE ZIP is now injected into the existing
  real-media CTest suite. It exercises native in-memory HFS forks, MooV
  resources and in-memory container admission, sound resources, pointer input,
  New-Game flow and wall-source census rather than silently skipping because
  the archive environment was unset.
- The Macintosh SDL bridge now preserves Return's source-owned modal action:
  `NEW_GAME` at the entrance and `CLOSE_CREDITS` in credits both reach the
  existing M11 Accept owner, while gameplay Return remains `WAKE`. This fixes
  a dropped keyboard route without inventing a selection or save; the retail
  movie, New-Game/runtime and Macintosh input-table regressions cover it.
- The native Macintosh QuickTime reader now validates the original rebased
  `moov`/`mdat` sample tables in RAM for every retail MooV: `Title.MooV`
  (Cinepak/`twos`), `Swoosh.MooV` (QuickTime Animation/`raw `), and
  `Credits.MooV`/`Ending.MooV` (Cinepak/`raw `). It derives every admitted
  sample span from original `stsc`/`stsz`/`stco` tables, with no extraction,
  placeholder frames, or host codec involved.
- `Swoosh.MooV` now has a dependency-free native playback lane: the original
  16-bit QuickTime Animation (`rle `) samples and `raw ` PCM packets are read
  directly from the admitted sample spans. Twelve authentic consecutive frames
  and their timing/audio handoff pass from the packed retail ZIP in RAM.
- All four supplied retail Macintosh movies now decode natively in memory:
  the 24-bit Cinepak (`cvid`) vector/codebook stream and signed `twos` PCM
  used by `Title.MooV`, the 16-bit Animation RLE/`raw ` lane in `Swoosh.MooV`,
  and Cinepak/`raw ` for `Credits.MooV` and `Ending.MooV`. The full authentic
  four-movie regression advances twelve frames from every stream with source
  timing and audio; the old FFmpeg configuration is no longer part of the
  Firestaff build or runtime.
- The M11 real-media route now drives the verified native Title film to its
  source completion, opens and closes Credits through its authentic input
  event, then enters the retained GAME_LOAD candidate. It must click the
  original 224×136 mirror viewport before the source DB3 mirror selection
  publishes the New Game STARTEND session; this preserves the two-stage
  source sequence instead of treating Enter as a synthetic champion choice.
- The real DOS English/French, Amiga, FM Towns, and Macintosh startup tests now
  each exercise the ordinary start-menu handoff separately from boot-probe
  mode. `FIRESTAFF_FAIL_IF_NO_LAUNCH` and `FIRESTAFF_EXIT_AFTER_LAUNCH` make a
  missing menu launch fail before the existing source-owned GDAT and movement
  assertions run.

- Native FM Towns ZIP/CUE/IMG intake reads original media in RAM, verifies the
  source-owned graphics/dungeon pair and preserves virtual source ownership.
- Real FM Towns M12/M11 startup, title and gameplay corpus checks pass with
  the authentic FM Towns archive and English DOS companion.
- The authentic Amiga installer archive now reaches title, original New Game,
  runtime and a visible native CHARSHEET inventory frame. Its 121×72 RAW4
  source is clipped to the original 119×70 destination using verified GDAT
  pixels and palette, rather than a substitute surface.
- The authentic PC-DOS ZIP now starts through both CLI and the start menu.
  M12 retains its verified `data/GRAPHICS.DAT` and `data/DUNGEON.DAT` virtual
  paths and the native DM2 boot owner reads them only in RAM.
- Generic ZIP hash discoveries remain diagnostic-only: they cannot claim a
  DM2 runtime route or redirect data into a cache. Only a supported,
  edition-specific archive owner may publish a native in-memory launch path.
- The authentic PC-DOS ZIP SKSAVE corpus is now also read directly in memory.
  The source scanner recognises all four `data/sksaveN.dat` primaries and four
  backups, records virtual archive paths and complete-file hashes, and can
  reread each receipt-bound payload without extracting a game-data member.
  The public slot scan, validity, and bounded-read APIs use the same virtual
  paths, so start-menu resume discovery does not require an unpacked save.
- The authentic Amiga installer now binds its native big-endian, 16-colour
  `INTERFACE_GENERAL/0` PalIRGB field 0 rather than PC field `0xfe`/PAL16.
  Its runtime HUD uses the source palette's physical-index receipt, matching
  the original 4-bit Amiga images without a fabricated local palette. The ZIP
  remains memory-owned through the native installer path. The current
  `dm2_v1_amiga_native_cli_boot` regression verifies the M12 New Game route,
  an accepted first frame with authentic assets and zero fallbacks, its
  presented screenshot, and the native input matrix. Physical M5 rendering
  and exact visual parity remain unverified.
- The authentic Macintosh retail ZIP now keeps its normal 256-row
  `PalIRGB`/`dtPalette16` pair rather than being mistaken for the Amiga
  16-colour palette layout solely because both formats are big-endian. Its
  start-menu/title/New Game/movement route produces an accepted M11 frame with
  real assets and zero fallback draws, directly from the original ZIP in RAM.
- The authentic Macintosh *First Chapter* demo is explicitly fail-closed in
  the CLI regression suite.  An explicit demo archive is isolated from all
  sibling media and must never become a retail DM2 runtime owner.
- The DOS archive save matrix now verifies every supplied primary and backup
  `SKSAVE` through the native, read-only start-menu resume handoff. It treats
  frame acceptance as a separate presentation boundary, so a valid source
  load is not rejected merely because its initial saved pose has not yet
  supplied a renderable viewport transaction.
- DM2 Macintosh real-media tests now select retail and First Chapter archives
  by their locked SHA-256 identities across the original and duplicate-suffixed
  filenames. This keeps retail boot coverage on the full archive and the
  negative admission test on the demo when both share a basename.
