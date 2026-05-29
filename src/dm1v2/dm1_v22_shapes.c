/*
 * dm1_v22_shapes.c — DM1 V2.2 Shape System Implementation
 *
 * V2.2 shape system: bridges dungeon cell data to modern renderer.
 *
 * Phase contract: this module defines the shape hierarchy and bridge
 * functions. Actual texture loading, material instantiation, and GPU
 * rendering is deferred to the asset pipeline agent.
 *
 * Source-lock anchors:
 *   ReDMCSB DUNGEON.C:2238-2246 square type → wall/corridor/door/stairs
 *   ReDMCSB DEFS.H:922-941 M034_SQUARE_TYPE enumeration
 *   ReDMCSB DUNVIEW.C:6697-6816 composition draw order
 *   ReDMCSB DUNGEON.C:35-44 direction step tables
 */

#include "dm1_v22_shapes.h"
#include "dm1_v2_viewport_renderer_pc34.h"
#include <string.h>

/* ── Module state ─────────────────────────────────────────────────── */

static int g_shapes_initialized = 0;

/* ── Material library ─────────────────────────────────────────────── */

/* Builtin materials. The asset pipeline agent will add more
 * (loaded from textures/material definition files) but we provide
 * defaults here so the renderer always has something to bind. */
enum { M11_V22_MAT_STONE_WALL = 0, M11_V22_MAT_STONE_FLOOR, M11_V22_MAT_DOOR,
       M11_V22_MAT_METAL, M11_V22_MAT_MAGICAL, M11_V22_MAT_CRYSTAL,
       M11_V22_MAT_COUNT };

static const M11_V22_Material g_builtin_materials[M11_V22_MAT_COUNT] = {
    [M11_V22_MAT_STONE_WALL] = {
        .diffuse_texture_id = 0,
        .normal_texture_id  = 0,
        .specular_texture_id = 0,
        .emission_texture_id = 0,
        .roughness = 0.85f,
        .metallic = 0.0f,
        .emission_strength = 0.0f
    },
    [M11_V22_MAT_STONE_FLOOR] = {
        .diffuse_texture_id = 0,
        .normal_texture_id  = 0,
        .specular_texture_id = 0,
        .emission_texture_id = 0,
        .roughness = 0.90f,
        .metallic = 0.0f,
        .emission_strength = 0.0f
    },
    [M11_V22_MAT_DOOR] = {
        .diffuse_texture_id = 0,
        .normal_texture_id  = 0,
        .specular_texture_id = 0,
        .emission_texture_id = 0,
        .roughness = 0.60f,
        .metallic = 0.15f,
        .emission_strength = 0.0f
    },
    [M11_V22_MAT_METAL] = {
        .diffuse_texture_id = 0,
        .normal_texture_id  = 0,
        .specular_texture_id = 0,
        .emission_texture_id = 0,
        .roughness = 0.30f,
        .metallic = 0.90f,
        .emission_strength = 0.0f
    },
    [M11_V22_MAT_MAGICAL] = {
        .diffuse_texture_id = 0,
        .normal_texture_id  = 0,
        .specular_texture_id = 0,
        .emission_texture_id = 0,
        .roughness = 0.50f,
        .metallic = 0.0f,
        .emission_strength = 1.0f
    },
    [M11_V22_MAT_CRYSTAL] = {
        .diffuse_texture_id = 0,
        .normal_texture_id  = 0,
        .specular_texture_id = 0,
        .emission_texture_id = 0,
        .roughness = 0.10f,
        .metallic = 0.0f,
        .emission_strength = 0.80f
    }
};

/* ── Wall shape variants ──────────────────────────────────────────── */

/* All wall variants use placeholder texture IDs.
 * The asset pipeline agent will wire real textures. */
static const M11_V22_WallShape g_wall_shapes[M11_V22_WALL_VARIANT_COUNT] = {
    /* D3 left — farthest depth, left lateral */
    [M11_V22_WALL_VARIANT_D3_LEFT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.5f, .roughness = 0.85f, .ao_strength = 0.8f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 0
    },
    /* D3 right */
    [M11_V22_WALL_VARIANT_D3_RIGHT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.5f, .roughness = 0.85f, .ao_strength = 0.8f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 1
    },
    /* D3 center */
    [M11_V22_WALL_VARIANT_D3_CENTER] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.6f, .roughness = 0.85f, .ao_strength = 0.9f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 1, .flipped = 0
    },
    /* D2 left */
    [M11_V22_WALL_VARIANT_D2_LEFT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.6f, .roughness = 0.80f, .ao_strength = 0.7f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 0
    },
    /* D2 right */
    [M11_V22_WALL_VARIANT_D2_RIGHT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.6f, .roughness = 0.80f, .ao_strength = 0.7f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 1
    },
    /* D2 center */
    [M11_V22_WALL_VARIANT_D2_CENTER] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.7f, .roughness = 0.80f, .ao_strength = 0.8f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 0
    },
    /* D1 left — closest depth walls */
    [M11_V22_WALL_VARIANT_D1_LEFT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.8f, .roughness = 0.75f, .ao_strength = 0.6f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 0
    },
    /* D1 right */
    [M11_V22_WALL_VARIANT_D1_RIGHT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.8f, .roughness = 0.75f, .ao_strength = 0.6f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 1
    },
    /* D1 center */
    [M11_V22_WALL_VARIANT_D1_CENTER] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.9f, .roughness = 0.75f, .ao_strength = 0.7f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 1, .inscription_slot = 0, .flipped = 0
    },
    /* D0 left — immediate proximity */
    [M11_V22_WALL_VARIANT_D0_LEFT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 1.0f, .roughness = 0.70f, .ao_strength = 0.5f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 0
    },
    /* D0 right */
    [M11_V22_WALL_VARIANT_D0_RIGHT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 1.0f, .roughness = 0.70f, .ao_strength = 0.5f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 1
    },
    /* D0 center */
    [M11_V22_WALL_VARIANT_D0_CENTER] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 1.0f, .roughness = 0.70f, .ao_strength = 0.5f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 0, .inscription_slot = 0, .flipped = 0
    },
    /* Door frame */
    [M11_V22_WALL_VARIANT_DOOR] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .normal_strength = 0.9f, .roughness = 0.60f, .ao_strength = 0.4f,
        .depth_offset = 0.0f, .corner_bevel = 0.0f,
        .corner_style = 0, .door_frame_present = 1, .inscription_slot = 0, .flipped = 0
    }
};

/* ── Floor shape library ──────────────────────────────────────────── */

static const M11_V22_FloorShape g_floor_shapes[FLOOR_SHAPE_COUNT] = {
    [FLOOR_SHAPE_PLAIN] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .tile_pattern = M11_V22_FLOOR_TILE_PLAIN,
        .pit_present = 0, .stairs_present = 0, .stairs_direction = 0,
        .depth_offset = 0.0f, .ao_strength = 0.9f, .roughness = 0.90f
    },
    [FLOOR_SHAPE_CRACKED] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .tile_pattern = M11_V22_FLOOR_TILE_CRACKED,
        .pit_present = 0, .stairs_present = 0, .stairs_direction = 0,
        .depth_offset = 0.0f, .ao_strength = 0.85f, .roughness = 0.90f
    },
    [FLOOR_SHAPE_MOSSY] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .tile_pattern = M11_V22_FLOOR_TILE_MOSSY,
        .pit_present = 0, .stairs_present = 0, .stairs_direction = 0,
        .depth_offset = 0.0f, .ao_strength = 0.80f, .roughness = 0.95f
    },
    [FLOOR_SHAPE_PIT] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .tile_pattern = M11_V22_FLOOR_TILE_PLAIN,
        .pit_present = 1, .stairs_present = 0, .stairs_direction = 0,
        .depth_offset = -20.0f, .ao_strength = 1.0f, .roughness = 1.0f
    },
    [FLOOR_SHAPE_STAIRS_UP] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .tile_pattern = M11_V22_FLOOR_TILE_PLAIN,
        .pit_present = 0, .stairs_present = 1, .stairs_direction = 0,
        .depth_offset = 0.0f, .ao_strength = 0.8f, .roughness = 0.85f
    },
    [FLOOR_SHAPE_STAIRS_DOWN] = {
        .base_texture_id = 0, .normal_map_id = 0,
        .tile_pattern = M11_V22_FLOOR_TILE_PLAIN,
        .pit_present = 0, .stairs_present = 1, .stairs_direction = 1,
        .depth_offset = 0.0f, .ao_strength = 0.8f, .roughness = 0.85f
    }
};

/* ── Default shape params (placeholder) ─────────────────────────── */

static const M11_V22_ShapeParams g_default_shape = {
    .type = M11_V22_SHAPE_WALL_STRAIGHT,
    .texture_id = 0,
    .normal_map_id = 0,
    .material_id = M11_V22_MAT_STONE_WALL,
    .color_tint = { 255, 255, 255, 255 },
    .lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT,
    .height_cm = 240.0f,
    .width_cm = 120.0f,
    .depth_cm = 20.0f,
    .vertical_flip = 0,
    .depth_offset = 0.0f,
    .ao_strength = 0.8f
};

/* ── Shape-to-cell bridge ────────────────────────────────────────── */

/* Map view square + lateral to wall variant.
 * Source-lock: DUNVIEW.C:8490-8542 composition order. */
static M11_V22_WallVariant wall_variant_for_square(int view_square, int lateral) {
    switch (view_square) {
        case DM1_V2_VIEW_SQUARE_D3L: return M11_V22_WALL_VARIANT_D3_LEFT;
        case DM1_V2_VIEW_SQUARE_D3R: return M11_V22_WALL_VARIANT_D3_RIGHT;
        case DM1_V2_VIEW_SQUARE_D3C: return M11_V22_WALL_VARIANT_D3_CENTER;
        case DM1_V2_VIEW_SQUARE_D2L: return M11_V22_WALL_VARIANT_D2_LEFT;
        case DM1_V2_VIEW_SQUARE_D2R: return M11_V22_WALL_VARIANT_D2_RIGHT;
        case DM1_V2_VIEW_SQUARE_D2C: return M11_V22_WALL_VARIANT_D2_CENTER;
        case DM1_V2_VIEW_SQUARE_D1L: return M11_V22_WALL_VARIANT_D1_LEFT;
        case DM1_V2_VIEW_SQUARE_D1R: return M11_V22_WALL_VARIANT_D1_RIGHT;
        case DM1_V2_VIEW_SQUARE_D1C: return M11_V22_WALL_VARIANT_D1_CENTER;
        case DM1_V2_VIEW_SQUARE_D0L: return M11_V22_WALL_VARIANT_D0_LEFT;
        case DM1_V2_VIEW_SQUARE_D0R: return M11_V22_WALL_VARIANT_D0_RIGHT;
        case DM1_V2_VIEW_SQUARE_D0C: return M11_V22_WALL_VARIANT_D0_CENTER;
        default: return M11_V22_WALL_VARIANT_D3_CENTER;
    }
}

M11_V22_WallShape m11_v22_wall_shape_get(M11_V22_WallVariant variant) {
    if (variant < 0 || variant >= M11_V22_WALL_VARIANT_COUNT)
        variant = M11_V22_WALL_VARIANT_D3_CENTER;
    return g_wall_shapes[variant];
}

M11_V22_FloorShape m11_v22_floor_shape_get(int dungeon_cell_type, int view_direction) {
    (void)view_direction;
    int type = (dungeon_cell_type >> 5) & 7;
    switch (type) {
        case 2: /* pit */
            return g_floor_shapes[FLOOR_SHAPE_PIT];
        case 3: /* stairs */
            return (dungeon_cell_type & 0x08) ? g_floor_shapes[FLOOR_SHAPE_STAIRS_DOWN]
                                               : g_floor_shapes[FLOOR_SHAPE_STAIRS_UP];
        default:
            return g_floor_shapes[FLOOR_SHAPE_PLAIN];
    }
}

const M11_V22_Material* m11_v22_material_get(int material_id) {
    if (material_id < 0 || material_id >= M11_V22_MAT_COUNT)
        return &g_builtin_materials[0];
    return &g_builtin_materials[material_id];
}

int m11_v22_material_count(void) {
    return M11_V22_MAT_COUNT;
}

/* Shape params builder from wall shape variant */
static M11_V22_ShapeParams params_from_wall(const M11_V22_WallShape* ws,
                                            M11_V22_ShapeType type,
                                            M11_V22_LightingMode mode) {
    M11_V22_ShapeParams p = g_default_shape;
    if (!ws) return p;
    p.type = type;
    p.texture_id = ws->base_texture_id;
    p.normal_map_id = ws->normal_map_id;
    p.material_id = (ws->roughness > 0.7f) ? M11_V22_MAT_STONE_WALL
                                           : M11_V22_MAT_METAL;
    p.lighting_mode = mode;
    p.ao_strength = ws->ao_strength;
    p.depth_offset = ws->depth_offset;
    p.vertical_flip = ws->flipped;
    if (ws->door_frame_present) {
        p.type = M11_V22_SHAPE_WALL_DOORWAY;
        p.material_id = M11_V22_MAT_DOOR;
    }
    if (ws->inscription_slot) {
        p.type = M11_V22_SHAPE_WALL_INSCRIPTION;
    }
    return p;
}

M11_V22_ShapeParams m11_v22_shape_for_cell(int dungeon_cell_type,
                                           int view_direction,
                                           int depth,
                                           int lateral) {
    (void)view_direction;
    M11_V22_ShapeParams p = g_default_shape;
    int type = (dungeon_cell_type >> 5) & 7;

    if (type == 0) { /* wall */
        /* Map depth+lateral to wall variant */
        int view_square = dm1_v2_vp_square_id(depth, lateral);
        M11_V22_WallVariant variant = wall_variant_for_square(view_square, lateral);
        M11_V22_WallShape ws = m11_v22_wall_shape_get(variant);
        p = params_from_wall(&ws, M11_V22_SHAPE_WALL_STRAIGHT, M11_V22_LIGHT_DUNGEON_AMBIENT);
    } else if (type == 1) { /* corridor */
        p.type = M11_V22_SHAPE_FLOOR_PLAIN;
        p.material_id = M11_V22_MAT_STONE_FLOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (type == 2) { /* pit */
        p.type = M11_V22_SHAPE_FLOOR_PIT;
        p.material_id = M11_V22_MAT_STONE_FLOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (type == 3) { /* stairs */
        int stairs_dir = (dungeon_cell_type >> 3) & 1;
        p.type = stairs_dir ? M11_V22_SHAPE_FLOOR_STAIRS_DOWN : M11_V22_SHAPE_FLOOR_STAIRS_UP;
        p.material_id = M11_V22_MAT_STONE_FLOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (type == 4) { /* door */
        p.type = M11_V22_SHAPE_WALL_DOORWAY;
        p.material_id = M11_V22_MAT_DOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (type == 5) { /* teleporter */
        p.type = M11_V22_SHAPE_FIELD_TELEPORTER;
        p.material_id = M11_V22_MAT_MAGICAL;
        p.lighting_mode = M11_V22_LIGHT_MAGICAL_GLOW;
        p.color_tint[0] = 200; p.color_tint[1] = 100; p.color_tint[2] = 255; p.color_tint[3] = 255;
    } else if (type == 6) { /* fake wall */
        p.type = M11_V22_SHAPE_WALL_STRAIGHT;
        p.material_id = M11_V22_MAT_STONE_WALL;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    }

    return p;
}

M11_V22_ShapeParams m11_v22_shape_for_view_square(int view_square,
                                                  int element,
                                                  int direction) {
    (void)direction;
    M11_V22_ShapeParams p = g_default_shape;

    if (element == DM1_V2_ELEMENT_WALL) {
        M11_V22_WallVariant variant = wall_variant_for_square(view_square, 0);
        M11_V22_WallShape ws = m11_v22_wall_shape_get(variant);
        p = params_from_wall(&ws, M11_V22_SHAPE_WALL_STRAIGHT, M11_V22_LIGHT_DUNGEON_AMBIENT);
    } else if (element == DM1_V2_ELEMENT_DOOR_FRONT) {
        p.type = M11_V22_SHAPE_WALL_DOORWAY;
        p.material_id = M11_V22_MAT_DOOR;
        p.lighting_mode = M11_V22_LIGHT_TORCH_LIT;
    } else if (element == DM1_V2_ELEMENT_DOOR_SIDE) {
        p.type = M11_V22_SHAPE_WALL_STRAIGHT;
        p.material_id = M11_V22_MAT_DOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (element == DM1_V2_ELEMENT_STAIRS_FRONT) {
        p.type = M11_V22_SHAPE_FLOOR_STAIRS_UP;
        p.material_id = M11_V22_MAT_STONE_FLOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (element == DM1_V2_ELEMENT_STAIRS_SIDE) {
        p.type = M11_V22_SHAPE_WALL_STRAIGHT;
        p.material_id = M11_V22_MAT_STONE_WALL;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (element == DM1_V2_ELEMENT_PIT) {
        p.type = M11_V22_SHAPE_FLOOR_PIT;
        p.material_id = M11_V22_MAT_STONE_FLOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    } else if (element == DM1_V2_ELEMENT_TELEPORTER) {
        p.type = M11_V22_SHAPE_FIELD_TELEPORTER;
        p.material_id = M11_V22_MAT_MAGICAL;
        p.lighting_mode = M11_V22_LIGHT_MAGICAL_GLOW;
    } else if (element == DM1_V2_ELEMENT_CORRIDOR) {
        p.type = M11_V22_SHAPE_FLOOR_PLAIN;
        p.material_id = M11_V22_MAT_STONE_FLOOR;
        p.lighting_mode = M11_V22_LIGHT_DUNGEON_AMBIENT;
    }

    return p;
}

/* ── Initialization ───────────────────────────────────────────────── */

void m11_v22_shapes_init(void) {
    if (g_shapes_initialized) return;
    g_shapes_initialized = 1;
}

const char* m11_v22_shapes_source_evidence(void) {
    return
        "DM1 V2.2 Shape System — parameterized dungeon geometry for modern rendering\n"
        "\n"
        "Source-lock anchors:\n"
        "  DUNGEON.C:2238-2246  square type decode (wall/corridor/door/stairs/pit/teleporter)\n"
        "  DEFS.H:922-941        M034_SQUARE_TYPE enumeration (type in bits 5-7 of square byte)\n"
        "  DUNVIEW.C:6697-6816   D3L/D3R/D3C wall draw and occlusion\n"
        "  DUNVIEW.C:6721-6816   D1C door-front floor ornament, back objects, door, front objects\n"
        "  DUNVIEW.C:6816-6827   D0L/D0R/D0C pit draw\n"
        "  DUNVIEW.C:6828        D0 field draw (teleporter/fluxcage)\n"
        "  DUNGEON.C:35-44        direction step tables for square coordinate mapping\n"
        "  DUNVIEW.C:8490-8542   viewport composition square visitation order (D3→D2→D1→D0)\n"
        "\n"
        "Design:\n"
        "  dungeon_cell (raw square byte) → type (bits 5-7) + aspect (bit 3)\n"
        "  type + view_direction → M11_V22_ShapeType via m11_v22_shape_for_cell()\n"
        "  M11_V22_ShapeType + M11_V22_ShapeParams → renderer batch draw call\n"
        "\n"
        "Wall variants (D3/D2/D1/D0 × L/C/R) carry depth-specific:\n"
        "  - normal_strength (deeper walls softer normals)\n"
        "  - ao_strength (deeper walls stronger AO)\n"
        "  - roughness (D0 walls slightly smoother from proximity)\n"
        "  - flipped (alternating wall bitmap per DUNGEON.C:1371 parity rule)\n"
        "  - inscription_slot / door_frame_present (conditional on cell content)\n"
        "\n"
        "Phase contract: placeholder textures/materials for now; asset pipeline\n"
        "agent will wire real texture IDs and PBR material parameters.\n";
}