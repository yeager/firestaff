#ifndef FIRESTAFF_CSB_V1_F0346_F0365_PANEL_INPUT_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0346_F0365_PANEL_INPUT_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_PanelInputFunctionPc34 {
    CSB_V1_PANEL_INPUT_F0346 = 346, CSB_V1_PANEL_INPUT_F0347 = 347,
    CSB_V1_PANEL_INPUT_F0348 = 348, CSB_V1_PANEL_INPUT_F0349 = 349,
    CSB_V1_PANEL_INPUT_F0350 = 350, CSB_V1_PANEL_INPUT_F0351 = 351,
    CSB_V1_PANEL_INPUT_F0352 = 352, CSB_V1_PANEL_INPUT_F0353 = 353,
    CSB_V1_PANEL_INPUT_F0354 = 354, CSB_V1_PANEL_INPUT_F0355 = 355,
    CSB_V1_PANEL_INPUT_F0356 = 356, CSB_V1_PANEL_INPUT_F0357 = 357,
    CSB_V1_PANEL_INPUT_F0358 = 358, CSB_V1_PANEL_INPUT_F0359 = 359,
    CSB_V1_PANEL_INPUT_F0360 = 360, CSB_V1_PANEL_INPUT_F0361 = 361,
    CSB_V1_PANEL_INPUT_F0362 = 362, CSB_V1_PANEL_INPUT_F0363 = 363,
    CSB_V1_PANEL_INPUT_F0364 = 364, CSB_V1_PANEL_INPUT_F0365 = 365
} CSB_V1_PanelInputFunctionPc34;

typedef struct CSB_V1_PanelInputRawMaterialPc34 {
    const uint8_t *champion_record; size_t champion_record_size; uint32_t champion_record_identity;
    const uint8_t *graphics_material; size_t graphics_material_size; uint32_t graphics_material_identity;
    const uint8_t *dungeon_material; size_t dungeon_material_size; uint32_t dungeon_material_identity;
    const uint8_t *input_material; size_t input_material_size; uint32_t input_material_identity;
    const uint8_t *platform_material; size_t platform_material_size; uint32_t platform_material_identity;
    int authenticated_pc34;
} CSB_V1_PanelInputRawMaterialPc34;

typedef struct CSB_V1_PanelInputAuditReceiptPc34 {
    int raw_material_admitted;
    int champion_material_required;
    int graphics_material_required;
    int dungeon_material_required;
    int input_material_required;
    int platform_material_required;
    int read_only_query;
    int runtime_execution_blocked;
    int platform_behavior_fail_closed;
    CSB_V1_PanelInputFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_PanelInputAuditReceiptPc34;

int csb_v1_f0346_f0365_panel_input_audit_pc34(
    const CSB_V1_PanelInputRawMaterialPc34 *raw,
    CSB_V1_PanelInputFunctionPc34 function_id,
    CSB_V1_PanelInputAuditReceiptPc34 *out);

#endif
