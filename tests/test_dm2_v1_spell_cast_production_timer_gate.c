/* Production build gate: reduced spell-cast receipts may not manufacture
 * unsupported source timers.  The verified 0x5e summon payload is admitted;
 * light/projectile/cloud routes still require their complete owners.
 * Source: SKProject SKULLWIN/c_events.cpp cast branches and c_tim_proc.cpp. */
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
    DM2_V1_SourceTimer timer;

    memset(&cast, 0, sizeof(cast));
    memset(&champion, 0, sizeof(champion));
    dm2_v1_source_timer_queue_init(&queue);
    cast.valid = 1;
    cast.cast_success = 1;
    cast.timer_kind = DM2_V1_SPELL_TIMER_LIGHT;
    cast.timer_duration = 3;

    receipt = dm2_v1_spell_cast_player_apply(
        &cast, &champion, 0, NULL, &queue, 100u, 0, 4, 5, 0);

    if (receipt.valid || receipt.applied || receipt.timer_enqueued ||
        receipt.timer_ticket != 0u || queue.count != 0 ||
        champion.mana != 0u || champion.hand_cooldown[0] != 0u) {
        fputs("production spell-cast timer gate failed\n", stderr);
        return 1;
    }
    puts("DM2 production spell-cast timer gate passed");

    dm2_v1_source_timer_queue_init(&queue);
    memset(&cast, 0, sizeof(cast));
    memset(&champion, 0, sizeof(champion));
    cast.valid = 1;
    cast.cast_success = 1;
    cast.timer_kind = DM2_V1_SPELL_TIMER_SUMMON;
    cast.object_effect = DM2_OBJECT_EFFECT_SUMMON_ATTACK_MINION;
    receipt = dm2_v1_spell_cast_player_apply(
        &cast, &champion, 0, NULL, &queue, 200u, 3, 0x1e, 0x1f, 0);
    memset(&timer, 0, sizeof(timer));
    if (receipt.timer_ticket == 0u ||
        !dm2_v1_source_timer_peek_ticket(&queue, receipt.timer_ticket,
                                          &timer)) {
        fputs("production summon timer ticket missing\n", stderr);
        return 1;
    }
    if (!receipt.valid || !receipt.applied || !receipt.timer_enqueued ||
        queue.count != 1u || timer.type != 0x5eu ||
        (uint16_t)timer.value_a != (uint16_t)(0x1e | (0x1fu << 8)) ||
        timer.value_b != DM2_OBJECT_EFFECT_SUMMON_ATTACK_MINION ||
        timer.reserved != 0) {
        fputs("production summon timer admission failed\n", stderr);
        return 1;
    }
    puts("DM2 production summon timer admission passed");
    return 0;
}
