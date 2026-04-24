# Firestaff V2 stone golem family

This directory holds a bounded Wave 1 creature-family prototype for **Stone Golem**.

## What exists now

- front-view distance set: near / mid / far
- 4K masters in `masters/4k/`
- exact 50% 1080p derivatives in `exports/1080p/`
- schema-compatible manifest entry for this family

## Workflow contract

- Start from one cleaned source pose reference.
- Generate or paint **4K-first** masters.
- Derive 1080p by exact 50% downscale.
- Keep the original DM depth ladder feel: **smoother is allowed, slower is not**.
- When animation frames are added later, preserve the original cycle duration and gameplay travel speed.

## Current limitations

- one front-view family only
- no side, back, hit, or attack poses yet
- no runtime wiring in the active V1 parity path
- source isolation is still heuristic unless a manual mask is supplied

## Generator

Use `tools/generate_v2_creature_family.py` for repeatable first-pass family exports.
