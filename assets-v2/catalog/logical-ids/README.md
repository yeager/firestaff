# Firestaff V2 Shared Logical Asset IDs

This directory is the first catalog layer for V2 logical asset IDs shared by **DM1**, **CSB**, and **DM2**.

It is deliberately not a rendering change and not a claim that every listed asset already exists. The catalog gives V2 planning and future manifest/runtime work a stable vocabulary above game-specific source indices, checksums, and generated file paths.

## Files

- `shared-v2-logical-ids.json` — first shared logical ID catalog covering UI, dungeon-view, inventory, champions, spells, objects, creatures, and audio.
- `firestaff-v2-shared-logical-id-catalog.schema.json` — JSON Schema for the catalog shape.

## ID policy

Use:

```text
fs.v2.shared.<category>.<family>.<role>[.<state>]
```

Do not put source graphic indices, checksums, resolution names, or dates in logical IDs. Those belong in manifests, source maps, and content-addressed storage.

## Boundaries

- V2 only.
- Planning/catalog layer only.
- DM1/CSB/DM2 shared semantics only; game-specific exceptions should become overlays later.
- No V1 parity, M10, renderer, or asset-byte changes are implied by this catalog.
