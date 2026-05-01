#ifndef REDMCSB_MEMORY_MOVEMENT_PC34_COMPAT_H
#define REDMCSB_MEMORY_MOVEMENT_PC34_COMPAT_H

/*
 * Movement data layer for ReDMCSB PC 3.4.
 *
 * Pure functions for party/thing movement — NO side effects, NO UI,
 * NO render, NO sound, NO sensor execution, NO combat.
 *
 * Source: MOVESENS.C, DEFS.H from the original DM/CSB source.
 *
 * v2-design: Movement is a pure function of (position, input) → new position.
 * Sensor identification is separate from execution.
 */

#include <stdint.h>

#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* ---- Movement input actions ---- */
#define MOVE_FORWARD    0
#define MOVE_RIGHT      1
#define MOVE_BACKWARD   2
#define MOVE_LEFT       3
#define MOVE_TURN_RIGHT 4
#define MOVE_TURN_LEFT  5
#define MOVE_ACTION_COUNT 6

/* ---- Movement result codes ---- */
#define MOVE_OK             0
#define MOVE_BLOCKED_WALL   1
#define MOVE_BLOCKED_DOOR   2
#define MOVE_BLOCKED_BOUNDS 3
#define MOVE_TURN_ONLY      4  /* Turn succeeded, no position change */

/* ---- Movement result struct ---- */
struct MovementResult_Compat {
    int resultCode;       /* MOVE_OK, MOVE_BLOCKED_*, MOVE_TURN_ONLY */
    int newMapX;          /* New X after move (or unchanged if blocked) */
    int newMapY;          /* New Y after move */
    int newDirection;     /* New direction after move/turn */
    int newMapIndex;      /* New map index (same unless teleporter — future) */
};

#define MOVEMENT_POST_MOVE_CHAIN_LIMIT 100

struct PostMoveResolution_Compat {
    int transitioned;
    int chainCount;
    int pitCount;
    int teleporterCount;
    int finalMapX;
    int finalMapY;
    int finalDirection;
    int finalMapIndex;
    int championFallDamage[CHAMPION_MAX_PARTY];
};

/*
 * Compute next direction after a turn.
 * Pure function, no side effects.
 *
 * turnRight: DIR_NORTH→DIR_EAST→DIR_SOUTH→DIR_WEST→DIR_NORTH
 * turnLeft:  DIR_NORTH→DIR_WEST→DIR_SOUTH→DIR_EAST→DIR_NORTH
 */
int F0700_MOVEMENT_TurnDirection_Compat(int currentDir, int turnRight);

/*
 * Compute the map offset (dx, dy) for a movement action given current direction.
 *
 * Forward/North:  dx=0, dy=-1
 * Forward/East:   dx=+1, dy=0
 * Forward/South:  dx=0, dy=+1
 * Forward/West:   dx=-1, dy=0
 *
 * Left/Right/Backward are relative to facing direction.
 */
void F0701_MOVEMENT_GetStepDelta_Compat(
    int direction,
    int moveAction,
    int* outDx,
    int* outDy);

/*
 * Attempt a movement action for the party.
 *
 * Pure function: reads dungeon tile data to check walkability,
 * writes result to outResult.  Does NOT modify party state.
 *
 * For turns (MOVE_TURN_LEFT, MOVE_TURN_RIGHT): always succeeds,
 * sets resultCode = MOVE_TURN_ONLY.
 *
 * For steps (MOVE_FORWARD/BACKWARD/LEFT/RIGHT): checks bounds and
 * source-semantic square passability (wall, door state, fake-wall bits,
 * stairs consequence square).
 *
 * Returns 1 if result is MOVE_OK or MOVE_TURN_ONLY, 0 if blocked.
 */
int F0702_MOVEMENT_TryMove_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    int moveAction,
    struct MovementResult_Compat* outResult);

/* ---- Sensor identification on square ---- */

/*
 * Sensor identification result — which sensors exist on a square.
 * Does NOT execute the sensor, just identifies it.
 */
struct SensorOnSquare_Compat {
    int found;                   /* 1 if at least one sensor found */
    int sensorIndex;             /* Index into things.sensors[] of FIRST sensor */
    int sensorType;              /* Type code of first sensor */
    unsigned short sensorData;   /* Data of first sensor */
    /* Remote target (if applicable) */
    int targetMapX;
    int targetMapY;
    int targetCell;
    int isLocal;                 /* 1 if localEffect, 0 if remote */
    int totalSensorsOnSquare;    /* How many sensors chained on this square */
};

/*
 * Identify sensors on a given square.
 *
 * Walks the thing linked-list for the given square and finds
 * all SENSOR things.  Reports the FIRST sensor found.
 *
 * Does NOT execute any sensor logic.
 *
 * Returns 1 if at least one sensor found, 0 otherwise.
 */
int F0703_MOVEMENT_IdentifySensorsOnSquare_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex,
    int mapX,
    int mapY,
    struct SensorOnSquare_Compat* outSensor);

int F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    uint32_t gameTick,
    struct PostMoveResolution_Compat* outResolution);

/* ---- Stairs transition resolution ---- */

struct StairsTransitionResult_Compat {
    int transitioned;   /* 1 if a stairs square was consumed */
    int stairUp;        /* 1 = ascended, 0 = descended (only valid when transitioned) */
    int fromMapIndex;   /* map index before the transition */
    int toMapIndex;     /* map index after the transition */
    int newMapX;        /* clamped destination X on the target map */
    int newMapY;        /* clamped destination Y on the target map */
    int newDirection;   /* F0155_DUNGEON_GetStairsExitDirection result */
};

/*
 * If the party currently stands on a stairs square, resolve the level
 * transition: decide ascend/descend from MASK0x0004_STAIRS_UP, map
 * through F0154_DUNGEON_GetLocationAfterLevelChange semantics
 * (offsetMapX/offsetMapY + Level delta, not map-index +/-), and set the
 * exit facing via F0155_DUNGEON_GetStairsExitDirection.  Pure function —
 * does NOT mutate the party.  Returns 1 if a stairs transition was
 * resolved (transitioned=1), 0 otherwise.
 *
 * Source mapping: CLIKMENU.C:F0364_COMMAND_TakeStairs and
 * DUNGEON.C:F0154/F0155.
 */
int F0705_MOVEMENT_ResolveStairsTransition_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    struct StairsTransitionResult_Compat* outResult);

/* ---- Shared square passability ---- */

/*
 * Shared source-faithful square passability helper used by both party
 * movement legality and creature walkability decisions.  Corresponds to
 * the square-element checks inside F0366_COMMAND_ProcessTypes3To6_MoveParty /
 * DUNGEON.C: walls block, corridor/pit/teleporter are open, fake-walls
 * are passable only when OPEN (0x04) or IMAGINARY (0x01), stairs are
 * passable as a consequence square, doors are passable when the
 * door-state low bits are open (0), closed one-fourth (1), or destroyed
 * (5).  No thing-list checks — those are for sensor/group processing.
 *
 * Returns 1 if the square is passable, 0 otherwise.  Out-of-bounds or
 * missing tile data returns 0.
 */
int F0706_MOVEMENT_IsSquarePassable_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY);

/*
 * Passability context flag.
 *
 * F0706 is the party-side square-passability owner: the party may step
 * onto a stairs square because, per F0267_MOVE_GetMoveResult_CPSCE,
 * stairs are a legal "consequence" square that triggers a map
 * transition.  Creatures in DM1 PC 3.4, however, never enter stairs
 * squares — the ReDMCSB creature-movement legality in GROUP.C /
 * F0264_MOVE_IsSquareAccessibleForCreature treats stairs as blocked.
 *
 * MOVEMENT_PASS_CTX_PARTY preserves the existing F0706 behaviour.
 * MOVEMENT_PASS_CTX_CREATURE rejects stairs and otherwise shares the
 * same element/door decoding as F0706 so party and creature
 * walkability stay in sync for walls, corridors, pits, teleporters,
 * fake-wall bits, and door-state bits.
 */
#define MOVEMENT_PASS_CTX_PARTY    0
#define MOVEMENT_PASS_CTX_CREATURE 1

/*
 * Pass 39: shared square-passability owner with an explicit context.
 *
 * Creature context rejects stairs (source: ReDMCSB creature-movement
 * legality in GROUP.C / F0264_MOVE_IsSquareAccessibleForCreature);
 * every other element decodes identically to F0706.  This keeps one
 * source-faithful decoder and prevents the divergent custom rules that
 * used to live in m11_square_walkable_for_creature.
 *
 * Returns 1 if the square is passable for the given context, 0
 * otherwise.  Out-of-bounds or missing tile data returns 0.
 */
int F0707_MOVEMENT_IsSquarePassableForContext_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int passContext);

/*
 * Source-locked party/group collision gate.
 *
 * ReDMCSB CLIKMENU.C / F0366_COMMAND_ProcessTypes3To6_MoveParty first
 * rejects impassable destination squares (wall, closed door, closed real
 * fake-wall).  Only if that square is otherwise passable, and only when the
 * party has at least one champion, it checks F0175_GROUP_GetThing on the
 * destination square and blocks the step when a creature group is present.
 * The original empty-party bug is preserved here: championCount == 0 skips
 * the group collision check.
 *
 * Returns 1 when the requested cardinal step is blocked by a group, 0
 * otherwise.  Turn-only commands never report group blocking.
 */
int F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    int moveAction);

/*
 * Source-locked movement/projectile interlock cell resolver.
 *
 * ReDMCSB MOVESENS.C:F0266 builds two four-cell occupancy maps while
 * a party or creature group moves one square: one for impacts on the
 * source square and, only for adjacent moves where an edge occupant crosses
 * between cells, an intermediary map used to test projectiles already on
 * the destination square before the final move is linked.  This helper
 * implements only that pure cell-map transformation; projectile deletion,
 * damage, and event mutation remain outside the movement core.
 *
 * sourceOrdinalInCell values are ReDMCSB ordinals (0 empty, 1..4 occupied).
 * Returns 1 when destination-square projectile impacts must be checked, 0
 * otherwise.  Non-adjacent moves leave both output maps as the source map
 * and return 0.
 *
 * Source: MOVESENS.C:272-310, including the documented cell-2 -> cell-3
 * intermediary case; impact predicate downstream is PROJEXPL.C:F0217.
 */
int F0709_MOVEMENT_BuildIntermediaryProjectileImpactCells_Compat(
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    const unsigned char sourceOrdinalInCell[4],
    unsigned char destinationOrdinalInCell[4],
    unsigned char intermediaryOrdinalInCell[4]);

#endif
