# Pass 72 — source-driven DM1 viewport state probe

Date: 2026-04-26

## Goal

Replace DOSBox-keystroke guessing as the next viewport parity anchor with a deterministic state composer that reads the original DM1 PC 3.4 `DUNGEON.DAT` and `GRAPHICS.DAT` directly.

This pass does **not** claim full pixel overlay parity. It locks the exact source state that later overlay passes should render from.

## Added gate

- `probes/m11/firestaff_m11_viewport_state_probe.c`
- CMake target: `firestaff_m11_viewport_state_probe`
- CTest: `m11_viewport_state`

The probe:

1. resolves `DUNGEON.DAT` and `GRAPHICS.DAT` from a data directory;
2. initialises `GameWorld_Compat` via `F0882_WORLD_InitFromDungeonDat_Compat()`;
3. samples the party start map/X/Y/direction from source state;
4. composes a 3×3 relative viewport neighborhood (depth 0..2, lanes left/center/right);
5. traverses square thing chains and counts doors, groups, items, sensors, text strings, teleporters, projectiles, and explosions;
6. queries critical GRAPHICS.DAT viewport asset dimensions.

## Evidence output

Generated with:

```sh
cmake --build build --target firestaff_m11_viewport_state_probe -j4
./build/firestaff_m11_viewport_state_probe \
  verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA \
  verification-m11/viewport-state
```

Output:

- `verification-m11/viewport-state/dm1_viewport_state_probe.md`
- `verification-m11/viewport-state/dm1_viewport_state_probe.json`

Key locked start state:

| Field | Value |
| --- | --- |
| mapIndex | 0 |
| mapX | 1 |
| mapY | 3 |
| direction | 2 / SOUTH |

Critical GRAPHICS.DAT viewport assets queried successfully:

| graphic | dimensions |
| ---: | --- |
| 0 | 224×136 |
| 78 | 224×97 |
| 79 | 224×39 |
| 8 | 67×29 |
| 9 | 87×25 |
| 10 | 87×45 |
| 42 | 256×32 |

## Gate result

```text
ctest --test-dir build -R m11_viewport_state --output-on-failure
100% tests passed, 0 tests failed out of 1
```

## Next blocker removed

The viewport parity path now has a deterministic source-state anchor. The remaining blocker is renderer parity from this state into the 224×136 viewport and surrounding V1 chrome, not original-data discovery or emulator input sequencing.
