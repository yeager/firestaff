#ifndef FIRESTAFF_CSB_V1_F0786_F0805_PANEL_LAYOUT_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0786_F0805_PANEL_LAYOUT_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>
typedef struct CSB_V1_PanelLayoutRawPc34 {
    const uint8_t *graphics; size_t graphics_size; uint32_t graphics_identity;
    const uint8_t *zone; size_t zone_size; uint32_t zone_identity;
    const uint8_t *dungeon; size_t dungeon_size; uint32_t dungeon_identity;
    const uint8_t *sound; size_t sound_size; uint32_t sound_identity;
    const uint8_t *package; size_t package_size; uint32_t package_identity;
    int authenticated_pc34;
} CSB_V1_PanelLayoutRawPc34;
typedef struct CSB_V1_PanelLayoutReceiptPc34 {
    int raw_material_admitted, existing_runtime_owner_preserved;
    int graphics_required, zone_required, dungeon_required, sound_required, package_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    int function_number; const char *source_evidence;
} CSB_V1_PanelLayoutReceiptPc34;
int csb_v1_f0786_f0805_panel_layout_audit_pc34(const CSB_V1_PanelLayoutRawPc34 *raw,
    int function_number, CSB_V1_PanelLayoutReceiptPc34 *out);
#endif
