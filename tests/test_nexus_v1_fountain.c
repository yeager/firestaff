
#include <stdio.h>
#include <string.h>
#include "nexus_v1_fountain.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: init */
    {
        Nexus_FountainManager mgr;
        nexus_v1_fountain_manager_init(&mgr);
        expect(mgr.count == 0, "init count 0");
    }

    /* Test 2: register */
    {
        Nexus_FountainManager mgr;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_WATER, 5, 3, 3, 100);
        expect(idx == 0, "first fountain idx 0");
        expect(mgr.count == 1, "count 1");
    }

    /* Test 3: find at position */
    {
        Nexus_FountainManager mgr;
        nexus_v1_fountain_manager_init(&mgr);
        nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_HEALTH, 2, 4, 5, 50);
        expect(nexus_v1_fountain_find_at(&mgr, 2, 4) == 0, "found");
        expect(nexus_v1_fountain_find_at(&mgr, 0, 0) == -1, "not found");
    }

    /* Test 4: drink water fountain */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_WATER, 1, 1, 2, 200);
        memset(&ch, 0, sizeof(ch));
        ch.water = 500;
        expect(nexus_v1_fountain_drink(&mgr, idx, &ch), "drink ok");
        expect(ch.water == 700, "water restored");
        expect(nexus_v1_fountain_uses_left(&mgr, idx) == 1, "1 use left");
    }

    /* Test 5: drink health fountain */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_HEALTH, 1, 1, -1, 30);
        memset(&ch, 0, sizeof(ch));
        ch.health = 50;
        ch.max_health = 100;
        expect(nexus_v1_fountain_drink(&mgr, idx, &ch), "drink health");
        expect(ch.health == 80, "health restored");
    }

    /* Test 6: drink mana fountain */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_MANA, 1, 1, 1, 40);
        memset(&ch, 0, sizeof(ch));
        ch.mana = 10;
        ch.max_mana = 100;
        expect(nexus_v1_fountain_drink(&mgr, idx, &ch), "drink mana");
        expect(ch.mana == 50, "mana restored");
    }

    /* Test 7: poison fountain */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_POISON, 1, 1, 1, 25);
        memset(&ch, 0, sizeof(ch));
        ch.health = 80;
        ch.max_health = 100;
        expect(nexus_v1_fountain_drink(&mgr, idx, &ch), "drink poison");
        expect(ch.health == 55, "health reduced");
    }

    /* Test 8: empty fountain */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_WATER, 1, 1, 0, 100);
        memset(&ch, 0, sizeof(ch));
        expect(!nexus_v1_fountain_drink(&mgr, idx, &ch), "empty fountain");
    }

    /* Test 9: NULL safety */
    {
        nexus_v1_fountain_manager_init(NULL);
        expect(nexus_v1_fountain_register(NULL, 0, 0, 0, 0, 0) == -1, "NULL register");
        expect(nexus_v1_fountain_find_at(NULL, 0, 0) == -1, "NULL find");
        expect(!nexus_v1_fountain_drink(NULL, 0, NULL), "NULL drink");
        expect(nexus_v1_fountain_uses_left(NULL, 0) == 0, "NULL uses");
    }

    /* Test 10: health cap */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_HEALTH, 1, 1, -1, 999);
        memset(&ch, 0, sizeof(ch));
        ch.health = 90;
        ch.max_health = 100;
        nexus_v1_fountain_drink(&mgr, idx, &ch);
        expect(ch.health == 100, "health capped at max");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus fountain system verified\n");
    return 0;
}
