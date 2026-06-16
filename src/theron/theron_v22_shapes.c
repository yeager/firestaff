/*
 * theron_v22_shapes.c — Theron V2.2 Modern Shape System
 *
 * See include/theron_v22_shapes.h for the design contract, source-lock
 * references, and the Theron-vs-DM1/CSB difference list.
 *
 * The 4x3 (4 depth x 3 lateral) layout matches DM1. The shape
 * bucketing uses the same THERON_SQUARE_* enum (mirrors DM1's M034)
 * and emits the same wall variant grid (D3L/D3R/D3C, ..., D0L/D0R/D0C,
 * DOOR, SECRET) with SECRET added for Theron's secret-door passage
 * mechanic. Theron-specific shapes (TELEPORTER, ALARM, FLOODED,
 * LIT_TORCH) are added on top of the DM1/CSB set.
 */

#include "theron_v22_shapes.h"
#include "theron_v1_world.h"

#include <string.h>

/* ── Default materials (Theron-specific palette) ─────────────────── */

static const Theron_V22_Material g_default_materials[] = {
    /* 0: Plain stone — dungeon walls (Theron uses 4bpp = 16 colors) */
    { 0, 0, 0, 0, 0.85f, 0.0f, 0.0f },
    /* 1: Mossy stone */
    { 1, 1, 0, 0, 0.70f, 0.0f, 0.0f },
    /* 2: Cracked stone */
    { 2, 2, 0, 0, 0.95f, 0.0f, 0.0f },
    /* 3: Iron-bound door */
    { 3, 3, 3, 0, 0.35f, 0.85f, 0.0f },
    /* 4: Pit darkness — slight emission (the void glow) */
    { 0, 0, 0, 4, 0.0f, 0.0f, 0.15f },
    /* 5: Teleporter warp — high emission, teal */
    { 5, 0, 0, 5, 0.20f, 0.10f, 0.80f },
    /* 6: Alarm pulse — red emission, alarm pulse glow */
    { 6, 0, 0, 6, 0.30f, 0.05f, 0.70f },
    /* 7: Flooded stone — wet, slightly reflective */
    { 7, 7, 7, 0, 0.40f, 0.20f, 0.0f },
    /* 8: Secret door — aged wood, hidden */
    { 8, 0, 0, 0, 0.80f, 0.0f, 0.0f },
    /* 9: Lit torch — hot emission, orange */
    { 9, 0, 0, 9, 0.50f, 0.10f, 0.90f },
    /* 10: UI chrome */
    { 10, 0, 0, 10, 0.60f, 0.10f, 0.05f },
};

#define THERON_V22_DEFAULT_MATERIAL_COUNT \
    (int)(sizeof(g_default_materials) / sizeof(g_default_materials[0]))

/* ── Wall shape variants (4x3) ─────────────────────────────────── */

static const Theron_V22_WallShape g_wall_shapes[THERON_V22_WALL_VARIANT_COUNT] = {
    [THERON_V22_WALL_VARIANT_D3_LEFT]   = { 0, 0, 0.8f, 0.02f, 1, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D3_RIGHT]  = { 0, 0, 0.8f, 0.02f, 1, 0, 0, 1 },
    [THERON_V22_WALL_VARIANT_D3_CENTER] = { 0, 0, 0.8f, 0.02f, 1, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D2_LEFT]   = { 0, 0, 0.9f, 0.03f, 1, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D2_RIGHT]  = { 0, 0, 0.9f, 0.03f, 1, 0, 0, 1 },
    [THERON_V22_WALL_VARIANT_D2_CENTER] = { 0, 0, 0.9f, 0.03f, 1, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D1_LEFT]   = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D1_RIGHT]  = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 1 },
    [THERON_V22_WALL_VARIANT_D1_CENTER] = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D0_LEFT]   = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_D0_RIGHT]  = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 1 },
    [THERON_V22_WALL_VARIANT_D0_CENTER] = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 0 },
    [THERON_V22_WALL_VARIANT_DOOR]      = { 3, 3, 0.6f, 0.05f, 0, 1, 0, 0 },
    [THERON_V22_WALL_VARIANT_SECRET]    = { 8, 0, 0.7f, 0.04f, 0, 0, 0, 0 },
};

/* ── Floor shape variants (with FLOODED) ─────────────────────────── */

static const Theron_V22_FloorShape g_floor_shapes[THERON_FLOOR_SHAPE_COUNT] = {
    [THERON_FLOOR_SHAPE_PLAIN]       = { 0, 0, THERON_V22_FLOOR_TILE_PLAIN,   0, 0, 0, 0, 0.0f, 0.6f, 0.85f },
    [THERON_FLOOR_SHAPE_CRACKED]     = { 2, 2, THERON_V22_FLOOR_TILE_CRACKED, 0, 0, 0, 0, 0.0f, 0.8f, 0.95f },
    [THERON_FLOOR_SHAPE_MOSSY]       = { 1, 1, THERON_V22_FLOOR_TILE_MOSSY,   0, 0, 0, 0, 0.0f, 0.7f, 0.70f },
    [THERON_FLOOR_SHAPE_PIT]         = { 0, 0, THERON_V22_FLOOR_TILE_PLAIN,   1, 0, 0, 0, -0.5f, 0.0f, 0.0f },
    [THERON_FLOOR_SHAPE_STAIRS_UP]   = { 0, 0, THERON_V22_FLOOR_TILE_PLAIN,   0, 0, 1, 0, 0.0f, 0.7f, 0.80f },
    [THERON_FLOOR_SHAPE_STAIRS_DOWN] = { 0, 0, THERON_V22_FLOOR_TILE_PLAIN,   0, 0, 1, 1, 0.0f, 0.7f, 0.80f },
    [THERON_FLOOR_SHAPE_FLOODED]     = { 7, 7, THERON_V22_FLOOR_TILE_PLAIN,   0, 1, 0, 0, 0.0f, 0.4f, 0.50f },
};

/* ── Shape type derivation from Theron cell type ────────────────── */

/* Theron uses its own square enum (THERON_SQUARE_* in
 * include/theron_v1_world.h) but the conceptual set matches
 * DM1's M034. For V2.2 we bucket them into the same categories. */
static Theron_V22_ShapeType theron_v22_shape_type_for_cell(int theron_cell_type) {
    /* Lower 4 bits = THERON_SQUARE_*; higher bits = door/stairs/pit
     * qualifiers. */
    int base = theron_cell_type & 0x0F;
    int flags = theron_cell_type & 0xF0;

    if (flags & 0x40) return THERON_V22_SHAPE_FLOOR_PIT;
    if (flags & 0x20) return THERON_V22_SHAPE_WALL_DOORWAY;
    if (flags & 0x10) {
        return (flags & 0x01) ? THERON_V22_SHAPE_FLOOR_STAIRS_DOWN
                              : THERON_V22_SHAPE_FLOOR_STAIRS_UP;
    }

    switch (base) {
        case THERON_SQUARE_WALL:        return THERON_V22_SHAPE_WALL_STRAIGHT;
        case THERON_SQUARE_FLOOR:       return THERON_V22_SHAPE_FLOOR_PLAIN;
        case THERON_SQUARE_PIT:         return THERON_V22_SHAPE_FLOOR_PIT;
        case THERON_SQUARE_STAIRS_UP:   return THERON_V22_SHAPE_FLOOR_STAIRS_UP;
        case THERON_SQUARE_STAIRS_DOWN: return THERON_V22_SHAPE_FLOOR_STAIRS_DOWN;
        case THERON_SQUARE_DOOR:        return THERON_V22_SHAPE_WALL_DOORWAY;
        case THERON_SQUARE_TELEPORTER:  return THERON_V22_SHAPE_FIELD_TELEPORTER;
        case THERON_SQUARE_ALARM:       return THERON_V22_SHAPE_FIELD_ALARM;
        case THERON_SQUARE_SECRET:      return THERON_V22_SHAPE_SECRET_DOOR;
        default:                        return THERON_V22_SHAPE_FLOOR_PLAIN;
    }
}

/* ── Material selection by shape type ───────────────────────────── */

static int theron_v22_material_id_for_shape(Theron_V22_ShapeType type) {
    switch (type) {
        case THERON_V22_SHAPE_WALL_STRAIGHT:      return 0;
        case THERON_V22_SHAPE_WALL_CORNER_INNER:  return 0;
        case THERON_V22_SHAPE_WALL_CORNER_OUTER:  return 0;
        case THERON_V22_SHAPE_WALL_ALCOVE:        return 1;
        case THERON_V22_SHAPE_WALL_INSCRIPTION:   return 2;
        case THERON_V22_SHAPE_FLOOR_PLAIN:        return 0;
        case THERON_V22_SHAPE_FLOOR_CRACKED:      return 2;
        case THERON_V22_SHAPE_FLOOR_MOSSY:        return 1;
        case THERON_V22_SHAPE_FLOOR_PIT:          return 4;
        case THERON_V22_SHAPE_FLOOR_STAIRS_UP:    return 0;
        case THERON_V22_SHAPE_FLOOR_STAIRS_DOWN:  return 0;
        case THERON_V22_SHAPE_FLOOR_DOOR:         return 3;
        case THERON_V22_SHAPE_CEILING_PLAIN:      return 0;
        case THERON_V22_SHAPE_CEILING_VAULTED:    return 0;
        case THERON_V22_SHAPE_FIELD_TELEPORTER:   return 5;
        case THERON_V22_SHAPE_FIELD_ALARM:        return 6;
        case THERON_V22_SHAPE_FIELD_FLUXCAGE:     return 5;
        case THERON_V22_SHAPE_FIELD_EXPLOSION:    return 4;
        case THERON_V22_SHAPE_SECRET_DOOR:        return 8;
        case THERON_V22_SHAPE_FLOODED:            return 7;
        case THERON_V22_SHAPE_LIT_TORCH:          return 9;
        case THERON_V22_SHAPE_TELEPORTER_TARGET:  return 5;
        case THERON_V22_SHAPE_UI_CHROME:          return 10;
        default:                                    return 0;
    }
}

/* ── Public bridge functions ─────────────────────────────────────── */

Theron_V22_ShapeParams theron_v22_shape_for_cell(int theron_cell_type,
                                                 int view_direction,
                                                 int depth,
                                                 int lateral) {
    Theron_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));

    /* Clamp Theron's 4x3 coords: depth 0..3, lateral -1..+1. */
    if (depth < 0) depth = 0;
    if (depth > 3) depth = 3;
    if (lateral < -1) lateral = -1;
    if (lateral > 1)  lateral = 1;
    if (view_direction < 0) view_direction = 0;
    if (view_direction > 3) view_direction = 3;

    p.type = theron_v22_shape_type_for_cell(theron_cell_type);
    p.material_id = theron_v22_material_id_for_shape(p.type);
    p.texture_id = p.material_id;
    p.normal_map_id = p.material_id;
    p.color_tint[0] = 255; p.color_tint[1] = 255;
    p.color_tint[2] = 255; p.color_tint[3] = 255;
    p.lighting_mode = THERON_V22_LIGHT_DUNGEON_AMBIENT;
    p.height_cm = 250.0f;
    p.width_cm  = 100.0f * (4 - depth);
    p.depth_cm  = 100.0f * (4 - depth);
    p.vertical_flip = 0;
    (void)lateral;
    return p;
}

Theron_V22_ShapeParams theron_v22_shape_for_view_square(int view_square,
                                                         int element,
                                                         int direction) {
    /* view_square: 0..11 (4x3 grid). element: 0=floor, 1=ceiling, 2=wall. */
    int theron_cell_type;
    int depth = view_square / 3;            /* 0=D0 (closest) */
    int lateral = (view_square % 3) - 1;

    if (element == 1) theron_cell_type = THERON_SQUARE_WALL;       /* ceiling plain */
    else if (element == 2) theron_cell_type = THERON_SQUARE_WALL;  /* wall */
    else theron_cell_type = THERON_SQUARE_FLOOR;
    (void)depth; (void)lateral;
    return theron_v22_shape_for_cell(theron_cell_type, direction, depth, lateral);
}

Theron_V22_WallShape theron_v22_wall_shape_get(Theron_V22_WallVariant variant) {
    if ((int)variant < 0 || (int)variant >= THERON_V22_WALL_VARIANT_COUNT) {
        return g_wall_shapes[THERON_V22_WALL_VARIANT_D0_CENTER];
    }
    return g_wall_shapes[variant];
}

Theron_V22_FloorShape theron_v22_floor_shape_get(int theron_cell_type, int view_direction) {
    int base = theron_cell_type & 0x0F;
    int flags = theron_cell_type & 0xF0;
    if (flags & 0x40) return g_floor_shapes[THERON_FLOOR_SHAPE_PIT];
    if (flags & 0x10) {
        return g_floor_shapes[(flags & 0x01) ? THERON_FLOOR_SHAPE_STAIRS_DOWN
                                            : THERON_FLOOR_SHAPE_STAIRS_UP];
    }
    switch (base) {
        case THERON_SQUARE_FLOOR:       return g_floor_shapes[THERON_FLOOR_SHAPE_PLAIN];
        case THERON_SQUARE_TELEPORTER:  return g_floor_shapes[THERON_FLOOR_SHAPE_FLOODED];
        case THERON_SQUARE_ALARM:       return g_floor_shapes[THERON_FLOOR_SHAPE_CRACKED];
        case THERON_SQUARE_STAIRS_UP:   return g_floor_shapes[THERON_FLOOR_SHAPE_STAIRS_UP];
        case THERON_SQUARE_STAIRS_DOWN: return g_floor_shapes[THERON_FLOOR_SHAPE_STAIRS_DOWN];
        default:                        return g_floor_shapes[THERON_FLOOR_SHAPE_PLAIN];
    }
    (void)view_direction;
}

const Theron_V22_Material* theron_v22_material_get(int material_id) {
    if (material_id < 0 || material_id >= THERON_V22_DEFAULT_MATERIAL_COUNT) {
        return &g_default_materials[0];
    }
    return &g_default_materials[material_id];
}

int theron_v22_material_count(void) {
    return THERON_V22_DEFAULT_MATERIAL_COUNT;
}

/* ── Theron-specific shapes ──────────────────────────────────────── */

Theron_V22_ShapeParams theron_v22_shape_for_teleporter(int active) {
    Theron_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = THERON_V22_SHAPE_FIELD_TELEPORTER;
    p.material_id = 5;                       /* teleporter warp */
    p.texture_id = 5;
    p.normal_map_id = 0;
    /* active 0 = dormant (dim teal), 1 = active (bright teal). */
    p.color_tint[0] = (uint8_t)(active ? 64 : 32);
    p.color_tint[1] = (uint8_t)(active ? 200 : 80);
    p.color_tint[2] = (uint8_t)(active ? 200 : 80);
    p.color_tint[3] = 255;
    p.lighting_mode = THERON_V22_LIGHT_MAGICAL_GLOW;
    p.height_cm = 30.0f;
    p.width_cm  = 80.0f;
    p.depth_cm  = 80.0f;
    p.vertical_flip = 0;
    return p;
}

Theron_V22_ShapeParams theron_v22_shape_for_alarm(int ringing) {
    Theron_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = THERON_V22_SHAPE_FIELD_ALARM;
    p.material_id = 6;                       /* alarm pulse */
    p.texture_id = 6;
    p.normal_map_id = 0;
    /* ringing 0 = silent (dark red), 1 = ringing (bright red pulse). */
    p.color_tint[0] = (uint8_t)(ringing ? 255 : 100);
    p.color_tint[1] = (uint8_t)(ringing ? 32  : 16);
    p.color_tint[2] = (uint8_t)(ringing ? 32  : 16);
    p.color_tint[3] = 255;
    p.lighting_mode = THERON_V22_LIGHT_ALARM_PULSE;
    p.height_cm = 20.0f;
    p.width_cm  = 60.0f;
    p.depth_cm  = 60.0f;
    p.vertical_flip = 0;
    return p;
}

Theron_V22_ShapeParams theron_v22_shape_for_secret_door(int open_progress) {
    Theron_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = THERON_V22_SHAPE_SECRET_DOOR;
    p.material_id = 8;                       /* aged wood */
    p.texture_id = 8;
    p.normal_map_id = 0;
    /* open_progress 0 = hidden, 100 = revealed. */
    p.color_tint[0] = (uint8_t)(40 + (open_progress * 200) / 100);
    p.color_tint[1] = (uint8_t)(40 + (open_progress * 200) / 100);
    p.color_tint[2] = (uint8_t)(40 + (open_progress * 200) / 100);
    p.color_tint[3] = 255;
    p.lighting_mode = THERON_V22_LIGHT_DUNGEON_AMBIENT;
    p.height_cm = 250.0f;
    p.width_cm  = 100.0f;
    p.depth_cm  = 30.0f;
    p.vertical_flip = 0;
    return p;
}

Theron_V22_ShapeParams theron_v22_shape_for_lit_torch(int torch_index) {
    Theron_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = THERON_V22_SHAPE_LIT_TORCH;
    p.material_id = 9;                       /* hot torch */
    p.texture_id = 9;
    p.normal_map_id = 0;
    /* torch_index 0..3 = the four Theron torch slots. */
    p.color_tint[0] = 255;
    p.color_tint[1] = (uint8_t)(180 + (torch_index * 5) % 60);
    p.color_tint[2] = (uint8_t)(40 + (torch_index * 10) % 40);
    p.color_tint[3] = 255;
    p.lighting_mode = THERON_V22_LIGHT_TORCH_LIT;
    p.height_cm = 80.0f;
    p.width_cm  = 20.0f;
    p.depth_cm  = 20.0f;
    p.vertical_flip = 0;
    return p;
}

/* ── Initialization ──────────────────────────────────────────────── */

static int g_theron_v22_shapes_initialized = 0;

void theron_v22_shapes_init(void) {
    if (g_theron_v22_shapes_initialized) return;
    g_theron_v22_shapes_initialized = 1;
}

const char* theron_v22_shapes_source_evidence(void) {
    return
        "Theron V2.2 shape system: 4x3 (4 depth x 3 lateral) layout.\n"
        "  THQUEST.ASM T400 - tile bank loading\n"
        "  THQUEST.ASM T520 - tile selection + viewport blit\n"
        "  THQUEST.ASM T600 - UI overlay zones\n"
        "  THQUEST.ASM T700 - teleporter warp dispatch (Theron-only)\n"
        "  THQUEST.ASM T800 - alarm / alert dispatch\n"
        "  HuC6260/HuC6270 datasheet - VDC/VCE rendering (PC Engine CD)\n"
        "  tqr_v1_phase2_data_formats_H2339.md §7 - tile data format\n"
        "  include/theron_v1_world.h - THERON_SQUARE_* enum (cell type source)\n"
        "Mirror of dm1_v22_shapes + csb_v22_shapes with Theron-only shapes:\n"
        "  THERON_V22_SHAPE_FIELD_ALARM  (T800 alert-all-creatures)\n"
        "  THERON_V22_SHAPE_SECRET_DOOR  (T800 hidden passage)\n"
        "  THERON_V22_SHAPE_FLOODED      (water/flooded squares)\n"
        "  THERON_V22_SHAPE_LIT_TORCH    (4+ torch slots)\n"
        "  THERON_V22_LIGHT_ALARM_PULSE  (red pulse glow)\n"
        "13 wall variants (3x4 + DOOR + SECRET) + 7 floor shapes\n"
        "Materials: 11 builtin (plain, mossy, cracked, iron door, pit,\n"
        "teleporter, alarm, flooded, secret, torch, UI)\n";
}
