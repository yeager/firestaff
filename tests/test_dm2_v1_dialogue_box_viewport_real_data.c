/* Canonical PC G1 GRAPHICS.DAT proof for c_dialog.cpp's save/load panel. */

#include "dm2_v1_boot.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *home = getenv("HOME");
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char data_root[1024];
    char graphics_path[1024];
    DM2_V1_BootProfile boot;
    DM2_V1_DialogueBoxHostCommand command;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int failures = 0;

    if (root && root[0]) {
        snprintf(data_root, sizeof(data_root), "%s/..", root);
        snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    } else if (home && home[0]) {
        snprintf(data_root, sizeof(data_root), "%s/.firestaff/data/dm2", home);
        snprintf(graphics_path, sizeof(graphics_path),
                 "%s/.firestaff/data/dm2/data/graphics.dat", home);
    } else {
        puts("SKIP: no DM2 data root");
        return 0;
    }
    {
        FILE *file = fopen(graphics_path, "rb");
        if (!file) {
            puts("SKIP: no local canonical DM2 GRAPHICS.DAT");
            return 0;
        }
        fclose(file);
    }
    dm2_v1_boot_profile_init(&boot);
    memset(&command, 0, sizeof(command));
    if (dm2_v1_boot_scan_assets(&boot, data_root) != 0 ||
        dm2_v1_boot_enter_game(&boot) != 0 ||
        !dm2_v1_boot_dialogue_box_host_command(&boot, &command)) {
        fputs("FAIL: canonical save/load dialogue command was not admitted\n",
              stderr);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    if (!command.valid || !command.draw.valid ||
        command.draw.gdat_category != DM2_GDAT_CATEGORY_DIALOG_BOXES ||
        command.draw.gdat_index != DM2_V1_DIALOGUE_BOX_INDEX ||
        command.draw.gdat_field != DM2_V1_DIALOGUE_BOX_FIELD ||
        command.draw.expanded_rect_index != DM2_V1_DIALOGUE_BOX_RECT_INDEX ||
        command.draw.plan_hash == 0u || command.command_hash == 0u ||
        command.rect.w <= 0 || command.rect.h <= 0) {
        fputs("FAIL: dialogue command did not preserve c_dialog source identity\n",
              stderr);
        ++failures;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(
        &viewport, dm2_v1_boot_viewport_asset_fetch, &boot);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, dm2_v1_boot_viewport_asset_palette_fetch, &boot);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_dialogue_box_host_command(
        &viewport, &command, 1);
    dm2_v1_render_dialogue_box(&viewport);
    if (viewport.gdat_dialogue_box_consumed_count != 1 ||
        viewport.gdat_dialogue_box_consumed_hash != command.command_hash ||
        viewport.blocked_material_draw_count != 0 ||
        viewport.gdat_interface_palette_consumed_count == 0) {
        fputs("FAIL: canonical dialogue panel did not consume its source pixels\n",
              stderr);
        ++failures;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(
        &viewport, dm2_v1_boot_viewport_asset_fetch, &boot);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, dm2_v1_boot_viewport_asset_palette_fetch, &boot);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_dialogue_box_host_command(
        &viewport, &command, 0);
    dm2_v1_render_dialogue_box(&viewport);
    if (viewport.gdat_dialogue_box_consumed_count != 0 ||
        viewport.blocked_material_draw_count != 0) {
        fputs("FAIL: inactive dialogue material was drawn or blocked\n", stderr);
        ++failures;
    }

    dm2_v1_boot_cleanup(&boot);
    if (failures) return 1;
    puts("PASS: canonical DIALOG_BOXES/0x81/0 reaches viewport only through active RECT_453 command");
    return 0;
}
