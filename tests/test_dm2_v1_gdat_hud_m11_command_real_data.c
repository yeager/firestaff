/* Canonical PC G1 GRAPHICS.DAT proof for the complete M11 HUD command plan. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
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
    char path[1024];
    char boot_root[1024];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_BootProfile boot;
    DM2_V1_GdatHudM11CommandPlan plan;
    DM2_V1_BootStartupMenuHudGdatReceipt menu_hud_gdat;
    DM2_V1_HudPartyState party;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int failures = 0;
    int expected_kind[DM2_V1_GDAT_HUD_M11_COMMAND_MAX] = {
        DM2_V1_GDAT_HUD_M11_COMMAND_TOP_BAR,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_STRIP,
        DM2_V1_GDAT_HUD_M11_COMMAND_GOLD_BOX,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
        DM2_V1_GDAT_HUD_M11_COMMAND_ACTION_ICON,
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
    memset(&menu_hud_gdat, 0, sizeof(menu_hud_gdat));
    memset(&party, 0, sizeof(party));
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
        menu_hud_gdat.hud_static_command_count != 9 ||
        menu_hud_gdat.hud_static_plan_hash == 0u ||
        !menu_hud_gdat.hud_palette_ready ||
        menu_hud_gdat.receipt_hash == 0u) {
        fputs("FAIL: startup menu/HUD GDAT receipt was incomplete\n", stderr);
        ++failures;
    }
    if (!plan.valid || plan.command_count != 9 ||
        plan.command_hash == 0u) {
        ++failures;
    }
    for (int i = 0; i < plan.command_count; ++i) {
        const DM2_V1_GdatHudM11Command *command = &plan.commands[i];
        if (command->kind != expected_kind[i] ||
            (i < 9 && (command->gdat_category != DM2_GDAT_CATEGORY_INTERFACE_GENERAL ||
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
    if (viewport.asset_hud_core_drawn_count != 9 ||
        viewport.asset_hud_portrait_drawn_count != 0 ||
        viewport.gdat_hud_material_plan_consumed_count !=
            9 ||
        viewport.fallback_hud_core_drawn_count != 0 ||
        viewport.fallback_hud_portrait_drawn_count != 0) {
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
        if (viewport.asset_hud_core_drawn_count >= 9 ||
            viewport.gdat_hud_material_plan_consumed_count >= 9 ||
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
