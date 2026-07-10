#include "dm1_v1_live_action_effects_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x, s) do { if (!(x)) { fprintf(stderr, "FAIL %s\n", s); ++failures; } } while (0)

int main(void)
{
    DM1_V1_LiveActionEffectsPc34 effects;
    DM1_V1_LiveActionEffectInputPc34 input;
    DM1_V1_LiveActionEffectsAdvancePlanPc34 advance;
    dm1_v1_live_action_effects_reset_pc34(&effects);
    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34;
    input.championIndex = 1;
    input.actionIndex = 2;
    input.disabledTicks = 2;
    input.sourceTick = 40;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "materialize lock");
    input.actionIndex = 7;
    input.disabledTicks = 3;
    CHECK(dm1_v1_live_action_effect_materialize_pc34(&effects, &input, NULL), "replace lock");
    CHECK(effects.count == 1 && effects.entries[0].actionIndex == 7, "one current lock per champion");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 40, &advance), "advance source tick");
    CHECK(effects.count == 1 && effects.entries[0].remainingTicks == 3, "source tick does not age new effect");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 41, &advance), "advance tick 41");
    CHECK(effects.entries[0].remainingTicks == 2, "lock ages once");
    CHECK(!dm1_v1_live_action_effects_advance_pc34(&effects, 41, &advance), "same tick cannot age twice");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 42, &advance), "advance tick 42");
    CHECK(dm1_v1_live_action_effects_advance_pc34(&effects, 43, &advance), "advance expiry");
    CHECK(advance.expiredCount == 1 && advance.expiredChampionIndex[0] == 1 &&
          advance.expiredActionIndex[0] == 7, "F0253 receipt on expiry");
    CHECK(effects.count == 0, "expired lock removed");
    printf("%s\n", failures ? "failed" : "ok: DM1 live action effects");
    return failures ? 1 : 0;
}
