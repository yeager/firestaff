#ifndef DM2_V1_STARTUP_MENU_H
#define DM2_V1_STARTUP_MENU_H

#include <stdint.h>

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
    DM2_V1_STARTUP_ACTION_NEW_GAME = 3
} DM2_V1_StartupActionKind;

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

void dm2_v1_startup_menu_init(DM2_V1_StartupMenu *menu,
                              const char *save_root);
int dm2_v1_startup_menu_count_rows(int resume_available,
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

#endif
