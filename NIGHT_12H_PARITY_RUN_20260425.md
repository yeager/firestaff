# 12h parity run — 2026-04-25 22:44 Europe/Stockholm

Owner instruction: go toward DM1 V1 1:1 parity, target at least ~3000 passes eventually. Run as many real passes as possible over the next 12h without user babysitting.

Current pass counter at start: 796.

Rules:
- No bullshit pass inflation.
- Prefer HUD/V1 interaction parity unless a stronger blocker appears.
- A pass batch must include real source-backed implementation or verification work.
- Each batch should include:
  - source anchors (ReDMCSB / DEFS / GRAPHICS.DAT / zones_h_reconstruction where applicable)
  - code changes
  - invariants/probes or equivalent verification
  - evidence markdown under parity-evidence/
  - ctest and M11 probe (or explicit blocker if a gate cannot run)
  - git commit and push
- Keep batches sequenced and small enough to review. Do not invent UI semantics.
- Avoid secret leakage. Do not touch unrelated untracked planning/assets unless needed.

Recent commits:
- 1888f8a honor v1 status hand wound graphics — passes 697–796
- 996c6f4 select v1 status acting hand graphic — passes 597–696
- bcaa7dc draw v1 champion status hand slots — passes 497–596
- eeeb289 remove invented v1 champion hud chrome — passes 397–496
- f8c43e5 wire champion mirror recruit flow into game view — passes 297–396

Completed batches:

- Passes 797–896 — V1 HUD empty hand icon parity hardening.
  - Source-backed `F0291_CHAMPION_DrawSlot` icon selection for empty ready/action hands:
    - normal ready/action icons `212/214`
    - wounded ready/action icons `213/215`
    - acting action hand changes box graphic `35` only, not selected icon
    - occupied wounded hands still use object icon resolver (`F0033_OBJECT_GetIconIndex` mirror)
  - Added shared `M11_GameView_GetV1StatusHandIconIndex(...)` and renderer/probe invariants `INV_GV_15K`..`INV_GV_15O`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase797_896_v1_hud_empty_hand_icons.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe -j2`; `ctest --test-dir build --output-on-failure`; `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `466/466 invariants passed`.

- Passes 897–996 — V1 HUD champion name zone/color parity.
  - Source-backed layout-696 `C159..C166` compact status-box name zones: 43×7 clear zone, 42×7 centered text child, 69 px champion stride.
  - Replaced invented in-status-box portrait/name-offset treatment in V1 with `F0292_CHAMPION_DrawState` behavior: clear dark gray, centered name, leader yellow / non-leader gold.
  - Added shared `M11_GameView_GetV1StatusNameZone(...)` / `M11_GameView_GetV1StatusNameColor(...)` and invariants `INV_GV_15E2`..`INV_GV_15E4`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase897_996_v1_hud_name_zones.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe -j2`; `ctest --test-dir build --output-on-failure`; `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `469/469 invariants passed`.

- Passes 997–1096 — V1 HUD dead champion name parity.
  - Source-backed dead status-box branch: graphic `C008`, centered champion name in `C163+n`, color `C13` lightest gray over `C01` dark gray.
  - Removed invented red `DEAD` label from V1 compact HUD; kept fallback/V2 behavior separate.
  - Added invariant `INV_GV_15E5`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase997_1096_v1_hud_dead_name.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe -j2`; `ctest --test-dir build --output-on-failure`; `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `470/470 invariants passed`.

- Passes 1097–1196 — V1 action icon area clear parity.
  - Source-backed `F0387_MENUS_DrawActionArea` idle/icon branch: fill `C011_ZONE_ACTION_AREA` black before drawing champion action-hand cells.
  - Added invariant `INV_GV_300A` proving the action-area top band is black in icon mode.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1097_1196_v1_action_icon_area_clear.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe -j2`; `ctest --test-dir build --output-on-failure`; `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `471/471 invariants passed`.

- Passes 1197–1216 — V1 action icon hatch lockout parity.
  - Source-backed `F0386_MENUS_DrawActionIcon` hatch branch for resting/candidate lockout: living action-hand cells get the VGA black checker pattern from `F8155_VIDRV_06_HatchScreenBox`.
  - Preserved dead-cell plain-black branch and normal idle/menu behavior.
  - Added invariant `INV_GV_300C` and fixed the synthetic action-cell scene so CTest exercises the asset-backed path.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1197_1216_v1_action_icon_hatch.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `472/472`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

Suggested next HUD targets:
1. Status hand object icon parity hardening: exact empty/occupied hand icon indices and pixel-backed invariants.
2. Name/title/status-box text zone placement/color parity.
3. Right-column action icon cell/menu parity and cross-checks against status action-hand state.
4. Spell area HUD parity if status box gets saturated.
5. Capture screenshots after meaningful visual batches.

Update this file with batch summaries if useful.

- Passes 1217–1236 — V1 action icon source zone helper hardening.
  - Added probe-visible helpers for layout-696 `C089..C092` action-cell zones and `C093..C096` inner icon zones.
  - Routed the V1 action icon renderer through the shared source-zone helper to keep drawing/pointer/probe geometry anchored to one source-backed definition.
  - Added invariant `INV_GV_300D` for first/fourth cell and inner icon geometry: cells `(233,86,20,35)` and `(299,86,20,35)`, inner icons `(235,95,16,16)` and `(301,95,16,16)`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1217_1236_v1_action_icon_zone_helpers.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `473/473`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1237–1256 — V1 action icon pointer zone parity.
  - Routed action-cell pointer hit testing through `M11_GameView_GetV1ActionIconCellZone(...)` so click geometry shares the same source-backed layout-696 `C089..C092` definition as drawing/probes.
  - Added invariant `INV_GV_300E`: a click at `(318,120)` activates champion ordinal 4 from source `C092` rightmost-cell geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1237_1256_v1_action_icon_pointer_zone.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `474/474`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1257–1276 — V1 status hand zone helper hardening.
  - Added `M11_GameView_GetV1StatusHandZone(...)` for layout-696 `C211..C218` status ready/action hand slot geometry.
  - Routed V1 status-hand drawing through the shared helper.
  - Added invariant `INV_GV_15E6`: slot 0 ready `(16,170,16,16)` and slot 3 action `(243,170,16,16)` match source child zones with 69 px status stride.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1257_1276_v1_status_hand_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `475/475`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.
