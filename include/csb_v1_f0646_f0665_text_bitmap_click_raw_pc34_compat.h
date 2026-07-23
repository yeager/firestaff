#ifndef FIRESTAFF_CSB_V1_F0646_F0665_TEXT_BITMAP_CLICK_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0646_F0665_TEXT_BITMAP_CLICK_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_TextBitmapClickFunctionPc34 {
    CSB_V1_TEXT_BITMAP_CLICK_F0646 = 646, CSB_V1_TEXT_BITMAP_CLICK_F0647 = 647,
    CSB_V1_TEXT_BITMAP_CLICK_F0648 = 648, CSB_V1_TEXT_BITMAP_CLICK_F0649 = 649,
    CSB_V1_TEXT_BITMAP_CLICK_F0650 = 650, CSB_V1_TEXT_BITMAP_CLICK_F0651 = 651,
    CSB_V1_TEXT_BITMAP_CLICK_F0652 = 652, CSB_V1_TEXT_BITMAP_CLICK_F0653 = 653,
    CSB_V1_TEXT_BITMAP_CLICK_F0654 = 654, CSB_V1_TEXT_BITMAP_CLICK_F0655 = 655,
    CSB_V1_TEXT_BITMAP_CLICK_F0656 = 656, CSB_V1_TEXT_BITMAP_CLICK_F0657 = 657,
    CSB_V1_TEXT_BITMAP_CLICK_F0658 = 658, CSB_V1_TEXT_BITMAP_CLICK_F0659 = 659,
    CSB_V1_TEXT_BITMAP_CLICK_F0660 = 660, CSB_V1_TEXT_BITMAP_CLICK_F0661 = 661,
    CSB_V1_TEXT_BITMAP_CLICK_F0662 = 662, CSB_V1_TEXT_BITMAP_CLICK_F0663 = 663,
    CSB_V1_TEXT_BITMAP_CLICK_F0664 = 664, CSB_V1_TEXT_BITMAP_CLICK_F0665 = 665
} CSB_V1_TextBitmapClickFunctionPc34;

typedef struct CSB_V1_TextBitmapClickRawMaterialPc34 {
    const uint8_t *text_material; size_t text_material_size; uint32_t text_material_identity;
    const uint8_t *font_material; size_t font_material_size; uint32_t font_material_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *zone_material; size_t zone_material_size; uint32_t zone_material_identity;
    const uint8_t *timeline_material; size_t timeline_material_size; uint32_t timeline_material_identity;
    const uint8_t *palette_material; size_t palette_material_size; uint32_t palette_material_identity;
    const uint8_t *dungeon_material; size_t dungeon_material_size; uint32_t dungeon_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *memory_material; size_t memory_material_size; uint32_t memory_material_identity;
    int authenticated_pc34;
} CSB_V1_TextBitmapClickRawMaterialPc34;

typedef struct CSB_V1_TextBitmapClickAuditReceiptPc34 {
    int raw_material_admitted, existing_runtime_owner_preserved;
    int text_material_required, font_material_required, graphics_material_required, zone_material_required;
    int timeline_material_required, palette_material_required, dungeon_material_required, input_material_required, memory_material_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    CSB_V1_TextBitmapClickFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_TextBitmapClickAuditReceiptPc34;

int csb_v1_f0646_f0665_text_bitmap_click_audit_pc34(
    const CSB_V1_TextBitmapClickRawMaterialPc34 *raw,
    CSB_V1_TextBitmapClickFunctionPc34 function_id,
    CSB_V1_TextBitmapClickAuditReceiptPc34 *out);

#endif
