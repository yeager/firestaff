# DM1 V1 champion-panel portrait box blit dispatch gate

Status: DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_BLIT_GATE_PC34_COMPAT_LOCKED

The F0292 -> F0354 dispatch predicate that controls when the inventory-
champion portrait is re-blitted into the 32x29 status-box portrait zone
during the V1 status-box redraw is source-locked to the
ReDMCSB CHAMDRAW.C F0292:757-1110 + TIMELINE.C F0254:1614-1637 +
CHAMDRAW.C F0293:1117-1143 dispatch tree, with the F0292 post-call
Attributes mask pinned to MASK0x0100_STATISTICS and the F0254 secondary
dispatch pinned to the inventory/non-inventory split.

## Primary Evidence
- PASS CHAMDRAW.C:757-760: F0292 short-circuits when none of the nine
  redraw-mask bits (NAME_TITLE | STATISTICS | LOAD | ICON | PANEL |
  STATUS_BOX | WOUNDS | VIEWPORT | ACTION_HAND) is set; the
  champion Attributes field is the only input to this gate.
- PASS CHAMDRAW.C:767-770: F0292 pre-routes the inventory champion
  through F0355_INVENTORY_Toggle_CPSE(C05_CHAMPION_SPECIAL_INVENTORY)
  whenever G0297_B_DrawFloorAndCeilingRequested is set; the status-box
  branch still runs after F0355 returns.
- PASS CHAMDRAW.C:771: F0292 enters the status-box branch only when
  MASK0x1000_STATUS_BOX is set, then resolves the
  C151+championIndex status-box zone rectangle through F0638_GetZone.
- PASS CHAMDRAW.C:784: F0292 forks the dead branch when
  L0865_ps_Champion->CurrentHealth == 0; the dead branch draws
  C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION and the champion name + action
  icon, never reaching F0354.
- PASS CHAMDRAW.C:810-812: F0292 calls F0354_INVENTORY_DrawStatusBoxPortrait
  only when L0863_B_IsInventoryChampion is true, and immediately after
  the call sets only MASK0x0100_STATISTICS in L0862_ui_ChampionAttributes
  so the bar graph + food/water + eye/mouth + load + icon + panel +
  action hand + viewport redraw chain continues for the inventory
  champion.
- PASS CHAMDRAW.C:813-814: F0292 takes the non-inventory-champion
  fallback path and sets NAME_TITLE | STATISTICS | WOUNDS | ACTION_HAND
  on L0862_ui_ChampionAttributes; F0354 is never called on this path.
- PASS CHAMDRAW.C:1110: F0292 clears all nine redraw-mask bits at the
  T0292042 label when the function returns, including the
  MASK0x0100_STATISTICS that was set at line 812.
- PASS TIMELINE.C:1614-1637: F0254_TIMELINE_ProcessEvent12_HideDamageReceived
  short-circuits at the dead-champion gate, routes the inventory
  champion through F0354 between F0077 and F0078, and routes the
  non-inventory champion through F0292 with only MASK0x0080_NAME_TITLE
  on Attributes.
- PASS CHAMDRAW.C:1117-1143: F0293_CHAMPION_DrawAllChampionStates
  OR-s the per-call P2062_ui_ argument into every active champion's
  Attributes and dispatches F0292 in champion-index order from
  C00_CHAMPION_FIRST to G0305-1.
- PASS DEFS.H:2188-2195: C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS
  base and C151..C154 status-box zone stride used by the F0292
  status-box branch.
- PASS DEFS.H:3793: C175_ZONE_FIRST_CHAMPION_STATUS_BOX base used by
  F0354_INVENTORY_DrawStatusBoxPortrait.

## Local Gates
- PASS include/dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.h
- PASS src/dm1/dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.c
- PASS tests/test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.c
- PASS CMakeLists.txt

## Verification
- ./test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat:
  rc=0; assertions=103; failures=0
- CTest: dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat
  Passed 0.01 sec
- Phase A smoke: 23/23 invariants passed
- Full DM1 V1 champion panel ctest (27 tests): all passed
- Strict warnings build (cc -Wall -Wextra -Werror -O2) on both source
  and test: clean
- git diff --check: clean

## Non-Claims
- No real GRAPHICS.DAT, DUNGEON.DAT, savegame, or bitmap load.
- No original-vs-Firestaff pixel parity claim.
- No duplicate pass673 (mouth/eye status-box) gate.
- No duplicate pass683 (food/water status-box) gate.
- No duplicate pass760 (status-hand rotation) gate.
- No duplicate pass762 (C040 rotation race) gate.
- No duplicate dm1_v1_champion_panel_portrait_pc34_compat
  (F0354 portrait box rectangle + CM1 transparency + invisibility
  hatch) gate.
- No duplicate dm1_v1_champion_panel_portrait_state_redraw_pc34_compat
  (F0292 state redraw tuple) gate.

## Anchor Notes
- F0254:1635 sets only MASK0x0080_NAME_TITLE on the Attributes field
  before calling F0292 (the inventory champion's F0292 call would
  re-enter F0292 and trigger the F0292 dispatch tree; the test pins
  the F0254 non-inventory Attributes mask separately from the F0292
  non-inventory fallback mask).
- F0293:1117-1143 champion-index dispatch order is recorded alongside
  the F0293 dispatch site; the F0292 single-champion and F0254
  secondary dispatch paths do not record a per-tick order (-1).
