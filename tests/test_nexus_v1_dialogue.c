
#include <stdio.h>
#include <string.h>
#include "nexus_v1_dialogue.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: NPC manager init */
    {
        Nexus_NPCManager mgr;
        nexus_v1_npc_manager_init(&mgr);
        expect(mgr.count == 0, "npc count zero after init");
        expect(!nexus_v1_dialogue_is_active(&mgr.dialogue), "dialogue not active");
    }

    /* Test 2: register NPC */
    {
        Nexus_NPCManager mgr;
        int idx;
        nexus_v1_npc_manager_init(&mgr);
        idx = nexus_v1_npc_register(&mgr, 1, 5, 3, "Merchant", 0, 1, 0);
        expect(idx == 0, "first npc index 0");
        expect(mgr.count == 1, "count = 1");
        expect(mgr.npcs[0].map_x == 5, "npc x");
        expect(strcmp(mgr.npcs[0].name, "Merchant") == 0, "npc name");
    }

    /* Test 3: find NPC at position */
    {
        Nexus_NPCManager mgr;
        nexus_v1_npc_manager_init(&mgr);
        nexus_v1_npc_register(&mgr, 1, 5, 3, "A", 0, -1, 0);
        nexus_v1_npc_register(&mgr, 2, 8, 2, "B", 0, -1, 0);
        expect(nexus_v1_npc_find_at(&mgr, 5, 3) == 0, "find A");
        expect(nexus_v1_npc_find_at(&mgr, 8, 2) == 1, "find B");
        expect(nexus_v1_npc_find_at(&mgr, 0, 0) == -1, "find none");
    }

    /* Test 4: dialogue flow — add nodes, start, advance, close */
    {
        Nexus_DialogueState ds;
        const char *lines1[] = {"Hello adventurer!", "Welcome."};
        const char *lines2[] = {"Goodbye."};
        int n0, n1;
        nexus_v1_dialogue_init(&ds);
        n0 = nexus_v1_dialogue_add_node(&ds, "NPC", lines1, 2, 0);
        n1 = nexus_v1_dialogue_add_node(&ds, "NPC", lines2, 1, 0);
        expect(n0 == 0 && n1 == 1, "node indices");

        nexus_v1_dialogue_start(&ds, 0);
        expect(nexus_v1_dialogue_is_active(&ds), "active after start");
        expect(ds.current_line == 0, "line 0");

        nexus_v1_dialogue_advance(&ds);
        expect(ds.current_line == 1, "line 1");

        nexus_v1_dialogue_advance(&ds);
        expect(!nexus_v1_dialogue_is_active(&ds), "closed after last line");
    }

    /* Test 5: branching choices */
    {
        Nexus_DialogueState ds;
        const char *lines[] = {"Choose wisely."};
        const char *resp1[] = {"You chose A."};
        const char *resp2[] = {"You chose B."};
        int n0, n1, n2;
        nexus_v1_dialogue_init(&ds);
        n0 = nexus_v1_dialogue_add_node(&ds, "NPC", lines, 1, 0);
        n1 = nexus_v1_dialogue_add_node(&ds, "NPC", resp1, 1, 0);
        n2 = nexus_v1_dialogue_add_node(&ds, "NPC", resp2, 1, 0);
        nexus_v1_dialogue_node_add_choice(&ds, n0, "Option A", n1);
        nexus_v1_dialogue_node_add_choice(&ds, n0, "Option B", n2);

        nexus_v1_dialogue_start(&ds, 0);
        nexus_v1_dialogue_advance(&ds);
        expect(ds.selected_choice == 0, "choices presented");

        nexus_v1_dialogue_choose(&ds, 1);
        expect(ds.current_node == n2, "branched to B");
        expect(ds.current_line == 0, "reset to line 0");
    }

    /* Test 6: NPC interact starts dialogue */
    {
        Nexus_NPCManager mgr;
        const char *lines[] = {"Greetings."};
        nexus_v1_npc_manager_init(&mgr);
        nexus_v1_dialogue_add_node(&mgr.dialogue, "Guard", lines, 1, 0);
        nexus_v1_npc_register(&mgr, 1, 3, 3, "Guard", 0, -1, 0);
        expect(nexus_v1_npc_interact(&mgr, 0), "interact returns 1");
        expect(nexus_v1_dialogue_is_active(&mgr.dialogue), "dialogue started");
        expect(mgr.npcs[0].state == NEXUS_NPC_TALKING, "npc talking");
    }

    /* Test 7: NULL safety */
    {
        nexus_v1_npc_manager_init(NULL);
        expect(nexus_v1_npc_register(NULL, 0, 0, 0, NULL, 0, 0, 0) == -1,
               "NULL register");
        expect(nexus_v1_npc_find_at(NULL, 0, 0) == -1, "NULL find");
        expect(!nexus_v1_npc_interact(NULL, 0), "NULL interact");
        nexus_v1_dialogue_init(NULL);
        expect(!nexus_v1_dialogue_start(NULL, 0), "NULL start");
        expect(!nexus_v1_dialogue_advance(NULL), "NULL advance");
        expect(!nexus_v1_dialogue_choose(NULL, 0), "NULL choose");
        expect(!nexus_v1_dialogue_is_active(NULL), "NULL is_active");
        nexus_v1_dialogue_close(NULL);
    }

    /* Test 8: invalid choice index */
    {
        Nexus_DialogueState ds;
        const char *lines[] = {"Pick."};
        nexus_v1_dialogue_init(&ds);
        nexus_v1_dialogue_add_node(&ds, "X", lines, 1, 0);
        nexus_v1_dialogue_node_add_choice(&ds, 0, "Only", -1);
        nexus_v1_dialogue_start(&ds, 0);
        nexus_v1_dialogue_advance(&ds);
        expect(!nexus_v1_dialogue_choose(&ds, 5), "out of range choice");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus dialogue/NPC system verified\n");
    return 0;
}
