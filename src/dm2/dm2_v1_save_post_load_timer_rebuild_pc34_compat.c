/* DM2 post-load timer index rebuild.
 * Source: sksvgame.cpp:1351-1410 (DM2_3a15_020f).
 *
 * Phase 1: Clear hero[0..3].timeridx = -1
 * Phase 2: Walk timer array:
 *   type 0x0C: hero[actor].timeridx = timer_index
 *   type 0x1D-0x1E: record[valueA] byte@6 = timer_index */

#include "dm2_v1_save_post_load_timer_rebuild_pc34_compat.h"
#include <string.h>

#define TIMER_SIZE 12u
#define TIMER_OFF_TYPE   4u
#define TIMER_OFF_ACTOR  5u
#define TIMER_OFF_VALUEA 6u

int dm2_v1_post_load_timer_rebuild(
    const uint8_t *timer_array, int num_timers,
    int hero_count,
    const DM2_V1_TimerRebuildCallbacks *cb,
    DM2_V1_TimerRebuildReceipt *receipt)
{
    DM2_V1_TimerRebuildReceipt local;
    memset(&local, 0, sizeof(local));
    if (receipt) memset(receipt, 0, sizeof(*receipt));

    if (!cb || !receipt) {
        if (receipt) receipt->valid = 0;
        return -1;
    }

    if (hero_count > DM2_V1_TIMER_REBUILD_MAX_HEROES)
        hero_count = DM2_V1_TIMER_REBUILD_MAX_HEROES;

    /* Phase 1: Clear hero timeridx.
     * Source: sksvgame.cpp:1359-1370. */
    for (int h = 0; h < hero_count; h++) {
        if (cb->set_hero_timeridx)
            cb->set_hero_timeridx(cb->ctx, h, -1);
        local.hero_timeridx_cleared++;
    }

    if (!timer_array || num_timers <= 0) {
        local.valid = 1;
        *receipt = local;
        return 0;
    }

    /* Phase 2: Walk timer array.
     * Source: sksvgame.cpp:1374-1409. */
    for (int t = 0; t < num_timers; t++) {
        const uint8_t *tim = timer_array + (size_t)t * TIMER_SIZE;
        uint8_t type = tim[TIMER_OFF_TYPE];
        uint8_t actor = tim[TIMER_OFF_ACTOR];
        uint16_t valueA = (uint16_t)tim[TIMER_OFF_VALUEA] |
                          ((uint16_t)tim[TIMER_OFF_VALUEA + 1] << 8);

        if (type == 0x0C) {
            if (actor < (uint8_t)hero_count && cb->set_hero_timeridx) {
                cb->set_hero_timeridx(cb->ctx, (int)actor, (int16_t)t);
                local.hero_timeridx_set++;
            }
        } else if (type >= 0x1D && type <= 0x1E) {
            if (cb->set_record_timer_backlink) {
                cb->set_record_timer_backlink(cb->ctx, valueA, (int16_t)t);
                local.ornate_backlinks_set++;
            }
        }
        local.timers_scanned++;
    }

    local.valid = 1;
    *receipt = local;
    return 0;
}
