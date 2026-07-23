#ifndef FIRESTAFF_CSB_V1_F1046_F1065_SAVE_PLATFORM_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1046_F1065_SAVE_PLATFORM_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_SavePlatformRawPc34 { const uint8_t *save; size_t save_size; uint32_t save_identity; const uint8_t *graphics; size_t graphics_size; uint32_t graphics_identity; int authenticated_pc34; } CSB_V1_SavePlatformRawPc34;
typedef struct CSB_V1_SavePlatformReceiptPc34 { int raw_material_admitted,existing_runtime_owner_preserved; int save_required,graphics_required; int read_only_query,runtime_execution_blocked,platform_behavior_fail_closed; int function_number; const char *source_evidence; } CSB_V1_SavePlatformReceiptPc34;
int csb_v1_f1046_f1065_save_platform_audit_pc34(const CSB_V1_SavePlatformRawPc34 *raw,int function_number,CSB_V1_SavePlatformReceiptPc34 *out);
#endif
