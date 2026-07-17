#ifndef FIRESTAFF_DM2_V1_GDAT_PIT_M11_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_PIT_M11_RECEIPT_H

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_runtime.h"

#include <stdint.h>

/* Bounded c_gui_vp.cpp:4806-4856 DRAW_PIT_TILE admission.  Cell zero is
 * deliberately excluded: it calls SET_GRAPHICS_FLIP_FROM_POSITION and needs
 * a separate source-state receipt. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint8_t gdat_category;
    uint8_t graphicsset;
    uint8_t field;
    uint16_t rect_number;
    uint16_t light_parameter;
    uint8_t mirror_flip;
    uint8_t state_word_nonzero;
    uint32_t identity_hash;
} DM2_V1_GdatPitTransformReceipt;

typedef struct {
    int valid;
    int no_draw;
    DM2_V1_GdatPitTransformReceipt transform;
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_GdatGfxRawMaterialReceipt raw_material;
    uint16_t width;
    uint16_t height;
    uint16_t pixel_stride;
    const uint8_t *indexed_pixels;
    uint32_t indexed_pixel_count;
    DM2_ImageFormat format;
    uint8_t palette16[16];
    uint32_t decoded_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitM11Receipt;

/* PIT_TILE owns this borrowed decoded buffer independently of PIT_ROOF. */
typedef struct {
    int valid;
    int no_draw;
    const uint8_t *indexed_pixels;
    uint16_t width;
    uint16_t height;
    uint16_t pixel_stride;
    uint32_t indexed_pixel_count;
    uint32_t palette_hash;
    uint32_t material_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitMaterialHandoffReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint16_t rect_number;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t width;
    uint16_t height;
    uint32_t material_identity_hash;
    uint32_t table_hash;
    uint32_t row_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitRaw4Receipt;

/* DRAW_PIT_TILE reaches DRAW_DUNGEON_GRAPHIC, but the current source proof
 * stops before its per-cell QUERY_BLIT_RECT placement.  Bind the authentic
 * material to the viewport composition without inventing a draw slot. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t source_order_unresolved;
    uint32_t pit_material_identity_hash;
    uint32_t pit_transform_identity_hash;
    uint32_t summary_receipt_hash;
    uint32_t raw_material_receipt_hash;
    uint32_t session_identity;
    uint32_t data_epoch;
    uint32_t parent_composition_identity_hash;
    uint32_t parent_ordered_member_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitM11CompositionReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t source_order_unresolved;
    uint32_t pit_composition_identity_hash;
    uint32_t pit_raw4_identity_hash;
    uint32_t material_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitPlacementCompositionReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t normal_blit_unproven;
    uint32_t pit_placement_identity_hash;
    uint32_t pit_buffer_handoff_identity_hash;
    uint32_t generic_composition_identity_hash;
    uint32_t generic_ordered_member_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatPitM11ConsumeSlotReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint8_t blit_mode;
    uint16_t alpha_mask;
    uint16_t source_width;
    uint16_t source_height;
    uint32_t consume_slot_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitNormalRowReceipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t colors;
    uint8_t transformed_palette16[16];
    uint32_t material_identity_hash;
    uint32_t raw4_identity_hash;
    uint32_t placement_identity_hash;
    uint32_t normal_row_identity_hash;
    uint32_t raw7_hash;
    uint32_t transformed_palette_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitB073Receipt;

typedef struct {
    int valid;
    int no_draw;
    uint8_t view_cell;
    uint16_t query1;
    uint32_t raw4_identity_hash;
    uint32_t identity_hash;
} DM2_V1_GdatPitCropProvenanceReceipt;

int dm2_v1_gdat_pit_transform_receipt(uint8_t view_cell,
                                       uint16_t tile_state_word,
                                       uint16_t light_parameter,
                                       DM2_V1_GdatPitTransformReceipt *out_receipt);
int dm2_v1_gdat_pit_crop_provenance_intake(const DM2_V1_GdatPitM11Receipt *,
    const DM2_V1_GdatPitRaw4Receipt *, DM2_V1_GdatPitCropProvenanceReceipt *);
int dm2_v1_gdat_pit_m11_receipt_build(const DM2_V1_AssetLoader *loader,
                                      uint8_t graphicsset,
                                      uint8_t view_cell,
                                      uint16_t tile_state_word,
                                      uint16_t light_parameter,
                                      DM2_V1_GdatPitM11Receipt *out_receipt);
int dm2_v1_gdat_pit_raw4_receipt_build(const DM2_V1_AssetLoader *loader,
                                       const DM2_V1_GdatPitM11Receipt *material,
                                       DM2_V1_GdatPitRaw4Receipt *out_receipt);
int dm2_v1_gdat_pit_material_handoff_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    DM2_V1_GdatPitMaterialHandoffReceipt *out_receipt);
int dm2_v1_gdat_pit_m11_composition_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_GdatPitM11CompositionReceipt *out_receipt);
int dm2_v1_gdat_pit_m11_composition_receipt_matches(
    const DM2_V1_GdatPitM11CompositionReceipt *receipt,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition);
int dm2_v1_gdat_pit_placement_composition_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitM11CompositionReceipt *composition,
    DM2_V1_GdatPitPlacementCompositionReceipt *out_receipt);
int dm2_v1_gdat_pit_placement_composition_receipt_matches(
    const DM2_V1_GdatPitPlacementCompositionReceipt *receipt,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitM11CompositionReceipt *composition);
int dm2_v1_gdat_pit_m11_consume_slot_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitM11CompositionReceipt *pit_composition,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatPitM11ConsumeSlotReceipt *out_receipt);
int dm2_v1_gdat_pit_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell4_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell6_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell7_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell11_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell12_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell14_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell2_hflip_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt);
int dm2_v1_gdat_pit_cell5_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, DM2_V1_GdatPitNormalRowReceipt *);
int dm2_v1_gdat_pit_cell8_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, DM2_V1_GdatPitNormalRowReceipt *);
int dm2_v1_gdat_pit_cell13_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, DM2_V1_GdatPitNormalRowReceipt *);
int dm2_v1_gdat_pit_cell15_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, DM2_V1_GdatPitNormalRowReceipt *);
int dm2_v1_gdat_pit_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell4_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell6_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell7_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell11_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell12_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell14_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell2_hflip_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *hflip_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt);
int dm2_v1_gdat_pit_cell5_hflip_b073_receipt_build(const DM2_V1_AssetLoader *, const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, DM2_V1_GdatPitB073Receipt *);
int dm2_v1_gdat_pit_cell8_hflip_b073_receipt_build(const DM2_V1_AssetLoader *, const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, DM2_V1_GdatPitB073Receipt *);
int dm2_v1_gdat_pit_cell13_hflip_b073_receipt_build(const DM2_V1_AssetLoader *, const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, DM2_V1_GdatPitB073Receipt *);
int dm2_v1_gdat_pit_cell15_hflip_b073_receipt_build(const DM2_V1_AssetLoader *, const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, DM2_V1_GdatPitB073Receipt *);
int dm2_v1_gdat_pit_consume_cell1_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell4_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell6_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell7_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell11_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell12_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell14_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell2_hflip(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *hflip_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner);
int dm2_v1_gdat_pit_consume_cell5_hflip(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitMaterialHandoffReceipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, const DM2_V1_GdatPitB073Receipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, const DM2_V1_Dm2ViewportM11CompositionReceipt *, const DM2_V1_ViewportState *);
int dm2_v1_gdat_pit_consume_cell8_hflip(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitMaterialHandoffReceipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, const DM2_V1_GdatPitB073Receipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, const DM2_V1_Dm2ViewportM11CompositionReceipt *, const DM2_V1_ViewportState *);
int dm2_v1_gdat_pit_consume_cell13_hflip(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitMaterialHandoffReceipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, const DM2_V1_GdatPitB073Receipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, const DM2_V1_Dm2ViewportM11CompositionReceipt *, const DM2_V1_ViewportState *);
int dm2_v1_gdat_pit_consume_cell15_hflip(const DM2_V1_GdatPitM11Receipt *, const DM2_V1_GdatPitMaterialHandoffReceipt *, const DM2_V1_GdatPitRaw4Receipt *, const DM2_V1_GdatPitPlacementCompositionReceipt *, const DM2_V1_GdatPitNormalRowReceipt *, const DM2_V1_GdatPitB073Receipt *, const DM2_V1_GdatPitM11ConsumeSlotReceipt *, const DM2_V1_Dm2ViewportM11CompositionReceipt *, const DM2_V1_ViewportState *);

#endif
