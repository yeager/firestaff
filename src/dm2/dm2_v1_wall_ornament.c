#include "dm2_v1_wall_ornament.h"
#include <string.h>

static uint16_t dm2_wall_ornament_rect_read_u16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int dm2_v1_wall_ornament_rect_table_query(
    const uint8_t *raw_rects, size_t raw_length, uint16_t rect_number,
    DM2_V1_RectTableNode *out)
{
    uint16_t range_count;
    size_t ranges_end;
    size_t cursor;

    if (out) memset(out, 0, sizeof(*out));
    if (!raw_rects || !out || rect_number == 0u || raw_length < 4u ||
        dm2_wall_ornament_rect_read_u16(raw_rects) != 0xfc0du) {
        return 0;
    }
    range_count = dm2_wall_ornament_rect_read_u16(raw_rects + 2u);
    if ((size_t)range_count > (raw_length - 4u) / 4u) return 0;
    ranges_end = 4u + (size_t)range_count * 4u;
    cursor = ranges_end;
    for (uint16_t range = 0u; range < range_count; ++range) {
        size_t range_offset = 4u + (size_t)range * 4u;
        uint16_t first = dm2_wall_ornament_rect_read_u16(
            raw_rects + range_offset);
        uint16_t last = dm2_wall_ornament_rect_read_u16(
            raw_rects + range_offset + 2u);
        size_t count;
        size_t bytes;

        if (last < first) return 0;
        count = (size_t)last - (size_t)first + 1u;
        if (count > (raw_length - cursor) / 8u) return 0;
        bytes = count * 8u;
        if (rect_number >= first && rect_number <= last) {
            size_t node_offset = cursor +
                ((size_t)rect_number - (size_t)first) * 8u;
            out->rect_number = rect_number;
            out->x = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset);
            out->y = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset + 2u);
            out->width = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset + 4u);
            out->height = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset + 6u);
            return 1;
        }
        cursor += bytes;
    }
    return 0;
}

static uint32_t dm2_wall_ornament_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

int dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(
    const DM2_V1_AssetLoader *loader,
    uint8_t index,
    DM2_V1_WallOrnateAlcoveTypeReceipt *out)
{
    uint16_t alcove = 0u;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader ||
        !dm2_v1_asset_load_word_value(
            loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
            DM2_V1_WALL_ORNATE_ALCOVE_TYPE_FIELD, &alcove) ||
        alcove > DM2_V1_WALL_ORNATE_ALCOVE_TYPE_LIMIT) {
        return 0;
    }

    hash = dm2_wall_ornament_hash_step(hash, DM2_GDAT_CATEGORY_WALL_GFX);
    hash = dm2_wall_ornament_hash_step(hash, index);
    hash = dm2_wall_ornament_hash_step(
        hash, DM2_V1_WALL_ORNATE_ALCOVE_TYPE_FIELD);
    hash = dm2_wall_ornament_hash_step(hash, alcove);
    if (hash == 0u) return 0;

    out->valid = 1;
    out->wall_gfx_index = index;
    out->alcove_type = alcove;
    out->source_hash = hash;
    return 1;
}

int dm2_v1_wall_ornament_material_receipt(const DM2_V1_AssetLoader *loader,
                                          uint8_t index,
                                          uint8_t image_field,
                                          DM2_V1_WallOrnamentReceipt *out)
{
    DM2_V1_WallOrnateAlcoveTypeReceipt alcove_receipt;
    uint16_t colorkey, position, no_flip, alcove, displacement;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE: dtWordValue 04/05/07/0A
     * and dtImageOffset FD are all required before QUERY_TEMP_PICST. */
    if (!loader ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
                                      DM2_V1_WALL_ORNATE_COLORKEY_FIELD,
                                      &colorkey) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
                                      DM2_V1_WALL_ORNATE_POSITION_FIELD,
                                      &position) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
                                      DM2_V1_WALL_ORNATE_DO_NOT_FLIP_FIELD,
                                      &no_flip) ||
        !dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(loader, index, &alcove_receipt) ||
        !dm2_v1_asset_load_image_offset(loader, DM2_GDAT_CATEGORY_WALL_GFX,
                                        index,
                                        DM2_V1_WALL_ORNATE_ITEM_DISPLACEMENT_FIELD,
                                        &displacement) ||
        !dm2_v1_asset_load_image_metadata(loader, DM2_GDAT_CATEGORY_WALL_GFX,
                                          index, image_field,
                                          &out->image_metadata) ||
        out->image_metadata.bits_per_pixel != 4u ||
        !dm2_v1_asset_load_image_local_palette(loader,
                                               DM2_GDAT_CATEGORY_WALL_GFX,
                                               index, image_field,
                                               out->local_palette16,
                                               &out->local_palette_hash) ||
                                               out->local_palette_hash == 0u) return 0;
    alcove = alcove_receipt.alcove_type;
    if (position > 24u || alcove > DM2_V1_WALL_ORNATE_ALCOVE_TYPE_LIMIT) {
        return 0;
    }
    pixels = dm2_v1_asset_load_image_field(loader, DM2_GDAT_CATEGORY_WALL_GFX,
                                            index, image_field,
                                            &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        width != (int)out->image_metadata.width ||
        height != (int)out->image_metadata.height ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4)) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count == 0u || pixel_count > UINT32_MAX) {
        dm2_v1_asset_free_pixels(pixels);
        return 0;
    }
    for (size_t i = 0u; i < pixel_count; ++i) {
        hash = dm2_wall_ornament_hash_step(hash, pixels[i]);
    }
    dm2_v1_asset_free_pixels(pixels);
    if (hash == 0u) return 0;
    out->valid = 1; out->wall_gfx_index = index; out->colorkey = colorkey;
    out->position = position; out->do_not_flip = no_flip; out->alcove_type = alcove;
    out->item_inside_displacement = displacement;
    out->image_field = image_field;
    out->decoded_width = (uint16_t)width;
    out->decoded_height = (uint16_t)height;
    out->decoded_format = format;
    out->decoded_pixel_count = (uint32_t)pixel_count;
    out->decoded_pixels_hash = hash;
    hash = dm2_wall_ornament_hash_step(hash, out->image_metadata.metadata_hash);
    hash = dm2_wall_ornament_hash_step(hash, out->local_palette_hash);
    hash = dm2_wall_ornament_hash_step(hash, colorkey);
    hash = dm2_wall_ornament_hash_step(hash, position);
    hash = dm2_wall_ornament_hash_step(hash, no_flip);
    hash = dm2_wall_ornament_hash_step(hash, alcove);
    hash = dm2_wall_ornament_hash_step(hash, displacement);
    hash = dm2_wall_ornament_hash_step(hash, image_field);
    hash = dm2_wall_ornament_hash_step(hash, alcove_receipt.source_hash);
    if (hash == 0u) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->material_hash = hash;
    return 1;
}

int dm2_v1_wall_ornament_receipt(const DM2_V1_AssetLoader *loader,
                                 uint8_t index,
                                 DM2_V1_WallOrnamentReceipt *out)
{
    /* DRAW_WALL_ORNATE uses dtImage 1 for its front-facing normal path.
     * Side/flag-adjusted choices must use the explicit material entry point. */
    return dm2_v1_wall_ornament_material_receipt(
        loader, index, DM2_V1_WALL_ORNATE_FRONT_IMAGE_FIELD, out);
}

const char *dm2_v1_wall_ornament_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp GET_WALL_ORNATE_ALCOVE_TYPE "
           "queries WALL_GFX dtWordValue 0x0A; DRAW_WALL_ORNATE also "
           "requires dtWordValue 0x04/0x05/0x07 and dtImageOffset 0xFD "
           "before QUERY_TEMP_PICST image/local-palette materialization.";
}
