#ifndef DM1_V1_ACTUATOR_EXECUTION_PC34_COMPAT_H
#define DM1_V1_ACTUATOR_EXECUTION_PC34_COMPAT_H

/*
 * DM1 V1 Actuator Execution — pc34 compat layer.
 *
 * Source reference: ReDMCSB MOVESENS.C F0268_SENSOR_AddEvent lines 1136-1232
 * and TIMELINE.C event handlers for C024_EVENT_DOOR (lines ~600-660).
 *
 * This module bridges the gap between sensor trigger evaluation (which
 * produces a SensorActuatorDispatch_Compat) and the actual dungeon state
 * mutation. Without this layer, buttons and pressure plates evaluate
 * correctly but produce no effect — doors never open, pits never toggle.
 *
 * Actuator actions:
 *   - DOOR: set door state in square byte (bits 2:0), advancing toward
 *     open (state 0) or closed (state 4) in one step per animation tick.
 *     In ReDMCSB, doors animate via C024 timed events that step through
 *     states 0-4 over multiple ticks. This module provides both the
 *     immediate state-set (for testing) and the animation step function.
 *   - PIT: toggle bit 3 (MASK0x0008_PIT_OPEN) of the square byte.
 *   - FAKEWALL: toggle between DUNGEON_ELEMENT_FAKEWALL and
 *     DUNGEON_ELEMENT_CORRIDOR by rewriting bits 7:5 of the square byte.
 *   - TELEPORTER: no square-byte mutation needed; teleporter activation
 *     is handled by movement pipeline on entry.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SensorActuatorDispatch_Compat;
struct DungeonMapTiles_Compat;

/* Result of executing an actuator dispatch. */
struct Dm1V1ActuatorResult {
    int applied;           /* 1 if state was mutated */
    int previousState;     /* Door state before (0-5), or pit open (0/1) */
    int newState;          /* Door state after, or pit open (0/1) */
    int animationNeeded;   /* 1 if a timed event should be queued (door) */
    int targetMapX;
    int targetMapY;
};

/* Execute a sensor actuator dispatch against the dungeon map tiles.
 * Reads the target square, applies the action, and writes back.
 *
 * For doors:
 *   OPEN  -> sets state to DM1_DOOR_STATE_OPEN (0)
 *   CLOSE -> sets state to DM1_DOOR_STATE_CLOSED (4)
 *   TOGGLE -> flips between OPEN and CLOSED
 *
 * For pits:
 *   OPEN  -> sets PIT_OPEN bit
 *   CLOSE -> clears PIT_OPEN bit
 *   TOGGLE -> XORs PIT_OPEN bit
 *
 * For fakewalls:
 *   OPEN  -> changes element to CORRIDOR
 *   CLOSE -> changes element to FAKEWALL
 *   TOGGLE -> flips between CORRIDOR and FAKEWALL
 *
 * Returns 1 on success, 0 on invalid args or out-of-bounds target. */
int dm1_v1_actuator_execute_dispatch_pc34(
    const struct SensorActuatorDispatch_Compat* dispatch,
    struct DungeonMapTiles_Compat* tiles,
    int mapWidth,
    int mapHeight,
    struct Dm1V1ActuatorResult* outResult);

/* Step a door animation by one tick.
 * Source: ReDMCSB TIMELINE.C C024_EVENT_DOOR handler.
 *
 * If opening (targetState < currentState): decrements door state by 1.
 * If closing (targetState > currentState): increments door state by 1.
 * Returns 1 if the door has not yet reached targetState (more ticks needed).
 * Returns 0 if the door reached targetState or on error. */
int dm1_v1_actuator_door_animation_step_pc34(
    struct DungeonMapTiles_Compat* tiles,
    int mapWidth,
    int mapHeight,
    int mapX,
    int mapY,
    int targetState);

/* Read the current door state from a square byte. Bits 2:0.
 * Same as M036_DOOR_STATE in ReDMCSB DEFS.H. */
static inline int dm1_v1_actuator_read_door_state(unsigned char squareByte) {
    return squareByte & 0x07;
}

/* Write a new door state into a square byte. Preserves bits 7:3. */
static inline unsigned char dm1_v1_actuator_write_door_state(
    unsigned char squareByte, int newState) {
    return (unsigned char)((squareByte & 0xF8) | (newState & 0x07));
}

/* Read pit-open flag from a square byte. Bit 3. */
static inline int dm1_v1_actuator_read_pit_open(unsigned char squareByte) {
    return (squareByte >> 3) & 1;
}

/* Write pit-open flag into a square byte. Preserves other bits. */
static inline unsigned char dm1_v1_actuator_write_pit_open(
    unsigned char squareByte, int open) {
    if (open)
        return squareByte | 0x08;
    else
        return squareByte & (unsigned char)~0x08;
}

/* Read element type from a square byte. Bits 7:5. */
static inline int dm1_v1_actuator_read_element(unsigned char squareByte) {
    return (squareByte >> 5) & 0x07;
}

/* Write element type into a square byte. Preserves bits 4:0. */
static inline unsigned char dm1_v1_actuator_write_element(
    unsigned char squareByte, int element) {
    return (unsigned char)((squareByte & 0x1F) | ((element & 0x07) << 5));
}

const char* dm1_v1_actuator_execution_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
