# ReDMCSB Firestaff complete audit

Updated: 2026-10-31

This is the DM1/CSB source-review contract for comparing ReDMCSB against
Firestaff. ReDMCSB is the primary functional reference for DM1 and CSB V1.
If Firestaff and ReDMCSB disagree without stronger original-runtime evidence,
Firestaff is treated as wrong. DM2 must be reviewed against skproject; DMWeb
and Greatstone provide supporting reference material for all three games.
A game-level result is complete only when every supported platform lane is
covered independently.

## Scope

The requested parity target covers all three games and every platform lane
Firestaff currently supports. The current documented runtime lanes are:

| Game | Supported platform lanes in Firestaff |
|---|---|
| DM1 | PC DOS, Atari ST, Amiga, FM Towns |
| CSB | Atari ST, Amiga, FM Towns |
| DM2 | DOS, Amiga, FM Towns, Macintosh |

These are separate implementation targets: evidence from one platform or
edition does not close another lane. DM1 and CSB behavior uses ReDMCSB as the
primary functional source; DM2 uses skproject. DMWeb and Greatstone are
supporting references for all three games, with claims tied to the relevant
game, edition, and asset identity.

## Platform coverage register

The repository currently documents selected runtime routes for these lanes.
Those routes are evidence of bounded operation, not proof that every source
function is implemented on that platform. A lane remains `PARTIAL` until its
source functions are mapped to Firestaff behavior and platform-specific gates.

| Game | Platform lane | Existing bounded-route evidence | Parity status |
|---|---|---|---|
| DM1 | PC DOS | `README.md` platform summary; `docs/parity/PARITY_MATRIX_DM1_V1.md` (PC DOS target) | PARTIAL |
| DM1 | Atari ST | `README.md` platform summary; `docs/parity/REDMCSB_DM1_CSB_AUDIT.md` | PARTIAL |
| DM1 | Amiga | `README.md` platform summary; `docs/parity/REDMCSB_DM1_CSB_AUDIT.md` | PARTIAL |
| DM1 | FM Towns | `README.md` platform summary; `docs/parity/REDMCSB_DM1_CSB_AUDIT.md` | PARTIAL |
| CSB | Atari ST | `docs/parity/PARITY_MATRIX_CSB_V1.md`, original-media route table | PARTIAL |
| CSB | Amiga | `docs/parity/PARITY_MATRIX_CSB_V1.md`, original-media route table | PARTIAL |
| CSB | FM Towns | `docs/parity/PARITY_MATRIX_CSB_V1.md`, original-media route table | PARTIAL |
| DM2 | DOS | `docs/parity/DM2_V1_REAL_MEDIA_RUNTIME_AUDIT.md`, DOS English/French rows | PARTIAL |
| DM2 | Amiga | `docs/parity/DM2_V1_REAL_MEDIA_RUNTIME_AUDIT.md`, Amiga row | PARTIAL |
| DM2 | FM Towns | `docs/parity/DM2_V1_REAL_MEDIA_RUNTIME_AUDIT.md`, FM Towns row | PARTIAL |
| DM2 | Macintosh | `README.md` Macintosh English route; Macintosh First Chapter demo remains explicitly unsupported | PARTIAL |

F0275 has one bounded DM1 PC34 M11 C080 behavior regression in this review:
successful non-head C017 removal and the linked-head exception. ReDMCSB's
`MOVESENS.C:1415-1427` compiles the unlink under MEDIA009/MEDIA529. MEDIA009
covers the supported S10-S21, G14/G20/G21, and A20-A22 editions; MEDIA529
includes I34, A31/A36, F31, and their supported language variants. MEDIA508
is the separate F20/X30/P20 set and does not contain I34. Thus C017 unlink is
source-applicable to the documented DM1/CSB lanes, while the current live M11
integration regression is PC34-only. Each other lane still needs runtime
integration and original-media evidence. CSB's bounded PC34 C011/C012 routes
are tracked separately and do not close its other platform lanes.

The audit below is the DM1/CSB source-function inventory, not a substitute for
the DM2 skproject inventory or the per-platform status register. Completion
claims must be made per game and per supported platform lane.

The source review covers behavior and presentation, not only existing
completion percentages:

| Lane | ReDMCSB source roots | Firestaff surface | Status |
|---|---|---|---|
| movement_input | COMMAND.C, CLIKMENU.C, MOVESENS.C, GAMELOOP.C, MOVESENS.C sensor sections, DUNGEON.C | dm1_v1_movement*, dm1_v1_input*, dm1_v1_collision*, dm1_v1_sensor* | active_audit |
| viewport_hud_render | DUNVIEW.C, DRAWVIEW.C, PANEL.C, CHAMDRAW.C, CHAMPION.C, PORTRAIT.C, TEXT.C, DEFS.H | dm1_v1_viewport*, dm1_v1_champion_panel*, dm1_v1_portrait*, dm1_v1_palette*, dm1_v1_screen* | active_audit |
| gameplay_systems | GROUP.C, PROJEXPL.C, CASTER.C, OBJECT.C, CHAMPION.C, TIMELINE.C, LOADSAVE.C, DUNGEON.C | dm1_v1_combat*, dm1_v1_creature*, dm1_v1_projectile*, dm1_v1_spell*, dm1_v1_object*, dm1_v1_inventory*, dm1_v1_save*, dm1_v1_food* | active_audit |
| frontend_route_capture | TITLE.C, ENTRANCE.C, MENU.C, MENUDRAW.C, COMMAND.C, CLIKVIEW.C, GAMELOOP.C, ENDGAME.C | dm1_v1_entrance*, dm1_v1_menu*, boot_*pc34*, V1-sensitive parts of main_loop_m11, m11_game_view, menu_startup_m12 | active_audit |

## Required outcome for every row

Each audited row must end in exactly one of these states:

| State | Meaning |
|---|---|
| MATCHED | Firestaff behavior is source-locked to ReDMCSB and has a passing gate. |
| FIXED | A ReDMCSB-proven deviation was corrected and gated in Firestaff. |
| KNOWN_DIFF | Firestaff intentionally differs, and the difference is unreachable in normal DM1 V1 parity play or otherwise justified by source/runtime evidence. |
| MISSING | ReDMCSB behavior exists, Firestaff does not implement it yet, and the gap is recorded with a focused blocker/gate. |
| BLOCKED | The audit cannot finish without missing original data, missing runtime capture, or a tool/environment issue. |

Rows cannot be counted as complete from internal consistency probes alone.
They need ReDMCSB source anchors and a Firestaff gate or a named blocker.

## Immediate high-risk gaps

These are known from the current source tree and must be retired or explicitly
bounded before DM1 V1 can be called complete:

| Gap | Evidence | Required action |
|---|---|---|
| Creature AI fidelity | `src/memory/memory_creature_ai_pc34_compat.c` lists all 27 DM1 creature profiles at FULL tier, while `F0804_CREATURE_Tick_Compat` retains a tier-0 fallback. | Do not report a broad profile-stub gap: compare each source behavior and platform profile against its gate; separately track any simplified or deferred mechanics and confirm whether any profile actually selects the fallback. |
| Magic/combat fidelity | `memory_magic_pc34_compat` implements a bounded PC 3.4 spell and defence path; its header records the historical stub that it replaced. | Compare the implemented path against ReDMCSB for each supported DM1/CSB platform and edition; list only source-confirmed missing semantics as `MISSING`. |
| Sensor execution unsupported types | Floor C10 group-split Steal-XP and deferred CLEAR/TOGGLE rotation have source-shaped M11 WALK_ON consumers. The F0248 wall-event route awards cell-specific C10 Steal-XP to the active leader and applies OnceOnly before dispatch. A bounded DM1 PC34 object add/remove sensor adapter now runs after successful M11 keyboard and rendered-pile pickup/drop chain changes, carrying the F0276 audible-sensor switch-sound receipt. M11 now also prints visible destination-square TextStrings in source chain order for accepted DM1 PC34 movement and first party placement, matching F0276's C1_TEXT_TYPE_MESSAGE path. A focused startup-placement regression confirms chain order and invisible-string suppression; ordinary movement integration, other mutation routes, wall-cell object branches, local effects, uncovered sensor-event paths, full platform coverage and original-media proof remain open. | Continue MOVESENS.C F0270/F0271/F0276 audit across every event path and platform; add ordinary movement integration coverage, cover remaining local-effect types, and keep original-media claims separate from source-level behavior. |
| Original overlay regression absent | docs/parity/COMPLETION_MATRIX.md gives DM1 V1 original_overlay_regression 0/10. | Keep source fixes separate from pixel claims until representative original overlay frames exist. |
| V1-visible shell leakage | Previous parity docs identify custom M11/M12 surfaces that must not decide original behavior. | Audit normal V1 route and keep invented/debug surfaces unreachable unless source-backed. |

## Cross-game completion blockers

The repository's current completion model remains explicitly partial for each
primary V1 target; its scores are summaries, not function inventories:

| Game | Current repository status | Concrete missing proof |
|---|---|---|
| DM1 V1 | 78%, `playable_verified_partial` in `parity-evidence/verification/firestaff_completion_matrix.json` | Original same-state overlay regression is 0/10; audio timing/overlap and wider per-edition functional coverage remain open. |
| CSB V1 | 43%, `runtime_boundary_verified_partial` in `docs/parity/PARITY_MATRIX_CSB_V1.md` | Broad real-data adventuring, save/reload, original overlays, and audio playback/timing remain open across Atari ST, Amiga, and FM Towns. |
| DM2 V1 | 36%, `runtime_slices_verified_partial` in `parity-evidence/verification/firestaff_completion_matrix.json` | The parity matrix is incomplete (4/10); renderer and original-overlay coverage are 3/20 and 0/10, and runtime wiring remains incomplete. |

These blockers must be decomposed into source-function rows and closed for each
supported platform lane. Do not infer platform parity from aggregate scores.

## Source-symbol inventory findings

The current callable-symbol audits expose concrete incomplete work. These are
triage inputs, not direct counts of missing gameplay features: platform APIs,
ABI aliases, unreferenced routines, and weak numeric matches must be separated
from reachable game behavior before an implementation claim is made.

| Reference inventory | Current row counts | Confirmed interpretation |
|---|---|---|
| ReDMCSB `docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv` | 2,133 total: 801 `MISSING`, 640 `UNCERTAIN_NUMBERED_EVIDENCE`, 520 `IMPLEMENTED_NARROW`, 97 `VERIFIED_SOURCE_MAPPING`, 64 `SOURCE_NONAPPLICABLE`, 5 `PC34_SOURCE_IMPLEMENTED`, 6 `PARTIAL` | Not yet a complete DM1/CSB game-function inventory. The symbol backlog contains 1,032 open rows after existing dispositions; review reachable gameplay/UI functions per platform and explicitly classify OS/ABI-only routines. FIO1 F1321 and F1324 now map to their tested PC34 bounded path-substitution and source no-op contracts; Amiga ABI/device semantics remain outside those adapters. ReDMCSB F0906/F0907 now map to an Amiga raw-ADF admission check and boot-sector fallback, including the exact embedded-NUL comparison, exercised against authentic A20E media and the nested-archive CLI startup path. F0099's standalone PC3.4 row-flip adapter is contract-only; M11 flips pixels while blitting in separate live paths, which does not prove the source function's copy ownership or all caller destinations. ReDMCSB uses MEDIA009 F0130 and MEDIA529 F0655/F0616+F1006 branches; per-edition/platform equivalence remains open. F0116/F0117 now record their tested PC3.4 D3L/D3R scheduler, wall/ornament, door, pit/stairs, floor-ornament, thing-order and field slices; right-side door-button coverage is also source-locked. Their full dispatcher behavior across DM1/CSB editions and supported platforms remains open. F0348 records bounded DM1/CSB statistic-adjustment routes as `PARTIAL`; DM1 tests historical edition thresholds. The CSB PC3.4 live ROS-mouth regression now covers values 120, 121, 150, 151, 169 and 170, including the strict diminishing thresholds and cap; all other edition/platform routes remain open. F0349 records bounded DM1/CSB mouth-consumption routes as `PARTIAL`: CSB has native PC3.4 effects for C06-C14 plus separate water, waterskin, and food consumers; full RNG/timer and cross-platform proof remains open. F0358 records the tested PC3.4 mouse-route resolver as `IMPLEMENTED_NARROW`; generated-table/media and other-platform equivalence remain open. F0359 records the tested DM1 PC3.4 click enqueue and locked-queue replay route as `IMPLEMENTED_NARROW`; other platform release semantics and CSB lane remain open. F0365 records the tested DM1 PC3.4 turn route as `IMPLEMENTED_NARROW`. F0376 now records the shared inclusive source-box helper, whose 38-assertion PC34 edge/equivalence gate passes. F0377 now preserves the PC34 F0375 throw lane where it overlaps the D1C door-button box; its boundary regression passes. F0374/F0375 now have bounded PC34 runtime and resolver paths: F0374 object-pile floor drop and F0375 side-zone throw are covered by focused drop/throw admission checks, while full edition/platform click-zone parity and special alcove/fountain cases remain open. F0366 records the bounded DM1 PC3.4 step route as `IMPLEMENTED_NARROW`: Firestaff now resolves teleporter records through ReDMCSB’s compact flagged-square SFT index, and the movement integration test passes with source-valid floor-sensor fixtures. CSB V1 compatibility-runtime turn/step cases also cover directional C003 turn sensors, stairs, and an open-corridor step; original-platform and cross-platform verification remain open. |
| skproject `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` | 1,751 total: 1,110 `IMPLEMENTED_PARITY`, 560 `NOT_APPLICABLE_ARCH`, 73 `NONAPPLICABLE`, 5 `PARTIAL`, and 3 `PARTIAL_SOURCE_OWNER` | `DM2_ALLOC_NEW_RECORD`, `DM2_ALLOC_NEW_DBITEM`, `DM2_ALLOC_NEW_DBITEM_DEEPLY`, and `DM2_RECYCLE_A_RECORD_FROM_THE_WORLD` now have bounded source-owner paths: the private GAME_LOAD/SKSAVE clone commits direct DB0/ordinary-DB2/DB3/DB7/DB9 returns plus selected DB5/6/8/10/14 routes, while the generated-drop path recycles selected DB5/6/8/10 records and directly clears DB7/DB9 records in place. They remain partial: complete DB0-DB15 cursor and owner traversal, protected maps, DB4/creature-bearing DB14 deletion, timer/CAII interactions, and all-platform runtime behavior are not covered. `DM2_DEALLOC_RECORD` remains partial outside its bounded creature-delete and CAII delete-tail consumers. `DM2_ARRANGE_DUNGEON` has a bounded mutation owner integrated into New Game; source-by-source mutation and all-edition/platform parity proof remain open. `DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` remains partial; bounded flags-3 ceiling and creature F8 branches use distinct skproject falloff tables, including the distance-over-eight case where only creature light remains. A bounded class-5 live DBE/DBF tail scan now decodes DBE selector FF80 and DBF zero/2/30 selectors, matching sklight.cpp:398-405 and feeds `v1e0978` with its own distance falloff; focused term/accumulator checks and the FM Towns map-3/map-38 gameplay route pass, but the real-media route has not proven a nonzero retail DBE/DBF contribution. Flags-4 mode-7 now retains ordinary floor `RG6` beyond distance 8 while clearing only weather-scaled `vw_24`, matching sklight.cpp:455-481; distance-9 regressions cover both terms. The live callback also admits the sklight.cpp class-2 no-record flags-3 ceiling branch after complete source-input checks. The live mode-8/mode-7 runtime now attempts any authenticated dynamic-light map position and requires complete source-walk receipts; unknown tile/record branches remain fail-closed. The mode-7 flags-3 class-0 no-op at skproject c_light.cpp:233-243 is now covered by a focused helper regression. The class-1 floor-actuator branch now queries authenticated GDAT F8/99 values and retains the distinct source weather term and F8 high-bit gate; focused accumulator regressions cover both. A new FM Towns map-3 (8,4) real-media regression is compiled but has not run locally because original media is unavailable. Mode-7 flags-3 now also summarizes class-3 and direction-matched class-4 tiles and scans up to 256 authenticated record links for DBE/DBF darkness plus creature F8, with focused accumulator tests; flags-4 class-3/4 is a no-op for any accepted record-chain root, class-4 direction-mismatched summaries now accept source-selected adjacent roots after validating class-0..3 record walks (or the source early return for higher classes). `query_0cee_1a46` only changes side-decoration summary words, not the tile/root fields consumed by `c_light.cpp`; ordered DB2 static/animated text decorations now use source GDAT animation timing, and the ordered query feeds DM2 door-button metadata with the current tick and map wall-GFX list; full wall-side decoration integration and source-case runtime coverage remain open. Existing runtime evidence remains FM Towns-only on maps 3 and 38; source record paths and DOS, Amiga, and Macintosh dynamic-light runtime coverage remain open. The DOS New Game start is static-light map 0 at (1,8), so it does not exercise the dynamic path. These behaviors are not closed by source mapping alone. |

The inventory dispositions are repository evidence and must be checked against
the locked source revision before implementation. For DM2 record allocation,
deallocation, and recycling, implement the linked map-chain, recycler, record
pool, CAII, and timer transaction as one source-owned behavior rather than
exposing isolated free-list helpers.

## Gate

Run:

    python3 tools/verify_redmcsb_firestaff_complete_audit.py
