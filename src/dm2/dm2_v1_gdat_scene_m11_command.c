#include "dm2_v1_gdat_scene_m11_command.h"

#include <string.h>

#define DM2_V1_GDAT_SCENE_VIEWPORT_WIDTH 224u
#define DM2_V1_GDAT_SCENE_VIEWPORT_HEIGHT 136u

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int16_t read_le16s(const uint8_t *bytes)
{
    return (int16_t)read_le16(bytes);
}

uint32_t dm2_v1_gdat_scene_m11_command_pixel_hash(
    const DM2_V1_GdatSceneM11Command *command)
{
    if (!command || !command->pixels || command->width == 0u ||
        command->height == 0u) {
        return 0u;
    }
    return hash_bytes(2166136261u, command->pixels,
                      (size_t)command->width * command->height);
}

uint32_t dm2_v1_gdat_scene_m11_command_geometry_hash(
    const DM2_V1_GdatSceneM11Command *command,
    const DM2_V1_GdatSceneBlitRect *rect)
{
    uint32_t hash = 2166136261u;

    if (!command || !rect || command->width == 0u || command->height == 0u ||
        rect->width != command->width || rect->height != command->height ||
        rect->x < 0 || rect->y < 0) {
        return 0u;
    }
    hash = hash_bytes(hash, &command->field, sizeof(command->field));
    hash = hash_bytes(hash, (const uint8_t *)&command->width,
                      sizeof(command->width));
    hash = hash_bytes(hash, (const uint8_t *)&command->height,
                      sizeof(command->height));
    return hash_bytes(hash, (const uint8_t *)rect, sizeof(*rect));
}

uint32_t dm2_v1_gdat_scene_query_blit_rect_hash(
    const DM2_V1_GdatSceneQueryBlitRectReceipt *receipt)
{
    uint32_t hash = 2166136261u;

    if (!receipt || !receipt->valid ||
        receipt->floor_rect_number != DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
        receipt->ceiling_rect_number != DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER ||
        receipt->table_hash == 0u || receipt->floor_row_hash == 0u ||
        receipt->ceiling_row_hash == 0u) {
        return 0u;
    }
    hash = hash_bytes(hash, (const uint8_t *)&receipt->floor_rect_number,
                      sizeof(receipt->floor_rect_number));
    hash = hash_bytes(hash, (const uint8_t *)&receipt->ceiling_rect_number,
                      sizeof(receipt->ceiling_rect_number));
    hash = hash_bytes(hash, (const uint8_t *)&receipt->table_hash,
                      sizeof(receipt->table_hash));
    hash = hash_bytes(hash, (const uint8_t *)&receipt->floor_row_hash,
                      sizeof(receipt->floor_row_hash));
    return hash_bytes(hash, (const uint8_t *)&receipt->ceiling_row_hash,
                      sizeof(receipt->ceiling_row_hash));
}

static const uint8_t *find_compressed_rect_row(const uint8_t *table,
                                                size_t table_size,
                                                uint16_t rect_number)
{
    uint16_t group_count;
    size_t rows_offset;

    if (!table || table_size < 4u || read_le16(table) != 0xfc0du) return NULL;
    group_count = read_le16(table + 2u);
    if (group_count == 0u || (size_t)group_count > (table_size - 4u) / 4u) {
        return NULL;
    }
    rows_offset = 4u + (size_t)group_count * 4u;
    for (uint16_t group = 0u; group < group_count; ++group) {
        uint16_t first = read_le16(table + 4u + (size_t)group * 4u);
        uint16_t last = read_le16(table + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;

        if (count == 0u || count > (table_size - rows_offset) / 8u) {
            return NULL;
        }
        if (rect_number >= first && rect_number <= last) {
            return table + rows_offset + (size_t)(rect_number - first) * 8u;
        }
        rows_offset += count * 8u;
    }
    return NULL;
}

static int decode_viewport_root_rect(const uint8_t *table, size_t table_size,
                                     const uint8_t *row, uint16_t rect_number,
                                     uint16_t width, uint16_t height,
                                     DM2_V1_GdatSceneBlitRect *out_rect)
{
    const uint8_t *reference;
    const uint8_t *clip;

    /* skproject QUERY_BLIT_RECT (098D:0602-0BBD) handles 700/701 through
     * x=11/14 -> record x=1 -> viewport clip x=9. 11 reduces to top-left
     * anchor 1 and 14 to bottom-left anchor 4; their x=1 reference applies
     * no adjustment, and record 3 clips only to (0,0,224,136). No alternate
     * anchor, source crop, or nested clip is admitted here. */
    if (!table || !row || !out_rect || width == 0u || height == 0u ||
        (read_le16s(row) != 11 && read_le16s(row) != 14) ||
        read_le16s(row + 4u) != 0 ||
        read_le16s(row + 6u) != 0) {
        return 0;
    }
    reference = find_compressed_rect_row(table, table_size, read_le16(row + 2u));
    if (!reference || read_le16s(reference) != 1 ||
        read_le16s(reference + 4u) != 0 || read_le16s(reference + 6u) != 0) {
        return 0;
    }
    clip = find_compressed_rect_row(table, table_size, read_le16(reference + 2u));
    if (!clip || read_le16s(clip) != 9 || read_le16s(clip + 2u) != 0 ||
        read_le16s(clip + 4u) != DM2_V1_GDAT_SCENE_VIEWPORT_WIDTH ||
        read_le16s(clip + 6u) != DM2_V1_GDAT_SCENE_VIEWPORT_HEIGHT ||
        width > DM2_V1_GDAT_SCENE_VIEWPORT_WIDTH ||
        height > DM2_V1_GDAT_SCENE_VIEWPORT_HEIGHT) {
        return 0;
    }
    out_rect->rect_number = rect_number;
    out_rect->x = 0;
    out_rect->y = read_le16s(row) == 11 ? 0 :
        (int16_t)(DM2_V1_GDAT_SCENE_VIEWPORT_HEIGHT - height);
    out_rect->width = width;
    out_rect->height = height;
    return 1;
}

void dm2_v1_gdat_scene_m11_command_plan_free(DM2_V1_GdatSceneM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < 2; ++i) dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

int dm2_v1_gdat_scene_light_m11_receipt(
    const DM2_V1_GdatSceneM11CommandPlan *plan,
    DM2_V1_GdatSceneLightM11Receipt *out_receipt)
{
    DM2_V1_GdatSceneLightM11Receipt candidate;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!plan || !plan->valid || plan->command_hash == 0u ||
        plan->commands[0].raw_hash == 0u || plan->commands[1].raw_hash == 0u ||
        plan->ambient_darkness > 8u) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.graphicsset = plan->graphicsset;
    candidate.ambient_light = plan->ambient_light;
    candidate.highest_light_level = plan->highest_light_level;
    candidate.ambient_darkness = plan->ambient_darkness;
    candidate.scene_control_hash = plan->command_hash;
    hash = hash_bytes(hash, &candidate.graphicsset, sizeof(candidate.graphicsset));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.scene_control_hash,
                      sizeof(candidate.scene_control_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.ambient_light,
                      sizeof(candidate.ambient_light));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.highest_light_level,
                      sizeof(candidate.highest_light_level));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.ambient_darkness,
                      sizeof(candidate.ambient_darkness));
    if (!hash) return 0;
    candidate.receipt_hash = hash;
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_c_light_m11_receipt_build(
    const DM2_V1_GdatSceneLightM11Receipt *scene,
    const DM2_V1_CLightSourceState *source,
    DM2_V1_CLightM11Receipt *out_receipt)
{
    DM2_V1_CLightM11Receipt candidate;
    int level;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!scene || !scene->valid || scene->scene_control_hash == 0u ||
        !source || !source->valid || source->source_state_hash == 0u ||
        source->dynamic_map > 1u || source->base_light > 5u ||
        source->darkness_offset > 12u) {
        return 0;
    }

    /* SKProject c_light.cpp::DM2_RECALC_LIGHT_LEVEL: a non-dynamic map
     * begins at level one; an admitted dynamic map supplies v1e0974. The
     * final c_light operation subtracts v1e0978 and clamps to [0, 5]. */
    level = source->dynamic_map ? source->base_light : 1;
    level -= source->darkness_offset;
    if (level < 0) level = 0;
    if (level > 5) level = 5;

    memset(&candidate, 0, sizeof(candidate));
    candidate.graphicsset = scene->graphicsset;
    candidate.light_level = (uint8_t)level;
    candidate.dynamic_map = source->dynamic_map;
    candidate.scene_control_hash = scene->scene_control_hash;
    candidate.source_state_hash = source->source_state_hash;
    hash = hash_bytes(hash, &candidate.graphicsset,
                      sizeof(candidate.graphicsset));
    hash = hash_bytes(hash, &candidate.light_level,
                      sizeof(candidate.light_level));
    hash = hash_bytes(hash, &candidate.dynamic_map,
                      sizeof(candidate.dynamic_map));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.scene_control_hash,
                      sizeof(candidate.scene_control_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.source_state_hash,
                      sizeof(candidate.source_state_hash));
    if (hash == 0u) return 0;
    candidate.receipt_hash = hash;
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_c_light_m11_palette_darkness(
    const DM2_V1_GdatSceneLightM11Receipt *scene,
    const DM2_V1_CLightM11Receipt *receipt,
    uint8_t *out_darkness)
{
    if (!out_darkness) return 0;
    *out_darkness = 0u;
    if (!scene || !scene->valid || scene->scene_control_hash == 0u ||
        !receipt || !receipt->valid || receipt->receipt_hash == 0u ||
        receipt->source_state_hash == 0u || receipt->light_level > 5u ||
        receipt->graphicsset != scene->graphicsset ||
        receipt->scene_control_hash != scene->scene_control_hash) {
        return 0;
    }
    /* SKProject/SKULLWIN/c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT (32CB:5D13):
     * _4976_5a88 = glbLightLevel * 10. */
    *out_darkness = (uint8_t)(receipt->light_level * 10u);
    return 1;
}

int dm2_v1_gdat_scene_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatSceneM11CommandPlan *out_plan)
{
    static const uint8_t fields[2] = { DM2_GDAT_GFXSET_FLOOR, DM2_GDAT_GFXSET_CEIL };
    static const uint16_t rect_numbers[2] = {
        DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER,
        DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER
    };
    DM2_V1_GdatSceneM11CommandPlan candidate;
    uint32_t hash = 2166136261u;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    memset(&candidate, 0, sizeof(candidate));
    if (!loader || !dm2_v1_asset_loader_verify(loader) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, DM2_GDAT_GFXSET_SCENE_COLORKEY, &candidate.scene_colorkey) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, DM2_GDAT_GFXSET_SCENE_FLAGS, &candidate.scene_flags) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, DM2_GDAT_GFXSET_HIGHEST_LIGHT_LEVEL, &candidate.highest_light_level) ||
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, DM2_GDAT_GFXSET_AMBIANT_DARKNESS, &candidate.ambient_darkness) ||
        candidate.ambient_darkness > 8u) return 0;
    /* skproject QUERY_GDAT_ENTRY_DATA_INDEX returns zero for a missing
     * dtWordValue entry. GRAPHICSSET 2 in the canonical PC corpus has no
     * AMBIANT_LIGHT row, so preserve that source zero without borrowing a
     * control value from another graphics set. */
    candidate.ambient_light = 0u;
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_GFXSET_AMBIANT_LIGHT, &candidate.ambient_light);
    candidate.graphicsset = graphicsset;
    if (!dm2_v1_gdat_scene_query_blit_rect_receipt(
            loader, &candidate.query_blit_rect) ||
        !candidate.query_blit_rect.valid ||
        candidate.query_blit_rect.floor_rect_number !=
            DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
        candidate.query_blit_rect.ceiling_rect_number !=
            DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER) {
        return 0;
    }
    candidate.query_blit_rect_hash =
        dm2_v1_gdat_scene_query_blit_rect_hash(&candidate.query_blit_rect);
    if (!candidate.query_blit_rect_hash) return 0;
    for (int i = 0; i < 2; ++i) {
        DM2_V1_GdatSceneM11Command *command = &candidate.commands[i];
        const uint8_t *raw;
        size_t raw_size = 0u;
        int width = 0, height = 0;
        command->field = fields[i];
        raw = dm2_v1_asset_load_sized(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
                                      graphicsset, fields[i], &raw_size);
        command->pixels = dm2_v1_asset_load_image_field(loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, fields[i], &width,
            &height, &command->format);
        if (!raw || raw_size == 0u || !command->pixels || width <= 0 || height <= 0 ||
            command->format == DM2_IMG_FMT_UNKNOWN ||
            !dm2_v1_asset_load_image_local_palette(loader,
                DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, fields[i],
                command->palette16, &command->palette_hash) || !command->palette_hash) {
            dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0;
        }
        command->width = (uint16_t)width; command->height = (uint16_t)height;
        command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
        command->decoded_hash = dm2_v1_gdat_scene_m11_command_pixel_hash(command);
        if (!command->raw_hash || !command->decoded_hash) {
            dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0;
        }
        hash = hash_bytes(hash, (const uint8_t *)&command->raw_hash, sizeof(command->raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->decoded_hash,
                          sizeof(command->decoded_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->palette_hash, sizeof(command->palette_hash));
    }
    {
        const uint8_t *table;
        size_t table_size = 0u;
        table = dm2_v1_asset_load_typed_sized(
            loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
            DM2_GDAT_ENTRY_TYPE_RAW4, 0, &table_size);
        for (int i = 0; i < 2; ++i) {
            const uint8_t *row = find_compressed_rect_row(
                table, table_size, rect_numbers[i]);
            if (!decode_viewport_root_rect(table, table_size, row, rect_numbers[i],
                                         candidate.commands[i].width,
                                         candidate.commands[i].height,
                                         &candidate.rects[i])) {
                dm2_v1_gdat_scene_m11_command_plan_free(&candidate);
                return 0;
            }
            candidate.commands[i].geometry_hash =
                dm2_v1_gdat_scene_m11_command_geometry_hash(
                    &candidate.commands[i], &candidate.rects[i]);
            if (!candidate.commands[i].geometry_hash) {
                dm2_v1_gdat_scene_m11_command_plan_free(&candidate);
                return 0;
            }
            hash = hash_bytes(hash, (const uint8_t *)&candidate.rects[i],
                              sizeof(candidate.rects[i]));
            hash = hash_bytes(hash,
                              (const uint8_t *)&candidate.commands[i].geometry_hash,
                              sizeof(candidate.commands[i].geometry_hash));
        }
    }
    hash = hash_bytes(hash,
                      (const uint8_t *)&candidate.query_blit_rect.table_hash,
                      sizeof(candidate.query_blit_rect.table_hash));
    hash = hash_bytes(hash,
                      (const uint8_t *)&candidate.query_blit_rect.floor_row_hash,
                      sizeof(candidate.query_blit_rect.floor_row_hash));
    hash = hash_bytes(hash,
                      (const uint8_t *)&candidate.query_blit_rect.ceiling_row_hash,
                      sizeof(candidate.query_blit_rect.ceiling_row_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.query_blit_rect_hash,
                      sizeof(candidate.query_blit_rect_hash));
    hash ^= candidate.scene_colorkey; hash *= 16777619u;
    hash ^= candidate.scene_flags; hash *= 16777619u;
    hash ^= candidate.ambient_light; hash *= 16777619u;
    hash ^= candidate.highest_light_level; hash *= 16777619u;
    hash ^= candidate.ambient_darkness; hash *= 16777619u;
    if (!hash) { dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0; }
    candidate.command_hash = hash; candidate.valid = 1; *out_plan = candidate;
    return 1;
}

int dm2_v1_gdat_scene_query_blit_rect_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatSceneQueryBlitRectReceipt *out_receipt)
{
    const uint8_t *table;
    const uint8_t *floor_row;
    const uint8_t *ceiling_row;
    size_t table_size = 0u;
    DM2_V1_GdatSceneQueryBlitRectReceipt candidate;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !dm2_v1_asset_loader_verify(loader)) return 0;
    table = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &table_size);
    floor_row = find_compressed_rect_row(
        table, table_size, DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER);
    ceiling_row = find_compressed_rect_row(
        table, table_size, DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER);
    if (!floor_row || !ceiling_row) return 0;

    memset(&candidate, 0, sizeof(candidate));
    candidate.floor_rect_number = DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER;
    candidate.ceiling_rect_number = DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER;
    candidate.table_hash = hash_bytes(2166136261u, table, table_size);
    candidate.floor_row_hash = hash_bytes(2166136261u, floor_row, 8u);
    candidate.ceiling_row_hash = hash_bytes(2166136261u, ceiling_row, 8u);
    if (!candidate.table_hash || !candidate.floor_row_hash ||
        !candidate.ceiling_row_hash) return 0;
    candidate.valid = 1;
    *out_receipt = candidate;
    return 1;
}
