#include "csb_v1_f0386_f0405_action_viewport_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures, assertions;
static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) { ++failures; fprintf(stderr, "FAIL:%d: %s\n", line, expression); }
}
#define CHECK(c) check((c), #c, __LINE__)

static CSB_V1_ActionViewportRawMaterialPc34 material(
    uint8_t *champion, uint8_t *graphics, uint8_t *dungeon, uint8_t *input, uint8_t *timeline)
{
    CSB_V1_ActionViewportRawMaterialPc34 raw;
    memset(&raw, 0, sizeof(raw));
    raw.champion_record = champion; raw.champion_record_size = 8; raw.champion_record_identity = 0x38600001u;
    raw.graphics_material = graphics; raw.graphics_material_size = 8; raw.graphics_material_identity = 0x38600002u;
    raw.dungeon_material = dungeon; raw.dungeon_material_size = 8; raw.dungeon_material_identity = 0x38600003u;
    raw.input_material = input; raw.input_material_size = 8; raw.input_material_identity = 0x38600004u;
    raw.timeline_material = timeline; raw.timeline_material_size = 8; raw.timeline_material_identity = 0x38600005u;
    raw.authenticated_pc34 = 1;
    return raw;
}

static void test_receipt_is_not_execution(void)
{
    uint8_t champion[8] = {1}, graphics[8] = {2}, dungeon[8] = {3}, input[8] = {4}, timeline[8] = {5}, before[8];
    CSB_V1_ActionViewportRawMaterialPc34 raw = material(champion, graphics, dungeon, input, timeline);
    CSB_V1_ActionViewportAuditReceiptPc34 receipt;

    memcpy(before, champion, sizeof(champion));
    CHECK(csb_v1_f0386_f0405_action_viewport_audit_pc34(&raw, CSB_V1_ACTION_VIEWPORT_F0402, &receipt) == 1);
    CHECK(receipt.champion_material_required && receipt.dungeon_material_required && receipt.timeline_material_required);
    CHECK(receipt.runtime_execution_blocked && receipt.platform_behavior_fail_closed);
    CHECK(memcmp(before, champion, sizeof(champion)) == 0);

    raw.timeline_material_identity = 0;
    CHECK(csb_v1_f0386_f0405_action_viewport_audit_pc34(&raw, CSB_V1_ACTION_VIEWPORT_F0404, &receipt) == 0);
}

static void test_every_source_symbol(void)
{
    uint8_t champion[8] = {1}, graphics[8] = {2}, dungeon[8] = {3}, input[8] = {4}, timeline[8] = {5};
    CSB_V1_ActionViewportRawMaterialPc34 raw = material(champion, graphics, dungeon, input, timeline);
    CSB_V1_ActionViewportAuditReceiptPc34 receipt;
    int id;

    for (id = CSB_V1_ACTION_VIEWPORT_F0386; id <= CSB_V1_ACTION_VIEWPORT_F0405; ++id) {
        CHECK(csb_v1_f0386_f0405_action_viewport_audit_pc34(&raw, (CSB_V1_ActionViewportFunctionPc34)id, &receipt) == 1);
        CHECK(receipt.function_id == (CSB_V1_ActionViewportFunctionPc34)id && receipt.source_evidence != NULL);
        CHECK(receipt.raw_material_admitted && receipt.runtime_execution_blocked);
    }
    CHECK(csb_v1_f0386_f0405_action_viewport_audit_pc34(&raw, (CSB_V1_ActionViewportFunctionPc34)385, &receipt) == 0);
}

int main(void)
{
    test_receipt_is_not_execution();
    test_every_source_symbol();
    printf("csb_v1_f0386_f0405_action_viewport_raw: %d/%d assertions passed\n", assertions - failures, assertions);
    return failures ? 1 : 0;
}
