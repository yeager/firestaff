#ifndef FIRESTAFF_CSB_V1_F0486_F0505_GRAPHICS_LZW_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0486_F0505_GRAPHICS_LZW_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_GraphicsLzwFunctionPc34 {
    CSB_V1_GRAPHICS_LZW_F0486 = 486, CSB_V1_GRAPHICS_LZW_F0487 = 487,
    CSB_V1_GRAPHICS_LZW_F0488 = 488, CSB_V1_GRAPHICS_LZW_F0489 = 489,
    CSB_V1_GRAPHICS_LZW_F0490 = 490, CSB_V1_GRAPHICS_LZW_F0491 = 491,
    CSB_V1_GRAPHICS_LZW_F0492 = 492, CSB_V1_GRAPHICS_LZW_F0493 = 493,
    CSB_V1_GRAPHICS_LZW_F0494 = 494, CSB_V1_GRAPHICS_LZW_F0495 = 495,
    CSB_V1_GRAPHICS_LZW_F0496 = 496, CSB_V1_GRAPHICS_LZW_F0497 = 497,
    CSB_V1_GRAPHICS_LZW_F0498 = 498, CSB_V1_GRAPHICS_LZW_F0499 = 499,
    CSB_V1_GRAPHICS_LZW_F0500 = 500, CSB_V1_GRAPHICS_LZW_F0501 = 501,
    CSB_V1_GRAPHICS_LZW_F0502 = 502, CSB_V1_GRAPHICS_LZW_F0503 = 503,
    CSB_V1_GRAPHICS_LZW_F0504 = 504, CSB_V1_GRAPHICS_LZW_F0505 = 505
} CSB_V1_GraphicsLzwFunctionPc34;

typedef struct CSB_V1_GraphicsLzwRawMaterialPc34 {
    const uint8_t *memory_material; size_t memory_material_size; uint32_t memory_material_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *cache_material; size_t cache_material_size; uint32_t cache_material_identity;
    const uint8_t *compressed_material; size_t compressed_material_size; uint32_t compressed_material_identity;
    const uint8_t *audio_material; size_t audio_material_size; uint32_t audio_material_identity;
    const uint8_t *platform_material; size_t platform_material_size; uint32_t platform_material_identity;
    int authenticated_pc34;
} CSB_V1_GraphicsLzwRawMaterialPc34;

typedef struct CSB_V1_GraphicsLzwAuditReceiptPc34 {
    int raw_material_admitted, existing_runtime_owner_preserved;
    int memory_material_required, graphics_material_required, cache_material_required;
    int compressed_material_required, audio_material_required, platform_material_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    CSB_V1_GraphicsLzwFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_GraphicsLzwAuditReceiptPc34;

int csb_v1_f0486_f0505_graphics_lzw_audit_pc34(
    const CSB_V1_GraphicsLzwRawMaterialPc34 *raw,
    CSB_V1_GraphicsLzwFunctionPc34 function_id,
    CSB_V1_GraphicsLzwAuditReceiptPc34 *out);

#endif
