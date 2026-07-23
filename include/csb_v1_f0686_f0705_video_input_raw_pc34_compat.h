#ifndef FIRESTAFF_CSB_V1_F0686_F0705_VIDEO_INPUT_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0686_F0705_VIDEO_INPUT_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>
typedef enum CSB_V1_VideoInputFunctionPc34 {
    CSB_V1_VIDEO_INPUT_F0686=686,CSB_V1_VIDEO_INPUT_F0687=687,CSB_V1_VIDEO_INPUT_F0688=688,CSB_V1_VIDEO_INPUT_F0689=689,CSB_V1_VIDEO_INPUT_F0690=690,CSB_V1_VIDEO_INPUT_F0691=691,CSB_V1_VIDEO_INPUT_F0692=692,CSB_V1_VIDEO_INPUT_F0693=693,CSB_V1_VIDEO_INPUT_F0694=694,CSB_V1_VIDEO_INPUT_F0695=695,CSB_V1_VIDEO_INPUT_F0696=696,CSB_V1_VIDEO_INPUT_F0697=697,CSB_V1_VIDEO_INPUT_F0698=698,CSB_V1_VIDEO_INPUT_F0699=699,CSB_V1_VIDEO_INPUT_F0700=700,CSB_V1_VIDEO_INPUT_F0701=701,CSB_V1_VIDEO_INPUT_F0702=702,CSB_V1_VIDEO_INPUT_F0703=703,CSB_V1_VIDEO_INPUT_F0704=704,CSB_V1_VIDEO_INPUT_F0705=705
} CSB_V1_VideoInputFunctionPc34;
typedef struct CSB_V1_VideoInputRawMaterialPc34 {
    const uint8_t *graphics_material;size_t graphics_material_size;uint32_t graphics_material_identity;
    const uint8_t *palette_material;size_t palette_material_size;uint32_t palette_material_identity;
    const uint8_t *text_material;size_t text_material_size;uint32_t text_material_identity;
    const uint8_t *font_material;size_t font_material_size;uint32_t font_material_identity;
    const uint8_t *zone_material;size_t zone_material_size;uint32_t zone_material_identity;
    const uint8_t *dungeon_material;size_t dungeon_material_size;uint32_t dungeon_material_identity;
    const uint8_t *input_material;size_t input_material_size;uint32_t input_material_identity;
    const uint8_t *champion_material;size_t champion_material_size;uint32_t champion_material_identity;
    const uint8_t *platform_material;size_t platform_material_size;uint32_t platform_material_identity;
    const uint8_t *timing_material;size_t timing_material_size;uint32_t timing_material_identity;
    const uint8_t *memory_material;size_t memory_material_size;uint32_t memory_material_identity;
    int authenticated_pc34;
} CSB_V1_VideoInputRawMaterialPc34;
typedef struct CSB_V1_VideoInputAuditReceiptPc34 {
    int raw_material_admitted,existing_runtime_owner_preserved;
    int graphics_material_required,palette_material_required,text_material_required,font_material_required,zone_material_required,dungeon_material_required,input_material_required,champion_material_required,platform_material_required,timing_material_required,memory_material_required;
    int read_only_query,runtime_execution_blocked,platform_behavior_fail_closed;
    CSB_V1_VideoInputFunctionPc34 function_id;const char *source_evidence;
} CSB_V1_VideoInputAuditReceiptPc34;
int csb_v1_f0686_f0705_video_input_audit_pc34(const CSB_V1_VideoInputRawMaterialPc34 *raw,CSB_V1_VideoInputFunctionPc34 function_id,CSB_V1_VideoInputAuditReceiptPc34 *out);
#endif
