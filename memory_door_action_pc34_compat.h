#ifndef REDMCSB_MEMORY_DOOR_ACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_DOOR_ACTION_PC34_COMPAT_H

/*
 * Door / click-on-wall action layer for ReDMCSB PC 3.4 — Passes 31, 38.
 *
 * Source-faithful owners for:
 *   - resolving the target door-state when the party toggles / clicks
 *     the front-cell door (door_action path in MOVESENS.C / DUNGEON.C,
 *     adjacent to F0275_SENSOR_IsTriggeredByClickOnWall);
 *   - routing a click on a front wall/cell to the correct category of
 *     actuator or sensor trigger;
 *   - stepping an animating door through intermediate states 1..3 and
 *     building/advancing the TIMELINE_EVENT_DOOR_ANIMATE event that
 *     carries the animation (Pass 38 — models F0241_TIMELINE_Process
 *     Event1_DoorAnimation in TIMELINE.C).
 *
 * Core helpers are pure: they read square bytes / things tables,
 * produce a structured result, and never mutate dungeon/party state
 * themselves.  Callers (compat runtime or thin M11 shims) apply the
 * result — this is what moves door state ownership out of
 * m11_game_view.c and back into the compat layer.
 *
 * v1 coverage:
 *   - Front-door toggle: OPEN (state 0) <-> CLOSED (state 4) with the
 *     three animating intermediate states 1..3 (Pass 38) dispatched by
 *     the tick orchestrator through TIMELINE_EVENT_DOOR_ANIMATE.
 *     DESTROYED (state 5) is surfaced with DOOR_ACTION_KIND_DESTROYED
 *     so the caller can emit the "no longer blocking" message without
 *     re-mutating the square.
 *   - Click-on-wall routing: distinguishes FRONT_DOOR_TOGGLE vs
 *     FRONT_CELL_SENSOR_TRIGGER (routed through the sensor compat
 *     identification helper) vs NO_ACTION.  Does not execute the
 *     sensor effect itself; Pass 32 wires that into the runtime.
 *
 * Out-of-scope for Pass 38:
 *   - Champion damage from a closing horizontal/vertical door on the
 *     party square (ReDMCSB F0241 branch, BUG0_78 in the PC 3.4 source).
 *   - Creature damage / death from a closing door on the creature
 *     square (F0191_GROUP_GetDamageAllCreaturesOutcome branch).
 *   - Sensor-driven door actuation re-entering the animation scheduler
 *     (still routed through the M11 shim in Pass 31 scope).
 */

#include <stdint.h>

#include "memory_movement_pc34_compat.h"

struct TimelineEvent_Compat;  /* forward decl; defined in memory_timeline_pc34_compat.h */

/* ---- Door action kinds ---- */
#define DOOR_ACTION_NONE                 0
#define DOOR_ACTION_OPEN                 1  /* door was closed, target state = 0 */
#define DOOR_ACTION_CLOSE                2  /* door was open,   target state = 4 */
#define DOOR_ACTION_DESTROYED            3  /* destroyed; no state change */

/* ---- Click-on-wall routing kinds ---- */
#define CLICK_ON_WALL_NONE               0  /* nothing to do (empty wall, out of range) */
#define CLICK_ON_WALL_FRONT_DOOR_TOGGLE  1  /* front cell is a door; caller should run the door action */
#define CLICK_ON_WALL_FRONT_CELL_SENSOR  2  /* front cell hosts a sensor thing list */

/* ---- Door-animation effects (mirror ReDMCSB C00/C01/C02 effect codes) ---- */
#define DOOR_EFFECT_SET                  0  /* opening: state walks CLOSED..OPEN (4..0) */
#define DOOR_EFFECT_CLEAR                1  /* closing: state walks OPEN..CLOSED (0..4) */
#define DOOR_EFFECT_TOGGLE               2  /* caller requests toggle; F0714 resolves to SET/CLEAR */

/* ---- Door-animation step result ---- */
#define DOOR_ANIM_STEP_NO_CHANGE         0  /* destroyed / already at target / not a door — stop */
#define DOOR_ANIM_STEP_ADVANCED          1  /* newState is a new intermediate step; reschedule */
#define DOOR_ANIM_STEP_REACHED_TARGET    2  /* newState is the final state (0 or 4); do not reschedule */

struct DoorAnimationStep_Compat {
    int kind;               /* DOOR_ANIM_STEP_* */
    int mapIndex;
    int mapX;
    int mapY;
    int effect;             /* DOOR_EFFECT_SET or DOOR_EFFECT_CLEAR (resolved; never TOGGLE) */
    int oldDoorState;       /* state before this step (0..5) */
    int newDoorState;       /* state after this step  (0..5) */
    int doorVertical;       /* 1 if the door byte has the vertical bit (0x08) */
};

/* ---- Door toggle result ---- */
struct DoorToggleResult_Compat {
    int kind;               /* DOOR_ACTION_* */
    int mapIndex;           /* target square map index */
    int mapX;               /* target square X */
    int mapY;               /* target square Y */
    int oldDoorState;       /* previous low-nibble state (0..7), -1 if N/A */
    int newDoorState;       /* new low-nibble state (0..7), -1 if no mutation */
    int doorVertical;       /* 1 if the door is vertical orientation (bit 3) */
};

/* ---- Click-on-wall routing result ---- */
struct ClickOnWallResult_Compat {
    int kind;               /* CLICK_ON_WALL_* */
    int mapIndex;
    int mapX;
    int mapY;
    int elementType;        /* DUNGEON_ELEMENT_* of the clicked square */
    int doorState;          /* low nibble for DOOR; -1 otherwise */
    int hasSensor;          /* 1 if the clicked square has a sensor thing list */
};

/*
 * Resolve the target door action for a toggle at (mapIndex, mapX, mapY).
 *
 * Pure: reads the square byte via the dungeon tile state and fills in
 * outResult.  Never mutates dungeon or party.  Returns 1 if the target
 * square is a door (result populated), 0 otherwise.
 *
 * The caller is expected to:
 *   1. apply result.newDoorState to the square (via the existing
 *      compat square accessor) if result.kind is OPEN or CLOSE;
 *   2. skip mutation for DESTROYED (just emit the "destroyed" message);
 *   3. emit the door-animation / sound notifications it already does.
 *
 * Source mapping: door actuator branch referenced by
 * F0275_SENSOR_IsTriggeredByClickOnWall and the door-type helpers in
 * DUNGEON.C.
 */
int F0715_DOOR_ResolveToggleAction_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    struct DoorToggleResult_Compat* outResult);

/*
 * Route a click on the front cell (relative to the party) to its
 * source-faithful action category.  Pure: reads square byte + thing-list
 * bit and fills outResult.  Does not execute a sensor or toggle a door.
 *
 * Callers that want the door toggled apply the DoorToggleResult
 * returned via F0715; callers that want the sensor run route through
 * the Pass 32 sensor runtime owner.
 *
 * Source mapping: F0275_SENSOR_IsTriggeredByClickOnWall in MOVESENS.C.
 */
int F0716_DOOR_RouteFrontCellClick_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct PartyState_Compat* party,
    struct ClickOnWallResult_Compat* outResult);

/*
 * Pass 38 — door animation ownership.
 *
 * F0714_DOOR_ResolveAnimationEffect_Compat:
 *   Given the current door square state and a requested effect
 *   (SET / CLEAR / TOGGLE), resolve the effect into a concrete
 *   SET or CLEAR that the animation scheduler will drive.
 *
 *   Mirror of the TOGGLE-resolution branch in
 *   F0244_TIMELINE_ProcessEvent10_Square_Door in TIMELINE.C.
 *
 *   Returns:
 *     1 if the square is a non-destroyed door and outEffect has been
 *       set to DOOR_EFFECT_SET or DOOR_EFFECT_CLEAR;
 *     0 otherwise (not a door, destroyed, already at target, or
 *       null out-pointer).
 */
int F0714_DOOR_ResolveAnimationEffect_Compat(
    const struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int requestedEffect,
    int* outEffect,
    int* outCurrentState);

/*
 * F0713_DOOR_BuildAnimationEvent_Compat:
 *   Build a TIMELINE_EVENT_DOOR_ANIMATE event for a given door
 *   square, firing on startTick so that the first step runs on the
 *   same tick the toggle was requested (chained follow-up steps are
 *   rescheduled by the dispatcher at gameTick+1).  Event aux encoding:
 *     aux0 = new door state at the time the event fires (-1 initially
 *            for a "first step" event; the dispatcher reads the
 *            current state from the square and steps it)
 *     aux1 = effect (DOOR_EFFECT_SET or DOOR_EFFECT_CLEAR)
 *
 *   Returns 1 on success, 0 on invalid inputs.
 */
int F0713_DOOR_BuildAnimationEvent_Compat(
    int mapIndex,
    int mapX,
    int mapY,
    int effect,
    uint32_t startTick,
    struct TimelineEvent_Compat* outEvent);

/*
 * F0712_DOOR_StepAnimation_Compat:
 *   Execute one animation step for a door at (mapIndex, mapX, mapY)
 *   with the given effect.  Pure w.r.t. the dungeon pointer: it
 *   reads and writes the low 3 bits of the square byte directly
 *   (this is the single mutation site in the Pass-38 animation
 *   owner; callers that want a pure form can pre-read and post-write
 *   themselves — the helper returns the intended newDoorState on
 *   outStep.newDoorState regardless).
 *
 *   Step semantics mirror F0241_TIMELINE_ProcessEvent1_DoorAnimation
 *   in TIMELINE.C:
 *     - DESTROYED (state 5) — no change, kind=NO_CHANGE.
 *     - effect=SET  (opening): if state==OPEN (0)   -> REACHED_TARGET,
 *                              else newState = state - 1.
 *     - effect=CLEAR (closing): if state==CLOSED(4) -> REACHED_TARGET,
 *                              else newState = state + 1.
 *     - When newState reaches 0 (SET) or 4 (CLEAR) the kind becomes
 *       REACHED_TARGET so the caller can stop rescheduling.
 *
 *   mutateSquare == 1 applies the newState to the dungeon byte.
 *   mutateSquare == 0 leaves the dungeon untouched (used for
 *   deterministic testing of the stepper).
 */
int F0712_DOOR_StepAnimation_Compat(
    struct DungeonDatState_Compat* dungeon,
    int mapIndex,
    int mapX,
    int mapY,
    int effect,
    int mutateSquare,
    struct DoorAnimationStep_Compat* outStep);

#endif
