#include "csb_v1_character_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_champion_init(void)
{
    CSB_V1_Champion c;
    memset(&c, 0xFF, sizeof(c));
    csb_v1_champion_init(&c);
    assert(c.Statistics[0][0] == 0 || c.Statistics[0][0] <= 255);
}

static void test_party_init_default(void)
{
    CSB_V1_PartyState party;
    memset(&party, 0xFF, sizeof(party));
    csb_v1_character_init_default(&party);
    assert(party.ChampionCount == 0);
    assert(party.LeaderIndex == -1 || party.LeaderIndex == 0);
}

static void test_champion_is_dead_alive(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    int dead = csb_v1_champion_is_dead(&c);
    (void)dead;
    assert(dead == 0 || dead == 1);
}

static void test_champion_kill(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    c.Statistics[0][0] = 50;
    c.Statistics[0][1] = 100;
    int rc = csb_v1_champion_kill(&c);
    (void)rc;
    int dead = csb_v1_champion_is_dead(&c);
    (void)dead;
    assert(dead == 1);
}

static void test_champion_resurrect(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    c.Statistics[0][0] = 50;
    c.Statistics[0][1] = 100;
    csb_v1_champion_kill(&c);
    int rc = csb_v1_champion_resurrect(&c);
    (void)rc;
    int dead = csb_v1_champion_is_dead(&c);
    (void)dead;
    assert(dead == 0);
}

static void test_champion_reincarnate(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    c.Statistics[0][0] = 50;
    c.Statistics[0][1] = 100;
    c.Statistics[1][0] = 30;
    c.Statistics[1][1] = 60;
    c.Statistics[2][0] = 20;
    c.Statistics[2][1] = 40;
    csb_v1_champion_kill(&c);
    int rc = csb_v1_champion_reincarnate(&c);
    (void)rc;
    int dead = csb_v1_champion_is_dead(&c);
    (void)dead;
    assert(dead == 0);
}

static void test_get_set_stat(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    csb_v1_champion_set_stat(&c, 0, 0, 42);
    int v = csb_v1_champion_get_stat(&c, 0, 0);
    (void)v;
    assert(v == 42);
}

static void test_get_set_skill(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    csb_v1_champion_set_skill(&c, 0, 100);
    int v = csb_v1_champion_get_skill(&c, 0);
    (void)v;
    assert(v == 100);
}

static void test_get_load(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    c.Load = 250;
    int ld = csb_v1_champion_get_load(&c);
    (void)ld;
    assert(ld == 250);
}

static void test_maximum_load(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    csb_v1_champion_set_stat(&c, 0, 0, 50);
    csb_v1_champion_set_stat(&c, 0, 1, 50);
    unsigned int ml = csb_v1_champion_get_maximum_load(&c);
    (void)ml;
    assert(ml > 0);
}

static void test_maximum_load_null(void)
{
    unsigned int ml = csb_v1_champion_get_maximum_load(NULL);
    (void)ml;
    assert(ml == 0);
}

static void test_movement_ticks_null(void)
{
    unsigned int mt = csb_v1_champion_get_movement_ticks(NULL);
    (void)mt;
    assert(mt == 2);
}

static void test_movement_ticks_light(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    csb_v1_champion_set_stat(&c, 0, 0, 100);
    csb_v1_champion_set_stat(&c, 0, 1, 100);
    c.Load = 0;
    csb_v1_champion_recompute_load(&c);
    unsigned int mt = csb_v1_champion_get_movement_ticks(&c);
    (void)mt;
    assert(mt >= 2);
}

static void test_is_overloaded(void)
{
    CSB_V1_Champion c;
    csb_v1_champion_init(&c);
    csb_v1_champion_set_stat(&c, 0, 0, 10);
    csb_v1_champion_set_stat(&c, 0, 1, 10);
    c.Load = 9999;
    int over = csb_v1_champion_is_overloaded(&c);
    (void)over;
    assert(over == 1);
}

static void test_import_dm1_null(void)
{
    int rc = csb_v1_character_import_dm1_save(NULL, NULL);
    (void)rc;
    assert(rc != 0);
}

static void test_import_dm1_buffer_null(void)
{
    int rc = csb_v1_character_import_dm1_buffer(NULL, NULL, 0);
    (void)rc;
    assert(rc != 0);
}

static void test_slot_count(void)
{
    assert(CSB_V1_SLOT_COUNT == 30);
    assert(CSB_V1_STAT_COUNT == 7);
    assert(CSB_V1_SKILL_COUNT == 16);
    assert(CSB_V1_MAX_CHAMPIONS == 4);
}

static void test_dm1_save_offsets(void)
{
    assert(CSB_V1_DM1_CHAMP_OFF_NAME == 0);
    assert(CSB_V1_DM1_CHAMP_OFF_HEALTH == 8);
    assert(CSB_V1_DM1_CHAMP_OFF_STR == 20);
    assert(CSB_V1_DM1_CHAMP_SIZE == 116);
}

static void test_source_evidence(void)
{
    const char *ev = csb_v1_character_source_evidence();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_champion_init();
    test_party_init_default();
    test_champion_is_dead_alive();
    test_champion_kill();
    test_champion_resurrect();
    test_champion_reincarnate();
    test_get_set_stat();
    test_get_set_skill();
    test_get_load();
    test_maximum_load();
    test_maximum_load_null();
    test_movement_ticks_null();
    test_movement_ticks_light();
    test_is_overloaded();
    test_import_dm1_null();
    test_import_dm1_buffer_null();
    test_slot_count();
    test_dm1_save_offsets();
    test_source_evidence();

    puts("ok: CSB character (Q-CSB-05) 19 tests passed");
    return 0;
}
