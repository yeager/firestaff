#ifndef FIRESTAFF_CSB_V1_F0526_F0545_PLATFORM_INPUT_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0526_F0545_PLATFORM_INPUT_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_PlatformInputFunctionPc34 {
    CSB_V1_PLATFORM_INPUT_F0526 = 526, CSB_V1_PLATFORM_INPUT_F0527 = 527,
    CSB_V1_PLATFORM_INPUT_F0528 = 528, CSB_V1_PLATFORM_INPUT_F0529 = 529,
    CSB_V1_PLATFORM_INPUT_F0530 = 530, CSB_V1_PLATFORM_INPUT_F0531 = 531,
    CSB_V1_PLATFORM_INPUT_F0532 = 532, CSB_V1_PLATFORM_INPUT_F0533 = 533,
    CSB_V1_PLATFORM_INPUT_F0534 = 534, CSB_V1_PLATFORM_INPUT_F0535 = 535,
    CSB_V1_PLATFORM_INPUT_F0536 = 536, CSB_V1_PLATFORM_INPUT_F0537 = 537,
    CSB_V1_PLATFORM_INPUT_F0538 = 538, CSB_V1_PLATFORM_INPUT_F0539 = 539,
    CSB_V1_PLATFORM_INPUT_F0540 = 540, CSB_V1_PLATFORM_INPUT_F0541 = 541,
    CSB_V1_PLATFORM_INPUT_F0542 = 542, CSB_V1_PLATFORM_INPUT_F0543 = 543,
    CSB_V1_PLATFORM_INPUT_F0544 = 544, CSB_V1_PLATFORM_INPUT_F0545 = 545
} CSB_V1_PlatformInputFunctionPc34;

typedef struct CSB_V1_PlatformInputRawMaterialPc34 {
    const uint8_t *platform_material; size_t platform_material_size; uint32_t platform_material_identity;
    const uint8_t *floppy_material; size_t floppy_material_size; uint32_t floppy_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *memory_material; size_t memory_material_size; uint32_t memory_material_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    int authenticated_pc34;
} CSB_V1_PlatformInputRawMaterialPc34;

typedef struct CSB_V1_PlatformInputAuditReceiptPc34 {
    int raw_material_admitted, existing_runtime_owner_preserved;
    int platform_material_required, floppy_material_required, input_material_required;
    int memory_material_required, graphics_material_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    CSB_V1_PlatformInputFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_PlatformInputAuditReceiptPc34;

int csb_v1_f0526_f0545_platform_input_audit_pc34(
    const CSB_V1_PlatformInputRawMaterialPc34 *raw,
    CSB_V1_PlatformInputFunctionPc34 function_id,
    CSB_V1_PlatformInputAuditReceiptPc34 *out);

#endif
