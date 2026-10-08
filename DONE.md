# Firestaff DONE — cross-game completed work

- 2026-10-08: Re-ran the authentic-media CLI/startup matrix for DM1, CSB and
  DM2 across the locally supplied DOS, Macintosh, Atari ST, Amiga and FM Towns
  editions. Of the 24 matrix tests, 23 passed and the loose-file Atari R1 Hint
  Oracle test skipped because its two standalone STX paths are absent. The
  separate `csb_v1_hint_oracle_native_7z_cli_boot` passed against the original
  Atari multi-member preservation archive and verified both CLI and M12 menu
  handoffs. This yields 24 passing original-media startup routes and one
  documented skip; it does not claim complete game or visual parity. The
  original-media `m12_persisted_root_original_handoff` also passed in 49.06
  seconds, and `return_to_menu_rescans_dm1_csb_dm2_real_media` passed in
  214.75 seconds, verifying that gameplay returns to M12 and rescans DM1, CSB
  and DM2 with the supplied media still discoverable.

- 2026-10-08: Fixed CSB Amiga 3.3 multiplayer startup handoff. ReDMCSB
  `COMPILE.H:270-272` identifies A33M `KAOS.FTL` as `C03_GAME`, and
  `APPA.C:71-81` routes its EN/FR/GE choices through that program. Firestaff
  previously accepted only A31M's `KAOS.FTL` hash, so authentic A33M remained
  at the completed `TITL.DAT` title. The handoff and sidecar admission now
  accept the exact A33M digest `dc2f97e177843046a969ebc2d7b74778` from the
  selected ADF; fingerprint lookup recognizes it as original CSB Amiga
  `KAOS.FTL`, and M12 labels the shared media family 3.1/3.3. The registered
  `csb_v1_amiga33_original_adf_cli_boot` passed in 70.05 seconds using the
  original ADF repackaged into ZIP. The new
  `csb_v1_amiga33_loose_folder_cli_boot` passed in 71.01 seconds against the
  same original ADF installed as AmigaDOS files. The new
  `csb_v1_amiga33_m12_m11_real_media_handoff` passed against the original ZIP,
  covering real M12 selection, package ownership, native title, language
  handoff, entrance, and live runtime (55 assertions, 2.78 seconds). The three
  original-media routes cover CLI title,
  original/modern presentation modes, the input matrix, M12 start-menu runtime
  and AUTO Amiga route. The existing A31 test passed in 70.96 seconds, the
  fingerprint test passed, and the M11 handoff regression passed in 1.46
  seconds. This verifies startup and input, not full campaign parity.

- 2026-10-08: `test_csb_v1_amiga_adf_archive_cli_boot.sh` passed in 10.25
  seconds against the installed original `Chaos Strikes Back (FTL).zip`.
  The ZIP-to-ADF route reached the source entrance, launched through M12, and
  verified native movement. This confirms the archive-backed Amiga media
  route; it does not establish full Amiga parity.

- 2026-10-08: With the current launcher freshly linked,
  `firestaff_cli_startup_diagnostics_real_media` passed in 124.92 seconds
  against installed original media. It verified that `--debug` reports search
  roots, candidates and selected editions, `--verbose` lists candidate files,
  DM2 FM Towns logs all 225 timed title frames, and DM1/CSB/DM2 AUTO select
  FM Towns when it is available. It also checked unsupported DM1 PC-98 media
  stays unselectable. SDL dummy-video diagnostics do not verify a physical
  desktop display.

- 2026-10-08: With the current launcher freshly linked,
  `return_to_menu_rescans_dm1_csb_dm2_real_media` passed in 222.06 seconds
  against the installed authentic DM1, CSB and DM2 FM Towns archives. It
  launched DM2 from the menu, returned from runtime, and verified that all
  three game roots were rescanned and rediscovered. This is headless menu-flow
  evidence, not physical M5 display or input verification.

- 2026-10-08: Removed a 200 ms polling ceiling from the asynchronous M12 data
  directory scan regression. The test now waits up to five seconds while
  continuing to pump menu updates; all original result assertions remain. The
  focused CTest passed four consecutive times with isolated scratch roots and
  once with the host's default `TMPDIR`, after a transient timeout on the old
  ceiling.

- 2026-10-08: Re-ran the installed original-media startup matrices on the
  local macOS host: 11 DM1 CLI routes across PC/DOS, Atari ST and FM Towns,
  plus Amiga HD/v2.0 CLI and v2.0 title/entrance checks; six CSB Atari ST,
  Amiga and FM Towns routes; and five DM2 DOS English/French, Macintosh,
  Amiga and FM Towns routes all passed. The shared startup
  diagnostics test also passed, including the 225-frame DM2 FM Towns title
  trace, default data-root/AUTO selection, and rejection of PC-98 as a
  selectable DM1 platform. CSB's French Atari preservation ZIP case was not
  run because its expected archive is not installed. SDL dummy-driver checks
  establish startup/runtime receipts, not physical display, audio, or HiDPI
  behavior. Separately, GitHub Actions run 37766018435 passed on Windows,
  macOS and Ubuntu, including the focused Windows DM1/CSB/DM2 startup contracts.

- 2026-10-08: The persisted-root M12-to-M11 regression now covers authenticated
  CSB Amiga 3.1 English and verifies that the reopened root retains its native
  A31E boot profile. Against installed original data, the expanded persisted
  root test passed in 43.04 seconds, the dedicated A31E startup handoff passed
  in 1.84 seconds, the mixed CSB FM Towns AUTO route passed in 53.35 seconds,
  and the DM1/CSB/DM2 return-to-menu rescan passed in 217.65 seconds.
  A fresh original-media rerun of the cross-game return-to-menu rescan also
  passed on 2026-10-08 in 218.60 seconds.
  The M12 scan-progress test passed with the full five-game corpus enabled,
  checking completion/cancellation callbacks, the visible lower-middle bar,
  and discovery of all five installed game roots.
  DM1 Atari ST 1.1 also passed its authentic CLI/M12/Hall route in 63.42
  seconds. These checks verify startup and menu flow, not visual parity or
  physical M5 HiDPI behavior.

- 2026-10-07: The authentic all-games launcher regression now asserts that
  AUTO selects the matched FM Towns edition for DM1, CSB and DM2 whenever it
  is installed. The test passes against the external original-media
  collection; this verifies startup selection and does not claim visual
  parity or physical M5 behavior.

- 2026-10-07: The authentic multi-edition startup matrix passed all 14
  installed DM1 and five DM2 CLI-start tests, plus seven CSB CLI-start tests.
  Two CSB cases skipped because the Atari R1 campaign/Utility Disk and French
  Atari preservation ZIP are not installed. The DM1/CSB/DM2 mixed-media AUTO,
  selected-root launcher preparation, menu return/rescan and startup-diagnostic
  tests also passed. These checks establish startup routing and source-owned
  runtime receipts, not visual parity or physical M5 HiDPI behavior.

- 2026-10-07: Extended `return_to_menu_rescans_dm1_csb_dm2_real_media` with an
  authentic DM1 FM Towns run. The normal M12 route reaches the first HoC frame,
  returns to the launcher, and rediscovers all three original DM1, CSB, and DM2
  collections. The complete regression passes in 210.66 seconds, including
  the existing DM1 PC, CSB FM Towns, and DM2 AUTO return routes.

- 2026-10-07: Extended the persisted-root original-media launcher regression
  to cover DM1 Amiga 2.0, CSB FM Towns Japanese and DM2 Macintosh retail in
  addition to the existing DM1 PC 3.4, CSB FM Towns English and DM2 DOS cases.
  Each selected edition survives menu reopen and reaches its source-owned M11
  startup state. The focused original-media test passes against the external
  collection; the full local CTest catalogue remains unavailable because one
  unrelated test requires the absent ReDMCSB checkout.

- 2026-10-07: Three original-media DM1 CLI startup routes passed: DOS English
  in 45.14 seconds, nested PC-34 ZIP in 230.82 seconds, and FM Towns in
  262.33 seconds. The PC-34 route also rejected an isolated copy missing its
  required SWSH prelude, consumed the complete 23-step C001 title through
  M12, and reached the HoC runtime. FM Towns completed its platform-specific
  boot and movement checks. These are headless startup/input checks, not
  physical M5 or visual-parity claims.

- 2026-10-07: DM1 Amiga HD and Amiga v2.0 original-media CLI startup tests
  passed in 13.02 and 59.09 seconds. The HD route reached its authenticated
  runtime startup; v2.0 completed its source disk/title and Hall of Champions
  route. These headless checks do not establish physical M5 presentation or
  visual parity.

- 2026-10-07: `dm1_v1_dos_fr_zip_cli_boot` passed in 14.43 seconds with the
  original French DOS ZIP and required PC-34 source media. This verifies the
  localized CLI/title/menu startup path against authentic data.

- 2026-10-07: `dm1_v1_atari_st_11_archive_cli_boot` passed in 63.50 seconds
  against original DM1 Atari ST 1.1 media, exercising its CLI/start-menu
  startup path. This verifies that edition's source startup route; it does
  not claim physical M5 output or visual parity.

- 2026-10-07: CMake now enables the real-corpus M12 rescan integration check
  when all five installed game directories are present. `m12_data_dir_cancel`
  passed against `/Volumes/Extern-disk/FirestaffUserData/data`, proving the
  selected DM1 leaf is promoted to the collection root, all five original
  game datasets are rediscovered, and reopening the launcher exposes them.
  The check uses installed media and skips unsupported packed archives rather
  than extracting them through external tools.

- 2026-10-07: Extended `return_to_menu_rescans_dm1_csb_dm2_real_media` to
  launch DM2 without `--platform`, using the normal FM Towns default and its
  original Japanese archive. The complete real-media regression passed again
  on 2026-10-07 in 134.92 seconds: DM1 PC-34, CSB FM Towns and DM2 FM Towns
  each reached a loaded game, returned to M12, and rediscovered authentic DM1,
  CSB and DM2 media. This proves the return rescan works for the prioritized
  three-game set, including DM2's default platform.

- 2026-10-06: The initial full game-data scan and subsequent launcher rescans
  now share the modern true-color progress presentation instead of switching
  back to the indexed legacy screen. The progress panel retains localized game
  and scan-step labels. A renderer regression checks the 50% fill and task
  label; the current-source CLI build and data-directory regression pass, and
  a headless startup-menu scan over the installed five-game corpus returns to
  the menu without a renderer error. An opt-in regression starts with the
  installed DM1 leaf and verifies that the return rescan discovers all five
  installed games. Physical desktop interaction still needs a separate check.
  Direct CLI probes without `--platform` also pass against installed original
  media: DM1 reaches a loaded runtime; CSB reaches map 4 with its authenticated
  party; DM2 defaults to FM Towns and reaches map 0 with real graphics and no
  fallback draws. The CSB and DM2 probes include source menu inputs to enter
  new games.

- 2026-10-07: `m11_direct_launch_prepare_all_games` passed against a temporary
  root containing only symlinks to the installed original DM1, CSB and DM2
  media. It recorded 148 passing assertions, no failures and four explicit
  skips; Nexus and Theron were not staged. For each available game, the test
  exercised direct `--game` preparation and the visible M12 game/platform/
  custom/launch handoff into M11, preserving the hash-verified selected edition.
  The separate direct boot probes reached each game's source-owned startup
  checkpoint. This does not claim that every edition reaches its first M11
  runtime frame through the visible menu.

- 2026-10-06: The 480x270 legacy launcher now routes mouse clicks to its
  visible platform and presentation cards. The route is limited to those two
  card stages, and clicks outside the card are ignored. The focused M12
  polished UI flow passes with the updated menu and hit-test code; the touched
  engine and UI translation units pass syntax checks. Full hosted CI is pending.

- 2026-10-06: The legacy palette menu now renders the active platform and
  presentation card flow instead of showing detailed game options while its
  input handler changes cards. Platform readiness follows the selected
  architecture; the V2.1 card and detailed-options action have explicit
  labels. The compact layout is covered at 320x200 and 320x100, and the new
  V2.1 label has reviewed translations. GitHub Actions run 37502261102 passed
  on Linux, macOS and Windows; the M12 flow regression passed on all three,
  while the separate M12 menu-selection regression passed on Linux and macOS
  and was skipped by its Windows workflow condition.

- 2026-10-06: DM1 Amiga ADF startup now keeps the authenticated dungeon's
  little-endian F0434 fields separate from the big-endian IMG2 graphics flag.
  The installed `[HD]` disk enters through its direct runtime route, while the
  v2.0 floppy preservation set completes SWSH/TITLE/ENTRANCE and the first
  Hall runtime frame. Original-media tests pass for Amiga HD, Amiga v2.0,
  English DOS, Atari ST v1.2, FM Towns JA/EN and CSB Amiga; DM2 startup
  diagnostics and six focused DM1/CSB/DM2 launcher CTests also pass. This
  proves these routes only, not full platform/gameplay or visual parity.

- 2026-10-06: `--debug` now reports scan roots with elapsed time, presented
  startup phase/frame changes with elapsed host time, selected
  game/platform/edition and source path, DM1's source-phase handoff, DM2 FM
  Towns TWANIM/Timer-A progress, and Macintosh Title.MooV frame progress.
  Verbose renderer output includes logical-window and drawable dimensions for
  HiDPI diagnosis. Authenticated DM1, CSB and DM2
  direct launches, DM1 M12 menu launch, DM2 Macintosh movie playback and the
  persisted-root original-media handoff passed locally. SDL dummy-driver runs
  do not establish physical M5 presentation or visual/audio parity.

- 2026-10-05: Consolidated 51 optional DM1/CSB CMake test registrations into
  one helper, removing 680 lines from the root build file. Every target and
  CTest name remains separate. A before/after local CMake configuration has
  byte-identical CTest JSON and identical compile commands and Ninja link
  rules for all 51 targets; hosted platform builds await this commit.

- 2026-10-05: Moved only the DM1, CSB and DM2 launcher card RGB arrays from
  the mixed generated-card C source to three exact 129,600-byte project-owned
  binaries and deterministic build-time C generation. Each array compares
  byte for byte with its previous initializer; the Nexus/Theron arrays and
  lookup logic remain byte-identical. The full batch removes about 21,500
  tracked text lines and 1.17 MB of tracked bytes. Generated C compilation and
  local CMake configuration pass; hosted platform builds await this commit.

- 2026-10-04: The 17,071-line launcher readme-logo RGB initializer is now an
  exact 307,200-byte project-owned binary. The shared build generator preserves
  the compiled logo symbol and rail symbol; both generated C files pass syntax
  compilation and local CMake configuration succeeds. The logo bytes match the
  original initializer (SHA-256 `d8b16134f3917dee186ef726e0e13062491cd88eb4bf6edd342be28cbbc7ad34`).
  Hosted Linux, macOS and Windows builds passed in run 37238198304.

- 2026-10-04: Replaced the launcher's 41,498-line rail RGB initializer with
  the same 746,880 bytes stored as a project-owned binary and a deterministic
  build-time C generator. The original initializer and binary compare byte for
  byte (SHA-256 `f9e5da21319ef238f523938fa5dbcf58c95d888708cade4eb89635067776b5a3`);
  the generated C compiles and local CMake configuration succeeds. This reduces
  tracked text by 41,431 net lines and tracked bytes by about 2.38 MB without
  changing startup artwork. Hosted Linux, macOS and Windows builds passed in
  run 37237242658.

- 2026-10-04: The persisted-root original-media M12 regression now follows
  the authenticated DM2 FM Towns AUTO selection through its full SWOOSH/TITLE
  sequence, the source New Game target, and the first mirror action into a
  loaded one-champion dungeon session. It passes with the local mixed DM1,
  CSB and DM2 collection under SDL dummy drivers. This is a menu-to-game
  functional check; it does not establish physical M5 input or visual parity.

- 2026-10-03: The opt-in persisted-root original-media handoff test now also
  checks AUTO after a stale matched version row for DM1, CSB and DM2. Each
  launch intent selects authenticated FM Towns media, and M11 owns the DM1
  startup receipt, CSB English FM Towns program/graphics, and DM2 Japanese
  FM Towns disc. The macOS Actions-built binary passed against the local
  collection. Explicit PC 3.4/CSB FM Towns/DOS cases remain covered. This
  proves menu-to-runtime handoff, not gameplay or physical M5 presentation.

- 2026-10-03: An opt-in original-media test persists the selected M12 data
  root, reopens the menu, and calls the normal launch-intent and selected-entry
  handoff for DM1 PC 3.4, CSB FM Towns and DM2 DOS. A macOS Actions-built
  binary passed locally against the authenticated collection with SDL dummy
  drivers. It verifies source-owned M11 startup state for these three
  editions, not later gameplay or every platform.

- 2026-10-03: The start menu keeps an authenticated data root selected with
  the folder picker when reopened, even when the default root admits more
  games. A macOS GitHub Actions build ran the original DM1 PC 3.4 picker
  corpus locally: the previous build failed the reopen check and the corrected
  build passed. The fallback policy test and hosted cross-platform build pass.
  This picker test alone verifies root selection, not game launch.

- 2026-09-29: DM1 PC/F20 no longer treats one presented title frame as a
  completed intro. All source steps or all TITLE.DAT frames and the final
  guard must finish before Entrance; an interrupted title aborts the handoff.
  The post-launch regression and an authentic PC-34 M12-to-M11 startup pass.
- 2026-09-29: CSB Atari ST SND1 zero-amplitude PSG samples now remain silent
  in loud and soft modes. Focused PCM regressions cover both mute tables and
  a nonzero sample; physical-device output is not established by this check.
  Three sounds in the supplied Atari archive remain rejected by the bounded
  source decoder, as recorded in TODO-csb.md.

- 2026-09-29: Release CI verified repeated Windows config and JSON writes.
  iOS packaging then exposed LZMA ARM CRC intrinsics incompatible with the
  baseline Apple Clang target. An iOS-only source definition selects software
  CRC; arm64 iPhoneOS compilation, no-CRC-instruction assembly inspection and
  the standard CRC32 test vector pass locally. Hosted iOS packaging remains
  the final check against the release runner compiler.

- 2026-09-29: Release CI exposed Windows CRT rename rejecting existing
  configuration files. Config and both JSON exports now use native replacement
  without deleting the previous file first. Three focused launcher CTests
  pass locally, including second-write readback and failed-replacement retention;
  Windows verification awaits the next hosted run.

- 2026-09-29: Translated the five new Custom Music folder dialog messages
  into Swedish and recorded them in the catalog maintenance table. Reviewed
  wording against Swedish terminology/translation memory and checked the
  selected strings with l10n-lint, svlang, Hunspell and GNU gettext.

- 2026-09-29: Added an explicit hosted CI build/run step for the launcher
  options, data-directory cancellation and font/artpack dialog regressions
  on Linux, macOS and Windows. The exact three-test command passes locally
  and workflow YAML parses. Hosted results remain pending publication;
  these tests cover settings/ownership contracts, not original-media parity.

- 2026-09-29: DM2 launcher save discovery honors FIRESTAFF_DM2_SAVE_ROOT,
  matching the existing runtime override, and requires hash-admitted DM2
  media rather than a directory named dm2. Candidates still pass primary
  DAT and original SKSave checks. Save-manifest import preserves the chosen
  data root and persists the validated fallback save instead of overwriting
  it with a missing imported path. The authentic DOS corpus regression
  failed before the fix and passes afterward, including the no-media gate.
  Six focused CTests pass, plus rebuilt DM1 PC34 332, CSB Amiga 48 and
  DM2 DOS 60 original-media handoff assertions.

- 2026-09-29: Quick Resume retains its remembered save path while disabled
  and reprobes when enabled through Settings. An unrelated settings save
  no longer erases that path. A new isolated test reads the authentic Amiga
  v2.0 nested ZIP/ADF save in memory: OFF-save and ON/OFF/ON checks failed
  before the fix and pass afterward, without fabricated saves or forced
  asset availability. Five focused CTests and rebuilt original-media handoff
  checks pass (DM1 PC34 332, CSB Amiga 48, DM2 DOS 60 assertions). This
  verifies menu preferences, not new save support.

- 2026-09-29: Unicode Font and Artpack dialogs now apply selections and
  persist settings only from main-thread Update. Independent result tokens
  survive menu destruction until callbacks return; cancellation and
  overlong paths preserve the prior choice. All four focused dialog tests
  pass, including worker delivery, late callback after owner free and an
  actual installed font path persisted to isolated configuration. Artpack
  coverage uses admission metadata, not artwork/rendering evidence.
  Rebuilt original-media handoff checks pass DM1 PC34 332, CSB Amiga 48
  and DM2 DOS 60 assertions.

- 2026-09-29: Data Directory folder callbacks now publish an independently
  owned result; Update starts the scan on the main thread. Destruction is
  safe before a late callback. An original PC34 archive passes folder
  selection, hash admission and isolated configuration persistence. Rebuilt
  launcher checks pass DM1 PC34 332, CSB Amiga 48 and DM2 DOS 60 assertions.
  Process-specific configuration/originals overrides isolate integration
  checks without changing HOME or scanning the installed media collection.
  All three focused dialog/handoff CTests pass; desktop interaction remains
  a separate verification task.

- 2026-09-29: Connected Custom Music folder selection to a native folder
  picker. Admission stores a complete absolute directory and preserves the
  previous choice on invalid/cancelled input. Native callbacks publish to
  an independently owned result; the main-thread Update applies it, and late
  callbacks remain safe after menu destruction. Worker cancellation and
  late-completion tests pass in the launcher-options handoff test.
  Soundtrack file resolution now returns absolute paths and rejects short
  output buffers; a test derives a WAV from authenticated SONG PCM and
  verifies relative-path resolution, WAV readability and missing-file fallback.
  Both focused CTests pass. Rebuilt original-media launcher checks pass
  DM1 PC34 332, CSB Amiga 48 and DM2 DOS 60 assertions. These checks do not
  prove native desktop picker
  interaction or replacement soundtrack playback, which remains unwired.

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

- 2026-09-29: Stopped substituting SONG.DAT for positive PC34 CD-track
  requests, matching ReDMCSB MUSIC.C/IO.C and IBMIO.C F8123's empty PC driver
  operation. Explicit title playback remains separate, as does FM Towns CDDA.
  Authentic tests cover track IDs 1, 15, 20 and 5 with stopped music and with
  a partially consumed paused title queue, preserving gain and effects.
  DM1 launcher checks pass 280 assertions and CSB Amiga checks pass 48,
  without skips. Continuous title looping remains tracked in TODO.md.

- 2026-09-29: Restored keyboard Left/Right navigation between launcher
  Settings tabs. The SDL key route now emits menu navigation in Settings
  while preserving active gameplay and text-editor input. The focused
  regression exercises Game to Graphics and back through the production
  mapping and Settings handler; the narrow build and CTest pass.

- 2026-09-29: Isolated DM2 Mac film PCM in a dedicated stream with live
  master gain, selected-device routing, host pause and shutdown ownership.
  Credits cancellation/reopen clears only film audio, preserving original
  snd-resource effects. EOF queues any decoder tail before waiting for SDL
  drain, then clears the movie owner. Authentic tests pass cancellation with
  queued Credits PCM plus snd 10001, reopen, normal final-frame/tail drain,
  four movie gain readbacks and explicit unavailable-audio operation. The
  menu-to-gameplay and live-clock checks remain enabled. Narrow build and
  DM1/CSB original-media launcher regressions pass; physical listening and
  FFmpeg-specific end-packet execution remain separate verification work.

- 2026-09-29: Added opt-in full-duration DOS intro playback to the authentic
  DM2 launcher test (`FIRESTAFF_DM2_LIVE_INTRO=1`). A fresh selected-media
  session runs without clock writes, fast-forward or audio-queue clearing;
  it verifies ordered delivery of all 217 frames, the 18,082,176-us source
  duration, completion without rejection and return to the source menu.
  This host completes in 18.717 seconds with SDL dummy audio; all 63 checks
  pass without skips. Audible synchronization and physical display delivery
  are separate from this real-time host-clock test.

- 2026-09-29: Shared live SDL3 logical/drawable dimension queries between
  presentation, pointer mapping and window-size getters, closing the stale
  render/live-input split before resize-event handling. Dummy probes retain
  cached dimensions. Geometry tests pass, and an opt-in native Cocoa probe
  passes actual grow/shrink before M11 HandleResize: 900x650 to 1060x770 to
  940x670. The native test checks production presentation bounds, size getters
  and pointer mapping. Its drawable density is 1x; Retina hardware remains
  unverified. The sandbox cannot open Cocoa, so this probe ran on the host.

- 2026-09-29: Kept the final DOS MVE page for its complete source timer
  period and retained queued SDL PCM until drained before releasing the movie
  owner. Pause rebases the final-page timestamp; no-device and explicit
  boot-probe fast-forward retain their separate contracts. Authentic INTRO
  tests pass the exact final-period boundary and normal final-packet drain.
  The test locates the last audio-bearing boundary before the original's
  eleven terminal image-only boundaries, without fabricated PCM. DOS launcher
  checks pass 60 assertions with SDL dummy output and 55 with unavailable
  output, all without skips; the source presenter test and narrow build pass.

- 2026-09-29: Corrected SDL3 pointer mapping for fixed-scale HiDPI windows.
  Logical window coordinates now map through the same drawable rectangle
  as rendering, with independent density ratios per axis and rejected bars.
  Native dimension-query failures reject input; dummy probes retain cached
  dimensions. Geometry regressions pass for 1x-4x, both integer settings,
  FIT, Retina and mixed-axis density. The entrance command suite passes 308
  checks, and authentic DM1/CSB launcher regressions pass. This does not prove
  native Retina event delivery or fix the pre-resize live/cache discrepancy.

- 2026-09-29: Added a live-clock check before the DM2 Mac test's accelerated
  movie traversal. Eight authentic title-frame transitions now run under
  SDL's unmodified monotonic clock; the test requires held frames, rejects
  frames ahead of their source timestamp and fails if progress stalls.
  The narrow build and full M12/movie/runtime test pass. This checks a short
  pacing segment, not full-film timing, physical audio/video sync or HiDPI.

- 2026-09-29: Extended the authentic DM2 Macintosh retail movie regression
  through normal M12 pointer selection: game card, Mac platform, Custom
  options and verified launch intent. The same session passes title/credits,
  timer/focus pause, New Game and source mirror selection, then draws an
  accepted runtime frame with real assets, no core fallbacks and zero fallback
  draws. The narrow build/test passes on the installed retail ZIP. A separate
  CLI boot probe also reaches its admitted startup phase. This proves the
  headless menu/API path, not desktop input delivery or visual parity.

- 2026-09-29: Bound both temporary DM1 title-music owners to authenticated
  SONG.DAT beside the selected GRAPHICS.DAT, including virtual archive paths.
  Missing/incompatible companions clear initialization-time fallback music;
  the runtime binder also clears before path-validation short circuits.
  The original-media SWSH/SONG test verifies exact selection, invalid rebind,
  queued-music removal and no resurrection on host resume. DM1 launcher
  coverage remains 248 passed with no skips across four presentation modes.
  Narrow app build and refreshed localization source references pass.

- 2026-09-29: Applied launcher master/music/SFX and mute before the four
  temporary DM1/CSB SWSH/title audio owners queue their first source sound.
  Authentic DM1 SONG.DAT/SWSH checks pass with reduced gain, mute and retained
  host pause. The equivalent CSB PC34 test is wired but skips locally because
  its authenticated package is unavailable; CSB runtime coverage is not claimed.
  Mac QuickTime's complete movie mix now uses master gain, consistently with
  DOS MVE, instead of dropping all film audio when Music is zero. SDL readback
  of authentic Title/Swoosh/Credits/Ending PCM passes four master/music settings;
  Mac movie startup/menu/pause regressions and the narrow app build also pass.
  These checks do not establish physical-output listening or full startup parity.

- 2026-09-29: Shared the selected playback-device name across M11 effects,
  SONG.DAT, CDDA and the independent DM2 GDAT/MVE devices. A leaf SDL library
  resolves the current name on every stream open, with default fallback for
  empty or unavailable names, avoiding stale device IDs and circular runtime
  dependencies. Authentic DM1 launcher tests pass 248 checks, including
  actual SDL device names for all three M11 streams. Real GDAT and MVE tests
  verify the selected opened device; empty/missing-name fallback and no-device
  MVE also pass. Narrow app/standalone builds and localization checks pass.
  Dummy-device evidence does not establish physical multi-device hotplug.

- 2026-09-29: Wired DM2 launcher master/SFX gain to the GDAT effect backend
  before lazy device open, after binding the authenticated source session.
  The device applies user gain separately from original voice attenuation;
  mute preserves source voice progression. DOS MVE's separate mixed-audio
  stream receives master/mute before its first PCM packet. Runtime setters
  retain queue/pause ownership and the no-device MVE path retains source
  receipts. Authentic DOS launcher checks pass 49 assertions, including
  changed gain and muted/unmuted relaunch. Real GDAT playback and complete
  intro/end MVE packet tests pass, including forced no-device delivery;
  Mac retail movie regressions also pass. Narrow app build passes.

- 2026-09-29: Connected master/music gain to the dedicated CDDA stream.
  FM Towns CD music now responds to either mute and to volume changes after
  PCM is queued, without clearing its position or changing pause ownership.
  The opt-in audio transport regression passes with one second of authentic
  DM1 FM Towns track 2 read from the retail CUE/BIN ZIP in memory. It checks
  gain before/after queueing, both mutes, full gain, queue retention and
  overlapping source/host pause. Assertions remain active in Release builds.
  This is SDL dummy-device evidence, not a physical-output listening test.

- 2026-09-29: Gave native SONG.DAT playback a dedicated SDL music stream.
  Source track zero and Music Off now stop queued music without touching SFX;
  subsequent requests replace the prior queue. Master/music gain changes
  already queued playback. Host pause retains PCM, resumes only its own
  suspension, and cannot resurrect music stopped while paused. Rebinding and
  shutdown clear the music owner. Authentic DM1 DOS launcher tests pass 228
  checks across four modes using selected SONG.DAT and SND3, including a live
  effects queue. CSB Amiga 48, DM2 DOS 44 and Mac retail movie tests pass.
  Narrow app build and independent review pass. This preserves the existing
  sequence decoder; it does not establish per-track selection/loop parity.

- 2026-09-29: Refreshed startup-menu POT/PO source references after AUTO PAUSE
  changed main-loop line numbers. This fixes the localization-catalogs failure
  in Actions run 36539879762; local `bash po/update.sh --check` passes with
  current catalogs and no structural errors.

- 2026-09-29: Implemented DM1/CSB/DM2 AUTO PAUSE runtime wiring. Focus
  loss and session-timer pause use independent reason bits, preserving audio
  suspension until the last owner releases it. Source idle/food/movie clocks
  and input stop during focus pause. Synchronous intro waits measure active
  time, and the main loop rebases after pause or a completed synchronous
  launch. New pause ownership resets the gesture recognizer.
  Authentic-media checks: DM1 DOS 172, CSB Amiga 48, DM2 DOS 44 pass without
  skips; Mac retail movie focus/timer pause, resume and New Game also pass.
  Authentic Atari MINI.DAT cold-resume checks reject a stale swipe across
  focus pause and accept a fresh swipe. Main-loop
  syntax and HUD source gates pass. The optional Cocoa probe opens a window
  outside the sandbox but cannot acquire actual input focus; desktop focus
  behavior through the complete main loop remains an explicit TODO.

- 2026-09-29: Fixed session-timer startup and frame-time accounting. Timer
  initialization now follows per-game startup resets, and millisecond carry
  preserves ordinary subsecond frames. The deadline requests a redraw and
  its dialog is composed after source rendering, including CSB/DM2 early
  return paths. Forced pause gates idle/food clocks, pointer input and direct
  save/load shortcuts while permitting dismissal of a pending F0349 command.
  Host audio pause preserves queued PCM and existing device/CDDA pause state;
  DOS MVE and Mac movie draws freeze decoding and resume with rebased clocks.
  Evidence: real-media handoff checks DM1 152, CSB Amiga 48, DM2 DOS 41,
  all without failures/skips; DOS test requires an active authentic MVE.
  Mac full retail Title.MooV pause/audio/resume and subsequent New Game pass.
  Repeated paused CSB redraw is byte-stable. The real DM1 object corpus passes
  43 chest residents and 976 records, including timer dismissal during the
  original food wait and source C08 completion. Main-loop syntax check and
  independent review pass. The HUD source gate follows the extracted source
  renderer and also locks source-before-timer composition in the public wrapper.
  Audio evidence uses SDL dummy devices; physical
  audio output and window-focus AUTO PAUSE remain separate verification/work.

- 2026-09-29: Launcher map/log preferences now replace stale QoL runtime
  values at DM1/CSB/DM2 start: minimap visibility, size and corner, plus
  combat-log visibility and line limit. Existing source-kind overlay gates
  remain unchanged. Real-media handoff checks pass: DM1 DOS 3.4 112,
  CSB Amiga A31E 45, DM2 DOS English 36, all without failures or skips.
  The data-free QoL configuration contract also passes. These checks prove
  live preference transfer, not overlay visuals or map-export contents.

- 2026-09-29: The DM1/CSB/DM2 launcher now applies the current automap
  visit-recording preference when starting a game, replacing the stale value
  loaded before menu edits. Authentic DM1 DOS 3.4 handoff: 104 passed,
  0 failed, 0 skipped, including enabled/disabled runtime values across four
  presentation modes. This verifies the preference handoff, not map-export
  contents or visual overlay parity.

- 2026-09-29: DM1/CSB/DM2 launcher speed now reaches the live QoL timing
  owner using the existing in-game Cheats mapping (50/100/150 percent).
  Cheats off or invalid speed restores 100 percent; the selected game takes
  precedence over the previously active global multiplier at launch.
  Authentic DOS 3.4 M12/M11 handoff: 100 passed, 0 failed, 0 skipped,
  including all three speeds and the cheats-off reset. Authentic CSB Amiga
  A31E handoff passes 44 checks including 50 percent; DM2 DOS English passes
  35 checks including 150 percent. Both report zero failures and zero skips.

- 2026-09-29: Made the Custom menu SPEED HOTKEYS status tile non-interactive
  so clicking it cannot change simulation speed. Removed its SPEED selection
  highlight. The existing pointer launch test passes after rebuilding.

- 2026-09-29: Fixed the source packaging omission behind issue #12. The
  published 3.0.348 SDL3.dll imports libiconv-2.dll, while its ZIP contains only
  SDL3.dll. Windows packaging now resolves and copies the recursive PE import
  tree and rejects unresolved DLLs. CMake import-graph checks with fixture
  objdump output verify transitive copying, Windows/API-set exclusions and
  failure on a missing libiconv dependency. Hosted Windows CI now stages the
  actual shipping executable and runs --help without MSYS2 on PATH; that
  native check and a replacement release remain pending.

- 2026-09-28: Added a read-only comparison view for two authentic Saturn
  Backup RAM images. It shows payload-relative changed ranges and bounded raw
  byte previews without assigning field meanings or writing either image.
  The BKR self-test now requires exactly the four captured samples with their
  recorded SHA-256 digests; the regression passed on TRV2 against those real
  files. The remaining Saturn payload schema and native save/load consumer are
  still unresolved.

- 2026-09-28: Added the authenticated Japanese Saturn `RLOWFIX.BIN` MD5 to
  Nexus's shared canonical asset table. The prior boot-profile gate accepted
  this real 72,332-byte roster archive, but the engine's named-file reader
  rejected it when seeding PLRD champions. A regional CUE regression now
  verifies the Japanese source receipt, RES* decode and 20-row champion pool;
  it passes on TRV2 against `.firestaff/data/nexus`. Regional checks skip only
  when that region's real CUE is absent. English and French hashes remain
  independently authenticated.

- 2026-09-26: The M12 launcher-options regression now pins its audio `OFF`
  assertion to explicit English rather than inheriting the host's AUTO
  locale. This prevents a Swedish host from incorrectly failing the test on
  the correct localized value `AV`; the options-handoff, M12 direct-click, and
  CLI option-form tests pass together.

- 2026-09-26: The active M11 CLI now rejects invalid `--game` values and
  malformed, missing, zero, negative, and out-of-range `--width`/`--height`
  values before game-data scanning or renderer initialization. Dimensions from
  1 through 4096 remain accepted. The CLI option regression passes; the former
  `--width nonsense` case now exits with code 2 instead of `rc=-6`. The
  authentic CSB Atari STX menu-to-C200 runtime regression still passes.

- 2026-09-26: DM1's authentic DOS English, Amiga HD and FM Towns CLI/start-menu
  regressions pass from installed original archives. The nested DOS archive
  also passes the full PC-34 M12 Hall-of-Champions and input/capture matrix;
  this rerun passed after an earlier attempt was interrupted by host disk
  exhaustion. FM Towns passed its two original-program input matrices.

- 2026-09-26: Verified authentic DM1 Atari ST M12-to-runtime routes and C127
  Hall-of-Champions recruitment on all six admitted editions: English v1.0a,
  v1.0b, v1.1 and v1.2, German v1.2, and French v1.3. The edition-specific
  CLI checks use the original archives/STX, require the menu startup handoff
  and HoC first-frame runtime receipt, then confirm source-coordinate pointer
  recruitment. The nested English v1.2 archive also passes its movement/input
  checks; German and French pass their native movement checks. The local CI
  production/source-boundary CTest selection also passes all 48 tests after
  building its missing test executables.

- 2026-09-24: Made M11's `--scale-mode` parser reject malformed and out-of-range
  values before renderer initialization. It accepts the documented numeric
  modes 0..5 and equivalent `1x`, `2x`, `3x`, `4x`, `fit`, and `stretch`
  names. The data-free CLI regression covers every accepted form plus blank,
  nonnumeric, uppercase, negative and out-of-range input. The local M11 build
  and CLI regression pass.

- 2026-09-24: DM1 Atari ST now has a platform-specific M12-to-HoC handoff.
  Its receipt follows STARTUP1.C's direct entrance route, excludes the PC34
  SWSH/title and special-palette phases, and requires the common entrance
  command before creating the HoC first-frame runtime receipt. The interactive
  entrance accepts the Atari source mouse route and ignores keyboard entrance
  shortcuts. Authentic Atari ST v1.2 English, German and French start-menu
  regressions, the DM1 PC34 start regression, the startup state-machine gate,
  HiDPI pointer mapping and entrance command dispatch tests pass. Headless
  media tests use the bounded test handoff and do not claim host-window capture
  completeness. Atari's F0437 title presentation and recruitable champions
  remain open.

- 2026-09-24: Corrected DM1 Atari ST's startup media receipt to preserve
  ReDMCSB STARTUP1.C's F0437-before-F0441 order. The Atari plan now requires a
  typed F0437 completion boundary without borrowing PC34 SWSH timing or
  palettes. Startup state-machine and sequence gates pass; Atari F0437's
  original-pixel presentation and palette remain open.

- 2026-09-24: Extended the DM1 F0128 M11 wiring regression with an optional
  authentic Atari ST v1.2 runtime path. It enters through the normal M12
  launcher handoff and proves the source dungeon is mounted and its live
  F0128 plan is valid, ready, and dispatched. The focused data-backed check
  passes all 30 assertions; the CTest remains skip-safe without user media.
  Atari palette ownership is still unverified, so this receipt does not claim
  that the rendered viewport has authentic non-black pixels.

- 2026-09-24: Corrected the real-media M11 launcher regression so it requires
  the full DM1 Hall-of-Champions pixel-capture chain only when a non-dummy
  presentation window exists. Headless runs now assert that unavailable
  capture evidence stays unclaimed. The authentic Atari ST direct-launch and
  ordinary M12 handoff cases pass for DM1 and CSB; DM1's zero-champion startup
  and CSB's zero-champion startup remain open gameplay gaps in TODO.

- 2026-09-24: Fixed native startup from authentic Theron's Quest US
  CloneCD-derived raw CUE/BIN media. Its Track 02 slice omits the 225-sector
  pregap, so campaign-mask and retrieval-text decoders now use CloneCD's
  compact source layout while the verified receipts retain the normalized US
  regional identity. On TRV2, the real-media CUE regression passed; the full
  all-games launch gate also passed (207 passed, 0 failed, 3 optional skips).
  The authentic ZIP's direct, keyboard-card and mouse-card routes also pass on
  TRV2, including the normal launcher handoff into a source-backed runtime.
  These startup receipts do not claim complete Theron gameplay or parity.

- 2026-09-24: The all-games direct-launch integration now advances Theron's
  authentic Japanese M12-selected edition through the ordinary M12-to-M11
  handoff and source-owned startup inputs, then asserts its loaded runtime
  party receipt. On TRV2 with authentic files under `.firestaff/data/theron`,
  the full gate passed 207 assertions with 0 failures and 3 optional
  media-specific skips. First presented runtime-frame coverage remains open
  in TODO.

- 2026-09-24: Fixed CSB Quick Resume dropping a validated save path unless an
  optional DSA-corpus identity had been bound. The M12 intent now carries the
  exact path for an ordinary CSB save after its native full-resume validation;
  the focused gate passes with the DSA identity explicitly unset. Authentic
  DSA-bearing save behavior remains an open media-corpus item.

- 2026-09-24: Fixed DM2's explicit `--menu --game dm2 --save` path so Enter
  selects M12 Quick Resume instead of discarding the path on a new-game row.
  Against the authentic DOS ZIP, direct resume and M12 resume now verify all
  eight `SKSAVE0–3` primary/backup slots at their original saved poses.

- 2026-09-24: The authentic DM2 French DOS ZIP now passes the normal M12 menu
  through its MVE and New-Game inputs to the native runtime receipt. Its
  retail initial party is present at `(map=0,x=1,y=8,direction=0,count=1)`;
  the separate direct probe retains the French GDAT hash and movement check.

- 2026-09-24: Fixed the authentic CSB Atari ST test's invalid `--menu` plus
  `--boot-probe` combination. Its normal M12-to-M11 route now asserts the
  source-owned entrance state at `csb-entrance-4`. Reaching gameplay through
  the Atari C200 primary-mouse command remains open in TODO; the initial roster
  still has zero champions.

- 2026-09-24: Fixed the two standalone Theron roster test targets to link
  `firestaff_theron`, which supplies the production Track 02 MD5-to-variant
  resolver required by the champion initialization path. Both focused roster
  tests now link and pass on the local macOS build.

- 2026-09-24: Normal CLI scripts now honor `waitN` / `wait:N` as main-loop
  frames and emit both mouse-down and mouse-up for `click:x:y`, matching a
  complete left click. The release is required by the source-owned CSB Amiga
  A31 AppB language selector. With authentic A31 media, the M12 start-menu
  route now verifies the handoff through its first loaded runtime frame.
  Authentic DM1 PC 3.4 and Atari ST v1.2 start-menu routes now verify their
  first runtime frame. German Atari ST 1.2 and French Atari ST 1.3 now have
  separate authenticated M12-to-runtime receipt checks. The authentic DM2 DOS
  English start menu also passes through its source MVE intro to the first
  presented runtime frame. DM2 Amiga now advances the complete original
  SWSH/TITL streams and selects New Game through its authenticated GDAT pointer
  rectangle before verifying the loaded retail party and presented runtime
  frame.
  PC 3.4 Original/Modern C040, C007 and the complete native input matrix
  still pass. The focused build and launcher
  contract test pass; the remote CSB test skips because that host has no game
  media. Cross-platform launch coverage remains open in TODO.

- 2026-09-23: Startup-menu audio-device preferences now decode quoted TOML
  strings before storing them, so names containing quotes or backslashes remain
  stable across repeated save/load cycles. A focused regression covers that
  round trip with an isolated config directory; the existing M12 settings-tab
  navigation regression also passes. Removed the committed
  `startup-menu.toml`, which contained machine-specific paths and an
  exponentially escaped audio-device value, and ignored that per-user file.
  Both focused regressions pass.

- 2026-09-06: M10 projectile collision admission now checks current-cell
  party/group occupants before forward movement (PROJEXPL.C F0219:687-697).
  It uses the source square's GROUP rather than requiring an active AI row,
  respects living cell occupancy and C48 grace, and preserves party-first
  priority. The same-square group test failed before the change; positive
  group/party-priority and negative grace/empty-cell cases now pass. Seven
  authentic DOS spell/throw regressions also pass. These bounded ownership
  fixtures do not prove all creature footprints or original capture parity.

- 2026-09-06: F0215 no longer skips C14 retirement when a surviving
  monster retains the thrown weapon in GROUP.Slot. A source-shaped RAM
  timeline regression proves the dagger has one group owner, the raw and
  decoded carrier is free, and the launch-square SFT is compacted. The test
  fails specifically on C14 retirement with the old early return and passes
  with the fix. Five authentic DOS spell tests now also require the whole
  fresh C14 pool to be unused after impact. All six tests and the production
  build pass. This fixture is not an emulator or full gameplay parity claim.

- 2026-09-06: Pending source C48/C49 events now prevent premature M11
  projectile stepping, not just repeat stepping after M10 dispatch. The
  source M10 terminal path unlinks C14 before clearing Next, preserving
  following C15 records and compacting empty SFT entries before item drops.
  Authentic DOS tests now reuse the original launch square for the F0213
  party-burst test instead of relocating the party to the impact square.
  Nine spell/throw/lifecycle regressions pass and the production build
  succeeds. GROUP.Slot transfers and broader emulator timing remain open.

- 2026-09-06: Source-bound DM1 Fireball/Lightning now apply F0213's initial
  burst after successful C15/C25 publication. Both use fire damage, source
  party precedence, nonmaterial quartering and fire-resistance RNG. C25
  consumes its separate attack roll without repeating party/group damage.
  An independent arithmetic test covers RNG stages/resistance/immunity;
  authentic DOS tests verify immediate party health loss and no C25 repeat
  using RAM-only impact placement. Eight final focused tests pass; the
  production binary builds. Legacy ownerless consumers retain their prior
  behavior until separately migrated. C25 unlink now maintains empty-square
  SFT flags/offsets through F0515. Cross-platform and emulator parity remain
  incomplete; the separate C14 empty-launch-square gap is recorded in TODO.

- 2026-09-06: Extended authentic DOS spell lifecycle verification to all
  four one-shot effects (including no premature retirement), and Poison
  Cloud's complete three-point decay through final raw/decoded C15 free.
  Fixed continuation events retaining the pre-decay owner fingerprint,
  which stalled the cloud after its first decay. All five original-media
  spell tests pass; damage timing and emulator comparisons remain open.

- 2026-09-06: Authentic DOS Poison Bolt now verifies raw/decoded C15
  retirement through its due C25 event and absence of a stale render
  receipt. This exposed two lifecycle defects: M11 advanced source-bound
  effects before their M10 event, and M10 despawn omitted original C15
  unlink/free. The source timeline now owns these effects exclusively;
  F0220 removes its authenticated source Thing before retiring the host
  slot. Five real-media spell captures and two DM1/CSB explosion runtime
  regressions pass. Emulator timing/RNG comparison remains open.

- 2026-09-06: Original DOS public spell tests now cover Fireball, Lightning,
  Harm Non Material, Poison Bolt and Poison Cloud with authentic C14 Slots,
  C15 fingerprints and captures in V1/V2.0/V2.1. Added missing FF86 launch
  mapping; corrected Poison Bolt impact to noncentered C006 rather than
  lingering C007. Raw spell C14 records retain the original magical Thing
  instead of the host no-carried-object sentinel (PROJEXPL.C F0212:76,
  F0213:149, F0217:574-585). RAM-only party setup leaves media unchanged.
  This proves the tested DOS path, not cross-platform or emulator parity.

- 2026-09-06: Authentic public LO FUL IR now reaches an original-data C15
  render receipt after wall impact. Fixed three exposed production gaps:
  fresh PC startup omitted F0434/G0236's seven spare Thing pools and 300 SFT
  entries; wall explosions were placed inside the blocker instead of the
  source square; F0115 explosion scheduling confused scheduler ordinals
  with viewport square enums. Allocation is atomic/idempotent, preserves
  original records, honors source caps and excludes save imports. Its unit
  test covers free markers, capped preservation and late-failure rollback.
  The former false-pass C15 test now binds the DOS ZIP, casts via public
  rune input, advances native ticks, requires a raw-C15 fingerprint and
  final capture in V1/V2.0/V2.1, then rejects stale missing-material capture.
  Production build and seven focused regressions pass (0.67s, no skips).
  Party setup is RAM-only; original maps/media are neither edited nor
  extracted. This is not complete emulator or cross-platform parity.

- 2026-09-06: DM1 F0115 live projectile-art selection now binds camera
  direction/cell, view lane and map parity together. Corrected rotating
  weapons' front/back bitmap choice, perpendicular cell-dependent flips,
  parallel vertical flips and SIDE lane behavior (DUNVIEW.C:5731-5805).
  Admission loads the actual directional bitmap instead of direction zero.
  Independent literal oracle covers 2,240 combinations plus 448 center-lane
  wrapper checks and invalid-output immutability. The original DOS archive
  supplies both an object-art weapon and a rotating projectile-art weapon
  to public THROW, original bitmap capture and raw-record rejection tests
  across V1/V2.0/V2.1. Three focused tests pass (0.20s); four CSB viewport,
  DM1 presentation/spell and FM Towns regressions pass (12.04s). Camera and
  mastery fixtures remain RAM-only; this is not emulator flight parity.

- 2026-09-06: DM1 live projectile presentation now passes camera-relative
  cells to C2900, packed F0115 ordering and flip selection, not absolute
  dungeon cells. The original DOS throw regression exposed the wrong-side
  render, then passed all 12 D0C camera-cell/presentation combinations
  (Original, V2.0, V2.1): exact source X/Y for front cells and no projectile
  receipt for back cells. Party/camera setup remains RAM-only; original
  object records and graphics are unchanged. Production build and focused
  throw/presentation/render checks pass. The legacy C15 test lacks local
  media binding and is not counted as additional runtime evidence.

- 2026-09-06: Fixed DM1 PC34 thrown-object material admission for fresh live
  projectiles without fabricating a saved-C14 receipt. Original raw object
  records and mounted decoder bitmaps remain required; explicit saved
  catalogs retain their validation. Corrected the D0C source-table index
  from host enum 12 (D3L coordinates) to source index 0. The original DOS
  ZIP regression now actually runs, verifies object-art capture, rejects a
  RAM-corrupted object record and checks throw-driven Ninja level-up.
  Missing media is a CTest skip, not a pass. Production build, three focused
  tests (0.21s) and four XP/action/spell/F20 regressions (15.21s) pass.
  Fixtures reposition the party/camera in RAM; HoC and emulator flight
  comparisons remain open. No game media was changed or extracted.

- 2026-09-06: Unified live DM1 XP-to-level-up mutation/publication across
  spell success/failure, parry, melee, sensors, throw, actions and influence.
  Success XP moved into M10 before cooldown; M11 no longer awards it again.
  Original I34E public Mon Light casts cross a RAM-configured threshold in
  all three modes, publish maxima without refilling current vitals and
  survive animation/redraw without XP/stat/RNG replay. Production build and
  five final regressions pass (0.49s), following four expanded checks (16.30s).
  See the XP transaction document for fixture and remaining parity boundaries.

- 2026-09-06: Level-up exposes an explicitly validated antimagic rule;
  admitted DM1 F20 M11 magic/influence awards select modulo 3, while the
  PC34 wrapper retains modulo 4. The independent oracle now covers 48
  class/level/seed/rule combinations and rejects invalid rules without
  mutations. Production build and four skill/XP/action/F20 original-media
  regressions pass (11.82s). Remaining edition admission and orchestrator
  XP-to-level-up publication remain open; this is not full XP parity.

- 2026-09-06: Live PC34 lifecycle level-up follows F0304's fixed RNG
  order, fighter/ninja/priest health factors, one mana random draw,
  PC34 antimagic and stamina random bonus. Independent unit fixtures
  compare all maxima, untouched champion fields, markers and final RNG
  for 24 class/level/seed combinations. Production build, exact oracle
  (0.04s) and three DM1 XP/spell/action regressions (8.94s) pass. This
  is source-contract coverage, not original-emulator progression proof.

- 2026-09-06: Fresh CSB FM Towns boot binds BASE.C's original RNG seed
  31459 rather than the PC default zero. Existing source-save restoration
  remains authoritative after boot. Original EN/JP media handoff tests
  assert the title seed and pass their existing routes (15.01s).

- 2026-09-06: CSB public F0303 mastery query restores original flags,
  temporary/base XP averaging, edition signedness, uncapped threshold loop,
  resting override and original action-hand/neck bonuses. Five authentic
  modifier objects pass EN/JP F31 RAM-equipment checks with full champion
  restoration; no object records are fabricated. Production build and
  three initial checks pass (13.67s), followed by three expanded runtime,
  signedness and DSA regressions (0.05s). Opaque imported-summary fixtures
  explicitly ignore equipment when checking XP. Full casting, original
  F0304 mutation and emulator-route comparisons remain open.

- 2026-09-06: FM Towns Game handoff binds original G0487 from the verified
  CHTWE/CHTWJ executable spans, decoding all 29 little-endian spell records
  and retaining source offset/hash. Original-media tests independently
  compare every field, confirm Zokathra object/map descriptors and reject
  a RAM-mutated spell byte. No game data is extracted or compiled into a
  substitute table. Production build and EN/JP media tests pass (23.45s).
  Original cast execution and other editions' table admission remain open.

- 2026-09-06: DM1/CSB spell panel close/open no longer writes a cleared
  host buffer into the champion's paid incantation. F0394 clears selection
  and display only; reopening restores champion-owned symbols/step. CSB
  resolves its caster from runtime state. DM1 DOS/F20 original-asset RAM
  fixtures preserve a non-leader's runes/champion/mana through public
  reopen/reselection; F31 original-party tests additionally compare the
  black closed panel and unchanged source RNG across EN/JP and all three
  modes. Production build, seven regressions (36.71s, no skips) and four
  additional door-spell/Nexus/input/skill regressions (1.49s) pass.
  This is not an original-emulator timing or successful CSB cast claim.

- 2026-09-06: Removed the unused false Zokathra fireball helper, header
  and test target; corrected its direct documentation claims against
  MENU.C:1994-2027. Genuine DM1 object-creation tests remain. Added the
  original CSB cast transaction contract, separating retail edition tables
  from CSBWin-only parsers/DSA abort semantics. F0394's runtime selector now
  accepts -1 without changing champion runes, mana or leader; focused RAM
  contract tests cover clear/no-op/reselect and invalid negative indices.
  Production build and four runtime/input/media regressions pass (30.93s).
  This does not implement the remaining original CSB cast executor.

- 2026-09-06: CSB FM Towns spell-area rendering consumes original C009
  87x25 and M653, with the C696 Japanese eight-pixel vertical offset also
  applied to pointer input. Caster, incantation and symbol step come from
  the source runtime; a deliberately stale host caster cannot alter pixels.
  EN/JP original-media tests cover panel borders, available/selected glyphs,
  public rune entry and recant in Original/V2.0/V2.1. Production build and
  seven combined regressions pass (26.97 seconds, no skips). Japanese-name
  system glyphs, spell execution and emulator timing parity remain open.

- 2026-09-06: CSB FM Towns idle action cells render original C042..C048
  atlas icons through a fresh source-party mirror after viewport restoration.
  EN/JP placement follows C089..C096; Japanese pointer admission uses the
  full 62-pixel cell height. Original-media tests independently compare all
  pixels of the one eligible original cell and click its lower edge in each
  language/mode (six cases). Production build and seven combined regressions
  pass (25.36 seconds); both final media tests also pass. Hatched-cell pixel
  comparisons, remaining cell edges and the broader F31 HUD remain open.

- 2026-09-06: Restored the shared F0386 empty-hand hatch gate using
  ACTIDRAW.C:247-288: C201 selection does not bypass cooldown, candidate
  or resting markings. The companion state resolver also preserves the
  empty hand when applying F0330 timing. Predicate/state regressions cover
  enabled gates, expiry and invalid/dead champions. Production build,
  seven combined DM1/CSB regressions and three focused checks pass.
  These checks do not establish emulator pixel parity or restore the
  still-missing CSB FM Towns idle-icon compositor.

- 2026-09-06: CSB FM Towns closed-inventory movement panels consume cached
  original C013 with EN/JP C009 placement and Japanese source clipping.
  Full panel comparisons pass in both languages across Original/V2.0/V2.1.
  Real-party mouse rotations exposed unsupported LEFT/RIGHT tokens in the
  shared arrow path; explicit TURN_LEFT/TURN_RIGHT fixes those failures.
  Japanese input bypasses obsolete PC arrow geometry. Source party facing
  changes correctly without changing map position. Production build and
  seven combined regressions pass (25.13 seconds, no skips).

- 2026-09-06: CSB FM Towns viewport raster, receipt hash and HUD restoration
  share the original C696 C007 origin: English (0,33), Japanese (0,31),
  replacing the inherited (48,33) offset. Local sprite coordinates remain
  unchanged; full-page inscriptions and C080 admission/normalization use
  the selected F31 origin. Original-media final-frame tests compare all
  224x136 pixels through the independent aperture hash in both languages
  and Original/V2.0/V2.1, including an active menu beside the viewport.
  Emulator pixel parity and authentic C080 interaction sequences remain open.
  Production build and seven targeted regressions pass (22.80 seconds,
  no skips).

- 2026-09-06: CSB FM Towns active action menus now use package-bound native
  IMG2 C010 pixels after viewport restoration, with a fresh source-party
  mirror and no fabricated DM1 receipt. English text uses original M653;
  Japanese remains background-only. Final-frame source-border comparisons
  failed in all six EN/JP presentation cases before the fix and pass in
  Original, V2.0 and V2.1 afterward. Broader HUD admission, viewport origin
  and Japanese text remain open; this is not full menu or emulator parity.
  Production build and seven combined regressions pass without skips
  (22.72 seconds).

- 2026-09-06: DOS and English/Japanese FM Towns original-media tests now
  locate existing C04 groups through native square/thing traversal and
  source-owned map handoff. Nine positive non-leader hits across Original,
  V2.0 and V2.1 require actor-owned damage receipts and original group HP
  decreases while preserving leader/facing. No dungeon, monster or weapon
  records are generated; the RAM party is relocated and equipped from
  existing records. This is integration evidence, not an emulator trace.

- 2026-09-06: CSB FM Towns action clicks use authenticated English/Japanese
  C696 C082..C084 and C098 rectangles instead of seven-pixel text rows.
  Original MINI.DAT handoff tests verify name-band rejection, both inclusive
  Pass corners without stamina/leader changes, and first-row dispatch.
  Seven combined DM1/CSB regressions pass without skips (21.36 seconds).
  See parity-evidence/csb-fmtowns-action-regions.md for original
  hashes and region records. Menu pixel parity remains a separate gap.

- 2026-09-06: Extended the bounded RAM F0407/F0231 combat regression to a
  non-leader actor facing south while the leader and party face east.
  Surviving and fatal hits require positive damage, matching target HP
  writeback, the actor's damage receipt, unchanged leader/facing, and no
  action or skill XP awarded to the leader. Fatal and nonfatal cases
  retain equal increased actor XP (no duplicate kill reward). The old
  Fighter-only comparison missed STAB's Ninja XP; the regression now sums
  all 20 skills and requires an actual increase over the seeded baseline.
  It uses a complete four-champion formation for the non-leader case.
  The action runtime suite passes (7.65 seconds). This fixture is not original-media
  or emulator parity evidence; authentic target coverage remains open.

- 2026-09-06: Attack inspection feedback names the dispatched champion,
  not the party leader, using the pre-tick actor snapshot. A distinct-name
  non-leader regression failed before the fix and passes with original DOS
  and English/Japanese FM Towns media in Original, V2.0 and V2.1. All five
  targeted tests, including action/stamina runtime coverage, pass without
  skips (12.89 seconds). This does not establish full combat parity.

- 2026-09-06: Original-media DOS and English/Japanese FM Towns tests select
  an existing dungeon weapon and dispatch STAB/SWING through mouse input in
  Original, V2.0 and V2.1. The non-leader actor receives the action and stamina
  cost without changing the leader. Damage emission ownership is checked
  when present; these cases do not prove a successful hit against a target.
  Four targeted regression tests pass without skips (11.91 seconds).

- 2026-09-06: English FM Towns active-menu text follows F0768's original
  name/action baselines and7/12-character padding using original M653.
  Empty names remain empty rather than becoming invented EMPTY labels.
  Full87x45 panel comparisons pass for four actors and three modes with
  empty/nonempty names. Existing dungeon weapons supply one/two/three-row
  action sets:36 full English panels and36 Japanese border comparisons.
  Removed the Japanese text adapter's second synthetic-fill call from the
  live path.4/4 targeted tests pass,no skips(12.51 seconds). Japanese text
  remains unsupported by this renderer, not proven by background checks.

- 2026-09-06: Action dispatch passes its attacker explicitly in TickInput
  instead of changing the party leader. Original-media DOS/F20EN/F20JP
  PUNCH/KICK/WAR CRY row/gap tests preserve leader ownership across three
  modes; spell/XP regressions pass. Reconciled the old geometry test with
  I34's single C009 and source Pass behavior; it reopens the menu before
  testing row gaps. Expanded selection passes4/4,no skips(13.58 seconds).

- 2026-09-06: FM Towns active-menu pointers use EN/JP C098 and C082–C084
  region geometry rather than seven-pixel font rows. Original-media Pass
  tests cover both corners in three modes and JP's left-outside point,
  preserving leader/stamina/cooldowns (3/3 tests,no skips,11.17 seconds).
  Direct row-action/boundary parity remains to be tested independently.

- 2026-09-06: Removed the FM Towns live active-menu solid-colour substitute
  and icon overpaint. Original C010(EN87x45/JP96x72) now supplies the menu
  background. All actors/three modes pass right-border source-pixel checks;
  3/3 targeted original-media regressions pass (11.58 seconds,no skips).
  Full-panel/text and one/two-action coverage remain open.

- 2026-09-06: Japanese FM Towns action-icon mouse routing uses C089–C092
  at y94..155. Original-media tests cover both inclusive corners of all
  four cells in three modes, alive admission/dead rejection and unchanged
  leader ownership. Full executable build and3/3 targeted tests passed
  without skips (12.04 seconds), including movement/spell/DOS regressions.

- 2026-09-06: Japanese DM1 FM Towns movement clicks resolve the original
  six19-pixel-high regions instead of DOS coordinates. Original-media mouse
  tests turn left/right in all three source-HUD modes while preserving map
  position; spell/action pixel and DOS XP regressions also pass (3/3,
  no skips,11.02 seconds). Traversal and boundary coverage remain open.

- 2026-09-06: FM Towns idle action cells use original language-specific
  icon centering; Japanese movement graphics use the96x41 source with its
  nine-column parent clip. The Japanese message clear respects its224x33
  container instead of erasing movement controls. Full original-media cell
  and movement comparisons pass across three modes alongside spell/input
  and DOS XP regressions (3/3 tests, no skips,10.98 seconds). Pointer,
  hatching and Japanese text parity remain open.

- 2026-09-06: Japanese FM Towns spell pointer coordinates now follow C013's
  eight-pixel displacement. Caster selection visits all four absolute slots.
  Original-media DOS/EN/JP tests cover parent opening, sparse slot-3 caster
  selection, rune mana debit and recant without refund in all three source-HUD
  modes. The three targeted tests passed without skips (14.90 seconds).

- 2026-09-06: DM1 FM Towns spell rendering consumes the original 96x25
  C009 with its source-owned nine-column clip. Japanese C013 placement is
  eight pixels below English; the Japanese idle action clear no longer
  overwrites its bottom six rows. Original-media EN/JP and DOS spell-panel
  tests plus DOS XP regression passed (3/3, no skips). This establishes
  ASCII/rune panel rendering in Original/V2.0/V2.1, not Japanese text,
  action-icon/input geometry or full emulator parity.

- 2026-09-06: Simplified agent instructions, moved architecture and historical
  statistics into docs/PROJECT_GUIDE.md, corrected the reference path and
  consolidated verification/push rules while retaining data/runtime/security
  boundaries and explicit approval for releases.

- 2026-09-06: DM1 original YA protection expires on the correct live tick
  for only its recipient in twenty real-media regression cases.

- 2026-09-06: DM1 YA potions now give individual recipient shield and
  schedule C72 for that champion, verified using original potion records.

- 2026-09-06: DM1 antivenin now cancels the recipient's poison timeline
  and counter; original-media regressions preserve another poisoned champion.

- 2026-09-06: DM1 original water-flask consumption is verified across
  inventory owners, including empty-flask identity and carried weight.

- 2026-09-06: DM1 original waterskin tests additionally prove uncapped
  +800 water and rejection of empty skins for dehydrated champions.

- 2026-09-06: Original DM1 waterskins pass cross-owner drinking/depletion
  checks across five Atari/Amiga editions using normal mouse input.

- 2026-09-06: DM1 lethal poison reaches final-death cleanup through the
  normal idle update in twenty original-media regression cases.

- 2026-09-06: Twenty DM1 original-media cases verify live C75 damage
  and source-timed rescheduling while inventory remains open.

- 2026-09-06: DM1 no longer pauses idle simulation merely because its
  inventory is open; original-media tick/owner regressions pass.

- 2026-09-06: DM1 removes pending poison events when their champion dies;
  twenty original-media death regressions preserve unrelated queue entries.

- 2026-09-06: DM1 death ownership now has twenty registered original-media
  regressions across five Atari/Amiga editions, including living-caster
  preservation and final-death spell state. All passed locally.

- 2026-09-06: DM1 PC3.4 startup and 36 legacy-edition CLI/menu launches
  pass with the rebuilt application after leader-input fixes.

- 2026-09-06: DM1 newly selected leaders align with party direction;
  verified with five original Atari/Amiga input regressions.

- 2026-09-06: Revalidated all six DM1 original-media object/scroll corpus
  tests after the leader-selection fixes; all passed without skips.

- 2026-09-06: DM1 keyboard/mouse leader selection rejects zero-health
  champions; all five original Atari/Amiga load regressions pass.

- 2026-09-06: Restored DM1 source-layout name-click leader selection;
  five original Atari/Amiga tests verify press/release weight ownership.

- 2026-09-06: Fixed delayed DM1 held-weight transfer on leader selection;
  two-way keyboard cycling passes five original Atari/Amiga regressions.

- 2026-09-06: Five original DM1 Atari/Amiga load tests verify that a
  leader's floor drop preserves a second champion's carried object/load.

- 2026-09-06: Fixed stale DM1 carried load after a floor drop; five original
  Atari/Amiga load regressions now pass. Full encumbrance parity remains open.

- 2026-09-06: Five original DM1 Atari/Amiga editions pass occupied
  action-hand exchanges in both directions for all 606 allocated objects
  in Original/V2.1, retaining both Thing identities after mouse release.

- 2026-09-06: Legacy DM1 equipment tests independently decode F0141
  object-info indices from normalized dungeon bytes before selecting
  original G0237 masks. All five edition tests pass.

- 2026-09-06: Five original DM1 Atari/Amiga editions pass 307,900
  placement, pickup and rejection checks across all 30 inventory slots
  in Original/V2.1, using original G0237 masks and source G0038 rules.

- 2026-09-06: DM1 Atari/Amiga runtime equipment masks match original
  graphic-559 G0237 words for 606 allocated objects in each of five
  editions; all source-mask and transfer checks pass.

- 2026-09-06: Atari/Amiga real-media verification skips only absent paths;
  archive-open errors now fail. Negative checks confirm ENOENT returns 77
  and ENOTDIR returns 1 for both test binaries.

- 2026-09-06: Five original DM1 Atari/Amiga editions pass 230,280
  place/pickup transactions across both hands and all 17 backpack slots
  in Original/V2.1, including identity checks after mouse release.

- 2026-09-06: Original DM1 Atari/Amiga scrolls now exercise action-hand
  placement and pickup, checking held/slot identity before and after mouse
  release in five editions and Original/V2.1; all tests pass.

- 2026-09-06: Original C033 border pixels now verify all 30 DM1 inventory
  slots across five Atari/Amiga editions in Original/V2.1; all tests pass.

- 2026-09-06: Fixed DM1 Atari/Amiga inventory admission of original padded
  C033 graphics and scroll baseline conversion. Five real-media editions
  pass 350 scroll/mode raster checks in Original/V2.1.

- 2026-09-06: Corrected DM1 PC3.4 scroll text position from original C696
  and F0341/F0644 evidence. All 35 original scrolls now pass the corrected
  raster oracle in Original/V2.1; other media coordinates are unchanged.

- 2026-09-06: Fixed DM1 scroll cells to use the original six-column white
  background; three original-scroll raster failures are resolved and the
  complete PC3.4 corpus passes in Original/V2.1.

- 2026-09-06: DM1 scroll raster evidence now explicitly requires loaded
  font data and nonzero glyph ink; the original-media corpus passes.

- 2026-09-06: DM1 original-scroll tests now compare panel/text raster
  placement in Original/V2.1 using original C023 and M653 materials;
  shared layout/font decoding and transparent backgrounds remain separate.

- 2026-09-06: DM1 PC3.4 scroll eye tests now check original C023 border
  pixels in Original/V2.1; the complete original-object corpus passes.

- 2026-09-06: DM1 original-scroll eye checks now assert source text equality
  against each scroll's C02 reference, not merely successful panel routing.

- 2026-09-05: DM1 legacy original-media launch matrix now covers V2.0 as
  well as Original/V2.1: all 36 CLI/menu launches across six editions pass.

- 2026-09-05: DM1 JDM title verification distinguishes absent optional media
  from invalid supplied media; positive, failure and skip paths are checked.

- 2026-09-05: DM1 FM Towns original EN/JP launch/input matrix passes;
  Japanese title-receipt assertions remain active in Release test builds,
  and unavailable media no longer counts as a successful title test.

- 2026-09-05: DM1 PC3.4 passes same-chest refresh and cross-chest owner
  transitions in Original/V2.1 alongside the full original-object corpus.

- 2026-09-05: Rebuilt Firestaff passes Atari/Amiga CSB CLI startup; original
  Amiga ZIP/ADF C025 admission now verifies exact dimensions and pixel range.

- 2026-09-05: Atari CSB chest comparisons now include all 10,512 panel-area
  pixels and the 848 transparency-key positions; three presentation modes
  pass against original materials and the pre-open viewport.

- 2026-09-05: Atari CSB chest composition now uses C025 and original icon
  atlas crops. Source-material pixel checks pass in Original, V2.0 and V2.1;
  transparency-background and emulator parity are not yet established.

- 2026-09-05: Original Atari CSB chest pickups pass in three presentation
  modes using C232-relative native input geometry. Chest rendering remains
  a separate open requirement.

- 2026-09-05: Atari CSB passes 22,797 original-object backpack drags across
  Original, V2.0 and V2.1, checking source/destination/leader-hand ownership.

- 2026-09-05: Atari CSB passes 40,230 occupied-slot exchange/rejection checks
  using distinct original objects and original C559 acceptance masks, across
  Original, V2.0 and V2.1, in addition to empty-slot checks.

- 2026-09-05: The independent original C559 inventory oracle also passes in
  V2.0 and V2.1. Rebuilt Firestaff passes the original Atari STX CLI startup
  and scripted native runtime-input regression after the mouse-release fix.

- 2026-09-05: Independently verified Atari CSB allowed-slot values for all
  447 allocated objects against original C559 bytes; Original mode passes
  the full 30-slot input sweep using those bytes as the acceptance oracle.

- 2026-09-05: Atari CSB original-object input coverage now spans all 30
  inventory slots, including equipment rejection: 40,230 checks pass across
  Original, V2.0 and V2.1. Independent object-mask decoding remains separate.

- 2026-09-05: Original Atari CSB backpack verification now covers 447
  allocated original objects across 17 slots in three presentation modes;
  all 22,797 pickup/replacement roundtrips pass.

- 2026-09-05: Fixed an original-media Atari CSB inventory regression where
  releasing the mouse undid pickup. The new original-weapon roundtrip failed
  before the fix and passes in Original, V2.0 and V2.1 afterward.

- 2026-09-05: Hardened the CSB M11 HUD regression: failure to start selected
  media is now a test failure, not a successful skip. Invalid presentation
  selection uses CTest's skip code. Original Atari MINI.DAT passes the
  rebuilt test in Original, V2.0 and V2.1.

- 2026-09-05: Original CSB FM Towns EN/JP tests also verify reopening an
  already-open chest with a hole and switching chest owners while holding
  an original resident. Both container lists and the held item survive in
  Original and V2.1; save/resume is outside this interaction check.

Reviewed 2026-08-25. This ledger contains completed, evidence-backed work
only. Active work is in `TODO.md` and `TODO-<game>.md`.

- 2026-09-05: Rebuilt Firestaff after the shared CSB chest-slot change and
  passed six original-media regressions: DM1 object names/full inventory
  corpus plus CSB Atari ST, Amiga, FM Towns English and Japanese startup
  paths. Startup success does not establish Atari/Amiga chest-click parity.

- 2026-09-05: CSB M11 now retains open G0425 slot positions while keeping
  runtime-linked container contents synchronized. Original F31 EN/JP tests
  pass pickup, same-slot release, permitted replacement and close-order
  checks for all 60 residents in Original and V2.1. Pre-placed equipment
  lacking the container mask remains correctly rejected on reinsertion;
  only those rejected test placements are reset. The eye-close regression
  also passes. Atari/Amiga geometry and native save-resume remain separate.

- 2026-09-05: Fixed CSB FM Towns chest pointer admission. Boot retains
  C537..C544 from the original item-696 C106/C101/C100 graph alongside the
  30 inventory boxes; input admits chest children only for an open chest,
  and same-slot release does not repeat the exchange. Both original EN/JP
  tests pick up all 60 residents across 14 containers with independent chain
  restoration. Continuous replacement and open-slot persistence remain open.

- 2026-09-05: The CSB FM Towns original-media M11 test now traverses
  GAME/Enter into the original MINI.DAT party/dungeon before inventory
  inspection. Both English and Japanese pass with one original champion,
  14 readable containers and 60 visible residents. This establishes media
  ownership for subsequent chest input tests, not slot-persistence parity.

- 2026-09-05: Rebuilt the Firestaff application with the live chest fixes.
  The original English DOS archive CLI/menu regression passes, including
  Original, V2.0 and V2.1 launch modes. Unreleased notes now distinguish the
  verified interaction repair from the deferred changed-dungeon save gap.

- 2026-09-05: Fixed two live DM1 chest interaction faults: same-slot release
  now resolves C101/G0456 without repeating the press exchange, and open
  G0425 slots retain holes until F0334 relinks on close. The original PC3.4
  corpus preserves all 43 chest residents through pickup/replacement/close
  in Original and V2.1, and the full 611-record inventory matrix passes.
  Eye-close and HoC regressions pass; snapshot preparation closes the chest
  and publishes its current chain. Full changed-dungeon save persistence
  remains an explicitly deferred, separately reproduced limitation.

- 2026-09-05: Original PC3.4 scroll records now exercise the live inventory
  eye route in Original and V2.1 after the full slot matrix. Each opens its
  own scroll panel, not a generic object dialog, renders through M11 and
  retains the held Thing after release. The combined corpus passes; this
  checks routing and ownership, not text/pixel equality with an emulator.

- 2026-09-05: The 611-record PC3.4 inventory matrix additionally seeds a
  distinct, slot-admissible original resident in every slot. Original and
  V2.1 swaps preserve both Thing identities in both directions; denied
  incoming objects preserve the resident and held Thing across press/release.
  The combined empty/occupied 30-slot matrix passes. This uses controlled
  in-memory placements, not a proof of floor/chest ownership or all item pairs.

- 2026-09-05: Expanded the 611-record PC3.4 mouse corpus to all 30 inventory
  slots in Original and V2.1 (36,660 object/slot/mode combinations). Source
  G0038 masks determine admission; permitted placements roundtrip and denied
  placements preserve the held Thing across press/release. All checks pass.
  Occupied-slot swaps and chest contents remain separate unfinished coverage.

- 2026-09-05: The original PC3.4 object corpus now exercises live action-hand
  placement and retrieval through mouse press/release for all 611 decoded
  weapon, armour, scroll, potion, container and junk records in Original and
  V2.1. All roundtrips preserve Thing identity and single-exchange ownership.
  Existing name/icon checks remain; no replacement game records are created.
  Missing media is explicitly skipped. This does not cover every inventory
  slot, chest interaction, or emulator-rendered pixel equivalence.

- 2026-09-05: The original PC3.4 archive's live HoC pointer sweep now runs
  in both Original and V2.1, requiring all 24 candidates rather than any
  nonempty subset. Both modes select all 24 through the rendered input path;
  the existing resurrection/reincarnation regression and side/depth viewport
  material sweep pass. The test now honors TMPDIR instead of hardcoding a
  temporary location. This does not establish emulator pixel parity.

- 2026-09-05: Both optional real-media HoC mirror tests now return CTest's
  explicit skip code when no data directory is selected, rather than a false
  pass. With the existing French DOS original files selected, the directional
  test checks 24 sensors and 24 distinct portraits; the material test also
  passes. These are sensor/material-plan checks, not rendered-pixel parity.

- 2026-09-05: CSB Amiga and FM Towns EN/JP runtime-transition tests now
  explicitly cover V1 and V2.1. Both modes retain Amiga's first UP movement
  and the Towns original MINI.DAT map/party seed through Game/Enter input.
  All three original-media scripts pass; complete rendering parity is open.

- 2026-09-05: CSB Atari ST original STX regression explicitly checks both
  V1 and V2.1 through CLI and menu to a loaded dungeon, retaining the
  requested presentation mode. Existing title, input and pointer-launch
  assertions pass in the same run; audiovisual parity remains separate.

- 2026-09-05: Atari audio rejection clears the previous accepted flag,
  hash, period and sample count. Regression tests cover invalid fingerprints,
  short SND1 streams and recovery on the next valid request; original DM1
  Atari EN/DE/FR audio corpus tests continue to pass.

- 2026-09-05: Original/Modern CLI/menu coverage extends to original Atari
  DE/FR and FM Towns JP (12 additional combinations). Japanese launches
  must retain both JDATA and JDM fingerprints, preventing an English
  fallback from satisfying the presentation-mode regression.

- 2026-09-05: The native Paula-volume PCM entry point supports all 0..64
  levels instead of rejecting everything except 64. Tests lock all 65 gains,
  including silence, without changing sample cadence; original DM1 Amiga
  2.0/HD transport regressions pass. Asymmetric stereo remains unimplemented.

- 2026-09-05: DM1 Amiga uses an explicit MEDIA413 sound table instead of
  deriving periods from the PC table. Water elemental attack record 571
  now uses source period 138 rather than PC's 112. Original 2.0/HD corpus
  checks validate the corrected sample cadence across all engine events.

- 2026-09-05: DM1 original Atari English, Amiga 2.0 and FM Towns English
  media pass explicit V1/V2.1 launches through CLI and menu (12 combinations).
  The new real-media tests check selected mode and loaded runtime rather
  than accepting a default-mode launch as evidence for both presentations.

- 2026-09-05: Original DOS DM1 archive boot checks now explicitly select
  V1, V2.0 and V2.1 through both CLI and menu, asserting the resulting mode,
  original graphics fingerprint and loaded runtime. This closes a test gap
  where a successful default-mode launch did not prove mode selection;
  it does not establish filter/rendering parity or other-platform coverage.

- 2026-09-05: Rebuilt CSB startup regressions pass with original Atari STX,
  Amiga and FM Towns EN/JP media after the audio fixes. These cover their
  existing CLI/runtime and, where included, menu/input assertions; they do
  not establish complete gameplay or emulator audiovisual parity.

- 2026-09-05: The data-free Amiga audio regression now locks byte-sample
  cadence and signed amplitude timing, catching the previous extra clock
  division independently of availability of original game media.

- 2026-09-05: Rebuilt CLI/menu/input regressions pass after the legacy audio
  changes: original DM1 DOS English, Atari EN/DE/FR, Amiga 2.0/HD and FM
  Towns EN/JP. The Towns check independently reloads both language programs
  for all seven directional/action commands. These are startup/input checks,
  not emulator pixel, audio-waveform or complete gameplay parity proofs.

- 2026-09-05: Legacy DM1 startup clears the PC SND3 bank without attempting
  to parse Atari/Amiga/FM Towns media as PC audio. Original-media startup
  tests assert an empty PC bank for these editions; the PC3.4 regression
  continues to require all original SND3 entries.

- 2026-09-05: F31 EN/JP audio corpus tests independently locate each of
  the 35 selected records in the original container and compare payload
  bytes. Oversized sample counts in private RAM copies are rejected for
  every event with empty output; original archives are never modified.

- 2026-09-05: CSB FM Towns reads the BE16 PCM sample count without requiring
  exactly two unused tail bytes. Original explosion record 675 has 3970
  samples in 3973 bytes and was incorrectly rejected. EN/JP original-media
  tests now cover all 35 sound events and compare every host output sample;
  declared samples must still fit entirely within the source record.

- 2026-09-05: DM1 Amiga local sound events select original PCM records,
  skipping the two-byte header as SOUND.C requires. Original 2.0/HD tests
  compare every resampled byte value and period across all 35 engine events.
  The common Amiga PCM transport no longer incorrectly halves the audio
  clock a second time. The current clock is NTSC; PAL selection, stereo
  attenuation and channel arbitration are not covered by this change.

- 2026-09-05: DM1 FM Towns local effects use retained F20 unsigned PCM,
  not the PC SND3 bank or F31 signed bytes. DATA.C MEDIA507 selects the
  22 original records; TOWNSIO.C supplies BE16 length, the 31936-sample
  limit and 5500 Hz cadence. EN/JP original-media checks compare every
  host output sample to the selected source record. No BIOS or extraction
  is required; original channel/timing/distance parity remains open.

- 2026-09-05: CSB FM Towns PCM host gain uses the original 1..127 driver
  domain instead of saturating it with PC's 1..3 divisor. Original EN/JP
  archive tests cover all 127 gain steps; direct local effects request 127.
  This verifies transport scaling, not distance-event or emulator parity.

- 2026-09-05: DM1 Atari gameplay dispatch selects original SND1/PSG instead
  of the PC SND3 bank. ReDMCSB event-index translation preserves missing
  Atari effects as silence and the entrance Timer-A period as 145. Original
  EN/DE/FR transport checks cover every engine sound index and reject the
  three known short streams without generated markers. Original-emulator
  timing, arbitration and electrical-output parity remain unproven.

- 2026-09-05: Original Atari EN/DE/FR audio characterization verifies all
  22 transport payloads byte-for-byte against the DM1 graphics reader.
  Nineteen decode within their record boundaries; three known short streams
  are explicitly tested for safe rejection. This is not playback parity.

- 2026-09-05: Rebuilt post-M653-fix startup regressions pass on original
  Amiga 2.0/HD and FM Towns EN/JP media, including CLI/menu handoffs and
  the English/Japanese input matrices. The legacy corpus test additionally
  decodes all 532 admitted images for each FM Towns language and Amiga 2.0.

- 2026-09-05: DM1 legacy/Atari startup binds the raw original M653 font from
  retained media bytes instead of attempting the PC3.4 file-state loader.
  All 768 bytes match original Atari EN/DE/FR, Amiga 2.0/HD and FM Towns JP
  records; the PC3.4 object/pickup regression also passes. FM Towns system
  Kanji glyphs are a separate route and are not supplied by this M653 fix.

- 2026-09-05: The Atari bitmap API now enforces the same source-record
  classification as the production asset loader. Text, sound, font and code
  records remain available through raw reads but cannot enter raster decode.
  EN/DE/FR original-media checks and the Atari container unit test pass.

- 2026-09-05: The Atari original-media gate now raster-decodes every one of
  the 532 admitted image records and checks every output pixel is 4bpp.
  English 1.2, German 1.2 and French 1.3 all pass, in addition to their 563
  raw-record and 199-name checks. This is decoder coverage, not a same-state
  original framebuffer comparison or proof of viewport composition.

- 2026-09-05: Amiga M564 regression now uses M12's authenticated edition
  selection and native archive handoff. All 199 original object-name indices
  pass for the 2.0 and HD ZIP→ZIP→ADF packages. This validates name bytes,
  not inventory glyph pixels or the outstanding gameplay palette capture.

- 2026-09-05: Extended the Atari name/all-record gate to hash-selected German
  1.2 and French 1.3 original disk containers. Both verify all 199 names and
  563 expanded lengths, alongside English 1.2. Rebuilt native CLI/menu/input
  regression scripts pass for all three editions after the decoder change.

- 2026-09-05: DM1 Atari raw graphics now use ReDMCSB F0497's dictionary
  convention and F0496 repetition output instead of the incompatible generic
  LZW end-code route. M564 binds from the retained original Atari bytes.
  The authentic English 1.2 archive verifies all 199 object-name indices and
  all 563 expanded record lengths; Atari container and STX unit tests pass.
  Expanded-length checks are not a pixel-parity claim.

- 2026-09-05: CSB's public hand-name accessor preserves UTF-8 boundaries
  when copying the already-localized cached label into a smaller UI buffer.
  The hand/no-DM1-fallback regression passes with additional empty-output,
  exact-fit multibyte-character and buffer-guard checks.

- 2026-09-05: DM1 translated/Japanese object labels clip only at complete
  UTF-8 boundaries after full-source catalog lookup. The real JDATA first
  weapon hand-label test checks every output capacity and its guard byte;
  all original Japanese names/actions and the PC3.4 pickup/cursor regression
  pass. Original non-UTF-8 fallback labels retain their existing byte encoding.

- 2026-09-05: DM1 Japanese FM Towns actions now consume the authenticated
  JDM.EXP load-image pool at `0x243bc`, instead of falling through to PC3.4
  English action names. The receipt retains original CP932 bytes; M11 converts
  to UTF-8 at catalog lookup. All 44 names match the reviewed JDM pool, all
  199 real Japanese object-name checks pass, and the original-disc English
  startup/menu-owner regression passes. This does not establish glyph parity.

- 2026-09-05: Fixed DM1 legacy M564 binding to read the already-owned original
  GRAPHICS bytes instead of reopening a diagnostic display path through the
  PC3.4 container decoder. Authenticated Japanese FM Towns now uses F0031's
  NUL framing and native CP932-to-UTF-8 keys with expansion capacity and
  overflow rejection. All 199 JDATA names match their original indices;
  real PC3.4 hand/pickup tests and both-endian raw-record bounds tests pass.

- 2026-09-05: Corrected the historical pass627 capture guidance: accept an
  authenticated same-state original reference independently of whether its
  pixels match Firestaff. Removed the unsupported assertion that no renderer
  changes could be needed. Source review also identified the open F20J M564
  name-framing defect, now recorded in the DM1 work list.

- 2026-09-05: Original-media startup/input regressions pass for DM1 DOS 3.4,
  Atari EN/DE/FR, Amiga 2.0/HD, FM Towns EN/JP, and CSB Atari/Amiga/FM Towns
  English. The FM Towns DM1 gate now independently checks all seven input
  commands against Japanese JDM/graphics fingerprints as well as English EDM.
  These are native boot and bounded input checks, not original pixel parity.

- 2026-09-05: Added 216 authentic FM Towns Japanese CSB catalog keys to
  every CSB locale, including 39 reviewed Swedish action translations.
  All 218 extracted keys pass the native PO loader lookup/fallback check;
  catalog regeneration and completion statistics are current.

- 2026-09-05: CSB object-name presentation obtains the complete original
  name before converting its encoding and looking it up in the catalog.
  Small UI buffers can no longer change the lookup key or split a UTF-8
  character during final clipping. The native engine builds and Japanese
  FM Towns CLI/menu startup passes with the original CD archive.

- 2026-09-05: Native CP932 conversion now binds CSB Japanese object/action
  names to UTF-8 catalog keys. The same decoder serves the source extractor,
  removing its iconv dependency. All 65,792 one/two-byte inputs match Python's
  standard CP932 codec; buffer/error tests pass, and the 218-message original
  Japanese FM Towns corpus is byte-identical to the prior iconv extraction.

- 2026-09-05: CSB source-text extraction now propagates CP932 conversion
  failure instead of silently omitting an entry and reporting success. Its
  UTF-8 buffer covers the runtime text bound's worst-case expansion. The
  authenticated FM Towns Japanese corpus remains byte-identical after the
  change, preserving the 218-message extraction evidence.

- 2026-09-05: The optional CSB source-text extraction tool now links CMake's
  Iconv target explicitly, resolving the missing macOS iconv symbols. Its
  Linux ARM64 build passes. Runtime game targets do not depend on Iconv.

- 2026-09-05: Savegame Editor bundling selects Python before compiling
  gettext catalogs. The complete Linux ARM64 bundle and its self-test pass.
  Translation calls outside f-string expressions preserve extraction with
  older gettext versions; the current full catalog check passes.
- 2026-09-05: Standalone dungeon-loader tests link the authentic FM Towns
  receipt implementation. The creature-map test and all 28 scroll text
  assertions pass locally after the CI linker failure was reproduced.

- DM1 D3--D1 ordinary explosion rendering now follows F0115 call ownership:
  every MAIN/DOORPASS callback restarts its C15 list after packed-cell
  material, so door rear explosions render before F0111 and front-pass
  explosions render afterward. PC 3.4 item-696 C3014/C3031 anchors clip real
  GRAPHICS.DAT material to the 224x136 viewport; the global replay is removed.

- DM1 F0124's post-transaction Thieves Eye D1C wall restore now executes
  through a dedicated D1C square-tail scheduler callback between D1C and
  F0125. This covers wall routes that correctly have no F0115 step and removes
  the last direct restore call outside the verified callback stream.

- DM1 F0125/F0126 now rasterize every ordinary D0L/D0R C15 record inside
  the owning F0115 callback. The runtime receipt retains source cell and
  centered state, rotates non-centered cells relative to party direction,
  and places real F0114 material at the PC 3.4 item-696 C3014/C3031 anchors;
  unsupported back-cell projections and missing source graphics fail closed.

- DM1 F0128 now consumes F0125/F0126 D0L/D0R F0115 creature material from
  authentic G2033 rows 11/12 and each following F0113 teleporter field in the
  same scheduler callback stream. Negative G2028 continues to suppress side
  items/projectiles; no host placement or substitute bitmap was introduced.

- DM1 F0128 now consumes F0127's complete D0C `F0115_MAIN` transaction from
  its verified `C0x0021` callback step. Real PC 3.4 floor-item, projectile and
  restarted C15 explosion consumers all finish before F0113; the former direct
  item/projectile calls and source-invalid post-field explosion call are gone.

- DM1 F0128 now invokes the real PC 3.4 D1C Hall-of-Champions C346/C026
  mirror transaction from its owning F0107 scheduler callback. The separate
  post-F0107 mirror call was deleted, while the existing fail-closed C127,
  backing and portrait receipts remain the only material authority.

- DM1 F0128 F0104 door frames now rasterize in their own scheduler callback
  phase before optional F0110 and F0111. Centre and side F0111 helpers no
  longer duplicate frame pixels; open-door and D3L2/D3R2 frames use the same
  original GRAPHICS.DAT-backed plans.

- DM1 F0128 F0110 door buttons now have one callback owner at their exact
  source boundary between frame and F0111. The scheduler emits F0110 only for
  F0117 D3R and centre F0118/F0121/F0124; the direct D3R and centre replay was
  deleted. A real PC 3.4 HoC door proves F0108, DOORPASS1, F0110 and F0111
  callback receipts without modifying the dungeon or extracting its media.

- DM1 F0128's complete D3--D1 `F0104_WALL_MATERIAL` family now rasterizes
  through the verified per-square callback at the owning outer, side or centre
  square. The three later wall replay call sites were deleted; all pixels still
  come from the mounted PC 3.4 `GRAPHICS.DAT` wall consumers.

- DM1 F0128's D3--D1 foreground tail is now scheduler-callback owned. For
  each exact target square the verified plan dispatches F0104 pit/stairs,
  F0108 floor ornament, F0112 ceiling pit, F0115 `MAIN`/`DOORPASS2`, and
  F0113 field operations after that square's F0111 door boundary. The former
  hand-written per-square span loop was deleted, leaving one operation owner
  and preserving `DOORPASS1 -> F0111 -> DOORPASS2`. A new receipt is asserted
  by the authentic PC 3.4 HoC ZIP regression; the native ZIP CLI boot and the
  asset-free no-fallback scheduler gate also pass.

- DM1 F0128 now rasterizes the terminal F0125/F0126/F0127 D0
  F0104/F0112/F0113 primitives from the verified scheduler execute callback;
  the former per-square D0 replay owner was removed. F0127 remains split at
  its authentic boundary so D0C floor items/projectiles precede its F0113
  field overlay. A frame-local receipt counts callback-owned source steps, and
  the real in-memory PC 3.4 HoC regression proves that the mounted archive
  executes this path while retaining four-direction, F0115 and mirror/HUD
  behavior.

- DM1 F0128's complete pre-F0111 F0115 `DOORPASS1` family now rasterizes
  from the verified scheduler execute callback at each existing square-local
  door boundary. The hand-written span scan was deleted; the callback consumes
  the scheduler's exact square and cell-order word for center, side and D3
  outer lanes. A callback receipt and a real PC 3.4 ZIP dungeon-door probe
  prove that an authentic door produces exactly one callback-owned rear
  partition before its F0111 panel.

- DM1 PC 3.4 HoC launch-to-mirror production route is covered without a test
  teleport. The real archive supplies the collision graph and C127 owner; BFS
  derives a route from retail tuple `(map 0, 1,3,South)`, public M11 input
  replays it, then the test draws and clicks the published C026 rectangle and
  completes C040/C160 through the pointer owner. This proves the earlier
  zero-valued entrance summary fields are not a gameplay-path failure.

- DM1 FM Towns native title cadence now follows the source-owned EDM.EXP
  schedule: only the 18 zoom frames wait one 60 Hz VBlank, followed by the
  separate two-VBlank return guard. Cumulative 16/17 ms host delays produce
  exactly 300 ms for the zoom and 333 ms overall instead of the former 374 ms
  drift. `dm1_v1_fmtowns_title` and the authentic in-memory FM Towns ZIP
  CLI/menu/TMENU-to-EDM/CDDA/input test both pass.
- DM1 ReDMCSB `SPELFAIL.C:F0410` failures now publish through the real
  `TEXT.C:F0051/F0047` C015 message state instead of remaining host telemetry.
  The source fragments are appended separately, use C04 cyan, synchronize the
  70-tick expiry to the cast tick, and retain exact practice, meaningless-spell
  and empty-flask text. The focused text-state and authenticated M653 C015
  consumer gates pass. `MENU.C:F0381` now also owns the visible FLIP heads/
  tails result, including the source newline, bounded `@p` replacement, C04
  colour and 70-tick lifetime; malformed replacement tokens fail closed.
- Normal play now loads all five isolated game PO domains (`dm1`, `csb`, `dm2`,
  `nexus`, `theron`) in addition to the startup-menu domain, using the language
  selected in the launcher instead of incorrectly re-reading only the host AUTO
  locale. DM1 retail-derived object/action names, sensor/timeline messages and
  scroll text are resolved only at their final presentation boundary, leaving
  original data untouched and using the exact decoded source string as fallback.
  F0410 uses portable named fields so translations can reorder champion and
  skill names; a focused real-catalog gate verifies lookup, expansion and exact
  fallback, while the original English C015 source-lock remains green.
- DM1 C015 storage/wrapping now advances by Unicode codepoint rather than UTF-8
  byte, and its Original renderer keeps M653 for ASCII while drawing supported
  Latin-extended translated glyphs through the built-in Unicode table. Unknown
  codepoints produce one replacement glyph rather than corrupt M653 atlas reads.
- CSB's synthetic, unwired 33-key catalog—including the nonexistent `CSB PC
  3.4` save/edition label—was replaced by 220 unique C699/M564 msgids extracted
  in memory from the supplied original Amiga archive. The extraction tool
  records the admitted GRAPHICS/DUNGEON hashes, rejects media without both
  authenticated tables, emits valid POT, and never writes extracted game data.
  Exact matching DM1 translations seed CSB locale entries without cross-domain
  runtime fallback; every unmatched entry retains its source-text fallback.
- CSB FM Towns source-text extraction now follows the explicit CHTWE/CHTWJ
  launch selection and reads M564 item 694 plus executable-owned DYNA_BUTTONS
  directly from retained CD members. English and Japanese both produce
  `msgfmt`-valid UTF-8 POT output; F31J follows its source Shift-JIS NUL/byte-1
  object-name rule instead of treating Japanese high bytes as row delimiters.
- DM1 PC 3.4 inventory source-lock verification now reads the canonical
  `GRAPHICS.DAT` member directly from the original ZIP, with no extracted
  test copy. Mouth-consumable fixtures carry matching raw C08/C10 records and
  the chest/scroll fixture carries a byte-accurate raw C09/C10 chain, so the
  hardened runtime cannot silently fall back to decoded test structs. The
  focused pickup/inventory/eye/mouth/drop group passes 15/15, including the
  formerly skipped real-media F0731/F0734 material gate.
- Native M12/M11 launcher and boot-probe infrastructure is in production and
  accepts original data without runtime emulator dependencies.
- Real-media focused regressions cover DM1 PC 3.4, DM2 FM Towns, CSB Atari
  STX, Theron JP CUE startup, and Nexus title-resource intake.
- CI enforces media hygiene, native builds and deterministic checks on Linux,
  macOS and Windows.
- DM1/CSB F0128 D3L2/D3R2 door composition consumes the authenticated rear
  Thing pass before the F0676/F0677 door occluder and the front Thing pass
  afterward; focused DM1, CSB and live HoC render gates pass.
- CSB product support and documentation expose only the original Atari ST,
  Amiga and FM Towns editions. Retired PC-labelled probes, targets and
  documentation rows are removed; the complete CSB-labelled regression was
  green (155/155) before removal of the redundant negative PC test, and the
  resulting focused launcher/runtime suite is green (8/8).
- High-resolution nearest-neighbour RGBA expansion now uses exact quotient /
  remainder coordinate stepping instead of per-pixel integer division. The
  output mapping matched the former formula over 1,974,784 tested coordinates;
  a 320x200-to-3840x2160 arithmetic microbenchmark improved by 1.38x, while
  a per-frame 256-entry RGBA lookup table also removes repeated palette
  resolution from every expanded output pixel. The DM1/CSB filter, resolution
  selector and runtime-popup tests remain green.
- CSB FM Towns no longer carries SWITCHTW's C26 menu palette into Entrance.
  The native handoff locates and admits C28_ENTRANCE_CSB (G8174) inside each
  already hash-verified CHTWE/CHTWJ executable and presents those six-bit DAC
  values. The supplied ZIP proves the real offsets as `0x35898` (English) and
  `0x35a78` (Japanese); both archive tests and the English SWITCHTW-to-C004 M11
  route pass without extracting game files.
- CSB FM Towns now also releases temporary C28 after the Prison doors and
  selects the authentic C00_LIGHT0--C05_LIGHT5 dungeon palette. All six
  contiguous G8151--G8156 COLOR_DEF rows are admitted from the same verified
  CHTWE/CHTWJ executable; real offsets are `0x35494` (English) and `0x35674`
  (Japanese). Both full M11 language routes prove that the first live dungeon
  frame publishes a LIGHT palette rather than retaining C28.
# CSB Atari source text receipt

- Bound the supplied S21E STX directly to its authentic 563-item DMCSB1
  GRAPHICS.DAT and decoded the 1848-byte item-556 M564 stream in memory. All
  199 source object-name rows authenticate. F0913 now decodes the complete
  145,418-byte START.PAK body without over-reading fourteen words, while the
  44-row G0490 action table is uniquely admitted from GRAPHICS.DAT C560 item
  560 at offset 0x174 as required by STARTUP2 F0750. The resulting 221 unique
  player strings come only from the selected STX; no extracted game-data file
  or cross-platform name table is used.
  The live Atari boot/runtime now uses that native IMG/LZW M564/C560 path as
  well: BLOCK, FUSE and object names survive STX boot-to-runtime without the
  invalid PC record-694/699 decoder or compiled DM1 fallbacks.
  FM Towns English and Japanese likewise bind live M564 from retained packed
  CDATA/CJDATA GRAPHICS.DAT item 694 and G0490 from hash-admitted CHTWE/CHTWJ
  DYNA_BUTTONS at `0x29f50`/`0x2a0ec`. All 44 rows and N/X sentinels are
  validated before use, with the edition's English high-bit or Shift-JIS
  decoder and no loose-file, PC-record or cross-language fallback.
  Amiga A31M proves the corresponding big-endian DMCSB2 M564/C699 binding
  through the real ZIP-to-ADF M12/TITL/APPB/KAOS/M11 handoff. A31E/A35E,
  where G0490 is compiled rather than a GRAPHICS.DAT record, accept it only
  from the exact hash-verified APPB.FTL in the same admitted disk context.

- DM1 C080 leader-hand throws now preserve the original screen coordinates
  passed by CLIKVIEW.C F0377 into F0375. Viewport-origin subtraction remains
  confined to the later clickable-box loop, restoring the complete F0329 to
  F0328 throw route. F0190 death smoke also uses source-owned F0887/C15 in a
  loaded real dungeon and a bounded F0821/timeline fallback only in test/probe
  worlds with no source explosion table. The action/stamina matrix improved
  from 1280/64 to 1384/0. Its stale throw, G0243 smoke, F0381 lifetime and
  F0401 fear-delay oracles were corrected directly from source arithmetic.
  CLIMB DOWN now applies MOVESENS.C's rope stamina cost without generic pit
  damage. Projectile F0213 impacts now publish their authentic C25 lifecycle,
  including real C15 ownership where available; poison C25/C75 ordering and
  harm-non-material killed-all behavior are source-verified. Nine focused
  source/raw-data regressions pass.
