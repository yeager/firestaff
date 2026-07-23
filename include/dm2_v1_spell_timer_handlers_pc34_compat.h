/* dm2_v1_spell_timer_handlers_pc34_compat.h — DM2-007 bounded spell-effect
 * timer bodies.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:3980-4230 DM2_PROCEED_TIMERS
 *         skproject/SKULLWIN/c_tim_proc.cpp:918-959  DM2_PROCESS_TIMER_LIGHT
 *         skproject/SKULLWIN/c_tim_proc.cpp:4111-4123 hero-flag 0x47 slice
 *         skproject/SKULLWIN/c_tim_proc.cpp:4129-4163 DM2_PROCESS_TIMER_0x48
 *         skproject/SKULLWIN/c_tim_proc.cpp:4165-4178 DM2_PROCESS_POISON 0x4b
 *
 * The module binds the spell timer requests emitted by
 * dm2_v1_spell_cast_player_apply() to source-named handler bodies for the
 * effect timers that do not require unproven DB object/creature creation:
 *   0x46 DM2_PROCESS_TIMER_LIGHT
 *   0x47 hero enchantment flag countdown
 *   0x48 per-hero enchantment power decay
 *   0x4b poison tick decay
 *
 * The 0x19 cloud, 0x1e missile step and 0x5e summon timers are intentionally
 * left fail-closed in the dispatcher until their DB-record owners are proven.
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

enum {
    DM2_V1_SPELL_TIMER_HANDLER_MAX_CHAMPIONS = 4,
    /* Source: c_tim_proc.cpp:939 — DM2_PROCESS_TIMER_LIGHT requeues every
     * 8 ticks while RG4W (the remaining duration word) is non-zero. */
    DM2_V1_SPELL_TIMER_LIGHT_REQUEUE_DELAY = 8,
    /* Source: c_tim_proc.cpp:4121 — party.hero[...].heroflag |= 0x4000.
     * The Firestaff DM2_ChampionRecord hero_flag is currently uint8_t, so
     * this bounded slice proxies the low byte of that source bitfield. */
    DM2_V1_SPELL_TIMER_HEROFLAG_AURA_BIT = 0x40
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

/* Install the spell-effect handlers into a dispatcher.  The dispatcher's
 * context is set to ctx; existing non-NULL handlers are overwritten. */
void dm2_v1_spell_timer_handlers_install(
    DM2_V1_TimerDispatcher *dispatcher,
    DM2_V1_SpellTimerHandlerContext *ctx);

const DM2_V1_SpellTimerHandlerReceipt *dm2_v1_spell_timer_handler_receipt(
    const DM2_V1_SpellTimerHandlerContext *ctx);

const char *dm2_v1_spell_timer_handlers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SPELL_TIMER_HANDLERS_PC34_COMPAT_H */
