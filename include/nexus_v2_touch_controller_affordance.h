#ifndef FIRESTAFF_NEXUS_V2_TOUCH_CONTROLLER_AFFORDANCE_H
#define FIRESTAFF_NEXUS_V2_TOUCH_CONTROLLER_AFFORDANCE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Nexus V2 Touch/Controller Affordance — Phase 6
 *
 * Experimental V2 gesture/controller affordance labels for Dungeon Master
 * Nexus. The local NEXUS_CMD_* outputs are not authenticated Saturn button
 * mappings and this helper is not evidence of retail input parity. Production
 * input remains gated until a real control-to-action trace is available.
 *
 * This helper describes an experimental interface only. Saturn button
 * assignments, action dispatch, and controller ergonomics remain unverified.
 */

/* ── Affordance IDs ─────────────────────────────────────────────────── */

typedef enum {
    NEXUS_V2_AFFORDANCE_NONE = 0,

    /* Experimental local mapping from touch gesture labels to command IDs. */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP,           /* NEXUS_CMD_FORWARD      */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,        /* NEXUS_CMD_BACKWARD     */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,        /* NEXUS_CMD_TURN_LEFT    */
    NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,       /* NEXUS_CMD_TURN_RIGHT   */

    /* Experimental touch edge-zone labels. */
    NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,  /* NEXUS_CMD_STRAFE_LEFT  */
    NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, /* NEXUS_CMD_STRAFE_RIGHT */

    /* Experimental controller direction labels. */
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP,
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,
    NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,

    /* Experimental left-stick direction labels. */
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,
    NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,

    /* Right-stick directions are experimental affordance labels only. */
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
    int routeKind;                           /* legacy local labels: 0 or 1 */
} Nexus_V2_TouchControllerAffordanceRoute;

/* ── Function declarations ─────────────────────────────────────────── */

/* Map an experimental affordance label to a local command identifier.
 * This mapping is not an authenticated Saturn control mapping. */
int nexus_v2_touch_controller_affordance_movement_command(
    Nexus_V2_TouchControllerAffordance affordance);

/* Classify an affordance as touch or controller origin. */
Nexus_V2_TouchControllerInputKind
nexus_v2_touch_controller_affordance_input_kind(
    Nexus_V2_TouchControllerAffordance affordance);

/* Apply the local diagnostic toggle to an affordance label.
 * Acceptance does not dispatch a game action or establish retail behavior. */
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
