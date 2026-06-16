/* test_theron_v22_shapes_pc34.c
 *
 * Theron V2.2 modern shape book unit test. Mirrors the CSB V2.2
 * shape coverage with Theron-specific extras (teleporter, alarm,
 * secret door, flooded, lit torch).
 */
#include "theron_v22_shapes.h"
#include "theron_v1_world.h"
#include "theron_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_init(void) {
    theron_v22_shapes_init();
    check(1, "init");
}

static void t_material_count(void) {
    int n = theron_v22_material_count();
    check(n >= 10, "materials >= 10");
    check(n <= 32, "materials <= 32");
}

static void t_material_oob(void) {
    const Theron_V22_Material* m = theron_v22_material_get(-1);
    check(m != NULL, "OOB material -1 not null");
    m = theron_v22_material_get(9999);
    check(m != NULL, "OOB material 9999 not null");
    check(m == theron_v22_material_get(0), "OOB material returns default[0]");
}

static void t_material_teleporter(void) {
    /* Material 5 = teleporter warp, must have high emission. */
    const Theron_V22_Material* tele = theron_v22_material_get(5);
    check(tele != NULL && tele->emission_strength >= 0.5f, "teleporter emission");
}

static void t_material_alarm(void) {
    /* Material 6 = alarm pulse, must have red-tinted emission. */
    const Theron_V22_Material* alarm = theron_v22_material_get(6);
    check(alarm != NULL && alarm->emission_strength >= 0.5f, "alarm emission");
}

static void t_wall_variants(void) {
    check(THERON_V22_WALL_VARIANT_COUNT == 14, "13 + DOOR + SECRET = 14");
    Theron_V22_WallShape secret = theron_v22_wall_shape_get(THERON_V22_WALL_VARIANT_SECRET);
    check(secret.base_texture_id == 8, "SECRET variant base tex (aged wood)");
}

static void t_wall_oob(void) {
    Theron_V22_WallShape w = theron_v22_wall_shape_get((Theron_V22_WallVariant)9999);
    check(w.base_texture_id == 0, "OOB wall variant returns D0C default");
}

static void t_floor_shape_basic(void) {
    /* THERON_SQUARE_FLOOR (1) -> plain floor. */
    Theron_V22_FloorShape f = theron_v22_floor_shape_get(THERON_SQUARE_FLOOR, 0);
    check(f.pit_present == 0, "plain floor no pit");
    check(f.stairs_present == 0, "plain floor no stairs");
    /* THERON_SQUARE_PIT (2) -> pit. */
    Theron_V22_FloorShape pit = theron_v22_floor_shape_get(THERON_SQUARE_PIT, 0);
    check(pit.pit_present == 1, "pit square -> pit_present=1");
    /* THERON_SQUARE_TELEPORTER (5) -> flooded. */
    Theron_V22_FloorShape tele = theron_v22_floor_shape_get(THERON_SQUARE_TELEPORTER, 0);
    check(tele.flooded_present == 1, "teleporter square -> flooded_present=1");
    /* THERON_SQUARE_ALARM (6) -> cracked. */
    Theron_V22_FloorShape alarm = theron_v22_floor_shape_get(THERON_SQUARE_ALARM, 0);
    check(alarm.base_texture_id == 2, "alarm square -> cracked floor");
}

static void t_shape_for_cell(void) {
    /* Wall (THERON_SQUARE_WALL = 0). */
    Theron_V22_ShapeParams p = theron_v22_shape_for_cell(THERON_SQUARE_WALL, 0, 0, 0);
    check(p.type == THERON_V22_SHAPE_WALL_STRAIGHT, "THERON_SQUARE_WALL -> wall straight");
    /* Floor (1). */
    Theron_V22_ShapeParams floor = theron_v22_shape_for_cell(THERON_SQUARE_FLOOR, 0, 1, 0);
    check(floor.type == THERON_V22_SHAPE_FLOOR_PLAIN, "THERON_SQUARE_FLOOR -> floor plain");
    /* Pit (2). */
    Theron_V22_ShapeParams pit = theron_v22_shape_for_cell(THERON_SQUARE_PIT, 0, 2, -1);
    check(pit.type == THERON_V22_SHAPE_FLOOR_PIT, "THERON_SQUARE_PIT -> floor pit");
    /* Door (4). */
    Theron_V22_ShapeParams door = theron_v22_shape_for_cell(THERON_SQUARE_DOOR, 0, 1, 0);
    check(door.type == THERON_V22_SHAPE_WALL_DOORWAY, "THERON_SQUARE_DOOR -> wall doorway");
    /* Teleporter (5). */
    Theron_V22_ShapeParams tele = theron_v22_shape_for_cell(THERON_SQUARE_TELEPORTER, 0, 0, 0);
    check(tele.type == THERON_V22_SHAPE_FIELD_TELEPORTER, "THERON_SQUARE_TELEPORTER -> field teleporter");
    /* Alarm (6). */
    Theron_V22_ShapeParams alarm = theron_v22_shape_for_cell(THERON_SQUARE_ALARM, 0, 0, 0);
    check(alarm.type == THERON_V22_SHAPE_FIELD_ALARM, "THERON_SQUARE_ALARM -> field alarm");
}

static void t_shape_clamp(void) {
    Theron_V22_ShapeParams p = theron_v22_shape_for_cell(THERON_SQUARE_FLOOR, 0, 99, 99);
    check(p.type >= 0 && p.type < THERON_V22_SHAPE_COUNT, "clamp depth=99");
    Theron_V22_ShapeParams p2 = theron_v22_shape_for_cell(THERON_SQUARE_FLOOR, 99, 0, 0);
    check(p2.type >= 0 && p2.type < THERON_V22_SHAPE_COUNT, "clamp dir=99");
}

static void t_shape_for_view_square(void) {
    /* view_square 0..11 covers 4x3. element 0=floor, 1=ceiling, 2=wall. */
    Theron_V22_ShapeParams floor = theron_v22_shape_for_view_square(0, 0, 0);
    check(floor.type == THERON_V22_SHAPE_FLOOR_PLAIN, "vq 0,elem 0 = floor");
    Theron_V22_ShapeParams wall = theron_v22_shape_for_view_square(11, 2, 0);
    check(wall.type == THERON_V22_SHAPE_WALL_STRAIGHT, "vq 11,elem 2 = wall");
}

static void t_teleporter_active(void) {
    Theron_V22_ShapeParams dormant = theron_v22_shape_for_teleporter(0);
    Theron_V22_ShapeParams active = theron_v22_shape_for_teleporter(1);
    check(dormant.type == THERON_V22_SHAPE_FIELD_TELEPORTER, "teleporter type");
    check(dormant.lighting_mode == THERON_V22_LIGHT_MAGICAL_GLOW, "teleporter magical glow");
    /* Active teleporter is brighter than dormant. */
    check(active.color_tint[1] > dormant.color_tint[1], "active teleporter brighter");
    check(active.color_tint[2] > dormant.color_tint[2], "active teleporter brighter B");
}

static void t_alarm_ring(void) {
    Theron_V22_ShapeParams silent = theron_v22_shape_for_alarm(0);
    Theron_V22_ShapeParams ringing = theron_v22_shape_for_alarm(1);
    check(silent.type == THERON_V22_SHAPE_FIELD_ALARM, "alarm type");
    check(ringing.lighting_mode == THERON_V22_LIGHT_ALARM_PULSE, "alarm pulse");
    check(ringing.color_tint[0] >= 200, "alarm ringing red channel bright");
    check(silent.color_tint[0] < 200, "alarm silent red channel dim");
}

static void t_secret_door_progress(void) {
    Theron_V22_ShapeParams closed = theron_v22_shape_for_secret_door(0);
    Theron_V22_ShapeParams open = theron_v22_shape_for_secret_door(100);
    check(closed.type == THERON_V22_SHAPE_SECRET_DOOR, "secret type");
    check(closed.color_tint[0] <= 60, "secret hidden dim");
    check(open.color_tint[0] >= 200, "secret revealed bright");
}

static void t_lit_torch(void) {
    Theron_V22_ShapeParams t0 = theron_v22_shape_for_lit_torch(0);
    Theron_V22_ShapeParams t3 = theron_v22_shape_for_lit_torch(3);
    check(t0.type == THERON_V22_SHAPE_LIT_TORCH, "torch type");
    check(t0.lighting_mode == THERON_V22_LIGHT_TORCH_LIT, "torch lit");
    /* Each torch index has a slightly different tint. */
    int differ = (t0.color_tint[1] != t3.color_tint[1] ||
                  t0.color_tint[2] != t3.color_tint[2]);
    check(differ, "torch 0 vs 3 differ in tint");
}

static void t_evidence(void) {
    const char* ev = theron_v22_shapes_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "T400") != NULL, "ev T400");
    check(strstr(ev, "T520") != NULL, "ev T520");
    check(strstr(ev, "T600") != NULL, "ev T600");
    check(strstr(ev, "T700") != NULL, "ev T700 teleporter");
    check(strstr(ev, "T800") != NULL, "ev T800 alarm");
    check(strstr(ev, "ALARM") != NULL, "ev ALARM");
    check(strstr(ev, "SECRET_DOOR") != NULL || strstr(ev, "secret") != NULL, "ev secret");
    check(strstr(ev, "FLOODED") != NULL || strstr(ev, "flooded") != NULL, "ev flooded");
}

static void t_4x3_coverage(void) {
    /* 4x3 = 12 wall variants (D3..D0 x L/C/R = 12) + DOOR + SECRET = 14. */
    for (int d = 0; d < 4; d++) {
        for (int lat = -1; lat <= 1; lat++) {
            Theron_V22_ShapeParams p = theron_v22_shape_for_cell(THERON_SQUARE_WALL, 0, d, lat);
            check(p.type >= 0 && p.type < THERON_V22_SHAPE_COUNT, "4x3 cell valid");
        }
    }
    check(THERON_V22_WALL_VARIANT_COUNT == 14, "12 + DOOR + SECRET = 14");
}

int main(void) {
    printf("=== Theron V2.2 shape book test ===\n");
    t_init();
    t_material_count();
    t_material_oob();
    t_material_teleporter();
    t_material_alarm();
    t_wall_variants();
    t_wall_oob();
    t_floor_shape_basic();
    t_shape_for_cell();
    t_shape_clamp();
    t_shape_for_view_square();
    t_teleporter_active();
    t_alarm_ring();
    t_secret_door_progress();
    t_lit_torch();
    t_evidence();
    t_4x3_coverage();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
