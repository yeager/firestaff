
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_engine.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_status.h"
#include "nexus_v1_hunger.h"
#include "nexus_v1_messages.h"
#include "nexus_v1_damage_indicator.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_encumbrance.h"
#include "nexus_v1_switches.h"
#include "nexus_v1_containers.h"

static int g_fail;

static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); g_fail++; }
}

static Nexus_V1_Engine *create_minimal_engine(void) {
    int i;
    Nexus_V1_Engine *e = calloc(1, sizeof(*e));
    if (!e) return NULL;
    e->initialized = 1;
    e->game.game_started = 1;
    nexus_v1_champions_init(&e->champions);
    nexus_v1_champion_recruit(&e->champions, 0);
    e->champions.champions[0].health = 100;
    e->champions.champions[0].max_health = 100;
    e->champions.champions[0].stamina = 50;
    e->champions.champions[0].max_stamina = 50;
    e->champions.champions[0].alive = 1;
    e->champions.champions[0].food = 100;
    e->champions.champions[0].water = 100;
    e->champions.champions[0].strength = 30;
    e->champions.champions[0].dexterity = 20;
    nexus_v1_encumbrance_recalc_max_load(&e->champions.champions[0]);
    nexus_v1_hunger_init(&e->hunger);
    nexus_v1_messages_init(&e->messages);
    nexus_v1_damage_display_init(&e->damage_display);
    nexus_v1_action_timers_init(&e->action_timers);
    nexus_v1_spawners_init(&e->spawners);
    nexus_v1_formation_init(&e->formation, 1);
    nexus_v1_door_manager_init(&e->doors);
    nexus_v1_gameover_init(&e->gameover);
    nexus_v1_trap_manager_init(&e->traps);
    nexus_v1_creatures_init(&e->creatures);
    nexus_v1_projectiles_init(&e->projectiles);
    nexus_v1_light_init(&e->light);
    nexus_v1_rest_init(&e->rest);
    nexus_v1_experience_init(&e->experience);
    nexus_v1_fountain_manager_init(&e->fountains);
    nexus_v1_switch_manager_init(&e->switches);
    nexus_v1_container_manager_init(&e->containers);
    for (i = 0; i < 4; i++)
        nexus_v1_status_init(&e->champion_status[i]);

    e->mechanics = calloc(1, sizeof(Nexus_MechanicsState));
    nexus_mechanics_init(e->mechanics, 5, 5, 0);
    return e;
}

static void destroy_engine(Nexus_V1_Engine *e) {
    if (e) {
        free(e->mechanics);
        free(e);
    }
}

int main(void) {
    /* Test 1: tick increments counter */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        nexus_v1_tick(e);
        expect(e->game.tick_count == 1, "tick counter incremented");
        destroy_engine(e);
    }

    /* Test 2: poison ticks reduce health */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int old_hp;
        nexus_v1_status_apply(&e->champion_status[0],
                              NEXUS_STATUS_POISON, 10, 8);
        old_hp = e->champions.champions[0].health;
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health < old_hp,
               "poison tick reduced health");
        destroy_engine(e);
    }

    /* Test 3: message queue ticks */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        nexus_v1_message_push_ex(&e->messages, "Test message", 2, 0);
        nexus_v1_tick(e);
        nexus_v1_tick(e);
        expect(e->messages.count == 0,
               "message expired after ticks");
        destroy_engine(e);
    }

    /* Test 4: multiple ticks don't crash */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int i;
        for (i = 0; i < 100; i++)
            nexus_v1_tick(e);
        expect(e->game.tick_count == 100, "100 ticks completed");
        destroy_engine(e);
    }

    /* Test 5: dead champion not poisoned further */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        e->champions.champions[0].health = 1;
        e->champions.champions[0].alive = 1;
        nexus_v1_status_apply(&e->champion_status[0],
                              NEXUS_STATUS_POISON, 50, 20);
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health == 0 &&
               e->champions.champions[0].alive == 0,
               "champion dies from poison");
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health == 0,
               "dead champion not poisoned further");
        destroy_engine(e);
    }

    /* Test 6: NULL engine doesn't crash */
    {
        nexus_v1_tick(NULL);
        expect(1, "NULL engine safe");
    }

    /* Test 7: uninitialized engine doesn't tick */
    {
        Nexus_V1_Engine *e = calloc(1, sizeof(*e));
        e->initialized = 0;
        nexus_v1_tick(e);
        expect(e->game.tick_count == 0, "uninitialized engine no tick");
        free(e);
    }

    /* Test 8: creature tick with chase AI */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int type_idx, cid;
        Nexus_CreatureType *ct;

        type_idx = e->creatures.type_count;
        ct = &e->creatures.types[type_idx];
        memset(ct, 0, sizeof(*ct));
        ct->health = 50;
        ct->attack = 10;
        ct->defense = 5;
        ct->speed = 3;
        ct->detection_range = 80;
        ct->experience_value = 20;
        e->creatures.type_count++;

        memset(e->current_level.squares, 1, sizeof(e->current_level.squares));
        cid = nexus_v1_creature_spawn_on_level(&e->creatures, type_idx,
                                                8, 5, 0, 0);
        expect(cid >= 0, "creature spawned");
        expect(e->creatures.active[cid].x == 8, "creature at x=8");

        { int t; for (t = 0; t < 3; t++) nexus_v1_tick(e); }
        expect(e->creatures.active[cid].state == 1,
               "uncaptured creature AI remains in its source-bound spawn state");
        expect(e->creatures.active[cid].x == 8,
               "uncaptured creature AI does not move the production actor");
        destroy_engine(e);
    }

    /* Test 9: creature attacks adjacent party */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int type_idx, cid, old_hp, i;
        Nexus_CreatureType *ct;

        type_idx = e->creatures.type_count;
        ct = &e->creatures.types[type_idx];
        memset(ct, 0, sizeof(*ct));
        ct->health = 50;
        ct->attack = 10;
        ct->defense = 5;
        ct->speed = 5;
        ct->detection_range = 80;
        e->creatures.type_count++;

        memset(e->current_level.squares, 1, sizeof(e->current_level.squares));
        cid = nexus_v1_creature_spawn_on_level(&e->creatures, type_idx,
                                                6, 5, 0, 0);
        old_hp = e->champions.champions[0].health;
        for (i = 0; i < 20; i++)
            nexus_v1_tick(e);
        expect(e->champions.champions[0].health == old_hp,
               "uncaptured creature attack does not mutate the party");
        destroy_engine(e);
    }

    /* Test 10: gameover triggers on all dead */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        e->champions.champions[0].health = 0;
        e->champions.champions[0].alive = 0;
        nexus_v1_tick(e);
        expect(e->gameover.state == NEXUS_GAMEOVER_DEFEAT, "gameover detected");
        destroy_engine(e);
    }

    /* Test 11: light tick doesn't crash */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        nexus_v1_light_torch_on(&e->light, 100);
        nexus_v1_tick(e);
        expect(e->light.torch_ticks < 100, "torch burned down");
        destroy_engine(e);
    }

    /* Test 12: needs_redraw set on state change */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        e->game.needs_redraw = 0;
        nexus_mechanics_push_command(e->mechanics, NEXUS_CMD_TURN_LEFT);
        nexus_v1_tick(e);
        expect(e->game.needs_redraw == 1, "needs_redraw set after turn");
        destroy_engine(e);
    }

    /* Test 13: interaction remains no-op until the Saturn action dispatcher
     * is captured; isolated switch/container modules remain testable above. */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int switch_idx;
        int container_idx;
        switch_idx = nexus_v1_switch_register(&e->switches,
                                              NEXUS_SWITCH_LEVER,
                                              5, 5,
                                              NEXUS_SWITCH_TARGET_DOOR,
                                              0,
                                              0);
        container_idx = nexus_v1_container_register(&e->containers,
                                                     NEXUS_CONTAINER_CHEST,
                                                     5, 5,
                                                     0,
                                                     -1);
        nexus_mechanics_push_command(e->mechanics, NEXUS_CMD_INTERACT);
        nexus_v1_tick(e);
        expect(switch_idx == 0 &&
                   nexus_v1_switch_get_state(&e->switches, switch_idx) == 0,
               "unproven interaction does not toggle a real switch");
        expect(container_idx < 0,
               "unproven interaction rejects synthetic container admission");
        destroy_engine(e);
    }

    /* Test 14: real retail engines do not apply DM1-derived hunger/thirst
     * damage while PLRD provisions and the Saturn consumer are unbound. */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int old_stamina = e->champions.champions[0].stamina;
        e->source = NEXUS_SRC_EXTRACTED;
        e->champions.champions[0].food = 0;
        e->champions.champions[0].water = 0;
        e->mechanics->food_drain_timer = 1;
        e->mechanics->water_drain_timer = 1;
        nexus_v1_tick(e);
        expect(e->champions.champions[0].stamina == old_stamina &&
               e->mechanics->food_drain_timer == 1 &&
               e->mechanics->water_drain_timer == 1,
               "uncaptured retail provisions do not mutate stamina");
        destroy_engine(e);
    }

    /* Test 15: real retail engines do not apply unbound DM1 rest/status/light
     * consumers.  The fixture source remains eligible for unit behavior. */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int old_hp = e->champions.champions[0].health;
        int old_rest_ticks;
        int old_light_ticks;
        e->source = NEXUS_SRC_EXTRACTED;
        nexus_v1_rest_start(&e->rest);
        e->rest.regen_timer = 1;
        old_rest_ticks = e->rest.rest_ticks;
        nexus_v1_status_apply(&e->champion_status[0],
                              NEXUS_STATUS_POISON, 5, 8);
        old_light_ticks = e->light.decay_timer;
        e->light.torch_active = 1;
        e->light.torch_ticks = 5;
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health == old_hp,
               "uncaptured retail status does not mutate health");
        expect(e->rest.rest_ticks == old_rest_ticks,
               "uncaptured retail rest does not mutate timer");
        expect(e->light.torch_ticks == 5 &&
               e->light.decay_timer == old_light_ticks,
               "uncaptured retail light does not mutate timer");
        expect(e->champion_status[0].ticks[NEXUS_STATUS_POISON] == 5,
               "uncaptured retail status does not expire");
        destroy_engine(e);
    }

    /* Test 16: the engine-level hunger loop is also closed for retail data;
     * place both accumulators immediately before their mutation threshold. */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int old_hp = e->champions.champions[0].health;
        int old_food = e->champions.champions[0].food;
        int old_water = e->champions.champions[0].water;
        e->source = NEXUS_SRC_EXTRACTED;
        e->hunger.food_accumulator = NEXUS_HUNGER_ACCUM_THRESH - 1;
        e->hunger.water_accumulator = NEXUS_HUNGER_ACCUM_THRESH - 1;
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health == old_hp &&
               e->champions.champions[0].food == old_food &&
               e->champions.champions[0].water == old_water &&
               e->hunger.food_accumulator == NEXUS_HUNGER_ACCUM_THRESH - 1 &&
               e->hunger.water_accumulator == NEXUS_HUNGER_ACCUM_THRESH - 1,
               "uncaptured retail hunger does not mutate provisions or health");
        destroy_engine(e);
    }

    /* Test 17: retail ticks do not advance unbound action/door/trap state. */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        Nexus_V1_Trap trap;
        int door;
        e->source = NEXUS_SRC_EXTRACTED;
        nexus_v1_action_start_cooldown(&e->action_timers, 0, 12, NULL);
        door = nexus_v1_door_register(&e->doors, 5, 4, 0, 0, -1, 0, 0);
        nexus_v1_door_try_open(&e->doors, door, -1, 0);
        memset(&trap, 0, sizeof(trap));
        trap.kind = NEXUS_TRAP_PRESSURE_PLATE;
        trap.armed = 1;
        trap.rearm_ticks = 8;
        nexus_v1_trap_add(&e->traps, &trap);
        nexus_v1_tick(e);
        expect(e->action_timers.cooldown[0] == 12,
               "uncaptured retail action cooldown does not tick");
        expect(e->doors.doors[door].anim_frame == 0 &&
               e->doors.doors[door].state == NEXUS_DOOR_OPENING,
               "uncaptured retail door animation does not tick");
        expect(e->traps.traps[0].cooldown == 0,
               "uncaptured retail trap timer does not tick");
        destroy_engine(e);
    }

    /* Test 18: retail movement remains available on decoded floor geometry,
     * but its DM1-derived step-stamina mutation stays closed. */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int old_stamina;
        int old_x = e->mechanics->party_x;
        int old_y = e->mechanics->party_y;
        memset(e->current_level.squares, 1, sizeof(e->current_level.squares));
        e->current_level.width = 64;
        e->current_level.height = 64;
        e->source = NEXUS_SRC_EXTRACTED;
        old_stamina = e->champions.champions[0].stamina;
        nexus_mechanics_push_command(e->mechanics, NEXUS_CMD_FORWARD);
        nexus_v1_tick(e);
        expect(e->mechanics->party_x != old_x || e->mechanics->party_y != old_y,
               "retail movement still follows decoded floor geometry");
        expect(e->champions.champions[0].stamina == old_stamina,
               "uncaptured retail step stamina does not mutate");
        destroy_engine(e);
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }

    printf("ok: Nexus tick integration verified (18 tests)\n");
    return 0;
}
