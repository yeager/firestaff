/* Test DM2 creature AI specification table and lookup.
 * Verifies table values against kaitable.h dAITableGenuine. */
#include "dm2_v1_creature_ai_spec_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

static uint16_t mock_gdat_direct(void *ctx, uint8_t creature_type, uint8_t stat_id)
{
    (void)ctx; (void)stat_id;
    return creature_type;
}

static void test_table_size(void)
{
    assert(dm2_v1_creature_ai_table_count() == 62);
    assert(dm2_v1_creature_ai_table() != NULL);
    printf("  PASS: table_size\n");
}

static void test_tree_row0(void)
{
    const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec_by_row(0);
    assert(ai != NULL);
    assert(ai->flags == 0x0045);
    assert(ai->armor_class == 255);
    assert(ai->base_hp == 700);
    assert(ai->defense == 255);
    assert(dm2_v1_creature_ai_is_static(ai));
    printf("  PASS: tree_row0\n");
}

static void test_scout_minion_row13(void)
{
    const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec_by_row(13);
    assert(ai != NULL);
    assert(ai->flags == 0xCC40);
    assert(ai->armor_class == 130);
    assert(ai->base_hp == 200);
    assert(ai->attack_strength == 0);
    assert(ai->defense == 100);
    assert(!dm2_v1_creature_ai_is_static(ai));
    assert(ai->flags & DM2_AI_FLAG_CAN_TRAVEL_MAPS);
    printf("  PASS: scout_minion_row13\n");
}

static void test_dragoth_row30(void)
{
    const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec_by_row(30);
    assert(ai != NULL);
    assert(ai->base_hp == 1500);
    assert(ai->attack_strength == 135);
    assert(ai->defense == 170);
    assert(ai->flags & DM2_AI_FLAG_DRAGOTH_SPECIAL);
    printf("  PASS: dragoth_row30\n");
}

static void test_ghost_row61(void)
{
    const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec_by_row(61);
    assert(ai != NULL);
    assert(ai->flags == 0x8C78);
    assert(ai->base_hp == 150);
    assert(ai->attack_strength == 40);
    assert(ai->defense == 150);
    assert(ai->flags & DM2_AI_FLAG_GHOSTLIKE);
    printf("  PASS: ghost_row61\n");
}

static void test_out_of_range(void)
{
    assert(dm2_v1_creature_ai_spec_by_row(-1) == NULL);
    assert(dm2_v1_creature_ai_spec_by_row(62) == NULL);
    assert(dm2_v1_creature_ai_spec_by_row(999) == NULL);
    printf("  PASS: out_of_range\n");
}

static void test_two_stage_lookup(void)
{
    const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec_from_type(
        30, mock_gdat_direct, NULL);
    assert(ai != NULL);
    assert(ai->base_hp == 1500);

    ai = dm2_v1_creature_ai_spec_from_type(0, mock_gdat_direct, NULL);
    assert(ai != NULL);
    assert(ai->flags == 0x0045);
    printf("  PASS: two_stage_lookup\n");
}

static void test_null_callback(void)
{
    assert(dm2_v1_creature_ai_spec_from_type(0, NULL, NULL) == NULL);
    printf("  PASS: null_callback\n");
}

static void test_static_objects(void)
{
    int static_count = 0;
    for (int i = 0; i < 62; i++) {
        const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec_by_row(i);
        if (dm2_v1_creature_ai_is_static(ai))
            static_count++;
    }
    assert(static_count > 10);
    assert(static_count < 62);
    printf("  PASS: static_objects (found %d)\n", static_count);
}

int main(void) {
    test_table_size();
    test_tree_row0();
    test_scout_minion_row13();
    test_dragoth_row30();
    test_ghost_row61();
    test_out_of_range();
    test_two_stage_lookup();
    test_null_callback();
    test_static_objects();

    printf("PASS: dm2_v1_creature_ai_spec (9 tests)\n");
    return 0;
}
