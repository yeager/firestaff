/*
 * CTest gate for CSB V1 DSA single-step trigger.
 *
 *   Focused regression for one known script id and a known op sequence
 *   (SET, TOGGLE, TEST, END) on the global flag array.  This is the
 *   single step the CSBWin DSA interpreter (CSBWin/DSA.cpp) takes when
 *   a QueueDSASwitchAction completes and a chaos-cast script is
 *   triggered, exposed through the csb_v1_chaos_magic_pc34_compat
 *   compat layer.
 *
 * Source anchors (CSBWin):
 *   CSBWin/DSA.cpp:485  QueueDSASwitchAction
 *   CSBWin/DSA.cpp:574  EX_NOOP
 *   CSBWin/DSA.cpp:764  EX_GOSUB
 *   CSBWin/DSA.cpp:247  EXECUTIONPACKET
 *   CSBWin/Chaos.cpp:60 _CALL0-_CALL9 dispatch frame set
 *   CSBWin/Chaos.cpp:584 InitializeE
 *
 * Source anchors (Firestaff compat layer):
 *   include/csb_v1_chaos_magic_pc34_compat.h
 *   src/csb/csb_v1_chaos_magic_pc34_compat.c
 *     csb_v1_chaos_init (line 15)
 *     csb_v1_chaos_trigger (line 50)
 *     csb_v1_dsa_execute_step (line 58)
 *
 * Scope:
 *   - One chaos state, one installed script (script id 0).
 *   - One opcode sequence of well-known length.
 *   - One well-known global flag index.
 *
 * Out of scope (covered by adjacent tests, not duplicated here):
 *   - Cooldown ticks / cast begin-cancel lifecycle
 *     (tests/test_csb_v1_chaos_cast_cooldown_pc34_compat.c).
 *   - Pre-cast targeting / dispatch frame selection
 *     (tests/test_csb_v1_chaos_cast_targeting_pc34_compat.c).
 *   - Multi-script tick loops
 *     (csb_v1_chaos_tick is exercised by the same test below but
 *      not as a primary subject).
 *
 * Adjacent runtime-adjacent guards covered here:
 *   - csb_v1_chaos_trigger out-of-range script_id rejection
 *     (test_trigger_out_of_range_script_id_rejects_cleanly, the
 *      actuator-target-data guard for the chaos dispatcher).
 */

#include "csb_v1_chaos_magic_pc34_compat.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

/* The known square / section: one script, one flag, one op stream. */
#define DSA_KNOWN_SCRIPT_ID   0
#define DSA_KNOWN_FLAG        42
#define DSA_KNOWN_OTHER_FLAG  43
#define DSA_KNOWN_MESSAGE_ID  77

static int g_assertions;
static int g_failures;

static void check(int cond, const char *anchor, const char *msg)
{
    ++g_assertions;
    if (cond) {
        printf("  PASS: %s [%s]\n", msg, anchor);
    } else {
        ++g_failures;
        printf("  FAIL: %s [%s]\n", msg, anchor);
    }
}

static void install_single_script(CSB_V1_ChaosMagicState *chaos,
    uint16_t *bytecode, int bytecode_words)
{
    csb_v1_chaos_init(chaos);
    chaos->script_count = 1;
    chaos->scripts[DSA_KNOWN_SCRIPT_ID].bytecode = bytecode;
    chaos->scripts[DSA_KNOWN_SCRIPT_ID].bytecode_len = bytecode_words;
}

/* ----------------------------------------------------------------
 * Test 1: one known script, one DSA trigger, single SET step.
 * ----------------------------------------------------------------
 * After csb_v1_chaos_trigger(script_id=0):
 *   - script 0 is active, pc = 0, sp = 0
 *   - global flag DSA_KNOWN_FLAG starts at 0
 * After one csb_v1_dsa_execute_step call:
 *   - pc advances past the SET opcode and its flag argument
 *   - the flag is set
 *   - the script is still active (no END yet)
 *
 * Source: csb_v1_dsa_execute_step (CSB DSA interpreter) -> SET case.
 */
static void test_single_step_set_flips_flag(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG,
        CSB_DSA_OP_END
    };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    check(chaos.flags[DSA_KNOWN_FLAG] == 0,
          "include/csb_v1_chaos_magic_pc34_compat.h:CSB_V1_ChaosMagicState",
          "flag is cleared before trigger");
    check(csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "trigger accepts the known script id");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "known script is active after trigger");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "known script pc starts at zero");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "single SET step keeps the script active");
    check(chaos.flags[DSA_KNOWN_FLAG] == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
          "single SET step writes the known flag");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 2,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
          "SET consumes opcode + flag argument");
}

/* ----------------------------------------------------------------
 * Test 2: single TOGGLE step on a known flag.
 * ----------------------------------------------------------------
 * Flag starts at 1, trigger fires, one toggle step flips to 0 and
 * leaves the script active.
 *
 * Source: csb_v1_dsa_execute_step (CSB DSA interpreter) -> TOGGLE case.
 */
static void test_single_step_toggle_flips_flag(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_TOGGLE, (uint16_t)DSA_KNOWN_FLAG,
        CSB_DSA_OP_END
    };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    chaos.flags[DSA_KNOWN_FLAG] = 1;
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "single TOGGLE step keeps the script active");
    check(chaos.flags[DSA_KNOWN_FLAG] == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TOGGLE",
          "single TOGGLE step flips the known flag from 1 to 0");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 2,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TOGGLE",
          "TOGGLE consumes opcode + flag argument");
}

/* ----------------------------------------------------------------
 * Test 3: single END step deactivates the script.
 * ----------------------------------------------------------------
 * After a single END step on a 1-op script, the script goes inactive
 * and the chaos tick should report 0 active scripts.
 *
 * Source: csb_v1_dsa_execute_step (CSB DSA interpreter) -> END case.
 *         csb_v1_chaos_tick (compat wrapper around execute_step).
 */
static void test_single_step_end_deactivates_script(void)
{
    uint16_t script[] = { CSB_DSA_OP_END };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "END-only script is active after trigger");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_END",
          "single END step returns inactive");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_END",
          "single END step deactivates the script");
    check(csb_v1_chaos_tick(&chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_tick",
          "chaos tick reports zero active scripts after END");
}

/* ----------------------------------------------------------------
 * Test 4: single TEST step on an unset flag is a fall-through.
 * ----------------------------------------------------------------
 * The TEST opcode reads flag + target; when the flag is unset and
 * the target would be a jump, the script must not jump and must
 * remain active.  This protects against a future bug that would
 * accidentally evaluate the unset flag as truthy.
 *
 * Source: csb_v1_dsa_execute_step (CSB DSA interpreter) -> TEST case.
 */
static void test_single_step_test_unset_flag_no_jump(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_TEST, (uint16_t)DSA_KNOWN_FLAG, 2, /* would jump to END */
        CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_OTHER_FLAG,
        CSB_DSA_OP_END
    };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    check(chaos.flags[DSA_KNOWN_FLAG] == 0,
          "include/csb_v1_chaos_magic_pc34_compat.h:CSB_V1_ChaosMagicState",
          "test flag starts cleared");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "single TEST step on unset flag keeps the script active");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 3,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "TEST consumes opcode + flag + target, no jump taken");
    check(chaos.flags[DSA_KNOWN_OTHER_FLAG] == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "non-jumping TEST does not run subsequent SET");
}

/* ----------------------------------------------------------------
 * Test 5: single TEST step on a set flag jumps to a target section.
 * ----------------------------------------------------------------
 * When the flag is set, TEST should jump and execute the target SET.
 *
 * Source: csb_v1_dsa_execute_step (CSB DSA interpreter) -> TEST case.
 */
static void test_single_step_test_set_flag_jumps_to_target(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG,
        CSB_DSA_OP_TEST, (uint16_t)DSA_KNOWN_FLAG, 5,
        CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_OTHER_FLAG,
        CSB_DSA_OP_END
    };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
          "initial TEST fixture sets the known flag");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 2,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
          "SET consumes opcode + flag argument");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "TEST on a set flag jumps to target section");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 5,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "jump target enters the SET opcode section");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
          "jumped TEST executes the target section SET");
    check(chaos.flags[DSA_KNOWN_OTHER_FLAG] == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "jumped section updates the secondary flag");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 7,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_TEST",
          "after target SET, pc now points at END");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_END",
          "END in target section deactivates the script");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_END",
          "jumped script deactivates at END");
}

/* ----------------------------------------------------------------
 * Test 6: invalid bytecode conditions must not crash, must not
 * advance pc, and must return 0 (inactive).
 * ----------------------------------------------------------------
 * This covers the three documented guard conditions:
 *   - script == NULL
 *   - state == NULL
 *   - script->active == 0
 *   - script->bytecode == NULL
 *
 * Source: csb_v1_dsa_execute_step guard prologue.
 */
static void test_single_step_guards(void)
{
    uint16_t script[] = { CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);

    check(csb_v1_dsa_execute_step(NULL, &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "null script pointer is rejected");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], NULL) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "null state pointer is rejected");

    /* Deactivated script must be a no-op. */
    chaos.scripts[DSA_KNOWN_SCRIPT_ID].active = 0;
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "inactive script is a no-op step");
    check(chaos.flags[DSA_KNOWN_FLAG] == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "inactive script does not mutate global flags");

    /* Script with NULL bytecode must deactivate. */
    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    chaos.scripts[DSA_KNOWN_SCRIPT_ID].bytecode = NULL;
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "null bytecode deactivates the script");

    /* Out-of-range script pc must deactivate without writing flags. */
    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc = 99;
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "out-of-range pc deactivates the script");
    check(chaos.flags[DSA_KNOWN_FLAG] == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "out-of-range pc does not mutate global flags");

    /* DELAY step with N ticks must not run opcodes for N ticks.
     *
     * Step 1: read DELAY opcode + load delay_ticks = 2 (no tick decrement).
     * Step 2: delay_ticks-- (2 -> 1).
     * Step 3: delay_ticks-- (1 -> 0).
     * Step 4: read SET opcode + write flag.
     */
    {
        uint16_t delay_script[] = {
            CSB_DSA_OP_DELAY, 2,
            CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG,
            CSB_DSA_OP_END
        };
        install_single_script(&chaos, delay_script,
            (int)(sizeof(delay_script) / sizeof(delay_script[0])));
        (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
        (void)csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos);
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].delay_ticks == 2,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_DELAY",
              "first DELAY step loads the delay argument without decrementing");
        (void)csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos);
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].delay_ticks == 1,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
              "second step decrements the remaining delay");
        (void)csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos);
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].delay_ticks == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
              "third step drains the last delay tick");
        check(chaos.flags[DSA_KNOWN_FLAG] == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_DELAY",
              "DELAY blocks the next opcode until ticks reach zero");
        (void)csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos);
        check(chaos.flags[DSA_KNOWN_FLAG] == 1,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_DELAY",
              "after delay elapses, the next opcode runs");
    }
}

/* ----------------------------------------------------------------
 * Test 7: malformed DSA operands reject the script without mutation.
 * ----------------------------------------------------------------
 * Imported or damaged DSA bytecode can carry edge flag ids or jump
 * targets.  The VM must stop cleanly instead of partially executing a
 * target reference.
 *
 * Source: csb_v1_dsa_execute_step malformed operand guards.
 */
static void test_malformed_target_operands_reject_cleanly(void)
{
    CSB_V1_ChaosMagicState chaos;

    {
        uint16_t script[] = { CSB_DSA_OP_SET };
        install_single_script(&chaos, script,
            (int)(sizeof(script) / sizeof(script[0])));
        (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
        check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID],
              &chaos) == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
              "truncated SET operand rejects the script");
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_reject_malformed_at",
              "truncated SET deactivates cleanly");
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_reject_malformed_at",
              "truncated SET leaves pc at the malformed opcode");
        check(chaos.flags[DSA_KNOWN_FLAG] == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
              "truncated SET does not mutate flags");
    }

    {
        uint16_t script[] = { CSB_DSA_OP_SET, 256 };
        install_single_script(&chaos, script,
            (int)(sizeof(script) / sizeof(script[0])));
        (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
        check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID],
              &chaos) == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_flag_is_valid",
              "out-of-range flag id rejects the script");
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_reject_malformed_at",
              "out-of-range flag leaves pc at the malformed opcode");
        check(chaos.flags[DSA_KNOWN_FLAG] == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
              "out-of-range flag does not mutate nearby flags");
    }

    {
        uint16_t script[] = {
            CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG,
            CSB_DSA_OP_TEST, (uint16_t)DSA_KNOWN_FLAG, 5
        };
        install_single_script(&chaos, script,
            (int)(sizeof(script) / sizeof(script[0])));
        (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
        check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID],
              &chaos) == 1,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_SET",
              "edge-target fixture first sets the test flag");
        check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID],
              &chaos) == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_target_is_valid",
              "TEST target equal to bytecode_len rejects the script");
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 2,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_reject_malformed_at",
              "bad TEST target leaves pc at the TEST opcode");
    }

    {
        uint16_t script[] = {
            CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG,
            CSB_DSA_OP_TEST, (uint16_t)DSA_KNOWN_FLAG, 0xFFFFu
        };
        install_single_script(&chaos, script,
            (int)(sizeof(script) / sizeof(script[0])));
        (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
        (void)csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID],
              &chaos);
        check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID],
              &chaos) == 0,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_target_is_valid",
              "large TEST target rejects the script");
        check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 2,
              "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_reject_malformed_at",
              "large TEST target leaves pc at the TEST opcode");
    }
}

/* ----------------------------------------------------------------
 * Test 7b: out-of-range script id must not crash, must not mutate,
 * and must return -1.
 * ----------------------------------------------------------------
 * csb_v1_chaos_trigger is the runtime-adjacent entry point that turns
 * a CSB chaos cast into an active DSA script.  It is the "actuator
 * target data" surface for the chaos system: a malformed script_id
 * (negative, equal to script_count, or beyond CSB_V1_MAX_DSA_SCRIPTS)
 * must be rejected cleanly without writing into the scripts[] table.
 *
 * Source: csb_v1_chaos_trigger guard prologue (script_id range check
 *         against state->script_count) and CSBWin/Chaos.cpp _CALL0..9
 *         dispatch frame selection.
 */
static void test_trigger_out_of_range_script_id_rejects_cleanly(void)
{
    CSB_V1_ChaosMagicState chaos;
    uint16_t script[] = {
        CSB_DSA_OP_SET, (uint16_t)DSA_KNOWN_FLAG,
        CSB_DSA_OP_END
    };

    /* Single known script -> script_count == 1.
     * Valid script_id is 0.  Out-of-range cases:
     *   -1 (negative)
     *   1 (== script_count)
     *   CSB_V1_MAX_DSA_SCRIPTS (way past the loaded table)
     *   INT_MAX (worst-case saturated script_id) */
    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));

    /* Negative script_id must reject without activating. */
    check(csb_v1_chaos_trigger(&chaos, -1) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "negative script_id rejects the trigger");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "negative script_id leaves the known script inactive");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "negative script_id does not advance pc");

    /* script_id equal to script_count (one past the last valid index)
     * must reject without activating. */
    check(csb_v1_chaos_trigger(&chaos, (int)chaos.script_count) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "script_id == script_count rejects the trigger");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "script_id == script_count leaves the known script inactive");

    /* Script_id far above the loaded table must reject.  This is the
     * actuator-target-data guard the chaos dispatcher needs when a
     * imported script table reports an out-of-range script id. */
    check(csb_v1_chaos_trigger(&chaos, CSB_V1_MAX_DSA_SCRIPTS) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "script_id == CSB_V1_MAX_DSA_SCRIPTS rejects the trigger");
    check(csb_v1_chaos_trigger(&chaos, CSB_V1_MAX_DSA_SCRIPTS + 1) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "script_id == CSB_V1_MAX_DSA_SCRIPTS+1 rejects the trigger");
    check(csb_v1_chaos_trigger(&chaos, 0x7fffffff) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "saturated script_id rejects the trigger");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "saturated script_id leaves the known script inactive");

    /* NULL state pointer must reject cleanly. */
    check(csb_v1_chaos_trigger(NULL, DSA_KNOWN_SCRIPT_ID) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "null state pointer rejects the trigger");

    /* Sanity: a valid script_id still works after the rejection path
     * has been exercised.  The known script activates and a SET step
     * writes the known flag, proving the rejected requests left the
     * runtime in a usable state. */
    check(csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "valid script_id still triggers after out-of-range rejects");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].active == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "valid script_id leaves the script active after out-of-range rejects");
    (void)csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos);
    check(chaos.flags[DSA_KNOWN_FLAG] == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "valid script_id still mutates flags after out-of-range rejects");
}

/* ----------------------------------------------------------------
 * Test 8: single MESSAGE step records one deterministic dispatch.
 * ----------------------------------------------------------------
 * This is intentionally only the DSA/message dispatch boundary: one
 * target message id enters the compat state as one observable dispatch
 * record.  The renderer/text-log path remains outside this VM gate.
 *
 * Source: CSBWin/DSA.cpp QueueDSASwitchAction TT_DESSAGE branch and
 *         ProcessDSATimer6 message-column execution; ReDMCSB TEXT.C
 *         F0047_TEXT_MESSAGEAREA_PrintMessage for the eventual text
 *         surface.
 */
static void test_single_step_message_records_dispatch(void)
{
    uint16_t script[] = {
        CSB_DSA_OP_MESSAGE, (uint16_t)DSA_KNOWN_MESSAGE_ID,
        CSB_DSA_OP_END
    };
    CSB_V1_ChaosMagicState chaos;

    install_single_script(&chaos, script, (int)(sizeof(script) / sizeof(script[0])));
    check(chaos.dispatch_count == 0,
          "include/csb_v1_chaos_magic_pc34_compat.h:CSB_V1_ChaosMagicState",
          "message dispatch count starts at zero");
    check(chaos.last_dispatch.kind == CSB_V1_DSA_DISPATCH_NONE,
          "include/csb_v1_chaos_magic_pc34_compat.h:CSB_V1_DSADispatchRecord",
          "last dispatch starts empty");
    (void)csb_v1_chaos_trigger(&chaos, DSA_KNOWN_SCRIPT_ID);
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "single MESSAGE step keeps the script active");
    check(chaos.dispatch_count == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_record_dispatch",
          "single MESSAGE step records exactly one dispatch");
    check(chaos.last_dispatch.kind == CSB_V1_DSA_DISPATCH_MESSAGE,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "dispatch kind is MESSAGE");
    check(chaos.last_dispatch.opcode == CSB_DSA_OP_MESSAGE,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "dispatch records the source opcode");
    check(chaos.last_dispatch.operand == DSA_KNOWN_MESSAGE_ID,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "dispatch records the known message target");
    check(chaos.last_dispatch.op_pc == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "dispatch records the opcode pc");
    check(chaos.scripts[DSA_KNOWN_SCRIPT_ID].pc == 2,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "MESSAGE consumes opcode + message argument");
    check(chaos.flags[DSA_KNOWN_FLAG] == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_MESSAGE",
          "MESSAGE dispatch does not mutate DSA flags");
    check(csb_v1_dsa_execute_step(&chaos.scripts[DSA_KNOWN_SCRIPT_ID], &chaos) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:CSB_DSA_OP_END",
          "END after MESSAGE deactivates the script");
}

/* ----------------------------------------------------------------
 * Test 9: source-evidence string must cite CSBWin DSA anchors so
 * future readers can trace the test back to ReDMCSB / CSBWin.
 * ---------------------------------------------------------------- */
static void test_source_evidence_anchors(void)
{
    const char *ev = csb_v1_chaos_source_evidence();
    check(ev != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence string is non-null");
    check(strstr(ev, "DSA.cpp") != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence cites CSBWin/DSA.cpp");
    check(strstr(ev, "Chaos.cpp") != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence cites CSBWin/Chaos.cpp");
    check(strstr(ev, "InitializeE") != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence cites InitializeE");
    check(strstr(ev, "_CALL0") != NULL || strstr(ev, "_CALL") != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence cites _CALL0-_CALL9 dispatch frame set");
    check(strstr(ev, "TT_DESSAGE") != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence cites TT_DESSAGE message dispatch");
    check(strstr(ev, "TEXT.C") != NULL,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_source_evidence",
          "source evidence cites ReDMCSB TEXT.C message surface");
}

int main(void)
{
    printf("=== CSB V1 DSA Trigger Single Step Gate ===\n");
    printf("probe=csb_v1_dsa_trigger_single_step\n");
    printf("sourceEvidence=%s\n\n", csb_v1_chaos_source_evidence());

    test_single_step_set_flips_flag();
    test_single_step_toggle_flips_flag();
    test_single_step_end_deactivates_script();
    test_single_step_test_unset_flag_no_jump();
    test_single_step_test_set_flag_jumps_to_target();
    test_single_step_guards();
    test_malformed_target_operands_reject_cleanly();
    test_trigger_out_of_range_script_id_rejects_cleanly();
    test_single_step_message_records_dispatch();
    test_source_evidence_anchors();

    printf("\nassertions=%d failures=%d\n", g_assertions, g_failures);
    if (g_failures == 0) {
        printf("PASS csb_v1_dsa_trigger_single_step assertions=%d failures=0\n",
               g_assertions);
        return 0;
    }
    return 1;
}
