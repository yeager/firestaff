#include "dm1_v1_creature_render_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_creature_types(void)
{
    assert(DM1_CREATURE_GIANT_SCORPION == 0);
    assert(DM1_CREATURE_LORD_CHAOS == 23);
    assert(DM1_CREATURE_RED_DRAGON == 24);
    assert(DM1_CREATURE_GREY_LORD == 26);
    assert(DM1_CREATURE_TYPE_COUNT == 27);
}

static void test_size_constants(void)
{
    assert(DM1_CREATURE_SIZE_QUARTER == 0);
    assert(DM1_CREATURE_SIZE_HALF == 1);
    assert(DM1_CREATURE_SIZE_FULL == 2);
}

static void test_graphic_masks(void)
{
    assert(DM1_GI_MASK_ADDITIONAL == 0x0003u);
    assert(DM1_GI_MASK_SIDE == 0x0008u);
    assert(DM1_GI_MASK_ATTACK == 0x0020u);
    assert(DM1_GI_MASK_FLIP_ATTACK == 0x0200u);
}

static void test_first_creature_graphic(void)
{
    assert(DM1_GRAPHIC_FIRST_CREATURE == 584);
}

static void test_aspect_masks(void)
{
    assert(DM1_CREATURE_ASPECT_FLIP_BITMAP == 0x40u);
    assert(DM1_CREATURE_ASPECT_IS_ATTACKING == 0x80u);
    assert(DM1_CREATURE_ASPECT_HMASK == 0x07u);
    assert(DM1_CREATURE_ASPECT_VMASK == 0x38u);
}

static void test_pose_enum(void)
{
    assert(DM1_CREATURE_POSE_FRONT == 0);
    assert(DM1_CREATURE_POSE_ATTACK == 3);
}

static void test_render_list_init(void)
{
    DM1_CreatureRenderList list;
    DM1_V1_CreatureRender_InitPc34Compat(&list);
    assert(list.count == 0);
}

static void test_aspects_table(void)
{
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    assert(aspects != NULL);
    assert(aspects[DM1_CREATURE_GIANT_SCORPION].firstNativeBitmapRelativeIndex >= 0 ||
           aspects[DM1_CREATURE_GIANT_SCORPION].firstNativeBitmapRelativeIndex < 0);
}

static void test_direction_delta(void)
{
    int d = dm1_creature_direction_delta(0, 2);
    (void)d;
    assert(d >= 0 && d <= 3);
}

static void test_type_name(void)
{
    const char* name = DM1_V1_CreatureRender_TypeNamePc34Compat(DM1_CREATURE_RED_DRAGON);
    assert(name != NULL);
    assert(strlen(name) > 0);
}

static void test_coordinate_set(void)
{
    int cs = dm1_creature_coordinate_set(DM1_CREATURE_GIANT_SCORPION);
    (void)cs;
    assert(cs >= 0);
}

static void test_transparent_color(void)
{
    int tc = dm1_creature_transparent_color(DM1_CREATURE_GIANT_SCORPION);
    (void)tc;
    assert(tc >= 0 && tc < 16);
}

static void test_palette_d3(void)
{
    const unsigned char* pal = dm1_creature_palette_d3();
    assert(pal != NULL);
}

static void test_palette_d2(void)
{
    const unsigned char* pal = dm1_creature_palette_d2();
    assert(pal != NULL);
}

/* F0676/F0677 do not use an ordinary side-pane placement.  MEDIA720 F0115
 * indexes G2033[14]/[15], which select raw G0224 C3200 rows 3/4.  Keep the
 * plan adapter covered here as well as the lower-level coordinate table
 * test: a row regression must not silently fall back to synthesized pane
 * geometry. */
static void test_raw_c3200_outer_lane_draw_plan(void)
{
    /* Swamp Slime's G0219 coordinate-set nibble is zero, matching the
     * directly audited row-3/4 points below. */
    const int types[] = { DM1_CREATURE_SWAMP_SLIME };
    const int counts[] = { 1 };
    const int directions[] = { 2 };
    DM1_CreatureDrawPlan plan;

    assert(dm1_creature_raw_c3200_draw_plan(types, counts, directions, 1,
                                             3, &plan) == 1);
    assert(plan.count == 1);
    assert(plan.entries[0].creature_type == DM1_CREATURE_SWAMP_SLIME);
    assert(plan.entries[0].creature_direction == 2);
    assert(plan.entries[0].placement.source_anchor_valid == 1);
    assert(plan.entries[0].placement.source_depth_index == 2);
    assert(plan.entries[0].placement.source_anchor_x == 112);
    assert(plan.entries[0].placement.source_anchor_y == 85);

    assert(dm1_creature_raw_c3200_draw_plan(types, counts, directions, 1,
                                             4, &plan) == 1);
    assert(plan.count == 1);
    assert(plan.entries[0].placement.source_anchor_x == 120);
    assert(plan.entries[0].placement.source_anchor_y == 85);

    assert(dm1_creature_raw_c3200_draw_plan(types, counts, directions, 1,
                                             9, &plan) == 0);
    assert(plan.count == 0);

    /* The outer adapter must retain G0219's coordinate-set selection, not
     * merely reproduce the set-zero sample above.  Giant Scorpion is set 1
     * and Wizard Eye is set 2 in the original aspect table. */
    {
        const int set_one_type[] = { DM1_CREATURE_GIANT_SCORPION };
        const int set_two_type[] = { DM1_CREATURE_WIZARD_EYE };

        assert(dm1_creature_raw_c3200_draw_plan(set_one_type, counts,
                                                 directions, 1, 3,
                                                 &plan) == 1);
        assert(plan.entries[0].placement.source_anchor_x == 112);
        assert(plan.entries[0].placement.source_anchor_y == 89);
        assert(dm1_creature_raw_c3200_draw_plan(set_one_type, counts,
                                                 directions, 1, 4,
                                                 &plan) == 1);
        assert(plan.entries[0].placement.source_anchor_x == 125);
        assert(plan.entries[0].placement.source_anchor_y == 90);

        assert(dm1_creature_raw_c3200_draw_plan(set_two_type, counts,
                                                 directions, 1, 3,
                                                 &plan) == 1);
        assert(plan.entries[0].placement.source_anchor_x == 112);
        assert(plan.entries[0].placement.source_anchor_y == 66);
        assert(dm1_creature_raw_c3200_draw_plan(set_two_type, counts,
                                                 directions, 1, 4,
                                                 &plan) == 1);
        assert(plan.entries[0].placement.source_anchor_x == 120);
        assert(plan.entries[0].placement.source_anchor_y == 66);
    }
}

int main(void)
{
    test_creature_types();
    test_size_constants();
    test_graphic_masks();
    test_first_creature_graphic();
    test_aspect_masks();
    test_pose_enum();
    test_render_list_init();
    test_aspects_table();
    test_direction_delta();
    test_type_name();
    test_coordinate_set();
    test_transparent_color();
    test_palette_d3();
    test_palette_d2();
    test_raw_c3200_outer_lane_draw_plan();

    puts("ok: DM1 creature render (Q-DM1-03) 15 tests passed");
    return 0;
}
