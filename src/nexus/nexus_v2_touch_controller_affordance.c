/*
 * nexus_v2_touch_controller_affordance.c — Nexus V2 Touch/Controller Affordances
 *
 * Nexus V2 Phase 6 — Touch/controller ergonomics
 *
 * Experimental UI metadata only. No retail Saturn button-to-action mapping
 * or VDP/input-dispatch consumer is admitted. Local NEXUS_CMD_* values are
 * API identifiers, not proof of original controller semantics.
 *
 * Architecture:
 *   The experimental mapper produces local NEXUS_CMD_* values. It is not
 *   connected to an authenticated Saturn input-dispatch route.
 *
 * The local toggle controls only this helper's returned diagnostic metadata.
 * The helper is not connected to a game input queue or VDP presentation path.
 *
 * DM1/DM2 mappings are not used as Saturn input evidence. Control layouts
 * remain unverified until retail action dispatch is joined to real input.
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

/* Apply the diagnostic toggle and return local metadata. */

Nexus_V2_TouchControllerAffordanceRoute
nexus_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    Nexus_V2_TouchControllerAffordance aff) {
    int cmd = nexus_v2_touch_controller_affordance_movement_command(aff);
    int routeKind;

    /* Keep the historical routeKind value for API compatibility. It does
     * not assert that a V1 retail input route was observed or preserved. */
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
        "Nexus V2 Touch/Controller Affordance — experimental UI mapper only\n"
        "No authenticated retail Saturn input-dispatch consumer receipt\n"
        "NEXUS_CMD_* values are local API identifiers, not verified button semantics\n"
        "DM1/DM2 references are comparisons only and do not establish Saturn behavior\n"
        "Production input remains gated until authentic control-to-action evidence exists\n";
}
