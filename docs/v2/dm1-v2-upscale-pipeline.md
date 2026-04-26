# DM1 V2.1 upscale pipeline scout

## Scope

This is a planning scaffold for a V2-only upscale pipeline. It does not add art, redistribute original assets, alter renderers, or change the active V2 manifests/catalogs. The goal is to define the production contract before any worker creates files under the asset tree.

## Locked production model

- DM1 logical source dimensions remain the measurement authority.
- V2.1 masters are painted or restored at **10x** the DM1 logical source size.
- 1080p shipping derivatives are produced only by exact **50% downscale** from the approved 10x master.
- The 1080p derivative is therefore 5x the original logical source, even when it is displayed inside a 1920x1080 presentation envelope.
- Approved masters and derivatives are stored by checksum and referenced from manifests later; this scout does not edit schemas or catalogs.

## Size contract

| Class | Original logical size | V2.1 master | Derived 1080p export | Notes |
|---|---:|---:|---:|---|
| Full gameplay field | 320x200 | 3200x2000 | 1600x1000 | Centered in 16:9 presentation space. |
| Viewport base | 224x136 | 2240x1360 | 1120x680 | Preserves DM1 composition and crop. |
| Action area | 87x45 | 870x450 | 435x225 | Layered UI family; no action glyphs here. |
| Spell area | 87x25 | 870x250 | 435x125 | Rune/text overlays remain separate. |
| Status box frame | 67x29 | 670x290 | 335x145 | Left/right variants share the same contract. |
| Party HUD slot cell | 18x18 | 180x180 | 90x90 | Strict 10x baseline; any richer exception must be recorded per family. |

Existing Wave 1 documents may choose a larger master for specific readability reasons. This V2.1 scout treats that as an explicit family exception, not the default upscale rule.

## Checksum store integration

Each produced artifact should be staged into a content-addressed store before it is referenced by a manifest:

1. Compute SHA-256 over the exact file bytes.
2. Store the blob under a checksum-derived key such as `sha256/<first-two>/<full-sha256>`.
3. Record sidecar metadata beside the logical asset record, not inside the binary:
   - logical asset id
   - source logical size
   - master size
   - derived size
   - pipeline version
   - source provenance note
   - generation or paintover notes
   - checksum for master
   - checksum for derived export
4. Manifests should reference the logical asset id and checksum keys, not local workstation paths.
5. The store must not include unlicensed original DM1 assets or source files extracted from them.

## Dry-run design

The dry run must be safe on a clean checkout and must not create assets. It should only validate a recipe and print the actions that a real pipeline would later perform:

- verify that the declared master size is exactly 10x the original logical size
- verify that the derived size is exactly half of the master size
- report the relative master and derived output names that would be used
- report the checksum-store keys that would be populated after real files exist
- fail if any recipe path is absolute or tries to escape the repository

Prototype command:

```bash
python3 tools/v2_upscale_dry_run.py --emit-template
python3 tools/v2_upscale_dry_run.py --recipe docs/v2/v2-upscale-recipe.example.json
```

The first command prints a JSON recipe template to standard output. The second command validates a recipe and prints a JSON plan only; it does not read image pixels or write assets.

## Non-goals

- No renderer changes.
- No original asset redistribution.
- No changes to active V2 schemas, manifests, or logical-id catalogs in this scout branch.
- No generated PNG, PDF, or binary asset outputs.
- No quality claim for any future art until visual review approves both the 10x master and the half-scale derivative.

## Handoff checklist

Before production starts, a follow-up branch should decide:

- the final checksum-store directory layout
- whether checksum metadata lives in manifests or adjacent provenance records
- the approved resampling kernel for 50% derivatives
- the naming convention for master and derived files
- the policy for family-specific exceptions such as oversized HUD-cell masters
