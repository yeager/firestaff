
#ifndef NEXUS_V1_GAME_H
#define NEXUS_V1_GAME_H
#include <stdint.h>
#include "nexus_v1_dungeon.h"

/* Nexus V1 game state — ties together DM1 logic + Nexus 3D assets. */

/* The verified Nexus V1 new-game pose is level 0, cell (11,29), facing
 * north. It is a host-owned start request, but it must be accepted by the
 * decoded Structure1B cell before mechanics or the viewport consume it. */
#define NEXUS_V1_INITIAL_PARTY_LEVEL 0
#define NEXUS_V1_INITIAL_PARTY_X 11
#define NEXUS_V1_INITIAL_PARTY_Y 29
#define NEXUS_V1_INITIAL_PARTY_DIR 0

typedef enum {
    NEXUS_V1_DUNGEON_START_MISSING = 0,
    NEXUS_V1_DUNGEON_START_READY = 1,
    NEXUS_V1_DUNGEON_START_BLOCKED_LEVEL = 2,
    NEXUS_V1_DUNGEON_START_BLOCKED_COORDINATE = 3,
    NEXUS_V1_DUNGEON_START_BLOCKED_CELL = 4
} Nexus_V1_DungeonStartStatus;

typedef struct {
    Nexus_V1_DungeonStartStatus status;
    int level;
    int requested_x;
    int requested_y;
    int requested_dir;
    int party_x;
    int party_y;
    int party_dir;
    int square_type;
    uint16_t collision_ref;
    uint16_t post_grid_0x30_ref;
    int dmweb_container;
    int dgn_cell_consumed;
    int blocks_runtime;
    int fallback_visuals_permitted;
} Nexus_V1_DungeonStartReceipt;

typedef struct {
    int current_level;
    int party_x, party_y, party_dir;
    int champion_count;
    int tick_count;        /* total game ticks since start */
    int game_started;
    const char *data_dir;
    char level_path[512];
    Nexus_V1_DungeonStartReceipt dungeon_start;
} Nexus_V1_GameState;

void nexus_v1_game_init(Nexus_V1_GameState *state, const char *data_dir);
int nexus_v1_game_load_level(Nexus_V1_GameState *state, int level);
int nexus_v1_game_resolve_dungeon_start(
    const Nexus_V1_Level *level,
    int requested_level,
    int requested_x,
    int requested_y,
    int requested_dir,
    Nexus_V1_DungeonStartReceipt *out_receipt);
int nexus_v1_game_apply_dungeon_start(
    Nexus_V1_GameState *state,
    const Nexus_V1_DungeonStartReceipt *receipt);
const char *nexus_v1_dungeon_start_status_name(
    Nexus_V1_DungeonStartStatus status);

/* CD Audio track mapping (Track 2-9 = game music) */
int nexus_v1_cd_track_for_level(int level);

#endif
