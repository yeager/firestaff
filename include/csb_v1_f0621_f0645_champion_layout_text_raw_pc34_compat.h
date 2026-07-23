#ifndef FIRESTAFF_CSB_V1_F0621_F0645_CHAMPION_LAYOUT_TEXT_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0621_F0645_CHAMPION_LAYOUT_TEXT_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_ChampionLayoutTextFunctionPc34 {
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0621 = 621, CSB_V1_CHAMPION_LAYOUT_TEXT_F0622 = 622,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0623 = 623, CSB_V1_CHAMPION_LAYOUT_TEXT_F0624 = 624,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0625 = 625, CSB_V1_CHAMPION_LAYOUT_TEXT_F0626 = 626,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0627 = 627, CSB_V1_CHAMPION_LAYOUT_TEXT_F0628 = 628,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0629 = 629, CSB_V1_CHAMPION_LAYOUT_TEXT_F0630 = 630,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0631 = 631, CSB_V1_CHAMPION_LAYOUT_TEXT_F0632 = 632,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0633 = 633, CSB_V1_CHAMPION_LAYOUT_TEXT_F0634 = 634,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0635 = 635, CSB_V1_CHAMPION_LAYOUT_TEXT_F0636 = 636,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0637 = 637, CSB_V1_CHAMPION_LAYOUT_TEXT_F0638 = 638,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0639 = 639, CSB_V1_CHAMPION_LAYOUT_TEXT_F0640 = 640,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0641 = 641, CSB_V1_CHAMPION_LAYOUT_TEXT_F0642 = 642,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0643 = 643, CSB_V1_CHAMPION_LAYOUT_TEXT_F0644 = 644,
    CSB_V1_CHAMPION_LAYOUT_TEXT_F0645 = 645
} CSB_V1_ChampionLayoutTextFunctionPc34;

typedef struct CSB_V1_ChampionLayoutTextRawMaterialPc34 {
    const uint8_t *champion_material; size_t champion_material_size; uint32_t champion_material_identity;
    const uint8_t *zone_layout_material; size_t zone_layout_material_size; uint32_t zone_layout_material_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *font_material; size_t font_material_size; uint32_t font_material_identity;
    const uint8_t *text_material; size_t text_material_size; uint32_t text_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *memory_material; size_t memory_material_size; uint32_t memory_material_identity;
    int authenticated_pc34;
} CSB_V1_ChampionLayoutTextRawMaterialPc34;

typedef struct CSB_V1_ChampionLayoutTextAuditReceiptPc34 {
    int raw_material_admitted, existing_runtime_owner_preserved;
    int champion_material_required, zone_layout_material_required, graphics_material_required;
    int font_material_required, text_material_required, input_material_required, memory_material_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    CSB_V1_ChampionLayoutTextFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_ChampionLayoutTextAuditReceiptPc34;

int csb_v1_f0621_f0645_champion_layout_text_audit_pc34(
    const CSB_V1_ChampionLayoutTextRawMaterialPc34 *raw,
    CSB_V1_ChampionLayoutTextFunctionPc34 function_id,
    CSB_V1_ChampionLayoutTextAuditReceiptPc34 *out);

#endif
