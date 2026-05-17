#ifndef DM1_V1_COLLISION_DOOR_PC34_COMPAT_H
#define DM1_V1_COLLISION_DOOR_PC34_COMPAT_H

/*
 * DM1 V1 Collision Detection and Door Interaction — unified feature module.
 *
 * Consolidates wall/pit/door collision logic with door-click interaction
 * into a single feature surface.  All functions are pure (read-only on
 * dungeon/things/party state) except where explicitly noted (door state
 * mutation on toggle).
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 *   MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE (line 316-850)
 *     — Master move-result resolver.  Checks destination square type,
 *       handles teleporter/pit chains, resolves sensor addition/removal.
 *       Our collision gate mirrors the destination-square element checks
 *       inside the loop at MOVESENS.C:481 (AL0709_i_DestinationSquareType).
 *
 *   CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty (line 180-347)
 *     — Party step dispatcher.  Resolves destination coordinates, checks:
 *       * Wall:     L1116_i_SquareType == C00_ELEMENT_WALL -> blocked (line 274)
 *       * Door:     M036_DOOR_STATE != 0,1,5 -> blocked (line 276-278)
 *       * FakeWall: !OPEN && !IMAGINARY -> blocked (line 280-281)
 *       * Group:    F0175_GROUP_GetThing != ENDOFLIST -> blocked (line 289-290)
 *       Uses G0233/G0234 direction-to-step tables for relative movement.
 *
 *   CLIKVIEW.C:F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor
 *     — Front-wall click: computes front cell from party position + direction,
 *       bounds-checks, then calls F0275_SENSOR_IsTriggeredByClickOnWall.
 *
 *   MOVESENS.C:F0275_SENSOR_IsTriggeredByClickOnWall (line 1309)
 *     — Identifies wall-click sensors and door actuators on the front cell.
 *       Routes to door toggle or sensor trigger based on square element type.
 *
 *   MOVESENS.C:F0264_MOVE_IsLevitating (line 128-170)
 *     — Determines if a thing levitates (skips pit fall).
 *       Groups: check creature attribute MASK0x0020_LEVITATION.
 *       Projectiles: always levitate.
 *       Explosions: levitate in later versions (BUG0_26 fix).
 *
 * Door state encoding (low 3 bits of square byte, DEFS.H):
 *   0 = C0_DOOR_STATE_OPEN
 *   1 = C1_DOOR_STATE_CLOSED_ONE_FOURTH  (party can pass through)
 *   2 = C2_DOOR_STATE_CLOSED_HALF        (blocks movement)
 *   3 = C3_DOOR_STATE_CLOSED_THREE_FOURTH (blocks movement)
 *   4 = C4_DOOR_STATE_CLOSED             (blocks movement)
 *   5 = C5_DOOR_STATE_DESTROYED          (permanently passable)
 *   Bit 3 (0x08): vertical door orientation
 *   Bit 4 (0x10): thing-list present
 *
 * Pit state encoding (DEFS.H):
 *   Bit 3 (0x08): MASK0x0008_PIT_OPEN — pit is open (fall through)
 *   Bit 0 (0x01): MASK0x0001_PIT_IMAGINARY — invisible pit, no fall
 */

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Door state constants (mirror ReDMCSB DEFS.H) ---- */
#define DM1_DOOR_STATE_OPEN                  0
#define DM1_DOOR_STATE_CLOSED_ONE_FOURTH     1
#define DM1_DOOR_STATE_CLOSED_HALF           2
#define DM1_DOOR_STATE_CLOSED_THREE_FOURTH   3
#define DM1_DOOR_STATE_CLOSED                4
#define DM1_DOOR_STATE_DESTROYED             5

/* ---- Pit attribute masks (mirror ReDMCSB DEFS.H) ---- */
#define DM1_PIT_MASK_OPEN       0x08  /* MASK0x0008_PIT_OPEN */
#define DM1_PIT_MASK_IMAGINARY  0x01  /* MASK0x0001_PIT_IMAGINARY */

/* ---- Collision result codes ---- */
#define DM1_COLLISION_PASSABLE          0
#define DM1_COLLISION_BLOCKED_WALL      1
#define DM1_COLLISION_BLOCKED_DOOR      2
#define DM1_COLLISION_BLOCKED_FAKEWALL  3
#define DM1_COLLISION_BLOCKED_BOUNDS    4
#define DM1_COLLISION_BLOCKED_GROUP     5

/* ---- Pit detection results ---- */
#define DM1_PIT_NONE                    0  /* Not a pit square */
#define DM1_PIT_OPEN_REAL               1  /* Open real pit — fall through */
#define DM1_PIT_OPEN_IMAGINARY          2  /* Open imaginary pit — no fall */
#define DM1_PIT_CLOSED                  3  /* Closed pit — safe to walk */

/* ---- Door click results ---- */
#define DM1_DOOR_CLICK_NONE             0
#define DM1_DOOR_CLICK_TOGGLED_OPEN     1  /* Door was closed, now opening */
#define DM1_DOOR_CLICK_TOGGLED_CLOSED   2  /* Door was open, now closing */
#define DM1_DOOR_CLICK_DESTROYED        3  /* Door is destroyed, no change */
#define DM1_DOOR_CLICK_NO_DOOR          4  /* Front cell is not a door */
#define DM1_DOOR_CLICK_NO_BUTTON        5  /* Front door has no DOOR->Button */

/* ---- Collision query result ---- */
struct Dm1V1CollisionResult {
    int code;               /* DM1_COLLISION_* */
    int elementType;        /* DUNGEON_ELEMENT_* of destination square */
    int doorState;          /* Door state (0-5) if element is DOOR, -1 otherwise */
    int doorPassable;       /* 1 if door state allows passage (0,1,5), 0 otherwise */
    int mapX;               /* Destination X */
    int mapY;               /* Destination Y */
    int mapIndex;           /* Destination map index */
};

/* ---- Pit query result ---- */
struct Dm1V1PitResult {
    int code;               /* DM1_PIT_* */
    int pitOpen;            /* 1 if MASK0x0008_PIT_OPEN set */
    int pitImaginary;       /* 1 if MASK0x0001_PIT_IMAGINARY set */
    int mapX;
    int mapY;
    int mapIndex;
};

/* ---- Door interaction result ---- */
struct Dm1V1DoorInteractionResult {
    int code;                       /* DM1_DOOR_CLICK_* */
    int frontMapX;                  /* Front cell X */
    int frontMapY;                  /* Front cell Y */
    int frontMapIndex;              /* Front cell map index */
    int previousDoorState;          /* Door state before interaction */
    int newDoorState;               /* Door state after interaction (-1 if no change) */
    int doorVertical;               /* 1 if vertical orientation */
    int doorHasButton;              /* 1 if decoded DOOR->Button gate passed */
    int hasSensorOnSquare;          /* 1 if front cell also has a sensor thing list */
    struct DoorToggleResult_Compat  toggleDetail;   /* Full compat toggle result */
    struct ClickOnWallResult_Compat clickDetail;    /* Full compat click routing */
};

/*
 * Check collision for a party step to a destination square.
 *
 * This is the unified collision gate that mirrors the checks inside
 * CLIKMENU.C:F0366 (lines 274-290):
 *   1. Wall -> DM1_COLLISION_BLOCKED_WALL
 *   2. Door with state not in {0,1,5} -> DM1_COLLISION_BLOCKED_DOOR
 *   3. Closed real fake-wall (not OPEN and not IMAGINARY) -> DM1_COLLISION_BLOCKED_FAKEWALL
 *   4. Group on destination (when championCount > 0) -> DM1_COLLISION_BLOCKED_GROUP
 *   5. Out of bounds -> DM1_COLLISION_BLOCKED_BOUNDS
 *   6. Everything else -> DM1_COLLISION_PASSABLE
 *
 * Pure function.  Does NOT modify party or dungeon state.
 *
 * Source: CLIKMENU.C:274-290 (element checks), MOVESENS.C:481 (F0267
 * destination square type in the teleporter/pit chain loop).
 */
int DM1_V1_Collision_CheckStep(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    int moveAction,
    struct Dm1V1CollisionResult* outResult);

/*
 * Check if a specific square is passable, with full door-state detail.
 *
 * Pure function.  Returns 1 if passable, 0 if blocked.
 *
 * Source: CLIKMENU.C:F0366 element checks; MOVESENS.C:F0267 chain checks.
 */
int DM1_V1_Collision_QuerySquare(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct Dm1V1CollisionResult* outResult);

/*
 * Detect pit state at a given position.
 *
 * Source: MOVESENS.C:F0267 (lines 493-500) — the pit branch inside
 * the chained-move loop:
 *   (AL0709_i_DestinationSquareType == C02_ELEMENT_PIT)
 *   && !L0713_B_ThingLevitates
 *   && M007_GET(AL0708_i_DestinationSquare, MASK0x0008_PIT_OPEN)
 *   && !M007_GET(AL0708_i_DestinationSquare, MASK0x0001_PIT_IMAGINARY)
 *
 * Pure function.
 */
int DM1_V1_Collision_DetectPit(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct Dm1V1PitResult* outResult);

/*
 * Process a door click interaction on the front cell.
 *
 * Combines F0716_DOOR_RouteFrontCellClick_Compat (click routing from
 * CLIKVIEW.C:F0372) with F0715_DOOR_ResolveToggleAction_Compat (door
 * toggle from MOVESENS.C:F0275) into a single call.
 *
 * Pure function — does NOT mutate dungeon state.
 *
 * Source: CLIKVIEW.C:F0377:365-385 (front DOOR->Button gate),
 * CLIKVIEW.C:F0372, MOVESENS.C:F0275.
 */
int DM1_V1_Door_ProcessClick(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    struct Dm1V1DoorInteractionResult* outResult);

/*
 * Apply a door toggle to the dungeon square byte.
 *
 * Single mutation point for player-initiated door state changes.
 *
 * Source: DUNGEON.C square byte mutation after F0275 dispatch.
 *
 * Returns 1 on success, 0 on invalid args or non-door square.
 */
int DM1_V1_Door_ApplyToggle(
    struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int newDoorState);

/*
 * Query whether a door is passable at its current state.
 *
 * Source: CLIKMENU.C:276-278.
 *
 * Returns 1 if passable, 0 if blocked, -1 if not a door square.
 */
int DM1_V1_Door_IsPassable(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY);

/*
 * Source evidence string for this feature module.
 */
const char* DM1_V1_CollisionDoor_SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_COLLISION_DOOR_PC34_COMPAT_H */

/* Column-major tile index — canonical per ReDMCSB DUNGEON.C F0151:
 *   G0271_ppuc_SquareFirstThingData[mapIndex][mapX * mapHeight + mapY]
 * C 2D arrays tiles[x][y] with tiles[DM1_MAX_MAP_W][DM1_MAX_MAP_H]
 * naturally store column-major: &tiles[x][0] is a column of height elements. */
static inline int dm1_tile_index(int mapX, int mapY, int mapHeight) {
    return mapX * mapHeight + mapY;
}
