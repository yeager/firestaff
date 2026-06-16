/**
 * firestaff_csb_v22_shapes_probe.c
 *
 * CSB V2.2 modern shape book headless probe.
 */
#include "csb_v22_shapes.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_init(void) {
    csb_v22_shapes_init();
    check(1, "init");
}

static void p_materials(void) {
    int n = csb_v22_material_count();
    check(n >= 10, "material count >= 10");
    const CSB_V22_Material* chaos = csb_v22_material_get(5);
    check(chaos != NULL && chaos->emission_strength >= 0.5f, "chaos emission");
    const CSB_V22_Material* iron = csb_v22_material_get(6);
    check(iron != NULL && iron->metallic >= 0.5f, "iron metallic");
}

static void p_wall_variants(void) {
    check(CSB_V22_WALL_VARIANT_COUNT == 11, "9 + DOOR + PRISON = 11");
    CSB_V22_WallShape prison = csb_v22_wall_shape_get(CSB_V22_WALL_VARIANT_PRISON);
    check(prison.base_texture_id == 6, "PRISON base tex");
}

static void p_shape_for_cell(void) {
    CSB_V22_ShapeParams p = csb_v22_shape_for_cell(0x04, 0, 1, 0);
    check(p.type == CSB_V22_SHAPE_FLOOR_PLAIN, "cell 0x04 -> floor plain");
    CSB_V22_ShapeParams door = csb_v22_shape_for_cell(0x20, 0, 1, 0);
    check(door.type == CSB_V22_SHAPE_WALL_DOORWAY, "cell 0x20 -> doorway");
    CSB_V22_ShapeParams pit = csb_v22_shape_for_cell(0x40, 0, 0, 0);
    check(pit.type == CSB_V22_SHAPE_FLOOR_PIT, "cell 0x40 -> pit");
}

static void p_prison_door(void) {
    CSB_V22_ShapeParams c = csb_v22_shape_for_prison_door(0);
    CSB_V22_ShapeParams o = csb_v22_shape_for_prison_door(100);
    check(c.color_tint[0] <= 80, "prison closed dim");
    check(o.color_tint[0] >= 240, "prison open bright");
}

static void p_chaos_runes(void) {
    CSB_V22_ShapeParams r0 = csb_v22_shape_for_chaos_rune(0);
    CSB_V22_ShapeParams r1 = csb_v22_shape_for_chaos_rune(1);
    check(r0.type == CSB_V22_SHAPE_CHAOS_RUNE, "r0 chaos rune");
    check(r0.lighting_mode == CSB_V22_LIGHT_CHAOS_GLOW, "r0 chaos glow");
    int differ = (r0.color_tint[0] != r1.color_tint[0] ||
                  r0.color_tint[1] != r1.color_tint[1] ||
                  r0.color_tint[2] != r1.color_tint[2]);
    check(differ, "consecutive runes differ");
}

static void p_dsa_scroll(void) {
    CSB_V22_ShapeParams s = csb_v22_shape_for_dsa_scroll(0);
    check(s.type == CSB_V22_SHAPE_DSA_SCROLL, "scroll type");
    check(s.lighting_mode == CSB_V22_LIGHT_MAGICAL_GLOW, "scroll glow");
}

static void p_evidence(void) {
    const char* ev = csb_v22_shapes_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "9-square") != NULL, "ev 9-square");
    check(strstr(ev, "PRISON") != NULL, "ev PRISON");
    check(strstr(ev, "CHAOS") != NULL, "ev CHAOS");
    check(strstr(ev, "DSA") != NULL, "ev DSA");
}

int main(void) {
    printf("=== CSB V2.2 shape book probe ===\n");
    p_init(); p_materials(); p_wall_variants();
    p_shape_for_cell(); p_prison_door();
    p_chaos_runes(); p_dsa_scroll(); p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
