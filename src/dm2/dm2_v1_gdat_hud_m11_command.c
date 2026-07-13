/* skproject/SKWIN/SkWinCore.cpp loads INTERFACE_GENERAL images through
 * LOAD_GDAT_INTERFACE_00_02 before c_gui_vp draws HUD chrome. M11 receives
 * only fully decoded, exact-source commands; it cannot request a substitute. */

#include "dm2_v1_gdat_hud_m11_command.h"

#include "dm2_v1_boot.h"

#include <stdlib.h>
#include <string.h>

static uint32_t dm2_v1_gdat_hud_hash_bytes(uint32_t hash,
                                            const uint8_t *bytes,
                                            size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

void dm2_v1_gdat_hud_m11_command_plan_free(
    DM2_V1_GdatHudM11CommandPlan *plan)
{
    int i;
    if (!plan) return;
    for (i = 0; i < DM2_V1_GDAT_HUD_M11_COMMAND_MAX; ++i) {
        dm2_v1_asset_free_pixels(plan->commands[i].pixels);
        plan->commands[i].pixels = NULL;
    }
    memset(plan, 0, sizeof(*plan));
}

static int dm2_v1_gdat_hud_add_command(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatHudM11CommandPlan *plan,
    int kind,
    int viewport_gdat_index,
    const DM2_V1_ViewportRect *destination)
{
    DM2_V1_GdatHudM11Command *command;
    const uint8_t *raw;
    size_t raw_size = 0u;
    int logical_field;

    if (!loader || !plan || !destination || plan->command_count < 0 ||
        plan->command_count >= DM2_V1_GDAT_HUD_M11_COMMAND_MAX) {
        return 0;
    }
    logical_field = DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE -
        viewport_gdat_index;

    command = &plan->commands[plan->command_count];
    memset(command, 0, sizeof(*command));
    command->kind = kind;
    command->viewport_gdat_index = viewport_gdat_index;
    command->destination = *destination;
    if (!dm2_v1_boot_hud_core_asset_address(
            logical_field,
            &command->gdat_category, &command->gdat_index,
            &command->gdat_field)) {
        return 0;
    }
    raw = dm2_v1_asset_load_sized(loader, command->gdat_category,
                                  command->gdat_index, command->gdat_field,
                                  &raw_size);
    if (!raw || raw_size == 0u || raw_size > UINT32_MAX ||
        !dm2_v1_asset_load_image_local_palette(
            loader, command->gdat_category, command->gdat_index,
            command->gdat_field, command->palette16, &command->palette_hash)) {
        return 0;
    }
    command->pixels = dm2_v1_asset_load_image_field(
        loader, command->gdat_category, command->gdat_index,
        command->gdat_field, &command->width, &command->height,
        &command->format);
    if (!command->pixels || command->width <= 0 || command->height <= 0 ||
        command->format == DM2_IMG_FMT_UNKNOWN || command->palette_hash == 0u) {
        dm2_v1_asset_free_pixels(command->pixels);
        command->pixels = NULL;
        return 0;
    }
    command->raw_byte_count = (uint32_t)raw_size;
    command->raw_hash = dm2_v1_gdat_hud_hash_bytes(2166136261u, raw, raw_size);
    if (command->raw_hash == 0u) {
        dm2_v1_asset_free_pixels(command->pixels);
        command->pixels = NULL;
        return 0;
    }
    ++plan->command_count;
    return 1;
}

int dm2_v1_gdat_hud_m11_command_plan_build(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatHudM11CommandPlan *out_plan)
{
    DM2_V1_HudChromeRenderPlan chrome;
    uint32_t hash = 2166136261u;
    int i;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!loader || !dm2_v1_asset_loader_verify(loader) ||
        !dm2_v1_viewport_build_hud_chrome_plan(0, &chrome) ||
        !dm2_v1_gdat_hud_add_command(loader, out_plan,
            DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR,
            chrome.top_bar_gdat_index, &chrome.top_bar_rect) ||
        !dm2_v1_gdat_hud_add_command(loader, out_plan,
            DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP,
            chrome.action_strip_gdat_index, &chrome.action_strip_rect) ||
        !dm2_v1_gdat_hud_add_command(loader, out_plan,
            DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX,
            chrome.gold_box_gdat_index, &chrome.gold_box_rect)) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    for (i = 0; i < chrome.action_icon_count; ++i) {
        if (!dm2_v1_gdat_hud_add_command(loader, out_plan,
                DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
                chrome.action_icons[i].gdat_index,
                &chrome.action_icons[i].frame_rect)) {
            dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
            return 0;
        }
    }
    if (!dm2_v1_gdat_hud_add_command(loader, out_plan,
            DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL,
            chrome.portrait_panel_gdat_index, &chrome.portrait_panel_rect) ||
        out_plan->command_count != DM2_V1_GDAT_HUD_M11_COMMAND_MAX) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    for (i = 0; i < out_plan->command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &out_plan->commands[i];
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
                                           (const uint8_t *)&command->kind,
                                           sizeof(command->kind));
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
                                           (const uint8_t *)&command->raw_hash,
                                           sizeof(command->raw_hash));
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
                                           (const uint8_t *)&command->palette_hash,
                                           sizeof(command->palette_hash));
    }
    out_plan->command_hash = hash ? hash : 1u;
    out_plan->valid = 1;
    return 1;
}
