#ifndef DM2_V1_STARTUP_MENU_H
#define DM2_V1_STARTUP_MENU_H

#include <stdint.h>

#include "dm2_v1_startup_layout.h"

typedef enum {
    DM2_V1_STARTUP_ROW_NONE = 0,
    DM2_V1_STARTUP_ROW_CONTINUE = 1,
    DM2_V1_STARTUP_ROW_SLOT = 2,
    DM2_V1_STARTUP_ROW_NEW_GAME = 3
} DM2_V1_StartupRowKind;

typedef enum {
    DM2_V1_STARTUP_ACTION_NONE = 0,
    DM2_V1_STARTUP_ACTION_CONTINUE = 1,
    DM2_V1_STARTUP_ACTION_LOAD_SLOT = 2,
    DM2_V1_STARTUP_ACTION_NEW_GAME = 3,
    DM2_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER = 4
} DM2_V1_StartupActionKind;

typedef enum {
    DM2_V1_STARTUP_INPUT_NONE = 0,
    DM2_V1_STARTUP_INPUT_UP = 1,
    DM2_V1_STARTUP_INPUT_DOWN = 2,
    DM2_V1_STARTUP_INPUT_ACCEPT = 3,
    DM2_V1_STARTUP_INPUT_ACTION = 4,
    DM2_V1_STARTUP_INPUT_BACK = 5
} DM2_V1_StartupInput;

typedef struct {
    DM2_V1_StartupActionKind kind;
    int row;
    int slot;
} DM2_V1_StartupAction;

typedef struct {
    char save_root[512];
    int resume_available;
    unsigned int slot_mask;
    int row_count;
    int selected_row;
} DM2_V1_StartupMenu;

enum {
    DM2_V1_STARTUP_ROW_LABEL_CAPACITY = 48
};

typedef struct {
    DM2_V1_StartupRowKind kind;
    int row;
    int slot;
    int selected;
    DM2_V1_StartupRect rect;
    DM2_V1_StartupRect highlight_rect;
    int text_x;
    int text_y;
    char label[DM2_V1_STARTUP_ROW_LABEL_CAPACITY];
} DM2_V1_StartupRenderRow;

void dm2_v1_startup_menu_init(DM2_V1_StartupMenu *menu,
                              const char *save_root);
int dm2_v1_startup_menu_count_rows(int resume_available,
                                   unsigned int slot_mask);
int dm2_v1_startup_menu_refresh(DM2_V1_StartupMenu *menu,
                                int resume_available,
                                unsigned int slot_mask);
int dm2_v1_startup_menu_row_at(const DM2_V1_StartupMenu *menu,
                               int row,
                               DM2_V1_StartupRowKind *out_kind,
                               int *out_slot);
int dm2_v1_startup_menu_move_selected(DM2_V1_StartupMenu *menu,
                                      int delta);
int dm2_v1_startup_menu_activate_selected(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_input(DM2_V1_StartupMenu *menu,
                                     DM2_V1_StartupInput input,
                                     DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_hit(DM2_V1_StartupMenu *menu,
                                   const DM2_V1_StartupHit *hit,
                                   DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_build_render_rows(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupRenderRow *rows,
    int max_rows);
int dm2_v1_startup_receipt_phase(int startup_menu_active,
                                 char *out_phase,
                                 int out_phase_size,
                                 int *out_startup_active);

#endif
