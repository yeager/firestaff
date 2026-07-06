#ifndef NEXUS_V1_STARTUP_MENU_H
#define NEXUS_V1_STARTUP_MENU_H

#include <stdint.h>
#include <stddef.h>

#include "nexus_v1_save.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NEXUS_V1_STARTUP_ROW_NONE = 0,
    NEXUS_V1_STARTUP_ROW_SLOT = 1,
    NEXUS_V1_STARTUP_ROW_NEW_GAME = 2
} Nexus_V1_StartupRowKind;

typedef struct {
    char save_dir[512];
    unsigned int slot_mask;
    int row_count;
    int selected_row;
    Nexus_V1_SaveSlot slots[NEXUS_SAVE_MAX_SLOTS];
} Nexus_V1_StartupMenu;

void nexus_v1_startup_menu_init(Nexus_V1_StartupMenu *menu,
                                const char *save_dir);
int nexus_v1_startup_menu_scan(Nexus_V1_StartupMenu *menu);
int nexus_v1_startup_menu_row_at(const Nexus_V1_StartupMenu *menu,
                                 int row,
                                 Nexus_V1_StartupRowKind *out_kind,
                                 int *out_slot);
int nexus_v1_startup_menu_selected_path(const Nexus_V1_StartupMenu *menu,
                                        char *out_path,
                                        size_t out_path_size);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_STARTUP_MENU_H */
