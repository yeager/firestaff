
#include "nexus_v1_switches.h"
#include <string.h>

void nexus_v1_switch_manager_init(Nexus_SwitchManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_switch_register(Nexus_SwitchManager *mgr,
    int type, int map_x, int map_y,
    int target_type, int target_id, int once_only) {
    Nexus_Switch *s;
    if (!mgr || mgr->count >= NEXUS_MAX_SWITCHES) return -1;
    s = &mgr->switches[mgr->count];
    memset(s, 0, sizeof(*s));
    s->active = 1;
    s->type = type;
    s->map_x = map_x;
    s->map_y = map_y;
    s->target_type = target_type;
    s->target_id = target_id;
    s->once_only = once_only;
    return mgr->count++;
}

int nexus_v1_switch_find_at(const Nexus_SwitchManager *mgr,
    int map_x, int map_y) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->switches[i].active &&
            mgr->switches[i].map_x == map_x &&
            mgr->switches[i].map_y == map_y)
            return i;
    }
    return -1;
}

Nexus_SwitchResult nexus_v1_switch_activate(Nexus_SwitchManager *mgr,
    int switch_idx) {
    Nexus_SwitchResult r = {0, 0, -1};
    Nexus_Switch *s;
    if (!mgr || switch_idx < 0 || switch_idx >= mgr->count)
        return r;
    s = &mgr->switches[switch_idx];
    if (!s->active) return r;
    if (s->once_only && s->used) return r;

    s->state = !s->state;
    s->used = 1;
    r.activated = 1;
    r.target_type = s->target_type;
    r.target_id = s->target_id;
    return r;
}

int nexus_v1_switch_get_state(const Nexus_SwitchManager *mgr, int switch_idx) {
    if (!mgr || switch_idx < 0 || switch_idx >= mgr->count) return 0;
    return mgr->switches[switch_idx].state;
}
