#include "dm2_v1_gdat_scene_m11_command.h"

#include <string.h>

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0u; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
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
