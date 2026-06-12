#include "dm1_v1_viewport_d2c_center_wall_composition_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id,
               needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static const DM1_V1_D2CCenterCompositionOpPc34 *step_at(
    const DM1_V1_D2CCenterCompositionTracePc34 *trace,
    size_t index)
{
    if (!trace || index >= trace->step_count) return NULL;
    return &trace->steps[index];
}

static void expect_step(const char *id,
                        const DM1_V1_D2CCenterCompositionTracePc34 *trace,
                        size_t index,
                        DM1_V1_D2CCenterCompositionStepPc34 step,
                        uint16_t cell_order,
                        const char *anchor)
{
    const DM1_V1_D2CCenterCompositionOpPc34 *op = step_at(trace, index);

    expect_int(id, op ? (int)op->step : -1, (int)step, anchor);
    expect_int(id, op ? (int)op->cell_order : -1, (int)cell_order, anchor);
}

static void test_d2c_dispatch_and_metadata(void)
{
    DM1_V1_D2CCenterCompositionTracePc34 t =
        dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
            DM1_V1_D2C_CENTER_COMPOSITION_PC34_TELEPORTER);

    expect_int("d2c.view_square", t.view_square_index, 6,
               "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    expect_int("d2c.depth", t.view_depth, 2,
               "ReDMCSB DUNVIEW.C:370-377 G2027[6]");
    expect_int("d2c.lane", t.view_lane, 0,
               "ReDMCSB DUNVIEW.C:370-377 G2026[6]");
    expect_int("d2c.field_aspect", t.field_aspect_index, 7,
               "ReDMCSB DUNVIEW.C:370-377 G2035[6]");
    expect_int("d2c.wall_zone", t.wall_zone_pc34, 709,
               "ReDMCSB DEFS.H:4049 C709_ZONE_WALL_D2C");
    expect_step("d2c.dispatch", &t, 0,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_DISPATCH_D2C,
                0, "ReDMCSB DUNVIEW.C:8520-8521 F0128");
}

static void test_wall_routes_are_separate_from_open_center_field(void)
{
    DM1_V1_D2CCenterCompositionTracePc34 plain =
        dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
            DM1_V1_D2C_CENTER_COMPOSITION_PC34_WALL_PLAIN);
    DM1_V1_D2CCenterCompositionTracePc34 alcove =
        dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
            DM1_V1_D2C_CENTER_COMPOSITION_PC34_WALL_ALCOVE);

    expect_int("wall_plain.step_count", (int)plain.step_count, 4,
               "ReDMCSB DUNVIEW.C:7289-7312 F0121");
    expect_step("wall_plain.body", &plain, 1,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_WALL_BODY,
                0, "ReDMCSB DUNVIEW.C:7289-7307 F0121");
    expect_step("wall_plain.f0107", &plain, 2,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0107_FRONT_WALL_ORNAMENT,
                0, "ReDMCSB DUNVIEW.C:7308-7310 F0121/F0107");
    expect_step("wall_plain.return", &plain, 3,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_RETURN,
                0, "ReDMCSB DUNVIEW.C:7312 F0121");
    expect_bool("wall_plain.no_f0108", plain.calls_f0108_floor_ornament, false,
                "ReDMCSB DUNVIEW.C:7312 returns before 7314/7357");
    expect_bool("wall_plain.no_f0115", plain.calls_f0115_open_or_alcove, false,
                "ReDMCSB DUNVIEW.C:7312 returns before 7368");
    expect_bool("wall_plain.no_field", plain.calls_f0113_center_field, false,
                "ReDMCSB DUNVIEW.C:7370-7388 teleporter-only tail");

    expect_int("wall_alcove.step_count", (int)alcove.step_count, 4,
               "ReDMCSB DUNVIEW.C:7308-7310/7367-7368 F0121");
    expect_step("wall_alcove.order", &alcove, 3,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_OPEN_OR_ALCOVE,
                0x0000, "ReDMCSB DEFS.H:2658 C0x0000_CELL_ORDER_ALCOVE");
    expect_bool("wall_alcove.calls_f0115", alcove.calls_f0115_open_or_alcove, true,
                "ReDMCSB DUNVIEW.C:7309-7310/7367-7368 F0121/F0115");
}

static void test_door_front_two_pass_composition(void)
{
    DM1_V1_D2CCenterCompositionTracePc34 door =
        dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
            DM1_V1_D2C_CENTER_COMPOSITION_PC34_DOOR_FRONT);

    expect_int("door.step_count", (int)door.step_count, 5,
               "ReDMCSB DUNVIEW.C:7313-7342 F0121");
    expect_step("door.f0108", &door, 1,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0108_FLOOR_ORNAMENT,
                0, "ReDMCSB DUNVIEW.C:7314 F0121/F0108");
    expect_step("door.rear_f0115", &door, 2,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_REAR,
                0x0218, "ReDMCSB DUNVIEW.C:7315; DEFS.H:2669");
    expect_step("door.f0111", &door, 3,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0111_DOOR_BODY,
                0, "ReDMCSB DUNVIEW.C:7317-7339 F0121/F0111");
    expect_step("door.front_f0115", &door, 4,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_FRONT,
                0x0349, "ReDMCSB DUNVIEW.C:7341-7342; DEFS.H:2672");
    expect_bool("door.calls_f0115_rear", door.calls_f0115_rear, true,
                "ReDMCSB DUNVIEW.C:7315 F0121/F0115");
    expect_bool("door.calls_f0115_front", door.calls_f0115_front, true,
                "ReDMCSB DUNVIEW.C:7341-7342/7367-7368 F0121/F0115");
    expect_bool("door.no_center_field", door.calls_f0113_center_field, false,
                "ReDMCSB DUNVIEW.C:7370-7388 teleporter-only tail");
}

static void test_open_and_teleporter_center_field_composition(void)
{
    DM1_V1_D2CCenterCompositionTracePc34 corridor =
        dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
            DM1_V1_D2C_CENTER_COMPOSITION_PC34_CORRIDOR);
    DM1_V1_D2CCenterCompositionTracePc34 teleporter =
        dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
            DM1_V1_D2C_CENTER_COMPOSITION_PC34_TELEPORTER);

    expect_int("corridor.step_count", (int)corridor.step_count, 4,
               "ReDMCSB DUNVIEW.C:7353-7368 F0121");
    expect_step("corridor.f0108", &corridor, 1,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0108_FLOOR_ORNAMENT,
                0, "ReDMCSB DUNVIEW.C:7357 F0121/F0108");
    expect_step("corridor.f0112", &corridor, 2,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0112_CEILING_PIT,
                0, "ReDMCSB DUNVIEW.C:7358-7365 F0121/F0112");
    expect_step("corridor.f0115", &corridor, 3,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_OPEN_OR_ALCOVE,
                0x3421, "ReDMCSB DUNVIEW.C:7356-7368; DEFS.H:2676");
    expect_bool("corridor.no_field", corridor.calls_f0113_center_field, false,
                "ReDMCSB DUNVIEW.C:7370-7388 teleporter-only tail");

    expect_int("teleporter.step_count", (int)teleporter.step_count, 5,
               "ReDMCSB DUNVIEW.C:7353-7388 F0121");
    expect_step("teleporter.field", &teleporter, 4,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0113_CENTER_FIELD,
                0, "ReDMCSB DUNVIEW.C:7370-7388 F0121/F0113");
    expect_bool("teleporter.field_enabled", teleporter.calls_f0113_center_field, true,
                "ReDMCSB DUNVIEW.C:7370-7388 F0121/F0113");
    expect_bool("teleporter.c10", teleporter.field_uses_c10_transparency, true,
                "ReDMCSB DUNVIEW.C:4382-4474 F0113; DEFS.H:2088");
}

static void test_c10_layer_preserves_prior_composition_pixels(void)
{
    uint8_t viewport[6] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
    const uint8_t wall[6] = { 10, 0x31, 0x32, 10, 0x34, 0x35 };
    const uint8_t field[6] = { 0x41, 10, 0x43, 10, 10, 0x46 };

    expect_int("c10.wall.writes",
               dm1_v1_viewport_d2c_center_wall_composition_apply_c10_layer_pc34(
                   wall, viewport, 6,
                   DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH),
               4, "ReDMCSB DUNVIEW.C:3048-3058 F0100; DEFS.H:2088");
    expect_int("c10.wall.preserve0", viewport[0], 0x20,
               "ReDMCSB DUNVIEW.C:3048-3058 C10 skip");
    expect_int("c10.wall.write1", viewport[1], 0x31,
               "ReDMCSB DUNVIEW.C:3048-3058 opaque write");

    expect_int("c10.field.writes",
               dm1_v1_viewport_d2c_center_wall_composition_apply_c10_layer_pc34(
                   field, viewport, 6,
                   DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH),
               3, "ReDMCSB DUNVIEW.C:4382-4474 F0113; DEFS.H:2088");
    expect_int("c10.field.write0", viewport[0], 0x41,
               "ReDMCSB DUNVIEW.C:4382-4474 F0113 opaque write");
    expect_int("c10.field.preserve1", viewport[1], 0x31,
               "ReDMCSB DUNVIEW.C:4382-4474 F0113 C10 skip");
    expect_int("c10.field.preserve3", viewport[3], 0x20,
               "ReDMCSB DUNVIEW.C:4382-4474 F0113 C10 skip");
    expect_int("c10.invalid",
               dm1_v1_viewport_d2c_center_wall_composition_apply_c10_layer_pc34(
                   NULL, viewport, 6,
                   DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH),
               -1, "ReDMCSB DUNVIEW.C:4382-4474 synthetic helper rejects null source");
}

static void test_d2c_door_front_pixel_pass_order_and_c10(void)
{
    DM1_V1_D2CDoorFrontPixelTracePc34 trace;

    expect_int("door_pixel.compose",
               dm1_v1_viewport_d2c_door_front_compose_pixel_pc34(
                   0x11, 0x21, 0x31, 0x41, 0x51,
                   DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH,
                   &trace),
               1, "ReDMCSB DUNVIEW.C:7314-7342 F0121");
    expect_int("door_pixel.rear_order", trace.rear_cell_order, 0x0218,
               "ReDMCSB DUNVIEW.C:7315; DEFS.H:2669");
    expect_int("door_pixel.front_order", trace.front_cell_order, 0x0349,
               "ReDMCSB DUNVIEW.C:7341-7342; DEFS.H:2672");
    expect_int("door_pixel.rear_count", trace.rear_cell_count, 2,
               "ReDMCSB DUNVIEW.C:4561-4564 F0115");
    expect_int("door_pixel.rear_cell0", trace.rear_cells[0], 1,
               "ReDMCSB DEFS.H:2669 BACKLEFT");
    expect_int("door_pixel.rear_cell1", trace.rear_cells[1], 2,
               "ReDMCSB DEFS.H:2669 BACKRIGHT");
    expect_int("door_pixel.front_count", trace.front_cell_count, 2,
               "ReDMCSB DUNVIEW.C:4561-4564 F0115");
    expect_int("door_pixel.front_cell0", trace.front_cells[0], 4,
               "ReDMCSB DEFS.H:2672 FRONTLEFT");
    expect_int("door_pixel.front_cell1", trace.front_cells[1], 3,
               "ReDMCSB DEFS.H:2672 FRONTRIGHT");
    expect_int("door_pixel.after_floor", trace.after_floor_ornament, 0x21,
               "ReDMCSB DUNVIEW.C:7314 F0108 first");
    expect_int("door_pixel.after_rear", trace.after_rear_f0115, 0x31,
               "ReDMCSB DUNVIEW.C:7315 F0115 pass 1 before door");
    expect_int("door_pixel.after_door", trace.after_f0111_door, 0x41,
               "ReDMCSB DUNVIEW.C:7336-7339 F0111 after pass 1");
    expect_int("door_pixel.after_front", trace.after_front_f0115, 0x51,
               "ReDMCSB DUNVIEW.C:7341-7342 F0115 pass 2 last");

    expect_int("door_pixel.c10.compose",
               dm1_v1_viewport_d2c_door_front_compose_pixel_pc34(
                   0x66, 10, 0x32, 10, 0x52,
                   DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH,
                   &trace),
               1, "ReDMCSB DUNVIEW.C:3048-3058/F0115 C10 preservation");
    expect_bool("door_pixel.c10.floor", trace.floor_transparent, true,
                "ReDMCSB DUNVIEW.C:7314 floor C10 preserves initial");
    expect_int("door_pixel.c10.after_floor", trace.after_floor_ornament, 0x66,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_bool("door_pixel.c10.rear", trace.rear_transparent, false,
                "ReDMCSB DUNVIEW.C:7315 opaque rear thing pass");
    expect_int("door_pixel.c10.after_rear", trace.after_rear_f0115, 0x32,
               "ReDMCSB DUNVIEW.C:7315 rear F0115 writes");
    expect_bool("door_pixel.c10.door", trace.door_transparent, true,
                "ReDMCSB DUNVIEW.C:7336-7339 transparent door pixel preserves rear");
    expect_int("door_pixel.c10.after_door", trace.after_f0111_door, 0x32,
               "ReDMCSB DUNVIEW.C:4218-4337/F0111 C10 skip");
    expect_bool("door_pixel.c10.front", trace.front_transparent, false,
                "ReDMCSB DUNVIEW.C:7341-7342 opaque front thing pass");
    expect_int("door_pixel.c10.after_front", trace.after_front_f0115, 0x52,
               "ReDMCSB DUNVIEW.C:7341-7342 front F0115 writes last");
    expect_int("door_pixel.null_out",
               dm1_v1_viewport_d2c_door_front_compose_pixel_pc34(
                   0, 0, 0, 0, 0,
                   DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH,
                   NULL),
               0, "synthetic helper rejects null trace");
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e =
        dm1_v1_viewport_d2c_center_wall_composition_source_evidence_pc34();

    expect_contains("evidence.contract", e, "Source-locked contract gate only",
                    "source evidence marker");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8520-8521",
                    "ReDMCSB DUNVIEW.C:8520-8521 F0128");
    expect_contains("evidence.f0121", e, "DUNVIEW.C:7244-7388",
                    "ReDMCSB DUNVIEW.C:7244-7388 F0121");
    expect_contains("evidence.wall_return", e, "line 7312 returns",
                    "ReDMCSB DUNVIEW.C:7312 F0121");
    expect_contains("evidence.door_rear", e, "C0x0218",
                    "ReDMCSB DEFS.H:2669");
    expect_contains("evidence.door_front", e, "C0x0349",
                    "ReDMCSB DEFS.H:2672");
    expect_contains("evidence.open_order", e, "C0x3421",
                    "ReDMCSB DEFS.H:2676");
    expect_contains("evidence.field", e, "DUNVIEW.C:7370-7388",
                    "ReDMCSB DUNVIEW.C:7370-7388 F0113 tail");
    expect_contains("evidence.f0113", e, "DUNVIEW.C:4382-4474 F0113",
                    "ReDMCSB DUNVIEW.C:4382-4474 F0113");
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581 F0115",
                    "ReDMCSB DUNVIEW.C:4547-4581 F0115");
    expect_contains("evidence.c10", e, "DEFS.H:2088",
                    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_contains("evidence.orders", e, "DEFS.H:2656-2677",
                    "ReDMCSB DEFS.H:2656-2677 cell orders");
    expect_contains("evidence.zone", e, "C709_ZONE_WALL_D2C",
                    "ReDMCSB DEFS.H:4049 C709_ZONE_WALL_D2C");
}

int main(void)
{
    test_d2c_dispatch_and_metadata();
    test_wall_routes_are_separate_from_open_center_field();
    test_door_front_two_pass_composition();
    test_open_and_teleporter_center_field_composition();
    test_c10_layer_preserves_prior_composition_pixels();
    test_d2c_door_front_pixel_pass_order_and_c10();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2c_center_wall_composition_pc34_compat "
               "failures=%d assertions=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2c_center_wall_composition_pc34_compat "
           "%d/%d assertions\n", g_assertions, g_assertions);
    return 0;
}
