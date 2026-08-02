
#include "nexus_v1_formation.h"
#include <string.h>

void nexus_v1_formation_init(Nexus_Formation *f, int party_count) {
    int i;
    if (!f) return;
    memset(f, -1, sizeof(*f));
    for (i = 0; i < party_count && i < NEXUS_MAX_PARTY; i++)
        f->positions[i] = i;
}

void nexus_v1_formation_swap(Nexus_Formation *f, int slot_a, int slot_b) {
    int tmp;
    if (!f || slot_a < 0 || slot_a >= NEXUS_MAX_PARTY ||
        slot_b < 0 || slot_b >= NEXUS_MAX_PARTY) return;
    tmp = f->positions[slot_a];
    f->positions[slot_a] = f->positions[slot_b];
    f->positions[slot_b] = tmp;
}

int nexus_v1_formation_is_front(const Nexus_Formation *f, int party_slot) {
    if (!f || party_slot < 0 || party_slot >= NEXUS_MAX_PARTY) return 0;
    return f->positions[party_slot] <= NEXUS_POS_FRONT_RIGHT;
}

int nexus_v1_formation_slot_at(const Nexus_Formation *f,
                                int position, int party_count) {
    int i;
    if (!f) return -1;
    for (i = 0; i < party_count && i < NEXUS_MAX_PARTY; i++) {
        if (f->positions[i] == position) return i;
    }
    return -1;
}

int nexus_v1_formation_melee_target(const Nexus_Formation *f,
                                     int party_count, int rand_val) {
    int front[NEXUS_MAX_PARTY], rear[NEXUS_MAX_PARTY];
    int nf = 0, nr = 0, i;
    if (!f || party_count <= 0) return 0;

    for (i = 0; i < party_count && i < NEXUS_MAX_PARTY; i++) {
        if (f->positions[i] <= NEXUS_POS_FRONT_RIGHT)
            front[nf++] = i;
        else
            rear[nr++] = i;
    }

    if (nf == 0 && nr == 0) return 0;
    if (nf == 0) return rear[rand_val % nr];
    if (nr == 0) return front[rand_val % nf];

    if (rand_val < 75)
        return front[rand_val % nf];
    else
        return rear[(rand_val - 75) % nr];
}

int nexus_v1_formation_ranged_target(int party_count, int rand_val) {
    if (party_count <= 0) return 0;
    return rand_val % party_count;
}
