#include "dm1/dm1_v1_viewport_d2c_stairs_pit_dispatch_pc34_compat.h"

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

static void expect_size(const char *id, size_t got, size_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%zu want=%zu at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %zu (%s)\n", id, want, anchor);
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

static DM1_V1_D2CDispatchOutputPc34 probe(DM1_V1_D2CDispatchInputPc34 input,
                                          const char *id)
{
    DM1_V1_D2CDispatchOutputPc34 output;
    expect_bool(id,
                dm1_v1_viewport_d2c_stairs_pit_dispatch_probe_pc34(&input, &output),
                true,
                "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7256-7368");
    return output;
}

static void test_stairs_up_front_dispatch(void)
{
    const DM1_V1_D2CDispatchInputPc34 input = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT, true, false, 12, 0x1234
    };
    const DM1_V1_D2CDispatchOutputPc34 out = probe(input, "stairs_up.probe");

    expect_int("stairs_up.route", out.route_taken,
               DM1_V1_D2C_DISPATCH_PC34_ROUTE_STAIRS_UP_FRONT,
               "DUNVIEW.C:F0121:7257-7268");
    expect_bool("stairs_up.f0104", out.used_f0104, true,
                "DUNVIEW.C:F0121:7267; F0104:3113-3156");
    expect_bool("stairs_up.f0105_not_center", out.used_f0105, false,
                "DUNVIEW.C:F0121:7257-7288 D2C front stairs use F0104, not F0105");
    expect_int("stairs_up.slot", out.native_bitmap_index,
               DM1_V1_D2C_DISPATCH_PC34_STAIRS_UP_SLOT_D2C,
               "DEFS.H:2443-2445 C03; DUNVIEW.C:F0121:7267");
    expect_int("stairs_up.zone", out.zone_index,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_UP_D2C,
               "DEFS.H:4144-4146 C806; DUNVIEW.C:F0121:7267");
    expect_int("stairs_up.view_square", out.view_square_index,
               DM1_V1_D2C_DISPATCH_PC34_VIEW_SQUARE_D2C,
               "DEFS.H:2596-2604 M603_VIEW_SQUARE_D2C");
    expect_bool("stairs_up.tail_f0115", out.used_f0115, true,
                "DUNVIEW.C:F0121:7288,7355-7368");
    expect_int("stairs_up.cell_order", out.cell_order_called,
               DM1_V1_D2C_DISPATCH_PC34_CELL_ORDER_OPEN,
               "DUNVIEW.C:F0121:7355-7356; DEFS.H:2676");
}

static void test_stairs_down_front_dispatch(void)
{
    const DM1_V1_D2CDispatchInputPc34 input = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT, false, false, 13, 0x2345
    };
    const DM1_V1_D2CDispatchOutputPc34 out = probe(input, "stairs_down.probe");

    expect_int("stairs_down.route", out.route_taken,
               DM1_V1_D2C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT,
               "DUNVIEW.C:F0121:7269-7288");
    expect_bool("stairs_down.f0104", out.used_f0104, true,
                "DUNVIEW.C:F0121:7285; F0104:3113-3156");
    expect_int("stairs_down.slot", out.native_bitmap_index,
               DM1_V1_D2C_DISPATCH_PC34_STAIRS_DOWN_SLOT_D2C,
               "DEFS.H:2450-2452 C10; DUNVIEW.C:F0121:7285");
    expect_int("stairs_down.zone", out.zone_index,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D2C,
               "DEFS.H:4157-4159 C819; DUNVIEW.C:F0121:7285");
    expect_bool("stairs_down.tail_f0108", out.used_f0108_floor_ornament, true,
                "DUNVIEW.C:F0121:7288,7355-7357");
    expect_bool("stairs_down.tail_f0112", out.used_f0112_ceiling_pit, true,
                "DUNVIEW.C:F0121:7359-7365");
    expect_bool("stairs_down.tail_f0115", out.used_f0115, true,
                "DUNVIEW.C:F0121:7367-7368; F0115:4547-4581");
}

static void test_open_pit_bug0_64_order(void)
{
    const DM1_V1_D2CDispatchInputPc34 visible = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT, false, false, 21, 0x3456
    };
    const DM1_V1_D2CDispatchInputPc34 invisible = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT, false, true, 22, 0x4567
    };
    const DM1_V1_D2CDispatchOutputPc34 visible_out = probe(visible, "pit.visible.probe");
    const DM1_V1_D2CDispatchOutputPc34 invisible_out = probe(invisible,
                                                             "pit.invisible.probe");

    expect_int("pit.visible.route", visible_out.route_taken,
               DM1_V1_D2C_DISPATCH_PC34_ROUTE_OPEN_PIT,
               "DUNVIEW.C:F0121:7343-7352");
    expect_int("pit.visible.graphic", visible_out.native_bitmap_index,
               DM1_V1_D2C_DISPATCH_PC34_FLOOR_PIT_D2C_GRAPHIC,
               "DEFS.H:2331-2342 M757; DUNVIEW.C:F0121:7351");
    expect_int("pit.invisible.graphic", invisible_out.native_bitmap_index,
               DM1_V1_D2C_DISPATCH_PC34_INVISIBLE_FLOOR_PIT_D2C_GRAPHIC,
               "DEFS.H:2331-2342 M763; DUNVIEW.C:F0121:7351");
    expect_int("pit.zone", visible_out.zone_index,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_FLOOR_PIT_D2C,
               "DEFS.H:4202-4207 C856; DUNVIEW.C:F0121:7351");
    expect_bool("pit.f0104", visible_out.used_f0104, true,
                "DUNVIEW.C:F0121:7351; F0104:3113-3156");
    expect_bool("pit.bug0_64", visible_out.bug0_64_floor_ornament_after_open_pit, true,
                "DUNVIEW.C:F0121:7357 BUG0_64");
    expect_bool("pit.order.bitmap_before_ornament",
                visible_out.pit_bitmap_order < visible_out.floor_ornament_order,
                true,
                "DUNVIEW.C:F0121:7343-7357");
    expect_bool("pit.order.ornament_before_thing",
                visible_out.floor_ornament_order < visible_out.thing_pass_order,
                true,
                "DUNVIEW.C:F0121:7357-7368");
    expect_int("pit.ceiling.graphic", visible_out.ceiling_pit_graphic,
               DM1_V1_D2C_DISPATCH_PC34_CEILING_PIT_D2C_GRAPHIC,
               "DEFS.H:2248-2253 C065; DUNVIEW.C:F0121:7365");
    expect_int("pit.ceiling.zone", visible_out.ceiling_pit_zone,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_CEILING_PIT_D2C,
               "DEFS.H:4211-4218 C865; DUNVIEW.C:F0121:7365");
}

static void test_non_stair_pit_noop_and_tail_routes(void)
{
    const DM1_V1_D2CDispatchInputPc34 wall = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_WALL, false, false, 31, 0x5678
    };
    const DM1_V1_D2CDispatchInputPc34 corridor = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_CORRIDOR, false, false, 32, 0x6789
    };
    const DM1_V1_D2CDispatchInputPc34 unsupported = {99, false, false, 33, 0x789a};
    const DM1_V1_D2CDispatchOutputPc34 wall_out = probe(wall, "wall.probe");
    const DM1_V1_D2CDispatchOutputPc34 corridor_out = probe(corridor, "corridor.probe");
    const DM1_V1_D2CDispatchOutputPc34 unsupported_out = probe(unsupported,
                                                               "unsupported.probe");

    expect_int("wall.route", wall_out.route_taken,
               DM1_V1_D2C_DISPATCH_PC34_ROUTE_WALL_RETURN,
               "DUNVIEW.C:F0121:7289-7312");
    expect_bool("wall.no_f0104", wall_out.used_f0104, false,
                "DUNVIEW.C:F0121:7289-7312 wall path returns");
    expect_bool("wall.no_f0115", wall_out.used_f0115, false,
                "DUNVIEW.C:F0121:7312 returns before T0121016");
    expect_bool("wall.return_before_tail", wall_out.wall_returned_before_tail, true,
                "DUNVIEW.C:F0121:7308-7312");
    expect_int("corridor.route", corridor_out.route_taken,
               DM1_V1_D2C_DISPATCH_PC34_ROUTE_CORRIDOR_TAIL,
               "DUNVIEW.C:F0121:7353-7368");
    expect_bool("corridor.no_stair_pit_bitmap", corridor_out.used_f0104, false,
                "DUNVIEW.C:F0121:7353-7356");
    expect_bool("corridor.f0115", corridor_out.used_f0115, true,
                "DUNVIEW.C:F0121:7367-7368");
    expect_int("unsupported.route", unsupported_out.route_taken,
               DM1_V1_D2C_DISPATCH_PC34_ROUTE_UNSUPPORTED,
               "DUNVIEW.C:F0121 switch default has no stairs/pit dispatch");
    expect_bool("unsupported.no_f0104", unsupported_out.used_f0104, false,
                "DUNVIEW.C:F0121 switch default");
}

static void test_thing_pass_and_center_field_geometry(void)
{
    DM1_V1_D2CCenterGeometryPc34 geometry;
    const DM1_V1_D2CDispatchInputPc34 teleporter = {
        DM1_V1_D2C_DISPATCH_PC34_ELEMENT_TELEPORTER, false, true, 41, 0x89ab
    };
    const DM1_V1_D2CDispatchOutputPc34 out = probe(teleporter, "teleporter.probe");

    expect_bool("geometry.probe",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_center_geometry_pc34(10, 20,
                                                                              &geometry),
                true,
                "DUNVIEW.C:F0128:8520-8521 D2C relative movement");
    expect_int("geometry.depth", geometry.relative_depth, 2,
               "DUNVIEW.C:F0128:8520");
    expect_int("geometry.lateral", geometry.relative_lateral, 0,
               "DUNVIEW.C:F0128:8520");
    expect_int("geometry.map_x", geometry.resolved_map_x, 12,
               "DUNVIEW.C:F0128:8520");
    expect_int("geometry.map_y", geometry.resolved_map_y, 20,
               "DUNVIEW.C:F0128:8520");
    expect_int("geometry.view_square", geometry.view_square_index,
               DM1_V1_D2C_DISPATCH_PC34_VIEW_SQUARE_D2C,
               "DEFS.H:2596-2604 M603");
    expect_int("geometry.field_zone", geometry.field_zone_index,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_FIELD_D2C,
               "DEFS.H:4046-4052 C709");
    expect_bool("teleporter.field", out.used_f0113_field, true,
                "DUNVIEW.C:F0121:7373-7386");
    expect_int("teleporter.field_zone", out.zone_index,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_FIELD_D2C,
               "DUNVIEW.C:F0121:7385-7386; DEFS.H:4049");
    expect_bool("teleporter.f0115_before_field", out.used_f0115, true,
                "DUNVIEW.C:F0121:7367-7386");
}

static void test_f0105_transparency_contract(void)
{
    const uint8_t source[6] = {1, 10, 2, 3, 4, 10};
    uint8_t destination[8] = {99, 99, 99, 99, 99, 99, 99, 99};
    const DM1_V1_D2CF0105BlitInputPc34 input = {
        source, sizeof(source), destination, sizeof(destination), 3, 2, 4,
        DM1_V1_D2C_DISPATCH_PC34_STAIRS_DOWN_SLOT_D2C,
        DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D2C
    };
    DM1_V1_D2CF0105BlitOutputPc34 output;

    expect_bool("f0105.blit",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_f0105_blit_pc34(&input, &output),
                true,
                "DUNVIEW.C:F0105:3185-3247");
    expect_bool("f0105.used", output.used_f0105, true,
                "DUNVIEW.C:F0105:3185");
    expect_bool("f0105.flip", output.copied_with_horizontal_flip, true,
                "DUNVIEW.C:F0105:3218-3239");
    expect_int("f0105.transparent_color", output.transparent_color,
               DM1_V1_D2C_DISPATCH_PC34_COLOR_TRANSPARENT,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_size("f0105.writes", output.writes, 4,
                "DUNVIEW.C:F0105:3218-3239 C10 transparent skip");
    expect_size("f0105.skips", output.transparent_skips, 2,
                "DUNVIEW.C:F0105:3218-3239 C10 transparent skip");
    expect_int("f0105.dest0", destination[0], 2,
               "DUNVIEW.C:F0105 horizontal flip copies row 0 from right");
    expect_int("f0105.dest1.skip", destination[1], 99,
               "DEFS.H:2088 transparent C10 leaves destination unchanged");
    expect_int("f0105.dest4.skip", destination[4], 99,
               "DEFS.H:2088 transparent C10 leaves destination unchanged");
    expect_int("f0105.dest5", destination[5], 4,
               "DUNVIEW.C:F0105 horizontal flip row 1");
    expect_int("f0105.zone", output.zone_index,
               DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D2C,
               "DUNVIEW.C:F0105:3193-3195 zone parameter");
}

static void test_metadata_bindings(void)
{
    const uint8_t open_pit =
        (uint8_t)((DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT << 5) |
                  DM1_V1_D2C_DISPATCH_PC34_MASK_PIT_OPEN);
    const uint8_t invisible_pit =
        (uint8_t)(open_pit | DM1_V1_D2C_DISPATCH_PC34_MASK_PIT_INVISIBLE);
    const uint8_t closed_pit =
        (uint8_t)(DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT << 5);
    const uint8_t stairs_front_up =
        (uint8_t)((DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS << 5) |
                  DM1_V1_D2C_DISPATCH_PC34_MASK_STAIRS_UP);
    const uint8_t stairs_side_down =
        (uint8_t)((DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS << 5) |
                  DM1_V1_D2C_DISPATCH_PC34_MASK_STAIRS_NS);
    DM1_V1_D2CMetadataOutputPc34 out;

    expect_bool("metadata.open_pit",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
                    &(const DM1_V1_D2CMetadataInputPc34){open_pit, 0, 51, 0x1111},
                    &out),
                true,
                "DUNGEON.C:F0172:2628-2650");
    expect_int("metadata.open_pit.element", out.element,
               DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT,
               "DUNGEON.C:F0172:2628-2631");
    expect_bool("metadata.open_pit.visible_flag", out.pit_or_teleporter_visible,
                false,
                "DUNGEON.C:F0172:2629-2631");
    expect_int("metadata.open_pit.first_thing", out.first_thing, 0x1111,
               "DUNGEON.C:F0163:1769-1838; F0164:1840-1905; F0172:2721");

    expect_bool("metadata.invisible_pit",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
                    &(const DM1_V1_D2CMetadataInputPc34){invisible_pit, 0, 52, 0x2222},
                    &out),
                true,
                "DUNGEON.C:F0172:2628-2650");
    expect_bool("metadata.invisible_pit.visible_flag", out.pit_or_teleporter_visible,
                true,
                "DUNGEON.C:F0172:2629-2631");

    expect_bool("metadata.closed_pit",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
                    &(const DM1_V1_D2CMetadataInputPc34){closed_pit, 0, 53, 0x3333},
                    &out),
                true,
                "DUNGEON.C:F0172:2632-2650");
    expect_int("metadata.closed_pit.element", out.element,
               DM1_V1_D2C_DISPATCH_PC34_ELEMENT_CORRIDOR,
               "DUNGEON.C:F0172:2647");
    expect_bool("metadata.closed_pit.footprints", out.footprints_allowed, true,
                "DUNGEON.C:F0172:2648");

    expect_bool("metadata.stairs_front",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
                    &(const DM1_V1_D2CMetadataInputPc34){stairs_front_up, 1, 54, 0x4444},
                    &out),
                true,
                "DUNGEON.C:F0172:2693-2697");
    expect_int("metadata.stairs_front.element", out.element,
               DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT,
               "DUNGEON.C:F0172:2693-2694");
    expect_bool("metadata.stairs_front.up", out.stairs_up, true,
                "DUNGEON.C:F0172:2695");
    expect_bool("metadata.stairs_front.no_footprints", out.footprints_allowed, false,
                "DUNGEON.C:F0172:2696");

    expect_bool("metadata.stairs_side",
                dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
                    &(const DM1_V1_D2CMetadataInputPc34){stairs_side_down, 1, 55, 0x5555},
                    &out),
                true,
                "DUNGEON.C:F0172:2693-2697");
    expect_int("metadata.stairs_side.element", out.element,
               DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_SIDE,
               "DUNGEON.C:F0172:2693-2694");
    expect_bool("metadata.stairs_side.up", out.stairs_up, false,
                "DUNGEON.C:F0172:2695");
}

static void test_followups_and_evidence(void)
{
    size_t count = 0;
    const DM1_V1_D2CFollowUpWritePc34 *followups =
        dm1_v1_viewport_d2c_stairs_pit_dispatch_followups_pc34(&count);
    const DM1_V1_D2CDispatchEvidencePc34 *e =
        dm1_v1_viewport_d2c_stairs_pit_dispatch_evidence_pc34();

    expect_nonnull("followups.nonnull", followups, "DUNVIEW.C:F0128:8503-8517");
    expect_size("followups.count", count, 4, "DUNVIEW.C:F0128:8503-8517");
    expect_int("followups.d2l2.depth", followups[0].relative_depth, 2,
               "DUNVIEW.C:F0128:8503-8504");
    expect_int("followups.d2l2.lateral", followups[0].relative_lateral, -2,
               "DUNVIEW.C:F0128:8503-8504");
    expect_int("followups.d2r2.lateral", followups[1].relative_lateral, 2,
               "DUNVIEW.C:F0128:8507-8508");
    expect_int("followups.d2l.lateral", followups[2].relative_lateral, -1,
               "DUNVIEW.C:F0128:8512-8513");
    expect_int("followups.d2r.lateral", followups[3].relative_lateral, 1,
               "DUNVIEW.C:F0128:8516-8517");
    expect_contains("followups.name", followups[3].function_name, "F0120",
                    "DUNVIEW.C:F0128:8517");

    expect_nonnull("evidence.nonnull", e, "source evidence");
    if (!e) return;
    expect_contains("evidence.f0121", e->f0121_body_lines, "7256-7368",
                    "DUNVIEW.C:F0121");
    expect_contains("evidence.f0104", e->f0104_lines, "3113-3156",
                    "DUNVIEW.C:F0104");
    expect_contains("evidence.f0105", e->f0105_lines, "3185-3247",
                    "DUNVIEW.C:F0105");
    expect_contains("evidence.f0115", e->f0115_lines, "4547-4581",
                    "DUNVIEW.C:F0115");
    expect_contains("evidence.f0128", e->f0128_lines, "8503-8517",
                    "DUNVIEW.C:F0128");
    expect_contains("evidence.dungeon.f0163", e->dungeon_metadata_lines, "F0163",
                    "DUNGEON.C:F0163:1769-1838");
    expect_contains("evidence.dungeon.f0172", e->dungeon_metadata_lines, "F0172",
                    "DUNGEON.C:F0172:2466-2523");
    expect_contains("evidence.defs.c10", e->defs_lines, "C10",
                    "DEFS.H:2088");
    expect_contains("evidence.defs.zone", e->defs_lines, "4144-4162",
                    "DEFS.H:4144-4162");
    expect_contains("evidence.contract", e->contract_note, "Contract-only",
                    "contract-only gate");
}

int main(void)
{
    test_stairs_up_front_dispatch();
    test_stairs_down_front_dispatch();
    test_open_pit_bug0_64_order();
    test_non_stair_pit_noop_and_tail_routes();
    test_thing_pass_and_center_field_geometry();
    test_f0105_transparency_contract();
    test_metadata_bindings();
    test_followups_and_evidence();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
