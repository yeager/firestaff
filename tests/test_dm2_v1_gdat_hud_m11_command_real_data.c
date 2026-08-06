/* Canonical PC G1 GRAPHICS.DAT proof for the complete M11 HUD command plan. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_gdat_hud_m11_command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int unexpected_fetches;

static int unexpected_asset_fetch(void *user, int index,
                                  const uint8_t **pixels, int *width,
                                  int *height, int *stride)
{
    (void)user; (void)index; (void)pixels; (void)width; (void)height;
    (void)stride;
    ++unexpected_fetches;
    return -1;
}

static int unexpected_palette_fetch(void *user, int index, uint8_t palette[16],
                                    uint32_t *hash)
{
    (void)user; (void)index; (void)palette; (void)hash;
    ++unexpected_fetches;
    return -1;
}

int main(void)
{
    const char *home = getenv("HOME");
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[2048];
    char boot_root[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_BootProfile boot;
    DM2_V1_GdatHudM11CommandPlan plan;
    DM2_V1_GdatHudM11CommandPlan portrait_plan;
    DM2_V1_BootStartupMenuHudGdatReceipt menu_hud_gdat;
    DM2_V1_HudPartyState party;
    DM2_V1_ViewportState viewport;
    DM2_V1_InterfacePalette interface_palette;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int failures = 0;
    int expected_kind[DM2_V1_GDAT_HUD_M11_COMMAND_MAX] = {
        DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP,
        DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX,
        DM2_V1_GDAT_HUD_M11_COMMAND_PORTRAIT_PANEL
    };

    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/graphics.dat", root);
        snprintf(boot_root, sizeof(boot_root), "%s/..", root);
    } else if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm2/data/graphics.dat",
                 home);
        snprintf(boot_root, sizeof(boot_root), "%s/.firestaff/data/dm2",
                 home);
    } else {
        puts("SKIP: no DM2 data root");
        return 0;
    }
    if (!read_file(path, &graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 GRAPHICS.DAT");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    dm2_v1_boot_profile_init(&boot);
    memset(&plan, 0, sizeof(plan));
    memset(&portrait_plan, 0, sizeof(portrait_plan));
    memset(&menu_hud_gdat, 0, sizeof(menu_hud_gdat));
    memset(&party, 0, sizeof(party));
    memset(&interface_palette, 0, sizeof(interface_palette));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical DM2 GRAPHICS.DAT was not admitted\n", stderr);
        free(graphics);
        return 1;
    }
    if (dm2_v1_boot_scan_assets(&boot, boot_root) != 0 ||
        dm2_v1_boot_enter_game(&boot) != 0) {
        fputs("FAIL: canonical DM2 boot profile was not entered\n", stderr);
        dm2_v1_boot_cleanup(&boot);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (!dm2_v1_boot_gdat_hud_static_m11_command_plan(&boot, 0, &plan)) {
        fputs("FAIL: canonical HUD command layout was not source-bound\n", stderr);
        dm2_v1_gdat_hud_m11_command_plan_free(&plan);
        dm2_v1_boot_cleanup(&boot);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (!dm2_v1_boot_startup_menu_hud_gdat_receipt(&boot, &menu_hud_gdat) ||
        !menu_hud_gdat.valid ||
        !menu_hud_gdat.title_image_ready ||
        !menu_hud_gdat.menu_image_ready ||
        menu_hud_gdat.title_width != 320 ||
        menu_hud_gdat.title_height != 200 ||
        menu_hud_gdat.menu_width != 320 ||
        menu_hud_gdat.menu_height != 200 ||
        menu_hud_gdat.title_raw_hash == 0u ||
        menu_hud_gdat.title_pixel_hash == 0u ||
        menu_hud_gdat.menu_raw_hash == 0u ||
        menu_hud_gdat.menu_pixel_hash == 0u ||
        !menu_hud_gdat.pointer_layout_ready ||
        !menu_hud_gdat.new_game_click_ready ||
        !menu_hud_gdat.resume_click_surface_ready ||
        menu_hud_gdat.pointer_table_hash == 0u ||
        !menu_hud_gdat.interface_palette_ready ||
        menu_hud_gdat.interface_palette_hash == 0u ||
        !menu_hud_gdat.hud_static_plan_ready ||
        menu_hud_gdat.hud_static_command_count !=
            DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
        menu_hud_gdat.hud_static_plan_hash == 0u ||
        !menu_hud_gdat.hud_palette_ready ||
        menu_hud_gdat.receipt_hash == 0u) {
        fputs("FAIL: startup menu/HUD GDAT receipt was incomplete\n", stderr);
        ++failures;
    }
    if (!plan.valid || plan.command_count !=
            DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
        plan.command_hash == 0u) {
        ++failures;
    }
    for (int i = 0; i < plan.command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &plan.commands[i];
        if (command->kind != expected_kind[i] ||
            (i < DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT &&
             (command->gdat_category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
                       command->gdat_index < 2 || command->gdat_index > 6)) ||
            command->width <= 0 || command->height <= 0 || !command->pixels ||
            command->format == DM2_IMG_FMT_UNKNOWN || command->raw_hash == 0u ||
            command->decoded_hash == 0u || command->decoded_hash !=
                dm2_v1_gdat_hud_m11_command_pixel_hash(command) ||
            command->raw_byte_count == 0u || command->palette_hash == 0u ||
            !command->material_source_bytes ||
            command->material_source_byte_count != command->raw_byte_count ||
            command->material_receipt_hash == 0u ||
            command->destination.w <= 0 || command->destination.h <= 0) {
            ++failures;
        }
        printf("command=%d type=%d source=%d/%d/%d %dx%d dst=%d,%d %dx%d\n",
               i, command->kind, command->gdat_category, command->gdat_index,
               command->gdat_field, command->width, command->height,
               command->destination.x, command->destination.y,
               command->destination.w, command->destination.h);
    }
    party.champion_count = 1;
    party.champions[0].occupied = 1;
    party.champions[0].portrait_type_source_bound = 1;
    party.champions[0].portrait_index = 0xffu;
    if (!dm2_v1_gdat_hud_m11_command_plan_build_for_party(
            &loader, &party, &portrait_plan) ||
        portrait_plan.command_count !=
            DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT + 1 ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].kind !=
            DM2_V1_GDAT_HUD_M11_COMMAND_CHAMPION_PORTRAIT ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].gdat_category !=
            DM2_GDAT_CATEGORY_MISCELLANEOUS ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].gdat_index != 0xfe ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].gdat_field != 0xfe ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].width != 31 ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].height != 31 ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].raw_hash == 0u ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].decoded_hash == 0u ||
        portrait_plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT].palette_hash == 0u) {
        fputs("FAIL: HeroType 255 did not retain the original GDAT default\n",
              stderr);
        ++failures;
    }
    dm2_v1_gdat_hud_m11_command_plan_free(&portrait_plan);
    memset(&party, 0, sizeof(party));
    memset(framebuffer, 0, sizeof(framebuffer));
    unexpected_fetches = 0;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_hud_party(&viewport, &party);
    dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, unexpected_palette_fetch, NULL);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_hud_material_plan(&viewport, &plan);
    dm2_v1_render_ui_chrome(&viewport);
    /* HUD names still use the separately source-gated dt07 font route. The
     * image-family proof below excludes that known no-draw lookup. */
    if (viewport.asset_hud_core_drawn_count !=
            DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
        viewport.asset_hud_portrait_drawn_count != 0 ||
        viewport.gdat_hud_material_plan_consumed_count !=
            DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
        viewport.fallback_hud_core_drawn_count != 0 ||
        viewport.fallback_hud_portrait_drawn_count != 0 ||
        !viewport.last_hud_top_bar_material_request.valid ||
        viewport.last_hud_top_bar_material_request.indexed_pixels !=
            plan.commands[0].pixels ||
        viewport.last_hud_top_bar_material_request.width !=
            plan.commands[0].width ||
        viewport.last_hud_top_bar_material_request.height !=
            plan.commands[0].height ||
        viewport.last_hud_top_bar_material_request.stride !=
            plan.commands[0].width ||
        !viewport.last_hud_status_panel_material_request.valid ||
        viewport.last_hud_status_panel_material_request.indexed_pixels !=
            plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT - 1].pixels ||
        viewport.last_hud_status_panel_material_request.width !=
            plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT - 1].width ||
        viewport.last_hud_status_panel_material_request.height !=
            plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT - 1].height ||
        viewport.last_hud_status_panel_material_request.stride !=
            plan.commands[DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT - 1].width) {
        fputs("FAIL: HUD plan did not render directly from canonical GDAT material\n",
              stderr);
        fprintf(stderr, "core=%d portrait=%d consumed=%d fallback-core=%d fallback-portrait=%d callbacks=%d\n",
                viewport.asset_hud_core_drawn_count,
                viewport.asset_hud_portrait_drawn_count,
                viewport.gdat_hud_material_plan_consumed_count,
                viewport.fallback_hud_core_drawn_count,
                viewport.fallback_hud_portrait_drawn_count, unexpected_fetches);
        ++failures;
    }
    {
        DM2_V1_HudHandActionSource source;
        DM2_V1_BootExpandedRectReceipt raw4_receipt;
        DM2_V1_GdatRaw4BlitPlacement source_placement;
        DM2_V1_ViewportHudPresentationCommand hand_command;
        const uint8_t *hand_pixels = NULL;
        int hand_width = 0;
        int hand_height = 0;
        int hand_stride = 0;
        const int hand_gdat_index =
            dm2_v1_viewport_hud_hand_action_graphic_index(1, 1);

        memset(&source, 0, sizeof(source));
        memset(&raw4_receipt, 0, sizeof(raw4_receipt));
        memset(&source_placement, 0, sizeof(source_placement));
        memset(&hand_command, 0, sizeof(hand_command));
        if (!dm2_v1_boot_interface_palette(&boot, &interface_palette) ||
            !interface_palette.hash || hand_gdat_index == 0 ||
            dm2_v1_boot_viewport_asset_fetch(
                &boot, hand_gdat_index, &hand_pixels, &hand_width,
                &hand_height, &hand_stride) != 0 || !hand_pixels ||
            hand_width <= 0 || hand_height <= 0 || hand_stride < hand_width ||
            !dm2_v1_boot_query_expanded_rect_receipt(
                &boot, 0x48u, &raw4_receipt) || !raw4_receipt.valid ||
            !raw4_receipt.raw4_hash ||
            !dm2_v1_gdat_door_overlay_query_raw4_blit_placement(
                dm2_v1_boot_asset_loader(&boot), 0x48u, hand_width,
                hand_height, &source_placement)) {
            fputs("FAIL: canonical hand-action GDAT/RAW4 route was unavailable\n",
                  stderr);
            ++failures;
        } else {
            party.champion_count = 1;
            party.leader_index = 0;
            party.champions[0].occupied = 1;
            memset(framebuffer, 0, sizeof(framebuffer));
            dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
            dm2_v1_viewport_set_hud_party(&viewport, &party);
            dm2_v1_viewport_set_asset_provider(
                &viewport, dm2_v1_boot_viewport_asset_fetch, &boot);
            dm2_v1_viewport_set_asset_palette_provider(
                &viewport, dm2_v1_boot_viewport_asset_palette_fetch, &boot);
            dm2_v1_viewport_set_asset_loader(
                &viewport, dm2_v1_boot_asset_loader(&boot));
            dm2_v1_viewport_set_source_materials_required(&viewport, 1);
            dm2_v1_viewport_set_scene_map_load_token(&viewport, 42u);
            dm2_v1_viewport_set_gdat_scene_control(
                &viewport, 1, 0, 0x48414e44u, 0u, 0u, 0u, 0u, 0u,
                0u, 0u, 0u, 0u, 0u);
            dm2_v1_viewport_set_gdat_interface_palette(
                &viewport, 1, interface_palette.hash,
                interface_palette.palette16);
            dm2_v1_viewport_set_gdat_hud_material_plan(&viewport, &plan);
            source.valid = 1;
            source.player_index = 0u;
            source.possession_index = 1u;
            source.left_or_right = 1u;
            source.player_position = 2u;
            source.party_direction = 0u;
            source.gdat_category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
            source.gdat_subcategory = 4u;
            source.gdat_entry = 5u;
            source.rectno = 0x48u;
            source.map_load_token = 42u;
            source.scene_control_hash = 0x48414e44u;
            source.palette_hash = interface_palette.hash;
            source.raw4_hash = raw4_receipt.raw4_hash;
            source.source_rect = (DM2_V1_ViewportRect){
                source_placement.source_x, source_placement.source_y,
                source_placement.destination.w, source_placement.destination.h };
            source.destination_rect = source_placement.destination;
            dm2_v1_viewport_set_hud_hand_action_source(&viewport, &source);
            dm2_v1_render_ui_chrome(&viewport);
            if (!dm2_v1_viewport_last_hud_hand_action_presentation_command(
                    &viewport, &hand_command) || !hand_command.valid ||
                hand_command.material.gdat_index != hand_gdat_index ||
                hand_command.material.width != hand_width ||
                hand_command.material.height != hand_height ||
                hand_command.destination_rect.x != source_placement.destination.x ||
                hand_command.destination_rect.y != source_placement.destination.y ||
                hand_command.destination_rect.w != source_placement.destination.w ||
                hand_command.destination_rect.h != source_placement.destination.h) {
                fprintf(stderr,
                        "FAIL: canonical hand action did not consume exact RAW4 placement "
                        "(source=%d command=%d blocked=%08x src=%d,%d %dx%d dst=%d,%d %dx%d image=%dx%d)\n",
                        viewport.hud_hand_action_source.valid,
                        hand_command.valid, viewport.blocked_material_mask,
                        source.source_rect.x, source.source_rect.y,
                        source.source_rect.w, source.source_rect.h,
                        source.destination_rect.x, source.destination_rect.y,
                        source.destination_rect.w, source.destination_rect.h,
                        hand_width, hand_height);
                fprintf(stderr,
                        "  hand guards: party=%d occupied=%d map=%u/%u scene=%08x/%08x pal=%08x/%08x raw=%08x\n",
                        viewport.hud_party.champion_count,
                        viewport.hud_party.champions[0].occupied,
                        viewport.hud_hand_action_source.map_load_token,
                        viewport.gdat_scene_map_load_token,
                        viewport.hud_hand_action_source.scene_control_hash,
                        viewport.gdat_scene_control_hash,
                        viewport.hud_hand_action_source.palette_hash,
                        viewport.gdat_interface_palette_hash,
                        viewport.hud_hand_action_source.raw4_hash);
                ++failures;
            }
            source.destination_rect.x++;
            dm2_v1_viewport_set_hud_hand_action_source(&viewport, &source);
            dm2_v1_render_ui_chrome(&viewport);
            if (dm2_v1_viewport_last_hud_hand_action_presentation_command(
                    &viewport, &hand_command) || hand_command.valid ||
                (viewport.blocked_material_mask &
                 DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE) == 0u) {
                fputs("FAIL: altered hand-action destination reached M11\n", stderr);
                ++failures;
            }
        }
    }
    {
        uint8_t saved_palette_byte = plan.commands[0].palette16[0];

        plan.commands[0].palette16[0] ^= 0x01u;
        memset(framebuffer, 0, sizeof(framebuffer));
        unexpected_fetches = 0;
        dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
        dm2_v1_viewport_set_hud_party(&viewport, &party);
        dm2_v1_viewport_set_asset_provider(&viewport, unexpected_asset_fetch, NULL);
        dm2_v1_viewport_set_asset_palette_provider(
            &viewport, unexpected_palette_fetch, NULL);
        dm2_v1_viewport_set_source_materials_required(&viewport, 1);
        dm2_v1_viewport_set_gdat_hud_material_plan(&viewport, &plan);
        dm2_v1_render_ui_chrome(&viewport);
        plan.commands[0].palette16[0] = saved_palette_byte;
        if (viewport.asset_hud_core_drawn_count >=
                DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
            viewport.gdat_hud_material_plan_consumed_count >=
                DM2_V1_GDAT_HUD_M11_STATIC_COMMAND_COUNT ||
            (viewport.blocked_material_mask &
                DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE) == 0u ||
            viewport.fallback_hud_core_drawn_count != 0 ||
            viewport.fallback_hud_portrait_drawn_count != 0 ||
            unexpected_fetches != 0) {
            fputs("FAIL: altered HUD palette reached the viewport or fallback path\n",
                  stderr);
            ++failures;
        }
    }
    dm2_v1_gdat_hud_m11_command_plan_free(&plan);
    dm2_v1_boot_cleanup(&boot);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failures != 0) {
        fprintf(stderr, "FAIL: %d invalid GDAT HUD command(s)\n", failures);
        return 1;
    }
    puts("PASS: real GRAPHICS.DAT yields a complete fail-closed M11 HUD command family");
    return 0;
}
