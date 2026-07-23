#ifndef FIRESTAFF_CSB_V1_F1026_F1045_PLATFORM_VIDEO_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1026_F1045_PLATFORM_VIDEO_PC34_COMPAT_H
#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_PlatformVideoRawPc34 { const uint8_t *package; size_t package_size; uint32_t package_identity; const uint8_t *graphics; size_t graphics_size; uint32_t graphics_identity; const uint8_t *text; size_t text_size; uint32_t text_identity; int authenticated_pc34; } CSB_V1_PlatformVideoRawPc34;
typedef struct CSB_V1_PlatformVideoReceiptPc34 { int raw_material_admitted,existing_runtime_owner_preserved; int package_required,graphics_required,text_required; int read_only_query,runtime_execution_blocked,platform_behavior_fail_closed; int function_number; const char *source_evidence; } CSB_V1_PlatformVideoReceiptPc34;
int csb_v1_f1026_f1045_platform_video_audit_pc34(const CSB_V1_PlatformVideoRawPc34 *raw,int function_number,CSB_V1_PlatformVideoReceiptPc34 *out);
#endif
