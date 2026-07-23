#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_FOOD_WATER_STATUS_BOX_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_FOOD_WATER_STATUS_BOX_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "asset_loader_m11.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 67
#define DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34 29
#define DM1_V1_CPFW_STATUS_BOX_BYTES_PC34 \
    (DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 * DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34)
#define DM1_V1_CPFW_MAX_OPERATIONS_PC34 12
#define DM1_V1_CPFW_CHEST_SLOT_COUNT_PC34 8

typedef enum {
    DM1_V1_CPFW_OP_CHEST_CLOSE_PC34 = 1,
    DM1_V1_CPFW_OP_PANEL_CLOSE_BRACKET_PC34 = 2,
    DM1_V1_CPFW_OP_STATUS_BOX_FILL_PC34 = 3,
    DM1_V1_CPFW_OP_STATUS_BOX_BORDER_PC34 = 4,
    DM1_V1_CPFW_OP_PANEL_EMPTY_PC34 = 5,
    DM1_V1_CPFW_OP_FOOD_LABEL_PC34 = 6,
    DM1_V1_CPFW_OP_WATER_LABEL_PC34 = 7,
    DM1_V1_CPFW_OP_FOOD_BAR_PC34 = 8,
    DM1_V1_CPFW_OP_WATER_BAR_PC34 = 9
} dm1_v1_champion_panel_food_water_status_box_operation_kind_pc34_t;

typedef struct {
    int contract_only;
    int champion_count;
    int status_box_first_zone;
    int status_box_last_zone;
    int status_box_width;
    int status_box_height;
    int status_box_stride;
    int status_box_fill_color;
    int status_box_border_transparent_color;
    int panel_empty_graphic;
    int food_label_graphic;
    int water_label_graphic;
    int food_label_zone;
    int water_label_zone;
    int food_bar_zone;
    int water_bar_zone;
    int food_base_color;
    int water_base_color;
    int warning_yellow_color;
    int warning_red_color;
    const char *chest_close_anchor;
    const char *menu_anchor;
    const char *panel_draw_anchor;
    const char *draw_state_anchor;
    const char *champion_state_anchor;
    const char *panel_close_anchor;
    const char *defs_anchor;
} dm1_v1_champion_panel_food_water_status_box_contract_pc34_t;

typedef struct {
    int inventory_champion_ordinal;
    int party_champion_count;
    int current_health;
    int food;
    int water;
    int poison_event_count;
    uint16_t open_chest_thing;
    uint16_t chest_slots[DM1_V1_CPFW_CHEST_SLOT_COUNT_PC34];
} dm1_v1_champion_panel_food_water_status_box_input_pc34_t;

typedef struct {
    dm1_v1_champion_panel_food_water_status_box_operation_kind_pc34_t kind;
    int sequence;
    int graphic_id;
    int zone_id;
    int transparent_color;
    int fill_color;
    int amount;
    int proportional_units;
    const char *sourceEvidence;
} dm1_v1_champion_panel_food_water_status_box_operation_pc34_t;

typedef struct {
    int champion_index;
    int zone_id;
    int screen_x;
    int screen_y;
    int width;
    int height;
    int fill_color;
    int border_transparent_color;
    int fill_pixel_count;
    int transparent_border_pixel_count;
    uint8_t bytes[DM1_V1_CPFW_STATUS_BOX_BYTES_PC34];
} dm1_v1_champion_panel_food_water_status_box_frame_pc34_t;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    int shadow_offset;
} dm1_v1_champion_panel_food_water_bar_zone_pc34_t;

typedef struct {
    int valid;
    int rejected_invalid_champion;
    int rejected_dead_champion;
    int contract_only;
    int loads_graphics_dat;
    int loads_dungeon_dat;
    int close_before_status_box;
    int status_box_before_panel;
    int chest_was_open;
    int open_chest_cleared;
    int non_empty_chest_slots;
    uint16_t relink_first_thing;
    uint16_t relink_last_thing;
    int inventory_champion_index;
    int party_champion_count;
    int g0423_inventory_champion_ordinal;
    int g0305_party_champion_count;
    int food_counter;
    int water_counter;
    int poison_event_count;
    int food_word_color;
    int water_word_color;
    int food_bar_color;
    int water_bar_color;
    int food_bar_units;
    int water_bar_units;
    int menu_f0409_f0410_f0411_disambiguated;
    int operation_count;
    dm1_v1_champion_panel_food_water_status_box_operation_pc34_t
        operations[DM1_V1_CPFW_MAX_OPERATIONS_PC34];
    dm1_v1_champion_panel_food_water_status_box_frame_pc34_t frame;
    const char *sourceEvidence;
} dm1_v1_champion_panel_food_water_status_box_result_pc34_t;

/*
 * F0134 fills the alive champion status rectangle with C12.  F0135 then
 * composes the PANEL.C F0345 C020/C030/C031 GRAPHICS.DAT surfaces into the
 * inventory panel.  Keep that transaction explicitly source-backed: callers
 * must pass decoded original pixels and may not replace a missing surface
 * with host text or a generated panel.
 */
typedef struct {
    int graphics_dat_backed;
    int graphic_id;
    int width;
    int height;
    const uint8_t *pixels;
} dm1_v1_champion_panel_food_water_material_surface_pc34_t;

typedef struct {
    int admitted;
    int rejected_missing_panel;
    int rejected_missing_food_label;
    int rejected_missing_water_label;
    int graphics_dat_loader_ready;
    int indexed_vga4_format_valid;
    int rejected_invalid_pixel_format;
    int f0134_status_fill_color;
    int f0135_panel_graphic;
    int f0135_food_label_graphic;
    int f0135_water_label_graphic;
    uint32_t panel_pixel_fingerprint;
    uint32_t food_label_pixel_fingerprint;
    uint32_t water_label_pixel_fingerprint;
    const char *sourceEvidence;
} dm1_v1_champion_panel_food_water_material_receipt_pc34_t;

/* The production F0134/F0135 handoff pins the same PC34 light-0 palette
 * that the indexed panel pixels use, then verifies C101/C500/C501 placement
 * before M11 can consume the sequence. */
typedef struct {
    int admitted;
    int noDraw;
    int paletteSourceBound;
    int livePlacementValid;
    uint32_t paletteFingerprint;
    int panelX;
    int panelY;
    int foodLabelX;
    int foodLabelY;
    int waterLabelX;
    int waterLabelY;
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t material;
    const char *sourceEvidence;
} dm1_v1_champion_panel_food_water_runtime_receipt_pc34_t;

const dm1_v1_champion_panel_food_water_status_box_contract_pc34_t *
dm1_v1_champion_panel_food_water_status_box_contract_pc34(void);

const char *
dm1_v1_champion_panel_food_water_status_box_source_evidence_pc34(void);

dm1_v1_champion_panel_food_water_status_box_input_pc34_t
dm1_v1_champion_panel_food_water_status_box_default_input_pc34(void);

dm1_v1_champion_panel_food_water_status_box_result_pc34_t
dm1_v1_champion_panel_food_water_status_box_probe_pc34(
    const dm1_v1_champion_panel_food_water_status_box_input_pc34_t *input);

int dm1_v1_champion_panel_food_water_material_admit_pc34(
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *panel,
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *food_label,
    const dm1_v1_champion_panel_food_water_material_surface_pc34_t *water_label,
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t *out_receipt);

/* Production caller for PANEL.C F0345's C020/C030/C031 path.  It accepts
 * only live slots decoded from the active original GRAPHICS.DAT loader; no
 * generated panel or text fallback is available through this boundary. */
int dm1_v1_champion_panel_food_water_material_admit_graphics_slots_pc34(
    const M11_AssetSlot *panel,
    const M11_AssetSlot *food_label,
    const M11_AssetSlot *water_label,
    dm1_v1_champion_panel_food_water_material_receipt_pc34_t *out_receipt);

int dm1_v1_champion_panel_food_water_material_admit_runtime_pc34(
    const M11_AssetSlot *panel,
    const M11_AssetSlot *food_label,
    const M11_AssetSlot *water_label,
    const uint8_t *palette,
    size_t paletteByteCount,
    int panelX,
    int panelY,
    int foodLabelX,
    int foodLabelY,
    int waterLabelX,
    int waterLabelY,
    int framebufferWidth,
    int framebufferHeight,
    dm1_v1_champion_panel_food_water_runtime_receipt_pc34_t *out_receipt);

dm1_v1_champion_panel_food_water_bar_zone_pc34_t
dm1_v1_champion_panel_food_bar_zone_pc34(void);
dm1_v1_champion_panel_food_water_bar_zone_pc34_t
dm1_v1_champion_panel_water_bar_zone_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
