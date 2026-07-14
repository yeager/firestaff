#include "dm2_v1_gdat_door_overlay_m11_command.h"

#include "dm2_v1_viewport_renderer.h"

#include <string.h>

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

void dm2_v1_gdat_door_overlay_m11_command_plan_free(
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan)
{
    if (!plan) return;
    for (int i = 0; i < DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX; ++i)
        dm2_v1_asset_free_pixels(plan->commands[i].pixels);
    memset(plan, 0, sizeof(*plan));
}

static int add_overlay(const DM2_V1_AssetLoader *loader,
                       DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
                       const DM2_V1_DoorRender *door, int kind)
{
    DM2_V1_GdatDoorOverlayM11Command *command;
    int gdat_index = kind == DM2_V1_GDAT_DOOR_OVERLAY_ORNATE
        ? door->ornate_gdat_index : door->destroyed_mask_gdat_index;
    int category = kind == DM2_V1_GDAT_DOOR_OVERLAY_ORNATE
        ? DM2_GDAT_CATEGORY_DOOR_GFX : DM2_GDAT_CATEGORY_DOORS;
    int index = kind == DM2_V1_GDAT_DOOR_OVERLAY_ORNATE
        ? door->ornament_index : door->door_gfx_index;
    int field = dm2_v1_viewport_door_panel_field_for_square(door->view_square);
    const uint8_t *raw;
    size_t raw_size = 0u;
    int width = 0, height = 0;

    if (!gdat_index) return 1;
    if (plan->command_count >= DM2_V1_GDAT_DOOR_OVERLAY_M11_COMMAND_MAX ||
        index < 0 || index > 0xff || field < 0) return 0;
    command = &plan->commands[plan->command_count];
    raw = dm2_v1_asset_load_sized(loader, category, index, field, &raw_size);
    command->pixels = dm2_v1_asset_load_image_field(loader, category, index,
                                                      field, &width, &height, NULL);
    if (!raw || !raw_size || !command->pixels || width <= 0 || height <= 0 ||
        !dm2_v1_asset_load_image_local_palette(loader, category, index, field,
                                                command->palette16,
                                                &command->palette_hash) ||
        !command->palette_hash) return 0;
    command->gdat_index = gdat_index;
    command->view_square = (uint8_t)door->view_square;
    command->kind = (uint8_t)kind;
    command->category = (uint8_t)category;
    command->entry_index = (uint8_t)index;
    command->field = (uint8_t)field;
    command->width = (uint16_t)width;
    command->height = (uint16_t)height;
    command->raw_hash = hash_bytes(2166136261u, raw, raw_size);
    return command->raw_hash != 0u && ++plan->command_count;
}

int dm2_v1_gdat_door_overlay_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader, const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan)
{
    DM2_V1_GdatDoorOverlayM11CommandPlan candidate;
    uint32_t hash = 2166136261u;
    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    memset(&candidate, 0, sizeof(candidate));
    if (!loader || !door_plan || !dm2_v1_asset_loader_verify(loader)) return 0;
    for (int i = 0; i < door_plan->door_count; ++i) {
        if (!add_overlay(loader, &candidate, &door_plan->doors[i],
                         DM2_V1_GDAT_DOOR_OVERLAY_ORNATE) ||
            !add_overlay(loader, &candidate, &door_plan->doors[i],
                         DM2_V1_GDAT_DOOR_OVERLAY_DESTROYED_MASK)) goto fail;
    }
    if (!candidate.command_count) goto fail;
    for (int i = 0; i < candidate.command_count; ++i) {
        hash = hash_bytes(hash, (const uint8_t *)&candidate.commands[i].raw_hash,
                          sizeof(candidate.commands[i].raw_hash));
        hash = hash_bytes(hash, (const uint8_t *)&candidate.commands[i].palette_hash,
                          sizeof(candidate.commands[i].palette_hash));
    }
    candidate.command_hash = hash ? hash : 1u;
    candidate.valid = 1;
    *out_plan = candidate;
    return 1;
fail:
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&candidate);
    return 0;
}
