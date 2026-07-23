#ifndef FIRESTAFF_CSB_V1_F1106_F1125_MEDIA_PALETTE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1106_F1125_MEDIA_PALETTE_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_MediaPaletteRawPc34 { const uint8_t *package; size_t package_size; uint32_t package_identity; const uint8_t *palette; size_t palette_size; uint32_t palette_identity; int authenticated_pc34; } CSB_V1_MediaPaletteRawPc34;
typedef struct CSB_V1_MediaPaletteReceiptPc34 { int raw_material_admitted,existing_runtime_owner_preserved; int package_required,palette_required; int read_only_query,runtime_execution_blocked,platform_behavior_fail_closed; int function_number; const char *source_evidence; } CSB_V1_MediaPaletteReceiptPc34;
int csb_v1_f1106_f1125_media_palette_audit_pc34(const CSB_V1_MediaPaletteRawPc34 *raw,int function_number,CSB_V1_MediaPaletteReceiptPc34 *out);
#endif
