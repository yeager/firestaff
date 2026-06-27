/*
 * Headless probe for the cross-game runtime gesture navigation gate.
 *
 * Mirror of tests/test_runtime_gesture_navigation_gate_pc34_compat.c
 * but exposed as a probe binary for the pool/heartbeat automation.
 *
 * The probe is data-free: it does not require any game data on disk.
 *
 * Source lock: see header + tests/test_runtime_gesture_navigation_gate_pc34_compat.c.
 */

#include <stdio.h>
#include <string.h>

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

static int probe_settings(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;

    /* TOUCH-mode input but touchLevel OFF -> gate disabled. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_OFF_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    EXPECT_INT("settings.touchOff.disables",
               GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 0);

    /* AUTO + MINIMAL activates. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_AUTO_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_MINIMAL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    EXPECT_INT("settings.autoMinimal.activates",
               GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 1);
    EXPECT_NOT_ZERO("settings.autoMinimal.gateActive", snap.gateActive);

    /* TOUCH + FULL activates. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    EXPECT_INT("settings.touchFull.activates",
               GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 1);

    /* KB-only + FULL disables. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    EXPECT_INT("settings.kbOnly.disables",
               GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 0);

    /* Gamepad-only + FULL disables. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_GAMEPAD_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    EXPECT_INT("settings.gamepadOnly.disables",
               GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap), 0);

    return 1;
}

static int probe_swipe_routes(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    EXPECT_UINT("swipe.forward",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 180, 160, 100, NULL),
                (unsigned int)FS_CMD_MOVE_FORWARD);
    EXPECT_UINT("swipe.backward",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 100, 160, 180, NULL),
                (unsigned int)FS_CMD_MOVE_BACKWARD);
    EXPECT_UINT("swipe.turnLeft",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 220, 100, 100, 100, NULL),
                (unsigned int)FS_CMD_TURN_LEFT);
    EXPECT_UINT("swipe.turnRight",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 100, NULL),
                (unsigned int)FS_CMD_TURN_RIGHT);

    /* Below threshold: no command. */
    EXPECT_UINT("swipe.belowThreshold",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 110, 110, NULL),
                (unsigned int)FS_CMD_NONE);

    /* Determinism. */
    {
        FS_Command a = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 100, NULL);
        FS_Command b = GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 100, 100, 220, 100, NULL);
        EXPECT_UINT("swipe.deterministic", (unsigned int)a, (unsigned int)b);
        EXPECT_UINT("swipe.deterministic.value", (unsigned int)a, (unsigned int)FS_CMD_TURN_RIGHT);
    }
    return 1;
}

static int probe_gate_inactive(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
    EXPECT_UINT("gate.inactive.swipe",
                (unsigned int)GESTURENAV_Compat_ResolveSwipePc34Compat(&snap, 160, 180, 160, 100, NULL),
                (unsigned int)FS_CMD_NONE);
    EXPECT_ZERO("gate.inactive.edge",
                GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 32));
    return 1;
}

static int probe_queue_integration(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    FS_InputQueue q;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    fs_input_queue_init(&q);
    EXPECT_INT("queue.enqueue.forward",
               GESTURENAV_Compat_EnqueueSwipePc34Compat(&snap, &q, 160, 180, 160, 100), 1);
    EXPECT_INT("queue.count.forward", fs_input_queue_count(&q), 1);
    {
        FS_InputEvent evt;
        EXPECT_INT("queue.pop.forward", fs_input_queue_pop(&q, &evt), 1);
        EXPECT_UINT("queue.pop.forward.cmd",
                    (unsigned int)evt.cmd,
                    (unsigned int)FS_CMD_MOVE_FORWARD);
    }

    fs_input_queue_init(&q);
    EXPECT_INT("queue.enqueue.turnLeft",
               GESTURENAV_Compat_EnqueueSwipePc34Compat(&snap, &q, 220, 100, 100, 100), 1);
    {
        FS_InputEvent evt;
        EXPECT_INT("queue.pop.turnLeft", fs_input_queue_pop(&q, &evt), 1);
        EXPECT_UINT("queue.pop.turnLeft.cmd",
                    (unsigned int)evt.cmd,
                    (unsigned int)FS_CMD_TURN_LEFT);
    }
    return 1;
}

static int probe_touch_targets(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    int safeCount;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.gameId = GESTURE_NAV_GAME_DM1_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
    safeCount = GESTURENAV_Compat_ClassifyTouchTargetsPc34Compat(&snap);
    EXPECT_INT("touchTarget.allSafe", safeCount, 4);
    EXPECT_INT("touchTarget.underCount.zero", snap.targetUnderCount, 0);
    EXPECT_INT("touchTarget.disabledCount.zero", snap.targetDisabledCount, 0);
    EXPECT_INT("touchTarget.outOfBoundsCount.zero", snap.targetOutOfBoundsCount, 0);
    return 1;
}

static int probe_safety_predicate(void) {
    EXPECT_INT("safety.16x16",
               GESTURENAV_Compat_IsTargetSafePc34Compat(16, 16), 1);
    EXPECT_INT("safety.24x24",
               GESTURENAV_Compat_IsTargetSafePc34Compat(24, 24), 1);
    EXPECT_ZERO("safety.15x16",
                GESTURENAV_Compat_IsTargetSafePc34Compat(15, 16));
    EXPECT_ZERO("safety.16x15",
                GESTURENAV_Compat_IsTargetSafePc34Compat(16, 15));
    EXPECT_ZERO("safety.zeroWidth",
                GESTURENAV_Compat_IsTargetSafePc34Compat(0, 24));
    EXPECT_ZERO("safety.negative",
                GESTURENAV_Compat_IsTargetSafePc34Compat(-1, 24));
    return 1;
}

static int probe_cross_game(void) {
    GestureNavConfigPc34Compat cfg;
    int perGame[5] = { -1, -1, -1, -1, -1 };
    int activated;
    int i;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    activated = GESTURENAV_Compat_VerifyCrossGameActivationPc34Compat(&cfg, perGame);
    EXPECT_INT("crossGame.activated", activated, 5);
    for (i = 0; i < 5; ++i) {
        EXPECT_INT("crossGame.perGame", perGame[i], 1);
    }
    return 1;
}

static int probe_edge_strafe(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    FS_InputQueue q;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    cfg.enableEdgeStrafe = 1;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);

    EXPECT_INT("edgeStrafe.leftZone",
               GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 32), 1);
    EXPECT_ZERO("edgeStrafe.center",
                GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 160));
    EXPECT_INT("edgeStrafe.rightZone",
               GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap, 300), 1);

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

    /* V2 disabled: no-op. */
    {
        GestureNavConfigPc34Compat cfg2 = cfg;
        GestureNavSnapshotPc34Compat snap2;
        cfg2.enableEdgeStrafe = 0;
        GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg2, &snap2);
        EXPECT_ZERO("edgeStrafe.v2Disabled",
                    GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(&snap2, 32));
    }
    return 1;
}

static int probe_tap(void) {
    GestureNavConfigPc34Compat cfg;
    GestureNavSnapshotPc34Compat snap;
    TouchPointerDispatchPc34Compat dispatch;
    memset(&cfg, 0, sizeof(cfg));
    cfg.inputMode = GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT;
    cfg.touchLevel = GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT;
    GESTURENAV_Compat_ApplyConfigPc34Compat(&cfg, &snap);
    EXPECT_INT("tap.forward",
               GESTURENAV_Compat_ResolveTapPc34Compat(&snap, 275, 135,
                                                      TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                      &dispatch),
               1);
    EXPECT_UINT("tap.forward.commandId",
                dispatch.commandId,
                DM1_V1_COMMAND_MOVE_FORWARD);
    return 1;
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("probe=firestaff_runtime_gesture_navigation_gate\n");
    printf("sourceEvidence=%s\n", GESTURENAV_Compat_GetSourceEvidence());

    probe_settings();
    probe_swipe_routes();
    probe_gate_inactive();
    probe_queue_integration();
    probe_touch_targets();
    probe_safety_predicate();
    probe_cross_game();
    probe_edge_strafe();
    probe_tap();

    printf("firestaffRuntimeGestureNavigationGatePass=%u\n", g_pass);
    printf("firestaffRuntimeGestureNavigationGateFail=%u\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
