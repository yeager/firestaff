/*
 * nexus_v2_touch_controller_affordance.c — Nexus V2 Touch/Controller Affordances
 *
 * Nexus V2 Phase 6 — Touch/controller ergonomics
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
 *   Nexus inherits the DM1 movement engine (step/turn/stamina logic)
 *   via nexus_v1_movement.c.  This file labels V2 affordances and maps
 *   them to the same NEXUS_CMD_* enum values that V1 uses.
 *
 *   V2 affordance → NEXUS_CMD_* → nexus_input_queue_push() → nexus_movement_tick()
 *   → either V1 source route (disabled) or V2 presentation route (enabled).
 *
 * V1 invariant: with v2PresentationEnabled=0, all affordances return
 * accepted=0 so the V1 mouse/touch/click route matrix is sole input path.
 * V2 presentation only: NEXUS_V2_AFFORDANCE_* are metadata labels that
 * route through the existing Nexus movement command queue.
 *
 * Differences from DM1 V2 Phase 6 (dm1_v2_touch_controller_affordance_pc34):
 *   - Nexus uses NEXUS_CMD_* (1-6) vs DM1 C001-C006 command ids
 *   - No pinch-to-zoom (Nexus/Saturn has no zoomable minimap)
 *   - No shoulder button mappings (no champion-cycle/chaos-magic in Nexus)
 *   - Right stick turns only (Saturn gamepad layout)
 *
 * Differences from DM2 V2 Phase 6 (dm2_v2_touch_controller_affordance):
 *   - DM2 uses DM1_V2_MovementCommand enum (shared C001-C006 system)
 *   - Nexus uses its own NEXUS_CMD_* enum (1-6, same movement semantics)
 *   - Identical behavior: right stick turns only, no shoulder buttons
 */

#include "nexus_v2_touch_controller_affordance.h"
#include "nexus_v1_movement.h"
#include <string.h>

/* ── Internal helpers ─────────────────────────────────────────────────── */

static Nexus_V2_TouchControllerAffordanceRoute
make_route(int accepted,
           Nexus_V2_TouchControllerAffordance aff,
           int cmd,
           int routeKind) {
    Nexus_V2_TouchControllerAffordanceRoute r;
    r.accepted = accepted;
    r.v2Only = 1;
    r.inputKind = nexus_v2_touch_controller_affordance_input_kind(aff);
    r.affordance = aff;
    r.movementCommand = cmd;
    r.routeKind = routeKind;
    return r;
}

/* Route kind constants (parallel to DM1_V2_MovementCommandRouteKind) */
#define NEXUS_V2_ROUTE_KIND_V1_SOURCE       0
#define NEXUS_V2_ROUTE_KIND_V2_PRESENTATION 1

/* ── Affordance → movement command mapping ─────────────────────────────
 *
 * Source: nexus_v1_movement.h NEXUS_CMD_* (1-6), CLIKMENU.C:224-233.
 * Affordance         → NEXUS_CMD_*        → Movement
 * -----------------------------------------------------------------------
 * SWIPE_UP / DPAD_UP / L_STICK_UP    → FORWARD      → step forward
 * SWIPE_DOWN / DPAD_DOWN / L_STICK_DOWN → BACKWARD   → step backward
 * SWIPE_LEFT / DPAD_LEFT / R_STICK_LEFT → TURN_LEFT  → turn left (no step)
 * SWIPE_RIGHT / DPAD_RIGHT / R_STICK_RIGHT → TURN_RIGHT → turn right (no step)
 * EDGE_STRAFE_LEFT / L_STICK_LEFT    → STRAFE_LEFT  → lateral step left
 * EDGE_STRAFE_RIGHT / L_STICK_RIGHT  → STRAFE_RIGHT → lateral step right
 */

int
nexus_v2_touch_controller_affordance_movement_command(
    Nexus_V2_TouchControllerAffordance aff) {
    switch (aff) {
        /* Forward/backward */
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
            return NEXUS_CMD_FORWARD;

        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
            return NEXUS_CMD_BACKWARD;

        /* Turn left/right */
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
            return NEXUS_CMD_TURN_LEFT;

        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return NEXUS_CMD_TURN_RIGHT;

        /* Strafe left/right — maps to NEXUS_CMD_STRAFE_LEFT/RIGHT
         * Nexus V1 already supports strafe (nexus_v1_movement.c handles it);
         * edge-strafe affordance is V2 presentation only */
        case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
            return NEXUS_CMD_STRAFE_LEFT;

        case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
            return NEXUS_CMD_STRAFE_RIGHT;

        case NEXUS_V2_AFFORDANCE_NONE:
        default:
            return NEXUS_CMD_NONE;
    }
}

/* ── Input-kind classification ─────────────────────────────────────────── */

Nexus_V2_TouchControllerInputKind
nexus_v2_touch_controller_affordance_input_kind(
    Nexus_V2_TouchControllerAffordance aff) {
    switch (aff) {
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP:
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
        case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
        case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
            return NEXUS_V2_AFFORDANCE_INPUT_TOUCH;

        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
        case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
            return NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER;

        case NEXUS_V2_AFFORDANCE_NONE:
        default:
            return NEXUS_V2_AFFORDANCE_INPUT_NONE;
    }
}

/* ── Route dispatch ──────────────────────────────────────────────────────
 *
 * Source: nexus_v1_movement.c:nexus_movement_tick(), GAMELOOP.C:164-219.
 *
 * V2 affordances are accepted only when v2PresentationEnabled is true.
 * When false, all affordances return accepted=0, preserving the V1
 * mouse/touch/click matrix as sole input path.
 *
 * V2 route: affordance → NEXUS_CMD_* → nexus_input_queue_push()
 * The movement command adapter routes through the Nexus V1 movement
 * pipeline which handles the cooldown, stamina, and collision checks.
 */

Nexus_V2_TouchControllerAffordanceRoute
nexus_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    Nexus_V2_TouchControllerAffordance aff) {
    int cmd = nexus_v2_touch_controller_affordance_movement_command(aff);
    int routeKind;

    /* V1 parity guard: reject all affordances when V2 presentation is
     * disabled. This preserves the V1 mouse/touch/click route matrix
     * (DM1 CLIKMENU.C:142/180, nexus_v1_movement.c:nexus_movement_tick())
     * as the sole input path for non-V2 builds. */
    if (!v2PresentationEnabled) {
        routeKind = NEXUS_V2_ROUTE_KIND_V1_SOURCE;
        return make_route(0, aff, cmd, routeKind);
    }

    /* No movement command → accepted=0 (no valid V2-only affordances
     * without movement commands in current enum) */
    if (cmd == NEXUS_CMD_NONE) {
        routeKind = NEXUS_V2_ROUTE_KIND_V2_PRESENTATION;
        return make_route(0, aff, cmd, routeKind);
    }

    /* Valid movement command → accepted, routed as V2 presentation */
    routeKind = NEXUS_V2_ROUTE_KIND_V2_PRESENTATION;
    return make_route(1, aff, cmd, routeKind);
}

/* ── Debug name lookup ────────────────────────────────────────────────── */

const char *
nexus_v2_touch_controller_affordance_name(
    Nexus_V2_TouchControllerAffordance aff) {
    switch (aff) {
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP:              return "touch_swipe_up";
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:            return "touch_swipe_down";
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:            return "touch_swipe_left";
        case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:           return "touch_swipe_right";
        case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:       return "touch_edge_strafe_left";
        case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:      return "touch_edge_strafe_right";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP:          return "controller_dpad_up";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:        return "controller_dpad_down";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:        return "controller_dpad_left";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:       return "controller_dpad_right";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:    return "controller_left_stick_up";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:  return "controller_left_stick_down";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:  return "controller_left_stick_left";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT: return "controller_left_stick_right";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT: return "controller_right_stick_left";
        case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:return "controller_right_stick_right";
        case NEXUS_V2_AFFORDANCE_NONE:
        default:                                               return "none";
    }
}

/* ── Source evidence ─────────────────────────────────────────────────── */

const char *
nexus_v2_touch_controller_affordance_source_evidence(void) {
    return
        "Nexus V2 Touch/Controller Affordance — Phase 6\n"
        "Source: DMDF/DGN level format — Nexus movement grid\n"
        "Source: Saturn NEXUS.BIN input surface data\n"
        "Source: Saturn SDK joystick mapping (D-pad, analog sticks)\n"
        "Source: DM1 CLIKMENU.C:224-233 arrow deltas / blocked movement\n"
        "Source: DM1 CLIKMENU.C:237-255 F0325 stamina cost before movement\n"
        "Source: DM1 CLIKMENU.C:142 F0365_COMMAND_ProcessTypes1To2_TurnParty (turn)\n"
        "Source: DM1 CLIKMENU.C:180 F0366_COMMAND_ProcessTypes3To6_MoveParty (step)\n"
        "Source: DM1 REALTIME.ASM T048 input dispatch\n"
        "Source: DM1 GAMELOOP.C:164-219 V1 input wait/command queue loop\n"
        "Source: nexus_v1_movement.c:nexus_movement_tick() V1 tick handler\n"
        "Source: nexus_v1_movement.h:NEXUS_CMD_* (1-6 movement commands)\n"
        "Nexus inherits DM1 movement engine via nexus_v1_movement.c.\n"
        "V2 affordances map to NEXUS_CMD_* (1-6), same as V1 input path.\n"
        "No pinch-to-zoom (Nexus/Saturn has no zoomable minimap).\n"
        "No shoulder button mappings (Nexus has no champion-cycle/chaos-magic).\n"
        "Right stick turns only (Saturn gamepad layout).\n"
        "V1 touch/click parity: all affordances rejected when v2PresentationEnabled=0.\n"
        "Nexus vs DM1 V2 Phase 6: NEXUS_CMD_* enum vs C001-C006 (same semantics).\n"
        "Nexus vs DM2 V2 Phase 6: same movement model, different enum namespace.\n";
}