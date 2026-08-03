
#include <stdio.h>
#include <string.h>
#include "nexus_v1_switches.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: init */
    {
        Nexus_SwitchManager mgr;
        nexus_v1_switch_manager_init(&mgr);
        expect(mgr.count == 0, "init count 0");
    }

    /* Test 2: register */
    {
        Nexus_SwitchManager mgr;
        int idx;
        nexus_v1_switch_manager_init(&mgr);
        idx = nexus_v1_switch_register(&mgr, NEXUS_SWITCH_LEVER, 3, 5,
                                       NEXUS_SWITCH_TARGET_DOOR, 42, 0);
        expect(idx == 0, "first idx 0");
        expect(mgr.switches[0].target_id == 42, "target id");
    }

    /* Test 3: find at */
    {
        Nexus_SwitchManager mgr;
        nexus_v1_switch_manager_init(&mgr);
        nexus_v1_switch_register(&mgr, NEXUS_SWITCH_BUTTON, 2, 3,
                                 NEXUS_SWITCH_TARGET_DOOR, 1, 0);
        expect(nexus_v1_switch_find_at(&mgr, 2, 3) == 0, "found");
        expect(nexus_v1_switch_find_at(&mgr, 0, 0) == -1, "not found");
    }

    /* Test 4: activate lever toggles state */
    {
        Nexus_SwitchManager mgr;
        Nexus_SwitchResult r;
        int idx;
        nexus_v1_switch_manager_init(&mgr);
        idx = nexus_v1_switch_register(&mgr, NEXUS_SWITCH_LEVER, 1, 1,
                                       NEXUS_SWITCH_TARGET_DOOR, 5, 0);
        expect(nexus_v1_switch_get_state(&mgr, idx) == 0, "initial off");
        r = nexus_v1_switch_activate(&mgr, idx);
        expect(r.activated, "activated");
        expect(r.target_id == 5, "target 5");
        expect(nexus_v1_switch_get_state(&mgr, idx) == 1, "toggled on");
        nexus_v1_switch_activate(&mgr, idx);
        expect(nexus_v1_switch_get_state(&mgr, idx) == 0, "toggled off");
    }

    /* Test 5: once-only switch */
    {
        Nexus_SwitchManager mgr;
        Nexus_SwitchResult r;
        int idx;
        nexus_v1_switch_manager_init(&mgr);
        idx = nexus_v1_switch_register(&mgr, NEXUS_SWITCH_BUTTON, 1, 1,
                                       NEXUS_SWITCH_TARGET_TRAP, 3, 1);
        r = nexus_v1_switch_activate(&mgr, idx);
        expect(r.activated, "first use ok");
        r = nexus_v1_switch_activate(&mgr, idx);
        expect(!r.activated, "second use blocked");
    }

    /* Test 6: pressure plate returns target type */
    {
        Nexus_SwitchManager mgr;
        Nexus_SwitchResult r;
        int idx;
        nexus_v1_switch_manager_init(&mgr);
        idx = nexus_v1_switch_register(&mgr, NEXUS_SWITCH_PRESSURE, 4, 4,
                                       NEXUS_SWITCH_TARGET_TELEPORTER, 7, 0);
        r = nexus_v1_switch_activate(&mgr, idx);
        expect(r.target_type == NEXUS_SWITCH_TARGET_TELEPORTER, "target type");
    }

    /* Test 7: NULL safety */
    {
        Nexus_SwitchResult r;
        nexus_v1_switch_manager_init(NULL);
        expect(nexus_v1_switch_register(NULL, 0, 0, 0, 0, 0, 0) == -1, "NULL reg");
        expect(nexus_v1_switch_find_at(NULL, 0, 0) == -1, "NULL find");
        r = nexus_v1_switch_activate(NULL, 0);
        expect(!r.activated, "NULL activate");
        expect(nexus_v1_switch_get_state(NULL, 0) == 0, "NULL state");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus switch system verified\n");
    return 0;
}
