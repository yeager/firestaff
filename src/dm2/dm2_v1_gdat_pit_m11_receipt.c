#include "dm2_v1_gdat_pit_m11_receipt.h"

#include <string.h>

static const uint16_t s_pit_rect[16] = {
    0x35e, 0x35d, 0x35f, 0x35b, 0x35a, 0x35c, 0x358, 0x357,
    0x359, 0xffff, 0xffff, 0x355, 0x354, 0x356, 0x352, 0x353
};
static const uint8_t s_pit_flip[16] = {
    0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1
};
static const uint8_t s_pit_open_field[16] = {
    0x6b, 0x6c, 0x6c, 0x6e, 0x6f, 0x6f, 0x71, 0x72,
    0x72, 0xff, 0xff, 0x76, 0x77, 0x77, 0x79, 0x79
};
static const uint8_t s_pit_closed_field[16] = {
    0x82, 0x83, 0x83, 0x85, 0x86, 0x86, 0x88, 0x89,
    0x89, 0xff, 0xff, 0x76, 0x77, 0x77, 0x79, 0x79
};

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

int dm2_v1_gdat_pit_crop_provenance_intake(const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4, DM2_V1_GdatPitCropProvenanceReceipt *out)
{
    uint32_t hash = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* SKWIN c_gui_vp.cpp:251-291 -> c_image.cpp:229-260: QUERY_BLIT_RECT may
     * mutate source coordinates. Root RAW4 proves neither crop nor chaining. */
    if (!material || !raw4 || !material->valid || !raw4->valid ||
        material->transform.rect_number != raw4->rect_number ||
        raw4->material_identity_hash != material->identity_hash) return 0;
    out->valid = 1; out->no_draw = 1; out->view_cell = material->transform.view_cell;
    out->query1 = material->transform.rect_number; out->raw4_identity_hash = raw4->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out->view_cell, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out->query1, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out->raw4_identity_hash, 4u);
    out->identity_hash = hash ? hash : 1u;
    return 1;
}

static const uint8_t *find_raw4_row(const uint8_t *table, size_t size,
                                    uint16_t wanted)
{
    uint16_t groups;
    size_t offset;
    uint16_t group;
    if (!table || size < 4u || read_le16(table) != 0xfc0du) return NULL;
    groups = read_le16(table + 2u);
    if (!groups || (size_t)groups > (size - 4u) / 4u) return NULL;
    offset = 4u + (size_t)groups * 4u;
    for (group = 0u; group < groups; ++group) {
        uint16_t first = read_le16(table + 4u + (size_t)group * 4u);
        uint16_t last = read_le16(table + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;
        if (!count || count > (size - offset) / 8u) return NULL;
        if (wanted >= first && wanted <= last)
            return table + offset + (size_t)(wanted - first) * 8u;
        offset += count * 8u;
    }
    return NULL;
}

int dm2_v1_gdat_pit_transform_receipt(uint8_t view_cell,
                                       uint16_t tile_state_word,
                                       uint16_t light_parameter,
                                       DM2_V1_GdatPitTransformReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    uint8_t field;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gui_vp.cpp:4806-4856; dm2data.cpp:776-800.  The caller's
     * word is precisely ptr1e1044 + view_cell * 18 + 8. */
    if (view_cell == 0u || view_cell >= 16u || light_parameter > 640u ||
        s_pit_rect[view_cell] == 0xffffu) return 0;
    field = tile_state_word == 0u ? s_pit_open_field[view_cell] :
        s_pit_closed_field[view_cell];
    if (field == 0xffu) return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->view_cell = view_cell;
    out_receipt->gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    out_receipt->field = field;
    out_receipt->rect_number = s_pit_rect[view_cell];
    out_receipt->light_parameter = light_parameter;
    out_receipt->mirror_flip = s_pit_flip[view_cell];
    out_receipt->state_word_nonzero = tile_state_word != 0u;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->view_cell, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->field, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->rect_number,
                      sizeof(out_receipt->rect_number));
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->light_parameter,
                      sizeof(out_receipt->light_parameter));
    hash = hash_bytes(hash, &out_receipt->mirror_flip, 1u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_m11_receipt_build(const DM2_V1_AssetLoader *loader,
                                      uint8_t graphicsset,
                                      uint8_t view_cell,
                                      uint16_t tile_state_word,
                                      uint16_t light_parameter,
                                      DM2_V1_GdatPitM11Receipt *out_receipt)
{
    DM2_V1_GdatPitM11Receipt candidate;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !dm2_v1_gdat_pit_transform_receipt(
            view_cell, tile_state_word, light_parameter, &candidate.transform))
        return 0;
    candidate.transform.graphicsset = graphicsset;
    if (!dm2_v1_query_gdat_summary_image_receipt(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
            candidate.transform.field, &candidate.summary) ||
        !candidate.summary.accepted || candidate.summary.gdat_bypassed_for_ff ||
        candidate.summary.colors != 16u || !candidate.summary.palette_hash ||
        !dm2_v1_gdat_image_raw_material_receipt(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
            candidate.transform.field, &candidate.raw_material) ||
        !candidate.raw_material.receipt_hash || !candidate.raw_material.source_bytes ||
        candidate.raw_material.source_byte_count == 0u) return 0;
    pixels = dm2_v1_asset_load_image_field(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
        graphicsset, candidate.transform.field, &width, &height, &candidate.format);
    if (!pixels || width <= 0 || height <= 0 || candidate.format != DM2_IMG_FMT_U4 ||
        (size_t)width > SIZE_MAX / (size_t)height) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    candidate.width = (uint16_t)width;
    candidate.height = (uint16_t)height;
    candidate.pixel_stride = (uint16_t)width;
    candidate.indexed_pixels = pixels;
    candidate.indexed_pixel_count = (uint32_t)((size_t)width * (size_t)height);
    memcpy(candidate.palette16, candidate.summary.palette16, 16u);
    candidate.palette_hash = hash_bytes(2166136261u, candidate.palette16, 16u);
    candidate.decoded_hash = hash_bytes(2166136261u, pixels,
                                        (size_t)width * (size_t)height);
    if (!candidate.decoded_hash || candidate.palette_hash == 0u ||
        candidate.palette_hash != candidate.summary.palette_hash) return 0;
    hash = hash_bytes(hash, (const uint8_t *)&candidate.transform.identity_hash,
                      sizeof(candidate.transform.identity_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.summary.receipt_hash,
                      sizeof(candidate.summary.receipt_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.raw_material.receipt_hash,
                      sizeof(candidate.raw_material.receipt_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.decoded_hash,
                      sizeof(candidate.decoded_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.palette_hash,
                      sizeof(candidate.palette_hash));
    candidate.valid = 1;
    candidate.no_draw = 1;
    candidate.identity_hash = hash ? hash : 1u;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_gdat_pit_material_handoff_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    DM2_V1_GdatPitMaterialHandoffReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!material || !material->valid || !material->no_draw ||
        !material->indexed_pixels || !material->width || !material->height ||
        material->pixel_stride != material->width ||
        material->indexed_pixel_count != (uint32_t)material->width * material->height ||
        !material->palette_hash || !material->identity_hash) return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->indexed_pixels = material->indexed_pixels;
    out_receipt->width = material->width;
    out_receipt->height = material->height;
    out_receipt->pixel_stride = material->pixel_stride;
    out_receipt->indexed_pixel_count = material->indexed_pixel_count;
    out_receipt->palette_hash = material->palette_hash;
    out_receipt->material_identity_hash = material->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->width, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->height, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->pixel_stride, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->indexed_pixel_count, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->palette_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_raw4_receipt_build(const DM2_V1_AssetLoader *loader,
                                       const DM2_V1_GdatPitM11Receipt *material,
                                       DM2_V1_GdatPitRaw4Receipt *out_receipt)
{
    const uint8_t *table;
    const uint8_t *row;
    size_t size = 0u;
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_gui_vp.cpp:251-291 passes table1d6c70[cell] unchanged as query1;
     * c_image.cpp:229-337 and c_xrect.cpp:217-474 then resolve it.  Admit
     * only the complete root row: no chained rectangle, crop or global clip. */
    if (!loader || !material || !material->valid || !material->no_draw ||
        !material->identity_hash || !material->width || !material->height)
        return 0;
    table = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0, DM2_GDAT_ENTRY_TYPE_RAW4,
        0, &size);
    row = find_raw4_row(table, size, material->transform.rect_number);
    if (!table || !row || read_le16(row) != 1u || read_le16(row + 2u) != 0u)
        return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->rect_number = material->transform.rect_number;
    out_receipt->destination_x = (int16_t)read_le16(row + 4u);
    out_receipt->destination_y = (int16_t)read_le16(row + 6u);
    out_receipt->width = material->width;
    out_receipt->height = material->height;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->table_hash = hash_bytes(2166136261u, table, size);
    out_receipt->row_hash = hash_bytes(2166136261u, row, 8u);
    if (!out_receipt->table_hash || !out_receipt->row_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt)); return 0;
    }
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->rect_number, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->table_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->row_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_m11_composition_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_GdatPitM11CompositionReceipt *out_receipt)
{
    DM2_V1_GdatPitM11CompositionReceipt candidate;
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gui_vp.cpp:234-292 selects the pit material and immediately
     * delegates to DRAW_DUNGEON_GRAPHIC.  No current evidence identifies a
     * per-cell QUERY_BLIT_RECT destination, so this attaches the material to
     * one live composition transaction but deliberately grants no draw slot. */
    if (!material || !composition || !material->valid || !material->no_draw ||
        !material->identity_hash || !material->transform.identity_hash ||
        !material->summary.receipt_hash || !material->raw_material.receipt_hash ||
        !composition->valid || !composition->no_draw ||
        !composition->identity_hash || !composition->ordered_member_hash ||
        !composition->session_identity || !composition->data_epoch) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate.valid = 1;
    candidate.no_draw = 1;
    candidate.source_order_unresolved = 1u;
    candidate.pit_material_identity_hash = material->identity_hash;
    candidate.pit_transform_identity_hash = material->transform.identity_hash;
    candidate.summary_receipt_hash = material->summary.receipt_hash;
    candidate.raw_material_receipt_hash = material->raw_material.receipt_hash;
    candidate.session_identity = composition->session_identity;
    candidate.data_epoch = composition->data_epoch;
    candidate.parent_composition_identity_hash = composition->identity_hash;
    candidate.parent_ordered_member_hash = composition->ordered_member_hash;
    hash = hash_bytes(hash, (const uint8_t *)&candidate.pit_material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.pit_transform_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.summary_receipt_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.raw_material_receipt_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.session_identity, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.data_epoch, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.parent_composition_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.parent_ordered_member_hash, 4u);
    candidate.identity_hash = hash ? hash : 1u;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_gdat_pit_m11_composition_receipt_matches(
    const DM2_V1_GdatPitM11CompositionReceipt *receipt,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition)
{
    DM2_V1_GdatPitM11CompositionReceipt candidate;
    return receipt && receipt->valid && receipt->no_draw &&
        receipt->source_order_unresolved == 1u &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(
            material, composition, &candidate) &&
        candidate.identity_hash == receipt->identity_hash;
}

int dm2_v1_gdat_pit_placement_composition_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitM11CompositionReceipt *composition,
    DM2_V1_GdatPitPlacementCompositionReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!material || !raw4 || !composition || !material->valid ||
        !material->no_draw || !raw4->valid || !raw4->no_draw ||
        !composition->valid || !composition->no_draw ||
        composition->source_order_unresolved != 1u ||
        raw4->material_identity_hash != material->identity_hash ||
        raw4->rect_number != material->transform.rect_number ||
        raw4->width != material->width || raw4->height != material->height ||
        !raw4->identity_hash ||
        composition->pit_material_identity_hash != material->identity_hash)
        return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->source_order_unresolved = 1u;
    out_receipt->pit_composition_identity_hash = composition->identity_hash;
    out_receipt->pit_raw4_identity_hash = raw4->identity_hash;
    out_receipt->material_identity_hash = material->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->pit_composition_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->pit_raw4_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_placement_composition_receipt_matches(
    const DM2_V1_GdatPitPlacementCompositionReceipt *receipt,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitM11CompositionReceipt *composition)
{
    DM2_V1_GdatPitPlacementCompositionReceipt candidate;
    return receipt && receipt->valid && receipt->no_draw &&
        receipt->source_order_unresolved == 1u &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(
            material, raw4, composition, &candidate) &&
        candidate.identity_hash == receipt->identity_hash;
}

int dm2_v1_gdat_pit_m11_consume_slot_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitM11CompositionReceipt *pit_composition,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatPitM11ConsumeSlotReceipt *out_receipt)
{
    DM2_V1_ViewportSurfaceSnapshot snapshot;
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!material || !handoff || !pit_composition || !placement ||
        !composition || !owner || !material->valid || !material->no_draw ||
        !handoff->valid || !handoff->no_draw ||
        handoff->indexed_pixels != material->indexed_pixels ||
        handoff->width != material->width || handoff->height != material->height ||
        handoff->pixel_stride != material->pixel_stride ||
        handoff->indexed_pixel_count != material->indexed_pixel_count ||
        handoff->palette_hash != material->palette_hash ||
        handoff->material_identity_hash != material->identity_hash ||
        !pit_composition->valid || !pit_composition->no_draw ||
        pit_composition->source_order_unresolved != 1u ||
        !placement->valid || !placement->no_draw ||
        placement->source_order_unresolved != 1u ||
        placement->pit_composition_identity_hash != pit_composition->identity_hash ||
        placement->material_identity_hash != material->identity_hash ||
        !composition->valid || !composition->no_draw ||
        !composition->identity_hash || !composition->ordered_member_hash ||
        !composition->session_identity || !composition->data_epoch ||
        !dm2_v1_viewport_surface_snapshot(owner, &snapshot) ||
        !snapshot.framebuffer || !snapshot.generation ||
        composition->surface_before.framebuffer != snapshot.framebuffer ||
        composition->surface_after.framebuffer != snapshot.framebuffer ||
        composition->surface_before.width != snapshot.width ||
        composition->surface_after.width != snapshot.width ||
        composition->surface_before.height != snapshot.height ||
        composition->surface_after.height != snapshot.height ||
        composition->surface_before.stride != snapshot.stride ||
        composition->surface_after.stride != snapshot.stride ||
        composition->surface_before.resolution != snapshot.resolution ||
        composition->surface_after.resolution != snapshot.resolution ||
        composition->surface_before.generation != snapshot.generation ||
        composition->surface_after.generation != snapshot.generation)
        return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->normal_blit_unproven = 1u;
    out_receipt->pit_placement_identity_hash = placement->identity_hash;
    out_receipt->pit_buffer_handoff_identity_hash = handoff->identity_hash;
    out_receipt->generic_composition_identity_hash = composition->identity_hash;
    out_receipt->generic_ordered_member_hash = composition->ordered_member_hash;
    out_receipt->surface_generation = snapshot.generation;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->pit_placement_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->pit_buffer_handoff_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->generic_composition_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->generic_ordered_member_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->surface_generation, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

static int pit_normal_row_receipt_build_for_cell(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt,
    uint8_t expected_cell)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_gui_vp.cpp:251-291 passes table1d6c90[cell] as DRAW_DUNGEON_GRAPHIC
     * blitmode.  For cell 1 it is zero; c_image.cpp:229-337 then dispatches
     * c_gfx_blit.cpp:345-549's default row walk: top-to-bottom, left-to-right.
     * B073 palette mutation is still unavailable to PIT_TILE, so this proves
     * order only and deliberately cannot authorize a framebuffer write. */
    if (!material || !raw4 || !placement || !slot || !material->valid || !material->no_draw ||
        !raw4->valid || !raw4->no_draw || !slot->valid || !slot->no_draw ||
        slot->normal_blit_unproven != 1u ||
        material->transform.view_cell != expected_cell ||
        material->transform.mirror_flip != 0u || raw4->rect_number != material->transform.rect_number ||
        raw4->material_identity_hash != material->identity_hash ||
        placement->pit_raw4_identity_hash != raw4->identity_hash ||
        raw4->width != material->width || raw4->height != material->height ||
        !slot->identity_hash) return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->view_cell = material->transform.view_cell;
    out_receipt->blit_mode = 0u;
    out_receipt->alpha_mask = material->transform.light_parameter;
    out_receipt->source_width = material->width;
    out_receipt->source_height = material->height;
    out_receipt->consume_slot_identity_hash = slot->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->consume_slot_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->alpha_mask, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->source_width, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->source_height, 2u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

static int pit_hflip_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt, uint8_t cell)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_gui_vp.cpp:251-291 gives cell 2 table1d6c90's hflip form; DRAW_PICST
     * reaches c_gfx_blit.cpp's reverse-X row walk.  No crop or chained clip. */
    if (!material || !raw4 || !placement || !slot || !material->valid || !material->no_draw ||
        material->transform.view_cell != cell || material->transform.mirror_flip != 1u ||
        !raw4->valid || !raw4->no_draw || !placement->valid || !placement->no_draw ||
        !slot->valid || !slot->no_draw || slot->normal_blit_unproven != 1u ||
        raw4->rect_number != material->transform.rect_number ||
        raw4->material_identity_hash != material->identity_hash ||
        placement->pit_raw4_identity_hash != raw4->identity_hash ||
        raw4->width != material->width || raw4->height != material->height ||
        !slot->identity_hash) return 0;
    out_receipt->valid = 1; out_receipt->no_draw = 1; out_receipt->view_cell = cell;
    out_receipt->blit_mode = 1u; out_receipt->alpha_mask = material->transform.light_parameter;
    out_receipt->source_width = material->width; out_receipt->source_height = material->height;
    out_receipt->consume_slot_identity_hash = slot->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->consume_slot_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->alpha_mask, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->source_width, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->source_height, 2u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    if (!material || (material->transform.view_cell != 1u && material->transform.view_cell != 3u))
        return 0;
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, material->transform.view_cell);
}

int dm2_v1_gdat_pit_cell4_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, 4u);
}

int dm2_v1_gdat_pit_cell6_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, 6u);
}

int dm2_v1_gdat_pit_cell7_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, 7u);
}

int dm2_v1_gdat_pit_cell11_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, 11u);
}

int dm2_v1_gdat_pit_cell12_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, 12u);
}

int dm2_v1_gdat_pit_cell14_normal_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_normal_row_receipt_build_for_cell(material, raw4, placement, slot,
        out_receipt, 14u);
}

int dm2_v1_gdat_pit_cell2_hflip_row_receipt_build(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    DM2_V1_GdatPitNormalRowReceipt *out_receipt)
{
    return pit_hflip_row_receipt_build(material, raw4, placement, slot, out_receipt, 2u);
}
int dm2_v1_gdat_pit_cell5_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, DM2_V1_GdatPitNormalRowReceipt *o) { return pit_hflip_row_receipt_build(m, r, p, s, o, 5u); }
int dm2_v1_gdat_pit_cell8_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, DM2_V1_GdatPitNormalRowReceipt *o) { return pit_hflip_row_receipt_build(m, r, p, s, o, 8u); }
int dm2_v1_gdat_pit_cell13_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, DM2_V1_GdatPitNormalRowReceipt *o) { return pit_hflip_row_receipt_build(m, r, p, s, o, 13u); }
int dm2_v1_gdat_pit_cell15_hflip_row_receipt_build(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, DM2_V1_GdatPitNormalRowReceipt *o) { return pit_hflip_row_receipt_build(m, r, p, s, o, 15u); }

static int pit_b073_receipt_build_for_cell(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt,
    uint8_t expected_cell, uint8_t expected_blit_mode)
{
    const uint8_t *raw7;
    size_t raw7_size = 0u, data_bytes = 0u, offset, lookup_offset;
    uint8_t count;
    uint16_t color;
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_image.cpp:450-475 invokes DM2_query_B073 before DRAW_PICST;
     * c_querydb.cpp:2506-2668 maps each local-palette byte through RAW7's
     * count/left/right/lookup program.  Admit its bounded no-cache path only. */
    if (!loader || !material || !raw4 || !placement || !normal_rows ||
        !material->valid || !material->no_draw ||
        material->transform.view_cell != expected_cell ||
        !raw4->valid || !raw4->no_draw || !placement->valid || !placement->no_draw ||
        !normal_rows->valid || !normal_rows->no_draw || normal_rows->blit_mode != expected_blit_mode ||
        raw4->material_identity_hash != material->identity_hash ||
        placement->pit_raw4_identity_hash != raw4->identity_hash ||
        normal_rows->source_width != material->width || normal_rows->source_height != material->height)
        return 0;
    raw7 = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0, DM2_GDAT_ENTRY_TYPE_RAW7, 2, &raw7_size);
    if (!raw7 || raw7_size < 2u || !(count = raw7[0]) || count > raw7_size - 1u)
        return 0;
    for (color = 0u; color < count; ++color) data_bytes += raw7[1u + color];
    offset = 1u + count;
    if (data_bytes > (raw7_size - offset) / 2u) return 0;
    lookup_offset = offset + data_bytes * 2u;
    for (color = 0u; color < 16u; ++color) {
        uint8_t index = material->palette16[color];
        size_t lookup = lookup_offset + (size_t)index * 2u;
        uint8_t group, subindex, length;
        size_t group_offset = offset, right_offset;
        uint16_t i;
        if (lookup > raw7_size || raw7_size - lookup < 2u) return 0;
        group = raw7[lookup]; subindex = raw7[lookup + 1u];
        if (group >= count) return 0;
        for (i = 0u; i < group; ++i) group_offset += raw7[1u + i];
        length = raw7[1u + group];
        right_offset = offset + data_bytes + (group_offset - offset);
        if (!length || subindex >= length || group_offset + length > offset + data_bytes ||
            right_offset + length > lookup_offset) return 0;
        out_receipt->transformed_palette16[color] = raw7[right_offset + subindex];
    }
    out_receipt->valid = 1; out_receipt->no_draw = 1; out_receipt->colors = 16u;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->raw4_identity_hash = raw4->identity_hash;
    out_receipt->placement_identity_hash = placement->identity_hash;
    out_receipt->normal_row_identity_hash = normal_rows->identity_hash;
    out_receipt->raw7_hash = hash_bytes(2166136261u, raw7, raw7_size);
    out_receipt->transformed_palette_hash = hash_bytes(2166136261u,
        out_receipt->transformed_palette16, 16u);
    if (!out_receipt->raw7_hash || !out_receipt->transformed_palette_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt)); return 0;
    }
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->raw4_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->placement_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->normal_row_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->raw7_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->transformed_palette_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    if (!material || (material->transform.view_cell != 1u && material->transform.view_cell != 3u))
        return 0;
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, material->transform.view_cell, 0u);
}

int dm2_v1_gdat_pit_cell4_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, 4u, 0u);
}

int dm2_v1_gdat_pit_cell6_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, 6u, 0u);
}

int dm2_v1_gdat_pit_cell7_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, 7u, 0u);
}

int dm2_v1_gdat_pit_cell11_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, 11u, 0u);
}

int dm2_v1_gdat_pit_cell12_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, 12u, 0u);
}

int dm2_v1_gdat_pit_cell14_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        normal_rows, out_receipt, 14u, 0u);
}

int dm2_v1_gdat_pit_cell2_hflip_b073_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *hflip_rows,
    DM2_V1_GdatPitB073Receipt *out_receipt)
{
    return pit_b073_receipt_build_for_cell(loader, material, raw4, placement,
        hflip_rows, out_receipt, 2u, 1u);
}
int dm2_v1_gdat_pit_cell5_hflip_b073_receipt_build(const DM2_V1_AssetLoader *l, const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *h, DM2_V1_GdatPitB073Receipt *o) { return pit_b073_receipt_build_for_cell(l, m, r, p, h, o, 5u, 1u); }
int dm2_v1_gdat_pit_cell8_hflip_b073_receipt_build(const DM2_V1_AssetLoader *l, const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *h, DM2_V1_GdatPitB073Receipt *o) { return pit_b073_receipt_build_for_cell(l, m, r, p, h, o, 8u, 1u); }
int dm2_v1_gdat_pit_cell13_hflip_b073_receipt_build(const DM2_V1_AssetLoader *l, const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *h, DM2_V1_GdatPitB073Receipt *o) { return pit_b073_receipt_build_for_cell(l, m, r, p, h, o, 13u, 1u); }
int dm2_v1_gdat_pit_cell15_hflip_b073_receipt_build(const DM2_V1_AssetLoader *l, const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *h, DM2_V1_GdatPitB073Receipt *o) { return pit_b073_receipt_build_for_cell(l, m, r, p, h, o, 15u, 1u); }

static int pit_b073_receipt_identity_matches(const DM2_V1_GdatPitB073Receipt *receipt)
{
    uint32_t hash = 2166136261u;
    if (!receipt || !receipt->identity_hash || receipt->colors != 16u ||
        receipt->transformed_palette_hash != hash_bytes(2166136261u,
            receipt->transformed_palette16, 16u)) return 0;
    hash = hash_bytes(hash, (const uint8_t *)&receipt->material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&receipt->raw4_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&receipt->placement_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&receipt->normal_row_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&receipt->raw7_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&receipt->transformed_palette_hash, 4u);
    return receipt->identity_hash == (hash ? hash : 1u);
}

static int pit_consume_normal_for_cell(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    uint8_t expected_cell)
{
    DM2_V1_ViewportSurfaceSnapshot snapshot;
    uint16_t x, y;
    uint8_t alpha;
    if (!material || !handoff || !raw4 || !placement || !normal_rows || !b073 ||
        !slot || !composition || !owner || !material->valid || !material->no_draw ||
        !handoff->valid || !handoff->no_draw || !raw4->valid || !raw4->no_draw ||
        !placement->valid || !placement->no_draw || !normal_rows->valid ||
        !normal_rows->no_draw || !b073->valid || !b073->no_draw || !slot->valid ||
        !pit_b073_receipt_identity_matches(b073) ||
        !slot->no_draw || slot->normal_blit_unproven != 1u ||
        slot->generic_composition_identity_hash != composition->identity_hash ||
        slot->generic_ordered_member_hash != composition->ordered_member_hash ||
        handoff->indexed_pixels != material->indexed_pixels ||
        handoff->width != material->width || handoff->height != material->height ||
        handoff->pixel_stride != material->pixel_stride ||
        handoff->indexed_pixel_count != material->indexed_pixel_count ||
        handoff->palette_hash != material->palette_hash ||
        handoff->material_identity_hash != material->identity_hash ||
        raw4->identity_hash != placement->pit_raw4_identity_hash ||
        normal_rows->consume_slot_identity_hash != slot->identity_hash ||
        b073->material_identity_hash != material->identity_hash ||
        b073->raw4_identity_hash != raw4->identity_hash ||
        b073->placement_identity_hash != placement->identity_hash ||
        b073->normal_row_identity_hash != normal_rows->identity_hash ||
        normal_rows->blit_mode != 0u ||
        normal_rows->view_cell != expected_cell ||
        normal_rows->source_width != handoff->width ||
        normal_rows->source_height != handoff->height ||
        !dm2_v1_viewport_surface_snapshot(owner, &snapshot) ||
        snapshot.generation != slot->surface_generation ||
        snapshot.framebuffer != composition->surface_before.framebuffer ||
        snapshot.framebuffer != composition->surface_after.framebuffer ||
        snapshot.generation != composition->surface_before.generation ||
        snapshot.generation != composition->surface_after.generation ||
        raw4->destination_x < 0 || raw4->destination_y < 0 ||
        (uint32_t)raw4->destination_x + handoff->width > snapshot.width ||
        (uint32_t)raw4->destination_y + handoff->height > snapshot.height ||
        snapshot.stride < snapshot.width) return 0;
    for (y = 0u; y < handoff->height; ++y)
        for (x = 0u; x < handoff->width; ++x)
            if (handoff->indexed_pixels[(size_t)y * handoff->pixel_stride + x] > 0x0fu)
                return 0;
    alpha = (uint8_t)(normal_rows->alpha_mask & 0x0fu);
    for (y = 0u; y < handoff->height; ++y) {
        uint8_t *dst = snapshot.framebuffer +
            (size_t)(raw4->destination_y + y) * snapshot.stride + raw4->destination_x;
        const uint8_t *src = handoff->indexed_pixels + (size_t)y * handoff->pixel_stride;
        for (x = 0u; x < handoff->width; ++x)
            if (src[x] != alpha) dst[x] = b073->transformed_palette16[src[x]];
    }
    return 1;
}

int dm2_v1_gdat_pit_consume_cell1_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    if (!normal_rows || (normal_rows->view_cell != 1u && normal_rows->view_cell != 3u))
        return 0;
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, normal_rows->view_cell);
}

int dm2_v1_gdat_pit_consume_cell4_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, 4u);
}

int dm2_v1_gdat_pit_consume_cell6_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, 6u);
}

int dm2_v1_gdat_pit_consume_cell7_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, 7u);
}

int dm2_v1_gdat_pit_consume_cell11_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, 11u);
}

int dm2_v1_gdat_pit_consume_cell12_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, 12u);
}

int dm2_v1_gdat_pit_consume_cell14_normal(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *normal_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner)
{
    return pit_consume_normal_for_cell(material, handoff, raw4, placement, normal_rows,
        b073, slot, composition, owner, 14u);
}

static int pit_consume_hflip_for_cell(
    const DM2_V1_GdatPitM11Receipt *material,
    const DM2_V1_GdatPitMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRaw4Receipt *raw4,
    const DM2_V1_GdatPitPlacementCompositionReceipt *placement,
    const DM2_V1_GdatPitNormalRowReceipt *hflip_rows,
    const DM2_V1_GdatPitB073Receipt *b073,
    const DM2_V1_GdatPitM11ConsumeSlotReceipt *slot,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner, uint8_t cell)
{
    DM2_V1_ViewportSurfaceSnapshot snapshot;
    uint16_t x, y;
    uint8_t alpha;
    if (!material || !handoff || !raw4 || !placement || !hflip_rows || !b073 || !slot ||
        !composition || !owner || !material->valid || !material->no_draw ||
        material->transform.view_cell != cell || material->transform.mirror_flip != 1u ||
        !handoff->valid || !handoff->no_draw || handoff->indexed_pixels != material->indexed_pixels ||
        handoff->width != material->width || handoff->height != material->height ||
        handoff->pixel_stride != material->pixel_stride ||
        handoff->indexed_pixel_count != material->indexed_pixel_count ||
        handoff->palette_hash != material->palette_hash || handoff->material_identity_hash != material->identity_hash ||
        !raw4->valid || !raw4->no_draw || !placement->valid || !placement->no_draw ||
        raw4->identity_hash != placement->pit_raw4_identity_hash ||
        !hflip_rows->valid || !hflip_rows->no_draw || hflip_rows->view_cell != cell ||
        hflip_rows->blit_mode != 1u || hflip_rows->consume_slot_identity_hash != slot->identity_hash ||
        hflip_rows->source_width != handoff->width || hflip_rows->source_height != handoff->height ||
        !b073->valid || !b073->no_draw || !pit_b073_receipt_identity_matches(b073) ||
        b073->material_identity_hash != material->identity_hash || b073->raw4_identity_hash != raw4->identity_hash ||
        b073->placement_identity_hash != placement->identity_hash || b073->normal_row_identity_hash != hflip_rows->identity_hash ||
        !slot->valid || !slot->no_draw || slot->normal_blit_unproven != 1u ||
        slot->generic_composition_identity_hash != composition->identity_hash ||
        slot->generic_ordered_member_hash != composition->ordered_member_hash ||
        !dm2_v1_viewport_surface_snapshot(owner, &snapshot) || snapshot.generation != slot->surface_generation ||
        snapshot.framebuffer != composition->surface_before.framebuffer ||
        snapshot.framebuffer != composition->surface_after.framebuffer ||
        snapshot.generation != composition->surface_before.generation ||
        snapshot.generation != composition->surface_after.generation || raw4->destination_x < 0 || raw4->destination_y < 0 ||
        (uint32_t)raw4->destination_x + handoff->width > snapshot.width ||
        (uint32_t)raw4->destination_y + handoff->height > snapshot.height || snapshot.stride < snapshot.width) return 0;
    for (y = 0u; y < handoff->height; ++y)
        for (x = 0u; x < handoff->width; ++x)
            if (handoff->indexed_pixels[(size_t)y * handoff->pixel_stride + x] > 0x0fu) return 0;
    alpha = (uint8_t)(hflip_rows->alpha_mask & 0x0fu);
    for (y = 0u; y < handoff->height; ++y) {
        uint8_t *dst = snapshot.framebuffer + (size_t)(raw4->destination_y + y) * snapshot.stride + raw4->destination_x;
        const uint8_t *src = handoff->indexed_pixels + (size_t)y * handoff->pixel_stride;
        for (x = 0u; x < handoff->width; ++x) {
            uint8_t index = src[handoff->width - 1u - x];
            if (index != alpha) dst[x] = b073->transformed_palette16[index];
        }
    }
    return 1;
}

int dm2_v1_gdat_pit_consume_cell2_hflip(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitMaterialHandoffReceipt *h, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *q, const DM2_V1_GdatPitB073Receipt *b, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, const DM2_V1_Dm2ViewportM11CompositionReceipt *c, const DM2_V1_ViewportState *o) { return pit_consume_hflip_for_cell(m,h,r,p,q,b,s,c,o,2u); }
int dm2_v1_gdat_pit_consume_cell5_hflip(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitMaterialHandoffReceipt *h, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *q, const DM2_V1_GdatPitB073Receipt *b, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, const DM2_V1_Dm2ViewportM11CompositionReceipt *c, const DM2_V1_ViewportState *o) { return pit_consume_hflip_for_cell(m,h,r,p,q,b,s,c,o,5u); }
int dm2_v1_gdat_pit_consume_cell8_hflip(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitMaterialHandoffReceipt *h, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *q, const DM2_V1_GdatPitB073Receipt *b, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, const DM2_V1_Dm2ViewportM11CompositionReceipt *c, const DM2_V1_ViewportState *o) { return pit_consume_hflip_for_cell(m,h,r,p,q,b,s,c,o,8u); }
int dm2_v1_gdat_pit_consume_cell13_hflip(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitMaterialHandoffReceipt *h, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *q, const DM2_V1_GdatPitB073Receipt *b, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, const DM2_V1_Dm2ViewportM11CompositionReceipt *c, const DM2_V1_ViewportState *o) { return pit_consume_hflip_for_cell(m,h,r,p,q,b,s,c,o,13u); }
int dm2_v1_gdat_pit_consume_cell15_hflip(const DM2_V1_GdatPitM11Receipt *m, const DM2_V1_GdatPitMaterialHandoffReceipt *h, const DM2_V1_GdatPitRaw4Receipt *r, const DM2_V1_GdatPitPlacementCompositionReceipt *p, const DM2_V1_GdatPitNormalRowReceipt *q, const DM2_V1_GdatPitB073Receipt *b, const DM2_V1_GdatPitM11ConsumeSlotReceipt *s, const DM2_V1_Dm2ViewportM11CompositionReceipt *c, const DM2_V1_ViewportState *o) { return pit_consume_hflip_for_cell(m,h,r,p,q,b,s,c,o,15u); }
