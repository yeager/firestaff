
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
    int needs_redraw;
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

/* Event system — 61 event types from DM.BIN yam\event.c.
 * String table at DM.BIN 0x036D04-0x037024. */
typedef enum {
    NEXUS_EV_NO_EVENT = 0,
    NEXUS_EV_SOFT_RESET,
    NEXUS_EV_DEBUG,
    NEXUS_EV_GO_FORWARD,
    NEXUS_EV_GO_BACKWARD,
    NEXUS_EV_GO_RIGHT,
    NEXUS_EV_GO_LEFT,
    NEXUS_EV_TURN_LEFT,
    NEXUS_EV_TURN_RIGHT,
    NEXUS_EV_CHG_VIEW,
    NEXUS_EV_GET_ITEM,
    NEXUS_EV_CANCEL,
    NEXUS_EV_INVENTORY,
    NEXUS_EV_EXIT_INV,
    NEXUS_EV_CHG_EXTINFO,
    NEXUS_EV_LEV_BOX,
    NEXUS_EV_SET_LEAD,
    NEXUS_EV_GO_3DVIEW,
    NEXUS_EV_SELECT_3DVIEW,
    NEXUS_EV_EXIT_3DVIEW,
    NEXUS_EV_FINDNEXT_3DVIEW,
    NEXUS_EV_GO_SPELL,
    NEXUS_EV_EXIT_SPELL,
    NEXUS_EV_MODE_CHG,
    NEXUS_EV_GO_MAP,
    NEXUS_EV_REST_PARTY,
    NEXUS_EV_WAKE_UP,
    NEXUS_EV_PAUSE_GAME,
    NEXUS_EV_RESUME_GAME,
    NEXUS_EV_DYNAMIC,
    NEXUS_EV_GETITEM,
    NEXUS_EV_SPELL,
    NEXUS_EV_DO_SPELL,
    NEXUS_EV_SPELL_CMD,
    NEXUS_EV_INV_SLOT,
    NEXUS_EV_CHG_PLAYER,
    NEXUS_EV_LOCKON,
    NEXUS_EV_INV_SEL,
    NEXUS_EV_OK,
    NEXUS_EV_RING_SELECT,
    NEXUS_EV_LIST_BOX,
    NEXUS_EV_SUBSQ,
    NEXUS_EV_MOUSE_EXIT,
    NEXUS_EV_MOTION_CHG,
    NEXUS_EV_INV_EQUIP,
    NEXUS_EV_PBOX,
    NEXUS_EV_MOUSE_CLICK,
    NEXUS_EV_EXIT,
    NEXUS_EV_CHG_SPD,
    NEXUS_EV_WIN_OK,
    NEXUS_EV_ANL_MOV,
    NEXUS_EV_ANL_MOV2,
    NEXUS_EV_ANL_RING,
    NEXUS_EV_ANL_LOOK,
    NEXUS_EV_PLRSET,
    NEXUS_EV_SAVE,
    NEXUS_EV_SAVELOAD,
    NEXUS_EV_WAIT,
    NEXUS_EV_SEMI_AUTO,
    NEXUS_EV_TITLE,
    NEXUS_EV_CONFIG,
    NEXUS_EV_COUNT
} Nexus_EventType;

typedef struct {
    Nexus_EventType type;
    int param1;
    int param2;
} Nexus_Event;

const char *nexus_v1_event_name(Nexus_EventType ev);
int nexus_v1_event_dispatch(Nexus_V1_GameState *state, const Nexus_Event *ev);

/* Translate an event type to a NEXUS_CMD_* command (nexus_v1_movement.h).
 * Returns NEXUS_CMD_NONE if the event has no direct command mapping.
 * Source: yam\event.c dispatch table → yam\inventry.c/timeline.c command queue. */
int nexus_v1_event_to_command(Nexus_EventType ev);

#endif

