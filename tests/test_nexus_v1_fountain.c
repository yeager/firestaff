
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

    /* Test 2: unproven registration remains blocked */
    {
        Nexus_FountainManager mgr;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_WATER, 5, 3, 3, 100);
        expect(idx == -1, "unproven fountain registration blocked");
        expect(mgr.count == 0, "blocked registration leaves count 0");
    }

    /* Test 3: find at position */
    {
        Nexus_FountainManager mgr;
        nexus_v1_fountain_manager_init(&mgr);
        nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_HEALTH, 2, 4, 5, 50);
        expect(nexus_v1_fountain_find_at(&mgr, 2, 4) == -1,
               "unproven fountain not found");
        expect(nexus_v1_fountain_find_at(&mgr, 0, 0) == -1, "not found");
    }

    /* Test 4: unproven water effect remains blocked */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_WATER, 1, 1, 2, 200);
        memset(&ch, 0, sizeof(ch));
        ch.water = 500;
        expect(!nexus_v1_fountain_drink(&mgr, idx, &ch),
               "unproven water effect blocked");
        expect(ch.water == 500, "blocked water leaves champion unchanged");
        expect(nexus_v1_fountain_uses_left(&mgr, idx) == 0,
               "blocked fountain has no uses");
    }

    /* Test 5: unproven health effect remains blocked */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_HEALTH, 1, 1, -1, 30);
        memset(&ch, 0, sizeof(ch));
        ch.health = 50;
        ch.max_health = 100;
        expect(!nexus_v1_fountain_drink(&mgr, idx, &ch),
               "unproven health effect blocked");
        expect(ch.health == 50, "blocked health leaves champion unchanged");
    }

    /* Test 6: unproven mana effect remains blocked */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_MANA, 1, 1, 1, 40);
        memset(&ch, 0, sizeof(ch));
        ch.mana = 10;
        ch.max_mana = 100;
        expect(!nexus_v1_fountain_drink(&mgr, idx, &ch),
               "unproven mana effect blocked");
        expect(ch.mana == 10, "blocked mana leaves champion unchanged");
    }

    /* Test 7: unproven poison effect remains blocked */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_POISON, 1, 1, 1, 25);
        memset(&ch, 0, sizeof(ch));
        ch.health = 80;
        ch.max_health = 100;
        expect(!nexus_v1_fountain_drink(&mgr, idx, &ch),
               "unproven poison effect blocked");
        expect(ch.health == 80, "blocked poison leaves champion unchanged");
    }

    /* Test 8: empty/unproven fountain */
    {
        Nexus_FountainManager mgr;
        Nexus_V1_Champion ch;
        int idx;
        nexus_v1_fountain_manager_init(&mgr);
        idx = nexus_v1_fountain_register(&mgr, NEXUS_FOUNTAIN_WATER, 1, 1, 0, 100);
        memset(&ch, 0, sizeof(ch));
        expect(!nexus_v1_fountain_drink(&mgr, idx, &ch),
               "empty fountain remains blocked");
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
        expect(ch.health == 90, "blocked health leaves cap test unchanged");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus fountain system verified\n");
    return 0;
}
