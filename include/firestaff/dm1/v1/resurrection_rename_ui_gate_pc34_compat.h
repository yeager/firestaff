#ifndef FIRESTAFF_DM1_V1_RESURRECTION_RENAME_UI_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RESURRECTION_RENAME_UI_GATE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_RESURRECT_PC34_COMPAT 160
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT 161
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_CANCEL_PC34_COMPAT 162
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_BACKSPACE_PC34_COMPAT 165
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT 166
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_TITLE_PC34_COMPAT 167
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT 168
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_Z_PC34_COMPAT 193
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_COMMA_PC34_COMPAT 194
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_PERIOD_PC34_COMPAT 195
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SEMICOLON_PC34_COMPAT 196
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_COLON_PC34_COMPAT 197
#define DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SPACE_PC34_COMPAT 198

#define DM1_V1_RESURRECTION_RENAME_UI_FIELD_NONE_PC34_COMPAT 0
#define DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT 1
#define DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT 2

#define DM1_V1_RESURRECTION_RENAME_UI_NAME_MAX_PC34_COMPAT 7
#define DM1_V1_RESURRECTION_RENAME_UI_TITLE_MAX_PC34_COMPAT 19
#define DM1_V1_RESURRECTION_RENAME_UI_PANEL_GRAPHIC_PC34_COMPAT 27
#define DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_X_PC34_COMPAT 80
#define DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_Y_PC34_COMPAT 223
#define DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_W_PC34_COMPAT 52
#define DM1_V1_RESURRECTION_RENAME_UI_PANEL_BOX_H_PC34_COMPAT 124

typedef struct DM1_V1_ResurrectionRenameUiGatePc34Compat {
    int panelCommand;
    int f0281RenameCallCount;
    int panelGraphicIndex;
    int panelBox[4];
    int fieldMode;
    int characterIndex;
    int cursorX;
    int cursorY;
    int returned;
    int okAccepted;
    int rejectedLeadingSpaceCount;
    int rejectedFullTitleCount;
    int ignoredBackspaceAtEmptyNameCount;
    char name[DM1_V1_RESURRECTION_RENAME_UI_NAME_MAX_PC34_COMPAT + 1];
    char title[DM1_V1_RESURRECTION_RENAME_UI_TITLE_MAX_PC34_COMPAT + 1];
} DM1_V1_ResurrectionRenameUiGatePc34Compat;

void
dm1_v1_resurrection_rename_ui_gate_init_pc34(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int panel_command);

int
dm1_v1_resurrection_rename_ui_gate_apply_ascii_pc34(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int ch);

int
dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
    DM1_V1_ResurrectionRenameUiGatePc34Compat *state,
    int command);

int
dm1_v1_resurrection_rename_ui_gate_run_self_test_pc34(void);

const char *
dm1_v1_resurrection_rename_ui_gate_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
