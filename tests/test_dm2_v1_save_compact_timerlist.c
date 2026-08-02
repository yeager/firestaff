/* Test DM2 COMPACT_TIMERLIST. */
#include "dm2_v1_save_compact_timerlist_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Simple timer entry: [type, id] */
#define ENTRY_SIZE 2
#define TYPE_OFF 0

int main(void) {
    uint8_t timers[10 * ENTRY_SIZE];

    /* Test 1: no empty entries — nothing changes. */
    memset(timers, 0, sizeof(timers));
    timers[0] = 1; timers[1] = 10;
    timers[2] = 2; timers[3] = 20;
    timers[4] = 3; timers[5] = 30;
    dm2_v1_compact_timerlist(timers, ENTRY_SIZE, TYPE_OFF, 3);
    assert(timers[0] == 1 && timers[2] == 2 && timers[4] == 3);

    /* Test 2: first entry empty, second non-empty. */
    memset(timers, 0, sizeof(timers));
    timers[0] = 0; timers[1] = 0;
    timers[2] = 5; timers[3] = 50;
    timers[4] = 7; timers[5] = 70;
    dm2_v1_compact_timerlist(timers, ENTRY_SIZE, TYPE_OFF, 3);
    assert(timers[0] == 5);
    assert(timers[1] == 50);

    /* Test 3: gap in middle — after compaction, gap is filled from next. */
    memset(timers, 0, sizeof(timers));
    timers[0] = 1; timers[1] = 10;
    timers[2] = 0; timers[3] = 0;
    timers[4] = 3; timers[5] = 30;
    timers[6] = 4; timers[7] = 40;
    dm2_v1_compact_timerlist(timers, ENTRY_SIZE, TYPE_OFF, 3);
    assert(timers[0] == 1);
    assert(timers[2] == 3);
    assert(timers[4] == 4);

    /* Test 4: NULL input. */
    dm2_v1_compact_timerlist(NULL, ENTRY_SIZE, TYPE_OFF, 3);

    printf("PASS: dm2_v1_save_compact_timerlist\n");
    return 0;
}
