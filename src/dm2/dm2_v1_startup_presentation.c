#include "dm2_v1_startup_presentation.h"
#include "dm2_v1_asset_loader.h"

#include <stdio.h>
#include <string.h>

static void dm2_v1_startup_draw_clear(DM2_V1_StartupDrawCommand *command)
{
    if (!command) {
        return;
    }
    memset(command, 0, sizeof(*command));
    command->kind = DM2_V1_STARTUP_DRAW_NONE;
    command->row = -1;
    command->transparent_color = -1;
}

static int dm2_v1_startup_push_gdat_image(DM2_V1_StartupDrawCommand *commands,
                                          int max_commands,
                                          int *count,
                                          int category,
                                          int index,
                                          int field,
                                          const DM2_V1_StartupRect *rect,
                                          int transparent_color)
{
    DM2_V1_StartupDrawCommand *command;
    if (!commands || !count || !rect || max_commands <= 0 ||
        *count < 0 || *count >= max_commands) {
        return 0;
    }
    command = &commands[*count];
    dm2_v1_startup_draw_clear(command);
    command->kind = DM2_V1_STARTUP_DRAW_GDAT_IMAGE;
    command->rect = *rect;
    command->gdat_category = category;
    command->gdat_index = index;
    command->gdat_field = field;
    command->transparent_color = transparent_color;
    ++(*count);
    return 1;
}

static int dm2_v1_startup_push_rect(DM2_V1_StartupDrawCommand *commands,
                                    int max_commands,
                                    int *count,
                                    DM2_V1_StartupDrawKind kind,
                                    DM2_V1_StartupStyle style,
                                    const DM2_V1_StartupRect *rect,
                                    int row)
{
    DM2_V1_StartupDrawCommand *command;
    if (!commands || !count || !rect || max_commands <= 0 ||
        *count < 0 || *count >= max_commands) {
        return 0;
    }
    command = &commands[*count];
    dm2_v1_startup_draw_clear(command);
    command->kind = kind;
    command->style = style;
    command->rect = *rect;
    command->row = row;
    ++(*count);
    return 1;
}

static int dm2_v1_startup_push_text(DM2_V1_StartupDrawCommand *commands,
                                    int max_commands,
                                    int *count,
                                    DM2_V1_StartupStyle style,
                                    int x,
                                    int y,
                                    int row,
                                    const char *text)
{
    DM2_V1_StartupDrawCommand *command;
    if (!commands || !count || !text || max_commands <= 0 ||
        *count < 0 || *count >= max_commands) {
        return 0;
    }
    command = &commands[*count];
    dm2_v1_startup_draw_clear(command);
    command->kind = DM2_V1_STARTUP_DRAW_TEXT;
    command->style = style;
    command->x = x;
    command->y = y;
    command->row = row;
    snprintf(command->text, sizeof(command->text), "%s", text);
    ++(*count);
    return 1;
}

int dm2_v1_startup_row_label(DM2_V1_StartupRowKind kind,
                             int slot,
                             char *out_label,
                             int out_label_size)
{
    if (!out_label || out_label_size <= 0) {
        return 0;
    }
    out_label[0] = '\0';
    if (kind == DM2_V1_STARTUP_ROW_CONTINUE) {
        snprintf(out_label, (size_t)out_label_size, "CONTINUE");
        return 1;
    }
    if (kind == DM2_V1_STARTUP_ROW_SLOT) {
        if (slot < 0) {
            return 0;
        }
        snprintf(out_label, (size_t)out_label_size, "LOAD SLOT %02d", slot);
        return 1;
    }
    if (kind == DM2_V1_STARTUP_ROW_NEW_GAME) {
        snprintf(out_label, (size_t)out_label_size, "NEW GAME");
        return 1;
    }
    return 0;
}

int dm2_v1_startup_presentation_build(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    DM2_V1_StartupRect rect;
    int count = 0;
    int row;

    if (!menu || !out_commands || max_commands <= 0) {
        return 0;
    }
    for (row = 0; row < max_commands; ++row) {
        dm2_v1_startup_draw_clear(&out_commands[row]);
    }
    /* skproject/SKWin: ANIM_BOOTSTRAP_TITLE() leads into the GDAT-backed
     * title/main-menu art; category 0x05 is TITLE and field 0x01 is the
     * 320x200 base title surface in PC GRAPHICS.DAT. */
    rect.x = 0;
    rect.y = 0;
    rect.w = 320;
    rect.h = 200;
    if (!dm2_v1_startup_push_gdat_image(out_commands,
                                        max_commands,
                                        &count,
                                        DM2_GDAT_CATEGORY_TITLE,
                                        0,
                                        1,
                                        &rect,
                                        -1) ||
        !dm2_v1_startup_panel_rect(&rect) ||
        !dm2_v1_startup_push_rect(out_commands,
                                  max_commands,
                                  &count,
                                  DM2_V1_STARTUP_DRAW_FILL_RECT,
                                  DM2_V1_STARTUP_STYLE_PANEL,
                                  &rect,
                                  -1) ||
        !dm2_v1_startup_push_rect(out_commands,
                                  max_commands,
                                  &count,
                                  DM2_V1_STARTUP_DRAW_OUTLINE_RECT,
                                  DM2_V1_STARTUP_STYLE_BORDER,
                                  &rect,
                                  -1) ||
        !dm2_v1_startup_push_text(out_commands,
                                  max_commands,
                                  &count,
                                  DM2_V1_STARTUP_STYLE_TITLE,
                                  DM2_V1_STARTUP_TITLE_X,
                                  DM2_V1_STARTUP_TITLE_Y,
                                  -1,
                                  "DUNGEON MASTER II") ||
        !dm2_v1_startup_push_text(out_commands,
                                  max_commands,
                                  &count,
                                  DM2_V1_STARTUP_STYLE_TEXT,
                                  DM2_V1_STARTUP_SUBTITLE_X,
                                  DM2_V1_STARTUP_SUBTITLE_Y,
                                  -1,
                                  "SELECT GAME")) {
        return 0;
    }
    for (row = 0; row < menu->row_count; ++row) {
        DM2_V1_StartupRowKind kind = DM2_V1_STARTUP_ROW_NONE;
        int slot = -1;
        char label[64];
        DM2_V1_StartupRect row_rect;
        DM2_V1_StartupStyle text_style = DM2_V1_STARTUP_STYLE_TEXT;

        if (!dm2_v1_startup_menu_row_at(menu, row, &kind, &slot) ||
            !dm2_v1_startup_row_rect(row, &row_rect) ||
            !dm2_v1_startup_row_label(kind,
                                      slot,
                                      label,
                                      (int)sizeof(label))) {
            continue;
        }
        if (row == menu->selected_row) {
            DM2_V1_StartupRect highlight_rect;
            if (!dm2_v1_startup_row_highlight_rect(row, &highlight_rect) ||
                !dm2_v1_startup_push_rect(out_commands,
                                          max_commands,
                                          &count,
                                          DM2_V1_STARTUP_DRAW_FILL_RECT,
                                          DM2_V1_STARTUP_STYLE_SELECTED_FILL,
                                          &highlight_rect,
                                          row)) {
                return 0;
            }
            text_style = DM2_V1_STARTUP_STYLE_SELECTED_TEXT;
        }
        if (!dm2_v1_startup_push_text(out_commands,
                                      max_commands,
                                      &count,
                                      text_style,
                                      DM2_V1_STARTUP_ROW_TEXT_X,
                                      row_rect.y + 2,
                                      row,
                                      label)) {
            return 0;
        }
    }
    if (!dm2_v1_startup_push_text(out_commands,
                                  max_commands,
                                  &count,
                                  DM2_V1_STARTUP_STYLE_TEXT,
                                  DM2_V1_STARTUP_FOOTER_X,
                                  DM2_V1_STARTUP_FOOTER_Y,
                                  -1,
                                  "ENTER/ACTION STARTS")) {
        return 0;
    }
    return count;
}

int dm2_v1_startup_presentation_build_from_snapshot(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    DM2_V1_StartupMenu menu;

    if (!dm2_v1_startup_menu_from_snapshot(snapshot, &menu)) {
        return 0;
    }
    return dm2_v1_startup_presentation_build(&menu,
                                             out_commands,
                                             max_commands);
}

int dm2_v1_startup_presentation_build_from_facts(
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    DM2_V1_StartupMenuSnapshot snapshot;

    if (!dm2_v1_startup_menu_snapshot_from_facts(&snapshot,
                                                 save_root,
                                                 fallback_save_root,
                                                 resume_available,
                                                 slot_mask,
                                                 selected_row)) {
        return 0;
    }
    return dm2_v1_startup_presentation_build_from_snapshot(&snapshot,
                                                           out_commands,
                                                           max_commands);
}

int dm2_v1_startup_presentation_build_from_host_facts(
    const DM2_V1_StartupHostFacts *facts,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!facts) {
        return 0;
    }
    return dm2_v1_startup_presentation_build_from_facts(
        facts->save_root,
        facts->fallback_save_root,
        facts->resume_available,
        facts->slot_mask,
        facts->selected_row,
        out_commands,
        max_commands);
}

int dm2_v1_startup_execute_draw_commands(
    const DM2_V1_StartupDrawCommand *commands,
    int command_count,
    const DM2_V1_StartupDrawExecutor *executor)
{
    int i;

    if (!commands || command_count < 0 || !executor ||
        !executor->draw_gdat_image || !executor->fill_rect ||
        !executor->outline_rect || !executor->draw_text) {
        return 0;
    }
    for (i = 0; i < command_count; ++i) {
        const DM2_V1_StartupDrawCommand *command = &commands[i];
        switch (command->kind) {
        case DM2_V1_STARTUP_DRAW_GDAT_IMAGE:
            (void)executor->draw_gdat_image(executor->userdata, command);
            break;
        case DM2_V1_STARTUP_DRAW_FILL_RECT:
            executor->fill_rect(executor->userdata, command);
            break;
        case DM2_V1_STARTUP_DRAW_OUTLINE_RECT:
            executor->outline_rect(executor->userdata, command);
            break;
        case DM2_V1_STARTUP_DRAW_TEXT:
            executor->draw_text(executor->userdata, command);
            break;
        case DM2_V1_STARTUP_DRAW_NONE:
        default:
            break;
        }
    }
    return 1;
}
