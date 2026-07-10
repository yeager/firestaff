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

/* The final M11 surface owns every decoded GDAT image for exactly one
 * presentation phase.  Keep that phase on the command instead of making the
 * host infer it from category/field pairs after decoding. */
typedef enum {
    DM2_V1_FRAME_OWNER_NONE = 0,
    DM2_V1_FRAME_OWNER_STARTUP_TITLE = 1,
    DM2_V1_FRAME_OWNER_STARTUP_MENU = 2
} DM2_V1_FrameOwner;

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
    DM2_V1_FrameOwner frame_owner;
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
    int menu_gdat_found;
    int menu_gdat_category;
    int menu_gdat_index;
    int menu_gdat_field;
    int skproject_title_query_ready;
    int skproject_menu_query_ready;
    int skproject_title_category;
    int skproject_title_index;
    int skproject_credit_screen_field;
    int skproject_menu_screen_field;
    DM2_V1_StartupRect title_rect;
    DM2_V1_StartupRect menu_rect;
    DM2_V1_StartupRect panel_rect;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int hud_runtime_ready;
    int hud_overlay_suppressed;
    int title_backdrop_ready;
    int resume_menu_ready;
    int save_slot_menu_ready;
    int new_game_menu_ready;
    int full_start_graphics_ready;
} DM2_V1_StartupRenderReceipt;

typedef struct DM2_V1_StartupViewReceipt {
    int valid;
    int command_count;
    DM2_V1_StartupMenuStateReceipt menu_state;
    DM2_V1_StartupRenderReceipt render;
    DM2_V1_StartupRuntimeHandoffReceipt runtime_handoff;
} DM2_V1_StartupViewReceipt;

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
int dm2_v1_startup_presentation_view_receipt_from_host_facts(
    const DM2_V1_StartupHostFacts *facts,
    int hud_runtime_ready,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands,
    DM2_V1_StartupViewReceipt *out_receipt);
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
