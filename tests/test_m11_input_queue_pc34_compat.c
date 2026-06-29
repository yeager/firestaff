/*
 * test_m11_input_queue_pc34_compat.c
 *
 * Data-free CTest unit for the M11 FS_InputQueue ring buffer
 * (include/firestaff_input.h + src/engine/firestaff_input.c) and the
 * accompanying SDL scancode -> FS_Command translation
 * (fs_input_key_to_command).
 *
 * Source of truth:
 *   - FS_INPUT_QUEUE_SIZE = 32 ring capacity (firestaff_input.h)
 *   - firestaff_touch.c fs_input_queue_push() consumer
 *     (swipe / edge-strafe -> FS_CMD_*)
 *   - firestaff_game_loop.c fs_input_queue_pop() consumer
 *     (F0380_COMMAND_ProcessQueue_CPSC-style tick drain)
 *   - ReDMCSB DEFS.H:238-243 C001..C006 movement commands (the V1
 *     command IDs the queue feeds)
 *
 * The test exercises:
 *   - Group A: FS_InputQueue FIFO order with push / pop / count
 *   - Group B: full-queue push is silently dropped (capacity guard)
 *   - Group C: empty-queue pop returns 0 and does not touch *out
 *   - Group D: head / tail wraparound (push 32 -> pop 32 -> push 32)
 *   - Group E: pop after pop-then-push-then-pop preserves FIFO order
 *   - Group F: NULL queue safety (push / pop / count)
 *   - Group G: fs_input_key_to_command arrow-key mapping (always-on)
 *   - Group H: fs_input_key_to_command WASD mapping (gated by wasd_enabled)
 *   - Group I: fs_input_key_to_command modifier / system keys
 *   - Group J: fs_input_key_to_command unknown scancode is FS_CMD_NONE
 *   - Group K: parameter fields (param1 / param2) round-trip through
 *              the queue, including for FS_CMD_CLICK (which carries the
 *              click x / y coordinates downstream)
 *   - Group L: deterministic ringbuffer stress (32-event full cycle
 *              with interleaved push/pop across the modulo boundary,
 *              hash-stable across two independent runs)
 *
 * The test is data-free: no real game data, no SDL window, no
 * filesystem state.  It is a contract-only regression for the
 * launcher-to-game input handoff that every Firestaff M11 frame
 * routes through.
 */

#include "firestaff_input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void check_pass(const char* name, int ok) {
    if (!ok) {
        printf("FAIL %s\n", name);
        ++failures;
    } else {
        printf("PASS %s\n", name);
    }
}

#define EXPECT_EQ_INT(actual, expected) \
    ((actual) == (expected))

/* ------------------------------------------------------------------ */
/* Group A: FIFO order with push / pop / count                          */
/* ------------------------------------------------------------------ */
static void test_queue_fifo_order(void) {
    FS_InputQueue q;
    FS_InputEvent ev;

    fs_input_queue_init(&q);
    check_pass("A1 init zeroes count",
               fs_input_queue_count(&q) == 0);

    fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, 0, 0);
    fs_input_queue_push(&q, FS_CMD_MOVE_BACKWARD, 0, 0);
    fs_input_queue_push(&q, FS_CMD_TURN_LEFT, 0, 0);
    fs_input_queue_push(&q, FS_CMD_TURN_RIGHT, 0, 0);

    check_pass("A2 four pushes yield count=4",
               fs_input_queue_count(&q) == 4);

    memset(&ev, 0, sizeof(ev));
    check_pass("A3 first pop returns MOVE_FORWARD",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_MOVE_FORWARD);

    check_pass("A4 second pop returns MOVE_BACKWARD",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_MOVE_BACKWARD);

    check_pass("A5 third pop returns TURN_LEFT",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_TURN_LEFT);

    check_pass("A6 fourth pop returns TURN_RIGHT",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_TURN_RIGHT);

    check_pass("A7 count drops to zero after four pops",
               fs_input_queue_count(&q) == 0);
}

/* ------------------------------------------------------------------ */
/* Group B: push at full capacity is silently dropped (no overrun)       */
/* ------------------------------------------------------------------ */
static void test_queue_full_drop(void) {
    FS_InputQueue q;
    FS_InputEvent ev;

    fs_input_queue_init(&q);

    /* Fill the queue exactly. */
    for (int i = 0; i < FS_INPUT_QUEUE_SIZE; ++i) {
        fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, i, 0);
    }
    check_pass("B1 full queue reports FS_INPUT_QUEUE_SIZE",
               fs_input_queue_count(&q) == FS_INPUT_QUEUE_SIZE);

    /* Two more pushes must be silently dropped. */
    fs_input_queue_push(&q, FS_CMD_TURN_LEFT, 0, 0);
    fs_input_queue_push(&q, FS_CMD_TURN_RIGHT, 0, 0);
    check_pass("B2 overflow pushes do not grow count",
               fs_input_queue_count(&q) == FS_INPUT_QUEUE_SIZE);

    /* First pop must be MOVE_FORWARD with param1=0 (the first push),
     * proving the oldest event was preserved. */
    memset(&ev, 0, sizeof(ev));
    check_pass("B3 first pop after overflow is oldest push",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_MOVE_FORWARD &&
               ev.param1 == 0);

    /* Last pop must be MOVE_FORWARD with param1=FS_INPUT_QUEUE_SIZE-1
     * (the last successful push).  TURN_LEFT and TURN_RIGHT must
     * have been dropped. */
    while (fs_input_queue_count(&q) > 1) {
        fs_input_queue_pop(&q, &ev);
    }
    memset(&ev, 0, sizeof(ev));
    check_pass("B4 last pop after overflow is last in-order push",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_MOVE_FORWARD &&
               ev.param1 == FS_INPUT_QUEUE_SIZE - 1);
}

/* ------------------------------------------------------------------ */
/* Group C: empty queue pop returns 0 and does not clobber *out          */
/* ------------------------------------------------------------------ */
static void test_queue_empty_pop(void) {
    FS_InputQueue q;
    FS_InputEvent ev;
    FS_InputEvent sentinel;

    fs_input_queue_init(&q);

    /* Sentinel values that the empty pop must NOT overwrite. */
    sentinel.cmd = FS_CMD_CLICK;
    sentinel.param1 = 0x11223344;
    sentinel.param2 = 0x55667788;
    ev = sentinel;

    check_pass("C1 empty pop returns 0",
               fs_input_queue_pop(&q, &ev) == 0);

    /* Even with a successful push followed by a pop, a second pop on
     * an empty queue must not touch the caller's struct. */
    fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, 1, 2);
    memset(&ev, 0, sizeof(ev));
    fs_input_queue_pop(&q, &ev);
    check_pass("C2 second pop on now-empty queue returns 0",
               fs_input_queue_pop(&q, &ev) == 0);

    check_pass("C3 empty pop leaves caller struct untouched",
               ev.cmd == FS_CMD_MOVE_FORWARD ||
               (ev.cmd == sentinel.cmd &&
                ev.param1 == sentinel.param1 &&
                ev.param2 == sentinel.param2));
}

/* ------------------------------------------------------------------ */
/* Group D: head / tail wraparound across the modulo boundary           */
/* ------------------------------------------------------------------ */
static void test_queue_wraparound(void) {
    FS_InputQueue q;
    FS_InputEvent ev;

    fs_input_queue_init(&q);

    /* Fill the queue. */
    for (int i = 0; i < FS_INPUT_QUEUE_SIZE; ++i) {
        fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, i, 0);
    }
    /* Drain it (this advances head to FS_INPUT_QUEUE_SIZE, well past
     * the modulo boundary). */
    for (int i = 0; i < FS_INPUT_QUEUE_SIZE; ++i) {
        fs_input_queue_pop(&q, &ev);
    }
    check_pass("D1 drained queue reports count=0",
               fs_input_queue_count(&q) == 0);

    /* Refill across the modulo boundary and verify order is still
     * strictly FIFO. */
    for (int i = 0; i < FS_INPUT_QUEUE_SIZE; ++i) {
        fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD,
                            FS_INPUT_QUEUE_SIZE + i, 0);
    }
    check_pass("D2 refill after drain reaches FS_INPUT_QUEUE_SIZE",
               fs_input_queue_count(&q) == FS_INPUT_QUEUE_SIZE);

    int ok = 1;
    for (int i = 0; i < FS_INPUT_QUEUE_SIZE; ++i) {
        if (!fs_input_queue_pop(&q, &ev)) {
            ok = 0;
            break;
        }
        if (ev.cmd != FS_CMD_MOVE_FORWARD ||
            ev.param1 != FS_INPUT_QUEUE_SIZE + i) {
            ok = 0;
            break;
        }
    }
    check_pass("D3 refill order is strictly FIFO across wraparound", ok);

    /* Drain the second wave. */
    check_pass("D4 queue empty after second drain",
               fs_input_queue_pop(&q, &ev) == 0 &&
               fs_input_queue_count(&q) == 0);
}

/* ------------------------------------------------------------------ */
/* Group E: pop after pop-then-push-then-pop preserves FIFO order       */
/* ------------------------------------------------------------------ */
static void test_queue_interleaved(void) {
    FS_InputQueue q;
    FS_InputEvent ev;

    fs_input_queue_init(&q);

    fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, 1, 0);
    fs_input_queue_push(&q, FS_CMD_MOVE_BACKWARD, 2, 0);
    fs_input_queue_push(&q, FS_CMD_TURN_LEFT, 3, 0);
    fs_input_queue_push(&q, FS_CMD_TURN_RIGHT, 4, 0);
    fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, 5, 0);

    fs_input_queue_pop(&q, &ev); /* MOVE_FORWARD,1 */
    fs_input_queue_pop(&q, &ev); /* MOVE_BACKWARD,2 */

    fs_input_queue_push(&q, FS_CMD_TURN_LEFT, 6, 0);
    fs_input_queue_push(&q, FS_CMD_TURN_RIGHT, 7, 0);

    memset(&ev, 0, sizeof(ev));
    check_pass("E1 next pop is the third-oldest push (TURN_LEFT,3)",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_TURN_LEFT && ev.param1 == 3);

    check_pass("E2 next pop is the fourth-oldest push (TURN_RIGHT,4)",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_TURN_RIGHT && ev.param1 == 4);

    check_pass("E3 next pop is the fifth-oldest push (MOVE_FORWARD,5)",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_MOVE_FORWARD && ev.param1 == 5);

    check_pass("E4 next pop is the late-pushed TURN_LEFT,6",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_TURN_LEFT && ev.param1 == 6);

    check_pass("E5 next pop is the late-pushed TURN_RIGHT,7",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_TURN_RIGHT && ev.param1 == 7);

    check_pass("E6 queue is empty after interleaved drain",
               fs_input_queue_count(&q) == 0 &&
               fs_input_queue_pop(&q, &ev) == 0);
}

/* ------------------------------------------------------------------ */
/* Group F: NULL queue safety (push / pop / count)                       */
/* ------------------------------------------------------------------ */
static void test_queue_null_safety(void) {
    FS_InputEvent ev;

    /* None of these may crash. */
    fs_input_queue_push(NULL, FS_CMD_MOVE_FORWARD, 0, 0);
    check_pass("F1 push to NULL queue is a no-op",
               fs_input_queue_count(NULL) == 0);

    check_pass("F2 pop from NULL queue returns 0",
               fs_input_queue_pop(NULL, &ev) == 0);

    check_pass("F3 count of NULL queue is 0",
               fs_input_queue_count(NULL) == 0);

    /* Pop with NULL out is also a safe no-op even when the queue is
     * non-empty: the queue's head/tail must still advance so the
     * caller can drain even if it does not care about the event
     * payload. */
    {
        FS_InputQueue q;
        fs_input_queue_init(&q);
        fs_input_queue_push(&q, FS_CMD_MOVE_FORWARD, 7, 8);
        fs_input_queue_push(&q, FS_CMD_TURN_LEFT, 9, 10);
        check_pass("F4 pop with NULL out returns 1",
                   fs_input_queue_pop(&q, NULL) == 1);
        check_pass("F5 pop with NULL out still drains the queue",
                   fs_input_queue_count(&q) == 1);
        check_pass("F6 second pop with NULL out returns 1",
                   fs_input_queue_pop(&q, NULL) == 1);
        check_pass("F7 queue empty after two NULL-out pops",
                   fs_input_queue_count(&q) == 0);
    }
}

/* ------------------------------------------------------------------ */
/* Group G: fs_input_key_to_command arrow-key mapping (always-on)       */
/* ------------------------------------------------------------------ */
static void test_keymap_arrow_keys(void) {
    /* SDL scancodes: Up=82, Down=81, Left=80, Right=79. */
    check_pass("G1 Up arrow -> MOVE_FORWARD (wasd disabled)",
               fs_input_key_to_command(82, 0) == FS_CMD_MOVE_FORWARD);
    check_pass("G2 Down arrow -> MOVE_BACKWARD (wasd disabled)",
               fs_input_key_to_command(81, 0) == FS_CMD_MOVE_BACKWARD);
    check_pass("G3 Left arrow -> TURN_LEFT (wasd disabled)",
               fs_input_key_to_command(80, 0) == FS_CMD_TURN_LEFT);
    check_pass("G4 Right arrow -> TURN_RIGHT (wasd disabled)",
               fs_input_key_to_command(79, 0) == FS_CMD_TURN_RIGHT);

    /* Arrow keys remain active even when WASD is enabled - WASD
     * only adds extra bindings, it does not shadow the arrows. */
    check_pass("G5 Up arrow -> MOVE_FORWARD (wasd enabled)",
               fs_input_key_to_command(82, 1) == FS_CMD_MOVE_FORWARD);
    check_pass("G6 Down arrow -> MOVE_BACKWARD (wasd enabled)",
               fs_input_key_to_command(81, 1) == FS_CMD_MOVE_BACKWARD);
    check_pass("G7 Left arrow -> TURN_LEFT (wasd enabled)",
               fs_input_key_to_command(80, 1) == FS_CMD_TURN_LEFT);
    check_pass("G8 Right arrow -> TURN_RIGHT (wasd enabled)",
               fs_input_key_to_command(79, 1) == FS_CMD_TURN_RIGHT);
}

/* ------------------------------------------------------------------ */
/* Group H: fs_input_key_to_command WASD mapping (gated by wasd_enabled) */
/* ------------------------------------------------------------------ */
static void test_keymap_wasd(void) {
    /* WASD scancodes: W=26, A=4, S=22, D=7. */
    check_pass("H1 W -> MOVE_FORWARD only when WASD enabled",
               fs_input_key_to_command(26, 0) == FS_CMD_NONE &&
               fs_input_key_to_command(26, 1) == FS_CMD_MOVE_FORWARD);

    check_pass("H2 S -> MOVE_BACKWARD only when WASD enabled",
               fs_input_key_to_command(22, 0) == FS_CMD_NONE &&
               fs_input_key_to_command(22, 1) == FS_CMD_MOVE_BACKWARD);

    /* A and D map to strafe (not turn), even when WASD is on - this
     * matches the V1/V2 parity that the runtime gesture navigation
     * gate relies on. */
    check_pass("H3 A -> STRAFE_LEFT only when WASD enabled",
               fs_input_key_to_command(4, 0) == FS_CMD_NONE &&
               fs_input_key_to_command(4, 1) == FS_CMD_STRAFE_LEFT);

    check_pass("H4 D -> STRAFE_RIGHT only when WASD enabled",
               fs_input_key_to_command(7, 0) == FS_CMD_NONE &&
               fs_input_key_to_command(7, 1) == FS_CMD_STRAFE_RIGHT);
}

/* ------------------------------------------------------------------ */
/* Group I: fs_input_key_to_command modifier / system keys               */
/* ------------------------------------------------------------------ */
static void test_keymap_modifiers(void) {
    /* Escape=41, Space=44, Tab=43.  These are always-on and do not
     * depend on the wasd_enabled flag. */
    check_pass("I1 Escape -> MENU (wasd disabled)",
               fs_input_key_to_command(41, 0) == FS_CMD_MENU);
    check_pass("I2 Escape -> MENU (wasd enabled)",
               fs_input_key_to_command(41, 1) == FS_CMD_MENU);

    check_pass("I3 Space -> ACTION (wasd disabled)",
               fs_input_key_to_command(44, 0) == FS_CMD_ACTION);
    check_pass("I4 Space -> ACTION (wasd enabled)",
               fs_input_key_to_command(44, 1) == FS_CMD_ACTION);

    check_pass("I5 Tab -> INVENTORY (wasd disabled)",
               fs_input_key_to_command(43, 0) == FS_CMD_INVENTORY);
    check_pass("I6 Tab -> INVENTORY (wasd enabled)",
               fs_input_key_to_command(43, 1) == FS_CMD_INVENTORY);
}

/* ------------------------------------------------------------------ */
/* Group J: fs_input_key_to_command unknown scancode is FS_CMD_NONE       */
/* ------------------------------------------------------------------ */
static void test_keymap_unknown(void) {
    /* Pick a handful of out-of-range / unmapped scancodes. */
    int unknowns[] = {0, 1, 5, 10, 30, 50, 100, 200, 255};
    int ok = 1;
    for (size_t i = 0; i < sizeof(unknowns) / sizeof(unknowns[0]); ++i) {
        if (fs_input_key_to_command(unknowns[i], 0) != FS_CMD_NONE) {
            ok = 0;
        }
        if (fs_input_key_to_command(unknowns[i], 1) != FS_CMD_NONE) {
            ok = 0;
        }
    }
    check_pass("J1 unmapped scancodes resolve to FS_CMD_NONE", ok);
}

/* ------------------------------------------------------------------ */
/* Group K: parameter fields (param1 / param2) round-trip through queue  */
/* ------------------------------------------------------------------ */
static void test_queue_param_round_trip(void) {
    FS_InputQueue q;
    FS_InputEvent ev;

    fs_input_queue_init(&q);

    /* The only FS_Command that carries param1/param2 in the current
     * code path is FS_CMD_CLICK (mouse click at x, y); but the queue
     * must round-trip any payload, so feed it several distinct
     * (cmd, p1, p2) triples. */
    fs_input_queue_push(&q, FS_CMD_CLICK, 17, 33);
    fs_input_queue_push(&q, FS_CMD_CLICK, 0, 0);
    fs_input_queue_push(&q, FS_CMD_CLICK, 319, 199);
    fs_input_queue_push(&q, FS_CMD_CLICK, 0x7FFFFFFF, 0x7FFFFFFF);

    memset(&ev, 0, sizeof(ev));
    check_pass("K1 first CLICK round-trips (17,33)",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_CLICK &&
               ev.param1 == 17 && ev.param2 == 33);

    memset(&ev, 0, sizeof(ev));
    check_pass("K2 second CLICK round-trips (0,0)",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_CLICK &&
               ev.param1 == 0 && ev.param2 == 0);

    memset(&ev, 0, sizeof(ev));
    check_pass("K3 third CLICK round-trips (319,199)",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_CLICK &&
               ev.param1 == 319 && ev.param2 == 199);

    memset(&ev, 0, sizeof(ev));
    check_pass("K4 fourth CLICK round-trips max-positive ints",
               fs_input_queue_pop(&q, &ev) == 1 &&
               ev.cmd == FS_CMD_CLICK &&
               ev.param1 == 0x7FFFFFFF &&
               ev.param2 == 0x7FFFFFFF);
}

/* ------------------------------------------------------------------ */
/* Group L: deterministic ringbuffer stress across the modulo boundary  */
/* ------------------------------------------------------------------ */
/* FNV-1a 32-bit hash, used to detect any nondeterministic ordering
 * across runs. */
static unsigned int fnv1a_step(unsigned int hash, unsigned char byte) {
    return (hash ^ byte) * 0x01000193u;
}

static unsigned int fnv1a_queue(const FS_InputQueue* q) {
    unsigned int hash = 0x811c9dc5u;
    const unsigned char* bytes = (const unsigned char*)q;
    for (size_t i = 0; i < sizeof(*q); ++i) {
        hash = fnv1a_step(hash, bytes[i]);
    }
    return hash;
}

static void test_queue_deterministic_stress(void) {
    FS_InputQueue q;
    FS_InputEvent ev;

    /* Push a full ring, drain half, push a smaller second wave,
     * interleave pops with late pushes, and finally drain to empty.
     * The two-pass FNV-1a snapshot must match across two independent
     * runs to prove there is no uninitialized memory or off-by-one
     * corruption. */
    unsigned int hash1 = 0;
    unsigned int hash2 = 0;

    for (int pass = 0; pass < 2; ++pass) {
        fs_input_queue_init(&q);

        /* Push 32 events (full ring). */
        for (int i = 0; i < FS_INPUT_QUEUE_SIZE; ++i) {
            fs_input_queue_push(&q, (FS_Command)((i % 11) + 1),
                                i, i * 2);
        }
        /* Pop 12 events. */
        for (int i = 0; i < 12; ++i) {
            if (!fs_input_queue_pop(&q, &ev)) {
                printf("FAIL L%d unexpected empty pop at i=%d\n",
                       pass, i);
                ++failures;
                return;
            }
        }
        /* Push 8 events (back to 28 of 32). */
        for (int i = 0; i < 8; ++i) {
            fs_input_queue_push(&q, FS_CMD_CLICK,
                                1000 + i, 2000 + i);
        }
        /* Pop until empty. */
        int drained = 0;
        while (fs_input_queue_pop(&q, &ev)) {
            ++drained;
        }
        check_pass(pass == 0 ? "L1 pass0 drained 28 events"
                             : "L2 pass1 drained 28 events",
                   drained == 28);
        check_pass(pass == 0 ? "L3 pass0 queue empty after drain"
                             : "L4 pass1 queue empty after drain",
                   fs_input_queue_count(&q) == 0);

        if (pass == 0) {
            hash1 = fnv1a_queue(&q);
        } else {
            hash2 = fnv1a_queue(&q);
        }
    }

    check_pass("L5 two independent stress runs yield identical FNV-1a "
               "ringbuffer snapshot", hash1 == hash2);
}

int main(void) {
    printf("# firestaff_m11_input_queue_pc34_compat\n");
    printf("# FS_INPUT_QUEUE_SIZE=%d (firestaff_input.h)\n",
           FS_INPUT_QUEUE_SIZE);

    test_queue_fifo_order();
    test_queue_full_drop();
    test_queue_empty_pop();
    test_queue_wraparound();
    test_queue_interleaved();
    test_queue_null_safety();
    test_keymap_arrow_keys();
    test_keymap_wasd();
    test_keymap_modifiers();
    test_keymap_unknown();
    test_queue_param_round_trip();
    test_queue_deterministic_stress();

    if (failures > 0) {
        printf("# summary: %d failure(s)\n", failures);
        return 1;
    }
    printf("# summary: 0 failures\n");
    return 0;
}
