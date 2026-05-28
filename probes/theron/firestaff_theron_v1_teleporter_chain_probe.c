/*
 * firestaff_theron_v1_teleporter_chain_probe.c
 *
 * Theron's Quest V1 Phase 5 — Focused probe: teleporter chain resolution.
 *
 * Mechanic: TQ teleporter squares link to objects; if target is itself a
 * teleporter, the chain continues up to THERON_TELEPORTER_CHAIN_MAX (100).
 * The party should end up at the non-teleporter endpoint.
 *
 * Source: movement_features.md — ReDMCSB MOVESENS.C:475-537
 *         THQUEST.ASM T600 — map transitions
 *
 * Compile: gcc -o probe probes/theron/firestaff_theron_v1_teleporter_chain_probe.c \
 *            -I include -L build -lfirestaff_theron -lm -Wall -O2
 * Run:     ./probe
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "theron_v1_world.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_boot.h"
#include "theron_v1_combat.h"

/* ── Stub out missing symbols that mechanics.c calls but are not yet ──
 *    implemented in the current theron_v1 source set.              ── */
int  theron_v1_play_sound(Theron_SoundID id)       { (void)id; return 0; }
void theron_v1_champion_die(Theron_V1_World *w, int s)    { (void)w; (void)s; }
void theron_v1_creature_ai_tick(Theron_V1_World *w)      { (void)w; }
Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *w, int lvl, int x, int y) {
    (void)w; (void)lvl; (void)x; (void)y; return NULL;
}
int theron_v1_champion_attack(Theron_V1_World *w, int champ_slot, int creature_id) {
    (void)w; (void)champ_slot; (void)creature_id; return -1;
}

/* ── Minimal test world ───────────────────────────────────────────── */
static void init_world_with_teleporter_chain(
    Theron_V1_World *world,
    int tp0_x, int tp0_y,   /* entry teleporter */
    int tp1_x, int tp1_y,   /* intermediate */
    int dest_x, int dest_y  /* final non-teleporter target */
)
{
    memset(world, 0, sizeof(*world));
    world->current_level = 0;

    /* Level 0 — 8x8 grid, all floor except our teleporter chain */
    Theron_V1_Level *lvl = &world->levels[0][0];
    lvl->width  = 8;
    lvl->height = 8;
    lvl->level_index = 0;
    memset(lvl->squares, THERON_SQUARE_FLOOR, sizeof(lvl->squares));

    /* Place entry teleporter at tp0 */
    lvl->squares[tp0_y][tp0_x] = THERON_SQUARE_TELEPORTER;

    /* Objects: teleporter chain objects */
    world->object_count = 3;
    world->objects[0].id       = 10;
    world->objects[0].type     = THERON_OBJTYPE_TELEPORTER;
    world->objects[0].x        = tp0_x;
    world->objects[0].y        = tp0_y;
    world->objects[0].level    = 0;
    world->objects[0].linked_id = 11;   /* points to intermediate */

    world->objects[1].id       = 11;
    world->objects[1].type     = THERON_OBJTYPE_TELEPORTER;
    world->objects[1].x        = tp1_x;
    world->objects[1].y        = tp1_y;
    world->objects[1].level    = 0;
    world->objects[1].linked_id = 12;  /* points to final destination */

    world->objects[2].id       = 12;
    world->objects[2].type     = THERON_OBJTYPE_CHEST;  /* non-teleporter endpoint */
    world->objects[2].x        = dest_x;
    world->objects[2].y       = dest_y;
    world->objects[2].level   = 0;

    /* Party starts at teleporter entry square */
    world->party.leader_x = tp0_x;
    world->party.leader_y = tp0_y;
    world->party.leader_dir = 0;
    world->party.champion_count = 1;
    world->party.active_slot = 0;
    world->party.champions[0].alive = 1;
    world->party.champions[0].health = 50;
}

/* ── Test helper ────────────────────────────────────────────────────── */
static int probe_ok = 1;

static void check_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s: got %d want %d\n", label, got, want);
        probe_ok = 0;
    }
}

int main(void)
{
    printf("=== Theron V1 — Teleporter Chain Resolution Probe ===\n\n");

    /* ── Test 1: Two-teleporter chain → final non-teleporter ── */
    {
        Theron_V1_World world;
        init_world_with_teleporter_chain(&world, 3, 3, 5, 5, 7, 7);

        printf("[test:two-hop-chain]\n");
        printf("tp0=(3,3) → tp1=(5,5) → dest=(7,7) endpoint\n");

        int r = theron_v1_teleporter_resolve(&world, 3, 3);

        printf("resolve returned=%d transition_pending=%d\n", r, world.transition_pending);
        printf("party leader pos=(%d,%d)\n", world.party.leader_x, world.party.leader_y);
        printf("spawn pos=(%d,%d) level=%d type=%d\n",
               world.transition_spawn_x, world.transition_spawn_y,
               world.transition_target_level, world.transition_type);

        check_int("resolve returned 0", r, 0);
        check_int("transition pending", world.transition_pending, 1);
        check_int("transition type TELEPORTER", world.transition_type, THERON_TRANSITION_TELEPORTER);
        check_int("spawn x at dest", world.transition_spawn_x, 7);
        check_int("spawn y at dest", world.transition_spawn_y, 7);
        check_int("target level = 0", world.transition_target_level, 0);
        check_int("party leader x updated", world.party.leader_x, 7);
        check_int("party leader y updated", world.party.leader_y, 7);
    }

    printf("\n");

    /* ── Test 2: Dead-end teleporter (no linked target) ── */
    {
        Theron_V1_World world;
        memset(&world, 0, sizeof(world));
        world.current_level = 0;

        Theron_V1_Level *lvl = &world.levels[0][0];
        lvl->width = lvl->height = 4;
        memset(lvl->squares, THERON_SQUARE_FLOOR, sizeof(lvl->squares));
        lvl->squares[1][1] = THERON_SQUARE_TELEPORTER;

        world.object_count = 1;
        world.objects[0].id = 99;
        world.objects[0].type = THERON_OBJTYPE_TELEPORTER;
        world.objects[0].x = 1;
        world.objects[0].y = 1;
        world.objects[0].level = 0;
        world.objects[0].linked_id = 999; /* no matching object */

        world.party.leader_x = 1;
        world.party.leader_y = 1;

        printf("[test:dead-end-teleporter]\n");
        int r = theron_v1_teleporter_resolve(&world, 1, 1);
        printf("resolve returned=%d party at=(%d,%d) spawn=(%d,%d)\n",
               r, world.party.leader_x, world.party.leader_y,
               world.transition_spawn_x, world.transition_spawn_y);

        /* Should fall back to placing party at the teleporter square itself */
        check_int("resolve returned 0", r, 0);
        check_int("party stays at teleporter x", world.party.leader_x, 1);
        check_int("party stays at teleporter y", world.party.leader_y, 1);
        check_int("transition pending", world.transition_pending, 1);
    }

    printf("\n");

    /* ── Test 3: Single teleporter → non-teleporter ── */
    {
        Theron_V1_World world;
        memset(&world, 0, sizeof(world));
        world.current_level = 0;

        Theron_V1_Level *lvl = &world.levels[0][0];
        lvl->width = lvl->height = 6;
        memset(lvl->squares, THERON_SQUARE_FLOOR, sizeof(lvl->squares));
        lvl->squares[2][2] = THERON_SQUARE_TELEPORTER;

        world.object_count = 2;
        world.objects[0].id = 7;
        world.objects[0].type = THERON_OBJTYPE_TELEPORTER;
        world.objects[0].x = 2;
        world.objects[0].y = 2;
        world.objects[0].level = 0;
        world.objects[0].linked_id = 8;

        world.objects[1].id = 8;
        world.objects[1].type = THERON_OBJTYPE_POTION;  /* non-teleporter */
        world.objects[1].x = 5;
        world.objects[1].y = 5;
        world.objects[1].level = 0;

        world.party.leader_x = 2;
        world.party.leader_y = 2;

        printf("[test:single-teleporter]\n");
        int r = theron_v1_teleporter_resolve(&world, 2, 2);
        printf("resolve returned=%d party pos=(%d,%d) spawn=(%d,%d)\n",
               r, world.party.leader_x, world.party.leader_y,
               world.transition_spawn_x, world.transition_spawn_y);

        check_int("resolve returned 0", r, 0);
        check_int("party x = dest", world.party.leader_x, 5);
        check_int("party y = dest", world.party.leader_y, 5);
        check_int("transition pending", world.transition_pending, 1);
        check_int("spawn x", world.transition_spawn_x, 5);
        check_int("spawn y", world.transition_spawn_y, 5);
    }

    printf("\n=== SUMMARY ===\n");
    printf("probe_ok=%d\n", probe_ok ? 1 : 0);
    printf("locks=movement_features.md(MOVESENS.C:475-537),THQUEST.ASM:T600\n");
    return probe_ok ? 0 : 1;
}