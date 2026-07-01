#ifndef FS_GESTURE_NAVIGATION_GATE_H
#define FS_GESTURE_NAVIGATION_GATE_H

/*
 * fs_gesture_navigation_gate.h — Cross-game runtime gesture navigation gate
 *
 * This header is the public surface for the Firestaff cross-game
 * runtime gesture navigation gate.  It is the seam between
 * platform-level finger/touch events and the per-game movement/turn
 * command pipeline.
 *
 * Scope (intentionally narrow):
 *
 *  1. **Recognition** — Convert a sequence of touch DOWN / MOVE / UP
 *     events into one of the FsGestureType values listed below.
 *  2. **Translation** — Map a recognised gesture to the right
 *     movement/turn command for the active game.  DM1, CSB, DM2,
 *     Nexus and Theron all have their own FsGestureGameCommand
 *     rows; the gate looks up the row for the active FsGameId.
 *  3. **Touch target safety** — Audit a set of touch zones (x, y, w,
 *     h) and report any zone below the safety minimum.  This is the
 *     "UI scaling and touch-target audit across launcher and game
 *     views" cross-cutting concern.
 *
 * What this gate does NOT do (left to the existing systems):
 *  - It does not own a real SDL_FingerEvent queue.  Callers feed in
 *    events via fs_gesture_feed_down/move/up and the gate produces
 *    FsGestureEvent records.  The existing firestaff_touch.c owns
 *    the DM1 V1 SDL3 plumbing.
 *  - It does not bypass keyboard routes.  When settings disable
 *    gestures for the active game, the recognizer still runs but
 *    every translation is rejected with FS_GESTURE_DISABLED.
 *  - It does not change game-logic.  All gestures ultimately resolve
 *    to the same source-owned movement/turn command IDs that
 *    ReDMCSB routes dispatch.
 *
 * Source-lock anchors (full citation list in
 * docs/source-lock/fs_gesture_navigation_gate.md):
 *
 *   ReDMCSB COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC
 *     consumes the queued source command and dispatches C001/C002
 *     to F0365 turns and C003..C006 to F0366 movement without
 *     knowing whether the command came from keyboard, mouse, or
 *     synthesized touch.
 *
 *   ReDMCSB CLIKMENU.C:142-174 F0365 turn handling.
 *   ReDMCSB CLIKMENU.C:180-347 F0366 movement handling.
 *
 *   ReDMCSB DEFS.H:238-243 owns movement command IDs C001..C006
 *     which are the cross-game movement/turn command constants the
 *     gate maps to.
 *
 *   ReDMCSB COMMAND.C:375-405 mouse movement-arrow zones (the
 *     existing touch/click zone matrix that the gesture recognizer
 *     does not duplicate).
 *
 *   ReDMCSB COMMAND.C:1379-1449 F0358_COMMAND_GetCommandFromMouseInput_CPSC
 *     the source hit-test the gesture path bypasses by translating
 *     to the same C001..C006 command IDs directly.
 *
 * Settings gate: a single boolean `enabled` per active game is
 *   read from the M12 config (M12_Config.dm1V2AccessibilityTouchEnabled
 *   + sibling per-game entries).  The gate is OFF by default and
 *   must be explicitly enabled.
 *
 * Data-free: this header does NOT load any game asset.  All gesture
 *   recognition + per-game command translation + touch-target audit
 *   run from pure C11 logic on synthetic input.  This keeps the gate
 *   CI-greppable, headless-testable, and free of any proprietary
 *   asset dependency.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Active game id ─────────────────────────────────────────────────── */
/* Mirrors FS_GameId in include/firestaff_game_loop.h.  Defined again
 * here so this header is self-contained for non-engine callers (M12
 * launcher, headless probes, runtime tests).  Values must stay in
 * sync with firestaff_game_loop.h. */
typedef enum {
    FS_GG_GAME_DM1 = 0,
    FS_GG_GAME_CSB,
    FS_GG_GAME_DM2,
    FS_GG_GAME_NEXUS,
    FS_GG_GAME_THERON = 5 /* placeholder; matches firestaff_cli.c */
} FsGgGameId;

#define FS_GG_GAME_ID_COUNT 6
#define FS_GG_GAME_ID_INVALID (-1)

/* ── Recognised gesture types ───────────────────────────────────────── */
typedef enum {
    FS_GG_GESTURE_NONE = 0,
    FS_GG_GESTURE_SWIPE_UP,          /* primary forward */
    FS_GG_GESTURE_SWIPE_DOWN,        /* primary backward */
    FS_GG_GESTURE_SWIPE_LEFT,        /* turn left */
    FS_GG_GESTURE_SWIPE_RIGHT,       /* turn right */
    FS_GG_GESTURE_LONG_PRESS,        /* hold > threshold  → action/inventory */
    FS_GG_GESTURE_DOUBLE_TAP,        /* two quick taps   → accept/forward */
    FS_GG_GESTURE_EDGE_STRAFE_LEFT,  /* left-edge swipe  → strafe left */
    FS_GG_GESTURE_EDGE_STRAFE_RIGHT, /* right-edge swipe → strafe right */
    FS_GG_GESTURE_PINCH_ZOOM_IN,     /* two-finger spread */
    FS_GG_GESTURE_PINCH_ZOOM_OUT,    /* two-finger pinch */
    FS_GG_GESTURE_TAP,               /* single tap (no movement) */
    FS_GG_GESTURE_DRAG,              /* moved beyond tap tolerance */
    FS_GG_GESTURE_TYPE_COUNT
} FsGestureType;

/* ── Recognizer dispatch reason ─────────────────────────────────────── */
/* Why a translation was rejected — useful for telemetry and audit. */
typedef enum {
    FS_GG_OK = 0,
    FS_GG_DISABLED,                  /* settings gate OFF for this game */
    FS_GG_UNSUPPORTED_GAME,          /* game id outside the per-game table */
    FS_GG_UNSUPPORTED_GESTURE,       /* gesture not in the per-game row */
    FS_GG_NO_PENDING_GESTURE,        /* no recognizer output yet */
    FS_GG_TAP_NOT_MOVEMENT,          /* TAP gestures do not move */
    FS_GG_QUEUE_FULL,                /* input queue rejected the push */
    FS_GG_NULL_ARG,                  /* null out pointer */
    FS_GG_NOT_INITIALIZED,           /* fs_gesture_gate_init not called */
    FS_GG_DISPATCH_REASON_COUNT
} FsGestureDispatchReason;

/* Human-readable name of a gesture (for telemetry/log lines). */
const char* fs_gesture_type_name(FsGestureType type);

/* Human-readable name of a dispatch reason. */
const char* fs_gesture_dispatch_reason_name(FsGestureDispatchReason reason);

/* ── Recognizer events ──────────────────────────────────────────────── */
/* What a recognizer step produced.  Dispatch rows below are filled by
 * fs_gesture_recognizer_step. */
typedef enum {
    FS_GG_FEED_DOWN = 1,
    FS_GG_FEED_MOVE = 2,
    FS_GG_FEED_UP   = 3
} FsGestureFeedKind;

typedef struct {
    FsGestureFeedKind kind;        /* DOWN / MOVE / UP */
    int x;                         /* framebuffer x (already scaled) */
    int y;                         /* framebuffer y */
    uint32_t nowMs;                /* monotonic clock at the event */
} FsGestureFeedEvent;

/* ── Per-game translation row ───────────────────────────────────────── */
/* Game-specific movement/turn command id.  For DM1/CSB/DM2/Nexus the
 * id is the source ReDMCSB C001..C006 command id (DM1_V1_COMMAND_*).
 * For Theron the id is a smaller, equivalent TQR.PCE id reserved by
 * theron_v1_mechanics; the recognizer treats Theron ids as opaque
 * and surfaces them through the gate's translation record. */
typedef struct {
    int accepted;
    FsGestureType gesture;
    FsGgGameId game;
    int gameCommand;                 /* game-specific command id, -1 if N/A */
    int isMovement;                  /* 1 if it changes the party pose */
    int isTurn;                      /* 1 if it changes facing */
    const char* sourceEvidence;      /* ReDMCSB / sibling citation */
} FsGestureGameCommand;

/* ── Touch target safety ────────────────────────────────────────────── */
typedef struct {
    int x, y, w, h;
    const char* groupName;          /* movement.turn_left / etc.; nullable */
} FsGestureZone;

typedef struct {
    int zoneOrdinal;
    const char* groupName;
    int w, h;
    int meetsMinimum;               /* 1 if w>=min AND h>=min */
    int meetsRecommended;           /* 1 if w>=rec AND h>=rec */
    int minShortSide;               /* the shorter side for this zone */
} FsGestureZoneAudit;

#define FS_GG_ZONE_AUDIT_MAX 64

typedef struct {
    int minimumSidePx;              /* platform minimum (default 24) */
    int recommendedSidePx;          /* platform recommended (default 44) */
    int totalZones;                 /* input count */
    int zonesBelowMinimum;          /* count of zones that fail minimum */
    int zonesBelowRecommended;      /* count that fail recommended */
    FsGestureZoneAudit audits[FS_GG_ZONE_AUDIT_MAX];
} FsGestureZoneAuditReport;

/* ── Lifecycle ──────────────────────────────────────────────────────── */
/* Initialize the gate.  Idempotent.  After init the recognizer is in
 * a known-clean state and the default settings (all OFF) are applied.
 * Returns 1 on success, 0 on null args. */
int fs_gesture_gate_init(void);

/* Shut the gate down.  After shutdown all feeds return
 * FS_GG_NOT_INITIALIZED.  Idempotent. */
void fs_gesture_gate_shutdown(void);

/* Returns 1 if the gate is initialized. */
int fs_gesture_gate_is_initialized(void);

/* ── Settings gate ──────────────────────────────────────────────────── */
/* Per-game enable/disable.  Recognizer runs in both states; the gate
 * refuses translations when the active game is disabled.  Returns
 * the previous value (0 or 1) for the game, or -1 on bad game id. */
int fs_gesture_gate_set_enabled(FsGgGameId game, int enabled);

/* Returns 1 if enabled, 0 if disabled, -1 on bad game id. */
int fs_gesture_gate_is_enabled(FsGgGameId game);

/* Returns the number of games currently enabled (0..FS_GG_GAME_ID_COUNT). */
int fs_gesture_gate_enabled_count(void);

/* Sets the active game id (used by translation + audit headers).  The
 * recognizer itself is game-agnostic; only the translation step
 * inspects this value.  Returns the previous active id. */
FsGgGameId fs_gesture_gate_set_active_game(FsGgGameId game);

/* Returns the active game id. */
FsGgGameId fs_gesture_gate_active_game(void);

/* ── Recognizer step ────────────────────────────────────────────────── */
/* Feed one DOWN / MOVE / UP event.  Call this from the platform
 * layer (SDL_FingerEvent adapter or equivalent).  Output goes into
 * `outEvent`; the gesture field is FS_GG_GESTURE_NONE when no
 * gesture was completed on this step.  `outEvent->kind` is one of:
 *   - "DOWN received"   — finger down, recognizer armed
 *   - "MOVE received"   — finger moved, may upgrade to DRAG/PINCH
 *   - "UP completed"    — gesture fired (gesture field is set)
 * Returns 1 on accepted feed, 0 on bad args. */
int fs_gesture_recognizer_step(const FsGestureFeedEvent* ev,
                               FsGestureType* outGesture);

/* Returns the last gesture the recognizer fired, or NONE if none. */
FsGestureType fs_gesture_recognizer_last_gesture(void);

/* Returns the finger-id of the recognizer currently tracking, or -1
 * if no finger is active.  The current gate only tracks one finger. */
int fs_gesture_recognizer_active_finger_id(void);

/* Clear the recognizer state without shutting the gate down.  Use
 * after a settings change, focus loss, or menu transition. */
void fs_gesture_recognizer_reset(void);

/* ── Translation ────────────────────────────────────────────────────── */
/* Translate the last recognized gesture into a per-game movement
 * command record.  When the gate is disabled for the active game
 * the function still runs but `accepted=0` and
 * `reason=FS_GG_DISABLED`.  Use fs_gesture_translate_with_reason to
 * also retrieve the rejection reason. */
FsGestureGameCommand fs_gesture_translate(FsGgGameId game,
                                          FsGestureType gesture);

/* Same as fs_gesture_translate but the caller supplies an out
 * parameter for the dispatch reason (set on accept or reject).
 * Returns 1 on accepted, 0 on rejected (reason still populated). */
int fs_gesture_translate_with_reason(FsGgGameId game,
                                     FsGestureType gesture,
                                     FsGestureGameCommand* outCmd,
                                     FsGestureDispatchReason* outReason);

/* Returns the row count in the per-game gesture → command table
 * (kept stable for tests). */
unsigned int fs_gesture_per_game_table_size(void);

/* ── Touch target safety ────────────────────────────────────────────── */
/* Audit a list of zones against the platform minimum / recommended
 * touch-target sizes.  Returns the number of zones audited (clamped
 * to FS_GG_ZONE_AUDIT_MAX). 0 if zones or out are null. */
int fs_gesture_audit_zones(const FsGestureZone* zones,
                           int zoneCount,
                           int minimumSidePx,
                           int recommendedSidePx,
                           FsGestureZoneAuditReport* out);

/* Default platform targets.  24 px is the Apple HIG/IBM minimum
 * for a touchable UI element; 44 px is the recommended tap target. */
#define FS_GG_PLATFORM_MIN_TARGET_PX     24
#define FS_GG_PLATFORM_RECOMMENDED_PX    44

/* Convenience: audit the built-in per-game touch-zone table that
 * ships with this gate.  Useful for a one-call sanity check. */
int fs_gesture_audit_builtin_zones(FsGestureZoneAuditReport* out);

/* ── Observability ──────────────────────────────────────────────────── */
/* Counts of accept / reject since init (or last reset).  Use
 * fs_gesture_gate_reset_counters() to clear. */
typedef struct {
    uint32_t feedCount;            /* total feeds accepted */
    uint32_t gestureCount;         /* total gestures recognized */
    uint32_t translateAccepted;    /* total translations accepted */
    uint32_t translateRejected;    /* total translations rejected */
    uint32_t auditRuns;            /* total zone audits run */
    uint32_t auditZonesFlagged;    /* sum of audits with meetsMinimum==0 */
} FsGestureGateCounters;

void fs_gesture_gate_reset_counters(void);
FsGestureGateCounters fs_gesture_gate_counters(void);

/* ── Source evidence ────────────────────────────────────────────────── */
const char* fs_gesture_gate_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FS_GESTURE_NAVIGATION_GATE_H */
