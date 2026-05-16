
#include "nexus_v1_game.h"
#include <string.h>
#include <stdio.h>

void nexus_v1_game_init(Nexus_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 11;
    state->party_y = 29;
    state->party_dir = 0;  /* North — same as DM1 */
}

int nexus_v1_game_load_level(Nexus_V1_GameState *state, int level) {
    char path[512];
    if (!state || !state->data_dir || level < 0 || level > 15) return -1;
    snprintf(path, sizeof(path), "%s/LEV%02d.DGN", state->data_dir, level);
    state->current_level = level;
    printf("Nexus: loading level %d from %s\n", level, path);
    return 0;
}

/* Map dungeon levels to CD audio tracks (Track 2-9).
 * 8 audio tracks for 16 levels — each track covers 2 levels. */
int nexus_v1_cd_track_for_level(int level) {
    if (level < 0 || level > 15) return 2; /* default: track 2 */
    return 2 + (level / 2);  /* Track 2-9 */
}

