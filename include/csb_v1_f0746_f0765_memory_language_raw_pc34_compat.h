#ifndef FIRESTAFF_CSB_V1_F0746_F0765_MEMORY_LANGUAGE_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0746_F0765_MEMORY_LANGUAGE_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef struct CSB_V1_MemoryLanguageRawPc34 {
    const uint8_t *graphics; size_t graphics_size; uint32_t graphics_identity;
    const uint8_t *text; size_t text_size; uint32_t text_identity;
    const uint8_t *language; size_t language_size; uint32_t language_identity;
    const uint8_t *memory; size_t memory_size; uint32_t memory_identity;
    const uint8_t *package; size_t package_size; uint32_t package_identity;
    int authenticated_pc34;
} CSB_V1_MemoryLanguageRawPc34;

typedef struct CSB_V1_MemoryLanguageReceiptPc34 {
    int raw_material_admitted, existing_runtime_owner_preserved;
    int graphics_required, text_required, language_required, memory_required, package_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    int function_number;
    const char *source_evidence;
} CSB_V1_MemoryLanguageReceiptPc34;

int csb_v1_f0746_f0765_memory_language_audit_pc34(
    const CSB_V1_MemoryLanguageRawPc34 *raw, int function_number,
    CSB_V1_MemoryLanguageReceiptPc34 *out);

#endif
