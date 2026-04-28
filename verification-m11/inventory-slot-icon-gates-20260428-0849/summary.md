# Inventory slot/icon gate rerun — 2026-04-28 08:49 CEST

Lane: Inventory / DM1 V1.

## Scope

Focused verification of the current DM1 V1 inventory source-slot/icon path after the earlier inventory `rc=126` concern. No runtime code change was needed in this pass: the checked-in slot probe is executable on N2 and direct invocation returns `0`.

## Commands

```sh
git status --short --branch
git log --oneline -5
./run_firestaff_memory_graphics_dat_slots_probe.sh
cmake --build build --target firestaff_m11_game_view_probe -j2
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
./run_firestaff_m11_ingame_capture_smoke.sh
```

## Results

- `./run_firestaff_memory_graphics_dat_slots_probe.sh`: PASS, `rc=0`; log: `slots_probe.log`.
- `cmake --build build --target firestaff_m11_game_view_probe -j2`: PASS during this pass; target rebuilt before probe run.
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`: PASS, `578/578 invariants passed`; log: `game_view_probe.log`.
- `./run_firestaff_m11_ingame_capture_smoke.sh`: PASS, `In-game capture smoke PASS: 6 screenshots`; log: `ingame_capture_smoke.log`.

## Inventory gates covered

- Source slot namespace/gates: `INV_GV_353`–`INV_GV_358`, `INV_GV_436`, `INV_GV_438`.
- Inventory viewport/backdrop and dynamic icons: `INV_GV_359`–`INV_GV_363`.
- Object/icon palette distinction: `INV_GV_409`–`INV_GV_412`, `INV_GV_300K`–`INV_GV_300L`, `INV_GV_309B`.
- Capture smoke confirms deterministic action-vs-inventory dagger palette checks.

## Blockers

None for this bounded inventory gate rerun. The remaining known product gap is still model coverage for original backpack source boxes beyond the compact Firestaff champion slots (`C528..C536`), already documented in phase 2957–2976.
