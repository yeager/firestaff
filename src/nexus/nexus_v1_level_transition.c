
#include "nexus_v1_level_transition.h"
#include <string.h>

void nexus_v1_transition_table_init(Nexus_V1_TransitionTable *table)
{
    if (!table) return;
    memset(table, 0, sizeof(*table));
}

int nexus_v1_transition_add(Nexus_V1_TransitionTable *table,
                            const Nexus_V1_Transition *t)
{
    if (!table || !t || table->count >= NEXUS_MAX_TRANSITIONS)
        return 0;
    if (t->kind == NEXUS_TRANSITION_NONE)
        return 0;
    table->transitions[table->count++] = *t;
    return 1;
}

const Nexus_V1_Transition *nexus_v1_transition_find(
    const Nexus_V1_TransitionTable *table,
    int level, int x, int y)
{
    int i;
    if (!table) return NULL;
    for (i = 0; i < table->count; ++i) {
        const Nexus_V1_Transition *t = &table->transitions[i];
        if (t->src_level == level && t->src_x == x && t->src_y == y)
            return t;
    }
    return NULL;
}

int nexus_v1_transition_apply(const Nexus_V1_Transition *t,
                              int *out_level, int *out_x, int *out_y,
                              int *out_dir)
{
    if (!t || t->kind == NEXUS_TRANSITION_NONE)
        return 0;
    if (out_level) *out_level = t->dst_level;
    if (out_x) *out_x = t->dst_x;
    if (out_y) *out_y = t->dst_y;
    if (out_dir) *out_dir = t->dst_dir;
    return 1;
}
