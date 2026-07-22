#include "dm2_v1_gdat_wall_m11_command.h"

#include "dm2_v1_viewport_renderer.h"

#include <limits.h>
#include <string.h>

#define DM2_V1_GDAT_WALL_IMG_HEADER_SIZE 10u
#define DM2_V1_GDAT_WALL_LOCAL_PALETTE_SIZE 16u

/* Exact SKProject dm2data.cpp::table1d6b15, consumed by
 * c_gui_vp.cpp::DM2_DRAW_WALL for moving signed RAW4 queries. */
static const int8_t s_dm2_draw_wall_movement_offsets[23] = {
    0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4
};

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    return hash_bytes(hash, (const uint8_t *)&value, sizeof(value));
}

static int wall_source_raw_index(const DM2_V1_AssetLoader *loader,
                                 const uint8_t *source_bytes,
                                 size_t source_byte_count,
                                 uint16_t *out_raw_index)
{
    uint16_t raw_index;

    if (out_raw_index) *out_raw_index = 0u;
    if (!loader || !loader->data || !loader->raw_offsets ||
        !loader->raw_sizes || !source_bytes || !source_byte_count ||
        !out_raw_index) return 0;
    for (raw_index = 0u; raw_index < loader->raw_data_count; ++raw_index) {
        if (loader->raw_sizes[raw_index] == source_byte_count &&
            loader->data + loader->raw_offsets[raw_index] == source_bytes) {
            *out_raw_index = raw_index;
            return 1;
        }
    }
    return 0;
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM2_V1_WallRawRect;

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int16_t read_le16s(const uint8_t *bytes)
{
    return (int16_t)read_le16(bytes);
}

static int load_graphicsset_wall_local_palette(
    const DM2_V1_AssetLoader *loader, int graphicsset, int field,
    uint8_t out_palette16[16], uint32_t *out_hash)
{
    const uint8_t *raw;
    size_t raw_size = 0u;

    if (out_hash) *out_hash = 0u;
    if (!out_palette16) return 0;
    memset(out_palette16, 0, DM2_V1_GDAT_WALL_LOCAL_PALETTE_SIZE);
    raw = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_ENTRY_TYPE_IMAGE, field, &raw_size);
    if (!raw ||
        raw_size < DM2_V1_GDAT_WALL_IMG_HEADER_SIZE +
            DM2_V1_GDAT_WALL_LOCAL_PALETTE_SIZE) {
        return 0;
    }
    /* DM2_DRAW_WALL consumes QUERY_TEMP_PICST's image-local palette from the
     * source GRAPHICSSET image record. Canonical wall fields include C8/IMG9
     * records that weather/environment helpers intentionally reject, so bind
     * the wall command to the source record's trailing 16-byte palette here. */
    memcpy(out_palette16,
           raw + raw_size - DM2_V1_GDAT_WALL_LOCAL_PALETTE_SIZE,
           DM2_V1_GDAT_WALL_LOCAL_PALETTE_SIZE);
    if (out_hash) {
        *out_hash = hash_bytes(2166136261u, out_palette16,
                               DM2_V1_GDAT_WALL_LOCAL_PALETTE_SIZE);
    }
    return !out_hash || *out_hash != 0u;
}

static const uint8_t *find_raw4_row(const uint8_t *table, size_t table_size,
                                    uint16_t rect_number)
{
    uint16_t groups;
    size_t offset;

    if (!table || table_size < 4u || read_le16(table) != 0xfc0du) return NULL;
    groups = read_le16(table + 2u);
    if (groups == 0u || (size_t)groups > (table_size - 4u) / 4u) return NULL;
    offset = 4u + (size_t)groups * 4u;
    for (uint16_t group = 0u; group < groups; ++group) {
        uint16_t first = read_le16(table + 4u + (size_t)group * 4u);
        uint16_t last = read_le16(table + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;

        if (count == 0u || count > (table_size - offset) / 8u) return NULL;
        if (rect_number >= first && rect_number <= last) {
            return table + offset + (size_t)(rect_number - first) * 8u;
        }
        offset += count * 8u;
    }
    return NULL;
}

/* c_xrect.cpp::DM2_QUERY_RECT expands the compact RAW4 row encoding before
 * DM2_QUERY_BLIT_RECT interprets its chain. */
static int decode_raw4_rect(const uint8_t *table, size_t table_size,
                            uint16_t rect_number, DM2_V1_WallRawRect *out)
{
    const uint8_t *row;
    uint16_t groups;
    size_t offset;
    uint8_t mask = 0x1fu;

    if (!out || !table || table_size < 4u || read_le16(table) != 0xfc0du)
        return 0;
    row = find_raw4_row(table, table_size, rect_number);
    if (!row) return 0;
    groups = read_le16(table + 2u);
    offset = 4u + (size_t)groups * 4u;
    for (uint16_t group = 0u; group < groups; ++group) {
        uint16_t first = read_le16(table + 4u + (size_t)group * 4u);
        uint16_t last = read_le16(table + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;
        if (count == 0u || count > (table_size - offset) / 8u) return 0;
        if (rect_number >= first && rect_number <= last) {
            uint16_t x0 = read_le16(table + offset);
            uint16_t y0 = read_le16(table + offset + 2u);
            for (size_t i = 0u; i < count; ++i) {
                const uint8_t *candidate = table + offset + i * 8u;
                int16_t width = read_le16s(candidate + 4u);
                int16_t height = read_le16s(candidate + 6u);
                if (read_le16(candidate) != x0) mask &= (uint8_t)~0x02u;
                if (read_le16(candidate + 2u) != y0) mask &= (uint8_t)~0x01u;
                if (read_le16(candidate + 2u) > 0xffu) mask &= (uint8_t)~0x04u;
                if (width < 0 || width > 0xff || height < 0 || height > 0xff)
                    mask &= (uint8_t)~0x10u;
                if (width < -128 || width > 127 || height < -128 || height > 127)
                    mask &= (uint8_t)~0x08u;
            }
            if (mask & 0x03u) mask &= (uint8_t)~0x04u;
            out->x = (mask & 0x04u) ? (int)row[0] :
                ((mask & 0x02u) ? (int)(uint8_t)x0 : (int)read_le16s(row));
            out->y = (mask & 0x04u) ? (int)row[2] :
                ((mask & 0x01u) ? (int)(int16_t)y0 :
                 (int)read_le16s(row + 2u));
            if (mask & 0x08u) {
                out->w = (int)(int8_t)row[4]; out->h = (int)(int8_t)row[6];
            } else if (mask & 0x10u) {
                out->w = (int)row[4]; out->h = (int)row[6];
            } else {
                out->w = (int)read_le16s(row + 4u);
                out->h = (int)read_le16s(row + 6u);
            }
            return 1;
        }
        offset += count * 8u;
    }
    return 0;
}

static int decode_anchor(int mode, int x0, int y0, int width, int height,
                         int *out_x, int *out_y)
{
    if (!out_x || !out_y || width <= 0 || height <= 0 || mode < 0 || mode > 8)
        return 0;
    switch (mode) {
    case 0: *out_x = x0 - (width + 1) / 2; *out_y = y0 - (height + 1) / 2; break;
    case 1: *out_x = x0; *out_y = y0; break;
    case 2: *out_x = x0 - width + 1; *out_y = y0; break;
    case 3: *out_x = x0 - width + 1; *out_y = y0 - height + 1; break;
    case 4: *out_x = x0; *out_y = y0 - height + 1; break;
    case 5: *out_x = x0 - (width + 1) / 2; *out_y = y0; break;
    case 6: *out_x = x0 - width + 1; *out_y = y0 - (height + 1) / 2; break;
    case 7: *out_x = x0 - (width + 1) / 2; *out_y = y0 - height + 1; break;
    default: *out_x = x0; *out_y = y0 - (height + 1) / 2; break;
    }
    return 1;
}

/* Exact bounded c_xrect.cpp::DM2_QUERY_BLIT_RECT route for DRAW_WALL.
 * It covers the observed RAW4 chain grammar while preserving source crop
 * offsets. Any unobserved global clip branch remains a no-draw. */
static int query_raw4_wall_blit_rect(
    const uint8_t *table, size_t table_size, uint16_t rect_number,
    int width, int height, int query_offset_x, int query_offset_y,
    DM2_V1_WallRawRect *out_destination, int *out_source_x,
    int *out_source_y)
{
    DM2_V1_WallRawRect current;
    DM2_V1_WallRawRect clip = { -10000, -10000, 20000, 20000 };
    int x0, y0, mode, pending_anchor = 0;
    int source_x = width;
    int source_y = height;
    int signed_rect = query_offset_x != 0 || query_offset_y != 0;

    if (!out_destination || !out_source_x || !out_source_y || width <= 0 ||
        height <= 0 || !decode_raw4_rect(table, table_size, rect_number,
                                         &current) ||
        current.x == 9 || current.x < 0 || current.x > 18) return 0;
    mode = current.x;
    if (mode > 8) { mode -= 10; x0 = 0; y0 = 0; }
    else { x0 = current.w; y0 = current.h; }
    if (signed_rect) {
        x0 += query_offset_x;
        y0 += query_offset_y;
        source_x = 0;
        source_y = 0;
    }
    for (int guard = 0; current.y != 0 && guard < 64; ++guard) {
        DM2_V1_WallRawRect next;
        int nested = current.x >= 10 && current.x <= 18;
        if (!decode_raw4_rect(table, table_size, (uint16_t)current.y, &next))
            return 0;
        if (nested) {
            DM2_V1_WallRawRect leaf;
            int dx = next.w;
            int dy = next.h;

            /* c_xrect.cpp::DM2_QUERY_BLIT_RECT's mode 10..18 path.  The
             * first successor supplies a nested anchor, its successor the
             * clipped leaf rectangle. */
            if (!next.y || !decode_raw4_rect(table, table_size,
                                              (uint16_t)next.y, &leaf) ||
                next.x < 0 || next.x > 8) return 0;
            switch (next.x) {
            case 0: dy -= (leaf.h + 1) / 2; /* fall through */
            case 5: dx -= (leaf.w + 1) / 2; break;
            case 1: break;
            case 3: dy -= leaf.h - 1; /* fall through */
            case 2: dx -= leaf.w - 1; break;
            case 6: dx -= leaf.w - 1; /* fall through */
            case 8: dy -= (leaf.h + 1) / 2; break;
            case 7: dx -= (leaf.w + 1) / 2; /* fall through */
            case 4: dy -= leaf.h - 1; break;
            default: return 0;
            }
            clip.x += dx;
            if (dx > clip.x) clip.x = dx;
            if (leaf.w + dx <= clip.x + clip.w - 1)
                clip.w = leaf.w - clip.x + dx;
            else
                clip.w = leaf.w + dx;
            clip.y += dy;
            if (clip.y < dy) clip.y = dy;
            if (dy + leaf.h <= clip.y + clip.h - 1)
                clip.h = dy + leaf.h - clip.y;
            switch (current.x - 10) {
            case 0: dy += (leaf.h + 1) / 2; /* fall through */
            case 5: dx += (leaf.w + 1) / 2; break;
            case 1: break;
            case 3: dy += leaf.h - 1; /* fall through */
            case 2: dx += leaf.w - 1; break;
            case 6: dx += leaf.w - 1; /* fall through */
            case 8: dy += (leaf.h + 1) / 2; break;
            case 7: dx += (leaf.w + 1) / 2; /* fall through */
            case 4: dy += leaf.h - 1; break;
            default: return 0;
            }
            x0 += dx + current.w;
            y0 += dy + current.h;
            current = leaf;
        } else if (next.x == 1) {
            x0 += next.w;
            y0 += next.h;
            clip.x += next.w;
            clip.y += next.h;
        } else if (next.x == 9) {
            int dx = next.w;
            int dy = next.h;
            if (current.x < 0 || current.x > 8 ||
                !decode_anchor(current.x, current.w, current.h,
                               next.w, next.h, &dx, &dy)) return 0;
            if (pending_anchor) {
                x0 += dx;
                y0 += dy;
                clip.x += dx;
                clip.y += dy;
                pending_anchor = 0;
            }
            if (dx > clip.x) clip.x = dx;
            if (clip.w + clip.x - 1 >= dx + next.w)
                clip.w = next.w - clip.x + dx;
            if (clip.y < dy) clip.y = dy;
            dy += next.h;
            if (clip.y + clip.h - 1 >= dy) clip.h = dy - clip.y;
        } else if (next.x >= 0 && next.x <= 8) {
            pending_anchor = 1;
        } else {
            return 0;
        }
        if (!nested) current = next;
    }
    if (current.y != 0 || !decode_anchor(mode, x0, y0, width, height,
                                          &out_destination->x,
                                          &out_destination->y) ||
        clip.w <= 0 || clip.h <= 0) return 0;
    {
        int dx = clip.x - out_destination->x;
        int dy = clip.y - out_destination->y;
        if (dx > 0) {
            source_x = dx;
            out_destination->x = clip.x;
            out_destination->w = width - dx < clip.w ? width - dx : clip.w;
        } else {
            source_x = 0;
            out_destination->w = width < dx + clip.w ? width : dx + clip.w;
        }
        if (dy > 0) {
            source_y = dy;
            out_destination->y = clip.y;
            out_destination->h = height - dy < clip.h ? height - dy : clip.h;
        } else {
            source_y = 0;
            out_destination->h = height < dy + clip.h ? height : dy + clip.h;
        }
    }
    if (out_destination->x < 0 || out_destination->y < 0 ||
        out_destination->w <= 0 || out_destination->h <= 0 ||
        source_x < 0 || source_y < 0 || source_x + out_destination->w > width ||
        source_y + out_destination->h > height) return 0;
    *out_source_x = source_x;
    *out_source_y = source_y;
    return 1;
}

static int wall_cell_for_square(int square, int *out_cell, int *out_flip)
{
    /* Firestaff's admitted ten-panel route is the same source draw subset
     * that currently owns GRAPHICSSET fields 0x24..0x2d.  The field itself
     * is the only proven bridge to the `iViewportCell + 0x22` RAW4 input. */
    static const uint8_t flips[DM2_SQ_COUNT] = {
        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1
    };
    int field;

    if (!out_cell || !out_flip || square < 0 || square >= DM2_SQ_COUNT)
        return 0;
    field = dm2_v1_viewport_wall_field_for_square(square);
    if (field < 0x22 || field >= 0x40) return 0;
    *out_cell = field - 0x22;
    *out_flip = flips[square];
    return 1;
}

void dm2_v1_gdat_wall_m11_command_plan_free(DM2_V1_GdatWallM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < DM2_V1_GDAT_WALL_M11_COMMAND_MAX; ++i)
        dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

int dm2_v1_gdat_wall_m11_command_plan_build_for_movement(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    int movement_active, DM2_V1_GdatWallM11CommandPlan *out_plan)
{
    DM2_V1_GdatWallM11CommandPlan candidate;
    uint32_t hash = 2166136261u;
    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    memset(&candidate, 0, sizeof(candidate));
    if (!loader || !dm2_v1_asset_loader_verify(loader)) return 0;
    candidate.graphicsset = graphicsset;
    for (int square = 0; square < DM2_SQ_COUNT; ++square) {
        int field = dm2_v1_viewport_wall_field_for_square(square);
        DM2_V1_GdatWallM11Command *command;
        const uint8_t *raw;
        const uint8_t *raw4;
        const uint8_t *raw4_row;
        size_t raw_size = 0u;
        size_t raw4_size = 0u;
        int width = 0, height = 0;
        int cell, mirror_flip;
        int offset_x, offset_y;
        int source_x, source_y;
        uint16_t raw_index;
        DM2_V1_GdatGfxRawMaterialReceipt material;
        DM2_V1_WallRawRect destination;
        DM2_V1_GdatImageMetadata metadata;
        if (field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST) continue;
        /* DM2_DRAW_DUNGEON_TILES schedules only the ten side/deep wall cells;
         * D0C is the front-player tile and D3C has no wall field. */
        if (dm2_v1_viewport_draw_dungeon_tiles_pass_for_square(square) < 0)
            continue;
        if (candidate.command_count >= DM2_V1_GDAT_WALL_M11_COMMAND_MAX) goto fail;
        if (!wall_cell_for_square(square, &cell, &mirror_flip)) goto fail;
        command = &candidate.commands[candidate.command_count];
        raw = dm2_v1_asset_load_sized(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
                                      graphicsset, field, &raw_size);
        command->pixels = dm2_v1_asset_load_image_field(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, field,
            &width, &height, NULL);
        if (!raw || !raw_size || raw_size > UINT32_MAX || !command->pixels ||
            width <= 0 || height <= 0 ||
            !load_graphicsset_wall_local_palette(
                loader, graphicsset, field, command->palette16,
                &command->palette_hash) || !command->palette_hash ||
            !dm2_v1_asset_load_image_metadata(
                loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, field,
                &metadata)) goto fail;
        if (!wall_source_raw_index(loader, raw, raw_size, &raw_index) ||
            !dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
                loader, raw_index, &material) || material.source_bytes != raw ||
            material.source_byte_count != raw_size || !material.receipt_hash) {
            goto fail;
        }
        command->material_raw_index = material.raw_index;
        command->material_source_bytes = material.source_bytes;
        command->material_source_byte_count = material.source_byte_count;
        command->material_receipt_hash = material.receipt_hash;
        raw4 = dm2_v1_asset_load_typed_sized(
            loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
            DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw4_size);
        command->rect_number = (uint16_t)(0x2be + cell);
        raw4_row = find_raw4_row(raw4, raw4_size, command->rect_number);
        offset_x = metadata.query_offset_x;
        offset_y = metadata.query_offset_y;
        command->movement_active = movement_active ? 1u : 0u;
        command->movement_query_offset_y = 0;
        if (command->movement_active) {
            if (cell < 0 ||
                cell >= (int)(sizeof(s_dm2_draw_wall_movement_offsets) /
                               sizeof(s_dm2_draw_wall_movement_offsets[0]))) {
                goto fail;
            }
            command->movement_query_offset_y =
                (int8_t)-s_dm2_draw_wall_movement_offsets[cell];
            offset_y += command->movement_query_offset_y;
        }
        if (mirror_flip) offset_x = -offset_x;
        if (!raw4 || !raw4_size || !raw4_row ||
            !query_raw4_wall_blit_rect(raw4, raw4_size, command->rect_number,
                                       width, height, offset_x, offset_y,
                                       &destination, &source_x, &source_y) ||
            destination.x > INT16_MAX || destination.y > INT16_MAX ||
            destination.w > UINT16_MAX || destination.h > UINT16_MAX ||
            source_x > UINT16_MAX || source_y > UINT16_MAX) goto fail;
        command->view_square = (uint8_t)square;
        command->field = (uint8_t)field;
        command->width = (uint16_t)width;
        command->height = (uint16_t)height;
        command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
        command->decoded_hash = hash_bytes(
            2166136261u, command->pixels, (size_t)width * (size_t)height);
        command->source_x = (uint16_t)source_x;
        command->source_y = (uint16_t)source_y;
        command->source_width = (uint16_t)destination.w;
        command->source_height = (uint16_t)destination.h;
        command->destination_x = (uint16_t)destination.x;
        command->destination_y = (uint16_t)destination.y;
        command->destination_width = (uint16_t)destination.w;
        command->destination_height = (uint16_t)destination.h;
        command->mirror_flip = (uint8_t)mirror_flip;
        command->rect_table_hash = hash_bytes(2166136261u, raw4, raw4_size);
        command->rect_row_hash = hash_bytes(2166136261u, raw4_row, 8u);
        command->metadata_hash = metadata.metadata_hash;
        command->geometry_hash = hash_bytes(
            2166136261u, (const uint8_t *)&command->source_x,
            sizeof(command->source_x) + sizeof(command->source_y) +
                sizeof(command->source_width) + sizeof(command->source_height) +
                sizeof(command->destination_x) + sizeof(command->destination_y) +
                sizeof(command->destination_width) + sizeof(command->destination_height));
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->rect_number);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->mirror_flip);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->movement_active);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          (uint8_t)command->movement_query_offset_y);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->rect_table_hash);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->rect_row_hash);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->metadata_hash);
        command->geometry_hash = hash_u32(command->geometry_hash,
                                          command->material_receipt_hash);
        if (!command->raw_hash || !command->decoded_hash ||
            !command->destination_width || !command->destination_height ||
            !command->rect_table_hash || !command->rect_row_hash ||
            !command->metadata_hash || !command->geometry_hash ||
            !command->material_source_bytes ||
            command->material_source_byte_count != raw_size ||
            !command->material_receipt_hash) goto fail;
        hash = hash_bytes(hash, (const uint8_t *)&command->raw_hash, sizeof(command->raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->decoded_hash,
                          sizeof(command->decoded_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->palette_hash, sizeof(command->palette_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->material_receipt_hash,
                          sizeof(command->material_receipt_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->geometry_hash,
                          sizeof(command->geometry_hash));
        ++candidate.command_count;
    }
    if (!candidate.command_count) goto fail;
    candidate.command_hash = hash ? hash : 1u;
    candidate.valid = 1;
    *out_plan = candidate;
    return 1;
fail:
    dm2_v1_gdat_wall_m11_command_plan_free(&candidate);
    return 0;
}

int dm2_v1_gdat_wall_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatWallM11CommandPlan *out_plan)
{
    return dm2_v1_gdat_wall_m11_command_plan_build_for_movement(
        loader, graphicsset, 0, out_plan);
}
