#ifndef FIRESTAFF_CSB_V1_F0386_F0405_ACTION_VIEWPORT_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0386_F0405_ACTION_VIEWPORT_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_ActionViewportFunctionPc34 {
    CSB_V1_ACTION_VIEWPORT_F0386 = 386, CSB_V1_ACTION_VIEWPORT_F0387 = 387,
    CSB_V1_ACTION_VIEWPORT_F0388 = 388, CSB_V1_ACTION_VIEWPORT_F0389 = 389,
    CSB_V1_ACTION_VIEWPORT_F0390 = 390, CSB_V1_ACTION_VIEWPORT_F0391 = 391,
    CSB_V1_ACTION_VIEWPORT_F0392 = 392, CSB_V1_ACTION_VIEWPORT_F0393 = 393,
    CSB_V1_ACTION_VIEWPORT_F0394 = 394, CSB_V1_ACTION_VIEWPORT_F0395 = 395,
    CSB_V1_ACTION_VIEWPORT_F0396 = 396, CSB_V1_ACTION_VIEWPORT_F0397 = 397,
    CSB_V1_ACTION_VIEWPORT_F0398 = 398, CSB_V1_ACTION_VIEWPORT_F0399 = 399,
    CSB_V1_ACTION_VIEWPORT_F0400 = 400, CSB_V1_ACTION_VIEWPORT_F0401 = 401,
    CSB_V1_ACTION_VIEWPORT_F0402 = 402, CSB_V1_ACTION_VIEWPORT_F0403 = 403,
    CSB_V1_ACTION_VIEWPORT_F0404 = 404, CSB_V1_ACTION_VIEWPORT_F0405 = 405
} CSB_V1_ActionViewportFunctionPc34;

typedef struct CSB_V1_ActionViewportRawMaterialPc34 {
    const uint8_t *champion_record; size_t champion_record_size; uint32_t champion_record_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *dungeon_material; size_t dungeon_material_size; uint32_t dungeon_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *timeline_material; size_t timeline_material_size; uint32_t timeline_material_identity;
    int authenticated_pc34;
} CSB_V1_ActionViewportRawMaterialPc34;

typedef struct CSB_V1_ActionViewportAuditReceiptPc34 {
    int raw_material_admitted;
    int champion_material_required;
    int graphics_material_required;
    int dungeon_material_required;
    int input_material_required;
    int timeline_material_required;
    int read_only_query;
    int runtime_execution_blocked;
    int platform_behavior_fail_closed;
    CSB_V1_ActionViewportFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_ActionViewportAuditReceiptPc34;

int csb_v1_f0386_f0405_action_viewport_audit_pc34(
    const CSB_V1_ActionViewportRawMaterialPc34 *raw,
    CSB_V1_ActionViewportFunctionPc34 function_id,
    CSB_V1_ActionViewportAuditReceiptPc34 *out);

#endif
