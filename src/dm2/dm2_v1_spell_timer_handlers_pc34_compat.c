/* dm2_v1_spell_timer_handlers_pc34_compat.c — DM2-007 bounded spell-effect
 * timer bodies.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp
 */

#include "dm2_v1_spell_timer_handlers_pc34_compat.h"

#include "dm2_v1_spell.h"
#include "dm2_v1_projectile_pc34_compat.h"

#include <string.h>

static uint32_t spell_timer_pack_ticks_and_map(uint32_t tick, int map_id)
{
    return ((uint32_t)(map_id & 0xff) << 24) |
           (tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
}

static int dm2_v1_spell_timer_handler_index_valid(
    const DM2_V1_SpellTimerHandlerContext *ctx, int idx)
{
    return ctx != NULL && ctx->champions != NULL &&
           idx >= 0 && idx < ctx->champion_count &&
           idx < DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS;
}

/* 0x46 DM2_PROCESS_TIMER_LIGHT (c_tim_proc.cpp:918-959).
 *
 * The original reads timer A as a signed word, looks up two entries in
 * table1d6702 indexed by |A| and |A|-1, and adds the difference to the global
 * light word ddat.savegames1.w_00.  If the decremented duration word (RG4W)
 * is non-zero it requeues type 0x46 with A=RG4W, actor=0, due tick +8.
 *
 * This bounded slice does not materialise table1d6702; it applies a unit
 * light adjustment per pop and requeues with value_a decremented by one,
 * which is the same recurrence shape that the source table implements for
 * a linear ramp. */
static int dm2_v1_spell_timer_handle_light(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    int16_t a;
    int16_t rg4;
    int sign;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.light_dispatched++;
    a = timer->value_a;
    if (a == 0) {
        /* Source early-return when A == 0; still count as consumed. */
        ctx->receipt.light_adjustments++;
        return 1;
    }

    sign = (a < 0) ? -1 : 1;
    /* Bounded approximation of table1d6702[|a|] - table1d6702[|a|-1]. */
    ctx->light_level = (int16_t)(ctx->light_level + sign);
    ctx->receipt.light_adjustments++;

    rg4 = (int16_t)((a < 0) ? a + 1 : a - 1);
    ctx->light_remaining = rg4;
    if (rg4 != 0 && ctx->queue != NULL) {
        DM2_V1_SourceTimer next;
        memset(&next, 0, sizeof(next));
        next.type = DM2_V1_TIMER_LIGHT;
        next.actor = 0;
        next.value_a = rg4;
        next.ticks_and_map = spell_timer_pack_ticks_and_map(
            ctx->game_tick + DM2_V1_SPELL_TIMER_LIGHT_REQUEUE_DELAY,
            ctx->map_id);
        dm2_v1_source_timer_enqueue(ctx->queue, &next, 0);
        ctx->receipt.light_requeued++;
    }
    return 1;
}

/* 0x47 hero enchantment flag countdown (c_tim_proc.cpp:4111-4123).
 *
 * The source decrements ddat.savegames1.b_02 once per pop; when it reaches
 * zero and v1e0976 is non-zero it sets party.hero[v1e0976-1].heroflag |=
 * 0x4000.  In the original, b_02 is a refcount incremented by each aura
 * cast (c_events.cpp case 3) and decremented by each 0x47 timer pop.
 *
 * This bounded slice treats ctx->hero_ench_countdown as that refcount.
 * The caller should initialise it to the number of active aura timers;
 * each 0x47 pop decrements it and sets the flag when the refcount falls
 * to zero.  The timer value_a (original delay) is intentionally ignored,
 * matching the source handler which does not read it. */
static int dm2_v1_spell_timer_handle_hero_ench_flag(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.hero_ench_dispatched++;

    if (ctx->hero_ench_countdown <= 0) {
        /* Stray timer with no active aura: fail-closed rather than
         * underflowing the source refcount. */
        return 0;
    }

    ctx->hero_ench_countdown--;
    if (ctx->hero_ench_countdown == 0) {
        int target = (ctx->hero_ench_target_index >= 0)
                         ? ctx->hero_ench_target_index
                         : (int)timer->actor;
        ctx->receipt.hero_ench_countdown_expired = 1;
        if (dm2_v1_spell_timer_handler_index_valid(ctx, target)) {
            ctx->champions[target].hero_flag |=
                (uint8_t)DM2_V1_SPELL_TIMER_HEROFLAG_AURA_BIT;
            ctx->receipt.hero_ench_flag_set = 1;
        }
    }
    return 1;
}

/* 0x48 enchantment power decay (c_tim_proc.cpp:4129-4163).
 *
 * For every hero whose bit is set in the actor bitmask, the source subtracts
 * timer value_a from party.hero[i].ench_power and clamps at zero.  The spell
 * cast player currently emits a single-actor timer, so the bounded slice
 * treats actor as a one-hot index and writes the clamped result back to the
 * champion body_flag proxy field. */
static int dm2_v1_spell_timer_handle_ench_power(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    int amount;
    int actor;
    int i;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.ench_power_dispatched++;
    amount = (int)timer->value_a;
    actor = (int)timer->actor;

    for (i = 0; i < ctx->champion_count &&
                i < DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS; ++i) {
        int mask = 1 << i;
        if ((actor & mask) == 0) continue;
        if (!dm2_v1_spell_timer_handler_index_valid(ctx, i)) continue;

        ctx->ench_power[i] = (int16_t)(ctx->ench_power[i] - (int16_t)amount);
        if (ctx->ench_power[i] < 0) {
            ctx->ench_power[i] = 0;
        }
        /* Bounded proxy writeback to the existing champion record field. */
        ctx->champions[i].body_flag = (uint8_t)ctx->ench_power[i];
        ctx->receipt.ench_power_decays[i] = amount;
    }
    return 1;
}

/* 0x4b poison tick (c_tim_proc.cpp:4165-4178).
 *
 * The source decrements party.hero[actor].poisoned, subtracts timer value_a
 * from party.hero[actor].poison, then calls DM2_PROCESS_POISON.  This bounded
 * slice performs the two decrements and clamps poison_strength at zero;
 * DM2_PROCESS_POISON damage application remains unproven and fail-closed. */
static int dm2_v1_spell_timer_handle_poison(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    int actor;
    int amount;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    actor = (int)timer->actor;
    if (!dm2_v1_spell_timer_handler_index_valid(ctx, actor)) return 0;

    ctx->receipt.poison_dispatched++;
    amount = (int)timer->value_a;

    if (ctx->champions[actor].poison_value > 0) {
        ctx->champions[actor].poison_value--;
        ctx->receipt.poison_value_decays[actor] = 1;
    }

    ctx->poison_strength[actor] =
        (int16_t)(ctx->poison_strength[actor] - (int16_t)amount);
    if (ctx->poison_strength[actor] < 0) {
        ctx->poison_strength[actor] = 0;
    }
    ctx->receipt.poison_strength_decays[actor] = amount;
    return 1;
}

/* 0x19 DM2_PROCESS_TIMER_19 / DM2_PROCESS_TIMER_CLOUD
 * (c_tim_proc.cpp:4195-4213).
 *
 * The source steps an active cloud through its DB14 record.  A reduced timer
 * without that record identity cannot truthfully recreate the cloud state. */
static int dm2_v1_spell_timer_handle_cloud(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.cloud_dispatched++;
    ctx->receipt.cloud_origin_x = (int)timer->value_a;
    ctx->receipt.cloud_origin_y = (int)timer->value_b;
    ctx->receipt.cloud_object_effect = (int)timer->reserved;
    /* DM2_PROCESS_TIMER_19 advances a pre-existing DB14 record.  The timer
     * carries its record handle and the source reads the actual DB14 layout;
     * this Firestaff timer contract carries only a coordinate/effect tuple.
     * Never invent duration or effect bytes from that incomplete payload.
     * Source: SKProject SKULLWIN/c_tim_proc.cpp:4195-4213. */
    ctx->receipt.cloud_record_creation_failed = 1;
    return 1;
}

/* Map DM2_OBJECT_EFFECT_* to a DM2 V1 projectile subtype.
 * Source: skproject/SKWIN/SkWinCore.cpp:27038-27096 (spell→OBJECT_EFFECT and
 *         c_creature.cpp projectile dispatch). */
int dm2_v1_spell_timer_object_effect_to_projectile_subtype(int object_effect)
{
    switch (object_effect) {
    case DM2_OBJECT_EFFECT_FIREBALL:
        return DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL;
    case DM2_OBJECT_EFFECT_LIGHTNING:
        return DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING;
    case DM2_OBJECT_EFFECT_DISPELL:
        return DM2_PROJ_SUBTYPE_MAGICAL_DISPELL;
    case DM2_OBJECT_EFFECT_POISON_CLOUD:
        return DM2_PROJ_SUBTYPE_MAGICAL_POISON_CLOUD;
    case DM2_OBJECT_EFFECT_POISON_BOLT:
        return DM2_PROJ_SUBTYPE_MAGICAL_POISON_BOLT;
    case DM2_OBJECT_EFFECT_POISON_BLOB:
        return DM2_PROJ_SUBTYPE_MAGICAL_POISON_BLOB;
    default:
        return -1;
    }
}

/* 0x1e DM2_STEP_MISSILE (c_tim_proc.cpp:442-563).
 *
 * The source reads the DB14 flying-item record referenced by the timer and
 * either steps it or deletes it.  Its record handle, owner, direction and
 * energy are not present in this reduced timer contract, so this route must
 * not create a substitute DB14/DB10 pair. */
static int dm2_v1_spell_timer_handle_projectile(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.missile_dispatched++;
    ctx->receipt.missile_origin_x = (int)timer->value_a;
    ctx->receipt.missile_origin_y = (int)timer->value_b;
    ctx->receipt.missile_object_effect = (int)timer->reserved;
    ctx->receipt.missile_projectile_slot = -1;
    return 1;
}

/* 0x5e DM2_ALLOC_NEW_CREATURE (c_tim_proc.cpp:4268-4280).
 *
 * The source creates a DB4 record through its complete allocation and CAII
 * handoff.  This reduced timer lacks the source-owned map/record context, so
 * it cannot construct a creature from a spell-effect value alone. */
static int dm2_v1_spell_timer_handle_summon(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.summon_dispatched++;
    ctx->receipt.summon_origin_x = (int)timer->value_a;
    ctx->receipt.summon_origin_y = (int)timer->value_b;
    ctx->receipt.summon_object_effect = (int)timer->reserved;
    ctx->receipt.summon_failed_no_data = 1;
    return 1;
}

void dm2_v1_spell_timer_handler_context_init(
    DM2_V1_SpellTimerHandlerContext *ctx,
    DM2_ChampionRecord *champions,
    int champion_count,
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    int map_id)
{
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->champions = champions;
    ctx->champion_count = champion_count;
    ctx->queue = queue;
    ctx->game_tick = game_tick;
    ctx->map_id = map_id;
    ctx->receipt.valid = 1;
}

void dm2_v1_spell_timer_handler_context_init_ex(
    DM2_V1_SpellTimerHandlerContext *ctx,
    DM2_ChampionRecord *champions,
    int champion_count,
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    int map_id,
    struct DM2_V1_RecordPoolSet *pool_set,
    struct DM2_V1_DungeonData *dungeon,
    void *caii)
{
    if (!ctx) return;
    dm2_v1_spell_timer_handler_context_init(
        ctx, champions, champion_count, queue, game_tick, map_id);
    ctx->record_pool_set = pool_set;
    ctx->dungeon = dungeon;
    ctx->caii = caii;
}

void dm2_v1_spell_timer_handlers_install(
    DM2_V1_TimerDispatcher *dispatcher,
    DM2_V1_SpellTimerHandlerContext *ctx)
{
    if (!dispatcher || !ctx) return;
    /* Do NOT overwrite dispatcher->context here.  In the live runtime the
     * dispatcher is shared between spell-effect handlers (which need ctx) and
     * other handlers (door-step, actuator, weather) that need the runtime
     * state.  Callers must set dispatcher->context to ctx explicitly when
     * spell handlers are the only consumers, or use dm2_v1_spell_timer_dispatch
     * to forward with an explicit context. */
    dispatcher->handlers[DM2_V1_TIMER_LIGHT] = dm2_v1_spell_timer_handle_light;
    dispatcher->handlers[DM2_V1_TIMER_HERO_ENCH_FLAG] =
        dm2_v1_spell_timer_handle_hero_ench_flag;
    dispatcher->handlers[DM2_V1_TIMER_ENCH_POWER] =
        dm2_v1_spell_timer_handle_ench_power;
    dispatcher->handlers[DM2_V1_TIMER_POISON] = dm2_v1_spell_timer_handle_poison;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_CLOUD] =
        dm2_v1_spell_timer_handle_cloud;
    dispatcher->handlers[DM2_V1_TIMER_STEP_MISSILE] =
        dm2_v1_spell_timer_handle_projectile;
    dispatcher->handlers[DM2_V1_TIMER_ALLOC_NEW_CREATURE] =
        dm2_v1_spell_timer_handle_summon;
}

int dm2_v1_spell_timer_dispatch(
    DM2_V1_SpellTimerHandlerContext *ctx,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    if (!ctx || !timer) return 0;
    switch (timer->type) {
    case DM2_V1_TIMER_LIGHT:
        return dm2_v1_spell_timer_handle_light(ctx, timer, source_index, receipt);
    case DM2_V1_TIMER_HERO_ENCH_FLAG:
        return dm2_v1_spell_timer_handle_hero_ench_flag(
            ctx, timer, source_index, receipt);
    case DM2_V1_TIMER_ENCH_POWER:
        return dm2_v1_spell_timer_handle_ench_power(
            ctx, timer, source_index, receipt);
    case DM2_V1_TIMER_POISON:
        return dm2_v1_spell_timer_handle_poison(
            ctx, timer, source_index, receipt);
    case DM2_V1_TIMER_PROCESS_CLOUD:
        return dm2_v1_spell_timer_handle_cloud(
            ctx, timer, source_index, receipt);
    case DM2_V1_TIMER_STEP_MISSILE:
        return dm2_v1_spell_timer_handle_projectile(
            ctx, timer, source_index, receipt);
    case DM2_V1_TIMER_ALLOC_NEW_CREATURE:
        return dm2_v1_spell_timer_handle_summon(
            ctx, timer, source_index, receipt);
    default:
        return 0;
    }
}

const DM2_V1_SpellTimerHandlerReceipt *dm2_v1_spell_timer_handler_receipt(
    const DM2_V1_SpellTimerHandlerContext *ctx)
{
    if (!ctx) return NULL;
    return &ctx->receipt;
}

const char *dm2_v1_spell_timer_handlers_source_evidence(void)
{
    return
        "DM2 V1 spell-effect timer handlers — source-bound timer slice\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:918-959 DM2_PROCESS_TIMER_LIGHT\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:4111-4123 hero-flag 0x47 slice\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:4129-4163 0x48 ench_power decay\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:4165-4178 0x4b poison tick\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:4195-4213 DM2_PROCESS_TIMER_19 cloud\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563 DM2_STEP_MISSILE (0x1e)\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:4268-4280 DM2_ALLOC_NEW_CREATURE (0x5e)\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:3980-4230 DM2_PROCEED_TIMERS\n"
        "Source: skproject/SKULLWIN/c_record.cpp:1076-1139 DM2_ALLOC_NEW_RECORD\n"
        "Source: skproject/SKULLWIN/c_record.cpp:1142-1165 DM2_ALLOC_NEW_DBITEM\n"
        "Source: skproject/SKULLWIN/c_1c9a.cpp:5772-5894 DM2_ALLOC_CAII_TO_CREATURE\n";
}
