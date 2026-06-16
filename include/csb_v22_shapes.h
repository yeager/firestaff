#ifndef FIRESTAFF_CSB_V22_SHAPES_H
#define FIRESTAFF_CSB_V22_SHAPES_H

/*
 * csb_v22_shapes.h — CSB V2.2 Modern Shape System
 *
 * CSB V2.2 ("Modern Graphics") renders CSB's 9-square viewport
 * (3 depth x 3 lateral = 9 cells) using modern parameterized art
 * at 1920x1080. Parallel to dm1_v22_shapes.h; same shape API
 * style (ShapeType, ShapeParams, Material, Wall/Floor) but CSB-
 * specific variants for the 9-square layout and CSB-only shapes
 * (prison door, chaos rune, DSA scroll, Lord Order statue).
 *
 * 9-square layout (CSBWin/Viewport.cpp:7290):
 *   depth D2 (farthest):  L  C  R
 *   depth D1 (middle):    L  C  R
 *   depth D0 (closest):   L  C  R
 * (3 depth x 3 lateral = 9 cells, vs DM1's 4 x 3 = 12 cells)
 *
 * CSB-specific shapes beyond DM1's set:
 *   - CSB_V22_SHAPE_PRISON_DOOR      (entrance sequence)
 *   - CSB_V22_SHAPE_CHAOS_RUNE       (CSB-only, cast markers)
 *   - CSB_V22_SHAPE_DSA_SCROLL       (DSA call surface)
 *   - CSB_V22_SHAPE_LORD_ORDER       (endgame statue)
 *
 * Design contract:
 *   CSB square type + view position -> CSB_V22_ShapeType ->
 *     CSB_V22_ShapeParams -> CSB_V22_WallShape / CSB_V22_FloorShape ->
 *     renderer batch draw
 *
 * Phase: CSB V2.2 shapes are defined here; the actual texture /
 * material loading and GPU rendering is deferred to the CSB V2.2
 * modern asset pipeline. This module is the *selector* and the
 * *parameter book*; the renderer is the consumer.
 *
 * Source-lock references:
 *   - CSBWin/Viewport.cpp:7290  CSB 9-square viewport (7290 lines)
 *   - CSBWin/Chaos.cpp:60-69    DSA call dispatch (chaos magic)
 *   - ReDMCSB DUNGEON.C:35-44   direction step tables (N/E/S/W)
 *   - ReDMCSB ENTRANCE.C         CSB prison door + intro
 *   - ReDMCSB GAMELOOP.C:150-155 V1 tick cadence (CSB shares)
 *   - dm1_v22_shapes.h           mirror reference
 *
 * Module: src/csb/csb_v22_shapes.c
 * Test:   tests/test_csb_v22_shapes_pc34.c
 * Probe:  probes/firestaff_csb_v22_shapes_probe.c
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Shape types ──────────────────────────────────────────────────── */

typedef enum {
    /* ── Wall shapes (mirror of DM1 V2.2 set) ──────────────────── */
    CSB_V22_SHAPE_WALL_STRAIGHT      = 0,
    CSB_V22_SHAPE_WALL_CORNER_INNER  = 1,
    CSB_V22_SHAPE_WALL_CORNER_OUTER  = 2,
    CSB_V22_SHAPE_WALL_DOORWAY       = 3,
    CSB_V22_SHAPE_WALL_ALCOVE        = 4,
    CSB_V22_SHAPE_WALL_INSCRIPTION   = 5,

    /* ── Floor shapes ──────────────────────────────────────────── */
    CSB_V22_SHAPE_FLOOR_PLAIN        = 10,
    CSB_V22_SHAPE_FLOOR_CRACKED      = 11,
    CSB_V22_SHAPE_FLOOR_MOSSY        = 12,
    CSB_V22_SHAPE_FLOOR_PIT          = 13,
    CSB_V22_SHAPE_FLOOR_STAIRS_UP    = 14,
    CSB_V22_SHAPE_FLOOR_STAIRS_DOWN  = 15,
    CSB_V22_SHAPE_FLOOR_DOOR         = 16,

    /* ── Ceiling shapes ────────────────────────────────────────── */
    CSB_V22_SHAPE_CEILING_PLAIN      = 20,
    CSB_V22_SHAPE_CEILING_VAULTED    = 21,

    /* ── Creature / Item shapes ────────────────────────────────── */
    CSB_V22_SHAPE_CREATURE           = 30,
    CSB_V22_SHAPE_CREATURE_PROJECTILE = 31,
    CSB_V22_SHAPE_ITEM               = 40,
    CSB_V22_SHAPE_ITEM_FLOOR         = 41,
    CSB_V22_SHAPE_ITEM_PROJECTILE    = 42,

    /* ── CSB-only field shapes ─────────────────────────────────── */
    CSB_V22_SHAPE_FIELD_TELEPORTER   = 50,
    CSB_V22_SHAPE_FIELD_FLUXCAGE     = 51,
    CSB_V22_SHAPE_FIELD_EXPLOSION    = 52,
    CSB_V22_SHAPE_FIELD_CHAOS_RIFT   = 53, /* CSB-only */

    /* ── CSB-only UI / Chrome shapes ───────────────────────────── */
    CSB_V22_SHAPE_UI_CHROME          = 60,
    CSB_V22_SHAPE_UI_PORTRAIT        = 61,
    CSB_V22_SHAPE_UI_MESSAGE_LOG     = 62,
    CSB_V22_SHAPE_UI_INVENTORY_GRID  = 63,
    CSB_V22_SHAPE_UI_DSA_RUNE        = 64, /* CSB-only chaos runes */

    /* ── CSB-only narrative shapes ─────────────────────────────── */
    CSB_V22_SHAPE_PRISON_DOOR        = 70, /* ENTRANCE.C prison door */
    CSB_V22_SHAPE_DSA_SCROLL         = 71, /* CSBWin/Chaos.cpp:60-69 */
    CSB_V22_SHAPE_LORD_ORDER         = 72, /* Endgame statue */
    CSB_V22_SHAPE_CHAOS_RUNE         = 73, /* Cast markers */

    CSB_V22_SHAPE_COUNT
} CSB_V22_ShapeType;

/* ── Lighting modes (mirror of DM1 V2.2) ────────────────────────── */

typedef enum {
    CSB_V22_LIGHT_FULL_BRIGHT      = 0,
    CSB_V22_LIGHT_DUNGEON_AMBIENT  = 1,
    CSB_V22_LIGHT_TORCH_LIT        = 2,
    CSB_V22_LIGHT_MAGICAL_GLOW     = 3,
    /* CSB-only: chaos magic produces a teal/green dynamic glow. */
    CSB_V22_LIGHT_CHAOS_GLOW       = 4,
    CSB_V22_LIGHT_COUNT
} CSB_V22_LightingMode;

/* ── Material descriptor (mirror of DM1) ─────────────────────────── */

typedef struct {
    int diffuse_texture_id;
    int normal_texture_id;
    int specular_texture_id;
    int emission_texture_id;
    float roughness;
    float metallic;
    float emission_strength;
} CSB_V22_Material;

/* ── Shape parameters (mirror of DM1) ───────────────────────────── */

typedef struct {
    CSB_V22_ShapeType    type;
    int                  texture_id;
    int                  normal_map_id;
    int                  material_id;
    uint8_t              color_tint[4];
    CSB_V22_LightingMode lighting_mode;
    float                height_cm;
    float                width_cm;
    float                depth_cm;
    int                  vertical_flip;
} CSB_V22_ShapeParams;

/* ── Wall shape variant for 9-square (3x3) layout ────────────────── */

typedef enum {
    CSB_V22_WALL_VARIANT_D2_LEFT   = 0,
    CSB_V22_WALL_VARIANT_D2_RIGHT  = 1,
    CSB_V22_WALL_VARIANT_D2_CENTER = 2,
    CSB_V22_WALL_VARIANT_D1_LEFT   = 3,
    CSB_V22_WALL_VARIANT_D1_RIGHT  = 4,
    CSB_V22_WALL_VARIANT_D1_CENTER = 5,
    CSB_V22_WALL_VARIANT_D0_LEFT   = 6,
    CSB_V22_WALL_VARIANT_D0_RIGHT  = 7,
    CSB_V22_WALL_VARIANT_D0_CENTER = 8,
    CSB_V22_WALL_VARIANT_DOOR      = 9,
    CSB_V22_WALL_VARIANT_PRISON    = 10, /* CSB-only */
    CSB_V22_WALL_VARIANT_COUNT
} CSB_V22_WallVariant;

typedef struct {
    int base_texture_id;
    int normal_map_id;
    float normal_strength;
    float corner_bevel;
    int corner_style;
    int door_frame_present;
    int inscription_slot;
    int flipped;
} CSB_V22_WallShape;

typedef enum {
    CSB_V22_FLOOR_TILE_PLAIN   = 0,
    CSB_V22_FLOOR_TILE_CRACKED = 1,
    CSB_V22_FLOOR_TILE_MOSSY   = 2,
    CSB_V22_FLOOR_TILE_COUNT
} CSB_V22_FloorTilePattern;

typedef enum {
    CSB_FLOOR_SHAPE_PLAIN       = 0,
    CSB_FLOOR_SHAPE_CRACKED     = 1,
    CSB_FLOOR_SHAPE_MOSSY       = 2,
    CSB_FLOOR_SHAPE_PIT         = 3,
    CSB_FLOOR_SHAPE_STAIRS_UP   = 4,
    CSB_FLOOR_SHAPE_STAIRS_DOWN = 5,
    CSB_FLOOR_SHAPE_COUNT
} CSB_FloorShapeType;

typedef struct {
    int base_texture_id;
    int normal_map_id;
    CSB_V22_FloorTilePattern tile_pattern;
    int pit_present;
    int stairs_present;
    int stairs_direction;
    float depth_offset;
    float ao_strength;
    float roughness;
} CSB_V22_FloorShape;

/* ── Bridge functions (mirror of DM1 V2.2 API) ──────────────────── */

/* The 9-square bridge: given a CSB square type + view position
 * (depth 0..2, lateral -1/0/+1) + view direction (0..3 N/E/S/W),
 * returns the shape parameters the renderer should batch-draw. */
CSB_V22_ShapeParams csb_v22_shape_for_cell(int csb_cell_type,
                                            int view_direction,
                                            int depth,
                                            int lateral);

/* 9-square specific helper: depth and lateral are explicit. */
CSB_V22_ShapeParams csb_v22_shape_for_view_square(int view_square,
                                                    int element,
                                                    int direction);

CSB_V22_WallShape  csb_v22_wall_shape_get(CSB_V22_WallVariant variant);
CSB_V22_FloorShape csb_v22_floor_shape_get(int csb_cell_type, int view_direction);
const CSB_V22_Material* csb_v22_material_get(int material_id);
int csb_v22_material_count(void);

/* CSB-specific helpers. */
CSB_V22_ShapeParams csb_v22_shape_for_prison_door(int open_progress);
CSB_V22_ShapeParams csb_v22_shape_for_chaos_rune(int rune_index);
CSB_V22_ShapeParams csb_v22_shape_for_dsa_scroll(int scroll_index);

/* ── Initialization ──────────────────────────────────────────────── */

void csb_v22_shapes_init(void);
const char* csb_v22_shapes_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_SHAPES_H */
