#ifndef FIRESTAFF_CSB_V2_TOUCH_CONTROLLER_AFFORDANCE_H
#define FIRESTAFF_CSB_V2_TOUCH_CONTROLLER_AFFORDANCE_H

#include "dm1_v2_movement_command_adapter_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CSB V2 Touch/Controller Affordance - gesture and gamepad mappings for
 * Chaos Strikes Back V2 presentation mode.
 *
 * Source-lock anchors (shared DM1/CSB engine via ReDMCSB Common Toolchains):
 * - ReDMCSB COMMAND.C:108-113  mouse movement zones (C001-C006)
 * - ReDMCSB COMMAND.C:254-291  keyboard mapping tables for C001..C006
 * - ReDMCSB COMMAND.C:1379     F0358_COMMAND_GetCommandFromMouseInput_CPSC
 * - ReDMCSB COMMAND.C:1452     F0359_COMMAND_ProcessClick_CPSC
 * - ReDMCSB COMMAND.C:1641,1643 primary-to-secondary mouse queue dispatch
 * - ReDMCSB COMMAND.C:1693     F0360_COMMAND_ProcessPendingClick
 * - ReDMCSB CLIKMENU.C:142     F0365_COMMAND_ProcessTypes1To2_TurnParty
 * - ReDMCSB CLIKMENU.C:180     F0366_COMMAND_ProcessTypes3To6_MoveParty
 * - ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop
 *
 * CSB-specific references:
 * - CSBWin/Chaos.cpp:60-69  DSA call dispatch (_CALL0-_CALL9)
 * - CSBWin/DSA.cpp         DSA interpreter (chaos magic scripts)
 * - csb_v1_chaos_magic_pc34_compat.c:50  csb_v1_chaos_trigger
 *
 * Architecture:
 * - V2 touch swipes and controller inputs map to movement commands (C001-C006)
 * - V2-only controller buttons (bumpers) map to CSB-specific actions
 * - When V2 presentation is disabled, all affordances are rejected so the
 *   existing V1 mouse/touch/click route matrix remains the only input path
 * - Pinch-to-zoom: not supported in CSB V2 (CSB minimap is different from DM1)
 * - V1 touch/click parity preserved via touch_pointer_input_pc34_compat.c
 *
 * Differences from DM1 V2 Phase 6:
 * - No pinch-to-zoom affordance (CSB has no zoomable minimap)
 * - Right bumper maps to chaos magic activation (DM1: unused)
 * - Left bumper maps to champion cycle (DM1: unused) */

/* Affordance IDs */

typedef enum {
    CSB_V2_AFFORDANCE_NONE = 0,

    /* Touch swipe gestures - dungeon viewport */
    CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP,
    CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,
    CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,
    CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,

    /* Touch edge-strafe (hold left/right edge + tap centre) */
    CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,
    CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT,

    /* Controller D-pad - identical mapping to swipe gestures */
    CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP,
    CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,
    CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,
    CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,

    /* Left stick - forward/back with strafing on left/right */
    CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,
    CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,
    CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,
    CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,

    /* Right stick - turning (left/right) */
    CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,
    CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT,

    /* Shoulder buttons - CSB-specific actions (no DM1 equivalent) */
    /* Left bumper: cycle to next champion (same as C084/C085 but one-shot) */
    CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER,
    /* Right bumper: activate/cycle chaos magic (CSB-only system) */
    CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER
} CSB_V2_TouchControllerAffordance;

/* Input kind */

typedef enum {
    CSB_V2_AFFORDANCE_INPUT_NONE = 0,
    CSB_V2_AFFORDANCE_INPUT_TOUCH = 1,
    CSB_V2_AFFORDANCE_INPUT_CONTROLLER = 2
} CSB_V2_TouchControllerInputKind;

/* Routing result */

typedef struct {
    int accepted;                              /* 1 if affordance was accepted */
    int v2Only;                                 /* always 1: V2-only affordances */
    CSB_V2_TouchControllerInputKind inputKind;
    CSB_V2_TouchControllerAffordance affordance;
    DM1_V2_MovementCommand movementCommand;    /* C001-C006 equivalent, or NONE */
    DM1_V2_MovementCommandRoute route;         /* V1 source or V2 presentation route */
} CSB_V2_TouchControllerAffordanceRoute;

/* Function declarations */

/* Map an affordance to its corresponding DM1 V2 movement command.
 * Affordances that are not movement-related return MOVEMENT_COMMAND_NONE. */
DM1_V2_MovementCommand csb_v2_touch_controller_affordance_movement_command(
    CSB_V2_TouchControllerAffordance affordance);

/* Classify an affordance as touch or controller origin. */
CSB_V2_TouchControllerInputKind csb_v2_touch_controller_affordance_input_kind(
    CSB_V2_TouchControllerAffordance affordance);

/* Route an affordance through the V2 presentation pipeline.
 * Returns accepted=1 only when v2PresentationEnabled is true AND the
 * affordance maps to a valid movement command (or a CSB-specific action).
 * When v2PresentationEnabled is false, all affordances are rejected so
 * the V1 mouse/touch/click route matrix is the sole input path. */
CSB_V2_TouchControllerAffordanceRoute csb_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    CSB_V2_TouchControllerAffordance affordance);

/* Human-readable name for debugging/logging. */
const char* csb_v2_touch_controller_affordance_name(
    CSB_V2_TouchControllerAffordance affordance);

/* Source-lock evidence string for verification scripts. */
const char* csb_v2_touch_controller_affordance_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_TOUCH_CONTROLLER_AFFORDANCE_H */
