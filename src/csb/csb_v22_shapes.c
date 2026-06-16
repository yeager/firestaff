/*
 * csb_v22_shapes.c — CSB V2.2 Modern Shape System
 *
 * See include/csb_v22_shapes.h for the design contract, source-lock
 * references, and the CSB-vs-DM1 difference list.
 *
 * This module is the *shape selector* + *parameter book* for CSB V2.2.
 * The actual texture / material loading and GPU rendering is deferred
 * to the CSB V2.2 modern asset pipeline (not yet authored — the
 * selection API and this book are the V2.2 Phase 8 deliverable for
 * CSB; the renderer is a follow-up).
 */

#include "csb_v22_shapes.h"
#include "csb_v2_presentation_mode_pc34.h"

#include <string.h>

/* ── Default materials (CSB-specific palette) ────────────────────── */

static const CSB_V22_Material g_default_materials[] = {
    /* 0: Plain stone — dungeon walls (no V1 material change) */
    { 0, 0, 0, 0, 0.85f, 0.0f, 0.0f },
    /* 1: Mossy stone — slightly more reflective, less rough */
    { 1, 1, 0, 0, 0.70f, 0.0f, 0.0f },
    /* 2: Cracked stone — high roughness, normal map strong */
    { 2, 2, 0, 0, 0.95f, 0.0f, 0.0f },
    /* 3: Iron-bound door — metallic, low roughness */
    { 3, 3, 3, 0, 0.35f, 0.85f, 0.0f },
    /* 4: Pit darkness — no diffuse, low emission (the void glow) */
    { 0, 0, 0, 4, 0.0f, 0.0f, 0.15f },
    /* 5: Chaos rune — high emission, teal/green */
    { 5, 0, 0, 5, 0.20f, 0.10f, 0.80f },
    /* 6: Prison door iron — metallic, scratched */
    { 6, 6, 6, 0, 0.55f, 0.70f, 0.0f },
    /* 7: Lord Order statue — high specular, polished marble */
    { 7, 7, 7, 0, 0.25f, 0.30f, 0.0f },
    /* 8: DSA scroll — parchment, low metallic, slight emission */
    { 8, 0, 0, 8, 0.90f, 0.0f, 0.10f },
    /* 9: UI chrome — flat, low specular, slight emission for highlight */
    { 9, 0, 0, 9, 0.60f, 0.10f, 0.05f },
};

#define CSB_V22_DEFAULT_MATERIAL_COUNT \
    (int)(sizeof(g_default_materials) / sizeof(g_default_materials[0]))

/* ── Wall shape variants (9-square) ──────────────────────────────── */

static CSB_V22_WallShape make_wall_shape(int base, int normal,
                                          int door_frame, int flipped) {
    CSB_V22_WallShape w;
    memset(&w, 0, sizeof(w));
    w.base_texture_id = base;
    w.normal_map_id = normal;
    w.normal_strength = 0.8f;
    w.corner_bevel = 0.02f;
    w.corner_style = 1; /* beveled */
    w.door_frame_present = door_frame;
    w.inscription_slot = 0;
    w.flipped = flipped;
    return w;
}

static const CSB_V22_WallShape g_wall_shapes[CSB_V22_WALL_VARIANT_COUNT] = {
    [CSB_V22_WALL_VARIANT_D2_LEFT]   = { 0, 0, 0.8f, 0.02f, 1, 0, 0, 0 },
    [CSB_V22_WALL_VARIANT_D2_RIGHT]  = { 0, 0, 0.8f, 0.02f, 1, 0, 0, 1 },
    [CSB_V22_WALL_VARIANT_D2_CENTER] = { 0, 0, 0.8f, 0.02f, 1, 0, 0, 0 },
    [CSB_V22_WALL_VARIANT_D1_LEFT]   = { 0, 0, 0.9f, 0.03f, 1, 0, 0, 0 },
    [CSB_V22_WALL_VARIANT_D1_RIGHT]  = { 0, 0, 0.9f, 0.03f, 1, 0, 0, 1 },
    [CSB_V22_WALL_VARIANT_D1_CENTER] = { 0, 0, 0.9f, 0.03f, 1, 0, 0, 0 },
    [CSB_V22_WALL_VARIANT_D0_LEFT]   = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 0 },
    [CSB_V22_WALL_VARIANT_D0_RIGHT]  = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 1 },
    [CSB_V22_WALL_VARIANT_D0_CENTER] = { 0, 0, 1.0f, 0.04f, 2, 0, 0, 0 },
    [CSB_V22_WALL_VARIANT_DOOR]      = { 3, 3, 0.6f, 0.05f, 0, 1, 0, 0 },
    [CSB_V22_WALL_VARIANT_PRISON]    = { 6, 6, 0.5f, 0.06f, 0, 1, 0, 0 },
};

/* ── Floor shape variants ────────────────────────────────────────── */

static const CSB_V22_FloorShape g_floor_shapes[CSB_FLOOR_SHAPE_COUNT] = {
    [CSB_FLOOR_SHAPE_PLAIN]       = { 0, 0, CSB_V22_FLOOR_TILE_PLAIN,   0, 0, 0, 0.0f, 0.6f, 0.85f },
    [CSB_FLOOR_SHAPE_CRACKED]     = { 2, 2, CSB_V22_FLOOR_TILE_CRACKED, 0, 0, 0, 0.0f, 0.8f, 0.95f },
    [CSB_FLOOR_SHAPE_MOSSY]       = { 1, 1, CSB_V22_FLOOR_TILE_MOSSY,   0, 0, 0, 0.0f, 0.7f, 0.70f },
    [CSB_FLOOR_SHAPE_PIT]         = { 0, 0, CSB_V22_FLOOR_TILE_PLAIN,   1, 0, 0, -0.5f, 0.0f, 0.0f },
    [CSB_FLOOR_SHAPE_STAIRS_UP]   = { 0, 0, CSB_V22_FLOOR_TILE_PLAIN,   0, 1, 0, 0.0f, 0.7f, 0.80f },
    [CSB_FLOOR_SHAPE_STAIRS_DOWN] = { 0, 0, CSB_V22_FLOOR_TILE_PLAIN,   0, 1, 1, 0.0f, 0.7f, 0.80f },
};

/* ── Shape type derivation from CSB cell type ────────────────────── */

/* CSB uses ReDMCSB's M034_SQUARE_TYPE enum (DEFS.H:922). For the V2.2
 * book, we bucket them into the same categories as DM1 V2.2. */
static CSB_V22_ShapeType csb_v22_shape_type_for_cell(int csb_cell_type) {
    /* Lower 4 bits = M034_SQUARE_TYPE; higher bits = door/stairs/pit
     * qualifiers (mirrors DM1's M034). For V2.2 we just bucket. */
    int base = csb_cell_type & 0x0F;
    int flags = csb_cell_type & 0xF0;

    if (flags & 0x40) return CSB_V22_SHAPE_FLOOR_PIT;        /* pit */
    if (flags & 0x20) return CSB_V22_SHAPE_WALL_DOORWAY;     /* door */
    if (flags & 0x10) {
        return (flags & 0x01) ? CSB_V22_SHAPE_FLOOR_STAIRS_DOWN
                              : CSB_V22_SHAPE_FLOOR_STAIRS_UP;
    }

    switch (base) {
        case 0:  return CSB_V22_SHAPE_WALL_STRAIGHT;       /* solid wall */
        case 1:  return CSB_V22_SHAPE_WALL_CORNER_INNER;
        case 2:  return CSB_V22_SHAPE_WALL_CORNER_OUTER;
        case 3:  return CSB_V22_SHAPE_WALL_ALCOVE;
        case 4:  return CSB_V22_SHAPE_FLOOR_PLAIN;
        case 5:  return CSB_V22_SHAPE_FLOOR_CRACKED;
        case 6:  return CSB_V22_SHAPE_FLOOR_MOSSY;
        case 7:  return CSB_V22_SHAPE_CEILING_PLAIN;
        case 8:  return CSB_V22_SHAPE_CEILING_VAULTED;
        case 9:  return CSB_V22_SHAPE_WALL_INSCRIPTION;
        default: return CSB_V22_SHAPE_FLOOR_PLAIN;          /* safe default */
    }
}

/* ── Material selection by cell type ─────────────────────────────── */

static int csb_v22_material_id_for_shape(CSB_V22_ShapeType type) {
    switch (type) {
        case CSB_V22_SHAPE_WALL_STRAIGHT:      return 0;
        case CSB_V22_SHAPE_WALL_CORNER_INNER:  return 0;
        case CSB_V22_SHAPE_WALL_CORNER_OUTER:  return 0;
        case CSB_V22_SHAPE_WALL_ALCOVE:        return 1;
        case CSB_V22_SHAPE_WALL_INSCRIPTION:   return 2;
        case CSB_V22_SHAPE_FLOOR_PLAIN:        return 0;
        case CSB_V22_SHAPE_FLOOR_CRACKED:      return 2;
        case CSB_V22_SHAPE_FLOOR_MOSSY:        return 1;
        case CSB_V22_SHAPE_FLOOR_PIT:          return 4;
        case CSB_V22_SHAPE_FLOOR_STAIRS_UP:    return 0;
        case CSB_V22_SHAPE_FLOOR_STAIRS_DOWN:  return 0;
        case CSB_V22_SHAPE_FLOOR_DOOR:         return 3;
        case CSB_V22_SHAPE_CEILING_PLAIN:      return 0;
        case CSB_V22_SHAPE_CEILING_VAULTED:    return 0;
        case CSB_V22_SHAPE_PRISON_DOOR:        return 6;
        case CSB_V22_SHAPE_CHAOS_RUNE:         return 5;
        case CSB_V22_SHAPE_DSA_SCROLL:         return 8;
        case CSB_V22_SHAPE_LORD_ORDER:         return 7;
        case CSB_V22_SHAPE_UI_CHROME:          return 9;
        default:                                return 0;
    }
}

/* ── Public bridge functions ─────────────────────────────────────── */

CSB_V22_ShapeParams csb_v22_shape_for_cell(int csb_cell_type,
                                            int view_direction,
                                            int depth,
                                            int lateral) {
    CSB_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));

    /* Clamp CSB's 9-square coords. depth 0..2, lateral -1..+1. */
    if (depth < 0) depth = 0;
    if (depth > 2) depth = 2;
    if (lateral < -1) lateral = -1;
    if (lateral > 1)  lateral = 1;
    if (view_direction < 0) view_direction = 0;
    if (view_direction > 3) view_direction = 3;

    p.type = csb_v22_shape_type_for_cell(csb_cell_type);
    p.material_id = csb_v22_material_id_for_shape(p.type);
    p.texture_id = p.material_id;        /* default 1:1 with material */
    p.normal_map_id = p.material_id;
    p.color_tint[0] = 255; p.color_tint[1] = 255;
    p.color_tint[2] = 255; p.color_tint[3] = 255;
    p.lighting_mode = CSB_V22_LIGHT_DUNGEON_AMBIENT;
    p.height_cm = 250.0f;
    p.width_cm  = 100.0f * (3 - depth);    /* narrower for far depth */
    p.depth_cm  = 100.0f * (3 - depth);
    p.vertical_flip = 0;
    (void)lateral;  /* cell type already covers lateral flavour in CSB */
    return p;
}

CSB_V22_ShapeParams csb_v22_shape_for_view_square(int view_square,
                                                    int element,
                                                    int direction) {
    /* view_square: 0..8 (3x3 grid). element: 0=floor, 1=ceiling, 2=wall.
     * direction: 0..3. */
    int csb_cell_type;
    int depth = view_square / 3;            /* 0=D0 (closest) */
    int lateral = (view_square % 3) - 1;    /* -1/0/+1 */

    if (element == 1) csb_cell_type = 7;    /* ceiling plain */
    else if (element == 2) csb_cell_type = 0;  /* wall */
    else csb_cell_type = 4;                /* floor plain */
    (void)depth; (void)lateral;
    return csb_v22_shape_for_cell(csb_cell_type, direction, depth, lateral);
}

CSB_V22_WallShape csb_v22_wall_shape_get(CSB_V22_WallVariant variant) {
    if ((int)variant < 0 || (int)variant >= CSB_V22_WALL_VARIANT_COUNT) {
        return g_wall_shapes[CSB_V22_WALL_VARIANT_D0_CENTER];
    }
    return g_wall_shapes[variant];
}

CSB_V22_FloorShape csb_v22_floor_shape_get(int csb_cell_type, int view_direction) {
    int base = csb_cell_type & 0x0F;
    int flags = csb_cell_type & 0xF0;
    if (flags & 0x40) return g_floor_shapes[CSB_FLOOR_SHAPE_PIT];
    if (flags & 0x10) {
        return g_floor_shapes[(flags & 0x01) ? CSB_FLOOR_SHAPE_STAIRS_DOWN
                                              : CSB_FLOOR_SHAPE_STAIRS_UP];
    }
    switch (base) {
        case 5:  return g_floor_shapes[CSB_FLOOR_SHAPE_CRACKED];
        case 6:  return g_floor_shapes[CSB_FLOOR_SHAPE_MOSSY];
        default: return g_floor_shapes[CSB_FLOOR_SHAPE_PLAIN];
    }
    (void)view_direction;
}

const CSB_V22_Material* csb_v22_material_get(int material_id) {
    if (material_id < 0 || material_id >= CSB_V22_DEFAULT_MATERIAL_COUNT) {
        return &g_default_materials[0];
    }
    return &g_default_materials[material_id];
}

int csb_v22_material_count(void) {
    return CSB_V22_DEFAULT_MATERIAL_COUNT;
}

/* ── CSB-specific shapes ─────────────────────────────────────────── */

CSB_V22_ShapeParams csb_v22_shape_for_prison_door(int open_progress) {
    CSB_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = CSB_V22_SHAPE_PRISON_DOOR;
    p.material_id = 6;                       /* iron */
    p.texture_id = 6;
    p.normal_map_id = 6;
    /* open_progress 0..100 -> tint shift from dark iron to bright sky */
    p.color_tint[0] = (uint8_t)(64 + (open_progress * 191) / 100);
    p.color_tint[1] = (uint8_t)(64 + (open_progress * 191) / 100);
    p.color_tint[2] = (uint8_t)(64 + (open_progress * 191) / 100);
    p.color_tint[3] = 255;
    p.lighting_mode = CSB_V22_LIGHT_FULL_BRIGHT;
    p.height_cm = 300.0f;
    p.width_cm = 200.0f;
    p.depth_cm = 30.0f;
    p.vertical_flip = 0;
    return p;
}

CSB_V22_ShapeParams csb_v22_shape_for_chaos_rune(int rune_index) {
    CSB_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = CSB_V22_SHAPE_CHAOS_RUNE;
    p.material_id = 5;                       /* chaos glow */
    p.texture_id = 5;
    p.normal_map_id = 5;
    /* Each rune gets a slightly different tint based on index */
    switch (rune_index % 4) {
        case 0: p.color_tint[0] = 64;  p.color_tint[1] = 255; p.color_tint[2] = 64;  break;
        case 1: p.color_tint[0] = 64;  p.color_tint[1] = 64;  p.color_tint[2] = 255; break;
        case 2: p.color_tint[0] = 255; p.color_tint[1] = 64;  p.color_tint[2] = 64;  break;
        case 3: p.color_tint[0] = 255; p.color_tint[1] = 64;  p.color_tint[2] = 255; break;
    }
    p.color_tint[3] = 255;
    p.lighting_mode = CSB_V22_LIGHT_CHAOS_GLOW;
    p.height_cm = 50.0f;
    p.width_cm  = 50.0f;
    p.depth_cm  = 5.0f;
    p.vertical_flip = 0;
    return p;
}

CSB_V22_ShapeParams csb_v22_shape_for_dsa_scroll(int scroll_index) {
    CSB_V22_ShapeParams p;
    memset(&p, 0, sizeof(p));
    p.type = CSB_V22_SHAPE_DSA_SCROLL;
    p.material_id = 8;                       /* parchment */
    p.texture_id = 8;
    p.normal_map_id = 0;
    p.color_tint[0] = 240;
    p.color_tint[1] = 220;
    p.color_tint[2] = 180;
    p.color_tint[3] = 255;
    p.lighting_mode = CSB_V22_LIGHT_MAGICAL_GLOW;
    p.height_cm = 40.0f;
    p.width_cm  = 60.0f;
    p.depth_cm  = 2.0f;
    p.vertical_flip = 0;
    (void)scroll_index;
    return p;
}

/* ── Initialization ──────────────────────────────────────────────── */

static int g_csb_v22_shapes_initialized = 0;

void csb_v22_shapes_init(void) {
    if (g_csb_v22_shapes_initialized) return;
    g_csb_v22_shapes_initialized = 1;
}

const char* csb_v22_shapes_source_evidence(void) {
    return
        "CSB V2.2 shape system: 9-square (3x3) layout, modern shape book.\n"
        "  CSBWin/Viewport.cpp:7290 - CSB 9-square viewport (3 depth x 3 lateral)\n"
        "  CSBWin/Chaos.cpp:60-69   - DSA call dispatch + chaos rune shape\n"
        "  ReDMCSB DUNGEON.C:35-44  - direction step tables (N/E/S/W)\n"
        "  ReDMCSB ENTRANCE.C       - CSB prison door intro shape\n"
        "  ReDMCSB GAMELOOP.C:150-155 - V1 tick cadence (CSB shares)\n"
        "  ReDMCSB DEFS.H:922 M034_SQUARE_TYPE - cell type decode\n"
        "  ReDMCSB DUNVIEW.C:6697-6816 - composition draw order\n"
        "Mirror of dm1_v22_shapes with CSB-specific shapes:\n"
        "  CSB_V22_SHAPE_PRISON_DOOR  (ENTRANCE.C)\n"
        "  CSB_V22_SHAPE_CHAOS_RUNE   (CSBWin/Chaos.cpp)\n"
        "  CSB_V22_SHAPE_DSA_SCROLL   (chaos magic surface)\n"
        "  CSB_V22_SHAPE_LORD_ORDER   (endgame statue)\n"
        "  CSB_V22_LIGHT_CHAOS_GLOW   (CSB-only lighting mode)\n"
        "9-square wall variants: 9 + DOOR + PRISON = 11 entries\n"
        "Materials: 10 builtin (plain, mossy, cracked, iron door, pit, chaos, prison, statue, scroll, UI)\n";
}
