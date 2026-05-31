#ifndef FIRESTAFF_DM2_V2_TOUCH_CONTROLLER_AFFORDANCE_H
#define FIRESTAFF_DM2_V2_TOUCH_CONTROLLER_AFFORDANCE_H

#include "dm1_v2_movement_command_adapter_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM2 V2 Touch/Controller Affordance — Phase 6
 *
 * V2-only gesture/controller affordances for Dungeon Master II: Skullkeep.
 * Maps touch swipes and controller inputs to DM1_V2_MovementCommand
 * equivalents (C001-C006), using the shared DM1 movement engine adapter.
 * V1 touch/click parity is preserved: when v2PresentationEnabled is false,
 * all affordances are rejected so the V1 mouse/touch/click route matrix
 * remains the sole input path.
 *
 * Differences from DM1 V2 Phase 6 (dm1_v2_touch_controller_affordance_pc34):
 *   - No pinch-to-zoom (DM2 has no zoomable minimap equivalent)
 *   - No shoulder button mappings (DM2 has no champion-cycle or
 *     chaos-magic equivalents — those are CSB-specific actions)
 *   - Edge-strafe is V2-only and maps to C004/C006 (same as DM1)
 *
 * Source-lock anchors:
 *   SKULL.ASM T520  — party/movement tick
 *   SKULL.ASM T048  — input dispatch
 *   SKULL.ASM T560  — dungeon viewport rendering
 *   ReDMCSB COMMAND.C:108-113  mouse movement zones C001-C006
 *   ReDMCSB COMMAND.C:254-291 keyboard tables for C001..C006
 *   ReDMCSB CLIKMENU.C:142   F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   ReDMCSB CLIKMENU.C:180   F0366_COMMAND_ProcessTypes3To6_MoveParty
 *   ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop
 *
 * Architecture:
 *   V2 affordance → DM1_V2_MovementCommand → dm1_v2_movement_command_route_for_presentation
 *   → either V1 source route (disabled) or V2 presentation route (enabled).
 *   DM2 inherits the DM1 movement engine (C001-C006) verbatim.
 */

/* ── Affordance IDs ─────────────────────────────────────────────────── */

typedef enum {
    DM2_V2_AFFORDANCE_NONE = 0,

    /* Touch swipe gestures — map to movement commands */
    DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP,       /* C003 MOVE FORWARD */
    DM2_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,      /* C005 MOVE BACKWARD */
    DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,      /* C001 TURN LEFT     */
    DM2_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,     /* C002 TURN RIGHT    */

    /* Touch edge-strafe — V2-only, V1 path ignores edge zones */
    DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,  /* C006 MOVE LEFT  */
    DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, /* C004 MOVE RIGHT */

    /* Controller D-pad — identical mapping to swipe gestures */
    DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP,
    DM2_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,
    DM2_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,
    DM2_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,

    /* Left stick — forward/back with strafing on left/right */
    DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,
    DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,
    DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,
    DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,

    /* Right stick — turning only (no movement) */
    DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,  /* C001 TURN LEFT  */
    DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT  /* C002 TURN RIGHT */
} DM2_V2_TouchControllerAffordance;

/* ── Input kind ─────────────────────────────────────────────────────── */

typedef enum {
    DM2_V2_AFFORDANCE_INPUT_NONE = 0,
    DM2_V2_AFFORDANCE_INPUT_TOUCH = 1,
    DM2_V2_AFFORDANCE_INPUT_CONTROLLER = 2
} DM2_V2_TouchControllerInputKind;

/* ── Routing result ────────────────────────────────────────────────── */

typedef struct {
    int accepted;                             /* 1 if affordance was accepted   */
    int v2Only;                              /* always 1: V2-only affordances  */
    DM2_V2_TouchControllerInputKind inputKind;
    DM2_V2_TouchControllerAffordance affordance;
    DM1_V2_MovementCommand movementCommand;  /* C001-C006, or NONE             */
    DM1_V2_MovementCommandRoute route;      /* V1 source or V2 presentation  */
} DM2_V2_TouchControllerAffordanceRoute;

/* ── Function declarations ─────────────────────────────────────────── */

/* Map an affordance to its corresponding DM1 V2 movement command.
 * Returns MOVEMENT_COMMAND_NONE for NONE (no pending action). */
DM1_V2_MovementCommand
dm2_v2_touch_controller_affordance_movement_command(
    DM2_V2_TouchControllerAffordance affordance);

/* Classify an affordance as touch or controller origin. */
DM2_V2_TouchControllerInputKind
dm2_v2_touch_controller_affordance_input_kind(
    DM2_V2_TouchControllerAffordance affordance);

/* Route an affordance through the V2 presentation pipeline.
 * Returns accepted=1 only when v2PresentationEnabled is true AND
 * the affordance maps to a valid movement command.
 * When v2PresentationEnabled is false, all affordances are rejected
 * so the V1 mouse/touch/click route matrix is the sole input path. */
DM2_V2_TouchControllerAffordanceRoute
dm2_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    DM2_V2_TouchControllerAffordance affordance);

/* Human-readable name for debugging/logging. */
const char *
dm2_v2_touch_controller_affordance_name(
    DM2_V2_TouchControllerAffordance affordance);

/* Source-lock evidence string for verification scripts. */
const char *
dm2_v2_touch_controller_affordance_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_TOUCH_CONTROLLER_AFFORDANCE_H */