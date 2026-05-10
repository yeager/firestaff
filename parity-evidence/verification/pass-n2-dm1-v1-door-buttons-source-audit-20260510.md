# N2 DM1 V1 door-with-buttons source audit — 2026-05-10

Scope: ReDMCSB source audit plus Firestaff verification for Daniel's DM1 V1 door-with-buttons blocker. This artifact intentionally records references and outcomes only, not source excerpts.

## Source anchors audited

- ReDMCSB `DUNVIEW.C:1210` defines `G0208_aaauc_Graphic558_DoorButtonCoordinateSets`.
- ReDMCSB `DUNVIEW.C:4119-4212` contains `F0110_DUNGEONVIEW_DrawDoorButton`, including graphic selection from `M634_GRAPHIC_FIRST_DOOR_BUTTON`, coordinate-set selection, and clickable-box storage.
- ReDMCSB `DUNVIEW.C:6593`, `6738`, `7333`, and `7902` call the door-button draw path for D3R, D3C, D2C, and D1C visible door-button cases.
- ReDMCSB `CLIKVIEW.C:367-379`, `407-431`, and `469-496` route door-button/wall-ornament viewport clicks through the C05 front-wall clickable cell toward sensor processing.
- ReDMCSB `MOVESENS.C:1309` is the wall-click sensor trigger entry reached by that route.
- ReDMCSB `DEFS.H:2316`, `2387`, and `2647` define the relevant graphic-base and C05 view-cell constants for the PC/graphics variants.

## Firestaff implementation anchors checked

- `m11_game_view.c:8591` binds the DM1 V1 door-button graphic base.
- `m11_game_view.c:10496-10553` draws center door buttons at source coordinates and gates them on the door's button flag.
- `m11_game_view.c:10555-10600` draws the D3R side door button and keeps the side-lane occlusion guard.
- `m11_game_view.c:18462-18463` preserves center-button then D3R-button draw order after door world rendering.
- Existing gates lock coordinate, source-order, occlusion, and Daniel-priority coverage.

## Verification run

Host: N2 / `firestaff-worker`
Repo: `/home/trv2/work/firestaff`
Build dir: `/home/trv2/work/build-door-buttons-n2`
Git HEAD at verification start: `5970f28`

Commands run:

- Configure: `cmake -S . -B ../build-door-buttons-n2 -G Ninja -DBUILD_TESTING=ON`
- Build: `cmake --build ../build-door-buttons-n2 --target test_dm1_v1_viewport_3d_pc34_compat firestaff_m11 -j2`
- Tests: `ctest --output-on-failure -R "(pass456_dm1_v1_door_title_end_priority_gate|v1_door_button_ornament_coordinates_gate|v1_viewport_center_door_occlusion_gate|v1_viewport_d1c_doorpass_source_lock_gate|dm1_v1_viewport_3d_source_lock)"`

Result: 5/5 targeted tests passed.

## Conclusion

No additional runtime code change was needed in this pass: the current N2 branch already contains the ReDMCSB-aligned door-with-buttons implementation and the targeted gate set passes cleanly. This artifact closes the requested N2 source-audit/probe lane with references and reproducible gates.
