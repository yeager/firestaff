# DM1 all-graphics parity — phase 897–996: V1 HUD champion name zones

## Scope

Continue HUD/V1 parity by replacing invented compact status-box name/icon placement with the source champion-name zone geometry and colors.

## Source anchors

- `zones_h_reconstruction.json` / GRAPHICS.DAT entry 696:
  - `C159..C162_ZONE_CHAMPION_n_STATUS_BOX_NAME`: clear zones are 43×7 at each status-box origin.
  - `C163..C166_ZONE_FIRST_CHAMPION_NAME`: type-18 text zones at +1 x offset, clipped to 42×7.
- `firestaff_pc34_core_amalgam.c:11496-11507` (`F0292_CHAMPION_DrawState`): non-inventory status boxes clear the champion-name zone to `C01_COLOR_DARK_GRAY`, then print centered champion name in `C163+n`.
- `firestaff_pc34_core_amalgam.c:11494`: leader champion uses `C11_COLOR_YELLOW`; non-leaders use `C09_COLOR_GOLD`.
- `firestaff_pc34_core_amalgam.c:11552-11558`: champion icon drawing is a separate `MASK0x0400_ICON` path; it is not placed inside the compact status-box name zone.

## Implemented

- Added V1 name-zone constants for layout-696 `C159..C166` geometry.
- Added shared probe helpers:
  - `M11_GameView_GetV1StatusNameZone(...)`
  - `M11_GameView_GetV1StatusNameColor(...)`
- V1 compact HUD now:
  - clears the 43×7 name zone to source dark gray (`C01`),
  - centers the champion name in the 42×7 text child zone,
  - uses yellow for active/leader and gold for non-leaders,
  - removes the invented in-status-box champion portrait/name-offset treatment from V1 mode.
- V2/debug fallback keeps the older text treatment so this pass is scoped to V1 parity.

## New invariants

- `INV_GV_15E2`: status name clear zones match layout-696 geometry (x step 69, 43×7).
- `INV_GV_15E3`: name colors follow F0292 leader yellow / non-leader gold.
- `INV_GV_15E4`: rendered framebuffer contains the expected source colors inside the compact name zones.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe -j2`
- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `469/469 invariants passed`

## Probe excerpt

```text
PASS INV_GV_15E V1 champion HUD draws source ready/action hand slot zones inside the status box
PASS INV_GV_15E2 V1 champion HUD name clear zones match layout-696 C159..C162 geometry
PASS INV_GV_15E3 V1 champion HUD name colors follow F0292 leader yellow / non-leader gold
PASS INV_GV_15E4 V1 champion HUD renders source-colored names inside the compact status name zones
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
# summary: 469/469 invariants passed
```
