#ifndef NEXUS_V1_STARTUP_MENU_H
#define NEXUS_V1_STARTUP_MENU_H

#include <stdint.h>
#include <stddef.h>

#include "nexus_v1_champions.h"
#include "nexus_v1_save.h"
#include "nexus_v1_startup_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NEXUS_V1_STARTUP_ROW_NONE = 0,
    NEXUS_V1_STARTUP_ROW_SLOT = 1,
    NEXUS_V1_STARTUP_ROW_NEW_GAME = 2
} Nexus_V1_StartupRowKind;

typedef enum {
    NEXUS_V1_STARTUP_ACTION_NONE = 0,
    NEXUS_V1_STARTUP_ACTION_LOAD_SLOT = 1,
    NEXUS_V1_STARTUP_ACTION_NEW_GAME = 2,
    NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER = 3,
    NEXUS_V1_STARTUP_ACTION_HOLD_TITLE = 4,
    NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT = 5,
    NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT = 6,
    NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE = 7,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR = 8,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED = 9,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_SKIPPED = 10,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_REMOVED = 11,
    NEXUS_V1_STARTUP_ACTION_START_DUNGEON = 12,
    NEXUS_V1_STARTUP_ACTION_NEED_CHAMPION = 13
} Nexus_V1_StartupActionKind;

typedef enum {
    NEXUS_V1_STARTUP_INPUT_NONE = 0,
    NEXUS_V1_STARTUP_INPUT_UP = 1,
    NEXUS_V1_STARTUP_INPUT_DOWN = 2,
    NEXUS_V1_STARTUP_INPUT_ACCEPT = 3,
    NEXUS_V1_STARTUP_INPUT_ACTION = 4,
    NEXUS_V1_STARTUP_INPUT_BACK = 5
} Nexus_V1_StartupInput;

typedef struct {
    Nexus_V1_StartupActionKind kind;
    int row;
    int slot;
    char path[512];
} Nexus_V1_StartupAction;

typedef struct {
    char save_dir[512];
    unsigned int slot_mask;
    int row_count;
    int selected_row;
    Nexus_V1_SaveSlot slots[NEXUS_SAVE_MAX_SLOTS];
} Nexus_V1_StartupMenu;

enum {
    NEXUS_V1_STARTUP_SAVE_ROW_LABEL_CAPACITY = 96
};

typedef struct {
    Nexus_V1_StartupRowKind kind;
    int row;
    int slot;
    int selected;
    Nexus_V1_StartupRect rect;
    Nexus_V1_StartupRect highlight_rect;
    int text_x;
    int text_y;
    char label[NEXUS_V1_STARTUP_SAVE_ROW_LABEL_CAPACITY];
} Nexus_V1_StartupSaveRenderRow;

void nexus_v1_startup_menu_init(Nexus_V1_StartupMenu *menu,
                                const char *save_dir);
int nexus_v1_startup_menu_scan(Nexus_V1_StartupMenu *menu);
int nexus_v1_startup_menu_refresh(Nexus_V1_StartupMenu *menu,
                                  unsigned int slot_mask);
int nexus_v1_startup_menu_row_at(const Nexus_V1_StartupMenu *menu,
                                 int row,
                                 Nexus_V1_StartupRowKind *out_kind,
                                 int *out_slot);
int nexus_v1_startup_menu_selected_path(const Nexus_V1_StartupMenu *menu,
                                        char *out_path,
                                        size_t out_path_size);
int nexus_v1_startup_menu_move_selected(Nexus_V1_StartupMenu *menu,
                                        int delta);
int nexus_v1_startup_menu_activate_selected(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_title_handle_input(int title_frame,
                                        unsigned int slot_mask,
                                        Nexus_V1_StartupInput input,
                                        Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_boot_handle_input(int boot_frame,
                                       unsigned int slot_mask,
                                       Nexus_V1_StartupInput input,
                                       Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_title_handle_hit(int title_frame,
                                      unsigned int slot_mask,
                                      Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_input(Nexus_V1_StartupMenu *menu,
                                       Nexus_V1_StartupInput input,
                                       Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_hit(Nexus_V1_StartupMenu *menu,
                                     const Nexus_V1_StartupHit *hit,
                                     Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_build_save_render_rows(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupSaveRenderRow *rows,
    int max_rows);
int nexus_v1_startup_champion_handle_input(Nexus_V1_ChampionPool *pool,
                                           int *cursor,
                                           unsigned int slot_mask,
                                           Nexus_V1_StartupInput input,
                                           Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_hit(Nexus_V1_ChampionPool *pool,
                                         int *cursor,
                                         unsigned int slot_mask,
                                         const Nexus_V1_StartupHit *hit,
                                         Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_receipt_phase(int title_active,
                                   int save_select_active,
                                   int champion_select_active,
                                   int title_frame,
                                   char *out_phase,
                                   int out_phase_size,
                                   int *out_startup_active,
                                   int *out_startup_frame);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_STARTUP_MENU_H */
