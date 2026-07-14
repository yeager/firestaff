#include "dm2_v1_gdat_wall_m11_command.h"

#include "dm2_v1_viewport_renderer.h"

#include <string.h>

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

void dm2_v1_gdat_wall_m11_command_plan_free(DM2_V1_GdatWallM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < DM2_V1_GDAT_WALL_M11_COMMAND_MAX; ++i)
        dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

int dm2_v1_gdat_wall_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, uint8_t graphicsset,
    DM2_V1_GdatWallM11CommandPlan *out_plan)
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
        const DM2_WallFrame *frame;
        const uint8_t *raw;
        size_t raw_size = 0u;
        int width = 0, height = 0;
        if (field < DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST) continue;
        if (candidate.command_count >= DM2_V1_GDAT_WALL_M11_COMMAND_MAX) goto fail;
        frame = dm2_v1_get_wall_frame(square);
        if (!frame || frame->byte_width <= 0 || frame->height <= 0 ||
            frame->right_x < frame->left_x || frame->bottom_y < frame->top_y ||
            frame->blit_x < 0 || frame->blit_y < 0) goto fail;
        command = &candidate.commands[candidate.command_count];
        raw = dm2_v1_asset_load_sized(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
                                      graphicsset, field, &raw_size);
        command->pixels = dm2_v1_asset_load_image_field(
            loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, field,
            &width, &height, NULL);
        if (!raw || !raw_size || !command->pixels || width <= 0 || height <= 0 ||
            !dm2_v1_asset_load_image_local_palette(
                loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, field,
                command->palette16, &command->palette_hash) || !command->palette_hash) goto fail;
        command->view_square = (uint8_t)square;
        command->field = (uint8_t)field;
        command->width = (uint16_t)width;
        command->height = (uint16_t)height;
        command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
        command->decoded_hash = hash_bytes(
            2166136261u, command->pixels, (size_t)width * (size_t)height);
        command->source_x = (uint16_t)frame->blit_x;
        command->source_y = (uint16_t)frame->blit_y;
        command->source_width = (uint16_t)frame->byte_width;
        command->source_height = (uint16_t)frame->height;
        command->destination_x = (uint16_t)frame->left_x;
        command->destination_y = (uint16_t)frame->top_y;
        command->destination_width =
            (uint16_t)(frame->right_x - frame->left_x + 1);
        command->destination_height =
            (uint16_t)(frame->bottom_y - frame->top_y + 1);
        command->geometry_hash = hash_bytes(
            2166136261u, (const uint8_t *)&command->source_x,
            sizeof(command->source_x) + sizeof(command->source_y) +
                sizeof(command->source_width) + sizeof(command->source_height) +
                sizeof(command->destination_x) + sizeof(command->destination_y) +
                sizeof(command->destination_width) + sizeof(command->destination_height));
        if (!command->raw_hash || !command->decoded_hash ||
            !command->destination_width || !command->destination_height ||
            !command->geometry_hash) goto fail;
        hash = hash_bytes(hash, (const uint8_t *)&command->raw_hash, sizeof(command->raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->decoded_hash,
                          sizeof(command->decoded_hash));
        hash = hash_bytes(hash, (const uint8_t *)&command->palette_hash, sizeof(command->palette_hash));
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
