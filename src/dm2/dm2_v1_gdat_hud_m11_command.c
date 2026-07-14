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

uint32_t dm2_v1_gdat_hud_m11_command_pixel_hash(
    const DM2_V1_GdatHudM11Command *command)
{
    if (!command || !command->pixels || command->width <= 0 ||
        command->height <= 0) return 0u;
    return dm2_v1_gdat_hud_hash_bytes(2166136261u, command->pixels,
                                      (size_t)command->width * command->height);
}

static uint32_t dm2_v1_gdat_hud_command_hash(
    const DM2_V1_GdatHudM11CommandPlan *plan)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!plan || plan->command_count <= 0 ||
        plan->command_count > DM2_V1_GDAT_HUD_M11_COMMAND_MAX) return 0u;
    for (i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &plan->commands[i];
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&command->kind,
                                           sizeof(command->kind));
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&command->raw_hash,
                                           sizeof(command->raw_hash));
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
            (const uint8_t *)&command->decoded_hash,
            sizeof(command->decoded_hash));
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&command->palette_hash,
                                           sizeof(command->palette_hash));
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&command->destination,
                                           sizeof(command->destination));
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
            (const uint8_t *)&command->destination_rect_id,
            sizeof(command->destination_rect_id));
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
            (const uint8_t *)&command->destination_table_hash,
            sizeof(command->destination_table_hash));
    }
    return hash ? hash : 1u;
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
    command->viewport_gdat_index = kind ==
        DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT
        ? dm2_v1_viewport_hud_portrait_graphic_index(viewport_gdat_index)
        : viewport_gdat_index;
    command->destination = *destination;
    if (kind == DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT) {
        command->gdat_category = DM2_GDAT_CATEGORY_CHAMPIONS;
        command->gdat_index = viewport_gdat_index;
        command->gdat_field = DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD;
    } else if (!dm2_v1_boot_hud_core_asset_address(
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
    command->decoded_hash = dm2_v1_gdat_hud_m11_command_pixel_hash(command);
    if (command->raw_hash == 0u || command->decoded_hash == 0u) {
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
                &chrome.action_icons[i].fill_rect)) {
            dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
            return 0;
        }
    }
    if (!dm2_v1_gdat_hud_add_command(loader, out_plan,
            DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL,
            chrome.portrait_panel_gdat_index, &chrome.portrait_panel_rect) ||
        out_plan->command_count != 9) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    out_plan->command_hash = dm2_v1_gdat_hud_command_hash(out_plan);
    out_plan->valid = 1;
    return 1;
}

int dm2_v1_gdat_hud_m11_command_plan_build_for_party(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_HudPartyState *party,
    DM2_V1_GdatHudM11CommandPlan *out_plan)
{
    DM2_V1_HudChromeRenderPlan chrome;
    int slot;

    if (!party || !dm2_v1_gdat_hud_m11_command_plan_build(loader, out_plan) ||
        !dm2_v1_viewport_build_hud_chrome_plan_for_party(0, party, &chrome)) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    for (slot = 0; slot < chrome.champion_slot_count; ++slot) {
        const DM2_V1_HudChampionSlotRender *champ = &chrome.champion_slots[slot];
        if (!champ->occupied || !champ->portrait_type_source_bound ||
            !dm2_v1_gdat_hud_add_command(loader, out_plan,
                DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT,
                champ->portrait_index, &champ->portrait_rect)) {
            dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
            return 0;
        }
    }
    if (out_plan->command_count != 13) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    out_plan->command_hash = dm2_v1_gdat_hud_command_hash(out_plan);
    return 1;
}

int dm2_v1_gdat_hud_m11_command_plan_bind_portrait_destinations(
    DM2_V1_GdatHudM11CommandPlan *plan,
    const DM2_V1_ViewportRect portrait_destinations[4],
    uint32_t source_table_hash)
{
    int slot;
    if (!plan || !plan->valid || plan->command_count !=
        DM2_V1_GDAT_HUD_M11_COMMAND_MAX || !portrait_destinations ||
        source_table_hash == 0u) return 0;
    for (slot = 0; slot < 4; ++slot) {
        DM2_V1_GdatHudM11Command *command = &plan->commands[9 + slot];
        const DM2_V1_ViewportRect *destination = &portrait_destinations[slot];
        if (command->kind != DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT ||
            command->gdat_category != DM2_GDAT_CATEGORY_CHAMPIONS ||
            command->gdat_index != slot || command->gdat_field != 0 ||
            destination->x < 0 || destination->y < 0 || destination->w <= 0 ||
            destination->h <= 0 || destination->x + destination->w > DM2_VP_WIDTH ||
            destination->y + destination->h > DM2_VP_HEIGHT) {
            dm2_v1_gdat_hud_m11_command_plan_free(plan);
            return 0;
        }
        command->destination = *destination;
        command->destination_rect_id = (uint16_t)(173 + slot);
        command->destination_table_hash = source_table_hash;
    }
    plan->command_hash = dm2_v1_gdat_hud_command_hash(plan);
    if (plan->command_hash == 0u) {
        dm2_v1_gdat_hud_m11_command_plan_free(plan);
        return 0;
    }
    return 1;
}
