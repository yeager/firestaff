/*
 * Unit test for the cross-game runtime gesture navigation gate.
 *
 * Data-free: no game data is read. Uses only the existing
 * touch_pointer_input / touch_click_zone_matrix / fs_input_queue
 * surfaces that the gate composes on top of.
 *
 * Source lock for the gate invariants:
 *   - firestaff_touch.c swipe detection (40px threshold by default).
 *   - firestaff_input.c FS_Command enum + queue push/pop.
 *   - src/ui/menu_startup_m12.c inputModeIndex + touchControlsIndex.
 *   - touch_click_zone_matrix_pc34_compat.c movement.turn_left etc.
 *   - ReDMCSB COMMAND.C:396-405 movement arrow table.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "runtime_gesture_navigation_gate_pc34_compat.h"
#include "touch_click_zone_matrix_pc34_compat.h"
#include "touch_pointer_input_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define EXPECT_INT(label, actual, expected) do {                              \
    int _a = (int)(actual);                                                   \
    int _e = (int)(expected);                                                 \
    if (_a == _e) { ++g_pass; }                                               \
    else { ++g_fail; printf("FAIL %s actual=%d expected=%d\n", label, _a, _e); } \
} while (0)

#define EXPECT_UINT(label, actual, expected) do {                             \
    unsigned int _a = (unsigned int)(actual);                                 \
    unsigned int _e = (unsigned int)(expected);                               \
    if (_a == _e) { ++g_pass; }                                               \
    else { ++g_fail; printf("FAIL %s actual=%u expected=%u\n", label, _a, _e); } \
} while (0)

#define EXPECT_NOT_ZERO(label, value) do {                                    \
    if ((value) != 0) { ++g_pass; }                                           \
    else { ++g_fail; printf("FAIL %s expected non-zero\n", label); }          \
} while (0)

#define EXPECT_ZERO(label, value) do {                                        \
    if ((value) == 0) { ++g_pass; }                                           \
    else { ++g_fail; printf("FAIL %s expected zero, got %d\n", label, (int)(value)); } \
} while (0)

static int run_settings_gate(void) {
    /* touch OFF disables the gate. */
    {
        GestureNavConfigPc34Compat cfg;
        GestureNavSnapshotPc34Compat snap;
        memset(&cfg, 0, sizeof(cfg));
        cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
        cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_OFF_PC34_COMPAT;
        cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
        EXPECT_INT("settings.touchOff.disables", GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 0);
        EXPECT_ZERO("settings.touchOff.gateActive", snap.gateActive);
    }
    /* AUTO + MINIMAL activates the gate. */
    {
        GestureNavConfigPc34Compat cfg;
        GestureNavSnapshotPc34Compat snap;
        memset(&cfg, 0, sizeof(cfg));
        cfg.inputMode = GESTURE_NAV_INPUT_MODE_AUTO_PC34_COMPAT;
        cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_MINIMAL_PC34_COMPAT;
        cfg.gameId = GESTURE_NAV_GAME_CSB_PC34_COMPAT;
        EXPECT_INT("settings.autoMinimal.activates", GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 1);
        EXPECT_NOT_ZERO("settings.autoMinimal.gateActive", snap.gateActive);
    }
    /* TOUCH + FULL activates the gate. */
    {
        GestureNavConfigPc34Compat cfg;
        GestureNavSnapshotPc34Compat snap;
        memset(&cfg, 0, sizeof(cfg));
        cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
        cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
        cfg.gameId = GESTURE_NAV_GAME_DM2_PC34_COMPAT;
        EXPECT_INT("settings.touchFull.activates", GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 1);
    }
    /* TOUCH + LARGE activates the gate. */
    {
        GestureNavConfigPc34Compat cfg;
        GestureNavSnapshotPc34Compat snap;
        memset(&cfg, 0, sizeof(cfg));
        cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
        cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_LARGE_PC34_COMPAT;
        cfg.gameId = GESTURE_NAV_GAME_NEXUS_PC34_COMPAT;
        EXPECT_INT("settings.touchLarge.activates", GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 1);
    }
    /* KEYBOARD+MOUSE-only disables the gate. */
    {
        GestureNavConfigPc34Compat cfg;
        GestureNavSnapshotPc34Compat snap;
        memset(&cfg, 0, sizeof(cfg));
        cfg.inputMode = GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT;
        cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
        cfg.gameId = GESTURE_NAV_GAME_THERON_PC34_COMPAT;
        EXPECT_INT("settings.kbOnly.disables", GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 0);
    }
    /* GAMEPAD-only disables the gate (controller has its own path). */
    {
        GestureNavConfigPc34Compat cfg;
        GestureNavSnapshotPc34Compat snap;
        memset(&cfg, 0, sizeof(cfg));
        cfg.inputMode = GESTURE_NAV_INPUT_MODE_GAMEPAD_PC34_COMPAT;
        cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
        cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
        EXPECT_INT("settings.gamepadOnly.disables", GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 0);
    }
    return 1;
}

static int run_swipe_directions(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    int dir = -1;

    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    /* forward: start (160, 180) -> end (160, 100). dy = -80, |dy| > |dx|. */
    EXPECT_UINT("swipe.forward",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 180, 160, 100, &dir),
                (unsigned int)FS_CMD_MOVE_FORWARD);
    EXPECT_INT("swipe.forward.dir", dir, 1);

    /* backward: dy positive. */
    EXPECT_UINT("swipe.backward",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 100, 160, 180, &dir),
                (unsigned int)FS_CMD_MOVE_BACKWARD);
    EXPECT_INT("swipe.backward.dir", dir, 3);

    /* turn left: dx negative, |dx| > |dy|. */
    EXPECT_UINT("swipe.turnLeft",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 220, 100, 100, 100, &dir),
                (unsigned int)FS_CMD_TURN_LEFT);
    EXPECT_INT("swipe.turnLeft.dir", dir, 0);

    /* turn right: dx positive, |dx| > |dy|. */
    EXPECT_UINT("swipe.turnRight",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 100, &dir),
                (unsigned int)FS_CMD_TURN_RIGHT);
    EXPECT_INT("swipe.turnRight.dir", dir, 2);

    /* Diagonal: dx and dy both above threshold. Dominant axis wins. */
    {
        int dir2 = -1;
        FS_Command c = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 130, &dir2);
        /* |dx|=120, |dy|=30 -> dx wins -> turn right. */
        EXPECT_UINT("swipe.diagonal.dominantDx", (unsigned int)c, (unsigned int)FS_CMD_TURN_RIGHT);
        EXPECT_INT("swipe.diagonal.dominantDx.dir", dir2, 2);
    }
    {
        int dir2 = -1;
        FS_Command c = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 130, 220, &dir2);
        /* |dx|=30, |dy|=120 -> dy wins -> backward. */
        EXPECT_UINT("swipe.diagonal.dominantDy", (unsigned int)c, (unsigned int)FS_CMD_MOVE_BACKWARD);
        EXPECT_INT("swipe.diagonal.dominantDy.dir", dir2, 3);
    }
    /* Equal magnitudes: dy wins (tie -> vertical). */
    {
        int dir2 = -1;
        FS_Command c = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 200, 200, &dir2);
        /* |dx|=|dy|=100 -> code path picks dy branch (absDx > absDy is false). */
        EXPECT_UINT("swipe.diagonal.tieDy", (unsigned int)c, (unsigned int)FS_CMD_MOVE_BACKWARD);
        EXPECT_INT("swipe.diagonal.tieDy.dir", dir2, 3);
    }

    /* Below threshold: no command. */
    EXPECT_UINT("swipe.belowThreshold",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 110, 110, &dir),
                (unsigned int)FS_CMD_NONE);
    EXPECT_INT("swipe.belowThreshold.dir", dir, -1);

    /* Zero-length path: no command. */
    EXPECT_UINT("swipe.zeroLength",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 100, 100, &dir),
                (unsigned int)FS_CMD_NONE);

    return 1;
}

static int run_gate_inactive(void) {
    /* Gate is OFF under KEYBOARD+MOUSE-only. Swipe should be FS_CMD_NONE. */
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
    EXPECT_UINT("gate.off.swipe",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 180, 160, 100, NULL),
                (unsigned int)FS_CMD_NONE);

    /* Touch OFF, TOUCH-mode input. */
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_OFF_PC34_COMPAT;
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
    EXPECT_UINT("gate.touchOff.swipe",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 180, 160, 100, NULL),
                (unsigned int)FS_CMD_NONE);
    return 1;
}

static int run_queue_integration(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    FS_InputQueue q;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    /* forward swipe -> queue contains FS_CMD_MOVE_FORWARD. */
    fs_input_queue_init(&q);
    EXPECT_INT("queue.enqueue.forward",
               GESTURENAV_Compat_EnqueueSwipePc34Compat(&snap, &q, 160, 180, 160, 100), 1);
    EXPECT_INT("queue.count.forward", fs_input_queue_count(&q), 1);
    {
        FS_InputEvent evt;
        EXPECT_INT("queue.pop.forward", fs_input_queue_pop(&q, &evt), 1);
        EXPECT_UINT("queue.pop.forward.cmd", (unsigned int)evt.cmd, (unsigned int)FS_CMD_MOVE_FORWARD);
    }

    /* turn right swipe -> queue contains FS_CMD_TURN_RIGHT. */
    fs_input_queue_init(&q);
    EXPECT_INT("queue.enqueue.turnRight",
               GESTURENAV_Compat_EnqueueSwipePc34Compat(&snap, &q, 100, 100, 220, 100), 1);
    {
        FS_InputEvent evt;
        EXPECT_INT("queue.pop.turnRight", fs_input_queue_pop(&q, &evt), 1);
        EXPECT_UINT("queue.pop.turnRight.cmd", (unsigned int)evt.cmd, (unsigned int)FS_CMD_TURN_RIGHT);
    }

    /* Below-threshold swipe does NOT enqueue. */
    fs_input_queue_init(&q);
    EXPECT_ZERO("queue.enqueue.belowThreshold",
                GESTURENAV_Compat_EnqueueSwipePc34Compat(&snap, &q, 100, 100, 110, 110));
    EXPECT_ZERO("queue.count.belowThreshold", fs_input_queue_count(&q));

    /* Determinism: same swipe twice produces the same FS_Command. */
    {
        FS_Command a = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 100, NULL);
        FS_Command b = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 100, NULL);
        EXPECT_UINT("determinism.turnRight", (unsigned int)a, (unsigned int)b);
    }
    return 1;
}

static int run_touch_target_safety(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    int safeCount;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
    safeCount = GESTURENAV_Compat_ClassifyTouchTargetsPc34Compat(&snap);
    /* The four canonical movement/turning zones are 27-28 wide and 21
     * tall in the matrix, all >= MIN_TARGET_PX (16) on both axes. */
    EXPECT_INT("touchTarget.allSafe", safeCount, 4);
    EXPECT_INT("touchTarget.targetUnderCount.zero", snap.targetUnderCount, 0);
    EXPECT_INT("touchTarget.targetDisabledCount.zero", snap.targetDisabledCount, 0);
    EXPECT_INT("touchTarget.targetOutOfBoundsCount.zero", snap.targetOutOfBoundsCount, 0);
    /* Each direction's fsCommand matches the binding. */
    EXPECT_UINT("touchTarget.turnLeft",
                (unsigned int)snap.targets[0].fsCommand,
                (unsigned int)FS_CMD_TURN_LEFT);
    EXPECT_UINT("touchTarget.forward",
                (unsigned int)snap.targets[1].fsCommand,
                (unsigned int)FS_CMD_MOVE_FORWARD);
    EXPECT_UINT("touchTarget.turnRight",
                (unsigned int)snap.targets[2].fsCommand,
                (unsigned int)FS_CMD_TURN_RIGHT);
    EXPECT_UINT("touchTarget.backward",
                (unsigned int)snap.targets[3].fsCommand,
                (unsigned int)FS_CMD_MOVE_BACKWARD);
    {
        int i;
        for (i = 0; i < 4; ++i) {
            EXPECT_INT("touchTarget.zoneFound", snap.targets[i].zoneFound, 1);
            EXPECT_INT("touchTarget.safety",
                       snap.targets[i].safety,
                       GESTURE_NAV_TARGET_SAFETY_SAFE_PC34_COMPAT);
        }
    }
    return 1;
}

static int run_safety_predicate(void) {
    /* 16x16 = threshold. */
    EXPECT_INT("safety.atThreshold.16x16",
               GESTURENAV_Compat_IsTargetSafePc34Compat(16, 16), 1);
    /* 24x24 = recommended. */
    EXPECT_INT("safety.recommended.24x24",
               GESTURENAV_Compat_IsTargetSafePc34Compat(24, 24), 1);
    /* Below threshold. */
    EXPECT_ZERO("safety.belowThreshold.15x16",
                GESTURENAV_Compat_IsTargetSafePc34Compat(15, 16));
    EXPECT_ZERO("safety.belowThreshold.16x15",
                GESTURENAV_Compat_IsTargetSafePc34Compat(16, 15));
    /* Zero / negative. */
    EXPECT_ZERO("safety.zeroWidth", GESTURENAV_Compat_IsTargetSafePc34Compat(0, 24));
    EXPECT_ZERO("safety.zeroHeight", GESTURENAV_Compat_IsTargetSafePc34Compat(24, 0));
    EXPECT_ZERO("safety.negative", GESTURENAV_Compat_IsTargetSafePc34Compat(-1, 24));
    return 1;
}

static int run_cross_game_parity(void) {
    GestureNavConfigPc34Compat cfg;
    int perGame[5] = { -1, -1, -1, -1, -1 };
    int activated;
    int i;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    activated = GESTURENAV_Compat_VerifyCrossGameActivationPc34Compat(&cfg, perGame);
    /* All 5 games should activate identically under the same config. */
    EXPECT_INT("crossGame.activated", activated, 5);
    for (i = 0; i < 5; ++i) {
        EXPECT_INT("crossGame.perGame", perGame[i], 1);
    }
    /* Same config but KB-only should disable ALL games. */
    {
        GestureNavConfigPc34Compat cfg2 = cfg;
        int perGame2[5] = { -1, -1, -1, -1, -1 };
        int activated2;
        cfg2.inputMode = GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT;
        activated2 = GESTURENAV_Compat_VerifyCrossGameActivationPc34Compat(&cfg2, perGame2);
        EXPECT_INT("crossGame.kbOnly.activated", activated2, 0);
        for (i = 0; i < 5; ++i) {
            EXPECT_INT("crossGame.kbOnly.perGame", perGame2[i], 0);
        }
    }
    /* Same config but touch OFF should disable ALL games. */
    {
        GestureNavConfigPc34Compat cfg3 = cfg;
        int perGame3[5] = { -1, -1, -1, -1, -1 };
        int activated3;
        cfg3.touchLevel = GESTURE_NAV_TOUCH_LEVEL_OFF_PC34_COMPAT;
        activated3 = GESTURENAV_Compat_VerifyCrossGameActivationPc34Compat(&cfg3, perGame3);
        EXPECT_INT("crossGame.touchOff.activated", activated3, 0);
        for (i = 0; i < 5; ++i) {
            EXPECT_INT("crossGame.touchOff.perGame", perGame3[i], 0);
        }
    }
    return 1;
}

static int run_edge_strafe(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    FS_InputQueue q;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    cfg.enableEdgeStrafe = 1;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    /* Left edge at fb width 320, edge 20% = 64px. */
    EXPECT_INT("edgeStrafe.leftZone",
               GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 32), 1);
    /* Center is NOT in edge. */
    EXPECT_ZERO("edgeStrafe.center",
                GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 160));
    /* Right edge. */
    EXPECT_INT("edgeStrafe.rightZone",
               GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 300), 1);

    /* Enqueue: left start emits FS_CMD_STRAFE_LEFT. */
    fs_input_queue_init(&q);
    EXPECT_INT("edgeStrafe.enqueue.left",
               GESTURENAV_Compat_EnqueueEdgeStrafePc34Compat(&snap, &q, 32, 320), 1);
    {
        FS_InputEvent evt;
        EXPECT_INT("edgeStrafe.pop.left", fs_input_queue_pop(&q, &evt), 1);
        EXPECT_UINT("edgeStrafe.pop.left.cmd",
                    (unsigned int)evt.cmd,
                    (unsigned int)FS_CMD_STRAFE_LEFT);
    }

    /* Enqueue: right start emits FS_CMD_STRAFE_RIGHT. */
    fs_input_queue_init(&q);
    EXPECT_INT("edgeStrafe.enqueue.right",
               GESTURENAV_Compat_EnqueueEdgeStrafePc34Compat(&snap, &q, 300, 320), 1);
    {
        FS_InputEvent evt;
        EXPECT_INT("edgeStrafe.pop.right", fs_input_queue_pop(&q, &evt), 1);
        EXPECT_UINT("edgeStrafe.pop.right.cmd",
                    (unsigned int)evt.cmd,
                    (unsigned int)FS_CMD_STRAFE_RIGHT);
    }

    /* Center emits nothing. */
    fs_input_queue_init(&q);
    EXPECT_ZERO("edgeStrafe.enqueue.center",
                GESTURENAV_Compat_EnqueueEdgeStrafePc34Compat(&snap, &q, 160, 320));
    EXPECT_ZERO("edgeStrafe.count.center", fs_input_queue_count(&q));

    /* V2 disabled: edge strafe becomes no-op. */
    {
        GestureNavConfigPc34Compat cfg2 = cfg;
        GestureNavSnapshotPc34Compat snap2;
        cfg2.enableEdgeStrafe = 0;
        GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg2, &snap2);
        EXPECT_ZERO("edgeStrafe.v2Disabled.left",
                    GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap2, 32));
        fs_input_queue_init(&q);
        EXPECT_ZERO("edgeStrafe.v2Disabled.enqueue.left",
                    GESTURENAV_Compat_EnqueueEdgeStrafePc34Compat(&snap2, &q, 32, 320));
    }
    return 1;
}

static int run_tap_resolution(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    TouchPointerDispatchPc34Compat dispatch;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode  = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    /* Tap on the center of the movement.forward zone (263, 125). */
    EXPECT_INT("tap.forward.dispatches",
               GESTURENAV_Compat_ResolveTapPc34Compat(&snap, 275, 135,
                                                      TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                      &dispatch),
               1);
    EXPECT_UINT("tap.forward.commandId",
                dispatch.commandId,
                DM1_V1_COMMAND_MOVE_FORWARD);
    EXPECT_INT("tap.forward.shouldDispatchClick", dispatch.shouldDispatchClick, 1);

    /* Tap off the matrix returns 0 (no zone hit).
     * (319, 199) is the bottom-right corner: no registered SCREEN_RELATIVE
     * LEFT-button zone covers it (viewport.dungeon ends at x=223, HUD ends
     * at y=29, movement ends at y=168, freeze_game is x=0..1). */
    EXPECT_ZERO("tap.miss",
                GESTURENAV_Compat_ResolveTapPc34Compat(&snap, 319, 199,
                                                       TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                       &dispatch));

    /* Gate OFF: tap returns 0 regardless. */
    {
        GestureNavConfigPc34Compat cfg2;
        GestureNavSnapshotPc34Compat snap2;
        memset(&cfg2, 0, sizeof(cfg2));
        cfg2.inputMode  = GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT;
        cfg2.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
        cfg2.gameId     = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
        GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg2, &snap2);
        EXPECT_ZERO("tap.gateOff",
                    GESTURENAV_Compat_ResolveTapPc34Compat(&snap2, 275, 135,
                                                           TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                           &dispatch));
    }

    /* Zero button mask: 0. */
    EXPECT_ZERO("tap.zeroButtonMask",
                GESTURENAV_Compat_ResolveTapPc34Compat(&snap, 275, 135, 0u, &dispatch));
    return 1;
}

int main(void) {
    printf("probe=runtime_gesture_navigation_gate_pc34_compat\n");
    printf("sourceEvidence=%s\n", GESTURENAV_Compat_GetSourceEvidence());

    run_settings_gate();
    run_swipe_directions();
    run_gate_inactive();
    run_queue_integration();
    run_touch_target_safety();
    run_safety_predicate();
    run_cross_game_parity();
    run_edge_strafe();
    run_tap_resolution();

    printf("runtimeGestureNavigationGatePass=%u\n", g_pass);
    printf("runtimeGestureNavigationGateFail=%u\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
