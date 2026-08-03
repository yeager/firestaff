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

    /* Selected inventory slot for NEXUS_CMD_USE_ITEM.
     * UI sets this before pushing the use-item command.
     * Source: DM1 CLIKMENU.C item click -> command dispatch. */
    int use_item_slot;

    /* Spell casting state — rune selection buffer.
     * UI pushes runes one at a time; NEXUS_CMD_CAST_SPELL resolves
     * and casts.  Source: DM1 COMMAND.C rune entry + F0412 cast. */
    int spell_power;       /* selected power rune (0-5), or -1 */
    int spell_element;     /* selected element rune (0-3), or -1 */
    int spell_form;        /* selected form rune (0-3), or -1 */
    int spell_align;       /* alignment (0-1), or -1 */

    int set_leader_slot;   /* party slot (0-3) for NEXUS_CMD_SET_LEADER */
    int throw_slot;        /* inventory slot for NEXUS_CMD_THROW */
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

/* Set the inventory slot that NEXUS_CMD_USE_ITEM will consume.
 * slot is an index into the party leader's inventory[30]. */
void nexus_mechanics_set_use_item_slot(Nexus_MechanicsState *st, int slot);

/* Set spell runes before pushing NEXUS_CMD_CAST_SPELL.
 * Source: DM1 COMMAND.C rune entry sequence. */
void nexus_mechanics_set_spell_runes(Nexus_MechanicsState *st,
                                     int power, int element, int form, int align);

/* Clear the spell rune buffer. */
void nexus_mechanics_clear_spell(Nexus_MechanicsState *st);

/* Set the party slot (0-3) for NEXUS_CMD_SET_LEADER. */
void nexus_mechanics_set_leader_slot(Nexus_MechanicsState *st, int slot);

/* Load real Track 1 mechanics data for the current engine level.
 * Resets and repopulates door/teleporter/stair/pit/altar/floor-item registries
 * from authenticated DGN Structure1F records.  Synthetic fallbacks are blocked
 * when real records are present.
 * Source: DMWeb DGN Structure1F layout, DM1 MOVESENS.C/CHAMPION.C item use,
 *         ReDMCSB DUNGEON.C / COMMAND.C. */
int nexus_v1_mechanics_load_level(Nexus_V1_Engine *engine, int level_index);

/* ═══════════════════════════════════════════════════════════════════
 * UI event dispatch — routes panel actions to mechanics commands
 * Source: DM1 CLIKMENU.C F0366 click→command dispatch
 * ═══════════════════════════════════════════════════════════════════ */

#define NEXUS_UI_EVENT_MAP       0x01
#define NEXUS_UI_EVENT_SPELL     0x02
#define NEXUS_UI_EVENT_INVENTORY 0x03
#define NEXUS_UI_EVENT_REST      0x04
#define NEXUS_UI_EVENT_SAVE      0x05
#define NEXUS_UI_EVENT_SET_LEADER 0x06
#define NEXUS_UI_EVENT_THROW     0x07

int nexus_mechanics_dispatch_event(Nexus_MechanicsState *st,
                                   Nexus_V1_Engine *engine,
                                   int event_type, int param);

#endif /* NEXUS_V1_MECHANICS_H */
