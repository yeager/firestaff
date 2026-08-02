
#include "nexus_v1_game.h"
#include "asset_find_by_hash.h"
#include <string.h>
#include <stdio.h>

static const char *const g_nexus_level_md5[16] = {
    "603ec9c531a92539babdda84ab09e78e",
    "751e1442bf7dccbd41bf146b5be144ab",
    "e2cb85d9fedc27f894a84e0f465fcde1",
    "19637d6b59849565f64565aed786d7ea",
    "85abc1b822e5c66ec4e99f1f676c140e",
    "ed5d54ab0ac1c927c1346dd966c8a5cc",
    "58c336ff6146e7216f0081e726823ea1",
    "c19e6038a017a320515ecbb66f6da197",
    "9bfc31bea631345a3660c2645be0e95b",
    "32a6450f29eb7babd73fcbe7a0310f22",
    "2928440e9c21457929f1323a28a42f70",
    "d7be5cd0d6e5c10afe99ec9950614fad",
    "db1cf70d6730615f73f191fad5e11e32",
    "f8876d0181d79727013236a6b597b99b",
    "a634dd5e95567ecbbbc332350c8cf12b",
    "5e6e237074f1e6b0decc629868a51f3c"
};

static int nexus_v1_file_exists(const char *path) {
    FILE *fp;
    if (!path || !path[0]) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

void nexus_v1_game_init(Nexus_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = NEXUS_V1_INITIAL_PARTY_X;
    state->party_y = NEXUS_V1_INITIAL_PARTY_Y;
    state->party_dir = NEXUS_V1_INITIAL_PARTY_DIR;
    state->dungeon_start.status = NEXUS_V1_DUNGEON_START_MISSING;
    state->dungeon_start.level = NEXUS_V1_INITIAL_PARTY_LEVEL;
    state->dungeon_start.requested_x = NEXUS_V1_INITIAL_PARTY_X;
    state->dungeon_start.requested_y = NEXUS_V1_INITIAL_PARTY_Y;
    state->dungeon_start.requested_dir = NEXUS_V1_INITIAL_PARTY_DIR;
    state->dungeon_start.party_x = NEXUS_V1_INITIAL_PARTY_X;
    state->dungeon_start.party_y = NEXUS_V1_INITIAL_PARTY_Y;
    state->dungeon_start.party_dir = NEXUS_V1_INITIAL_PARTY_DIR;
    state->dungeon_start.blocks_runtime = 1;
}

int nexus_v1_game_resolve_dungeon_start(
    const Nexus_V1_Level *level,
    int requested_level,
    int requested_x,
    int requested_y,
    int requested_dir,
    Nexus_V1_DungeonStartReceipt *out_receipt)
{
    Nexus_V1_DungeonStartReceipt receipt;
    Nexus_V1_DgnCellGeometry cell;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DUNGEON_START_MISSING;
    receipt.level = requested_level;
    receipt.requested_x = requested_x;
    receipt.requested_y = requested_y;
    receipt.requested_dir = requested_dir & 3;
    receipt.party_x = requested_x;
    receipt.party_y = requested_y;
    receipt.party_dir = requested_dir & 3;
    receipt.fallback_visuals_permitted = 0;
    receipt.blocks_runtime = 1;

    if (!level || level->width <= 0 || level->height <= 0) {
        *out_receipt = receipt;
        return 0;
    }
    if (requested_level != NEXUS_V1_INITIAL_PARTY_LEVEL) {
        receipt.status = NEXUS_V1_DUNGEON_START_BLOCKED_LEVEL;
        *out_receipt = receipt;
        return 0;
    }
    if (requested_x < 0 || requested_y < 0 ||
        requested_x >= level->width || requested_y >= level->height ||
        nexus_v1_level_get_cell_geometry(level, requested_x, requested_y,
                                         &cell) != 0) {
        receipt.status = NEXUS_V1_DUNGEON_START_BLOCKED_COORDINATE;
        *out_receipt = receipt;
        return 0;
    }

    receipt.square_type = cell.square_type;
    receipt.collision_ref = cell.collision_ref;
    receipt.post_grid_0x30_ref = cell.post_grid_0x30_ref;
    receipt.dmweb_container = level->geometry_info.dmweb_container;
    receipt.dgn_cell_consumed = 1;
    if (cell.square_type == 0 || cell.collision_ref == 0x0fffU) {
        receipt.status = NEXUS_V1_DUNGEON_START_BLOCKED_CELL;
        *out_receipt = receipt;
        return 0;
    }

    receipt.status = NEXUS_V1_DUNGEON_START_READY;
    receipt.blocks_runtime = 0;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_game_apply_dungeon_start(
    Nexus_V1_GameState *state,
    const Nexus_V1_DungeonStartReceipt *receipt)
{
    if (!state || !receipt || receipt->status != NEXUS_V1_DUNGEON_START_READY ||
        receipt->blocks_runtime || receipt->fallback_visuals_permitted) {
        return 0;
    }
    state->current_level = receipt->level;
    state->party_x = receipt->party_x;
    state->party_y = receipt->party_y;
    state->party_dir = receipt->party_dir & 3;
    state->dungeon_start = *receipt;
    return 1;
}

const char *nexus_v1_dungeon_start_status_name(
    Nexus_V1_DungeonStartStatus status)
{
    switch (status) {
    case NEXUS_V1_DUNGEON_START_MISSING: return "missing";
    case NEXUS_V1_DUNGEON_START_READY: return "ready";
    case NEXUS_V1_DUNGEON_START_BLOCKED_LEVEL: return "blocked-level";
    case NEXUS_V1_DUNGEON_START_BLOCKED_COORDINATE: return "blocked-coordinate";
    case NEXUS_V1_DUNGEON_START_BLOCKED_CELL: return "blocked-cell";
    default: return "unknown";
    }
}

int nexus_v1_game_load_level(Nexus_V1_GameState *state, int level) {
    char path[512];
    if (!state || !state->data_dir || level < 0 || level > 15) return -1;

    if (asset_find_by_md5(state->data_dir,
                          g_nexus_level_md5[level],
                          path,
                          (int)sizeof(path),
                          8)) {
        snprintf(state->level_path, sizeof(state->level_path), "%s", path);
        state->current_level = level;
        printf("Nexus: loading level %d from %s\n", level, state->level_path);
        return 0;
    }

    snprintf(path, sizeof(path), "%s/LEV%02d.DGN", state->data_dir, level);
    if (!nexus_v1_file_exists(path)) {
        return -1;
    }
    snprintf(state->level_path, sizeof(state->level_path), "%s", path);
    state->current_level = level;
    printf("Nexus: loading level %d from %s\n", level, state->level_path);
    return 0;
}

/* Map dungeon levels to CD audio tracks (Track 2-9).
 * 8 audio tracks for 16 levels — each track covers 2 levels. */
int nexus_v1_cd_track_for_level(int level) {
    if (level < 0 || level > 15) return 2;
    return 2 + (level / 2);
}

/* Event name table — matches DM.BIN string table at 0x036D04-0x037024.
 * Source: yam\event.c */
static const char *const g_event_names[NEXUS_EV_COUNT] = {
    "EV_NO_EVENT", "EV_SOFT_RESET", "EV_DEBUG",
    "EV_GO_FORWARD", "EV_GO_BACKWARD", "EV_GO_RIGHT", "EV_GO_LEFT",
    "EV_TURN_LEFT", "EV_TURN_RIGHT", "EV_CHG_VIEW",
    "EV_GET_ITEM", "EV_CANCEL", "EV_INVENTORY", "EV_EXIT_INV",
    "EV_CHG_EXTINFO", "EV_LEV_BOX", "EV_SET_LEAD",
    "EV_GO_3DVIEW", "EV_SELECT_3DVIEW", "EV_EXIT_3DVIEW",
    "EV_FINDNEXT_3DVIEW",
    "EV_GO_SPELL", "EV_EXIT_SPELL", "EV_MODE_CHG", "EV_GO_MAP",
    "EV_REST_PARTY", "EV_WAKE_UP", "EV_PAUSE_GAME", "EV_RESUME_GAME",
    "EV_DYNAMIC", "EV_GETITEM", "EV_SPELL", "EV_DO_SPELL",
    "EV_SPELL_CMD", "EV_INV_SLOT", "EV_CHG_PLAYER", "EV_LOCKON",
    "EV_INV_SEL", "EV_OK", "EV_RING_SELECT", "EV_LIST_BOX",
    "EV_SUBSQ", "EV_MOUSE_EXIT", "EV_MOTION_CHG", "EV_INV_EQUIP",
    "EV_PBOX", "EV_MOUSE_CLICK", "EV_EXIT", "EV_CHG_SPD",
    "EV_WIN_OK",
    "EV_ANL_MOV", "EV_ANL_MOV2", "EV_ANL_RING", "EV_ANL_LOOK",
    "EV_PLRSET", "EV_SAVE", "EV_SAVELOAD", "EV_WAIT",
    "EV_SEMI_AUTO", "EV_TITLE", "EV_CONFIG"
};

const char *nexus_v1_event_name(Nexus_EventType ev) {
    if (ev < 0 || ev >= NEXUS_EV_COUNT) return "EV_UNKNOWN";
    return g_event_names[ev];
}

int nexus_v1_event_dispatch(Nexus_V1_GameState *state, const Nexus_Event *ev) {
    if (!state || !ev) return -1;

    switch (ev->type) {
    case NEXUS_EV_NO_EVENT:
        return 0;

    case NEXUS_EV_GO_FORWARD:
    case NEXUS_EV_GO_BACKWARD:
    case NEXUS_EV_GO_RIGHT:
    case NEXUS_EV_GO_LEFT:
    case NEXUS_EV_TURN_LEFT:
    case NEXUS_EV_TURN_RIGHT:
        state->tick_count++;
        return 1;

    case NEXUS_EV_GET_ITEM:
    case NEXUS_EV_GETITEM:
        return 1;

    case NEXUS_EV_INVENTORY:
    case NEXUS_EV_EXIT_INV:
    case NEXUS_EV_INV_SLOT:
    case NEXUS_EV_INV_SEL:
    case NEXUS_EV_INV_EQUIP:
        return 1;

    case NEXUS_EV_GO_SPELL:
    case NEXUS_EV_EXIT_SPELL:
    case NEXUS_EV_SPELL:
    case NEXUS_EV_DO_SPELL:
    case NEXUS_EV_SPELL_CMD:
        return 1;

    case NEXUS_EV_REST_PARTY:
    case NEXUS_EV_WAKE_UP:
    case NEXUS_EV_PAUSE_GAME:
    case NEXUS_EV_RESUME_GAME:
        return 1;

    case NEXUS_EV_SAVE:
    case NEXUS_EV_SAVELOAD:
        return 1;

    default:
        return 0;
    }
}
