/*
 * Pass 31 bounded probe — door actuation + wall-click sensor routing
 * migration.
 *
 * Verifies the new compat owners introduced in Pass 31:
 *   - F0715_DOOR_ResolveToggleAction_Compat computes open/close/destroyed
 *     targets for a door square without mutating state;
 *   - F0716_DOOR_RouteFrontCellClick_Compat categorises a click on the
 *     front cell into FRONT_DOOR_TOGGLE, FRONT_CELL_SENSOR, or NONE
 *     without executing a sensor or toggling a door.
 *
 * Like the Pass 30 probe, this builds a 4x4 in-memory fixture and does
 * not depend on DUNGEON.DAT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_door_action_pc34_compat.h"

#define MAP_W 4
#define MAP_H 4

static int g_pass = 0;
static int g_fail = 0;

static unsigned char sqb(int elementType, int attribs) {
    return (unsigned char)(((elementType & 7) << 5) | (attribs & 0x1F));
}

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static void build_fixture(struct DungeonDatState_Compat* dungeon,
                          unsigned char* squareData) {
    int c, r;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(squareData, 0, (size_t)MAP_W * MAP_H);
    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(struct DungeonMapDesc_Compat));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(struct DungeonMapTiles_Compat));
    dungeon->maps[0].width = MAP_W;
    dungeon->maps[0].height = MAP_H;
    dungeon->tiles[0].squareData = squareData;
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;

    for (c = 0; c < MAP_W; ++c) {
        for (r = 0; r < MAP_H; ++r) {
            squareData[c * MAP_H + r] = sqb(DUNGEON_ELEMENT_WALL, 0);
        }
    }

    /* Door squares:
     *   (0,1) DOOR open  (attrib 0, no things)
     *   (1,1) DOOR closed (attrib 4, no things)
     *   (2,1) DOOR destroyed (attrib 5, no things)
     *   (3,1) DOOR vertical closed (attrib 0x0C = 4 | 0x08 vertical, no things)
     * Corridor squares:
     *   (0,0) CORRIDOR (no things)
     *   (1,0) CORRIDOR with thing-list bit set (but we will not populate things)
     * Wall squares:
     *   (0,2) WALL with thing-list bit set (simulated button-wall)
     */
    squareData[0 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 0);
    squareData[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 4);
    squareData[2 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 5);
    squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_DOOR, 0x0C);

    squareData[0 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareData[0 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_WALL, 0);   /* no thing-list; things NULL in this probe */
}

static void free_fixture(struct DungeonDatState_Compat* dungeon) {
    free(dungeon->maps);
    free(dungeon->tiles);
    memset(dungeon, 0, sizeof(*dungeon));
}

int main(void) {
    struct DungeonDatState_Compat dungeon;
    unsigned char squareData[MAP_W * MAP_H];
    struct DoorToggleResult_Compat action;
    struct ClickOnWallResult_Compat click;
    struct PartyState_Compat party;
    int rc;

    build_fixture(&dungeon, squareData);

    /* ---- F0715 door toggle resolution ---- */

    /* Open door (state 0) -> CLOSE (new state 4). */
    rc = F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 0, 1, &action);
    record("P31_F0715_OPEN_TO_CLOSE",
           rc == 1 && action.kind == DOOR_ACTION_CLOSE &&
               action.oldDoorState == 0 && action.newDoorState == 4,
           "open door resolves to CLOSE with new state 4");

    /* Closed door (state 4) -> OPEN (new state 0). */
    rc = F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &action);
    record("P31_F0715_CLOSE_TO_OPEN",
           rc == 1 && action.kind == DOOR_ACTION_OPEN &&
               action.oldDoorState == 4 && action.newDoorState == 0,
           "closed door resolves to OPEN with new state 0");

    /* Destroyed door (state 5) -> DESTROYED (no mutation). */
    rc = F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 2, 1, &action);
    record("P31_F0715_DESTROYED",
           rc == 1 && action.kind == DOOR_ACTION_DESTROYED &&
               action.oldDoorState == 5 && action.newDoorState == -1,
           "destroyed door resolves to DESTROYED with no state mutation");

    /* Vertical door: orientation bit reported. */
    rc = F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 3, 1, &action);
    record("P31_F0715_VERTICAL",
           rc == 1 && action.doorVertical == 1 &&
               (action.kind == DOOR_ACTION_OPEN || action.kind == DOOR_ACTION_CLOSE),
           "vertical door reports doorVertical=1");

    /* Not a door: corridor at (0,0) returns 0. */
    rc = F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 0, 0, &action);
    record("P31_F0715_NOT_DOOR",
           rc == 0,
           "non-door square returns 0 and does not populate a toggle");

    /* NULL / OOB safety. */
    record("P31_F0715_NULL_OUT",
           F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 0, 0, NULL) == 0,
           "F0715 with NULL outResult returns 0");
    record("P31_F0715_OOB",
           F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, -1, 0, &action) == 0 &&
           F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 99, 0, 0, &action) == 0,
           "F0715 with out-of-range coordinates returns 0");

    /* ---- F0716 click-on-wall routing ---- */

    /* Party at (0,2) facing north: front cell is (0,1) = DOOR open. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 0; party.mapY = 2; party.direction = 0;
    rc = F0716_DOOR_RouteFrontCellClick_Compat(&dungeon, NULL, &party, &click);
    record("P31_F0716_FRONT_DOOR",
           rc == 1 && click.kind == CLICK_ON_WALL_FRONT_DOOR_TOGGLE &&
               click.elementType == DUNGEON_ELEMENT_DOOR &&
               click.doorState == 0 &&
               click.mapX == 0 && click.mapY == 1,
           "front cell door produces FRONT_DOOR_TOGGLE routing");

    /* Party at (1,2) facing north: front cell is (1,1) = DOOR closed. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 1; party.mapY = 2; party.direction = 0;
    rc = F0716_DOOR_RouteFrontCellClick_Compat(&dungeon, NULL, &party, &click);
    record("P31_F0716_FRONT_DOOR_CLOSED",
           rc == 1 && click.kind == CLICK_ON_WALL_FRONT_DOOR_TOGGLE &&
               click.doorState == 4,
           "front cell closed door still routes to FRONT_DOOR_TOGGLE (state=4)");

    /* Party at (0,1) facing west: front cell is (-1,1) = out of bounds. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 0; party.mapY = 1; party.direction = 3;
    rc = F0716_DOOR_RouteFrontCellClick_Compat(&dungeon, NULL, &party, &click);
    record("P31_F0716_OOB",
           (rc == 0 || click.kind == CLICK_ON_WALL_NONE),
           "click off the map bounds returns NONE");

    /* Party at (0,1) facing east: front cell is (1,1) = DOOR closed. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 0; party.mapY = 1; party.direction = 1;
    rc = F0716_DOOR_RouteFrontCellClick_Compat(&dungeon, NULL, &party, &click);
    record("P31_F0716_FRONT_DOOR_EAST",
           rc == 1 && click.kind == CLICK_ON_WALL_FRONT_DOOR_TOGGLE &&
               click.mapX == 1 && click.mapY == 1,
           "front-cell direction math is correct for east");

    /* Party at (0,3) facing north: front cell is (0,2) = WALL with no
     * thing-list bit set in this fixture → NONE (no button-like sensor). */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 0; party.mapY = 3; party.direction = 0;
    rc = F0716_DOOR_RouteFrontCellClick_Compat(&dungeon, NULL, &party, &click);
    record("P31_F0716_WALL_NO_SENSOR",
           (rc == 0 || click.kind == CLICK_ON_WALL_NONE) &&
               click.elementType == DUNGEON_ELEMENT_WALL,
           "wall with no thing-list returns NONE");

    /* NULL safety. */
    record("P31_F0716_NULL_OUT",
           F0716_DOOR_RouteFrontCellClick_Compat(&dungeon, NULL, &party, NULL) == 0,
           "F0716 with NULL outResult returns 0");

    free_fixture(&dungeon);

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
