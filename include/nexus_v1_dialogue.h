
#ifndef NEXUS_V1_DIALOGUE_H
#define NEXUS_V1_DIALOGUE_H

/* Nexus V1 dialogue and NPC interaction system.
 * DM Nexus (Saturn) features NPC encounters with branching dialogue,
 * shops, and quest-related conversations.
 * Source: SDDRVS.TSK Saturn script driver NPC interaction tables. */

#include <stdint.h>

#define NEXUS_MAX_DIALOGUE_LINES 8
#define NEXUS_MAX_DIALOGUE_CHOICES 4
#define NEXUS_MAX_NPCS 32
#define NEXUS_DIALOGUE_LINE_LEN 80

enum {
    NEXUS_NPC_IDLE     = 0,
    NEXUS_NPC_TALKING  = 1,
    NEXUS_NPC_SHOP     = 2,
    NEXUS_NPC_QUEST    = 3
};

typedef struct {
    char text[NEXUS_DIALOGUE_LINE_LEN];
    int next_node;
} Nexus_DialogueChoice;

typedef struct {
    char speaker[32];
    char lines[NEXUS_MAX_DIALOGUE_LINES][NEXUS_DIALOGUE_LINE_LEN];
    int line_count;
    Nexus_DialogueChoice choices[NEXUS_MAX_DIALOGUE_CHOICES];
    int choice_count;
    int auto_advance_ticks;
} Nexus_DialogueNode;

#define NEXUS_MAX_DIALOGUE_NODES 64

typedef struct {
    Nexus_DialogueNode nodes[NEXUS_MAX_DIALOGUE_NODES];
    int node_count;
    int current_node;
    int current_line;
    int active;
    int ticks_on_line;
    int selected_choice;
} Nexus_DialogueState;

typedef struct {
    int active;
    int npc_id;
    int map_x, map_y;
    int state;
    int dialogue_start_node;
    int shop_id;
    uint8_t flags;
    char name[32];
} Nexus_NPC;

#define NEXUS_NPC_FLAG_ONESHOT  0x01
#define NEXUS_NPC_FLAG_HOSTILE  0x02

typedef struct {
    Nexus_NPC npcs[NEXUS_MAX_NPCS];
    int count;
    Nexus_DialogueState dialogue;
} Nexus_NPCManager;

void nexus_v1_npc_manager_init(Nexus_NPCManager *mgr);

int nexus_v1_npc_register(Nexus_NPCManager *mgr,
    int npc_id, int map_x, int map_y,
    const char *name, int dialogue_start_node,
    int shop_id, uint8_t flags);

int nexus_v1_npc_find_at(const Nexus_NPCManager *mgr, int map_x, int map_y);

int nexus_v1_npc_interact(Nexus_NPCManager *mgr, int npc_idx);

void nexus_v1_dialogue_init(Nexus_DialogueState *ds);

int nexus_v1_dialogue_start(Nexus_DialogueState *ds, int start_node);

int nexus_v1_dialogue_advance(Nexus_DialogueState *ds);

int nexus_v1_dialogue_choose(Nexus_DialogueState *ds, int choice_idx);

int nexus_v1_dialogue_is_active(const Nexus_DialogueState *ds);

void nexus_v1_dialogue_close(Nexus_DialogueState *ds);

int nexus_v1_dialogue_add_node(Nexus_DialogueState *ds,
    const char *speaker,
    const char *const *lines, int line_count,
    int auto_advance_ticks);

int nexus_v1_dialogue_node_add_choice(Nexus_DialogueState *ds,
    int node_idx, const char *text, int next_node);

#endif
