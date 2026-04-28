# Lane 1 viewport/world visuals follow-up

Date (UTC): 2026-04-28T07:09:30Z
Lane: DM1 V1 viewport/world visuals (walls/items/ornaments/creatures/draw-order/evidence)
Host: N2

## Commands

```sh
python3 tools/verify_v1_viewport_draw_order_gate.py
python3 tools/verify_v1_viewport_occlusion_gate.py
cmake --build build --target firestaff_m11_game_view_probe -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
```

## Result

PASS.

- Draw-order source-shape gate passed: wall/door ornaments before open-cell contents; open-cell order floor ornaments -> floor items -> creatures -> projectiles/effects.
- Occlusion source-shape gate passed: center-lane blocker-derived maxVisibleForward is wired through pits, floor ornaments, stairs, teleporter fields, side walls/doors/door ornaments/destroyed-door masks before farther-cell sampling.
- `firestaff_m11_game_view_probe` built and passed: `578/578 invariants passed`.

## Evidence

- Full command log: `output.log`
- No viewport/world renderer code fix was made; bounded probes did not expose a small source-faithful lane-local defect.
- Remaining blocker: no semantically matched original DM1 gameplay route/capture was added in this follow-up, so this remains source/data/probe parity evidence rather than pixel-perfect original-runtime parity.
