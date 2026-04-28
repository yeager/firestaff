# Pass 110 — V1 HUD type-7 bar-value resolver

Scope: DM1 PC 3.4 V1 champion top-row/status panel evidence only.

This pass closes the Pass 107 honesty gap by independently resolving the layout-696 `type=7` HP/stamina/mana value zones into the same 4×25 rectangles used by the active champion HUD overlay.

## Result

- resolver JSON: `parity-evidence/overlays/pass110_v1_hud_type7_bar_value_resolver.json`
- source zones: `zones_h_reconstruction.json`
- overlay checked: `parity-evidence/overlays/pass83/pass83_champion_hud_zone_overlay_stats.json`
- resolved bars: `12`
- pass: `True`
- problems: `0`

## Locked geometry

- slot origins remain `0/69/138/207` from `C151..C154`.
- each bar region remains `C183..C186` → `C187..C190` → `C191..C194`.
- each `type=7` value zone (`C195..C206`) resolves to a 4×25 bottom-flush container at x offsets `46/53/60` within each 67×29 status box.
- sample fill math locks full HP (`25px`), half stamina (`12px`), and min-1px mana behavior for non-zero values.

## Honesty boundary

This is source-chain + overlay-geometry evidence, not original DOS runtime pixel parity. Original-runtime top-row overlay comparison remains blocked on a stable original gameplay reference frame.

## Gate

```sh
python3 tools/pass110_v1_hud_type7_bar_value_resolver.py
python3 -m json.tool parity-evidence/overlays/pass110_v1_hud_type7_bar_value_resolver.json >/dev/null
git diff --check
```
