# Pass 76 — Firestaff capture-route state probe

Date: 2026-04-26

## Goal

Stop treating the six Firestaff screenshot fixtures as opaque pixels. This pass adds a state probe that mirrors `verification-screens/capture_firestaff_ingame_series.c` and records the exact Firestaff route state behind each capture.

This is a Firestaff fixture-route contract only. It does **not** claim the original DOS route is semantically identical.

## Added gate

- `probes/m11/firestaff_m11_capture_route_state_probe.c`
- CMake target: `firestaff_m11_capture_route_state_probe`
- CTest: `m11_capture_route_state`

## Evidence output

Generated with:

```sh
./build/firestaff_m11_capture_route_state_probe \
  verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA \
  verification-m11/capture-route-state
```

Outputs:

- `verification-m11/capture-route-state/pass76_capture_route_state_probe.md`
- `verification-m11/capture-route-state/pass76_capture_route_state_probe.json`

## Locked fixture route

| capture | action | result | tick | map | x | y | dir | spellOpen | runes | inventoryOpen | rightHandThing |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 01_ingame_start_latest | start | 1 | 0 | 0 | 1 | 3 | 2 | 0 | 0 | 0 | `0x1400` |
| 02_ingame_turn_right_latest | right | 1 | 1 | 0 | 1 | 3 | 3 | 0 | 0 | 0 | `0x1400` |
| 03_ingame_move_forward_latest | up | 1 | 2 | 0 | 0 | 3 | 3 | 0 | 0 | 0 | `0x1400` |
| 04_ingame_spell_panel_latest | spell_rune_1 | 1 | 2 | 0 | 0 | 3 | 3 | 1 | 1 | 0 | `0x1400` |
| 05_ingame_after_cast_latest | spell_rune_4+rune_4+spell_cast | 1 | 3 | 0 | 0 | 3 | 3 | 0 | 0 | 0 | `0x1400` |
| 06_ingame_inventory_panel_latest | spell_clear+inventory | 1 | 3 | 0 | 0 | 3 | 3 | 0 | 0 | 1 | `0x1400` |

Pass 77 fixed the earlier one-rune bug: `05_ingame_after_cast_latest` now enters low-power Ful Ir via `RUNE_4`, `RUNE_4`, then `SPELL_CAST`; the gate verifies cast input is consumed, tick advances to 3, and the spell panel closes.

## Impact

Pass 74 measured high original-vs-Firestaff deltas but could not say whether the fixture states matched. Pass 76 now makes the Firestaff side explicit, so the next original-route work can target these exact states instead of guessing from filenames.
