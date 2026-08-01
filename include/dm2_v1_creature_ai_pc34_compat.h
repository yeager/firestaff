#ifndef FIRESTAFF_DM2_V1_CREATURE_AI_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_AI_PC34_COMPAT_H

/*
 * dm2_v1_creature_ai_pc34_compat.h — DM2 creature AI subsystem.
 *
 * Ports DM2_THINK_CREATURE, DM2_ROTATE_CREATURE, and the PROCEED_XACT
 * handler table from skproject/SKULLWIN/c_ai.cpp and c_creature.cpp.
 *
 * THINK_CREATURE (c_ai.cpp:5649): the main AI tick. Called from timer
 * proc (c_tim_proc.cpp:4088) for each creature. Resolves creature at
 * tile via GET_CREATURE_AT, prepares local creature vars via
 * PREPARE_LOCAL_CREATURE_VAR, then runs the XACT state machine.
 *
 * ROTATE_CREATURE (c_creature.cpp:58-101): rotates a creature's facing
 * direction. Updates word at record+0xe bits 14-15 (direction field).
 * If AI spec flag bit 0 set, also rotates all possession records.
 *
 * PROCEED_XACT handlers (c_ai.cpp): 30+ handlers numbered 56-88 that
 * implement creature behaviors — move, attack, cast, flee, patrol,
 * guard, follow, etc.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp, c_creature.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XACT action types — the behavior opcodes in the AI state machine.
 * Each maps to a PROCEED_XACT_NN handler in c_ai.cpp */
typedef enum {
    DM2_XACT_STOP              = 56,
    DM2_XACT_STOP_CLEAR        = 57,
    DM2_XACT_MOVE_FORWARD      = 59,
    DM2_XACT_ATTACK            = 62,
    DM2_XACT_CAST_SPELL        = 63,
    DM2_XACT_SHOOT_MISSILE     = 64,
    DM2_XACT_FLEE              = 65,
    DM2_XACT_MOVE_RANDOM       = 66,
    DM2_XACT_MOVE_TO_TARGET    = 67,
    DM2_XACT_GUARD_POSITION    = 68,
    DM2_XACT_SOUND             = 69,
    DM2_XACT_FACE_PARTY        = 70,
    DM2_XACT_PATROL            = 71,
    DM2_XACT_REGENERATE        = 72,
    DM2_XACT_SPAWN             = 73,
    DM2_XACT_OPEN_DOOR         = 74,
    DM2_XACT_USE_ITEM          = 75,
    DM2_XACT_MOVE_BACKWARD     = 76,
    DM2_XACT_FOLLOW            = 77,
    DM2_XACT_SLEEP             = 87,
    DM2_XACT_WAKE              = 88
} DM2_V1_XactType;

/* ROTATE_CREATURE request */
typedef struct {
    int16_t creature_handle;
    int16_t new_direction;
    int rotate_relative;
} DM2_V1_RotateCreatureRequest;

/* ROTATE_CREATURE receipt */
typedef struct {
    int valid;
    int fail_closed;
    int16_t old_direction;
    int16_t new_direction;
    int possessions_rotated;
} DM2_V1_RotateCreatureReceipt;

/* THINK_CREATURE request */
typedef struct {
    int16_t tile_x;
    int16_t tile_y;
    int16_t map_level;
    uint32_t game_tick;
} DM2_V1_ThinkCreatureRequest;

/* THINK_CREATURE receipt */
typedef struct {
    int valid;
    int fail_closed;
    int creature_found;
    int16_t creature_handle;
    DM2_V1_XactType xact_executed;
    int xact_succeeded;
    int creature_moved;
    int creature_attacked;
    int creature_cast;
    int creature_died;
} DM2_V1_ThinkCreatureReceipt;

int dm2_v1_rotate_creature(
    const DM2_V1_RotateCreatureRequest *request,
    DM2_V1_RotateCreatureReceipt *receipt);

int dm2_v1_think_creature(
    const DM2_V1_ThinkCreatureRequest *request,
    DM2_V1_ThinkCreatureReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_AI_PC34_COMPAT_H */
