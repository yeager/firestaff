/**
 * firestaff_theron_v22_shapes_probe.c
 *
 * Theron V2.2 modern shape book headless probe.
 */
#include "theron_v22_shapes.h"
#include "theron_v1_world.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_init(void) {
    theron_v22_shapes_init();
    check(1, "init");
}

static void p_materials(void) {
    int n = theron_v22_material_count();
    check(n >= 10, "material count >= 10");
    const Theron_V22_Material* tele = theron_v22_material_get(5);
    check(tele != NULL && tele->emission_strength >= 0.5f, "teleporter emission");
    const Theron_V22_Material* alarm = theron_v22_material_get(6);
    check(alarm != NULL && alarm->emission_strength >= 0.5f, "alarm emission");
}

static void p_wall_variants(void) {
    check(THERON_V22_WALL_VARIANT_COUNT == 14, "12 + DOOR + SECRET = ***");
    Theron_V22_WallShape secret = theron_v22_wall_shape_get(THERON_V22_WALL_VARIANT_SECRET);
    check(secret.base_texture_id == 8, "SECRET base tex (aged wood)");
}

static void p_shape_for_cell(void) {
    Theron_V22_ShapeParams p = theron_v22_shape_for_cell(THERON_SQUARE_FLOOR, 0, 1, 0);
    check(p.type == THERON_V22_SHAPE_FLOOR_PLAIN, "FLOOR -> floor plain");
    Theron_V22_ShapeParams door = theron_v22_shape_for_cell(THERON_SQUARE_DOOR, 0, 1, 0);
    check(door.type == THERON_V22_SHAPE_WALL_DOORWAY, "DOOR -> doorway");
    Theron_V22_ShapeParams tele = theron_v22_shape_for_cell(THERON_SQUARE_TELEPORTER, 0, 0, 0);
    check(tele.type == THERON_V22_SHAPE_FIELD_TELEPORTER, "TELEPORTER -> field teleporter");
    Theron_V22_ShapeParams alarm = theron_v22_shape_for_cell(THERON_SQUARE_ALARM, 0, 0, 0);
    check(alarm.type == THERON_V22_SHAPE_FIELD_ALARM, "ALARM -> field alarm");
}

static void p_teleporter(void) {
    Theron_V22_ShapeParams dormant = theron_v22_shape_for_teleporter(0);
    Theron_V22_ShapeParams active = theron_v22_shape_for_teleporter(1);
    check(dormant.type == THERON_V22_SHAPE_FIELD_TELEPORTER, "teleporter type");
    check(active.color_tint[1] > dormant.color_tint[1], "active teleporter brighter");
}

static void p_alarm(void) {
    Theron_V22_ShapeParams silent = theron_v22_shape_for_alarm(0);
    Theron_V22_ShapeParams ringing = theron_v22_shape_for_alarm(1);
    check(silent.type == THERON_V22_SHAPE_FIELD_ALARM, "alarm type");
    check(ringing.lighting_mode == THERON_V22_LIGHT_ALARM_PULSE, "alarm pulse glow");
    check(ringing.color_tint[0] >= 200, "ringing red bright");
}

static void p_secret_door(void) {
    Theron_V22_ShapeParams closed = theron_v22_shape_for_secret_door(0);
    Theron_V22_ShapeParams open = theron_v22_shape_for_secret_door(100);
    check(closed.color_tint[0] <= 60, "secret closed dim");
    check(open.color_tint[0] >= 200, "secret open bright");
}

static void p_lit_torch(void) {
    Theron_V22_ShapeParams t0 = theron_v22_shape_for_lit_torch(0);
    Theron_V22_ShapeParams t3 = theron_v22_shape_for_lit_torch(3);
    check(t0.lighting_mode == THERON_V22_LIGHT_TORCH_LIT, "torch lit");
    int differ = (t0.color_tint[1] != t3.color_tint[1] ||
                  t0.color_tint[2] != t3.color_tint[2]);
    check(differ, "torch 0 vs 3 differ in tint");
}

static void p_evidence(void) {
    const char* ev = theron_v22_shapes_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "T400") != NULL, "ev T400");
    check(strstr(ev, "T520") != NULL, "ev T520");
    check(strstr(ev, "T700") != NULL, "ev T700 teleporter");
    check(strstr(ev, "T800") != NULL, "ev T800 alarm");
    check(strstr(ev, "FLOODED") != NULL, "ev FLOODED");
    check(strstr(ev, "ALARM") != NULL, "ev ALARM");
}

int main(void) {
    printf("=== Theron V2.2 shape book probe ===\n");
    p_init(); p_materials(); p_wall_variants();
    p_shape_for_cell(); p_teleporter(); p_alarm();
    p_secret_door(); p_lit_torch(); p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
