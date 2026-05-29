# DM1 V2.2 Modern Asset Manifest

**Version:** 1.0  
**Asset mode:** DM1 V2.2 (Modern Graphics)  
**Manifest path:** `~/.firestaff/assets/dm1/modern/modern_asset_manifest.json`

---

## Overview

The **modern asset manifest** is a JSON catalog that lists every V2.2
modern asset available for DM1. It is the entry point the runtime bridge
(`DM1_V2_LoadModernAssetManifest()`) looks for to determine whether V2.2
assets are present; if `modern_asset_manifest.json` is absent the engine
silently falls back to the V2.1 EPX-upscales pipeline.

The manifest does **not** need to list every asset — missing entries
are treated the same as a missing manifest (silent V2.1 fallback). This
means V2.2 can be rolled out incrementally: add assets category by
category and the renderer falls back to V2.1 for any not-yet-converted
surfaces.

---

## File Location Convention

```
~/.firestaff/assets/dm1/modern/
    modern_asset_manifest.json   ← this file
    wall_shapes/
        wall_back_d3l2.png       ← 1920×1080 RGBA
        wall_back_d3r2.png
        wall_near_d2l2.png
        ...
    creature_shapes/
        creature_lizard.png
        ...
    item_shapes/
        ...
    ui_chrome/
        ...
    champion_portraits/
        ...
    dungeon_furniture/
        ...
    <id>.provenance.json         ← one per asset (optional, at same level)
```

---

## Manifest JSON Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "DM1 V2.2 Modern Asset Manifest",
  "type": "object",
  "required": ["version", "generated_at", "assets"],
  "properties": {
    "version": {
      "type": "string",
      "description": "Manifest schema version (1.0 for this spec)"
    },
    "game_id": {
      "type": "string",
      "const": "dm1"
    },
    "graphics_mode": {
      "type": "string",
      "const": "v2.2-modern"
    },
    "generated_at": {
      "type": "string",
      "format": "date-time"
    },
    "render_resolution": {
      "type": "object",
      "properties": {
        "width":  { "type": "integer", "const": 1920 },
        "height": { "type": "integer", "const": 1080 }
      }
    },
    "assets": {
      "type": "object",
      "description": "Top-level shape categories, each containing individual asset entries",
      "properties": {
        "wall_shapes": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/asset_entry"
          }
        },
        "creature_shapes": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/asset_entry"
          }
        },
        "item_shapes": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/asset_entry"
          }
        },
        "ui_chrome": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/asset_entry"
          }
        },
        "champion_portraits": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/asset_entry"
          }
        },
        "dungeon_furniture": {
          "type": "object",
          "additionalProperties": {
            "$ref": "#/$defs/asset_entry"
          }
        }
      }
    }
  },
  "$defs": {
    "asset_entry": {
      "type": "object",
      "required": ["id", "file", "width", "height", "format"],
      "properties": {
        "id": {
          "type": "string",
          "description": "Unique identifier, e.g. 'wall_back_d3l2'"
        },
        "source_file": {
          "type": "string",
          "description": "Original V1 GRAPHICS.DAT surface or 'generated' if no source"
        },
        "file": {
          "type": "string",
          "description": "Relative path under the modern/ directory, e.g. 'wall_shapes/wall_back_d3l2.png'"
        },
        "width":  { "type": "integer" },
        "height": { "type": "integer" },
        "format": {
          "type": "string",
          "enum": ["png", "tga", "webp"]
        },
        "provenance_ref": {
          "type": "string",
          "description": "Path to the .provenance.json file, relative to modern/"
        }
      }
    }
  }
}
```

---

## Shape Categories

### wall_shapes

All wall surfaces: D3L2, D3R2, D2L2, D2R2, D2C, D3C, door panel + frame,
wall ornaments, floor ornaments.

Maps to `DM1_V2_SURFACE_WALL_*`, `DM1_V2_SURFACE_DOOR`,
`DM1_V2_SURFACE_WALL_ORNAMENT`, `DM1_V2_SURFACE_FLOOR_ORNAMENT`.

### creature_shapes

All creature sprites, indexed by champion/enemy identity with posture
variants. Includes fluxcage field sprites.

Maps to `DM1_V2_SURFACE_CREATURE`, `DM1_V2_SURFACE_FLUXCAGE`.

### item_shapes

All dungeon objects: projectiles, explosions, treasure, props.

Maps to `DM1_V2_SURFACE_OBJECT`, `DM1_V2_SURFACE_PROJECTILE`,
`DM1_V2_SURFACE_EXPLOSION`.

### ui_chrome

Non-gameplay 2D elements: panel backgrounds, message log area, font
renderings, title screen, entrance animation.

Maps to `DM1_V2_SURFACE_PANEL_CHAMPION`, `DM1_V2_SURFACE_PANEL_MESSAGE`,
`DM1_V2_SURFACE_FONT`, `DM1_V2_SURFACE_TITLE`, `DM1_V2_SURFACE_ENTRANCE`.

### champion_portraits

Champion portrait images used in the champion selection and panel UI.

Maps to `DM1_V2_SURFACE_PANEL_CHAMPION`.

### dungeon_furniture

Static dungeon props that are part of the tile geometry: pits, water,
stairs rendered as standalone surfaces rather than V1 wall/floor
ornaments.

Maps to `DM1_V2_SURFACE_FLOOR_ORNAMENT` (extended set).

---

## Full Example Manifest

```json
{
  "version": "1.0",
  "game_id": "dm1",
  "graphics_mode": "v2.2-modern",
  "generated_at": "2026-05-15T14:32:00Z",
  "render_resolution": { "width": 1920, "height": 1080 },
  "assets": {
    "wall_shapes": {
      "wall_back_d3l2": {
        "id": "wall_back_d3l2",
        "source_file": "GRAPHICS.DAT",
        "file": "wall_shapes/wall_back_d3l2.png",
        "width": 1920,
        "height": 1080,
        "format": "png",
        "provenance_ref": "wall_shapes/wall_back_d3l2.provenance.json"
      },
      "door_panel_a": {
        "id": "door_panel_a",
        "source_file": "GRAPHICS.DAT",
        "file": "wall_shapes/door_panel_a.png",
        "width": 1920,
        "height": 1080,
        "format": "png"
      }
    },
    "creature_shapes": {
      "lizard_front": {
        "id": "lizard_front",
        "source_file": "GRAPHICS.DAT",
        "file": "creature_shapes/lizard_front.png",
        "width": 1920,
        "height": 1080,
        "format": "png"
      }
    },
    "item_shapes": {},
    "ui_chrome": {},
    "champion_portraits": {},
    "dungeon_furniture": {}
  }
}
```

---

## Loading Rules

1. **Presence check:** `DM1_V2_LoadModernAssetManifest()` opens
   `~/.firestaff/assets/dm1/modern/modern_asset_manifest.json`. If the
   file cannot be opened the function returns `0` and the engine uses V2.1.
2. **Silent fallback:** Parsing errors do NOT emit warnings; the engine
   falls back to V2.1 and logs internally.
3. **Incremental rollout:** If a shape category is absent or empty, the
   renderer substitutes the corresponding V2.1 EPX surface for that
   category only.
4. **Resolution contract:** All V2.2 assets are exactly 1920×1080 RGBA.
   The renderer does not re-scale modern assets at runtime.

---

## Renderer Integration

When `DM1_V2_IsModernAssetMode()` returns true, the renderer in
`m11_game_view.c` replaces each surface category lookup with a path
constructed as:

```
DM1_V2_GetModernAssetRoot() + "/" + manifest[category][id].file
```

The file is loaded with SDL_LoadBMP / IMG_Load (PNG via SDL_image) and
blitted directly to the 1920×1080 presentation surface without
additional scaling.