/*
 * test_m12_screen_reader.c — compile verification for menu_startup_a11y_m12
 *
 * Compiles against the full firestaff header set to confirm:
 * 1. m12_screen_reader_update() signature is compatible
 * 2. FS_AX_* types and fs_ax_* functions from firestaff_accessibility.h are reachable
 * 3. M12_StartupMenuState is fully defined (not just forward-declared)
 * 4. M12_MenuView, M12_MENU_VIEW_MAIN, etc. are valid enum constants
 */

#include "menu_startup_a11y_m12.h"

int main(void)
{
    M12_StartupMenuState state;
    memset(&state, 0, sizeof(state));

    /* Verify M12_MenuView enum values are valid constants */
    state.view = M12_MENU_VIEW_MAIN;
    state.view = M12_MENU_VIEW_SETTINGS;
    state.view = M12_MENU_VIEW_GAME_OPTIONS;
    state.view = M12_MENU_VIEW_MUSEUM;
    state.view = M12_MENU_VIEW_MESSAGE;
    state.view = M12_MENU_VIEW_CHANGELOG;
    state.view = M12_MENU_VIEW_ITEM_ENCYCLOPEDIA;

    /* Verify M12_GameOptRow enum */
    int row_count = M12_GAME_OPT_ROW_COUNT;
    (void)row_count;

    /* Verify M12_MenuEntry kind */
    state.entries[0].kind = M12_MENU_ENTRY_GAME;
    state.entries[0].available = 1;
    state.entries[0].title = "Dungeon Master";

    /* Verify fs_ax_is_enabled() is linked */
    (void)fs_ax_is_enabled();

    /* Call the screen reader update — verifies symbol is resolved */
    m12_screen_reader_update(&state);

    return 0;
}