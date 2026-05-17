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
