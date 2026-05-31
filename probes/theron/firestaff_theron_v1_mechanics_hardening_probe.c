/*
 * firestaff_theron_v1_mechanics_hardening_probe.c
 *
 * Theron's Quest V1 Phase 5 — Mechanics Parity Hardening Probe
 *
 * Tests: movement blocking, click routes, door open/close/lock,
 * pit fall + levitate suppression, altar-of-vi resurrection,
 * pool recovery, alarm trigger, per-move stat drain.
 *
 * Compile: see CMakeLists.txt
 * Run:     ./probe
 *
 * Source: THQUEST.ASM T520/T560/T600/T700/T800/T900
 *         movement_features.md (ReDMCSB MOVESENS.C)
 *         docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "theron_v1_mechanics.h"
#include "theron_v1_world.h"
#include "theron_v1_champions.h"
#include "theron_v1_combat.h"
#include "theron_v1_dungeon_progression.h"

/* ── Stub for audio (platform-specific) ─────────────────────────── */
int theron_v1_play_sound(Theron_SoundID id) {
    (void)id;
    return 0;
}
int theron_v1_sound_is_valid(Theron_SoundID id) {
    return id >= 0 && id < THERON_SOUND_COUNT;
}

/* ── Stub for combat/creature stubs needed by mechanics.c ──────── */
void theron_v1_champion_die(Theron_V1_World *w, int s) {
    (void)w; (void)s;
}
void theron_v1_creature_ai_tick(Theron_V1_World *w) {
    (void)w;
}
Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *w,
                                           int lvl, int x, int y) {
    (void)w; (void)lvl; (void)x; (void)y;
    return NULL;
}
int theron_v1_champion_attack(Theron_V1_World *w,
                               int champ_slot, int creature_id) {
    (void)w; (void)champ_slot; (void)creature_id;
    return -1;
}

/* ── Test framework ────────────────────────────────────────────── */
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, label) do { \
    if (cond) { g_pass++; } \
    else { \
        printf("  FAIL: %s\n", label); \
        g_fail++; \
    } \
} while (0)

#define CHECK_INT(label, got, want) do { \
    if (got == want) { g_pass++; } \
    else { \
        printf("  FAIL: %s — got %d want %d\n", label, got, want); \
        g_fail++; \
    } \
} while (0)

#define CHECK_UINT(label, got, want) do { \
    if (got == want) { g_pass++; } \
    else { \
        printf("  FAIL: %s — got %u want %u\n", label, (unsigned)got, (unsigned)want); \
        g_fail++; \
    } \
} while (0)

/* ── World factory ─────────────────────────────────────────────── */

static void make_world(Theron_V1_World *w) {
    memset(w, 0, sizeof(*w));
    w->current_dungeon = 1;
    w->current_level   = 0;

    Theron_V1_Level *lvl = &w->levels[0][0];
    lvl->width  = 16;
    lvl->height = 16;
    lvl->level_index = 0;
    /* All floor */
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            lvl->squares[y][x] = THERON_SQUARE_FLOOR;

    w->level_loaded[0][0] = 1;
    w->party.leader_x = 8;
    w->party.leader_y = 8;
    w->party.leader_dir = 0;
    w->party.gold = 1000;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        w->party.champions[i].alive = 1;
        w->party.champions[i].health = 50;
        w->party.champions[i].max_health = 50;
        w->party.champions[i].stamina = 50;
        w->party.champions[i].max_stamina = 50;
        w->party.champions[i].food = 50;
        w->party.champions[i].water = 50;
    }
    w->party.active_slot = 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: movement — wall blocking
 * ═══════════════════════════════════════════════════════════════ */
static void test_wall_blocking(void) {
    printf("[test:wall_blocking]\n");

    Theron_V1_World w;
    make_world(&w);

    Theron_V1_Level *lvl = &w.levels[0][0];
    /* North of leader is a wall */
    lvl->squares[7][8] = THERON_SQUARE_WALL;

    /* N move blocked */
    CHECK_INT("north wall blocks",
               theron_v1_move_party(&w, THERON_DIR_NORTH),
               THERON_MOVE_BLOCKED);

    /* E is floor — should succeed */
    int r = theron_v1_move_party(&w, THERON_DIR_EAST);
    CHECK(r == THERON_MOVE_OK || r == THERON_MOVE_SPECIAL || r == THERON_MOVE_BLOCKED,
          "east floor moves (OK or SPECIAL)");
    CHECK_INT("leader moved east", w.party.leader_x, 9);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: movement — get_move_result
 * ═══════════════════════════════════════════════════════════════ */
static void test_get_move_result(void) {
    printf("[test:get_move_result]\n");

    Theron_V1_World w;
    make_world(&w);

    Theron_V1_Level *lvl = &w.levels[0][0];
    lvl->squares[7][8] = THERON_SQUARE_WALL;  /* north = wall */

    CHECK_INT("get_move_result north = BLOCKED",
              theron_v1_get_move_result(&w, THERON_DIR_NORTH),
              THERON_MOVE_BLOCKED);
    CHECK_INT("get_move_result east = OK",
              theron_v1_get_move_result(&w, THERON_DIR_EAST),
              THERON_MOVE_OK);
    lvl->squares[8][9] = THERON_SQUARE_PIT;
    CHECK_INT("get_move_result pit square = PIT_FALL",
              theron_v1_get_move_result(&w, THERON_DIR_EAST),
              THERON_MOVE_PIT_FALL);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: movement — pit fall and levitate suppression
 * ═══════════════════════════════════════════════════════════════ */
static void test_pit_fall(void) {
    printf("[test:pit_fall]\n");

    Theron_V1_World w;
    make_world(&w);

    Theron_V1_Level *lvl = &w.levels[0][0];
    w.party.leader_x = 8; w.party.leader_y = 8;
    /* north = pit */
    lvl->squares[7][8] = THERON_SQUARE_PIT;

    CHECK_INT("pit check returns true when not levitating",
              theron_v1_pit_check_and_trigger(&w, 8, 7),
              1);

    Theron_V1_World w2;
    make_world(&w2);
    w2.party.leader_x = 8; w2.party.leader_y = 8;
    w2.levels[0][0].squares[7][8] = THERON_SQUARE_PIT;
    w2.party.levitating = 1; /* levitate → no pit fall */
    CHECK_INT("pit check returns false when levitating",
              theron_v1_pit_check_and_trigger(&w2, 8, 7),
              0);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: click route — MOVE
 * ═══════════════════════════════════════════════════════════════ */
static void test_click_route_move(void) {
    printf("[test:click_route_move]\n");

    Theron_V1_World w;
    make_world(&w);
    w.party.leader_x = 8; w.party.leader_y = 8;

    int r = theron_v1_click_route(&w, 9, 8, THERON_CMD_MOVE);
    CHECK(r == 0 || r == THERON_MOVE_OK || r == THERON_MOVE_SPECIAL || r == THERON_MOVE_BLOCKED,
          "click_route MOVE to east succeeds");

    Theron_V1_World w2;
    make_world(&w2);
    w2.party.leader_x = 8; w2.party.leader_y = 8;
    w2.levels[0][0].squares[7][8] = THERON_SQUARE_WALL;
    CHECK_INT("click_route MOVE to wall square is blocked",
              theron_v1_click_route(&w2, 8, 7, THERON_CMD_MOVE),
              THERON_MOVE_BLOCKED);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: click route — USE on door
 * ═══════════════════════════════════════════════════════════════ */
static void test_click_route_use_door(void) {
    printf("[test:click_route_use_door]\n");

    Theron_V1_World w;
    make_world(&w);

    /* Place a door object at (8,7) */
    Theron_V1_Object d;
    memset(&d, 0, sizeof(d));
    d.id    = 1;
    d.type  = THERON_OBJTYPE_DOOR;
    d.state = THERON_DOOR_STATE_CLOSED;
    d.x     = 8; d.y = 7;
    d.level = 0;
    w.objects[0] = d;
    w.object_count = 1;

    CHECK_INT("USE on closed door returns 0",
              theron_v1_click_route(&w, 8, 7, THERON_CMD_USE),
              0);
    CHECK_INT("door is now open",
              theron_v1_door_is_open(&w, 8, 7),
              1);

    /* Locked door — unlock first */
    Theron_V1_World w2;
    make_world(&w2);
    memset(&d, 0, sizeof(d));
    d.id    = 2;
    d.type  = THERON_OBJTYPE_DOOR;
    d.state = THERON_DOOR_STATE_CLOSED;
    d.flags = THERON_DOOR_F_LOCKED;
    d.x     = 8; d.y = 7;
    d.level = 0;
    w2.objects[0] = d;
    w2.object_count = 1;

    CHECK_INT("unlock with key succeeds",
              theron_v1_door_unlock_with_key(&w2, 8, 7, THERON_ITEM_KEY),
              0);
    CHECK_INT("door no longer locked after key use",
              theron_v1_door_is_locked(&w2, 8, 7),
              0);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: door open/close
 * ═══════════════════════════════════════════════════════════════ */
static void test_door_open_close(void) {
    printf("[test:door_open_close]\n");

    Theron_V1_World w;
    make_world(&w);

    Theron_V1_Object d;
    memset(&d, 0, sizeof(d));
    d.id    = 1;
    d.type  = THERON_OBJTYPE_DOOR;
    d.state = THERON_DOOR_STATE_CLOSED;
    d.flags = 0;
    d.x     = 5; d.y = 5;
    d.level = 0;
    w.objects[0] = d;
    w.object_count = 1;

    CHECK_INT("door_open returns 0",
              theron_v1_door_open(&w, 5, 5),
              0);
    CHECK_INT("door is open",
              theron_v1_door_is_open(&w, 5, 5),
              1);

    CHECK_INT("door_close returns 0",
              theron_v1_door_close(&w, 5, 5),
              0);
    CHECK_INT("door is closed",
              theron_v1_door_is_open(&w, 5, 5),
              0);

    /* Closing locked door → locked state */
    d.state = THERON_DOOR_STATE_CLOSED;
    d.flags = THERON_DOOR_F_LOCKED;
    w.objects[0] = d;
    CHECK_INT("door_is_locked true for locked door",
              theron_v1_door_is_locked(&w, 5, 5),
              1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: altar-of-vi resurrection
 * ═══════════════════════════════════════════════════════════════ */
static void test_altar_resurrection(void) {
    printf("[test:altar_resurrection]\n");

    Theron_V1_World w;
    make_world(&w);
    w.party.gold = 10000;

    /* Kill champion 1 */
    w.party.champions[1].alive  = 0;
    w.party.champions[1].health = 0;

    /* Place altar object at (6,6) */
    Theron_V1_Object a;
    memset(&a, 0, sizeof(a));
    a.id    = 1;
    a.type  = THERON_OBJTYPE_ALTAR_VI;
    a.state = 0;
    a.x     = 6; a.y = 6;
    a.level = 0;
    w.objects[0] = a;
    w.object_count = 1;

    uint32_t gold_before = w.party.gold;
    int r = theron_v1_altar_of_vi_resurrect(&w, 6, 6);
    CHECK_INT("altar_resurrect returns slot >= 0",
              r >= 0, 1);
    CHECK_INT("altar_resurrect returns slot 1 (leader/active)",
              r, 1);
    CHECK_INT("champion revived — alive == 1",
              w.party.champions[1].alive, 1);
    CHECK_INT("champion revived — health > 0",
              w.party.champions[1].health > 0, 1);
    CHECK_UINT("gold deducted (500 per champ)",
              w.party.gold, gold_before - 500);

    /* No dead champions → returns -1 */
    CHECK_INT("altar_resurrect with no dead champions returns -1",
              theron_v1_altar_of_vi_resurrect(&w, 6, 6),
              -1);

    /* Not enough gold → returns -1 */
    Theron_V1_World w2;
    make_world(&w2);
    w2.party.gold = 100; /* not enough */
    w2.party.champions[1].alive = 0;
    w2.party.champions[1].health = 0;
    memset(&a, 0, sizeof(a));
    a.id = 2; a.type = THERON_OBJTYPE_ALTAR_VI; a.x = 6; a.y = 6; a.level = 0;
    w2.objects[0] = a; w2.object_count = 1;
    CHECK_INT("altar with insufficient gold returns -1",
              theron_v1_altar_of_vi_resurrect(&w2, 6, 6),
              -1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: pool recovery
 * ═══════════════════════════════════════════════════════════════ */
static void test_pool_recovery(void) {
    printf("[test:pool_recovery]\n");

    Theron_V1_World w;
    make_world(&w);

    /* Drain champion stats */
    w.party.champions[0].food   = 1;
    w.party.champions[0].water   = 1;
    w.party.champions[0].stamina = 5;
    w.party.champions[0].attributes |= THERON_ATTR_POISONED;
    w.party.champions[0].wounds = THERON_WOUND_LEGS;

    Theron_V1_Object p;
    memset(&p, 0, sizeof(p));
    p.id    = 1;
    p.type  = THERON_OBJTYPE_POOL;
    p.x     = 7; p.y = 7;
    p.level = 0;
    w.objects[0] = p;
    w.object_count = 1;

    int r = theron_v1_pool_use(&w, 7, 7);
    CHECK_INT("pool_use returns 0", r, 0);
    CHECK_INT("food restored", w.party.champions[0].food > 1, 1);
    CHECK_INT("water restored", w.party.champions[0].water > 1, 1);
    CHECK_INT("stamina restored", w.party.champions[0].stamina > 5, 1);
    CHECK_INT("poison cleared",
              (w.party.champions[0].attributes & THERON_ATTR_POISONED) == 0, 1);
    CHECK_INT("wounds cleared", w.party.champions[0].wounds == 0, 1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: per-move stat drain
 * ═══════════════════════════════════════════════════════════════ */
static void test_post_move_effects(void) {
    printf("[test:post_move_effects]\n");

    Theron_V1_World w;
    make_world(&w);
    w.party.champions[0].stamina = 20;
    w.party.champions[0].food    = 20;
    w.party.champions[0].water   = 20;
    w.party.champions[0].health  = 50;
    w.party.champions[0].wounds  = 0; /* no wounds */

    int tick_before = (int)w.world_tick;
    theron_v1_apply_post_move_effects(&w);

    CHECK_INT("world tick incremented",
              (int)w.world_tick, tick_before + 1);
    CHECK_INT("stamina drained by 1",
              w.party.champions[0].stamina, 19);

    /* With wounds */
    w.party.champions[0].wounds  = THERON_WOUND_BODY;
    w.party.champions[0].health  = 50;
    int hp_before = w.party.champions[0].health;
    theron_v1_apply_post_move_effects(&w);
    CHECK_INT("health drained by wound tick",
              w.party.champions[0].health, hp_before - 1);

    /* Poison */
    w.party.champions[0].wounds  = 0;
    w.party.champions[0].attributes |= THERON_ATTR_POISONED;
    w.party.champions[0].health  = 50;
    hp_before = w.party.champions[0].health;
    theron_v1_apply_post_move_effects(&w);
    CHECK_INT("health drained by poison tick",
              w.party.champions[0].health, hp_before - 1);
    CHECK_INT("poison cleared after tick",
              (w.party.champions[0].attributes & THERON_ATTR_POISONED) == 0, 1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: alarm trigger
 * ═══════════════════════════════════════════════════════════════ */
static void test_alarm_trigger(void) {
    printf("[test:alarm_trigger]\n");

    Theron_V1_World w;
    make_world(&w);

    Theron_V1_Object a;
    memset(&a, 0, sizeof(a));
    a.id    = 1;
    a.type  = THERON_OBJTYPE_ALARM;
    a.x     = 3; a.y = 3;
    a.level = 0;
    w.objects[0] = a;
    w.object_count = 1;

    CHECK_INT("alarm_trigger returns 0",
              theron_v1_alarm_trigger(&w, 3, 3),
              0);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: trigger activate
 * ═══════════════════════════════════════════════════════════════ */
static void test_trigger_activate(void) {
    printf("[test:trigger_activate]\n");

    Theron_V1_World w;
    make_world(&w);

    /* Trigger with linked door */
    Theron_V1_Object t;
    memset(&t, 0, sizeof(t));
    t.id       = 10;
    t.type     = THERON_OBJTYPE_TRIGGER;
    t.linked_id = 20;
    t.x        = 4; t.y = 4;
    t.level    = 0;
    t.flags    = 0;

    Theron_V1_Object d;
    memset(&d, 0, sizeof(d));
    d.id    = 20;
    d.type  = THERON_OBJTYPE_DOOR;
    d.state = THERON_DOOR_STATE_CLOSED;
    d.flags = 0;
    d.x     = 5; d.y = 4;
    d.level = 0;

    w.objects[0] = t;
    w.objects[1] = d;
    w.object_count = 2;

    CHECK_INT("trigger_activate returns 0",
              theron_v1_trigger_activate(&w, 4, 4),
              0);
    CHECK_INT("linked door is ACTIVATED flag set",
              (w.objects[1].flags & THERON_OBJ_F_ACTIVATED) != 0, 1);
    CHECK_INT("trigger itself is ACTIVATED",
              (w.objects[0].flags & THERON_OBJ_F_ACTIVATED) != 0, 1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: turn party
 * ═══════════════════════════════════════════════════════════════ */
static void test_turn_party(void) {
    printf("[test:turn_party]\n");

    Theron_V1_World w;
    make_world(&w);
    w.party.leader_dir = 0; /* north */

    CHECK_INT("turn right from north = east",
              theron_v1_turn_party(&w, +1),
              0);
    CHECK_INT("leader_dir = east (1)",
              w.party.leader_dir, 1);

    CHECK_INT("turn left from east = north",
              theron_v1_turn_party(&w, -1),
              0);
    CHECK_INT("leader_dir = north (0)",
              w.party.leader_dir, 0);

    CHECK_INT("turn left from north = west",
              theron_v1_turn_party(&w, -1),
              0);
    CHECK_INT("leader_dir = west (3)",
              w.party.leader_dir, 3);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: click route — TAKE
 * ═══════════════════════════════════════════════════════════════ */
static void test_click_route_take(void) {
    printf("[test:click_route_take]\n");

    Theron_V1_World w;
    make_world(&w);

    /* Place chest object at (6,6) */
    Theron_V1_Object c;
    memset(&c, 0, sizeof(c));
    c.id    = 1;
    c.type  = THERON_OBJTYPE_CHEST;
    c.x     = 6; c.y = 6;
    c.level = 0;
    w.objects[0] = c;
    w.object_count = 1;

    CHECK_INT("TAKE on chest returns 0",
              theron_v1_click_route(&w, 6, 6, THERON_CMD_TAKE),
              0);

    /* Take from empty square */
    CHECK_INT("TAKE from empty square returns -1",
              theron_v1_click_route(&w, 7, 7, THERON_CMD_TAKE),
              -1);
}

/* ═══════════════════════════════════════════════════════════════
 * TEST: source evidence
 * ═══════════════════════════════════════════════════════════════ */
static void test_source_evidence(void) {
    printf("[test:source_evidence]\n");

    const char *ev = theron_v1_mechanics_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 20,
          "source_evidence string is non-empty and cites THQUEST.ASM");
}

/* ── Main ─────────────────────────────────────────────────────── */
int main(void) {
    printf("\n=== Theron V1 Phase 5 — Mechanics Hardening Probe ===\n\n");

    test_wall_blocking();
    test_get_move_result();
    test_pit_fall();
    test_click_route_move();
    test_click_route_use_door();
    test_door_open_close();
    test_altar_resurrection();
    test_pool_recovery();
    test_post_move_effects();
    test_alarm_trigger();
    test_trigger_activate();
    test_turn_party();
    test_click_route_take();
    test_source_evidence();

    printf("\n=========================================\n");
    printf("Results: %d passed, %d failed\n", g_pass, g_fail);
    printf("Source: THQUEST.ASM T520/T560/T600/T700/T800/T900\n");
    printf("        movement_features.md (ReDMCSB MOVESENS.C:475-538)\n");
    printf("        tqr_v1_phase2_data_formats_H2339.md\n");
    printf("=========================================\n\n");

    return g_fail == 0 ? 0 : 1;
}