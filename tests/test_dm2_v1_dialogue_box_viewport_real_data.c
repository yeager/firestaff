/* Canonical PC G1 GRAPHICS.DAT proof for c_dialog.cpp's save/load panel. */

#include "dm2_v1_boot.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char data_root[1024];
    char graphics_path[1024];
    DM2_V1_BootProfile boot;
    DM2_V1_DialogueBoxHostCommand command;
    DM2_V1_BootExpandedRectReceipt raw4_rect;
    DM2_V1_DialogueOpenPanelHostCommand open_panel;
    DM2_V1_DialogueSavePointerReceipt save_pointer;
    DM2_V1_DialogueSavePointerReceipt save_pointer_row_seven;
    DM2_V1_InterfacePalette interface_palette;
    DM2_V1_InterfaceActionTable action_table;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t action_palette[16];
    const uint8_t *font_rows = NULL;
    uint32_t font_hash = 0u;
    int box_ok;
    int open_ok;
    int pointer_ok;
    int failures = 0;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not configured");
        return 0;
    }
    snprintf(data_root, sizeof(data_root), "%s/..", root);
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    {
        FILE *file = fopen(graphics_path, "rb");
        if (!file) {
            fprintf(stderr,
                    "FAIL: configured DM2 GRAPHICS.DAT is unreadable: %s\n",
                    graphics_path);
            return 1;
        }
        fclose(file);
    }
    dm2_v1_boot_profile_init(&boot);
    memset(&command, 0, sizeof(command));
    memset(&open_panel, 0, sizeof(open_panel));
    memset(&save_pointer, 0, sizeof(save_pointer));
    memset(&save_pointer_row_seven, 0, sizeof(save_pointer_row_seven));
    memset(&interface_palette, 0, sizeof(interface_palette));
    memset(&action_table, 0, sizeof(action_table));
    if (dm2_v1_boot_scan_assets(&boot, data_root) != 0 ||
        dm2_v1_boot_enter_game(&boot) != 0) {
        fputs("FAIL: canonical DM2 boot profile was not admitted\n", stderr);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    box_ok = dm2_v1_boot_dialogue_box_host_command(&boot, &command);
    memset(&raw4_rect, 0, sizeof(raw4_rect));
    if (box_ok && !dm2_v1_boot_query_expanded_rect_receipt(
                      &boot, command.draw.expanded_rect_index, &raw4_rect)) {
        fputs("FAIL: canonical RAW4 expanded-rect receipt was not admitted\n",
              stderr);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    open_ok = dm2_v1_boot_dialogue_open_panel_host_command(&boot, &open_panel);
    /* c_dialog.cpp::DM2_dialog_2066_398a expands RECT_451 and lays out ten
     * rows from its source-owned y + 4 baseline. First admit the canonical
     * RAW4 event rectangle, then prove row seven from its measured baseline. */
    pointer_ok = dm2_v1_boot_dialogue_save_pointer_receipt(
        &boot, 451u, 451u, 60, &save_pointer);
    if (!box_ok || !open_ok ||
        !dm2_v1_boot_interface_palette(&boot, &interface_palette) ||
        !dm2_v1_boot_interface_action_table(&boot, &action_table) ||
        !dm2_v1_boot_interface_font_table(&boot, &font_rows, &font_hash)) {
        fprintf(stderr, "FAIL: canonical save/load dialogue command was not admitted (box=%d open=%d)\n",
                box_ok, open_ok);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    if (!raw4_rect.valid ||
        raw4_rect.rect_id != command.draw.expanded_rect_index ||
        !raw4_rect.raw4_bytes || raw4_rect.raw4_byte_count == 0u ||
        raw4_rect.raw4_hash == 0u || raw4_rect.receipt_hash == 0u ||
        memcmp(&raw4_rect.rect, &command.rect, sizeof(command.rect)) != 0) {
        fputs("FAIL: RAW4 receipt did not bind dialogue expanded rectangle\n",
              stderr);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    if (!pointer_ok || !save_pointer.valid ||
        save_pointer.event_rect_index != 451u ||
        save_pointer.event_top_left_index != 451u ||
        save_pointer.row_stride != 7 ||
        save_pointer.selected_slot < 0 || save_pointer.selected_slot > 10 ||
        save_pointer.command_hash == 0u) {
        fputs("FAIL: canonical save dialogue event rectangle did not admit source row selection\n",
              stderr);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    pointer_ok = dm2_v1_boot_dialogue_save_pointer_receipt(
        &boot, 451u, 451u,
        save_pointer.event_rect.y + save_pointer.top_left_y + 7 * 7,
        &save_pointer_row_seven);
    if (!pointer_ok || !save_pointer_row_seven.valid ||
        save_pointer_row_seven.selected_slot != 7 ||
        dm2_v1_boot_dialogue_save_pointer_receipt(
            &boot, 451u, 451u,
            save_pointer.event_rect.y + save_pointer.top_left_y - 1,
            &save_pointer_row_seven) || save_pointer_row_seven.valid) {
        fputs("FAIL: canonical save dialogue selection did not preserve the source row boundary\n",
              stderr);
        dm2_v1_boot_cleanup(&boot);
        return 1;
    }
    memcpy(action_palette, interface_palette.palette16, sizeof(action_palette));
    if (!dm2_v1_interface_action_table_remap_palette(
            &action_table, action_palette, 16u, 0u, -1, -1)) {
        fputs("FAIL: canonical dialogue text palette was not admitted\n", stderr);
        ++failures;
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
    dm2_v1_viewport_set_gdat_interface_font(&viewport, font_rows, font_hash);
    dm2_v1_viewport_set_gdat_interface_text_palette(
        &viewport, 1, interface_palette.hash, action_palette);
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
    dm2_v1_viewport_set_gdat_interface_font(&viewport, font_rows, font_hash);
    dm2_v1_viewport_set_gdat_interface_text_palette(
        &viewport, 1, interface_palette.hash, action_palette);
    dm2_v1_viewport_set_gdat_dialogue_open_panel_host_command(
        &viewport, &open_panel, 1);
    dm2_v1_render_dialogue_open_panel(&viewport);
    if (!open_panel.valid || !open_panel.draw.valid ||
        strcmp((const char *)open_panel.draw.version_text, "V1.0") != 0 ||
        strcmp((const char *)open_panel.draw.text[0], "SAVE") != 0 ||
        strcmp((const char *)open_panel.draw.text[1], "CANCEL") != 0 ||
        open_panel.draw.source_text_hash[0] == 0u ||
        open_panel.draw.source_text_hash[1] == 0u ||
        open_panel.version_text_rect.w <= 0 ||
        open_panel.primary_text_rect.w <= 0 ||
        open_panel.secondary_text_rect.w <= 0 ||
        viewport.gdat_dialogue_open_panel_consumed_count != 4 ||
        viewport.gdat_dialogue_open_panel_consumed_hash != open_panel.command_hash ||
        viewport.blocked_material_draw_count != 0) {
        fprintf(stderr, "FAIL: canonical open dialogue panel did not consume its source labels (%s/%s/%s)\n",
                open_panel.draw.version_text, open_panel.draw.text[0],
                open_panel.draw.text[1]);
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
    puts("PASS: canonical DIALOG_BOXES/0x81/0 dialogue routes reach viewport only through active source commands");
    return 0;
}
