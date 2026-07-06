#ifndef DM2_V1_STARTUP_PRESENTATION_H
#define DM2_V1_STARTUP_PRESENTATION_H

#include "dm2_v1_startup_layout.h"
#include "dm2_v1_startup_menu.h"

typedef enum {
    DM2_V1_STARTUP_DRAW_NONE = 0,
    DM2_V1_STARTUP_DRAW_FILL_RECT = 1,
    DM2_V1_STARTUP_DRAW_OUTLINE_RECT = 2,
    DM2_V1_STARTUP_DRAW_TEXT = 3
} DM2_V1_StartupDrawKind;

typedef enum {
    DM2_V1_STARTUP_STYLE_PANEL = 0,
    DM2_V1_STARTUP_STYLE_BORDER = 1,
    DM2_V1_STARTUP_STYLE_TITLE = 2,
    DM2_V1_STARTUP_STYLE_TEXT = 3,
    DM2_V1_STARTUP_STYLE_SELECTED_FILL = 4,
    DM2_V1_STARTUP_STYLE_SELECTED_TEXT = 5
} DM2_V1_StartupStyle;

typedef struct {
    DM2_V1_StartupDrawKind kind;
    DM2_V1_StartupStyle style;
    DM2_V1_StartupRect rect;
    int x;
    int y;
    int row;
    char text[64];
} DM2_V1_StartupDrawCommand;

int dm2_v1_startup_row_label(DM2_V1_StartupRowKind kind,
                             int slot,
                             char *out_label,
                             int out_label_size);
int dm2_v1_startup_presentation_build(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);

#endif
