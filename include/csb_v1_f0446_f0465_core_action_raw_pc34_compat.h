#ifndef FIRESTAFF_CSB_V1_F0446_F0465_CORE_ACTION_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0446_F0465_CORE_ACTION_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_CoreActionFunctionPc34 {
    CSB_V1_CORE_ACTION_F0446 = 446, CSB_V1_CORE_ACTION_F0447 = 447,
    CSB_V1_CORE_ACTION_F0448 = 448, CSB_V1_CORE_ACTION_F0449 = 449,
    CSB_V1_CORE_ACTION_F0450 = 450, CSB_V1_CORE_ACTION_F0451 = 451,
    CSB_V1_CORE_ACTION_F0452 = 452, CSB_V1_CORE_ACTION_F0453 = 453,
    CSB_V1_CORE_ACTION_F0454 = 454, CSB_V1_CORE_ACTION_F0455 = 455,
    CSB_V1_CORE_ACTION_F0456 = 456, CSB_V1_CORE_ACTION_F0457 = 457,
    CSB_V1_CORE_ACTION_F0458 = 458, CSB_V1_CORE_ACTION_F0459 = 459,
    CSB_V1_CORE_ACTION_F0460 = 460, CSB_V1_CORE_ACTION_F0461 = 461,
    CSB_V1_CORE_ACTION_F0462 = 462, CSB_V1_CORE_ACTION_F0463 = 463,
    CSB_V1_CORE_ACTION_F0464 = 464, CSB_V1_CORE_ACTION_S0465 = 465
} CSB_V1_CoreActionFunctionPc34;

typedef struct CSB_V1_CoreActionRawMaterialPc34 {
    const uint8_t *platform_material; size_t platform_material_size; uint32_t platform_material_identity;
    const uint8_t *memory_material; size_t memory_material_size; uint32_t memory_material_identity;
    const uint8_t *floppy_material; size_t floppy_material_size; uint32_t floppy_material_identity;
    const uint8_t *save_material; size_t save_material_size; uint32_t save_material_identity;
    const uint8_t *dungeon_material; size_t dungeon_material_size; uint32_t dungeon_material_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *timeline_material; size_t timeline_material_size; uint32_t timeline_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *audio_material; size_t audio_material_size; uint32_t audio_material_identity;
    int authenticated_pc34;
} CSB_V1_CoreActionRawMaterialPc34;

typedef struct CSB_V1_CoreActionAuditReceiptPc34 {
    int raw_material_admitted;
    int existing_runtime_owner_preserved;
    int platform_material_required, memory_material_required, floppy_material_required;
    int save_material_required, dungeon_material_required, graphics_material_required;
    int timeline_material_required, input_material_required, audio_material_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    CSB_V1_CoreActionFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_CoreActionAuditReceiptPc34;

int csb_v1_f0446_f0465_core_action_audit_pc34(
    const CSB_V1_CoreActionRawMaterialPc34 *raw,
    CSB_V1_CoreActionFunctionPc34 function_id,
    CSB_V1_CoreActionAuditReceiptPc34 *out);

#endif
