/*
 * firestaff_theron_v1_cross_route_mechanics_probe.c
 *
 * Theron's Quest V1 cross-route mechanics runtime evidence.
 *
 * This is a data-free route probe: it builds a small synthetic dungeon and
 * walks a deterministic path through movement, door auto-open, pool recovery,
 * alarm/spawner activation, trigger-linked object activation, teleporter
 * resolution, pit fall damage, post-move drain, and click-route TAKE.
 *
 * Source: THQUEST.ASM T520/T600/T700/T800/T900
 *         docs/source-lock/movement_features.md
 *         docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "theron_v1_champions.h"
#include "theron_v1_combat.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_world.h"

int theron_v1_play_sound(Theron_SoundID id) {
    (void)id;
    return 0;
}

int theron_v1_sound_is_valid(Theron_SoundID id) {
    return id >= 0 && id < THERON_SOUND_COUNT;
}

void theron_v1_champion_die(Theron_V1_World *w, int s) {
    (void)w;
    (void)s;
}

void theron_v1_creature_ai_tick(Theron_V1_World *w) {
    (void)w;
}

Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *w,
                                           int lvl,
                                           int x,
                                           int y) {
    (void)w;
    (void)lvl;
    (void)x;
    (void)y;
    return NULL;
}

int theron_v1_champion_attack(Theron_V1_World *w,
                              int champ_slot,
                              int creature_id) {
    (void)w;
    (void)champ_slot;
    (void)creature_id;
    return -1;
}

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(label, cond) do { \
    if (cond) { \
        g_pass++; \
    } else { \
        printf("FAIL: %s\n", label); \
        g_fail++; \
    } \
} while (0)

#define CHECK_INT(label, got, want) do { \
    int g_ = (got); \
    int w_ = (want); \
    if (g_ == w_) { \
        g_pass++; \
    } else { \
        printf("FAIL: %s got=%d want=%d\n", label, g_, w_); \
        g_fail++; \
    } \
} while (0)

#define CHECK_U64(label, got, want) do { \
    uint64_t g_ = (got); \
    uint64_t w_ = (want); \
    if (g_ == w_) { \
        g_pass++; \
    } else { \
        printf("FAIL: %s got=0x%llx want=0x%llx\n", label, \
               (unsigned long long)g_, (unsigned long long)w_); \
        g_fail++; \
    } \
} while (0)

typedef struct {
    int rc_door;
    int rc_pool;
    int rc_alarm;
    int rc_trigger;
    int rc_teleport;
    int rc_pit;
    int rc_take;
    int pit_preview;
    int final_x;
    int final_y;
    uint64_t world_tick;
    uint64_t state_hash;
    int door_state;
    int pool_state;
    uint32_t trigger_flags;
    uint32_t linked_flags;
    uint32_t spawner_flags;
    int transition_pending;
    int transition_type;
    int transition_spawn_x;
    int transition_spawn_y;
    int active_health;
    int active_stamina;
    int active_food;
    int active_water;
} RouteSummary;

static Theron_V1_Object make_object(int id,
                                    int type,
                                    int x,
                                    int y,
                                    int linked_id) {
    Theron_V1_Object o;
    memset(&o, 0, sizeof(o));
    o.id = id;
    o.type = (uint8_t)type;
    o.x = x;
    o.y = y;
    o.level = 0;
    o.linked_id = linked_id;
    return o;
}

static void make_route_world(Theron_V1_World *w) {
    memset(w, 0, sizeof(*w));
    w->current_dungeon = 1;
    w->current_level = 0;

    Theron_V1_Level *lvl = &w->levels[0][0];
    lvl->width = 16;
    lvl->height = 16;
    lvl->level_index = 0;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            lvl->squares[y][x] = THERON_SQUARE_FLOOR;
        }
    }

    lvl->squares[8][9] = THERON_SQUARE_DOOR;
    lvl->squares[8][10] = THERON_SQUARE_POOL;
    lvl->squares[8][11] = THERON_SQUARE_ALARM;
    lvl->squares[8][12] = THERON_SQUARE_TRIGGER;
    lvl->squares[8][13] = THERON_SQUARE_TELEPORTER;
    lvl->squares[10][13] = THERON_SQUARE_PIT;
    w->level_loaded[0][0] = 1;

    w->party.leader_x = 8;
    w->party.leader_y = 8;
    w->party.leader_dir = THERON_DIR_EAST;
    w->party.active_slot = 0;
    w->party.gold = 1000;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &w->party.champions[i];
        c->alive = 1;
        c->health = 50;
        c->max_health = 50;
        c->stamina = 50;
        c->max_stamina = 50;
        c->food = 50;
        c->water = 50;
    }

    w->objects[0] = make_object(1, THERON_OBJTYPE_DOOR, 9, 8, 0);
    w->objects[0].state = THERON_DOOR_STATE_CLOSED;
    w->objects[1] = make_object(2, THERON_OBJTYPE_POOL, 10, 8, 0);
    w->objects[2] = make_object(3, THERON_OBJTYPE_ALARM, 11, 8, 0);
    w->objects[3] = make_object(4, THERON_OBJTYPE_CREATURE_SPAWNER, 15, 15, 0);
    w->objects[4] = make_object(5, THERON_OBJTYPE_DOOR, 12, 9, 0);
    w->objects[4].state = THERON_DOOR_STATE_CLOSED;
    w->objects[5] = make_object(6, THERON_OBJTYPE_TRIGGER, 12, 8, 5);
    w->objects[6] = make_object(7, THERON_OBJTYPE_TELEPORTER, 13, 8, 8);
    w->objects[7] = make_object(8, THERON_OBJTYPE_TRIGGER, 14, 10, 0);
    w->objects[8] = make_object(9, THERON_OBJTYPE_CHEST, 12, 10, 0);
    w->object_count = 9;
}

static RouteSummary run_route(void) {
    Theron_V1_World w;
    make_route_world(&w);

    RouteSummary s;
    memset(&s, 0, sizeof(s));

    s.rc_door = theron_v1_move_party(&w, THERON_DIR_EAST);
    s.rc_pool = theron_v1_move_party(&w, THERON_DIR_EAST);
    s.rc_alarm = theron_v1_move_party(&w, THERON_DIR_EAST);
    s.rc_trigger = theron_v1_move_party(&w, THERON_DIR_EAST);
    s.rc_teleport = theron_v1_move_party(&w, THERON_DIR_EAST);
    s.pit_preview = theron_v1_get_move_result(&w, THERON_DIR_WEST);
    s.rc_pit = theron_v1_move_party(&w, THERON_DIR_WEST);
    s.rc_take = theron_v1_click_route(&w, 12, 10, THERON_CMD_TAKE);

    s.final_x = w.party.leader_x;
    s.final_y = w.party.leader_y;
    s.world_tick = w.world_tick;
    s.state_hash = theron_v1_world_hash(&w);
    s.door_state = w.objects[0].state;
    s.pool_state = w.objects[1].state;
    s.spawner_flags = w.objects[3].flags;
    s.linked_flags = w.objects[4].flags;
    s.trigger_flags = w.objects[5].flags;
    s.transition_pending = w.transition_pending;
    s.transition_type = w.transition_type;
    s.transition_spawn_x = w.transition_spawn_x;
    s.transition_spawn_y = w.transition_spawn_y;

    const Theron_V1_Champion *active = &w.party.champions[w.party.active_slot];
    s.active_health = active->health;
    s.active_stamina = active->stamina;
    s.active_food = active->food;
    s.active_water = active->water;
    return s;
}

static void check_summary(const RouteSummary *s) {
    CHECK_INT("closed door auto-opens and moves", s->rc_door, THERON_MOVE_OK);
    CHECK_INT("pool square moves", s->rc_pool, THERON_MOVE_OK);
    CHECK_INT("alarm square moves", s->rc_alarm, THERON_MOVE_OK);
    CHECK_INT("trigger square moves", s->rc_trigger, THERON_MOVE_OK);
    CHECK_INT("teleporter square returns teleport", s->rc_teleport, THERON_MOVE_TELEPORT);
    CHECK_INT("pit preview reports fall", s->pit_preview, THERON_MOVE_PIT_FALL);
    CHECK_INT("pit move returns fall", s->rc_pit, THERON_MOVE_PIT_FALL);
    CHECK_INT("click-route TAKE resolves object", s->rc_take, 0);

    CHECK_INT("final x after teleporter plus pit", s->final_x, 13);
    CHECK_INT("final y after teleporter plus pit", s->final_y, 10);
    CHECK_U64("six post-move ticks", s->world_tick, 6u);
    CHECK("route hash is non-zero", s->state_hash != 0);

    CHECK_INT("door object is open", s->door_state, THERON_DOOR_STATE_OPEN);
    CHECK_INT("pool object used", s->pool_state, THERON_OBJ_F_USED);
    CHECK("alarm activates spawner", (s->spawner_flags & THERON_OBJ_F_ACTIVATED) != 0);
    CHECK("trigger activates linked object", (s->linked_flags & THERON_OBJ_F_ACTIVATED) != 0);
    CHECK("trigger marks itself active", (s->trigger_flags & THERON_OBJ_F_ACTIVATED) != 0);

    CHECK_INT("teleporter transition pending", s->transition_pending, 1);
    CHECK_INT("teleporter transition type", s->transition_type, THERON_TRANSITION_TELEPORTER);
    CHECK_INT("teleporter spawn x", s->transition_spawn_x, 14);
    CHECK_INT("teleporter spawn y", s->transition_spawn_y, 10);

    CHECK_INT("active champion pit health", s->active_health, 30);
    CHECK_INT("active champion post-pit stamina", s->active_stamina, 40);
    CHECK_INT("active champion post-route food", s->active_food, 45);
    CHECK_INT("active champion post-route water", s->active_water, 45);
}

static void check_determinism(void) {
    RouteSummary a = run_route();
    RouteSummary b = run_route();

    check_summary(&a);
    CHECK("route summaries byte-identical", memcmp(&a, &b, sizeof(a)) == 0);
    CHECK_U64("route hashes deterministic", a.state_hash, b.state_hash);
}

static void check_source_evidence(void) {
    const char *evidence = theron_v1_mechanics_source_evidence();
    CHECK("mechanics evidence string present", evidence && strlen(evidence) > 40);
    CHECK("mechanics evidence cites THQUEST", evidence && strstr(evidence, "THQUEST.ASM"));
    CHECK("mechanics evidence cites MOVESENS", evidence && strstr(evidence, "MOVESENS.C"));
}

int main(void) {
    printf("probe=theron_v1_cross_route_mechanics\n");
    check_determinism();
    check_source_evidence();

    printf("theronV1CrossRouteMechanics: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
