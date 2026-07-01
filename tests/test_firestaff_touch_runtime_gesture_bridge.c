#include "firestaff_input.h"
#include "firestaff_touch.h"

#include <stdio.h>
#include <string.h>

/* Data-free bridge test for firestaff_touch.c -> runtime gesture gate ->
 * FS_InputQueue. Source lock: ReDMCSB COMMAND.C F0380 consumes C001..C006
 * movement IDs from the same queue; firestaff_touch.c must not bypass the
 * runtime gesture policy when converting touch gestures into commands. */

static FS_InputQueue g_queue;
static int failures = 0;

FS_InputQueue *fs_g_input_queue_get(void) {
    return &g_queue;
}

static void check(const char* name, int ok) {
    if (!ok) {
        printf("FAIL %s\n", name);
        ++failures;
    } else {
        printf("PASS %s\n", name);
    }
}

static void reset_queue(void) {
    fs_input_queue_init(&g_queue);
}

static int pop_cmd(FS_Command* outCmd) {
    FS_InputEvent ev;
    memset(&ev, 0, sizeof(ev));
    if (!fs_input_queue_pop(&g_queue, &ev)) return 0;
    if (outCmd) *outCmd = ev.cmd;
    return 1;
}

static FirestaffRuntimeGestureNavPolicy touch_policy(
    int touchEnabled,
    int v2Enabled,
    int v1ParityPreserve) {

    FirestaffRuntimeGestureNavPolicy policy;
    memset(&policy, 0, sizeof(policy));
    policy.accessibilityTouchEnabled = touchEnabled;
    policy.v2PresentationEnabled = v2Enabled;
    policy.v1ParityPreserve = v1ParityPreserve;
    return policy;
}

static void test_disabled_swipe_rejected(void) {
    FirestaffRuntimeGestureNavPolicy policy = touch_policy(0, 1, 0);
    FirestaffRuntimeGestureNavResult result;
    memset(&result, 0, sizeof(result));

    reset_queue();
    check("A1 disabled swipe returns 0",
          firestaff_touch_emit_swipe_runtime(100, 150, 100, 90,
                                             &policy, &result) == 0);
    check("A2 disabled swipe reports policy rejection",
          result.decision == RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED);
    check("A3 disabled swipe queues nothing",
          fs_input_queue_count(&g_queue) == 0);
}

static void test_enabled_swipe_queues_forward(void) {
    FirestaffRuntimeGestureNavPolicy policy = touch_policy(1, 0, 1);
    FirestaffRuntimeGestureNavResult result;
    FS_Command cmd = FS_CMD_NONE;
    memset(&result, 0, sizeof(result));

    reset_queue();
    check("B1 enabled swipe queues command",
          firestaff_touch_emit_swipe_runtime(100, 150, 100, 90,
                                             &policy, &result) == 1);
    check("B2 enabled swipe decision is forward",
          result.decision == RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD);
    check("B3 enabled swipe command code is FS_CMD_MOVE_FORWARD",
          result.commandCode == FS_CMD_MOVE_FORWARD);
    check("B4 enabled swipe queue count is one",
          fs_input_queue_count(&g_queue) == 1);
    check("B5 enabled swipe pop is MOVE_FORWARD",
          pop_cmd(&cmd) && cmd == FS_CMD_MOVE_FORWARD);
}

static void test_ambiguous_swipe_does_not_queue(void) {
    FirestaffRuntimeGestureNavPolicy policy = touch_policy(1, 1, 0);
    FirestaffRuntimeGestureNavResult result;
    memset(&result, 0, sizeof(result));

    reset_queue();
    check("C1 diagonal swipe returns 0",
          firestaff_touch_emit_swipe_runtime(100, 100, 150, 150,
                                             &policy, &result) == 0);
    check("C2 diagonal swipe is ambiguous",
          result.decision == RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS);
    check("C3 diagonal swipe queues nothing",
          fs_input_queue_count(&g_queue) == 0);
}

static void test_edge_strafe_policy(void) {
    FirestaffRuntimeGestureNavPolicy v1Policy = touch_policy(1, 0, 1);
    FirestaffRuntimeGestureNavPolicy v2Policy = touch_policy(1, 1, 0);
    FirestaffRuntimeGestureNavResult result;
    FS_Command cmd = FS_CMD_NONE;
    memset(&result, 0, sizeof(result));

    reset_queue();
    check("D1 V1 edge-strafe returns 0",
          fs_touch_emit_edge_strafe_runtime(20, 320, &v1Policy, &result) == 0);
    check("D2 V1 edge-strafe rejected as V1-only",
          result.decision == RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY);
    check("D3 V1 edge-strafe queues nothing",
          fs_input_queue_count(&g_queue) == 0);

    reset_queue();
    memset(&result, 0, sizeof(result));
    check("D4 V2 right edge-strafe queues command",
          fs_touch_emit_edge_strafe_runtime(300, 320, &v2Policy, &result) == 1);
    check("D5 V2 right edge-strafe decision",
          result.decision == RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT);
    check("D6 V2 right edge-strafe pop is STRAFE_RIGHT",
          pop_cmd(&cmd) && cmd == FS_CMD_STRAFE_RIGHT);
}

static void test_compat_wrappers_still_emit(void) {
    FS_Command cmd = FS_CMD_NONE;

    reset_queue();
    firestaff_touch_emit_swipe(100, 150, 100, 90);
    check("E1 compat swipe wrapper queues forward",
          pop_cmd(&cmd) && cmd == FS_CMD_MOVE_FORWARD);

    reset_queue();
    cmd = FS_CMD_NONE;
    fs_touch_emit_edge_strafe(2, 320);
    check("E2 compat edge wrapper queues strafe left",
          pop_cmd(&cmd) && cmd == FS_CMD_STRAFE_LEFT);
}

int main(void) {
    printf("test=firestaff_touch_runtime_gesture_bridge\n");

    test_disabled_swipe_rejected();
    test_enabled_swipe_queues_forward();
    test_ambiguous_swipe_does_not_queue();
    test_edge_strafe_policy();
    test_compat_wrappers_still_emit();

    printf("firestaffTouchRuntimeGestureBridgeOk=%u\n", failures == 0 ? 1u : 0u);
    return failures == 0 ? 0 : 1;
}
