#ifndef DM2_V1_SAVE_COMPACT_TIMERLIST_PC34_COMPAT_H
#define DM2_V1_SAVE_COMPACT_TIMERLIST_PC34_COMPAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 COMPACT_TIMERLIST — remove empty timer slots before save.
 * Source: sksvgame.cpp:1715-1737.
 *
 * Walks num_timers entries. When a no-type (type == 0) entry is found,
 * copies the next non-empty entry into it and clears the source.
 * Timer entry size is parameterized for decoupling from runtime struct. */

/* Compact timer array in place.
 * timers: raw byte array of timer entries.
 * entry_size: size of each timer entry in bytes.
 * type_offset: byte offset of the type field within each entry.
 * num_timers: number of entries to process.
 *
 * No-type entries (type byte == 0) are filled from the next non-empty entry. */
void dm2_v1_compact_timerlist(
    uint8_t *timers,
    size_t entry_size,
    size_t type_offset,
    int num_timers);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_COMPACT_TIMERLIST_PC34_COMPAT_H */
