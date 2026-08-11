#include "dm2_v1_gdat_scene_m11_command.h"

#include <string.h>

#define DM2_V1_GDAT_SCENE_VIEWPORT_WIDTH 224u
#define DM2_V1_GDAT_SCENE_VIEWPORT_HEIGHT 136u
#define DM2_V1_GDAT_SCENE_IMG3_HEADER_SIZE 10u
#define DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE 16u

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

static int scene_source_raw_index(const DM2_V1_AssetLoader *loader,
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

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static int16_t read_le16s(const uint8_t *bytes)
{
    return (int16_t)read_le16(bytes);
}

static int dm2_v1_gdat_scene_image_local_palette(
    const DM2_V1_AssetLoader *loader, int graphicsset, int field,
    uint8_t out_palette16[16], uint32_t *out_hash)
{
    const uint8_t *raw;
    size_t raw_size = 0u;
    size_t palette_offset;

    if (out_hash) *out_hash = 0u;
    if (!out_palette16) return 0;
    memset(out_palette16, 0, DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE);
    if (loader && loader->gdat_version == 4u) {
        DM2_V1_InterfacePalette palette;
        if (!dm2_v1_asset_load_interface_palette(
                loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, field,
                &palette) || palette.hash == 0u) {
            return 0;
        }
        memcpy(out_palette16, palette.palette16,
               DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE);
        if (out_hash) *out_hash = hash_bytes(
            2166136261u, out_palette16,
            DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE);
        return !out_hash || *out_hash != 0u;
    }
    raw = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_ENTRY_TYPE_IMAGE, field, &raw_size);
    if (!raw ||
        raw_size < DM2_V1_GDAT_SCENE_IMG3_HEADER_SIZE +
            DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE) {
        return 0;
    }
    /* QUERY_TEMP_PICST binds the image-local palette carried by the source
     * record.  The canonical G1 dungeon planes include both IMG3 and IMG9
     * encodings, so retain the record's trailing 16-byte palette instead of
     * deriving one from format-specific pixel payload heuristics. */
    palette_offset = raw_size - DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE;
    memcpy(out_palette16, raw + palette_offset,
           DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE);
    if (out_hash) {
        *out_hash = hash_bytes(2166136261u, out_palette16,
                               DM2_V1_GDAT_SCENE_LOCAL_PALETTE_SIZE);
    }
    return !out_hash || *out_hash != 0u;
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

static uint32_t dm2_v1_gdat_scene_draw_order_hash(
    const DM2_V1_GdatSceneM11CommandPlan *plan)
{
    uint32_t hash = 2166136261u;

    if (!plan || plan->draw_order[0] != 1u || plan->draw_order[1] != 0u ||
        plan->commands[0].field != DM2_GDAT_GFXSET_FLOOR ||
        plan->commands[1].field != DM2_GDAT_GFXSET_CEIL ||
        plan->rects[0].rect_number != DM2_V1_GDAT_SCENE_FLOOR_RECT_NUMBER ||
        plan->rects[1].rect_number != DM2_V1_GDAT_SCENE_CEILING_RECT_NUMBER) {
        return 0u;
    }
    for (size_t i = 0u; i < 2u; ++i) {
        const DM2_V1_GdatSceneM11Command *command =
            &plan->commands[plan->draw_order[i]];
        const DM2_V1_GdatSceneBlitRect *rect =
            &plan->rects[plan->draw_order[i]];

        if (!command->raw_hash || !command->decoded_hash ||
            !command->palette_hash || !command->geometry_hash) {
            return 0u;
        }
        hash = hash_bytes(hash, &plan->draw_order[i],
                          sizeof(plan->draw_order[i]));
        hash = hash_bytes(hash, &command->field, sizeof(command->field));
        hash = hash_bytes(hash, (const uint8_t *)&rect->rect_number,
                          sizeof(rect->rect_number));
        hash = hash_bytes(hash, (const uint8_t *)&command->raw_hash,
                          sizeof(command->raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->decoded_hash,
                          sizeof(command->decoded_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->palette_hash,
                          sizeof(command->palette_hash));
        hash = hash_bytes(hash, &command->palette_darkness,
                          sizeof(command->palette_darkness));
        hash = hash_bytes(hash, &command->palette_translation_field,
                          sizeof(command->palette_translation_field));
        hash = hash_bytes(hash,
                          (const uint8_t *)&command->palette_translation_hash,
                          sizeof(command->palette_translation_hash));
        hash = hash_bytes(hash,
                          (const uint8_t *)&command->palette_light_receipt_hash,
                          sizeof(command->palette_light_receipt_hash));
        hash = hash_bytes(hash,
                          (const uint8_t *)&command->palette_transform_hash,
                          sizeof(command->palette_transform_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->geometry_hash,
                          sizeof(command->geometry_hash));
    }
    return hash;
}

int dm2_v1_gdat_scene_m11_command_plan_draw_order_valid(
    const DM2_V1_GdatSceneM11CommandPlan *plan)
{
    return plan && plan->draw_order_hash != 0u &&
        plan->draw_order_hash == dm2_v1_gdat_scene_draw_order_hash(plan);
}

int dm2_v1_gdat_scene_m11_command_plan_refresh_draw_order(
    DM2_V1_GdatSceneM11CommandPlan *plan)
{
    uint32_t hash;

    if (!plan || !plan->valid || plan->command_hash == 0u) return 0;
    hash = dm2_v1_gdat_scene_draw_order_hash(plan);
    if (hash == 0u) return 0;
    plan->draw_order_hash = hash;
    return 1;
}

int dm2_v1_gdat_scene_m11_plane_palette_darkness(
    uint8_t field, uint8_t c_light_parameter, uint8_t *out_darkness)
{
    if (!out_darkness) return 0;
    *out_darkness = 0u;
    if ((field != DM2_GDAT_GFXSET_FLOOR && field != DM2_GDAT_GFXSET_CEIL) ||
        c_light_parameter > 64u) {
        return 0;
    }
    /* SkWinCore.cpp::_32cb_0804: _4976_4226 starts {0,0,12,28,46}; the
     * floor/ceiling fields are classes 0/1, hence 64-((64-0)*(64-p)>>6)=p. */
    *out_darkness = c_light_parameter;
    return 1;
}

int dm2_v1_gdat_scene_m11_plane_translation_field(
    uint8_t field, int movement_active, uint8_t *out_field)
{
    if (!out_field) return 0;
    *out_field = 0u;
    if (field != DM2_GDAT_GFXSET_FLOOR && field != DM2_GDAT_GFXSET_CEIL) {
        return 0;
    }
    /* SkWinCore.cpp::_32cb_0804: moving cls4 >= 0 reads _4976_4221[cls4]
     * then increments cls4 by nine before QUERY_GDAT_ENTRY_IF_LOADABLE. */
    *out_field = movement_active ? (uint8_t)(field + 9u) : field;
    return 1;
}

int dm2_v1_gdat_scene_m11_translate_palette(
    uint8_t *palette, uint32_t palette_count,
    const uint8_t *translation, size_t translation_size,
    uint32_t *out_translation_hash)
{
    uint32_t hash;

    if (out_translation_hash) *out_translation_hash = 0u;
    if (!palette || palette_count == 0u || palette_count > 256u ||
        !translation || translation_size < 256u) {
        return 0;
    }
    hash = hash_bytes(2166136261u, translation, 256u);
    if (hash == 0u) return 0;
    for (uint32_t i = 0u; i < palette_count; ++i) {
        palette[i] = translation[palette[i]];
    }
    if (out_translation_hash) *out_translation_hash = hash;
    return 1;
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

static int dm2_v1_c_light_m11_receipt_build_bound(
    const DM2_V1_GdatSceneLightM11Receipt *scene,
    const DM2_V1_CLightSourceState *source,
    uint32_t map_descriptor_hash,
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
    candidate.map_descriptor_hash = map_descriptor_hash;
    candidate.source_state_hash = source->source_state_hash;
    hash = hash_bytes(hash, &candidate.graphicsset,
                      sizeof(candidate.graphicsset));
    hash = hash_bytes(hash, &candidate.light_level,
                      sizeof(candidate.light_level));
    hash = hash_bytes(hash, &candidate.dynamic_map,
                      sizeof(candidate.dynamic_map));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.scene_control_hash,
                      sizeof(candidate.scene_control_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.map_descriptor_hash,
                      sizeof(candidate.map_descriptor_hash));
    hash = hash_bytes(hash, (const uint8_t *)&candidate.source_state_hash,
                      sizeof(candidate.source_state_hash));
    if (hash == 0u) return 0;
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
    return dm2_v1_c_light_m11_receipt_build_bound(
        scene, source, 0u, out_receipt);
}

int dm2_v1_c_light_m11_receipt_build_for_map(
    const DM2_V1_GdatSceneLightM11Receipt *scene,
    const DM2_V1_CLightMapDescriptorReceipt *map,
    const DM2_V1_CLightSourceState *source,
    DM2_V1_CLightM11Receipt *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!map || !map->valid || map->descriptor_hash == 0u ||
        map->dynamic_light > 1u || !source ||
        source->dynamic_map != map->dynamic_light) {
        return 0;
    }
    return dm2_v1_c_light_m11_receipt_build_bound(
        scene, source, map->descriptor_hash, out_receipt);
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
        !dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, DM2_GDAT_GFXSET_SCENE_FLAGS, &candidate.scene_flags)) return 0;
    /* skproject UPDATE_GFXSET admits the active GRAPHICSSET scene material
     * after SCENE_COLORKEY/SCENE_FLAGS plus its floor/ceiling IMG3 records.
     * Light control words are retained for c_light consumers when present,
     * but missing light/weather semantics must not block the source-owned
     * dungeon surface or trigger a borrowed graphics-set fallback. */
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_GFXSET_HIGHEST_LIGHT_LEVEL, &candidate.highest_light_level);
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_GFXSET_AMBIANT_DARKNESS, &candidate.ambient_darkness);
    if (candidate.ambient_darkness > 8u) return 0;
    /* skproject QUERY_GDAT_ENTRY_DATA_INDEX returns zero for a missing
     * dtWordValue entry. GRAPHICSSET 2 in the canonical PC corpus has no
     * AMBIANT_LIGHT row, so preserve that source zero without borrowing a
     * control value from another graphics set. */
    candidate.ambient_light = 0u;
    (void)dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_GFXSET_AMBIANT_LIGHT, &candidate.ambient_light);
    candidate.trim_wall_d1_present = dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_GFXSET_TRIM_WALL_D1, &candidate.trim_wall_d1);
    candidate.trim_wall_d2_present = dm2_v1_asset_load_word_value(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
        DM2_GDAT_GFXSET_TRIM_WALL_D2, &candidate.trim_wall_d2);
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
        int palette_ok;
        DM2_V1_GdatGfxRawMaterialReceipt material;
        uint16_t raw_index;
        command->field = fields[i];
        raw = dm2_v1_asset_load_sized(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
                                      graphicsset, fields[i], &raw_size);
        command->pixels = dm2_v1_asset_load_image_field(loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, fields[i], &width,
            &height, &command->format);
        palette_ok = dm2_v1_gdat_scene_image_local_palette(loader, graphicsset,
            fields[i], command->palette16, &command->palette_hash);
        if (!raw || raw_size == 0u || !command->pixels || width <= 0 || height <= 0 ||
            command->format == DM2_IMG_FMT_UNKNOWN ||
            !palette_ok ||
            !command->palette_hash) {
            dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0;
        }
        if (!scene_source_raw_index(loader, raw, raw_size, &raw_index) ||
            !dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
                loader, raw_index, &material) ||
            material.source_bytes != raw ||
            material.source_byte_count != raw_size ||
            !material.receipt_hash) {
            dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0;
        }
        command->width = (uint16_t)width; command->height = (uint16_t)height;
        command->material_raw_index = material.raw_index;
        command->material_source_bytes = material.source_bytes;
        command->material_source_byte_count = material.source_byte_count;
        command->material_receipt_hash = material.receipt_hash;
        command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
        command->decoded_hash = dm2_v1_gdat_scene_m11_command_pixel_hash(command);
        if (!command->raw_hash || !command->decoded_hash ||
            !command->material_source_bytes ||
            !command->material_source_byte_count ||
            !command->material_receipt_hash) {
            dm2_v1_gdat_scene_m11_command_plan_free(&candidate); return 0;
        }
        hash = hash_bytes(hash, (const uint8_t *)&command->raw_hash, sizeof(command->raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->decoded_hash,
                          sizeof(command->decoded_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->palette_hash, sizeof(command->palette_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->material_receipt_hash,
                          sizeof(command->material_receipt_hash));
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
    /* c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT: ceiling 0x2bc, then floor 0x2bd. */
    candidate.draw_order[0] = 1u;
    candidate.draw_order[1] = 0u;
    candidate.draw_order_hash = dm2_v1_gdat_scene_draw_order_hash(&candidate);
    if (!candidate.draw_order_hash) {
        dm2_v1_gdat_scene_m11_command_plan_free(&candidate);
        return 0;
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
    hash = hash_bytes(hash, (const uint8_t *)&candidate.draw_order_hash,
                      sizeof(candidate.draw_order_hash));
    hash ^= candidate.scene_colorkey; hash *= 16777619u;
    hash ^= candidate.scene_flags; hash *= 16777619u;
    hash ^= candidate.ambient_light; hash *= 16777619u;
    hash ^= candidate.highest_light_level; hash *= 16777619u;
    hash ^= candidate.ambient_darkness; hash *= 16777619u;
    hash ^= candidate.trim_wall_d1; hash *= 16777619u;
    hash ^= candidate.trim_wall_d2; hash *= 16777619u;
    hash ^= candidate.trim_wall_d1_present; hash *= 16777619u;
    hash ^= candidate.trim_wall_d2_present; hash *= 16777619u;
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
