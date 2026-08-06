#include "dm2_v1_gdat_door_overlay_m11_command.h"

#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_viewport_renderer.h"




#include <limits.h>
#include <string.h>

#define DM2_V1_GDAT_DOOR_LOCAL_PALETTE_SIZE 16u

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    return hash_bytes(hash, (const uint8_t *)&value, sizeof(value));
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM2_V1_DoorRawRect;

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int16_t read_le16s(const uint8_t *bytes)
{
    return (int16_t)read_le16(bytes);
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

/* c_xrect.cpp::DM2_QUERY_RECT materializes a compressed raw4 row as a
 * c_rinfo.  Keep the table parsing local to this source-owned M11 command. */
static int decode_raw4_rect(const uint8_t *table, size_t table_size,
                            uint16_t rect_number, DM2_V1_DoorRawRect *out)
{
    const uint8_t *row;
    uint16_t groups;
    size_t offset;
    uint8_t mask = 0x1fu;

    if (!out || !table || table_size < 4u || read_le16(table) != 0xfc0du) return 0;
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
                ((mask & 0x01u) ? (int)(int16_t)y0 : (int)read_le16s(row + 2u));
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

/* Exact bounded c_xrect.cpp::DM2_QUERY_BLIT_RECT subset used by the closed
 * DRAW_DOOR panel: no signed query, no override mode and no global clip.
 * Unsupported raw4 grammar fails closed. */
static int query_raw4_blit_rect(const uint8_t *table, size_t table_size,
                                uint16_t rect_number, int width, int height,
                                int query_offset_x, int query_offset_y,
                                int signed_rect, int override_mode,
                                DM2_V1_DoorRawRect *out,
                                int *out_source_x, int *out_source_y)
{
    DM2_V1_DoorRawRect current;
    DM2_V1_DoorRawRect clip = { -10000, -10000, 20000, 20000 };
    int x0, y0, mode, pending_anchor = 0;

    if (!out || width <= 0 || height <= 0 ||
        !decode_raw4_rect(table, table_size, rect_number, &current) ||
        current.x == 9 || current.x < 0 || current.x > 18) return 0;
    mode = override_mode >= 0 ? override_mode : current.x;
    if (override_mode >= 0) current.x = (int16_t)override_mode;
    if (mode > 8) { mode -= 10; x0 = 0; y0 = 0; }
    else { x0 = current.w; y0 = current.h; }
    /* DRAW_PICST marks the raw rectangle signed when QUERY_TEMP_PICST's
     * queried image offset is nonzero.  DM2_QUERY_BLIT_RECT then applies the
     * offset before it resolves the source image's width/height. */
    if (signed_rect) {
        x0 += query_offset_x;
        y0 += query_offset_y;
    }
    for (int guard = 0; current.y != 0 && guard < 64; ++guard) {
        DM2_V1_DoorRawRect next;
        if (!decode_raw4_rect(table, table_size, (uint16_t)current.y, &next)) return 0;
        if (current.x >= 10 && current.x <= 18) {
            /* The canonical closed-door roots do not use c_xrect's nested
             * clipping grammar. Keep an unproven branch fail-closed. */
            return 0;
        }
        if (next.x == 1) {
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
        current = next;
    }
    if (current.y != 0 || !decode_anchor(mode, x0, y0, width, height,
                                          &out->x, &out->y) ||
        clip.w <= 0 || clip.h <= 0) return 0;
    {
        int dx = clip.x - out->x;
        int dy = clip.y - out->y;
        if (out_source_x) *out_source_x = dx > 0 ? dx : 0;
        if (out_source_y) *out_source_y = dy > 0 ? dy : 0;
        if (dx > 0) { out->x = clip.x; out->w = width - dx < clip.w ? width - dx : clip.w; }
        else out->w = width < dx + clip.w ? width : dx + clip.w;
        if (dy > 0) { out->y = clip.y; out->h = height - dy < clip.h ? height - dy : clip.h; }
        else out->h = height < dy + clip.h ? height : dy + clip.h;
    }
    return out->x >= 0 && out->y >= 0 && out->w > 0 && out->h > 0;
}

int dm2_v1_gdat_door_overlay_query_raw4_blit_placement(
    const DM2_V1_AssetLoader *loader,
    uint16_t rect_number,
    int image_width,
    int image_height,
    DM2_V1_GdatRaw4BlitPlacement *out_placement)
{
    const uint8_t *table;
    size_t table_size = 0u;
    DM2_V1_DoorRawRect rect;

    if (!out_placement || image_width <= 0 || image_height <= 0) return 0;
    memset(out_placement, 0, sizeof(*out_placement));
    if (!loader || !dm2_v1_asset_loader_verify(loader)) return 0;
    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &table_size);
    if (!table || !table_size ||
        !query_raw4_blit_rect(table, table_size, rect_number,
                              image_width, image_height,
                              0, 0, 0, -1, &rect,
                              &out_placement->source_x,
                              &out_placement->source_y) ||
        rect.x < 0 || rect.y < 0 || rect.w <= 0 || rect.h <= 0) {
        memset(out_placement, 0, sizeof(*out_placement));
        return 0;
    }
    out_placement->destination.x = rect.x;
    out_placement->destination.y = rect.y;
    out_placement->destination.w = rect.w;
    out_placement->destination.h = rect.h;
    return 1;
}

int dm2_v1_gdat_door_overlay_panel_rect_number(int view_square,
                                               uint16_t *out_rect_number)
{
    /* SKWIN/skval1.h tlbRectnoDoorPosition, indexed by viewport cell.
     * DRAW_DOOR admits these centre cells only on the current M11 route. */
    if (!out_rect_number) return 0;
    switch (view_square) {
    case DM2_SQ_D0C: *out_rect_number = 0x0ee2u; return 1;
    case DM2_SQ_D1C: *out_rect_number = 0x0eceu; return 1;
    case DM2_SQ_D2C: *out_rect_number = 0x0eb0u; return 1;
    case DM2_SQ_D3C: *out_rect_number = 0x0e92u; return 1;
    default: return 0;
    }
}

int dm2_v1_gdat_door_overlay_button_rect_number(int view_square,
                                                uint16_t *out_rect_number)
{
    /* SKWIN/skval1.h tlbRectnoDoorButton for the default door button at the
     * centre-line cells used by DRAW_DEFAULT_DOOR_BUTTON. */
    if (!out_rect_number) return 0;
    switch (view_square) {
    case DM2_SQ_D0C: *out_rect_number = 0x7a2u; return 1;
    case DM2_SQ_D1C: *out_rect_number = 0x7a1u; return 1;
    case DM2_SQ_D2C: *out_rect_number = 0x7a0u; return 1;
    default: return 0;
    }
}

int dm2_v1_gdat_door_overlay_query_raw4_destination_rect(
    const DM2_V1_AssetLoader *loader,
    uint16_t rect_number,
    int image_width,
    int image_height,
    DM2_V1_ViewportRect *out_rect)
{
    DM2_V1_GdatRaw4BlitPlacement placement;

    if (!out_rect || image_width <= 0 || image_height <= 0) return 0;
    out_rect->x = 0;
    out_rect->y = 0;
    out_rect->w = 0;
    out_rect->h = 0;
    if (!dm2_v1_gdat_door_overlay_query_raw4_blit_placement(
            loader, rect_number, image_width, image_height, &placement)) {
        return 0;
    }
    *out_rect = placement.destination;
    return 1;
}

static int bind_door_panel_geometry(
    const DM2_V1_AssetLoader *loader, const DM2_V1_DoorRender *door,
    DM2_V1_GdatDoorOverlayM11Command *command, int horizontal_part)
{
    uint16_t rect_number;

    const uint8_t *table;
    const uint8_t *row;
    size_t table_size = 0u;
    DM2_V1_DoorRawRect rect;

    if (!loader || !door || !command) return 0;
    if (door->door_state == 0u) return 1;
    if (!dm2_v1_gdat_door_overlay_panel_rect_number(door->view_square, &rect_number)) return 0;
    command->source_x = 0u;
    command->source_y = 0u;
    command->source_width = command->width;
    command->source_height = command->height;
    /* SKWIN/SkWinCore.cpp::DRAW_DOOR adds the intermediate state (1..3)
     * to tlbRectnoDoorPosition for vertical openings. Horizontal openings
     * halve the source bitmap and draw right then left through distinct RAW4
     * positions. Keep both commands in one source-owned transaction. */
    if (door->door_state < 4u) {
        if (door->door_opening_dir == 0u) {
            uint16_t half_width;
            if (horizontal_part < 0 || horizontal_part > 1 ||
                command->width < 2u || (command->width & 1u) != 0u) {
                return 0;
            }
            half_width = (uint16_t)(command->width / 2u);
            command->source_width = half_width;
            if (horizontal_part == 0) {
                /* source right half, rect base + state + 6 */
                command->source_x = half_width;
                rect_number = (uint16_t)(rect_number + door->door_state + 6u);
            } else {
                /* source left half, rect base + state + 3 */
                rect_number = (uint16_t)(rect_number + door->door_state + 3u);
            }
        } else {
            if (horizontal_part >= 0) return 0;
            rect_number = (uint16_t)(rect_number + door->door_state);
        }
    } else if (horizontal_part >= 0) {
        return 0;
    }
    command->rect_number = rect_number;
    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &table_size);
    row = find_raw4_row(table, table_size, command->rect_number);
    if (!table || !table_size || !row ||
        !query_raw4_blit_rect(table, table_size, command->rect_number,
                             command->source_width, command->source_height,
                             0, 0, 0, -1, &rect, NULL, NULL) ||
        rect.x > INT16_MAX || rect.y > INT16_MAX ||
        rect.w > UINT16_MAX || rect.h > UINT16_MAX) {
        return 0;
    }
    command->rect_x = (int16_t)rect.x;
    command->rect_y = (int16_t)rect.y;
    command->rect_width = (uint16_t)rect.w;
    command->rect_height = (uint16_t)rect.h;
    command->rect_table_hash = hash_bytes(2166136261u, table, table_size);
    command->rect_row_hash = hash_bytes(2166136261u, row, 8u);
    command->geometry_hash = 2166136261u;
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_number);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      (uint32_t)(uint16_t)command->rect_x);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      (uint32_t)(uint16_t)command->rect_y);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_width);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_height);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->source_x);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->source_y);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->source_width);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->source_height);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_table_hash);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_row_hash);
    return command->rect_table_hash != 0u && command->rect_row_hash != 0u &&
           command->geometry_hash != 0u;
}

/* DRAW_DOOR_FRAMES uses QUERY_TEMP_PICST with a GRAPHICSSET image, a fixed
 * 64/64 scale, source rect mode 4 (left) or 3 (right), and the image's
 * QUERY_GDAT_SUMMARY_IMAGE offset.  This is deliberately separate from the
 * centre-door-frame route: its destination belongs to RAW4, not wall boxes. */
static int bind_door_side_frame_geometry(
    const DM2_V1_AssetLoader *loader, const DM2_V1_DoorRender *door,
    int side, DM2_V1_GdatDoorOverlayM11Command *command)
{
    DM2_V1_GdatImageMetadata metadata;
    DM2_V1_DoorRawRect rect;
    const uint8_t *table;
    const uint8_t *row;
    size_t table_size = 0u;
    int offset_x;
    int offset_y;

    if (!loader || !door || !command || side < 0 || side > 1 ||
        door->side_frame_graphicsset_field[side] == 0u ||
        door->side_frame_rect_number[side] <= 0 || command->width == 0u ||
        command->height == 0u ||
        !dm2_v1_asset_load_image_metadata(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, door->graphicsset_index,
            door->side_frame_graphicsset_field[side], &metadata)) {
        return 0;
    }
    offset_x = metadata.query_offset_x;
    offset_y = metadata.query_offset_y;
    command->mirror_flip = (uint8_t)door->side_frame_mirror_flip[side];
    if (command->mirror_flip & 1u) offset_x = -offset_x;
    command->source_x = 0u;
    command->source_y = 0u;
    command->source_width = command->width;
    command->source_height = command->height;
    command->rect_number = (uint16_t)door->side_frame_rect_number[side];
    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &table_size);
    row = find_raw4_row(table, table_size, command->rect_number);
    if (!table || !table_size || !row ||
        !query_raw4_blit_rect(table, table_size, command->rect_number,
                              command->source_width, command->source_height,
                              offset_x, offset_y,
                              offset_x != 0 || offset_y != 0,
                              side == 0 ? 4 : 3, &rect, NULL, NULL) ||
        rect.x > INT16_MAX || rect.y > INT16_MAX ||
        rect.w > UINT16_MAX || rect.h > UINT16_MAX) {
        return 0;
    }
    command->rect_x = (int16_t)rect.x;
    command->rect_y = (int16_t)rect.y;
    command->rect_width = (uint16_t)rect.w;
    command->rect_height = (uint16_t)rect.h;
    command->rect_table_hash = hash_bytes(2166136261u, table, table_size);
    command->rect_row_hash = hash_bytes(2166136261u, row, 8u);
    command->geometry_hash = 2166136261u;
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_number);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      (uint32_t)(uint16_t)command->rect_x);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      (uint32_t)(uint16_t)command->rect_y);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_width);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_height);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      (uint32_t)(uint16_t)offset_x);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      (uint32_t)(uint16_t)offset_y);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      metadata.metadata_hash);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->mirror_flip);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_table_hash);
    command->geometry_hash = hash_u32(command->geometry_hash,
                                      command->rect_row_hash);
    return command->rect_table_hash != 0u && command->rect_row_hash != 0u &&
           metadata.metadata_hash != 0u && command->geometry_hash != 0u;
}

/* SKWINSPX v0/skglobal.cpp glbTabYAxisDistance and
 * v4/SkWinCore.cpp DRAW_DOOR: D0/D1/D2/D3 center cells are 0/3/6/11.
 * The initial 0x40 stretch is overridden for D0 by the source's mandatory
 * image-zero branch (SkWinCore.cpp:46431-46441), which uses 0x71. */
static int resolve_draw_door_distance(int view_square,
                                      uint8_t *out_distance,
                                      uint8_t *out_stretch,
                                      uint8_t *out_light)
{
    uint8_t distance;
    if (!out_distance || !out_stretch || !out_light) return 0;
    switch (view_square) {
    case DM2_SQ_D0C: distance = 0u; break;
    case DM2_SQ_D1C: distance = 1u; break;
    case DM2_SQ_D2C: distance = 2u; break;
    case DM2_SQ_D3C: distance = 3u; break;
    default: return 0;
    }
    *out_distance = distance;
    *out_stretch = distance == 0u ? 0x71u : 0x40u;
    *out_light = 0u;
    return 1;
}

static uint8_t draw_door_distance_stretch(uint8_t distance)
{
    /* kskval1.h tlbDistanceStretch = { 0x60, 0x40, 0x2b, 0x1c, 0x13 }. */
    static const uint8_t stretch[] = { 0x60u, 0x40u, 0x2bu, 0x1cu, 0x13u };
    return distance < sizeof(stretch) ? stretch[distance] : 0u;
}

int dm2_v1_gdat_door_light_palette_darkness(uint8_t light,
                                            uint8_t light_palette,
                                            uint8_t *out_darkness)
{
    /* SKProject skval1.h `_4976_4226`, consumed by _32cb_0804 when the
     * player is stationary. `light` is DISPLAY_VIEWPORT's glbLightLevel*10. */
    static const uint8_t distance_darkness[] = { 0u, 0u, 12u, 28u, 46u };
    uint32_t distance;
    uint32_t result;

    if (!out_darkness || light > 64u ||
        light_palette >= sizeof(distance_darkness)) {
        return 0;
    }
    distance = distance_darkness[light_palette];
    result = 64u - (((64u - distance) * (64u - light)) >> 6);
    if (result > 64u) return 0;
    *out_darkness = (uint8_t)result;
    return 1;
}

static uint32_t command_plan_hash(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    uint32_t hash = 2166136261u;

    if (!plan || plan->command_count == 0u ||
        plan->command_count > DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX) {
        return 0u;
    }
    for (uint8_t i = 0u; i < plan->command_count; ++i) {
        const DM2_V1_GdatDoorOverlayM11Command *command = &plan->commands[i];
        hash = hash_u32(hash, command->raw_hash);
        hash = hash_u32(hash, command->decoded_hash);
        hash = hash_u32(hash, command->palette_hash);
        hash = hash_u32(hash, command->material_receipt_hash);
        hash = hash_u32(hash, command->selection_hash);
        hash = hash_u32(hash, command->geometry_hash);
        hash = hash_u32(hash, command->palette_transform_hash);
    }
    return hash ? hash : 1u;
}

int dm2_v1_gdat_door_overlay_m11_command_plan_refresh_hash(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    uint32_t hash;

    if (!plan) return 0;
    hash = command_plan_hash(plan);
    if (!hash) return 0;
    plan->command_hash = hash;
    plan->valid = 1;
    return 1;
}

static int command_draw_controls_valid(
    const DM2_V1_GdatDoorOverlayM11Command *command)
{
    uint8_t distance;
    uint8_t stretch;
    uint8_t light;

    if (!command || command->kind != DM2_V1_GDAT_DOOR_PANEL ||
        !resolve_draw_door_distance(command->view_square, &distance,
                                    &stretch, &light)) {
        return 0;
    }
    if (command->draw_distance != distance) return 0;
    if (command->field == (distance == 0u ? 0u : distance - 1u)) {
        return command->stretch_dual == stretch &&
               command->light_palette == light;
    }
    /* DRAW_DOOR retries DOORS/image 0 only after the selected distance image
     * cannot load. D0 already selects image zero through the first branch. */
    return distance != 0u && command->field == 0u &&
           command->stretch_dual == draw_door_distance_stretch(distance) &&
           command->light_palette == distance;
}

int dm2_v1_gdat_door_overlay_m11_command_plan_draw_controls_valid(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    if (!plan || !plan->valid || plan->command_count == 0u ||
        plan->command_count > DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX) {
        return 0;
    }
    if (plan->command_hash != command_plan_hash(plan)) return 0;
    for (uint8_t i = 0u; i < plan->command_count; ++i) {
        const DM2_V1_GdatDoorOverlayM11Command *command = &plan->commands[i];
        if (command->kind == DM2_V1_GDAT_DOOR_PANEL &&
            !command_draw_controls_valid(command)) {
            return 0;
        }
    }
    return 1;
}

void dm2_v1_gdat_door_overlay_m11_command_plan_free(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX; ++i)
        dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

/* SKProject SkWinCore.cpp DM2_DRAW_DOOR/DRAW_DOOR_FRAMES resolves these
 * category/index/field triples before the first door blit. */
static int resolve_material_address(const DM2_V1_DoorRender *door, int kind,
                                    int *out_gdat_index, int *out_category,
                                    int *out_index, int *out_field)
{
    int gdat_index;
    int packed;
    if (!door || !out_gdat_index || !out_category || !out_index || !out_field) return 0;
    switch (kind) {
    case DM2_V1_GDAT_DOOR_PANEL:
        gdat_index = door->panel_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        if (gdat_index <= DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
            gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE) {
            packed = DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - gdat_index;
            *out_index = (packed >> DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT) & 0xff;
            *out_field = packed & DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK;
        } else {
            *out_index = 0;
            *out_field = DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index;
        }
        break;
    case DM2_V1_GDAT_DOOR_OVERLAY_ORNATE:
        gdat_index = door->ornate_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOOR_GFX;
        *out_index = door->door_ornate_gfx_index;
        *out_field = dm2_v1_viewport_door_panel_field_for_square(door->view_square);
        break;
    case DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK:
        gdat_index = door->destroyed_mask_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOORS;
        *out_index = door->door_gfx_index;
        *out_field = dm2_v1_viewport_door_panel_field_for_square(door->view_square);
        break;
    case DM2_V1_GDAT_DOOR_FRAME:
        gdat_index = door->frame_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        /* SKWIN/SkWinCore.cpp::DRAW_DOOR_FRAMES captures glbMapGraphicsSet
         * before resolving either frame image.  An unbound value is invalid;
         * the legacy default exists only for callers that have not attached a
         * source scene receipt. */
        *out_index = door->graphicsset_index;
        if (*out_index < 0 || *out_index > 0xff) return 0;
        *out_field = dm2_v1_viewport_door_frame_field_for_square(door->view_square);
        break;
    case DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT:
    case DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT: {
        const int side = kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT ? 0 : 1;
        gdat_index = door->side_frame_gdat_index[side];
        *out_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
        *out_index = door->graphicsset_index;
        *out_field = door->side_frame_graphicsset_field[side];
        break;
    }
    case DM2_V1_GDAT_DOOR_BUTTON:
        if (door->button_source_kind != 1) return 1;
        gdat_index = door->button_gdat_index;
        *out_category = DM2_GDAT_CATEGORY_DOOR_BUTTONS;
        *out_index = 0;
        *out_field = DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - gdat_index;
        break;
    default: return 0;
    }
    if (gdat_index == 0 || *out_index < 0 || *out_index > 0xff ||
        *out_field < 0 || *out_field > 0xff) return 0;
    *out_gdat_index = gdat_index;
    return 1;
}

static int load_door_local_palette(const DM2_V1_AssetLoader *loader,
                                   int category, int index, int field,
                                   uint8_t out_palette16[16],
                                   uint32_t *out_hash)
{
    const uint8_t *raw;
    size_t raw_size = 0u;

    if (out_hash) *out_hash = 0u;
    if (!out_palette16) return 0;
    memset(out_palette16, 0, DM2_V1_GDAT_DOOR_LOCAL_PALETTE_SIZE);
    raw = dm2_v1_asset_load_typed_sized(
        loader, category, index, DM2_GDAT_ENTRY_TYPE_IMAGE, field, &raw_size);
    if (!raw || raw_size < DM2_V1_GDAT_DOOR_LOCAL_PALETTE_SIZE) {
        return 0;
    }
    memcpy(out_palette16,
           raw + raw_size - DM2_V1_GDAT_DOOR_LOCAL_PALETTE_SIZE,
           DM2_V1_GDAT_DOOR_LOCAL_PALETTE_SIZE);
    if (out_hash) {
        *out_hash = hash_bytes(2166136261u, out_palette16,
                               DM2_V1_GDAT_DOOR_LOCAL_PALETTE_SIZE);
        return *out_hash != 0u;
    }
    return 1;
}

static int source_raw_index(const DM2_V1_AssetLoader *loader,
                            const uint8_t *source_bytes,
                            size_t source_byte_count,
                            uint16_t *out_raw_index)
{
    uint16_t raw_index;

    if (out_raw_index) *out_raw_index = 0u;
    if (!loader || !loader->data || !loader->raw_offsets ||
        !loader->raw_sizes || !source_bytes || !source_byte_count ||
        !out_raw_index) {
        return 0;
    }
    for (raw_index = 0u; raw_index < loader->raw_data_count; ++raw_index) {
        if (loader->raw_sizes[raw_index] == source_byte_count &&
            loader->data + loader->raw_offsets[raw_index] == source_bytes) {
            *out_raw_index = raw_index;
            return 1;
        }
    }
    return 0;
}

static int add_material(const DM2_V1_AssetLoader *loader,
                        DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
                        const DM2_V1_DoorRender *door, int kind,
                        int horizontal_part, int movement_active)
{
    DM2_V1_GdatDoorOverlayM11Command *command;
    int gdat_index, category, index, field;
    const uint8_t *raw;
    size_t raw_size = 0u;
    int width = 0, height = 0;
    DM2_V1_GdatGfxRawMaterialReceipt gfx256_material;
    uint16_t raw_index;

    if (kind == DM2_V1_GDAT_DOOR_BUTTON && door->button_source_kind != 1) return 1;
    if ((kind == DM2_V1_GDAT_DOOR_OVERLAY_ORNATE && !door->ornate_gdat_index) ||
        (kind == DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK &&
         !door->destroyed_mask_gdat_index)) return 1;
    if (!resolve_material_address(door, kind, &gdat_index, &category, &index, &field)) return 0;
    if (plan->command_count >= DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX ||
        index < 0 || index > 0xff || field < 0) return 0;
    command = &plan->commands[plan->command_count];
    if (kind == DM2_V1_GDAT_DOOR_PANEL &&
        (!resolve_draw_door_distance(door->view_square,
                                     &command->draw_distance,
                                     &command->stretch_dual,
                                     &command->light_palette) ||
         field != (command->draw_distance == 0u
                       ? 0 : (int)command->draw_distance - 1))) {
        return 0;
    }
    raw = dm2_v1_asset_load_sized(loader, category, index, field, &raw_size);
    command->pixels = dm2_v1_asset_load_image_field(loader, category, index,
                                                      field, &width, &height, NULL);
    /* DRAW_DOOR retries image zero when the distance-selected image cannot
     * be loaded. Its retry is a real DOORS GDAT image, with the table-owned
     * distance stretch and light palette, rather than substitute artwork. */
    if (kind == DM2_V1_GDAT_DOOR_PANEL &&
        (!raw || !raw_size || !command->pixels || width <= 0 || height <= 0) &&
        field != 0 && command->draw_distance != 0u) {
        dm2_v1_asset_free_pixels(command->pixels);
        command->pixels = NULL;
        raw_size = 0u;
        field = 0;
        raw = dm2_v1_asset_load_sized(loader, category, index, field, &raw_size);
        command->pixels = dm2_v1_asset_load_image_field(loader, category, index,
                                                          field, &width, &height, NULL);
        command->stretch_dual = draw_door_distance_stretch(command->draw_distance);
        command->light_palette = command->draw_distance;
    }
    if (!raw || !raw_size || !command->pixels || width <= 0 || height <= 0 ||
        !load_door_local_palette(loader, category, index, field,
                                 command->palette16,
                                 &command->palette_hash) ||
        !command->palette_hash) return 0;
    /* DRAW_DOOR's panel retry above is the only permitted fallback. Its
     * selected source interval can be a non-dtImage GDAT row, so runtime
     * binds the actual row through GFX256 rather than inventing a GFX16 tuple. */
    if (!source_raw_index(loader, raw, raw_size, &raw_index) ||
        !dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
            loader, raw_index, &gfx256_material) ||
        gfx256_material.source_bytes != raw ||
        gfx256_material.source_byte_count != raw_size ||
        gfx256_material.raw_index != raw_index ||
        !gfx256_material.receipt_hash) {
        return 0;
    }
    command->gdat_index = gdat_index;
    command->view_square = (uint8_t)door->view_square;
    command->kind = (uint8_t)kind;
    command->category = (uint8_t)category;
    command->entry_index = (uint8_t)index;
    command->field = (uint8_t)field;
    command->width = (uint16_t)width;
    command->height = (uint16_t)height;
    command->door_opening_dir = door->door_opening_dir;
    command->door_state = door->door_state;
    command->door_open_pct = door->door_open_pct;
    command->movement_active = movement_active ? 1u : 0u;
    command->material_raw_index = gfx256_material.raw_index;
    command->material_source_bytes = gfx256_material.source_bytes;
    command->material_source_byte_count = gfx256_material.source_byte_count;
    command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
    command->decoded_hash = hash_bytes(2166136261u, command->pixels,
                                       (size_t)width * (size_t)height);
    command->selection_hash = 2166136261u;
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->view_square);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_gfx_index);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_opening_dir);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_state);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_open_pct);
    command->selection_hash = hash_u32(command->selection_hash,
                                       command->movement_active);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->ornament_index);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)door->door_ornate_gfx_index);
    command->selection_hash = hash_u32(command->selection_hash,
                                       (uint32_t)field);
    command->material_receipt_hash = gfx256_material.receipt_hash;
    if (kind == DM2_V1_GDAT_DOOR_PANEL) {
        if (!bind_door_panel_geometry(loader, door, command,
                                      horizontal_part)) return 0;
        if (!dm2_v1_asset_load_word_value(
                loader, DM2_GDAT_CATEGORY_DOORS, index,
                DM2_V1_DOOR_GDAT_COLORKEY_FIELD, &command->color_key)) {
            return 0;
        }
        if (command->color_key > 0xffu) return 0;
        /* DM2_QUERY_GDAT_ENTRY_DATA_INDEX returns zero for a missing
         * dtWordValue entry (skgdtqdb.cpp:105-116), exactly the value that
         * makes DRAW_DOOR_FRAMES retain frames. This is query semantics, not
         * a visual fallback. */
        (void)dm2_v1_asset_load_word_value(
            loader, DM2_GDAT_CATEGORY_DOORS, index,
            DM2_V1_DOOR_GDAT_NO_FRAMES_FIELD, &command->no_frames);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->color_key);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->no_frames);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->draw_distance);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->stretch_dual);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->light_palette);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->geometry_hash);
        command->selection_hash = hash_u32(command->selection_hash,
                                           (uint32_t)(horizontal_part + 1));
    } else if (kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT ||
               kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT) {
        const int side = kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT ? 0 : 1;
        if (!bind_door_side_frame_geometry(loader, door, side, command)) {
            return 0;
        }
        /* DRAW_DOOR_FRAMES passes the scene-owned colour key, not a DOORS
         * entry.  It is carried by the G1 scene transaction at draw time. */
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->geometry_hash);
        command->selection_hash = hash_u32(command->selection_hash,
                                           command->mirror_flip);
    }
    return command->material_source_bytes != NULL &&
           command->material_source_byte_count != 0u &&
           command->material_receipt_hash != 0u &&
           command->raw_hash != 0u && command->decoded_hash != 0u &&
           command->selection_hash != 0u && ++plan->command_count;
}

int dm2_v1_gdat_door_overlay_m11_command_plan_build_for_movement(
    const DM2_V1_AssetLoader *loader, const DM2_V1_DoorRenderPlan *door_plan,
    int movement_active,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan)
{
    DM2_V1_GdatDoorOverlayM11CommandPlan candidate;
    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    memset(&candidate, 0, sizeof(candidate));
    if (!loader || !door_plan || !dm2_v1_asset_loader_verify(loader)) return 0;
    for (int i = 0; i < door_plan->door_count; ++i) {
        int no_frames;
        if (door_plan->doors[i].door_state > 0u &&
            door_plan->doors[i].door_state < 4u &&
            door_plan->doors[i].door_opening_dir == 0u) {
            if (!add_material(loader, &candidate, &door_plan->doors[i],
                              DM2_V1_GDAT_DOOR_PANEL, 0, movement_active) ||
                !add_material(loader, &candidate, &door_plan->doors[i],
                              DM2_V1_GDAT_DOOR_PANEL, 1,
                              movement_active)) goto fail;
        } else if (!add_material(loader, &candidate, &door_plan->doors[i],
                                 DM2_V1_GDAT_DOOR_PANEL, -1,
                                 movement_active)) goto fail;
        no_frames = candidate.commands[candidate.command_count - 1].no_frames != 0u;
        if (
            !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_OVERLAY_ORNATE, -1, movement_active) ||
            !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK, -1, movement_active)) goto fail;
        if ((!no_frames && door_plan->doors[i].frame_gdat_index != 0 &&
             !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_FRAME, -1, movement_active)) ||
            !add_material(loader, &candidate, &door_plan->doors[i], DM2_V1_GDAT_DOOR_BUTTON, -1, movement_active)) goto fail;
        if (!no_frames) {
            for (int side = 0; side < 2; ++side) {
                const int kind = side == 0 ? DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT
                                           : DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT;
                if (door_plan->doors[i].side_frame_gdat_index[side] != 0 &&
                    !add_material(loader, &candidate, &door_plan->doors[i],
                                  kind, -1, movement_active)) goto fail;
            }
        }
    }
    if (!candidate.command_count ||
        !dm2_v1_gdat_door_overlay_m11_command_plan_refresh_hash(&candidate)) {
        goto fail;
    }
    *out_plan = candidate;
    return 1;
fail:
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&candidate);
    return 0;
}

int dm2_v1_gdat_door_overlay_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan)
{
    return dm2_v1_gdat_door_overlay_m11_command_plan_build_for_movement(
        loader, door_plan, 0, out_plan);
}
