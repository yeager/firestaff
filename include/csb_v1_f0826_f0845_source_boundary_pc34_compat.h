#ifndef FIRESTAFF_CSB_V1_F0826_F0845_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0826_F0845_SOURCE_BOUNDARY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_F0826_F0845_RawPc34 {
    const uint8_t *package; size_t package_size; uint32_t package_identity;
    const uint8_t *dungeon; size_t dungeon_size; uint32_t dungeon_identity;
    int authenticated_pc34;
} CSB_V1_F0826_F0845_RawPc34;
typedef struct CSB_V1_F0826_F0845_ReceiptPc34 {
    int source_symbol_missing, raw_material_rejected;
    int runtime_execution_blocked, platform_behavior_fail_closed;
    int function_number;
    const char *source_evidence;
} CSB_V1_F0826_F0845_ReceiptPc34;
int csb_v1_f0826_f0845_source_boundary_pc34(const CSB_V1_F0826_F0845_RawPc34 *raw,
    int function_number, CSB_V1_F0826_F0845_ReceiptPc34 *out);
#endif
