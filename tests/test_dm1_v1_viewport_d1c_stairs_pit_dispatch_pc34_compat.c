#include "dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat.h"

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

static void expect_nonnull(const char *id, const void *got, const char *anchor)
{
    ++g_assertions;
    if (!got) {
        printf("FAIL %s got=NULL at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonnull (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack, const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static DM1_V1_D1CDispatchOutputPc34 probe(DM1_V1_D1CDispatchInputPc34 input,
                                          const char *id)
{
    DM1_V1_D1CDispatchOutputPc34 output;
    expect_bool(id,
                dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_probe(&input, &output),
                true,
                "DUNVIEW.C:7727-7958 F0124");
    return output;
}

enum {
    D1C_SENTINEL_VIEWPORT_W = 224,
    D1C_SENTINEL_VIEWPORT_H = 136,
    D1C_SENTINEL_STAIRS_UP_X = 112,
    D1C_SENTINEL_STAIRS_UP_Y = 101,
    D1C_SENTINEL_STAIRS_DOWN_X = 112,
    D1C_SENTINEL_STAIRS_DOWN_Y = 114,
    D1C_SENTINEL_FLOOR_PIT_X = 112,
    D1C_SENTINEL_FLOOR_PIT_Y = 105,
    D1C_SENTINEL_FIELD_X = 112,
    D1C_SENTINEL_FIELD_Y = 68,
    D1C_SENTINEL_STAIRS_UP_PIXEL = 0x51,
    D1C_SENTINEL_STAIRS_DOWN_PIXEL = 0x52,
    D1C_SENTINEL_FLOOR_PIT_PIXEL = 0x53,
    D1C_SENTINEL_FIELD_PIXEL = 0x54
};

static unsigned char *pixel_at(unsigned char *frame, int x, int y)
{
    return &frame[(y * D1C_SENTINEL_VIEWPORT_W) + x];
}

static unsigned char pixel_get(const unsigned char *frame, int x, int y)
{
    return frame[(y * D1C_SENTINEL_VIEWPORT_W) + x];
}

static void mark_d1c_feature_zone_pixel(unsigned char *frame,
                                        const DM1_V1_D1CDispatchOutputPc34 *out)
{
    /*
     * Test-only pixel sentinel for the MEDIA720 D1C route zones selected in
     * ReDMCSB DUNVIEW.C:7753-7781, 7912-7921, 7939-7957 and named in
     * DEFS.H:4148 C809, 4161 C822, 4206 C859, 4052 C712.
     */
    switch (out->zone_index) {
    case DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_UP_D1C:
        *pixel_at(frame, D1C_SENTINEL_STAIRS_UP_X, D1C_SENTINEL_STAIRS_UP_Y) =
            D1C_SENTINEL_STAIRS_UP_PIXEL;
        break;
    case DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D1C:
        *pixel_at(frame, D1C_SENTINEL_STAIRS_DOWN_X, D1C_SENTINEL_STAIRS_DOWN_Y) =
            D1C_SENTINEL_STAIRS_DOWN_PIXEL;
        break;
    case DM1_V1_D1C_DISPATCH_PC34_ZONE_FLOOR_PIT_D1C:
        *pixel_at(frame, D1C_SENTINEL_FLOOR_PIT_X, D1C_SENTINEL_FLOOR_PIT_Y) =
            D1C_SENTINEL_FLOOR_PIT_PIXEL;
        break;
    case DM1_V1_D1C_DISPATCH_PC34_ZONE_FIELD_D1C:
        *pixel_at(frame, D1C_SENTINEL_FIELD_X, D1C_SENTINEL_FIELD_Y) =
            D1C_SENTINEL_FIELD_PIXEL;
        break;
    default:
        break;
    }
}

static void expect_d1c_feature_pixels(const char *id,
                                      const unsigned char *frame,
                                      int stairs_up,
                                      int stairs_down,
                                      int floor_pit,
                                      int field)
{
    char label[96];

    snprintf(label, sizeof(label), "%s.stairs_up_pixel", id);
    expect_int(label,
               pixel_get(frame, D1C_SENTINEL_STAIRS_UP_X, D1C_SENTINEL_STAIRS_UP_Y),
               stairs_up,
               "DUNVIEW.C:7753-7763 C809 sentinel");
    snprintf(label, sizeof(label), "%s.stairs_down_pixel", id);
    expect_int(label,
               pixel_get(frame, D1C_SENTINEL_STAIRS_DOWN_X, D1C_SENTINEL_STAIRS_DOWN_Y),
               stairs_down,
               "DUNVIEW.C:7764-7781 C822 sentinel");
    snprintf(label, sizeof(label), "%s.floor_pit_pixel", id);
    expect_int(label,
               pixel_get(frame, D1C_SENTINEL_FLOOR_PIT_X, D1C_SENTINEL_FLOOR_PIT_Y),
               floor_pit,
               "DUNVIEW.C:7912-7921 C859 sentinel");
    snprintf(label, sizeof(label), "%s.field_pixel", id);
    expect_int(label,
               pixel_get(frame, D1C_SENTINEL_FIELD_X, D1C_SENTINEL_FIELD_Y),
               field,
               "DUNVIEW.C:7939-7957 C712 sentinel");
}

static void test_stairs_up_front_media720_route(void)
{
    const DM1_V1_D1CDispatchInputPc34 input = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT, true, 7, 0, 0x1234, false
    };
    const DM1_V1_D1CDispatchOutputPc34 out = probe(input, "stairs_up.probe");

    expect_int("stairs_up.route", (int)out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_UP_FRONT, "DUNVIEW.C:7753-7763");
    expect_bool("stairs_up.used_f0104", out.used_f0104, true, "DUNVIEW.C:7762");
    expect_int("stairs_up.native_slot", out.native_bitmap_index, 5,
               "DEFS.H:2446 C05_STAIRS_BITMAP_UP_FRONT_D1C; DUNVIEW.C:7762");
    expect_int("stairs_up.zone", out.zone_index, 809,
               "DEFS.H:4148 C809_ZONE_STAIRS_UP_FRONT_D1C");
    expect_int("stairs_up.cell_order", out.cell_order_called, 0x3421,
               "DUNVIEW.C:7925 C0x3421");
    expect_bool("stairs_up.f0115", out.used_f0115, true, "DUNVIEW.C:7937");
    expect_bool("stairs_up.f0113", out.used_f0113, false, "DUNVIEW.C:7939-7957 only teleporter");
    expect_bool("stairs_up.no_door", out.used_f0111_door, false, "DUNVIEW.C:7873-7911 excluded");
}

static void test_stairs_down_front_media720_route(void)
{
    const DM1_V1_D1CDispatchInputPc34 input = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT, false, 8, 0, 0x2345, false
    };
    const DM1_V1_D1CDispatchOutputPc34 out = probe(input, "stairs_down.probe");

    expect_int("stairs_down.route", (int)out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT, "DUNVIEW.C:7764-7781");
    expect_bool("stairs_down.used_f0104", out.used_f0104, true, "DUNVIEW.C:7780");
    expect_int("stairs_down.native_slot", out.native_bitmap_index, 12,
               "DEFS.H:2453 C12_STAIRS_BITMAP_DOWN_FRONT_D1C; DUNVIEW.C:7780");
    expect_int("stairs_down.zone", out.zone_index, 822,
               "DEFS.H:4161 C822_ZONE_STAIRS_DOWN_FRONT_D1C");
    expect_int("stairs_down.view_square", out.view_square_index, 3,
               "DEFS.H:2599 M606_VIEW_SQUARE_D1C MEDIA720");
    expect_bool("stairs_down.wall_blit", out.wall_blit_called, false,
                "DUNVIEW.C:7784 wall case not taken");
}

static void test_floor_pit_route(void)
{
    const DM1_V1_D1CDispatchInputPc34 input = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR_PIT, false, 9, 0, 0x3456, false
    };
    const DM1_V1_D1CDispatchOutputPc34 out = probe(input, "floor_pit.probe");

    expect_int("floor_pit.element_alias", input.element, 2,
               "DEFS.H:1009 C02_ELEMENT_PIT; floor-pit aspect route");
    expect_int("floor_pit.route", (int)out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_FLOOR_PIT, "DUNVIEW.C:7912-7921");
    expect_bool("floor_pit.used_f0104", out.used_f0104, true, "DUNVIEW.C:7920");
    expect_int("floor_pit.native_graphic", out.native_bitmap_index, 55,
               "DEFS.H:2338 M759_GRAPHIC_FLOOR_PIT_D1C MEDIA720");
    expect_int("floor_pit.zone", out.zone_index, 859,
               "DEFS.H:4206 C859_ZONE_FLOORPIT_D1C");
    expect_bool("floor_pit.f0108_floor_ornament", out.used_f0108_floor_ornament, true,
                "DUNVIEW.C:7926 BUG0_64 open pit still reaches floor ornament");
    expect_bool("floor_pit.f0112_ceiling_pit", out.used_f0112_ceiling_pit, true,
                "DUNVIEW.C:7933-7934");
    expect_bool("floor_pit.f0113", out.used_f0113, false, "DUNVIEW.C:7939-7957 only teleporter");
}

static void test_wall_alcove_dispatch_only(void)
{
    const DM1_V1_D1CDispatchInputPc34 input = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_WALL, false, 0, 4, 0x4567, true
    };
    const DM1_V1_D1CDispatchOutputPc34 out = probe(input, "wall_alcove.probe");

    expect_int("wall_alcove.route", (int)out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_WALL_ALCOVE, "DUNVIEW.C:7784-7844");
    expect_bool("wall_alcove.wall_blit_flag", out.wall_blit_called, true,
                "DUNVIEW.C:7824-7840 wall bitmap path referenced only");
    expect_bool("wall_alcove.f0107", out.used_f0107_alcove, true, "DUNVIEW.C:7842");
    expect_bool("wall_alcove.f0115", out.used_f0115, true, "DUNVIEW.C:7843");
    expect_int("wall_alcove.cell_order", out.cell_order_called, 0x0000,
               "DEFS.H:2658 C0x0000_CELL_ORDER_ALCOVE");
    expect_int("wall_alcove.wall_view", out.view_wall_index, 14,
               "DEFS.H:2710 M587_VIEW_WALL_D1C_FRONT MEDIA720");
    expect_bool("wall_alcove.no_stairs_f0104", out.used_f0104, false,
                "DUNVIEW.C:7753 stairs case not taken");
    expect_bool("wall_alcove.no_field", out.used_f0113, false, "DUNVIEW.C:7939-7957 not reached");
}

static void test_wall_no_alcove_does_not_run_thing_pass(void)
{
    const DM1_V1_D1CDispatchInputPc34 input = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_WALL, false, 0, 0, 0x5678, false
    };
    const DM1_V1_D1CDispatchOutputPc34 out = probe(input, "wall_plain.probe");

    expect_int("wall_plain.route", (int)out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_WALL_NO_ALCOVE, "DUNVIEW.C:7784-7844");
    expect_bool("wall_plain.wall_blit_flag", out.wall_blit_called, true, "DUNVIEW.C:7824-7840");
    expect_bool("wall_plain.f0107_probe", out.used_f0107_alcove, true, "DUNVIEW.C:7842");
    expect_bool("wall_plain.no_f0115", out.used_f0115, false, "DUNVIEW.C:7842-7844 alcove false");
    expect_bool("wall_plain.no_cell_order", out.cell_order_called_valid, false,
                "DUNVIEW.C:7843 guarded by F0107 true");
    expect_bool("wall_plain.no_floor_ornament", out.used_f0108_floor_ornament, false,
                "DUNVIEW.C:7872 wall case returns");
    expect_bool("wall_plain.no_stairs_f0104", out.used_f0104, false,
                "DUNVIEW.C:7753 stairs case not taken");
}

static void test_open_floor_and_teleporter_tail(void)
{
    const DM1_V1_D1CDispatchInputPc34 floor = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR, false, 11, 0, 0x6789, false
    };
    const DM1_V1_D1CDispatchInputPc34 teleporter = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_TELEPORTER, false, 12, 0, 0x789a, false
    };
    const DM1_V1_D1CDispatchOutputPc34 floor_out = probe(floor, "open_floor.probe");
    const DM1_V1_D1CDispatchOutputPc34 teleporter_out = probe(teleporter, "teleporter.probe");

    expect_int("open_floor.route", (int)floor_out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_OPEN_FLOOR, "DUNVIEW.C:7922-7938");
    expect_bool("open_floor.no_f0104", floor_out.used_f0104, false, "DUNVIEW.C:7923");
    expect_bool("open_floor.f0115", floor_out.used_f0115, true, "DUNVIEW.C:7937");
    expect_bool("open_floor.no_f0113", floor_out.used_f0113, false,
                "DUNVIEW.C:7939-7957 teleporter-only field guard");
    expect_int("open_floor.cell_order", floor_out.cell_order_called, 0x3421,
               "DEFS.H:2676 C0x3421");
    expect_int("open_floor.floor_ornament_ordinal", floor_out.floor_ornament_ordinal, 11,
               "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL; DUNVIEW.C:7926");

    expect_int("teleporter.route", (int)teleporter_out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_TELEPORTER_FIELD, "DUNVIEW.C:7939-7957");
    expect_bool("teleporter.f0113", teleporter_out.used_f0113, true, "DUNVIEW.C:7955");
    expect_int("teleporter.field_zone", teleporter_out.zone_index, 712,
               "DEFS.H:4052 C712_ZONE_WALL_D1C; DUNVIEW.C:7955");
    expect_bool("teleporter.no_f0104", teleporter_out.used_f0104, false, "DUNVIEW.C:7922");
    expect_bool("teleporter.f0115_before_field", teleporter_out.used_f0115, true,
                "DUNVIEW.C:7937 precedes field guard");
}

static void test_d1c_feature_zone_pixel_sentinel_gate(void)
{
    const DM1_V1_D1CDispatchInputPc34 stairs_up = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT, true, 0, 0, 0, false
    };
    const DM1_V1_D1CDispatchInputPc34 stairs_down = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT, false, 0, 0, 0, false
    };
    const DM1_V1_D1CDispatchInputPc34 floor_pit = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR_PIT, false, 0, 0, 0, false
    };
    const DM1_V1_D1CDispatchInputPc34 teleporter = {
        DM1_V1_D1C_DISPATCH_PC34_ELEMENT_TELEPORTER, false, 0, 0, 0, false
    };
    unsigned char frame[D1C_SENTINEL_VIEWPORT_W * D1C_SENTINEL_VIEWPORT_H];
    DM1_V1_D1CDispatchOutputPc34 out;

    memset(frame, 0, sizeof(frame));
    out = probe(stairs_up, "pixel_stairs_up.probe");
    mark_d1c_feature_zone_pixel(frame, &out);
    expect_d1c_feature_pixels("pixel_stairs_up", frame,
                              D1C_SENTINEL_STAIRS_UP_PIXEL, 0, 0, 0);

    memset(frame, 0, sizeof(frame));
    out = probe(stairs_down, "pixel_stairs_down.probe");
    mark_d1c_feature_zone_pixel(frame, &out);
    expect_d1c_feature_pixels("pixel_stairs_down", frame,
                              0, D1C_SENTINEL_STAIRS_DOWN_PIXEL, 0, 0);

    memset(frame, 0, sizeof(frame));
    out = probe(floor_pit, "pixel_floor_pit.probe");
    mark_d1c_feature_zone_pixel(frame, &out);
    expect_d1c_feature_pixels("pixel_floor_pit", frame,
                              0, 0, D1C_SENTINEL_FLOOR_PIT_PIXEL, 0);

    memset(frame, 0, sizeof(frame));
    out = probe(teleporter, "pixel_teleporter.probe");
    mark_d1c_feature_zone_pixel(frame, &out);
    expect_d1c_feature_pixels("pixel_teleporter", frame,
                              0, 0, 0, D1C_SENTINEL_FIELD_PIXEL);
}

static void test_evidence_block(void)
{
    const DM1_V1_D1CDispatchEvidencePc34 *e =
        dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_evidence();

    expect_nonnull("evidence.nonnull", e, "source evidence");
    if (!e) return;
    expect_contains("evidence.f0124", e->draw_square_source_lines, "7727-7958",
                    "DUNVIEW.C:7727-7958");
    expect_contains("evidence.square_aspect", e->defs_square_aspect_lines, "2547-2559",
                    "DEFS.H:2547-2559");
    expect_contains("evidence.elements", e->defs_square_aspect_lines, "1007-1017",
                    "DEFS.H:1007-1017");
    expect_contains("evidence.m606", e->defs_square_aspect_lines, "M606",
                    "DEFS.H:2595-2600");
    expect_contains("evidence.m587", e->defs_square_aspect_lines, "M587",
                    "DEFS.H:2695-2710");
    expect_contains("evidence.c809", e->defs_zone_lines, "C809", "DEFS.H:4148");
    expect_contains("evidence.c822", e->defs_zone_lines, "C822", "DEFS.H:4161");
    expect_contains("evidence.c859", e->defs_zone_lines, "C859", "DEFS.H:4206");
    expect_contains("evidence.c712_d1c_not_d1r", e->defs_zone_lines, "C712",
                    "DEFS.H:4052 D1C field zone");
    expect_contains("evidence.stairs_up", e->stairs_up_source_lines, "7753-7763",
                    "DUNVIEW.C:7753-7763");
    expect_contains("evidence.stairs_down", e->stairs_down_source_lines, "7764-7781",
                    "DUNVIEW.C:7764-7781");
    expect_contains("evidence.floor_pit", e->floor_pit_source_lines, "7912-7921",
                    "DUNVIEW.C:7912-7921");
    expect_contains("evidence.wall", e->wall_source_lines, "7784-7844",
                    "DUNVIEW.C:7784-7844");
    expect_contains("evidence.field", e->field_source_lines, "7939-7957",
                    "DUNVIEW.C:7939-7957");
    expect_contains("evidence.dispatch", e->dispatch_source_lines, "8530-8535",
                    "DUNVIEW.C:8530-8535");
    expect_contains("evidence.non_overlap_wall", e->non_overlap_note, "F0100",
                    "non-overlap note");
    expect_contains("evidence.non_overlap_door", e->non_overlap_note, "F0111",
                    "non-overlap note");
    expect_contains("evidence.no_asset_parity", e->non_overlap_note, "real-asset bitmap parity",
                    "non-overlap note");
}

int main(void)
{
    test_stairs_up_front_media720_route();
    test_stairs_down_front_media720_route();
    test_floor_pit_route();
    test_wall_alcove_dispatch_only();
    test_wall_no_alcove_does_not_run_thing_pass();
    test_open_floor_and_teleporter_tail();
    test_d1c_feature_zone_pixel_sentinel_gate();
    test_evidence_block();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
