# Pass 77 — make after-cast fixture actually cast

Date: 2026-04-26

## Goal

Fix the misleading `05_ingame_after_cast_latest` deterministic capture. Pass 76 proved it previously tried `SPELL_CAST` after one rune, so the input was ignored and the screenshot was still a one-rune spell panel.

## Change

`verification-screens/capture_firestaff_ingame_series.c` now keeps `04_ingame_spell_panel_latest` as the one-rune spell-panel fixture, then enters a deterministic low-power Ful Ir sequence before the after-cast capture:

```text
RUNE_1, save 04 spell panel
RUNE_4, RUNE_4, SPELL_CAST, save 05 after cast
```

This encodes the intended low-power Fireball/Ful Ir route through the current M11 rune UI.

## State evidence

`firestaff_m11_capture_route_state_probe` now locks:

| capture | action | result | tick | map | x | y | dir | spellOpen | runes | inventoryOpen |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 04_ingame_spell_panel_latest | spell_rune_1 | 1 | 2 | 0 | 0 | 3 | 3 | 1 | 1 | 0 |
| 05_ingame_after_cast_latest | spell_rune_4+rune_4+spell_cast | 1 | 3 | 0 | 0 | 3 | 3 | 0 | 0 | 0 |
| 06_ingame_inventory_panel_latest | spell_clear+inventory | 1 | 3 | 0 | 0 | 3 | 3 | 0 | 0 | 1 |

## Measurement update

`tools/pass74_fullscreen_panel_pair_compare.py` now reads Firestaff PPM captures directly instead of stale PNG siblings, so it measures the regenerated deterministic captures.

Current pass-74 deltas remain high, which is expected until the original DOS route is matched to the Firestaff fixture state.

## Gates

```text
cmake --build build --target firestaff_m11_capture_route_state_probe capture_ingame_series -j4
./build/firestaff_m11_capture_route_state_probe ...
./run_firestaff_m11_ingame_capture_smoke.sh
python3 tools/pass74_fullscreen_panel_pair_compare.py
ctest --test-dir build -R 'm11_(viewport_state|capture_route_state)' --output-on-failure
```
