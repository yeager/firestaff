
#include "nexus_v1_triggers.h"
#include <string.h>

void nexus_v1_triggers_init(Nexus_TriggerManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_trigger_register(Nexus_TriggerManager *mgr,
                               int src_x, int src_y, int src_wall,
                               int tgt_x, int tgt_y,
                               int type, int action, int once_only) {
    Nexus_Trigger *t;
    if (!mgr || mgr->count >= NEXUS_MAX_TRIGGERS) return -1;
    t = &mgr->triggers[mgr->count];
    t->source_x = src_x;
    t->source_y = src_y;
    t->source_wall = src_wall;
    t->target_x = tgt_x;
    t->target_y = tgt_y;
    t->trigger_type = type;
    t->action = action;
    t->activated = 0;
    t->once_only = once_only;
    t->used = 0;
    mgr->count++;
    return mgr->count - 1;
}

int nexus_v1_triggers_on_step(Nexus_TriggerManager *mgr, int x, int y) {
    int i, fired = 0;
    if (!mgr) return 0;
    for (i = 0; i < mgr->count; i++) {
        Nexus_Trigger *t = &mgr->triggers[i];
        if (t->source_x != x || t->source_y != y) continue;
        if (t->trigger_type != NEXUS_TRIGGER_FLOOR_PLATE &&
            t->trigger_type != NEXUS_TRIGGER_FLOOR_PAD) continue;
        if (t->once_only && t->used) continue;
        t->activated = 1;
        if (t->once_only) t->used = 1;
        fired++;
    }
    return fired;
}

int nexus_v1_triggers_off_step(Nexus_TriggerManager *mgr, int x, int y) {
    int i, deactivated = 0;
    if (!mgr) return 0;
    for (i = 0; i < mgr->count; i++) {
        Nexus_Trigger *t = &mgr->triggers[i];
        if (t->source_x != x || t->source_y != y) continue;
        if (t->trigger_type != NEXUS_TRIGGER_FLOOR_PLATE) continue;
        if (t->activated) {
            t->activated = 0;
            deactivated++;
        }
    }
    return deactivated;
}

int nexus_v1_triggers_wall_click(Nexus_TriggerManager *mgr,
                                  int x, int y, int wall) {
    int i, fired = 0;
    if (!mgr || wall < 0 || wall > 3) return 0;
    for (i = 0; i < mgr->count; i++) {
        Nexus_Trigger *t = &mgr->triggers[i];
        if (t->source_x != x || t->source_y != y) continue;
        if (t->trigger_type != NEXUS_TRIGGER_WALL_SWITCH) continue;
        if (t->source_wall != wall) continue;
        if (t->once_only && t->used) continue;
        t->activated = !t->activated;
        if (t->once_only) t->used = 1;
        fired++;
    }
    return fired;
}

int nexus_v1_trigger_get_action(const Nexus_TriggerManager *mgr, int index) {
    if (!mgr || index < 0 || index >= mgr->count) return -1;
    return mgr->triggers[index].action;
}

int nexus_v1_trigger_get_target(const Nexus_TriggerManager *mgr, int index,
                                 int *out_x, int *out_y) {
    if (!mgr || index < 0 || index >= mgr->count) return -1;
    if (out_x) *out_x = mgr->triggers[index].target_x;
    if (out_y) *out_y = mgr->triggers[index].target_y;
    return 0;
}

int nexus_v1_triggers_count(const Nexus_TriggerManager *mgr) {
    if (!mgr) return 0;
    return mgr->count;
}
