#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"

#include <string.h>

enum {
    kNameStartX = 177,
    kNameStartY = 91,
    kTitleStartX = 105,
    kTitleStartY = 109,
    kTextStepX = 6
};

static int is_rename_live(
    const DM1_V1_ResurrectionRenameUiGatePc34Compat *state)
{
    return state && state->f0281RenameCallCount == 1 && !state->returned;
}

static void set_name_field(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int character_index)
{
    state->fieldMode = DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT;
    state->characterIndex = character_index;
    state->cursorX = kNameStartX + (character_index * kTextStepX);
    state->cursorY = kNameStartY;
}

static void set_title_field(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int character_index)
{
    state->fieldMode = DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT;
    state->characterIndex = character_index;
    state->cursorX = kTitleStartX + (character_index * kTextStepX);
    state->cursorY = kTitleStartY;
}

static void proceed_to_title(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state)
{
    set_title_field(state, 0);
}

static int append_char(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int ch)
{
    char *target;
    int max_len;

    if (!is_rename_live(state)) {
        return 0;
    }
    if (ch >= 'a' && ch <= 'z') {
        ch -= 32;
    }
    if (!((ch >= 'A' && ch <= 'Z') ||
          ch == '.' || ch == ',' || ch == ';' || ch == ':' || ch == ' ')) {
        return 0;
    }
    if (state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT &&
        ch == ' ' &&
        state->characterIndex == 0) {
        state->rejectedLeadingSpaceCount++;
        return 0;
    }
    if (state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT) {
        target = state->name;
        max_len = DM1_V1_RESURRECTION_RENAME_UI_NAME_MAX_PC34_COMPAT;
    } else if (state->fieldMode ==
               DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT) {
        target = state->title;
        max_len = DM1_V1_RESURRECTION_RENAME_UI_TITLE_MAX_PC34_COMPAT;
    } else {
        return 0;
    }
    if (state->characterIndex >= max_len) {
        if (state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT) {
            state->rejectedFullTitleCount++;
        }
        return 0;
    }

    target[state->characterIndex++] = (char)ch;
    target[state->characterIndex] = '\0';
    state->cursorX += kTextStepX;
    if (state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT &&
        state->characterIndex ==
            DM1_V1_RESURRECTION_RENAME_UI_NAME_MAX_PC34_COMPAT) {
        proceed_to_title(state);
    }
    return 1;
}

static int handle_backspace(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state)
{
    if (!is_rename_live(state)) {
        return 0;
    }
    if (state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT &&
        state->characterIndex == 0) {
        state->ignoredBackspaceAtEmptyNameCount++;
        return 0;
    }
    if (state->characterIndex == 0 &&
        state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT) {
        int len = (int)strlen(state->name);
        if (len <= 0) {
            set_name_field(state, 0);
            return 0;
        }
        set_name_field(state, len - 1);
        state->name[state->characterIndex] = '\0';
        return 1;
    }
    state->characterIndex--;
    if (state->fieldMode ==
        DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT) {
        state->title[state->characterIndex] = '\0';
    } else {
        state->name[state->characterIndex] = '\0';
    }
    state->cursorX -= kTextStepX;
    return 1;
}

static int handle_return_or_title(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state)
{
    if (!is_rename_live(state)) {
        return 0;
    }
    if (state->fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT &&
        state->characterIndex > 0) {
        proceed_to_title(state);
        return 1;
    }
    return 0;
}

static int handle_ok(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state)
{
    int len;

    if (!is_rename_live(state)) {
        return 0;
    }
    if (state->fieldMode !=
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT &&
        state->characterIndex <= 0) {
        return 0;
    }
    len = (int)strlen(state->name);
    while (len > 0 && state->name[len - 1] == ' ') {
        state->name[--len] = '\0';
    }
    if (len <= 0) {
        return 0;
    }
    state->returned = 1;
    state->okAccepted = 1;
    return 1;
}

void
dm1_v1_resurrection_rename_ui_gate_init_pc34(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int panel_command)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelCommand = panel_command;
    state->fieldMode = DM1_V1_RESURRECTION_RENAME_UI_FIELD_NONE_PC34_COMPAT;
    state->panelGraphicIndex = -1;
    state->panelBox[0] = DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_X_PC34_COMPAT;
    state->panelBox[1] = DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_Y_PC34_COMPAT;
    state->panelBox[2] = DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_W_PC34_COMPAT;
    state->panelBox[3] = DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_H_PC34_COMPAT;

    if (panel_command ==
        DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT) {
        /* ReDMCSB REVIVE.C F0282 lines 806-808 calls F0281 only for
         * C161 reincarnate. F0281 lines 407-419 blit C027 into G0032,
         * erase Name/Title, and place the cursor at the name field. */
        state->f0281RenameCallCount = 1;
        state->panelGraphicIndex =
            DM1_V1_RESURRECTION_RENAME_UI_PANEL_GRAPHIC_PC34_COMPAT;
        set_name_field(state, 0);
    }
}

int
dm1_v1_resurrection_rename_ui_gate_apply_ascii_pc34(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int ch)
{
    if (ch == '\b') {
        return handle_backspace(state);
    }
    if (ch == '\r') {
        return handle_return_or_title(state);
    }
    return append_char(state, ch);
}

int
dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int command)
{
    if (command >= DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT &&
        command <= DM1_V1_RESURRECTION_RENAME_UI_COMMAND_Z_PC34_COMPAT) {
        return append_char(state,
                           'A' +
                               (command -
                                DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT));
    }
    switch (command) {
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_BACKSPACE_PC34_COMPAT:
        return handle_backspace(state);
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT:
        return handle_ok(state);
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_TITLE_PC34_COMPAT:
        return handle_return_or_title(state);
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_COMMA_PC34_COMPAT:
        return append_char(state, ',');
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_PERIOD_PC34_COMPAT:
        return append_char(state, '.');
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SEMICOLON_PC34_COMPAT:
        return append_char(state, ';');
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_COLON_PC34_COMPAT:
        return append_char(state, ':');
    case DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SPACE_PC34_COMPAT:
        return append_char(state, ' ');
    default:
        return 0;
    }
}

int
dm1_v1_resurrection_rename_ui_gate_run_self_test_pc34(void)
{
    DM1_V1_ResurrectionRenameUiGatePc34Compat state;
    int i;

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_RESURRECT_PC34_COMPAT);
    if (state.f0281RenameCallCount != 0 || state.fieldMode !=
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NONE_PC34_COMPAT) {
        return 0;
    }
    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_CANCEL_PC34_COMPAT);
    if (state.f0281RenameCallCount != 0 || state.fieldMode !=
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NONE_PC34_COMPAT) {
        return 0;
    }

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT);
    if (state.f0281RenameCallCount != 1 ||
        state.panelGraphicIndex !=
            DM1_V1_RESURRECTION_RENAME_UI_PANEL_GRAPHIC_PC34_COMPAT ||
        state.fieldMode != DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT ||
        state.cursorX != kNameStartX ||
        state.cursorY != kNameStartY ||
        state.name[0] != '\0' ||
        state.title[0] != '\0') {
        return 0;
    }
    if (dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
            &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SPACE_PC34_COMPAT) != 0 ||
        state.rejectedLeadingSpaceCount != 1 ||
        state.name[0] != '\0') {
        return 0;
    }
    if (dm1_v1_resurrection_rename_ui_gate_apply_ascii_pc34(&state, 'z') != 1 ||
        strcmp(state.name, "Z") != 0) {
        return 0;
    }
    if (dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
            &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT) != 1 ||
        state.okAccepted != 1) {
        return 0;
    }

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT);
    for (i = 0; i < DM1_V1_RESURRECTION_RENAME_UI_NAME_MAX_PC34_COMPAT; ++i) {
        if (dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                &state,
                DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT + i) != 1) {
            return 0;
        }
    }
    if (strcmp(state.name, "ABCDEFG") != 0 ||
        state.fieldMode != DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT ||
        state.characterIndex != 0 ||
        state.cursorX != kTitleStartX ||
        state.cursorY != kTitleStartY) {
        return 0;
    }
    if (dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
            &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_BACKSPACE_PC34_COMPAT) != 1 ||
        strcmp(state.name, "ABCDEF") != 0 ||
        state.fieldMode != DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT ||
        state.characterIndex != 6) {
        return 0;
    }
    return 1;
}

const char *
dm1_v1_resurrection_rename_ui_gate_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:338-342 C160/C161/C162/C165/C166, "
           "DEFS.H:343-374 C167/C168..C198 rename commands, "
           "DEFS.H:2186-2188 C027 rename panel graphic, DEFS.H:2902-2905 "
           "C1/C2 rename field modes; DATA.C:89-91/428-430 G0051/G0052/"
           "G0053 rename strings; REVIVE.C F0281:407-419 blits C027 into "
           "G0032 and starts empty Name at 177,91; REVIVE.C F0281:448-464 "
           "gates OK until title mode or non-empty name and trims right "
           "spaces; REVIVE.C F0281:515-529 uppercases input, rejects leading "
           "space, appends allowed letters/punctuation/space, and moves "
           "7-char names to title; REVIVE.C F0281:535-545 moves Return from "
           "name to title; REVIVE.C F0281:549-567 backspaces from empty "
           "title into the name; REVIVE.C F0282:744-807 calls F0281 only "
           "for C161 reincarnate, not C160 resurrect or C162 cancel.";
}
