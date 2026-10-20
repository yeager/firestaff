# Firestaff TODO — active cross-game work

2026-10-20 DM2 CUE image reuse: the explicit ZIP reader now shares one
cue-selected original image between the Macintosh HFS probe and the FM Towns
disc probe. The pre-change sample showed separate full ZIP inflations beneath
`dm2_v1_mac_media_read_zip` and `dm2_v1_boot_load_fmtowns_disc_from_zip`; the
post-change sample showed a single image inflation followed by both parsers.
The authenticated installed-media scan still reported DM1, CSB and DM2 READY.
Its one warm timing changed from 12.788 s to 10.206 s, which is not enough
repeated evidence to claim a stable wall-clock gain. The original-media DM2
Mac, FM Towns and renamed-ZIP directory tests passed (575 s total). The full
launcher scan remains synchronous and still delays first menu interaction.

2026-10-20 DM2 directory ZIP scan audit: removed the second DOS-only sweep of
three canonical archive names. The earlier bounded inventory already visits
every ZIP in each scan root and its `dm2` child, and authenticates each archive
through the same exact DM2 boot-profile hash gate. The rebuilt `firestaff`
target passed, and `dm2_v1_renamed_zip_directory_real_media` passed against the
installed original archives (73.23 seconds), confirming renamed ZIP discovery
still works. This focused test does not establish a measurable full-scan speedup
or complete DM2 platform parity; the broader startup and runtime gaps below
remain open.

2026-10-20 virtual-member digest reuse: M12 source-sidecar checks now consult
the active scan's verified virtual-path MD5 receipt before decoding a member
again. A receipt is accepted only while the enclosing archive's size and
modification time still match and the archive remains readable. The focused
`asset_find_by_hash` regression passed, and the authentic original-media CLI
startup diagnostics passed for the installed collection: the launcher found
all five games, DM1/CSB/DM2 platform routes resolved, and no-game startup stayed
in the menu. A follow-up menu scan measured 12,252 ms (12,048 ms on repeat)
with the persistent scan cache already warm; these timings do not prove a cold
scan improvement, and the menu still waits about 12 seconds on this M4 setup.
An isolated-home `--scan-data --debug` run with the same authentic collection
on the v3.0.397 build measured 27.90 seconds with a cold cache and 12.35 seconds
on the warm repeat; all five games reported READY. A 20-second macOS sample
recorded 638 of 1,795 samples inside `m12_fill_game_versions`; nested 7z→ADF
and CAPS/MFM decoding were among its hot paths. The cold run has no comparable
pre-change baseline, so it does not establish a cold-cache regression or gain.
Nexus still lacks its original Saturn title-capture gate for an interactive
menu handoff; Theron and all native presentation/input/audio routes retain
their separate runtime gaps.

2026-10-20 current-source scan profiling: on the authentic 436-file, 4.3 GiB
collection, the complete five-game scan fell from 16,088 ms to 14,596 ms after
M12 began reusing the already authenticated DM2 Macintosh HFS ZIP profile
instead of reopening the same archive in a second admission pass. This is a
1,492 ms reduction in the measured scan phase; the full scan still delays the
menu, and headless timing does not establish native presentation or input.
`m11_direct_launch_prepare_all_games` passed with 219 assertions, no failures
and three scoped skips. The authentic DM2 Macintosh M12 route also reached
runtime through the title movie and mirror selection, moved twice, passed AUTO
selection and Back/exit checks, and preserved the original archive. The
Nexus title gate and other per-game/per-platform runtime gaps remain open.

2026-10-19 full-library scan cache follow-up: the persistent digest cache now
uses indexed path lookups instead of walking every cached member for each
archive entry. Native 7z entries and nested Amiga ADF contents also reuse
digests while validating the source archive's size and modification time.
Two no-game debug starts against the installed 436-file, 4.3 GiB original-media
collection completed in 20.98 and 18.00 seconds, including the requested
one-second menu duration; the earlier runs took about 27 seconds for the scan.
The complete original-media DM1/CSB/DM2 CLI diagnostics passed in 268.36
seconds. The scan still delays the interactive menu, and these headless runs
do not establish native video/audio or physical-input behavior.

2026-10-19 persisted single-file launcher selection: the no-game startup now
promotes a remembered media file to its surrounding collection root, so the
default launcher can discover neighboring DM1, CSB and DM2 installations.
An explicit file-valued `--data-dir` remains scoped to that selected source.
Verified with an isolated config pointing at the authentic DM1 Amiga 3.6 ZIP;
the default launch stayed in `mode=menu`, found DM1/CSB/DM2, and did not hand
off to a game. Startup responsiveness remains an open issue because the full
collection scan still takes 18 to 21 seconds in the current Mac mini M4 debug
runs.

2026-10-19 full-library no-selection launch check on the Mac mini M4:
starting the current build with `--menu --debug --data-dir
~/.firestaff/data` and no `--game` reported `game=none mode=menu`, found
authentic original data for DM1, CSB, DM2, Nexus and Theron's Quest, and made
no game-handoff. The local `m11_direct_launch_prepare_all_games` CTest also
passed against the installed original-media collection. The no-selection
scanner reported completion at about 27 seconds on both initial and repeated
runs; the second run took 57.18 seconds total with a 30-second menu duration,
so the measured scan accounts for most of that startup delay. These checks
used SDL dummy video/audio and confirm availability/menu state, not native
window interaction or first gameplay frames for all five titles. Treat the
long full-library scan as an open startup-performance gap.

2026-10-19 current-source original-media CLI startup matrix:
`ctest --test-dir .codex-scratch/scan-locale-build-xcode -j 2
--output-on-failure -R '^(dm1|csb|dm2)_v1_.*native_cli_boot$'` passed 11/11
in 885.86 seconds. The matrix covered DM1 PC34; CSB Atari STX, Amiga, FM
Towns and Amiga 3.5 CTRaw, plus the in-memory hint-oracle utility; and DM2
DOS English/French, Macintosh, Amiga and FM Towns. The companion
`test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh` also exited 0 against
the installed originals, including DM1/CSB/DM2 return-to-launcher flows,
direct-game exit behavior and CSB media removal/addition reflected by a fresh
scan. Both runs used a local macOS build with SDL dummy video/audio and
authentic data from `~/.firestaff/data`; they do not establish native display,
physical input/audio, or Windows/Linux runtime behavior. DM1's other admitted
platforms are not covered by this 11-test CTest selection.

2026-10-19 current-source no-argument launcher retest:
`firestaff_cli_startup_diagnostics_real_media` passed in 224.29 seconds with
authentic DM1, CSB and DM2 archives. Starting with no game selection stayed in
`game=none mode=menu`, discovered all three games from the isolated default
`~/.firestaff/data/{dm1,csb,dm2}` layout, and completed the menu loop without a
game handoff. The same test checked DM2 FM Towns AUTO selection and explicit
platform resolution. SDL dummy video/audio verifies CLI/menu state, not native
window display or physical input.

2026-10-18 current-source no-argument launcher regression:
`tests/test_firestaff_cli_startup_diagnostics_real_media.sh` now starts the
local build without `--game` using authentic DM1 DOS, CSB Amiga and DM2 DOS
archives in an isolated default `~/.firestaff/data/{dm1,csb,dm2}` tree. The
no-game route reports `game=none mode=menu`, discovers all three games, runs
the menu loop to its no-launch exit, and never hands off to gameplay. SDL
dummy video/audio does not verify native display or physical input.

2026-10-19: the registered CTest for that original-media diagnostics matrix
passed through CTest with the installed authentic archives in 224.47 seconds.
Its timeout was raised from 240 to 300 seconds to leave headroom for slower
hosts.

2026-10-15 fresh original-media startup diagnostics retest on the Mac mini M4:
`test_firestaff_cli_startup_diagnostics_real_media.sh` passed for DM1, CSB and
DM2. It verified bare `--game` discovery from each per-game data directory,
DM2's complete 225-frame FM Towns title trace and source completion time,
mixed-edition selection, DM1's FM Towns and PC intro traces, CSB AUTO priority,
and uppercase game selection with authentic availability reporting. The test
uses SDL dummy video/audio and local original media; it does not establish
native display/input/audio parity or replace the pending GitHub Actions result
for commit `3fd0f5886`.

- Verify the data-free SDL3 close-event test in Actions on Windows, macOS and
  Linux, and run the end-to-end originals-rescan test with authentic media on
  the other supported hosts. On macOS, the latter passes against authentic
  DM1 PC 3.4 for keyboard and mouse return paths; each close cancels scanning
  without redrawing the launcher. `M11_Render_PumpEvents` also drains queued
  SDL3 window-close requests left pending during a synchronous scan. The
  initial direct `--game` scan separately verifies that DM1, CSB and DM2 never
  enter game startup when authentic local media is available.
  `m12_data_dir_cancel` covers scanner cancellation at the M12 boundary. The
  full local return/rescan matrix passed against installed originals in 1,178
  seconds; its CTest timeout is now 1,800 seconds to leave runner headroom.
  A fresh 2026-10-14 `mouse-return` run passed for DM1 PC, CSB FM Towns and
  DM2 AUTO with original media. Each game returned to the launcher and
  reported updated availability for all five games. The run used SDL's dummy
  video/audio drivers; native display and input remain separate checks.
  The companion 2026-10-14 `direct-exit` variant also passed for DM1, CSB and
  DM2: a direct `--game` launch exits after Return to Menu without reopening
  the launcher.
  `return_to_menu_rescans_media_mutations_real_media` passed in 94.22 seconds:
  removing CSB media during authentic DM1 gameplay made CSB unavailable after
  return while DM1 and DM2 remained available; adding the original CSB archive
  during gameplay made it available on return.
  The DM1 window-close cancellation matrix also passed for keyboard and mouse
  return: closing during an authentic originals rescan cancels it without
  completing a scan or leaving the process stuck.
  On 2026-10-14, the data-free `asset_scan_access_denied` and
  `m12_data_dir_cancel` checks also passed locally. They cover unreadable-path
  messaging and M12 cancellation; they do not emulate macOS's native privacy
  consent dialog.

- Extend selected-root M12-to-M11 launch verification beyond the original-media
  DM1 PC 3.4, CSB FM Towns and DM2 DOS cases to other supported editions and
  physical desktop interaction. The opt-in test covers those three source-owned
  startup states after reopening the persisted collection root. It now also
  checks DM1 Amiga 2.0 and Atari ST 1.2, CSB FM Towns Japanese and Amiga 3.1
  English, and DM2 Macintosh retail through the reopened-root handoff. The
  CSB Amiga leg verifies the native A31E M11 profile; dedicated original-media
  tests advance A31M and A33M through Amiga startup to runtime. A33M tests
  cover CLI ZIP -> ADF, an installed AmigaDOS folder built from the original
  Amiga 3.3 ADF, and the M12-to-M11 menu handoff; its separate `KAOS.FTL`
  C03_GAME digest is locked. The DM2 Amiga English route now also reopens the
  original collection root, selects the Amiga edition in M12, plays the
  source SWSH/TITL sequence, and enters the original New Game runtime through
  its GDAT pointer. Its AUTO
  DM2 FM Towns leg now finishes the source title and uses the retail New Game
  and mirror targets to enter a loaded one-champion session. The same
  authenticated Towns corpus now passes the full M12 game/platform/custom/
  launch route through the original 225-frame HME-242 title, New Game and
  mirror selection to a presented runtime frame with no core fallback draws.
  Other editions, physical M5 input and full playability remain open.

  Local verification on 2026-10-10, using a fresh Release build on the Mac
  mini M4 and original media from the user's data directory, passed the
  native CLI/start-menu boot suites for DM1 PC 3.4, Amiga 2.0, Amiga 3.6,
  Atari ST 1.2, and FM Towns English/Japanese; CSB Amiga 3.1 and 3.3, Atari
  ST, and FM Towns English/Japanese; and DM2 DOS English, Amiga English,
  Macintosh English, and FM Towns. DM1 Amiga 3.6 now has a direct CLI receipt
  and a pointer-driven M12 launch through the first runtime frame at its
  authentic Hall position. CSB Amiga 3.3 now passes the original CLI input
  matrix and M12 launch through its first source-owned runtime frame; the
  AUTO route also selects the installed A31E package with a clean config.
  The DM2 Towns test also confirmed bare `--game dm2` AUTO
  selection with competing installed editions, a stale saved PC choice, and
  the first authentic runtime map. These boot and bounded input checks do not
  establish full gameplay or physical MacBook Pro M5 behavior.
  On 2026-10-12, `dm2_v1_fmtowns_native_cli_boot` passed against the complete
  original-media directory in 344 seconds. It revalidated stale-PC preference
  override, mixed-edition M12 selection, the original 225-frame Towns title,
  and the first champion through the normal CLI and menu loops without
  boot-probe fast-forward.

- Verify iOS release packaging after the LZMA software-CRC selection fix
  using the hosted Apple Clang 15 compiler. Local iPhoneOS compilation and
  CRC-vector checks pass; complete release publication depends on all jobs.

- Verify the SDL3 drawable-coordinate input fix with native Retina events.
  Geometry and original-media launcher checks pass. Rendering, input and
  size getters now share live dimensions. On 2026-10-11, five focused DM1
  HiDPI mouse/input gates passed with MacBook logical and 2x drawable
  dimensions; `m11_display_aspect_present_rect` also passed its Retina input
  matrix and opt-in native Cocoa grow/shrink probe before resize-event
  handling. This host's live drawable is 1x. Physical MacBook Pro M5/Retina
  delivery remains unverified.

- Verify physical CSB intro audio output and complete the temporary-intro
  volume check across another authenticated edition. The Amiga 3.1
  M12/M11 real-media handoff now passes 55 assertions with no skips, including
  source-bound SWSH sample identity, launcher master/music/SFX preferences,
  mute gain and preserved host pause. This checks the audio owner and sample
  path but does not establish audible device output. CSB has no original DOS
  release, so the test uses its authenticated Amiga package instead of a
  PC34 substitute.

- Complete desktop end-to-end verification of Accessibility AUTO PAUSE.
  DM1/CSB/DM2 focus policy, independent timer/focus pause ownership, source
  clock/audio suspension and synchronous intro time rebasing are implemented.
  Authentic-media API tests pass; the optional Cocoa window probe cannot
  acquire actual input focus in this agent session and therefore fails.
  Verify real focus loss/regain through the complete main loop, including
  synchronous intros, without accumulated-time replay, on supported desktops.
  Physical audio and MacBook Pro M5 HiDPI behavior remain unverified here.

- Verify physical PC34 SONG output and hardware timing. Bounded selector
  looping, authentic PCM at both loop seams, source-requested 11126 Hz,
  Credits continuity and stop/rebind/pause ownership now pass local tests.
  PIT/Tandy device divisor timing and physical listening remain unverified.

- Connect the active Audio SOUNDTRACK choice to playback. The menu exposes
  ORIGINAL/REMASTERED/CUSTOM and exports `soundtrackMode`. The DM1 PC34
  selector now consumes that handoff after its authenticated `SONG.DAT`
  binds. It accepts a bounded WAV replacement and retains the original score
  for missing, malformed or unsupported media. Other editions/games and
  in-game music tracks still have no soundtrack-mode consumer.
  Audit on 2026-09-29: no local remastered/custom music directory is
  installed. Folder selection and absolute-path resolution are implemented;
  no replacement media is installed, so only fallback has been exercised
  through M12. Authentic PCM-derived WAV tests exercise full-length decode,
  malformed-file fallback, selector continuity and native stop ownership.
  Verify actual replacement listening and device behavior; define explicit
  game/track identities and stable roots before extending beyond the DM1 PC34
  selector. Do not replace DM2 movie PCM wholesale: it may include non-music.

- Verify native desktop file/folder selection and persistence for Data
  Directory, Custom Music, Unicode Font and Artpack. Their owned-result
  callbacks now defer menu changes to Update, with cancellation/late-result
  tests. The native Cocoa Data Directory picker was opened, selected the
  external original-media root, received the user's macOS access approval,
  and saved `/Volumes/Extern-disk/FirestaffUserData/data`. After relaunch,
  startup diagnostics read the saved root and matched DM1, CSB and DM2 original
  graphics and dungeon files. This verifies the Data Directory picker and
  persisted selection, but not native Custom Music, Unicode Font or Artpack
  dialogs. The separate `test_m12_data_dir_dialog_real` still covers the
  asynchronous result callback, main-thread admission and menu reconstruction.

- Extend COLORBLIND beyond the modern launcher and its scan overlay into
  explicitly host-owned in-game UI/HUD layers. M12 now applies the selected
  mode to the modern launcher surface; original game frames use a separate
  render path and remain untouched. The menu-render regression confirms an
  enabled mode changes launcher pixels and OFF reproduces them bit-for-bit;
  the matrix tests also pass. Keep source-owned game imagery outside the
  transform and verify each host-owned HUD layer before claiming full
  coverage. Visual parity remains deferred.

- Extend UI SCALE to host-owned in-game HUD text and connect AMBIENT to a
  concrete runtime consumer. The modern M12 launcher and both scan-progress
  surfaces now scale menu text from the saved 100/150/200 setting; controls
  and their pointer rectangles remain in the same canvas coordinates. The
  scan/menu regression verifies changed text pixels and the same tab hit at
  100% and 200%. Game-HUD scaling remains open. `M11_Ambient_Tick` is still a
  no-op with no caller, so AMBIENT remains an unimplemented setting.

- Wire the Custom menu PATCH choice to defined, source-backed behavior.
  Audit on 2026-09-29: `usePatch` / `gameUsePatch` are only read by M12
  display/configuration code. `M12_StartupMenu_GetLaunchIntent` copies the
  field, but `M11_GameView_StartFromMenu` does not consume it. No patch-set
  semantics were found in the current documentation. Define the supported
  changes per original edition before implementing or advertising PATCHED.
  2026-10-13 correction: the menu now marks PATCH as UNAVAILABLE, ignores
  pointer/keyboard cycling, and clears stale legacy `use_patch` values from
  the launch intent. The TOML/JSON field remains readable for compatibility.
  Source-backed patch definitions and runtime application are still open;
  this prevents a false PATCHED claim but does not implement patch support.

- Publish the verified Windows DLL bundle in the next explicitly requested
  release. The isolated Windows runtime test passed in hosted CI run
  36529772413. Issue #12 affects the existing 3.0.348 ZIP; a source packaging
  fix does not replace that release asset.

- Extend real-media start-menu launch coverage for DM1, CSB, DM2, Nexus and
  Theron's Quest. `m11_direct_launch_prepare_all_games` now drives the game-card,
  verified-platform and custom-options screens with installed original media,
  checks that the launch intent retains the authenticated edition selected on
  the platform card, and reaches M11 through that intent. A 2026-10-07 run
  against a temporary root with only original DM1/CSB/DM2 media passed 148
  assertions with zero failures and four skips; Nexus and Theron were not
  staged. This confirms direct `--game` preparation and the M12 handoff into
  M11 for each staged game, while separate boot probes do not prove that the
  visible menu path reaches the first runtime frame. It still needs to advance
  each remaining game from this same menu handoff to its first source-owned
  runtime frame. Theron's authentic Japanese edition now reaches a source-owned
  runtime receipt through the same M12-selected M11 menu handoff; verify that
  path through its first presented runtime frame.
  A 2026-10-11 rerun against the complete installed original-media collection
  passed 210 assertions with no failures and three scoped skips; none of the
  five games was skipped for missing data. It verified the direct `--game`
  preparation and source-specific M11 selected-entry boot receipt for DM1,
  CSB, DM2, Nexus and Theron's Quest. The remaining gap is the visible menu
  card sequence through a first presented runtime frame for the games without
  that end-to-end evidence.
  A 2026-10-12 rerun from the external-disk original-media collection passed
  219 assertions with no failures and three scoped skips; all five game data
  sets were present and admitted. This revalidates the M12 card/platform/custom
  selection and M11 handoff contracts, but still does not prove first presented
  runtime frames for every title.
  A 2026-10-13 rerun on the Mac mini M4 against the configured original-media
  directory again passed 219 assertions with zero failures and three scoped
  skips. The authenticated Theron's Quest US Track 02 route reached a
  source-owned runtime frame with level 0 loaded and one champion. Nexus reached
  its authentic title from the installed English Saturn CUE, but its receipt
  remained blocked at
  `menu-bpk-prs3-capture-required`; no original Saturn presentation capture is
  installed in that data directory, so the interactive Nexus start-menu path
  remains incomplete. This gate checks the direct-launch/M12 handoff contracts,
  not visible physical-menu interaction or all five first runtime frames.
  A fresh 2026-10-14 run of `m11_direct_launch_prepare_all_games` on the
  current M4 build with the full installed original-media root passed 219
  assertions, failed none, and reported three scoped skips. It revalidated
  all five selected-entry startup paths. The authentic Theron USA Track 02
  reached level 0 with one champion; Nexus remained fail-closed at
  `menu-bpk-prs3-capture-required`. This contract test does not simulate
  physical M12 card interaction or establish first presented gameplay frames
  for every game; the Nexus launch and remaining per-game menu-to-runtime
  gaps stay open.
  Authentic DM1 PC 3.4 and CSB Amiga A31M now have this runtime-frame
  assertion. The CSB route waits through TITL.DAT and uses the native AppB
  language choice; the native Amiga input matrix reaches runtime, and the
  authentic FTL A31E archive is discovered through AUTO and reaches runtime
  from the start menu. Authentic DM1 DOS English (including the nested PC-34
  archive), Amiga HD, and FM Towns now also pass their original-media CLI/menu
  startup routes. DM1 Amiga v2.0 now also reaches its first Hall runtime frame
  through the pointer-driven M12 IMG2 route, and the same menu test restores
  its authentic save-disk state through Quick Resume; the new-game start has
  zero champions. Authentic DM1 Atari ST v1.2 and DM2 DOS English follow normal
  M12 selection and source-owned startup input to a presented first runtime
  frame; authentic DM2 Amiga now does the same through its
  original SWSH/TITL and GDAT New Game pointer route. DM2 French DOS now also
  reaches its authenticated initial party through the normal M12 → MVE →
  New-Game route, with direct-probe movement checked separately. The optional
  authentic CSB Atari ST route now completes ordinary M12 selection and
  ANIMATE.SCR/FTLCODE startup through the source-owned entrance view; the
  normal CLI route now clicks the authentic C200 hit box and verifies the
  runtime startup receipt after the doors open. Its initial state has zero
  champions, so this does not prove a playable campaign party. The optional
  DM2 Macintosh retail regression now drives M12 game/platform/Custom/launch
  pointer selection through films, New Game and mirror selection to an
  accepted original-asset runtime frame with no core fallback draws; its
  separate CLI boot probe also passes the startup boundary. The optional
  authentic French DOS EUDATA ZIP now passes the normal M12 new-game route
  through its first runtime frame and source-owned movement. The separate
  original-save Quick Resume route remains unverified: the installed French
  ZIP contains neither DMSAVE.DAT nor DMSAVE.BAK. English Atari ST 1.2,
  German Atari ST 1.2, French Atari ST 1.3 and English ST 1.0a/1.0b/1.1 also
  reach DM1 runtime from M12 with no selected champions. All six source
  packages now have verified movement and pointer
  recruitment of C127 ordinal 14 through the normal M12 route; German v1.2
  also passes the direct CLI route. Repeat on any other admitted Atari profile.
  Atari now uses a distinct STARTUP1.C/F0437/F0441 media receipt, skips PC
  SWSH and PC34 special palettes, and routes entrance input through Atari's
  source mouse command. Authentic English, German and French Atari ST M12
  regressions now require the applied startup handoff and HoC first-frame
  receipt before accepting the live runtime state. Source review of ReDMCSB
  STARTUP1.C:160-173 shows the Atari path continues after F0441 through the
  F0435 load loop and F0462_StartGame before runtime. Firestaff's M12 handoff
  begins with zero champions. Normal M12-to-runtime routes and C127 pointer
  recruitment now pass against authentic Hall data on all six admitted Atari
  editions: English v1.0a, v1.0b, v1.1 and v1.2, German v1.2, and French
  v1.3. Their live runtime receipts require the startup handoff and HoC first
  frame. `DUNGEON.FTL` is only an optional custom-dungeon path in ReDMCSB
  LOADSAVE.C; its absence from clean retail STX disks does not block built-in
  new-game startup. The Atari receipt covers title/entrance; its F0437
  presentation/palette remains open.
  The mixed-root DM1 M12 regression now starts the authentic PC 3.4, Atari ST
  v1.2, and Amiga 2.0 editions explicitly from one data directory that also
  contains FM Towns; AUTO still selects Towns. Each selected edition reaches
  its source-owned first runtime frame and matches its original graphics
  identity. This is SDL dummy-driver evidence and does not verify Retina
  drawable/input behavior or pixel parity.
  Headless receipts do not claim host capture, and visual parity remains
  deferred.
  The shared authentic-media M12 regression now covers one session through
  DM1 runtime -> return/full rescan -> CSB runtime -> return/full rescan. It
  passed locally on 2026-10-09 with the published v3.0.374 binary and original
  Towns media; these test edits still need an Actions run, and hosted CI may
  skip this route when licensed archives are unavailable.
  The dedicated DM1/CSB/DM2 return-to-menu regression now also proves that a
  newly added original CSB archive becomes selectable after the live rescan;
  it passed alongside the existing archive-removal case on 2026-10-10. Hosted
  CI may skip media-dependent assertions when licensed archives are absent.
  On 2026-10-12, the six-session CSB mixed-original-media M12 matrix passed:
  AUTO selected FM Towns; the platform cards reached Atari ST and Amiga
  A31M; and explicit FM Towns, Atari ST and Amiga selections retained the
  authenticated edition through startup. Direct CLI selection also reached
  the source-owned startup phase for all three native platforms. This does not
  establish CSB visual parity or a populated playable party for every edition.
  Other DM1 editions, remaining DM2 platforms, Nexus
  and Theron still need equivalent evidence. Keep this separate from visual
  parity.
  On 2026-10-20, `m12_main_menu_runtime_real_media` passed 1/1 in 297.93
  seconds against the installed original FM Towns archives. It starts with no
  game selected, uses the visible M12 keyboard card flow, and checks the first
  active runtime for DM1, CSB and DM2. DM1 reaches its HoC first frame; CSB
  loads the original MINI.DAT party and a nonzero viewport receipt; DM2
  completes all 225 original FM Towns title frames, loads the first champion,
  and accepts a runtime frame with no core fallbacks. The check also hashes
  each original archive before and after launch. This closes the no-game
  main-menu-to-runtime gap for these three default FM Towns editions. It does
  not establish physical input/display parity or coverage for every edition;
  Nexus and Theron remain outside this ownership scope.

- Extend F0219 current-cell collision coverage to original-media encounters,
  half-square creature footprints, nonmaterial/Black Flame cases and inactive
  group destinations. The source-cell tests cover centered and quarter-cell
  admission plus party priority; destination admission still uses active AI.

- Extend F0213 initial-burst ownership to CSB/legacy ownerless creation
  consumers; their previous delayed burst remains isolated from the fixed
  source-bound DM1 transaction. Verify packed source/target MapXCombo damage
  coordinates, group kill cleanup and original emulator RNG/damage traces.
- Extend source C14 retirement verification to original-media GROUP.Slot
  transfers, thrown-potion consumption, projectile collisions and allocation failures;
  audit all remaining host unlink paths against F0164. Verify complete
  original-emulator flight timing beyond the authentic DOS regression lane.

- Extend fresh-game G0236 pool admission to the separately authenticated
  Atari/Amiga/FM Towns paths and CSB. Audit runtime capacities against source
  pool/event counts (including 50 fresh C15 slots), pool recycling and all
  resource-exhaustion behavior. Save/import expansion must remain disabled.
- Extend ordinary C15 runtime captures to all visible lanes, explosion
  families and original-emulator timing/pixels. Five D1C spells in three modes
  does not prove complete explosions or kinetic-energy scaling parity.
  Verify raw C006 unlink timing and RNG against an original emulator trace.

- Extend the complete projectile-orientation query to CSB's live material
  owner with original edition-specific weapon aspects and view lanes;
  its legacy sprite query still uses subtype and direction-only bitmap
  selection. Verify all authentic projectile-art families, side/deep lanes,
  both map parities and kinetic-energy scaling through full runtime captures.

- Extend DM1 thrown-object capture coverage to the separate Hall of Champions
  renderer, all visible flight cells and natural-play emulator comparisons.
  The DOS PC34 D0C real-media regression uses a RAM-configured party/camera
  and mastery threshold; it does not establish complete flight/render parity.

- Extend the [DM1 world XP transaction](docs/parity/DM1_ORIGINAL_XP_TRANSACTION.md)
  evidence to original-emulator level crossings for melee, parry, sensors,
  throw and actions; finish original recently-upgraded flags and localized
  message-area timing. Audit XP saturation and maximum-stat edge semantics.
- Bind edition-specific level-up antimagic behavior for remaining early
  DM1 editions (modulo 3 versus PC34/late-Amiga two bits) and compare complete
  level transitions against original emulator traces, including source UI.

- Extend authentic combat coverage to multi-creature selection, reach
  blocking and exact damage/XP comparisons against original emulator traces.
  The relocated-party original-group tests do not prove a natural play route.

- Complete CSB's source-owned cast execution. For authenticated FM Towns
  media, M11 now performs F0409's G0487 lookup and F0408's meaningless-spell
  clear, including the lone-power-symbol path, without touching RNG, effects,
  XP or the timeline; a valid source spell remains fail-closed rather than
  executing DM1 effects against CSB state. Spell-panel raster/input
  verification does not close this gap.
  Verify Japanese caster names through their authentic
  whole-string glyph path and compare spell-panel timing with an emulator.
  Follow [the original cast contract](docs/parity/CSB_ORIGINAL_CAST_CONTRACT.md)
  for edition tables and complete effect/XP/timer transactions; CSBWin-only
  parser or abort-path tests are not original-game casting evidence.
  Connect F31's admitted 29-record table to the complete transaction and
  admit Atari/Amiga tables.
  Implement original F0304 practice/level-up mutation and bind resting state
  end-to-end; verify Firestaff mastery bonuses with authentic object evidence.
  Bind G0361 at the live creature-attack boundary and preserve original
  level-up RNG ordering, recently-upgraded flags and localized messages.

- Complete CSB FM Towns action-menu pixel parity and all row boundaries.
  Source C696 now owns language-specific action/Pass pointer geometry;
  remaining Japanese glyph and visual-composition gaps are independent.
  Replace the generic HUD's PC-only material admission: original F31 C011
  is 14x39, not 14x26; Japanese C010 is 96x72 and C013 is 96x41.
  Extend the corrected C007 viewport origin to emulator pixel comparisons
  and authentic C080 edge/pickup/throw/sensor interaction sequences.
  Do not enable the whole generic HUD merely by accepting its asset sizes.
  Verify all idle-cell click edges/gaps and source cooldown transitions.
  Verify empty-hand
  cooldown expiry against original emulator pixels, beyond predicate tests.
  Extend source C013 movement-control verification beyond closed-inventory
  panel pixels and rotation to traversal, exact edges/gaps and inventory mode.

- Complete Japanese FM Towns active-menu text and remaining row boundaries.
  Reconcile the old JDM mixed-font adapter with TEXT2.C:75–105: the original
  chooses the Japanese path for a whole string, then F0952 uses 8x16 or 16x16
  system glyphs per unit. Its ASCII/M653-per-byte mixture is not equivalent.
  JAPANESE.C:242–269 calls EGB_sjisString; do not add a runtime BIOS dependency.
  Extend full C010 panel tests to Japanese labels; English one/two/three
  action panels are covered using existing dungeon weapon records.
  Retire obsolete solid-fill shim/tests and misleading colour-selector
  names; destination region IDs11/77/79 are not palette values.

- Extend DM1 FM Towns Japanese movement pointer coverage to traversal and
  exact boundaries, and action-cell coverage to outside edges. Verify
  hatching against the original JDM registry. Verify Japanese message
  wrapping and glyph metrics within its224x33 container. The unsupported
  Japanese menu text adapter is no longer called by the live renderer.
  Extend Japanese spell input verification to casting and outside-parent
  boundaries; verify Japanese text separately from the ASCII/rune oracle.

- Complete DM1 inventory-owner regression for Modern composed-HUD input
  and remaining consumers; the source-layout owner/leader separation is
  implemented and covered by original-media tests.

- Verify DM1 death/resurrection leader ownership with original-media
  runtime sequences, beyond isolated leader-selection admission checks.

- Extend DM1 carried-load verification to Modern composed-HUD leader switching, cross-champion
  exchanges and open-chest mutations; see TODO-dm1.md for remaining scope.

- Compare Atari CSB's full chest composition with a same-state original
  runtime capture; source-material composition tests are not emulator parity.

- Extend Atari CSB's original-object inventory corpus to equipment/chest
  drag destinations and native chest-panel interactions.

- Extend original-media chest interaction verification to CSB Atari and
  Amiga, including panel geometry, same-owner refresh and owner switching;
  successful startup alone does not establish inventory interaction parity.

Reviewed 2026-09-23. This file contains only work that is still open. Game
details and acceptance evidence belong in `TODO-<game>.md`; completed work is
recorded in `DONE-<game>.md`. Historical mixed logs are retained as
`HISTORY-archived-2026-08-08.md` and in Git history, not as active work.

- Bind all production paths to user-supplied original media under
  `~/.firestaff/data/<game>`; do not substitute generated gameplay assets.
- Keep every platform claim at the strongest real-media evidence level in
  `docs/PLATFORM_STATUS.md`; a parser or fixture is never a playable-route
  claim.
- For each open behavior: obtain original-media evidence, bind it to an
  original consumer, add a native regression, then record the result in DONE.
- Keep external emulators and disassemblers development-only. Firestaff must
  run the games natively and may not depend on them at runtime.

Current authoritative status: `docs/PROJECT_STATUS.md`,
`docs/PLATFORM_STATUS.md`, and `docs/MISSING_FUNCTIONS_BY_GAME.md`.
