/*
 * nexus_v1_doors.c — Nexus door state machine and animation runtime.
 *
 * Source-lock: ReDMCSB DUNGEON.C F0107 door panel states,
 *              MOVESENS.C F0267 door interaction.
 */

#include "nexus_v1_doors.h"
#include <string.h>

void nexus_v1_door_manager_init(Nexus_DoorManager *mgr)
{
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_door_register(Nexus_DoorManager *mgr,
    int map_x, int map_y, int map_index,
    int type, int key_id, int bash_resistance, int flags)
{
    if (!mgr || mgr->count >= NEXUS_MAX_DOORS) return -1;

    int idx = mgr->count++;
    Nexus_Door *d = &mgr->doors[idx];
    memset(d, 0, sizeof(*d));
    d->state = NEXUS_DOOR_CLOSED;
    d->type = (uint8_t)type;
    d->key_id = (uint8_t)key_id;
    d->map_x = (int16_t)map_x;
    d->map_y = (int16_t)map_y;
    d->map_index = (int16_t)map_index;
    d->bash_resistance = (uint8_t)bash_resistance;
    d->flags = (uint8_t)flags;
    if (key_id > 0)
        d->state = NEXUS_DOOR_LOCKED;
    return idx;
}

int nexus_v1_door_find(const Nexus_DoorManager *mgr,
    int map_x, int map_y, int map_index)
{
    if (!mgr) return -1;
    for (int i = 0; i < mgr->count; i++) {
        const Nexus_Door *d = &mgr->doors[i];
        if (d->map_x == map_x && d->map_y == map_y &&
            d->map_index == map_index)
            return i;
    }
    return -1;
}

int nexus_v1_door_try_open(Nexus_DoorManager *mgr, int door_idx,
    int held_key_id, int bash_strength)
{
    if (!mgr || door_idx < 0 || door_idx >= mgr->count)
        return NEXUS_DOOR_RESULT_NO_KEY;

    Nexus_Door *d = &mgr->doors[door_idx];

    if (d->state == NEXUS_DOOR_OPEN || d->state == NEXUS_DOOR_OPENING)
        return NEXUS_DOOR_RESULT_ALREADY_OPEN;

    if (d->state == NEXUS_DOOR_LOCKED) {
        if (held_key_id > 0 && held_key_id == d->key_id) {
            d->state = NEXUS_DOOR_OPENING;
            d->anim_frame = 0;
            return NEXUS_DOOR_RESULT_OK;
        }
        if (bash_strength > 0 && bash_strength >= d->bash_resistance) {
            d->state = NEXUS_DOOR_OPENING;
            d->anim_frame = 0;
            return NEXUS_DOOR_RESULT_BASHED;
        }
        if (bash_strength > 0)
            return NEXUS_DOOR_RESULT_BASH_FAIL;
        return NEXUS_DOOR_RESULT_LOCKED;
    }

    d->state = NEXUS_DOOR_OPENING;
    d->anim_frame = 0;
    return NEXUS_DOOR_RESULT_OK;
}

int nexus_v1_door_close(Nexus_DoorManager *mgr, int door_idx)
{
    if (!mgr || door_idx < 0 || door_idx >= mgr->count) return -1;
    Nexus_Door *d = &mgr->doors[door_idx];
    if (d->state != NEXUS_DOOR_OPEN) return -1;
    d->state = NEXUS_DOOR_CLOSING;
    d->anim_frame = NEXUS_DOOR_ANIM_TICKS;
    return 0;
}

int nexus_v1_door_toggle(Nexus_DoorManager *mgr, int door_idx)
{
    if (!mgr || door_idx < 0 || door_idx >= mgr->count) return -1;
    Nexus_Door *d = &mgr->doors[door_idx];
    if (d->state == NEXUS_DOOR_OPEN) {
        d->state = NEXUS_DOOR_CLOSING;
        d->anim_frame = NEXUS_DOOR_ANIM_TICKS;
        return 0;
    }
    if (d->state == NEXUS_DOOR_CLOSED) {
        d->state = NEXUS_DOOR_OPENING;
        d->anim_frame = 0;
        return 0;
    }
    return -1;
}

void nexus_v1_door_tick(Nexus_DoorManager *mgr)
{
    if (!mgr) return;
    for (int i = 0; i < mgr->count; i++) {
        Nexus_Door *d = &mgr->doors[i];
        if (d->state == NEXUS_DOOR_OPENING) {
            d->anim_frame++;
            if (d->anim_frame >= NEXUS_DOOR_ANIM_TICKS) {
                d->state = NEXUS_DOOR_OPEN;
                d->anim_frame = NEXUS_DOOR_ANIM_TICKS;
            }
        } else if (d->state == NEXUS_DOOR_CLOSING) {
            if (d->anim_frame > 0)
                d->anim_frame--;
            if (d->anim_frame == 0) {
                d->state = NEXUS_DOOR_CLOSED;
            }
        }
    }
}

int nexus_v1_door_is_passable(const Nexus_DoorManager *mgr, int door_idx)
{
    if (!mgr || door_idx < 0 || door_idx >= mgr->count) return 0;
    return mgr->doors[door_idx].state == NEXUS_DOOR_OPEN ||
           mgr->doors[door_idx].state == NEXUS_DOOR_OPENING;
}

float nexus_v1_door_anim_progress(const Nexus_DoorManager *mgr, int door_idx)
{
    if (!mgr || door_idx < 0 || door_idx >= mgr->count) return 0.0f;
    return (float)mgr->doors[door_idx].anim_frame / (float)NEXUS_DOOR_ANIM_TICKS;
}
