# ReDMCSB Firestaff complete audit

Updated: 2026-05-14

This is the top-level source-review contract for comparing ReDMCSB against
Firestaff. ReDMCSB is the oracle for DM1 V1 behavior. If Firestaff and
ReDMCSB disagree without stronger original-runtime evidence, Firestaff is
treated as wrong.

## Scope

The audit covers source-backed behavior and presentation, not only existing
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
| Creature AI stubs | memory_creature_ai_pc34_compat.c still labels most creature profiles as stubs. | Replace source-proven simple cases first; create MISSING gates for complex spellcaster/archenemy behavior. |
| Magic/combat defence stubs | memory_magic_pc34_compat.h and combat probes still describe fire/magic/psychic defence paths as stubbed. | Audit CASTER.C plus combat/champion source, then fix or gate missing semantics. |
| Sensor execution unsupported types | Sensor probes still classify rare/complex sensors as unsupported. | Audit MOVESENS.C sensor sections; implement safe source-backed sensor classes or gate each missing type. |
| Original overlay regression absent | docs/parity/COMPLETION_MATRIX.md gives DM1 V1 original_overlay_regression 0/10. | Keep source fixes separate from pixel claims until representative original overlay frames exist. |
| V1-visible shell leakage | Previous parity docs identify custom M11/M12 surfaces that must not decide original behavior. | Audit normal V1 route and keep invented/debug surfaces unreachable unless source-backed. |

## Active review lanes

Started 2026-05-14 on N2 with GPT-5.5 workers:

| Lane | Child session | Write ownership |
|---|---|---|
| movement_input | agent:main:subagent:97014287-28a4-4ed7-b552-85dda6fe87f6 | movement/input/collision/sensor files and focused gates |
| viewport_hud_render | agent:main:subagent:dd408b84-9613-4b85-a733-d9923a61534f | viewport/HUD/render files and focused gates |
| gameplay_systems | agent:main:subagent:70aca479-e754-4758-84fe-68fd9008ec84 | gameplay systems files and focused gates |
| frontend_route_capture | agent:main:subagent:109267ba-c350-4cf2-aa77-b337253ed5c2 | frontend/V1 route/capture files and focused gates |

Workers may create local N2 commits only. Main verifies, integrates, gates,
updates completion percentages and pushes.

## Gate

Run:

    python3 tools/verify_redmcsb_firestaff_complete_audit.py
