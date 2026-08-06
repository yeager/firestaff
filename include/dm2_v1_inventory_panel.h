#ifndef FIRESTAFF_DM2_V1_INVENTORY_PANEL_H
#define FIRESTAFF_DM2_V1_INVENTORY_PANEL_H

/*
 * DM2 V1 inventory/item-panel view helper.
 *
 * This is a data-free bridge around the existing DM2 champion inventory,
 * leader-hand possession, and DB handle helpers. It does not dispatch input
 * or render pixels.
 *
 * Source anchors:
 *   ReDMCSB DEFS.H:779-810 slot indices, CM1 leader hand sentinel.
 *   ReDMCSB PANEL.C:1127-1200 object-description panel text/icon route.
 *   ReDMCSB PANEL.C:1658-1692 action-hand item panel fallback route.
 *   ReDMCSB CHAMPION.C:250-268/270-282 leader-hand put/remove state.
 *   ReDMCSB LOADSAVE.C:1535-1537/2744 leader-hand object persistence.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_save_load.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_INV_SLOT_LEADER_HAND      (-1)
#define DM2_V1_INV_SLOT_READY_HAND       0
#define DM2_V1_INV_SLOT_ACTION_HAND      1
#define DM2_V1_INV_SLOT_HEAD             2
#define DM2_V1_INV_SLOT_TORSO            3
#define DM2_V1_INV_SLOT_LEGS             4
#define DM2_V1_INV_SLOT_FEET             5
#define DM2_V1_INV_SLOT_NECK             10
#define DM2_V1_INV_SLOT_BACKPACK_FIRST   13
#define DM2_V1_INV_SLOT_COUNT            30

/* c_gui_draw.cpp::DM2_DRAW_ITEM_SURVEY draws its static description frame
 * through DRAW_STATIC_PIC(7, 0, 1, RECT_1EE, NOALPHA). */
#define DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT 0x01eeu

typedef struct {
    uint32_t object_id;
    const char *description;
} DM2_V1_InventoryPanelDescription;

typedef struct {
    int selected_slot;
    uint32_t object_id;
    int has_object;
    int db_resolved;
    uint8_t db_pool;
    uint32_t db_index;
    char description[64];
} DM2_V1_InventoryPanelItemView;

/* A deliberately small, indexed target for authenticated HUD material.
 * The owner supplies the exact source rect and may not request scaling or a
 * host-colour replacement through this interface. */
typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int stride;
} DM2_V1_InventoryPanelHudSurface;

typedef struct {
    uint16_t rect_number;
    int destination_x;
    int destination_y;
    uint16_t width;
    uint16_t height;
    uint8_t transparent_index;
} DM2_V1_InventoryPanelHudBlit;

typedef struct {
    int valid;
    uint8_t category;
    uint8_t index;
    uint8_t image_field;
    uint16_t expanded_rect_index;
    uint16_t source_raw_index;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t raw_hash;
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_InventoryPanelHandSlotBackdropReceipt;

typedef DM2_V1_InventoryPanelHandSlotBackdropReceipt
    DM2_V1_InventoryPanelSurveyFrameReceipt;

typedef struct {
    int valid;
    uint32_t drawn_pixel_count;
    uint32_t transparent_pixel_count;
    uint32_t identity_hash;
} DM2_V1_InventoryPanelHudConsumptionReceipt;

const char *dm2_v1_inventory_slot_label(int slot);
int dm2_v1_inventory_slot_is_equipment(int slot);

int dm2_v1_inventory_panel_select_item(
    const DM2_ChampionRecord *champion,
    const DM2_LeaderPossession *leader_hand,
    int selected_slot,
    const DM2_DB_State *db,
    const DM2_V1_InventoryPanelDescription *descriptions,
    size_t description_count,
    DM2_V1_InventoryPanelItemView *out);

/* SKProject SKWIN c_gui_draw.cpp::DM2_DRAW_HAND_ACTION_ICONS.  This binds
 * INTERFACE_GENERAL/4/the exact action field to the source expanded rect;
 * invalid possession, direction, palette or raw material stays no-draw. */
int dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t possession_index,
    uint8_t left_or_right,
    uint8_t player_position,
    uint8_t party_direction,
    DM2_V1_InventoryPanelHandSlotBackdropReceipt *out_receipt);

/* SKProject SKWIN c_gui_draw.cpp::DM2_DRAW_ITEM_SURVEY static frame. */
int dm2_v1_inventory_panel_survey_frame_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_InventoryPanelSurveyFrameReceipt *out_receipt);

/* Materialize a receipt only as an exact-size, local-palette indexed blit.
 * The shared primitive is intentionally not public to generic UI callers. */
int dm2_v1_inventory_panel_consume_hand_slot_backdrop(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandSlotBackdropReceipt *receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_consumption);
int dm2_v1_inventory_panel_consume_survey_frame(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelSurveyFrameReceipt *receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_consumption);

const char *dm2_v1_inventory_panel_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_INVENTORY_PANEL_H */
