# DM1 all-graphics parity — phase 1197–1216: V1 action icon hatch lockout

## Scope

Tighten right-column action icon parity for the source lockout hatch applied while the party is resting or a champion mirror candidate panel is active.

## Source anchors

- `firestaff_pc34_core_amalgam.c:11826-11863` (`F0386_MENUS_DrawActionIcon`): after drawing a living champion action icon cell, source calls `F0136_VIDEO_HatchScreenBox(P0760_ui_ChampionIndex + C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION, C00_COLOR_BLACK)` when `G0299_ui_CandidateChampionOrdinal` or `G0300_B_PartyIsResting` is set.
- `dm7z-extract/Toolchains/Common/Source/VIDEODRV.C:3243-3295` (`F8155_VIDRV_06_HatchScreenBox`): VGA hatch leaves odd `(x ^ y)` pixels unchanged and forces even checker pixels to black.
- `firestaff_pc34_core_amalgam.c:11858-11861`: dead champions still use the plain black-cell branch and return before the living-cell hatch branch.

## Implemented

- Added a small VGA-faithful `m11_hatch_rect()` helper using the source `(x ^ y)` checker pattern.
- Applied hatch to living V1 action-hand icon cells when M11's source-backed global lockout states are active:
  - `state->resting` mirrors `G0300_B_PartyIsResting`.
  - `state->candidateMirrorOrdinal > 0` / `candidateMirrorPanelActive` mirrors the champion-candidate panel path.
- Preserved the dead-champion black-cell branch and normal idle/menu action-area behavior.
- Fixed the probe's synthetic action-cell scene to mark the game view active, so the asset-backed action-cell render path is actually exercised under CTest.
- Updated the legacy phase-A smoke compile script so the modern startup renderer's generated card/logo sources link in the manual launcher smoke path.

## New invariant

- `INV_GV_300C`: V1 action-hand icon cells have the source black checker hatch during rest/candidate lockout.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `472/472 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300 action-hand icon cells: both living champions get cyan backdrop (or no assets)
PASS INV_GV_300A action icon mode fills the source action area top band black before drawing cells
PASS INV_GV_300B action-hand icon cells: empty living hand blits source empty-hand icon
PASS INV_GV_300C action-hand icon cells hatch living cells during rest/candidate lockout
PASS INV_GV_301 action-hand icon cells: dead champion cell is solid black
# summary: 472/472 invariants passed
```
