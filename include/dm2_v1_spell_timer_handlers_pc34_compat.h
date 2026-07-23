/* dm2_v1_spell_timer_handlers_pc34_compat.h — DM2-007 bounded spell-effect
 * timer bodies.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:3980-4230 DM2_PROCEED_TIMERS
 *         skproject/SKULLWIN/c_tim_proc.cpp:918-959  DM2_PROCESS_TIMER_LIGHT
 *         skproject/SKULLWIN/c_tim_proc.cpp:4111-4123 hero-flag 0x47 slice
 *         skproject/SKULLWIN/c_tim_proc.cpp:4129-4163 DM2_PROCESS_TIMER_0x48
 *         skproject/SKULLWIN/c_tim_proc.cpp:4165-4178 DM2_PROCESS_POISON 0x4b
 *         skproject/SKULLWIN/c_tim_proc.cpp:4195-4213 DM2_PROCESS_TIMER_19 cloud
 *         skproject/SKULLWIN/c_tim_proc.cpp:442-563   DM2_STEP_MISSILE (0x1e)
 *         skproject/SKULLWIN/c_tim_proc.cpp:4268-4280 DM2_ALLOC_NEW_CREATURE (0x5e)
 *
 * The module binds the spell timer requests emitted by
 * dm2_v1_spell_cast_player_apply() to source-named handler bodies.
 * Proven handlers:
 *   0x46 DM2_PROCESS_TIMER_LIGHT
 *   0x47 hero enchantment flag countdown
 *   0x48 per-hero enchantment power decay
 *   0x4b poison tick decay
 * Bounded cycle-12 handlers (fail-closed when real DB/creature data is absent):
 *   0x19 DM2_PROCESS_TIMER_19 cloud step
 *   0x1e DM2_STEP_MISSILE -> DM2 V1 projectile/flying-item instantiation
 *   0x5e DM2_ALLOC_NEW_CREATURE -> CAII summon-creature record instantiation
 */

#ifndef FIRESTAFF_DM2_V1_SPELL_TIMER_HANDLERS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SPELL_TIMER_HANDLERS_PC34_COMPAT_H

#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_timeline.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations — heavy headers are included only in the .c file. */
struct DM2_V1_RecordPoolSet;
struct DM2_V1_DungeonData;
/* DM2_V1_CaiiArray is an anonymous typedef, so the .c file stores it as
 * a void* and casts back after including dm2_v1_caii_alloc_pc34_compat.h. */

enum {
    DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS = 4,
    /* Source: c_tim_proc.cpp:939 — DM2_PROCESS_TIMER_LIGHT requeues every
     * 8 ticks while RG4W (the remaining duration word) is non-zero. */
    DM2_V1_SPELL_TIMER_LIGHT_REQUEUE_DELAY = 8,
    /* Source: c_tim_proc.cpp:4121 — party.hero[...].heroflag |= 0x4000.
     * The Firestaff DM2_ChampionRecord hero_flag is currently uint8_t, so
     * this bounded slice proxies the low byte of that source bitfield. */
    DM2_V1_SPELL_TIMER_HEROFLAG_AURA_BIT = 0x40,
    /* DM2-007 cycle 13: bounded defaults for cloud/missile DB14 records.
     * These stand in for source fields until the exact byte semantics are
     * proven; they are exposed as constants so tests can assert them. */
    DM2_V1_SPELL_TIMER_CLOUD_INITIAL_DURATION = 8,
    DM2_V1_SPELL_TIMER_CLOUD_REQUEUE_DELAY = 4,
    DM2_V1_SPELL_TIMER_MISSILE_ENERGY = 100,
    DM2_V1_SPELL_TIMER_MISSILE_STEP_ENERGY = 8
};

/* Per-handler observability receipt.  It records what the bounded bodies
 * actually mutated so callers can gate M11 feedback on source-named state
 * changes rather than on synthetic simulation. */
typedef struct {
    int valid;

    /* 0x46 light */
    int light_dispatched;
    int light_requeued;
    int light_adjustments;

    /* 0x47 hero enchantment flag */
    int hero_ench_dispatched;
    int hero_ench_countdown_expired;
    int hero_ench_flag_set;

    /* 0x48 enchantment power decay */
    int ench_power_dispatched;
    int ench_power_decays[DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS];

    /* 0x4b poison */
    int poison_dispatched;
    int poison_value_decays[DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS];
    int poison_strength_decays[DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS];

    /* 0x19 cloud — bounded cycle-13 real-data slice */
    int cloud_dispatched;
    int cloud_origin_x;
    int cloud_origin_y;
    int cloud_object_effect;
    int cloud_record_creation_failed; /* 1 when DB record owner absent */
    int cloud_record_created;         /* 1 when a DB14 cloud record was allocated */
    int16_t cloud_record_handle;      /* DB14 handle of the stepped record */
    int cloud_duration_remaining;     /* DB14 byte@4 after decrement */
    int cloud_requeued;               /* 1 when the timer was requeued */

    /* 0x1e missile — bounded cycle-13 real-data slice */
    int missile_dispatched;
    int missile_projectile_accepted;
    int missile_projectile_slot;
    int missile_object_effect;
    int missile_origin_x;
    int missile_origin_y;
    int missile_record_created;       /* 1 when a DB14 flying-item record was allocated */
    int16_t missile_record_handle;    /* DB14 handle */
    int16_t missile_object_handle;    /* DB item record referenced by DB14::w2 */

    /* 0x5e summon — bounded cycle-13 real-data slice */
    int summon_dispatched;
    int summon_record_created;        /* 1 when a fresh DB4 creature record was allocated */
    int16_t summon_record_handle;     /* DB4 handle */
    int summon_caii_allocated;
    int summon_caii_slot_index;
    int summon_timer_scheduled;
    int summon_record_index;
    int summon_creature_type;
    int summon_object_effect;
    int summon_origin_x;
    int summon_origin_y;
    int summon_failed_no_data; /* 1 when no real DB4/cell data available */
} DM2_V1_SpellTimerHandlerReceipt;

/* Caller-owned context.  The handlers mutate the supplied champion records
 * and/or the source-named state fields below.  NULL champions or an out-of-range
 * actor causes the matching handler to return 0 (rejected) without fabricating
 * data. */
typedef struct {
    DM2_ChampionRecord *champions;
    int champion_count;

    /* Queue used by the light handler to requeue the next 8-tick step. */
    DM2_V1_SourceTimerQueue *queue;

    /* Current tick/map used for the light requeue. */
    uint32_t game_tick;
    int map_id;

    /* Source-named runtime state proxies.  These mirror the original globals
     * without committing to a full world-model layout. */
    int16_t light_level;        /* mirrors ddat.savegames1.w_00 */
    int16_t light_remaining;    /* RG4W from DM2_PROCESS_TIMER_LIGHT */
    int hero_ench_countdown;    /* mirrors ddat.savegames1.b_02 */
    int hero_ench_target_index; /* mirrors v1e0976 - 1 */
    int16_t ench_power[DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS];
    int16_t poison_strength[DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS];

    /* DM2-007 cycle-12: DB/creature data required by cloud, missile and summon
     * handlers.  All three handlers stay fail-closed when the real data is
     * absent; these pointers are never dereferenced speculatively. */
    struct DM2_V1_RecordPoolSet *record_pool_set;
    struct DM2_V1_DungeonData *dungeon;
    void *caii; /* DM2_V1_CaiiArray *, stored opaque to avoid a heavy header. */

    DM2_V1_SpellTimerHandlerReceipt receipt;
} DM2_V1_SpellTimerHandlerContext;

/* Initialise a context.  champions may be NULL; handlers then reject mutations
 * that need a live champion record. */
void dm2_v1_spell_timer_handler_context_init(
    DM2_V1_SpellTimerHandlerContext *ctx,
    DM2_ChampionRecord *champions,
    int champion_count,
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    int map_id);

/* Cycle-12 extended init: also wire record pools, dungeon and CAII so cloud,
 * missile and summon handlers can operate on real DB/creature data when it is
 * available.  Any pointer may be NULL; the handlers fail closed when data they
 * need is absent. */
void dm2_v1_spell_timer_handler_context_init_ex(
    DM2_V1_SpellTimerHandlerContext *ctx,
    DM2_ChampionRecord *champions,
    int champion_count,
    DM2_V1_SourceTimerQueue *queue,
    uint32_t game_tick,
    int map_id,
    struct DM2_V1_RecordPoolSet *pool_set,
    struct DM2_V1_DungeonData *dungeon,
    void *caii);

/* Install the spell-effect handlers into a dispatcher.  The dispatcher's
 * handler slots are overwritten; the dispatcher's context pointer is NOT
 * changed, so callers can share the dispatcher with other handler groups. */
void dm2_v1_spell_timer_handlers_install(
    DM2_V1_TimerDispatcher *dispatcher,
    DM2_V1_SpellTimerHandlerContext *ctx);

/* Dispatch one timer through the spell-effect handlers using an explicit
 * context.  This lets a shared dispatcher (e.g. dm2_v1_runtime_tick) forward
 * spell timers to the spell-handler context while keeping its own context for
 * other handlers.  Returns 1 when a spell handler consumed the timer, 0
 * otherwise (including unknown spell timer types). */
int dm2_v1_spell_timer_dispatch(
    DM2_V1_SpellTimerHandlerContext *ctx,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt);

/* Map a DM2_OBJECT_EFFECT_* value to a DM2 projectile subtype.
 * Returns a DM2_PROJ_SUBTYPE_* constant, or -1 when the effect has no proven
 * projectile route. */
int dm2_v1_spell_timer_object_effect_to_projectile_subtype(int object_effect);

const DM2_V1_SpellTimerHandlerReceipt *dm2_v1_spell_timer_handler_receipt(
    const DM2_V1_SpellTimerHandlerContext *ctx);

const char *dm2_v1_spell_timer_handlers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SPELL_TIMER_HANDLERS_PC34_COMPAT_H */
