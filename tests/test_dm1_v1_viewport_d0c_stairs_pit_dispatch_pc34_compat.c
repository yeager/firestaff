#include "src/dm1/dm1_v1_viewport_d0c_stairs_pit_dispatch_pc34_compat.h"

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

static const DM1_V1_D0CStairsPitDispatchContractPc34 *contract(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c =
        dm1_v1_viewport_d0c_stairs_pit_dispatch_contract_pc34_compat();
    expect_nonnull("contract.nonnull", c,
                   "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310");
    return c;
}

static void test_contract_identity_and_anchors(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_bool("contract.contract_only", c->contract_only, true,
                "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310");
    expect_contains("anchor.contract_only", c->s_contract_anchor, "contract_only=1",
                    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310");
    expect_contains("anchor.d0c_function", c->s_d0c_anchor,
                    "F0127_DUNGEONVIEW_DrawSquareD0C",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310");
    expect_contains("anchor.dispatch", c->s_dispatch_anchor, "8538-8542",
                    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8538-8542");
    expect_contains("anchor.elements", c->s_defs_element_anchor, "C19_ELEMENT_STAIRS_FRONT",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8274; DEFS.H:1007-1017");
    expect_contains("anchor.elements.pit", c->s_defs_element_anchor, "C02_ELEMENT_PIT",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8284; DEFS.H:1007-1017");
    expect_contains("anchor.square_aspect", c->s_defs_square_aspect_anchor,
                    "M609_VIEW_SQUARE_D0C",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294; DEFS.H:2573-2602");
}

static void test_f0098_precedes_stairs_and_pit_overrides(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("f0098.anchor", c->s_f0098_anchor,
                    "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
                    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339");
    expect_int("f0098.order", c->f0098_order, 10,
               "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339");
    expect_int("d0c.switch.order", c->d0c_switch_order, 20,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8184-8185");
    expect_int("stairs.override.order", c->stairs_override_order, 30,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8273");
    expect_int("pit.override.order", c->pit_override_order, 30,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8284");
    expect_bool("f0098.before.stairs", c->f0098_order < c->stairs_override_order, true,
                "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8273");
    expect_bool("f0098.before.pit", c->f0098_order < c->pit_override_order, true,
                "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8284");
    expect_bool("f0098.contract.before.stairs", c->f0098_precedes_stairs_override, true,
                "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8273");
    expect_bool("f0098.contract.before.pit", c->f0098_precedes_pit_override, true,
                "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8284");
}

static void test_stairs_down_excludes_thing_pass_and_floor_ornament(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("stairs.down.anchor", c->s_stairs_down_anchor, "8255-8273",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273");
    expect_int("stairs.down.slot.left", c->stairs_down_slot_left, 13,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8265-8270; DEFS.H:2440-2454");
    expect_int("stairs.down.zone.left", c->stairs_down_zone_left, 824,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8265; DEFS.H:4163");
    expect_int("stairs.down.zone.right", c->stairs_down_zone_right, 825,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8270; DEFS.H:4164");
    expect_bool("stairs.down.no_f0115", c->stairs_down_calls_f0115, false,
                "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273 breaks before 8294 F0115");
    expect_bool("stairs.down.no_f0108", c->stairs_down_calls_f0108_floor_ornament, false,
                "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273 breaks before DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-3980");
    expect_contains("stairs.no_f0108.anchor", c->s_no_f0108_anchor, "F0108",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273; DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-3980");
}

static void test_stairs_up_uses_both_d0c_halves(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("stairs.up.anchor", c->s_stairs_up_anchor, "8242-8254",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8242-8254");
    expect_int("stairs.up.slot.left", c->stairs_up_slot_left, 6,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8252-8253; DEFS.H:2440-2454");
    expect_int("stairs.up.zone.left", c->stairs_up_zone_left, 811,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8252; DEFS.H:4150");
    expect_int("stairs.up.zone.right", c->stairs_up_zone_right, 812,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8253; DEFS.H:4151");
}

static void test_pit_keeps_thing_pass_enabled(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("pit.anchor", c->s_pit_anchor, "8274-8284",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8284");
    expect_int("pit.floor.graphic", c->floor_pit_graphic, 57,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8282; DEFS.H:2338-2346");
    expect_int("pit.floor.invisible.graphic", c->invisible_floor_pit_graphic, 63,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8282; DEFS.H:2338-2346");
    expect_int("pit.floor.zone", c->floor_pit_zone, 862,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8282; DEFS.H:4209");
    expect_int("pit.ceiling.order", c->ceiling_pit_order, 40,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293");
    expect_int("pit.thing.order", c->thing_pass_order, 50,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294");
    expect_bool("pit.f0115.enabled", c->pit_calls_f0115, true,
                "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8294");
    expect_bool("pit.ceiling.before.thing", c->ceiling_pit_order < c->thing_pass_order, true,
                "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8294");
    expect_int("pit.cell_order", c->cell_order_backleft_backright, 0x0021,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294; DEFS.H:2656-2663");
    expect_int("pit.view_square.media720", c->media720_view_square_d0c, 0,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294; DEFS.H:2595-2602");
}

static void test_ceiling_dispatch_excludes_row_flip(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("ceiling.anchor", c->s_ceiling_pit_anchor, "8285-8293",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293");
    expect_int("ceiling.graphic", c->ceiling_pit_graphic, 69,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8292; DEFS.H:2253");
    expect_int("ceiling.zone", c->ceiling_pit_zone, 871,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8292; DEFS.H:4218");
    expect_bool("ceiling.no_f0099", c->ceiling_dispatch_calls_f0099, false,
                "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293 excludes F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal");
    expect_contains("ceiling.no_f0099.anchor", c->s_no_f0099_anchor, "excludes F0099",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293");
}

static void test_field_tail_is_after_ordinary_thing_pass(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("thing.anchor", c->s_thing_pass_anchor, "8294",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294");
    expect_contains("field.anchor", c->s_field_anchor, "8295-8310",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8295-8310");
    expect_int("field.order", c->field_order, 60,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8295-8310");
    expect_bool("thing.before.field", c->thing_pass_order < c->field_order, true,
                "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294-8310");
    expect_int("field.zone", c->field_zone, 715,
               "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8308; DEFS.H:4055");
}

static void test_source_evidence_mentions_required_paths(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c = contract();
    if (!c) return;

    expect_contains("evidence.contract", c->s_source_evidence, "contract_only=1",
                    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310");
    expect_contains("evidence.f0098", c->s_source_evidence, "F0098",
                    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339");
    expect_contains("evidence.stairs", c->s_source_evidence, "stairs",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8273");
    expect_contains("evidence.pit", c->s_source_evidence, "pit",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8294");
    expect_contains("evidence.no_f0099", c->s_source_evidence, "excludes F0099",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293");
    expect_contains("evidence.no_f0108", c->s_source_evidence, "excludes F0115 and F0108",
                    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273; DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-3980");
}

int main(void)
{
    test_contract_identity_and_anchors();
    test_f0098_precedes_stairs_and_pit_overrides();
    test_stairs_down_excludes_thing_pass_and_floor_ornament();
    test_stairs_up_uses_both_d0c_halves();
    test_pit_keeps_thing_pass_enabled();
    test_ceiling_dispatch_excludes_row_flip();
    test_field_tail_is_after_ordinary_thing_pass();
    test_source_evidence_mentions_required_paths();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
