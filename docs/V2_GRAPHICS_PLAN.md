# Firestaff V2 Graphics Plan

## Decisions (Daniel 2026-05-17)

- **v2.1 resolution:** Exact 10x
- **v2.2 art style:** 3D-rendered 2D — light DM feel but modern 2026
- **Priority order:** DM1 → CSB → DM2 → Nexus (last)
- **GPT-5.5:** Available again Wednesday morning (2026-05-21)

## Tiers

| Tier | Description |
|------|-------------|
| v2.0 | Original graphics + filter pipeline (CRT, palette boost, dither cleanup, sharpening) |
| v2.1 | 10x AI upscale via GPT-5.5 hybrid, faithful DM aesthetic |
| v2.2 | Modern 3D-rendered 2D art, all new assets, light DM feel |

## Runtime

- Hot-swap between v1/v2.0/v2.1/v2.2 in settings menu
- Fallback: missing v2.x asset → show v1 original
- Filter chain (v2.0) is runtime; v2.1/v2.2 are pre-baked

## Asset Structure

```
assets/{dm1,csb,dm2,nexus}/
  v1/         — extracted original PNGs + manifest.json
  v2.0/       — (runtime filters, no separate assets needed)
  v2.1/       — 10x upscaled PNGs + manifest.json
  v2.2/       — modern 3D-rendered 2D PNGs + manifest.json
```

## Phases

0. Asset extraction — all 4 games (now → Tue)
1. v2.0 filter pipeline (Tue → Thu)
2. v2.1 AI upscale — GPT-5.5 (Wed → Fri)
3. v2.2 modern art (after v2.1 QA)
4. Runtime integration (parallel with fas 1-2)

## Animation Smoothing (Daniel 2026-05-17)

V2.1 and V2.2 must have smoother animations compared to V1 originals.

### V2.1 (Upscaled)
- Interpolated tile-to-tile movement (no snap)
- Smooth 90° turning with intermediate frames
- Door open/close animation with tweened frames
- Creature frame interpolation (original 1-3 frames → smooth blend)
- Level transition fades

### V2.2 (Modern)
- All V2.1 smoothing plus:
- Frame-rate independent animation (60fps+)
- Camera easing (ease-in/out on steps and turns)
- Particle effects (magic, torches, water, dust)
- Creature skeletal/spritesheet animation with additional frames
- UI animation (inventory open/close, health bar transitions)
- Environmental animation (flickering lights, ambient movement)

### Implementation
- `src/dm1v2/dm1_v2_animation_interpolator.c` — lerp/slerp engine for movement, turning, doors
- Per-mode animation config: V1 = instant (original), V2.0 = instant (filtered), V2.1 = interpolated, V2.2 = full smooth
- Animation speed tied to game tick but rendered at display refresh rate
- Original game logic unchanged — interpolation is purely visual
