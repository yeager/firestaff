#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_ai_prepare_pc34_compat.h"

/* ------------------------------------------------------------------ */
/* DM2_4EA8 — count animation frames                                  */
/* ------------------------------------------------------------------ */

static void test_count_anim_frames_null(void)
{
    int r = dm2_v1_creature_count_animation_frames(NULL, 0, 0);
    assert(r == 1);
    printf("  PASS: count_anim_frames_null\n");
}

static void test_count_anim_frames_single(void)
{
    /* One entry with byte@1 bits 4-7 = 0 => count = 1 */
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
    int r = dm2_v1_creature_count_animation_frames(data, 4, 0);
    assert(r == 1);
    printf("  PASS: count_anim_frames_single\n");
}

static void test_count_anim_frames_three(void)
{
    /* Three entries: first two have bits 4-7 nonzero, third has zero */
    uint8_t data[12] = {
        0x00, 0x30, 0x00, 0x00,  /* byte@1 = 0x30, bits 4-7 = 3 */
        0x00, 0x10, 0x00, 0x00,  /* byte@1 = 0x10, bits 4-7 = 1 */
        0x00, 0x00, 0x00, 0x00,  /* byte@1 = 0x00, bits 4-7 = 0 => stop */
    };
    int r = dm2_v1_creature_count_animation_frames(data, 12, 0);
    assert(r == 3);
    printf("  PASS: count_anim_frames_three\n");
}

static void test_count_anim_frames_with_offset(void)
{
    /* Start at offset 1 (skip first 4 bytes) */
    uint8_t data[12] = {
        0x00, 0x20, 0x00, 0x00,  /* offset 0: skipped */
        0x00, 0x40, 0x00, 0x00,  /* offset 1: byte@1 bits 4-7 = 4 */
        0x00, 0x00, 0x00, 0x00,  /* offset 2: stop */
    };
    int r = dm2_v1_creature_count_animation_frames(data, 12, 1);
    assert(r == 2);
    printf("  PASS: count_anim_frames_with_offset\n");
}

static void test_count_anim_frames_bounds(void)
{
    /* Data too short for even one entry check */
    uint8_t data[1] = {0x00};
    int r = dm2_v1_creature_count_animation_frames(data, 1, 0);
    assert(r == 1);
    printf("  PASS: count_anim_frames_bounds\n");
}

/* ------------------------------------------------------------------ */
/* DM2_2c1d_09d9 — party power level                                  */
/* ------------------------------------------------------------------ */

static void test_power_level_null(void)
{
    int r = dm2_v1_compute_party_power_level(NULL);
    assert(r == 1);
    printf("  PASS: power_level_null\n");
}

static void test_power_level_no_heroes(void)
{
    DM2_V1_PartySkillData party;
    memset(&party, 0, sizeof(party));
    party.heroes_in_party = 0;
    int r = dm2_v1_compute_party_power_level(&party);
    assert(r == 1);
    printf("  PASS: power_level_no_heroes\n");
}

static void test_power_level_low_skills(void)
{
    /* Sum < 512 => level 1 */
    DM2_V1_PartySkillData party;
    memset(&party, 0, sizeof(party));
    party.heroes_in_party = 1;
    party.skill[0][0] = 100;
    party.skill[0][1] = 100;
    party.skill[0][2] = 100;
    party.skill[0][3] = 100;
    /* sum = 400 < 512 */
    int r = dm2_v1_compute_party_power_level(&party);
    assert(r == 1);
    printf("  PASS: power_level_low_skills\n");
}

static void test_power_level_exact_512(void)
{
    /* Sum = 512 => shift once => level 2 */
    DM2_V1_PartySkillData party;
    memset(&party, 0, sizeof(party));
    party.heroes_in_party = 1;
    party.skill[0][0] = 128;
    party.skill[0][1] = 128;
    party.skill[0][2] = 128;
    party.skill[0][3] = 128;
    /* sum = 512 => 512 >= 512, shift to 256, level = 2 */
    int r = dm2_v1_compute_party_power_level(&party);
    assert(r == 2);
    printf("  PASS: power_level_exact_512\n");
}

static void test_power_level_high(void)
{
    /* Sum = 4096 => 4096/512=8, log2(8)=3, level = 3+1 = 4 */
    DM2_V1_PartySkillData party;
    memset(&party, 0, sizeof(party));
    party.heroes_in_party = 4;
    party.skill[0][0] = 256;
    party.skill[0][1] = 256;
    party.skill[0][2] = 256;
    party.skill[0][3] = 256;
    /* sum = 1024 per hero... wait, 4 heroes * 4 skills * 256 = 4096 */
    party.skill[1][0] = 256;
    party.skill[1][1] = 256;
    party.skill[1][2] = 256;
    party.skill[1][3] = 256;
    party.skill[2][0] = 256;
    party.skill[2][1] = 256;
    party.skill[2][2] = 256;
    party.skill[2][3] = 256;
    party.skill[3][0] = 256;
    party.skill[3][1] = 256;
    party.skill[3][2] = 256;
    party.skill[3][3] = 256;
    /* sum = 4096 => shifts: 4096->2048(2)->1024(3)->512(4)->256(5) => level 5 */
    int r = dm2_v1_compute_party_power_level(&party);
    assert(r == 5);
    printf("  PASS: power_level_high\n");
}

static void test_power_level_1024(void)
{
    /* Sum = 1024 => 1024 >= 512, shift to 512 (level 2), still >= 512,
     * shift to 256 (level 3) => return 3 */
    DM2_V1_PartySkillData party;
    memset(&party, 0, sizeof(party));
    party.heroes_in_party = 1;
    party.skill[0][0] = 256;
    party.skill[0][1] = 256;
    party.skill[0][2] = 256;
    party.skill[0][3] = 256;
    /* sum = 1024 */
    int r = dm2_v1_compute_party_power_level(&party);
    assert(r == 3);
    printf("  PASS: power_level_1024\n");
}

/* ------------------------------------------------------------------ */
/* PREPARE/UNPREPARE context                                           */
/* ------------------------------------------------------------------ */

static void test_prepare_null_safety(void)
{
    DM2_V1_PrepareCreatureContextReceipt receipt;
    void *r = dm2_v1_prepare_creature_ai_context(NULL, &receipt);
    assert(r == NULL);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_prepare_creature_ai_context(NULL, NULL);
    assert(r == NULL);
    printf("  PASS: prepare_null_safety\n");
}

static void test_prepare_fail_closed(void)
{
    DM2_V1_PrepareCreatureContextRequest req;
    DM2_V1_PrepareCreatureContextReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.creature_handle = 0x1000;
    req.tile_x = 5;
    req.tile_y = 10;
    req.map_level = 0;
    req.timer_type = 0x22;
    req.game_tick = 100;
    void *r = dm2_v1_prepare_creature_ai_context(&req, &receipt);
    assert(r == NULL);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    assert(receipt.source_evidence[0] != '\0');
    printf("  PASS: prepare_fail_closed\n");
}

static void test_unprepare_null_buffer(void)
{
    DM2_V1_UnprepareCreatureContextReceipt receipt;
    int r = dm2_v1_unprepare_creature_ai_context(NULL, &receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    assert(receipt.cleared == 1);
    printf("  PASS: unprepare_null_buffer\n");
}

static void test_unprepare_with_buffer(void)
{
    DM2_V1_UnprepareCreatureContextReceipt receipt;
    int dummy = 42;
    int r = dm2_v1_unprepare_creature_ai_context(&dummy, &receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.restored == 1);
    printf("  PASS: unprepare_with_buffer\n");
}

static void test_unprepare_null_receipt(void)
{
    int r = dm2_v1_unprepare_creature_ai_context(NULL, NULL);
    assert(r == 0);
    printf("  PASS: unprepare_null_receipt\n");
}

/* ------------------------------------------------------------------ */
/* DM2_13e4_01a3 — init AI state                                       */
/* ------------------------------------------------------------------ */

static void test_init_ai_state_fail_closed(void)
{
    DM2_V1_InitCreatureAiStateReceipt receipt;
    int r = dm2_v1_init_creature_ai_state(&receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    assert(receipt.source_evidence[0] != '\0');
    printf("  PASS: init_ai_state_fail_closed\n");
}

static void test_init_ai_state_null_receipt(void)
{
    int r = dm2_v1_init_creature_ai_state(NULL);
    assert(r == 0);
    printf("  PASS: init_ai_state_null_receipt\n");
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_062e — get creature action flags                           */
/* ------------------------------------------------------------------ */

static void test_action_flags_null_safety(void)
{
    DM2_V1_GetCreatureActionFlagsReceipt receipt;
    int r = dm2_v1_get_creature_action_flags(NULL, 0, NULL, 0, 0, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_get_creature_action_flags(NULL, 0, NULL, 0, 0, NULL);
    assert(r == 0);
    printf("  PASS: action_flags_null_safety\n");
}

static void test_action_flags_no_table(void)
{
    /* CAII slot byte@0x12 = 0xff => no action table */
    uint8_t slot[0x14];
    memset(slot, 0, sizeof(slot));
    slot[0x12] = 0xff;
    slot[0x13] = 0x00;

    DM2_V1_ActionTableSet tables;
    memset(&tables, 0, sizeof(tables));
    tables.table_count = 0;

    DM2_V1_GetCreatureActionFlagsReceipt receipt;
    int r = dm2_v1_get_creature_action_flags(slot, 0x14, &tables, 0, 0, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.no_action_table == 1);
    assert(receipt.action_flags == 0);
    printf("  PASS: action_flags_no_table\n");
}

static void test_action_flags_resolved(void)
{
    uint8_t slot[0x14];
    memset(slot, 0, sizeof(slot));
    slot[0x12] = 0;   /* table index */
    slot[0x13] = 1;   /* entry index */

    DM2_V1_ActionTableEntry entries[3];
    memset(entries, 0, sizeof(entries));
    entries[1].bytes[5] = 0xa0;  /* flags: bits 5,7 set => 0xa0 & 0xe0 = 0xa0 */

    DM2_V1_ActionTableSet tables;
    memset(&tables, 0, sizeof(tables));
    tables.tables[0] = entries;
    tables.table_count = 1;

    DM2_V1_GetCreatureActionFlagsReceipt receipt;
    int r = dm2_v1_get_creature_action_flags(slot, 0x14, &tables, 3, 3, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.action_flags == 0xa0);
    printf("  PASS: action_flags_resolved\n");
}

static void test_action_flags_party_map_mismatch(void)
{
    uint8_t slot[0x14];
    memset(slot, 0, sizeof(slot));
    slot[0x12] = 0;
    slot[0x13] = 0;

    DM2_V1_ActionTableEntry entries[1];
    memset(entries, 0, sizeof(entries));
    entries[0].bytes[5] = 0x40;  /* bits 5-6 = 0x40 => party map check */

    DM2_V1_ActionTableSet tables;
    memset(&tables, 0, sizeof(tables));
    tables.tables[0] = entries;
    tables.table_count = 1;

    DM2_V1_GetCreatureActionFlagsReceipt receipt;
    int r = dm2_v1_get_creature_action_flags(slot, 0x14, &tables, 3, 5, &receipt);
    assert(r == 1);
    assert(receipt.party_map_mismatch == 1);
    assert(receipt.action_flags == 0);
    printf("  PASS: action_flags_party_map_mismatch\n");
}

/* ------------------------------------------------------------------ */
/* Animation timing                                                    */
/* ------------------------------------------------------------------ */

static void test_anim_timing_4000_fail_closed(void)
{
    DM2_V1_CreatureAnimTimingReceipt receipt;
    int r = dm2_v1_creature_animation_timing_4000(&receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    printf("  PASS: anim_timing_4000_fail_closed\n");
}

static void test_anim_timing_4000_null_receipt(void)
{
    int r = dm2_v1_creature_animation_timing_4000(NULL);
    assert(r == 0);
    printf("  PASS: anim_timing_4000_null_receipt\n");
}

static void test_anim_timing_2000_fail_closed(void)
{
    DM2_V1_CreatureAnimTimingReceipt receipt;
    int r = dm2_v1_creature_animation_timing_2000(&receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    printf("  PASS: anim_timing_2000_fail_closed\n");
}

static void test_anim_timing_2000_null_receipt(void)
{
    int r = dm2_v1_creature_animation_timing_2000(NULL);
    assert(r == 0);
    printf("  PASS: anim_timing_2000_null_receipt\n");
}

/* ------------------------------------------------------------------ */

int main(void)
{
    printf("test_dm2_v1_creature_ai_prepare_pc34_compat:\n");

    /* Animation frame counting */
    test_count_anim_frames_null();
    test_count_anim_frames_single();
    test_count_anim_frames_three();
    test_count_anim_frames_with_offset();
    test_count_anim_frames_bounds();

    /* Party power level */
    test_power_level_null();
    test_power_level_no_heroes();
    test_power_level_low_skills();
    test_power_level_exact_512();
    test_power_level_high();
    test_power_level_1024();

    /* Prepare/unprepare context */
    test_prepare_null_safety();
    test_prepare_fail_closed();
    test_unprepare_null_buffer();
    test_unprepare_with_buffer();
    test_unprepare_null_receipt();

    /* Init AI state */
    test_init_ai_state_fail_closed();
    test_init_ai_state_null_receipt();

    /* Action flags */
    test_action_flags_null_safety();
    test_action_flags_no_table();
    test_action_flags_resolved();
    test_action_flags_party_map_mismatch();

    /* Animation timing */
    test_anim_timing_4000_fail_closed();
    test_anim_timing_4000_null_receipt();
    test_anim_timing_2000_fail_closed();
    test_anim_timing_2000_null_receipt();

    printf("All tests passed.\n");
    return 0;
}
