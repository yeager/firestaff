
#ifndef NEXUS_V1_GAME_H
#define NEXUS_V1_GAME_H
#include <stdint.h>

/* Nexus V1 game state — ties together DM1 logic + Nexus 3D assets. */

typedef struct {
    int current_level;
    int party_x, party_y, party_dir;
    int champion_count;
    int game_started;
    const char *data_dir;
} Nexus_V1_GameState;

void nexus_v1_game_init(Nexus_V1_GameState *state, const char *data_dir);
int nexus_v1_game_load_level(Nexus_V1_GameState *state, int level);

/* CD Audio track mapping (Track 2-9 = game music) */
int nexus_v1_cd_track_for_level(int level);

#endif

