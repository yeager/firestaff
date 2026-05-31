/*
 * dm2_v2_touch_controller_affordance.c — DM2 V2 Touch/Controller Affordances
 *
 * DM2 V2 Phase 6 — Touch/controller ergonomics
 *
 * Source-lock anchors:
 *   SKULL.ASM T520  — party/movement tick
 *   SKULL.ASM T048  — input dispatch
 *   SKULL.ASM T560  — dungeon viewport rendering
 *   ReDMCSB COMMAND.C:108-113  mouse movement zones C001-C006
 *   ReDMCSB COMMAND.C:254-291  keyboard tables for C001..C006
 *   ReDMCSB CLIKMENU.C:142    F0365_COMMAND_ProcessTypes1To2_TurnParty
 *   ReDMCSB CLIKMENU.C:180    F0366_COMMAND_ProcessTypes3To6_MoveParty
 *   ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop
 *
 * Architecture:
 *   DM2 inherits the DM1 movement engine (C001-C006) verbatim.
 *   DM2 V2 affordances map to those same C001-C006 movement commands
 *   via the shared DM1_V2_MovementCommand enum.
 *   Edge-strafe is V2-only: V1 path (DEFS.H:238-243) ignores edge zones.
 *   No pinch-to-zoom (DM2 has no zoomable minimap).
 *   No shoulder button mappings (DM2 has no champion-cycle or
 *     chaos-magic equivalents — CSB-only actions).
 *
 * V1 invariant: with v2PresentationEnabled=0, all affordances return
 * accepted=0 so the V1 mouse/touch/click route matrix is sole input path.
 * V2 presentation only: DM2_V2_AFFORDANCE_* are metadata labels that
 * route through the existing movement command adapter.
 */

#include "dm2_v2_touch_controller_affordance.h"
#include <string.h>

/* ── Internal helpers ─────────────────────────────────────────────────── */

static DM2_V2_TouchControllerAffordanceRoute
make_route(int accepted,
           DM2_V2_TouchControllerAffordance aff,
           DM1_V2_MovementCommand cmd,
           DM1_V2_MovementCommandRoute mvRoute) {
    DM2_V2_TouchControllerAffordanceRoute r;
    r.accepted = accepted;
    r.v2Only = 1;
    r.inputKind = dm2_v2_touch_controller_affordance_input_kind(aff);
    r.affordance = aff;
    r.movementCommand = cmd;
    r.route = mvRoute;
    return r;
}

/* ── Affordance → movement command mapping ─────────────────────────────
 *
 * ReDMCSB COMMAND.C:108-113 defines C001-C006 as the movement/turn
 * command ids that CLIKMENU.C:142 (F0365 turn) and CLIKMENU.C:180
 * (F0366 move) consume.  DM2 inherits the DM1 movement engine
 * verbatim; this mapping is identical to DM1 V2 Phase 6.
 *
 * Affordance         → Movement command  → Source id
 * -----------------------------------------------------
 * SWIPE_UP / DPAD_UP / L_STICK_UP  → MOVE_FORWARD  → C003
 * SWIPE_DOWN / DPAD_DOWN / L_STICK_DOWN → MOVE_BACK → C005
 * SWIPE_LEFT / DPAD_LEFT / R_STICK_LEFT → TURN_LEFT → C001
 * SWIPE_RIGHT / DPAD_RIGHT / R_STICK_RIGHT → TURN_R → C002
 * EDGE_STRAFE_LEFT / L_STICK_LEFT  → MOVE_LEFT    → C006
 * EDGE_STRAFE_RIGHT / L_STICK_RIGHT → MOVE_RIGHT   → C004
 */

DM1_V2_MovementCommand
dm2_v2_touch_controller_affordance_movement_command(
    DM2_V2_TouchControllerAffordance aff) {
    switch (aff) {
        /* Forward/backward */
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD;

        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD;

        /* Turn left/right */
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_LEFT;

        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT;

        /* Strafe left/right — V2-only edge zones map to C006/C004
         * ReDMCSB DEFS.H:238-243: C005/C006 are strafe commands
         * used only when the pointer is in the edge zones.
         * V1 mouse path ignores edge zones (V2-only affordance). */
        case DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT;

        case DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
            return DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT;

        case DM2_V2_AFFORDANCE_NONE:
        default:
            return DM1_V2_MOVEMENT_COMMAND_NONE;
    }
}

/* ── Input-kind classification ─────────────────────────────────────────── */

DM2_V2_TouchControllerInputKind
dm2_v2_touch_controller_affordance_input_kind(
    DM2_V2_TouchControllerAffordance aff) {
    switch (aff) {
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
            return DM2_V2_AFFORDANCE_INPUT_TOUCH;

        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
        case DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
        case DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return DM2_V2_AFFORDANCE_INPUT_CONTROLLER;

        case DM2_V2_AFFORDANCE_NONE:
        default:
            return DM2_V2_AFFORDANCE_INPUT_NONE;
    }
}

/* ── Route dispatch ──────────────────────────────────────────────────────
 *
 * ReDMCSB GAMELOOP.C:164-219: V1 input wait/command queue loop.
 * V2 affordances are accepted only when v2PresentationEnabled is true.
 * When false, all affordances return accepted=0, preserving the V1
 * mouse/touch/click matrix (F0358/F0359) as sole input path.
 *
 * V2 route: affordance → movement command → V2 presentation adapter
 * The movement command adapter routes through dm1_v2_movement_command_
 * route_for_presentation(1, cmd), which returns the V2 presentation
 * route binding to CLIKMENU.C:142/180.
 */

DM2_V2_TouchControllerAffordanceRoute
dm2_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    DM2_V2_TouchControllerAffordance aff) {
    DM1_V2_MovementCommand cmd =
        dm2_v2_touch_controller_affordance_movement_command(aff);
    DM1_V2_MovementCommandRoute mvRoute;

    /* V1 parity guard: reject all affordances when V2 presentation is
     * disabled. This preserves the V1 mouse/touch/click route matrix
     * (ReDMCSB DEFS.H:197-211, COMMAND.C:1379-1452, CLIKMENU.C:142/180)
     * as the sole input path for non-V2 builds.
     * movementCommand is preserved so callers can identify which affordance
     * was rejected (for logging/debugging); the V1 routeKind signals that
     * the command was NOT injected into the V1 movement pipeline. */
    if (!v2PresentationEnabled) {
        mvRoute = dm1_v2_movement_command_route_for_presentation(
            0, cmd);  /* cmd preserved for debug; routeKind=V1_SOURCE */
        return make_route(0, aff, cmd, mvRoute);
    }

    /* No movement command → accepted only if it is a valid V2-only
     * affordance with a defined route but no movement command.
     * (DM2 has no pinch-to-zoom or shoulder buttons, so this path
     * returns accepted=0 with NONE command for all current affordances.) */
    if (cmd == DM1_V2_MOVEMENT_COMMAND_NONE) {
        mvRoute = dm1_v2_movement_command_route_for_presentation(
            1, DM1_V2_MOVEMENT_COMMAND_NONE);
        return make_route(0, aff, cmd, mvRoute);
    }

    mvRoute = dm1_v2_movement_command_route_for_presentation(1, cmd);
    return make_route(1, aff, cmd, mvRoute);
}

/* ── Debug name lookup ────────────────────────────────────────────────── */

const char *
dm2_v2_touch_controller_affordance_name(
    DM2_V2_TouchControllerAffordance aff) {
    switch (aff) {
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP:               return "touch_swipe_up";
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:             return "touch_swipe_down";
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:             return "touch_swipe_left";
        case DM2_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:            return "touch_swipe_right";
        case DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:      return "touch_edge_strafe_left";
        case DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:     return "touch_edge_strafe_right";
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP:          return "controller_dpad_up";
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:        return "controller_dpad_down";
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:        return "controller_dpad_left";
        case DM2_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:       return "controller_dpad_right";
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:    return "controller_left_stick_up";
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:  return "controller_left_stick_down";
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:  return "controller_left_stick_left";
        case DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:  return "controller_left_stick_right";
        case DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT: return "controller_right_stick_left";
        case DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:return "controller_right_stick_right";
        case DM2_V2_AFFORDANCE_NONE:
        default:                                             return "none";
    }
}

/* ── Source evidence ─────────────────────────────────────────────────── */

const char *
dm2_v2_touch_controller_affordance_source_evidence(void) {
    return
        "DM2 V2 Touch/Controller Affordance — Phase 6\n"
        "Source: SKULL.ASM T520  — party/movement tick\n"
        "Source: SKULL.ASM T048  — input dispatch\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering\n"
        "Source: ReDMCSB COMMAND.C:108-113 C001-C006 movement zones\n"
        "Source: ReDMCSB COMMAND.C:254-291 C001-C006 keyboard tables\n"
        "Source: ReDMCSB CLIKMENU.C:142 F0365_COMMAND_ProcessTypes1To2_TurnParty\n"
        "Source: ReDMCSB CLIKMENU.C:180 F0366_COMMAND_ProcessTypes3To6_MoveParty\n"
        "Source: ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop\n"
        "DM2 inherits DM1 movement engine (C001-C006) verbatim.\n"
        "V2 affordances route through dm1_v2_movement_command_route_for_presentation.\n"
        "No pinch-to-zoom (DM2 has no zoomable minimap).\n"
        "No shoulder button mappings (DM2 has no champion-cycle/chaos-magic).\n"
        "V1 touch/click parity: all affordances rejected when v2PresentationEnabled=0.\n"
        "Edge-strafe is V2-only (V1 path ignores edge zones per DEFS.H:238-243).\n"
        "DM2 vs DM1 V2 Phase 6: no pinch-to-zoom, no shoulder button mappings.\n"
        "DM2 vs CSB V2 Phase 6: no champion-cycle (left bumper), no chaos-magic (right bumper).\n";
}