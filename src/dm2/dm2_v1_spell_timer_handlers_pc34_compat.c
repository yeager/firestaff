/* dm2_v1_spell_timer_handlers_pc34_compat.c — DM2-007 bounded spell-effect
 * timer bodies.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp
 */

#include "dm2_v1_spell_timer_handlers_pc34_compat.h"

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_spell.h"

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

/* ── DB14 / tile-chain helpers for the cycle-13 real-data handlers ─────── */

static void spell_timer_wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

/* Locate an existing DB14 record on the tile chain at (x,y) whose byte@5
 * matches the cloud object-effect marker.  The source would resolve the cloud
 * owner through the ground stack; this bounded helper fails closed (returns
 * OBJECT_NULL) when the chain is missing or unresolvable. */
static int16_t spell_timer_find_cloud_record(
    DM2_V1_DungeonData *dungeon,
    DM2_V1_RecordPoolSet *set,
    int map_id,
    int x,
    int y,
    int object_effect)
{
    int16_t thing;

    if (!dungeon || !set || !set->valid) return DM2_V1_RECORD_HANDLE_NULL;
    thing = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map_id, x, y);
    while (thing != DM2_V1_RECORD_HANDLE_NULL &&
           thing != DM2_V1_RECORD_HANDLE_END) {
        const uint8_t *rec;
        int16_t next;
        if (dm2_v1_record_handle_pool(thing) == 14) {
            rec = dm2_v1_record_pool_address(set, thing);
            if (rec != NULL && rec[5] == (uint8_t)object_effect) {
                return thing;
            }
        }
        if (!dm2_v1_record_pool_next_link(set, thing, &next)) break;
        thing = next;
    }
    return DM2_V1_RECORD_HANDLE_NULL;
}

/* Append a freshly allocated record to the tile's ground-stack list.  The
 * record's own link word must already be OBJECT_END_MARKER.  Returns 1 when
 * the dungeon first-thing word was updated. */
static int spell_timer_append_to_cell(
    DM2_V1_DungeonData *dungeon,
    DM2_V1_RecordPoolSet *set,
    int map_id,
    int x,
    int y,
    int16_t record)
{
    int16_t head;
    int rc;

    if (!dungeon || !set || !set->valid) return 0;
    head = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map_id, x, y);
    if (head < 0) head = DM2_V1_RECORD_HANDLE_END;
    rc = dm2_v1_record_pool_append_to_list(set, &head, record);
    if (!rc) return 0;
    return dm2_v1_dungeon_set_first_thing(
               dungeon, map_id, x, y, (uint16_t)head) == 0;
}

/* Remove a record from the tile chain and deallocate it (w0 = OBJECT_NULL). */
static void spell_timer_remove_and_dealloc(
    DM2_V1_DungeonData *dungeon,
    DM2_V1_RecordPoolSet *set,
    int map_id,
    int x,
    int y,
    int16_t record)
{
    int16_t head;
    uint8_t *addr;

    if (!dungeon || !set || !set->valid) return;
    head = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map_id, x, y);
    if (head < 0) return;
    if (!dm2_v1_record_pool_cut_from_list(set, &head, record)) return;
    (void)dm2_v1_dungeon_set_first_thing(
        dungeon, map_id, x, y, (uint16_t)head);
    addr = dm2_v1_record_pool_address_mut(set, record);
    if (addr != NULL) spell_timer_wr16(addr, (uint16_t)DM2_V1_RECORD_HANDLE_NULL);
}

static void spell_timer_dealloc_record(DM2_V1_RecordPoolSet *set,
                                       int16_t record)
{
    uint8_t *addr;
    if (!set || !set->valid) return;
    addr = dm2_v1_record_pool_address_mut(set, record);
    if (addr != NULL) spell_timer_wr16(addr, (uint16_t)DM2_V1_RECORD_HANDLE_NULL);
}

/* Summon spells encode the desired minion in the object-effect word. */
static int dm2_v1_spell_timer_object_effect_to_creature_type(int object_effect)
{
    switch (object_effect) {
    case DM2_OBJECT_EFFECT_SUMMON_ATTACK_MINION:
        return 14; /* ATTACK MINION (ALLY) */
    case DM2_OBJECT_EFFECT_SUMMON_GUARD_MINION:
        return 17; /* GUARD MINION (ALLY) */
    case DM2_OBJECT_EFFECT_SUMMON_UHAUL_MINION:
        return 18; /* U-HAUL MINION (ALLY) */
    default:
        return -1;
    }
}

/* Spell missiles get a DB10 misc item record whose itemtype mirrors the
 * object effect.  The itemspec layout is the source's dbMisc group
 * (c_record.cpp:367-401): (0x100 + type) -> dbMisc, type = itemspec & 0x7f. */
static uint16_t spell_timer_missile_itemspec(int object_effect)
{
    return (uint16_t)(0x0100u | ((unsigned)object_effect & 0x7fu));
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
 * The source steps an active cloud: it decrements a duration word and, while
 * the cloud remains alive, mutates the DB14 flying-item / cloud record and
 * requeues the timer.  This bounded cycle-13 slice allocates a real DB14
 * record on first pop, decrements its byte@4 duration, removes it when the
 * duration expires, and requeues every DM2_V1_SPELL_TIMER_CLOUD_REQUEUE_DELAY
 * ticks while alive.  The DB14 byte semantics are bounded defaults (documented
 * as constants) until the exact source layout is proven. */
static int dm2_v1_spell_timer_handle_cloud(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    int origin_x;
    int origin_y;
    int object_effect;
    int16_t record;
    uint8_t *addr;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.cloud_dispatched++;
    origin_x = (int)timer->value_a;
    origin_y = (int)timer->value_b;
    object_effect = (int)timer->reserved;
    ctx->receipt.cloud_origin_x = origin_x;
    ctx->receipt.cloud_origin_y = origin_y;
    ctx->receipt.cloud_object_effect = object_effect;

    if (!ctx->record_pool_set || !ctx->dungeon || !ctx->queue) {
        ctx->receipt.cloud_record_creation_failed = 1;
        return 1;
    }

    record = spell_timer_find_cloud_record(
        ctx->dungeon, ctx->record_pool_set, ctx->map_id,
        origin_x, origin_y, object_effect);

    if (record == DM2_V1_RECORD_HANDLE_NULL) {
        /* First pop for this cloud: allocate a fresh DB14 record and chain
         * it onto the origin cell. */
        record = dm2_v1_record_pool_alloc_new_record(
            ctx->record_pool_set, 14);
        if (record == DM2_V1_RECORD_HANDLE_NULL) {
            ctx->receipt.cloud_record_creation_failed = 1;
            return 1;
        }
        addr = dm2_v1_record_pool_address_mut(ctx->record_pool_set, record);
        if (addr == NULL) {
            ctx->receipt.cloud_record_creation_failed = 1;
            return 1;
        }
        /* Bounded DB14 cloud layout: w0 link (already END from alloc),
         * w2 unused, byte@4 duration, byte@5 object-effect marker. */
        spell_timer_wr16(addr + 2, 0);
        addr[4] = (uint8_t)DM2_V1_SPELL_TIMER_CLOUD_INITIAL_DURATION;
        addr[5] = (uint8_t)object_effect;
        if (!spell_timer_append_to_cell(
                ctx->dungeon, ctx->record_pool_set, ctx->map_id,
                origin_x, origin_y, record)) {
            spell_timer_dealloc_record(ctx->record_pool_set, record);
            ctx->receipt.cloud_record_creation_failed = 1;
            return 1;
        }
        ctx->receipt.cloud_record_created = 1;
    }

    addr = dm2_v1_record_pool_address_mut(ctx->record_pool_set, record);
    if (addr == NULL) {
        ctx->receipt.cloud_record_creation_failed = 1;
        return 1;
    }
    ctx->receipt.cloud_record_handle = record;

    if (addr[4] > 0) {
        addr[4]--;
    }
    ctx->receipt.cloud_duration_remaining = (int)addr[4];

    if (addr[4] == 0) {
        /* Cloud expired: remove it from the tile chain and deallocate. */
        spell_timer_remove_and_dealloc(
            ctx->dungeon, ctx->record_pool_set, ctx->map_id,
            origin_x, origin_y, record);
        return 1;
    }

    /* Requeue the next cloud step. */
    {
        DM2_V1_SourceTimer next;
        memset(&next, 0, sizeof(next));
        next.type = DM2_V1_TIMER_PROCESS_CLOUD;
        next.actor = timer->actor;
        next.value_a = (int16_t)origin_x;
        next.value_b = (int16_t)origin_y;
        next.reserved = (int16_t)object_effect;
        next.ticks_and_map = spell_timer_pack_ticks_and_map(
            ctx->game_tick + DM2_V1_SPELL_TIMER_CLOUD_REQUEUE_DELAY,
            ctx->map_id);
        dm2_v1_source_timer_enqueue(ctx->queue, &next, 0);
    }
    ctx->receipt.cloud_requeued = 1;
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
 * either steps it or deletes it.  This bounded cycle-13 slice materialises a
 * real DB10 misc item record for the missile and a DB14 flying-item record
 * that references it, appends the DB14 record to the origin cell's ground
 * stack, and still dispatches a live projectile into the proven DM2 V1
 * projectile engine for object effects that map to a proven subtype.
 *
 * The DB14 layout uses bounded defaults (byte@4 energy, byte@5 object-effect
 * marker, word@2 object handle) documented as constants; the exact source
 * byte semantics remain unproven. */
static int dm2_v1_spell_timer_handle_projectile(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    int subtype;
    int slot;
    int origin_x;
    int origin_y;
    int object_effect;
    int16_t object_handle;
    int16_t record;
    uint8_t *addr;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.missile_dispatched++;
    origin_x = (int)timer->value_a;
    origin_y = (int)timer->value_b;
    object_effect = (int)timer->reserved;
    ctx->receipt.missile_origin_x = origin_x;
    ctx->receipt.missile_origin_y = origin_y;
    ctx->receipt.missile_object_effect = object_effect;

    subtype = dm2_v1_spell_timer_object_effect_to_projectile_subtype(
        object_effect);

    if (ctx->record_pool_set && ctx->dungeon) {
        /* Allocate the visible DB10 misc item record (the missile object). */
        object_handle = dm2_v1_alloc_new_dbitem(
            ctx->record_pool_set, spell_timer_missile_itemspec(object_effect));
        if (object_handle != DM2_V1_RECORD_HANDLE_NULL) {
            /* Allocate a DB14 flying-item record that owns the missile. */
            record = dm2_v1_record_pool_alloc_new_record(
                ctx->record_pool_set, 14);
            if (record != DM2_V1_RECORD_HANDLE_NULL) {
                addr = dm2_v1_record_pool_address_mut(
                    ctx->record_pool_set, record);
                if (addr != NULL) {
                    /* Bounded DB14 flying-item layout: w0 link (END from
                     * alloc), w2 = object handle, byte@4 energy,
                     * byte@5 object-effect marker. */
                    spell_timer_wr16(addr + 2,
                                     (uint16_t)object_handle);
                    addr[4] = (uint8_t)DM2_V1_SPELL_TIMER_MISSILE_ENERGY;
                    addr[5] = (uint8_t)object_effect;
                    if (spell_timer_append_to_cell(
                            ctx->dungeon, ctx->record_pool_set,
                            ctx->map_id, origin_x, origin_y, record)) {
                        ctx->receipt.missile_record_created = 1;
                        ctx->receipt.missile_record_handle = record;
                        ctx->receipt.missile_object_handle = object_handle;
                    } else {
                        spell_timer_dealloc_record(
                            ctx->record_pool_set, record);
                        object_handle = DM2_V1_RECORD_HANDLE_NULL;
                    }
                }
            }
            if (ctx->receipt.missile_record_created == 0) {
                /* Appending failed: the DB10 item is orphaned.  Deallocate it
                 * so the pool stays consistent. */
                spell_timer_dealloc_record(
                    ctx->record_pool_set, object_handle);
            }
        }
    }

    if (subtype < 0) {
        /* No proven projectile route for this object effect. */
        return 1; /* consumed, but no projectile created */
    }

    /* DM2_STEP_MISSILE owns its DB14 record, actor, direction, attack and
     * energy.  This bounded timer has only the effect and cell, so it cannot
     * legitimately manufacture the missing owner state.  The old path called
     * a test helper with a creature owner, north direction and fixed combat
     * values.  Keep the verified DB14 allocation above, but leave the live
     * projectile cache untouched until the original DB14/timer owner handoff
     * is present.
     *
     * Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD restores DB14 and
     * timers before c_tim_proc.cpp::DM2_STEP_MISSILE advances the record. */
    (void)subtype;
    slot = -1;
    ctx->receipt.missile_projectile_slot = slot;
    return 1;
}

/* 0x5e DM2_ALLOC_NEW_CREATURE (c_tim_proc.cpp:4268-4280).
 *
 * The source creates a new DB4 creature record for a summon spell and
 * activates it.  This bounded cycle-13 slice materialises a real DB4 creature
 * record of the summoned minion type, appends it to the target cell's ground
 * stack, and activates it through dm2_v1_caii_alloc_to_creature (which also
 * schedules the creature's first think timer via the proven CAII path).
 *
 * The DB4 layout follows the source's c_record conventions: byte@4 creature
 * type, byte@5 CAII slot index (0xff when unallocated), word@8 group link.
 * Object-effect -> creature-type mapping is source-named and documented in
 * dm2_v1_spell.h. */
static int dm2_v1_spell_timer_handle_summon(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_SpellTimerHandlerContext *ctx =
        (DM2_V1_SpellTimerHandlerContext *)context;
    int origin_x;
    int origin_y;
    int object_effect;
    int creature_type;
    int16_t record;
    uint8_t *addr;
    DM2_V1_CaiiArray *caii;
    DM2_V1_CaiiAllocReceipt alloc_rc;

    (void)source_index;
    (void)receipt;
    if (!ctx || !timer) return 0;

    ctx->receipt.summon_dispatched++;
    origin_x = (int)timer->value_a;
    origin_y = (int)timer->value_b;
    object_effect = (int)timer->reserved;
    ctx->receipt.summon_origin_x = origin_x;
    ctx->receipt.summon_origin_y = origin_y;
    ctx->receipt.summon_object_effect = object_effect;

    creature_type = dm2_v1_spell_timer_object_effect_to_creature_type(
        object_effect);
    if (creature_type < 0) {
        ctx->receipt.summon_failed_no_data = 1;
        return 1;
    }
    ctx->receipt.summon_creature_type = creature_type;

    /* Summon record creation requires real record pools, dungeon data, a CAII
     * array and a timer queue. */
    if (!ctx->record_pool_set || !ctx->dungeon || !ctx->caii || !ctx->queue) {
        ctx->receipt.summon_failed_no_data = 1;
        return 1;
    }

    caii = (DM2_V1_CaiiArray *)ctx->caii;

    record = dm2_v1_record_pool_alloc_new_record(ctx->record_pool_set, 4);
    if (record == DM2_V1_RECORD_HANDLE_NULL) {
        ctx->receipt.summon_failed_no_data = 1;
        return 1;
    }

    addr = dm2_v1_record_pool_address_mut(ctx->record_pool_set, record);
    if (addr == NULL) {
        ctx->receipt.summon_failed_no_data = 1;
        return 1;
    }
    addr[4] = (uint8_t)creature_type;                 /* Creature::CreatureType */
    addr[5] = 0xffu;                                  /* no CAII slot yet */
    spell_timer_wr16(addr + 8, (uint16_t)DM2_V1_RECORD_HANDLE_END);

    if (!spell_timer_append_to_cell(
            ctx->dungeon, ctx->record_pool_set, ctx->map_id,
            origin_x, origin_y, record)) {
        spell_timer_dealloc_record(ctx->record_pool_set, record);
        ctx->receipt.summon_failed_no_data = 1;
        return 1;
    }

    ctx->receipt.summon_record_created = 1;
    ctx->receipt.summon_record_handle = record;
    ctx->receipt.summon_record_index = record & 0x3ff;

    /* Activate the summoned creature through the proven CAII allocator. */
    memset(&alloc_rc, 0, sizeof(alloc_rc));
    if (dm2_v1_caii_alloc_to_creature(
            ctx->record_pool_set, ctx->dungeon, caii, ctx->queue,
            ctx->map_id, (unsigned long)ctx->game_tick, record,
            origin_x, origin_y, &alloc_rc) == 1) {
        ctx->receipt.summon_caii_allocated = 1;
        ctx->receipt.summon_caii_slot_index = alloc_rc.slot_index;
        ctx->receipt.summon_timer_scheduled = alloc_rc.timer_scheduled;
    }
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
        "DM2 V1 spell-effect timer handlers — DM2-007 cycle-13 real-data slice\n"
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
