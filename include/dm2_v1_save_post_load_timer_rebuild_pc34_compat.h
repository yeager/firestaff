#ifndef DM2_V1_SAVE_POST_LOAD_TIMER_REBUILD_PC34_COMPAT_H
#define DM2_V1_SAVE_POST_LOAD_TIMER_REBUILD_PC34_COMPAT_H

/* DM2 post-load timer index rebuild.
 * Source: sksvgame.cpp:1351-1410 (DM2_3a15_020f).
 *
 * After loading a save game, rebuilds cross-references:
 * - Hero timeridx fields (type 0x0C timers)
 * - Record ornate animator back-links (types 0x1D-0x1E) */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_TIMER_REBUILD_MAX_HEROES 4

typedef struct {
    void *ctx;
    /* Set hero[hero_idx].timeridx = timer_idx. */
    void (*set_hero_timeridx)(void *ctx, int hero_idx, int16_t timer_idx);
    /* Set record[link] byte offset 6 word = timer_idx.
     * link is the 16-bit record link from timer.valueA. */
    void (*set_record_timer_backlink)(void *ctx, uint16_t link, int16_t timer_idx);
} DM2_V1_TimerRebuildCallbacks;

typedef struct {
    int valid;
    int hero_timeridx_cleared;
    int hero_timeridx_set;
    int ornate_backlinks_set;
    int timers_scanned;
} DM2_V1_TimerRebuildReceipt;

/* Rebuild timer cross-references after loading.
 * timer_array: packed timer records (12 bytes each).
 * num_timers: count of timers.
 * hero_count: number of heroes (max 4).
 * Returns 0 on success. */
int dm2_v1_post_load_timer_rebuild(
    const uint8_t *timer_array, int num_timers,
    int hero_count,
    const DM2_V1_TimerRebuildCallbacks *cb,
    DM2_V1_TimerRebuildReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_POST_LOAD_TIMER_REBUILD_PC34_COMPAT_H */
