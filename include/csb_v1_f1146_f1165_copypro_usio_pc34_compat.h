#ifndef FIRESTAFF_CSB_V1_F1146_F1165_COPYPRO_USIO_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1146_F1165_COPYPRO_USIO_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_CopyproUsioRawPc34 { const uint8_t *package; size_t package_size; uint32_t package_identity; const uint8_t *save; size_t save_size; uint32_t save_identity; int authenticated_pc34; } CSB_V1_CopyproUsioRawPc34;
typedef struct CSB_V1_CopyproUsioReceiptPc34 { int raw_material_admitted,existing_runtime_owner_preserved; int package_required,save_required; int read_only_query,runtime_execution_blocked,platform_behavior_fail_closed; int function_number; const char *source_evidence; } CSB_V1_CopyproUsioReceiptPc34;
int csb_v1_f1146_f1165_copypro_usio_audit_pc34(const CSB_V1_CopyproUsioRawPc34 *raw,int function_number,CSB_V1_CopyproUsioReceiptPc34 *out);
#endif
