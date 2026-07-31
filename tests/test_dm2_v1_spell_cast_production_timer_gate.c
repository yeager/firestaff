/* Production build gate: reduced spell-cast receipts may not manufacture
 * source timers.  Source: SKProject SKULLWIN/c_events.cpp cast branches and
 * c_tim_proc.cpp timer consumers require the complete record/timer owner. */
#include "dm2_v1_spell_cast_player.h"
#include "dm2_v1_timeline.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_SpellCastPlayerReceipt cast;
    DM2_V1_SpellCastApplyReceipt receipt;
    DM2_ChampionRecord champion;
    DM2_V1_SourceTimerQueue queue;

    memset(&cast, 0, sizeof(cast));
    memset(&champion, 0, sizeof(champion));
    dm2_v1_source_timer_queue_init(&queue);
    cast.valid = 1;
    cast.cast_success = 1;
    cast.timer_kind = DM2_V1_SPELL_TIMER_LIGHT;
    cast.timer_duration = 3;

    receipt = dm2_v1_spell_cast_player_apply(
        &cast, &champion, 0, NULL, &queue, 100u, 0, 4, 5, 0);

    if (!receipt.valid || !receipt.applied || receipt.timer_enqueued ||
        receipt.timer_ticket != 0u || queue.count != 0) {
        fputs("production spell-cast timer gate failed\n", stderr);
        return 1;
    }
    puts("DM2 production spell-cast timer gate passed");
    return 0;
}
