#include "csb_v1_chaos_cast_cooldown_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void install_call_frame_script(CSB_V1_ChaosMagicState *chaos,
    uint16_t *script, int script_words)
{
    csb_v1_chaos_init(chaos);
    chaos->script_count = 1;
    chaos->scripts[0].bytecode = script;
    chaos->scripts[0].bytecode_len = script_words;
}

static void test_cast_to_ready_lifecycle(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_CALL, 5,
        CSB_DSA_OP_SET, 7,
        CSB_DSA_OP_END,
        CSB_DSA_OP_SET, 3,
        CSB_DSA_OP_RETURN
    };
    CSB_V1_ChaosMagicState chaos;
    CSB_V1_ChaosCastCooldownState cast;
    int status;

    install_call_frame_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    csb_v1_chaos_cast_cooldown_init(&cast, &chaos, 3);

    CHECK(csb_v1_chaos_cast_cooldown_begin(&cast, 0) == CSB_V1_CHAOS_CAST_RUNNING,
          "idle begin accepts a chaos cast");
    CHECK(csb_v1_chaos_cast_cooldown_begin(&cast, 0) == CSB_V1_CHAOS_CAST_BUSY,
          "running DSA cast rejects a second begin");

    CHECK(csb_v1_chaos_cast_cooldown_tick(&cast) == CSB_V1_CHAOS_CAST_RUNNING,
          "tick 1 enters the DSA call frame");
    CHECK(cast.dsa_calls_executed == 1 && cast.dsa_call_depth == 1,
          "CSBWin DSA _CALL frame is counted");
    CHECK(csb_v1_chaos_cast_cooldown_tick(&cast) == CSB_V1_CHAOS_CAST_RUNNING,
          "tick 2 executes called bytecode");
    CHECK(chaos.flags[3] == 1, "called DSA frame mutates its flag");
    CHECK(csb_v1_chaos_cast_cooldown_tick(&cast) == CSB_V1_CHAOS_CAST_RUNNING,
          "tick 3 returns from the call frame");
    CHECK(cast.dsa_call_depth == 0, "DSA call frame unwinds to depth zero");
    CHECK(csb_v1_chaos_cast_cooldown_tick(&cast) == CSB_V1_CHAOS_CAST_RUNNING,
          "tick 4 resumes caller bytecode");
    CHECK(chaos.flags[7] == 1, "caller DSA bytecode resumes after return");
    CHECK(csb_v1_chaos_cast_cooldown_tick(&cast) == CSB_V1_CHAOS_CAST_COOLDOWN,
          "END transitions the accepted cast into cooldown");
    CHECK(cast.cooldown_ticks == 3 && cast.casts_completed == 1,
          "cooldown is installed after DSA completion");

    status = CSB_V1_CHAOS_CAST_COOLDOWN;
    while (status == CSB_V1_CHAOS_CAST_COOLDOWN) {
        status = csb_v1_chaos_cast_cooldown_tick(&cast);
    }
    CHECK(status == CSB_V1_CHAOS_CAST_READY, "cooldown ticks down to ready");
    CHECK(cast.cooldown_ticks == 0, "ready state has zero cooldown ticks");
    CHECK(csb_v1_chaos_cast_cooldown_begin(&cast, 0) == CSB_V1_CHAOS_CAST_RUNNING,
          "ready state accepts the next cast");
}

static void test_partial_cancel_resets_cooldown(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_CALL, 5,
        CSB_DSA_OP_SET, 9,
        CSB_DSA_OP_END,
        CSB_DSA_OP_DELAY, 4,
        CSB_DSA_OP_RETURN
    };
    CSB_V1_ChaosMagicState chaos;
    CSB_V1_ChaosCastCooldownState cast;

    install_call_frame_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    csb_v1_chaos_cast_cooldown_init(&cast, &chaos, 5);

    CHECK(csb_v1_chaos_cast_cooldown_begin(&cast, 0) == CSB_V1_CHAOS_CAST_RUNNING,
          "second lifecycle begins from idle");
    CHECK(csb_v1_chaos_cast_cooldown_tick(&cast) == CSB_V1_CHAOS_CAST_RUNNING,
          "partial lifecycle enters a DSA call frame");
    csb_v1_chaos_cast_cooldown_cancel(&cast);
    CHECK(cast.active_script_id == -1, "cancel clears the active cast id");
    CHECK(cast.cooldown_ticks == 0, "partial cancel resets cooldown to ready");
    CHECK(chaos.scripts[0].active == 0 && chaos.scripts[0].sp == 0,
          "partial cancel unwinds active DSA bytecode state");
    CHECK(cast.casts_canceled == 1, "partial cancel is counted");
    CHECK(csb_v1_chaos_cast_cooldown_begin(&cast, 0) == CSB_V1_CHAOS_CAST_RUNNING,
          "cancelled partial cast does not leave the next cast stuck");
}

static void test_source_lock_comment_contract(void)
{
    const char *anchors =
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9\n"
        "CSBWin/Chaos.cpp:584 InitializeE\n"
        "CSBWin/DSA.cpp:764-808 EX_GOSUB\n"
        "CSBWin/DSA.cpp:5053-5120 Execute\n"
        "CSBWin/DSA.cpp:5329-5441 ProcessDSATimer6\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos\n"
        "ReDMCSB COMMAND.C:2302-2306 spell-area command gate\n"
        "ReDMCSB GAMELOOP.C:150-155 tick decrement\n"
        "ReDMCSB MENU.C:2036-2039 spell disable ticks\n";

    CHECK(strstr(anchors, "CSBWin/DSA.cpp:5053-5120") != NULL,
          "source lock cites DSA Execute dispatch");
    CHECK(strstr(anchors, "ReDMCSB COMMAND.C:2302-2306") != NULL,
          "source lock cites COMMAND.C cast gate");
    CHECK(strstr(anchors, "ReDMCSB GAMELOOP.C:150-155") != NULL,
          "source lock cites tick decrement");
}

int main(void)
{
    printf("=== CSB V1 Chaos Cast/Cooldown Regression Gate ===\n\n");
    test_cast_to_ready_lifecycle();
    test_partial_cancel_resets_cooldown();
    test_source_lock_comment_contract();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
