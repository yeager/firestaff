#ifndef DM2_V1_STARTUP_PRESENTATION_H
#define DM2_V1_STARTUP_PRESENTATION_H

#include "dm2_v1_startup_layout.h"
#include "dm2_v1_startup_menu.h"

typedef enum {
    DM2_V1_STARTUP_DRAW_NONE = 0,
    DM2_V1_STARTUP_DRAW_FILL_RECT = 1,
    DM2_V1_STARTUP_DRAW_OUTLINE_RECT = 2,
    DM2_V1_STARTUP_DRAW_TEXT = 3,
    DM2_V1_STARTUP_DRAW_GDAT_IMAGE = 4
} DM2_V1_StartupDrawKind;

typedef enum {
    DM2_V1_STARTUP_STYLE_PANEL = 0,
    DM2_V1_STARTUP_STYLE_BORDER = 1,
    DM2_V1_STARTUP_STYLE_TITLE = 2,
    DM2_V1_STARTUP_STYLE_TEXT = 3,
    DM2_V1_STARTUP_STYLE_SELECTED_FILL = 4,
    DM2_V1_STARTUP_STYLE_SELECTED_TEXT = 5
} DM2_V1_StartupStyle;

typedef struct DM2_V1_StartupDrawCommand {
    DM2_V1_StartupDrawKind kind;
    DM2_V1_StartupStyle style;
    DM2_V1_StartupRect rect;
    int x;
    int y;
    int row;
    int gdat_category;
    int gdat_index;
    int gdat_field;
    int transparent_color;
    char text[64];
} DM2_V1_StartupDrawCommand;

typedef struct DM2_V1_StartupRenderReceipt {
    int valid;
    int command_count;
    int row_count;
    int selected_row;
    int title_gdat_found;
    int title_gdat_category;
    int title_gdat_index;
    int title_gdat_field;
    DM2_V1_StartupRect title_rect;
    DM2_V1_StartupRect panel_rect;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int hud_runtime_ready;
    int hud_overlay_suppressed;
} DM2_V1_StartupRenderReceipt;

typedef int (*DM2_V1_StartupDrawGdatImageFn)(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command);

typedef void (*DM2_V1_StartupDrawRectFn)(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command);

typedef void (*DM2_V1_StartupDrawTextFn)(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command);

typedef struct {
    void *userdata;
    DM2_V1_StartupDrawGdatImageFn draw_gdat_image;
    DM2_V1_StartupDrawRectFn fill_rect;
    DM2_V1_StartupDrawRectFn outline_rect;
    DM2_V1_StartupDrawTextFn draw_text;
} DM2_V1_StartupDrawExecutor;

int dm2_v1_startup_row_label(DM2_V1_StartupRowKind kind,
                             int slot,
                             char *out_label,
                             int out_label_size);
int dm2_v1_startup_presentation_build(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);
int dm2_v1_startup_presentation_build_from_snapshot(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);
int dm2_v1_startup_presentation_build_from_facts(
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);
int dm2_v1_startup_presentation_build_from_host_facts(
    const DM2_V1_StartupHostFacts *facts,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);
int dm2_v1_startup_presentation_render_receipt(
    const DM2_V1_StartupMenu *menu,
    const DM2_V1_StartupDrawCommand *commands,
    int command_count,
    int hud_runtime_ready,
    DM2_V1_StartupRenderReceipt *out_receipt);
int dm2_v1_startup_presentation_receipt(int startup_menu_active,
                                        char *out_phase,
                                        int out_phase_size,
                                        int *out_startup_active,
                                        char *out_animation,
                                        int out_animation_size,
                                        int *out_animation_active,
                                        int *out_title_frame,
                                        int *out_title_frame_max,
                                        int *out_title_ready);
int dm2_v1_startup_execute_draw_commands(
    const DM2_V1_StartupDrawCommand *commands,
    int command_count,
    const DM2_V1_StartupDrawExecutor *executor);

#endif
