#include "csb_v1_f0290_f0305_post_champion_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int assertions;

static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL:%d: %s\n", line, expression);
    }
}

#define CHECK(condition) check((condition), #condition, __LINE__)

static CSB_V1_PostChampionRawMaterialPc34 raw_material(
    uint8_t *champion, uint8_t *graphics, uint8_t *dungeon)
{
    CSB_V1_PostChampionRawMaterialPc34 raw;
    memset(&raw, 0, sizeof(raw));
    raw.champion_record = champion;
    raw.champion_record_size = 16;
    raw.champion_record_identity = 0x29000001u;
    raw.graphics_material = graphics;
    raw.graphics_material_size = 8;
    raw.graphics_material_identity = 0x29000002u;
    raw.dungeon_material = dungeon;
    raw.dungeon_material_size = 8;
    raw.dungeon_material_identity = 0x29000003u;
    raw.authenticated_pc34 = 1;
    return raw;
}

static void test_source_groups_require_their_actual_material(void)
{
    uint8_t champion[16] = {0};
    uint8_t graphics[8] = {0};
    uint8_t dungeon[8] = {0};
    uint8_t champion_before[sizeof(champion)];
    CSB_V1_PostChampionRawMaterialPc34 raw = raw_material(champion, graphics, dungeon);
    CSB_V1_PostChampionAuditReceiptPc34 receipt;

    memcpy(champion_before, champion, sizeof(champion));
    CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
        &raw, CSB_V1_POST_CHAMPION_F0290, &receipt) == 1);
    CHECK(receipt.graphics_material_required && !receipt.dungeon_material_required);
    CHECK(receipt.runtime_execution_blocked && receipt.platform_behavior_fail_closed);
    CHECK(memcmp(champion, champion_before, sizeof(champion)) == 0);

    raw.graphics_material = NULL;
    CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
        &raw, CSB_V1_POST_CHAMPION_F0290, &receipt) == 0);
    raw = raw_material(champion, graphics, dungeon);
    raw.dungeon_material_identity = 0;
    CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
        &raw, CSB_V1_POST_CHAMPION_F0297, &receipt) == 0);
}

static void test_read_only_queries_are_not_executed(void)
{
    uint8_t champion[16] = {0};
    uint8_t graphics[8] = {0};
    uint8_t dungeon[8] = {0};
    CSB_V1_PostChampionRawMaterialPc34 raw = raw_material(champion, graphics, dungeon);
    CSB_V1_PostChampionAuditReceiptPc34 receipt;

    CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
        &raw, CSB_V1_POST_CHAMPION_F0294, &receipt) == 1);
    CHECK(receipt.read_only_query && receipt.dungeon_material_required);
    CHECK(receipt.runtime_execution_blocked);
    CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
        &raw, CSB_V1_POST_CHAMPION_F0303, &receipt) == 1);
    CHECK(receipt.read_only_query && !receipt.graphics_material_required &&
          !receipt.dungeon_material_required);
    CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
        &raw, (CSB_V1_PostChampionFunctionPc34)295, &receipt) == 0);
}

static void test_all_disjoint_symbols_are_audited(void)
{
    static const CSB_V1_PostChampionFunctionPc34 functions[] = {
        CSB_V1_POST_CHAMPION_F0290, CSB_V1_POST_CHAMPION_F0291,
        CSB_V1_POST_CHAMPION_F0292, CSB_V1_POST_CHAMPION_F0293,
        CSB_V1_POST_CHAMPION_F0294, CSB_V1_POST_CHAMPION_F0296,
        CSB_V1_POST_CHAMPION_F0297, CSB_V1_POST_CHAMPION_F0298,
        CSB_V1_POST_CHAMPION_F0299, CSB_V1_POST_CHAMPION_F0300,
        CSB_V1_POST_CHAMPION_F0301, CSB_V1_POST_CHAMPION_F0302,
        CSB_V1_POST_CHAMPION_F0303, CSB_V1_POST_CHAMPION_F0304,
        CSB_V1_POST_CHAMPION_F0305
    };
    uint8_t champion[16] = {0};
    uint8_t graphics[8] = {0};
    uint8_t dungeon[8] = {0};
    CSB_V1_PostChampionRawMaterialPc34 raw = raw_material(champion, graphics, dungeon);
    CSB_V1_PostChampionAuditReceiptPc34 receipt;
    size_t i;

    for (i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
        CHECK(csb_v1_f0290_f0305_post_champion_audit_pc34(
            &raw, functions[i], &receipt) == 1);
        CHECK(receipt.function_id == functions[i] && receipt.source_evidence != NULL);
    }
}

int main(void)
{
    test_source_groups_require_their_actual_material();
    test_read_only_queries_are_not_executed();
    test_all_disjoint_symbols_are_audited();
    printf("csb_v1_f0290_f0305_post_champion_raw: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures == 0 ? 0 : 1;
}
