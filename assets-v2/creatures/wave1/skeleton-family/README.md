# Firestaff V2 skeleton family (first pass)

This is the first bounded V2 creature family added only to make an honest initial in-game 4K render possible.

## What exists now

- `fs-v2-skeleton-front-near.4k.png`
- `fs-v2-skeleton-front-mid.4k.png`
- `fs-v2-skeleton-front-far.4k.png`
- matching 1080p exports

## Scope

This is **not** a full creature production set.

It is only:

- one creature family: skeleton
- one facing family: front view
- three distance-scaled variants for the initial screenshot path

## Current limitations

- no side, back, or attack poses yet
- not yet wired into the live low-resolution M11 creature renderer
- still derived from existing project reference art rather than a final pose-complete V2 paintover set

## Usage in this pass

The assets are used by `tools/render_v2_initial_4k.py` to produce the first bounded in-game V2 4K screenshot composition on top of a real M11 capture scene.
