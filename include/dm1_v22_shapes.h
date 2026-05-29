/*
 * dm1_v22_shapes.h — DM1 V2.2 Shape System
 *
 * V2.2 ("Modern Graphics") renders dungeon walls, floors, creatures,
 * items, and UI using modern parameterized art at 1920x1080.
 *
 * The shape system defines geometry descriptors that bridge between
 * the dungeon data layer (DUNGEON.DAT square types) and the modern
 * renderer pipeline (batched GPU draws, PBR materials, dynamic lighting).
 *
 * Design contract:
 *   dungeon cell → M11_V22_ShapeType → M11_V22_ShapeParams
 *                → M11_V22_WallShape / M11_V22_FloorShape
 *                → renderer batch draw
 *
 * Phase: V2.2 shapes are defined here; actual texture/material loading
 * and GPU rendering is deferred to the asset pipeline agent.
 *
 * Source-lock anchors:
 *   ReDMCSB DUNGEON.C:2238-2246 square type → wall/corridor/door/stairs
 *   ReDMCSB DEFS.H:922-941 M034_SQUARE_TYPE enumeration
 *   ReDMCSB DUNVIEW.C:6697-6816 composition draw order (D3/D2/D1/D0)
 *   ReDMCSB DUNGEON.C:35-44 direction step tables for view mapping
 */

#ifndef FIRESTAFF_DM1_V22_SHAPES_H
#define FIRESTAFF_DM1_V22_SHAPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ── Shape types ─────────────────────────────────────────────────── */

/* Every distinct geometry piece rendered in V2.2 has a shape type.
 * These map to renderer draw calls; the shape params carry texture/
 * material/lighting data for each type. */
typedef enum {
    /* ── Wall shapes ──────────────────────────────────────────────── */
    M11_V22_SHAPE_WALL_STRAIGHT      = 0,  /* D1/D2 straight wall face */
    M11_V22_SHAPE_WALL_CORNER_INNER  = 1,  /* Inner corner (L-shape)   */
    M11_V22_SHAPE_WALL_CORNER_OUTER  = 2,  /* Outer corner (Z-shape)   */
    M11_V22_SHAPE_WALL_DOORWAY       = 3,  /* Door opening (with frame) */
    M11_V22_SHAPE_WALL_ALCOVE        = 4,  /* Alcove / recessed niche  */
    M11_V22_SHAPE_WALL_INSCRIPTION   = 5,  /* Wall with inscription     */

    /* ── Floor shapes ────────────────────────────────────────────── */
    M11_V22_SHAPE_FLOOR_PLAIN        = 10, /* Plain stone floor tile    */
    M11_V22_SHAPE_FLOOR_CRACKED      = 11, /* Cracked stone variant     */
    M11_V22_SHAPE_FLOOR_MOSSY       = 12, /* Moss-covered variant      */
    M11_V22_SHAPE_FLOOR_PIT         = 13, /* Pit (no floor, dark void) */
    M11_V22_SHAPE_FLOOR_STAIRS_UP    = 14, /* Stairs ascending           */
    M11_V22_SHAPE_FLOOR_STAIRS_DOWN  = 15, /* Stairs descending          */
    M11_V22_SHAPE_FLOOR_DOOR        = 16, /* Floor under door square   */

    /* ── Ceiling shapes ───────────────────────────────────────────── */
    M11_V22_SHAPE_CEILING_PLAIN      = 20, /* Plain ceiling tile        */
    M11_V22_SHAPE_CEILING_VAULTED   = 21, /* Vaulted/arched ceiling     */

    /* ── Creature shapes ─────────────────────────────────────────── */
    M11_V22_SHAPE_CREATURE          = 30, /* Generic creature sprite   */
    M11_V22_SHAPE_CREATURE_PROJECTILE = 31, /* Creature attack projectile */

    /* ── Item shapes ─────────────────────────────────────────────── */
    M11_V22_SHAPE_ITEM              = 40, /* Dungeon object/item        */
    M11_V22_SHAPE_ITEM_FLOOR       = 41, /* Floor-placed item ornament */
    M11_V22_SHAPE_ITEM_PROJECTILE   = 42, /* Item/thrown projectile     */

    /* ── Field shapes ────────────────────────────────────────────── */
    M11_V22_SHAPE_FIELD_TELEPORTER  = 50, /* Teleporter field           */
    M11_V22_SHAPE_FIELD_FLUXCAGE    = 51, /* Fluxcage effect field      */
    M11_V22_SHAPE_FIELD_EXPLOSION   = 52, /* Explosion visual           */

    /* ── UI / Chrome shapes ───────────────────────────────────────── */
    M11_V22_SHAPE_UI_CHROME         = 60, /* Panel frame / border       */
    M11_V22_SHAPE_UI_PORTRAIT       = 61, /* Champion portrait panel    */
    M11_V22_SHAPE_UI_MESSAGE_LOG    = 62, /* Message log area           */
    M11_V22_SHAPE_UI_INVENTORY_GRID = 63, /* Inventory grid cell        */

    M11_V22_SHAPE_COUNT
} M11_V22_ShapeType;

/* ── Lighting modes ──────────────────────────────────────────────── */

/* Lighting mode affects how the renderer computes per-pixel illumination.
 * TORCH_LIT and MAGICAL_GLOW use distance-falloff from champion position;
 * DUNGEON_AMBIENT uses the 6-level palette darkness. */
typedef enum {
    M11_V22_LIGHT_FULL_BRIGHT    = 0, /* No darkness — title screens */
    M11_V22_LIGHT_DUNGEON_AMBIENT = 1, /* 6-level palette darkness     */
    M11_V22_LIGHT_TORCH_LIT      = 2, /* Torch radius + ambient      */
    M11_V22_LIGHT_MAGICAL_GLOW   = 3, /* Magical effect glow         */
    M11_V22_LIGHT_COUNT
} M11_V22_LightingMode;

/* ── Material descriptor ─────────────────────────────────────────── */

/* PBR-inspired material properties for modern rendering.
 * These drive the shader parameters for a shape's material. */
typedef struct {
    int diffuse_texture_id;   /* Primary albedo texture */
    int normal_texture_id;   /* Normal map for surface detail */
    int specular_texture_id; /* Specular / roughness map */
    int emission_texture_id; /* Self-illuminating texture (magical items) */
    float roughness;         /* 0.0 (mirror) .. 1.0 (matte) */
    float metallic;          /* 0.0 (dielectric) .. 1.0 (metal) */
    float emission_strength;  /* Emission intensity multiplier */
} M11_V22_Material;

/* ── Shape parameters ───────────────────────────────────────────── */

/* The canonical shape descriptor passed to the renderer.
 * Contains everything needed to batch-draw a single shape instance:
 * geometry type, texture/material bindings, per-instance tint, and
 * lighting mode. */
typedef struct {
    M11_V22_ShapeType   type;
    int                 texture_id;        /* Primary diffuse texture */
    int                 normal_map_id;     /* Normal map texture      */
    int                 material_id;       /* M11_V22_Material index  */
    uint8_t             color_tint[4];    /* RGBA tint (0..255)      */
    M11_V22_LightingMode lighting_mode;   /* How this shape is lit   */
    float               height_cm;        /* Shape height in cm      */
    float               width_cm;         /* Shape width in cm       */
    float               depth_cm;         /* Shape depth in cm       */
    int                 vertical_flip;     /* 1=flip Y (ceiling look) */
    float               depth_offset;    /* Z offset into screen    */
    float               ao_strength;     /* Ambient occlusion mult  */
} M11_V22_ShapeParams;

/* ── Wall shape variants ─────────────────────────────────────────── */

/* D1/D2/D3/D0 wall rendering uses variant descriptors that carry
 * the specific texture, normal strength, roughness, and AO settings
 * for each wall configuration. */
typedef struct {
    /* Base texture IDs */
    int                 base_texture_id;
    int                 normal_map_id;

    /* Material properties */
    float               normal_strength;   /* Normal map intensity */
    float               roughness;         /* Surface roughness   */
    float               ao_strength;       /* Ambient occlusion   */

    /* Geometry offsets */
    float               depth_offset;      /* Z offset for parallax */
    float               corner_bevel;      /* Corner bevel radius   */

    /* Style flags */
    int                 corner_style;      /* 0=plain, 1=beveled, 2=round */
    int                 door_frame_present; /* 1=door frame around opening */
    int                 inscription_slot;  /* 1=inscription overlay slot  */
    int                 flipped;           /* 1=horizontally flipped      */
} M11_V22_WallShape;

/* Predefined wall shape variants indexed by wall type and depth.
 * Source-lock: DUNVIEW.C:6697-6816 D3L/D3R/D3C, D2L/D2R/D2C,
 * D1L/D1R/D1C, D0L/D0R/D0C composition order. */
typedef enum {
    M11_V22_WALL_VARIANT_D3_LEFT    = 0,
    M11_V22_WALL_VARIANT_D3_RIGHT   = 1,
    M11_V22_WALL_VARIANT_D3_CENTER  = 2,
    M11_V22_WALL_VARIANT_D2_LEFT    = 3,
    M11_V22_WALL_VARIANT_D2_RIGHT   = 4,
    M11_V22_WALL_VARIANT_D2_CENTER  = 5,
    M11_V22_WALL_VARIANT_D1_LEFT    = 6,
    M11_V22_WALL_VARIANT_D1_RIGHT   = 7,
    M11_V22_WALL_VARIANT_D1_CENTER  = 8,
    M11_V22_WALL_VARIANT_D0_LEFT    = 9,
    M11_V22_WALL_VARIANT_D0_RIGHT   = 10,
    M11_V22_WALL_VARIANT_D0_CENTER  = 11,
    M11_V22_WALL_VARIANT_DOOR       = 12,  /* Door frame variant */
    M11_V22_WALL_VARIANT_COUNT
} M11_V22_WallVariant;

/* ── Floor/Ceiling shape variants ───────────────────────────────── */

/* Floor shapes carry tile pattern info and pit/stairs state. */
typedef enum {
    M11_V22_FLOOR_TILE_PLAIN   = 0,
    M11_V22_FLOOR_TILE_CRACKED = 1,
    M11_V22_FLOOR_TILE_MOSSY   = 2,
    M11_V22_FLOOR_TILE_COUNT
} M11_V22_FloorTilePattern;

/* Floor shape geometry variants — indices into g_floor_shapes[].
 * These map to dungeon cell types and carry pit/stairs/door geometry. */
typedef enum {
    FLOOR_SHAPE_PLAIN       = 0,
    FLOOR_SHAPE_CRACKED    = 1,
    FLOOR_SHAPE_MOSSY      = 2,
    FLOOR_SHAPE_PIT        = 3,
    FLOOR_SHAPE_STAIRS_UP  = 4,
    FLOOR_SHAPE_STAIRS_DOWN = 5,
    FLOOR_SHAPE_COUNT      /* sentinel for array sizing */
} FloorShapeType;

typedef struct {
    int                 base_texture_id;
    int                 normal_map_id;
    M11_V22_FloorTilePattern tile_pattern;
    int                 pit_present;       /* 1=pit (no floor mesh)  */
    int                 stairs_present;    /* 1=stairs geometry      */
    int                 stairs_direction; /* 0=up, 1=down          */
    float               depth_offset;     /* Z offset              */
    float               ao_strength;
    float               roughness;
} M11_V22_FloorShape;

/* ── Shape-to-renderer bridge ────────────────────────────────────── */

/* m11_v22_shape_for_cell — the canonical bridge function.
 *
 * Given a dungeon cell type and view direction, returns the appropriate
 * shape parameters for rendering. This is the primary integration point
 * between the dungeon data layer and the V2.2 modern renderer.
 *
 * dungeon_cell_type: raw square byte from DUNGEON.DAT (DEFS.H:922 M034)
 * view_direction:    0..3 (N/E/S/W)
 * depth:             0..3 (D0=closest, D3=farthest)
 * lateral:          -1/0/+1 (left/center/right)
 *
 * Returns: filled M11_V22_ShapeParams with type, textures, materials, etc.
 *          For unimplemented shapes, returns the PLAIN default with
 *          placeholder texture IDs.
 *
 * Source-lock: DUNGEON.C:2238-2246 square type decode, DUNGEON.C:35-44
 * direction step tables. */
M11_V22_ShapeParams m11_v22_shape_for_cell(int dungeon_cell_type,
                                           int view_direction,
                                           int depth,
                                           int lateral);

/* m11_v22_shape_for_view_square — higher-level variant using view square.
 * Maps directly from DM1_V2_ViewSquare + element → shape params. */
M11_V22_ShapeParams m11_v22_shape_for_view_square(int view_square,
                                                   int element,
                                                   int direction);

/* Get the wall shape variant for a given D3/D2/D1/D0 + lateral position.
 * Returns placeholder if variant not yet implemented. */
M11_V22_WallShape m11_v22_wall_shape_get(M11_V22_WallVariant variant);

/* Get floor shape for a given cell type. */
M11_V22_FloorShape m11_v22_floor_shape_get(int dungeon_cell_type,
                                            int view_direction);

/* Get material by ID. Returns a reference to a builtin default material
 * if id is out of range. */
const M11_V22_Material* m11_v22_material_get(int material_id);

/* Total number of registered materials. */
int m11_v22_material_count(void);

/* ── Initialization ───────────────────────────────────────────────── */

/* Initialize the shape system. Must be called before any shape/bridge
 * functions are used. */
void m11_v22_shapes_init(void);

/* Source evidence string. */
const char* m11_v22_shapes_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V22_SHAPES_H */