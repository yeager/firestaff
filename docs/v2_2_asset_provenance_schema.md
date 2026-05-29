# DM1 V2.2 Modern Asset Provenance Schema

**Version:** 1.0  
**Asset mode:** DM1 V2.2 (Modern Graphics) — 1920×1080 generated 3D-rendered 2D art  
**Location:** `~/.firestaff/assets/dm1/modern/`

---

## Overview

Every modern asset file shipped in the V2.2 pipeline carries a SHA-256
provenance record that encodes:

- Which original V1 surface it was derived from (if applicable)
- What generation tool and version produced it
- Timestamp of generation
- Style/parameters used so the look is reproducible

The provenance record is stored as a JSON object alongside the asset
in a file named `<asset-id>.provenance.json`, or as a top-level
`provenance` key inside `modern_asset_manifest.json`.

---

## Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "DM1 V2.2 Modern Asset Provenance",
  "type": "object",
  "required": ["id", "generated_at", "generator"],
  "properties": {
    "id": {
      "type": "string",
      "description": "Unique asset identifier matching the entry in modern_asset_manifest.json"
    },
    "derived_from": {
      "type": "object",
      "description": "Present when the asset was generated from an original V1 surface",
      "properties": {
        "source_game":    { "type": "string", "enum": ["dm1", "csb", "dm2", "nexus"] },
        "source_file":   { "type": "string" },
        "surface_category": { "type": "string" },
        "sha256":        { "type": "string", "pattern": "^[a-f0-9]{64}$" }
      },
      "required": ["source_file", "sha256"]
    },
    "generator": {
      "type": "object",
      "required": ["name", "version"],
      "properties": {
        "name":    { "type": "string" },
        "version": { "type": "string" },
        "url":     { "type": "string", "format": "uri" }
      }
    },
    "generated_at": {
      "type": "string",
      "format": "date-time",
      "description": "ISO 8601 timestamp of when the asset was generated"
    },
    "style": {
      "type": "object",
      "description": "Generation parameters that affect the visual style",
      "properties": {
        "render_resolution": {
          "type": "object",
          "properties": {
            "width":  { "type": "integer" },
            "height": { "type": "integer" }
          }
        },
        "lighting_model": { "type": "string" },
        "color_palette_source": { "type": "string" },
        "texture_style": { "type": "string" },
        "outline_style": { "type": "string" },
        "custom_params": {
          "type": "object",
          "additionalProperties": true
        }
      }
    },
    "output": {
      "type": "object",
      "description": "Final output file info",
      "properties": {
        "file":   { "type": "string" },
        "format": { "type": "string", "enum": ["png", "tga", "webp"] },
        "width":  { "type": "integer" },
        "height": { "type": "integer" },
        "sha256": { "type": "string", "pattern": "^[a-f0-9]{64}$" }
      },
      "required": ["file", "sha256"]
    }
  }
}
```

---

## Example

```json
{
  "id": "wall_back_d3l2",
  "derived_from": {
    "source_game": "dm1",
    "source_file": "GRAPHICS.DAT",
    "surface_category": "wall-back",
    "sha256": "a3f1b8c9d2e4f6a7b8c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a"
  },
  "generator": {
    "name": "firestaff-modern-asset-generator",
    "version": "1.0.0",
    "url": "https://github.com/yeager/firestaff"
  },
  "generated_at": "2026-05-15T14:32:00Z",
  "style": {
    "render_resolution": { "width": 1920, "height": 1080 },
    "lighting_model": "pbr-ambient-occlusion",
    "color_palette_source": "dm1-vga-palette-corrected",
    "texture_style": "hand-painted",
    "outline_style": "pixel-compatible"
  },
  "output": {
    "file": "wall_back_d3l2.png",
    "format": "png",
    "width": 1920,
    "height": 1080,
    "sha256": "b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3b4"
  }
}
```

---

## Design Notes

- Provenance is **append-only**: once an asset ships with a provenance
  record that record MUST NOT be modified. A new generation run creates
  a new provenance record with a new SHA-256.
- The `sha256` fields use **uppercase hexadecimal** (matching the OpenSSL
  convention used in the rest of Firestaff's asset pipeline).
- `surface_category` values MUST match the `DM1_V2_SurfaceCategory` enum
  names in `include/dm1_v2_asset_pipeline_pc34.h` (e.g. `wall-back`,
  `creature`, `object`).
- Generation tools that produce V2.2 assets MUST be versioned and
  referenced by `name` + `version` so any visual regression can be
  traced to a specific tool revision.