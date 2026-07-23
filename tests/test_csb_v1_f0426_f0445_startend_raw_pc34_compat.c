#include "csb_v1_f0426_f0445_startend_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures, assertions;
static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) { ++failures; fprintf(stderr, "FAIL:%d: %s\n", line, expression); }
}
#define CHECK(c) check((c), #c, __LINE__)

static CSB_V1_StartendRawMaterialPc34 material(
    uint8_t *dialog, uint8_t *save, uint8_t *palette, uint8_t *graphics,
    uint8_t *dungeon, uint8_t *timeline, uint8_t *input, uint8_t *audio)
{
    CSB_V1_StartendRawMaterialPc34 raw;
    memset(&raw, 0, sizeof(raw));
    raw.dialog_material = dialog; raw.dialog_material_size = 8; raw.dialog_material_identity = 0x42600001u;
    raw.save_material = save; raw.save_material_size = 8; raw.save_material_identity = 0x42600002u;
    raw.palette_material = palette; raw.palette_material_size = 8; raw.palette_material_identity = 0x42600003u;
    raw.graphics_material = graphics; raw.graphics_material_size = 8; raw.graphics_material_identity = 0x42600004u;
    raw.dungeon_material = dungeon; raw.dungeon_material_size = 8; raw.dungeon_material_identity = 0x42600005u;
    raw.timeline_material = timeline; raw.timeline_material_size = 8; raw.timeline_material_identity = 0x42600006u;
    raw.input_material = input; raw.input_material_size = 8; raw.input_material_identity = 0x42600007u;
    raw.audio_material = audio; raw.audio_material_size = 8; raw.audio_material_identity = 0x42600008u;
    raw.authenticated_pc34 = 1;
    return raw;
}

static void test_endgame_receipt_is_not_execution(void)
{
    uint8_t dialog[8] = {1}, save[8] = {2}, palette[8] = {3}, graphics[8] = {4};
    uint8_t dungeon[8] = {5}, timeline[8] = {6}, input[8] = {7}, audio[8] = {8}, before[8];
    CSB_V1_StartendRawMaterialPc34 raw = material(dialog, save, palette, graphics, dungeon, timeline, input, audio);
    CSB_V1_StartendAuditReceiptPc34 receipt;

    memcpy(before, dungeon, sizeof(dungeon));
    CHECK(csb_v1_f0426_f0445_startend_audit_pc34(&raw, CSB_V1_STARTEND_F0444, &receipt) == 1);
    CHECK(receipt.dungeon_material_required && receipt.timeline_material_required && receipt.audio_material_required);
    CHECK(receipt.runtime_execution_blocked && receipt.platform_behavior_fail_closed);
    CHECK(memcmp(before, dungeon, sizeof(dungeon)) == 0);
    raw.audio_material_identity = 0;
    CHECK(csb_v1_f0426_f0445_startend_audit_pc34(&raw, CSB_V1_STARTEND_F0445, &receipt) == 0);
}

static void test_every_source_symbol(void)
{
    uint8_t dialog[8] = {1}, save[8] = {2}, palette[8] = {3}, graphics[8] = {4};
    uint8_t dungeon[8] = {5}, timeline[8] = {6}, input[8] = {7}, audio[8] = {8};
    CSB_V1_StartendRawMaterialPc34 raw = material(dialog, save, palette, graphics, dungeon, timeline, input, audio);
    CSB_V1_StartendAuditReceiptPc34 receipt;
    int id;

    for (id = CSB_V1_STARTEND_F0426; id <= CSB_V1_STARTEND_F0445; ++id) {
        CHECK(csb_v1_f0426_f0445_startend_audit_pc34(&raw, (CSB_V1_StartendFunctionPc34)id, &receipt) == 1);
        CHECK(receipt.function_id == (CSB_V1_StartendFunctionPc34)id && receipt.source_evidence != NULL);
        CHECK(receipt.raw_material_admitted && receipt.runtime_execution_blocked);
    }
    CHECK(csb_v1_f0426_f0445_startend_audit_pc34(&raw, (CSB_V1_StartendFunctionPc34)425, &receipt) == 0);
}

int main(void)
{
    test_endgame_receipt_is_not_execution();
    test_every_source_symbol();
    printf("csb_v1_f0426_f0445_startend_raw: %d/%d assertions passed\n", assertions - failures, assertions);
    return failures ? 1 : 0;
}
