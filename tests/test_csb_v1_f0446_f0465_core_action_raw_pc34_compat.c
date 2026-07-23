#include "csb_v1_f0446_f0465_core_action_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures, assertions;
static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) { ++failures; fprintf(stderr, "FAIL:%d: %s\n", line, expression); }
}
#define CHECK(c) check((c), #c, __LINE__)

static CSB_V1_CoreActionRawMaterialPc34 material(uint8_t *platform, uint8_t *memory, uint8_t *floppy,
    uint8_t *save, uint8_t *dungeon, uint8_t *graphics, uint8_t *timeline, uint8_t *input, uint8_t *audio)
{
    CSB_V1_CoreActionRawMaterialPc34 raw;
    memset(&raw, 0, sizeof(raw));
    raw.platform_material = platform; raw.platform_material_size = 8; raw.platform_material_identity = 0x44600001u;
    raw.memory_material = memory; raw.memory_material_size = 8; raw.memory_material_identity = 0x44600002u;
    raw.floppy_material = floppy; raw.floppy_material_size = 8; raw.floppy_material_identity = 0x44600003u;
    raw.save_material = save; raw.save_material_size = 8; raw.save_material_identity = 0x44600004u;
    raw.dungeon_material = dungeon; raw.dungeon_material_size = 8; raw.dungeon_material_identity = 0x44600005u;
    raw.graphics_material = graphics; raw.graphics_material_size = 8; raw.graphics_material_identity = 0x44600006u;
    raw.timeline_material = timeline; raw.timeline_material_size = 8; raw.timeline_material_identity = 0x44600007u;
    raw.input_material = input; raw.input_material_size = 8; raw.input_material_identity = 0x44600008u;
    raw.audio_material = audio; raw.audio_material_size = 8; raw.audio_material_identity = 0x44600009u;
    raw.authenticated_pc34 = 1;
    return raw;
}

static void test_fuse_receipt_is_not_execution(void)
{
    uint8_t p[8] = {1}, m[8] = {2}, f[8] = {3}, s[8] = {4}, d[8] = {5}, g[8] = {6}, t[8] = {7}, i[8] = {8}, a[8] = {9}, before[8];
    CSB_V1_CoreActionRawMaterialPc34 raw = material(p, m, f, s, d, g, t, i, a);
    CSB_V1_CoreActionAuditReceiptPc34 receipt;
    memcpy(before, d, sizeof(d));
    CHECK(csb_v1_f0446_f0465_core_action_audit_pc34(&raw, CSB_V1_CORE_ACTION_F0446, &receipt) == 1);
    CHECK(receipt.dungeon_material_required && receipt.graphics_material_required && receipt.audio_material_required);
    CHECK(receipt.runtime_execution_blocked && receipt.platform_behavior_fail_closed && memcmp(before, d, sizeof(d)) == 0);
    raw.floppy_material_identity = 0;
    CHECK(csb_v1_f0446_f0465_core_action_audit_pc34(&raw, CSB_V1_CORE_ACTION_S0465, &receipt) == 0);
}

static void test_every_source_symbol(void)
{
    uint8_t p[8] = {1}, m[8] = {2}, f[8] = {3}, s[8] = {4}, d[8] = {5}, g[8] = {6}, t[8] = {7}, i[8] = {8}, a[8] = {9};
    CSB_V1_CoreActionRawMaterialPc34 raw = material(p, m, f, s, d, g, t, i, a);
    CSB_V1_CoreActionAuditReceiptPc34 receipt;
    int id;
    for (id = CSB_V1_CORE_ACTION_F0446; id <= CSB_V1_CORE_ACTION_S0465; ++id) {
        CHECK(csb_v1_f0446_f0465_core_action_audit_pc34(&raw, (CSB_V1_CoreActionFunctionPc34)id, &receipt) == 1);
        CHECK(receipt.function_id == (CSB_V1_CoreActionFunctionPc34)id && receipt.source_evidence != NULL);
        CHECK(receipt.raw_material_admitted && receipt.runtime_execution_blocked);
    }
    CHECK(csb_v1_f0446_f0465_core_action_audit_pc34(&raw, (CSB_V1_CoreActionFunctionPc34)445, &receipt) == 0);
}

int main(void)
{
    test_fuse_receipt_is_not_execution();
    test_every_source_symbol();
    printf("csb_v1_f0446_f0465_core_action_raw: %d/%d assertions passed\n", assertions - failures, assertions);
    return failures ? 1 : 0;
}
