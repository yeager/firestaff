/*
 * Cross-game runtime gesture navigation gate for movement/turning.
 *
 * Source lock:
 *   ReDMCSB WIP20210206, Toolchains/Common/Source/
 *   - COMMAND.C:396-405 active mouse table mapping the six movement
 *     arrow zones to C001/C003/C002/C006/C005/C004 (left button only).
 *   - COMMAND.C:1379-1449 F0358 source mouse zone hit-test.
 *   - COMMAND.C:1452-1644 F0359 source click queue primary-to-secondary.
 *   - COMMAND.C:2045-2156 F0380 dispatch to F0365 turns / F0366 movement.
 *   - COMMAND.C:2296-2324 C083 inventory / C111 action / C080 dispatch.
 *   - CLIKMENU.C:142-179 F0365 turn handler.
 *   - CLIKMENU.C:180-347 F0366 movement handler.
 *   - INPUT.C:641-664 raw left/right button forwarding.
 *
 *   firestaff_touch.c (ReDMCSB DEFS.H:238-243 C005/C006 strafe):
 *   - firestaff_touch_detect_swipe + FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX.
 *   - FIRESTAFF_TOUCH_TAP_TOLERANCE_PX / FIRESTAFF_TOUCH_LONG_PRESS_MS.
 *   - firestaff_touch_in_left_edge / right_edge + edge-zone fraction.
 *
 *   firestaff_input.c:
 *   - FS_Command enum (MOVE_FORWARD/BACKWARD/TURN_LEFT/RIGHT/STRAFE_*).
 *   - FS_InputQueue push/pop (FIRESTAFF input pipeline).
 *
 *   src/ui/menu_startup_m12.c (launcher-side settings):
 *   - g_inputModeLabels[]: AUTO, KEYBOARD+MOUSE, TOUCH, GAMEPAD.
 *   - g_touchControlsLabels[]: OFF, MINIMAL, FULL, LARGE.
 *   - M12_SETTINGS_ROW_INPUT_MODE / M12_SETTINGS_ROW_TOUCH_CONTROLS
 *     rows that gate the runtime input mode.
 *
 * Scope:
 *   The gate is data-free, cross-game, and additive - it does NOT add
 *   any new gesture behaviour beyond what firestaff_touch.c /
 *   touch_pointer_input.c already expose. It pins down the route and
 *   the safety contract so future touch affordances can build on top
 *   without regressing the basic movement/turning path.
 */

#include "runtime_gesture_navigation_gate_pc34_compat.h"

#include <string.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "touch_click_zone_matrix_pc34_compat.h"

/* Canonical command-id/FS_Command table for the four navigation
 * directions. Order is source-locked: forward/back are the Y-axis
 * swipes; left/right are the X-axis swipes. We keep the DM1 V1
 * command IDs from ReDMCSB DEFS so the gate is engine-agnostic. */
typedef struct GestureNavDirectionBindingPc34Compat {
    const char*             label;
    unsigned int            dm1CommandId;   /* C001..C006 family */
    FS_Command              fsCommand;
    int                     axisIsVertical; /* 1 = dy dominant, 0 = dx */
    int                     positiveSign;  /* +1 -> forward/right, -1 -> back/left */
} GestureNavDirectionBindingPc34Compat;

static const GestureNavDirectionBindingPc34Compat kBindings[4] = {
    /* Index 0: TURN_LEFT - horizontal swipe with negative dx */
    { "turn_left",  DM1_V1_COMMAND_TURN_LEFT,     FS_CMD_TURN_LEFT,     0, -1 },
    /* Index 1: MOVE_FORWARD - vertical swipe with negative dy */
    { "forward",    DM1_V1_COMMAND_MOVE_FORWARD,  FS_CMD_MOVE_FORWARD,  1, -1 },
    /* Index 2: TURN_RIGHT - horizontal swipe with positive dx */
    { "turn_right", DM1_V1_COMMAND_TURN_RIGHT,    FS_CMD_TURN_RIGHT,    0, +1 },
    /* Index 3: MOVE_BACKWARD - vertical swipe with positive dy */
    { "backward",   DM1_V1_COMMAND_MOVE_BACKWARD, FS_CMD_MOVE_BACKWARD, 1, +1 }
};

static void reset_target(GestureNavDirectionTargetPc34Compat* target) {
    if (!target) return;
    memset(target, 0, sizeof(*target));
    target->safety = GESTURE_NAV_TARGET_SAFETY_UNKNOWN_PC34_COMPAT;
}

void GESTURENAV_Compat_ResetSnapshotPc34Compat(GestureNavSnapshotPc34Compat* snapshot) {
    int i;
    if (!snapshot) return;
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->inputMode         = GESTURE_NAV_INPUT_MODE_AUTO_PC34_COMPAT;
    snapshot->touchLevel        = GESTURE_NAV_TOUCH_LEVEL_OFF_PC34_COMPAT;
    snapshot->gameId            = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    snapshot->swipeThresholdPx  = FIRESTAFF_GESTURE_NAV_DEFAULT_SWIPE_PX;
    snapshot->tapTolerancePx    = FIRESTAFF_GESTURE_NAV_DEFAULT_TAP_PX;
    snapshot->longPressMs       = FIRESTAFF_GESTURE_NAV_DEFAULT_LONG_MS;
    snapshot->edgeZonePercent   = FIRESTAFF_GESTURE_NAV_EDGE_ZONE_PERCENT;
    for (i = 0; i < 4; ++i) reset_target(&snapshot->targets[i]);
}

int GESTURENAV_Compat_ApplyConfigPc34Compat(const GestureNavConfigPc34Compat* config,
                                            GestureNavSnapshotPc34Compat* snapshot) {
    if (!config || !snapshot) return 0;
    GESTURENAV_Compat_ResetSnapshotPc34Compat(snapshot);

    snapshot->inputMode  = config->inputMode;
    snapshot->touchLevel = config->touchLevel;
    snapshot->gameId     = config->gameId;

    /* Gate activation rule (kept simple and launcher-aligned):
     *   touchLevel >= MINIMAL
     *   inputMode == AUTO || inputMode == TOUCH
     * KEYBOARD+MOUSE-only and GAMEPAD-only both disable the gate - their
     * own paths still work through the standard mouse / controller
     * command queues. */
    snapshot->gateActive = 0;
    if (snapshot->touchLevel >= GESTURE_NAV_TOUCH_LEVEL_MINIMAL_PC34_COMPAT) {
        if (snapshot->inputMode == GESTURE_NAV_INPUT_MODE_AUTO_PC34_COMPAT ||
            snapshot->inputMode == GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT) {
            snapshot->gateActive = 1;
        }
    }

    if (!config->enableEdgeStrafe) snapshot->edgeZonePercent = 0;
    return snapshot->gateActive;
}

int GESTURENAV_Compat_IsTargetSafePc34Compat(int widthPx, int heightPx) {
    if (widthPx <= 0 || heightPx <= 0) return 0;
    return (widthPx >= FIRESTAFF_GESTURE_NAV_MIN_TARGET_PX &&
            heightPx >= FIRESTAFF_GESTURE_NAV_MIN_TARGET_PX) ? 1 : 0;
}

int GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                                   int startX) {
    int fbW = FIRESTAFF_GESTURE_NAV_DEFAULT_FB_W;
    int edgePx;
    if (!snapshot) return 0;
    if (!snapshot->gateActive) return 0;
    if (snapshot->edgeZonePercent <= 0) return 0;
    edgePx = (fbW * snapshot->edgeZonePercent) / 100;
    if (edgePx <= 0) return 0;
    return (startX < edgePx || startX >= (fbW - edgePx)) ? 1 : 0;
}

int GESTURENAV_Compat_ClassifyTouchTargetsPc34Compat(GestureNavSnapshotPc34Compat* snapshot) {
    unsigned int zoneCount;
    unsigned int i;
    int safeCount = 0;
    int underCount = 0;
    int oobCount = 0;
    if (!snapshot) return 0;

    /* Reset target rows in this snapshot. */
    for (i = 0; i < 4; ++i) reset_target(&snapshot->targets[i]);

    if (!snapshot->gateActive) {
        for (i = 0; i < 4; ++i) {
            snapshot->targets[i].directionOrdinal = (int)i;
            snapshot->targets[i].commandId       = kBindings[i].dm1CommandId;
            snapshot->targets[i].fsCommand        = kBindings[i].fsCommand;
            snapshot->targets[i].label            = kBindings[i].label;
            snapshot->targets[i].safety           = GESTURE_NAV_TARGET_SAFETY_INPUT_DISABLED_PC34_COMPAT;
        }
        snapshot->targetSafeCount        = 0;
        snapshot->targetUnderCount       = 0;
        snapshot->targetDisabledCount    = 4;
        snapshot->targetOutOfBoundsCount = 0;
        return 0;
    }

    zoneCount = TOUCHCLICK_Compat_GetZoneCount();
    for (i = 0; i < 4; ++i) {
        GestureNavDirectionTargetPc34Compat* target = &snapshot->targets[i];
        unsigned int z;
        int found = 0;
        target->directionOrdinal = (int)i;
        target->commandId        = kBindings[i].dm1CommandId;
        target->fsCommand         = kBindings[i].fsCommand;
        target->label             = kBindings[i].label;

        for (z = 0; z < zoneCount; ++z) {
            TouchClickZonePc34Compat zone;
            if (!TOUCHCLICK_Compat_GetZone(z, &zone)) continue;
            if (zone.commandId != kBindings[i].dm1CommandId) continue;
            target->zone      = zone;
            target->zoneFound = 1;
            found = 1;
            break;
        }

        if (!found) {
            /* No zone registered for this direction in the matrix.
             * Classify as out-of-bounds: the gesture has no source
             * target to hit-test against. The gate still considers
             * the input safe (it can't accidentally hit a neighbouring
             * target), but it must report OOB so the launcher can
             * surface a configuration warning. */
            target->safety = GESTURE_NAV_TARGET_SAFETY_OUT_OF_BOUNDS_PC34_COMPAT;
            ++oobCount;
            continue;
        }

        /* Framebuffer bounds. The touch_click_zone_matrix records
         * zones in 320x200 framebuffer coordinates (TOUCH_CLICK_COORD_
         * SCREEN_RELATIVE_PC34_COMPAT). */
        if (target->zone.x < 0 || target->zone.y < 0 ||
            target->zone.x + target->zone.w > FIRESTAFF_GESTURE_NAV_DEFAULT_FB_W ||
            target->zone.y + target->zone.h > FIRESTAFF_GESTURE_NAV_DEFAULT_FB_H) {
            target->safety = GESTURE_NAV_TARGET_SAFETY_OUT_OF_BOUNDS_PC34_COMPAT;
            ++oobCount;
            continue;
        }

        if (GESTURENAV_Compat_IsTargetSafePc34Compat(target->zone.w, target->zone.h)) {
            target->safety = GESTURE_NAV_TARGET_SAFETY_SAFE_PC34_COMPAT;
            ++safeCount;
        } else {
            target->safety = GESTURE_NAV_TARGET_SAFETY_UNDER_THRESHOLD_PC34_COMPAT;
            ++underCount;
        }
    }

    snapshot->targetSafeCount        = safeCount;
    snapshot->targetUnderCount       = underCount;
    snapshot->targetDisabledCount    = 0;
    snapshot->targetOutOfBoundsCount = oobCount;
    return safeCount;
}

FS_Command GESTURENAV_Compat_ResolveSwipePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                                    int startX, int startY,
                                                    int endX, int endY,
                                                    int* outDirection) {
    int dx, dy, absDx, absDy;
    int idx = -1;
    if (outDirection) *outDirection = -1;
    if (!snapshot || !snapshot->gateActive) return FS_CMD_NONE;
    if (startX == endX && startY == endY) return FS_CMD_NONE;

    dx    = endX - startX;
    dy    = endY - startY;
    absDx = dx < 0 ? -dx : dx;
    absDy = dy < 0 ? -dy : dy;
    if (absDx < snapshot->swipeThresholdPx && absDy < snapshot->swipeThresholdPx) {
        return FS_CMD_NONE; /* Below threshold: not a swipe. */
    }

    if (absDx > absDy) {
        idx = (dx > 0) ? 2 : 0; /* right=2, left=0 */
    } else {
        idx = (dy > 0) ? 3 : 1; /* back=3, forward=1 */
    }

    if (idx < 0 || idx >= 4) return FS_CMD_NONE;
    if (outDirection) *outDirection = idx;
    return kBindings[idx].fsCommand;
}

int GESTURENAV_Compat_ResolveTapPc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                           int screenX, int screenY,
                                           unsigned int buttonMask,
                                           TouchPointerDispatchPc34Compat* outDispatch) {
    TouchPointerEventPc34Compat event;
    if (!snapshot || !snapshot->gateActive) return 0;
    if (buttonMask == 0u) return 0;

    memset(&event, 0, sizeof(event));
    event.action    = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space     = TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT;
    event.x         = screenX;
    event.y         = screenY;
    event.surfaceW  = 0;
    event.surfaceH  = 0;
    event.buttonMask = buttonMask;
    return TOUCHPOINTER_Compat_TranslateEvent(&event, outDispatch);
}

int GESTURENAV_Compat_EnqueueSwipePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                             FS_InputQueue* queue,
                                             int startX, int startY,
                                             int endX, int endY) {
    FS_Command cmd;
    if (!queue) return 0;
    cmd = GESTURENAV_Compat_ResolveSwipePc34Compat(snapshot, startX, startY, endX, endY, NULL);
    if (cmd == FS_CMD_NONE) return 0;
    fs_input_queue_push(queue, cmd, 0, 0);
    return 1;
}

int GESTURENAV_Compat_EnqueueEdgeStrafePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                                  FS_InputQueue* queue,
                                                  int startX, int framebufferW) {
    int fbW = framebufferW > 0 ? framebufferW : FIRESTAFF_GESTURE_NAV_DEFAULT_FB_W;
    int edgePx;
    FS_Command cmd = FS_CMD_NONE;
    if (!snapshot || !queue) return 0;
    if (!snapshot->gateActive) return 0;
    if (snapshot->edgeZonePercent <= 0) return 0;

    edgePx = (fbW * snapshot->edgeZonePercent) / 100;
    if (edgePx <= 0) return 0;
    if (startX < edgePx) {
        cmd = FS_CMD_STRAFE_LEFT;
    } else if (startX >= (fbW - edgePx)) {
        cmd = FS_CMD_STRAFE_RIGHT;
    }
    if (cmd == FS_CMD_NONE) return 0;
    fs_input_queue_push(queue, cmd, 0, 0);
    return 1;
}

int GESTURENAV_Compat_VerifyCrossGameActivationPc34Compat(
    const GestureNavConfigPc34Compat* baseConfig,
    int perGameActivation[5]) {
    int activated = 0;
    int i;
    if (!baseConfig || !perGameActivation) return 0;
    for (i = 0; i < 5; ++i) {
        GestureNavConfigPc34Compat cfg = *baseConfig;
        GestureNavSnapshotPc34Compat snap;
        cfg.gameId = (GestureNavGameIdPc34Compat)i;
        perGameActivation[i] = GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
        if (perGameActivation[i]) ++activated;
    }
    return activated;
}

const char* GESTURENAV_Compat_GetSourceEvidence(void) {
    return "ReDMCSB COMMAND.C:396-405 movement arrow mouse table (C001/C003/C002/C006/C005/C004); "
           "COMMAND.C:1379-1449 F0358 source mouse hit-test; COMMAND.C:1452-1644 F0359 click queue; "
           "COMMAND.C:2045-2156 F0380 queue dispatch to F0365 turns + F0366 movement; "
           "COMMAND.C:2296-2324 C083/C111/C080 dispatch; CLIKMENU.C:142-179 F0365 turn handling; "
           "CLIKMENU.C:180-347 F0366 movement handling; INPUT.C:641-664 raw button forwarding; "
           "DEFS.H:238-243 C005/C006 strafe commands; "
           "firestaff_touch.c FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX / TAP_TOLERANCE_PX / LONG_PRESS_MS / "
           "FIRESTAFF_TOUCH_EDGE_ZONE_FRAC; "
           "firestaff_input.c FS_Command enum and FS_InputQueue push/pop; "
           "src/ui/menu_startup_m12.c g_inputModeLabels[] AUTO/KEYBOARD+MOUSE/TOUCH/GAMEPAD and "
           "g_touchControlsLabels[] OFF/MINIMAL/FULL/LARGE gate the runtime input pipeline.";
}
