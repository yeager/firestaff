#include "dm2_v1_gdat_scene_m11_command.h"

#include <string.h>

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
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

void dm2_v1_gdat_scene_m11_command_plan_free(DM2_V1_GdatSceneM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < 2; ++i) dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

int dm2_v1_gdat_scene_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatSceneM11CommandPlan *out_plan)
{
    static const uint8_t fields[2] = { DM2_GDAT_GFXSET_FLOOR, DM2_GDAT_GFXSET_CEIL };
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
    /* skproject CHECK_RECOMPUTE_LIGHT clamps this exact GRAPHICSSET field at
     * eight. Every live G1 set carries it; AMBIANT_LIGHT does not, so it is
     * intentionally outside this fail-closed scene/light command family. */
    candidate.graphicsset = graphicsset;
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
        if (!command->raw_hash) { dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0; }
        hash = hash_bytes(hash, (const uint8_t *)&command->raw_hash, sizeof(command->raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->palette_hash, sizeof(command->palette_hash));
    }
    hash ^= candidate.scene_colorkey; hash *= 16777619u;
    hash ^= candidate.scene_flags; hash *= 16777619u;
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
