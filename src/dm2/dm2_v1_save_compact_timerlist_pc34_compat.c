/* DM2 COMPACT_TIMERLIST — remove empty timer slots before save.
 * Source: sksvgame.cpp:1715-1734. */

#include "dm2_v1_save_compact_timerlist_pc34_compat.h"
#include <string.h>

void dm2_v1_compact_timerlist(
    uint8_t *timers,
    size_t entry_size,
    size_t type_offset,
    int num_timers)
{
    if (!timers || num_timers <= 0 || entry_size == 0) return;

    for (int i = 0; i < num_timers; i++) {
        uint8_t *cur = timers + (size_t)i * entry_size;
        if (cur[type_offset] == 0) {
            uint8_t *next = cur + entry_size;
            while (next[type_offset] == 0)
                next += entry_size;
            memcpy(cur, next, entry_size);
            next[type_offset] = 0;
        }
    }
}
