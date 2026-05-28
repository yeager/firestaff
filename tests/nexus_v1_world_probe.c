/*
 * Nexus V1 Phase 3 — Core World Model Verification Probe
 * ========================================================
 * Headless probe: no game data required.
 * Exercises all Phase 3 systems: world init, object DB, timers,
 * events, transitions, and deterministic hash.
 *
 * Run:  SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe nexus_v1_world
 * Or:   cc -I include src/nexus/nexus_v1_world.c -o /tmp/nexus_v1_world_probe && /tmp/nexus_v1_world_probe
 *
 * Source-lock: DUNGEON.C F0029, F0044; MOVESENS.C F0067, F0071.
 * Provenance:  nexus_v1_phase2_data_formats_H2321.md.
 */

#include <stdio.h>
#include <string.h>
#include "nexus_v1_world.h"
#include "nexus_v1_dungeon.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { printf("  PASS: %s\n", msg); g_pass++; } \
    else      { printf("  FAIL: %s\n", msg); g_fail++; } \
} while (0)

static void write_be16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFFU);
}

static void write_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xFFU);
    p[2] = (uint8_t)((v >> 8) & 0xFFU);
    p[3] = (uint8_t)(v & 0xFFU);
}

/* ── World init ─────────────────────────────────────────────────── */
static void probe_init(void) {
    printf("\n[World Init]\n");
    Nexus_V1_World world;
    nexus_v1_world_init(&world);

    CHECK(world.party_level == 0,       "party_level starts at 0");
    CHECK(world.party_x == 11,         "party_x = 11 (DM1 entrance)");
    CHECK(world.party_y == 29,         "party_y = 29 (DM1 entrance)");
    CHECK(world.party_dir == 0,         "party_dir = 0 (North)");
    CHECK(world.world_tick == 0,        "world_tick starts at 0");
    CHECK(world.object_count == 0,      "no objects initially");
    CHECK(world.event_count == 0,       "no events initially");
    CHECK(world.timer_count == 0,       "no timers initially");
    CHECK(!world.transition_pending,    "no pending transition");
    CHECK(world.state_hash != 0,        "state_hash initialized");
}

/* ── Object database ─────────────────────────────────────────────── */
static void probe_objects(void) {
    printf("\n[Object Database]\n");
    Nexus_V1_World world;
    nexus_v1_world_init(&world);

    Nexus_V1_Object obj1 = {
        .id = 0, .type = NEXUS_OBJECT_CHEST, .state = 0,
        .x = 5, .y = 10, .level = 0, .quantity = 1,
        .linked_id = 0, .flags = 0
    };
    int id1 = nexus_v1_object_place(&world, &obj1);
    CHECK(id1 > 0, "object_place returns valid id");

    Nexus_V1_Object obj2 = {
        .id = 0, .type = NEXUS_OBJECT_DOOR, .state = 1,
        .x = 7, .y = 12, .level = 1, .quantity = 1,
        .linked_id = 0, .flags = 0
    };
    int id2 = nexus_v1_object_place(&world, &obj2);
    CHECK(id2 > 0 && id2 != id1, "second object gets different id");

    Nexus_V1_Object *found = nexus_v1_object_by_id(&world, id1);
    CHECK(found != NULL && found->type == NEXUS_OBJECT_CHEST,
          "object_by_id finds placed object");

    found = nexus_v1_object_at(&world, 0, 5, 10);
    CHECK(found != NULL && found->id == id1,
          "object_at finds object by position");

    found = nexus_v1_object_at(&world, 0, 99, 99);
    CHECK(found == NULL, "object_at returns NULL for empty cell");

    int r = nexus_v1_object_set_state(&world, id1, 2);
    CHECK(r == 0, "object_set_state succeeds");
    found = nexus_v1_object_by_id(&world, id1);
    CHECK(found->state == 2, "object state updated");

    r = nexus_v1_object_set_flag(&world, id1, NEXUS_OBJ_F_OPENED);
    CHECK(r == 0, "object_set_flag succeeds");
    found = nexus_v1_object_by_id(&world, id1);
    CHECK(found->flags & NEXUS_OBJ_F_OPENED, "flag set on object");

    r = nexus_v1_object_clear_flag(&world, id1, NEXUS_OBJ_F_OPENED);
    CHECK(r == 0, "object_clear_flag succeeds");
    found = nexus_v1_object_by_id(&world, id1);
    CHECK(!(found->flags & NEXUS_OBJ_F_OPENED), "flag cleared");

    r = nexus_v1_object_remove(&world, id1);
    CHECK(r == 0, "object_remove succeeds");
    found = nexus_v1_object_by_id(&world, id1);
    CHECK(found == NULL, "removed object no longer findable");

    CHECK(world.object_count == 1, "one object remains after removal");
}

/* ── Events ──────────────────────────────────────────────────────── */
static void probe_events(void) {
    printf("\n[Event System]\n");
    Nexus_V1_World world;
    nexus_v1_world_init(&world);

    int r = nexus_v1_event_fire(&world, NEXUS_EVT_PARTY_STEP,
                                  0, 5, 10, 0, 0);
    CHECK(r == 0, "event_fire succeeds");
    CHECK(world.event_count == 1, "event recorded");
    CHECK(world.events[0].type == NEXUS_EVT_PARTY_STEP, "event type correct");
    CHECK(world.events[0].x == 5 && world.events[0].y == 10,
          "event position recorded");
    CHECK(world.events[0].fired == 1, "event marked fired");

    /* Second event */
    nexus_v1_event_fire(&world, NEXUS_EVT_CREATURE_DEAD,
                         1, 0, 0, 3, 0); /* creature type 3 */
    CHECK(world.event_count == 2, "second event recorded");

    /* Clear level */
    nexus_v1_events_clear_level(&world, 1);
    CHECK(world.event_count == 1, "clear_level removes level-1 events");
    CHECK(world.events[0].type == NEXUS_EVT_PARTY_STEP,
          "level-0 event preserved");

    /* Clear all */
    nexus_v1_events_clear_all(&world);
    CHECK(world.event_count == 0, "clear_all removes all events");
}

/* ── Timers ──────────────────────────────────────────────────────── */
static void probe_timers(void) {
    printf("\n[Timer System]\n");
    Nexus_V1_World world;
    nexus_v1_world_init(&world);

    int id1 = nexus_v1_timer_add(&world, NEXUS_TIMER_REPEAT,
                                   0, 10, 10, NULL);
    CHECK(id1 > 0, "timer_add returns valid id");

    int id2 = nexus_v1_timer_add(&world, NEXUS_TIMER_ONESHOT,
                                   1, 5, 0, NULL);
    CHECK(id2 > 0, "second timer added");

    /* Tick timers — 5 ticks should fire the oneshot */
    int i;
    for (i = 0; i < 5; i++)
        nexus_v1_tick_timers(&world);
    CHECK(world.event_count == 1, "oneshot fires after 5 ticks");
    CHECK(world.events[0].type == NEXUS_EVT_TIMER_EXPIRED,
          "timer event type is TIMER_EXPIRED");
    CHECK(world.events[0].arg0 == id2, "timer event carries timer id");

    /* Repeat timer should still be active after 5 ticks */
    int repeat_active = 0;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if ((int)world.timers[i].id == id1 &&
            (world.timers[i].flags & NEXUS_TIMER_F_ACTIVE))
            repeat_active = 1;
    }
    CHECK(repeat_active, "repeat timer still active after 5 ticks");

    /* Tick to expiry for repeat timer */
    for (i = 0; i < 10; i++)
        nexus_v1_tick_timers(&world);

    /* Pause / resume */
    nexus_v1_timer_pause(&world, id1);
    int paused = 0;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if ((int)world.timers[i].id == id1 &&
            (world.timers[i].flags & NEXUS_TIMER_F_PAUSED))
            paused = 1;
    }
    CHECK(paused, "timer_pause sets PAUSED flag");

    nexus_v1_timer_resume(&world, id1);
    int resumed = 0;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if ((int)world.timers[i].id == id1 &&
            !(world.timers[i].flags & NEXUS_TIMER_F_PAUSED))
            resumed = 1;
    }
    CHECK(resumed, "timer_resume clears PAUSED flag");

    /* Remove */
    nexus_v1_timer_remove(&world, id1);
    int removed = 1;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if ((int)world.timers[i].id == id1 &&
            (world.timers[i].flags & NEXUS_TIMER_F_ACTIVE))
            removed = 0;
    }
    CHECK(removed, "timer_remove deactivates timer");

    /* Clear level */
    int id3 = nexus_v1_timer_add(&world, NEXUS_TIMER_REPEAT, 0, 3, 3, NULL);
    nexus_v1_timers_clear_level(&world, 0);
    int level_cleared = 1;
    for (i = 0; i < NEXUS_MAX_TIMERS; i++) {
        if (world.timers[i].level == 0 &&
            (world.timers[i].flags & NEXUS_TIMER_F_ACTIVE))
            level_cleared = 0;
    }
    (void)id3;
    CHECK(level_cleared, "timers_clear_level deactivates level-0 timers");
}

/* ── Level transitions ───────────────────────────────────────────── */
static void probe_transitions(void) {
    printf("\n[Level Transitions]\n");
    Nexus_V1_World world;
    nexus_v1_world_init(&world);
    world.party_level = 3;

    /* Add a timer to verify it gets cleared on transition */
    int tid = nexus_v1_timer_add(&world, NEXUS_TIMER_REPEAT, 3, 5, 5, NULL);
    nexus_v1_event_fire(&world, NEXUS_EVT_PARTY_STEP, 3, 1, 2, 0, 0);

    int r = nexus_v1_transition_queue(&world, 4, 15, 15);
    CHECK(r == 0, "transition_queue succeeds");
    CHECK(world.transition_pending, "transition_pending set");
    CHECK(world.transition_target == 4, "transition_target = 4");
    CHECK(world.transition_x == 15, "transition_x recorded");
    CHECK(world.transition_y == 15, "transition_y recorded");

    /* Execute via world_tick */
    nexus_v1_world_tick(&world);
    CHECK(!world.transition_pending, "transition executed");
    CHECK(world.party_level == 4, "party_level updated to 4");
    CHECK(world.party_x == 15, "party_x updated");
    CHECK(world.party_y == 15, "party_y updated");
    /* Events: prior PARTY_STEP (level 3) is cleared by
     * nexus_v1_events_clear_level(level=3) inside transition_execute.
     * LEVEL_LOADED is the only surviving event (level 4). */
    CHECK(world.event_count == 1,
          "1 event: LEVEL_LOADED (prior PARTY_STEP cleared on transition)");
    CHECK(world.events[0].type == NEXUS_EVT_LEVEL_LOADED,
          "first event is LEVEL_LOADED");

    /* Invalid level rejected */
    r = nexus_v1_transition_queue(&world, 99, 0, 0);
    CHECK(r == -1, "transition_queue rejects invalid level");
}

/* ── Deterministic hash ─────────────────────────────────────────── */
static void probe_hash(void) {
    printf("\n[World-State Hash (FNV-1a 64-bit)]\n");
    Nexus_V1_World w1, w2;
    nexus_v1_world_init(&w1);
    nexus_v1_world_init(&w2);

    uint64_t h1 = nexus_v1_world_hash(&w1);
    uint64_t h2 = nexus_v1_world_hash(&w2);
    CHECK(h1 == h2, "identical worlds produce identical hash");

    /* Mutate — party position change */
    w2.party_x = 12;
    uint64_t h3 = nexus_v1_world_hash(&w2);
    CHECK(h3 != h1, "hash changes on party position mutation");

    /* Mutate — object state change */
    nexus_v1_world_init(&w2);
    nexus_v1_object_place(&w2, &(Nexus_V1_Object){
        .type = NEXUS_OBJECT_CHEST, .state = 1, .x = 1, .y = 1,
        .level = 0, .quantity = 1, .flags = 0
    });
    uint64_t h4 = nexus_v1_world_hash(&w2);
    CHECK(h4 != h1, "hash changes on object placement");

    /* hash_inject seeds with provenance-locked value */
    nexus_v1_world_init(&w1);
    uint64_t seed = 0x444E5558UL; /* NEXUS_HASH_SEED_LEVEL00 */
    nexus_v1_world_hash_inject(&w1, seed);
    CHECK(w1.state_hash != FNV64_OFFSET,
          "hash_inject seeds state_hash away from FNV offset basis");

    /* Hash is stable across multiple calls */
    nexus_v1_world_init(&w2);
    uint64_t h5 = nexus_v1_world_hash(&w2);
    uint64_t h6 = nexus_v1_world_hash(&w2);
    CHECK(h5 == h6, "hash is stable across repeated calls");
}

/* ── World tick ─────────────────────────────────────────────────── */
static void probe_tick(void) {
    printf("\n[World Tick]\n");
    Nexus_V1_World world;
    nexus_v1_world_init(&world);

    int tid = nexus_v1_timer_add(&world, NEXUS_TIMER_ONESHOT, 0, 3, 0, NULL);
    uint64_t tick_before = world.world_tick;
    (void)tid;

    nexus_v1_world_tick(&world);
    CHECK(world.world_tick == tick_before + 1, "world_tick increments");

    /* After 3 ticks the oneshot fires */
    nexus_v1_world_tick(&world);
    nexus_v1_world_tick(&world);
    CHECK(world.event_count == 1, "timer fires after 3 ticks");
}

/* ── DGN level parse (synthetic) ─────────────────────────────────── */
static void probe_dungeon_parse(void) {
    printf("\n[DGN Level Parse — synthetic fixture]\n");
    /* Build a minimal DMWeb-style DGN block container.
     * Structure1B is a 64x64 grid, 8 bytes per cell. */
    uint8_t buf[NEXUS_DGN_BLOCK_SIZE * 20];
    uint8_t *structure1;
    uint8_t *grid;
    const int structure1_block = 1;
    const int structure1_blocks = 18;
    const int structure1_offset = structure1_block * NEXUS_DGN_BLOCK_SIZE;
    const int structure1b_rel = 0x40;
    memset(buf, 0, sizeof(buf));
    write_be16(buf + 0x0C, (uint16_t)structure1_block);
    write_be16(buf + 0x0E, (uint16_t)structure1_blocks);
    write_be32(buf + 0x10, (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES));
    structure1 = buf + structure1_offset;
    structure1[0] = 0x00;
    structure1[1] = 0x50;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    write_be32(structure1 + 0x14, (uint32_t)structure1b_rel);
    write_be32(structure1 + 0x18, (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES));
    grid = structure1 + structure1b_rel;
    {
        int wall_off = (5 * NEXUS_MAX_MAP_SIZE + 5) * NEXUS_DGN_STRUCTURE1B_CELL_BYTES;
        grid[wall_off + 6] = 0x0F;
        grid[wall_off + 7] = 0xFF;
    }

    Nexus_V1_Level level;
    int r = nexus_v1_level_load(&level, buf, (int)sizeof(buf), 0);
    CHECK(r == 0, "nexus_v1_level_load succeeds on synthetic DGN");
    CHECK(level.width == 64, "level width = 64");
    CHECK(level.height == 64, "level height = 64");
    CHECK(level.squares[5][5] == 0, "wall at (5,5)");
    CHECK(level.squares[1][1] == 1, "floor at (1,1)");
    CHECK(level.has_3d_geometry, "has_3d_geometry flag set");
    CHECK(level.geometry_offset > 0, "geometry_offset > 0");

    int sq = nexus_v1_level_get_square(&level, 5, 5);
    CHECK(sq == 0, "level_get_square returns wall");
    sq = nexus_v1_level_get_square(&level, 1, 1);
    CHECK(sq == 1, "level_get_square returns floor");

    /* Out-of-bounds returns wall */
    sq = nexus_v1_level_get_square(&level, 99, 99);
    CHECK(sq == 0, "out-of-bounds returns wall");
}

/* ── Main ───────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("═══════════════════════════════════════════════════\n");
    printf("  Nexus V1 Phase 3 — Core World Model Probe\n");
    printf("  Source-lock: DUNGEON.C F0029/F0044,\n");
    printf("              MOVESENS.C F0067/F0071\n");
    printf("═══════════════════════════════════════════════════\n");

    probe_init();
    probe_objects();
    probe_events();
    probe_timers();
    probe_transitions();
    probe_hash();
    probe_tick();
    probe_dungeon_parse();

    printf("\n═══════════════════════════════════════════════════\n");
    printf("  Results: %d PASS, %d FAIL\n", g_pass, g_fail);
    printf("═══════════════════════════════════════════════════\n");
    return g_fail > 0 ? 1 : 0;
}
