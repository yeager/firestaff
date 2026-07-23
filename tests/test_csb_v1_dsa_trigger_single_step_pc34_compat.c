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
#include "csb_v1_runtime_pc34_compat.h"

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

typedef struct {
    int calls;
    uint32_t location;
    uint8_t skin;
} PendingSkinWriteProbe;

static int pending_skin_write_probe(void *user, uint32_t location,
                                    uint8_t skin)
{
    PendingSkinWriteProbe *probe = (PendingSkinWriteProbe *)user;

    if (!probe) return 0;
    ++probe->calls;
    probe->location = location;
    probe->skin = skin;
    return 1;
}

static void put_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

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

/* The loader receives a short-lived byte buffer, just as the dungeon/save
 * reader does.  The decoded script must remain executable after it changes. */
static void test_loader_owns_validated_script_words(void)
{
    uint8_t data[] = {
        1, 0,             /* one script */
        6, 0, 6, 0,       /* byte offset / byte length */
        CSB_DSA_OP_SET, 0, DSA_KNOWN_FLAG, 0, CSB_DSA_OP_END, 0
    };
    CSB_V1_ChaosMagicState chaos;

    csb_v1_chaos_init(&chaos);
    check(csb_v1_chaos_load_scripts(&chaos, data, (int)sizeof(data)) == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_load_scripts",
          "loader accepts one bounded little-endian script");
    data[6] = CSB_DSA_OP_END;
    check(csb_v1_chaos_trigger(&chaos, 0) == 0,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_trigger",
          "loaded script is triggerable after input changes");
    check(csb_v1_dsa_execute_step(&chaos.scripts[0], &chaos) == 1 &&
              chaos.flags[DSA_KNOWN_FLAG] == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_dsa_execute_step",
          "loaded bytecode is owned and executes its original SET");
    csb_v1_chaos_cleanup(&chaos);
}

static void test_loader_rejects_malformed_table_without_mutation(void)
{
    uint8_t malformed[] = { 1, 0, 7, 0, 2, 0, 0, 0 };
    CSB_V1_ChaosMagicState chaos;

    csb_v1_chaos_init(&chaos);
    chaos.script_count = 17;
    chaos.flags[DSA_KNOWN_FLAG] = 1;
    check(csb_v1_chaos_load_scripts(&chaos, malformed, (int)sizeof(malformed)) == -1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_load_scripts",
          "loader rejects an odd byte offset");
    check(chaos.script_count == 17 && chaos.flags[DSA_KNOWN_FLAG] == 1,
          "src/csb/csb_v1_chaos_magic_pc34_compat.c:csb_v1_chaos_load_scripts",
          "malformed load leaves live chaos state unchanged");
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

static void test_csbwin_load_opcode_family(void)
{
    uint16_t integer[] = { 0x0686u, 0x1234u };
    uint16_t integer32[] = { 0x0786u, 0x5678u, 0x1234u };
    uint16_t dollar[] = { 0x0706u };
    uint16_t parameter[] = { 0x0006u };
    uint16_t illegal_abs32[] = { 0x0746u };
    uint32_t parameters[] = { 0x55u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_CSBWinDSALoadContext context;
    CSB_V1_CSBWinDSALoadExecution execution;

    memset(&action, 0, sizeof(action));
    memset(&context, 0, sizeof(context));
    context.master_location = (2u << 16) | (3u << 10) | (4u << 5) | 5u;
    context.parameters = parameters;
    context.parameter_count = 1;

    action.program_words = integer;
    action.program_word_count = 2;
    check(csb_v1_csbwin_dsa_execute_load_action(&action, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_LOAD_OK && execution.value == 0x1234u &&
              execution.next_state == 0 && execution.words_consumed == 2u,
          "CSBWin/DSA.cpp:1074-1189 EX_LOAD",
          "source LOAD INTEGER decodes exact command and operand words");
    action.program_words = integer32;
    action.program_word_count = 3;
    check(csb_v1_csbwin_dsa_execute_load_action(&action, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_LOAD_OK && execution.value == 0x12345678u,
          "CSBWin/DSA.cpp:1125-1137 EX_LOAD",
          "source LOAD INTEGER32 preserves little-endian word order");
    action.program_words = dollar;
    action.program_word_count = 1;
    check(csb_v1_csbwin_dsa_execute_load_action(&action, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_LOAD_OK &&
              execution.value == context.master_location,
          "CSBWin/DSA.cpp:1118-1124 EX_LOAD",
          "source LOAD DOLLAR returns packed master location");
    action.program_words = parameter;
    action.program_word_count = 1;
    check(csb_v1_csbwin_dsa_execute_load_action(&action, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_LOAD_OK && execution.value == 0x55u,
          "CSBWin/DSA.cpp:1138-1189 EX_LOAD",
          "source LOAD A reads the authenticated actuator-chain parameter");
    action.program_words = illegal_abs32;
    action.program_word_count = 1;
    check(csb_v1_csbwin_dsa_execute_load_action(&action, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_LOAD_SOURCE_ILLEGAL,
          "CSBWin/DSA.cpp:1138-1143 EX_LOAD",
          "source-unhandled LOAD ABS32 remains illegal rather than guessed");
}

static void test_csbwin_load_store_rejects_unowned_action(void)
{
    uint32_t parameters[] = { 0u };
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSALoadStoreContext context;
    CSB_V1_CSBWinDSALoadStoreExecution execution;

    csb_v1_chaos_init(&state);
    memset(&context, 0, sizeof(context));
    memset(&execution, 0, sizeof(execution));
    context.parameters = parameters;
    context.parameter_count = 1;
    check(csb_v1_csbwin_dsa_execute_authenticated_load_store_action(
              &state, 7, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_LOAD_STORE_NOT_AUTHENTICATED &&
              parameters[0] == 0u,
          "CSBWin/SaveGame.cpp ReadDSAs + DSA.cpp:1317-1385",
          "unowned action lookup rejects before STORE can mutate parameters");
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_stack_opcode_family(void)
{
    uint16_t arithmetic[] = {
        0x0686u, 3u, 0x0686u, 4u, 0x004bu,
        0x0686u, 2u, 0x094bu, 0x000du
    };
    uint16_t set_new_state[] = { 0x0686u, 9u, 0x068bu };
    uint16_t extended_state[] = { 0x0686u, 3u, 0x0686u, 4u, 0x804bu, 0xfffcu };
    uint16_t extended_store[] = { 0x0686u, 1u, 0x800du, 0xfffcu };
    uint16_t unsupported[] = {
        0x0686u, 9u, 0x0686u, 1u, 0x084bu, 0x000du
    };
    uint32_t parameters[] = { 0u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&action, 0, sizeof(action));
    csb_v1_chaos_init(&state);
    memset(&context, 0, sizeof(context));
    context.parameters = parameters;
    context.parameter_count = 1;
    action.dsa_id = 7u;
    action.state_index = 1u;
    state.imported_actions = &action;
    state.imported_action_count = 1;

    action.program_words = arithmetic;
    action.program_word_count = (int)(sizeof(arithmetic) / sizeof(arithmetic[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && parameters[0] == 14u &&
              execution.words_consumed == action.program_word_count &&
              execution.command_count == 6u && execution.stack_depth == 0u,
          "CSBWin/DSA.cpp:2324-2719 EX_AMPERSAND",
          "authenticated LOAD/AMPERSAND/STORE executes source stack arithmetic");

    action.program_words = set_new_state;
    action.program_word_count = (int)(sizeof(set_new_state) / sizeof(set_new_state[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && execution.forced_state == 9 &&
              execution.stack_depth == 0u,
          "CSBWin/DSA.cpp:2852-2858 STKOP_SetNewState",
          "SETNEWSTATE consumes its stack word and reports the source forced state");

    action.program_words = extended_state;
    action.program_word_count = (int)(sizeof(extended_state) / sizeof(extended_state[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && execution.next_state == 65532 &&
              execution.stack_depth == 1u,
          "CSBWin/DSA.cpp:2324-2344 EX_AMPERSAND",
          "AMPERSAND -4 sentinel consumes its raw unsigned extension word");

    action.program_words = extended_store;
    action.program_word_count = (int)(sizeof(extended_store) / sizeof(extended_store[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && parameters[0] == 1u &&
              execution.next_state == 65532,
          "CSBWin/DSA.cpp:1317-1385 EX_STORE",
          "STORE -16 sentinel preserves its raw unsigned extension word");

    parameters[0] = 77u;
    action.program_words = unsupported;
    action.program_word_count = (int)(sizeof(unsupported) / sizeof(unsupported[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 1u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED && parameters[0] == 77u,
          "CSBWin/DSA.cpp:2859-2915 EX_AMPERSAND",
          "unsupported world-mutating AMPERSAND subcode rejects without commit");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_setskin_waits_for_complete_action(void)
{
    /* CSBWin DSA.cpp:3122-3135 executes SETSKIN from the stack, while the
     * Firestaff authenticated boundary must reject an entire malformed DSA
     * action without publishing its save-owned EXPOOL change. */
    uint16_t malformed_after_setskin[] = {
        0x0686u, 31u,              /* LOAD INTEGER skin */
        0x0686u, 0x0421u,          /* LOAD INTEGER location */
        0x0115u,                   /* AMPERSAND2 SetSkin */
        0x0000u                    /* unsupported DSACMD_NOOP */
    };
    uint32_t parameters[] = { 77u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;
    PendingSkinWriteProbe probe;

    memset(&action, 0, sizeof(action));
    memset(&context, 0, sizeof(context));
    memset(&probe, 0, sizeof(probe));
    csb_v1_chaos_init(&state);
    action.dsa_id = 9u;
    action.state_index = 4u;
    action.program_words = malformed_after_setskin;
    action.program_word_count = (int)(sizeof(malformed_after_setskin) /
                                      sizeof(malformed_after_setskin[0]));
    state.imported_actions = &action;
    state.imported_action_count = 1;
    context.parameters = parameters;
    context.parameter_count = 1;
    context.set_skin = pending_skin_write_probe;
    context.skin_user = &probe;

    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 9, 4u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED &&
              probe.calls == 0 && parameters[0] == 77u,
          "CSBWin/DSA.cpp:3122-3135 + SaveGame.cpp EXPOOL",
          "SETSKIN waits for the complete authenticated action before save publication");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_filter_stack_runner(void)
{
    uint16_t store_parameter[] = { 0x0686u, 0x1234u, 0x000du };
    uint16_t unsupported[] = { 0x084bu };
    uint16_t direct_jump[] = { 0x014cu };
    CSB_V1_DSAImportedAction action;
    CSB_V1_DSAImportedAction forged;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    int parameters[] = { 77 };

    memset(&action, 0, sizeof(action));
    memset(&forged, 0, sizeof(forged));
    memset(&runner, 0, sizeof(runner));
    csb_v1_chaos_init(&state);
    action.dsa_id = 9u;
    action.state_index = 4u;
    action.program_words = store_parameter;
    action.program_word_count = (int)(sizeof(store_parameter) /
                                      sizeof(store_parameter[0]));
    state.imported_actions = &action;
    state.imported_action_count = 1;
    runner.programs = &state;
    runner.dsa_id = 9;
    runner.state_index = 4u;
    runner.action_ordinal = 0;
    runner.master_location = 0x12345u;

    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, parameters, 1, NULL, &runner) == 1 &&
              parameters[0] == 0x1234 && runner.execution_count == 1 &&
              runner.last_execution.words_consumed == 3u,
          "CSBWin/DSA.cpp:5315-5460 ProcessDSAFilter",
          "runtime filter runner executes only its authenticated pure stack action");

    forged = action;
    parameters[0] = 71;
    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &forged, parameters, 1, NULL, &runner) == 0 &&
              parameters[0] == 71 && runner.execution_count == 1,
          "CSBWin/SaveGame.cpp ReadDSAs + DSA.cpp:5315-5460",
          "filter runner rejects a forged action pointer without publishing state");

    action.program_words = NULL;
    action.program_word_count = 0;
    parameters[0] = 70;
    runner.global_variable_count = 1;
    runner.global_variables[0] = 0xfeedu;
    runner.last_execution.words_consumed = 19u;
    runner.last_transfer.final_state = 23;
    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, parameters, 1, NULL, &runner) == 0 &&
              parameters[0] == 70 && runner.global_variables[0] == 0xfeedu &&
              runner.execution_count == 1 && runner.transfer_execution_count == 0 &&
              runner.last_execution.words_consumed == 19u &&
              runner.last_transfer.final_state == 23,
          "CSBWin/SaveGame.cpp ReadDSAs + DSA.cpp:5053-5293",
          "zero-word authenticated action rejects before opcode dispatch without publication");

    action.program_words = unsupported;
    action.program_word_count = 1;
    parameters[0] = 70;
    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, parameters, 1, NULL, &runner) == 0 &&
              parameters[0] == 70 && runner.execution_count == 1,
          "CSBWin/DSA.cpp:2859-2915 EX_AMPERSAND",
          "filter runner rejects unsupported world behavior without parameter commit");

    action.program_words = direct_jump;
    action.program_word_count = 1;
    action.column = 2u;
    runner.global_variable_count = 0;
    parameters[0] = 69;
    check(csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
              &action, parameters, 1, NULL, &runner) == 1 &&
              parameters[0] == 69 && runner.execution_count == 2 &&
              runner.transfer_execution_count == 1 &&
              runner.last_transfer.transfer_count == 1u &&
              runner.last_transfer.final_state == 4 && runner.state_index == 4u,
          "CSBWin/DSA.cpp:5053-5293 Execute",
          "filter runner consumes an authenticated JUMP chain without a synthetic action");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_runtime_filter_adapter(void)
{
    uint16_t store_monster_id[] = { 0x0686u, 0x1234u, 0x000du };
    uint16_t store_global[] = { 0x0686u, 0x55aau, 0x0054u };
    CSB_V1_DSAImportedAction actions[2];
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeDSAFilterBinding binding;
    CSB_V1_RuntimeDSAFilterStackAdapter adapter;
    CSB_V1_RuntimeDSAFilterStackAdapter save_adapter;
    CSB_V1_DSAFilterRuntime filter;
    CSB_V1_DSAFilterRuntime save_filter;
    CSB_V1_AttackParameters parameters;
    uint8_t appended_tail[CSB_V1_CSBWIN_EXPOOL_BLOCK_BYTES];
    const uint8_t *global_payload = NULL;
    size_t global_payload_size = 0u;
    const uint32_t global_record_id = (5u << 24) | (4u << 16);
    const uint32_t global_bucket = 32u +
        ((global_record_id * 0xbb40e62du) >> 27);

    memset(actions, 0, sizeof(actions));
    memset(&profile, 0, sizeof(profile));
    memset(&binding, 0, sizeof(binding));
    memset(&adapter, 0, sizeof(adapter));
    memset(&save_adapter, 0, sizeof(save_adapter));
    memset(&filter, 0, sizeof(filter));
    memset(&save_filter, 0, sizeof(save_filter));
    memset(&parameters, 0, sizeof(parameters));
    memset(appended_tail, 0, sizeof(appended_tail));
    csb_v1_chaos_init(&profile.csbwin_extended_dsa_state);
    actions[0].dsa_id = 9u;
    actions[0].state_index = 4u;
    actions[0].program_words = store_monster_id;
    actions[0].program_word_count = (int)(sizeof(store_monster_id) /
                                          sizeof(store_monster_id[0]));
    profile.csbwin_extended_features_valid = 1;
    actions[1].dsa_id = 9u;
    actions[1].state_index = 4u;
    actions[1].program_words = store_global;
    actions[1].program_word_count = (int)(sizeof(store_global) /
                                           sizeof(store_global[0]));
    profile.csbwin_extended_dsa_state.imported_actions = actions;
    profile.csbwin_extended_dsa_state.imported_action_count = 2;
    /* CSBWin SaveGame.cpp stores sixteen ui32 globals in each EXPOOL global
     * record. This is a local format regression, not a corpus fixture. */
    put_le16(appended_tail, 2u, 18u);
    put_le32(appended_tail, (size_t)global_bucket * 4u, 1u);
    put_le32(appended_tail, 1u * 4u, 0u);
    put_le32(appended_tail, 2u * 4u, global_record_id);
    profile.csbwin_global_variables_valid = 1;
    profile.csbwin_global_variable_count = 16u;
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = sizeof(appended_tail);
    profile.csbwin_appended_tail_preserved_size = sizeof(appended_tail);
    memcpy(profile.csbwin_appended_tail, appended_tail, sizeof(appended_tail));
    profile.csbwin_appended_tail_fnv1a = fnv1a32(
        profile.csbwin_appended_tail,
        profile.csbwin_appended_tail_preserved_size);
    binding.dsa_id = 9u;

    check(csb_v1_runtime_bind_csbwin_attack_filter_stack_runtime(
              &profile, &binding, 4u, 0, 0x12345u, 6, &filter,
              &adapter) == 1 &&
              filter.programs == &profile.csbwin_extended_dsa_state &&
              filter.runner == csb_v1_runtime_csbwin_dsa_filter_stack_runner_callback &&
              filter.runner_user == &adapter && filter.loaded_level == 6 &&
              filter.attack_filter_dsa_id == 9 &&
              filter.attack_filter_state == 4u &&
              filter.attack_filter_action == 0,
          "CSBWin/Monster.cpp:1134-1180 ProcessDSAFilter",
          "runtime atomically installs the authenticated type-47 attack filter");
    parameters.monsterID = 77;
    parameters.monsterIndex = 13;

    check(csb_v1_dsa_filter_attack_preprocess_live(&parameters, &filter) == 1 &&
              parameters.monsterID == 0x1234 &&
              adapter.runner.execution_count == 1 &&
              filter.loaded_level == 6,
          "CSBWin/DSA.cpp:5315-5460 + Monster.cpp:1164-1167",
          "live monster-filter callback runs only the profile-authenticated stack action");

    check(csb_v1_runtime_bind_csbwin_attack_filter_stack_runtime(
              &profile, &binding, 4u, 1, 0x12345u, 6, &save_filter,
              &save_adapter) == 1 &&
              csb_v1_dsa_filter_attack_preprocess_live(
                  &parameters, &save_filter) == 1 &&
              profile.csbwin_global_variables[1] == 0x55aau &&
              csb_v1_runtime_locate_csbwin_appended_expool_record(
                  &profile, global_record_id, &global_payload,
                  &global_payload_size) == 1 && global_payload_size == 64u &&
              global_payload[4] == 0xaau && global_payload[5] == 0x55u,
          "CSBWin/Monster.cpp:1134-1180 + SaveGame.cpp EXPOOL globals",
          "bound attack callback atomically publishes GLOBALSTORE to save-owned EXPOOL");

    profile.csbwin_extended_dsa_state.imported_actions = NULL;
    profile.csbwin_extended_dsa_state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&profile.csbwin_extended_dsa_state);
}

static void test_csbwin_authenticated_local_variable_opcode_family(void)
{
    uint16_t round_trip[] = {
        0x0686u, 0x1234u, 0x0012u, 0x0011u, 0x000du
    };
    uint16_t undefined_fetch[] = { 0x00d1u, 0x000du };
    uint16_t extended_state[] = {
        0x0686u, 7u, 0x8012u, 0xfffcu, 0x8011u, 0xfffcu
    };
    uint16_t invalid_index[] = { 0x1f91u, 0x000du };
    uint32_t parameters[] = { 99u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&action, 0, sizeof(action));
    csb_v1_chaos_init(&state);
    memset(&context, 0, sizeof(context));
    context.parameters = parameters;
    context.parameter_count = 1;
    action.dsa_id = 7u;
    action.state_index = 2u;
    state.imported_actions = &action;
    state.imported_action_count = 1;

    action.program_words = round_trip;
    action.program_word_count = (int)(sizeof(round_trip) / sizeof(round_trip[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 2u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && parameters[0] == 0x1234u &&
              execution.stack_depth == 0u && execution.next_state == 0,
          "CSBWin/DSA.cpp:1191-1244 EX_VARIABLESTORE/EX_VARIABLEFETCH",
          "authenticated local variable store/fetch round-trips through the source stack");

    parameters[0] = 99u;
    action.program_words = undefined_fetch;
    action.program_word_count = (int)(sizeof(undefined_fetch) / sizeof(undefined_fetch[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 2u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && parameters[0] == 0u,
          "CSBWin/DSA.cpp:141-142,227-239 DSADBANK::Var/NoValue",
          "an undefined source local fetch supplies the source zero value");

    parameters[0] = 99u;
    action.program_words = extended_state;
    action.program_word_count = (int)(sizeof(extended_state) / sizeof(extended_state[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 2u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && parameters[0] == 99u &&
              execution.next_state == -4 && execution.stack_depth == 1u &&
              execution.words_consumed ==
              action.program_word_count,
          "CSBWin/DSA.cpp:1191-1244 EX_VARIABLESTORE/EX_VARIABLEFETCH",
          "variable MAXSTATE extensions decode as signed source i16 values");

    parameters[0] = 99u;
    action.program_words = invalid_index;
    action.program_word_count = (int)(sizeof(invalid_index) / sizeof(invalid_index[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 2u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL && parameters[0] == 99u,
          "CSBWin/CSB.h:2881-2886 DSAVARS",
          "out-of-bank variable indices reject before transactional parameter commit");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_global_variable_opcode_family(void)
{
    uint16_t round_trip[] = {
        0x0686u, 0x55aau, 0x0054u, 0x0053u, 0x000du
    };
    uint16_t extended_state[] = {
        0x8053u, 0xfffcu
    };
    uint16_t absent_global[] = { 0x0093u, 0x000du };
    uint32_t parameters[] = { 99u };
    uint32_t globals[] = { 0u, 0u };
    CSB_V1_DSAImportedAction action;
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;

    memset(&action, 0, sizeof(action));
    csb_v1_chaos_init(&state);
    memset(&context, 0, sizeof(context));
    context.parameters = parameters;
    context.parameter_count = 1;
    context.global_variables = globals;
    context.global_variable_count = 2;
    action.dsa_id = 7u;
    action.state_index = 3u;
    state.imported_actions = &action;
    state.imported_action_count = 1;

    action.program_words = round_trip;
    action.program_word_count = (int)(sizeof(round_trip) / sizeof(round_trip[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 3u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && globals[1] == 0x55aau &&
              parameters[0] == 0x55aau && execution.next_state == 0,
          "CSBWin/DSA.cpp:1244-1312 EX_GLOBALSTORE/EX_GLOBALFETCH",
          "authenticated global store/fetch stages then commits the source global bank");

    parameters[0] = 99u;
    globals[1] = 0x1234u;
    action.program_words = extended_state;
    action.program_word_count = (int)(sizeof(extended_state) / sizeof(extended_state[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 3u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && parameters[0] == 99u &&
              execution.next_state == -4 && execution.stack_depth == 1u,
          "CSBWin/Data.h:2268-2295 + DSA.cpp:1279-1312",
          "global MAXSTATE extension uses the source signed i16 decode");

    parameters[0] = 99u;
    globals[1] = 0x1234u;
    action.program_words = absent_global;
    action.program_word_count = (int)(sizeof(absent_global) / sizeof(absent_global[0]));
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 3u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL && parameters[0] == 99u &&
              globals[1] == 0x1234u,
          "CSBWin/DSA.cpp:1267-1275,1302-1311",
          "out-of-bank global access rejects before either runtime-owned surface commits");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_state_column_jump_dispatch(void)
{
    uint16_t ignored[] = { 0x0686u, 1u };
    uint16_t direct_jump[] = { 0x014cu };
    uint16_t extended_jump[] = { 0x9fccu, 0xfffeu, 300u, 9u };
    uint16_t malformed_jump[] = { 0x9fccu, 0xfffeu, 300u };
    CSB_V1_DSAImportedAction actions[5];
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAJumpDispatch dispatch;

    memset(actions, 0, sizeof(actions));
    csb_v1_chaos_init(&state);
    actions[0].dsa_id = 7u;
    actions[0].state_index = 4u;
    actions[0].column = 2u;
    actions[0].program_words = ignored;
    actions[0].program_word_count = 2;
    actions[1].dsa_id = 7u;
    actions[1].state_index = 4u;
    actions[1].column = 4u;
    actions[1].program_words = direct_jump;
    actions[1].program_word_count = 1;
    actions[2].dsa_id = 7u;
    actions[2].state_index = 7u;
    actions[2].column = 3u;
    actions[2].program_words = extended_jump;
    actions[2].program_word_count = 4;
    actions[3].dsa_id = 7u;
    actions[3].state_index = 8u;
    actions[3].column = 3u;
    actions[3].program_words = malformed_jump;
    actions[3].program_word_count = 3;
    actions[4].dsa_id = 7u;
    actions[4].state_index = 4u;
    actions[4].column = 2u;
    actions[4].program_words = direct_jump;
    actions[4].program_word_count = 1;
    state.imported_actions = actions;
    state.imported_action_count = 5;

    check(csb_v1_chaos_find_imported_action_column(&state, 7, 4u, 2u) ==
              &actions[0],
          "CSBWin/DSA.cpp:5717-5740 DSAState::Program",
          "state/column lookup preserves source file-order first-match ownership");
    check(csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
              &state, 7, 4u, 2u, &dispatch) ==
              CSB_V1_CSBWIN_DSA_JUMP_NOT_JUMP,
          "CSBWin/DSA.cpp:5104-5108 Execute",
          "a selected non-JUMP action is not reinterpreted as a dispatch opcode");
    check(csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
              &state, 7, 4u, 4u, &dispatch) == CSB_V1_CSBWIN_DSA_JUMP_OK &&
              dispatch.continuation_state == 4 && dispatch.target_state == 5u &&
              dispatch.target_column == 0u && dispatch.words_consumed == 1u,
          "CSBWin/Data.h:2090-2116 + DSA.cpp:812-849 EX_JUMP",
          "compact JUMP keeps its implicit column zero and source continuation");
    check(csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
              &state, 7, 7u, 3u, &dispatch) == CSB_V1_CSBWIN_DSA_JUMP_OK &&
              dispatch.continuation_state == 5 && dispatch.target_state == 300u &&
              dispatch.target_column == 9u && dispatch.words_consumed == 4u,
          "CSBWin/Data.h:2090-2116 + DSA.cpp:812-849 EX_JUMP",
          "JUMP decodes state and column extensions in source order without execution");
    dispatch.target_state = 77u;
    check(csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
              &state, 7, 8u, 3u, &dispatch) ==
              CSB_V1_CSBWIN_DSA_JUMP_MALFORMED && dispatch.target_state == 77u,
          "CSBWin/DSA.cpp:812-849 EX_JUMP",
          "truncated JUMP rejects transactionally without publishing a partial dispatch");
    check(csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
              &state, 7, 99u, 3u, &dispatch) ==
              CSB_V1_CSBWIN_DSA_JUMP_NOT_FOUND,
          "CSBWin/DSA.cpp:5092-5105 Execute",
          "missing state/column action leaves dispatch resolution inert");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_state_column_gosub_dispatch(void)
{
    uint16_t ignored[] = { 0x0686u, 1u };
    uint16_t direct_gosub[] = { 0x0145u };
    uint16_t extended_gosub[] = { 0x9fc5u, 0xfffeu, 300u, 9u };
    uint16_t malformed_gosub[] = { 0x9fc5u, 0xfffeu, 300u };
    CSB_V1_DSAImportedAction actions[5];
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAGosubDispatch dispatch;

    memset(actions, 0, sizeof(actions));
    csb_v1_chaos_init(&state);
    actions[0].dsa_id = 7u;
    actions[0].state_index = 4u;
    actions[0].column = 2u;
    actions[0].program_words = ignored;
    actions[0].program_word_count = 2;
    actions[1].dsa_id = 7u;
    actions[1].state_index = 4u;
    actions[1].column = 4u;
    actions[1].program_words = direct_gosub;
    actions[1].program_word_count = 1;
    actions[2].dsa_id = 7u;
    actions[2].state_index = 7u;
    actions[2].column = 3u;
    actions[2].program_words = extended_gosub;
    actions[2].program_word_count = 4;
    actions[3].dsa_id = 7u;
    actions[3].state_index = 8u;
    actions[3].column = 3u;
    actions[3].program_words = malformed_gosub;
    actions[3].program_word_count = 3;
    actions[4].dsa_id = 7u;
    actions[4].state_index = 4u;
    actions[4].column = 2u;
    actions[4].program_words = direct_gosub;
    actions[4].program_word_count = 1;
    state.imported_actions = actions;
    state.imported_action_count = 5;

    check(csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
              &state, 7, 4u, 2u, &dispatch) ==
              CSB_V1_CSBWIN_DSA_GOSUB_NOT_GOSUB,
          "CSBWin/DSA.cpp:764-808 EX_GOSUB",
          "a source-selected non-GOSUB action is not reinterpreted as a subroutine");
    check(csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
              &state, 7, 4u, 4u, &dispatch) == CSB_V1_CSBWIN_DSA_GOSUB_OK &&
              dispatch.continuation_state == 4 && dispatch.target_state == 5u &&
              dispatch.target_column == 0u && dispatch.subroutine_depth_delta == 1u &&
              dispatch.words_consumed == 1u,
          "CSBWin/Data.h:2093-2119 + DSA.cpp:764-808 EX_GOSUB",
          "compact GOSUB preserves its outer continuation and implicit column zero");
    check(csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
              &state, 7, 7u, 3u, &dispatch) == CSB_V1_CSBWIN_DSA_GOSUB_OK &&
              dispatch.continuation_state == 65541 && dispatch.target_state == 300u &&
              dispatch.target_column == 9u && dispatch.subroutine_depth_delta == 1u &&
              dispatch.words_consumed == 4u,
          "CSBWin/Data.h:2093-2119 + DSA.cpp:764-808 EX_GOSUB",
          "GOSUB preserves its raw unsigned MAXSTATE extension without entering Execute");
    dispatch.target_state = 77u;
    check(csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
              &state, 7, 8u, 3u, &dispatch) ==
              CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED && dispatch.target_state == 77u,
          "CSBWin/DSA.cpp:764-808 EX_GOSUB",
          "truncated GOSUB rejects transactionally without publishing a partial receipt");
    check(csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
              &state, 7, 99u, 3u, &dispatch) ==
              CSB_V1_CSBWIN_DSA_GOSUB_NOT_FOUND,
          "CSBWin/DSA.cpp:764-808 EX_GOSUB",
          "missing source state and column leaves GOSUB resolution inert");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_execute_transfer_subset(void)
{
    uint16_t jump[] = { 0x014cu };
    uint16_t gosub[] = { 0x0185u };
    uint16_t nested_gosub[] = { 0x01c5u };
    uint16_t nested_jump[] = { 0x020cu };
    uint16_t unsupported[] = { 0x0006u };
    CSB_V1_DSAImportedAction actions[5];
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAExecuteReceipt receipt;

    memset(actions, 0, sizeof(actions));
    csb_v1_chaos_init(&state);
    actions[0].dsa_id = 7u;
    actions[0].state_index = 4u;
    actions[0].column = 2u;
    actions[0].program_words = jump;
    actions[0].program_word_count = 1;
    actions[1].dsa_id = 7u;
    actions[1].state_index = 5u;
    actions[1].column = 0u;
    actions[1].program_words = unsupported;
    actions[1].program_word_count = 1;
    actions[2].dsa_id = 7u;
    actions[2].state_index = 6u;
    actions[2].column = 0u;
    actions[2].program_words = nested_gosub;
    actions[2].program_word_count = 1;
    actions[3].dsa_id = 7u;
    actions[3].state_index = 7u;
    actions[3].column = 0u;
    actions[3].program_words = nested_jump;
    actions[3].program_word_count = 1;
    actions[4].dsa_id = 7u;
    actions[4].state_index = 5u;
    actions[4].column = 0u;
    actions[4].program_words = gosub;
    actions[4].program_word_count = 1;
    state.imported_actions = actions;
    state.imported_action_count = 4;

    receipt.final_state = 77;
    check(csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
              &state, 7, 4u, 2u, 0, &receipt) ==
              CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED && receipt.final_state == 77,
          "CSBWin/DSA.cpp:5053-5293 Execute",
          "JUMP follows the first exact target action and rejects unsupported execution transactionally");

    actions[1].program_words = gosub;
    check(csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
              &state, 7, 4u, 2u, 0, &receipt) ==
              CSB_V1_CSBWIN_DSA_EXECUTE_OK && receipt.final_state == 4 &&
              receipt.transfer_count == 4u && receipt.maximum_subroutine_depth == 2u &&
              receipt.words_consumed == 4u,
          "CSBWin/DSA.cpp:764-808,5053-5293 Execute",
          "JUMP stays in-frame while nested GOSUB frames unwind to the outer first continuation");

    actions[1].program_words = unsupported;
    receipt.final_state = 88;
    check(csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
              &state, 7, 5u, 0u, 3, &receipt) ==
              CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED && receipt.final_state == 88,
          "CSBWin/DSA.cpp:5092-5108 Execute",
          "first file-order exact state-column action remains authoritative in the continuation executor");

    actions[1].program_words = gosub;
    receipt.final_state = 99;
    check(csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
              &state, 7, 5u, 0u,
              CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH - 1,
              &receipt) == CSB_V1_CSBWIN_DSA_EXECUTE_DEPTH_LIMIT &&
              receipt.final_state == 99,
          "CSBWin/DSA.cpp:764-808 EX_GOSUB",
          "bounded continuation stack rejects a further nested Execute frame without publishing a receipt");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

static void test_csbwin_authenticated_case_opcode_family(void)
{
    /* LOAD INTEGER 7; CASE nextState=1, count=1, key=7 -> state 9/column 0.
     * The target JUMP then reaches a missing state 10 and returns state 9,
     * exactly through the existing bounded Execute transfer owner. */
    uint16_t matching_case[] = {
        0x0686u, 7u, 0x0050u, 1u, 7u, 0u, 0x0900u, 0u
    };
    uint16_t missing_case[] = {
        0x0686u, 8u, 0x0050u, 1u, 7u, 0u, 0x0900u, 0u
    };
    uint16_t truncated_case[] = { 0x0686u, 7u, 0x0050u, 1u, 7u };
    uint16_t target_jump[] = { 0x028cu };
    uint16_t unsupported_target[] = { 0x0006u };
    CSB_V1_DSAImportedAction actions[2];
    CSB_V1_ChaosMagicState state;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;
    CSB_V1_CSBWinDSACoreProgramReceipt core;

    memset(actions, 0, sizeof(actions));
    memset(&context, 0, sizeof(context));
    csb_v1_chaos_init(&state);
    actions[0].dsa_id = 7u;
    actions[0].state_index = 4u;
    actions[0].column = 0u;
    actions[0].program_words = matching_case;
    actions[0].program_word_count = (int)(sizeof(matching_case) /
                                           sizeof(matching_case[0]));
    actions[1].dsa_id = 7u;
    actions[1].state_index = 9u;
    actions[1].column = 0u;
    actions[1].program_words = target_jump;
    actions[1].program_word_count = 1;
    state.imported_actions = actions;
    state.imported_action_count = 2;

    check(csb_v1_csbwin_dsa_verify_authenticated_core_program(
              &state, 7, 4u, 0, &core) == CSB_V1_CSBWIN_DSA_CORE_OK &&
              core.valid && core.stack_core && core.conditional_core &&
              core.words_consumed == actions[0].program_word_count,
          "CSBWin/Data.h:2208-2237 + DSA.cpp:981-1025 EX_CASE",
          "CASE admits only its complete source-owned ui32 target table");
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 4u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && execution.transfer_executed &&
              execution.transfer.final_state == 9 && execution.next_state == 5 &&
              execution.words_consumed == actions[0].program_word_count &&
              execution.stack_depth == 0u,
          "CSBWin/DSA.cpp:981-1025,5053-5293",
          "CASE matches through the authenticated target JUMP without a synthetic dispatch");

    actions[0].program_words = missing_case;
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 4u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_OK && !execution.transfer_executed &&
              execution.next_state == 1 && execution.stack_depth == 0u,
          "CSBWin/DSA.cpp:981-1025 EX_CASE",
          "CASE miss keeps the source relative NextState and does not dispatch a target");

    actions[0].program_words = matching_case;
    actions[1].program_words = unsupported_target;
    check(csb_v1_csbwin_dsa_execute_authenticated_stack_action(
              &state, 7, 4u, 0, &context, &execution) ==
              CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED,
          "CSBWin/DSA.cpp:981-1025,5053-5293",
          "CASE rejects an authenticated but unreviewed target opcode without dispatch");
    actions[1].program_words = target_jump;

    actions[0].program_words = truncated_case;
    actions[0].program_word_count = (int)(sizeof(truncated_case) /
                                           sizeof(truncated_case[0]));
    check(csb_v1_csbwin_dsa_verify_authenticated_core_program(
              &state, 7, 4u, 0, &core) == CSB_V1_CSBWIN_DSA_CORE_MALFORMED &&
              csb_v1_csbwin_dsa_execute_authenticated_stack_action(
                  &state, 7, 4u, 0, &context, &execution) ==
                  CSB_V1_CSBWIN_DSA_STACK_MALFORMED,
          "CSBWin/DSA.cpp:981-1025 EX_CASE",
          "truncated CASE table rejects before target dispatch or publication");

    state.imported_actions = NULL;
    state.imported_action_count = 0;
    csb_v1_chaos_cleanup(&state);
}

int main(void)
{
    printf("=== CSB V1 DSA Trigger Single Step Gate ===\n");
    printf("probe=csb_v1_dsa_trigger_single_step\n");
    printf("sourceEvidence=%s\n\n", csb_v1_chaos_source_evidence());

    test_single_step_set_flips_flag();
    test_loader_owns_validated_script_words();
    test_loader_rejects_malformed_table_without_mutation();
    test_single_step_toggle_flips_flag();
    test_single_step_end_deactivates_script();
    test_single_step_test_unset_flag_no_jump();
    test_single_step_test_set_flag_jumps_to_target();
    test_single_step_guards();
    test_malformed_target_operands_reject_cleanly();
    test_trigger_out_of_range_script_id_rejects_cleanly();
    test_single_step_message_records_dispatch();
    test_source_evidence_anchors();
    test_csbwin_load_opcode_family();
    test_csbwin_load_store_rejects_unowned_action();
    test_csbwin_authenticated_stack_opcode_family();
    test_csbwin_setskin_waits_for_complete_action();
    test_csbwin_authenticated_filter_stack_runner();
    test_csbwin_runtime_filter_adapter();
    test_csbwin_authenticated_local_variable_opcode_family();
    test_csbwin_authenticated_global_variable_opcode_family();
    test_csbwin_authenticated_state_column_jump_dispatch();
    test_csbwin_authenticated_state_column_gosub_dispatch();
    test_csbwin_authenticated_execute_transfer_subset();
    test_csbwin_authenticated_case_opcode_family();

    printf("\nassertions=%d failures=%d\n", g_assertions, g_failures);
    if (g_failures == 0) {
        printf("PASS csb_v1_dsa_trigger_single_step assertions=%d failures=0\n",
               g_assertions);
        return 0;
    }
    return 1;
}
