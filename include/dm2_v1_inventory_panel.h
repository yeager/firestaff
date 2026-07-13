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

/* A no-draw receipt for one item image that the source HUD route has already
 * selected.  ObjectID-to-GDAT routing remains owned by the original record
 * route; callers must provide that exact category/index/field triple. */
typedef struct {
    int valid;
    uint32_t object_id;
    uint8_t gdat_category;
    uint8_t gdat_index;
    uint8_t image_field;
    DM2_V1_GdatImageMetadata image_metadata;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint32_t decoded_pixel_count;
    uint32_t decoded_pixels_hash;
    uint32_t material_hash;
} DM2_V1_InventoryPanelGdatMaterialReceipt;

/* Binds a selected inventory object to its already-verified GDAT material.
 * This API never selects an icon or generates substitute pixels. */
typedef struct {
    int valid;
    int selected_slot;
    uint32_t object_id;
    uint8_t db_pool;
    uint32_t db_index;
    DM2_V1_InventoryPanelGdatMaterialReceipt material;
    uint32_t receipt_hash;
} DM2_V1_InventoryPanelHudReceipt;

/* Source-owned QUERY_BLIT_RECT output for DRAW_ITEM_ICON.  The panel cannot
 * invent a rect number, clipping region, source offset, or transparency key. */
typedef struct {
    uint16_t rect_number;
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    uint16_t width;
    uint16_t height;
    uint8_t transparent_index;
} DM2_V1_InventoryPanelHudBlit;

typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int stride;
} DM2_V1_InventoryPanelHudSurface;

typedef struct {
    int valid;
    uint16_t rect_number;
    uint16_t width;
    uint16_t height;
    uint32_t drawn_pixel_count;
    uint32_t transparent_pixel_count;
    uint32_t blit_hash;
} DM2_V1_InventoryPanelHudConsumptionReceipt;

/* skproject DRAW_ITEM_SURVEY probes this exact optional item image before it
 * presents the selected item's survey panel. */
#define DM2_V1_INVENTORY_SURVEY_PREVIEW_FIELD 0x11u
#define DM2_V1_INVENTORY_SURVEY_PREVIEW_RECT  0x01eeu
#define DM2_V1_INVENTORY_SURVEY_TRANSPARENCY  12u

typedef struct {
    int valid;
    uint16_t expanded_rect_index;
    uint8_t transparent_index;
    DM2_V1_InventoryPanelHudReceipt hud;
    uint32_t receipt_hash;
} DM2_V1_InventoryPanelSurveyPreviewReceipt;

/* DRAW_ITEM_IN_HAND copies one selected item's exact local palette into the
 * leader-hand picture, then blits the entire decoded image at its origin. */
typedef struct {
    int valid;
    uint16_t origin_width;
    uint16_t origin_height;
    DM2_V1_InventoryPanelHudReceipt hud;
    uint32_t receipt_hash;
} DM2_V1_InventoryPanelHandReceipt;

/* skproject DRAW_HAND_ACTION_ICONS paints this exact interface-GDAT backdrop
 * before compositing the held item.  The source's rect and dtImage field are
 * derived solely from its possession/side/direction inputs. */
typedef struct {
    int valid;
    uint16_t expanded_rect_index;
    uint8_t possession_index;
    uint8_t left_or_right;
    uint8_t image_field;
    DM2_V1_GdatImageMetadata image_metadata;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint32_t decoded_pixels_hash;
    uint32_t receipt_hash;
} DM2_V1_InventoryPanelHandSlotBackdropReceipt;

/* DRAW_ITEM_SURVEY opens the description panel with this static original
 * character-sheet image before it optionally draws an item-specific preview. */
typedef struct {
    int valid;
    uint16_t expanded_rect_index;
    uint8_t image_field;
    DM2_V1_GdatImageMetadata image_metadata;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t decoded_width;
    uint16_t decoded_height;
    DM2_ImageFormat decoded_format;
    uint32_t decoded_pixels_hash;
    uint32_t receipt_hash;
} DM2_V1_InventoryPanelSurveyFrameReceipt;

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

int dm2_v1_inventory_panel_gdat_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint32_t object_id,
    uint8_t gdat_category,
    uint8_t gdat_index,
    uint8_t image_field,
    DM2_V1_InventoryPanelGdatMaterialReceipt *out_receipt);

int dm2_v1_inventory_panel_hud_receipt(
    const DM2_V1_InventoryPanelItemView *item,
    const DM2_V1_InventoryPanelGdatMaterialReceipt *material,
    DM2_V1_InventoryPanelHudReceipt *out_receipt);

int dm2_v1_inventory_panel_consume_hud_material(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHudReceipt *hud_receipt,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt);

int dm2_v1_inventory_panel_survey_preview_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelItemView *item,
    uint8_t gdat_category,
    uint8_t gdat_index,
    DM2_V1_InventoryPanelSurveyPreviewReceipt *out_receipt);

int dm2_v1_inventory_panel_consume_survey_preview(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelSurveyPreviewReceipt *preview,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt);

int dm2_v1_inventory_panel_hand_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelItemView *item,
    uint8_t gdat_category,
    uint8_t gdat_index,
    uint8_t image_field,
    DM2_V1_InventoryPanelHandReceipt *out_receipt);

int dm2_v1_inventory_panel_consume_hand_item(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandReceipt *hand,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt);

int dm2_v1_inventory_panel_hand_slot_backdrop_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t possession_index,
    uint8_t left_or_right,
    uint8_t champion_direction,
    uint8_t player_direction,
    DM2_V1_InventoryPanelHandSlotBackdropReceipt *out_receipt);

int dm2_v1_inventory_panel_consume_hand_slot_backdrop(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelHandSlotBackdropReceipt *backdrop,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt);

int dm2_v1_inventory_panel_survey_frame_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_InventoryPanelSurveyFrameReceipt *out_receipt);

int dm2_v1_inventory_panel_consume_survey_frame(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_InventoryPanelSurveyFrameReceipt *frame,
    const DM2_V1_InventoryPanelHudBlit *blit,
    DM2_V1_InventoryPanelHudSurface *surface,
    DM2_V1_InventoryPanelHudConsumptionReceipt *out_receipt);

const char *dm2_v1_inventory_panel_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_INVENTORY_PANEL_H */
