/*
 * nexus_v1_doors.h — Nexus door state machine and animation runtime.
 *
 * Source-lock: ReDMCSB DUNGEON.C F0107 door panel states,
 *              MOVESENS.C F0267 door interaction,
 *              SDDRVS.TSK Saturn door script driver.
 */

#ifndef NEXUS_V1_DOORS_H
#define NEXUS_V1_DOORS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Door base states. */
enum {
    NEXUS_DOOR_CLOSED  = 0,
    NEXUS_DOOR_OPEN    = 1,
    NEXUS_DOOR_LOCKED  = 2,
    NEXUS_DOOR_OPENING = 3,
    NEXUS_DOOR_CLOSING = 4,
    NEXUS_DOOR_BASH_ATTEMPT = 5
};

/* Door type/material. */
enum {
    NEXUS_DOOR_TYPE_WOODEN  = 0,
    NEXUS_DOOR_TYPE_IRON    = 1,
    NEXUS_DOOR_TYPE_PORTCUL = 2,
    NEXUS_DOOR_TYPE_COUNT   = 3
};

/* Door interaction result. */
enum {
    NEXUS_DOOR_RESULT_OK       = 0,
    NEXUS_DOOR_RESULT_LOCKED   = 1,
    NEXUS_DOOR_RESULT_NO_KEY   = 2,
    NEXUS_DOOR_RESULT_BASHED   = 3,
    NEXUS_DOOR_RESULT_BASH_FAIL = 4,
    NEXUS_DOOR_RESULT_ALREADY_OPEN = 5
};

#define NEXUS_DOOR_ANIM_TICKS 8
#define NEXUS_MAX_DOORS 256

typedef struct {
    uint8_t state;
    uint8_t type;
    uint8_t key_id;
    uint8_t anim_frame;
    int16_t map_x, map_y;
    int16_t map_index;
    uint8_t bash_resistance;
    uint8_t flags;
} Nexus_Door;

#define NEXUS_DOOR_FLAG_BUTTON     0x01
#define NEXUS_DOOR_FLAG_ONESHOT    0x02
#define NEXUS_DOOR_FLAG_INVISIBLE  0x04

typedef struct {
    Nexus_Door doors[NEXUS_MAX_DOORS];
    int count;
} Nexus_DoorManager;

/* Initialize/reset the door manager. */
void nexus_v1_door_manager_init(Nexus_DoorManager *mgr);

/* Register a door at a map position. Returns door index or -1. */
int nexus_v1_door_register(Nexus_DoorManager *mgr,
    int map_x, int map_y, int map_index,
    int type, int key_id, int bash_resistance, int flags);

/* Find a door at a position. Returns door index or -1. */
int nexus_v1_door_find(const Nexus_DoorManager *mgr,
    int map_x, int map_y, int map_index);

/* Attempt to open a door. Checks key, bash, locked state.
 * held_key_id: the key the party is holding, or -1 for none.
 * bash_strength: strength for bash attempt, or 0 for no bash. */
int nexus_v1_door_try_open(Nexus_DoorManager *mgr, int door_idx,
    int held_key_id, int bash_strength);

/* Close an open door. */
int nexus_v1_door_close(Nexus_DoorManager *mgr, int door_idx);

/* Toggle a door (button/sensor trigger). */
int nexus_v1_door_toggle(Nexus_DoorManager *mgr, int door_idx);

/* Tick all door animations. Call once per game tick. */
void nexus_v1_door_tick(Nexus_DoorManager *mgr);

/* Query door passability. Returns 1 if passable (open), 0 if blocked. */
int nexus_v1_door_is_passable(const Nexus_DoorManager *mgr, int door_idx);

/* Get animation progress (0.0 = closed, 1.0 = fully open). */
float nexus_v1_door_anim_progress(const Nexus_DoorManager *mgr, int door_idx);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_DOORS_H */
