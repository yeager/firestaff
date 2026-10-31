#include "dm2_v1_wall_ornament.h"
#include <limits.h>
#include <string.h>

static uint16_t dm2_wall_ornament_rect_read_u16(const uint8_t *p,
                                                int big_endian)
{
    return big_endian
        ? (uint16_t)(((uint16_t)p[0] << 8) | p[1])
        : (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

int dm2_v1_wall_ornament_rect_number(
    int source_cell, int lateral, unsigned position, uint16_t *out_rect_number)
{
    static const uint16_t side_rect_base[16] = {
        0xffffu, 0x1149u, 0x1162u, 0xffffu,
        0x1194u, 0x11adu, 0xffffu, 0x11dfu,
        0x11f8u, 0x1211u, 0x122au, 0xffffu,
        0x125cu, 0x1275u, 0x128eu, 0x12a7u
    };
    unsigned base;

    if (out_rect_number) *out_rect_number = 0u;
    if (!out_rect_number || source_cell < 0 || source_cell >= 16 ||
        position >= 25u) return 0;
    if (lateral == 0) {
        base = 3100u + (unsigned)source_cell * 25u;
    } else {
        if (side_rect_base[source_cell] == 0xffffu) return 0;
        base = side_rect_base[source_cell];
    }
    if (base + position > UINT16_MAX) return 0;
    *out_rect_number = (uint16_t)(base + position);
    return 1;
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
        dm2_wall_ornament_rect_read_u16(raw_rects, 0) != 0xfc0du) {
        return 0;
    }
    range_count = dm2_wall_ornament_rect_read_u16(raw_rects + 2u, 0);
    if ((size_t)range_count > (raw_length - 4u) / 4u) return 0;
    ranges_end = 4u + (size_t)range_count * 4u;
    cursor = ranges_end;
    for (uint16_t range = 0u; range < range_count; ++range) {
        size_t range_offset = 4u + (size_t)range * 4u;
        uint16_t first = dm2_wall_ornament_rect_read_u16(
            raw_rects + range_offset, 0);
        uint16_t last = dm2_wall_ornament_rect_read_u16(
            raw_rects + range_offset + 2u, 0);
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
                raw_rects + node_offset, 0);
            out->y = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset + 2u, 0);
            out->width = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset + 4u, 0);
            out->height = (int16_t)dm2_wall_ornament_rect_read_u16(
                raw_rects + node_offset + 6u, 0);
            return 1;
        }
        cursor += bytes;
    }
    return 0;
}

static int dm2_v1_wall_ornament_rect_table_query_endian(
    const uint8_t *raw_rects, size_t raw_length, uint16_t rect_number,
    int big_endian, DM2_V1_RectTableNode *out)
{
    uint16_t range_count;
    size_t cursor;

    if (out) memset(out, 0, sizeof(*out));
    if (!raw_rects || !out || !rect_number || raw_length < 4u ||
        dm2_wall_ornament_rect_read_u16(raw_rects, big_endian) != 0xfc0du)
        return 0;
    range_count = dm2_wall_ornament_rect_read_u16(raw_rects + 2u, big_endian);
    if ((size_t)range_count > (raw_length - 4u) / 4u) return 0;
    cursor = 4u + (size_t)range_count * 4u;
    for (uint16_t range = 0u; range < range_count; ++range) {
        size_t range_offset = 4u + (size_t)range * 4u;
        uint16_t first = dm2_wall_ornament_rect_read_u16(
            raw_rects + range_offset, big_endian);
        uint16_t last = dm2_wall_ornament_rect_read_u16(
            raw_rects + range_offset + 2u, big_endian);
        size_t count;
        size_t bytes;
        if (last < first) return 0;
        count = (size_t)last - first + 1u;
        if (count > (raw_length - cursor) / 8u) return 0;
        bytes = count * 8u;
        if (rect_number >= first && rect_number <= last) {
            size_t node = cursor + ((size_t)rect_number - first) * 8u;
            out->rect_number = rect_number;
            out->x = (int16_t)dm2_wall_ornament_rect_read_u16(raw_rects + node, big_endian);
            out->y = (int16_t)dm2_wall_ornament_rect_read_u16(raw_rects + node + 2u, big_endian);
            out->width = (int16_t)dm2_wall_ornament_rect_read_u16(raw_rects + node + 4u, big_endian);
            out->height = (int16_t)dm2_wall_ornament_rect_read_u16(raw_rects + node + 6u, big_endian);
            return 1;
        }
        cursor += bytes;
    }
    return 0;
}

typedef struct {
    int x;
    int y;
    int width;
    int height;
} DM2_WallRect;

static int dm2_wall_rect_query(const uint8_t *raw, size_t length,
                               uint16_t rectno, int big_endian,
                               DM2_WallRect *out)
{
    DM2_V1_RectTableNode node;
    if (!dm2_v1_wall_ornament_rect_table_query_endian(
            raw, length, rectno, big_endian, &node))
        return 0;
    out->x = node.x;
    out->y = node.y;
    out->width = node.width;
    out->height = node.height;
    return 1;
}

static int dm2_wall_anchor_offset(int anchor, int width, int height,
                                  int *x, int *y)
{
    switch (anchor) {
    case 0: *x = -((width + 1) >> 1); *y = -((height + 1) >> 1); return 1;
    case 1: *x = 0; *y = 0; return 1;
    case 2: *x = -(width - 1); *y = 0; return 1;
    case 3: *x = -(width - 1); *y = -(height - 1); return 1;
    case 4: *x = 0; *y = -(height - 1); return 1;
    case 5: *x = -((width + 1) >> 1); *y = 0; return 1;
    case 6: *x = -(width - 1); *y = -((height + 1) >> 1); return 1;
    case 7: *x = -((width + 1) >> 1); *y = -(height - 1); return 1;
    case 8: *x = 0; *y = -((height + 1) >> 1); return 1;
    default: return 0;
    }
}

static int dm2_wall_relative_anchor(int anchor, int width, int height,
                                    int *x, int *y)
{
    switch (anchor) {
    case 0: *x = -((width + 1) >> 1); *y = -((height + 1) >> 1); return 1;
    case 1: *x = 0; *y = 0; return 1;
    case 2: *x = -(width - 1); *y = 0; return 1;
    case 3: *x = -(width - 1); *y = -(height - 1); return 1;
    case 4: *x = 0; *y = -(height - 1); return 1;
    case 5: *x = -((width + 1) >> 1); *y = 0; return 1;
    case 6: *x = -(width - 1); *y = -((height + 1) >> 1); return 1;
    case 7: *x = -((width + 1) >> 1); *y = -(height - 1); return 1;
    case 8: *x = 0; *y = -((height + 1) >> 1); return 1;
    default: return 0;
    }
}

static int dm2_wall_rect_containment_anchor(int anchor, int outer_width,
                                            int outer_height, int inner_width,
                                            int inner_height, int *x, int *y)
{
    switch (anchor) {
    case 0:
        *x = outer_width - ((inner_width + 1) >> 1);
        *y = outer_height - ((inner_height + 1) >> 1); return 1;
    case 1: *x = outer_width; *y = outer_height; return 1;
    case 2: *x = outer_width - (inner_width - 1); *y = outer_height; return 1;
    case 3:
        *x = outer_width - (inner_width - 1);
        *y = outer_height - (inner_height - 1); return 1;
    case 4: *x = outer_width; *y = outer_height - (inner_height - 1); return 1;
    case 5:
        *x = outer_width - ((inner_width + 1) >> 1); *y = outer_height; return 1;
    case 6:
        *x = outer_width - (inner_width - 1);
        /* skproject/SKWINSPX/src/v4/skbltgfx.cpp:1067-1070,
         * QUERY_BLIT_RECT case 6, leaves di at the parent height. */
        *y = inner_height;
        return 1;
    case 7:
        *x = outer_width - ((inner_width + 1) >> 1);
        *y = outer_height - (inner_height - 1); return 1;
    case 8:
        *x = outer_width; *y = outer_height - ((inner_height + 1) >> 1); return 1;
    default: return 0;
    }
}

static int dm2_wall_special_anchor_offset(int node_type, int width, int height,
                                          int *x, int *y)
{
    switch (node_type) {
    case 10: *x = (width + 1) >> 1; *y = (height + 1) >> 1; return 1;
    case 11: *x = 0; *y = 0; return 1;
    case 12: *x = width - 1; *y = 0; return 1;
    case 13: *x = width - 1; *y = height - 1; return 1;
    case 14: *x = 0; *y = height - 1; return 1;
    case 15: *x = (width + 1) >> 1; *y = 0; return 1;
    case 16: *x = width - 1; *y = (height + 1) >> 1; return 1;
    case 17: *x = (width + 1) >> 1; *y = height - 1; return 1;
    case 18: *x = 0; *y = (height + 1) >> 1; return 1;
    default: return 0;
    }
}

int dm2_v1_wall_ornament_query_blit_rect_with_offset(
    const uint8_t *raw_rects, size_t raw_length, uint16_t rect_number,
    int big_endian, int alignment_override, int image_width, int image_height,
    int picture_offset_x, int picture_offset_y,
    DM2_V1_WallOrnamentBlitRect *out)
{
    DM2_WallRect main_rect, clip = { -10000, -10000, 20000, 20000 };
    int alignment, base_x, base_y, deferred_alignment = 0;
    int offset_anchor;
    int output_width = image_width, output_height = image_height;
    uint16_t rectno = rect_number;
    int image_x, image_y, source_x, source_y, clipped_width, clipped_height;

    if (out) memset(out, 0, sizeof(*out));
    if (!out || !raw_rects || image_width <= 0 || image_height <= 0 ||
        picture_offset_x < INT16_MIN || picture_offset_x > INT16_MAX ||
        picture_offset_y < INT16_MIN || picture_offset_y > INT16_MAX ||
        rectno == 0xffffu) return 0;
    offset_anchor = (rectno & 0x8000u) != 0 ||
        picture_offset_x != 0 || picture_offset_y != 0;
    if (picture_offset_x != 0 || picture_offset_y != 0)
        rectno |= 0x8000u;
    if (rectno & 0x8000u) rectno &= 0x7fffu;
    if (!dm2_wall_rect_query(raw_rects, raw_length, rectno, big_endian, &main_rect))
        return 0;
    alignment = alignment_override == -1 ? main_rect.x : alignment_override;
    if (alignment <= 8) {
        base_x = main_rect.width;
        base_y = main_rect.height;
    } else {
        if (alignment == 9) return 0;
        alignment -= 10;
        base_x = 0;
        base_y = 0;
    }
    if (offset_anchor) {
        base_x += picture_offset_x;
        base_y += picture_offset_y;
    }

    for (unsigned step = 0u; main_rect.y != 0; ++step) {
        DM2_WallRect parent;
        int ax, ay;
        if (step >= 64u || main_rect.y < 0 || main_rect.y > UINT16_MAX ||
            !dm2_wall_rect_query(raw_rects, raw_length,
                                 (uint16_t)main_rect.y, big_endian,
                                 &parent)) return 0;
        if (main_rect.x >= 10 && main_rect.x <= 18) {
            DM2_WallRect ancestor;
            int relative_anchor = parent.x;
            int ancestor_id = parent.y;
            int rel_x = parent.width;
            int rel_y = parent.height;
            if (ancestor_id <= 0 || ancestor_id > UINT16_MAX ||
                !dm2_wall_rect_query(raw_rects, raw_length,
                                     (uint16_t)ancestor_id, big_endian,
                                     &ancestor) ||
                !dm2_wall_relative_anchor(relative_anchor,
                                          ancestor.width, ancestor.height,
                                          &ax, &ay)) return 0;
            ax += rel_x;
            ay += rel_y;
            clip.x += ax;
            if (clip.x < ax) clip.x = ax;
            if (ancestor.width + ax <= clip.x + clip.width - 1)
                clip.width = ancestor.width - clip.x + ax;
            clip.y += ay;
            if (clip.y < ay) clip.y = ay;
            if (ancestor.height + ay <= clip.y + clip.height - 1)
                clip.height = ancestor.height - clip.y + ay;
            if (!dm2_wall_special_anchor_offset(main_rect.x,
                                                ancestor.width,
                                                ancestor.height, &ax, &ay))
                return 0;
            base_x += ax + main_rect.width;
            base_y += ay + main_rect.height;
            main_rect = ancestor;
            continue;
        }

        if (parent.x == 1) {
            base_x += parent.width;
            base_y += parent.height;
            clip.x += parent.width;
            clip.y += parent.height;
        } else if (parent.x == 9) {
            if (!dm2_wall_rect_containment_anchor(
                    main_rect.x, main_rect.width, main_rect.height,
                    parent.width, parent.height, &ax, &ay)) return 0;
            if (deferred_alignment) {
                deferred_alignment = 0;
                base_x += ax;
                base_y += ay;
                clip.x += ax;
                clip.y += ay;
            }
            if (clip.x < ax) clip.x = ax;
            if (parent.width + ax <= clip.x + clip.width - 1)
                clip.width = parent.width - clip.x + ax;
            if (clip.y < ay) clip.y = ay;
            if (parent.height + ay <= clip.y + clip.height - 1)
                clip.height = parent.height - clip.y + ay;
        } else if (parent.x <= 8) {
            deferred_alignment = 1;
        }
        main_rect = parent;
    }

    if (!dm2_wall_anchor_offset(alignment, output_width, output_height,
                                &image_x, &image_y)) return 0;
    image_x += base_x;
    image_y += base_y;
    source_x = clip.x - image_x;
    if (source_x <= 0) {
        source_x = 0;
        clipped_width = output_width < clip.width + (clip.x - image_x)
            ? output_width : clip.width + (clip.x - image_x);
        out->destination_x = (int16_t)image_x;
    } else {
        out->destination_x = (int16_t)clip.x;
        clipped_width = output_width - source_x < clip.width
            ? output_width - source_x : clip.width;
    }
    source_y = clip.y - image_y;
    if (source_y <= 0) {
        source_y = 0;
        clipped_height = output_height < clip.height + (clip.y - image_y)
            ? output_height : clip.height + (clip.y - image_y);
        out->destination_y = (int16_t)image_y;
    } else {
        out->destination_y = (int16_t)clip.y;
        clipped_height = output_height - source_y < clip.height
            ? output_height - source_y : clip.height;
    }
    if (clipped_width <= 0 || clipped_height <= 0 ||
        image_x < INT16_MIN || image_x > INT16_MAX ||
        image_y < INT16_MIN || image_y > INT16_MAX ||
        clip.x < INT16_MIN || clip.x > INT16_MAX ||
        clip.y < INT16_MIN || clip.y > INT16_MAX ||
        source_x > INT16_MAX || source_y > INT16_MAX ||
        clipped_width > INT16_MAX || clipped_height > INT16_MAX) return 0;
    out->destination_width = (int16_t)clipped_width;
    out->destination_height = (int16_t)clipped_height;
    out->source_x = (int16_t)source_x;
    out->source_y = (int16_t)source_y;
    out->source_width = (int16_t)clipped_width;
    out->source_height = (int16_t)clipped_height;
    return 1;
}

int dm2_v1_wall_ornament_query_blit_rect(
    const uint8_t *raw_rects, size_t raw_length, uint16_t rect_number,
    int big_endian, int alignment_override, int image_width, int image_height,
    DM2_V1_WallOrnamentBlitRect *out)
{
    /* Keep the legacy high-bit convention: prior callers supplied image
     * dimensions as QUERY_BLIT_RECT's yourcx/yourcy offset values. Runtime
     * wall rendering uses the explicit-offset entry point instead. */
    return dm2_v1_wall_ornament_query_blit_rect_with_offset(
        raw_rects, raw_length, rect_number, big_endian, alignment_override,
        image_width, image_height,
        (rect_number & 0x8000u) ? image_width : 0,
        (rect_number & 0x8000u) ? image_height : 0, out);
}

int dm2_v1_wall_ornament_query_loaded_blit_rect(
    const DM2_V1_AssetLoader *loader, uint16_t rect_number,
    int alignment_override, int image_width, int image_height,
    DM2_V1_WallOrnamentBlitRect *out)
{
    const uint8_t *raw_rects;
    size_t raw_length = 0u;
    if (!loader || !dm2_v1_asset_loader_verify(loader)) return 0;
    raw_rects = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_length);
    if (!raw_rects || !raw_length) return 0;
    return dm2_v1_wall_ornament_query_blit_rect(
        raw_rects, raw_length, rect_number, loader->big_endian,
        alignment_override, image_width, image_height, out);
}

int dm2_v1_wall_ornament_query_loaded_blit_rect_with_offset(
    const DM2_V1_AssetLoader *loader, uint16_t rect_number,
    int alignment_override, int image_width, int image_height,
    int picture_offset_x, int picture_offset_y,
    DM2_V1_WallOrnamentBlitRect *out)
{
    const uint8_t *raw_rects;
    size_t raw_length = 0u;
    if (!loader || !dm2_v1_asset_loader_verify(loader)) return 0;
    raw_rects = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw_length);
    if (!raw_rects || !raw_length) return 0;
    return dm2_v1_wall_ornament_query_blit_rect_with_offset(
        raw_rects, raw_length, rect_number, loader->big_endian,
        alignment_override, image_width, image_height,
        picture_offset_x, picture_offset_y, out);
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
    if (!loader || !loader->loaded || !loader->entries) {
        return 0;
    }
    /* QUERY_GDAT_ENTRY_DATA_INDEX returns zero when dtWordValue 0x0A is
     * absent; GET_WALL_ORNATE_ALCOVE_TYPE passes that zero through. */
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
        DM2_V1_WALL_ORNATE_ALCOVE_TYPE_FIELD, &alcove);
    if (alcove > DM2_V1_WALL_ORNATE_ALCOVE_TYPE_LIMIT) {
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
    uint16_t colorkey = 0u, position = 0u, no_flip = 0u;
    uint16_t alcove, displacement = 0u;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;
    size_t pixel_count;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject SKWIN/SkWinCore.cpp DRAW_WALL_ORNATE reads dtWordValue
     * 04/05/07/0A for ordinary ornaments. dtImageOffset FD is queried only
     * by the Cryocell front-view branch, so it cannot gate normal materials. */
    if (!loader ||
        !dm2_v1_GET_WALL_ORNATE_ALCOVE_TYPE(loader, index, &alcove_receipt) ||
        !dm2_v1_asset_load_image_metadata(loader, DM2_GDAT_CATEGORY_WALL_GFX,
                                          index, image_field,
                                          &out->image_metadata) ||
        (out->image_metadata.bits_per_pixel != 4u &&
         out->image_metadata.bits_per_pixel != 8u) ||
        !dm2_v1_asset_load_image_local_palette(loader,
                                               DM2_GDAT_CATEGORY_WALL_GFX,
                                               index, image_field,
                                               out->local_palette16,
                                               &out->local_palette_hash) ||
                                               out->local_palette_hash == 0u) return 0;
    /* The source data query returns zero when any of these fields are absent.
     * Its draw routine then uses the scene colorkey and centered placement. */
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
        DM2_V1_WALL_ORNATE_COLORKEY_FIELD, &colorkey);
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
        DM2_V1_WALL_ORNATE_POSITION_FIELD, &position);
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
        DM2_V1_WALL_ORNATE_DO_NOT_FLIP_FIELD, &no_flip);
    alcove = alcove_receipt.alcove_type;
    /* Preserve Cryocell placement when present; ordinary ornaments do not
     * carry FD and the source never queries it on their draw path. */
    (void)dm2_v1_asset_load_image_offset(
        loader, DM2_GDAT_CATEGORY_WALL_GFX, index,
        DM2_V1_WALL_ORNATE_ITEM_DISPLACEMENT_FIELD, &displacement);
    if ((position != 0u &&
         ((position & 0xffu) == 0u || (position & 0xffu) > 25u)) ||
        alcove > DM2_V1_WALL_ORNATE_ALCOVE_TYPE_LIMIT) {
        return 0;
    }
    pixels = dm2_v1_asset_load_image_field(loader, DM2_GDAT_CATEGORY_WALL_GFX,
                                            index, image_field,
                                            &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 ||
        width != (int)out->image_metadata.width ||
        height != (int)out->image_metadata.height ||
        (format != DM2_IMG_FMT_IMG3 && format != DM2_IMG_FMT_U4 &&
         format != DM2_IMG_FMT_U8 && format != DM2_IMG_FMT_IMG9)) {
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
           "queries WALL_GFX dtWordValue 0x0A; ordinary DRAW_WALL_ORNATE "
           "reads dtWordValue 0x04/0x05/0x07, while the Cryocell front-view "
           "branch reads dtImageOffset 0xFD. IMG9 uses SUMMARY_IMAGE colors=-1.";
}
