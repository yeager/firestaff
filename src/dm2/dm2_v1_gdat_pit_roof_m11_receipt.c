#include "dm2_v1_gdat_pit_roof_m11_receipt.h"

#include <string.h>

static const uint16_t s_roof_rect[9] = {
    0x367, 0x366, 0x368, 0x364, 0x363, 0x365, 0x361, 0x360, 0x362
};
static const uint8_t s_roof_field[9] = {
    0x99, 0x9a, 0x9a, 0x9c, 0x9d, 0x9d, 0x9f, 0xa0, 0xa0
};
static const uint8_t s_roof_flip[9] = { 0, 0, 1, 0, 0, 1, 0, 0, 1 };

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
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

int dm2_v1_gdat_pit_roof_transform_receipt(
    uint8_t graphicsset, uint8_t view_cell, uint16_t light_parameter,
    const DM2_V1_GdatPitRoofSourceState *source,
    DM2_V1_GdatPitRoofTransformReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gui_vp.cpp:118-206; dm2data.cpp:814-816.  The source facts
     * are the terminal outputs of v1e12d6, LOCATE_OTHER_LEVEL and the remote
     * map tile read; no local-map projection is accepted here. */
    if (!source || view_cell == 0u || view_cell >= 9u ||
        light_parameter > 640u || !source->roof_enabled ||
        !source->locate_other_level_succeeded || source->remote_tile_type != 2u ||
        !source->remote_tile_bit_08 || !source->source_state_hash) return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->view_cell = view_cell;
    out_receipt->graphicsset = graphicsset;
    out_receipt->field = s_roof_field[view_cell];
    out_receipt->rect_number = s_roof_rect[view_cell];
    out_receipt->light_parameter = light_parameter;
    out_receipt->mirror_flip = s_roof_flip[view_cell];
    out_receipt->source_state_hash = source->source_state_hash;
    hash = hash_bytes(hash, &out_receipt->graphicsset, 1u);
    hash = hash_bytes(hash, &out_receipt->view_cell, 1u);
    hash = hash_bytes(hash, &out_receipt->field, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->rect_number, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->light_parameter, 2u);
    hash = hash_bytes(hash, &out_receipt->mirror_flip, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->source_state_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_m11_receipt_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset, uint8_t view_cell,
    uint16_t light_parameter, const DM2_V1_GdatPitRoofSourceState *source,
    DM2_V1_GdatPitRoofM11Receipt *out_receipt)
{
    DM2_V1_GdatPitRoofM11Receipt candidate;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !dm2_v1_gdat_pit_roof_transform_receipt(
            graphicsset, view_cell, light_parameter, source,
            &candidate.transform) ||
        !dm2_v1_query_gdat_summary_image_receipt(
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
    if (!candidate.decoded_hash || !candidate.palette_hash ||
        candidate.palette_hash != candidate.summary.palette_hash) return 0;
    hash = hash_bytes(hash, (const uint8_t *)&candidate.transform.identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.summary.receipt_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.raw_material.receipt_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.decoded_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&candidate.palette_hash, 4u);
    candidate.valid = 1;
    candidate.no_draw = 1;
    candidate.identity_hash = hash ? hash : 1u;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_gdat_pit_roof_material_handoff_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    DM2_V1_GdatPitRoofMaterialHandoffReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!material || !material->valid || !material->no_draw ||
        !material->indexed_pixels || !material->width || !material->height ||
        material->pixel_stride != material->width || !material->indexed_pixel_count ||
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

int dm2_v1_gdat_pit_roof_b073_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_CLightM11Receipt *light,
    DM2_V1_GdatPitRoofB073Receipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_image.cpp:450-475 calls B073 before DRAW_PICST.  c_querydb.cpp:
     * 2506-2672 reads the live light transaction; retain it as evidence only,
     * never approximate its palette conversion before a pixel consumer. */
    if (!material || !material->valid || !material->no_draw ||
        !material->identity_hash || !material->palette_hash || !light ||
        !light->valid || !light->receipt_hash || light->light_level > 64u)
        return 0;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->light_level = light->light_level;
    out_receipt->alphamask = material->transform.light_parameter;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->c_light_receipt_hash = light->receipt_hash;
    out_receipt->local_palette_hash = material->palette_hash;
    hash = hash_bytes(hash, (const uint8_t *)&material->identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&light->receipt_hash, 4u);
    hash = hash_bytes(hash, &out_receipt->light_level, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->alphamask, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->local_palette_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_raw4_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    DM2_V1_GdatPitRoofRaw4Receipt *out_receipt)
{
    const uint8_t *table;
    const uint8_t *row;
    size_t size = 0u;
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !material || !material->valid || !material->no_draw ||
        !material->identity_hash || material->width == 0u || material->height == 0u)
        return 0;
    table = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &size);
    row = find_raw4_row(table, size, material->transform.rect_number);
    /* Exact c_xrect.cpp:217-468 subset: root mode1=1/mode2=0 has no
     * successor, crop or implicit clip; all richer chains stay unavailable. */
    if (!table || !row || read_le16(row) != 1u || read_le16(row + 2u) != 0u)
        return 0;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->rect_number = material->transform.rect_number;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->destination_x = (int16_t)read_le16(row + 4u);
    out_receipt->destination_y = (int16_t)read_le16(row + 6u);
    out_receipt->width = material->width;
    out_receipt->height = material->height;
    out_receipt->table_hash = hash_bytes(2166136261u, table, size);
    out_receipt->row_hash = hash_bytes(2166136261u, row, 8u);
    if (!out_receipt->table_hash || !out_receipt->row_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt)); return 0;
    }
    hash = hash_bytes(hash, (const uint8_t *)&material->identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->rect_number, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->table_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->row_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_no_draw_admission(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_CLightM11Receipt *light,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4)
{
    return material && light && b073 && raw4 && material->valid &&
        material->no_draw && material->identity_hash != 0u && light->valid &&
        light->receipt_hash != 0u && b073->valid && b073->no_draw &&
        raw4->valid && raw4->no_draw &&
        b073->material_identity_hash == material->identity_hash &&
        raw4->material_identity_hash == material->identity_hash &&
        b073->c_light_receipt_hash == light->receipt_hash &&
        b073->local_palette_hash == material->palette_hash &&
        raw4->rect_number == material->transform.rect_number &&
        raw4->identity_hash != 0u;
}

int dm2_v1_gdat_pit_roof_alpha_blend_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4,
    uint16_t alphamask, uint8_t blit_mode,
    DM2_V1_GdatPitRoofAlphaBlendReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_image.cpp:450-475 supplies alphamask and the pit source flip
     * to DRAW_PICST.  c_gfx_blit.cpp:370-549 admits only the 4bpp-to-8bpp
     * masked normal/mirror paths here: mask is truncated to its low nibble
     * and matching source indices preserve the destination.  B073 palette
     * conversion and destination composition remain intentionally no-draw. */
    if (!material || !b073 || !raw4 || !material->valid || !material->no_draw ||
        !material->identity_hash || material->format != DM2_IMG_FMT_U4 ||
        !material->palette_hash || !b073->valid || !b073->no_draw ||
        !b073->identity_hash || !raw4->valid || !raw4->no_draw ||
        !raw4->identity_hash || b073->material_identity_hash != material->identity_hash ||
        raw4->material_identity_hash != material->identity_hash ||
        b073->local_palette_hash != material->palette_hash ||
        raw4->rect_number != material->transform.rect_number ||
        alphamask != material->transform.light_parameter ||
        b073->alphamask != alphamask || blit_mode > 1u ||
        blit_mode != material->transform.mirror_flip) return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->alphamask = alphamask;
    out_receipt->alpha_index = (uint8_t)(alphamask & 0x0fu);
    out_receipt->blit_mode = blit_mode;
    out_receipt->mirror_flip = material->transform.mirror_flip;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->b073_identity_hash = b073->identity_hash;
    out_receipt->raw4_identity_hash = raw4->identity_hash;
    out_receipt->palette_hash = material->palette_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->b073_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->raw4_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->alphamask, 2u);
    hash = hash_bytes(hash, &out_receipt->blit_mode, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->palette_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_b073_tables_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    DM2_V1_GdatPitRoofB073TablesReceipt *out_receipt)
{
    const uint8_t *raw7;
    size_t raw7_size = 0u;
    size_t table_data_bytes = 0u;
    size_t offset;
    uint8_t table_count;
    uint32_t hash = 2166136261u;
    uint16_t i;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gdatfile.cpp:1919-2003 loads INTERFACE_GENERAL/0/dt07/2.
     * Its first byte gives the number of nine-byte B073 table descriptors;
     * the following count bytes size both packed table regions, with the
     * remaining bytes becoming v1e0210.  Retain the exact raw program only;
     * c_querydb.cpp:2506-2668 palette mutation remains unavailable. */
    if (!loader || !material || !b073 || !material->valid || !material->no_draw ||
        !material->identity_hash || !b073->valid || !b073->no_draw ||
        !b073->identity_hash || b073->material_identity_hash != material->identity_hash)
        return 0;
    raw7 = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW7, 2, &raw7_size);
    if (!raw7 || raw7_size < 2u || raw7[0] == 0u) return 0;
    table_count = raw7[0];
    if ((size_t)table_count > raw7_size - 1u) return 0;
    for (i = 0u; i < table_count; ++i) {
        size_t length = raw7[1u + i];
        if (length > SIZE_MAX - table_data_bytes) return 0;
        table_data_bytes += length;
    }
    offset = 1u + (size_t)table_count;
    if (table_data_bytes > (raw7_size - offset) / 2u) return 0;
    offset += table_data_bytes * 2u;
    /* B073 reads two bytes at a color-derived v1e0210 offset. */
    if (raw7_size - offset < 2u || table_data_bytes > UINT16_MAX ||
        raw7_size - offset > UINT16_MAX) return 0;
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->table_count = table_count;
    out_receipt->table_data_bytes = (uint16_t)table_data_bytes;
    out_receipt->color_lookup_bytes = (uint16_t)(raw7_size - offset);
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->b073_identity_hash = b073->identity_hash;
    out_receipt->raw7_hash = hash_bytes(2166136261u, raw7, raw7_size);
    if (!out_receipt->raw7_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt)); return 0;
    }
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->b073_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->raw7_hash, 4u);
    hash = hash_bytes(hash, &out_receipt->table_count, 1u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->table_data_bytes, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->color_lookup_bytes, 2u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_b073_traversal_receipt_build(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofB073Receipt *b073,
    const DM2_V1_GdatPitRoofB073TablesReceipt *tables,
    DM2_V1_GdatPitRoofB073TraversalReceipt *out_receipt)
{
    const uint8_t *raw7;
    size_t raw7_size = 0u, data_bytes = 0u, offset, lookup_offset;
    uint8_t count;
    uint16_t color;
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_querydb.cpp:2506-2668, cache-free vl_08 == 0 path.  Each
     * palette byte indexes v1e0210, selects one v1e020c group and interval,
     * then moves away from ebx's alpha index when necessary. */
    if (!loader || !material || !b073 || !tables || !material->valid ||
        !material->no_draw || material->format != DM2_IMG_FMT_U4 ||
        !material->identity_hash || !b073->valid || !b073->no_draw ||
        !b073->identity_hash || b073->alphamask != material->transform.light_parameter ||
        !tables->valid || !tables->no_draw || !tables->identity_hash ||
        tables->material_identity_hash != material->identity_hash ||
        tables->b073_identity_hash != b073->identity_hash) return 0;
    raw7 = dm2_v1_asset_load_typed_sized(loader,
        DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0, DM2_GDAT_ENTRY_TYPE_RAW7,
        2, &raw7_size);
    if (!raw7 || raw7_size < 2u || raw7[0] == 0u ||
        hash_bytes(2166136261u, raw7, raw7_size) != tables->raw7_hash) return 0;
    count = raw7[0];
    if ((size_t)count > raw7_size - 1u) return 0;
    for (color = 0u; color < count; ++color) data_bytes += raw7[1u + color];
    offset = 1u + (size_t)count;
    if (data_bytes > (raw7_size - offset) / 2u) return 0;
    lookup_offset = offset + data_bytes * 2u;
    for (color = 0u; color < 16u; ++color) {
        uint8_t palette_index = material->palette16[color];
        uint8_t group, subindex, length, selected, value;
        size_t group_offset, left_base, right_base, lookup_index;
        uint16_t i;
        lookup_index = lookup_offset + (size_t)palette_index * 2u;
        if (lookup_index > raw7_size || raw7_size - lookup_index < 2u) return 0;
        group = raw7[lookup_index]; subindex = raw7[lookup_index + 1u];
        if (group >= count) return 0;
        group_offset = offset;
        for (i = 0u; i < group; ++i) group_offset += raw7[1u + i];
        length = raw7[1u + group];
        if (!length || subindex >= length || group_offset + length > raw7_size) return 0;
        left_base = group_offset;
        right_base = offset + data_bytes + (group_offset - offset);
        if (right_base + length > lookup_offset) return 0;
        selected = 0u;
        for (i = 0u; i + 1u < length; ++i) {
            uint8_t low = raw7[left_base + i];
            uint8_t high = raw7[left_base + i + 1u];
            if (color >= low && color <= high) { selected = (uint8_t)i; break; }
            selected = (uint8_t)(i + 1u);
        }
        value = raw7[right_base + selected];
        if (value == (uint8_t)b073->alphamask) {
            int left = (int)selected - 1, right = (int)selected + 1;
            do {
                if (left < 0) selected = (uint8_t)right++;
                else if (right >= length) selected = (uint8_t)left--;
                else if ((int)color - raw7[left_base + left] >=
                         (int)raw7[left_base + right] - (int)color)
                    selected = (uint8_t)left--;
                else selected = (uint8_t)right++;
                if (selected >= length) return 0;
                value = raw7[right_base + selected];
            } while (value == (uint8_t)b073->alphamask);
        }
        out_receipt->transformed_palette16[color] = value;
    }
    out_receipt->valid = 1; out_receipt->no_draw = 1; out_receipt->colors = 16u;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->b073_identity_hash = b073->identity_hash;
    out_receipt->tables_identity_hash = tables->identity_hash;
    out_receipt->input_palette_hash = material->palette_hash;
    out_receipt->transformed_palette_hash = hash_bytes(2166136261u,
        out_receipt->transformed_palette16, 16u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->tables_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->transformed_palette_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return out_receipt->transformed_palette_hash != 0u;
}

int dm2_v1_gdat_pit_roof_destination_receipt_build(
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4,
    const DM2_V1_GdatPitRoofAlphaBlendReceipt *alpha_blend,
    const DM2_V1_GdatPitRoofB073TraversalReceipt *traversal,
    uint16_t scale_x, uint16_t scale_y, int16_t source_x, int16_t source_y,
    uint8_t blit_mode, DM2_V1_GdatPitRoofDestinationReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_image.cpp:98-226 and :229-410: normal scale leaves source
     * dimensions intact; DRAW_PICST resolves query1 through QUERY_BLIT_RECT
     * before its mask-aware blit. This admits only the root RAW4 rectangle,
     * zero source crop, normal 0x40 scale, and source-proven H mirror. */
    if (!material || !raw4 || !alpha_blend || !traversal || !material->valid ||
        !material->no_draw || !raw4->valid || !raw4->no_draw ||
        !alpha_blend->valid || !alpha_blend->no_draw || !traversal->valid ||
        !traversal->no_draw || scale_x != 0x40u || scale_y != 0x40u ||
        source_x != 0 || source_y != 0 || blit_mode > 1u ||
        blit_mode != material->transform.mirror_flip ||
        raw4->material_identity_hash != material->identity_hash ||
        alpha_blend->material_identity_hash != material->identity_hash ||
        alpha_blend->raw4_identity_hash != raw4->identity_hash ||
        traversal->material_identity_hash != material->identity_hash ||
        alpha_blend->palette_hash != traversal->input_palette_hash ||
        raw4->width != material->width || raw4->height != material->height)
        return 0;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->destination_x = raw4->destination_x;
    out_receipt->destination_y = raw4->destination_y;
    out_receipt->source_width = material->width;
    out_receipt->source_height = material->height;
    out_receipt->scale_x = scale_x; out_receipt->scale_y = scale_y;
    out_receipt->blit_mode = blit_mode; out_receipt->alpha_index = alpha_blend->alpha_index;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->raw4_identity_hash = raw4->identity_hash;
    out_receipt->traversal_identity_hash = traversal->identity_hash;
    out_receipt->alpha_blend_identity_hash = alpha_blend->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->material_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->raw4_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->traversal_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->alpha_blend_identity_hash, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_surface_clip_receipt_build(
    const DM2_V1_GdatPitRoofDestinationReceipt *destination,
    uint16_t bitmap_width, uint16_t bitmap_height, uint8_t bitmap_resolution,
    uint32_t surface_identity_hash, int16_t clip_x, int16_t clip_y,
    uint16_t clip_width, uint16_t clip_height,
    DM2_V1_GdatPitRoofSurfaceClipReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_xrect.cpp:217-477 derives the final clip from the source bmp
     * dimensions, while c_image.cpp:229-410 takes destination stride and
     * resolution from gfxsys.bitmapptr. Admit only the untrimmed root clip. */
    if (!destination || !destination->valid || !destination->no_draw ||
        !destination->identity_hash || !bitmap_width || !bitmap_height ||
        bitmap_resolution != 8u || !surface_identity_hash || clip_x < 0 ||
        clip_y < 0 || clip_width != destination->source_width ||
        clip_height != destination->source_height ||
        clip_x != destination->destination_x || clip_y != destination->destination_y ||
        (uint32_t)clip_x + clip_width > bitmap_width ||
        (uint32_t)clip_y + clip_height > bitmap_height) return 0;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->bitmap_width = bitmap_width; out_receipt->bitmap_height = bitmap_height;
    out_receipt->bitmap_resolution = bitmap_resolution;
    out_receipt->clip_x = clip_x; out_receipt->clip_y = clip_y;
    out_receipt->clip_width = clip_width; out_receipt->clip_height = clip_height;
    out_receipt->surface_identity_hash = surface_identity_hash;
    out_receipt->destination_identity_hash = destination->identity_hash;
    hash = hash_bytes(hash, (const uint8_t *)&surface_identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&destination->identity_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&clip_x, 2u);
    hash = hash_bytes(hash, (const uint8_t *)&clip_y, 2u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_surface_binding_receipt_build(
    const DM2_V1_GdatPitRoofSurfaceClipReceipt *surface,
    const DM2_V1_ViewportSurfaceSnapshot *snapshot,
    DM2_V1_GdatPitRoofSurfaceBindingReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!surface || !snapshot || !surface->valid || !surface->no_draw ||
        !surface->identity_hash || !snapshot->framebuffer || !snapshot->generation ||
        snapshot->width != surface->bitmap_width || snapshot->height != surface->bitmap_height ||
        snapshot->resolution != surface->bitmap_resolution || snapshot->stride < snapshot->width ||
        snapshot->generation != surface->surface_identity_hash) return 0;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->framebuffer = snapshot->framebuffer;
    out_receipt->surface_receipt_hash = surface->identity_hash;
    out_receipt->generation = snapshot->generation;
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->surface_receipt_hash, 4u);
    hash = hash_bytes(hash, (const uint8_t *)&out_receipt->generation, 4u);
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_gdat_pit_roof_composition_surface_matches(
    const DM2_V1_GdatPitRoofSurfaceBindingReceipt *binding,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofMaterialHandoffReceipt *handoff)
{
    return binding && composition && material && handoff && binding->valid && binding->no_draw &&
        material->valid && material->no_draw && material->indexed_pixels &&
        material->indexed_pixel_count == (uint32_t)material->width * material->height &&
        material->pixel_stride == material->width && material->palette_hash &&
        material->identity_hash && handoff->valid && handoff->no_draw &&
        handoff->indexed_pixels == material->indexed_pixels &&
        handoff->width == material->width && handoff->height == material->height &&
        handoff->pixel_stride == material->pixel_stride &&
        handoff->indexed_pixel_count == material->indexed_pixel_count &&
        handoff->palette_hash == material->palette_hash &&
        handoff->material_identity_hash == material->identity_hash && handoff->identity_hash &&
        composition->valid && composition->no_draw && composition->identity_hash &&
        composition->ordered_member_hash != 0u &&
        composition->session_identity != 0u && composition->data_epoch != 0u &&
        composition->surface_before.generation == binding->generation &&
        composition->surface_after.generation == binding->generation &&
        composition->surface_before.framebuffer == binding->framebuffer &&
        composition->surface_after.framebuffer == binding->framebuffer &&
        composition->surface_generation_hash != 0u;
}

int dm2_v1_gdat_pit_roof_consume_ordered_m11(
    const DM2_V1_GdatPitRoofSurfaceBindingReceipt *binding,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_GdatPitRoofM11Receipt *material,
    const DM2_V1_GdatPitRoofMaterialHandoffReceipt *handoff,
    const DM2_V1_GdatPitRoofRaw4Receipt *raw4,
    const DM2_V1_GdatPitRoofAlphaBlendReceipt *alpha_blend,
    const DM2_V1_GdatPitRoofB073TraversalReceipt *traversal,
    const DM2_V1_GdatPitRoofDestinationReceipt *destination,
    const DM2_V1_GdatPitRoofSurfaceClipReceipt *surface)
{
    uint16_t y;
    /* SKWIN c_image.cpp:229-337 resolves DRAW_PICST's already queried
     * rectangle and destination bitmap before dispatching the blitter.
     * c_gfx_blit.cpp:345-549 then performs the masked 4bpp-to-8bpp row
     * conversion, with BLITMODE1 reversing only X.  This bounded consumer
     * accepts exactly the normal-scale root rectangle proven above; it uses
     * the authenticated handoff bytes directly and never asks GDAT to decode
     * or reload an image. */
    if (!dm2_v1_gdat_pit_roof_composition_surface_matches(
            binding, composition, material, handoff) ||
        !composition->m11_delivery_ready || !raw4 || !alpha_blend ||
        !traversal || !destination || !surface || !raw4->valid ||
        !raw4->no_draw || !alpha_blend->valid || !alpha_blend->no_draw ||
        !traversal->valid || !traversal->no_draw || !destination->valid ||
        !destination->no_draw || !surface->valid || !surface->no_draw ||
        destination->scale_x != 0x40u || destination->scale_y != 0x40u ||
        destination->source_width != handoff->width ||
        destination->source_height != handoff->height ||
        destination->blit_mode > 1u ||
        destination->blit_mode != alpha_blend->blit_mode ||
        destination->alpha_index != alpha_blend->alpha_index ||
        destination->material_identity_hash != handoff->material_identity_hash ||
        destination->raw4_identity_hash != raw4->identity_hash ||
        destination->traversal_identity_hash != traversal->identity_hash ||
        destination->alpha_blend_identity_hash != alpha_blend->identity_hash ||
        raw4->material_identity_hash != handoff->material_identity_hash ||
        alpha_blend->material_identity_hash != handoff->material_identity_hash ||
        alpha_blend->raw4_identity_hash != raw4->identity_hash ||
        alpha_blend->palette_hash != handoff->palette_hash ||
        traversal->material_identity_hash != handoff->material_identity_hash ||
        traversal->input_palette_hash != handoff->palette_hash ||
        surface->destination_identity_hash != destination->identity_hash ||
        surface->bitmap_resolution != 8u ||
        surface->clip_x != destination->destination_x ||
        surface->clip_y != destination->destination_y ||
        surface->clip_width != handoff->width ||
        surface->clip_height != handoff->height ||
        composition->surface_before.width != surface->bitmap_width ||
        composition->surface_after.width != surface->bitmap_width ||
        composition->surface_before.height != surface->bitmap_height ||
        composition->surface_after.height != surface->bitmap_height ||
        composition->surface_before.stride != composition->surface_after.stride ||
        composition->surface_before.stride < surface->bitmap_width ||
        composition->surface_before.resolution != surface->bitmap_resolution ||
        composition->surface_after.resolution != surface->bitmap_resolution)
        return 0;

    for (y = 0u; y < handoff->height; ++y) {
        uint16_t x;
        const uint8_t *src = handoff->indexed_pixels + (size_t)y * handoff->pixel_stride;
        for (x = 0u; x < handoff->width; ++x)
            if (src[x] > 0x0fu) return 0;
    }
    for (y = 0u; y < handoff->height; ++y) {
        uint16_t x;
        uint8_t *dst = binding->framebuffer +
            (size_t)(surface->clip_y + (int16_t)y) * composition->surface_before.stride +
            (size_t)surface->clip_x;
        const uint8_t *src = handoff->indexed_pixels + (size_t)y * handoff->pixel_stride;
        for (x = 0u; x < handoff->width; ++x) {
            uint16_t source_x = destination->blit_mode ?
                (uint16_t)(handoff->width - 1u - x) : x;
            uint8_t index = src[source_x];
            if (index != destination->alpha_index)
                dst[x] = traversal->transformed_palette16[index];
        }
    }
    return 1;
}
