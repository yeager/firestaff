
#include "nexus_v1_dialogue.h"
#include <string.h>

void nexus_v1_npc_manager_init(Nexus_NPCManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    nexus_v1_dialogue_init(&mgr->dialogue);
}

int nexus_v1_npc_register(Nexus_NPCManager *mgr,
    int npc_id, int map_x, int map_y,
    const char *name, int dialogue_start_node,
    int shop_id, uint8_t flags) {
    Nexus_NPC *n;
    if (!mgr || mgr->count >= NEXUS_MAX_NPCS) return -1;
    n = &mgr->npcs[mgr->count];
    memset(n, 0, sizeof(*n));
    n->active = 1;
    n->npc_id = npc_id;
    n->map_x = map_x;
    n->map_y = map_y;
    n->dialogue_start_node = dialogue_start_node;
    n->shop_id = shop_id;
    n->flags = flags;
    n->state = NEXUS_NPC_IDLE;
    if (name) {
        strncpy(n->name, name, sizeof(n->name) - 1);
        n->name[sizeof(n->name) - 1] = '\0';
    }
    return mgr->count++;
}

int nexus_v1_npc_find_at(const Nexus_NPCManager *mgr, int map_x, int map_y) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->npcs[i].active &&
            mgr->npcs[i].map_x == map_x &&
            mgr->npcs[i].map_y == map_y)
            return i;
    }
    return -1;
}

int nexus_v1_npc_interact(Nexus_NPCManager *mgr, int npc_idx) {
    Nexus_NPC *n;
    if (!mgr || npc_idx < 0 || npc_idx >= mgr->count) return 0;
    n = &mgr->npcs[npc_idx];
    if (!n->active) return 0;
    n->state = NEXUS_NPC_TALKING;
    return nexus_v1_dialogue_start(&mgr->dialogue, n->dialogue_start_node);
}

void nexus_v1_dialogue_init(Nexus_DialogueState *ds) {
    if (!ds) return;
    memset(ds, 0, sizeof(*ds));
    ds->current_node = -1;
}

int nexus_v1_dialogue_start(Nexus_DialogueState *ds, int start_node) {
    if (!ds || start_node < 0 || start_node >= ds->node_count) return 0;
    ds->active = 1;
    ds->current_node = start_node;
    ds->current_line = 0;
    ds->ticks_on_line = 0;
    ds->selected_choice = -1;
    return 1;
}

int nexus_v1_dialogue_advance(Nexus_DialogueState *ds) {
    Nexus_DialogueNode *node;
    if (!ds || !ds->active || ds->current_node < 0) return 0;
    node = &ds->nodes[ds->current_node];

    ds->current_line++;
    ds->ticks_on_line = 0;

    if (ds->current_line >= node->line_count) {
        if (node->choice_count > 0) {
            ds->selected_choice = 0;
            return 1;
        }
        nexus_v1_dialogue_close(ds);
        return 0;
    }
    return 1;
}

int nexus_v1_dialogue_choose(Nexus_DialogueState *ds, int choice_idx) {
    Nexus_DialogueNode *node;
    int next;
    if (!ds || !ds->active || ds->current_node < 0) return 0;
    node = &ds->nodes[ds->current_node];
    if (choice_idx < 0 || choice_idx >= node->choice_count) return 0;

    next = node->choices[choice_idx].next_node;
    if (next < 0 || next >= ds->node_count) {
        nexus_v1_dialogue_close(ds);
        return 0;
    }

    ds->current_node = next;
    ds->current_line = 0;
    ds->ticks_on_line = 0;
    ds->selected_choice = -1;
    return 1;
}

int nexus_v1_dialogue_is_active(const Nexus_DialogueState *ds) {
    if (!ds) return 0;
    return ds->active;
}

void nexus_v1_dialogue_close(Nexus_DialogueState *ds) {
    if (!ds) return;
    ds->active = 0;
    ds->current_node = -1;
    ds->current_line = 0;
    ds->selected_choice = -1;
}

int nexus_v1_dialogue_add_node(Nexus_DialogueState *ds,
    const char *speaker,
    const char *const *lines, int line_count,
    int auto_advance_ticks) {
    Nexus_DialogueNode *node;
    int i;
    if (!ds || ds->node_count >= NEXUS_MAX_DIALOGUE_NODES) return -1;
    node = &ds->nodes[ds->node_count];
    memset(node, 0, sizeof(*node));

    if (speaker) {
        strncpy(node->speaker, speaker, sizeof(node->speaker) - 1);
        node->speaker[sizeof(node->speaker) - 1] = '\0';
    }

    if (line_count > NEXUS_MAX_DIALOGUE_LINES)
        line_count = NEXUS_MAX_DIALOGUE_LINES;
    node->line_count = line_count;
    for (i = 0; i < line_count && lines && lines[i]; i++) {
        strncpy(node->lines[i], lines[i], NEXUS_DIALOGUE_LINE_LEN - 1);
        node->lines[i][NEXUS_DIALOGUE_LINE_LEN - 1] = '\0';
    }

    node->auto_advance_ticks = auto_advance_ticks;
    return ds->node_count++;
}

int nexus_v1_dialogue_node_add_choice(Nexus_DialogueState *ds,
    int node_idx, const char *text, int next_node) {
    Nexus_DialogueNode *node;
    Nexus_DialogueChoice *ch;
    if (!ds || node_idx < 0 || node_idx >= ds->node_count) return -1;
    node = &ds->nodes[node_idx];
    if (node->choice_count >= NEXUS_MAX_DIALOGUE_CHOICES) return -1;
    ch = &node->choices[node->choice_count];
    if (text) {
        strncpy(ch->text, text, NEXUS_DIALOGUE_LINE_LEN - 1);
        ch->text[NEXUS_DIALOGUE_LINE_LEN - 1] = '\0';
    }
    ch->next_node = next_node;
    return node->choice_count++;
}
