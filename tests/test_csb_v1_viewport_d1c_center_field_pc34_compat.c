#include "csb_v1_viewport_d1c_center_field_pc34_compat.h"

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

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static CSB_V1_D1CCenterFieldInputPc34 base_input(int element, bool has_alcove)
{
    CSB_V1_D1CCenterFieldInputPc34 in = {
        CSB_V1_D1C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX,
        CSB_V1_D1C_CENTER_FIELD_PC34_LANE,
        CSB_V1_D1C_CENTER_FIELD_PC34_DEPTH,
        CSB_V1_D1C_CENTER_FIELD_PC34_FIELD_ASPECT,
        has_alcove,
        element
    };
    return in;
}

static void test_i34_index_and_evidence_contract(void)
{
    const CSB_V1_D1CCenterFieldEvidencePc34 *ev =
        csb_v1_viewport_d1c_center_field_pc34_compat_evidence();

    expect_nonnull("d1c.evidence", ev, "ReDMCSB DUNVIEW.C:370-377");
    if (!ev) return;

    expect_int("d1c.view_square_index",
               CSB_V1_D1C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX, 3,
               "ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C");
    expect_int("d1c.lane", CSB_V1_D1C_CENTER_FIELD_PC34_LANE, 0,
               "ReDMCSB DUNVIEW.C:371 G2026[3]");
    expect_int("d1c.depth", CSB_V1_D1C_CENTER_FIELD_PC34_DEPTH, 1,
               "ReDMCSB DUNVIEW.C:372 G2027[3]");
    expect_int("d1c.field_aspect", CSB_V1_D1C_CENTER_FIELD_PC34_FIELD_ASPECT, 10,
               "ReDMCSB DUNVIEW.C:377 G2035[3]");
    expect_int("d1c.view_wall_front",
               CSB_V1_D1C_CENTER_FIELD_PC34_VIEW_WALL_D1C_FRONT, 14,
               "ReDMCSB DEFS.H:2710 M587_VIEW_WALL_D1C_FRONT");
    expect_contains("d1c.evidence.array", ev->array_anchor, "index 3",
                    "ReDMCSB DUNVIEW.C:370-377");
    expect_contains("d1c.evidence.dispatch", ev->dispatch_anchor, "8532-8533",
                    "ReDMCSB DUNVIEW.C:8532-8533");
    expect_contains("d1c.evidence.local_f0124", ev->body_anchor, "F0124",
                    "ReDMCSB DUNVIEW.C:7727");
    expect_contains("d1c.evidence.f0122_d1l_note", ev->body_anchor, "F0122",
                    "ReDMCSB DUNVIEW.C:8524-8525");
}

static void test_wall_routes(void)
{
    CSB_V1_D1CCenterFieldOutputPc34 wall_alcove =
        csb_v1_viewport_d1c_center_field_pc34_compat_probe(
            base_input(CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL, true));
    CSB_V1_D1CCenterFieldOutputPc34 wall_plain =
        csb_v1_viewport_d1c_center_field_pc34_compat_probe(
            base_input(CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL, false));

    expect_int("d1c.wall_alcove.route", (int)wall_alcove.route_taken,
               CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_FRONT_ALCOVE,
               "ReDMCSB DUNVIEW.C:7784-7872");
    expect_int("d1c.wall_alcove.wall_zone", wall_alcove.wall_zone_index, 712,
               "ReDMCSB DUNVIEW.C:7833-7840; DEFS.H:4052");
    expect_bool("d1c.wall_alcove.used_f0100", wall_alcove.used_f0100, true,
                "ReDMCSB DUNVIEW.C:7825");
    expect_bool("d1c.wall_alcove.used_f0107", wall_alcove.used_f0107_alcove, true,
                "ReDMCSB DUNVIEW.C:7842");
    expect_bool("d1c.wall_alcove.used_f0115", wall_alcove.used_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7843");
    expect_int("d1c.wall_alcove.cell_order", (int)wall_alcove.cell_order, 0x0000,
               "ReDMCSB DEFS.H:2658; DUNVIEW.C:7843");
    expect_bool("d1c.wall_alcove.no_f0113", wall_alcove.used_f0113_field, false,
                "ReDMCSB DUNVIEW.C:7872");
    expect_int("d1c.wall_plain.route", (int)wall_plain.route_taken,
               CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_FRONT_NO_ALCOVE,
               "ReDMCSB DUNVIEW.C:7784-7872");
    expect_bool("d1c.wall_plain.used_f0107", wall_plain.used_f0107_alcove, true,
                "ReDMCSB DUNVIEW.C:7842");
    expect_bool("d1c.wall_plain.no_f0115", wall_plain.used_f0115_thing_pass, false,
                "ReDMCSB DUNVIEW.C:7842-7872");
    expect_bool("d1c.wall_plain.no_f0113", wall_plain.used_f0113_field, false,
                "ReDMCSB DUNVIEW.C:7872");
}

static void test_no_wall_routes(void)
{
    CSB_V1_D1CCenterFieldOutputPc34 corridor =
        csb_v1_viewport_d1c_center_field_pc34_compat_probe(
            base_input(CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR, false));
    CSB_V1_D1CCenterFieldOutputPc34 teleporter =
        csb_v1_viewport_d1c_center_field_pc34_compat_probe(
            base_input(CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER, false));
    CSB_V1_D1CCenterFieldOutputPc34 door =
        csb_v1_viewport_d1c_center_field_pc34_compat_probe(
            base_input(CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR_FRONT, false));

    expect_int("d1c.corridor.route", (int)corridor.route_taken,
               CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_CORRIDOR_OPEN,
               "ReDMCSB DUNVIEW.C:7922-7937");
    expect_bool("d1c.corridor.no_f0100", corridor.used_f0100, false,
                "ReDMCSB DUNVIEW.C:7922-7937");
    expect_bool("d1c.corridor.no_f0107", corridor.used_f0107_alcove, false,
                "ReDMCSB DUNVIEW.C:7922-7937");
    expect_bool("d1c.corridor.no_f0113", corridor.used_f0113_field, false,
                "ReDMCSB DUNVIEW.C:7939-7956");
    expect_bool("d1c.corridor.used_f0115", corridor.used_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7937");
    expect_int("d1c.teleporter.route", (int)teleporter.route_taken,
               CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_TELEPORTER_FIELD,
               "ReDMCSB DUNVIEW.C:7942-7956");
    expect_int("d1c.teleporter.field_zone", teleporter.field_zone_index, 712,
               "ReDMCSB DUNVIEW.C:7955; DEFS.H:4052");
    expect_bool("d1c.teleporter.used_f0113", teleporter.used_f0113_field, true,
                "ReDMCSB DUNVIEW.C:7955");
    expect_bool("d1c.teleporter.no_f0100", teleporter.used_f0100, false,
                "ReDMCSB DUNVIEW.C:7922-7956");
    expect_bool("d1c.teleporter.no_f0107", teleporter.used_f0107_alcove, false,
                "ReDMCSB DUNVIEW.C:7922-7956");
    expect_bool("d1c.teleporter.used_f0115", teleporter.used_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7937");
    expect_int("d1c.door.route", (int)door.route_taken,
               CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_DOOR_FRONT,
               "ReDMCSB DUNVIEW.C:7873-7911");
    expect_int("d1c.door.cell_order", (int)door.cell_order, 0x0218,
               "ReDMCSB DEFS.H:2669; DUNVIEW.C:7875");
    expect_bool("d1c.door.used_f0115", door.used_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7875");
    expect_bool("d1c.door.no_f0113", door.used_f0113_field, false,
                "ReDMCSB DUNVIEW.C:7873-7911");
}

static void test_rejection_and_zone_constants(void)
{
    CSB_V1_D1CCenterFieldInputPc34 wrong = base_input(
        CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER, false);
    CSB_V1_D1CCenterFieldOutputPc34 rejected;

    wrong.view_square_index = 9;
    rejected = csb_v1_viewport_d1c_center_field_pc34_compat_probe(wrong);

    expect_int("d1c.reject.route", (int)rejected.route_taken,
               CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_INVALID,
               "ReDMCSB DEFS.H:2599 rejects non-I34 M606 index");
    expect_int("d1c.reject.wall_zone", rejected.wall_zone_index, -1,
               "contract helper rejects unresolved square");
    expect_int("d1c.media508.d1c_zone",
               CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA508_ZONE_WALL_D1C, 710,
               "ReDMCSB DEFS.H:4033 C710_ZONE_WALL_D1C");
    expect_int("d1c.media720.d1c_zone",
               CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_WALL_D1C, 712,
               "ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C");
    expect_int("d1c.media720.d1r_neighbor_zone",
               CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_WALL_D1R, 714,
               "ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R");
    expect_int("d1c.media720.stairs_up_d1c",
               CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_STAIRS_UP_FRONT_D1C, 809,
               "ReDMCSB DEFS.H:4148 C809_ZONE_STAIRS_UP_FRONT_D1C");
    expect_int("d1c.media720.stairs_up_d1r",
               CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_STAIRS_UP_FRONT_D1R, 810,
               "ReDMCSB DEFS.H:4149 C810_ZONE_STAIRS_UP_FRONT_D1R");
    expect_int("d1c.media720.stairs_down_d1c",
               CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_STAIRS_DOWN_FRONT_D1C, 822,
               "ReDMCSB DEFS.H:4161 C822_ZONE_STAIRS_DOWN_FRONT_D1C");
}

static void test_source_evidence_mentions_all_anchors(void)
{
    const char *e =
        csb_v1_viewport_d1c_center_field_pc34_compat_source_evidence();
    const CSB_V1_D1CCenterFieldEvidencePc34 *ev =
        csb_v1_viewport_d1c_center_field_pc34_compat_evidence();

    expect_nonnull("source_evidence.nonnull", e, "source evidence");
    expect_contains("source_evidence.contract", e, "Source-locked contract gate only",
                    "contract marker");
    expect_contains("source_evidence.no_asset_parity", e, "not full real-asset",
                    "contract marker");
    expect_contains("source_evidence.arrays", e, "DUNVIEW.C:370-377",
                    "ReDMCSB DUNVIEW.C:370-377");
    expect_contains("source_evidence.dispatch", e, "DUNVIEW.C:8532-8533",
                    "ReDMCSB DUNVIEW.C:8532-8533");
    expect_contains("source_evidence.wall", e, "DUNVIEW.C:7784-7872",
                    "ReDMCSB DUNVIEW.C:7784-7872");
    expect_contains("source_evidence.alcove", e, "DUNVIEW.C:7842-7843",
                    "ReDMCSB DUNVIEW.C:7842-7843");
    expect_contains("source_evidence.door", e, "DUNVIEW.C:7873-7911",
                    "ReDMCSB DUNVIEW.C:7873-7911");
    expect_contains("source_evidence.field", e, "DUNVIEW.C:7955",
                    "ReDMCSB DUNVIEW.C:7955");
    expect_contains("source_evidence.c712", e, "C712_ZONE_WALL_D1C",
                    "ReDMCSB DEFS.H:4052");
    expect_contains("source_evidence.c714", e, "C714",
                    "ReDMCSB DEFS.H:4054");
    expect_contains("source_evidence.c810", e, "C809/C810",
                    "ReDMCSB DEFS.H:4148-4149");
    expect_contains("source_evidence.c822", e, "C822",
                    "ReDMCSB DEFS.H:4161");
    expect_contains("evidence.zone_anchor", ev ? ev->zone_anchor : NULL, "C810",
                    "ReDMCSB DEFS.H:4149");
    expect_contains("evidence.source_note", ev ? ev->source_note : NULL,
                    "ambiguous", "contract note");
}

int main(void)
{
    test_i34_index_and_evidence_contract();
    test_wall_routes();
    test_no_wall_routes();
    test_rejection_and_zone_constants();
    test_source_evidence_mentions_all_anchors();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
