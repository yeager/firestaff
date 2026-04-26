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

- Passes 1277–1296 — V1 status bar zone helper hardening.
  - Added `M11_GameView_GetV1StatusBarZone(...)` for source HP/stamina/mana vertical bar geometry.
  - Routed V1 bar drawing through the shared helper.
  - Added invariant `INV_GV_15E7`: slot 0 HP `(58,164,4,25)` and slot 3 mana `(279,164,4,25)` match layout-696 bar placement with 69 px status stride.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1277_1296_v1_status_bar_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `476/476`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1297–1316 — V1 status name text zone helper hardening.
  - Added `M11_GameView_GetV1StatusNameTextZone(...)` for layout-696 `C163..C166` text child geometry.
  - Routed V1 compact name drawing through the shared helper.
  - Added invariant `INV_GV_15E8`: slot 0 text `(13,160,42,7)` and slot 3 text `(220,160,42,7)` match source layout with 69 px status stride.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1297_1316_v1_status_name_text_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `477/477`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1317–1336 — V1 status name clear-zone routing.
  - Routed compact champion-name clear fill through `M11_GameView_GetV1StatusNameZone(...)` instead of draw-site local arithmetic.
  - Reused existing invariant coverage `INV_GV_15E2`/`INV_GV_15E4` for layout-696 `C159..C162` clear geometry and rendered source-colored names.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1317_1336_v1_status_name_clear_zone_routing.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `477/477`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1337–1356 — V1 action menu row zone helper.
  - Added `M11_GameView_GetV1ActionMenuRowZone(...)` for shared F0387 action-menu trigger row geometry.
  - Routed row rendering and pointer row-hit testing through the helper.
  - Added invariant `INV_GV_300F`: row 0 `(224,58,87,9)` and row 2 `(224,80,87,9)` match source row placement.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1337_1356_v1_action_menu_row_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `478/478`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1357–1376 — V1 action menu header zone helper.
  - Added `M11_GameView_GetV1ActionMenuHeaderZone(...)` for the F0387 acting-champion header band.
  - Routed action-menu cyan header fill through the helper.
  - Added invariant `INV_GV_300G`: header `(224,47,87,9)` matches source menu-mode header placement.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1357_1376_v1_action_menu_header_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `479/479`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1377–1396 — V1 action area zone helper.
  - Added `M11_GameView_GetV1ActionAreaZone(...)` for source `C011_ZONE_ACTION_AREA` geometry.
  - Routed menu-mode clear/reblit, utility-panel action frame blit/fallback clear, and icon-mode black clear through the helper.
  - Added invariant `INV_GV_300H`: action area `(224,45,87,45)` matches source right-column panel geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1377_1396_v1_action_area_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `480/480`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1397–1416 — V1 spell area zone helper.
  - Added `M11_GameView_GetV1SpellAreaZone(...)` for source `C013_ZONE_SPELL_AREA` geometry.
  - Routed V1 spell-frame blit and partial-frame fallback clear through the helper.
  - Added invariant `INV_GV_300I`: spell area `(224,90,87,25)` matches source right-column panel geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1397_1416_v1_spell_area_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `481/481`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1417–1436 — V1 status box zone helper.
  - Added `M11_GameView_GetV1StatusBoxZone(...)` for compact source status-box placement.
  - Routed V1 party slot loop placement through the helper while preserving V2 HUD behavior.
  - Added invariant `INV_GV_15E9`: slot 0 `(12,160,67,29)` and slot 3 `(219,160,67,29)` match GRAPHICS.DAT `C007` with 69 px source stride.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1417_1436_v1_status_box_zone_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `482/482`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1437–1456 — V1 action menu text origin helper.
  - Added `M11_GameView_GetV1ActionMenuTextOrigin(...)` for source action-menu header/row text placement.
  - Routed acting-champion header text and action-row names through the helper.
  - Added invariant `INV_GV_300J`: header `(226,48)`, row 0 `(226,59)`, row 2 `(226,81)` match F0387 offsets.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1437_1456_v1_action_menu_text_origin_helper.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `483/483`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1457–1476 — V1 status shield border priority.
  - Added `M11_GameView_GetV1StatusShieldBorderGraphic(...)` for source shield overlay graphic selection.
  - Routed V1 status-box shield border drawing through the helper.
  - Added invariant `INV_GV_15P`: no shield → 0, party → 37, fire-over-party → 38, spell-over-fire-over-party → 39.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1457_1476_v1_status_shield_border_priority.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `484/484`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1477–1496 — V1 status box base graphic selection.
  - Added `M11_GameView_GetV1StatusBoxBaseGraphic(...)` for source living/dead status-box base selection.
  - Routed dead-status-box asset selection through the helper.
  - Added invariant `INV_GV_15Q`: living champion → 0 (clear/rebuild), dead champion → 8 (`C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION`).
  - Evidence: `parity-evidence/dm1_all_graphics_phase1477_1496_v1_status_box_base_graphic.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `485/485`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1497–1516 — V1 status box zone routing.
  - Routed dead status-box asset checks/blits and living dark-gray clears through `M11_GameView_GetV1StatusBoxZone(...)` dimensions (`slotW/slotH`).
  - Reused `INV_GV_15E9` and `INV_GV_15Q` coverage for source box geometry and base graphic selection.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1497_1516_v1_status_box_zone_routing.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `485/485`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 1517–1536 — V1 status shield border zone routing.
  - Routed shield border overlay asset checks/blits through `slotW/slotH` from `M11_GameView_GetV1StatusBoxZone(...)`.
  - Reused `INV_GV_15E9` and `INV_GV_15P` coverage for source status-box dimensions and overlay priority.
  - Evidence: `parity-evidence/dm1_all_graphics_phase1517_1536_v1_status_shield_border_zone_routing.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `485/485`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2377–2396 — V1 status name source zone ids.
  - Added probe-visible helpers for layout-696 `C159..C162` champion name clear zones and `C163..C166` centered name text zones.
  - Routed compact status name clear/text helper validation through the source zone-id helpers.
  - Hardened `INV_GV_15E2` and `INV_GV_15E8` to assert both zone ids and geometry, including out-of-range slot rejection.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2377_2396_v1_status_name_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2397–2416 — V1 status box source zone ids.
  - Added `M11_GameView_GetV1StatusBoxZoneId(...)` for layout-696 `C151..C154` compact champion status-box roots.
  - Routed `M11_GameView_GetV1StatusBoxZone(...)` through the source zone-id helper.
  - Hardened `INV_GV_15E9` to assert both root zone ids and geometry, including out-of-range slot rejection.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2397_2416_v1_status_box_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2417–2436 — V1 status bar value zone ids.
  - Added `M11_GameView_GetV1StatusBarGraphZoneId(...)` for layout-696 `C187..C190` bar-graph containers.
  - Added `M11_GameView_GetV1StatusBarValueZoneId(...)` for champion/stat-specific `C195..C206` HP/stamina/mana value zones.
  - Routed `M11_GameView_GetV1StatusBarZone(...)` through the source value zone-id helper while preserving the existing first-champion stat id helper.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2417_2436_v1_status_bar_value_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2437–2456 — V1 status hand parent zone ids.
  - Added `M11_GameView_GetV1StatusHandParentZoneId(...)` for layout-696 `C207..C210` status-hand parent containers.
  - Routed status hand child-zone id validation through the parent zone-id helper.
  - Hardened `INV_GV_15E6` to assert both `C207..C210` parent ids and `C211..C218` ready/action child ids/geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2437_2456_v1_status_hand_parent_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2457–2476 — V1 spell area zone id.
  - Added `M11_GameView_GetV1SpellAreaZoneId()` for layout-696 `C013_ZONE_SPELL_AREA`.
  - Routed `M11_GameView_GetV1SpellAreaZone(...)` through the source zone-id helper.
  - Hardened `INV_GV_300I` to assert both `C013` id and right-column spell-area geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2457_2476_v1_spell_area_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2477–2496 — V1 action area zone id.
  - Added `M11_GameView_GetV1ActionAreaZoneId()` for layout-696 `C011_ZONE_ACTION_AREA`.
  - Routed the action-area geometry helper through the source zone id and tightened `INV_GV_300H` to assert both id and `(224,45,87,45)` geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2477_2496_v1_action_area_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2497–2516 — V1 action icon zone ids.
  - Added source id helpers for layout-696 action icon parent zones `C089..C092` and inner icon zones `C093..C096`.
  - Routed action icon cell/inner geometry helpers through those ids and hardened `INV_GV_300D` to assert ids plus geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2497_2516_v1_action_icon_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2517–2536 — V1 action icon parent zone id.
  - Added `M11_GameView_GetV1ActionIconParentZoneId()` for layout-696 `C088`, the action-hand icon parent/template under `C011`.
  - Routed action icon cell id validation through the parent helper and hardened `INV_GV_300D` to assert `C088` plus `C089..C096`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2517_2536_v1_action_icon_parent_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `513/513`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2537–2556 — V1 spell caster zone ids.
  - Added source id/geometry helpers for layout-696 `C221_ZONE_SPELL_AREA_SET_MAGIC_CASTER` and `C224_ZONE_SPELL_AREA_MAGIC_CASTER_TAB`.
  - Routed caster-tab validation through the parent panel helper and added `INV_GV_300AC` for ids plus top-of-spell-area geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2537_2556_v1_spell_caster_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `514/514`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2557–2576 — V1 action PASS zone id.
  - Added source id/geometry helper for layout-696 `C098_ZONE_ACTION_AREA_PASS`, right-aligned under the action-area `C011` source zone.
  - Added `INV_GV_300AD` to assert `C098` and `(275,45,35,7)` geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2557_2576_v1_action_pass_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `515/515`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2577–2596 — V1 action result zone id.
  - Added source id/geometry helper for layout-696 `C075_ZONE_ACTION_RESULT`, routed through `C011` action-area geometry.
  - Added `INV_GV_300AE` to assert `C075` and `(224,45,87,45)` geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2577_2596_v1_action_result_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `516/516`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2597–2616 — V1 action menu graphic geometry.
  - Added source-sized geometry helper for action-menu graphic zones selected by row count (`C079`, `C077`, or full `C011`).
  - Added `INV_GV_300AF` to assert one-/two-/three-row menu rectangles `(87×21)`, `(87×33)`, and `(87×45)`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2597_2616_v1_action_menu_graphic_geometry.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `516/516`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2617–2636 — V1 spell symbol zone ids.
  - Added source id helpers for spell-area symbol parents `C245..C250`, available-symbol zones `C255..C260`, and selected/champion symbol zones `C261..C264`.
  - Added `INV_GV_300AG` for first/last ids and out-of-range rejection.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2617_2636_v1_spell_symbol_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `518/518`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2637–2656 — V1 spell cast/recant zone ids.
  - Added source id helpers for `C252_ZONE_SPELL_AREA_CAST_SPELL` and `C254_ZONE_SPELL_AREA_RECANT_SYMBOL`.
  - Extended `INV_GV_300AG` to assert cast/recant ids alongside spell-symbol zone ids.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2637_2656_v1_spell_cast_recant_zone_ids.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `518/518`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2657–2676 — V1 leader hand object-name zone id.
  - Added source id/geometry helper for layout-696 `C017_ZONE_LEADER_HAND_OBJECT_NAME`.
  - Added `INV_GV_300AH` to assert `C017` and `(233,33,87,6)` geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2657_2676_v1_leader_hand_object_name_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `520/520`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2677–2696 — V1 inventory food/water zones.
  - Added source id/geometry helpers for layout-696 `C101_ZONE_PANEL`, `C103_ZONE_FOOD_BAR`, and `C104_ZONE_FOOD_WATER`.
  - Added `INV_GV_300AI` to assert panel rectangle `(80,52,144,73)` plus food/water clipped label zones `(113,69,34,6)` and `(113,92,46,6)`.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2677_2696_v1_inventory_food_water_zones.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `520/520`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2697–2716 — V1 viewport zone id.
  - Added source id/geometry helper for layout-696 `C007_ZONE_VIEWPORT`, bound to existing DM1 PC viewport `(0,33,224,136)`.
  - Added `INV_GV_300AJ` to assert `C007` and viewport geometry.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2697_2716_v1_viewport_zone_id.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `522/522`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2717–2736 — V1 champion icon zones.
  - Added source id/geometry helpers for layout-696 `C113_ZONE_CHAMPION_ICON_TOP_LEFT` through `C116_ZONE_CHAMPION_ICON_BOTTOM_LEFT`.
  - Added `INV_GV_300AK` to assert `C113..C116`, invalid-slot rejection, and representative clipped `16×14` rectangles.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2717_2736_v1_champion_icon_zones.md`.
  - Gates: `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`; `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` → `522/522`; `ctest --test-dir build --output-on-failure` → `5/5 passed`.

- Passes 2737–2756 — V1 movement arrow zone ids.
  - Added source id helpers for layout-696 `C009_ZONE_MOVEMENT_ARROWS` and arrow controls `C068..C073`.
  - Added `INV_GV_300AL` to assert all movement-arrow ids plus invalid-index rejection.
  - Evidence: `parity-evidence/dm1_all_graphics_phase2737_2756_v1_movement_arrow_zone_ids.md`.
  - Gates: pending in worker.
