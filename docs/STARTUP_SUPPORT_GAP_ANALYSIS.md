# Five-game startup support gap analysis

Reconciled with published `main` at `5d612f62a` (Firestaff v3.0.415) on
2026-10-26. Its GitHub Actions release run `40325408361` completed the seven
platform build and packaging jobs successfully (Windows x86_64, macOS
x86_64/arm64, Linux x86_64/arm64, Android arm64 and iOS arm64), then published
23 release assets. These results verify compilation and packaging, not game
startup. The v3.0.415 release notes use the v3.0.414 runtime receipts; this
release did not run a new five-game startup matrix. Runtime receipts cited
below remain the recorded runs at their stated revisions; they were not rerun
during this documentation update.
The repository contains checks
against authentic media for all five games, but “available,”
“selected,” “handed off,” and “entered a playable runtime” are separate states.
The current evidence does not establish complete five-game startup support.

## Route matrix

| Game | Direct CLI evidence | Main-menu evidence | Current startup result |
|---|---|---|---|
| DM1 | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the mixed-media AUTO route enters through the no-game menu and reaches its authentic first runtime frame. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| CSB | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic starting party and viewport receipt. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| DM2 | The authentic-media CLI diagnostics cover direct selection; platform-specific checks include the complete timed FM Towns title sequence. The French DOS loose-file route now also preserves its selected graphics owner into the first loaded runtime. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic first party and frame. | First runtime is verified on bounded real-media routes, including the selected French DOS loose-file case. Other edition and native interaction coverage remains incomplete. |
| Theron | The CLI diagnostics reach its source-owned runtime handoff. | The picker test covers keyboard and mouse handoff; the runtime test checks a loaded authentic Track 02 state. Tab champion cycling and the documented Z/X/Space controls now exercise the normal SDL event loop on authentic USA Track 02. | Runtime state, native frames and these keyboard actions are reached, but the native dungeon image is not yet driven by the authenticated world/graphics state. A recent nonblack Firestaff capture has not been paired with an original-emulator capture. |
| Nexus | The latest recorded authentic-media CLI boot probe (v3.0.414) reports `title=1`, `levelLoaded=0`, `menu=0`, and blocker `menu-bpk-prs3-capture-required`. Keyboard and mouse selection tests reach the M12-to-M11 handoff, not an active game runtime. There is no Nexus case in the first-runtime probe matrix. | **Not startable as a playable game yet.** Retail Ymir captures document `NEW GAME`, `CONTINUE`, and `CONFIGURATION`, but Firestaff's production title/menu consumer and transition to an authenticated level remain gated. The 600-frame no-present Ymir replay reproduced the retail `Now Loading` image (SHA-256 `07a6f3a612ca75209cd6308852a7cf25b3d1f35943ce2f5774e720376ef7c61c`); this confirms only a retail loading checkpoint, not Firestaff menu or playable-runtime behavior ([capture evidence](nexus_startup.md#delayed-title-menu-confirm-observation-2026-10-26)). The Saturn BKR parser is container-level only and is not wired to save selection or loading. |

## What the existing checks prove

- [`tests/test_m12_main_menu_game_selection_real_media.sh`](../tests/test_m12_main_menu_game_selection_real_media.sh)
  verifies five-card keyboard/mouse selection and authentic-media handoff. Its
  success condition is a `game-handoff` receipt, not a loaded level.
  Duplicate first-row DM1, CSB and DM2 mouse handoffs were removed; the
  remaining five-card mouse matrix retains one center click for each card.
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
- The Theron Button II dispatch and non-advancing cancel behavior have
  source-backed coverage in the active runtime input and SDL event-loop checks.
  This closes that input-path defect; it does not establish full input or
  renderer parity.
- [`tests/test_firestaff_cli_startup_diagnostics_real_media.sh`](../tests/test_firestaff_cli_startup_diagnostics_real_media.sh)
  checks discovery and direct CLI startup for all five IDs. Its Nexus-specific
  assertion rejects a false `runtime-entered` receipt and requires the blocked
  menu/level gate, so a passing test is not evidence that Nexus is playable.
- [`nexus_v1_ymir_title_menu_vdp_bundle_20261025`](../parity-evidence/verification/nexus_v1_ymir_title_menu_vdp_bundle_20261025/README.md)
  records emulator evidence of the retail title menu. It explicitly does not
  satisfy the original-Saturn provenance gate or establish Firestaff menu
  rendering, input dispatch, or level startup.
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
