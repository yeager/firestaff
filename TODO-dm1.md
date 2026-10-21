2026-10-21 Apple IIGS startup follow-up: selected direct and M12 launches now
read the authenticated G20/G21 installation and play the native `C001`
`GRAPHICS.GAME` title raster through an 18-frame, nominal-60-Hz F0437 zoom
before opening the first world. The C001 frame and IIGS RGB4 zoom palette are
covered by `dm1_v1_iigs_graphics_real_media` against both original disk images.
The `dm1_v1_iigs_graphics_real_media` and `firestaff_iigs_cli_real_media`
CTest cases pass against both original disk images. The CLI test asserts title
frame 18 and F0437 completion precede `DM1 IIGS READY` for direct and M12
launches.
Current zoom sampling is explicitly nearest-neighbour and does not reproduce
the GS-specific F0129 scaler; palette fade stages, PRESENTS/MASTER overlays,
F0441 entrance, and hardware visual/input parity remain open. This is a
partial startup stage, not complete native startup parity.

2026-10-21 DM1 Quick Resume handoff: M12 passed Firestaff's own `FSM11QS1`
envelope to the original-save-only F0435 reader, so valid Firestaff saves
could not resume from the launcher. `M11_GameView_LoadDm1SavePath` now routes
that authenticated envelope through the world deserializer and hash check;
retail save files still use their existing F0435 readers. The quicksave path
also retains the open-chest reference in its runtime sidecar while F0334
flushes the container into the saved world. `test_m11_quick_resume_roundtrip`
passes with the authentic PC 3.4 archive for Firestaff and original PC34 save
flows. An older local Firestaff save deserialized but failed the current world
hash check and remains rejected; integrity checks were not relaxed.

2026-10-21 original-media CLI/menu regression matrix:
`dm1_v1_pc34_native_cli_boot`, `dm1_v1_atari_st_archive_cli_boot`,
`dm1_v1_amiga_v20_archive_cli_boot`, and `dm1_v1_fmtowns_archive_cli_boot`
passed 4/4 from the current source build. The tests used the installed original
PC 3.4, Atari ST 1.2, Amiga 2.0, and FM Towns archives and exercised their
covered direct and M12 launch routes. PC34 selector Resume input is covered,
but the local collection has no authentic PC34 save, so loading and restoring
one through that selector remains unverified. SDL dummy does not establish
native display, audio output, or physical input parity.

2026-10-21 original-edition startup follow-up:
`dm1_v1_atari_st_11_archive_cli_boot`, `dm1_v1_atari_st_10a_archive_cli_boot`,
`dm1_v1_atari_st_10b_archive_cli_boot`,
`dm1_v1_atari_st_de_archive_cli_boot`,
`dm1_v1_atari_st_fr_archive_cli_boot`,
`dm1_v1_amiga36_original_media_cli_boot`,
`dm1_v1_amiga20_french_archive_cli_boot`,
`dm1_v1_amiga20_german_archive_cli_boot`,
`dm1_v1_amiga22_german_archive_cli_boot`,
`dm1_v1_amiga21_22_english_archive_cli_boot`,
`dm1_v1_dos_en_archive_cli_boot`, `dm1_v1_dos_fr_zip_cli_boot`, and
`dm1_v1_pc34_nested_archive_cli_boot` passed against the installed original
archives. `firestaff_iigs_cli_real_media` and
`dm1_v1_iigs_startup_data_real_media` also passed against the original IIGS
software archive. These SDL dummy runs verify the named startup/metadata
routes, not native display, audio output, or physical input parity.

2026-10-21 no-game main-menu selection:
`test_m12_main_menu_game_selection_real_media` passed both keyboard and mouse
selection of DM1 from the no-game launcher, beginning with
`startup game=none mode=menu` and handing off to the authentic FM Towns
edition. The current-source `test_m12_main_menu_runtime_real_media` rerun also
reached DM1's first authentic HoC runtime from that no-game menu. The isolated
data root exposed only DM1, CSB and DM2 originals; both tests verified the
selected DM1 archive remained unchanged. SDL dummy output does not prove
native display, physical input, or audio parity.

2026-10-21 DM1 mixed-library platform-menu regression:
`dm1_v1_m12_auto_multi_platform_menu` passed against the installed original
FM Towns, PC 3.4, Atari ST 1.2, and Amiga 2.0 archives. It verified that AUTO
selects FM Towns by default, the M12 platform menu launches the chosen PC,
Atari ST, and Amiga edition, and keyboard selection from AUTO reaches PC 3.4.
The corresponding direct CLI launches also reached their authenticated first
runtime frame. This covers the tested start-menu and edition handoffs only;
SDL dummy does not prove native display, audio output, or physical input.

2026-10-21 Amiga HD/demo follow-up:
`dm1_v1_amiga_hd_archive_cli_boot` passed against the authentic Amiga HD ZIP.
`dm1_v1_amiga_demo_archive_cli_boot` skipped because the required original
`dm-amiga-demo-en.zip` is not present in the local or external archive cache;
the installed Amiga software archive does not contain that demo package.
Demo startup therefore remains unverified with authentic media; no substitute
payload was created.

2026-10-21 Atari outer-archive startup follow-up:
`dm1_v1_atari_st_outer_archive_cli_boot` passed against the authentic Atari ST
preservation ZIP. It exercised the nested ZIP/STX CLI route, the M12 platform
selection route, and a native movement probe at the expected first-Hall pose.
The test verified the outer archive remained the selected owner and that no
Amiga archive took over the launch.

2026-10-20 authentic DM1 PC-34 startup/Hall regression rerun:
`dm1_v1_pc34_native_cli_boot` passed with the installed DOS 3.4 ZIP. The
original-media run covered clean quit from the first I34E and SWSH frames,
ANIM/TITLE timing and selector-door progression, direct CLI and M12 launch,
Resume/Credits/Quit routes, Original and Modern HoC entry, and real mouse
recruitment plus champion switching 0→1→0. The full-redraw diagnostics also
reported frame rate, CPU/frame timing and explicit GPU availability. The
archive remained the source of the selected PC-34 graphics and dungeon. SDL
dummy video/audio does not verify native Mac input delivery, audible output,
or display parity.

2026-10-20 authentic Amiga 2.2 German startup:
`dm1_v1_amiga22_german_archive_cli_boot` passed against the original A22G disk
extracted from the installed 7z archive. Its 910,058-byte ADF remained
byte-identical through ZIP transport and startup. Direct
`--game dm1 --platform amiga` reached the authenticated runtime; both ZIP-backed
and direct-disk start-menu routes reached the first Hall frame at the expected
original pose. SDL dummy verifies these CLI/menu handoffs, not native display,
sound output or sustained campaign play.

2026-10-20 no-game startup scan: full collection admission is deferred until
the branded startup intro and its progress panel, so original archive checks
overlap that presentation. The launcher still waits for the complete scan
before accepting a game choice; direct `--game` startup remains synchronous so
its edition is authenticated before launch. A real-media close test confirms
that closing during the intro scan cancels before a game handoff. Two warm
cache no-game runs measured 12.252 and 12.048 seconds before this change; they
are not a cold-cache comparison. One full deferred scan reached 100% after
9.242 seconds with the local cache warm; this single run does not establish a
stable wall-time gain. Native display, input and scan responsiveness on macOS
remain unverified.

2026-10-20 PC34 selector timing diagnostic: ReDMCSB `SELECTOR.C` calls
`F8353_WaitVerticalBlank` once per `F8360` door frame, then calls
`F8323_WaitSoundPlayComplete` after the door loop. The original-media
`dm1_v1_pc34_native_cli_boot` run presented all 32 source-sized frames at
16–17 ms intervals and completed the visible animation in 518 ms with the
SDL dummy driver. It then waited 308 ms for the authentic SND3 switch/door
effects to drain, for an 826 ms total selector handoff. `--debug` now reports
door-animation time and post-animation audio wait separately; the regression
checks their sum and the 70 Hz source cadence. This matches the source ordering
and gives a more precise diagnosis of the reported roughly one-second
Entrance sequence. It does not establish native M4/M5 display or audible
timing; no animation cadence change is justified by the dummy-driver trace.

2026-10-21 current-source real-media launcher/rescan integration:
`return_to_menu_rescans_dm1_csb_dm2_real_media` passed its full `returns`
matrix on the current source, including DM1 PC34, FM Towns, Atari ST and Amiga
3.6 routes. Direct `--game dm1` exits after Return to Menu; menu-launched
sessions return to the launcher and rescan authentic DM1/CSB/DM2 availability.
This run completed in 1435.20 seconds with no test failures. The matrix does
not include IIGS, and its SDL dummy-driver runs verify the tested launch/input
transitions, not native display, audio or physical input parity.

2026-10-19 current-source PC34 HoC movement/champion-input retest:
`m11_dm1_hoc_c127_resurrect_reincarnate_full_pc34` passed against the
authentic PC34 archive. The real movement input route reaches a first C127,
recruits its champion, then follows a second BFS-derived route through M11
movement input to another original C127 and recruits again. The second mirror
is no longer reached by assigning the party pose directly. C013/C012 pointer
tests now click the actual top-row name boxes at y=3 and switch the active
champion 0→1→0. The test uses ReDMCSB-owned coordinates and original media;
SDL dummy input does not establish native M5 event delivery or HiDPI mapping.

2026-10-19 current-source PC34 combat recheck:
`m11_dm1_pc34_first_monster_combat_real` passed against the authentic DOS
archive. From the original HoC start, production movement reaches the first
map-one group; the original group moves into the forward cell and damages the
party, then a real pointer-driven PUNCH awards matching Ninja/Fight XP. A
repeat through the same original route verifies the live M11 body-shield
snapshot: the exact first attack falls from 28 HP damage to 17 with
`partyShieldDefense=100`. The test also confirms live F0309 capacity is used
after stamina and wound changes. This verifies that physical PC34 path, not
physical input, every combat type, or complete platform parity.

2026-10-19 current-source DM1 M12 platform-card regression:
`dm1_v1_m12_auto_multi_platform_menu` passed in 90.07 seconds against the
installed originals. AUTO selected FM Towns; the tested M12 platform cards
launched PC34, Atari ST 1.2, Amiga 2.0 and FM Towns source routes. The SDL
dummy run verifies launcher handoffs and scripted runtime input, not native
display or physical input/audio behavior.

2026-10-19 current-source DM1 Atari ST original-media matrix:
`dm1_v1_atari_st_archive_cli_boot`, `dm1_v1_atari_st_11_archive_cli_boot`,
`dm1_v1_atari_st_10b_archive_cli_boot`,
`dm1_v1_atari_st_10a_archive_cli_boot`,
`dm1_v1_atari_st_de_archive_cli_boot` and
`dm1_v1_atari_st_fr_archive_cli_boot` passed 6/6 in 288.96 seconds. The
English 1.0a, 1.0b, 1.1 and 1.2, German 1.2, and French 1.3 packages reached
their authentic CLI/M12 startup routes with source-owned Hall/input checks.
SDL dummy output does not verify native Atari display, audio or physical
input parity.

2026-10-19 current-source original-media DM1 Amiga 3.6 startup retest:
`dm1_v1_amiga36_original_media_cli_boot` passed in 25.61 seconds with the
installed authentic Amiga 3.6 ZIP. It verified explicit German CLI startup
and M12 menu launch. The test does not establish native Amiga display, audio
or physical input parity.

2026-10-19 current-source original-media DM1 Atari ST startup retest:
`dm1_v1_atari_st_archive_cli_boot` passed in 80.41 seconds with the installed
authentic Atari ST 1.2 ZIP. The M12 route reached the dungeon and accepted
the scripted movement sequence before the final 500-frame runtime probe.
SDL dummy output does not verify native Atari display, audio or physical
input parity.

2026-10-19 current-source original-media DM1 FM Towns startup retest:
`dm1_v1_fmtowns_archive_cli_boot` passed in 278.45 seconds with the installed
authentic Japanese/English ZIP. It covered AUTO/explicit English and Japanese
CLI routes, M12 language/menu selection at 1920x1080, and a 720-frame V1
input/runtime probe. SDL dummy output does not verify native display, audio
or physical input parity.

2026-10-19 current-source original-media DM1 PC34 startup retest:
`dm1_v1_pc34_native_cli_boot` passed in 408.29 seconds with the installed
authentic DOS archive. It covered direct CLI, M12 startup, PC34 intro and
save-route checks, scripted keyboard/mouse input, V1/V2 presentation and
1920x1080 launch sizing. SDL dummy output does not verify native window
presentation, physical input latency or GPU behavior.

2026-10-18 Amiga 2.x startup retest: the original software archive passed
`test_dm1_v1_amiga21_22_english_archive_cli_boot.sh` for English 2.1 and 2.2,
including authenticated ADF, graphics, SWSH, dungeon, title and entrance
receipts, direct CLI startup and M12 handoff. Its German 2.2 disk passed
`test_dm1_v1_amiga22_german_archive_cli_boot.sh` through direct CLI, ZIP-backed
menu, and direct-disk menu routes. The headless tests establish the first
runtime pose, not full gameplay, physical audio or presentation parity.

2026-10-18 French DOS ZIP startup retest: the published arm64 binary passed
`test_dm1_v1_dos_fr_zip_cli_boot.sh` with the authentic repacked
`Dungeon-Master_DOS_FR_EUDATA.zip`. Direct CLI and M12 launch reached the
native IMG3 runtime and accepted a source-owned movement step. The original
RAR 2.0 wrapper remains unsupported; the repacked ZIP and extracted original
files are supported paths. SDL dummy output does not establish native display,
input-latency or audio parity.

2026-10-18 Amiga 3.6 startup retest: the published arm64 binary passed
`test_dm1_v1_amiga36_original_media_cli_boot.sh` against the authentic
`Dungeon-Master_Amiga_36.zip`. Direct CLI archive and discovered-directory
routes and the M12 handoff reached the first runtime pose. The test verified
the native C696 entrance zones, 64 two-pixel door steps and fifteen palette
steps. SDL dummy output does not establish original-emulator frame alignment,
native audio, full gameplay or display parity.

2026-10-18 Amiga 3.6 language startup: the original-media regression now also
starts the hash-verified French and German dungeon files through direct CLI
`--lang fr` / `--lang de`, M11 direct runtime handoff and the M12 menu. The
matching source-language graphics selection through G2004 was implemented
and checked against original records on 2026-10-19 (see the entry below).

2026-10-18 v3.0.386 original-media CLI/menu retest: `test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh returns` passed the authentic DM1 PC, FM Towns, Atari ST and Amiga routes. The direct `--game dm1` route also exits after Return to Menu instead of reopening the launcher, and every tested return scan found DM1, CSB and DM2. `test_firestaff_cli_startup_diagnostics_real_media.sh` passed the FM Towns default-selection and original PC34/Towns title-cadence checks. These runs used the published arm64 binary with SDL dummy output; physical display, input latency and complete platform parity remain open.

2026-10-18 Apple IIGS startup retest: `test_firestaff_iigs_cli_real_media.sh`
passed for original G20E and G21E ProDOS media. Direct CLI reaches
`dm1-iigs-runtime` with the first world loaded and accepts the native keyboard
turn probe. M12 also reaches the same first-world handoff for each edition.
The original media archive hash is checked before and after. SDL dummy output
does not verify physical presentation; the title/entrance sequence and audible
DOC playback are still open.

2026-10-19 current-source Apple IIGS startup recheck: both
`firestaff_iigs_cli_real_media` and `m11_dm1_iigs_startup_real_media` passed
against the original G20E/G21E ProDOS media. Direct CLI and M12 startup covered
both editions; the native M11 input probe also stepped in the Hall. The source
archive hash remained unchanged. This confirms the existing first-world
handoff, not the unfinished IIGS intro, audible DOC playback or full campaign.

# Firestaff TODO — DM1

2026-10-20 PC34 I34E TITLE input ownership: ReDMCSB `ANIM.C:2113-2114`
checks `F0539_INPUT_Cconis` before allowing an interrupted frame, and PC34
`IO2.C:179-185` defines that call as keyboard-buffer presence. Firestaff now
ignores mouse clicks during the TITLE stream even on frames whose authentic
`BR` record permits a keyboard interrupt. The original-media
`dm1_v1_pc34_native_cli_boot` regression injects a click on the first frame,
asserts the 53-frame TITLE completes, and separately confirms an allowed
keyboard press still interrupts it. The complete PC34 CLI/M12/HoC input
matrix passed with the DOS retail ZIP. SDL dummy does not verify native Mac
mouse delivery or display parity.

2026-10-19 PC34 motion diagnostic retest:
`dm1_motion_present_diagnostics_real_media` passed against authentic DOS 3.4
media. On this host with SDL dummy output and `--debug-full-redraw`, the
20-second manual trace reached approximately 60 presented frames/s, 15–17%
CPU per core and 0.3–1.2 ms from completed source movement to Draw. The dummy
present call waited about 11–17 ms per frame; GPU use and physical scanout were
unavailable. ReDMCSB and Firestaff both bind movement to the source-controlled
tick/input loop. This does not explain the reported display lag on the user's
MacBook; native-display input-to-present timing and an original-executable
comparison remain open. No tick cadence change is justified by this evidence.

2026-10-19 Amiga 3.6 localized graphics: ReDMCSB `MEMORY.C:F0608` selects one
of ten G2004 source graphics by `G2000_Language`. The shared M11 asset loader
now applies the exact original English/French/German index table to both size
queries and loads after authentic A36 binding and before the first runtime
draw. The authentic A36 graphics test checks all ten source index mappings
and compares the seven bitmap records against their corresponding original
graphics for each language; the CLI test also checks the entrance record
remap during French/German startup. This closes the G2004 graphics-selection
gap, not translated text or visual parity.

2026-10-19 ReDMCSB DM1 timing-profile audit: `GAMELOOP.C:46-50` selects a
maximum count of 12 for the `MEDIA722` set, which includes PC34 (`I34E`/`I34M`)
and Amiga 3.6 (`A36M`). Amiga's `VBLANK.C:F0577` increments once per actual
PAL blank. Firestaff now configures A36M at 20 ms per blank and 12 blanks per
source tick, and its gameplay idle interval follows that 240 ms cadence (with
the existing speed multiplier applied). The focused cadence test checks the
239/240 ms boundary and speed scaling. PC34 remains a separate unresolved
path: `IO.C:F0782` adds `G2163_` to the wait count, `IO.C:3898` obtains that
increment from the PC IO driver callback, and `IBMIO.C:F8114` returns 3.
Its default IRQ path installs the callback from `IBMIO.C:F8117` and invokes
it from the timer handler while `G8028_` is zero, reaching the 12-count
threshold in four callbacks. PC34 now keeps this input-wait counter separate
from E0017's 20 ms display VBlank, adds three count units on each default
IBMIO BIOS/PIT callback, and gates the source game tick on the four callbacks.
The host's millisecond scheduler uses a nominal 55 ms callback (the source
period is 65,536 / 1,193,182 seconds, about 54.925 ms); the input-wait test
checks the 219/220 ms boundary and confirms independent 20 ms E0017 delivery.
The authentic PC34 first-monster harness also asserts that a direct M11
launch without a catalog hash receipt selects the PC34 timing profile.
This is a quantized source model, not a measured original-PC34 repeated-step
trace. Compare repeated-step traces against the original PC34 executable
before claiming movement-timing parity; the 60-FPS presentation diagnostic
does not measure game-tick rate.

2026-10-19 F0309/F0312 live capacity refresh: ReDMCSB `CHAMPION.C` F0312
calls F0309 while building every shield-strength contribution. Firestaff had
read the previous `maxLoad` cache in its M11 defender and orchestration paths;
it could therefore calculate with stale capacity after stamina, wounds,
strength or feet equipment changed. F0312 now derives capacity from the live
champion fields and authenticated feet icon, and M11 refreshes the cached
capacity at each completed DM1 source tick for every DM1 profile. ReDMCSB
`CHAMPION.C:F0306` also documents compiler-specific expression evaluation:
Megamax Atari and High-C FM Towns use the halved value in the second term;
PC34, Amiga 3.6 and IIGS use the original value there. Firestaff now selects
that formula by the active world profile in both the cache and F0312 paths.
The focused unit test checks both formula variants, and the original PC34 HoC
test changes the real champion's stamina and leg wound, verifies the live
F0312 result and post-tick cache, and retains natural first-monster movement,
damage and PUNCH XP checks. `DM1 READY` reports the selected F0306 operand
order, and the Amiga 3.6 original-media CLI matrix asserts the native profile
for its default, French and German starts. The original-media
`m11_dm1_source_f0306_real`
test also recruits an authentic Atari ST and FM Towns mirror champion, enters
the low-stamina branch, distinguishes the two F0306 formulas through F0312,
and verifies the idle-tick F0309 cache for both source profiles. IIGS runtime
formula coverage and authentic shield/throw scenarios remain open.

2026-10-15 offline-selection recovery: configured/selected path intent now
survives missing media and scanner recovery. The Settings path remains the
remembered preference; runtime access uses published scanner ownership.
Zero-ready, non-cancelled saved game-leaf scans may recover default media;
explicit selections and usable configured collections retain precedence.
Return-menu rescan retries reconnected directories, archives and virtual
locators by checking their outer container, without changing hash admission.
Actual OS volume unmount/reconnect and macOS privacy prompts remain integration
checks; task-owned filesystem absence/reconnect and original-media alias tests
cover the path/state contracts. Physical Retina and complete input latency
remain separate open work.

2026-10-15 startup scan feedback: deferred startup and return-menu scans now
finalize permission/no-data messages on the UI thread after completed work.
Pending, incomplete and cancelled scans do not publish dialogs. Authentic DM1
permission tests cover denial beside available media, restored access and
removing the last staged original-archive link. These use POSIX permission
bits; actual macOS privacy prompts remain a separate OS integration check.
Missing selected roots, archive paths and process-selected originals roots are
no longer recreated by no-data feedback. Only the built-in default root is
bootstrapped. The desktop CLI absence/default regression passes; actual OS
unmount/reconnect and mobile Documents paths remain separate integration checks.

2026-10-15 diagnostic hashing correction: lightweight trace/scriptwait metadata
and launch-scoped M12 host capture publication are implemented. Explicit full
capture requests still hash the current presented image. Capture proof is
cleared on launch/restart/return; dummy output cannot establish host proof.
Original-media tests preserve all RGBA bytes in five output sizes; a real
turn changes the explicit hash. Native Metal full-redraw measurement at
1920x1018 fell from about 90.5 percent to 31.7-32.1 percent of one CPU core,
with 62.5 rendered/presented fps and 4.14-4.21 ms average compose/present.
Physical Retina, GPU utilization and input-to-present latency remain open.

Movement stage diagnostics now identify completed source outcomes with a
per-session serial and optional performance-counter timestamp. --debug reports
source-outcome-to-Draw and successful Present-return timing for dequeued DM1
commands, with movement/turn/block flags. Fresh outcomes invalidate older draw
associations; retained-frame redraws do not duplicate the outcome report.
This timestamp is after the source/world effects, not at initial acceptance.

Full input latency remains open: retain one timestamp/identity through SDL
receipt, the pending host FIFO and actual source movement acceptance. Pointer
handling occurs inside m11_poll_menu_input before it returns. Do not equate
M11_GAME_INPUT_REDRAW with movement: inventory, cursor and resize also redraw.
Mark held samples and script dispatch separately from physical event receipt;
report blocked commands without attributing a later unrelated draw to them.
RecordPresentedFrame runs before Present and cannot prove host completion.
Successful Present return still does not establish physical scanout latency.

2026-10-15 HoC performance follow-up: precise host pacing reaches the
configured 62.5 full-redraw rate at 640x400 with original PC34 and dummy
output. Repeated-row reuse preserves every original-frame RGBA byte through
3840x2160. Before the diagnostic hashing correction above, native Cocoa
1920x1018 full-redraw CPU measured about 90.5 percent of one core. The applied renderer diagnostic confirms Metal and a 1x drawable
on this host; GPU utilization and physical Retina behavior remain unmeasured.
Measure remaining stage costs and received input, queue/accept, Draw and
successful Present-return timestamps. Existing dirty counters prove ordering
only. A GPU texture path must preserve capture dimensions and non-integer
sampling/hard edges. Ordinary V1 indexed HUD/TTF needs no high-resolution
RGBA overlay; the launcher's separate true-color text path must remain intact.

2026-10-15 secondary reference audit: ScummVM DM at
[`5eefba1a2cd412016e83a89eaae31376a95c9367`](https://github.com/scummvm/scummvm/tree/5eefba1a2cd412016e83a89eaae31376a95c9367/engines/dm)
is useful for Amiga 2.0 English and DOS 3.4 USA English comparisons;
its detector marks those editions as testing and Atari as unstable. It
provides no verification of our other edition owners. Follow-up checklist:

- Audit remaining material banks against `gfx.cpp:getGraphicIndex`
  (2026-2068): wall ornaments 121/259, floor ornaments 247/385 and door
  ornaments 303/441 are platform-specific bases. Verify against each
  owner's original metadata before changing a constant.
- Compare palette state transitions, including inventory return and darkness,
  with `gfx.cpp:drawViewport` (1952-2020): DOS VGA and Amiga palette switching
  are separate paths. Keep original program tables authoritative.
- Compare entrance background restoration with `dm.cpp:openEntranceDoors`
  (808-838). Its 31-step, four-pixel, delay(3) loop is a comparison case,
  not a universal timing rule for PC/Towns/A36. `DMEngine::delay` (242-248)
  approximates a vertical blank as 20 ms; do not replace original PC IRQ
  timing with this value.
- For the reported movement lag, compare input-to-compose ordering with
  `dm.cpp:gameloop` and `eventman.cpp:processCommandQueue/commandMoveParty`:
  accepted movement ends the input wait and the next loop composes the view.
  Native HiDPI/GPU latency still needs execution evidence.

Do not treat ScummVM screenshots as independent original-game proof. Its
DOS ornament route retains G0205 rather than native C696 for those draws,
its DOS portrait x88 differs from original C737's x96, and its shrink sampling
is not the source-verified PC Q6/early-native Q10 implementation. Preserve
our original-media oracles when comparing these paths. Detector MD5 values
cover a file prefix; they must not replace full-file authentication hashes.

2026-10-15 wall-ornament follow-up: PC34 native material selection and
C696 placement, plus early A20E/ST12 native bank/frame/palette routing,
pass independent original-data raster cases and the affected HoC replay.
Continue with native F20/A36 gameplay ornaments, early native door ornaments,
C127 backing for the other native owners and frame-aligned emulator comparisons.
These fixes do not close full viewport, native display or HiDPI verification.

2026-10-15 PC C127 follow-up: actual PC presentation now resolves its
native 48x43 backing through selected C696 without the former stretch or
D2 recolor. Retire the legacy shared front-mirror geometry/palette plan and
query API after other edition owners have their own verified adapters;
these compatibility tables are not authoritative PC presentation evidence.

2026-10-15 native-door audit: original ST graphic 558 matches the existing
G0196/G0207 mappings and both door palettes; the production native route
composes decorations before animation/cropping. No new decoder is justified
by that audit. Compare the Amiga metadata directly and broaden independent
decoration coverage before claiming all native door materials complete.

2026-10-15 F20 wall follow-up: native aliases, full-bitmap mirroring,
nine-wall C255 composition and JDM's y31 final C007 placement now pass
original-media pixel checks. Full Draw covers D0 and exposed D2L at the
authentic start and after one step, including both source program palettes.
Native viewport boundary and real mirror-edge clicks now pass; retain the
source F20 throw-zone audit separately. Keep other materials, inventory and frame-aligned
original-emulator comparisons open; this is not complete F20 viewport parity.

2026-10-15 early Amiga/Atari ornament follow-up: the lower-floor cyan/orange
rectangle is traced and corrected through bounded dungeon decompression
and native F0108 material routing. Original full-raster and actual-world
pixel checks pass. The same metadata fix restores wall/door ornament lists,
but door ornaments still require a native material source/pixel audit;
do not extend the floor result to full ornament or viewport parity.

2026-10-15 A36 C255 follow-up: all 30,464 original-media indexed pixels,
including outer and D0 walls, now pass an independent source-coordinate
oracle. Archive/directory CLI and M12 startup checks pass. Keep actual
gameplay viewport, audible output and original-emulator alignment separate
from this completed entrance-scene verification.

Completion scope: catalogue entries and authenticated files do not by
themselves prove complete platform support. Apple IIGS 2.0/2.1 now reach the
first world through direct CLI and M12 start-menu launch using the original
ProDOS owners; intro/entrance, audible DOC playback, and complete gameplay
remain open as recorded below. The test does not establish visual parity.
Sharp X68000 and SNES media archive names also exist locally; their content
and native runtime/startup gaps still need investigation. Do not infer playable
campaign support from file intake or claim all-platform support from this
bounded route.
PC-9821 support remains excluded by user instruction. Native Windows/Linux
and a physically active HiDPI display require separate execution evidence.

2026-10-15 PC34 near walls: source D0L/D0R callbacks and the unrelated
inventory poison-label fallback clear are corrected. Original full Draw
checks at (1,3,S) and after one real forward step verify both native wall
rectangles, source parity, complete VGA bank ownership and RGB palette.
Retain the first-view viewport follow-up: image0004's emulator pose is still
unproven. The exposed D2L discrepancy was traced to a native-PC guard excluding the
real catalogue route; both real startup routes now have original full Draw
coverage. The updated CLI start matches all 30,464 viewport RGB pixels in
image0004, but its emulator pose is still unproven. Keep actual original
pose and movement comparisons open; image equality alone is insufficient. Add
actual A36 D0 gameplay checks before extending this execution claim to
that edition. F20 HoC D0/D2L checks now pass as described above; remaining
materials still require verification. A20E/Atari passes do not establish full parity.

2026-10-15 A36 package ownership: desktop READY/preparation now require
same-owner authenticated graphics, VDEO.FTL and DUNGEON.DAT. Original bytes
for all three dungeon languages pass the explicit DUNGEON.DAT role; missing
or wrong drivers, another owner's driver and missing default files beside
DungeonF/G.dat fail admission. Implement and verify native language selection
from FILENAME.C (Dungeon~.dat) before admitting French/German-only filenames.
This package gate does not establish language routing or full visual parity.

2026-10-15 A36M entrance: the dedicated path now authenticates C696 and
same-owner VDEO.FTL, resolves the original door/viewport/transfer/button
zones, renders F0797's authored C255 scene, and uses the driver's fifteen
palette steps. Actual source widths and G2349=2 produce 64 opening frames,
not the previously assumed A20 count of 31. Original-pixel checks cover the
visible native C255 wall/ceiling/floor strips and all 65 closed/opening
pages. CLI archive, CLI directory and M12 captures agree at closed, first,
middle and final frames, with exact RGB4 channels. This does not establish
frame-aligned original-emulator parity, native audible output, or complete
runtime viewport/palette parity. Credits and Resume still need dedicated
interactive end-to-end evidence beyond the retained source control flow.

2026-10-15 native doors/stairs: A20E retail and Atari ST 1.2 now have 484
original-pixel material cases each for native stair/frame/door routing and
state clipping. The raw-square door-facing fix is shared across DM1; actual
PC34/A20E/ST12 original starts verify the differing square/animation axes.
A20E/ST12 command replays also reach the real stair view and operate the
button door after walking the original Hall. Extend execution coverage to
other catalogue editions and original-emulator pose comparisons. The cyan
rectangle in the stair capture was a floor item using the PC graphics bank,
not a champion portrait; the native F0115 correction now has original-pixel
and real Hall-walk coverage on A20E/ST12. Positive F0142 thrown-object
aspects now have native G0209/G0218/Y47 original-pixel coverage and real
champion-recruitment/waterskin-pickup/throw replays on both originals.
Native G0210 projectile selection and MEDIA720 PC34 kinetic-energy scaling
now reach the M11 viewport; unit coverage checks depth, maximum energy,
non-kinetic aspects and the two-unit scale floor. Original-media and
frame-aligned emulator comparison remain open. The actual A20E cast capture
still shows a separate cyan/orange rectangle over the
lower floor outside the corrected projectile: identify its owning draw
receipt before changing its material route. Atari's corresponding cast
capture does not show that rectangle. Complete native viewport parity
remains open.

2026-10-15 early Amiga/Atari runtime layers: original A20E background,
demo/Atari 1.2 native arrows, turn-button click strips and eleven MEDIA009
wall positions now have complete original-pixel regressions. Actual demo
and Atari dungeon captures show these layers. The bounded host cache
capability models native allocation ownership, not original hardware heap
accounting. Native A20E/ST12 door/stair material progress is recorded above;
verify other editions, remaining ornament routes, HUD interactions,
frame-aligned original captures and native audio. Early
Amiga base RGB4/copper-strip presentation is now original-data verified;
A36 external-driver bands and F0093 creature palette replacements remain
open. These targeted fixes do not complete all DM1 support.

2026-10-15 PC34 viewport followup: the specific D2 side-wall geometry and
fresh-key input delay have original-data/source evidence. These fixes do not
establish full viewport parity at every map/pose. The user clarified that
remaining wall artifacts and delayed wall presentation appear immediately
on entering the Hall of Champions, not only in distant D2 visibility. Keep
this first-view issue open and compare an authenticated matching original
pose; the earlier image0004 original capture has no proven map tuple. The
latest original-emulator capture attempt was blocked by active user input
and provides no new parity evidence. Debug performance: source redraws and compose costs are now counted, with an
original-media dummy SDL regression. A native M4/Cocoa PC34 initial-Hall
run reaches 60 source-draw/present FPS at density 1. HiDPI, other scenes and
platforms still require coverage; GPU utilization remains unavailable.

2026-10-15 repeated loose graphics: complete same-parent ownership is now
verified with original Amiga demo, Atari 1.2 English and PC34 English media,
including explicit orphan rejection beside a canonical original demo ZIP.
PC34 renamed original dungeon compatibility remains supported within the
selected graphics directory; duplicate aliases and unrelated subdirectories
are rejected. Other Atari/Amiga retail editions reuse existing catalog and
startup constraints but still need independent complete/orphan original-media
matrices. Amiga 3.6 and PC34 multilingual startup ownership need separate
source review. Detailed inventory rejection diagnostics and shared Amiga
startup metadata deduplication remain followups; no visual or audio parity
completion is claimed.

2026-10-15 PC34 loose startup: canonical and lowercase DATA/program spellings
now retain authenticated native TITLE and SELECTOR through CLI and M12,
including explicit graphics-file selection. Original-media HFSX tests reject
invalid/ambiguous local companions and unrelated parent fallback. Additional
retail editions, native Windows/Linux and display/audio parity remain open.

2026-10-15 loose-file startup: the authentic Amiga demo now passes CLI and
M12 using the same original bytes as its ZIP regression. The old scanner-to-
runtime failure is fixed, including the exact graphics filename handoff.
The original demo now also passes mixed-case Swoosh lookup and ambiguous
Swoosh/swoosh rejection on a case-sensitive HFSX volume. Original A22G retail
GRAPHICS.DAT/Dungeon.dat/DM/Swoosh also reaches source startup and runtime.
Original Atari 1.2 now passes unique upper/lower dungeon names via CLI and
lowercase via M12, rejecting ambiguity. The same original Atari files now
also pass unscoped M12 AUTO through the complete source title/entrance and
continued first-Hall runtime; the former handoff-only smoke remains separate.
The earlier two-second launch failure is not attributed to a specific cause.
Renamed loose graphics now pass the
original demo directory/file CLI and M12 matrix; original A22G also passes
selected-file M12 with renamed graphics. Other retail editions, filename
extensions/layouts and native Windows/Linux still need coverage. These checks
do not establish display/audio parity.

2026-10-15 I34E archive selector routing is verified for uppercase ZIP names
and wrapper directories through CLI and M12. Lookup uses the admitted graphics
member and retains its container/prefix. The original-media regression requires
selector completion as well as runtime, preventing another silent skip.

2026-10-15 FM Towns wall routing: the current implementation now selects
native F20 wall graphics and layout-696 crops for front and side wall host
handoffs, rather than stretching the LCR compounds to PC34 dimensions.
The route/crop and title unit checks pass and the application builds.
The complete authentic FM Towns archive CLI/menu, EDM/JDM startup, input
matrices and Hall-of-Champions handoff regression passed.
An original-media 720-frame boot reaches a loaded level and a nonblank
presented viewport (700,000 nonblack pixels, five colours). Runtime pixel comparisons
and remaining zone-blit routes are still open; this is not a parity claim.

2026-10-15 PC34 nested-ZIP I34E title fix: when starting from the authentic
`Dungeon-Master_DOS_EN.zip`, Firestaff found `GRAPHICS.DAT` under
`dungeon-master/dmaster/DATA` but looked for `TITLE` only at the archive root.
It therefore skipped the original I34E title animation. Runtime now derives
the sibling `dungeon-master/dmaster/TITLE` member from the authenticated
graphics path. The real ZIP run loaded the 12,002-byte original and presented
53 frames/265 source VBlanks in 3,786 ms; the nested-ZIP regression passed.
This closes the title-file lookup gap for this archive layout, not full
cross-platform intro or viewport parity.

2026-10-15 PC entrance-door source-offset correction: ReDMCSB
`ENTRANCE.C:790-824` prepares left-door C002 frames with 4/8/12-pixel
source shifts before `F0438` adds its frame offset; the prepared right-door
C003 frame shift cancels `F0438`'s source offset. Firestaff samples raw C002/
C003, so its prior left/right offsets produced the wrong moving door strips.
`ENTRANCE_Compat_GetDoorAnimationStep` now maps the raw left source to
`4 * animationStep` and the right source to zero. The focused composite
regression passed and checks the step-5 offsets. This fixes the identified
asset-coordinate mismatch; it does not establish native display cadence,
physical-window smoothness, or full title/entrance parity.

2026-10-19 DM1 PC movement/input follow-up: the authentic PC34 CLI regression
now runs two HoC recruits through SDL_PushEvent and the ordinary main-loop
event pump. `--debug` records `(80,3)` switching active champion 0→1 and
`(10,3)` switching 1→0 after source-space mapping; the current-source test
passes against the installed original PC34 ZIP. The selector's synchronous
Enter wait uses its existing test hook, while every subsequent key and mouse
input traverses the production SDL queue. This closes automated event-loop
coverage, not physical M5 Retina input/display verification. The DM1 V1 main
loop intentionally advances the source world at the original 200 ms tick
(five ticks per second); this is not evidence of movement interpolation or
scanout latency. Profile a real accepted movement on the target PC before
changing the source-owned viewport cadence. Viewport wall raster and motion
parity against DOSBox-X remain incomplete.

The I34E direct `--game dm1 --platform pc` route now stops at the closed
selector until Enter/E or a click on its original Enter hitbox; M12 New Game
remains the explicit Enter action. Its door animation already advances one
source VGA retrace per four-pixel step. On a 60 Hz display the 32 frames take
about 0.53 s, while the modeled 70 Hz PC source takes 0.46 s. Do not add
presentation frames without a new original comparison. Verify the native
mouse hitbox and first dungeon frame against a same-pose PC34 capture. If the
reported fast animation is a different door, identify that route separately.

2026-10-15 PC34 first-view comparison: the authentic `DM.EXE` and `DATA`
files from the user's English PC34 ZIP were run in DOSBox-X and compared with
Firestaff direct launch using the same archive. Both captured C007 pages are
320x200; 57,911 of 64,000 pixels match exactly, including the palette and main
corridor geometry. The original executable did not emit its map tuple, so
same-position parity is not established; the remaining differences lie around
C007 wall/floor edges. This startup-view sample does not establish full
gameplay viewport parity. The user's saved `display_brightness = 50` scales
presented RGB channels to one half; repeating with an isolated default profile
at 100 produces the same RGB palette values as the original capture. Thus the
dark colors in the saved-profile run come from its display-brightness setting,
not a PC34 palette-table mismatch. No user settings were changed, and original
media/captures remain untracked.

2026-10-15 FM Towns C255 entrance raster correction: the authenticated F20
source crops were being composed into the same framebuffer that the PC34
geometry-order pass subsequently drew over using its inert zero page. The
geometry walk now renders into a separate private buffer, preserving the real
FM Towns crop composition for presentation. The regression test requires
nonzero source pixels to survive that pass. The targeted viewport test passes,
and an authentic FM Towns `--boot-probe --debug` run reached `dm1-runtime`
with the expected DATA/GRAPHICS.DAT hash and a valid C255 receipt
(`requested=loaded=0x0000c433`, no missing records, viewport hash
`0x4fa3b1e8`, distinct from an all-zero raster). This proves the source-backed
entrance raster is populated; FM Towns viewport image parity still requires a
frame-aligned comparison against the original. No game data or captures are
tracked.

2026-10-15 FM Towns viewport capture test audit: the opt-in visual smoke test
captured an ordinary five-second launch, so its BMP could still show the
launcher scanning at 0% and pass by counting pixels across the whole screen.
It now captures a frame only after the authentic boot probe reports
`phase=dm1-runtime`, `startupActive=0`, `levelLoaded=1`, and the expected
FM Towns asset hash. The pixel check is restricted to the source viewport
rectangle `(0,33,224,136)` from ReDMCSB `COORD.C`/`DEFS.H`. This proves that the
runtime viewport is populated with rendered F20 scenery; it is a smoke check,
not a pixel-parity result. The full authentic FM Towns CLI/menu regression
with this opt-in check is running on original data. No game media or captures
are tracked.

2026-10-15 I34E TITLE palette audit: ReDMCSB `ANIM.C F8292_` dispatches to
VGA `VIDEODRV.C F8230`, which stores each source PL channel as
`(channel << 2) + 3` in the DAC6 palette. Firestaff had passed source channel
bytes to its RGB6 renderer; the first correction added the shift but omitted
the VGA offset. The bounded TITLE parser now validates 4-bit channels and
applies the complete VGA conversion. The authentic PC34 TITLE parser test and
native PC34 CLI startup passed against the original archive, including the
complete title and selector handoff. Title pixel parity still needs a
frame-aligned reference capture; cadence, entrance smoothness, and PC34/FM
Towns runtime viewport parity remain open. No game data or captures are added
to Git.

2026-10-14 parity investigation using authentic DM1 media and ReDMCSB source:
the FM Towns F20 title scaler is now ported from `BLTSHRNK.C` F0129/F0720
(6-bit fixed-point steps, centre-sampled source pixels). The expanded 18-size
pixel recurrence test passes on the Mac mini M4. Installed original
`DATA/GRAPHICS.DAT` records 75/76 were manually verified as 224x70 floor and
224x29 ceiling; M11 had mirrored the Towns floor using PC34's 97-row rectangle
beginning at row 39. The viewport now queries the loaded Towns graphic
dimensions, checks the selected bitmap against its flip-buffer capacity, and
copies back only the flipped source height. The native Xcode-configured
`firestaff` target build and the targeted viewport source-lock test pass. An
original FM Towns runtime frame comparison is still required before visual
parity can be marked complete. The I34E startup audit also confirms its DOS
parent runs ANIM/TITLE and SELECTOR before FIRES. Firestaff's authentic I34E
title/selector Enter path has since been implemented and is covered by
`dm1_v1_pc34_native_cli_boot` (see the records below); the earlier C001/F0441
gap statement is historical. Interactive legacy selector choices and the
first-runtime viewport comparison remain open. Neither investigation has
established full cross-platform visual parity yet.

2026-10-14 startup interruption: closing the host window during the source
SWSH prelude previously looked like a corrupt launch, although the same event
after the title was already handled as a clean exit. M11 now carries the
prelude's quit result through the DM1 selected-launch transaction and exits
without opening TITLE or Entrance. Authentic PC 3.4 and Amiga 2.0 regressions
inject a close after the first SWSH frame and check that runtime never starts;
their existing title-close checks remain in place. These SDL dummy-driver
checks verify interruption handling and source order, not native window event
delivery or visual parity.

2026-10-14 FM Towns title interruption fix on the Mac mini M4: closing the
host window after the first authentic EDM title frame previously returned
the same truthy result as a completed title. The caller then continued into
Entrance and reported launch failure. The title helper now returns the
explicit quit command for close/escape events, and the caller follows the
same clean application-exit path as the Atari title route. Quit checks now
cover VBlank waits and the final MASTER hold as well as the zoom loop. The
authentic `dm1_v1_fmtowns_archive_cli_boot` regression passed in 279.14
seconds with original English/Japanese FM Towns media; its injected M12 quit
asserts `launchedEver=1`, `active=0`, no launch-failed trace, and no Entrance
frames. This SDL dummy-video/audio test verifies the launcher control flow,
not native Cocoa close delivery or display parity.

The same close-during-title contract now covers the authentic Amiga v2.0
F0437 C001 zoom. A M12 launch closed after its first rendered zoom frame used
the supplied ZIP → ZIP → ADF preservation image; the regression passed in
`dm1_v1_amiga_v20_archive_cli_boot` and asserted a clean process exit with no
F0441 Entrance frames or launch-failed message. The SDL dummy-video/audio
route verifies cancellation and handoff cleanup, not native window events or
visual parity on Amiga hardware.

2026-10-14 issue #13 mixed-media reproduction on the Mac mini M4: the current
main build scanned the full staged DM1 data root, including FM Towns, PC EN/FR,
Atari ST and Amiga editions. With `--game dm1 --platform pc`, it selected the
PC 3.4 English graphics from `dos_extract` and loaded that edition's matching
`DUNGEON.DAT`; the boot receipt reported `phase=dm1-runtime` and
`levelLoaded=1`. This does not reproduce the reporter's Linux v3.0.380 failure
or prove a release fix. Issue #13 remains closed; the reporter explicitly
asked not to reopen it. The French DOS ZIP menu regression also passed after
its scripted Enter presses were spaced across startup frames (45.04 seconds).
The PC34 CLI regression now also stages the authentic loose release under
`~/.firestaff/data/dm1/dos_extract` beside the original sibling ZIPs, then
starts with `--game dm1 --platform pc` and no `--data-dir`. It passed in
373.10 seconds and asserted the selected PC 3.4 `GRAPHICS.DAT`, matching
`DUNGEON.DAT`, PC handoff, and loaded runtime. This guards the mixed-media
discovery route but still does not reproduce the reporter's Linux release.

2026-10-14 current-main runtime retest on the Mac mini M4:
`dm1_v1_pc34_native_cli_boot` passed in 356.45 seconds using authentic PC 3.4
media. It confirmed direct `--game dm1`, clean-config AUTO/start-menu launch,
and runtime handoff when the optional SWSH logo is absent. The same run passed
the C001 title, C004 Entrance animation, Hall route and scripted input checks.
GitHub issue #13 remains closed; its latest reporter comment compares
3.0.361 with 3.0.362 and later, while this test covers current main. No
released build containing the optional-SWSH fix has been verified by the
reporter. This SDL dummy-video/audio run does not replace a native Linux run.

2026-10-14 issue #13 version comparison: ran the published Linux x86_64
AppImages for 3.0.361, 3.0.362 and 3.0.380 in an x86_64 container under QEMU
on the local ARM64 Mac mini M4. The authentic PC 3.4 English GRAPHICS.DAT and
DUNGEON.DAT pair starts on 3.0.361, but 3.0.362 and 3.0.380 reject the same
pair when the separate `SWOOSH` file is absent. All versions reach runtime
when the original `SWOOSH` is present. ReDMCSB `APPA.C:51-68` ignores the
`FTL_SWSH` load result and continues to `FTL_ANIM`; therefore the English
PC 3.4 logo is optional. Commit `25fec5d7c` began treating a missing logo as a
failed launch, and `e7119f2ed` later also hid PC 3.4 English from the catalogue
when it was absent. Firestaff now admits the authenticated English graphics
and proceeds to the real dungeon while logging the skipped logo. The same M4
boot probe with only the authentic graphics/dungeon pair reached
`phase=dm1-runtime` and `levelLoaded=1`. The full
`dm1_v1_pc34_native_cli_boot` CTest passes on the Mac mini M4 with the original
retail archive (359.57 seconds), including the loose-file, missing-SWOOSH,
title, Entrance, Hall and input routes.
This reproduces the release failure under x86_64 emulation; it does not replace
a native Linux run on the reporter's machine. The issue status has not changed.

2026-10-14 chest-close evidence audit: ReDMCSB `CHEST.C`
`F0334_INVENTORY_CloseChest` (lines 79–132) clears `G0426` and rebuilds the
container chain from the eight visible `G0425` entries; the `CHANGE8_09_FIX`
media branch also clears consumed `G0425` entries. The existing
`firestaff_dm1_v1_chest_action_hand_close_runtime_probe` loads the selected
DM1 runtime and original graphics, then calls `seed_records` to replace
container 0, junk 0 and the champion hand with a constructed chest/item before
exercising the pointer route. It verifies source-pixel rendering and command
dispatch, but it does not verify closing an actual original-dungeon container
or edition-specific Thing ownership. Keep chest shutdown across authentic
editions open until a real loaded container is opened, mutated, closed and
checked through the normal M11 loop.

2026-10-14 native Cocoa retest on the Mac mini M4 with authenticated FM Towns
media: bare `--game dm1 --debug` selected FM Towns under AUTO and completed all
20 title frames. The title trace used PRESENTS, DUNGEON+MASTER, then MASTER;
frame 19 appeared at 423 ms and the complete title transaction took 1,641 ms,
including the configured 1,200 ms readability hold. A separate native
`--platform fm-towns` run accepted Return, presented all 31 entrance-door
frames at 16–18 ms intervals, and reached the first runtime frame at map 0,
party `(1,3)`, direction 2. Cocoa reported 1900×998 and 1920×1018
window/drawable pairs across the two runs, so this does not verify Retina
HiDPI output. The palette phase and frame cadence match the source in this M4
run, but a timing trace alone does not rule out perceived judder or verify the
exact release the user tested. The reported visual issue remains open pending
frame-by-frame evidence from that route.

The same native M4 build also completed `--game dm1 --platform pc --debug`
using the authentic PC 3.4 loose `GRAPHICS.DAT` and matching `DUNGEON.DAT`
under the installed `dos_extract` tree. It completed all 23 C001 title steps
in 4,045 ms and reached the first runtime frame. This verifies that this M4
data layout does not reproduce issue #13; it does not reproduce the reporter's
Linux host or close that issue's reported failure.

2026-10-14 PC 3.4 title timing diagnostics: `--debug` now emits all 23
source-owned C001 title events with requested and observed waits, presentation
time, total elapsed time and fast-forward state. The authentic PC 3.4 M12
CLI/start-menu regression passed on the Mac mini M4; its normal title route
measured 4,036 ms, with a maximum measured SDL present call of 3 ms and no
fast-forward. This confirms the tested host did not race through the source
waits. It does not verify Cocoa/Retina presentation on the MacBook Pro M5 or
compare the complete title frame and palette against an original capture.

2026-10-14 I34E startup link audit: ReDMCSB's `Toolchains/IBM PC/Source/I34E.LNK`
contains 36 objects and ends with `STARTUP2.OBJ`; it contains neither a title
nor an entrance object. In `STARTUP2.C`, the F0437 call is guarded by
`MEDIA274`, which excludes I34E, and F0441 is guarded by `MEDIA279`, also
excluding I34E. The I34E-specific `MEDIA707` branch calls F0435 directly and
ends the program if loading fails. `ENTRANCE.C:1280-1281` includes `TITLE.C`
only for `MEDIA514`, which also excludes I34E. At the time of this audit,
Firestaff still played its PC34 C001 title and C004/door animation through the
generic DM1 handoff. The authenticated I34E direct-launch route has since been
implemented as described below. Retain F0437/F0441 only on editions whose
startup source calls them. The previously logged PC34 title/entrance captures
remain useful Firestaff runtime evidence, not evidence that the original I34E
startup shows those sequences.

2026-10-14 fresh Mac mini M4 startup rerun against original media:
`dm1_v1_pc34_native_cli_boot` and `dm1_macos_real_data_boot` passed 2/2.
The PC 3.4 route completed its CLI, menu, Hall of Champions and input coverage
in 355.13 seconds; the Mac-specific authenticated media probe passed in 4.15
seconds. The runtime binary was built from `67b4731ca`; `6dd486e3e` adds only
documentation. Dummy-video coverage does not verify a physical Retina window,
native input or visual parity.

2026-10-14 Mac mini M4 HiDPI input regression: the current build passed
`m11_display_aspect_present_rect` and
`dm1_v1_hidpi_entrance_command_scale_gate_pc34_compat`. These checks exercise
Retina 2x and mixed logical/drawable dimensions, scaled presentation edges,
and the DM1 Entrance command hit regions against drawable-space input. They
verify the coordinate transform and source hit gate with controlled geometry;
the Mac mini's live drawable is not 2x, so physical Retina input remains open.

2026-10-14 native Apple M4 profiling with the authenticated Amiga v2.0
`DMGAMEG.DAT` save measured two ordinary gameplay windows and two forced
full-redraw windows. Without `--debug-full-redraw`, the main loop ran at
56.5–57.2 loops/s against its 62.5 target, while source-tick gameplay
rendering was 4.99–5.00 frames/s at about five game ticks/s. Process CPU was
12.4–13.6% of one core; frame compose/present averaged 21.12–23.57 ms and
peaked at 28.41–28.74 ms. With `--debug-full-redraw`, the renderer sustained
61.03–61.11 presented frames/s, 61.0–61.1 loops/s, zero over-budget loops,
and 16.28 ms average compose/present at 102% of one core. GPU utilization is
unavailable from the current SDL diagnostics. The Cocoa window and drawable
were both 1900x998, so this run does not verify a Retina 2x drawable. The
ordinary-mode cadence matches ReDMCSB `GAMELOOP.C`
`F0002_MAIN_GameLoop_CPSDF`, which draws the dungeon once per game-loop turn
and waits for input across up to 10 VBlanks; `VBLANK.C`
`F0577_VerticalBlank_Handler_CPSDF` advances that counter at display refresh.
Thus the five rendered frames per second follow the original Amiga loop's
cadence, while `--debug-full-redraw` measures host presentation capacity rather
than the game's intended update cadence. This does not by itself rule out
delayed input or animation regressions. The
authentic Hall of Champions startup route then produced three stable full-
redraw windows at 60.23–60.29 frames/s and 64.9–65.2% of one core. Its first
window included synchronous startup and Entrance time, falsely reporting a
multi-second main-loop stall. Debug performance windows now reset after a
successful menu launch so active-game timings exclude that synchronous work.
The native mouse-driven Hall interaction remains unverified.

2026-10-14 M4 real-media startup retest: `scripts/test_dm1_macos_real_data_boot.sh`
passed the authenticated PC and FM Towns CLI boot probes on the locally built
binary. It uses SDL dummy video, so this proves the source/runtime handoff, not
native window presentation, input or audio.

2026-10-14 Mac mini M4 PC-34 full regression:
`test_dm1_v1_pc34_native_cli_boot.sh`
passed with the original DOS English archive. The suite exercised default-root
discovery, `--game dm1`, M12 launch, the full Hall of Champions input route and
the complete keyboard/mouse input matrix. Authentic C001 first/mid/full zoom
frames and C004 closed/first-door frames were all captured and differed as
expected; Original and Modern presentation checks and the full-redraw debug
probe also passed. The test's default-root launch budget is now 30 seconds so
media scanning and the source-owned SWSH/title sequence can finish before its
runtime receipt is checked; a separate first-frame menu probe now allows 10
seconds for that same synchronous startup.

2026-10-13 current `c880f066a` M4 retest: the authentic Amiga v2.0 English
ZIP -> ZIP -> ADF CLI/menu route passed, including save resume, all 18 ordered
F0437 zoom presents, five palette fades, the 50 Hz source deadlines, all 31
F0441 door frames and first runtime movement. The title took 1,780 ms; door
intervals were 40-42 ms. The authentic FM Towns ZIP CLI/menu test also passed
for English and Japanese, including platform default selection without
`--platform`, 20 ordered title frames, the 1,200 ms MASTER hold, the entrance
cadence, JDM menu selection and the movement matrix. A separate native Cocoa
`--game dm1` run without `--platform` selected FM Towns and presented all 20
source title frames with PRESENTS, DUNGEON+MASTER and MASTER palette phases;
the source title completed in 1,661 ms with the 1,200 ms hold. Cocoa reported
a 1,920x1,018 window and drawable. The native run stopped at the interactive
entrance and does not verify native input, full Retina scaling or pixel parity.

2026-10-13 Amiga A20 F0437 source-wait follow-up on the Mac mini M4:
ReDMCSB `TITLE.C F0437` waits one VBlank and then PAL beam line 152 before its
first zoom blit; after the 25-VBlank title hold it waits at viewport bottom
before the MASTER blit and again before the final palette fade. Firestaff now
models those source waits, schedules the first zoom at the PAL line-152 phase,
and keeps each subsequent zoom deadline 20 ms apart. `--debug` records all 18
source deadlines and host-present intervals. The authentic-media Amiga v2.0
English CLI/menu regression asserts all 18 ordered presents and their cadence;
Amiga v2.0 English, French and German plus English v2.1/v2.2 archive startup
regressions pass on the M4. These use SDL dummy video and model the source beam
phase; physical beam timing, native 60 Hz frame delivery, HiDPI and pixel
parity remain unverified.

2026-10-13 Amiga A20 title pacing on the Mac mini M4: ReDMCSB
`PALETTE.C F0436` (`MEDIA433`) waits for VBlank between its eight palette
steps, and `TITLE.C F0437` gates each zoom blit on the source frame. Firestaff
previously waited 20 ms after presenting each fade or zoom frame, which added
the host renderer's VSync wait to the source interval. These events now use
absolute 20 ms deadlines, so a completed host presentation counts toward the
50 Hz interval. The authentic nested ZIP → ADF Amiga v2.0 startup test passes
on the M4 and now requires all five palette fades, a 150 ms final fade and at
least 1,600 ms for the complete title. Its M4 SDL dummy-video run measured
367 ms for the zoom, 501 ms for the hold, 168 ms for the final fade, and
1,721 ms total; all 30 Amiga entrance-door frame intervals measured 40–44 ms.
An authentic-media native Cocoa run on the Mac mini M4 then completed all five
fades in 1,698 ms: 366 ms zoom, 500 ms hold and 166 ms final fade. Cocoa
reported a 1,900x998 window and drawable on a 1,920x1,080 desktop capture, so
this run does not verify HiDPI scaling or pixel parity. The dummy-video timing
and native run also do not prove beam-line 152 timing; those M4 presentation
checks remain open.

2026-10-13 Amiga A20 F0437 source audit: ReDMCSB `TITLE.C` compiles the
`MEDIA009`, `MEDIA137` and `MEDIA413` branches for A20. The source performs five
palette fades (lines 114, 132, 173, 199 and 249), including a dark-blue fade
and screen clear after PRESENTS before the title zoom. The prior three-fade
implementation incorrectly skipped that source transition and retained
PRESENTS under the zoom. Firestaff now carries the authentic final SWSH palette
and indexed image into F0437, then restores the five-fade order and clears.
The authentic Mac mini M4 M12 CLI regressions pass for Amiga v2.0 English,
French and German media, plus English v2.1 and v2.2 media. The v2.0 English
regression requires five fades and at least 1,320 ms of title timing. Native
frame-by-frame visual parity remains open.

The earlier three-fade timing baseline (source commit `678a2d1e8`, then
`4db596e1e`) measured 552 ms for the 18-step zoom, 500 ms for `Delay(25L)`,
237 ms for the final fade and 1,943 ms overall. Those measurements confirm the
implemented waits only; the source audit above shows that the 3-fade sequence
was incomplete and those measurements do not verify A20 fidelity.

2026-10-13 Amiga F0441 door cadence correction on the Mac mini M4: ReDMCSB
`ENTRANCE.C F0441:233-236` waits two VBlanks after each door blit. Firestaff's
Amiga route had a zero-millisecond delay for those 31 door steps. It now targets
the source's 40 ms interval, accounts for time spent presenting the frame, and
reports each interval under `--debug`. The authentic Amiga v2.0 CLI/menu test
passes with all 31 ordered frames at 40-43 ms and reaches the first runtime
frame. This test uses SDL's dummy video driver, so native M4 door-frame delivery
and visual parity remain unverified.

2026-10-13 current-main retest on the Mac mini M4, source commit `e7119f2ed`:
the authentic PC 3.4 CLI/start-menu regression passed. In addition, a native
Cocoa run using the original English FM Towns archive presented the title
zoom through frame 10/20 with the corrected `DUNGEON+MASTER` palette; the
native window/drawable were 1900x998. This single captured frame confirms the
current palette presentation, not HiDPI scaling or complete physical-display
parity. SDL-dummy startup routes do not verify physical input or audible
output.

2026-10-13 local startup matrix on the Mac mini M4 host, source commit
`22bc77676`: the locally built binary passed authentic DOS English, Atari ST
v1.2, Amiga HD and FM Towns CLI/start-menu regressions
(`dm1_v1_dos_en_archive_cli_boot`, `dm1_v1_atari_st_archive_cli_boot`,
`dm1_v1_amiga_hd_archive_cli_boot`, `dm1_v1_fmtowns_archive_cli_boot`). The
FM Towns test also exercised its 720-frame intro/input route. GitHub Actions
run `38352901234` passed all jobs on Ubuntu, macOS and Windows. These local
routes used SDL's dummy video/audio drivers; they do not verify the M4's native
Retina window, physical input, audio output or visual parity.

2026-10-13 extended original-media archive matrix on the Mac mini M4, source
commit `458bf820e`: six CTest routes passed: PC 3.4 direct and nested ZIP
layouts, Atari ST 1.1/1.0b/1.0a preservation archives, and Amiga v2.0. Both
PC 3.4 routes exercised the long native-menu/input and presentation contract;
the Atari and Amiga routes reached their first runtime handoffs. All used SDL
dummy video/audio and do not establish native-window parity or audible output.

2026-10-13 UAE-1ADF intake: `firestaff_amiga_adf_visit_ofs_files` now reads
bounded UAE-1ADF tables, standard 11-sector OFS tracks and raw MFM tracks via
the shared MFM scanner. Its sector-presence map rejects duplicate sectors and
never turns the absent A22G sector into a zero-filled filesystem block; file
headers, extensions and data pointers must resolve to present sectors. The new
`test_firestaff_amiga_uae1adf_real_media` CTest reads the original A22G member
directly from the supplied solid 7z and verifies all 12 root files plus the
original hashes and sizes for `GRAPHICS.DAT`, `Dungeon.dat`, `dm` and `swoosh`.
It passes on the Mac mini M4. A byte-preserving ZIP transport probe recognizes
the shared German graphics/dungeon files as `amiga20-de`, then fails its
runtime handoff because the disk's `swoosh` is the French A20F hash rather than
the German A20G hash. This probe established safe source-file recovery and
kept the edition-authentication gate intact; a later exact A22G startup profile
and its direct-CLI/menu routes are verified in the 2026-10-20 entry above. The
in-process 7z scanner examines nested Amiga ADF members and retains
`archive::disk::file` provenance. Discovery from the complete mixed
preservation archive alone still does not select a unique playable disk.

2026-10-12 FTL decompression bounds: ReDMCSB `DECOMPDU.C`
`F0455_FLOPPY_DecompressDungeon` establishes the MSB-first token grammar for
the shared dungeon stream. The bounded reader now rejects truncated prefixes,
indices and literals instead of manufacturing zero bits after EOF. The new
`dungeon_decompressor_ftl_bounds` check passes, and the authentic Amiga 2.0
French dungeon/world startup regression passes with the change. This validates
one shared data-decoding boundary; it does not close the remaining gameplay,
platform or viewport work below.

2026-10-12 debug performance telemetry: `--debug` now enables the existing
in-game FPS overlay and emits five-second measurements for process CPU time,
configured loop target, actual loop cadence/over-budget loops, game-specific
simulation ticks, and successful game-frame presentations. Frame-composition
plus SDL present-call duration is reported separately; SDL does not expose
GPU utilization, so the log explicitly reports it unavailable. The opt-in
`--debug-full-redraw` diagnostic renders every loop to measure sustained full
frame composition and presentation capacity. Source timing
and host rendering must be compared separately: ReDMCSB `GAMELOOP.C`
`F0002_MAIN_GameLoop_CPSDF` and `VBLANK.C`
`F0577_VerticalBlank_Handler_CPSDF` show the original input/VBlank-driven
loop. GPU percentage requires platform tooling. A full build remains pending
because trv2 is not resolvable from this host and its macOS linker cannot link
against the installed 27.0 SDK.

2026-10-12 v3.0.379 startup regression retest: the released macOS arm64
binary passed authenticated return-to-menu and all-five-game rescan checks
for DM1 PC, Atari ST v1.2 and Amiga 3.6. Direct `--game dm1` exit behavior
passed with original FM Towns media. CLI diagnostics passed for default
per-game data discovery, mixed PC/FM Towns/unsupported PC-98 media priority,
and the original FM Towns and PC intro cadence. These runs use SDL's dummy
video driver; native M5 Retina input and pixel parity remain unverified.

2026-10-11 local startup retest: authentic FM Towns, Atari ST v1.2, English
DOS, and Amiga v2.0 New Game routes reached their first runtime frame. The
Atari ST v1.2 full CLI/menu/input regression passed. The full Amiga v2.0
CLI/menu/save-resume/input regression also passed against the authentic
ZIP -> ZIP -> ADF game and save media on both the local v3.0.375 build and the
packaged v3.0.376 app. The original save restored map 0, party `(4,15)`,
direction 2, four champions, and tick 292 through direct CLI and M12 Quick
Resume. This does not verify native M5 Retina window/input behavior or full
visual parity. The published v3.0.376 Linux x86_64 AppImage passed both
direct `--game dm1 --platform pc` and M12 New Game with authentic PC 3.4
media in a Linux/amd64 userspace under QEMU on a local ARM64 kernel. The
runtime receipt reports `levelLoaded=1`, the DM1 startup handoff, and the
first HoC frame. A later mixed-installation regression exposed and fixed an
edition mismatch between PC34 graphics and an Atari dungeon in the same data
root. Native x86_64 hardware and physical mouse input remain unverified. The separate
Linux emulated full-regression physical-card click failed, while a fresh
current-source macOS build passed that same scripted pointer route.

2026-10-11 mixed-root M12 regression: the same installed DM1 data directory
contained authentic English FM Towns, PC 3.4, Atari ST v1.2, and Amiga 2.0
archives. The expanded `dm1_v1_m12_auto_multi_platform_menu` test verified
AUTO -> FM Towns and explicit M12 -> PC, Atari ST, and Amiga selections
through their source-owned first runtime frames. Each receipt matched its
edition's original GRAPHICS.DAT identity and expected startup pose. The test
uses SDL's dummy video driver and does not verify native Retina input or
visual parity.

2026-10-12 follow-up: the mixed-root M12 test now also starts with AUTO and
uses keyboard navigation to select the authentic PC 3.4 platform card without
`--platform`; the PC graphics identity and first runtime frame are asserted.
The complete local FM Towns, PC 3.4, Atari ST 1.2 and Amiga 2.0 matrix passes.
Issue #13's authentic PC 3.4 CLI/menu/input regression also passes on the
current main binary. Native M5 Retina and physical-input validation remain
open.

2026-10-12 PC 3.4 title interruption check: the released v3.0.380 macOS arm64
binary, with the authentic PC 3.4 ZIP and an isolated launcher config, handled
a host quit injected after the first rendered C001 frame. The application
exited with `launchedEver=1`, `active=0`, and no launch-failure diagnostic.
The PC 3.4 real-media regression now covers the same interruption path already
tested for Atari ST French v1.3. This SDL dummy-driver check does not validate
native window-close or physical-input behavior.

2026-10-14 issue #13 retest: the complete `dm1_v1_pc34_native_cli_boot` route
passed on the Mac mini M4 with the original PC 3.4 EN ZIP and Atari ST v1.2
STX media. The production STX decoder supplied the authentic Atari
`DUNGEON.DAT`; when that file was placed ahead of the PC 3.4 installation, the
PC startup still selected its matching dungeon and reached the first runtime
frame. Direct CLI, M12 New Game, and the default `~/.firestaff/data/dm1`
installation also reached runtime. The menu scripts now leave space for the
source-owned intro before checking the runtime receipt. These SDL dummy-driver
checks verify data binding and startup flow, not native M5 input or visual
parity.

- Extend the authenticated Amiga F0437/F0441 title and entrance routes to the
  remaining editions after establishing each edition's original
  GRAPHICS.DAT/SWSH identity pair. A20 English, French and German now authenticate
  their own paired media and reach runtime through the original C001 title,
  RGB4 palette stages, authored zoom steps, C002/C003 doors and C004 entrance.
  Real-media CLI and M12 tests cover all three A20 language editions. Amiga
  2.1 and 2.2 English now authenticate the same original GRAPHICS.DAT/SWSH
  pair and dungeon data, then complete F0437/F0441 and reach runtime through
  both CLI and M12. The source archive labels both ADF dumps "Original (Not
  working)"; coverage uses their original files in ZIP transport wrappers and
  does not claim bootable ADFs. The authentic Amiga demo ZIP now authenticates
  its A20ED SWSH and graphics pair, renders the original C001 title, validates
  C002/C003/C004 entrance assets, and reaches the first runtime frame in CLI
  and M12 flows. Native renderer/input timing on each supported desktop remains open;
  these tests do not establish pixel parity or HiDPI behavior. The original A22 German 910,058-byte UAE-1ADF now passes file and
  startup admission. The bounded reader retains sector-presence metadata;
  no missing raw-track sector is synthesized. CLI and M12 tests run F0437,
  F0441 and reach runtime with the unchanged original disk. The strengthened
  M12 regression requires an active loaded first Hall through both ZIP and
  direct UAE-1ADF, with exact selected-file provenance; an accepted launch
  transaction alone no longer satisfies its menu checks.
  The four-file gate binds graphics 0679e39da9dcc2e855cb33c6c64ddcb5,
  dungeon 59e332cf47852d5c7c88a9538f0adcea,
  dm e8fc5da52d69aa61b8b5ac7b2e294ebd and
  swoosh 1038138978975415571a878bb08f54be from the same selected volume.
  Legacy two-file admission still rejects this pair; modified dm and missing
  dungeon also reject. ReDMCSB TITLE.C MEDIA413 includes A22G. Native display,
  audio timing and executable-state parity remain unverified; the archive's
  "Original (Not working)" label is not a native-port incompatibility result.
  A36M intentionally
  skips F0437: STARTUP1/STARTUP2 enter
  F0441 without calling the A20 title routine. A36M's exact GRAPHICS.DAT hash
  now selects the source F0466 decoder, and authentic C002/C003/C004/C005/C011
  media passes the real-media receipt test. Its CLI and M12 regressions now
  verify entrance completion and the first runtime pose. The ordinary data
  directory scan now discovers its ZIP -> ADF package as well as direct archive
  selection; an isolated original-media root exercises the M12 route.
  C696 now owns C427/C429/C430/C431/C432/C435/C436, including the native
  asymmetric doors and C011 overlay. Original VDEO.FTL supplies the
  fifteen-step palette increments; native source widths and the two-pixel
  G2349 advance produce 64 two-VBlank opening steps. The original-media
  crop regression and CLI/M12 phase captures pass. Original-emulator
  frame alignment, interactive Credits/Resume evidence, native audible
  output and full runtime viewport/palette parity remain unverified.

- Diagnose the 2026-10-06 MacBook Pro M5 dungeon report before changing the
  shared SDL presentation path. The attached screenshot's dungeon pixels are
  closer to the local DM2 FM Towns runtime capture than to the separate DM1
  capture, but the screenshot has no runtime metadata and is not sufficient
  to identify the game, platform or edition. The surrounding conversation
  points to DM2, so it is weak evidence for a DM1 regression. A separate
  identified DM1 launch is needed. Pixel-run inspection
  finds an approximately 10-device-pixel nearest-neighbour grid in the
  attached viewport, which does not by itself establish a Retina scaling
  defect. Capture the selected platform, asset identity, party/map pose and
  presented 320x200 frame from a DM1 launch, then trace the source-owned scene
  materials and projection for that exact edition.

- Verify full PC/F20 title playback from authentic GRAPHICS.DAT C001 media
  across each supported desktop target, including HiDPI presentation and
  event interruption after a rendered frame. The runtime now blocks Entrance
  unless all 23 C001 source steps finish. The production M11 path now chooses
  C001 even when the separate canonical TITLE animation is available, and the
  authentic PC3.4 menu test records the selected source and all 23 completed
  steps plus the source-owned SWSH, TITLE, ENTRANCE and full-graphics handoff
  receipts. The public `--game dm1` CLI probe now requires the same phases and
  first HoC frame. The mixed original-media AUTO regression also checks bare
  `--game dm1` against authentic FM Towns and PC 3.4 archives, requiring the
  FM Towns asset and EDM program identities. The opt-in real-media capture
  stores post-present C001
  first/mid/full zoom frames, closed C004 Entrance and the first door-opening
  frame, with only nonblank/content-change checks; it does not establish pixel
  parity. Atari ST Escape/window-close during C001/Entrance now exits
  through the launcher loop instead of being reported as missing/corrupt media;
  its authentic French v1.3 M12 regression now injects a host quit after the first
  rendered C001 frame and confirms clean exit before gameplay, then verifies
  the ordinary M12-to-runtime route. Cross-desktop HiDPI and live
  renderer/event-fault coverage remain open.

- The FM Towns title zoom palette correction has been visually checked on a
  Mac mini M4: native Cocoa capture at source frame 10/20 shows the red title
  details with the combined C13_DUNGEON/C14_MASTER palette. The connected
  display reports 1920x1080 and the game drawable 1900x998, so Retina/HiDPI
  scaling remains unverified. The native frame timing and entrance cadence
  were measured again against the corrected source order below.

  2026-10-13 source-order correction: ReDMCSB `TITLE.C:385-400` waits before
  each of the 18 zoom blits, then twice before `TITLE_MASTER` and once after
  it. Firestaff had placed the loop waits after zoom frames 1-18, which skipped
  the VBlank between `PRESENTS` and the first zoom and added an extra VBlank
  after the final zoom. The wait helper now places 18 waits after frame 0-17,
  preserving the source's 18 zoom intervals and two-VBlank MASTER transition;
  the final MASTER VBlank remains separate. The focused title unit test passes
  with Xcode 26.5's SDK, and the real-media test script passes shell syntax.
  A 2026-10-13 native Cocoa run of source commit `c023857b0` on the Mac mini M4
  used the authentic English FM Towns archive. The 20 title frames reached
  frame 19 at 396 ms; the complete sequence took 1,615 ms including the
  configured 1,200 ms MASTER hold. Zoom-frame intervals were 19-20 ms and the
  two-VBlank MASTER transition was 37 ms. After native Return at the closed
  entrance, all 31 door frames were presented 16-18 ms apart and the source
  handoff reached map 0 at `(1,3)`, direction 2, with zero champions. The
  native drawable measured 1900x998, so this is not HiDPI or pixel-parity
  evidence. A debug FPS sample accumulated about 104 seconds while the app was
  backgrounded waiting for Return; its 0.6 loops/sec is not an active-game
  performance measurement. The extra 1,200 ms MASTER hold remains a host
  readability choice beyond the source's 350 ms of waits.

  2026-10-13 current-source M4 regression: after rebuilding commit `65f564ae7`
  with Xcode 26.5, `dm1_v1_fmtowns_archive_cli_boot` passed against the
  installed original English/Japanese FM Towns archive (240 seconds). The
  test checked all 20 title and 31 entrance-door frames, source wait counts,
  stable 60 Hz intervals, the two-VBlank MASTER transition and 1,200 ms hold,
  both language/program routes, and the first runtime frame from M12. This
  confirms the corrected source cadence on M4 under SDL's dummy driver; it
  does not validate native-window frame delivery, HiDPI or pixel parity.

- Continue reducing M12 idle CPU cost if it can be done without losing frame
  rate, animation or input response. The shared loop now counts the whole
  16 ms frame period, including VSync present, and the 1920x1080 backdrop is
  cached for alternating frames while controls continue to animate. On 2026-10-13
  the Mac mini M4 native Cocoa run measured 59.5–59.7 frames/sec at a 62.5
  target and 72.6% of one core in the debug process; its window/drawable were
  1900x998. HiDPI behavior still needs a Retina display. The same build on SDL's
  dummy driver measured 58.2–58.3 frames/sec at 65% of one core. A prior native
  v3.0.380 sample measured 44.8–47.1% of one core but did not measure cadence.
  GPU utilization is not available through this report.

- Reproduce and correct the remaining reported retail HoC interaction/viewport
  set as one source-locked pass: presented-frame evidence for object-bearing
  Eye scroll reads, Vi altar wall material, stairs occlusion, complete door
  composition, pressure-plate near/far visibility, wall-torch placement, and
  held-item striping/loss on inventory placement. The authentic PC 3.4
  runtime regression now follows normal movement and C127/C040 recruitment to
  HoC (4,15), picks up the source WATER through a rendered C080 mouse target, and
  places it in backpack slot 1. It then picks up scroll 0 through C080 and
  holds Eye through separate production
  press/release calls. It now captures the presented C101 panel before,
  during and after Eye, matches opaque C023 source pixels and M653 text ink,
  and checks that the FOOD/WATER page returns unchanged after release. This
  proves the pickup-to-inventory and source-material presentation paths,
  but not DOS pixel parity or the other live combinations. Existing raw-format pointer, F0128
  scheduler, and individual material tests are necessary but do not prove
  these live combinations. Use an authenticated PC3.4 HoC route or a
  generated-in-original-runtime C13 save as the state driver; do not promote
  a hand-made save or substitute graphics from another platform. Capture the
  exact square/pose/input and compare source-order command receipts before
  changing a renderer or inventory transaction.

- Complete the validated original PC 3.4 same-state C127/C040 comparison
  against authentic DOS capture through a real pickup and inventory. The
  native PC3.4 regression now proves WATER pickup and backpack placement with
  production inputs, decodes Eye-held scroll text 33, and checks C023/M653
  Eye-held pixels, but the original-media capture route has not established
  native-vs-DOS
  inventory parity. A prior private capture set
  with three bit-identical Hall
  frames must not be used as source-modal proof. The separate private retail
  route visibly contains a C040 panel followed by C007 inventory state. A
  same-state native C040 measurement is now recorded, but it does not prove
  C007 or item-transaction parity. The historic retail hybrid
  route is mouse-mode launch (`DM -vv -sn -pm`), but `(260,84)` is the
  Resume row, not a verified Enter selection. Current DOSBox-X/Xvfb runs
  reproduce the keyboard Return handoff into the no-party Hall frame while
  source-space mouse-button transitions remain unproven.
  keypad-Enter (`0x001c`), an approximately eight-second door animation, and
  the 28-command `KP5/KP1/KP2/KP6` sequence before the C127 click `(112,83)`.
  Use the visibly distinct source C040 geometry to compare material/cadence
  to the same-state native frame, and then extend it through a real inventory
  interaction. The native PC 3.4 CLI regression now confirms one original
  C070 forward-arrow click moves the retail fresh party from `(1,3)` to
  `(1,4)`. A dedicated all-mouse Hall movement route remains desirable, but it
  must not replace the verified keypad sequence or be inferred from it. Keep
  raw captures operator-local under `.codex-scratch` and do not substitute a
  save, graphics, or a Firestaff-native result for this evidence.
  The authenticated route's recorded `(22,14)` click keeps the dungeon page
  visible in both the original capture and current native run; its historical
  `inventory` screenshot label is therefore not proof of a C017 transition.
  A later authenticated PC 3.4 route establishes C017 and a labelled
  empty-hand Eye hold after the C127/C040 recruit flow. The source Eye hit
  area `(20,53)` produces a verified held/released visual delta; the earlier
  `(24,77)` coordinate was a rejected cursor-only diagnostic. It does not
  establish a held object, an object-name/scroll read, or any
  pickup/placement result; those still require a real object-bearing route.
  A 2026-09-09 headless DOSBox 0.74 retry with
  the prior PC 3.4 stage reached the authentic Entrance screen but did not
  deliver its subsequent Hall keypad/mouse commands in either global or
  window-addressed X11 mode; its raw frames remain capture-health diagnostics
  only and must not be relabelled as C017. Reproduce this route in a DOSBox
  input environment that records the original command consumption before
  comparing C017 or pickup pixels.

- Add an original-media trace that covers F0205's G0395/G0396 half-pair owner
  through a same-tick map transition and slot reuse. The native transient
  source-slot implementation is covered by bounded C38 dispatch and lifecycle
  regressions, but has no authentic emulator trace yet.

- Validate the remaining attack-entry C32-C36 source decision tree. The raw
  active-group dispatcher already routes C32-C36 through F0179/F0208 and the
  clean group-timeline and creature-AI regression targets pass (2,609 checks);
  do not duplicate that logic in a second runtime path. Audit C32's lack of
  an individual slot without reproducing an out-of-bounds Aspect[-1] read.
  F0179 now carries its PC3.4 Couatl movement and Animated Armour ongoing-
  attack sound gates through committed reaction/fanout dispatch
  (GROUP.C:242, 270-280). F0183 map-entry initialization still has no
  result-emission handoff, and authentic live captures must validate sound
  priority/arbitration; do not replace these with a generic audio effect.
  Remaining integration requirements:
  - Audit remaining attack-entry transitions beyond C31-C37 with their
    own deletion/delay rules. Obtain turning-pair
    and emulator-trace coverage beyond the four already-facing slots.
  - Obtain original traces for approach entry and zero-tick chained dispatch.
    Preserve off-map handling and the separate priority gap.

- Extend native-save coverage from a fresh F0195/F0180 C37 admission to an
  authenticated original-media save-and-resume route. The G0377 regression now
  verifies the exact source-shaped C37 case, unique C04 ownership through the
  square chain, GLOBAL_DATA/ACTIVE_GROUP round-trip, and rejection of an
  unauthenticated C36 reaction. Other runtime-generated C29-C41 events still
  require an imported C3/C4 receipt; audit their original union ownership
  individually before widening export. The staged French DOS save corpus is
  absent here, so the focused regression does not establish original-media
  save/resume parity.

- Complete F0183/F0180 admission fidelity beyond the implemented F0179 aspect/RNG
  initialization and successful staged RNG publication. Obtain original traces
  beyond the bounded multi-group scan-order and partial-admission rollback tests.
  Check Couatl sound emission and edition-specific metadata beyond I34.
  F0180:333 event priority (255-MovementTicks) still needs integration.
  Timeline scheduling currently sorts only by
  time and preserves insertion order on ties; TIMELINE.C:143-151 requires
  time, descending original event type, descending priority, then source-slot
  order. The 44-byte serialized event has no dedicated priority/slot fields.
  Audit all event mappings and persistence before changing this shared queue;
  adding only a C37 priority would not establish original ordering.
  Do not treat the seeded two-event regression as
  proof of this complete admission sequence.

- Extend the new shared prior-square history to full original-runtime
  capture verification: compare consecutive live M11 wander moves and exact
  RNG with original execution and deferred
  removal/re-admission. Unadmitted legacy worlds still lack source history.
  Bounded C29/C37 two-move, staged map-transition rollback and lifecycle
  evidence is recorded in DONE-dm1.md; full original map-transition traces
  remain unverified.

- Extend F0412 RNG verification to M11's prevalidated XP/gate handoff and
  original execution traces. M10 XP/practice extraction is now corrected to
  BASE.C F0027's shifted sample; the shared multiplier matches BASE.C, not
  the distinct CEDT002 routine. The bounded fixtures and five original DOS
  media integrations do not establish complete cross-platform stream parity.

- Extend verification of newly allocated fixed drops through F0267's off-square source path.
  GROUP.C F0186:643 passes source X=-1; retain this distinction without relaxing
  source membership on `F0267_MOVE_MoveThingOnLoadedChain_Compat`.
  Pair the bounded direct-pit and teleporter/plate regressions with original
  captures; include multi-level falls and sensors at the landing square.
  Successful floor linking or event publication alone is not full move parity.
  Preserve failure ownership: the existing loaded-chain mover restores a
  failed destination link to its source, which is impossible for X=-1.
  A reserved-drop path must return the unlinked allocation to its caller
  without linking it to an invented source square. Audit destination sensor
  side effects on link failure before claiming transaction atomicity.
  Existing test owners to extend are `test_csb_v1_f0267_loaded_chain_pc34_compat`
  and `test_dm1_v1_f0267_local_sensor_rotation_pc34_compat`.
  Keep the complete reserve/RNG/publish operation shared by M10 and M11.
  Do not expose an unchecked
  public "fresh Thing" handle: the current ordinary-record validator accepts
  C05..C10 without checking raw ownership, and Next=END also describes existing
  floor/inventory tails. Keep source membership required on the existing public
  loaded-chain move API.

- Finish F0186 allocation/RNG ordering across runtime drop materializers.
  M10/M11/CSB use the streamed helper; CSB drops now share the F0190 caller RNG.
  Audit upstream RNG owners and nested sensor RNG effects beyond the verified
  fixed/carried-cell sequence and direct rollback observation.
  Extend M10 death-path RNG and reclamation evidence. See
  `docs/parity/DM1_FIXED_DROP_ALLOCATION_ORDER.md` for the confirmed mismatch
  and the required exhausted-pool regression matrix.

- Extend explosion F0191 verification to possession drops and original
  death-smoke captures across supported platforms.
  Nonlethal RNG/raw writeback, survivor compaction, whole-group retirement
  and source C15/C25 smoke publication are verified in bounded fixtures;
  they do not establish full death aftermath or emulator parity.

- Pair source-projectile Fluxcage passage with original emulator captures.
  The raw C14/C15 RAM regression and F0219 source audit do not establish
  full endgame or cross-platform parity. Extend real Lord Chaos/fusion
  coverage beyond F0221 lookup and source-owner metadata checks.

- Complete projectile occupancy parity for destination squares and active
  group overlays. The F0176 selector currently receives decoded group cells
  and uses the normalized representation documented in
  `docs/parity/DM1_ACTIVE_GROUP_CELL_OWNERSHIP.md`. Resolve raw C04 byte-5
  ownership before introducing an active-index dereference. It also receives
  the decoded primary direction; verify F0145/F0147 active-map overrides with original
  captures. Source C14 landing occupancy is now deferred to the next event,
  using its raw source-square lookup rather than active AI rows. Expand
  timing verification to party landings, teleporter exits and original
  emulator traces; audit legacy host-only destination collision separately.

- Extend F0304 reachability analysis beyond ordinary defined PC 3.4 paths.
  The source bounds in `parity-evidence/dm1-pc34-xp-award-reachability.md`
  rule out scaled-word overflow for the examined normal PC 3.4 awards,
  even at format-maximum difficulty. BUG0_81's uninitialized damage path
  needs original binary evidence, not a deterministic invented oracle.
  Extend the verified original-media Mon Light difficulty-6 integration to
  original input/presentation captures and other casts/attacks; primitive
  word-width tests are not naturally occurring original fights.
  Extend source-edition XP arithmetic verification to authentic Amiga 3.6
  startup and original boundary traces. Unknown identities retain the legacy
  signed default; add explicitly evidenced edition mappings rather than
  inferring signedness from graphics format. Audit remaining standalone legacy
  helpers and maintain common policy for all live query/award consumers. See
  `docs/parity/DM1_SKILL_ACCUMULATOR_AUDIT.md` for implementation and evidence.

- Extend rune lifecycle verification to original DOS/FM Towns media and
  rendered symbol rows after fourth-rune wrap/recant. Atari/Amiga input
  and debit tests do not establish visual parity for every platform.

- Extend spell-panel parity beyond the tested DOS/Atari/Amiga static
  panels and six successful DOS light-cast cases. Verify failure frames,
  other spell types, rune wrap/recant transitions and same-tick serial
  replacement; retain distinct edition-specific rendering and evidence.
  Add FM Towns spell pixel tests with authentic C009-size admission and
  edition registry evidence (English EDM.EXP regions already documented).
  Japanese caster names require TEXT2.C F0952, not M653 ASCII glyphs;
  verify JP registry bytes and that text path separately. Do not classify
  little-endian FM Towns as early Amiga merely due to its legacy loader.
  Original emulator/input captures and complete all-platform HUD parity
  remain unproven. V2.2 retains its explicitly permitted alternative art.

- Confirm the cause of the intermittent no-output early exit in the French
  Atari original-media test observed on September 6. The current M11 runtime
  options initializer sets `menuRequested` to zero, and its nearby
  comment says leaving it uninitialized previously let bare `--game dm1`
  fall back to M12. This is a plausible match for the old incident, not a
  verified historical fix: `c61f5aa62` is the repository's imported root
  snapshot, so Git contains no prior version or corrective diff. Against the
  authentic French Atari ST 1.3 archive, the current direct CLI/M12/runtime
  CTest passed four consecutive times on 2026-10-13. Do not conflate this
  startup issue with the separate French Atari SND1 source-index-1 rejection.

- Verify all VI wound/power combinations; ten-retry exhaustion, individual
  wound bits on alternate UseItem and combined wounds on mouth are covered.
- Investigate intermittent French Atari SND1 source-index-1 rejection in
  original-audio setup, observed before the restorative tests. Full rerun
  passed; do not conflate this with a fixed failure or the no-output exit.

- Extend original-media combat parity beyond the current PC 3.4 live route.
  Its authentic first-door transition now runs NEWMAP.C F0003 on every DM1
  runtime platform, activating original map-1 group 169 and its C37 behavior
  event instead of leaving the later map without active groups. Group moves
  now synchronize the C04 next-Thing link to both the decoded owner and raw
  source record, allowing F0209 to authenticate and continue later C37/C38
  events. The PC34 test verifies that the real group approaches and damages
  the party, then uses production pointer input for PUNCH and checks equal
  Ninja/Fight XP awards against MENU.C and CHAMPION.C. Authentic Amiga v2.0
  now has a separate M12 launcher-route regression through Hall selection,
  the first-door transition and group 169's live attack, including player
  PUNCH and paired XP. Exact damage/RNG still needs an original executable
  capture; Atari/Towns combat routes need equivalent original-media tests.
  Extend startup sentinel proof to remaining
  editions and actual launcher UI routes; completed regression evidence is in
  DONE-dm1.md.
  English and German Atari ST 1.2, French Atari ST 1.3, and English Atari ST
  1.0a, 1.0b and 1.1 reach `dm1-runtime` through M12 with first-runtime
  receipts at `championCount=0`. All six Atari ST editions now follow the
  normal M12 route to authentic C127 ordinal 14, confirm its C040 choice, and
  accept a gameplay turn after the panel closes. Authentic-media checks cover
  English 1.0a, 1.0b, 1.1 and 1.2, German 1.2, and French 1.3; the PC 3.4
  choice and gameplay route is also verified. See DONE-dm1.md. The remaining
  gap below is proving the complete source-owned campaign-start transition,
  not Hall selection or immediate gameplay input.
- Verify the remaining Atari ST new-game campaign and sensor effects beyond
  the now-covered initial party placement, Champion Hall selection, and
  first-turn checks on the normal M12 route. German Atari ST 1.2 presents authentic
  source pixels through M12: ReDMCSB DEFS.H MEDIA020 binds floor/ceiling records
  75/76, and DATA.C's six Atari RGB3 dungeon palettes are installed in M11.
  A captured 320x200 first runtime frame has 17,773 nonblack pixels in six
  colours; this is a visible-frame smoke check, not visual parity. M12's first
  runtime receipt starts with `championCount=0`; its normal menu route then
  reaches the adjacent tile for C127 ordinal 14, opens its source candidate
  panel, confirms C040, and accepts a following gameplay turn on English v1.2,
  German v1.2, French v1.3 and English v1.0a, v1.0b and v1.1. The source-owned
  campaign-start transition remains open beyond its first runtime frame. The
  authentic English v1.2 M12 regression now asserts the ordinary launcher path
  crosses F0441/F0435/F0462 to `dm1-runtime`, with the source party at map 0,
  (1,3), facing south, `championCount=0`, and `dm1StartupHandoffExecuted=1`.
  The production M11 handoff calls its dedicated party-placement branch; the
  ordinary Thing-chain F0267 helper's rejection of PARTY/GROUP does not apply
  to that branch. Source-order reads of DUNGEON.DAT from all six authenticated
  Atari ST editions correct an earlier C04 claim: map 0 (1,3) is raw square
  `0xb0` in each, with an
  inactive C01 teleporter (`0x0441`) followed by two C008 party-possession
  sensors (`0x0e14`, `0x0e12`). Both sensors require item 27; the fresh party
  has no champions or hand object. ReDMCSB MOVESENS.C F0267 therefore does
  not teleport the party, and F0274/F0276 produce no remote sensor effect.
  The fresh-start receipt and party tuple now pass on all
  six authenticated Atari ST editions. The real-media M12 probe now records
  the production F0267 placement receipt and its F0276 sensor-effect count;
  all six editions reach map 0 (1,3) with zero applied startup sensor effects.
  The identical source chains explain the zero-effect startup receipts.
  Later campaign sensor effects still need original-runtime evidence before
  claiming complete startup behavior.
  M11 now preserves the source F0276 C003 party-floor-sensor gates: empty
  parties do not trigger it, and nonzero sensor data must match the party
  direction. F0276's C006 group-generator and C007 floor-creature sensors are
  skipped during party walk-on; the timed C006 generator event remains
  separately dispatched. Focused regressions cover these gates. The authentic
  English v1.2 menu-to-HoC
  route passes after the changes, but does not verify every later sensor
  type/effect. Do not invent champions or
  treat the partial floor view as complete visual parity.
  ReDMCSB STARTUP1.C:162-174 runs F0441, retries F0435, then calls F0462 and
  places the party when `G0298_B_NewGame` is set. `DUNGEON.FTL` is only used by
  LOADSAVE.C's optional custom-dungeon path; its absence from standard STX
  disks does not block built-in new-game startup. The authenticated English
  v1.2 STX used by launch tests has only
  BOOTER, SWOOSH.IMG, START.PRG, GRAPHICS.DAT, START.PAK and DUNGEON.DAT in
  its root; the supplied German v1.2 and French v1.3 STX roots likewise
  contain no `DUNGEON.FTL`. Clean v1.0a/v1.0b and v1.1 STX roots were also
  verified without that campaign file. Atari ST 1.0b and 1.1 have authenticated
  `DUNGEON.DAT` admission and now recruit C127 ordinal 14 through normal M12
  input after their zero-champion first-runtime receipt. The matched v1.0a STX
  does the same after its existing M12 profile's
  `ebccb5f99c4437adcb34d9228b57eb6a` hash was added to M11. The Atari v1.0
  software archive also contains an authentic MSA Automation Disk whose root has
  `DMGAME.DAT` and `DMGAME.BAK`
  (47,710 bytes each). Both pass the original-save header checksum and classify
  as FormatID 1 / `ORIGINAL_DM1`. The primary `DMGAME.DAT` also authenticates
  all five F0435 save parts: 128-byte GLOBAL_DATA, 60 x 16-byte active groups,
  a 3,328-byte four-champion PARTY, 463 x 10-byte events and a 926-byte
  timeline. Its source state is four champions on map 2 at (11,14), facing
  direction 3. The Atari tail starts at byte 10,484 and is 37,226 bytes. Its
  big-endian F0434 header reports 14 maps, ornament seed 99, 12,366 raw-map
  bytes, 1,750 text words, 1,968 square-first-thing words, and the original
  thing counts. The 14 Atari map descriptors yield 412 cumulative columns;
  using the original thing-record widths, those sections account for exactly
  37,226 bytes. This structurally authenticates the v1.0 F0434 stream; this
  early edition has no appended F0422 dungeon checksum. The Atari runtime
  runtime adapter is not implemented. The new read-only Atari F0435/F0434
  receipt authenticates all five primary-save parts, DM1 campaign identity,
  the saved party coordinate against its map dimensions, and the exact v1.0
  tail length. Passing the original save to
  admitted v1.2 STX yields the same zero-champion fresh start as no save. Keep
  this save as a verified source-part candidate, not a verified playable
  runtime; F0435/F0434/F0436 Atari save import remains separate from new-game
  campaign startup. Independent F0435 part checksum matches are 128-byte
  GLOBAL_DATA, 960-byte ACTIVE_GROUP, 3,328-byte PARTY, 4,630-byte EVENTS,
  and 926-byte TIMELINE (the Atari v1.0 EVENT record is 10 bytes). The
  authentic automation-disk MSA SHA256 is
  `b11ca8a124b574738a243fbeabc30567071e63e640b6289bc55315cc27c4d859`; its
  matched v1.0 retail STX SHA256 is
  `d9588480091d3aca753d86efedf0f2336871c817138a7eaf900d1afb4ed0f9ed`.
  Save hashes: DAT
  `728682a977fa49a8a3dd9e3afc77afb2a90e02604234edfc19b8495e937e0fa7`, BAK
  `ad009d608ae843ca3e014af2f5bb291ee9e76002824c5c1af6a12c0606aa3353`.
  The backup passes its header, GLOBAL_DATA, ACTIVE_GROUP, and TIMELINE
  checksums, but its PARTY and EVENTS checksums do not match; the Atari receipt
  rejects it at PARTY. Do not offer it as a playable fallback.
  ReDMCSB LOADSAVE.C F0435 reads FormatID 1 without platform or dungeon ID,
  authenticates the five parts, then reads the Atari F0434 tail; the legacy
  DM1 detection uses 14 maps and ornament seed 99. The existing Firestaff Amiga
  format-5 adapter is not interchangeable: Atari v1.0 uses different champion
  records and does not preserve the Amiga header metadata. Until the actual
  Atari tail, four champions and runtime queues all pass an Atari-specific
  transactional adapter, resume must fail closed rather than silently starting
  a fresh party.
- Resolve the C006 generated-group cross-map teleporter path before promoting
  it as full parity. The source fixture contains raw C01
  (`00 0c 22 a0 00 01`) and now reaches the correct target map 1 / 2,1,
  preserving the source chain, target linkage, inactive cross-map state and
  target buzz. The remaining local regression mismatch is narrower: two
  assertions still expect the legacy host `CREATURE_TICK` label for the
  scheduled C37 wander. The native timeline represents C37 as
  `CREATURE_REACTION` with `aux2=C37`, which is also what its dispatch and
  save conversion consume. Reconcile that test expectation only after an
  original trace confirms the event-type mapping; do not relabel the runtime
  merely to make an old test pass. Continue to compare GROUP.C F0185:542-543,
  MOVESENS.C F0267:439-440,524 and F0262:76-83 with original execution.
  DUNGEON.C:1276-1278,1303-1305 selects ACTIVE_GROUP solely by party-map
  equality, so neither a decoded fallback nor reordered insertion is
  justified without further evidence. Obtain a runtime capture for this exact
  generation case.
- When savegame work resumes, inspect the legacy lifecycle probe's A5/A6
  size expectations (208/872) and J4 all-fields-max serialization failure
  against magicMapRefresh and the serialization contract. It runs against
  original French DOS DUNGEON.DAT but is not a passing full probe. Keep
  this separate from runtime XP verification; serialization remains deferred.
- Extend F0412 evidence to final earned XP, low-wisdom failure trajectories,
  and non-potion low-skill effects. Successful restorative casts at skill 1
  now preserve the original practice/potion RNG sequence; the full failure
  and effect matrix is not yet proven.

- Establish edition-appropriate food-command clocks and presentation waits
  for paths not covered by the verified top-level I34E archive binding,
  including loose/nested packages and other platforms. See
  `parity-evidence/dm1-consumption-timing-audit.md`; source-edge and audio
  queue tests do not prove wall-clock or raster-phase parity.

- Verify swallow timing/output against emulator captures and load changes through actual
  pickup/equipment input and presentation feedback before claiming complete
  consumable parity. Extend the unchanged-skill assertion beyond antivenin.

- If early S1.0/S1.1 media are enabled, dispatch their distinct
  F0348 thresholds/caps rather than applying the PC3.4/S1.2+ rule.

- Verify the now-separated body/elemental shield layers through actual
  M10/M11 attacks with original YA and independently derived damage.
  See `parity-evidence/dm1-shield-damage-layer-audit.md`; primitive tests
  and consumable tests do not establish full combat parity.

- Verify original YA potion combat-defense integration;
  recipient shield gain, scheduled ownership and live C72 expiry now have
  real-media tests.

- Extend the inventory-open idle-tick regression to other damage sources,
  torch/status expiry and Modern presentation modes.

- Extend poison-triggered death coverage to cross-map events and death
  with multiple surviving champions; final death through the timeline is covered.

- Extend death spell-input coverage to actual timeline dispatch;
  verify final-death UI and redraw against
  the original. Savegames remain deferred.

- Extend cross-owner consumption to original stat/healing/shield potions.
  Original antivenin cancellation and other-owner preservation are covered.
  Capped/uncapped waterskin depletion, empty rejection, hand retention and
  leader held weight pass on five editions. Check potion recipient stats,
  consumed Thing removal, event ownership and release behavior.

- Complete the broader regression of the now-enabled inventory/leader
  separation: full original-media corpus, application startup, Modern input,
  chest-restoring eye release, top-row highlights, death/revival and any
  remaining direct activeChampionIndex consumers. Verify cross-owner
  scroll/chest close/reopen sequences through normal input, not just explicit
  owner fixtures. Save persistence remains deferred.

- Verify leader/load transitions through actual death and resurrection,
  including a party with no living champions; controlled zero-health
  selection rejection does not cover the lifecycle.

- Extend load verification beyond floor-drop and second-champion isolation:
  Modern composed-HUD leader changes, cross-champion exchanges, full and partially emptied
  open chests, mutation rollback and weight-changing consumption/spells.
  Audit mutation paths that do not publish through m11_refresh_hash and
  compare platform-specific weight values against original media. The
  current load regression shares the F0140 weight decoder with the engine.

- Extend Atari/Amiga original-object coverage to occupied equipment and
  backpack exchanges and complete drag sequences. Independently verify
  source slot geometry and
  archive-to-normalized-dungeon decoding; those remain shared with the
  current empty-slot admission oracle.

- Verify FM Towns scroll baseline/line placement against its own
  F0341/F0644 paths and original layout data. PC3.4 and the tested
  Atari/Amiga editions now have source/data raster evidence; same-state
  emulator captures remain necessary for independent full-render parity.

Reviewed 2026-08-29. Only open work is listed here.

- Deferred savegame gap: native quicksave does not serialize dungeon Thing
  tables; dm1_v1_original_save_pc34_handoff_adopt_runtime_world reuses the
  original dungeon when the loaded blob lacks one. A chest-0 pickup of 28f1
  correctly leaves head 28f0 before serialization, but resume restores head
  28f1 while the hand also retains 28f1. Preserve changed Thing ownership in
  a future save-format repair; current chest interaction tests do not prove
  full save/resume fidelity. This existing limitation is separate from the
  corrected live G0425 slot persistence and same-slot release handling.

- Verify DM1 chest shutdown paths and extend owner-transition evidence
  beyond the original PC3.4 corpus; save/resume remains separately deferred.

- Extend original-media inventory interaction verification to chest contents,
  scroll text/pixel equality and supported editions beyond PC3.4. Original
  PC3.4 scroll-eye panel routing and held-object preservation pass in both modes.
  The 611-record PC3.4 Original/V2.1 press/release corpus covers all 30 empty
  slots, a distinct admissible resident per occupied-slot swap, source-mask
  rejections and identity preservation, not complete
  inventory or pixel parity.

- Complete legacy gameplay sound parity. Atari now selects original SND1
  records through a source-locked event map and retained GRAPHICS.DAT bytes;
  verify live event timing,
  arbitration and PSG output against original captures. The authenticated
  English, German and French Atari real-media tests now run with SDL's dummy
  playback device and require every accepted event to add decoded original
  PCM to the device queue; the three known short records must leave the queue
  unchanged. A live food-swallow event also requires one queue increment in
  each tested edition. This proves bounded source-to-queue transport, not audible output,
  queue-drain timing, arbitration under overlapping events or PSG waveform
  parity. FM Towns local and M10 tick events now read F20 unsigned PCM from
  retained media; M10 emission distance attenuation and per-index same-tick
  arbitration pass against English/Japanese original media. Source-owned
  F20 creature attack sound requests now carry F0207's event coordinates
  through the same per-sample tick queue, including the miss branch. The
  EN/JA audio test uses original C04 groups and PCM but constructs F0064
  receipts and moves the party pose in memory; it does not prove a naturally
  scheduled live F0207 attack. Drive a real original-dungeon attack through
  M10/M11, then verify original F20 driver channel scheduling, event ordering
  across separate M11 calls in one game tick and physical M5 speaker output.
  F0207 also requests an attack sound in the Atari, Amiga and PC source
  branches; the newly forwarded M10 receipt follows that call, but its
  overlap/priority behavior on those transports still needs original-media
  verification.
  The PC 3.4 first-monster test now verifies F0195 activation, group movement,
  natural party damage through the C37/C38 sequence, and a player PUNCH with
  paired XP. It does not verify the exact F0207 sound ordinal or waveform.
  Its coordinates and recruitment sequence have not been verified for the FM
  Towns dungeon.
  The FM Towns EN/JA sound test instead
  relocates the party in memory and constructs F0064 receipts from an
  original C04 and PCM; it cannot establish a scheduled F0207 attack.
  Amiga local
  effects now read original signed PCM with the native period; verify
  PAL/NTSC selection, stereo distance volumes and two-pair channel arbitration.
  Consult SOUND.C MEDIA007/MEDIA413/MEDIA488 and each
  DATA.C sound table before reusing any CSB transport: CSB's later sound
  indices and sample maps are not interchangeable with DM1's. Preserve the
  current no-generated-marker behavior while adding authenticated sample
  and live dispatch tests. CDDA/title success does not prove effect playback.
  Capture original F0060/F0061 runtime RAM boundaries for Atari sound indices
  1, 12 and 16 (records 534, 544 and 546): the bounded SND1 decoder rejects
  these original EN/DE/FR streams as short. Record 534 declares 100 samples
  but its 37-byte nibble stream describes 99. Do not invent a final sample
  or read beyond the allocated record to make playback appear complete.
  Hatari 2.6.1 can mount the original English 1.2 ZIP/STX read-only and
  capture RAM/screens through a VBL breakpoint, but the ReDMCSB bundle's
  `tos.img` is EmuTOS (SHA256
  `5393932066f3199a6a653dfd1f1524bb52375ae0ad0831720743c2e015360a2b`).
  The ST/1 MiB run reaches an EmuTOS bus-error screen at PC `00e14aac`,
  not the game sound consumer. Do not admit that dump as gameplay evidence;
  establish a compatible reference boot before taking the SND1 trace. On the
  current macOS host, direct `hatari --help` and `hatari --version` invocations
  also abort in AppKit's `NSMenuBarPresentationInstance` registration before
  emulation starts; the crash report is host-launch evidence, not game runtime
  evidence. No TOS ROM was found in the staged game-data or mounted
  Firestaff-archive trees.

- Compare original Atari pixel captures after the corrected F0497/F0496
  decoder. English, German and French original-media name/all-record checks
  and native CLI/menu/input regressions pass, but are not pixel comparisons.

- Complete Japanese FM Towns M564 catalog translations and rendered-glyph
  verification. All 199 names now pass original-JDATA framing/index/UTF-8
  checks, but these checks do not prove translated inventory/hand-label pixels.

- Complete l10n coverage for every player-facing string in Original and
  Modern/Custom: remaining item/action/spell names and dialog,
  remaining ending text, and every retail sensor/timeline/scroll string. Object and
  action names, F0410/F0381 messages, sensor/timeline text, scroll text, and
  wall inscriptions now cross the DM1-domain presentation boundary. The wall
  path is real-PC3.4 gated through F0168/M648 and has a Swedish Unicode raster
  proof; it is no longer open work. PANEL.C F0351 skill-rank, base-class and
  statistic labels now cross the same DM1-domain boundary in both the live eye
  readout and source-font inventory panel, with Swedish catalog coverage. The
  two authentic PC3.4 F0446 victory messages now also translate after their
  non-prose A/B ordering key is removed and before the message-area publish.
  remaining catalogs and call sites need
  equivalent coverage gates. Decode selected retail bytes first and retain
  them as the stable msgid/fallback; key ambiguous dungeon text by media
  identity plus text index. Never modify or replace original data.
- Validate the completed F0128 source-order executor against authenticated
  same-state original captures. HoC sensor/material-plan checks pass with
  French DOS original data (24 directional portraits), but do not render
  pixels. The PC3.4 live pointer sweep separately selects all 24 mirrors
  in Original and V2.1; host window scaling and emulator comparisons remain
  to be verified. Missing-media
  skips must not be counted as completed original-data verification.
  The live
  scheduler validates and dispatches every source step; D3--D1 wall, ornament,
  door, both F0115 partitions and foreground material are callback-owned at
  their square-local boundaries, and the hand-written scheduler-span and broad
  primitive replay paths are gone. No remaining implementation item in this
  section should reintroduce a parallel renderer.
  F0125--F0127 D0 F0104/F0112/F0113 primitives have been migrated into the
  callback and their separate replay deleted, retaining the F0127 split around
  its F0115 Thing consumers. F0127's complete D0C F0115 item, projectile and
  restarted explosion transaction is now callback-owned before F0113; the
  former direct passes, including the source-invalid post-field explosion
  placement, are deleted. F0125/F0126 now consume their D0L/D0R F0115
  creature rows and following F0113 field steps in the same callback stream;
  D0L, D0R and D0C are dispatched as three explicitly targeted callback
  transactions, so D0L's field tail completes before D0R and D0R completes
  before D0C primitives begin instead of using a class-wide D0 replay;
  the unreachable broad D3--D1 stairs fallback has also been deleted, leaving
  each authenticated F0104 stairs step solely owned by its square callback;
  G2028-negative item/projectile routes remain correctly suppressed. Their
  restarted C15 passes now preserve effect-list order, centered/cell identity,
  party-relative cell rotation, and the exact item-696 C3029/C3030 and
  C3061--C3064 anchors. Move each remaining authenticated F0104/F0107/
  F0108/F0111/F0113/F0115 consumer into that callback in source order, then
  delete its replay rather than running both paths. Center-square
  and side-square F0115 routes now consume their authentic one-based
  cell-order word, including the `DOORPASS1 → F0111 → DOORPASS2` partition
  for real floor items, creature groups and projectiles. The final occlusion
  replay now completes D3/D2 outer lanes and D3--D1 normal side lanes one
  source square at a time. D3L2/D3R2 now consume their previously missing
  `DOORPASS1` before F0676/F0677's door occluder; D2L2/D2R2 remain the
  source-authentic no-Thing routes. Normal side and center `DOORPASS1` is
  likewise callback-owned between each route's wall/ornament envelope and
  F0111 door. The callback-owned foreground phase then retains the exact
  `MAIN`/`DOORPASS2`/F0113 order together with F0104/F0108/F0112 material.
  Door-front F0108 is no longer part of that post-door tail: it has its own
  callback phase before `DOORPASS1`, matching the source
  `F0108 → DOORPASS1 → frame → F0110 → F0111 → DOORPASS2` transaction and preventing
  pressure plates from repainting a completed door.
  F0104 wall material for D3--D1 is now callback-owned at each square's first
  scheduler step; the former outer/side/centre wall replay calls are deleted.
  F0107's admitted 13-row wall-ornament family is now callback-owned one
  projection at a time, including D1C's source-owned C346/C026 champion
  mirror consumer, and the old broad and mirror replay calls are deleted. The
  D1C Thieves Eye wall restore is also callback-owned at F0124's square-tail
  boundary after the complete D1C transaction and before F0125; its former
  direct post-scheduler draw is gone.
  D3--D1 normal C15 explosions are now restarted after packed-cell material
  inside every owning F0115 callback, including both door partitions around
  F0111; the once-per-frame global replay is no longer called.
  MEDIA720 D3L2/D3R2 F0107 is now callback-owned from the authentic PC 3.4
  item-696 layout records `C1004 + set*15 + {0,1}`. These are centered/top/
  bottom-aligned anchors resolved with the source bitmap dimensions, not
  inferred G0205 rectangles. Item 558 was ruled out byte-for-byte: its real
  record at `0x25eed` is a 38-byte 16x7 image, not layout data.
  F0111 door frames/panels/ornaments/masks are now callback-owned at the
  plan's explicit door step. F0110 is likewise callback-owned only on the
  four source routes that contain it: exceptional D3R and centre D3C/D2C/D1C.
  The former direct side/centre button replay is deleted. F0104 door frames
  now rasterize through their own preceding callback phase, including open
  doors and D3L2/D3R2, while F0111 owns panel composition only. Replace remaining work only
  with the corresponding
  F0116--F0124 operation, never with a
  host-generated panel or a substitute asset.
  F0111 now resolves a DOOR Thing's Type bit through the current map's real
  `DoorSet0`/`DoorSet1` values, fixing iron/Ra panels on retail maps whose
  sets are not 0/1. F0111 now also consumes the animated Ra door's single
  `M004_RANDOM(4)`, composes the ordinary ornament, D1C Thieves Eye and
  destroyed mask into a complete native temporary door bitmap, applies the
  whole-bitmap horizontal/vertical flip, and only then clips the current
  opening state. The same transaction now owns all eight side-door F0111
  routes, including the exceptional D3L2/D3R2 slices; the old independent
  ornament and destroyed-mask viewport overlays are no longer dispatched.
  The pass1055 closed-D1C comparison remains candidate evidence only. Its
  original pass513 record does not bind F0128 map X/Y/direction or prove an
  F0097-presented frame, so the measured 429/8,448 panel-pixel delta cannot be
  used as a renderer oracle. Obtain an authenticated original same-pose
  debugger capture first, then close any proven differences and add
  route-specific captures for animated Ra, ornament, D1C Thieves Eye,
  destroyed-mask and opening-state variants; do not hide divergence behind a
  tolerance or nearest-neighbour pose match.
- Complete D3L2/D3R2 F0115 material consumers. The live object route now
  consumes the original layout-696 C2500 rows 3/4 through the decoded source
  Thing and GRAPHICS.DAT, including the depth-3 cell gate and C10 blit. The
  scheduler's F0676/F0677 rear/front partitions now own that real object
  pass; they do not borrow ordinary D3L/D3R pane geometry. For creatures,
  MEDIA720 F0115 maps `C14_VIEW_SQUARE_D3L2`/`C15_VIEW_SQUARE_D3R2` through
  `G2033[14]`/`G2033[15]` to raw C3200 rows 3/4 (`DUNVIEW.C:373, 5211,
  5613-5617`). The native raw-C3200 plan now consumes those exact rows and
  rejects blank source coordinates; the same F0676/F0677 consumer draws
  C2900 through its source row and original cell partition. The existing
  helper for ordinary D3/D2/D1 side rows remains separate. What remains is
  capture-backed pixel comparison for those uncommon creature/projectile
  routes, not a host pane substitute.
- Obtain authentic C13-save and original capture corpus for remaining HoC,
  top-row and action routes; bind each to the PC 3.4 runtime before promotion.
  The first required C13 scenario is now identified: the HoC revive alcove
  followed by the immediately lower stair square containing a monster. Capture
  the original route through both poses, preserving the save's dungeon-tail
  owner, active group and event state; do not replace that state with a
  hand-authored Firestaff save.
  The supplied French DOS save pair proves its own backed F0435/F0433 route,
  including direct CLI and start-menu resume, bounded movement, and turns on
  map 5 with its four saved champions, but does not substitute for these
  route-specific captures. A checksum-valid synthetic C13 seed without the
  matching source-owned bones Thing is deliberately rejected at F0435. A
  synthetic fixture may be used only to drive the original runtime to the
  required location and produce a new real-format save; it must never be
  promoted as preservation evidence itself. A usable C13 corpus must
  therefore preserve both the event and its dungeontail owner.
- Extend real-media parity beyond bounded Atari ST and Amiga routes to native
  end-to-end gameplay, input and presentation evidence. FM Towns now has a
  real ZIP CLI/start-menu receipt through TMENU → EDM/JDM. The authenticated
  M12 Version tile is tested selecting JDM without a CLI language override.
  Also covered are the
  18-frame English title plan, source-decoded PRESENTS/DUNGEON/MASTER palette
  transactions, CDDA title track, and its input matrix. The stripped JDM path
  now has its own unique disassembly fingerprint, recovered geometry/data
  owners, real-media C12/C13/C14 palette binding, and a native checksumless
  F20J JDATA dungeon handoff that reaches `levelLoaded=1` directly from the
  retail ZIP. Independent seven-command English/Japanese input matrices now
  pass against each edition's graphics and executable fingerprints. An
  authentic gameplay capture is still required before visual
  parity beyond those startup routes can be claimed.
- Bind an authenticated Amiga gameplay RGB4 table to the native renderer.
  The original v2.0 `dm` producer is implemented and source-gated: its
  in-memory 68000 receipt copies the saved table to a working table, adjusts
  each RGB4 component by one or two toward the target, and invokes the Copper
  builder eight times. It deliberately accepts no fabricated palette. What
  remains is a route-specific original gameplay table/capture that identifies
  the active source and target tables; do not substitute the PC VGA palette.
  The supplied ordinary Amiga save disk is now receipted as an authentic
  `DMGAMEG.DAT` session, but it is format-5/Amiga-compatible save material,
  not a Copper-list or framebuffer capture; it cannot alone identify the
  palette table active at a particular gameplay frame.
- Obtain an authentic nonzero-C1 Amiga format-5 `DMGAMEG.DAT` sample to
  exercise the implemented source-owned ACTIVE_GROUP adapter and additional
  C3/C4 event families. The admitted v2.0 save is a zero-active-group/C53
  session; it proves the full in-memory C1 ownership boundary but cannot by
  itself validate a live group or a non-C53 event route.
- The supplied French DOS ZIP → `dungeon_master.exe` SFX package uses a
  non-solid RAR 2.0 (`unp_ver=0x14`) stream. Native RAR2 decoding is
  intentionally out of scope; the launcher reports it as unsupported rather
  than misreporting the supplied original data as missing.
- Bind V2.2 presentation only to reviewed original material/pixels. Existing
  placeholder or procedural art remains fixture-only.

## Deferred original-data corpus

- Capture original PC 3.4 overlays and cadence for the now implemented F0444
  champion-screen input transition into C3 `THE END` and C005 credits. The
  separate non-victory F0435 restart success/failure branches are now wired
  without host relaunch, but still require an original runtime capture using
  an authentic loadable save and a rejected/corrupt save before pixel/cadence
  parity can be promoted. The live victory path correctly disables restart.

The missing C13 save/capture corpus is deferred rather than a release blocker
for the native media paths currently under active work. When an
operator-supplied corpus becomes available, add its hashes and provenance,
then promote only the routes it directly proves. A fixture-derived save may
exercise the original runtime, but cannot by itself close the preservation
evidence gap.

2026-10-14 I34E DOS launcher/source correlation: ReDMCSB
`Toolchains/Common/Source/DM.C:689-695, 1029-1055, 1087-1112` confirms that
`DM.EXE` is the hardware/configuration launcher: it defaults the game child
to `FIRES`, selects `VGA`, `EGA`, or `TANDY`, and `execl`s the selected driver
with `IBMIO`, the game executable, and options. `MKII.BAT:95-115` builds
`DM.EXE` only from `DMASM.ASM` and `DM.C`; `SWOOSH`, `ANIM`, `SELECTOR`, and
`FIRES` are separate executable targets. Together with `I34E.LNK` and
`STARTUP2.C:1443-1453`, this confirms that I34E FIRES itself loads directly
through F0435 and does not call the shared F0437 title or F0441 entrance.
The parent process chain is in `IBMIO.C:2411-2418, 2559-2572`: IBMIO runs
`swoosh`, then `anim title -ah -as`, then `selector`; selecting Enter or
Resume launches the game executable passed in as its first argument. The
full source route is `DM.EXE` -> VGA -> IBMIO -> SWOOSH -> ANIM/TITLE ->
SELECTOR -> FIRES. `VIDEODRV.C:3856-3992` confirms VGA installs mode 13h and
executes the IBMIO child. The I34E `ANIM` player reads a named animation file
(`ANIM.C:1806-1918`), and `SELECTOR.C:866-1000` builds the entrance/menu view
from GRAPHICS.DAT and handles Enter/Resume/Quit itself. `SWSH.C:1783-2180`
implements the separate FTL logo animation. Firestaff now plays the standalone
I34E title and the selector Enter transition before FIRES; the authentic
`dm1_v1_pc34_native_cli_boot` test verifies this path against PC34 media. An
authentic DOSBox run from the extracted PC 3.4 distribution showed the same
program order. Captures are in local scratch only and do not claim an M5 or
release comparison. The selector already supports Resume, Credits/return,
Quit by keyboard/window close, and its source Quit hitbox; authenticated
original-media regression coverage now exercises keyboard Quit and the Quit
hitbox alongside the existing Resume and Credits routes. Compare the first
runtime viewport against the original DOS capture separately. The existing
C001/Entrance timing captures remain valid Firestaff measurements, but are
not original-I34E parity evidence.

2026-10-14 earlier status (superseded by the direct-launch implementation
below): the bounded LZEXE 0.91 decoder extracted the original embedded
224x136 selector preview from the canonical I34E `SELECTOR` executable (MD5
`165ae5438ce2686ae62f2fbb61f3e831`) and expanded its IMG3 pixels without
bundling game media. A local sanitizer harness verified all 30,464 palette
indices against checksum `00c6756a`; at that point extraction was not yet
connected to the presentation or F8360 door animation.

2026-10-14 F0097 palette correction: Firestaff now preserves the I34E VGA
viewport bank 16..31 and maps it through the selected `G8151`..`G8156` row;
the surrounding screen continues to use entries 0..15. The source-backed
F0097 pixel/palette tests and authentic PC34 CLI boot pass. This closes only
the palette-indexing mismatch. It does not establish that the viewport's
geometry, wall/floor asset selection, title startup, or entrance animation
matches I34E. Capture the same map, party position and facing from original
FIRES and Firestaff, then compare the 224x136 viewport before claiming those
routes as parity.

2026-10-14 I34E TITLE and direct-launch SELECTOR implementation: ReDMCSB `ANIM.C:1975-1980` initializes `-as`, `ANIM.C:2041-2093` handles EN/DL frame records, P8 and PL, and `ANIM.C:2125-2134` applies an ending clear only under `-ae`, while `-ah` retains the palette. The bounded Firestaff parser validates the authentic PC34 `TITLE` SHA-256, dimensions and frame records; it reuses the shared F8288 decoder and presents the source palette with a host timing model. Firestaff runs authenticated `SWOOSH -> ANIM TITLE` before the launcher menu. For direct `--game dm1`, it now also loads the hash-authenticated I34E `SELECTOR` executable and the already matched PC34 `GRAPHICS.DAT`, extracts the original embedded G8250 224x136 preview, applies the source `G8148_ENTRANCE` palette and composes GRAPHICS.DAT indices 4/2/3 using ReDMCSB `SELECTOR.C:F8360` at four pixels per source VBlank. Direct launch is the selector's Enter choice: the source-sized door sequence completes before the existing FIRES/runtime handoff; that route avoids the later C001/F0437 and F0441 transaction. A local authentic-media `--game dm1 --platform pc --debug` probe reports 53 TITLE frames, 32 selector frames, and then `handoff=pc-img3`/`runtime-entered`; the selector CTest validates the extracted preview hash and checks closed/first-open F8360 composition. This does not port interactive SELECTOR input, the R/Q/Credits choices, or button hover/click feedback. As of 2026-10-15, ReDMCSB `SELECTOR.C:934-935,1026-1034` source audio is wired too: Firestaff binds SND3 items 672/673 from the selected `GRAPHICS.DAT`, queues event 1 at Enter and event 2 on the source retrigger cadence, and waits for the final SFX stream to drain. The authentic CLI test checks both queued events. This audio integration still does not establish sound waveform or hardware timing parity. It also does not replace M12's interactive launcher boundary, prove all-platform startup parity, or establish retail-hardware visual parity. Capture and compare the same selector and first FIRES runtime frames against the original PC34 executable before making broader parity claims.

2026-10-14 I34E cadence and integration test: `SELECTOR.C F8353` reaches `VIDEODRV.C F8153`, which waits on VGA status port 0x3DA. The host loop follows the existing 70 Hz PC34 VGA retrace model with rational deadlines rather than the 18.2 Hz DOS tick. A media-gated app CTest verifies the complete direct-launch stage order when authentic PC34 files are available and is omitted when those files are absent.

2026-10-14 FM Towns Entrance micro-dungeon: ReDMCSB `ENTRANCE.C F0797` constructs transient map C255 as a 5x5 wall grid with a south-facing corridor at column 2 and source square 7 open at (1,2); `COORD.C G2067/G2068` places the 224x136 view at (0,33) for EDM and (0,31) for JDM. Firestaff draws that transient map immediately before the original door-frame sequence and changes only that aperture. The first implementation mistakenly read PC34 MEDIA720 records 78/79 and wall IDs 95/96/101/105/106 from the Towns archive. MEDIA020 instead maps floor/ceiling to records 75/76 and the requested wall compounds to records 86/87/88. The corrected receipt reports semantic renderer slots and source record masks; the final-binary archive CTest requires `requested=loaded=0x0000c433`, `source-records=0x00003803`, and no missing records for EDM and JDM. The standalone renderer test uses authentic F20 dimensions and rejects unrelated PC34 graphic IDs. This verifies source map setup, asset-family selection, bounded source-asset composition, and aperture placement, but does not compare rendered pixels to a Tsugaru or retail-hardware capture; broader DM1 FM Towns gameplay viewport parity remains open.

2026-10-14 PC34 M12 menu launch: the authentic startup transition now continues from the I34E `ANIM/TITLE` frames into the source-sized `SELECTOR` Enter door sequence when the M12 launcher accepts New Game. Direct and menu launches share that single selector transition before FIRES, and the preplayed-title marker is consumed at the launch boundary so a later choice of another edition cannot skip PC34's title. `dm1_v1_pc34_native_cli_boot` passed against the original PC34 ZIP: 53 title frames and source VBlanks, 32 selector steps at 70 Hz, captured first/middle/final title frames and closed/step-zero/middle/open selector frames, authenticated matching dungeon data, menu runtime and movement, and 1920x1080 HiDPI-scaled launch probes. The first selector step is deliberately identical to the closed image; the later source frames show progressive opening. Boot-probe mode skips visible startup animation and is checked only for edition/runtime selection. This does not implement interactive I34E selector choices or establish first-dungeon viewport parity against the original DOS executable.

2026-10-18 PC34 selector controls: ReDMCSB `SELECTOR.C` maps `R` and its Resume hitbox to command 3, independently of Enter's door animation. Firestaff now routes keyboard `R` and the authenticated selector Resume hitbox through its existing DM1 Quick Resume intent when a DM1 save is available; the source switch sound plays and the FIRES door sequence is skipped. The Quit button hitbox exits like the source `Q` command. The Credits hitbox loads original GRAPHICS.DAT item 5 with the source C06 palette, waits for a key or mouse event as F8327 does, then restores the closed selector. The extended `dm1_v1_pc34_native_cli_boot` test checks Resume selection and a Credits-return-then-Enter sequence against original PC34 media. Original-frame visual comparison remains open.

2026-10-19 PC34 Resume evidence boundary: `dm1_v1_pc34_native_cli_boot` passes against the original English PC34 archive and exercises both keyboard `R` and the Resume hitbox, but supplies a deliberately missing `no-save.DAT` and only asserts that the selector chooses the Quick Resume route. It does not prove an authentic PC34 save is loaded or that its saved pose is restored. `dm1_v1_dos_fr_original_save_cli_resume` separately checks a real French PC34 save through direct CLI and the M12 Quick Resume row, not through the I34E selector. The configured `~/.firestaff/data/dm1` tree currently has no PC34 `DMSAVE.DAT`; selector-to-authentic-save restoration therefore remains unverified until an original PC34 save is available.

2026-10-15 I34E TITLE lifecycle correction: IBMIO G8102 (`IBMIO.C:2411-2418`) selects `title -ah -as`: ANIM's S flag erases before TITLE, E erases afterwards, and H retains the final palette (`ANIM.C:1880-1898,1974-1976,2127-2135`). The previous terminal wipe was incorrect. Frame timing remains open: PC34 uses raw EN/DL attributes, while the min-five branch at `ANIM.C:2044-2048` is PC98 and F2275 is FM Towns. IBMIO F8114 returns callback decrement 3; F8091 polling and the PIT interrupt path can invoke that callback (`IBMIO.C:671-672,1280-1290,1649-1650`). The TITLE scheduler now uses raw counters, callback decrement 3 and the default BIOS/PIT period (65536/1193182 seconds), resetting before decode and consuming callback boundaries during decode/presentation. Counter debt does not carry across EN/DL assignments. Its host phase starts at title-handler entry; the historical initial BIOS phase and original CPU decode duration remain unmeasured. The 64-chunk pre-erase is still a host presentation approximation. Calibrate against the authentic executable before claiming measured timing parity. There is no source-specified extra final hold or fade in this route.

### Implicit diagnostic archive admission (2026-10-15)

An original PC 3.4 ZIP in an isolated default originals root launches through
ordinary CLI startup, but the former implicit `--boot-probe` loose-only scan
rejected it with `game unavailable`. Diagnostic startup now uses the ordinary
bounded, hash-authenticated intake policy. The regression checks the exact
PC graphics MD5, archive member owner and loaded-level receipt, then verifies
ordinary startup and accepted movement for both offline folder and game-leaf
preferences. This admission check does not prove visual or physical display parity.

2026-10-15 folder-feedback follow-up: synchronous selection and asynchronous
completion now share one message policy. Empty selections retain installation
guidance; candidate-only and warning priorities retain their prior paths.
Both empty-directory regressions and authentic PC34 callback/persistence checks
pass. Native OS dialog presentation and candidate-only UI text still need their
own interaction evidence; callback tests do not establish physical dialog parity.

### Late folder-scan cancellation (2026-10-15)

UI cancellation now wins until the completed worker result is published, even
when its final progress callback ran before Cancel. A deterministic semaphore
regression verifies old selection, runtime source and saved config are retained;
the authentic PC34 variant also preserves playable availability. The interposed
thread-creation test runs on macOS/Linux and explicitly skips on Windows; add
equivalent native Windows execution evidence without relying on sleep timing.
The two full five-game corpus cases were not part of this scoped run.

2026-10-15 CI follow-up: Windows run 38804393070 failed
`dm1_v1_startup_intro_state_machine_gate` while creating a hardcoded `/tmp`
fixture directory. The task-local scratch replacement passes local CTest;
confirm its native Windows result in the next Actions run.

### Selected virtual graphics identity (2026-10-15)

DM1 virtual GRAPHICS.DAT is now authenticated on the exact consumed buffer,
so swapping a selected archive after scan cannot retain its previous graphics
identity. Original-only PC English/multilingual replacement regression passes;
Amiga 2.0 and Atari ST 1.2 still reach loaded probe states. Towns is a separate
unchanged path. Expand equivalent original-media stale-source checks to the
other loaders and native Windows staging; this does not establish full parity.

Explicit edition-selection follow-up (2026-10-15): original-media input tests
reproduced and now block unavailable Multilanguage selection without substituting
English. DM1/CSB/DM2 launch intents follow the gate's recovery policy. AUTO and
platform/version mismatch recovery remain; completed scans recover unavailable
saved editions before interaction. Deferred or cancelled scans must not infer
absence before inventory is known. Deferred original-media tests retain installed
Multilanguage and recover English only when the selected edition is absent.
Original-media pointer routes now cover CSB Amiga and DM2 DOS through the
platform and presentation cards: `csb_v1_amiga_adf_archive_cli_boot` and
`dm2_v1_dos_native_cli_boot` both pass on the installed data. Their scripted
mouse-only routes prove card selection and launch acceptance for those two
platforms, while keyboard routes separately cover runtime handoff. Pointer
selection across the remaining CSB/DM2 platform and language choices remains
open; menu-state fixtures alone do not establish those original-media routes.

### Loose selected graphics ownership (2026-10-15)

The selected loose graphics path now remains authoritative; changed or removed
files cannot silently choose a canonical sibling. Decoding consumes the same
buffer that passed its selected MD5 check. Already admitted virtual and loose
native buffers are reused, and rejected binding tears down the partial world.
Original English/multilingual canonical-replacement and renamed-file-removal
cases pass alongside positive starts. Keep negative Towns/native-edition and
Windows staging expansion open; broad platform/visual parity is not established.

### Native launcher rejection lifecycle (2026-10-15)

FM Towns native startup rejection now clears its already activated partial
world. A complete authentic English launcher starts; removing only the staged
TMENU.EXP link after scan rejects and leaves the state inactive. PC fresh-pool
allocation failure also tears down, but that allocation branch lacks fault
injection evidence. Native Windows symlink staging remains skipped.

Loose Towns inventory follow-up (2026-10-15): scan-time native receipt validation
now rejects incomplete English/Japanese source owners independently. When no
playable edition survives, existing required-file feedback lists the launcher
filenames. Valid Japanese/PC siblings remain available. Program reads use a
1 MiB per-file cap on the same open file; original supported native programs
pass. Virtual archive admission is unchanged and no per-frame I/O is added.
Duplicate native launcher owners are now searched in bounded, hash-matched
loose candidates before edition publication. A broken first copy no longer
hides a complete English copy; the authentic menu handoff and stale-source
rejection are covered. Invalid earlier roots are bypassed when another ready
root exists (cross-root behavior is source-reviewed, not runtime-tested).
Per-selected-edition diagnostics in mixed ready collections and native Windows
original-media staging remain open. Missing/wrong-language dungeon ownership
and recovery to a complete duplicate now have authentic local coverage; both
English and Japanese loose native worlds start with their own buffer decoder.
Cross-root runtime evidence and native Windows staging remain separate gaps.

2026-10-15 explicit Towns file selection: complete loose GRAPHICS.DAT requests
retain their exact owner; incomplete requested owners reject without selecting
a neighboring complete copy. Original-data scan and runtime assertions pass.
Explicit Towns missing-companion feedback now retains authenticated graphics
and identifies missing/wrong-language dungeon data or native programs; normal
menu input and both scan APIs have original-data coverage. Extend equivalent
feedback to other native owners and mixed-collection selected-edition failures.
Mixed loose Towns/PC34 English reports now refresh graphics
and dungeon provenance after incomplete native owners are removed. Both the
full inventory and selected-game scan have authentic EN→JP→PC coverage.
Final surviving loose-owner reports now also refresh Atari/Amiga dungeon
provenance. Original ST12 and A20 English cases pass beside rejected Towns
launchers through both scan APIs. Other native virtual-owner combinations and
native Windows original-media staging remain separate verification work.

2026-10-16 multilingual owner follow-up: original EUDATA regression tests
now prove exact selected-file ownership in both scan APIs, matching required
file provenance and runtime startup. Removing or replacing its standard
dungeon rejects without borrowing an English sibling. Same-directory renamed
original standard dungeons remain supported, as do explicitly supplied French
original dungeons with loose or virtual PC graphics. The subsequent PC language
routing and launcher-feedback entries below cover automatic French/German
selection. Language-only packages, other native owners' companion feedback and
native Windows original-media staging remain open; this is not full edition parity.

2026-10-16 ScummVM audit refresh: current master
`a3cf3746e44070528badfbdbb292edd9dd4e6340` has the same gfx.cpp blob as the
pinned audit above. DOS loadWallSet (3864-3909) skips offset 17 while publishing
the D3R2 buffer in the negative bitmap table. Its actual use depends on the
DOS layout route; no visible ScummVM defect was established. Firestaff's native
C10 maps to graphics 103 (86+17), consistent with original DUNVIEW.C
F0095:2153-2165. Preserve that original mapping. ScummVM's
setMultipleColorsInPalette (6108-6157) substitutes its dungeon palette for
G8175_CREAT_PAL, so native video-driver tables remain authoritative. Source
comparison found no newly proven Firestaff defect and did not execute ScummVM.

2026-10-16 native companion compatibility follow-up: original-data tests
reproduced scan admission of an incompatible PC34 dungeon beside ST12 graphics.
Loose and virtual native admission and retained runtime buffers now reject
the three known little-endian PC/Towns originals, sharing their exact identities.
ST12 and A20 English original startup, both scan APIs, all three loose foreign
replacements and restoration pass; explicit PC custom dungeon paths retain their
existing tests. Virtual ST12 own/PC/restored pairs and mixed ready-PC collections
also pass. Selected physical archives now exclude neighboring editions, including
late native preservation recovery after parent-directory scanning. Amiga demo,
A20 and A36 CLI probes retain their exact archive-owned dungeon reports; A20 also
passes a relative selected path. This does not establish arbitrary custom-format
validation or edition-exact compatibility for every native dungeon. Verify remaining
native owner combinations and archive-reader limits; native
Windows original-media staging remains open.

2026-10-16 duplicate virtual-owner follow-up: original ST12 ZIP copies reproduced
an incomplete first owner hiding a complete same-graphics installation. Bounded
container-aware candidate enumeration now selects a complete default owner,
including its startup companions, while explicit bad archive selections remain
terminal. Focused authentic-media coverage passes both scan APIs, exact graphics
and dungeon receipts, actual native runtime handoff, overflow without partial
results, and distinct same-hash members within a single ZIP. A20 without SWSH
is rejected; full original A20/A36/demo CLI probes load their worlds. This does
not establish every archive format/platform combination or visual parity.
Existing container/depth limits still apply. The later renamed-A20 follow-up
below extends generic ZIP/ZIP/OFS-ADF discovery; other disk formats remain
subject to their existing readers.

2026-10-16 late native admission follow-up: an incomplete canonical Amiga demo
could overwrite an already complete duplicate before final validation, hiding
that working installation again. Native Amiga admission now verifies full
same-owner startup completeness before changing any version or required-file
receipt. Authentic demo duplicate discovery, runtime handoff and explicit bad
source rejection pass both scan APIs. Demo coverage accepts the optional
FIRESTAFF_DM1_AMIGA_DEMO_ARCHIVE original-media path and does not depend on
Atari media being installed. Other native preservation shapes still need their
own original-media cases; this does not establish arbitrary archive parity.

2026-10-16 renamed A20 follow-up: the unchanged original retail archive failed
CLI startup under an arbitrary outer ZIP filename. Generic nested disk discovery
now routes ZIP/ZIP/ADF through the existing bounded OFS visitor, preserving all
actual container/member names. Both scan APIs discover an original archive alias
as an explicit file and through an isolated directory; all four handoffs load
its original world. Final focused CTests pass 2/2 (9.57 s), with demo original supplied
and no media skips. This covers the existing OFS decoder, not arbitrary Amiga
raw/MFM disks or multiple matching disk members within an inner ZIP.

2026-10-16 selected native missing-world feedback: original ST12 and A20
explicit loose graphics were incorrectly reported missing when their own dungeon
was absent. Unavailable DM1 scans now retain a separately authenticated selected
original graphics receipt after final launch gates, without restoring edition
rows or availability. Both APIs and ordinary menu input identify the missing
dungeon, and direct runtime remains blocked. Other missing native startup
companions, incompatible-world diagnostics and mixed-collection edition-specific
messages remain separate coverage gaps.

Source-audited next diagnostic cases (not yet original-media UI reproductions):
`m12_select_dm1_loose_file` clears incomplete native rows, after which
`m12_fill_required_files` may use the game-wide dungeon hash alternatives.
That list includes incompatible PC/Towns originals, so verify the selected
native sibling independently before publishing its dungeon receipt. An A20
world with missing SWSH also needs a dedicated startup-companion diagnostic;
otherwise both base roles may match while availability is correctly false and
the menu falls back to generic VERIFIED GAME DATA text. Preserve admission
and exact selected-source boundaries while fixing these reports.

2026-10-16 selected-owner role follow-up: authentic ST12/A20/A36 originals
now reproduce and cover incompatible PC/Towns dungeon diagnostics, independently
verified world receipts, and missing A20 SWSH/A36 VDEO.FTL. Default-world
identity and startup-companion readiness are separate shared checks; the report
reuses both without restoring availability or edition matches. Original-data
menu input identifies the actual missing role. Both scan APIs, explicit runtime
rejection, restored native starts and mixed surviving-owner reports pass. The
A22G `a22g-four-files` tuple now has original-media CLI and start-menu coverage;
source-reviewed alternate-tuple equivalence and its virtual-archive diagnostic
remain open.

2026-10-16 explicit archive diagnostic follow-up: selected unavailable DM1
archives now recover independently authenticated original graphics within that
exact physical owner and reuse the world/startup role checks. Authentic A20
without SWSH and ST12 with incompatible PC world cover both scan APIs, precise
same-owner receipts and ordinary menu refusal. Full ZIP scanning previously
cleared the more precise DM1 failure report after checking other games; it now
preserves only the bounded required-file rows through generic initialization,
without restoring versions, availability or runtime directories. Batch diagnostic
lookup chooses one graphics path per catalog hash; this does not prove complete
intra-archive duplicate diagnostics or directory-selected/mixed-edition messages.

2026-10-16 explicit ADF ownership: direct original Amiga 3.6 disk selection
previously launched a neighboring canonical Amiga 2.0 archive. DM1 intake now
keeps the selected OFS disk as its source root; shared CSB/DM2 per-game search
classification remains unchanged. Full file inventory retains the existing
per-game discovery paths and only accepts a selected container's own files.
Raw/MFM protection formats, every edition and complete visual parity remain
separate requirements; this change does not add a universal ADF decoder.

2026-10-16 PC multilingual language routing: ReDMCSB FILENAME.C F0745 and
STARTUP2.C select DUNGEONF.DAT/DUNGEONG.DAT for French/German. Runtime now
binds these choices to the selected multilingual graphics owner and validates
the retained original buffer against that language's complete-file hash.
The authentic French/German bodies omit a checksum trailer; the separate
bounded body reader follows LOADSAVE.C F0434's optional-checksum behavior
without relaxing generic save-tail validation. Language-only
packages now use an explicit EN/FR/DE scan policy carried into the result
snapshot. CLI overrides apply before the first scan; menu language changes
rescan the selected source asynchronously and reject stale-language results.
The same chosen-world policy controls admission, file reports and A36 runtime
preparation. Unspecified scanner calls retain the existing English policy.
Original-media coverage includes PCML/A36 French/German loose installations
without the standard world. Full language/platform coverage remains open.
Complete in-game translation and other multilingual platforms remain separate
work; this change targets original PC34 dungeon-language routing.

2026-10-16 PC multilingual launch feedback: a requested French/German world
is now revalidated at the launcher input and intent boundaries. Missing or
incorrect selected-language originals produce the existing localized missing-data
dialog naming only DUNGEONF.DAT/DUNGEONG.DAT. AUTO uses the actual chosen
edition rather than a stale saved version. The rendered launch gate remains
an inventory snapshot and performs no added language-file I/O. The subsequent
scan-policy fix above also covers language-only-package discovery.

2026-10-16 A36 original language worlds: CLI/menu language selection now
routes French/German to the selected Amiga 3.6 owner's DungeonF/DungeonG
compressed originals. Authenticate those exact retained bytes before native
decoding and retain their full declared decompressed lengths, as for the same
payloads in the earlier French/German releases. Original ADF, ZIP and loose
world startup are covered; wrong or missing language files reject without
borrowing English. Full source-language graphics selection via G2004 is now
implemented and covered by the Amiga 3.6 original-media graphics test above.
Explicit language inventory now admits supported originals without the
standard world, using the scan policy described above. These world-loading results do not establish full translated or visual parity.

2026-10-16 direct Atari ST disk coverage: the shared 1.0a/1.0b/1.1
original-media regression now selects an unwrapped uppercase-extension STX
beside an independent identical disk copy and its wrapped archive. CLI checks
require the expected graphics identity and loaded world; CLI and M12 loading
both require graphics and dungeon paths rooted in the explicitly selected disk.
The separate wrapped route retains interactive Hall of Champions, C127 selection
and movement checks. Direct original STX 1.0a/1.0b/1.1 now also complete the active Hall
startup, C127 ordinal-14 confirmation and subsequent movement through M12
(see DONE). The shared original-media regression runs both wrapped and raw
owners and retains exact provenance and original graphics hashes. Other
champions/routes and complete platform startup coverage remain open.

2026-10-16 startup-test scope correction: FIRESTAFF_FAIL_IF_NO_LAUNCH checks
whether a launch transaction was accepted, not whether the world finished
loading. An original PC34 SWSH interruption reports launchedEver=1, active=0,
levelLoaded=0 and exits normally even with that guard. A scan-time interruption
reports launchedEver=0 and a failing exit status for both CLI and menu. No
cleanup-guard defect was established; do not move it based on the earlier
short-duration observation. Functional startup proof must additionally require
a loaded world and its exact original graphics/dungeon provenance.

2026-10-16 language-scan cancellation: an interrupted language worker leaves
its previous inventory intact and preserves the requested language. The user
can now retry explicitly through Start; BACK and Update do not restart scans.
Quick Resume and Load return to their own views for the repeated action rather
than redirecting to New Game. Retry and the language snapshot gate apply only
to built-in DM1 catalogue entries. Worker creation failures use the existing
failure dialog with the original return view; allocation-failure injection and
native OS thread failures remain unexecuted integration cases.

2026-10-18 native Amiga launcher backup fallback: ReDMCSB LOADSAVE.C
F0435:2555-2584 opens the language-matched BAK when opening DAT fails, then
renames the backup over the primary at F0435:2901-2908. M11 and M12 now share
the matching DMGAME/DMGAMEF/DMGAMEG BAK path resolver. M12 offers Continue from
that sibling only when reading the configured DAT fails; an existing but
rejected DAT does not unlock the backup. Firestaff keeps original media
read-only and does not perform ReDMCSB's promotion rename. Authentic Amiga v2.0
tests verify the nested BAK is accepted by M11 and that M12 recognizes a
backup-only virtual disk made by changing only the primary file's directory
entry in an in-memory-derived ADF copy. Header, all five parts and dungeon-tail
authentication remain required. Other Amiga save formats and
nonzero-active-group corpus coverage remain open. Audit the PC multilingual
DMSAVEF/DMSAVEG browser filename variants against authentic saves separately.

2026-10-16 PC installation layout: original multilingual data under EUDATA
now authenticates the installation root's uniquely named SWOOSH. Both scan
APIs reject wrong original bytes, missing SWOOSH and arbitrary child directory
names; standalone original EUDATA transport behavior remains covered. Source:
FILENAME.C:45-51 and IBMIO.C G8102:2411-2413. Full desktop/platform coverage
remains open.

2026-10-16 CI handoff seed: launcher-option tests now explicitly exercise stale
English rejection followed by completed German snapshot admission. Production
language gates remain unchanged. Confirm the resulting Actions matrix before
claiming Windows/Linux/macOS execution of this corrected test.

2026-10-16 mixed-case Atari selection: full inventory now restores the selected
DM1 .StX owner rather than losing it after a same-hash neighboring disk wins
initial discovery. Original ST12 coverage checks both scanner APIs and ordinary
M12 initialization; actual ST11 full-menu startup reaches the first active Hall.
Mixed-case ST/MSA preservation inputs and other editions retain separate corpus
coverage requirements. Check the analogous CSB extension predicate during the
CSB phase; the DM1 reproduction does not establish CSB runtime behavior.

2026-10-16 Apple IIGS native intake: original G20E/G21E 800 KiB logical ProDOS
volumes now expose authenticated root/subdirectory files through raw PO, ZIP,
nested ZIP and native 7z APIs. No extraction/cache or new game admission follows.
The complete volume structure is checked before callbacks, and one reusable
buffer is reserved before delivery. Extended forks, DOS sector reordering and
2MG wrappers are outside the current reader. Windows execution still needs
original-media verification; CI compilation is a separate gate. Preflight has
bounded block/entry/depth/path counts but no cooperative cancellation callback.

Remaining native IIGS startup work:

- Bind GRAPHICS.GAME, DUNGEON.GAME, edition-specific DUNGEON.MASTER and
  SYSTEM/START to one volume. The preservation archive has six byte-identical
  graphics/world sources, but only two original START/program owners. A shared
  graphics digest alone cannot admit a complete edition.
- Wire the original-media-verified IIGS graphics reader/decoder into an
  explicitly native startup profile. The shared envelope
  follows MEMORY.C F0479:1330/1361-1381; native raster commands follow
  EXPAND.C F0466:324-845. Do not route IIGS through the FM Towns bitmap decoder.
  Keep record 21 as a native bitmap and record 12 as the unused native font.
- Connect the original-media-verified native LE 0x8104 dungeon intake to the
  explicitly native startup profile. It reuses the MSB-first FTL stream
  decoder (DECOMPDU.C:258-392, including XBA at 380) and the shared full LE
  world parser; preserve the authentic checksum trailer rather than applying
  the older BE route's edition-specific subtraction.
- Connect the original-program-verified native metadata reader to the GS
  runtime profile, rather than the Amiga M558 adapter. It now copies startup
  palettes, 17 UI boxes, 70 mandatory indices, object coordinates/aspects,
  native remaps and all 147 selected wall/floor/door/button frames. Creature
  and projectile aspect/replacement tables remain separate unfinished inputs.
  Read-only source/binary comparison found the GS DATA.C:211-230 entrance,
  interface and six-level dungeon palettes at file offsets 57647, 57679 and
  57711 in both original G20E/G21E programs. These are candidate locations,
  not runtime metadata admission; validate full table extents and native
  executable ownership before wiring them.
  STARTUP2.C:571-577 excludes IIGS from Atari's 558-562 metadata loads;
  authentic IIGS 558/562 are placeholders, not usable replacement tables.
  Independent source/binary comparison also located all 85 native six-byte
  OBJECT_ASPECT entries (DUNVIEW.C:1216-1302, DEFS.H:2020-2030) at file
  offset 53809 in both original programs, and the native UI box table starts
  at 57323. The early Amiga metadata adapter expects a real 4720-byte M558
  record and cannot be pointed at GS placeholders. Native TEXT.C:2016 loads
  record 557 unchanged into G1103: 768 bytes, MD5
  c2e4a7671e51524d85e25430be5ba818. TEXT.C:1094-1207 uses six rows of
  128 character bytes, glyph bits 4..0, a sixth background pixel, advance 6
  and top baseline-4. Explicit GS font binding/rendering now preserves these
  metrics and rejects unsupported codes; wire it into the native UI/text
  consumers. Generic DrawChar and CEDT metrics remain independent.
- Follow STARTUP1.C:192-205, STARTUP2.C:1381-1393, GSTHINGS.C F2179 and
  SWSHIIGS.C for SDL startup. Native audio and first active Hall/input checks
  remain unfinished. Do not advertise a playable IIGS platform until its own
  CLI and M12 execution proves that state with unchanged originals.

2026-10-16 IIGS integration map: reuse existing M12_ARCH_APPLE_IIGS
(menu_startup_m12.h), existing platform display names, asset_status_m12.c
catalogue/scans and main_loop_m11.c launch handoff. Complete-owner recovery
must collect every matching graphics candidate, then authenticate that same
PO chain's program, SYSTEM/START and main world before publishing readiness.
M11 initialization, the metadata cache and the ornament world reread all need
explicit GS dispatch; current BE/Towns flags and the Amiga M558 adapter are
incompatible. Keep FM Towns as AUTO default. Native audio, creature/projectile
metadata and source timing still need work before whole-platform startup proof.

2026-10-16 native GS audio source audit: SOUND.C F0062:767-791 and
F2189:809-856 use BE16 sample length plus signed 8-bit PCM, converted to
DOC unsigned bytes with zero remapped to one and a separate stop marker.
DATA.C:478-533 owns 34 event specs; later effects use records 563-574, while
538/548 are unused placeholders. Do not decode only the legacy 533-555 range
or borrow an Atari RLE/Paula/PIT cadence. F0060:933-1050 uses 15 stereo voice
pairs, native distance/master-volume quantization and DOC frequency increments.
Next steps: authenticate program-owned event/volume tables, verify each used
original sample, then implement native stereo phase/mix transport. Confirm the
DOC board clock against primary hardware documentation before fixing Hz. The
existing SDL transport patterns are reusable; other platforms' complete
playback wrappers are not native GS timing/voice substitutes.

2026-10-16 IIGS prerequisites: same-disk installation ownership and native audio
metadata/PCM/volume/pending rules now pass original-media tests. Next connect
complete owners to catalogue/CLI/M12 selection, then native game-view metadata
and input. Creature/projectile tables, DOC cadence/15-pair stereo transport,
actual audible playback and first active Hall movement remain unverified.
The helpers alone must not mark IIGS playable. Installation candidate-overflow
and allocation-failure injection are additional test gaps.

2026-10-16 ScummVM source reconnaissance: PC and Amiga use distinct wall banks,
layout/clipping and palette handling. Check HoC material/parity and completed
composition/presentation against ReDMCSB before changing rendering. ScummVM's
20 ms VBlank assumption and altered command-queue cooldown semantics are not
cross-platform original timing evidence; no new runtime bug was proven by
this read-only comparison.

2026-10-16 native IIGS runtime integration audit: the actual shared WORLD
constructor accepts the complete decoded LE original main/bonus worlds and
preserves their first map, location, south direction and empty party. This
is a constructor test, not CLI/M12 launch or movement evidence. Before native
launch, bind GS gameplay profiles between F0881 reset and F0195 activation.
F0890d currently hardcodes PC creature data, and direct CREATURE_GetProfile
consumers bypass that accessor. Both sets require per-world profile routing;
a global mutable platform switch would affect unrelated games. Full GS
CREATURE_INFO is 27x26 bytes at original program offset 45084, MD5
a158c8dd19f97c8cd4a8b905ff5d0e6a (both G20/G21). Scorpion Properties is
0x299B, versus PC 0x29AB. Fresh-pool G0236 values are shared with PC in the
original source; compiler flags F0306/F0307 are both zero for GS. Native
rendering/input/audio consumers and catalogue integration remain unfinished.

2026-10-16 IIGS native gameplay profiles: G0243 rows now bind per WORLD after
F0881 reset and before F0195 activation. Native-aware WORLD/M11 lookups,
compiler/skill flags, clone/reset/free and hash identity are implemented and
covered by targeted original tests/source review. No global platform switch.
All 27 defense, size and poison helper inputs exactly match GS originals;
these shared pure helpers can stay shared. Next connect the complete native
installation, world constructor, graphics/font and metadata consumers into
actual M11 startup, then CLI/M12. Native playback and first active Hall input
remain unverified. Save serialization of GS profile/admission is deferred.
Full builds/sanitizers remain on trv2; the host name still fails to resolve,
so this batch used bounded local builds and local-original targeted checks.

2026-10-16 IIGS CLI/M12 first-world startup is now exercised for both original
editions, including native keyboard turns, HoC forward/back steps, idle and
restart. Both scan APIs and launch preparation keep all four files on the
selected ProDOS disk; wrong/incomplete owners fail closed. Native diagnostic
phase is dm1-iigs-runtime; title counters stay zero because no title ran.
This is not complete game/platform support. Next bind native champion/mirror,
mouse, inventory/spell/interaction paths and the remaining door/ornament/
creature/scene consumers; complete source intro/entrance and DOC playback.
Basic source floor/ceiling/walls and HUD are drawn, with visual parity deferred.
Do not promote this constructor/first-world proof to a complete IIGS campaign.

Additional source-discovery evidence: the FM Towns AUTO comparison passes
with its original archive basename beside an authentic IIGS disk. The same
original Towns archive linked as towns.zip was not detected in that mixed
root. Investigate renamed-archive discovery separately; do not claim the
canonical-name priority test proves arbitrary archive-name handling.

2026-10-16 follow-up: the original FM Towns ZIP is now discovered by its
whole-container fingerprint, including the existing large-original-file
hash admission. The mixed `towns.zip`/IIGS CLI regression passes and retains
FM Towns AUTO priority. Inner graphics/world authentication is unchanged.
Renamed repacked containers are not covered by that original fingerprint.

Native IIGS mirror selection, cancellation and resurrection now preserve
original equipment ownership and bind all twenty gameplay skill experiences
and seven lifecycle statistics. Both original editions exercise ELIJA through
ordinary HoC movement, without a synthetic champion or relocation. This is
partial recruitment coverage, not proof for all 24 champions.

2026-10-16 equipment follow-up: native F0299/F0301 bonuses and activation
are wired in original Thing order, including typed/raw object synchronization.
Real walks to ZED, CHANI and BORIS verify torch activation, Moonstone maximum
mana and Rabbit's Foot Luck through selection/cancel/accept and later ticks.
An actual original scroll verifies its source-defined opening/icon operation.
The 24 original mirror chains contain no scroll, Jewel Symal or Illumulet;
their recruitment activation branches lack original runtime coverage. Other
modifier branches and all remaining heroes also need authentic checks.
Remaining: reincarnation/rename, full inventory/spell/interaction and scene
consumers, intro/entrance and audible DOC playback. Visual parity remains
deferred.

2026-10-16 first-descent follow-up: native movement/idle now uses the live
source movement queue, including floor effects and timeline dispatch. The
original HoC exit sensor opens its door; the real stairs reach map 1 with
all fifteen original C04 owners and native profiles. The existing F0194/F0195
transaction owns the map handoff; arrival events remain due on the following
tick. Native periodic effects retain 512-tick torch decay. Native Draw uses
the admitted F0337 light row from GS metadata, with interface colors separate.
Original-media checks distinguish a dimmer row from the old forced row 0.
The shared light admission now accepts bounded ordinary hand objects as zero
light and uses torch charge without an extra Lit condition, as F0337 does.
Remaining interaction, scene, intro and DOC gaps above are still open;
timeline-driven transfers and further levels need authentic runtime checks.

2026-10-16 ordinary IIGS inventory follow-up: native source tables now own
opening, hero switching and thirty body/backpack slots, with native icon banks
and source equipment removal/addition. Original torch/Moonstone/Rabbit's Foot
and occupied cross-champion swaps have runtime coverage. Inventory selection
must remain separate from leader ownership. Remaining: chest contents,
eye/mouth and consumption, native names/scroll text, reincarnation, rest and
all other interaction gaps. Savegame work remains deferred. Derived maxLoad
still needs source F0309 recomputation after strength/stamina/wound/Elven Boots
changes; these current gear cases do not prove that behavior. DOC playback,
intro/entrance and complete scene/campaign functionality remain unfinished.

2026-10-16 maximum-load correction: native IIGS now refreshes capacity
through the existing full F0309 routine, including original Elven Boots.
Both original LINFLAS recruitments and real inventory removal/re-equipment
prove 490/460/490 without changed champion or object fixtures. This closes
the stale-capacity/boots gap above. Runtime capacity under authentic wound,
stamina-depletion and strength-changing equipment scenarios still needs
coverage; these are not proved by the unhurt full-stamina LINFLAS case.

2026-10-16 native food follow-up: G0242 is decoded from offset 45068 in
both authenticated programs and checked over its full 16-byte original span.
Original SYRA Apple consumption is wired to the selected recipient with native
mouth frames and four presentation-gated Delay(8) waits. The host cadence is
an explicit nominal NTSC 60 Hz profile. Toolbox Reference Volume 1 pp.14-48/49
states that SetHeartBeat counts VBL interrupts; p.14-12 documents the machine's
50/60 selection, which these original game bytes do not select. PAL 50 Hz and
exact hardware-frequency validation remain open. Drinks, potions, empty-hand
food/water panel and remaining chest/eye/interaction paths remain unfinished.
Reference: https://mirrors.apple2.org.za/ftp.apple.asimov.net/documentation/programming/65816_gs/Apple%20IIGS%20Toolbox%20Reference%20Volume%201.pdf

2026-10-16 native audio follow-up: authenticated DOC bank, 15 stereo pairs,
64 KB source page allocator/eviction, rational scan clock and isolated SDL
transport are implemented. Native local/positioned/pending routing uses GS
sound IDs and original channel lookup. Both original editions verify complete
PCM against an independent scalar oracle, residency and ownership; SDL dummy
transport verifies format, volume, callback progress, pause/resume and teardown.
This closes the previously unbound DOC-backend gap for the wired requests.
Original party-enter audible floor sensor C01 now has a native request at its
triggering square; the Hall exit pressure plate is covered with original media.
Remaining: wall-click and WORLD object/group audible sensor receipts, original-
runtime projectile/creature sound scenarios and hardware recording comparisons.
The old host metallic/switch substitutes are deliberately not used for GS.
The native wall-bump path now uses F0286 and the WORLD pending combat/F0889
boundary in source order; original-media tests cover the living ELIJA collision.
Nominal clock 7159090 Hz and fixed host headroom remain explicit profiles.
Hardware reference: https://www.brutaldeluxe.fr/documentation/cortland/v4_13_EnsoniqDOC.pdf

2026-10-21 IIGS native entrance follow-up: after F0437 and opening the
selected G20/G21 world, Firestaff presents the original C004 entrance page
with C002/C003 closed doors, waits for a fresh mouse click in the native Enter
region, and runs the 31 F0438 door frames at nominal 60 Hz. F0439's inline
column-major C255 map is rendered from the selected original GRAPHICS.GAME
through the native IIGS wall/material compositor; the loaded campaign map and
party pose are restored immediately after rendering. Direct CLI and selected
M12 menu tests exercise both original editions and require the C255 route and
mouse click to dispatch C200 before the final frame. ReDMCSB IIGS ENTRANCE.C
F0439 constructs C255 inline; STARTEND.C F0797 is the equivalent helper used
by other builds and is not called on this path. Exact F0128 pixel parity,
native GS cadence, credits/bonus/resume commands, source audio sequencing,
and visual parity remain open. This is not a full IIGS entrance parity claim.

2026-10-21 IIGS ZIP launch verification: the original preservation Software.7z
is unpacked only by the test harness and its seven unchanged source disks are
repacked into a private ZIP. M11 admission, direct CLI boot probes, native
first-world ownership and restart/input checks pass from that ZIP with
external archive tools disabled. The user's original 7z remains byte-identical.
The built-in 7z reader does not enumerate this multi-member solid archive, so
runtime use of that exact 7z still requires the explicit external archive-tools
option; ZIP is the tested native route. Failure diagnostics now identify the
IIGS startup stage and selected owner when debug HUD tracing is enabled.
