#ifndef FIRESTAFF_DM2_V1_RUNTIME_H
#define FIRESTAFF_DM2_V1_RUNTIME_H
/*
 * dm2_v1_runtime.h — DM2 V1 Runtime Mechanics Parity API
 *
 * Phase 4: Movement, interactions, shops/NPCs, doors, pressure plates, triggers.
 *
 * Source-locks:
 *   SKULL.ASM T520  — party movement tick, collision, step behavior
 *   SKULL.ASM T560  — dungeon tick, door state machine, actuator processing
 *   SKULL.ASM T048  — input dispatch
 *   SKULL.ASM T800  — outdoor/shop/NPC entry points
 *   docs/dm2_triggers.md      — actuator taxonomy (40+ types)
 *   docs/dm2_actuators.md      — door step-behavior, actuator data
 *   docs/dm2_special_squares.md — teleporter, ladder, door behavior
 *   docs/dm2_sensors.md        — floor/wall sensor types
 *
 * DM2 key differences from DM1:
 *   - Actuator system: generic sensor/actuator separation (DM1: hardwired)
 *   - 40+ actuator types vs ~5 DM1 trigger types
 *   - Cross-map triggers via ACTUATOR_TYPE_CROSS_MAP (0x16)
 *   - Shop interface via ACTUATOR_TYPE_SHOP_PANEL (0x3F)
 *   - Door step-behavior (C00..C06_ACTEFFECT_STEP_*) independent of actuator
 *   - Per-square type in lower 5 bits (same as DM1)
 *   - Companion AI (4 allies, modes: follow/guard/aggressive)
 */

#include <stdint.h>
#include "dm2_v1_boot.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Core movement ──────────────────────────────────────────────── */

void dm2_v1_runtime_init(DM2_V1_BootProfile *boot_profile);
void dm2_v1_runtime_tick(void);
int  dm2_v1_runtime_can_move(void);
int  dm2_v1_runtime_move(int direction);        /* 0=N 1=E 2=S 3=W, returns 0=ok -1=blocked */
void dm2_v1_runtime_set_outdoor(int is_outdoor);/* 1=outdoor 0=dungeon */
void dm2_v1_runtime_set_position(int level, int x, int y, int dir);

/* ── Viewport rendering ─────────────────────────────────────────────── */

/* dm2_v1_runtime_render_frame — render DM2 V1 viewport frame.
 * This is the base V1 discrete renderer (no smooth interpolation).
 * Used by dm2_v2_runtime_render_frame as the base render target.
 *
 * Source: SKULL.ASM T560 — viewport frame rendering
 *         SKULL.ASM T600 — outdoor rendering
 */
int dm2_v1_runtime_render_frame(int party_dir, int party_x, int party_y,
                                  uint8_t *framebuffer, int fb_stride,
                                  int view_w, int view_h);

/* ── Party position accessors ─────────────────────────────────────── */

/* dm2_v1_runtime_get_party_x / _y / _dir — read V1-snapped party state.
 * These return the instant V1 game state (no interpolation).
 * For interpolated V2 state, use dm2_v2_runtime_smooth_query().
 *
 * Source: SKULL.ASM T520 — party position fields
 */
int dm2_v1_runtime_get_party_x(void);
int dm2_v1_runtime_get_party_y(void);
int dm2_v1_runtime_get_party_dir(void);

/* dm2_v1_runtime_has_dungeon_data — returns 1 if dungeon state is available.
 * Used by dm2_v2_runtime_render_frame to detect headless mode.
 * Source: Phase 5 runtime binding */
int dm2_v1_runtime_has_dungeon_data(void);

/* ── V2 Smooth Movement Callbacks ───────────────────────────────── */

/* DM2_V2_MoveCallback — called when party successfully moves.
 * Called from dm2_v1_runtime_move() on a successful move.
 * Parameters: from_x, from_y, to_x, to_y (V1 grid positions).
 * Context: called during V1 tick, before movement cooldown is set.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 */
typedef void (*DM2_V2_MoveCallback)(int from_x, int from_y, int to_x, int to_y);

/* DM2_V2_TurnCallback — called when party turns.
 * Parameters: from_dir, to_dir (0=N 1=E 2=S 3=W).
 *
 * Source: SKULL.ASM T520 — party/movement tick
 */
typedef void (*DM2_V2_TurnCallback)(int from_dir, int to_dir);

/* DM2_V2_StairsCallback — called when party uses stairs.
 * Parameters: from_x, from_y, to_x, to_y (grid positions)
 *             vert_offset (camera vertical displacement).
 *
 * Source: SKULL.ASM T520 — stairs movement
 */
typedef void (*DM2_V2_StairsCallback)(int from_x, int from_y,
                                      int to_x, int to_y,
                                      float vert_offset);

/* dm2_v1_runtime_set_move_callback — register V2 smooth move callback.
 * Only one callback can be registered at a time (replaces previous).
 * Pass NULL to deregister.
 *
 * Source: Phase 5 runtime binding
 */
void dm2_v1_runtime_set_move_callback(DM2_V2_MoveCallback cb);

/* dm2_v1_runtime_set_turn_callback — register V2 smooth turn callback. */
void dm2_v1_runtime_set_turn_callback(DM2_V2_TurnCallback cb);

/* dm2_v1_runtime_set_stairs_callback — register V2 stairs callback. */
void dm2_v1_runtime_set_stairs_callback(DM2_V2_StairsCallback cb);

/* ── Doors ─────────────────────────────────────────────────────────── */

/*
 * Door step-behavior (C00..C06_ACTEFFECT_STEP_*).
 * These are per-door record fields, not actuator types.
 * Source: docs/dm2_actuators.md "Step-behavior modes"
 *         docs/dm2_special_squares.md "DM2 has a second clan door type (0x0A)" */
typedef enum {
    DM2_DOOR_STEP_NONE          = 0,  /* nothing on step */
    DM2_DOOR_STEP_OPEN          = 1,  /* open on step */
    DM2_DOOR_STEP_CLOSE         = 2,  /* close on step away */
    DM2_DOOR_STEP_TOGGLE        = 3,  /* toggle on step */
    DM2_DOOR_STEP_OPEN_AUTOCLOSE = 4, /* open on step, auto-close */
    DM2_DOOR_STEP_FORCE_OPEN    = 5,  /* force open (from actuator) */
    DM2_DOOR_STEP_FORCE_CLOSE   = 6,  /* force close */
} DM2_DoorStepBehavior;

/* Door state machine (DoorBit09, DoorBit10).
 * Source: docs/dm2_actuators.md "Door Actuator Control"
 *         SKULL.ASM T560 door state transitions */
typedef enum {
    DM2_DOOR_CLOSED  = 0,
    DM2_DOOR_OPENING = 1,   /* DoorBit09=1, animates open */
    DM2_DOOR_OPEN    = 2,
    DM2_DOOR_CLOSING = 3,   /* DoorBit09=0, animates closed */
} DM2_DoorState;

/* Attempt to open or close the door at (level, x, y, facing_dir).
 * Returns 0 on success, -1 if no door at that location. */
int dm2_v1_runtime_door_action(int level, int x, int y, int facing_dir, int action);
/*  action: 0=open, 1=close, 2=toggle */

/* Get door state at a position. Returns -1 if no door. */
int dm2_v1_runtime_get_door_state(int level, int x, int y);

/* ── Teleporters ──────────────────────────────────────────────────── */

/* Teleport party to (target_level, tx, ty).
 * Source: docs/dm2_special_squares.md "SDFSM_CMD_X_TELEPORTER (4), SDFSM_CMD_X_ANCHOR (5)" */
int dm2_v1_runtime_teleport(int target_level, int tx, int ty);

/* Teleport scope — controls what can use the teleporter.
 * Source: DME.h:384 "Teleporter scope enum" */
typedef enum {
    DM2_TELEPORT_SCOPE_PARTY    = 0,  /* party only */
    DM2_TELEPORT_SCOPE_CREATURE = 1,  /* creatures only */
    DM2_TELEPORT_SCOPE_ALL      = 2,  /* everything */
    DM2_TELEPORT_SCOPE_OBJECT   = 3,  /* objects/items */
} DM2_TeleportScope;

/* ── Pressure plates / floor triggers ─────────────────────────────── */

/* Floor trigger types (sensor side).
 * Source: docs/dm2_sensors.md "Floor Sensors" table
 *         ACTUATOR_FLOOR_TYPE__* in defines.h */
typedef enum {
    DM2_FLOOR_EVERYTHING       = 0x01,  /* any entity steps */
    DM2_FLOOR_PARTY             = 0x03,  /* party member steps */
    DM2_FLOOR_ITEM              = 0x04,  /* item placed/dropped */
    DM2_FLOOR_CREATURE          = 0x07,  /* creature steps */
    DM2_FLOOR_ITEM_POSSESSION   = 0x08,  /* party member carrying item steps */
    DM2_FLOOR_TELEPORTER        = 0x2E,  /* party teleporter */
    DM2_FLOOR_SHOP              = 0x30,  /* party enters shop */
} DM2_FloorTriggerType;

/* Activate the floor trigger at (level, x, y). Returns 0 on success. */
int dm2_v1_runtime_floor_trigger(int level, int x, int y);

/* ── Actuators / triggers ──────────────────────────────────────────── */

/* Actuator type taxonomy (effect side).
 * Source: docs/dm2_triggers.md "Actuator Type Taxonomy"
 *         ACTUATOR_TYPE_* in defines.h */
typedef enum {
    DM2_ACTUATOR_DM1_WALL_SWITCH       = 0x01,
    DM2_ACTUATOR_ITEM_WATCHER          = 0x03,
    DM2_ACTUATOR_MISSILE_SHOOTER       = 0x08,
    DM2_ACTUATOR_WEAPON_SHOOTER        = 0x09,
    DM2_ACTUATOR_ITEM_SHOOTER           = 0x0E,
    DM2_ACTUATOR_CROSS_MAP             = 0x16,
    DM2_ACTUATOR_2_STATE_WALL_SWITCH   = 0x17,
    DM2_ACTUATOR_WALL_SWITCH           = 0x18,
    DM2_ACTUATOR_KEY_HOLE              = 0x1A,
    DM2_ACTUATOR_COUNTER               = 0x1D,
    DM2_ACTUATOR_TICK_GENERATOR        = 0x1E,
    DM2_ACTUATOR_RELAY_1               = 0x20,
    DM2_ACTUATOR_ARRIVAL_DEPARTURE     = 0x21,
    DM2_ACTUATOR_FLYING_ITEM_CATCHER   = 0x22,
    DM2_ACTUATOR_FLYING_ITEM_TELEPORTER = 0x23,
    DM2_ACTUATOR_SWITCH_SIGN_FOR_CREATURE = 0x26,
    DM2_ACTUATOR_CREATURE_GENERATOR    = 0x2E,
    DM2_ACTUATOR_WORK_TIMER            = 0x31,
    DM2_ACTUATOR_ITEM_GENERATOR        = 0x3C,
    DM2_ACTUATOR_RELAY_2               = 0x3D,
    DM2_ACTUATOR_SHOP_PANEL            = 0x3F,
    DM2_ACTUATOR_ITEM_RECYCLER         = 0x40,
    DM2_ACTUATOR_PUSH_BUTTON_WALL_SWITCH = 0x46,
    DM2_ACTUATOR_ITEM_CAPTURE          = 0x47,
    DM2_ACTUATOR_RESURECTOR            = 0x7E,
} DM2_ActuatorType;

/* Activate an actuator by type+position. Returns 0 on success. */
int dm2_v1_runtime_invoke_actuator(int level, int x, int y,
                                     DM2_ActuatorType type, uint16_t flag);

/* ── Shops / NPCs ──────────────────────────────────────────────────── */

/* Enter shop mode. DM2 shops are accessed via ACTUATOR_TYPE_SHOP_PANEL (0x3F).
 * Source: SKULL.ASM T800 outdoor/shop entry
 *         docs/dm2_interaction.md "SHOP_PANEL opens shop interface" */
int dm2_v1_runtime_enter_shop(int level, int x, int y);

/* Interact with merchant NPC (AI index 33).
 * CCM_MERCHANT_BEHAVIOR (0x0A) handles the shop interaction flow.
 * Source: dm2_v1_creature.h DM2_AI_MERCHANT=33
 *         DM2_CCM_MERCHANT_BEHAVIOR=0x0a */
int dm2_v1_runtime_npc_interact(int level, int x, int y);

/* ── Special squares ──────────────────────────────────────────────── */

/* Check if a square is passable (wall, pit, lava block movement).
 * Uses dm2_world_is_walkable internally. */
int dm2_v1_runtime_is_passable(int level, int x, int y);

/* Get the square type at a position (0..15, or -1 on error).
 * Lower 5 bits of tile word — same encoding as DM1. */
int dm2_v1_runtime_get_square_type(int level, int x, int y);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *dm2_v1_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RUNTIME_H */