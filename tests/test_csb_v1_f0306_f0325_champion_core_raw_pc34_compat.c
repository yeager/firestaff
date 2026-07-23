#include "csb_v1_f0306_f0325_champion_core_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int assertions;

static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) { ++failures; fprintf(stderr, "FAIL:%d: %s\n", line, expression); }
}
#define CHECK(condition) check((condition), #condition, __LINE__)

static CSB_V1_ChampionCoreRawMaterialPc34 raw_material(uint8_t *c, uint8_t *g,
                                                         uint8_t *d, uint8_t *t)
{
    CSB_V1_ChampionCoreRawMaterialPc34 raw;
    memset(&raw, 0, sizeof(raw));
    raw.champion_record = c; raw.champion_record_size = 16; raw.champion_record_identity = 0x30600001u;
    raw.graphics_material = g; raw.graphics_material_size = 8; raw.graphics_material_identity = 0x30600002u;
    raw.dungeon_material = d; raw.dungeon_material_size = 8; raw.dungeon_material_identity = 0x30600003u;
    raw.timeline_material = t; raw.timeline_material_size = 8; raw.timeline_material_identity = 0x30600004u;
    raw.authenticated_pc34 = 1;
    return raw;
}

static void test_material_requirements_and_nonmutation(void)
{
    uint8_t champion[16] = {0}, graphics[8] = {0}, dungeon[8] = {0}, timeline[8] = {0};
    uint8_t before[sizeof(champion)];
    CSB_V1_ChampionCoreRawMaterialPc34 raw = raw_material(champion, graphics, dungeon, timeline);
    CSB_V1_ChampionCoreAuditReceiptPc34 receipt;
    memcpy(before, champion, sizeof(champion));
    CHECK(csb_v1_f0306_f0325_champion_core_audit_pc34(&raw, CSB_V1_CHAMPION_CORE_F0306, &receipt) == 1);
    CHECK(receipt.read_only_query && receipt.runtime_execution_blocked);
    CHECK(memcmp(champion, before, sizeof(champion)) == 0);
    raw.dungeon_material = NULL;
    CHECK(csb_v1_f0306_f0325_champion_core_audit_pc34(&raw, CSB_V1_CHAMPION_CORE_F0312, &receipt) == 0);
    raw = raw_material(champion, graphics, dungeon, timeline);
    raw.timeline_material_identity = 0;
    CHECK(csb_v1_f0306_f0325_champion_core_audit_pc34(&raw, CSB_V1_CHAMPION_CORE_F0325, &receipt) == 0);
}

static void test_all_symbols_are_disjointly_audited(void)
{
    uint8_t champion[16] = {0}, graphics[8] = {0}, dungeon[8] = {0}, timeline[8] = {0};
    CSB_V1_ChampionCoreRawMaterialPc34 raw = raw_material(champion, graphics, dungeon, timeline);
    CSB_V1_ChampionCoreAuditReceiptPc34 receipt;
    int id;
    for (id = CSB_V1_CHAMPION_CORE_F0306; id <= CSB_V1_CHAMPION_CORE_F0325; ++id) {
        CHECK(csb_v1_f0306_f0325_champion_core_audit_pc34(
            &raw, (CSB_V1_ChampionCoreFunctionPc34)id, &receipt) == 1);
        CHECK(receipt.function_id == (CSB_V1_ChampionCoreFunctionPc34)id &&
              receipt.source_evidence != NULL);
        CHECK(receipt.platform_behavior_fail_closed);
    }
    CHECK(csb_v1_f0306_f0325_champion_core_audit_pc34(
        &raw, (CSB_V1_ChampionCoreFunctionPc34)305, &receipt) == 0);
}

int main(void)
{
    test_material_requirements_and_nonmutation();
    test_all_symbols_are_disjointly_audited();
    printf("csb_v1_f0306_f0325_champion_core_raw: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures == 0 ? 0 : 1;
}
