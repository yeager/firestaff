#include "dm1_v1_live_action_effects_pc34_compat.h"

#include <string.h>

const char *dm1_v1_live_action_effects_source_evidence_pc34(void)
{
    return "ReDMCSB MENU.C F0407:1053-1056,1270; PROJEXPL.C F0231:1416-1550; "
           "TIMELINE.C F0253:1574-1605";
}

void dm1_v1_live_action_effects_reset_pc34(DM1_V1_LiveActionEffectsPc34 *effects)
{
    if (effects) memset(effects, 0, sizeof(*effects));
}

int dm1_v1_live_action_effect_materialize_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    const DM1_V1_LiveActionEffectInputPc34 *input,
    DM1_V1_LiveActionEffectReceiptPc34 *outReceipt)
{
    int i;
    int slot = -1;
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!effects || !input || input->kind <= 0) return 0;
    /* F0407 has one live action-lock per champion.  Replace it in-place so
     * a later action in the same tick cannot leave an older timer alive. */
    if (input->kind == DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34) {
        for (i = 0; i < effects->count; ++i) {
            if (effects->entries[i].kind == input->kind &&
                effects->entries[i].championIndex == input->championIndex) {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0) {
        if (effects->count < DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34) {
            slot = effects->count++;
        } else {
            slot = 0;
            for (i = 1; i < effects->count; ++i) {
                if (effects->entries[i].serial < effects->entries[slot].serial) slot = i;
            }
        }
    }
    effects->entries[slot].kind = input->kind;
    effects->entries[slot].championIndex = input->championIndex;
    effects->entries[slot].actionIndex = input->actionIndex;
    effects->entries[slot].damage = input->damage;
    effects->entries[slot].combatOutcome = input->combatOutcome;
    effects->entries[slot].defenseDelta = input->defenseDelta;
    effects->entries[slot].doorAffected = input->doorAffected;
    effects->entries[slot].remainingTicks = (unsigned char)(input->disabledTicks > 255 ? 255 : input->disabledTicks);
    effects->entries[slot].sourceTick = input->sourceTick;
    effects->entries[slot].serial = ++effects->nextSerial;
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->materialized = 1;
        outReceipt->replaced = slot < effects->count - 1;
        outReceipt->slot = slot;
    }
    return 1;
}

int dm1_v1_live_action_effects_advance_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    uint32_t tick,
    DM1_V1_LiveActionEffectsAdvancePlanPc34 *outPlan)
{
    int i;
    if (outPlan) memset(outPlan, 0, sizeof(*outPlan));
    if (!effects || !outPlan || tick == effects->lastAdvancedTick) return 0;
    effects->lastAdvancedTick = tick;
    outPlan->valid = 1;
    outPlan->advanced = 1;
    for (i = 0; i < effects->count; ) {
        DM1_V1_LiveActionEffectPc34 *entry = &effects->entries[i];
        if (entry->sourceTick >= tick) { ++i; continue; }
        if (entry->remainingTicks > 0) --entry->remainingTicks;
        if (entry->remainingTicks > 0) { ++i; continue; }
        if (entry->kind == DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34 &&
            outPlan->expiredCount < DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34) {
            outPlan->expiredChampionIndex[outPlan->expiredCount] = entry->championIndex;
            outPlan->expiredActionIndex[outPlan->expiredCount] = entry->actionIndex;
            ++outPlan->expiredCount;
        }
        effects->entries[i] = effects->entries[effects->count - 1];
        --effects->count;
    }
    return 1;
}
