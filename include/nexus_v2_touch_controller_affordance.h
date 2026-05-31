#ifndef FIRESTAFF_NEXUS_V2_TOUCH_CONTROLLER_AFFORDANCE_H
#define FIRESTAFF_NEXUS_V2_TOUCH_CONTROLLER_AFFORDANCE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Nexus V2 Touch/Controller Affordance — Phase 6
 *
 * V2-only gesture/controller affordances for Dungeon Master Nexus (Saturn).
 * Maps touch swipes and controller inputs to Nexus movement commands
 * (NEXUS_CMD_FORWARD/BACKWARD/TURN_LEFT/TURN_RIGHT/STRAFE_LEFT/STRAFE_RIGHT),
 * using the Nexus V1 movement engine pipeline.
 * V1 touch/click parity is preserved: when v2PresentationEnabled is false,
 * all affordances are rejected so the V1 mouse/touch/click route matrix
 * remains the sole input path.
 *
 * Differences from DM1 V2 Phase 6 (dm1_v2_touch_controller_affordance_pc34):
 *   - Nexus uses NEXUS_CMD_* (1-6) vs DM1 C001-C006 command ids
 *     (same movement semantics, different enum values)
 *   - No pinch-to-zoom (Nexus/Saturn has no zoomable minimap equivalent)
 *   - No shoulder button mappings (Nexus has no champion-cycle or
 *     chaos-magic equivalents — those are CSB-specific actions)
 *   - No right-stick strafing (right stick = turning only on Saturn gamepad)
 *
 * Differences from DM2 V2 Phase 6 (dm2_v2_touch_controller_affordance):
 *   - DM2 uses DM1_V2_MovementCommand enum (shared C001-C006 system)
 *   - Nexus uses its own NEXUS_CMD_* enum (1-6, same movement semantics)
 *   - Nexus right stick turns only; DM2 right stick also turns only
 *     (identical behavior, different enum binding)
 *
 * Source-lock anchors:
 *   DMDF/DGN level format — Nexus movement grid
 *   Saturn NEXUS.BIN input surface data
 *   Saturn SDK joystick mapping (D-pad, analog sticks)
 *   DM1 CLIKMENU.C:224-233 arrow deltas / blocked movement
 *   DM1 CLIKMENU.C:237-255 F0325 stamina cost before movement
 *   DM1 CLIKMENU.C:142  F0365_COMMAND_ProcessTypes1To2_TurnParty (turn)
 *   DM1 CLIKMENU.C:180  F0366_COMMAND_ProcessTypes3To6_MoveParty (step)
 *   DM1 REALTIME.ASM T048  input dispatch
 *   DM1 GAMELOOP.C:164-219 V1 input wait/command queue loop
 *   nexus_v1_movement.c:nexus_movement_tick() V1 tick handler
 *   nexus_v1_movement.h:NEXUS_CMD_* (1-6 movement commands)
 *
 * Architecture:
 *   V2 affordance → NEXUS_CMD_* → nexus_movement_tick() input queue
 *   → either V1 source route (disabled) or V2 presentation route (enabled).
 *   Nexus inherits the DM1 movement engine (same step/turn/stamina logic)
 *   via nexus_v1_movement.c; this file labels V2 affordances and maps
 *   them to the same NEXUS_CMD_* enum values that V1 uses.
 */

/* ── Affordance IDs ─────────────────────────────────────────────────── */

typedef enum {
    NEXUS_V2_AFFORDANCE_NONE = 0,

    /* Touch swipe gestures — map to NEXUS_CMD_* movement commands */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP,           /* NEXUS_CMD_FORWARD      */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,        /* NEXUS_CMD_BACKWARD     */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,        /* NEXUS_CMD_TURN_LEFT    */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,       /* NEXUS_CMD_TURN_RIGHT   */

    /* Touch edge-strafe — V2-only, V1 path ignores edge zones
     * Note: Nexus strafe is supported in V1 already (NEXUS_CMD_STRAFE_LEFT/RIGHT),
     * so edge-strafe on touch is V2 affordance that maps to the same commands */
    NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,  /* NEXUS_CMD_STRAFE_LEFT  */
    NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, /* NEXUS_CMD_STRAFE_RIGHT */

    /* Controller D-pad — identical mapping to swipe gestures */
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP,
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,

    /* Left stick — forward/back with strafing on left/right */
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,

    /* Right stick — turning only (no movement), per Saturn gamepad layout */
    NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,  /* NEXUS_CMD_TURN_LEFT  */
    NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT  /* NEXUS_CMD_TURN_RIGHT */
} Nexus_V2_TouchControllerAffordance;

/* ── Input kind ─────────────────────────────────────────────────────── */

typedef enum {
    NEXUS_V2_AFFORDANCE_INPUT_NONE = 0,
    NEXUS_V2_AFFORDANCE_INPUT_TOUCH = 1,
    NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER = 2
} Nexus_V2_TouchControllerInputKind;

/* ── Routing result ────────────────────────────────────────────────── */

typedef struct {
    int accepted;                             /* 1 if affordance was accepted   */
    int v2Only;                              /* always 1: V2-only affordances  */
    Nexus_V2_TouchControllerInputKind inputKind;
    Nexus_V2_TouchControllerAffordance affordance;
    int movementCommand;                      /* NEXUS_CMD_*, or NEXUS_CMD_NONE */
    int routeKind;                           /* 0=V1_SOURCE, 1=V2_PRESENTATION */
} Nexus_V2_TouchControllerAffordanceRoute;

/* ── Function declarations ─────────────────────────────────────────── */

/* Map an affordance to its corresponding Nexus V1 movement command.
 * Returns NEXUS_CMD_NONE (0) for NONE (no pending action).
 * Source: nexus_v1_movement.h:NEXUS_CMD_* (1-6), CLIKMENU.C:224-233. */
int nexus_v2_touch_controller_affordance_movement_command(
    Nexus_V2_TouchControllerAffordance affordance);

/* Classify an affordance as touch or controller origin. */
Nexus_V2_TouchControllerInputKind
nexus_v2_touch_controller_affordance_input_kind(
    Nexus_V2_TouchControllerAffordance affordance);

/* Route an affordance through the V2 presentation pipeline.
 * Returns accepted=1 only when v2PresentationEnabled is true AND
 * the affordance maps to a valid movement command.
 * When v2PresentationEnabled is false, all affordances are rejected
 * so the V1 mouse/touch/click route matrix is the sole input path.
 * Source: nexus_v1_movement.c:nexus_movement_tick(), GAMELOOP.C:164-219. */
Nexus_V2_TouchControllerAffordanceRoute
nexus_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    Nexus_V2_TouchControllerAffordance affordance);

/* Human-readable name for debugging/logging. */
const char *
nexus_v2_touch_controller_affordance_name(
    Nexus_V2_TouchControllerAffordance affordance);

/* Source-lock evidence string for verification scripts. */
const char *
nexus_v2_touch_controller_affordance_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_TOUCH_CONTROLLER_AFFORDANCE_H */