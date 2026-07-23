#ifndef FIRESTAFF_CSB_V1_F0426_F0445_STARTEND_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0426_F0445_STARTEND_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_StartendFunctionPc34 {
    CSB_V1_STARTEND_F0426 = 426, CSB_V1_STARTEND_F0427 = 427,
    CSB_V1_STARTEND_F0428 = 428, CSB_V1_STARTEND_F0429 = 429,
    CSB_V1_STARTEND_F0430 = 430, CSB_V1_STARTEND_F0431 = 431,
    CSB_V1_STARTEND_F0432 = 432, CSB_V1_STARTEND_F0433 = 433,
    CSB_V1_STARTEND_F0434 = 434, CSB_V1_STARTEND_F0435 = 435,
    CSB_V1_STARTEND_F0436 = 436, CSB_V1_STARTEND_F0437 = 437,
    CSB_V1_STARTEND_F0438 = 438, CSB_V1_STARTEND_F0439 = 439,
    CSB_V1_STARTEND_F0440 = 440, CSB_V1_STARTEND_F0441 = 441,
    CSB_V1_STARTEND_F0442 = 442, CSB_V1_STARTEND_F0443 = 443,
    CSB_V1_STARTEND_F0444 = 444, CSB_V1_STARTEND_F0445 = 445
} CSB_V1_StartendFunctionPc34;

typedef struct CSB_V1_StartendRawMaterialPc34 {
    const uint8_t *dialog_material; size_t dialog_material_size; uint32_t dialog_material_identity;
    const uint8_t *save_material; size_t save_material_size; uint32_t save_material_identity;
    const uint8_t *palette_material; size_t palette_material_size; uint32_t palette_material_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *dungeon_material; size_t dungeon_material_size; uint32_t dungeon_material_identity;
    const uint8_t *timeline_material; size_t timeline_material_size; uint32_t timeline_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *audio_material; size_t audio_material_size; uint32_t audio_material_identity;
    int authenticated_pc34;
} CSB_V1_StartendRawMaterialPc34;

typedef struct CSB_V1_StartendAuditReceiptPc34 {
    int raw_material_admitted;
    int existing_runtime_owner_preserved;
    int dialog_material_required;
    int save_material_required;
    int palette_material_required;
    int graphics_material_required;
    int dungeon_material_required;
    int timeline_material_required;
    int input_material_required;
    int audio_material_required;
    int read_only_query;
    int runtime_execution_blocked;
    int platform_behavior_fail_closed;
    CSB_V1_StartendFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_StartendAuditReceiptPc34;

int csb_v1_f0426_f0445_startend_audit_pc34(
    const CSB_V1_StartendRawMaterialPc34 *raw,
    CSB_V1_StartendFunctionPc34 function_id,
    CSB_V1_StartendAuditReceiptPc34 *out);

#endif
