#ifndef NEXUS_V1_MECHANICS_H
#define NEXUS_V1_MECHANICS_H

/* Nexus V1 mechanics — assembled game loop combining all systems.
 * Order per tick: input → movement → square event → creature AI →
 * creature attack → resource drain → script VM → sound.
 * Source: DM1 CLIKMENU.C F0366, MOVESENS.C F0267, CHAMPION.C F0325,
 * F0209_GROUP_ProcessEvents29to41 (ReDMCSB), docs/nexus_*.md.
 *
 * IMPORTANT: Include nexus_v1_engine.h BEFORE this header in .c files
 * that use both Nexus_V1_Engine. nexus_v1_engine.h defines the struct
 * first and sets NEXUS_ENGINE_FWD_FROM_HEADERS before any includes,
 * allowing this header's mechanics_fwd.h to provide Nexus_V1_Engine
 * as a complete type before any function signature uses it by pointer. */

#include <stdint.h>

/* Forward declarations — Nexus_V1_Engine is fully defined in
 * nexus_v1_engine.h; MechanicsState is defined in this header.
 * Both are included via the mechanics_fwd header which avoids
 * circular conflicts via the NEXUS_ENGINE_FWD_FROM_HEADERS guard. */
#include "nexus_v1_mechanics_fwd.h"

/* Command types (mirrors DM1 input queue) — defined in nexus_v1_game.h */

/* Movement result codes — mirrors DM1 MOVESENS — defined in nexus_v1_movement.h */
#ifndef NEXUS_MOVE_OK
#define NEXUS_MOVE_OK 0
#endif

/* Integrated game state for mechanics — full party + input state */
struct Nexus_MechanicsState {
    /* Movement */
    int party_x, party_y, party_dir;
    int map_index;               /* current level 0-15 */

    /* Party status */
    int party_alive;            /* 1 = at least one champion alive */
    int gold_pieces;           /* accumulated gold */

    /* Game end state */
    int game_over;             /* 1 = game has ended (exit reached / all dead) */
    int game_over_reason;      /* 0=unset, 1=exit, 2=all_dead, 3=quit */

    /* Pending events */
    int pending_level_change;   /* target level or -1 */
    int pending_teleport;       /* 1 = teleport pending */
    int teleport_target_x, teleport_target_y, teleport_target_level;

    /* Movement cooldown */
    int move_cooldown_ticks;    /* ticks remaining before next move */

    /* Game tick counter */
    unsigned long total_ticks;

    /* Food/water drain counters */
    int food_drain_timer;       /* ticks until food drain */
    int water_drain_timer;      /* ticks until water drain */

    /* Input queue (movement commands) — FIFO, 8 deep */
    int input_queue[8];
    int input_head;
    int input_tail;
    int input_count;
};

/* ═══════════════════════════════════════════════════════════════════
 * Initialization and input
 * ═══════════════════════════════════════════════════════════════════ */

/* Init mechanics state at starting position (DM1 default: x=11,y=29,dir=0=N) */
void nexus_mechanics_init(Nexus_MechanicsState *st,
                            int start_x, int start_y, int start_dir);

/* Push a movement command into the input queue */
int nexus_mechanics_push_command(Nexus_MechanicsState *st, int command);

/* Pop next command from queue. Returns 1 if got one, 0 if empty. */
int nexus_mechanics_pop_command(Nexus_MechanicsState *st, int *out_cmd);

/* ═══════════════════════════════════════════════════════════════════
 * Main game tick — call each 55ms (18.2 Hz)
 * Returns 1 if viewport should be redrawn.
 * Engine pointer must be a fully-defined struct (include nexus_v1_engine.h
 * before passing).
 * Source: DM1 CLIKMENU.C:269-323 step result + cooldown,
 *         MOVESENS.C:752-783 walk-off/walk-on,
 *         CHAMPION.C:2025-2048 stamina decrement.
 * ═══════════════════════════════════════════════════════════════════ */
int nexus_mechanics_tick(Nexus_MechanicsState *st, Nexus_V1_Engine *engine);

/* ═══════════════════════════════════════════════════════════════════
 * Position and status queries
 * ═══════════════════════════════════════════════════════════════════ */

/* Check if at least one champion is alive. Requires engine pointer. */
int nexus_mechanics_party_alive(const Nexus_MechanicsState *st, Nexus_V1_Engine *engine);

/* Get party position and direction */
void nexus_mechanics_get_party_pos(const Nexus_MechanicsState *st,
                                     int *out_x, int *out_y, int *out_dir);

/* ═══════════════════════════════════════════════════════════════════
 * Event triggers (called by script VM or square event handlers)
 * ═══════════════════════════════════════════════════════════════════ */

/* Trigger a teleport warp */
void nexus_mechanics_teleport(Nexus_MechanicsState *st,
                                int target_x, int target_y, int target_level);

/* Trigger level transition via stairs/chute */
void nexus_mechanics_change_level(Nexus_MechanicsState *st, int target_level,
                                    int target_x, int target_y);

#endif /* NEXUS_V1_MECHANICS_H */