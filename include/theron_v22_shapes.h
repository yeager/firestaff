#ifndef FIRESTAFF_THERON_V22_SHAPES_H
#define FIRESTAFF_THERON_V22_SHAPES_H

/*
 * theron_v22_shapes.h — Theron V2.2 Modern Shape System
 *
 * Theron's Quest V2.2 ("Modern Graphics") renders Theron's
 * 4x3 dungeon viewport (4 depth x 3 lateral = 12 cells) using
 * modern parameterized art at 1920x1080. Parallel to
 * dm1_v22_shapes.h and csb_v22_shapes.h.
 *
 * 4x3 layout (Theron V1 viewport):
 *   depth D3 (farthest):  L  C  R
 *   depth D2:             L  C  R
 *   depth D1:             L  C  R
 *   depth D0 (closest):   L  C  R
 * (4 depth x 3 lateral = 12 cells, matches DM1)
 *
 * Theron-specific shapes beyond DM1's set:
 *   - THERON_V22_SHAPE_TELEPORTER  (HUCC6280 teleporter warp, big deal in Theron)
 *   - THERON_V22_SHAPE_ALARM       (alert-all-creatures alarm square)
 *   - THERON_V22_SHAPE_SECRET_DOOR (secret door passage, Theron-only)
 *   - THERON_V22_SHAPE_FLOODED     (Theron's flooded/water squares)
 *   - THERON_V22_SHAPE_LIT_TORCH   (Theron has 4+ torch slots, not 4 like DM1)
 *
 * Design contract:
 *   Theron square type + view position -> Theron_V22_ShapeType ->
 *     Theron_V22_ShapeParams -> Theron_V22_WallShape / Theron_V22_FloorShape ->
 *     renderer batch draw
 *
 * Phase: Theron V2.2 shapes are defined here; the actual texture /
 * material loading and GPU rendering is deferred to the Theron V2.2
 * modern asset pipeline. This module is the *selector* and the
 * *parameter book*.
 *
 * Source-lock references:
 *   - THQUEST.ASM T400  - tile bank loading
 *   - THQUEST.ASM T520  - tile selection
 *   - THQUEST.ASM T600  - UI overlay zones
 *   - THQUEST.ASM T700  - teleporter warp dispatch
 *   - THQUEST.ASM T800  - alarm / alert dispatch
 *   - HuC6260/HuC6270 datasheet - VDC/VCE rendering
 *   - tqr_v1_phase2_data_formats_H2339.md §7 - tile data format
 *   - include/theron_v1_world.h - THERON_SQUARE_* enum (cell type source)
 *   - include/dm1_v22_shapes.h, include/csb_v22_shapes.h - mirror references
 *
 * Module: src/theron/theron_v22_shapes.c
 * Test:   tests/test_theron_v22_shapes_pc34.c
 * Probe:  probes/firestaff_theron_v22_shapes_probe.c
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /* Wall shapes (mirror of DM1/CSB) */
    THERON_V22_SHAPE_WALL_STRAIGHT      = 0,
    THERON_V22_SHAPE_WALL_CORNER_INNER  = 1,
    THERON_V22_SHAPE_WALL_CORNER_OUTER  = 2,
    THERON_V22_SHAPE_WALL_DOORWAY       = 3,
    THERON_V22_SHAPE_WALL_ALCOVE        = 4,
    THERON_V22_SHAPE_WALL_INSCRIPTION   = 5,

    /* Floor shapes */
    THERON_V22_SHAPE_FLOOR_PLAIN        = 10,
    THERON_V22_SHAPE_FLOOR_CRACKED      = 11,
    THERON_V22_SHAPE_FLOOR_MOSSY        = 12,
    THERON_V22_SHAPE_FLOOR_PIT          = 13,
    THERON_V22_SHAPE_FLOOR_STAIRS_UP    = 14,
    THERON_V22_SHAPE_FLOOR_STAIRS_DOWN  = 15,
    THERON_V22_SHAPE_FLOOR_DOOR         = 16,

    /* Ceiling shapes */
    THERON_V22_SHAPE_CEILING_PLAIN      = 20,
    THERON_V22_SHAPE_CEILING_VAULTED    = 21,

    /* Creature / item shapes */
    THERON_V22_SHAPE_CREATURE           = 30,
    THERON_V22_SHAPE_CREATURE_PROJECTILE = 31,
    THERON_V22_SHAPE_ITEM               = 40,
    THERON_V22_SHAPE_ITEM_FLOOR         = 41,
    THERON_V22_SHAPE_ITEM_PROJECTILE    = 42,

    /* Field shapes (mirror + Theron-only) */
    THERON_V22_SHAPE_FIELD_TELEPORTER   = 50,
    THERON_V22_SHAPE_FIELD_ALARM        = 51, /* Theron-only */
    THERON_V22_SHAPE_FIELD_FLUXCAGE     = 52,
    THERON_V22_SHAPE_FIELD_EXPLOSION    = 53,

    /* UI / Chrome shapes (mirror) */
    THERON_V22_SHAPE_UI_CHROME          = 60,
    THERON_V22_SHAPE_UI_PORTRAIT        = 61,
    THERON_V22_SHAPE_UI_MESSAGE_LOG     = 62,
    THERON_V22_SHAPE_UI_INVENTORY_GRID  = 63,

    /* Theron-only narrative shapes */
    THERON_V22_SHAPE_SECRET_DOOR        = 70, /* THQUEST.ASM T800 */
    THERON_V22_SHAPE_FLOODED            = 71, /* Theron flooded square */
    THERON_V22_SHAPE_LIT_TORCH          = 72, /* Theron 4+ torch slots */
    THERON_V22_SHAPE_TELEPORTER_TARGET  = 73, /* warp destination */

    THERON_V22_SHAPE_COUNT
} Theron_V22_ShapeType;

typedef enum {
    THERON_V22_LIGHT_FULL_BRIGHT      = 0,
    THERON_V22_LIGHT_DUNGEON_AMBIENT  = 1,
    THERON_V22_LIGHT_TORCH_LIT        = 2,
    THERON_V22_LIGHT_MAGICAL_GLOW     = 3,
    /* Theron-only: alarm pulse glow (red on dark). */
    THERON_V22_LIGHT_ALARM_PULSE     = 4,
    THERON_V22_LIGHT_COUNT
} Theron_V22_LightingMode;

typedef struct {
    int diffuse_texture_id;
    int normal_texture_id;
    int specular_texture_id;
    int emission_texture_id;
    float roughness;
    float metallic;
    float emission_strength;
} Theron_V22_Material;

typedef struct {
    Theron_V22_ShapeType    type;
    int                     texture_id;
    int                     normal_map_id;
    int                     material_id;
    uint8_t                 color_tint[4];
    Theron_V22_LightingMode lighting_mode;
    float                   height_cm;
    float                   width_cm;
    float                   depth_cm;
    int                     vertical_flip;
} Theron_V22_ShapeParams;

typedef enum {
    THERON_V22_WALL_VARIANT_D3_LEFT   = 0,
    THERON_V22_WALL_VARIANT_D3_RIGHT  = 1,
    THERON_V22_WALL_VARIANT_D3_CENTER = 2,
    THERON_V22_WALL_VARIANT_D2_LEFT   = 3,
    THERON_V22_WALL_VARIANT_D2_RIGHT  = 4,
    THERON_V22_WALL_VARIANT_D2_CENTER = 5,
    THERON_V22_WALL_VARIANT_D1_LEFT   = 6,
    THERON_V22_WALL_VARIANT_D1_RIGHT  = 7,
    THERON_V22_WALL_VARIANT_D1_CENTER = 8,
    THERON_V22_WALL_VARIANT_D0_LEFT   = 9,
    THERON_V22_WALL_VARIANT_D0_RIGHT  = 10,
    THERON_V22_WALL_VARIANT_D0_CENTER = 11,
    THERON_V22_WALL_VARIANT_DOOR      = 12,
    THERON_V22_WALL_VARIANT_SECRET    = 13, /* Theron-only */
    THERON_V22_WALL_VARIANT_COUNT
} Theron_V22_WallVariant;

typedef struct {
    int base_texture_id;
    int normal_map_id;
    float normal_strength;
    float corner_bevel;
    int corner_style;
    int door_frame_present;
    int inscription_slot;
    int flipped;
} Theron_V22_WallShape;

typedef enum {
    THERON_V22_FLOOR_TILE_PLAIN   = 0,
    THERON_V22_FLOOR_TILE_CRACKED = 1,
    THERON_V22_FLOOR_TILE_MOSSY   = 2,
    THERON_V22_FLOOR_TILE_COUNT
} Theron_V22_FloorTilePattern;

typedef enum {
    THERON_FLOOR_SHAPE_PLAIN       = 0,
    THERON_FLOOR_SHAPE_CRACKED     = 1,
    THERON_FLOOR_SHAPE_MOSSY       = 2,
    THERON_FLOOR_SHAPE_PIT         = 3,
    THERON_FLOOR_SHAPE_STAIRS_UP   = 4,
    THERON_FLOOR_SHAPE_STAIRS_DOWN = 5,
    THERON_FLOOR_SHAPE_FLOODED     = 6, /* Theron-only */
    THERON_FLOOR_SHAPE_COUNT
} Theron_FloorShapeType;

typedef struct {
    int base_texture_id;
    int normal_map_id;
    Theron_V22_FloorTilePattern tile_pattern;
    int pit_present;
    int flooded_present;        /* Theron-only */
    int stairs_present;
    int stairs_direction;
    float depth_offset;
    float ao_strength;
    float roughness;
} Theron_V22_FloorShape;

Theron_V22_ShapeParams theron_v22_shape_for_cell(int theron_cell_type,
                                                 int view_direction,
                                                 int depth,
                                                 int lateral);

Theron_V22_ShapeParams theron_v22_shape_for_view_square(int view_square,
                                                         int element,
                                                         int direction);

Theron_V22_WallShape  theron_v22_wall_shape_get(Theron_V22_WallVariant variant);
Theron_V22_FloorShape theron_v22_floor_shape_get(int theron_cell_type, int view_direction);
const Theron_V22_Material* theron_v22_material_get(int material_id);
int theron_v22_material_count(void);

Theron_V22_ShapeParams theron_v22_shape_for_teleporter(int active);
Theron_V22_ShapeParams theron_v22_shape_for_alarm(int ringing);
Theron_V22_ShapeParams theron_v22_shape_for_secret_door(int open_progress);
Theron_V22_ShapeParams theron_v22_shape_for_lit_torch(int torch_index);

void theron_v22_shapes_init(void);
const char* theron_v22_shapes_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V22_SHAPES_H */
