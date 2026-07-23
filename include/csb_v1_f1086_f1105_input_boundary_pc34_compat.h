#ifndef FIRESTAFF_CSB_V1_F1086_F1105_INPUT_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1086_F1105_INPUT_BOUNDARY_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_InputBoundaryRawPc34 { const uint8_t *package; size_t package_size; uint32_t package_identity; const uint8_t *input; size_t input_size; uint32_t input_identity; int authenticated_pc34; } CSB_V1_InputBoundaryRawPc34;
typedef struct CSB_V1_InputBoundaryReceiptPc34 { int raw_material_admitted,existing_runtime_owner_preserved; int package_required,input_required; int read_only_query,runtime_execution_blocked,platform_behavior_fail_closed; int function_number; const char *source_evidence; } CSB_V1_InputBoundaryReceiptPc34;
int csb_v1_f1086_f1105_input_boundary_audit_pc34(const CSB_V1_InputBoundaryRawPc34 *raw,int function_number,CSB_V1_InputBoundaryReceiptPc34 *out);
#endif
