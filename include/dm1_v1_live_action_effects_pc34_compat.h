#ifndef FIRESTAFF_DM1_V1_LIVE_ACTION_EFFECTS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LIVE_ACTION_EFFECTS_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB: MENU.C F0407, PROJEXPL.C F0231 and TIMELINE.C F0253.
 * This is the DM1-owned live receipt for action results.  It keeps a
 * completed action in the same tick-addressed runtime domain as spells and
 * projectiles, rather than leaving presentation to inspect a stale emission. */
enum {
    DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34 = 16,
    DM1_V1_LIVE_ACTION_EFFECT_DAMAGE_PC34 = 1,
    DM1_V1_LIVE_ACTION_EFFECT_MISS_PC34 = 2,
    DM1_V1_LIVE_ACTION_EFFECT_DOOR_PC34 = 3,
    DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34 = 4,
    DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 = 5
};

typedef struct {
    int kind;
    int championIndex;
    int actionIndex;
    int damage;
    int combatOutcome;
    int defenseDelta;
    int doorAffected;
    unsigned char remainingTicks;
    uint32_t sourceTick;
    uint32_t serial;
} DM1_V1_LiveActionEffectPc34;

typedef struct {
    DM1_V1_LiveActionEffectPc34 entries[DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34];
    int count;
    uint32_t nextSerial;
    uint32_t lastAdvancedTick;
} DM1_V1_LiveActionEffectsPc34;

typedef struct {
    int kind;
    int championIndex;
    int actionIndex;
    int damage;
    int combatOutcome;
    int defenseDelta;
    int doorAffected;
    int disabledTicks;
    uint32_t sourceTick;
} DM1_V1_LiveActionEffectInputPc34;

typedef struct {
    int valid;
    int materialized;
    int replaced;
    int slot;
} DM1_V1_LiveActionEffectReceiptPc34;

typedef struct {
    int valid;
    int advanced;
    int expiredCount;
    int expiredChampionIndex[DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34];
    int expiredActionIndex[DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34];
} DM1_V1_LiveActionEffectsAdvancePlanPc34;

void dm1_v1_live_action_effects_reset_pc34(DM1_V1_LiveActionEffectsPc34 *effects);
int dm1_v1_live_action_effect_materialize_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    const DM1_V1_LiveActionEffectInputPc34 *input,
    DM1_V1_LiveActionEffectReceiptPc34 *outReceipt);
int dm1_v1_live_action_effects_advance_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    uint32_t tick,
    DM1_V1_LiveActionEffectsAdvancePlanPc34 *outPlan);
const char *dm1_v1_live_action_effects_source_evidence_pc34(void);

#endif
