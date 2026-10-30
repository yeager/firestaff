# Five-game startup support gap analysis

Evidence metadata refreshed on 2026-10-30 for `main` at
`44b0729b2231834f0b1f207a4fc86ba3cbdb57c4`. The latest published release
remains v3.0.420 (published 2026-10-29 from tag commit `8224d5c8`); current
`main` contains later source changes that are not in that package. The v3.0.420
workflow verifies packaging, not gameplay startup. The
route records below are individually dated and establish only the named
checks, not every edition or complete game parity. Real-media Swedish
startup-debug localization checks cover DM1 PC34,
CSB FM Towns, and DM2 FM Towns; these test explicit and saved preferences,
localized scan labels, edition selection, and the boot-probe boundary, not full
gameplay parity.
The latest `main` also adds read-only X68000 HDM, GRAPHICS.DAT IMG1, and X30J
dungeon-metadata readers verified against authentic Japanese media; these
parsers do not add an X68000 startup, rendering, audio, or input route.
It rejects conflicting CSB utility-disk and hint-oracle launch modes and
enables the DM2 source-level startup-menu timing regression. The latter checks
source tick sequencing; it does not measure native presentation timing.
Current authentic-media CLI routes
include DM1 PC34, DOS, Atari ST, Amiga and FM Towns; CSB Atari, Amiga and FM
Towns; DM2 DOS English/French, Macintosh English, FM Towns Japanese and Amiga
English. The supplied French-named Macintosh SIT currently extracts to the
same application, resource forks and DMFiles as the English Macintosh SIT, so
its successful startup is English-media evidence, not French-edition support.
Theron's Japanese Rev. 1 title-to-Akutuba plus US Continue to dungeon 2, level
0 are also covered. The `dm2_v1_mac_extracted_native_cli_boot` regression records
direct CLI and M12 startup of DM2 Macintosh English from extracted original
files to first runtime; the `.sit` container itself is not unpacked by the
ZIP-only CTest. The authentic French DM2 DOS ZIP now resides under the standard
`~/.firestaff/data/dm2` root, and its native CLI/M12 CTest passes with the
default archive path. CTest also accepts a custom
`FIRESTAFF_DM2_DOS_FR_ARCHIVE` path. The DM1 Amiga demo archive also passes
its authentic CLI/M12 startup test
(`tests/test_dm1_v1_amiga_demo_archive_cli_boot.sh`) against the original ZIP.
The CSB Amiga ZIP/ADF wrapper's authentic decoder/startup test was rerun on
2026-10-28 and passed direct CLI and M12 startup through the source entrance
to authentic movement. CSB Atari French preservation remains unverified. On
2026-10-28, `m12_main_menu_game_selection_real_media` also passed keyboard and
mouse selection/handoff for all five game cards against authentic local media,
including default-root launches; `firestaff_cli_startup_diagnostics_real_media`
passed its direct CLI source-resolution and platform checks. Dedicated
DM1 Amiga demo, CSB Amiga archive, and extracted DM2 Macintosh English tests
also passed their CLI and menu routes. These checks establish only the named
startup/handoff bounds. For Nexus, retail New Game input now reaches
the hash-verified `LEV01.DGN` loader and produces a source-bound
level-1 `(29,47,North)` receipt from keyboard and mouse routes; the separate
viewport/HUD gates still prevent a playable runtime handoff. These bounded CLI
checks use SDL dummy and do not establish native display, physical input, or
full gameplay. Recent
changes also add authentic Japanese Theron input evidence and Nexus retail
reference captures. Ymir reaches the Japanese and English three-choice title
menu on cold boot, and a separate Japanese diagnostic New Game replay reaches
the native 3D corridor. Firestaff now renders source-bound English and Japanese
New Game/Continue foreground sprites and selected Configuration sprites from
authentic `TITLE.BIN`; source-bound mouse selection is verified for the
Japanese Configuration row. The shared background/full-screen menu composition
is still unbound. The New Game path reaches the authenticated level loader and
emits the retail level-1 start receipt, but the viewport/HUD handoff remains
fail-closed.
Continue/save loading and Configuration destination behavior also remain
open, so Firestaff has not entered a Nexus playable level. Replaying the English
screenshot from its captured frame is pixel-exact. Its complete VDP1 command
trace has 126 informative source matches among 137 textured draws; 11 zero-
filled spans are excluded. The source join remains partial, with menu
background, full palette/display ownership, and destination execution
unproven. Runtime receipts cited below remain tied to their stated revisions
unless noted otherwise.

### 2026-10-30 DM1/CSB/DM2 menu return, rescan, and cache retest

On `main` at `9f4bb4df`, the local app was rebuilt after the shared media
scanner changes. Authentic-media M12 coverage passed for the DM1, CSB, and DM2
game cards; DM1 PC and Atari ST, CSB Atari ST, and DM2 PC and Amiga platform
routes; DM1 mixed-platform AUTO; and the CSB Atari handoff and mixed FM Towns
media matrix. The two small DM2 FM Towns and Amiga media tests initially had no
local executable, so their targets were built and both tests were rerun and
passed. The five direct DM2 CLI tests also passed with original DOS English,
DOS French, extracted Macintosh English, Amiga English, and FM Towns Japanese
media.

The real-media return-to-menu harness passed separately for DM1 PC34, CSB FM
Towns, DM2 DOS, and DM2 Amiga. Each route reached gameplay, returned to M12,
and reported current availability for all five games. The DM1 PC34 keyboard
and mouse window-close cases both cancelled during the return scan without
publishing a completed scan. Mutation checks removed the authentic CSB archive
while DM1 was running and confirmed CSB became unavailable after return; a
second run added authentic CSB media during DM1 gameplay and confirmed the
return scan discovered it. The original media were linked into isolated test
roots and were not modified.

The authentic CSB FM Towns cold/warm scan test passed: the cold scan
authenticated both original editions and the warm scan reused the verified
receipt. The DM2 DOS original-media directory-cancellation test passed for
scoped and unscoped scans; cancellation rolled back the partial publication
and preserved the archive hash. No new CLI, M12 return, scan-cancellation, or
cache-invalidation defect was reproduced in these checks. They use SDL's dummy
video driver and do not establish native display, physical input, full
animation playback, or playable-game parity.

### 2026-10-30 Nexus Japanese CUE-only CLI and menu admission

On TRV2, the updated native menu regression built against authentic original
media and passed `nexus_v1_native_menu_mouse_title` (CTest 1/1). It verifies
direct CLI launch and M12 selection for a Japanese Nexus disc supplied only
as its authentic CUE/BIN files; both preserve the exact selected CUE as the
runtime media owner. The regression also selects an authentic non-Nexus DM2
CUE while valid Nexus media is present and verifies that admission fails
closed instead of silently switching discs. The resulting Nexus receipt still
stops at `title-vdp-owner-compositor-capture-required`: this closes a media
admission and menu-handoff gap, not the title compositor, viewport/HUD, or
playable-game gaps. No synthetic game image or trace data is used by these
cases.

### 2026-10-30 DM1/CSB/DM2 real-media startup sweep

On the local host, the `main` build at `d0962f37` passed the debug boot-probe
matrix against the installed original-media collection: DM1 PC, Amiga, Atari
ST and FM Towns; CSB Amiga, Atari ST and FM Towns; DM2 PC, Amiga, FM Towns and
Macintosh. All 11 routes returned `boot-probe-ready`. DM1 reached
`dm1-runtime`; CSB reached its platform-specific entrance/animation/title
startup phases; DM2 reached `dm2-startup-menu`. The probes used SDL's dummy
video driver and establish only those reported startup receipts, not native
presentation, physical input, complete animation playback or gameplay parity.

A separate bare CLI check for each priority game omitted `--platform`,
`FIRESTAFF_DATA` and `FIRESTAFF_ORIGINALS_DIR`. With the standard user data
root, DM1 and CSB selected their installed FM Towns editions and DM2 selected
FM Towns Japanese; all three logged a direct game handoff. This verifies the
default data-root and platform-auto routes on this host only. It does not
replace per-platform testing or establish that a game is playable.

The three isolated no-game M12 checks also passed for DM1, CSB and DM2. The
CSB Amiga no-game M12 test passed separately. The DM1 Amiga 2.0 archive M12/CLI
test and the DM2 Macintosh extracted-file M12/direct-runtime test also passed.
Five additional authentic-media M12 routes passed: DM1 PC34 and Atari ST, CSB Atari
ST, and DM2 PC and Amiga. These verify game-card/platform selection and the
reported runtime handoff for those routes under SDL's dummy video driver; they
do not prove native presentation or full gameplay. On 2026-10-30, the broader
`m12_main_menu_game_selection_real_media` run reached 544.85 seconds before
failing its default-root Nexus assertion. Its 13 preceding DM1, CSB and DM2
routes passed. The assertion expected `DM.BIN`, while the authentic extracted
Japanese Nexus route handed off `RLOWFIX.BIN`; the test now selects that actual
source and requires the documented capture-gated title receipt. The test
timeout is 1200 seconds to cover the full authentic-media scan and route set.
The corrected run is pending, so the broader test is not yet counted as a pass.
The no-media `unsupported_game_platforms_cli_rejected_before_renderer` test
also passed, confirming invalid game/platform pairs fail before the renderer.

The latest Nexus evidence separates raw Japanese `DM.BIN` placement at
`0x06010000` from the older trace-specific static code mapping at
`0x06010040`. The English-version `MENU.BPK` has an authenticated hash and is
accepted by the regional snapshot analyzer. In the same-session English
capture, 86 of 162 surfaces have direct byte matches in VDP1 VRAM, but none of
137 textured commands starts inside those matched ranges. This establishes
`MENU.BPK` residency, not visible-menu ownership. Firestaff's bounded title
renderer instead uses separately verified `TITLE.BIN` foreground sprites; the
shared background and complete display composition remain unbound.
The matched Japanese runtime trace now binds a strided zero-fill loop to VDP1
CLUT address aliases, but it does not establish palette upload or ownership.
Since the earlier gap review, the M12 platform card now displays a pending
deferred scan as `SCANNING GAME DATA` instead of implying that files are
missing. The launch gate still waits for that game's completed scan receipt.
An authentic US Theron keypad test also corrects its expected pose after a
backward step followed by KP8; it changes the regression expectation, not game
movement behavior.
Two additional authentic US Theron cold-start probes reached only the
System Card; controller delivery was observed, but no CD interrupt, sector
transfer, or title-menu transition was established. This is negative evidence
for that bounded route and does not validate or invalidate viewport-click
semantics after a game-owned handoff.
The repository contains checks
against authentic media for all five games, but “available,”
“selected,” “handed off,” and “entered a playable runtime” are separate states.
The current evidence does not establish complete five-game startup support.

### 2026-10-29 selected-archive return-to-menu checks

The shared real-media return/rescan harness now has a dedicated selected-file
case for each priority game. On the current local build, CTest passed 3/3:
DM1 PC34 (`return_to_menu_rescans_dm1_selected_archive_real_media`, 60.20 s),
CSB FM Towns (`return_to_menu_rescans_csb_selected_archive_real_media`,
91.72 s), and DM2 DOS English
(`return_to_menu_rescans_selected_archive_real_media`, 55.36 s). Each case
starts from the exact archive path in an isolated `data/<game>` collection,
reaches that edition's first runtime, returns to M12, and verifies that the
scan infers the collection root and sees DM1, CSB, and DM2. The staged links
preserve the original archive bytes. An earlier generic alias produced
`source=none`; a focused 2026-10-29 regression now proves both explicit CLI
selection and no-game M12 discovery for an authentic CSB FM Towns ZIP under a
neutral basename. The scanner checks the in-archive CUE, raw CD layout, and
recognized graphics/dungeon hashes; the M12 test uses an isolated CSB-only
data root and verifies the original archive hash is unchanged. Automatic
discovery is bounded to direct ZIP children, and RAR admission remains limited
to its existing canonical route. Nexus and Theron are not staged in these
focused cases. These receipts prove only the named selected-archive startup
and return routes and do not establish full platform or gameplay parity.

### 2026-10-28 clean-main DM1/CSB/DM2 menu retest on TRV2

A detached, clean checkout of `main` at `3bbcc5cc` was configured with
`BUILD_TESTING=ON`, `CMAKE_BUILD_TYPE=Release`, and the TRV2
`~/.firestaff/data` root. The target app built successfully with at most two
compile jobs. The in-scope original-media test selection passed 3/3:
`m12_main_menu_dm1_only_real_media` (123.92 s),
`m12_main_menu_csb_only_real_media` (126.55 s), and
`m12_main_menu_dm2_only_real_media` (132.67 s). Each covers that game's
no-game M12 card selection and FM Towns handoff against its installed original
Japanese/English archive. The DM1 archive SHA-256 was
`c0c940e6e7c056998ce592c343f2817fabe071b05779fce305e7d927aa6721f3`, CSB was
`54b40c1fd0b18ca2df1dbcd70c6cb07fb0333d540b99f8108b1fca84aa192b88`, and
DM2 was `ed68fd6dc0072d7da9e29f87049fecf50da4414b938b140bcfdda5b9d5e69708`;
all three hashes matched before and after testing. These bounded SDL-dummy
checks establish only the named card-to-owner startup routes, not physical
display/input, native presentation, or complete platform parity.

### 2026-10-28 DM2 FM Towns debug trace follow-up

The first local run of the combined startup-diagnostics script failed its
TWANIM frame-trace assertion: the normal direct `--game dm2 --debug` route
completed the authenticated 225-frame sequence in 26,211 ms, but catch-up
batches hid some intermediate source states from the debug log. The trace now
records each DM2 FM Towns source state as its Timer-A tick is consumed. The
new focused `dm2_v1_fmtowns_debug_source_frame_trace_real_media` CTest passes
against the installed original Japanese FM Towns ZIP and records every
displayable source-frame index and completion without boot-probe fast-forward.
This closes the debug trace gap only. The runtime may still skip presenting
intermediate frames during scheduler catch-up; native display parity remains
unverified.

### 2026-10-28 current TRV2 no-game menu follow-up

The Debug build from this worktree ran against the currently installed
`~/.firestaff/data` corpus. `m12_main_menu_game_selection_real_media` passed
the default FM Towns launch handoffs for DM1, CSB, and DM2, plus DM1 PC34 and
Atari ST selection, before its exact DM1 Amiga 2.0 archive assertion failed.
The separately installed `Dungeon-Master_Amiga_EN.zip` HD package authenticates
as DM1 Amiga 2.0 and passed both direct CLI startup and a no-game M12 keyboard
selection to the `amiga20-en` handoff. The currently staged
`Dungeon-Master_Amiga_EN_Version-20.zip` copy on TRV2 did not pass its direct
CLI check; its digest and host-specific failure are recorded in
[`TODO-dm1.md`](../TODO-dm1.md). This is distinct from the current local
archive copy and must not be generalized to it.

### DM1 Amiga 2.0 local archive verification

On the local host, the current archive has SHA-256
`f2b76bb9a818590ee84db37e4bbb5fa84ce0fb93e3f2135807b70c0545fb6ec9`. A fresh
Debug `firestaff` build launched its exact nested retail ADF from direct CLI
and M12. `test_dm1_v1_amiga_v20_archive_cli_boot.sh` passed the authenticated
CLI, Quick Resume, menu, title, entrance-door, and movement checks. The
in-memory `test_dm1_v1_amiga_graphics_dat` passed 116/116 checks after its save
test setup was corrected to represent the Amiga big-endian target profile.
The boot probe identified the expected Amiga 2.0 English graphics MD5 and
reported `levelLoaded=1`; the test also confirmed the original archive hash
was unchanged. The different TRV2 archive hash
(`6977b1042769c0f6ea22cb42fa7308ed609242dc0eeebd1e00d68660bdb37e2b`)
still needs its own reproduction before that host-specific failure can be
resolved or attributed to runtime code.

With the standard per-user data root, the launcher scan completed with all
five games marked available. Keyboard selection then handed off Nexus Saturn
from the original `nexus/DM.BIN` and Theron PC Engine from the original
`theron/TQUS02.bin`. Together with the preceding DM1/CSB/DM2 menu handoffs and
the separate authentic Amiga HD handoff, this verifies tested menu selection
and source handoff for each game in this corpus. These exits occur at the
handoff boundary; they do not establish complete gameplay. Nexus still lacks
the viewport/HUD receipts required for a playable-level handoff, and SDL dummy
does not verify native display or physical input.

## 2026-10-28 local and TRV2 real-media retest

### Targeted DM1/CSB/DM2 regression follow-up

After `main` reached `744ac459`, a local Debug build ran a seven-case
real-media CTest selection covering DM1, CSB and DM2 only. Six tests passed:
the DM1, CSB and DM2 M12 card-to-runtime routes, the DM2 Amiga M12 runtime,
the DM1 PC-34 CLI/input matrix, and the CSB Atari STX CLI boot. The DM1 PC-34
test passed in 477 seconds and exercises more than startup, including source
menu interaction, HoC actions, inventory/champion input and debug performance
telemetry. The French DM2 DOS CLI test was marked `Skipped` immediately by
CTest in this build; this run therefore adds no French DOS result. The test
selection completed in 1,081.53 seconds. The run was on the local macOS host,
not `trv2`. `AGENTS.md` requires the dedicated `trv2` host for large test
selections, so this run is noncompliant and is retained as an exploratory
observation only; it is not accepted validation evidence. Its SDL-dummy
results do not establish native display/GPU behavior or complete gameplay
parity.

Accepted TRV2 rerun: the same seven-case selection was rebuilt and rerun on
the dedicated TRV2 host using PR #180's code revision `55122af6`. Six tests
passed on TRV2:
the DM1, CSB and DM2 M12 card-to-runtime routes, DM2 Amiga M12 runtime, DM1
PC-34 CLI/input matrix, and CSB Atari STX CLI boot. The DM1 PC-34 matrix took
700.23 seconds; the full selection took 923.43 seconds with at most two tests
running concurrently. The French DM2 DOS test skipped because its authentic
archive is not present on TRV2. These SDL-dummy results verify only the named
scripted routes, not native display/GPU behavior or full gameplay parity.

The installed DM2 Macintosh English StuffIt archive was recovered from the
user's Downloads folder and its SHA-256 matched the preservation listing. The
archive is retained under `~/.firestaff/data/dm2`; its original files were
extracted, preserving the source bytes, to `dm2/mac-en/Dungeon Master II`.
Firestaff does not discover the `.sit` archive itself during a normal
collection scan, but it does discover the extracted `DMFiles` directory. A
direct `--game dm2 --platform mac` run against that directory completed the
authentic title movie, loaded a level and entered the first runtime frame with
real assets and no core fallbacks. A one-frame boot probe also confirmed the
retail edition identity and selected graphics source. The later
`dm2_v1_mac_extracted_native_cli_boot` regression records both direct CLI and
M12 startup to the first runtime using the extracted original files, and
checks that the original media is unchanged.

The local real-media diagnostics script passed its five-game collection scan,
bare `--game` selection checks, DM1/CSB/DM2 priority-platform selection, and
the exercised startup-debug traces. Its dedicated DM2 Mac archive subsection
was skipped because that script expects a regular ZIP file. The dedicated
extracted-files regression covers the direct CLI and M12 routes, while the
`.sit` container itself remains outside the ZIP-only archive test. This local
run does not close the Nexus playable-start gate or establish native
display/input or full gameplay parity.

The missing DM1 Amiga playable demo was recovered from the user-provided
PlanetEmu reference. The SGT ZIP contains a bootable ADF; after unpacking its
filesystem, `graphics.dat` and `DemoDun.dat` matched the expected authentic MD5
values (`491ca939f9abb33ceeb26619b841fe91` and
`78848e1a2d3d5a11e5954deb8c7b772b`). Direct Amiga CLI startup and the M12 menu
both selected `amiga-demo-en` and reached the loaded runtime. The original ZIP
is preserved in the DM1 data folder. A Firestaff-compatible flat ZIP was made
from the unchanged original `graphics.dat`, `DemoDun.dat`, and `swoosh` files;
archive and loose-file CLI/M12 coverage then passed with the expected hashes.
No game data was synthesized.

That run exposed a test portability issue: APFS returned the selected source
path as `GRAPHICS.DAT`, while the regression expected `graphics.dat`. The test
now compares the path case-insensitively; the game/file identity remains the
assertion. The authenticated Amiga demo archive/loose-file CLI and M12 test
passes after this correction.

### DM1 Amiga 2.0–2.2 startup recheck

The authentic Amiga software archive exists at
`~/.firestaff/data/dm1/Game,Dungeon_Master,Amiga,Software.7z`, but the local
build initially lacked the `test_dm1_v1_amiga20_french_startup_real_media`
probe executable, so CTest marked the German 2.0, French 2.0, and English
2.1/2.2 routes skipped before reading the archive. After building that target,
all three skip-safe CTests passed against the original archive:

- German Amiga 2.0: the media probe verified the original ADF, graphics,
  SWSH, and dungeon hashes; direct CLI and M12 reached the loaded runtime.
- French Amiga 2.0: the probe selected the original ADF bytes, verified the
  source-owned graphics and SWSH identities, and direct CLI/M12 completed the
  authenticated title and entrance routes.
- English Amiga 2.1 and 2.2: both original ADFs passed their media receipts;
  each direct CLI and M12 route completed the source title, entrance, and
  paired-dungeon load.

The tests create their ZIP wrappers only in scratch from bytes read from the
original archive; the original archive is not modified. The Amiga 2.0 M12
regression also now spaces its scripted menu inputs across M12 frames and
allows 10 seconds for the launch, avoiding the earlier no-launch flake. That
targeted regression passed twice after the adjustment. These are bounded
startup receipts and do not establish native Amiga display/input or full
campaign parity.

### DM1, CSB, and DM2 CLI/M12 sweep

The 2026-10-28 local sweep used the installed authentic archives and extracted
original files. It exercised SDL-dummy CLI and M12 routes; it did not use
synthetic game data or establish native display/input parity.

- **DM1:** The 20-case edition matrix covered PC34 (flat and nested ZIP), DOS
  English/French, Atari ST English/German/French editions, Amiga HD/demo/2.0/
  2.0 German/French/2.1/2.2/3.6, and FM Towns. Sixteen cases passed in the
  first matrix invocation; the Amiga 2.0 M12 case was timing-sensitive and
  three language/version cases were skipped because their media-probe target
  had not been built. After building that target, German 2.0, French 2.0, and
  English 2.1/2.2 all passed against the original Amiga archive. After pacing
  the Amiga 2.0 menu inputs, its targeted CLI/M12 regression passed twice.
- **CSB:** Twelve of thirteen CLI tests passed across Atari STX/original and
  nested-ZIP routes, the utility disk/Hint Oracle, Amiga 3.1/3.3/3.5 and
  CTRaw, and FM Towns. The French Atari preservation test skipped because
  `~/.firestaff/data/csb/chaos_strikes_back_ftl_france.zip` is absent.
- **DM2:** DOS English, French DOS, extracted Macintosh English, and Amiga
  English passed. The French-named Macintosh SIT also reached runtime, but the
  local extracted application, resource forks and DMFiles are byte-identical
  to the English SIT; this is not a distinct French-edition result. The FM
  Towns Japanese M12 route also passed after its
  `test_dm2_v1_fmtowns_menu_source_rgb` helper target was
  built. The French DOS ZIP passed the native CLI/M12 CTest from the standard
  game-data root without an archive-path override. The Macintosh English ZIP
  wrapper is absent; the installed Mac `.sit` source is covered through the
  extracted-original-files CLI/M12 test.

Missing helper executables explain local CTest skips when the corresponding
source archives are present; build the named helper target before treating a
skip as absent media. The DM2 French test's CMake cache option
`FIRESTAFF_DM2_DOS_FR_ARCHIVE` keeps the standard game-data directory as its
default and allows another authentic archive path when needed. The French CSB
Atari archive remains unavailable locally, so that route is still unverified.
The local test matrix proves only
the listed startup/handoff paths; it does not close the reported renderer,
HUD, audio, physical-input, or full-gameplay gaps.

## Route matrix

| Game | Direct CLI evidence | Main-menu evidence | Current startup result |
|---|---|---|---|
| DM1 | Current-source authentic-media CLI routes pass for PC34 (including a nested archive), DOS English/French, Atari ST 1.0a/1.0b/1.1/1.2/German/French, Amiga HD/2.0/2.0 German/2.0 French/2.1–2.2 English/3.6, FM Towns, and the playable Amiga 2.0 SGT demo ADF, exercised through its extracted originals and a Firestaff-compatible ZIP made from those unchanged files. | The picker test covers keyboard and mouse handoff; the demo-specific M12 route selected authentic `amiga-demo-en` media and reached its loaded runtime. The mixed-media AUTO route enters through the no-game menu and reaches its authentic first runtime frame. | First runtime is verified for the named media routes, but native display/input, campaign depth, and full edition parity remain open. See [`TODO-dm1.md`](../TODO-dm1.md). |
| CSB | Current-source authentic-media CLI routes pass for Atari STX, original Atari, nested Atari ZIP, Atari Utility Disk 7z/Hint Oracle, Amiga A31E/A33M/A35E/CTRaw, and FM Towns English/Japanese. The Atari French archive is absent. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic starting party and viewport receipt. | First runtime is verified on bounded routes. In Amiga CSB, F0107 now uses source MEDIA720 scales and authentic Amiga C696 anchors for M615 ornaments; matched-frame, palette/light, trapdoor and platform-wide viewport parity remain open ([evidence and remaining gap](../TODO-csb.md)). Native physical input remains open. |
| DM2 | Current-source direct CLI routes pass for the original DOS English archive and the French DOS ZIP, Macintosh English extracted files, FM Towns Japanese, and Amiga English ZIP. The French-named Macintosh SIT also reaches runtime, but its extracted app, resource forks and DMFiles match the English SIT byte-for-byte; do not count it as French-edition evidence. | The picker test covers keyboard and mouse handoff for PC and FM Towns Japanese; Amiga has a no-game M12 runtime test; the Macintosh English regression covers extracted-file M12 launch. | First runtime is verified on bounded routes, including DOS English/French, Macintosh English, FM Towns Japanese, and Amiga English. Macintosh French identity and runtime remain unverified; physical input, native presentation and full edition parity remain open ([`TODO-dm2.md`](../TODO-dm2.md)). |
| Theron | Authentic Japanese Rev. 1 Track 02 reaches the native title → stage → Soul Room → Akutuba runtime route. The US Continue path restores an authentic save and reaches dungeon 2, level 0; all seven dungeon sources load from authentic regional media. | The picker test covers keyboard and mouse handoff; the runtime test checks a loaded authentic Track 02 state. The US no-game M12 route and Japanese Track 02 SDL test exercise authentic startup/input paths. Tab champion cycling and Z/X/Space exercise keyboard actions on authentic USA Track 02; the Japanese Track 02 SDL test covers swipe-up movement and swipe-left turning. The authentic US raw-bin regression checks backward followed by KP8 forward to the source-backed starting square. An authentic USA Ak-Tu-Ba save-state replay under standard PCE confirmed Button I queues `$50` at `$2905` and the guest consumes it; Button II did not. The save-state capture has no CD-to-RAM provenance and does not bind that dispatch to a world-state change, so it proves command-queue dispatch only. Separate bounded US reference cold starts reached only the System Card; they do not establish a game-owned menu transition. | Runtime state, native frames and bounded input routes are reached, but the native dungeon image is not yet driven by the authenticated world/graphics state. A recent nonblack Firestaff capture has not been paired with an original-emulator capture. Later progression, full save semantics, audio and end-to-end parity remain open. |
| Nexus | Direct `--game nexus` reaches the title with `levelLoaded=0`. Authentic CUE-only JP retail media now reaches the same capture-gated title through direct CLI and pointer-menu handoff. From the M11 retail menu, authentic New Game input parses hash-verified `DM.BIN`, loads hash-verified `LEV01.DGN`, validates `(29,47,North)`, and emits a level-start receipt. The viewport/HUD handoff still fails closed and restores the title, so this is not a playable runtime. Firestaff renders source-bound English and Japanese New Game/Continue foreground sprites and selected Configuration sprites from authentic `TITLE.BIN`; Japanese Configuration mouse selection is verified. The shared title background and full-screen composition remain unbound. | Authentic Ymir cold boots reach the Japanese and English three-choice menu; a separate Japanese retail replay reaches the native 3D corridor after New Game. These are reference captures, not Firestaff gameplay. Firestaff's M11 New Game route is verified to the loader receipt, but not to a presented viewport. Continue/save loading and Configuration destination behavior remain open. | **Not playable yet.** CLI and menu now admit the authentic CUE-only layout, but no Firestaff Nexus viewport, HUD, or playable gameplay route is verified. See [`TODO-nexus.md`](../TODO-nexus.md) and [`NEXUS_STRICT_FIDELITY_INVENTORY.md`](nexus/NEXUS_STRICT_FIDELITY_INVENTORY.md#startup). |

## What the existing checks prove

- [`tests/test_m12_main_menu_game_selection_real_media.sh`](../tests/test_m12_main_menu_game_selection_real_media.sh)
  verifies five-card keyboard/mouse selection and authentic-media handoff. Its
  success condition is a `game-handoff` receipt, not a loaded level.
  A 2026-10-27 TRV2 run against authentic media also verified the documented
  default `~/.firestaff/data/<game>` path for all five games: FM Towns DM1,
  CSB and DM2, Japanese Saturn Nexus, and PC Engine Theron. Each default-root
  case used an isolated HOME/config and checked that its original source file
  or archive remained unchanged. This proves discovery and menu handoff; it
  does not turn Nexus's capture-gated title route into a playable runtime.
  Duplicate first-row DM1, CSB and DM2 mouse handoffs were removed; the
  remaining five-card mouse matrix retains one center click for each card.
- `tests/test_m12_polished_ui_flow.c` now distinguishes pending deferred scans,
  active scans and missing media. Pending and active states render the
  scanning indicator, while a game remains launch-gated until its own scan
  receipt completes. This fixes the card's temporary status display; it does
  not turn a handoff into a playable start.
- The regular and sidebar game-card status now read the same launch gate as
  the modern status pill. A matched media version alone no longer paints the
  card as ready when startup proof, presentation or another launch requirement
  is still blocked. `TITLE START AVAILABLE` means the bounded title handoff is
  admitted; it does not claim that the game's menu or gameplay is ready.
- [`tests/test_m12_main_menu_runtime_real_media.sh`](../tests/test_m12_main_menu_runtime_real_media.sh)
  exercises first-runtime probes for DM1, CSB, DM2, and Theron. The verifier
  requires an active runtime and `levelLoaded=1`; Nexus is not included.
- [`tests/test_dm2_v1_loose_owner_real_media.sh`](../tests/test_dm2_v1_loose_owner_real_media.sh)
  verifies that choosing the authentic French DOS loose files from the main
  menu retains the selected French graphics hash through first runtime,
  despite an English graphics file in the same directory.
- [`tests/test_theron_v1_sdl_input_events.sh`](../tests/test_theron_v1_sdl_input_events.sh)
  sends Tab, Z and Space through SDL key-down events after authentic USA Track
  02 startup and checks champion cycling, Button I/II keyboard routing and
  gameplay ticks. It does not establish full game input or renderer parity.
- [`tests/test_theron_v1_jp_sdl_input_events.sh`](../tests/test_theron_v1_jp_sdl_input_events.sh)
  sends authentic Japanese Track 02 swipe-up and swipe-left events through the
  SDL queue and checks movement and turning. The test does not establish the
  complete touch-control map or state-driven renderer parity.
- The pointer-trace hook now identifies the complete `JSR $C1DF` opcode and
  operands without a caller-PC range. Its regression test inspects the
  instrumenter's generated condition and rejects missing instruction bytes or
  added predicates. A bounded authentic JP save-state repeat recorded calls at
  `$B2B1`, `$B37A`, and `$B437`, but no CD IRQ, transition, or command RAM
  writes. The trace still does not bind these calls to a source LBA, usable
  world-hit rectangles, or command dispatch; it adds diagnostic coverage, not
  native pointer support.
- The `m11_dm1_hoc_stair_torch_pc34` real-media test now sends the ordinary
  M11 backward-movement input from authentic PC34 HoC corridor cell (4,15)
  toward the original stairs at (3,15). The test target compiled and linked in
  GitHub Actions on Linux, macOS and Windows, then passed a local run against
  the installed original `Dungeon-Master_DOS_EN.zip`: backward input entered
  the authentic stairs and next level with the original torch carried and lit.
  This shows that the tested M11 route completes; the native-window hang report
  remains open until its event-loop/frame-presentation path is reproduced.
- A fresh current-main TRV2 build at `e3b6d015c` succeeded, then direct CLI
  boot probes against the authentic per-game `.firestaff/data` roots reached
  `runtime-entered` for DM1, CSB, DM2 (FM Towns) and Theron (PC Engine).
  Nexus reached the Saturn title and retained its explicit blocked receipt
  (`menu=0`, `levelLoaded=0`); it did not enter gameplay. The real-media M12
  menu selection script passed keyboard and mouse selection/handoff for all
  five cards, and default-root launches for DM1/CSB/DM2. Those five-card
  handoffs are not evidence of playable Nexus or Theron runtimes. The broad
  CLI edition matrix skipped with code 77 because TRV2 lacks its required
  authentic DM2 Macintosh archive. No synthetic or copied media was used.
- The published v3.0.414 macOS arm64 app was also exercised against the
  installed authentic media. `test_m12_main_menu_game_selection_real_media.sh`
  passed keyboard and mouse selection/handoff for all five cards and verified
  the default `.firestaff/data` menu route for DM1, CSB and DM2. The separate
  `test_nexus_v1_native_menu_mouse_title.sh` passed and confirmed that Nexus
  reaches the capture-gated title route, not a playable menu. Direct CLI
  `--boot-probe-frames 0` runs reported `runtime-entered` for DM1, CSB, DM2 and
  Theron, but only DM1 had `levelLoaded=1` at that zero-frame snapshot; the
  other three were still in their startup/title states. Nexus reported
  `title=1`, `levelLoaded=0`, `menu=0`, and
  `menu-bpk-prs3-capture-required`. Thus the v3.0.414 runtime evidence
  confirms all five launcher card selections, not playable starts for all five
  games. v3.0.415 did not add a new game-start run.
- `test_m12_main_menu_runtime_real_media.sh` then passed against the same
  v3.0.414 app and authentic collection for DM1, CSB, DM2 and Theron. Its probe
  requires an active runtime with `levelLoaded=1`; DM1 also had its HoC first
  frame, CSB its source party and viewport receipt, DM2 its completed Towns
  title and first party, and Theron its Track 02 runtime/party. The runtime
  test has no Nexus case; Nexus remains the known playable-start blocker.
- A fresh 2026-10-26 TRV2 run rebuilt main at `ed2ff82a` and repeated
  `test_m12_main_menu_runtime_real_media.sh` against the authentic installed
  collection. Its `levelLoaded=1` verifier passed the no-game menu routes for
  DM1, CSB, DM2 (FM Towns), and Theron (PC Engine). Separate zero-frame direct
  CLI probes against the same real data entered DM1's loaded runtime
  immediately, but sampled CSB, DM2, and Theron before their startup/title
  transitions completed. Nexus still reported
  `title=1`, `menu=0`, `levelLoaded=0`, and
  `menu-bpk-prs3-capture-required`. The runtime script omits Nexus, so these
  four passes do not close the five-game startup gap.
- A fresh 2026-10-27 TRV2 v3.0.415 build used authentic Japanese Nexus data.
  Direct `--game nexus --platform saturn` startup selected the hash-matched
  `DM.BIN` (MD5 `e88d60859f65f08fa622e1992b02280f`); after 110 boot-probe
  frames and one scripted Enter it reported `titleReady=1` but
  `levelLoaded=0`, `menu=0`, and
  `menu-bpk-prs3-capture-required`. The real-data
  `test_nexus_v1_native_menu_mouse_title.sh` passed and confirmed the M12
  mouse route reaches the same capture-gated title. These runs verify title
  handoff only; no Nexus menu or playable level was reached.
- The Theron Button II dispatch and non-advancing cancel behavior have
  source-backed coverage in the active runtime input and SDL event-loop checks.
  This closes that input-path defect; it does not establish full input or
  renderer parity.
- `tests/test_theron_v1_raw_bin_runtime_boot.sh` checks the authentic US Track
  02 route `down,key:kp8` as one backward step followed by one forward step,
  including both action receipts and the returned position. This corrects a
  prior expected-coordinate error; it does not add movement behavior or close
  the native viewport gap.
- [`tests/test_firestaff_cli_startup_diagnostics_real_media.sh`](../tests/test_firestaff_cli_startup_diagnostics_real_media.sh)
  checks discovery and direct CLI startup for all five IDs. Its Nexus-specific
  assertion rejects a false `runtime-entered` receipt and requires the blocked
  menu/level gate, so a passing test is not evidence that Nexus is playable.
- [`docs/nexus/nexus_startup.md`](nexus/nexus_startup.md) records authentic Ymir
  title, cold-boot, menu, load-row, and loaded-HUD observations. Current
  Firestaff code renders source-bound English/Japanese title-menu foregrounds
  from authentic `TITLE.BIN` and handles bounded row selection, but the shared
  background, complete composition, destination dispatch, Continue/save
  loading, and level startup remain open. These partial renderer results do
  not satisfy the `MENU.BPK` ownership gate or establish a playable level.
- These tests use SDL dummy video/audio and authentic installed media. They do
  not establish native window presentation, physical input, audible output,
  complete game playthroughs, or cross-platform gameplay parity.

## Launcher scan responsiveness

An earlier full-media profile on one Mac16,10 host scanned the installed
425-file / 4.3 GiB collection in 31.13 seconds cold and 11.91 seconds warm
with an isolated user cache. A six-second sample attributed substantial
work to CSB Amiga 3.1 package admission, repeated ZIP inflation, nested ADF
traversal, and later edition matching. Current main reuses a previously
hash-verified graphics match only after the same-container program package
passes; Amiga 3.1 English also has to pass its startup-media gate, while
incomplete candidates retain exhaustive fallback. On the same host, a later
current-source scan took 27.86 seconds cold and three warm runs had a median of
8.57 seconds. The focused authentic-media CTests for Amiga 3.5 English and the
Amiga 3.1 menu-to-runtime ZIP route passed. The observed reductions are 10.5%
cold and 28.0% warm, but uncontrolled OS file-cache state, one host, and one
prior timing sample prevent treating them as a general speedup. Full details
and limits are recorded in [`TODO.md`](../TODO.md).

A 2026-10-26 Debug scan with an isolated HOME/config took 26.84 seconds and
reported all five games READY. The standard scan explicitly skipped installed
`.7z` and `.rar` archives because external archive tools were not enabled, so
READY means the required media was found among scanned sources; it does not
mean every archive in the collection was inventoried. The OS file cache was
not controlled, so this is a single observation, not a cold-scan comparison.

The larger cross-game candidate-index opportunity remains open:
`m12_fill_game_versions` queries each game's edition hashes separately for
each root, and scan-cache hits do not avoid all directory traversal or
container admission. Any shared index must retain every edition result,
duplicate-file handling, complete/incomplete package admission, same-container
ownership and cancellation semantics. Preserve those invariants and the
reported launcher delay until a representative multi-edition profile proves a
safe reduction. Current main also removes a duplicate direct-root `file_md5`
call during path collection; the translation unit compiles for both M12 and
Nexus, but its runtime impact has not been measured. A current-main TRV2 scan
against authentic media confirms all five required profiles are found, but
skips some external-only archives. Two same-source warm A/B attempts to remove
redundant hash work produced identical reports and no measurable speedup; the
details and limits are recorded in [`TODO.md`](../TODO.md). See the version
pass in
[`asset_status_m12.c`](../src/shared/asset_status_m12.c) and traversal/cache
logic in [`asset_find_by_hash.c`](../src/shared/asset_find_by_hash.c).

The generic fast-candidate pass writes to output arrays that the following
`asset_find_all_by_md5_list` call clears on entry, so it currently warms the
metadata-validated file cache but does not seed the published inventory. Its
configured paths are within the recursive walk's depth limit, which follows
directory links via `stat()`. A same-source warm A/B scan showed no measurable
benefit from removing the pre-pass, so it remains in place pending a broader
profile that can justify changes to candidate ordering or cross-game indexing.

The published v3.0.414 macOS arm64 app scanned the installed authentic
collection from an isolated HOME/config in 30.08 seconds on the first run; two
warm repeats took 9.92 and 9.84 seconds (9.88-second median). All five games
were READY. Fourteen `.7z`/`.rar` candidates were explicitly skipped because
external archive tools were disabled. The OS file cache was not controlled,
and scan completion time is not a measurement of time to first menu input.
This remains a one-host profile, not evidence of a general speedup or full
archive inventory.

## Targeted authentic CLI and M12 verification (2026-10-27)

The v3.0.417 macOS arm64 executable was exercised with authentic archives from
the external Firestaff data collection. Tests used an isolated data root
containing only DM1, CSB, and DM2, with original archives reached through
symlinks. The picker test verified that source archive hashes stayed unchanged.

- `test_m12_main_menu_runtime_real_media.sh`, selected separately for DM1,
  CSB, and DM2, passed all three no-game M12 routes to the first loaded
  FM Towns runtime. The verifier requires an active game and `levelLoaded=1`.
- `test_m12_main_menu_game_selection_real_media.sh` passed keyboard and mouse
  selection for DM1, CSB, and DM2. Authentic menu handoffs were observed for
  DM1 FM Towns, PC-34, Atari ST, and Amiga; CSB FM Towns, Atari ST, and Amiga;
  and DM2 FM Towns and PC. This is edition-selection/handoff evidence, not a
  first-runtime receipt for every listed edition. The test reported the
  Nexus and Theron source archives unavailable in this isolated root.
- `test_dm1_v1_atari_st_archive_cli_boot.sh` passed against authentic English
  Atari ST v1.2 media. Direct startup and the M12 route reached the first HoC
  hall; the test confirmed the original C127 choice and passed its native
  input matrix.
- `test_dm1_v1_atari_st_11_archive_cli_boot.sh` passed against the authentic
  v1.1 STX member from the original software archive. Direct STX CLI and M12
  retained the selected disk owner, reached the first HoC hall, and confirmed
  the source C127 choice with live input.
- `test_dm1_v1_amiga36_original_media_cli_boot.sh` passed against authentic
  Amiga 3.6 media. Direct CLI and M12 reached the first runtime pose; English,
  French, and German dungeon remaps, the source C696 zones, 64 door steps, and
  fifteen native driver palette steps were verified. Original emulator frame
  alignment, audible output, and full viewport parity remain unverified.
- `test_dm1_v1_pc34_native_cli_boot.sh` passed against the authentic PC-34
  archive. Direct CLI, no-platform CLI, and M12 reached the loaded runtime;
  the test covered the 53-frame title, 32 selector-door steps at 70 Hz, the
  fresh-input selector, Resume/Credits/Quit routes, HoC choice, and the native
  input matrix. Its debug checks also verified FPS/CPU reporting and explicit
  GPU-availability reporting. These startup results do not prove full dungeon
  viewport or gameplay parity.
- `test_dm1_v1_fmtowns_archive_cli_boot.sh` passed against authentic English
  and Japanese FM Towns media. The M12 route reached the first runtime; title
  palette, 60 Hz timing, source-owned entrance cadence, and TMENU/EDM and
  TMENU/JDM handoffs with their native input matrices were verified.
- `test_firestaff_iigs_cli_real_media.sh` passed against authenticated original
  Apple IIGS 2.0 and 2.1 disk members. Both direct CLI and M12 routes reached
  active dungeon runtime; original title and mouse-gated entrance behavior
  were checked. GS F0128 pixel parity and GS F0129 remain unverified.
- `test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh direct-exit` passed
  for direct `--game dm1`, `--game csb`, and `--game dm2` launches. Each route
  reached its runtime handoff, accepted Return to Menu, then exited without
  reopening M12 or rescanning. The individual platform routes were DM1 PC-34,
  CSB Atari ST, and DM2 FM Towns in the separate real-media probes; the
  matrix's own direct-exit cases use FM Towns for all three.
- Separate direct `--game` boot probes omitted `--platform` for all three
  priority games and selected their authentic FM Towns editions, confirming
  the default-platform rule on the no-platform CLI path.
- `test_dm2_unsupported_platform_cli.sh` passed the DM1/CSB/DM2 direct and
  menu rejection matrix, including removed PC-9821 aliases, before any scan or
  startup began.
- CSB's authentic Atari STX, Amiga 3.3 loose media plus FTL Amiga archive, and
  FM Towns ZIP suites passed their native CLI/M12 startup checks. Atari reached
  the source C200 transition; Amiga reached A33M and FTL A31E runtime with the
  source input matrix; FM Towns reached the authentic `MINI.DAT` party through
  M12 and preserved selected loose-file ownership. These checks do not prove
  complete cross-platform viewport parity.
- DM2's authentic DOS ZIP suite passed the visible startup-menu, New Game,
  runtime viewport/floor/HUD and AUTO-discovery checks. Its Japanese FM Towns
  ZIP suite passed bare `--game dm2` default selection, New Game and first-
  champion runtime, with a 320x200 runtime capture. The authentic French
  Macintosh `.sit` suite reached the first real-asset runtime and confirmed
  the source files remained byte-identical.
- At the time of the macOS v3.0.417 run, the expected DM2 Amiga ZIP was a
  broken self-referencing symlink in that test's data directory, and no
  original Amiga archive was present on its external media. A later TRV2
  current-main run used the installed original ZIP: `dm2_v1_amiga_native_cli_boot`
  passed the direct CLI and pointer-menu routes, while the focused M12 runtime
  test reached level 0 with real assets. The Amiga archive remained unchanged.

These checks improve confidence in the targeted launcher routes, but they do
not establish complete startup parity. Remaining work includes first-runtime
verification for every selectable edition, native display and physical-input
paths, saved-game routes, and complete game sessions. No Nexus or Theron game
was intentionally launched in the isolated rerun.

### 2026-10-29 post-merge DM1/CSB/DM2 startup retest

On main at `737508d0`, the authentic-media no-game M12 first-runtime test
passed separately for DM1, CSB, and DM2 using their installed FM Towns
editions. The four alternate-platform M12-to-runtime routes passed as well:
DM1 PC, DM1 Atari ST, CSB Atari ST, and DM2 PC-DOS. The CSB Atari route found
the original Utility disk archive in `data/csb`, completed its startup
animation, and reached an active runtime with a nonzero viewport receipt and
one movement step. Its animation asset unit, real-media, and discovery tests
passed 3/3. The authentic DM1 Apple IIGS 2.0 and 2.1 CLI/M12 test also passed
for both versions and reached active dungeon runtime. These runs used SDL's
dummy video/audio drivers and do not verify a physical display or input
device. They verify the listed start routes only; additional selectable
editions, saved-game routes, native presentation, and complete gameplay
remain open.

## Latest local real-media menu check (2026-10-27)

The Firestaff executable was rebuilt from the working tree based on source
snapshot `88ef19df`, then tested with the authentic installations under
`~/.firestaff/data/`. The M12 game-selection check passed both keyboard and
mouse selection for DM1, CSB, DM2, Nexus and Theron's Quest. It confirmed the
source-owned handoff for each selected installation; the test also verifies
that source archive hashes remain unchanged.

The separate M12 first-runtime test passed for DM1, CSB, DM2 and Theron US.
Theron JP passed its direct native selection route. These checks establish
bounded runtime entry for those named routes only. A separate scan reported
all five data families as `READY`, which is a media-recognition result and
does not make every route playable.

Nexus selected the authentic Saturn title route but remained blocked at
`title-vdp-owner-compositor-capture-required` (`titleReady=1`,
`levelLoaded=0`, `menu=0`).
The retail title/menu consumer and its palette/display ownership still lack a
source-bound capture receipt, so this result is expected and the launcher
must keep its menu/gameplay gate closed. No fallback or synthetic game data
was used. The checks ran with original media in place and left its archive
contents unchanged.

The installed-data investigation found an edition-isolation bug when
extracted Nexus resources and multiple regional Saturn images share a
directory. M12 had labeled the extracted `DM.BIN` as Japanese even though the
Japanese and English releases share that executable, then opened the English
CUE as supplemental media. This is now resolved for extracted JP/EN/FR owners:
M12 uses authenticated regional `RLOWFIX.BIN` plus at least two localized UI
resource hashes, and the engine admits only supplemental media with the
canonical shared `DM.BIN` and matching regional members. The authentic JP and
staged EN menu tests passed on TRV2; the EN case also rejected the authentic
Japanese CUE as supplemental media.
The title compositor gate remains closed. A source-bound CUE-only route is
now verified for direct CLI and pointer-menu handoff using authentic Japanese
retail media. A selected authentic DM2 CUE beside that Nexus disc is rejected
instead of falling back to the sibling. See [Nexus TODO](../TODO-nexus.md#2026-10-30--admit-source-bound-saturn-cuebin-media).

These results update the earlier isolated rerun above, which did not include
Nexus or Theron. They do not establish native window presentation, audible
output, full campaign playability or visual parity. Test scripts:
`tests/test_m12_main_menu_game_selection_real_media.sh` and
`tests/test_m12_main_menu_runtime_real_media.sh`.

## Remaining startup work

1. **Nexus:** join authentic Saturn title/menu presentation, input dispatch,
   New Game/Continue selection, and the first `LEV01.DGN` position into one
   source-bound runtime path. Until that evidence exists, retain the startup
   gate and do not count menu-card selection as game startup. See
   [`nexus_startup.md`](nexus/nexus_startup.md) and [`TODO-nexus.md`](../TODO-nexus.md).
2. **Theron:** bind the authenticated VDC/VRAM/palette data to a state-driven
   dungeon renderer, then verify the first presented runtime frame and normal
   input against authentic US and Japanese Track 02 data. See
   [`TODO-theron.md`](../TODO-theron.md).
3. **All games:** extend bounded first-runtime receipts across the remaining
   supported editions and native display/input paths. Keep media discovery,
   CLI selection, menu selection, M11 handoff, first runtime, and complete
   gameplay as separately reported milestones.
