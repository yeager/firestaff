
#ifndef NEXUS_V1_LEVEL_TRANSITION_H
#define NEXUS_V1_LEVEL_TRANSITION_H

#include "nexus_v1_squares.h"

#define NEXUS_MAX_TRANSITIONS   64

typedef enum {
    NEXUS_TRANSITION_NONE = 0,
    NEXUS_TRANSITION_STAIRS_DOWN,
    NEXUS_TRANSITION_STAIRS_UP,
    NEXUS_TRANSITION_TELEPORTER,
    NEXUS_TRANSITION_PIT
} Nexus_TransitionKind;

typedef struct {
    Nexus_TransitionKind kind;
    int src_level, src_x, src_y;
    int dst_level, dst_x, dst_y, dst_dir;
} Nexus_V1_Transition;

typedef struct {
    Nexus_V1_Transition transitions[NEXUS_MAX_TRANSITIONS];
    int count;
} Nexus_V1_TransitionTable;

void nexus_v1_transition_table_init(Nexus_V1_TransitionTable *table);

int nexus_v1_transition_add(Nexus_V1_TransitionTable *table,
                            const Nexus_V1_Transition *t);

const Nexus_V1_Transition *nexus_v1_transition_find(
    const Nexus_V1_TransitionTable *table,
    int level, int x, int y);

int nexus_v1_transition_apply(const Nexus_V1_Transition *t,
                              int *out_level, int *out_x, int *out_y,
                              int *out_dir);

#endif
