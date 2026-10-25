# Five-game startup support gap analysis

Reviewed against `origin/main` at `e3b6d015c` on 2026-10-25. The repository
contains authentic-media route checks for all five games, but “available,”
“selected,” “handed off,” and “entered a playable runtime” are separate states.
The current evidence does not establish complete five-game startup support.

## Route matrix

| Game | Direct CLI evidence | Main-menu evidence | Current startup result |
|---|---|---|---|
| DM1 | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the runtime test checks an authentic first runtime frame. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| CSB | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic starting party and viewport receipt. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| DM2 | The authentic-media CLI diagnostics cover direct selection; platform-specific checks include the complete timed FM Towns title sequence. The French DOS loose-file route now also preserves its selected graphics owner into the first loaded runtime. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic first party and frame. | First runtime is verified on bounded real-media routes, including the selected French DOS loose-file case. Other edition and native interaction coverage remains incomplete. |
| Theron | The CLI diagnostics reach its source-owned runtime handoff. | The picker test covers keyboard and mouse handoff; the runtime test checks a loaded authentic Track 02 state. Tab champion cycling and the documented Z/Space wait keys now exercise the normal SDL event loop on authentic USA Track 02. | Runtime state, native frames and these keyboard actions are reached, but the native dungeon image is not yet driven by the authenticated world/graphics state. A recent nonblack Firestaff capture has not been paired with an original-emulator capture. |
| Nexus | The CLI diagnostics intentionally expect the title frame and a blocked startup receipt: `levelLoaded=0`, `menu=0`, blocker `menu-bpk-prs3-capture-required`. | Keyboard and mouse selection tests reach the M12-to-M11 handoff, not an active game runtime. There is no Nexus case in the first-runtime probe matrix. | **Not startable as a playable game yet.** A new Ymir title-menu VDP capture documents `NEW GAME`, `CONTINUE`, and `CONFIGURATION`, but Firestaff's production title/menu consumer and transition to an authenticated level remain gated. The Saturn BKR parser is container-level only and is not wired to save selection or loading. |

## What the existing checks prove

- [`tests/test_m12_main_menu_game_selection_real_media.sh`](../tests/test_m12_main_menu_game_selection_real_media.sh)
  verifies five-card keyboard/mouse selection and authentic-media handoff. Its
  success condition is a `game-handoff` receipt, not a loaded level.
- [`tests/test_m12_main_menu_runtime_real_media.sh`](../tests/test_m12_main_menu_runtime_real_media.sh)
  exercises first-runtime probes for DM1, CSB, DM2, and Theron. The verifier
  requires an active runtime and `levelLoaded=1`; Nexus is not included.
- [`tests/test_dm2_v1_loose_owner_real_media.sh`](../tests/test_dm2_v1_loose_owner_real_media.sh)
  verifies that choosing the authentic French DOS loose files from the main
  menu retains the selected French graphics hash through first runtime,
  despite an English graphics file in the same directory.
- [`tests/test_theron_v1_sdl_input_events.sh`](../tests/test_theron_v1_sdl_input_events.sh)
  sends Tab, Z and Space through SDL key-down events after authentic USA Track
  02 startup and checks champion cycling and gameplay ticks. It does not
  establish full game input or renderer parity.
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
- PR #74 (`feat(theron): route Button II keyboard binding`) remains open and
  should not be merged on mapper tests alone. The inactive startup dispatcher
  lacks Z/X routing, and the loaded-dungeon Theron consumer reports
  `UNKNOWN INPUT` for the mapped Back token. Require live SDL coverage for
  startup and dungeon paths before treating these bindings as supported.
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

The latest recorded full-media scan took 26.41 seconds on one Mac16,10 host
with an isolated user cache; the OS file-cache state was uncontrolled. It used
the installed 425-file / 4.3 GiB collection, found all five games READY, and
reported about 637 MB maximum resident memory. A five-second source sample
identified `m12_fill_game_versions` → `asset_find_all_by_md5_list` →
`scan_dir_by_md5_list`, including nested Amiga ADF/CAPS/MFM decoding. This
single measurement is not directly comparable to prior cold/warm samples and
does not establish a cross-platform speedup.

The source shows one bounded optimization candidate: `m12_fill_game_versions`
queries each game's edition hashes separately for each root. The scan cache
avoids rehashing unchanged physical files, but the version passes may still
repeat directory traversal and container admission. A shared candidate index
could reduce that work, but must retain every edition result, duplicate-file
handling, complete/incomplete package admission, same-container ownership and
cancellation semantics. Until those invariants are carried through an
implementation, keep the repeated scans and report the measured launcher
delay. See the scan profiles in [`TODO.md`](../TODO.md), the version pass in
[`asset_status_m12.c`](../src/shared/asset_status_m12.c), and traversal/cache
logic in [`asset_find_by_hash.c`](../src/shared/asset_find_by_hash.c).

One detail needs measurement before simplifying the current fast-candidate
pass: it writes candidate paths into the same output arrays that the following
`asset_find_all_by_md5_list` call clears on entry. The candidate hashes still
warm the metadata-validated file cache, while the recursive pass supplies the
published inventory. Removing that pass may reduce redundant probes, but its
value for named files that normal enumeration cannot reach has not been
measured. Preserve that route or provide equivalent evidence before removing
it.

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
