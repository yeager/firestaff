# Five-game startup support gap analysis

Reviewed against `origin/main` at `a42988705` on 2026-10-25. The repository
contains authentic-media route checks for all five games, but “available,”
“selected,” “handed off,” and “entered a playable runtime” are separate states.
The current evidence does not establish complete five-game startup support.

## Route matrix

| Game | Direct CLI evidence | Main-menu evidence | Current startup result |
|---|---|---|---|
| DM1 | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the runtime test checks an authentic first runtime frame. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| CSB | The authentic-media CLI diagnostics exercise direct selection and source-owned startup. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic starting party and viewport receipt. | First runtime is verified on bounded real-media routes. Platform-wide parity and native physical input remain open. |
| DM2 | The authentic-media CLI diagnostics cover direct selection; platform-specific checks include the complete timed FM Towns title sequence. | The picker test covers keyboard and mouse handoff; the runtime test checks the authentic first party and frame. | First runtime is verified on bounded real-media routes. Other edition and native interaction coverage remains incomplete. |
| Theron | The CLI diagnostics reach its source-owned runtime handoff. | The picker test covers keyboard and mouse handoff; the runtime test checks a loaded authentic Track 02 state. | Runtime state and native frames are reached, but the native dungeon image is not yet driven by the authenticated world/graphics state. A recent nonblack Firestaff capture has not been paired with an original-emulator capture. |
| Nexus | The CLI diagnostics intentionally expect the title frame and a blocked startup receipt: `levelLoaded=0`, `menu=0`, blocker `menu-bpk-prs3-capture-required`. | Keyboard and mouse selection tests reach the M12-to-M11 handoff, not an active game runtime. There is no Nexus case in the first-runtime probe matrix. | **Not startable as a playable game yet.** A new Ymir title-menu VDP capture documents `NEW GAME`, `CONTINUE`, and `CONFIGURATION`, but Firestaff's production title/menu consumer and transition to an authenticated level remain gated. The Saturn BKR parser is container-level only and is not wired to save selection or loading. |

## What the existing checks prove

- [`tests/test_m12_main_menu_game_selection_real_media.sh`](../tests/test_m12_main_menu_game_selection_real_media.sh)
  verifies five-card keyboard/mouse selection and authentic-media handoff. Its
  success condition is a `game-handoff` receipt, not a loaded level.
- [`tests/test_m12_main_menu_runtime_real_media.sh`](../tests/test_m12_main_menu_runtime_real_media.sh)
  exercises first-runtime probes for DM1, CSB, DM2, and Theron. The verifier
  requires an active runtime and `levelLoaded=1`; Nexus is not included.
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
