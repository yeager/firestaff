#ifndef FIRESTAFF_CSB_V1_F0906_F0925_SWOOSH_PRIMITIVE_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0906_F0925_SWOOSH_PRIMITIVE_RAW_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_SwooshPrimitiveRawPc34 {
 const uint8_t *package; size_t package_size; uint32_t package_identity;
 const uint8_t *graphics; size_t graphics_size; uint32_t graphics_identity;
 const uint8_t *sound; size_t sound_size; uint32_t sound_identity;
 const uint8_t *utility; size_t utility_size; uint32_t utility_identity;
 int authenticated_pc34;
} CSB_V1_SwooshPrimitiveRawPc34;
typedef struct CSB_V1_SwooshPrimitiveReceiptPc34 {
 int raw_material_admitted, existing_runtime_owner_preserved;
 int package_required, graphics_required, sound_required, utility_required;
 int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
 int function_number; const char *source_evidence;
} CSB_V1_SwooshPrimitiveReceiptPc34;
int csb_v1_f0906_f0925_swoosh_primitive_audit_pc34(const CSB_V1_SwooshPrimitiveRawPc34 *raw,int function_number,CSB_V1_SwooshPrimitiveReceiptPc34 *out);
#endif
