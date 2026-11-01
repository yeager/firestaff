# Firestaff TODO — active cross-game work

2026-11-01 startup scan localization coverage now includes all five games.
The Swedish real-media scan tests pass for DM1 PC, CSB FM Towns, DM2 FM Towns,
Nexus Saturn, and Theron PC Engine. Nexus and Theron use a source-bound
direct-media path, so their localized startup status and selected authentic
edition are asserted instead of the M12 directory-scan task labels used by
DM1, CSB, and DM2. This verifies diagnostic/menu localization and authentic
edition selection only; it does not establish visual progress-bar parity or
native-window behavior.

2026-11-01 launcher preset clarification: the hierarchical game-card flow's
"MODERN" preset selected V2.1 upscaling, not V2.2 modern artwork. Renamed the
choice to "V2.1 UPSCALED" so the five-game launcher describes the mode it
actually selects. The separate V2.2 mode continues to fall back to V2.1 for
all five games until each has an admitted finished-real art pack; no synthetic
art is used to create a visible difference.

2026-11-01 DM1 PC-34 original-media regression: CTest
`dm1_v1_pc34_native_cli_boot` passed in 542.11 seconds with the installed
retail archive. The regression covers M12 launch, clean host quit during SWSH
and I34E, loose-file GRAPHICS.DAT/DUNGEON.DAT pairing, missing optional SWSH
fallback, the authentic startup/HoC route, keyboard and mouse movement,
portrait/champion interaction, V1/V2 mode receipts and full-redraw telemetry.
This confirms the bounded PC-34 source/runtime interactions in the test; it
does not establish pixel parity with a reference capture or physical Retina
input and display behavior.

2026-11-01 current-build original-media menu retest: CTest
`m12_main_menu_game_selection_real_media` passed in 535.22 seconds with the
installed collection, exercising the no-game picker and game/platform/custom
selection paths for DM1, CSB, DM2, Nexus and Theron's Quest. The test is an
SDL dummy-driver interaction check, not native display or physical-input
evidence; Nexus still does not reach a playable dungeon runtime. The separate
`m12_main_menu_runtime_real_media` test passed in 725.27 seconds for the
source-owned DM1, CSB, DM2 and Theron runtime routes, including DM1 Amiga 3.6
and DM2 Amiga. Nexus is not part of that runtime test because its title/menu
consumer remains capture-gated. The unsupported game/platform CLI rejection
test also passed. These results verify the current startup routes while
leaving the documented Nexus and native-display/input gaps open.

2026-11-01 V2.2 asset-root follow-up: all five authentic-media CLI probes
reported that v22 resolved to v21. The profile stores modern packs in
`~/.firestaff/assets/<game>/modern`, while the default `~/.firestaff/data`
points to an external volume and the scanner supplies canonical media paths.
The shared path resolver now maps those paths back to the logical profile;
the focused symlink test covers all five game IDs and custom-root behavior.
The DM1/CSB art-admission and DM1 review-receipt gates are unchanged. DM2,
Nexus and Theron remain deliberately unavailable for v22 until their
game-specific finished-real materials and consumers are admitted. Rebuild and
rerun of the authentic-media mode matrix after the path fix confirms v22 still
resolves to v21 for all five games. DM1 now reads its manifest from the correct
profile path, but reports `source-material=0` and `review-receipt=0`: its
required slots use `pbr_hero`/`gpt-image-2`, not admitted source-export
material. CSB remains blocked because its source manifest has 26 unbound
projection records, so the material gate is partial. The path bug is fixed;
the user-visible mode difference remains limited because v21 upscales the
original renderer and v22 has no admitted replacement path in this installed
matrix. Close the visual-mode gap only when the source-backed materials and
game render consumers pass their gates.

2026-11-01 all-game presentation-mode audit: ran the authentic-media CLI boot
probe for V1, V2.1 and V2.2 across DM1, CSB, DM2, Theron's Quest and Nexus
using the installed `.firestaff/data` collection. V1 reported a 320x200
presentation target; V2.1 and V2.2 reported 640x400. In `--debug`, every V2.2
request resolved to V2.1 with `fallback=1` (DM1 reports `resolved=2,
admitted=0` through its game-specific real-art gate). Thus V2.1 currently
upscales original graphics, while V2.2 has no visible effect without an
admitted game-specific finished-real art pack. These headless receipts prove
mode selection and target dimensions, not native display or pixel-level
visual distinction. The DM1 modern-art directory exists, but its manifest
identifies generated `pbr_hero` and `gpt-image-2` artwork; it is not
source-derived original game media and is not counted as parity evidence.
Fixed M12 launch readiness to require the same reviewed real-art admission as
runtime: DM1 and CSB use their game-specific gates; DM2, Nexus and Theron's
Quest remain unavailable for V2.2 until their own finished-real packs are
supported. A follow-up found that this policy incorrectly blocked menu
launches even though the CLI and runtime resolved missing V2.2 art to V2.1.
The menu gate now preserves the requested mode, records the resolved V2.1
fallback, and allows the original game data to launch. Existing full-worktree Gitleaks scanning reports 94
fallback, and allows the original game data to launch. Existing full-worktree
Gitleaks scanning reports 94
`generic-api-key` findings in already-present files; the staged change scan
reports no leaks. See `m12_all_games_boot_readiness_contract_fixture`.

2026-11-01 CSB mixed-platform menu retest at `84aa6410a`: the authentic-media
`csb_v1_fmtowns_m12_auto_mixed_real_media` test passed in 499.74 seconds. With
original FM Towns, Atari ST game/utility disks, and Amiga media present
together, M12 AUTO selected FM Towns and reached its original MINI.DAT party;
manual menu selection reached the authentic Atari C200 runtime and the
Amiga-specific route; direct CLI platform requests were also checked. The
test verifies that original archives remain unchanged. This covers those
routes only, not every CSB edition or native input/display parity.

2026-11-01 current-main five-game launch retest at `bca19ec55`: the
authentic-media `m12_main_menu_game_selection_real_media` test passed in
519.94 seconds and `firestaff_cli_startup_diagnostics_real_media` passed in
171.33 seconds. The menu route exercised DM1, CSB, DM2, Nexus and Theron
through their authenticated menu handoffs; the CLI route checked game/media
selection and startup diagnostics across the installed originals. Nexus is
still asserted to stop at `title-vdp-owner-compositor-capture-required`, so
this does not prove a playable Nexus start. These SDL dummy checks verify the
named launch paths only, not native display/input or complete gameplay parity.

2026-10-31 Theron native real-media routes: the macOS Debug build passed the
authentic US `theron_v1_m11_real_bram_continue` and JP
`theron_v1_m11_jp_fresh_start` CTest cases. JP fresh start reaches source-
verified Ak-Tu-Ba level 0; US matching authentic Track 02 and Ak-Tu-Ba-complete
BRAM reach dungeon 2 through Continue and exercise bounded movement and item
transactions. Source-only loaders cover all seven dungeons in both regions.
These are bounded native routes, not complete parity. Original-frame pairing,
dynamic source-bound viewport, cold game-owned handoff/input, later campaign
and save semantics, combat, quest-item consumers, and audio remain open. Do
not infer stair or quest behavior from roster indices or fixtures. See the
[Theron capture-readiness record](docs/theron/THERON_CAPTURE_READINESS.md).
The separate saved-state PCE Fast trace contains 1,354 short-format
`$44CA/$44DB` rows, but standard-PCE and PCE Fast instrumentation share the
output path and label, so the input receipt is ambiguous and changed snapshots
remain unattributed. See its [capture report](docs/source-lock/theron-disassembly/theron-us-state-scripted-input-boundary-20261031.md).

2026-10-31 authentic-media launch audit: on base `d64d70d0`, the real-media
CLI matrix passed DM1 PC-34, CSB Atari ST/Amiga/FM Towns, and DM2
DOS/Amiga/FM Towns/extracted Macintosh; M12 platform selection and early-scan
input checks passed for DM1/CSB/DM2. After main advanced to `9cb30b20`, the
app was rebuilt and the following were rerun: direct `--game dm1`, `--game
csb`, and `--game dm2` with no `--platform` selected FM Towns from the
installed mixed original-media root; invalid-platform CLI handling passed;
and no-game M12 routes for DM1 Atari ST German 1.2 and CSB Atari ST French
reached first runtime (2/2). No launch failure was reproduced in those
checked routes. Full platform/gameplay parity remains open below.

2026-10-31 DM2 media identity follow-up: the installed DOS English ZIP has a
different wrapper hash from the previously locked ZIP, while its
`DATA/GRAPHICS.DAT` and `DATA/DUNGEON.DAT` match the authentic PC English
catalog. The FM Towns ZIP is a CloneCD wrapper; its `track01.iso` contains the
catalog-authenticated FM Towns `GRAPHICS.DAT`, `DUNGEON.DAT`, and `TWANIM.EXP`.
The Macintosh source present here is the English StuffIt archive, with its
extracted `Graphics.dat` matching the authentic Mac catalog. Updated
`dm2_v1_real_media_hash_lock` to accept these exact reviewed local container
identities while continuing to reject unknown hashes; the test passed against
the installed originals. DOS French and Amiga container identities also
passed. All archives were read in place and remain unchanged.

2026-10-30 authentic-media no-game menu retest at `20fe91e3`: the full
`test_m12_main_menu_game_selection_real_media.sh` run passed against the
installed `.firestaff/data` collection. It selected and handed off all five
games through the keyboard menu, including default-root launches for DM1,
CSB, DM2, Nexus Saturn, and Theron PC Engine; Nexus and Theron also passed
mouse selection. The Nexus collection contains multiple authentic regional
owners, so the test now validates that the selected edition is authenticated
and its source remains inside the real Nexus data root instead of assuming
Japanese `DM.BIN` always wins. These SDL dummy receipts prove menu selection
and original-media handoff only. Nexus still has no admitted playable level,
and native presentation/input parity is not established.

2026-10-30 DM1 Amiga 3.6 menu-runtime follow-up at `a59a4f03`: the new
focused CTest first selected the real Amiga 2.0 edition from the complete
collection, although the test expected Amiga 3.6. The menu did reach
`dm1-runtime`; the failure was the test's source/edition assumption. The
case now links only the authentic Amiga 3.6 ZIP into an isolated data root.
It passes through the no-game M12 menu, verifies the `amiga36-multi`
handoff and first runtime receipt, and leaves the original archive
unchanged. Full-collection automatic selection continues to prefer the
authenticated Amiga 2.0 edition in this installation; this result does not
claim an interactive 3.6 edition-selection route from that mixed collection.

2026-10-30 current-main scanner/rescan regression retest at `9f4bb4df`: after
rebuilding the shared scanner, the original-media M12 menu routes passed for
DM1, CSB, and DM2. Alternate routes passed for DM1 PC and Atari ST, CSB Atari
ST, and DM2 PC and Amiga; DM1 mixed-platform AUTO and the CSB Atari handoff and
mixed-media FM Towns matrix also passed. Direct DM2 CLI startup passed for DOS
English/French, extracted Macintosh English, Amiga English, and FM Towns
Japanese. Selected-file return-to-menu passed for DM1 PC34, CSB FM Towns, and
DM2 DOS; DM2 Amiga return-to-menu passed separately. Each return scan reported
current availability for all five games. Real-media keyboard and mouse window
close cancelled DM1's return scan without marking it complete. Removing and
adding original CSB media during DM1 play correctly changed availability on
return. The CSB FM Towns cold/warm cache check and DM2 DOS scoped/unscoped
directory cancellation check also passed; DM2 cancellation rolled back
partial scan publication and preserved the original archive. No start-menu,
CLI, rescan, cancellation, or cache defect was reproduced in these checks.
They use SDL's dummy video driver and do not establish native presentation,
physical input, complete animation playback, or playable-game parity. The
evidence is summarized in
[`docs/STARTUP_SUPPORT_GAP_ANALYSIS.md`](docs/STARTUP_SUPPORT_GAP_ANALYSIS.md).

2026-10-29 current-main startup retest at `737508d0` after PR #200: the
authentic-media no-game M12 first-runtime cases passed separately for DM1,
CSB, and DM2 using the installed FM Towns editions. The four alternate
platform menu-to-runtime cases also passed: DM1 PC, DM1 Atari ST, CSB Atari
ST, and DM2 PC-DOS. CSB Atari ST found the original Utility disk archive in
the per-game data directory and completed its startup animation before
reaching a nonzero viewport receipt and movement step. The CSB Atari
animation asset, real-media, and discovery tests passed 3/3. The original DM1
Apple IIGS 2.0/2.1 CLI and M12 real-media test reached active dungeon runtime
for both editions. The checks used authentic local media and verified source
archives remained unchanged where the harness provides hashes. These results
cover the named source paths only; they do not establish every edition,
native display/input parity, or full campaign playability. No new startup
failure was reproduced in these routes, and no release was created.

2026-10-29 CSB launcher edition-row follow-up, based on `584028e1`: the new
`m12_csb_amiga_version_menu_real_media` CTest passed against the installed
original CSB Amiga 3.5 English archive. It navigates the public M12 menu input
path from the CSB game card through the Amiga platform and Game Options, cycles
to the matched `amiga35-en` edition, and confirms the launch intent retains
that exact edition and architecture. This closes a regression-test gap for
same-platform multi-edition selection; it does not claim a full runtime boot
or Amiga gameplay parity. The test skips when authentic A35E data is absent.

2026-10-29 launcher keyboard-routing follow-up: the application event loop
previously translated horizontal arrows only in Settings. The real Game
Options view also needs them to change platform, edition, and row values.
Firestaff now routes Left/Right to Game Options while preserving active-game
controls and text editing. `m12_csb_amiga_version_menu_real_media` exercises
the shared live key mapper while selecting authentic CSB Amiga 3.5 English;
the full-app real-media picker matrix additionally selects authentic A31
multilingual CSB through an SDL Right key and verifies the edition handoff.
The targeted source test and a full-app SDL dummy session both passed. The
2026-10-29 platform-only authentic-media matrix completed successfully with
eight DM1/CSB/DM2 edition handoffs. These checks prove the menu input path and
handoff only, not native-device input or complete CSB gameplay parity.

2026-10-29 launcher data-root follow-up at `8a04294f`: the authentic-media
return-scan mutation test passed both directions. Removing the staged original
CSB archive while DM1 was running made CSB unavailable after returning to the
launcher while DM1 and DM2 remained available; adding the original CSB archive
during the same DM1 runtime made all three priority games available on return.
The DM2 DOS selected-archive test also passed after its staged archive was
removed during gameplay: DM2 returned to the launcher and the scan reported
the availability of the remaining original game media. A direct default-start
check with authentic data under the standard data root reported
`startup game=none mode=menu`, completed the scan for all five games, and
performed no game handoff.
The shared-collection M12 session also passed: it launched DM1 from an explicit
per-game data directory, rescanned the collection, launched CSB, returned and
rescanned again, while retaining authentic DM1 and CSB edition identities and
discovering DM2 from the same collection. The tests used the existing
2026-10-29 05:49 local app build, which includes the current runtime changes;
subsequent `main` commits are test or documentation changes. These SDL-dummy
receipts cover dynamic discovery and the named shared-root handoffs only. They
do not establish native display/input or complete gameplay parity. No launcher
behavior defect was reproduced in these cases.

2026-10-29 current-main DM1/CSB/DM2 launcher audit at `5b65ec09`: the
authentic-media no-game M12 menu tests passed separately for DM1 FM Towns,
CSB FM Towns, and DM2 FM Towns, including each game's first-runtime receipt
and Back/reselect route (3/3). The mixed game/platform picker test passed its
seven available alternate-edition routes: DM1 PC, Atari ST, and Amiga; CSB
Amiga and Atari ST; DM2 PC and Amiga. The unsupported-platform fail-fast
matrix and CLI option-form test also passed (2/2). The tested app binary was
built before `5b65ec09`, whose only change is the DM1 Atari STX test source;
the runtime code therefore matches this checkout. The three authentic-media
DM1 PC, CSB FM Towns, and DM2 FM Towns Swedish scan/debug localization checks
also passed (3/3). A separate data-root containing only DM1, CSB, and DM2
original archives passed 19 no-game menu handoffs across keyboard and mouse
selection, seven alternate platform editions, per-game collection roots, and
the standard `.firestaff/data` root. Mouse-driven Return to Menu also passed
for DM1 PC, CSB FM Towns, and DM2 FM Towns; each rescan reported all five
installed games available (3/3). These SDL dummy runs verify only the named
launcher selections and runtime handoffs, not native display, audio, physical
input, untested editions, or complete game parity. No source behavior change
was needed and no release was created.

The current five-game startup evidence matrix and remaining Nexus/Theron
runtime-entry gaps are consolidated in
[`docs/STARTUP_SUPPORT_GAP_ANALYSIS.md`](docs/STARTUP_SUPPORT_GAP_ANALYSIS.md).

2026-10-29 current-main startup regression check at `45ab34c5`: the
real-media M12 game-selection matrix passed (419.13 seconds) for keyboard and
mouse selection across the five game cards, per-game collection roots, and
the default `.firestaff/data` root. The real-media CLI startup-diagnostics
matrix also passed (133.59 seconds) for all five games, including FM Towns
defaults and invalid/unsupported platform handling. The existing v3.0.419
binary was built from the same runtime code; `45ab34c5` only changes a DM2
Macintosh test-media fallback. These checks prove the covered media-selection
and startup routes, not complete playable support, native display/input
parity, or every edition/platform combination. DM1 PC34's focused original-
media first-monster combat test passed in 1.83 seconds; it proves only the
tested encounter and PUNCH damage path. No release was created.

2026-10-29 main-menu-to-runtime follow-up at `f8b62ea1`: the four in-scope
real-media cases in `m12_main_menu_runtime_real_media` completed, and their
probe verifiers passed for DM1 FM Towns, CSB FM Towns, DM2 FM Towns, and DM2
Amiga, each with `levelLoaded=1`. The harness's own per-case source-hash checks
also completed for those cases. The combined process was stopped before its
out-of-scope Theron case; these four results are the only ones claimed here.
This confirms first-runtime entry for the named authentic routes, not full
campaign or platform parity.

2026-10-28 authentic-media status correction: the DM1 Amiga demo ZIP, the CSB
Amiga `Chaos Strikes Back (FTL).zip`, and the DM2 Macintosh English StuffIt
archive plus its extracted original files are present under the configured
`.firestaff/data` tree. The DM2 extracted-files regression added on `main`
records successful direct CLI and M12 startup to first runtime without
changing the original files. The `.sit` container itself is not unpacked by
the ZIP-only CTest. The older missing-media results below describe the Oct 27
sweep and should not be treated as current DM2 Macintosh evidence. For DM1 and
CSB, local archive integrity is verified; their specific startup/decoder
tests were not rerun in this check. A fresh 2026-10-28 run of
`dm2_v1_mac_extracted_native_cli_boot` passed both direct CLI and M12 launcher
routes against the original extracted Macintosh English files, including the
movie, new-game, champion selection, movement and first loaded runtime frame.
The test hashes the original Graphics, Dungeon and Title files before and
after. The same current-source check also passed the authentic DM1 Amiga demo
archive CLI/M12 test through the SWSH, title, entrance and runtime phases, and
the CSB Amiga ZIP-to-ADF CLI/M12 test through its source entrance and movement
receipt. Those tests hash or otherwise preserve their original archives. SDL
dummy verifies the exercised startup and runtime handoffs, not native
display/audio/input parity. See the per-game TODOs for remaining gaps.

2026-10-27 TRV2 verification at source revision `f582f57d` against the
authentic `.firestaff/data` roots: a clean Debug build produced `firestaff`
and both focused Theron disassembly/capture test targets. The disassembly
test passed; the real VRAM-capture CTest skipped without its required capture
environment. Supplying the retained cold-start capture explicitly still did
not bind it to the production viewport. The real-media M12 selection test
passed keyboard and mouse handoff for all five games, including the default
`.firestaff/data` and sibling-folder discovery cases. Its first-runtime test
passed for DM1, CSB, DM2 FM Towns, and Theron PC Engine. The 14-route CLI
boot sweep reached `boot-probe-ready` for 12 routes; DM2 Macintosh was
`missing-media` (no authentic Mac image in the configured DM2 data folder),
and Nexus Saturn remained blocked before its menu and playable level
(`menu=0`, `levelLoaded=0`). These headless receipts establish route/media
selection and bounded runtime entry only; they do not establish complete
gameplay or native presentation parity. No synthetic media was used and the
original archives were hash-checked by the menu tests.

2026-10-26 local M12 source audit found several paths using a fixed game
option slot as a menu-row index: CLI `--game` preselection, Quick Resume,
save-browser load/import, Quick Resume card art, selected-launch preparation,
direct CLI launch, and pointer cycling of presentation mode. The current
static row order masks the mismatch. The working-tree fix resolves menu rows
by game ID and keeps option access on the fixed game slot. Its reordered-row
Quick Resume regression now scans the configured authentic DM1 media root
and does not mark a game available by hand; the UI handoff state contains no
fabricated archive, hash, or save. The existing all-games M11 direct-launch
test also reorders each available game's row during its authentic-data path,
so it can verify both direct and menu launch against real media. Syntax checks
for the menu, pointer handler, M11 launch path and both tests pass with
existing Firestaff compile flags. Rebuilt `firestaff` and the three focused
test targets on the workstation with its five authentic `.firestaff/data`
directories; `m11_direct_launch_prepare_all_games`,
`m12_launcher_options_runtime_handoff`, and `m12_data_dir_cancel` all pass.
The last test covers the all-games rescan path that previously let an
unsupported virtual FM Towns image shadow a bootable DM2 PC archive. The
working diff also passes `git diff --check`.

2026-10-25 current-main TRV2 build and original-media startup retest:
`fast-dev` built commit `e3b6d015` successfully (2,342 Ninja steps, one job).
Direct `--game` boot probes using the per-game `.firestaff/data` folders
reached `runtime-entered` for DM1/CSB/DM2 (FM Towns) and Theron (PC Engine).
Nexus reached its Saturn title but correctly remained blocked with
`menu=0 levelLoaded=0`; this is not a playable start. The real-media M12 menu
selection script passed keyboard and mouse selection/handoff for all five
cards, and default-root menu launches for DM1/CSB/DM2. Handoff receipts do
not prove a playable Nexus or Theron runtime. The broader CLI edition matrix
returned its documented skip code 77 because this TRV2 data root lacks the
required authentic DM2 Macintosh archive; no substitute or synthetic media
was used. All probes used SDL dummy video/audio and do not establish native
display/input parity or complete gameplay.

2026-10-26 fresh TRV2 retest against the authentic `.firestaff/data` roots:
the no-game M12 menu to first-runtime test passed for DM1, CSB, DM2 (FM
Towns), and Theron (PC Engine), with the verifier requiring `levelLoaded=1`.
This confirms those four bounded menu routes on the rebuilt `ed2ff82a` main
binary. A separate zero-frame CLI snapshot against the same real data showed
why an immediate boot-probe is not a runtime matrix: DM1 already had
`levelLoaded=1`, while CSB, DM2, and Theron were still in their respective
startup states. Nexus remained `title=1`, `menu=0`, `levelLoaded=0`, blocked
by `menu-bpk-prs3-capture-required`; no playable Nexus start was proved. The
four-game runtime script has no Nexus case, and these SDL dummy runs do not
establish native presentation or full gameplay.

2026-10-27 TRV2 v3.0.415 recheck against authentic Japanese Nexus media:
direct `--game nexus --platform saturn` selected `DM.BIN` (MD5
`e88d60859f65f08fa622e1992b02280f`). After 110 boot-probe frames and one
scripted Enter, the receipt reported `titleReady=1`, `levelLoaded=0`,
`menu=0`, and `menu-bpk-prs3-capture-required`. The authentic-data
`test_nexus_v1_native_menu_mouse_title.sh` passed through the M12 mouse flow
to the same capture-gated title. This confirms Nexus title handoff only, not
a playable level or menu. Both runs used SDL dummy video/audio and preserved
the retail data.

2026-10-26 Theron keyboard gap follow-up: PR #74 was merged on 2026-10-25.
The earlier review blockers are resolved in the current source: the Theron
SDL mapper routes Z to Action and X to Back, and the loaded-dungeon runtime
consumes a standalone Back token without advancing the source clock. The
authentic USA Track 02 SDL event test sends Tab, Z, Space and X through the
normal event loop; `tests/test_theron_native_keyboard_mapping.c` also checks
the keyboard mappings. This closes those specific routing gaps only; it does
not establish native dungeon-image parity or complete Theron gameplay.

2026-10-25 DM1 Swedish startup diagnostics: an authentic PC 3.4
`--game dm1 --platform pc --lang sv --debug --boot-probe` previously printed
the scan phase only as its English internal key. Debug scan lines now retain
the stable `task` and `game` keys and add language-specific `task-label` and
`game-label` fields. The DM1-only original-media CTest passed with Swedish
catalogs loaded from `po/`; it also confirmed the authenticated `pc34-en`
selection and boot-probe receipt. This verifies localized debug diagnostics,
not native-window localization or visual parity.

2026-10-26 current-source cold launcher-scan profile: on the Mac16,10 host,
`--scan-data --debug` over the installed 425-file, 4.3 GiB original-media
collection completed in 31.13 s from an isolated home/config. A repeat with
the same scan cache completed in 11.91 s and reported all five games READY.
`/usr/bin/time -lp` recorded about 438 MiB maximum resident memory for the
cold run and 736 MiB for the warm run; peak footprint was about 810 MiB warm.
A six-second macOS `sample` placed substantial work in
`m12_admit_csb_amiga31_english_program_package` → `asset_find_by_md5`, including
repeated ZIP inflation and nested Amiga ADF traversal, followed by more
`m12_fill_game_versions` archive matching. This identifies a candidate
duplicate-admission path, not proof that every lookup is redundant: preserve
the same-owner and incomplete-first-copy checks when optimizing it. One host
and one cold/warm pair do not establish a cross-platform regression or speedup.
The follow-up now reuses an already hash-verified CSB Amiga graphics match
only after the same-container program package passes; A31E also must pass its
startup-media gate. Incomplete candidates retain the exhaustive fallback.
The focused real-media CTests passed for the original English A35 CTRaw package
(5.85 s) and the A31 M12-to-M11 handoff ZIP (2.15 s). These verify authentic
package admission and startup. A fresh current-source build scanned the same
425-file, 4.3 GiB collection on the same Mac16,10 in 27.86 s with an isolated
user cache; three warm repeats took 8.70, 8.57 and 8.54 s (median 8.57 s).
The prior build measured 31.13 s cold and 11.91 s warm, so the observed
reductions are 10.5% and 28.0%. DM1, CSB and DM2 remained READY in every run.
This is preliminary single-host evidence: the cold OS file-cache state was not
controlled, the old timing had one sample, and both builds used the same scan
options that skip external-only archives. Repeat with controlled cold caches
and another platform before treating the gain as general.

2026-10-25 fresh-cache startup scan profile on the same Mac16,10: the
v3.0.412 scanner completed `--scan-data --debug` against the installed
original-media root in 26.41 seconds (18.67 seconds user CPU, 3.28 seconds
system CPU). `/usr/bin/time -lp` reported 637,403,136 bytes maximum resident
set and 839,795,864 bytes peak footprint. All five games were reported READY;
external-only archive formats remained explicitly skipped. A five-second
macOS `sample` attributed the main-thread scan stack to
`m12_fill_game_versions` → `asset_find_all_by_md5_list` →
`scan_dir_by_md5_list`; the nested container path included native 7z ADF
visits and CAPS/MFM track decoding. This confirms a first-scan CPU and memory
cost remains for the full mixed-media root, even though normal launcher
scanning runs on a worker thread. The measurement used an isolated user cache,
but did not control the OS file cache and is not a cross-platform benchmark.
Investigate sharing authenticated candidate indexing across the per-game
version passes without weakening edition hashes, same-container ownership,
or cancellation; do not infer that repeated low-level samples alone prove a
candidate can be skipped.

2026-10-26 current-source isolated-cache scan: the Debug `firestaff` build
from `c61971089` completed `--scan-data --debug` against the installed
original-media collection in 26.84 seconds (18.44 seconds user CPU and 3.32
seconds system CPU). All five games were reported READY. The scan also
reported authentic `.7z` and `.rar` files as skipped because external archive
tools were not enabled; their members were therefore not part of the READY
result. The run used an isolated HOME/config but did not control the OS file
cache, so it is one current-source observation rather than a cold-scan
benchmark. No game data was extracted or copied.

2026-10-26 published v3.0.414 launcher scan against the same installed
original-media root: a fresh isolated HOME/config took 30.08 s; two warm
repeats took 9.92 s and 9.84 s (9.88 s median). The scanner reported all five
games READY, while explicitly skipping 14 external `.7z`/`.rar` candidates
because external archive tools were not enabled. The OS file cache was not
controlled, so this is not a cold-disk measurement or a complete inventory of
the collection. It confirms that full-root discovery still costs about 30 s
from an isolated application cache; it does not measure time to first menu
interaction. Profile this
packaged revision before retaining another scanner optimization.

2026-10-26 TRV2 current-main follow-up: a one-job `fast-dev` build of
`6f1f79ff5` completed, then `--scan-data` against TRV2's authentic
`.firestaff/data` root scanned the five-game collection in 14.77 s using the
existing user scan cache. The report marked DM1, CSB, DM2, Nexus and Theron
READY, while explicitly skipping several external-only archive candidates;
this is not complete archive inventory or a cold-cache measurement. Two
alternating three-run comparisons against binaries built from the same source
revision produced identical full scan output. Removing the generic named-file
pre-hash pass measured 9.295 s baseline and 9.257 s experimental median.
Reusing a successful physical-file hash instead of repeating the final
`file_md5` call measured 9.195 s baseline and 9.212 s experimental median.
These differences are within run variation, so neither source change was
retained. The recursive scan reaches the configured shallow candidate
directories; keep the known same-owner,
duplicate-edition, container and access-permission checks intact. Continue
with representative profiling before changing the broader edition index or
archive-admission work.

2026-10-27 current Mac workstation scan profile: the authentic media root at
`/Volumes/Extern-disk/FirestaffUserData/data` contained 408 files (3.2 GiB).
The current Debug executable scanned it in 17.32 s from a fresh isolated app
cache and 3.91 s on a warm repeat; `/usr/bin/time -lp` reported about 1.03 GiB
maximum resident memory for the first run and 568 MiB for the warm run. All
five games were marked READY, but external-only archives were explicitly
skipped, so this was not a complete archive inventory. The OS file cache was
not controlled. A five-second `sample` showed repeated native 7z solid-stream
decoding and Amiga ADF/CAPS/MFM traversal alongside recursive MD5 lookup;
it did not attribute the observed process peak to one allocation or one
archive. The current code repeats per-game version-hash passes over the same
roots, but the profile alone does not prove which walks can be safely shared.
Follow-up `vmmap` samples during a fresh-cache run recorded an 861 MiB
physical footprint and 1.0 GiB peak; about 850 MiB was reported in macOS's
`Malloc Large (empty)` regions. The resulting persistent cache contained only
1,960 rows (486 KiB), so that cache is not the source of the peak. This
indicates substantial freed large allocations retained in the process
allocator, but the region dump did not identify their allocation call sites.
The native 7z SDK allocator itself is capped at 96 MiB per archive visit; that
limit does not account for allocations made by other archive and disk-image
readers. Do not reduce supported media limits based on the peak alone.
Do not weaken full edition enumeration, same-container ownership, duplicate
hash handling, media-access validation, or archive completeness markers based
on this sample. Next profile allocation peaks and archive identity while
preserving the full all-game report, then compare any shared candidate index
against the byte-for-byte scan output on the same original-media root.

2026-10-27 CSB FM Towns intake memory follow-up: full macOS malloc stack
logging traced a 507,062,976-byte heap allocation to
`m12_admit_csb_fmtowns_archive` → `m12_csb_fmtowns_archive_read_image` →
`firestaff_zip_extract_by_name_with_progress`. That is the selected authentic
raw CD image being duplicated into a heap buffer. The intake now uses the
existing mapped-output ZIP reader and releases the mapping with its matching
API; the RAR/virtual-path fallback still uses ordinary owned memory. The
focused authentic `m12_csb_fmtowns_scan_cache_real` test passed for cold
admission of both language editions and warm receipt reuse (14.92 s). A fresh
isolated-cache scan of the same five-game original-media root completed in
12.60 s, marked all five games READY, and reported 908,148,736 bytes maximum
resident memory / 901,137,560 bytes peak footprint, versus 1,105,756,160 /
1,098,646,800 bytes in the earlier run. This is roughly an 18% lower recorded
memory peak, but OS file-cache state was uncontrolled and memory remains high;
the timing comparison is not a controlled speed benchmark. A post-change
high-water-mark trace confirms the same 507,062,976-byte image is still
resident as an anonymous mapping while the CD layout is checked; the change
avoids retaining that block in malloc's large-allocation cache between scans,
but does not make CD validation streaming or remove its roughly 0.5 GiB
working set. External-only
archive formats were skipped in both scans. Continue tracing the remaining
high-water allocations and verify all CSB FM Towns fallback formats before
considering this intake path fully closed.

Current-source M12 single-game menu retest at `c54007b45`: the separate
`test_m12_main_menu_{dm1,csb,dm2}_only_real_media.sh` checks passed with one
authentic FM Towns archive in each isolated `~/.firestaff/data/<game>` tree.
DM1, CSB and DM2 reached their first FM Towns runtime from the no-game menu
and passed Back/reselect with the same authentic runtime receipt. DM1's test
now validates the HoC first frame after both initial selection and Back/reselect
instead of exiting at the launch handoff. These are SDL dummy-driver
route/runtime checks, not native-window input, display, audio, or complete
platform parity.

2026-10-25 authentic priority-game startup and combat retest: the launcher
picker script passed with an isolated root containing only symlinks to the
installed DM1, CSB and DM2 originals. It passed keyboard and mouse card
selection, per-game-folder sibling discovery, default `~/.firestaff/data`
discovery and source-archive hash checks for all three games. Each menu route
launched its authentic FM Towns edition. A separate authentic DM1 PC 3.4 HoC
combat probe recorded a damaging player attack, 16 creature moves, six
creature attacks and champion HP loss. With party ShieldDefense set to 100,
the same sequence reduced observed group damage from 28 to 17 HP. The
authentic DM1 Amiga 2.0 first-monster combat test also passed. These checks
cover scripted SDL input and game-state receipts; they do not verify native
window input, real Retina scaling or complete playthroughs. A separate
authentic PC34 HoC test recruited two champions from original C127 mirrors
and switched between them through the C012/C013 name boxes; its original-data
route passed. Native physical mouse delivery and Retina-window click mapping
remain unverified.

2026-10-25 native DM1 PC34 startup timing: the authentic DOS archive ran in a
native Cocoa/Metal window on the Mac16,10 M4 host, whose display reported 60
Hz. The 32-frame selector door animation took 505 ms; the selector handoff
completed after 764 ms including a 259 ms post-animation audio wait. Presented
frame intervals were 14–17 ms with VSync active. The full authentic PC34
startup/CLI regression passed separately in 458.42 seconds and checks the
source's modeled 70 Hz door cadence. This is evidence for the M4 60 Hz host
only; it does not establish timing or HiDPI behavior on the reported MacBook
Pro M5. ReDMCSB ENTRANCE.C F0441 opens the door after the new-game command, so
it should not wait for a second confirmation after that command.

2026-10-25 authentic launcher return/rescan retest: the
`return_to_menu_rescans_dm1_csb_dm2_real_media` CTest passed in 1,527.71
seconds using installed original media. It exercised DM1 PC, FM Towns, Atari
ST and Amiga; CSB FM Towns, Atari ST and Amiga; and DM2 PC, Macintosh and
Amiga, including direct-exit and keyboard/mouse return routes. The focused
DM2 selected-archive return test also passed independently in 55.87 seconds.
These SDL dummy video/audio checks cover the scripted launcher/runtime handoff
and rescanning paths only; they do not establish native presentation, physical
input, audio, or complete gameplay parity.

2026-10-25 isolated priority-game menu retest:
`test_m12_main_menu_game_selection_real_media.sh` passed with a data root that
contained only symlinked original DM1, CSB and DM2 directories. Keyboard and
mouse selection launched each game's authenticated FM Towns edition; default
`~/.firestaff/data` discovery and sibling-game discovery from a per-game root
also passed. Nexus and Theron were absent from this isolated test and were not
started. These scripted SDL dummy checks do not establish native display,
physical input, audio, or complete gameplay parity.

2026-10-24 Ymir Japanese date/time setup VDP2 investigation: captures at frames
105 and 118 show NBG0 date/time text and cursor pixels plus NBG1 UI box shapes.
Both are 512×256 8bpp bitmap layers. Filtered traces have
zero drops, and byte-wise replay reconstructs the captured 128 KiB NBG1 span.
The retail-source analyzer now checks both layers, but exact bitmap and tested
palette joins remain absent; `source_join=unbound` and
`semantic_admission=blocked`. Current evidence does not tie this screen to
Nexus code/assets or prove the retail game booted, so it is not game startup
parity. A separate English-media trace does verify a runtime write of a
`TM.BIN` chunk, but the corresponding Japanese gameplay transition remains
unbound. Next priorities are capturing a matched Japanese source-PC/disc-read
trace, then joining the menu/HUD writes to retail assets. Do not enable
rendering from coincidence.

2026-10-24 Japanese title START follow-up: an authentic JP BIOS/CUE run
captured the Nexus title screen at frame 42869 and still showed it 299 frames
after a one-frame START pulse. A separate matched 70-frame input/control trace
at frames 42860--42929 has byte-identical VDP1/VDP2 output; its sampled SH-2
branch stays not-taken while `R2` briefly changes to `0x10`. The branch PC does
not map to the observed opcode under the current hash-verified `DM.BIN`
address mapping. This does not bind a menu action or title-to-menu transition;
keep startup and menu renderer admission closed. Detailed hashes and paths are
in `docs/nexus/NEXUS_RUNTIME_CAPTURE.md`.

2026-10-24 five-game direct CLI retest: the authentic-media
`firestaff_cli_startup_diagnostics_real_media` CTest passed in 246 seconds,
covering DM1, CSB, DM2, Nexus and Theron. It verifies selected source media,
boot phases, debug diagnostics and launcher/configuration behavior. One earlier
run failed the DM2 FM Towns TWANIM timing/frame assertion; an isolated replay
recorded all 224 intermediate frames and source completion at 26.2 seconds,
and this complete rerun passed. The earlier failure did not reproduce; the
test uses SDL dummy video/audio and is not native display or audio evidence.

2026-10-24 five-game main-menu selection retest: the authentic-media
`m12_main_menu_game_selection_real_media` CTest passed in 248 seconds. It
verified keyboard and mouse selection for all five titles, the three priority
games from default `~/.firestaff/data`, and sibling discovery when the selected
data folder is one game's directory. Test cases retain isolated config files
while sharing the identity-bound scan cache for unchanged source paths; this
keeps the complete menu matrix inside its CI timeout without weakening media
authentication.

2026-10-24 DM2 FM Towns warm scan reuse: after a full directory-discovery
boot-profile scan proves the exact HME-242 `DATA/GRAPHICS.DAT` and
`DATA/DUNGEON.DAT` pair, M12 can reuse those member MD5 receipts while the
source ZIP file identity and timestamps remain unchanged. Explicitly selected
archives still run the full boot scanner. The authentic `dm2_v1_fmtowns_native_cli_boot`
test passed with both cold and warm `--scan-data` runs and the full New Game
route (390 seconds on this machine). This avoids rescanning that same Towns
disc image on repeat directory scans; other DM2 archives are still checked.

2026-10-24 startup/CLI audit: expanded the DM1/CSB/DM2 unsupported-platform
test to cover every currently unsupported platform pairing in both direct and
menu-request modes. The 24-case live CLI matrix rejected each pair before
startup or scan diagnostics, and `dm2_unsupported_platform_cli` passed against
the configured build. The regression now runs those routes with `--debug` and
fails if scan or startup diagnostics appear before the rejection. With all five
original-media directories installed,
`--scan-data --debug` found all five games READY in 26.9 seconds on a fresh
isolated cache and 8.9 seconds on the warm repeat. A five-second warm-run
sample placed most active samples in DM2 ZIP profile admission and zlib image
inflation.

2026-10-25 incremental startup scan: the deferred launcher scan now publishes
completed DM1, CSB and DM2 receipts separately, so a ready game card can
launch while later media is still being checked;
Nexus and Theron remain gated until the shared metadata refresh completes.
The menu-state gate test passes, but the authentic-media startup test still
needs a host with the full DM1/CSB/DM2 collection. This scan-only work does not
prove native menu input or gameplay parity.

2026-10-26 original-media launcher retest: `m12_main_menu_game_selection_real_media`
passed for all five installed games through keyboard and mouse selection. It
also passed default `~/.firestaff/data` discovery and per-game-folder roots for
DM1, CSB and DM2. `m12_main_menu_runtime_real_media` reached the first authentic
runtime from the menu for DM1, CSB, DM2 and Theron. The CLI startup diagnostics
found all five games in the default data tree and resolved each bare `--game`
request to its original media. Nexus still stops at its title/startup gate and
does not reach an admitted playable level or menu, so its actual runtime start
remains unverified. The direct `--game` return-to-menu matrix also passed for
DM1, CSB and DM2: each session exited without reopening the launcher or
rescanning. The Swedish DM1 scan-label test, CLI option-form test, and
unsupported-platform direct/menu rejection matrix also passed. These scripted
runs used original media and SDL dummy video/audio; they do not establish native
display/input parity or complete game functionality.

2026-10-24 authentic launch-flow recheck: DM1's mixed-edition
`dm1_v1_m12_auto_multi_platform_menu` test reached its first runtime through
FM Towns AUTO, bare `--game dm1`, explicit CLI selection of PC/Atari ST/Amiga,
and M12 selection of Atari ST, Amiga and PC. The authentic source-owned
edition handoffs were checked for each route. The `direct-exit` mode of
`test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh` also passed for DM1,
CSB and DM2: each direct `--game` launch exits after Return to Menu without
opening the launcher. Both checks use SDL dummy video/audio; they do not prove
native input/presentation or full gameplay parity.

2026-10-24 fresh Xcode arm64 launcher verification: the Xcode 27.1 Release
binary passed `test_m12_main_menu_game_selection_real_media.sh` against the
installed originals. The no-game menu selected all five games by keyboard;
mouse selection passed for DM1, CSB, DM2, Nexus and Theron. DM1/CSB/DM2 also
passed default `~/.firestaff/data` discovery and single-game-folder scan-root
cases. The script checked each original source archive hash before and after
handoff. `test_m12_main_menu_runtime_real_media.sh` then reached the first
authentic runtime from the menu for DM1, CSB, DM2 and Theron. Nexus remains
verified only through menu handoff, not active-game runtime. Both runs use SDL
dummy video/audio and do not prove native presentation/input or full gameplay
parity.

2026-10-24 Theron native menu route: a local macOS Debug run with authentic JP
Rev 1 Track 02 used Firestaff's SDL input path to reach a loaded dungeon and
reported nonzero game ticks and rendered frames; its presented image was
black. The script sent discrete key-down events, not a continuous 3–5 second
hold. Continue native implementation and connect authentic graphics, palette
and tile mapping to the renderer. Do not use an emulator to run the game in
Firestaff. DMWeb's edition/manual pages and Greatstone SCK inform provenance
and media handling. See [`docs/theron/THERON_CAPTURE_READINESS.md`](docs/theron/THERON_CAPTURE_READINESS.md)
and [`docs/theron/theron_input.md`](docs/theron/theron_input.md).

2026-10-25 Amiga MFM scan hot-path measurement: a one-subtraction fast path
for wrapped bit reads avoids division on the common single-wrap case. On the
same isolated DM1/CSB/DM2 original-media root, four unmodified-build runs had
median process user time of 19.77 s and five optimized-build runs had median
18.90 s (about 4% lower). Wall time medians were 26.06 s and 24.68 s. These
small samples varied noticeably between runs and establish only a modest local
startup-scan gain on one Mac mini M4; they do not establish a cross-platform
improvement. All runs found DM1, CSB and DM2 READY, and the synthetic MFM scan
test passed. The original media remained in place and outside Git.

2026-10-24 startup scan diagnostics: `--debug` scan progress now includes
`delta-ms`, the time since the preceding distinct progress snapshot, alongside
total elapsed time. This makes long gaps between scanner updates visible next
to the current game, task and path; it does not by itself reduce scan time.
The cold five-game scan measurement below remains an open performance issue.

2026-10-23 cold five-game startup scan retest: a fresh isolated-home
`firestaff --scan-data --debug --data-dir ~/.firestaff/data` run took 19.20 s
wall time (17.60 s user CPU, 1.41 s system CPU) on the Mac mini M4 and reported
DM1, CSB, DM2, Nexus and Theron READY. A no-game launcher run against the same
original-media root reported `scan complete elapsed-ms=19552` with all five
games ready. Its SDL dummy render loop continued at about 60 FPS during the
scan; the process used about 1.4--1.5 cores in the sampled windows and GPU
utilization was unavailable under the dummy driver. A five-second macOS
`sample` placed the main-thread cost in hash-list asset discovery, including
native 7z member traversal and Amiga ADF CAPS/MFM decoding, plus ISO-file MD5
work. This confirms the cold scan delays the selectable game list while the
intro/progress loop continues rendering. The next performance work should
reduce repeated archive/member admission without weakening original-media
hash checks or losing edition discovery. This is one cold run on one host, not
a cross-platform benchmark.

2026-10-23 isolated DM1 launcher retest: the new
`m12_main_menu_dm1_only_real_media` CTest passed against the installed
original FM Towns archive. With only that archive exposed under the documented
`~/.firestaff/data/dm1` tree and no `--game` or `--data-dir`, the launcher
remained in the no-game menu, reported DM1 ready and the other four games
unavailable, and handed off to the authentic English FM Towns edition. The
source archive hash was unchanged. SDL dummy drivers verify the scripted menu
handoff, not native display/input or the full DM1 runtime.

2026-10-23 v3.0.409 original-media launcher retest: the current-source
`m12_main_menu_game_selection_real_media` CTest passed in 350.54 seconds. It
selected all five games from the no-game menu using authentic media, covered
keyboard selection for DM1, CSB, DM2, Nexus and Theron, mouse selection for
DM1/CSB/DM2, and repeated launches with each DM1/CSB/DM2 game directory as the
selected scan root. Source archives retained their original hashes. The
updated `m12_main_menu_runtime_real_media` CTest passed in 301.52 seconds and
reached authentic first runtime states for DM1, CSB, DM2 and Theron. Nexus is
still absent from this first-runtime matrix; its menu-selection/handoff test
does not prove entry into an active game. These SDL-dummy results do not prove
native display/input, full gameplay parity, or full Nexus runtime startup.

2026-10-23 current-source original-media launch retest: a focused CTest run
passed 10/10 selected launch checks against the installed `.firestaff/data`
collection. It covered authentic five-game CLI selection diagnostics, the
five-game no-game main-menu picker (keyboard, DM1/CSB/DM2 mouse selection,
default-root discovery, sibling-folder discovery, and Nexus/Theron handoff),
DM1 PC34, CSB Amiga, and DM2 DOS direct startup, DM1/CSB/DM2/Theron main-menu
runtime entry, Nexus Saturn-card discovery launch, and all-game launch
preparation. One separate Theron rendering capture test was skipped because
the active-dungeon VDC-state, SAT and VDC-I/O capture files are not present;
the authentic `TQUS02.bin` game data is present. These SDL-dummy runs establish
only their scripted launch and runtime receipts. Nexus's title-to-menu path,
native window/input behavior, complete gameplay parity, and the synchronous
full-library scan delay remain open.

2026-10-23 Nexus title-window input capture: a matched authentic JP Saturn
A-input/no-input pair on the M4 confirms that the retail `TM.BIN` controller
reader observes A at four source-verified SH-2 PCs during frames 10510--10511.
Both 40-frame raw captures are byte-identical across all eight VDP1/VDP2
regions, so this title window provides no evidence of a menu action or title
transition. The interactive Nexus start-menu handoff remains unresolved; the
capture hashes, source binding and limits are recorded in
`docs/nexus/NEXUS_RUNTIME_CAPTURE.md`.

2026-10-22 external archive scan diagnosis: the final `--scan-data` summary
called skipped archives "no extractor installed" even when `7zz`, `7z`, or
`bsdtar` was available and the scan had skipped it because external tools were
not enabled. The summary now checks each candidate and prints the correct
action. A real scan of `~/.firestaff/data` had all five games READY and
reproduced this message for optional archives.

2026-10-22 authentic five-game main-menu selection: the M12 original-media
game-picker regression now checks keyboard selection and source-bound launch
handoff for all five games. `m12_main_menu_game_selection_real_media` passed
1/1 in 189.98 seconds against the installed originals: DM1, CSB and DM2 FM
Towns; Nexus Saturn JP; and Theron PC Engine EN. Mouse selection remains
covered for DM1, CSB and DM2. The test hashes each selected original source
before and after launch to ensure scanning does not modify it. The SDL-dummy
test exits at handoff, so it does not prove native presentation, Nexus's
title-to-menu transition, or complete gameplay for Nexus or Theron.

2026-10-22 five-game CLI selection diagnostics: the shared supported-game
predicate omitted Nexus and Theron even though their slots and direct launch
paths are catalogued. Unqualified `--game nexus` and `--game theron` could
therefore retain a stale saved platform, and `--verbose`/`--debug` omitted
their selected-edition and launch-handoff receipts. The predicate now covers
all five game IDs. The original-media `firestaff_cli_startup_diagnostics_real_media`
CTest passed 1/1 in 249.96 seconds with the per-game CLI loop checking all five
editions, handoffs and startup-frame traces. A separate authentic boot-probe
matrix reached the runtime handoff for DM1, CSB, DM2, Nexus and Theron; debug
output now includes all five edition/source handoff receipts and source-bound
startup-frame traces. These SDL-dummy probes use zero runtime frames and
establish CLI selection and handoff only, not native presentation, interactive
input or complete gameplay parity.

2026-10-22 current-source launch verification: the focused real-media main-menu
selection/runtime CTests passed 5/5 for DM1/CSB/DM2 game selection and runtime,
Nexus startup-menu source and Saturn-card startup, and Theron's Track 02 scan
receipt. `m11_direct_launch_prepare_all_games` passed against the complete
installed original-media collection, including all five selected-entry
handoffs and the authentic Theron level-0 party. A separate 12-test CLI matrix
passed for DM1 PC34/Amiga/Atari ST/FM Towns, CSB Atari ST/Amiga/FM Towns
(English and Japanese), and DM2 DOS/Macintosh/Amiga/FM Towns. These CTests use
SDL dummy video/audio and establish their scripted launch/runtime contracts;
they do not prove native display/input or complete gameplay. Nexus remains
blocked after its authentic title because the retail PLRD-to-FACE roster join
and menu consumer are not source-bound. Theron reaches its authentic runtime
state but its dungeon presenter remains fail-closed without a verified Saturn
VDC/VCE capture. The local full-library scan reported all five games ready in
8.88 seconds; this is one warm M4 measurement, and scan responsiveness remains
open.

2026-10-22 authentic return-to-menu and rescan retest: the current-source
CTest selection `^return_to_menu_rescans_(dm1_csb_dm2|selected_archive|removed_archive|media_mutations)_real_media$`
passed 4/4 in 1463.60 seconds against installed original media. The broad
DM1/CSB/DM2 route covers menu return and rescanning across DM1 PC, FM Towns,
Atari ST and Amiga, CSB FM Towns and Amiga, and DM2 PC and Amiga. The focused
tests cover selected-archive changes, removed archives, and media mutations.
This verifies the tested local SDL-dummy launcher flows, not native display or
physical input. GitHub Actions run 39707573248 for the source commit also
completed successfully on Ubuntu, macOS and Windows.

2026-10-22 authentic CSB/DM2 CLI startup retest: the current-source CTest
selection `^(csb_v1_.*native_cli_boot|dm2_v1_.*native_cli_boot)$` passed
11/11 against installed original media. It covers CSB Atari STX, Amiga,
FM Towns, Amiga 3.5 CTRaw and Hint Oracle, plus DM2 DOS English/French,
Macintosh English/French, Amiga and FM Towns. The suite verifies the listed
CLI startup routes; only the Atari STX case carries the `menu` label, so this
does not establish M12 menu coverage for every edition. SDL dummy video/audio
does not prove native display, physical input, or full gameplay. The separate
DM1 authentic menu test passes PC, Atari ST, Amiga and FM Towns, and its combat
tests pass for those editions plus IIGS; complete five-game menu coverage
remains open.

2026-10-20 authentic scan-cache replacement: the persistent hash cache keyed
only on path, second-resolution mtime, and byte size. Replacing CSB FM Towns
`DUNGEON.DAT` with the authentic Amiga 3.3 file (same 2098-byte size and
restored Towns mtime) reproduced a stale hash in `--scan-data --verbose`.
Cache receipts now include mtime/ctime precision, device, inode, and size in a
versioned cache file. A repeated-start test then exposed a second defect: the
reader treated the inode field as the beginning of the digest. It now parses
all eight delimiters and accepts only exactly 32 hexadecimal digest characters.
The real-media regression checks a warm persisted read before replacing the
Towns file in place with the same-size Amiga original.

2026-10-20 DM2 archive-scan cancellation: the M12 DM2 ZIP profile now passes
scan progress into the inflater for the CUE-selected original disc image.
Closing or cancelling during that large decode stops extraction and discards
the partial profile before game availability can be published. The new
`m12_dm2_archive_scan_cancel_real_media` CTest cancelled the installed
Macintosh retail archive during inflation in 6.72 seconds; the normal
`test_dm2_v1_mac_boot_real_media` profile and boot test passed in 2.13 seconds.
Both use authentic local game media. The no-game intro's original-media
diagnostic also closes the window after DM2 disc-image inflation has begun and
requires the scan to cancel before any game handoff. This verifies scanner
cancellation and normal profile loading; native UI responsiveness on slower
hosts still needs runtime confirmation.

2026-10-20 unchanged-file scan hashing: named version candidates now use the
same metadata-validated physical-file digest cache as recursive discovery,
instead of re-reading each unchanged DAT directly during every startup scan.
On the Mac mini M4 with the installed original collection, three warm
`--scan-data --debug` runs completed in 8.308, 8.297 and 8.308 seconds and
reported all five games READY. The immediately preceding two warm runs took
9.98 and 9.96 seconds; this is a local comparison, not a cold-cache or
cross-platform result. A 3-second post-change `sample` now puts the leading
work in the authentic DM2 FM Towns ZIP image inflate; sample counts indicate
where the sampled interval spent time, not full-run elapsed percentages. The
full scan still blocks first menu interaction for about 8.3 seconds on this
setup, and the remaining archive decode still needs investigation.

2026-10-20 DM2 disc-image scan memory: the original Mac English BIN in
`Dungeon-Master-II-Skullkeep_Mac_EN (1).zip` expands to 758,851,632 bytes.
`sample` traced its M12 DM2 profile path through `zip_extract_impl`; standard
heap frees retained large empty regions after the five-game scan. The CUE-owned
image now uses an OS memory mapping, and the DM2 profile records whether it owns
that mapping so cleanup uses the matching release operation. On the same warm
M4 collection, process RSS after scanning fell from 1,236,640 KiB to 471,664
KiB (about 747 MiB); this is a single local measurement, not a platform-wide
benchmark. All five authentic installations still reported READY, and the
authentic DM2 Mac, FM Towns, and renamed-ZIP CTests passed (573.63 seconds).
The image still occupies memory while its HFS/CD structure is parsed, and the
first collection scan still blocks menu interaction for about ten seconds.

2026-10-20 post-mapping startup scan timing: the current M4 build ran
`firestaff --scan-data --debug` twice against the installed original-media
collection in 9.98 s and 9.96 s wall time (9.13 s user CPU and about 0.82 s
system CPU each). All five games reported READY in both runs. This confirms
that reducing retained heap did not remove the synchronous first-scan delay;
startup still spends nearly all elapsed time in CPU work before the menu is
interactive. These are repeat warm-cache measurements, not a cold-cache or
cross-platform benchmark. A four-second `sample` captured most main-thread
work in `m12_admit_dm2_directory_zips` and `dm2_v1_boot_scan_assets`, nested
inside ZIP deflate (`mz_inflate`/`tinfl_decompress`); another substantial
slice was version-candidate MD5 hashing. The profile reported 415 MiB physical
footprint at capture and 1.1 GiB peak, so mapped ownership reduced retained
heap after scanning but did not remove the peak working-set cost. Profile
sample counts are evidence for where to investigate, not exact elapsed-time
percentages.

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

2026-10-28 full-library scan latency correction: the measurements below still
describe the time to finish scanning the entire collection, but they overstate
the delay before the launcher appears. The current no-game path defers the
all-game scan, starts it alongside the three-second startup intro, and enters
the interactive menu while the worker continues. DM1, CSB and DM2 cards become
eligible as their scans finish. Nexus's BPK metadata receipt is refreshed at
the end of its own scan, so its card can unlock before Theron's later scan.
Theron remains gated until the final CUE ownership and loader-receipt refresh;
an explicit `--game` launch also keeps its synchronous edition-selection scan.
The long full-collection scan remains a performance gap, but it no longer
blocks the no-game launcher from opening. This correction is based on
`main_loop_m11.c`'s deferred-scan startup path, the Nexus/Theron media refresh
boundaries in `asset_status_m12.c`, and `menu_startup_m12.c`'s per-game
completion mask. On Oct 28, the fast-dev build scanned the installed
original-media collection in 6.52 seconds with the default external-tool
policy; all five games were READY, while 15 `.7z`, `.rar`, `.sit`, or related
archive candidates were reported as skipped. Repeating the scan with the
explicit `--enable-external-archive-tools` option read those candidates, again
reported all five games READY, and took 24.25 seconds. The second run followed
the first and the OS file cache was uncontrolled, so these are warm, local
observations rather than a cold-scan benchmark. They measure synchronous
`--scan-data`, not time to first interactive menu frame or native input.

2026-10-28 current-source progressive startup verification: the fast-dev
application build succeeded, and
`test_firestaff_cli_startup_diagnostics_real_media.sh` passed against the
installed original media. The no-game launcher discovered all five games;
its partial snapshot reported Nexus available while Theron's final scan was
still pending. Direct CLI boot probes selected the expected authentic
platforms for all five games. Nexus deliberately stopped at `title-entered`
with `levelLoaded=0` and a blocked startup receipt, so this verifies media
selection and menu admission timing, not playable Nexus gameplay. The script's
Macintosh ZIP branch skipped because this installation has extracted/SIT media
instead; the separate extracted-file DM2 test covers that layout. SDL dummy
output does not verify native display, audio or physical input, and this run
did not measure total scan time.

2026-10-19 full-library scan cache follow-up: the persistent digest cache now
uses indexed path lookups instead of walking every cached member for each
archive entry. Native 7z entries and nested Amiga ADF contents also reuse
digests while validating the source archive's size and modification time.
Two no-game debug starts against the installed 436-file, 4.3 GiB original-media
collection completed in 20.98 and 18.00 seconds, including the requested
one-second menu duration; the earlier runs took about 27 seconds for the scan.
The complete original-media DM1/CSB/DM2 CLI diagnostics passed in 268.36
seconds. Those historical runs predate the deferred interactive scan path;
they do not establish current menu latency, native video/audio, or
physical-input behavior.

2026-10-19 persisted single-file launcher selection: the no-game startup now
promotes a remembered media file to its surrounding collection root, so the
default launcher can discover neighboring DM1, CSB and DM2 installations.
An explicit file-valued `--data-dir` remains scoped to that selected source.
Verified with an isolated config pointing at the authentic DM1 Amiga 3.6 ZIP;
the default launch stayed in `mode=menu`, found DM1/CSB/DM2, and did not hand
off to a game. The recorded 18-to-21-second full-collection scan remains a
historical scan-cost measurement; current no-game menu entry overlaps scanning
with the startup intro. DM1/CSB/DM2 unlock progressively, Nexus after its own
BPK metadata refresh, and Theron after its final CUE/loader refresh.

2026-10-19 full-library no-selection launch check on the Mac mini M4:
starting the current build with `--menu --debug --data-dir
~/.firestaff/data` and no `--game` reported `game=none mode=menu`, found
authentic original data for DM1, CSB, DM2, Nexus and Theron's Quest, and made
no game-handoff. The local `m11_direct_launch_prepare_all_games` CTest also
passed against the installed original-media collection. The no-selection
scanner reported completion at about 27 seconds on both initial and repeated
runs; the second run took 57.18 seconds total with a 30-second menu duration.
These measurements predate the deferred interactive scan path and record full
scan cost rather than current menu-entry latency. These checks
used SDL dummy video/audio and confirm availability/menu state, not native
window interaction or first gameplay frames for all five titles. The long
full-library scan remains an open performance gap; see the 2026-10-28
correction above for its current effect on menu and per-game launch readiness.

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
  dialogs. `test_m12_data_dir_dialog_real` now covers the asynchronous result
  callback, main-thread admission and reopening with the saved per-game folder
  for authentic DM1, CSB and DM2 data. It also checks that the automatically
  selected edition is authenticated and remains selected after reopening. Each
  case skips cleanly when its local original-media folder is unavailable in CI.

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
recorded in `DONE-<game>.md`. Historical mixed logs are retained in
`docs/archive/HISTORY-archived-2026-08-08.md` and Git history, not as active work.

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
