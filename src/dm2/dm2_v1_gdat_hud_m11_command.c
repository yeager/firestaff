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

static int dm2_v1_gdat_hud_source_raw_index(
    const DM2_V1_AssetLoader *loader,
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
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
            (const uint8_t *)&command->material_raw_index,
            sizeof(command->material_raw_index));
        hash = dm2_v1_gdat_hud_hash_bytes(hash,
            (const uint8_t *)&command->material_receipt_hash,
            sizeof(command->material_receipt_hash));
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

uint32_t dm2_v1_gdat_hud_m11_command_plan_hash(
    const DM2_V1_GdatHudM11CommandPlan *plan)
{
    return dm2_v1_gdat_hud_command_hash(plan);
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

int dm2_v1_gdat_hud_summary_m11_receipt(
    const DM2_V1_GdatHudM11CommandPlan *plan, int vb_144, int field,
    DM2_V1_GdatHudSummaryM11Receipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gui_draw.cpp:926-942: QUERY_GDAT_SUMMARY_IMAGE(1,vb_144,field)
     * reaches QUERY_PICST_IT; this admits only an already exact HUD command. */
    if (!plan || !plan->valid || !plan->command_hash || vb_144 < 0 ||
        vb_144 > 0xff || field < 0 || field > 0xff) return 0;
    for (i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatHudM11Command *c = &plan->commands[i];
        if (c->gdat_category != 1 || c->gdat_index != vb_144 ||
            c->gdat_field != field || !c->pixels || c->width <= 0 ||
            c->height <= 0 || !c->decoded_hash || !c->palette_hash ||
            !c->destination_rect_id || !c->destination_table_hash) continue;
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&c->decoded_hash,
                                           sizeof(c->decoded_hash));
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&c->palette_hash,
                                           sizeof(c->palette_hash));
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&c->destination,
                                           sizeof(c->destination));
        hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&c->destination_table_hash,
                                           sizeof(c->destination_table_hash));
        out_receipt->valid = 1; out_receipt->no_draw = 1;
        out_receipt->category = 1u; out_receipt->index = (uint8_t)vb_144;
        out_receipt->field = (uint8_t)field; out_receipt->decoded_hash = c->decoded_hash;
        out_receipt->palette_hash = c->palette_hash;
        out_receipt->destination_rect_id = c->destination_rect_id;
        out_receipt->destination_hash = c->destination_table_hash;
        out_receipt->identity_hash = hash ? hash : 1u;
        return 1;
    }
    return 0;
}

int dm2_v1_gdat_hud_picst_transform_receipt(
    const DM2_V1_GdatHudSummaryM11Receipt *summary, int source_value,
    DM2_V1_GdatHudPicstTransformReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gui_draw.cpp:926-942: for 0..0x28, QUERY_PICST_IT receives
     * scale Y=0x35 and X=0x1f (<=0x0f) or 0x2f (>0x0f). */
    if (!summary || !summary->valid || !summary->no_draw ||
        !summary->identity_hash || !summary->decoded_hash ||
        !summary->palette_hash || !summary->destination_rect_id ||
        !summary->destination_hash || source_value < 0 || source_value > 0x28)
        return 0;
    out_receipt->scale_x = source_value <= 0x0f ? 0x1fu : 0x2fu;
    out_receipt->scale_y = 0x35u;
    out_receipt->destination_rect_id = summary->destination_rect_id;
    out_receipt->summary_identity_hash = summary->identity_hash;
    out_receipt->destination_hash = summary->destination_hash;
    hash = dm2_v1_gdat_hud_hash_bytes(hash,
        (const uint8_t *)&summary->identity_hash, sizeof(summary->identity_hash));
    hash = dm2_v1_gdat_hud_hash_bytes(hash,
        (const uint8_t *)&out_receipt->scale_x, sizeof(out_receipt->scale_x));
    hash = dm2_v1_gdat_hud_hash_bytes(hash,
        (const uint8_t *)&out_receipt->scale_y, sizeof(out_receipt->scale_y));
    out_receipt->identity_hash = hash ? hash : 1u;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    return 1;
}

static int dm2_v1_gdat_hud_stretched_size(int source_size, uint16_t scale,
                                           uint16_t *out_size)
{
    int64_t scaled;
    if (!out_size || source_size <= 0 || scale == 0u) return 0;
    /* SKWIN c_gfx_blit.cpp:886-894: (size * scale + scale / 2) >> 6. */
    scaled = (int64_t)source_size * scale + (scale >> 1);
    scaled >>= 6;
    if (scaled <= 0 || scaled > UINT16_MAX) return 0;
    *out_size = (uint16_t)scaled;
    return 1;
}

static int dm2_v1_gdat_hud_stretch256_index(int source_size, int target_size,
                                             int target_offset, int *out_index)
{
    int64_t source_fraction;
    if (!out_index || source_size <= 0 || target_size <= 0 ||
        target_offset < 0 || target_offset >= target_size) return 0;
    /* SKWIN c_gfx_blit.cpp:1004-1063 starts each axis at half a source
     * fraction, yielding floor((2*n + 1) * source / (2 * target)). */
    source_fraction = (int64_t)(2 * target_offset + 1) * source_size;
    *out_index = (int)(source_fraction / (2 * target_size));
    return *out_index >= 0 && *out_index < source_size;
}

int dm2_v1_gdat_hud_picst_draw_indexed(
    const DM2_V1_GdatHudM11CommandPlan *plan,
    const DM2_V1_GdatHudSummaryM11Receipt *summary,
    const DM2_V1_GdatHudPicstTransformReceipt *transform,
    int vb_144,
    int field,
    uint8_t *target_pixels,
    int target_width,
    int target_height,
    uint8_t out_palette16[16],
    DM2_V1_GdatHudPicstDrawReceipt *out_receipt)
{
    const DM2_V1_GdatHudM11Command *command = NULL;
    uint16_t scaled_width;
    uint16_t scaled_height;
    uint32_t hash = 2166136261u;
    int i;
    int y;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* SKWIN c_gui_draw.cpp:926-942 feeds SUMMARY_IMAGE through
     * QUERY_PICST_IT, c_image.cpp:106-200 scales 8-bit pixels with
     * stretch256, then c_image.cpp:70-96/DRAW_PICST places it in rect 57. */
    if (!plan || !plan->valid || !plan->command_hash || !summary ||
        !transform || !target_pixels || !out_palette16 || target_width <= 0 ||
        target_height <= 0 || !summary->valid || !summary->no_draw ||
        !transform->valid || !transform->no_draw ||
        summary->category != 1u || summary->index != (uint8_t)vb_144 ||
        summary->field != (uint8_t)field || !summary->identity_hash ||
        !transform->identity_hash ||
        transform->summary_identity_hash != summary->identity_hash ||
        transform->destination_rect_id != summary->destination_rect_id ||
        transform->destination_hash != summary->destination_hash) return 0;

    for (i = 0; i < plan->command_count; ++i) {
        const DM2_V1_GdatHudM11Command *candidate = &plan->commands[i];
        if (candidate->gdat_category == 1 && candidate->gdat_index == vb_144 &&
            candidate->gdat_field == field) {
            command = candidate;
            break;
        }
    }
    if (!command || command->format != DM2_IMG_FMT_U4 || !command->pixels ||
        command->width <= 0 || command->height <= 0 || !command->decoded_hash ||
        !command->palette_hash || !command->raw_hash ||
        !command->material_source_bytes || !command->material_source_byte_count ||
        !command->material_receipt_hash || !command->destination_rect_id ||
        !command->destination_table_hash ||
        dm2_v1_gdat_hud_m11_command_pixel_hash(command) != command->decoded_hash ||
        dm2_v1_gdat_hud_hash_bytes(2166136261u, command->palette16,
                                   sizeof(command->palette16)) != command->palette_hash ||
        command->decoded_hash != summary->decoded_hash ||
        command->palette_hash != summary->palette_hash ||
        command->destination_rect_id != summary->destination_rect_id ||
        command->destination_table_hash != summary->destination_hash ||
        !dm2_v1_gdat_hud_stretched_size(command->width, transform->scale_x,
                                         &scaled_width) ||
        !dm2_v1_gdat_hud_stretched_size(command->height, transform->scale_y,
                                         &scaled_height)) return 0;

    /* This admitted branch has a fully resolved destination. Source clips it
     * through QUERY_BLIT_RECT; until that table is separately received, only
     * the un-clipped exact-size case is safe to materialize. */
    if (command->destination.w != (int)scaled_width ||
        command->destination.h != (int)scaled_height || command->destination.x < 0 ||
        command->destination.y < 0 || command->destination.x > target_width - scaled_width ||
        command->destination.y > target_height - scaled_height) return 0;

    for (y = 0; y < scaled_height; ++y) {
        int source_y;
        int x;
        if (!dm2_v1_gdat_hud_stretch256_index(command->height, scaled_height,
                                               y, &source_y)) return 0;
        for (x = 0; x < scaled_width; ++x) {
            int source_x;
            if (!dm2_v1_gdat_hud_stretch256_index(command->width, scaled_width,
                                                   x, &source_x)) return 0;
            target_pixels[(command->destination.y + y) * target_width +
                          command->destination.x + x] =
                command->pixels[source_y * command->width + source_x];
        }
    }
    memcpy(out_palette16, command->palette16, sizeof(command->palette16));
    hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&summary->identity_hash,
                                      sizeof(summary->identity_hash));
    hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&transform->identity_hash,
                                      sizeof(transform->identity_hash));
    hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&command->decoded_hash,
                                      sizeof(command->decoded_hash));
    hash = dm2_v1_gdat_hud_hash_bytes(hash, (const uint8_t *)&command->palette_hash,
                                      sizeof(command->palette_hash));
    hash = dm2_v1_gdat_hud_hash_bytes(hash,
                                      (const uint8_t *)&command->destination_table_hash,
                                      sizeof(command->destination_table_hash));
    out_receipt->valid = 1;
    out_receipt->drawn = 1;
    out_receipt->destination_rect_id = command->destination_rect_id;
    out_receipt->width = scaled_width;
    out_receipt->height = scaled_height;
    out_receipt->decoded_hash = command->decoded_hash;
    out_receipt->palette_hash = command->palette_hash;
    out_receipt->summary_identity_hash = summary->identity_hash;
    out_receipt->transform_identity_hash = transform->identity_hash;
    out_receipt->destination_hash = command->destination_table_hash;
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
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
    DM2_V1_GdatGfxRawMaterialReceipt material;
    uint16_t raw_index;

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
    if (!dm2_v1_gdat_hud_source_raw_index(loader, raw, raw_size,
                                           &raw_index) ||
        !dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
            loader, raw_index, &material) ||
        material.source_bytes != raw ||
        material.source_byte_count != raw_size ||
        !material.receipt_hash) {
        return 0;
    }
    command->material_raw_index = material.raw_index;
    command->material_source_bytes = material.source_bytes;
    command->material_source_byte_count = material.source_byte_count;
    command->material_receipt_hash = material.receipt_hash;
    command->pixels = dm2_v1_asset_load_image_field(
        loader, command->gdat_category, command->gdat_index,
        command->gdat_field, &command->width, &command->height,
        &command->format);
    if (!command->pixels || command->width <= 0 || command->height <= 0 ||
        command->format == DM2_IMG_FMT_UNKNOWN || command->palette_hash == 0u ||
        !command->material_source_bytes ||
        command->material_source_byte_count == 0u ||
        command->material_receipt_hash == 0u) {
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
    int is_outdoor,
    DM2_V1_GdatHudM11CommandPlan *out_plan)
{
    DM2_V1_HudChromeRenderPlan chrome;
    int i;
    int expected_count = is_outdoor ? 8 : 9;

    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!loader || !dm2_v1_asset_loader_verify(loader) ||
        !dm2_v1_viewport_build_hud_chrome_plan(is_outdoor ? 1 : 0, &chrome) ||
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
    if (!is_outdoor &&
        (!dm2_v1_gdat_hud_add_command(loader, out_plan,
                DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL,
                chrome.portrait_panel_gdat_index,
                &chrome.portrait_panel_rect) ||
         out_plan->command_count != expected_count)) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    if (out_plan->command_count != expected_count) {
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
    int portrait_count = 0;

    if (!party || !dm2_v1_gdat_hud_m11_command_plan_build(loader, 0, out_plan) ||
        !dm2_v1_viewport_build_hud_chrome_plan_for_party(0, party, &chrome)) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    for (slot = 0; slot < chrome.champion_slot_count; ++slot) {
        const DM2_V1_HudChampionSlotRender *champ = &chrome.champion_slots[slot];
        if (!champ->occupied) {
            continue;
        }
        if (!champ->portrait_type_source_bound ||
            !dm2_v1_gdat_hud_add_command(loader, out_plan,
                DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT,
                champ->portrait_index, &champ->portrait_rect)) {
            dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
            return 0;
        }
        ++portrait_count;
    }
    /* DRAW_CHAMPION_PICTURE runs once per active squad member. Keep the
     * source chrome plus exactly the admitted HeroType images. */
    if (out_plan->command_count != 9 + portrait_count) {
        dm2_v1_gdat_hud_m11_command_plan_free(out_plan);
        return 0;
    }
    out_plan->command_hash = dm2_v1_gdat_hud_command_hash(out_plan);
    return 1;
}

int dm2_v1_gdat_hud_m11_command_plan_bind_portrait_destinations(
    DM2_V1_GdatHudM11CommandPlan *plan,
    const DM2_V1_HudPartyState *party,
    const DM2_V1_ViewportRect portrait_destinations[4],
    uint32_t source_table_hash)
{
    int slot;
    int command_index = 9;
    if (!plan || !plan->valid || plan->command_count < 9 ||
        plan->command_count > DM2_V1_GDAT_HUD_M11_COMMAND_MAX ||
        !party || !portrait_destinations ||
        source_table_hash == 0u) return 0;
    for (slot = 0; slot < 4; ++slot) {
        DM2_V1_GdatHudM11Command *command;
        const DM2_V1_ViewportRect *destination = &portrait_destinations[slot];
        if (slot >= party->champion_count || !party->champions[slot].occupied) {
            continue;
        }
        if (!party->champions[slot].portrait_type_source_bound) {
            dm2_v1_gdat_hud_m11_command_plan_free(plan);
            return 0;
        }
        if (command_index >= plan->command_count) break;
        command = &plan->commands[command_index];
        if (command->kind != DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT ||
            command->gdat_category != DM2_GDAT_CATEGORY_CHAMPIONS ||
            command->gdat_index != party->champions[slot].portrait_index ||
            command->gdat_field != 0 ||
            destination->x < 0 || destination->y < 0 || destination->w <= 0 ||
            destination->h <= 0 || destination->x + destination->w > DM2_VP_WIDTH ||
            destination->y + destination->h > DM2_VP_HEIGHT) {
            dm2_v1_gdat_hud_m11_command_plan_free(plan);
            return 0;
        }
        command->destination = *destination;
        command->destination_rect_id = (uint16_t)(173 + slot);
        command->destination_table_hash = source_table_hash;
        ++command_index;
    }
    if (command_index != plan->command_count) {
        dm2_v1_gdat_hud_m11_command_plan_free(plan);
        return 0;
    }
    plan->command_hash = dm2_v1_gdat_hud_command_hash(plan);
    if (plan->command_hash == 0u) {
        dm2_v1_gdat_hud_m11_command_plan_free(plan);
        return 0;
    }
    return 1;
}
