# Firestaff DONE — CSB

- 2026-10-08: With the current launcher freshly linked,
  `csb_v1_atari_stx_native_cli_boot` passed in 136.02 seconds against the
  authentic Atari STX. It verified title acceptance, the original map-0 start
  pose, first UP movement, remaining directional/action input receipts, and
  the C127 Champion Hall route. Dummy-video runtime evidence only.

- 2026-10-08: With the current launcher freshly linked,
  `csb_v1_fmtowns_native_cli_boot` passed in 111.88 seconds against the
  authentic FM Towns ZIP. The test verified native TITLE.ANM and AUTO
  selection, the first game state, M12 launch, and a captured F31 Entrance
  frame with the authenticated palette; optional save-resume coverage remains
  conditional on an installed original save. Dummy-video evidence only.

- 2026-10-08: Freshly linked the current launcher and reran
  `csb_v1_amiga_native_cli_boot`; it passed in 71.96 seconds against the
  authentic Amiga 3.1 archive. This verifies the native-data CLI startup route
  and its bounded M12/runtime assertions; it does not establish physical M5
  rendering or visual parity.

- 2026-10-08: `csb_v1_atari_st_m12_m11_real_media_handoff` passed against the
  authentic Atari ST v2.1 game and utility disks. Its staging directory now
  uses the test scratch root when `TMPDIR` is unset, rather than defaulting to
  `/tmp`; the original archive is extracted only into that temporary test
  directory.

- 2026-10-08: The persisted-root M12 handoff now selects the authenticated
  Amiga 3.1 English package and confirms the native A31E M11 boot profile.
  The expanded original-media CTest passed against the installed collection;
  the dedicated A31E title-to-runtime handoff also passed. The mixed-platform
  FM Towns AUTO test passed with original Amiga, Atari ST and FM Towns editions
  installed, reaching the source MINI.DAT party through M12.

- 2026-10-07: Seven installed authentic CSB CLI-start tests passed in the
  multi-edition startup matrix, covering Amiga, Atari ST and FM Towns media.
  The Atari R1 campaign/Utility Disk test and French Atari preservation ZIP
  test skipped because those specific archives are not installed; no substitute
  data was used.

- 2026-10-07: Added a mixed-original-media M12 AUTO regression for FM Towns.
  With authentic Amiga, Atari ST, and FM Towns packages installed together
  and no `--platform` option, the menu authenticates all three, selects the
  documented FM Towns default, consumes its source-owned TITLE.ANM and
  SWITCHTW/C004 input path, and reaches the original MINI.DAT party at map 4,
  `(22,18)`, facing south. The first presented dungeon frame has a nonzero
  viewport hash. The focused real-media CTest passes; this verifies routing
  and startup state, not visual parity.

- 2026-10-07: The authenticated Amiga 3.1 M12/M11 handoff regression passed
  with 55 assertions, no failures and no skips. It exercised the source-bound
  CSB SWSH sample through the temporary intro audio owner, checked the
  launcher master/music/SFX preferences, and confirmed mute preserves the
  sample identity, duration and host pause. This covers the original-media
  audio preference path; it does not verify physical speaker output.

- 2026-10-07: Both authentic CSB Amiga startup paths passed: the ADF archive
  route in 10.84 seconds and the native Amiga ZIP route in 70.34 seconds.
  These exercise separate original-media handoffs through the CLI/startup
  flow; neither claims physical M5 output or visual parity.

- 2026-10-07: Corrected `csb_v1_atari_original_archive_cli_boot` to consume
  the installed authentic STX-in-ZIP archive instead of its parallel `.7z`
  copy. The ZIP route passed in 3.28 seconds with external archive tools
  disabled, verifying the original Atari title hash, map-0 start pose, first
  UP movement and M12 start-menu launch.

- 2026-10-07: `csb_v1_atari_stx_native_cli_boot` passed in 134.07 seconds
  against the installed original Atari STX. The normal source-owned startup
  reached the expected CSB entrance route without a synthetic game-data
  substitute. This headless check does not establish physical M5 presentation
  or visual parity.

- 2026-10-07: Corrected the real-media M12/M11 Amiga handoff CTest to match
  the installed ZIP's verified A31M identity. The prior configuration required
  A31E and silently skipped, while the archive's authenticated GRAPHICS.DAT
  hash is A31M (`61fbfd56887c94adc26888a9491c6611`) and its source owner is
  `TITL.DAT`. The corrected test passes 55 assertions with no failures or
  skips and reports the `a31m-titl-dat` handoff from the original archive.
  This verifies the selected-menu handoff into the native title route; it does
  not replace the separate first-runtime-frame evidence or claim pixel parity.

- 2026-10-06: Repacked the authentic Atari ST v2.1 game-disk STX from the
  local preservation archive into ZIP for the native archive route. The direct
  CLI test now waits through the source ANIMATE.SCR sequence before Enter and
  asserts the original map-0 spawn at (9,0), facing south, with a nonzero
  viewport receipt; its first UP command reaches (9,1). Native STX and ZIP
  startup/input/menu routes pass against that original disk. These headless
  checks do not prove physical Mac M5 HiDPI display quality.

- 2026-10-06: Re-ran the authentic Atari ST archive startup/input route and
  FM Towns English start-menu route against the local original media. The
  Towns test confirms the MINI.DAT party at map 4, position (22,18), direction
  2, one champion, and a nonzero source viewport receipt; default bare
  `--game csb` selects the authenticated FM Towns edition in the current data
  collection. These SDL dummy-video tests do not verify physical Mac M5 HiDPI
  output, displayed viewport quality or audible device output.

- 2026-10-05: Bound the local Atari ST v2.1 `csb.s` textual disassembly to
  the PP hard-disk `CMAIN` image with three byte-for-byte code anchors at
  labels `u0000`, `u0006` and `u0624`; the exact `CMAIN` SHA-256 is recorded in
  `parity-evidence/csb_atari_v21_cmain_disassembly_binding.md`. The report
  limits this result to the PP hard-disk binary: retail STX-to-source binding
  and ReDMCSB F-number mapping remain open.

- 2026-10-04: F31 mode-2 gameplay sound requests now create the original
  C20 timeline event for the following game tick, before distance is tested.
  The due event reads C.SoundIndex separately from A.Priority and joins the
  existing per-index PCM queue; modes 0/1 and other editions retain their
  paths. English and Japanese original-media tests pass after the Entrance
  handoff and verify due-tick selection and following-tick playback with
  distinct sound priority and sample index. This test injects a C20 event
  into the authentic live runtime; natural closed-door melee and M5 speaker
  output remain unverified.

- 2026-10-04: The active F31 dungeon viewport now binds the original
  current-level floor-ornament selectors and dungeon seed before F0108/F0128
  rendering. English and Japanese original-media tests pass from the packed
  ZIP and compare the live render transaction's bound metadata with the
  authenticated MINI.DAT level. This checks source handoff, not full
  presented-frame or physical M5 viewport parity.

- 2026-10-03: Bare `--game csb` with a persisted collection root now scans
  its CSB leaf directly while retaining the root for the game menu. The
  original-media scan selects the authenticated FM Towns edition by default.
  This check covers selection and startup scanning, not physical audio or
  dungeon presentation.

- 2026-10-03: FM Towns F31 now retains the loudest pending request for each
  sound index and flushes all requested indices in source order on the next
  tick, following ReDMCSB SOUND.C F0064/F0065. Audible corridor generators
  and wall sensors now use the edition-specific source distance gate. The
  macOS Actions build and an original-media test passed for both F31 English
  and Japanese: two different authenticated sounds complete in one tick, all
  35 source PCM events decode, and the packed M11 runtime starts from the
  preserved ZIP. This is a queue and payload check; physical speaker output
  and full dungeon audio parity remain open.

- 2026-10-03: The original-media FM Towns CLI regression now covers bare
  `--game csb --data-dir <F31 ZIP>` as well as explicit `--platform fm-towns`.
  It requires the authenticated F31 English or Japanese edition, TITLE.ANM
  handoff, and an unloaded dungeon at the first frame. The full English
  CLI/menu/Entrance regression and the Japanese bare-title probe passed with
  the preserved F31 ZIP. This is startup-route coverage, not an audible-device
  or full gameplay parity claim.

- 2026-10-02: Extended the source DB5 weapon pass to the Atari D1C front
  cell with F0129 fixed-point D2 scaling and G0214 palette. The authentic
  Utility STX MINI map-9 pose draws weapon `0x144e` from original graphic 372;
  all 78 opaque scaled pixels match the independent source sampling receipt,
  and 73 pixels change when F0267 unlinks it. The Game/Utility STX boundary
  suite passes 4,554 checks with no failures or skips.

- 2026-10-02: The Atari ST viewport now draws first-and-only DB5 weapons
  in the open D1C and D0C back cells using the 46 source object aspects and
  original GRAPHICS.DAT bitmaps. A real Atari Game/Utility STX test resumes
  MINI map 6, matches weapon `0x1423` against graphic 372 in both positions,
  and confirms the pixels change when source F0267 removes it. All
  admitted presentation modes pass; other item cells and creature classes
  remain open.

- 2026-10-02: A C37 creature group falling through an open pit no longer
  requests the teleporter BUZZ sound. ReDMCSB MOVESENS.C F0267 reserves that
  request for audible teleporters. The existing group-fall fixture retains
  its creature movement sound and fails against the previous extra request.

- 2026-10-02: Atari ST PSG playback now interrupts its preceding queued cue
  when ReDMCSB SOUND.C F0060 replaces the Timer-A sample pointer and count.
  The authenticated v2.1 `ANIMATE.SCR` title cues occur at source VBlanks
  1107 and 1128; the first 3,103-sample cue lasts longer than their 21-VBlank
  separation. A dummy-SDL original-media test verifies that the second cue
  replaces the first queued PCM tail. Physical-device output and the three
  separately unresolved retail SND1 rows remain open.

- 2026-10-02: An authentic Atari ST M12-to-M11 route now recruits two
  champions, then presses F1 and F2. ReDMCSB PANEL.C F0355 keeps G0423's
  inventory owner separate from CLIKCHAM.C's G0411 leader; M11 now does the
  same. The original-media handoff test verifies F1 selects inventory ordinal
  1, F2 selects ordinal 2, and the first champion remains the leader in both
  the M11 mirror and GAMEBLOCK (4,553 checks, no failures).

- 2026-10-02: The production CLI script accepts source F1-F4 champion keys
  through the same SDL-to-CSB input mapping used by live keyboard events
  (ReDMCSB COMMAND.C:245-260). The authenticated Atari ST archive route now
  presses F1 then F2 after recruiting two C127 champions and confirms that
  the inventory panel opens while the two-champion party and map pose remain
  intact. The separate M12-to-M11 receipt identifies the selected champion.

- 2026-10-02: The authentic Atari ST CLI route now continues from its first
  recruited champion to the next source-authenticated C127 mirror. It follows
  open map squares (12,7) → (11,7) → (11,8), faces east toward the C127 wall
  at (12,8), selects its portrait at source screen center (112,82), confirms
  through C160, and resumes movement to (11,7). The final real-media receipt
  shows two champions, a closed candidate panel, and a nonzero viewport hash.
  The extended test uses `wait:32` host loop frames between commands so
  movement cooldowns expire; these waits are not original Atari VBlanks.

- 2026-10-01: The authentic Atari ST M12-to-M11 route now follows production
  input and collision from the untouched empty-party spawn at map 0 (9,0),
  facing south, to retail C127 ordinal 4 at (10,7), clicks its C026 portrait,
  and confirms C160 as party leader. The source viewport origin is (0,33), so
  the portrait click uses source screen coordinates (112,82). C040 command
  handling now precedes the overlapping C017 inventory hit tests, matching
  ReDMCSB COMMAND.C. The Atari handoff passed all four presentation modes
  (4,537 checks); a further original-media route confirms that movement
  resumes with the recruited leader and synchronized GAMEBLOCK coordinates.
  The original Atari 7z CLI regression now also drives this complete path with
  `wait:32` host loop-frame tokens, which let movement cooldowns expire, then
  asserts C127 ordinal 4 and one recruited champion, then verifies another
  eastward movement to (10,7) with a live viewport receipt. These waits are
  not original Atari VBlanks.
  The A31M real-media handoff passed 55 checks.

- 2026-10-01: Retired the CSB FM Towns startup test that required the
  separately supplied RAR and external archive tools. The registered native
  test uses the authenticated FM Towns ZIP and already covers CLI startup,
  start-menu selection, CUE/BIN handoff, and the live campaign route. Both
  English and Japanese ZIP-backed CTest startup/menu rows passed on the
  authenticated bilingual archive (111.57 and 111.16 seconds). RAR ingestion
  is no longer a requirement for this supported startup path.

- 2026-10-01: Re-ran the authentic A31 Amiga CLI suite on the macOS host.
  Original and Modern title handoffs reached runtime movement, the complete
  initial-input matrix passed, the M12 start menu published a nonzero source
  viewport receipt, and AUTO discovered the installed A31E archive. SDL used
  its dummy video/audio drivers, so this does not verify native HiDPI output
  or audible device playback on MacBook hardware.
- 2026-10-01: ReDMCSB confirms the Atari new-game start at map 0 (9,0),
  facing south, with zero champions is intentional. `LOADSAVE.C` initializes
  an empty CSB party, and `CLIKMENU.C` explains that movement without champions
  is safe on original maps that contain champion mirrors and no creature types.
  Firestaff's authentic-media regression verifies a route to a C127 mirror
  and recruitment of the first champion. The supplied Atari save disk contains
  no campaign save files; no party or save was synthesized.
- 2026-10-01: The original Atari ST v2.1 Utility Disk `ANIMATE.SCR` trace
  requires exactly 2,036 source VBlanks (40.72 seconds at 50 Hz) before its
  FTLCODE handoff. The real-media parser verifies the sequence and now prints
  that measured duration for startup diagnostics; host script `wait:N` counts
  loop frames and must not be confused with source VBlanks.

- 2026-10-01: The production CLI also reached CSB Atari ST runtime from the
  authentic ST 2.0/2.1 media in the installed 7z archive. The `ANIMATE.SCR`
  handoff completed, `levelLoaded=1`, the runtime advanced 2,459 ticks, and a
  nonzero source viewport receipt was published. This long source-sequence
  probe used SDL dummy video/audio and does not verify displayed viewport
  quality or audible sound.

- 2026-10-01: Removed the hash-based Atari ST SND1 final-hold approximation
  from production playback and deleted its public decoder API. ReDMCSB
  SOUND.C F0061 lines 1164-1209 disables Timer A when the declared sample
  counter reaches zero; it does not establish a substitute sample when
  packed source data ends early. The authentic hard-disk and retail v2.1
  floppy carriers still agree on all 22 row fingerprints. Their three
  bounded-decoder failures are now explicitly rejected by the real-media
  transport test instead of being presented as original audio. Recovering
  those sounds still requires an
  authentic Atari memory-boundary trace; see TODO-csb.md.

- 2026-09-29: The authentic FM Towns EN/JA Game-handoff regressions now run
  SDL's dummy audio backend and verify that C0_MUSIC_ENTRANCE resolves to
  physical CD-DA track 02 and queues the selected original CUE/IMG PCM in an
  active, unpaused stream with remaining source time. Both real-media tests
  pass. This proves source-track selection and stream acceptance, not audible
  hardware or original-vs-Firestaff sound parity.

- 2026-09-29: Atari ST SND1 playback now maps the F0061 all-zero PSG
  amplitude registers to PCM silence in both loud and soft source modes.
  A bounded source-format regression verifies that a held zero-level source
  remains silent and that a nonzero source-format level remains audible.
  Physical-device sound and captured retail sound parity remain open; the
  supplied Atari archive has three sound rows that the current
  packed-stream decoder still rejects, as tracked in TODO-csb.md.

## 2026-09-29 — Retain Amiga selection after presentation Back

- Verified the menu Back/reselect sequence with the authentic CSB Amiga
  archive and the existing A31E startup handoff checks. Added the sequence
  to the original-media startup script. This verifies launcher navigation;
  it does not establish complete game parity.

## 2026-09-29 — CSB M12 viewport receipt

- The runtime probe now reports the source-owned CSB viewport aperture hash.
  The Atari ST, Amiga and FM Towns real-media M12 regressions require a
  nonzero hash after the launcher reaches runtime. All three pass against
  their authentic STX/7z, Amiga ZIP and FM Towns ZIP data. This proves the
  source viewport renderer ran on the menu handoff; it does not establish
  native macOS display behavior, pixel parity or audible output.

## 2026-09-28 — Read the authentic Atari Utility Disk 7z in memory

- The bounded native 7z reader now handles the multi-member preservation
  archive used by CSB Utility Disk discovery. Authentic `HCSB.DAT`,
  `HCSB.HTC`, and `MINI.DAT` identities are read without unpacking the archive
  or enabling host extraction tools.
- `csb_v1_hint_oracle_native_7z_cli_boot` passes against the installed archive
  through both `--csb-hint-oracle` CLI startup and the normal M12 start-menu
  handoff. This verifies the Utility Disk/Hint Oracle route; it does not claim
  that the separate Atari campaign archive has native 7z startup support.

## 2026-09-28 — Installed-root AUTO start-menu route

- With a clean configuration and the authentic installed data root, AUTO
  selected the Amiga A31E FTL archive, reported its `a31e-appb-bjeload-c03`
  handoff, and the M12 start menu reached the source-owned runtime at map 0,
  position (9,0), facing south, with zero champions. The Amiga real-media CLI
  test now preserves this no-platform route when the original FTL archive is
  available, separately from its curated explicit-platform check.

- 2026-09-27: Corrected the M12 diagnostic for an authentic Atari ST archive
  when 7zz is installed but external archive scanning has not been opted in.
  The launcher now explains that scanning is disabled and names
  `--enable-external-archive-tools`; it no longer tells the user to install a
  reader that is already present. The focused opt-in-popup CTest passes with
  the supplied original archive, and the Atari ST CLI/start-menu regression
  still reaches the original C200 runtime handoff.

- 2026-09-26: The authentic Atari STX M12 launch regression now continues past
  ANIMATE.SCR to the source-owned C004 entrance, sends a primary click at the
  original C200 hit box, and requires the startup receipt to reach the
  inactive/runtime phase. The 1,800-frame script delay lets the retained
  animation finish before input; the 960x600 test surface maps window point
  `(813,156)` to source point `(271,52)`, inside the documented
  `(244,45,55,14)` rectangle. `csb_v1_atari_stx_native_cli_boot` passes against
  the original STX archive. The retail initial party still has zero champions,
  so this proves the entrance-to-runtime transition, not a playable party.

- 2026-09-26: Verified the Atari ST software archive's original save-disk MSA
  using `csb_v1_atari_msa`. The preserved image is a valid 720 KiB disk with
  an empty root directory (SHA-256
  `bca3db90f795c633fcb0cc7a10a4811dae616b7d8e7eb8b65b4f59af10598d29`), so
  it is not an authentic DSA-bearing campaign save. `TODO-csb.md` now
  distinguishes that blank formatted medium from the still-missing save corpus.

- 2026-09-25: Re-ran the authentic native startup checks for the Atari STX,
  Amiga 3.1 and FM Towns original packages. All three CTest routes passed;
  the Atari route covers title, runtime, input and keyboard/pointer launcher
  selection, while the Amiga and FM Towns checks cover their source-owned
  startup phases and native campaign handoff. This run does not establish
  missing save-corpus or original-frame parity items.

- 2026-09-25: Fixed the F31 FM Towns direct CLI save handoff to retain the
  source-owned switch/game bind and apply the selected F0435 startup state in
  explicit sequential steps. The authenticated Japanese `CSBGAME-JP.DAT`
  passes the native M11 handoff test, and the complete FM Towns CLI/menu
  regression confirms both direct launch and M12 Quick Resume restore it. The
  same regression verifies v20/v21 entrance routes; the save corpus is
  external and remains unchanged. No save-writing or other-edition parity is
  implied.

- 2026-09-25: Removed the manual loose-file prerequisite from the native Atari
  STX CLI/menu regression. When the default raw STX is absent, the test stages
  only the authentic English v2.1 STX member from the supplied Atari 7z into a
  temporary directory, then passes those raw bytes to Firestaff's native
  reader. The archive remains untouched and CI still skips cleanly when no
  original corpus is present. The full test now waits through the source
  `ANIMATE.SCR` sequence in the normal M12 loop and verifies the M11 dungeon
  entrance; it passed with automatic staging. The source state has zero
  champions, so it does not establish a playable campaign party.

- 2026-09-25: The A31M title-package search now bypasses single-path inventory
  hits when its batched MD5 request repeats the same hash. The recursive scan
  can therefore return distinct files instead of filling every slot with one
  cached path. The hash-scanner regression passes, and a scanner probe against
  the supplied Amiga 3.1 English/French/German 7z locates authentic `TITL.DAT`
  and `Graphics.DAT` in the same ADF. The archive is read in memory; no game
  data is extracted or written to disk.

- 2026-09-24: Kept explicitly selected Atari ST, STX, and MSA media as the M12
  search root instead of widening the scan to its parent directory. A parent
  scan had spent 23.6 seconds inflating and hashing unrelated neighboring
  archives before the direct Atari start; the same authentic STX now reaches
  the M11 boot probe in under 0.2 seconds after renderer initialization.
  Extension matching is case-insensitive, and the M12 regression preserves
  mixed-case `.St`, `.sTx`, and `.MsA` selections as their exact media roots.
  The Atari animation is held in a hash-verified, process-local session for
  its source VBlanks and SND1 cues; no game files are written to disk. The full
  authentic Atari CLI matrix, including M12 menu start through live gameplay,
  passes. The source state still has no champions, so no playable-party claim
  is made.

- 2026-09-24: FM Towns M11 now validates the selected native save filename
  before preparing a filesystem parent directory. This makes a read-only
  packed `MINI.DAT` fail at the explicit native-writeback boundary instead of
  being misreported as a filesystem save failure. The authentic-media
  `csb_v1_fmtowns_ja_m11_real_media_handoff` regression passes.

- 2026-09-24: M12's Quick Resume gate now validates an external FM Towns
  `CSBGAME-JP.DAT` against the selected, hash-verified F31J C03 program using
  the native F0435 reader. A real-media regression confirms the Japanese save
  path reaches the M12 launch intent, while the incoherent English candidate
  remains rejected. This covers the launcher handoff; Atari/Amiga DSA-bearing
  saves and CSBWin extended saves remain unavailable, as recorded in TODO.

- 2026-09-24: Fixed normal CSB Quick Resume so an authenticated save admitted
  by the complete native resume predicate crosses M12's launch intent without
  requiring the separate optional CSBWin DSA-corpus identity. The regression
  starts from a valid serialized CSB save with that identity unset and checks
  that the exact path reaches the intent. DSA-bearing original-media resume
  remains deferred with the missing source corpus documented below.

- 2026-09-24: Fixed the Atari ST real-media test so it no longer combines
  incompatible `--menu` and `--boot-probe` modes. Direct CLI probes verify
  Original/Modern presentation; the normal menu route verifies that the
  authentic STX reaches the live source gameplay runtime at map 0, position
  (9,0), facing south. This source state has zero champions, so it is not proof
  of a playable campaign party.

- 2026-09-16: Revalidated native launch coverage against the supplied retail
  Atari STX, Amiga ZIP/ADF, and FM Towns ZIP without external runtime
  emulators or extraction. Atari passed campaign title, input matrix and
  CLI/start-menu launch in Original and Modern. Amiga passed source startup,
  title input, runtime movement and start-menu media retention. FM Towns
  passed title, MINI.DAT initial party state and start-menu launch, while its
  captured Original, Modern and Custom Entrance paths retained C28 and closed
  C002/C003 doors without the broad-red C004 regression. These are bounded
  native launch/presentation checks; campaign saves, original emulator image
  pairs, audio comparisons and DSA-bearing saves remain open in `TODO-csb.md`.

- 2026-09-08: Added an authentic FM Towns Entrance screenshot regression for
  the F31 ZIP. It verifies the source C28 palette, closed C002/C003 entrance
  composition and a broad-red C004 failure signature. The verifier accepts
  C28's legitimate 40-pixel red title detail at `(224..233, 8..21)` while
  rejecting a large red field. The real-media result is 59,886 non-black
  pixels and 13 colours; no game member is materialised on disk.

- 2026-09-08: The Swedish F31 FM Towns catalogue is complete at the gettext
  level.  `msgfmt --check po/csb.sv.po` succeeds and
  `msgattrib --untranslated po/csb.sv.po` produces no entries.  This corrects
  an obsolete TODO count of 177 untranslated entries; it is not a claim that
  all languages, Japanese glyph-raster parity, or every live text consumer is
  fully verified.  The authenticated F31 graphics/dungeon identities remain
  `761d6fc588b31aeaaa9caf3725e111b9` and
  `7ca51c17ef8bd542ca5f0273672ec1a5`.

- 2026-09-07: Routed C37 wandering through the creature-owned F0202
  destination gate for wall/stairs, open pits with levitation, closed/
  imaginary fake walls, and the non-material door exception
  (GROUP.C:1500-1564, called from the C37 false-imaginary route). This
  replaces the generic gate's accidental admission of open pits and imaginary
  fake walls. It now also decodes the selected raw C00 door and applies the
  source horizontal threshold of one or vertical `M051_CREATURE_HEIGHT`
  threshold from creature Attributes[7:8]; malformed C00 ownership rejects
  the move. The full native CSB runtime accumulator passes 840 assertions,
  including a state-C2 vertical-door C37 fixture for a height-one creature.
  F0202 also rejects an open group-scope teleporter for a wary creature when
  its raw TargetMapIndex allowed-type list excludes that creature, matching
  F0139 rather than admitting it through the generic C05 gate. Archenemy
  fluxcage remains open work.

- 2026-09-07: C37 wandering no longer creates a local map/time-derived RNG.
  The native runtime advances the persistent `G0349` counterpart once for
  GROUP.C F0209's `M005_RANDOM(2)` gate and, when admitted, once for the
  absolute `M004_RANDOM(4)` direction. Its bounded direction scan now also
  consumes the source prior-square one-in-four draw only for that square.
  The regression seeds G0349 with 29 and proves east is chosen despite a
  north-facing C04, as well as the exact two-step resulting state. This is
  source/RNG evidence, not a claim that all F0202/F0267 C37 movement and
  attack branches or original-capture parity are complete.

- Extended the original-media FM Towns EN/JA startup matrix to include
  Filtered (v20), alongside Original (v1) and Upscaled (v21). Both complete
  scripts pass with optional user saves unset, checking retained mode,
  map 4/(22,18,2), one champion, title and menu admission. Tested against
  the existing public build at runtime 537ca520c; this is mode selection
  and initial-state evidence, not filter appearance or movement parity.

- Rechecked original FM Towns EN and JA archives with the rebuilt native
  executable (runtime 537ca520c): both pass TITLE.ANM, modal SWITCHTW,
  original MINI.DAT initial party state, v1/v21 runtime admission and start
  menu routes. Optional user saves were explicitly unset. Corrected the
  script's PASS wording from runtime/movement to initial party state because
  it does not prove dungeon movement. No pixel/audio/emulator parity claim.

- Rechecked native Atari nested ZIP/STX and Amiga ZIP/ADF startup with
  the rebuilt executable after the DM1 group-event changes (runtime
  537ca520c). Both original-media scripts pass title/entrance, menu launch
  and movement checks. The Atari test now explicitly unsets the diagnostic
  external-extractor opt-in, matching the Amiga contract, and passes when
  invoked with that flag set in its parent environment. These are bounded
  startup tests, not original rendered/audio or complete campaign parity.

- Japanese F31 startup independently passes the same native FM Towns CLI
  suite with FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE=ja and no optional user save.
  TITLE.ANM, SWITCHTW and MINI.DAT transitions pass in v1/v21, with map 4 /
  party 22,18,2 and one champion. Keyboard and mouse launcher receipts retain
  variant=csb-fmtowns-ja; the original archive hash remains unchanged. This
  closes Japanese coverage of these startup checks, not Japanese pixel/audio
  parity or a complete campaign verification.

- 2026-09-06 public executable recheck: authentic nested Atari ZIP->ZIP->STX
  CLI title, runtime entry, first UP movement and start-menu launch pass.
  Authentic Amiga ZIP->ADF entrance, CLI/menu, seven fresh-session input
  commands, pointer-only cards and unchanged archive hash pass. English FM
  Towns TITLE.ANM->SWITCHTW->MINI.DAT passes in v1/v21 with original map 4,
  party 22,18,2 and one champion; keyboard/mouse launcher paths and unchanged
  archive hash pass. Ran the existing platform CLI boot scripts against the
  current native binary. No optional user save was supplied. These headless
  receipts do not prove pixel/audio/timing parity or Japanese F31 coverage.

- 2026-09-06: Original FM Towns CDATA/CJDATA GRAPHICS.DAT regression now reads
  all 728 asset spans in each edition, checking contiguous offsets, in-buffer
  bounds and rejection of the first invalid index. ZIP/IMG and both files are
  read in memory. Both real-media cases pass; this proves container indexing,
  not palette correctness or visual parity.

- 2026-09-06: Group pit consequences check source levitation attributes before
  lower-map movement (MOVESENS.C F0264:136-146, F0267:538). The bounded C37
  flying-eye fixture preserves upper-map chain ownership, health and behavior
  scheduling; the non-levitating rat still falls. Disabling the guard fails
  three assertions; restoring and rebuilding passes the runtime regression.

- 2026-09-06: C25 group damage honors Defense=255 after the F0191 random
  damage draw (GROUP.C F0190:826-829, MEDIA720). A bounded Lord Chaos record
  retains health and carried possessions and emits no death smoke. Disabling
  the guard fails all three checks; restoring it and rebuilding passes the
  runtime regression. This does not verify other damage owners or original
  platform captures; the legacy C25 burst timing remains a separate boundary.

- 2026-09-06: The C25 group-damage owner now visits creature slots from Count
  down to zero (ReDMCSB GROUP.C F0191:961-967). The runtime test checks exact
  surviving HP and subsequent fixed-drop cells, including lower-slot death
  and survivor compaction. The existing flee-branch fixture uses seed
  `0xC5B10701` to reach that branch with the corrected draw order; its three
  behavior/active-state assertions remain intact. Runtime, F0191 fall receipt,
  F0266 move/projectile receipt and F0247 teleporter impact tests pass after
  rebuilding. AArch64 debugger rollback observation still passes. These checks
  do not establish original-media campaign or cross-platform parity.

## 2026-09-06 — Fixed and carried drop RNG ordering

- F0186 now allocates before cell RNG and publishes floor effects before the
  next optional decision. F0186/F0188 share F0190's caller RNG rather than
  independently deriving seeds from coordinates and time.
- F0190 rollback restores the external caller RNG as well as profile/dungeon.
  The runtime regression passes 822 assertions, including six fixed-drop cells
  and eight carried-drop cases. Reintroducing F0186 reseeding fails four cell
  checks. AArch64 GDB observes one failed-death rollback preserving its seed;
  removing restoration makes that observation fail, and the restored build passes.
- Evidence and scope: `docs/parity/DM1_FIXED_DROP_ALLOCATION_ORDER.md`.
  These are source-shaped native fixtures, not full original-game RNG parity.

## 2026-09-06 — Scoped combat regression check

- Rebuilt and passed four tests after shared DM1/CSB runtime changes:
  `csb_v1_grey_lord_combat_pc34_compat`,
  `csb_v1_combat_bugfix_helpers_pc34_compat`,
  `csb_v1_f0193_giggler_steal_receipt_pc34_compat`, and
  `m11_csb_f0247_boot_projectile_frame_pc34_compat`.
  These are bounded compatibility/runtime fixtures, not original-media
  playthroughs, full XP-path verification or proof of platform parity.

## 2026-09-05 — Amiga chest material admission

- The original ZIP/ADF graphics test now requires C025 to decode to exactly
  144x73 four-bit indexed pixels, matching CHEST.C F0333 and DATA.C G0032.
  The supplied preservation archive passes without disk extraction.
- Rebuilt Firestaff also passes Atari STX and Amiga CLI startup regressions
  after the Atari chest compositor change. Neither check establishes Amiga
  chest interaction or complete visual parity.

## 2026-09-05 — Atari chest transparency preservation

- Expanded the panel comparison to all 10,512 C025-area pixels, including
  its 848 transparency-key pixels. The oracle retains the pre-open viewport
  underneath key 8, then overlays original atlas crops and M653 text.
- Original, V2.0 and V2.1 pass. The Modern runs each cover 15 nonempty
  original containers. This establishes composition against supplied media
  and background preservation, not same-state emulator pixel parity.

## 2026-09-05 — Atari chest material composition

- Added the original C025 panel and eight original atlas-icon crops to the
  Atari inventory compositor, using retained open-chest slots so holes do
  not compact while the panel remains open.
- Original, V2.0 and V2.1 pass background and icon pixel comparisons for
  every tested nonempty original container, including the existing M653
  life-force overlay, alongside inventory and chest pickup checks.
- Transparent background preservation and same-state emulator comparison
  remain separate verification work; no full visual-parity claim is made.

## 2026-09-05 — Atari native chest pickup routing

- Added the eight open-chest input slots using the selected C232 icon
  coordinates and the Atari viewport origin (48,33). Chest boxes use their
  original 16x16 area, not the 18x18 champion-slot frames, and are inactive
  without an open chest.
- The original STX/MINI.DAT corpus picks up each resident of every nonempty
  container, retaining visible holes until close. This passes with the full
  inventory corpus in Original, V2.0 and V2.1.
- This is input evidence only: the Atari inventory compositor still needs
  its C025 chest background and resident-icon drawing layer.

## 2026-09-05 — Atari cross-slot backpack dragging

- Original, V2.0 and V2.1 pass 7,599 cross-slot drags each: every allocated
  original object is picked up in each backpack slot and released into the
  next slot (wrapping the seventeenth back to the first). Both source and
  destination ownership and the empty leader hand are asserted.
- The complete input corpus still passes its empty-slot and occupied-slot
  checks. Drag coverage is this 17-edge route, not every possible slot pair
  or equipment/chest destination.

## 2026-09-05 — Atari occupied-slot exchanges

- All 447 allocated original objects now also undergo occupied-slot input
  checks across all 30 slots in Original, V2.0 and V2.1. A distinct allocated
  original weapon occupies the destination; C559 bytes decide whether the
  incoming object swaps with it or leaves both owners unchanged.
- All 40,230 occupied-slot checks pass alongside the existing empty-slot
  pickup/replacement checks. Controlled placements are test-only; this does
  not claim all possible object pairs, drag destinations or chest panels.

## 2026-09-05 — Independent Atari allowed-slot oracle

- The original-object inventory test now expands selected GRAPHICS.DAT C559
  directly and reads the big-endian allowed-slot word from each six-byte
  ObjectInfo entry. A separate F0141 index calculation uses original dungeon
  record fields rather than the runtime's ObjectInfo helper.
- All 447 allocated objects agree with the runtime's allowed-slot values.
  Original, V2.0 and V2.1 each pass all 13,410 object/slot checks with this
  byte-derived oracle (40,230 total).

## 2026-09-05 — Atari equipment-slot input coverage

- Extended the allocated original-object corpus to all 30 inventory slots:
  13,410 object/slot checks per presentation, passing in Original, V2.0 and
  V2.1 (40,230 total). Both permitted replacement and rejected replacement
  retain the expected inventory/leader-hand ownership.
- Expected slot masks are transcribed independently from ReDMCSB DATA.C
  G0038; object allowed-slot values still use the runtime media decoder.
  This is input/ownership evidence, not independent proof of that decoder.

## 2026-09-05 — Original Atari backpack corpus

- Expanded the original STX/MINI.DAT pointer regression to all 447 allocated
  weapons, armour, scrolls, potions, containers and junk records across all
  17 backpack slots: 7,599 pickup/replacement roundtrips per presentation,
  passing in Original, V2.0 and V2.1 (22,797 total).
- Unused allocations are excluded using ReDMCSB DUNGEON.C F0166's
  `Next == THING_NONE` rule. Controlled placement is confined to the test
  runtime and each slot is restored; original media remains unchanged.
- Equipment restrictions, occupied-slot swaps, chest-panel interactions and
  pixel comparison with the original executable are separate coverage.

## 2026-09-05 — Atari inventory mouse release

- Fixed same-slot release undoing an Atari inventory pickup. The release
  guard now resolves Atari's native slot coordinates as well as F31's.
- Reproduced the failure and verified the fix in Original, V2.0 and V2.1
  with the original STX and utility MINI.DAT. The utility champion starts
  unequipped; the test temporarily places an existing dungeon weapon in a
  backpack slot and restores the empty slot afterward. No media is changed.
- This proves that pickup/replacement roundtrip, not every Atari item or
  chest slot. Full Atari/Amiga inventory coverage remains open.

## 2026-09-05 — F31 chest owner transitions

- Original EN/JP archive tests pass same-owner reopening with an empty slot
  and switching to another chest while holding the first chest's resident.
  They compare both original linked lists and the held Thing in Original
  and V2.1, using controlled in-memory placement of original records.
- The expected behavior follows ReDMCSB CHEST.C F0333, lines 30–75:
  retain same-owner slots, close the previous owner before loading another.
  These checks do not establish save/resume or pixel-level parity.

## 2026-09-05 — F31J F0168/F0646 inscription byte pipeline

- Implemented the distinct ReDMCSB F31J second decoding pass that restores the
  packed Shift-JIS stream from F0168's A..P representation, including literal
  prefix and terminal inscription-marker rules.
- Implemented bounded F0646 line selection with exact 16-pixel Shift-JIS,
  8-pixel ANK, zero-width control and explicit-break semantics. Truncated or
  malformed pairs fail closed.
- Verified the selected real FM Towns CD ZIP in both sessions: F31E exposes 41
  visible C02 strings with no high bytes; F31J exposes 46 with 557 high bytes.
  The selected CHTWE/CHTWJ dungeon is retained instead of borrowing English.
- Kept F0644 glyph rasterization closed. The retail CD calls the FM Towns EGB
  system font and contains no glyph ROM, so no game-media-only pixel-parity
  claim or M648 substitution was made.

- Bound FM Towns F31 C017 inventory drawing and pointer hit-testing to the
  selected retail `GRAPHICS.DAT` item 696. Both CDATA and CJDATA provide the
  same thirty C507..C536 children of the C105 16x16 record; boot retains the
  decoded same-session receipt and F31 fails closed rather than borrowing the
  PC/Atari table. Real EN/JA archive tests cover the receipt.

## 2026-09-05 — Side-aware F0172 unreadable inscriptions

- Added a source-owned F0172 wall-aspect receipt which selects C02 by the
  exact right/front/left F0107 view wall rather than by map square alone.
- Routed distant/side M615 through the existing wall-ornament blit and applied
  ReDMCSB G0190/G0204 one-to-three-line `0x4000` clipping semantics. D1C stays
  on the readable M648 transaction and unsupported or mismatched faces fail
  closed.
- Added focused face-selection and all-depth shift-table regressions; no
  synthetic runtime surface or post-render overlay was introduced.

## 2026-09-05 — F0373 levitating front-cell group parity

- Replaced the conservative all-group pickup rejection with the authentic
  ReDMCSB F0175 → F0144/F0264 → F0176 chain. A real C04 now blocks a
  front-square object only when its creature lacks the G0243 levitation bit
  and occupies the clicked cell; levitating groups no longer hide reachable
  floor objects (`parity-evidence/csb_v1_floor_pickup_f0373.md`).
- Party-map F0176 resolves C04 byte 5 as `ActiveGroupIndex` through the
  F0145/F0147 owner and reads effective Cells/Directions from that valid
  active slot; a nonzero-index regression prevents the former raw-byte bug.

## 2026-09-05 — Native C02 inscription decode ownership

- Visible wall TextStrings now decode from the selected CSB dungeon with the
  correct Atari/FM Towns and reversed Amiga bitfields. Invisible records fail
  closed and the C07 scroll offset path shares the corrected platform rule;
  no DM1 `world.things` text fallback is used
  (`parity-evidence/csb_v1_visible_wall_inscription_f0168.md`).

## 2026-09-05 — Platform-owned inscription material plan

- Locked Atari S20/S21/F20E to MEDIA020 M648 graphic 120 and authentic fixed
  G0203 geometry, and Amiga A31/A35 plus English FM Towns F31E to MEDIA720
  M648 graphic 258 and F0635 geometry. All admitted glyphs are authentic 8x8
  C10-transparent source material.
- FM Towns Japanese fails closed because ReDMCSB F0107 owns it through F0644
  and a different selected-media font pipeline. No English M648 or DM1 asset
  substitution is permitted.
- Added live F0172 publication from the native CSB front-wall Thing chain,
  including the retail BUG0_76 last-visible-C02 behavior. The Atari MEDIA020
  Original renderer now consumes that receipt and authentic graphic 120 in
  the candidate-page transaction, with CSB gettext applied only at the final
  presentation boundary and decoded retail English as fallback.
- Wired Amiga A31/A35 and FM Towns F31E to raw selected-container graphic 696,
  F0639 range parsing and strict F0635 C1000..C1003 anchors. Their Original
  front-wall draw now consumes selected M648 graphic 258 without PC/Atari
  rectangle or DM1 asset substitution.

## 2026-09-05 — Native F0349 water-potion mouth transaction

- Command 70 now applies the source-proven C15 water-potion branch atomically
  to the CSB runtime: water gain/cap, in-place C08 empty-flask transformation,
  statistics redraw, and leader-hand retention. Unsupported F0349 branches
  fail closed instead of touching the DM1 world mirror
  (`parity-evidence/csb_v1_mouth_water_potion_f0349.md`).
- The deterministic C09 food branch now uses the eight exact G0242 food
  amounts, caps Food at 2048, detaches and consumes the real Thing, clears the
  CSB leader hand, adjusts leader load, and requests the source swallow sound.
- C09 waterskins now use the exact subtype/icon/ChargeCount branch: +800 water
  capped at 2048, in-place charge decrement, charge-dependent load/icon
  update, retained leader hand, statistics redraw, and C08 swallow request.
- The remaining C08 potion family now implements F0348/F0349 stat, stamina,
  mana, health, wound-RNG, antivenin, and stacked YA timeline behavior. Every
  admitted potion becomes an empty flask with retained Power and corrected
  hand load; C72 expiry subtracts its own `B.Defense` rather than clearing all
  stacked shield defense.

## 2026-09-05 — F0375 left/right leader-hand throw cells

- Propagated F0375's explicit left/right side through F0329 into the native
  CSB projectile record. Left and right viewport halves now produce distinct
  source cells while restoring the action hand and clearing the leader hand
  (`parity-evidence/csb_v1_leader_hand_throw_side_f0375.md`).

## 2026-09-05 — Source-owned viewport floor pickup

- Routed C080 floor-pile clicks through the CSB-owned F0373 dungeon-chain
  mutation. Visible Atari/Amiga/FM Towns objects now move from the authentic
  square record into the leader hand instead of falling through the DM1-only
  world snapshot (`parity-evidence/csb_v1_floor_pickup_f0373.md`).

## 2026-09-05 — Live F0302 inventory transaction input

- Ordinary equipment and backpack clicks now refresh the CSB runtime-owned
  M516 party receipt before reading either possession. This closes the stale
  C017-panel path that could write an old mirrored Thing back over a current
  runtime slot; slot and leader-hand writes remain in the CSB runtime.

## 2026-09-05 — Native Eye/scroll text decoding

- C07 scrolls inspected through Eye now decode their platform-correct C02
  reference and authentic dungeon text pool in the CSB runtime. The final
  panel no longer depends on DM1 `world.things`, and localization uses the
  CSB domain (`parity-evidence/csb_v1_eye_scroll_f0341.md`).

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native direct-CLI and start-menu matrix against the staged
  original media. Atari ST/STX, nested and French preservation ZIP routes;
  Amiga ZIP→ADF routes; and English and Japanese FM Towns routes all reached
  the native campaign handoff. The Atari and Amiga M12→M11 source handoffs
  were also exercised from the selected in-memory media owners.

Reviewed 2026-08-29. Completed work only.

- Atari ST CSB gameplay now remains on the `CHANGE7_01_FIX` VBlank path
  after `ANIM.C` hands off to `FTLCODE`: every 50 Hz gameplay tick delivers
  the source VBlank model, whose palette-start callback installs the already
  verified `GRAPHICS.DAT` C232 light palette. The title's original P4B1
  palette remains title-owned. This is in-process scheduling and palette
  selection only; it neither changes simulation cadence nor creates media.

- `SWITCH.DAT` is now verified from the supplied CSB Utility Disk itself:
  `csb_v1_atari_switch_dat_real_media` opens the original STX, retains its
  7,405-byte `SWITCH.DAT` member only in process memory, validates the
  checksum/header/options/palette, and decodes every enabled source graphic.
  The separate compact fixture remains limited to malformed-input boundaries;
  it is not positive game-data evidence and neither route extracts media.

- M11's CSB query-world handoff now consumes the exact verified
  `CSB_V1_DungeonData::raw_data` bytes retained by the selected boot reader.
  It no longer attempts to reopen an STX/ADF/archive locator as a loose
  `DUNGEON.DAT`. Real Atari STX and Amiga ZIP → ADF launcher regressions prove
  title/start-menu handoff, native mirror/candidate flow, and the first
  runtime frame with no extracted game-data file; FM Towns uses the same
  bounded-memory source contract.

- The supplied CSB Utility Disk's supported single-member LZMA2 7z profile
  is now decoded by Firestaff in bounded memory. Its Atari STX member and
  nested files, including `START.PRG`, can be read without `7z`, `bsdtar`, or
  another external tool at runtime and without materializing game data on
  disk. The reader validates both 7z header CRCs, the extracted member CRC,
  declared sizes, and the member name; unsupported 7z structures remain
  closed rather than falling back to an external extractor. The M12
  regression runs against the supplied archive with external archive tools
  disabled.
- The supplied Amiga ZIP→ADF `Graphics.DAT` now has an in-memory native
  runtime-family receipt: inventory/panel, pit and field, stairs, wall and
  floor ornaments, and doors all decode through the big-endian IMG1 consumer
  without a PC3.4 fallback or an extracted game-data file.
- The active CSB Amiga runtime-graphics CTest is now that ZIP → ADF → OFS
  receipt itself (`csb_v1_amiga_runtime_graphics_real`), rather than a
  separate loose-`GRAPHICS.DAT` test that skipped against the supplied media.
- The supplied CSB Amiga A-disk now has a direct native graphics-format
  receipt. `test_csb_v1_amiga_graphics_dat` reads `Graphics.DAT` from the
  original ZIP → ADF chain in RAM through the AmigaDOS OFS reader, validates
  its Amiga record table and known language/version identity, then decodes
  the source C017 inventory panel at its original 224×136 dimensions. This
  is source-format evidence only; composed-screen capture parity remains
  active work.
- Since this original ADF route is staged, its graphics test no longer
  constructs positive IMG1 or `Graphics.DAT` stand-ins. Positive parsing and
  decode evidence comes only from the admitted ADF member; malformed-header
  checks remain as fail-closed boundaries.
- The supplied Amiga A31 archive now has the same first-runtime input matrix
  evidence as the Atari and FM Towns releases. Each direction, strafe and
  action is launched in a fresh native ZIP → ADF session, with the original
  title owner and campaign state retained in memory; no generated save or
  replacement dungeon is used.
- An authentic FS-UAE/Kickstart 1.3 comparison found and fixed Amiga RGB4
  palette quantization. CSB title, entrance, credits and runtime/HUD surfaces
  now expand original 0x0RGB registers directly to RGB8 instead of passing
  through the VGA six-bit DAC path; the screenshot writer also preserves the
  exact presented table. The real A31M handoff regression checks all title
  entries plus dungeon and C005 credits presentation. See
  `parity-evidence/csb_v1_amiga_rgb4_original_capture_20260905.md`.
- The native Atari STX CLI route verifies original title startup, runtime and
  start-menu entry using the supplied campaign media. Its source-owned input
  regression now also covers backward movement, both turns, both strafes and
  action from independent original STX sessions, each retaining a nonzero
  native viewport receipt. The unchanged initial strafe/action position is a
  recorded source result, not synthetic content.
- The supplied French Atari preservation ZIP now follows its original
  `ZIP → STX` path in RAM.  Its protected sector descriptors retain their
  logical order even when capture offsets are skewed, so the verified shared
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair reaches title, start menu, and first native
  movement without a replacement image or disk extraction.
- The M12/M11 Atari STX route now retains the hash-verified original
  `ANIMATE.SCR`/`ANIMATE.DAT` container when the selected runtime cache holds
  only `GRAPHICS.DAT`/`DUNGEON.DAT`; 50 Hz VBlank cadence, final FTLCODE
  handoff and first native HUD/viewport frame are exercised against that media.
  The completed 224×136 source-owned viewport publishes a nonzero FNV-1a
  receipt without being promoted as a PC F0128 runtime-session receipt.
- On that verified Atari route, native Enter/Accept now crosses the retained
  ANIM.C → FTLCODE handoff instead of being lost in the unrelated PC startup
  dispatcher. A requested CSB PC platform is explicitly rejected before any
  media or cache selection, because no original DOS/PC release exists.
- The Atari M12/M11 handoff regression now fails safely and precisely when a
  selected package cannot be opened, instead of cascading or crashing.
- The FM Towns F31 start-menu receipt now identifies its source-owned
  `TITLE.ANM` palette/frame handoff with a nonzero frame hash. An explicit
  F0435 user-save launch is kept distinct and proves the admitted C03
  executable handoff instead of claiming that it replayed the title.
- The current real-media launch matrix covers Atari STX, Amiga ZIP → ADF, and
  both English and Japanese FM Towns selection. Each route was exercised from
  its original source through title, normal start-menu launch, and the first
  native `UP` movement into the campaign. This is launch/runtime evidence,
  not a claim of complete campaign playthrough parity.
- Direct Amiga ZIP → ADF launches now identify A31M from `TITL.DAT` in the
  exact same virtual ADF as the selected `GRAPHICS.DAT`, rather than requiring
  an M12 cache leaf. The start menu publishes the source-owned `TITL.DAT`
  boundary and hash, while unrelated outer-archive or host files remain closed.
- Amiga A31E and A31M original ZIP → ADF media are read entirely in RAM.  The
  A31E direct C03 handoff verifies `APPB.FTL` and `BJELoad_R` through the same
  selected ADF as `GRAPHICS.DAT`, reaches `csb-entrance-0` with the original
  A31E hash, and does not create an asset-cache copy.  A31M's original
  `TITL.DAT`, `APPB.FTL` language page and `KAOS.FTL` continuation now use
  the same source locator and pass the real CLI and start-menu route into
  runtime after the old extracted cache is absent. The focused real-media
  regression uses virtual source locators rather than the legacy
  materialization API.
- The Atari R1 Hint Oracle now reads its hash-discovered `MINI.DAT` member
  directly into RAM before native GAMEBLOCK decoding.  It no longer writes an
  extracted Utility Disk save into an asset cache; a real STX CLI regression
  covers the direct `--csb-hint-oracle` route.
- CSB Utility Disk import now verifies the original archive member directly
  in bounded RAM.  UTIO.C sector 7 is read through the native STX transport
  reader when required, so the supplied Atari Utility Disk neither needs nor
  creates a transient ADF cache.
- The retired game-media disk-materialization switch now fails configuration
  and the media-admission source rejects direct activation.  Packed CSB
  formats therefore remain source-owned and in-memory only; a format without
  a native reader fails closed rather than creating a cache copy.
- A31M's source-owned Utility Disk DB2 instruction is decoded from the live
  selected dungeon, catalogued without the adjacent encoded champion-stat
  payloads, and translated only at the CSB PO presentation boundary. The
  real ZIP → ADF → M11 test proves the Swedish result and original fallback.
