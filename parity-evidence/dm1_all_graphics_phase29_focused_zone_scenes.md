# DM1 all-graphics phase 29 — focused pit/stairs/teleporter zone scenes

Date: 2026-04-25 14:28 Europe/Stockholm
Scope: Firestaff V1 / deterministic focused visual scenes for source-bound viewport zone work.

## Change

Added deterministic focused viewport scenes to `firestaff_m11_game_view_probe` for the recently implemented source-bound zone domains:

- D1C normal pit
- D1C invisible pit variant
- D1C stairs down/front
- D1C visible/open teleporter field

These are synthetic 5x5 dungeon scenes using original GRAPHICS.DAT assets, no debug HUD, party at `(2,3)` facing north, and a single feature at D1C `(2,2)`. This gives a small visual proof set independent of the normal party location screenshot.

## New invariants

The game-view probe now includes four focused visual-difference invariants:

- `INV_GV_38A`: D1C normal pit source blit changes the empty corridor frame
- `INV_GV_38B`: D1C invisible pit variant differs from normal pit
- `INV_GV_38C`: D1C stairs zone blit changes the empty corridor frame
- `INV_GV_38D`: D1C teleporter field zone blit changes the empty corridor frame

These are deliberately simple framebuffer-difference gates: they prove the isolated feature branch is active in a deterministic focused scene.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `366/366 invariants passed`
- CTest: `4/4 PASS`

Focused invariant output:

```text
PASS INV_GV_38A focused viewport: D1C normal pit source blit changes the corridor frame
PASS INV_GV_38B focused viewport: D1C invisible pit variant differs from normal pit
PASS INV_GV_38C focused viewport: D1C stairs zone blit changes the corridor frame
PASS INV_GV_38D focused viewport: D1C teleporter field zone blit changes the corridor frame
```

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase29-focused-zone-scenes-20260425-1428/normal/30_focused_empty_corridor_vga.ppm`
- `verification-m11/dm1-all-graphics/phase29-focused-zone-scenes-20260425-1428/normal/31_focused_d1c_normal_pit_vga.ppm`
- `verification-m11/dm1-all-graphics/phase29-focused-zone-scenes-20260425-1428/normal/32_focused_d1c_invisible_pit_vga.ppm`
- `verification-m11/dm1-all-graphics/phase29-focused-zone-scenes-20260425-1428/normal/33_focused_d1c_stairs_down_vga.ppm`
- `verification-m11/dm1-all-graphics/phase29-focused-zone-scenes-20260425-1428/normal/34_focused_d1c_teleporter_vga.ppm`

Scaled PNGs and viewport-only crops were generated next to the PPM files, including:

- `31_focused_d1c_normal_pit_vga_dungeon_viewport_only.png`
- `32_focused_d1c_invisible_pit_vga_dungeon_viewport_only.png`
- `33_focused_d1c_stairs_down_vga_dungeon_viewport_only.png`
- `34_focused_d1c_teleporter_vga_dungeon_viewport_only.png`

## Visual inspection

Vision inspection of corrected dungeon-viewport-only crops confirmed:

- normal pit is visibly distinct with bottom-center pit/ledge geometry
- invisible pit is visibly distinct from normal pit and hides the pit geometry
- stairs are clearly distinct with stairwell/railing geometry
- teleporter is clearly distinct with blue particle/portal field
- no UI bleed in the corrected viewport-only crops

## Remaining work

This proves D1C focused branches. Still needed for full lock:

- expand focused scenes to D0/D1/D2/D3 left/right/center placements
- compare against original DOSBox/ReDMCSB captures for equivalent synthetic/controlled setups
- decide deterministic-vs-random teleporter shimmer runtime policy
