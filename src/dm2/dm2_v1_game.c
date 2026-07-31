#include "dm2_v1_game.h"
#include <string.h>

void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    /* This is allocation only.  A real new game receives its party pose
     * from the hash-verified G1 header in dm2_v1_boot_enter_game(); session
     * fields such as gold and time arrive only from their original owner.
     * Do not seed a playable-looking state with the former fixture values. */
}

int dm2_v1_load_dungeon(DM2_V1_GameState *state) {
    (void)state;
    /* This compatibility shim has no destination for the parsed map and used
     * to report success after discovery alone. SKProject GAME_LOAD reaches
     * c_loadlevel's complete map/record handoff before publishing gameplay;
     * Firestaff's equivalent is dm2_v1_boot_enter_game(). Never advertise a
     * found DUNGEON.DAT as loaded when this API has not built that state. */
    return -1;
}

int dm2_v1_enter_shop(DM2_V1_GameState *state) {
    (void)state;
    /* SKProject routes commerce through the active shop-glass actuator,
     * its WALL_GFX/GDAT image chain and record-owned stock. An outdoor flag
     * and a host gold field cannot identify any of those source owners. */
    return -1;
}

int dm2_v1_is_outdoor(const DM2_V1_GameState *state) {
    return state ? state->outdoor : 0;
}
