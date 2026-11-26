/* DM2 COMPACT_TIMERLIST — remove empty timer slots before save.
 * Source: SKProject 2c59a2c, SKULLWIN/c_savegame.cpp:1715-1737 and
 * SKWINDOS/src/c_savegame.cpp:1098-1120. */

#include "dm2_v1_save_compact_timerlist_pc34_compat.h"
#include <stdint.h>
#include <string.h>

void dm2_v1_compact_timerlist(
    uint8_t *timers,
    size_t timer_capacity,
    size_t entry_size,
    size_t type_offset,
    int num_timers)
{
    if (!timers || num_timers <= 0 || entry_size == 0 ||
        type_offset >= entry_size ||
        (size_t)num_timers > timer_capacity ||
        timer_capacity > SIZE_MAX / entry_size) return;

    for (size_t i = 0u; i < (size_t)num_timers; ++i) {
        uint8_t *cur = timers + i * entry_size;
        if (cur[type_offset] == 0) {
            size_t next_index = i + 1u;
            while (next_index < (size_t)num_timers &&
                   timers[next_index * entry_size + type_offset] == 0u) {
                ++next_index;
            }
            /* Source code assumes a nonempty tail sentinel. Leave a valid
             * empty tail untouched instead of scanning beyond the array. */
            if (next_index == (size_t)num_timers) break;
            {
                uint8_t *next = timers + next_index * entry_size;
                memcpy(cur, next, entry_size);
                next[type_offset] = 0u;
            }
        }
    }
}
