# ReDMCSB Firestaff complete audit

Updated: 2026-10-30

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
| Sensor execution unsupported types | Floor C10 group-split Steal-XP and deferred CLEAR/TOGGLE rotation have source-shaped M11 WALK_ON consumers. The F0248 wall-event route awards cell-specific C10 Steal-XP to the active leader and applies OnceOnly before dispatch. Other local effects, uncovered sensor-event paths, full platform coverage and original-media proof remain open. | Continue MOVESENS.C F0270/F0271/F0276 audit across every event path and platform; test remaining local-effect types and keep original-media claims separate from source-level behavior. |
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
| ReDMCSB `docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv` | 2,133 total: 810 `MISSING`, 650 `UNCERTAIN_NUMBERED_EVIDENCE`, 505 `IMPLEMENTED_NARROW`, 96 `VERIFIED_SOURCE_MAPPING`, 64 `SOURCE_NONAPPLICABLE`, 5 `PC34_SOURCE_IMPLEMENTED`, 3 `PARTIAL` | Not yet a complete DM1/CSB game-function inventory. The symbol backlog contains 1,064 open rows after existing dispositions; review reachable gameplay/UI functions per platform and explicitly classify OS/ABI-only routines. F0348 records bounded DM1/CSB statistic-adjustment routes as `PARTIAL`; DM1 tests historical edition thresholds, and the CSB mouth-runtime regression covers stat gains below the diminishing thresholds. F0349 records bounded DM1/CSB mouth-consumption routes as `PARTIAL`: CSB has native PC3.4 effects for C06-C14 plus separate water, waterskin, and food consumers; full RNG/timer and cross-platform proof remains open. |
| skproject `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` | 1,751 total: 1,111 `IMPLEMENTED_PARITY`, 560 `NOT_APPLICABLE_ARCH`, 73 `NONAPPLICABLE`, 4 `PARTIAL`, and 3 `PARTIAL_SOURCE_OWNER` | `DM2_ALLOC_NEW_RECORD`, `DM2_ALLOC_NEW_DBITEM`, `DM2_ALLOC_NEW_DBITEM_DEEPLY`, and `DM2_RECYCLE_A_RECORD_FROM_THE_WORLD` now have bounded source-owner paths: the private GAME_LOAD/SKSAVE clone commits direct DB0/ordinary-DB2/DB3/DB7/DB9 returns plus selected DB5/6/8/10/14 routes, while the generated-drop path recycles selected DB5/6/8/10 records and directly clears DB7/DB9 records in place. They remain partial: complete DB0-DB15 cursor and owner traversal, protected maps, DB4/creature-bearing DB14 deletion, timer/CAII interactions, and all-platform runtime behavior are not covered. `DM2_DEALLOC_RECORD` remains partial outside its bounded creature-delete and CAII delete-tail consumers. `DM2_ARRANGE_DUNGEON` has a bounded mutation owner integrated into New Game; source-by-source mutation and all-edition/platform parity proof remain open. `DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` remains partial. These behaviors are not closed by source mapping alone. |

The inventory dispositions are repository evidence and must be checked against
the locked source revision before implementation. For DM2 record allocation,
deallocation, and recycling, implement the linked map-chain, recycler, record
pool, CAII, and timer transaction as one source-owned behavior rather than
exposing isolated free-list helpers.

## Gate

Run:

    python3 tools/verify_redmcsb_firestaff_complete_audit.py
