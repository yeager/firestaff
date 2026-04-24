# Firestaff V2 creature pipeline (bounded Wave 1)

## Purpose

This is a **V2-only** creature asset workflow for bounded family prep, upscale, and export work.
It exists to let creature-family experiments move forward without touching the active V1 parity track.

## Workflow

1. Pick one creature family and one pose group.
2. Start from one source reference image or paintover base.
3. Isolate the subject with either:
   - a supplied manual mask, or
   - the generator's heuristic background-removal pass.
4. When heuristic extraction is not clean enough, check in a manual mask and a paintover-safe workflow plate beside the family.
5. Produce **4K-first** masters for the current depth ladder:
   - near: `1200x1200`
   - mid: `860x860`
   - far: `560x560`
6. Export approved **1080p** assets by exact 50% downscale.
7. If a family gets animation frames, store a timing file beside the sheet and keep the total cycle duration locked.
8. Record provenance and scale intent in a family spec and manifest.

## Directory contract

```text
assets-v2/creatures/wave1/
  CREATURE_PIPELINE.md
  specs/
    <family>-family.md
  <family>-family/
    README.md
    masters/4k/
    exports/1080p/
    workflow/
    animations/   # only when a timing-locked sheet exists
  previews/
```

## Timing contract

V2 may add smoother animation later, but it must preserve the same **perceived timing and speed** as the original behavior.

That means:
- extra in-between frames are allowed
- longer action cycles are not
- smoother walks are allowed
- slower travel or attack feel is not

When a family expands beyond stills, store the original cycle duration beside the animation sheet and keep the V2 frame schedule locked to that duration.

## Current generator

Single-family generation:

```bash
python3 tools/generate_v2_creature_family.py \
  --source assets/cards/creatures/<slug>.png \
  --slug <slug> \
  --display-name "<Name>" \
  --family-dir assets-v2/creatures/wave1/<slug>-family
```

Full currently discoverable pack regeneration:

```bash
python3 tools/generate_v2_creature_wave1_pack.py
```

The full-pack generator reads `creature_family_inventory.json`, covers every currently inventoried Wave 1 creature family, refreshes per-family manifests, and rewrites the aggregate manifest `assets-v2/manifests/firestaff-v2-wave1-creatures.manifest.json`.

Optional:
- `--mask path/to/manual-mask.png`
- `--graphics-index <n>`
- `--original-width <w> --original-height <h>`

## Boundaries

- No V1 runtime wiring in this workflow.
- No claim that these first-pass families are final production art.
- No repo reorganization.
- Manifest/spec updates should stay in English and remain explicit about provenance and current limitations.
- Families that exist only as tiny reference sprites are still valid for bounded workflow coverage, but should be upgraded to stronger source art before any production-quality paintover pass.
