# Five-game startup support gap analysis

Reconciled with repository `main` at `372a19e2` on 2026-10-27. The latest
published release is v3.0.417 (published at 06:59 UTC). GitHub Actions release
run `40463344704` completed its seven platform build and packaging jobs
successfully (Windows x86_64, macOS x86_64/arm64, Linux x86_64/arm64, Android
arm64 and iOS arm64), then published 23 release assets. These results verify
compilation and packaging, not game startup. Recent changes add authentic
Japanese Theron input evidence and Nexus retail reference captures. The Nexus
captures verify a no-save route to the native three-choice menu and a separate
diagnostic replay to the native 3D corridor; they do not establish Firestaff's
Nexus startup or renderer. A separate English retail boot capture reached
`0DMSTRT.BIN` but ended after a CD Block soft reset, without a `DM.BIN` read or
a complete VDP1 command list. It confirms startup-file access and menu-surface residency, but not menu
display or Firestaff startup. Runtime receipts cited below remain
tied to their stated revisions unless noted otherwise.
The latest Nexus evidence separates raw Japanese `DM.BIN` placement at
`0x06010000` from the older trace-specific static code mapping at
`0x06010040`. The English retail `MENU.BPK` now has an authenticated hash and
is accepted by the regional snapshot analyzer. In the same-session English
capture, 86 of 162 surfaces have direct byte matches in VDP1 VRAM, but none of
132 textured commands starts inside those matched ranges; the captured command
list is incomplete. This establishes asset residency, not menu display or
source/CLUT ownership, so the English renderer gate remains closed.
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

## Route matrix

| Game | Direct CLI evidence | Main-menu evidence | Current startup result |
|---|---|---|---|
| DM1 | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the mixed-media AUTO route enters through the no-game menu and reaches its authentic first runtime frame. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| CSB | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic starting party and viewport receipt. | First runtime is verified on bounded real-media routes. In Amiga CSB, the shared F0107 ornament planner still applies PC34 placement rules; the missing work is native Amiga coordinate mapping and matched-frame verification, not blitter scaling, flip, palette mapping or transparency ([authentic evidence and remaining gap](../TODO-csb.md)). Platform-wide parity and native physical input remain open. |
| DM2 | The authentic-media CLI diagnostics cover direct selection; platform-specific checks include the complete timed FM Towns title sequence. The French DOS loose-file route now also preserves its selected graphics owner into the first loaded runtime. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic first party and frame. | First runtime is verified on bounded real-media routes, including the selected French DOS loose-file case. Other edition and native interaction coverage remains incomplete. |
| Theron | The CLI diagnostics reach its source-owned runtime handoff. | The picker test covers keyboard and mouse handoff; the runtime test checks a loaded authentic Track 02 state. Tab champion cycling and Z/X/Space exercise keyboard actions on authentic USA Track 02; the Japanese Track 02 SDL test covers swipe-up movement and swipe-left turning. The authentic US raw-bin regression checks backward followed by KP8 forward to the source-backed starting square. An authentic USA Ak-Tu-Ba save-state replay under standard PCE confirmed Button I queues `$50` at `$2905` and the guest consumes it; Button II did not. The save-state capture has no CD-to-RAM provenance and does not bind that dispatch to a world-state change, so it proves command-queue dispatch only. Separate bounded US reference cold starts reached only the System Card; they do not establish a game-owned menu transition. | Runtime state, native frames and these input routes are reached, but the native dungeon image is not yet driven by the authenticated world/graphics state. A recent nonblack Firestaff capture has not been paired with an original-emulator capture. |
| Nexus | A historical TRV2 v3.0.415 CLI run used authentic Japanese `DM.BIN` (MD5 `e88d60859f65f08fa622e1992b02280f`): after 110 boot-probe frames and one scripted Enter, it reported `titleReady=1`, `levelLoaded=0`, `menu=0`, blocker `menu-bpk-prs3-capture-required`. Current source has advanced: the real English `MENU.BPK` byte decoder verifies all 162 PRS3 surfaces, but keeps decoded pixels out of the renderer until Saturn palette, VDP1 upload and display ownership are joined to a matching capture. The authentic English Ymir capture logs `0DMSTRT.BIN`, then a CD Block soft reset; it has no `DM.BIN` read and no complete VDP1 command list. Of 162 authentic English menu surfaces, 86 have direct byte matches in VDP1 VRAM, but none of 132 textured command starts falls within those ranges. This establishes residency only. The authentic-data mouse test still reaches only the capture-gated title route, and no Nexus case exists in the first-runtime probe matrix. | An authentic Japanese retail Ymir cold boot with no emulator or game save reaches the native three-choice title menu after scheduled Start inputs. No-input, Down/Continue, and Configuration behavior are separately verified from diagnostic state 3. From that same no-save menu state, a separate replay with `NEW GAME` selected and a two-frame A input reaches the native 3D corridor viewport after 1,200 frames; the complete VDP1 trace has zero drops, and the capture has no party HUD. These are retail emulator checkpoints, not Firestaff startup or presentation evidence. Firestaff's title composition, menu rendering/control, Continue with real save data, stable gameplay, HUD and viewport parity remain unverified. Japanese capture evidence does not satisfy the source-bound English renderer gate. The Saturn BKR parser remains container-level and is not wired to save selection or loading ([English boot attempt](nexus_startup.md#english-retail-boot-attempt-and-menu-asset-residency-2026-10-27), [cold-boot retail menu and navigation](nexus_startup.md#cold-boot-title-menu-and-retail-menu-navigation-2026-10-27), [New Game viewport checkpoint](nexus_startup.md#new-game-reaches-the-native-3d-viewport-2026-10-27), [SCU-to-VDP1 correlation](nexus_startup.md#scu-dma-to-vdp1-frame-correlation-2026-10-27), [retail Continue evidence](nexus_startup.md#retail-continue-with-an-authentic-save-2026-10-25), [current decoder and strict boundary](NEXUS_STRICT_FIDELITY_INVENTORY.md#startup)). | **Not startable as a playable game yet.** |

## What the existing checks prove

- [`tests/test_m12_main_menu_game_selection_real_media.sh`](../tests/test_m12_main_menu_game_selection_real_media.sh)
  verifies five-card keyboard/mouse selection and authentic-media handoff. Its
  success condition is a `game-handoff` receipt, not a loaded level.
  Duplicate first-row DM1, CSB and DM2 mouse handoffs were removed; the
  remaining five-card mouse matrix retains one center click for each card.
- `tests/test_m12_polished_ui_flow.c` now distinguishes pending deferred scans,
  active scans and missing media. Pending and active states render the
  scanning indicator, while a game remains launch-gated until its own scan
  receipt completes. This fixes the card's temporary status display; it does
  not turn a handoff into a playable start.
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
- [`docs/nexus_startup.md`](nexus_startup.md) records authentic Ymir title,
  cold-boot, title, menu, load-row, and loaded-HUD observations from Japanese
  retail media. A direct cold-boot capture now reaches the native title menu;
  separate saved-state runs characterize title-menu and gameplay input. These
  do not prove that Firestaff renders or dispatches the menu. They do not
  satisfy the English `MENU.BPK` source-binding gate or establish Firestaff
  save selection/loading or level startup.
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

## Remaining startup work

1. **Nexus:** join authentic Saturn title/menu presentation, input dispatch,
   New Game/Continue selection, and the first `LEV01.DGN` position into one
   source-bound runtime path. Until that evidence exists, retain the startup
   gate and do not count menu-card selection as game startup. See
   [`nexus_startup.md`](nexus_startup.md) and [`TODO-nexus.md`](../TODO-nexus.md).
2. **Theron:** bind the authenticated VDC/VRAM/palette data to a state-driven
   dungeon renderer, then verify the first presented runtime frame and normal
   input against authentic US and Japanese Track 02 data. See
   [`TODO-theron.md`](../TODO-theron.md).
3. **All games:** extend bounded first-runtime receipts across the remaining
   supported editions and native display/input paths. Keep media discovery,
   CLI selection, menu selection, M11 handoff, first runtime, and complete
   gameplay as separately reported milestones.
