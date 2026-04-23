# First V2 in-game 4K render path

This note describes the smallest honest path now present for an initial Firestaff V2 in-game 4K render.

## What it does

1. builds and runs a real M11 capture scene
2. captures a no-creature baseline frame and a creature-present frame
3. composes a 3840x2160 PNG using:
   - the real captured in-game scene
   - existing Wave 1 V2 4K UI shells
   - the first-pass V2 skeleton family

## Run

```bash
cmake -S . -B build
cmake --build build --target firestaff_m11_v2_initial_4k_capture
mkdir -p verification-screens/v2-initial-4k
FIRESTAFF_V2_VERTICAL_SLICE=1 \
./build/firestaff_m11_v2_initial_4k_capture \
  verification-screens/v2-initial-4k \
  "$HOME/.firestaff/data"
python3 tools/render_v2_initial_4k.py
```

## Outputs

- `verification-screens/v2-initial-4k/base_scene.ppm`
- `verification-screens/v2-initial-4k/creature_scene.ppm`
- `verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png`

## What is still placeholder

- the viewport world render remains the live low-resolution M11 render, scaled for the 4K composition
- only the skeleton front-view family is available for V2 creature art in this pass
- creature placement in the 4K composition is still an offline capture/composite step, not yet a fully native real-time 4K renderer
