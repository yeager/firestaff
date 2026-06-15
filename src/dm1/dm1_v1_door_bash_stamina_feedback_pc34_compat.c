/*
 * DM1 V1 door-bash stamina feedback contract implementation.
 *
 * Source-locked to ReDMCSB MENU.C:1272-1273 + MENU.C:1311-1319 +
 * MENU.C:1620-1624 + CHAMPION.C:1078-1103 F0306 +
 * CHAMPION.C:1237-1303 F0312 + CHAMPION.C:2025-2049 F0325 +
 * G0494_auc_Graphic560_ActionStamina[44] + DEFS.H:4 M005_RANDOM. See
 * include/dm1_v1_door_bash_stamina_feedback_pc34_compat.h for the
 * full anchor list.
 *
 * This is a contract-only gate: it does not play sounds, schedule
 * real timeline events, mutate the M516_CHAMPIONS array, or open
 * GRAPHICS.DAT / DUNGEON.DAT. The M11 driver still owns the
 * F0064_SOUND_RequestPlay_CPSD emission, the F0325 mutation, the
 * F0330_CHAMPION_DisableAction call, and the F0238_TIMELINE_AddEvent
 * call.
 */

#include "dm1_v1_door_bash_stamina_feedback_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB MENU.C:1311-1316 bash action set:
 *   case C030_ACTION_BASH:
 *   case C018_ACTION_HACK:
 *   case C019_ACTION_BERZERK:
 *   case C007_ACTION_KICK:
 *   case C013_ACTION_SWING:
 *   case C002_ACTION_CHOP:
 * Mirror of DM1_V1_DoorBashStaminaActionIsBashPc34; the table form
 * is used by the action-cost lookup.
 */
static const uint8_t s_door_bash_stamina_actions[] = {
    0x30, /* C030 BASH   */
    0x18, /* C018 HACK   */
    0x13, /* C019 BERZRK */
    0x07, /* C007 KICK   */
    0x0D, /* C013 SWING  */
    0x02  /* C002 CHOP   */
};

/*
 * ReDMCSB MENU.C:292-337 G0494_auc_Graphic560_ActionStamina[44]
 * byte-stable across ReDMCSB DM1 V1 variants (Atari ST, FM-Towns,
 * PC 3.4 Turbo C++ 1.01). The bash-family entries sit at the
 * positions used by MENU.C:1272-1273.
 *
 * Index map (P0788_i_ActionIndex):
 *   0x30 BASH    -> 9
 *   0x18 HACK    -> 6
 *   0x13 BERZERK -> 40
 *   0x07 KICK    -> 3
 *   0x0D SWING   -> 2
 *   0x02 CHOP    -> 10
 *
 * The other 38 entries are not consumed by the bash feedback path.
 */
static const struct {
    uint8_t action_ordinal;
    uint8_t stamina_cost;
} s_door_bash_stamina_table[] = {
    { 0x30, 9  }, /* C030 BASH   */
    { 0x18, 6  }, /* C018 HACK   */
    { 0x13, 40 }, /* C019 BERZRK */
    { 0x07, 3  }, /* C007 KICK   */
    { 0x0D, 2  }, /* C013 SWING  */
    { 0x02, 10 }  /* C002 CHOP   */
};

#define DOOR_BASH_STAMINA_TABLE_LEN_PC34 \
    (sizeof(s_door_bash_stamina_table) / \
     sizeof(s_door_bash_stamina_table[0]))

int16_t M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
    int16_t current_stamina,
    int16_t maximum_stamina,
    int16_t base_value)
{
    int16_t half_max;
    int32_t value_after_stamina;

    if (base_value <= 0) return 0;
    if (maximum_stamina <= 0) return base_value;

    half_max = maximum_stamina / 2;
    if (half_max <= 0) return base_value;

    if (current_stamina >= half_max) {
        /* ReDMCSB CHAMPION.C:1097-1102: stamina is at or above half
         * of the maximum, so the value is returned unchanged. */
        return base_value;
    }

    /*
     * ReDMCSB CHAMPION.C:1094-1095 (BUGX_XX comment):
     *     return (P0641_i_Value >>= 1)
     *          + (int16_t)(((long)P0641_i_Value
     *                       * (long)L0925_i_CurrentStamina)
     *                     / L0926_i_HalfMaximumStamina);
     * The contract pins the "first operand evaluated first" form so
     * the post-stamina value is deterministic on the bash path.
     */
    value_after_stamina = (int32_t)base_value >> 1;
    value_after_stamina +=
        (int32_t)((int32_t)(base_value >> 1) * (int32_t)current_stamina) /
        (int32_t)half_max;
    if (value_after_stamina < 0) value_after_stamina = 0;
    if (value_after_stamina > 32767) value_after_stamina = 32767;
    return (int16_t)value_after_stamina;
}

bool M11_GameView_DoorBashStaminaActionIsBashPc34(uint8_t action_ordinal)
{
    size_t i;
    for (i = 0;
         i < sizeof(s_door_bash_stamina_actions) /
             sizeof(s_door_bash_stamina_actions[0]);
         ++i) {
        if (s_door_bash_stamina_actions[i] == action_ordinal) return true;
    }
    return false;
}

bool M11_GameView_DoorBashStaminaActionCostPc34(
    uint8_t action_ordinal,
    uint8_t *out_cost,
    uint8_t *out_random_bit)
{
    size_t i;

    if (!out_cost || !out_random_bit) return false;

    /*
     * ReDMCSB MENU.C:1272-1273 always evaluates M005_RANDOM(2) even
     * for actions that do not consume stamina. The bash family
     * never has a 0 table cost, so the random bit is always added;
     * we still wire the lookup so the random-bit surface is owned
     * by the bash dispatch and not by a generic action-cost helper.
     */
    for (i = 0; i < DOOR_BASH_STAMINA_TABLE_LEN_PC34; ++i) {
        if (s_door_bash_stamina_table[i].action_ordinal == action_ordinal) {
            *out_cost = s_door_bash_stamina_table[i].stamina_cost;
            *out_random_bit =
                (action_ordinal & 0x01u) ? 1u : 0u; /* M005 default */
            return true;
        }
    }
    *out_cost = 0;
    *out_random_bit = 0;
    return false;
}

bool M11_GameView_DoorBashStaminaResolvePc34(
    const DM1_V1_DoorBashStaminaInputPc34 *input,
    DM1_V1_DoorBashStaminaResultPc34 *out)
{
    uint8_t table_cost = 0;
    uint8_t random_bit = 0;
    int16_t stamina_after;
    int16_t overflow;
    int16_t bash_strength_arg;
    bool strength_capped;

    if (!input || !out) return false;
    memset(out, 0, sizeof(*out));

    out->current_stamina_before = input->current_stamina_before;
    out->maximum_stamina = input->maximum_stamina;
    out->action_disabled_ticks = 0;
    out->destruction_delay_ticks = 0;

    if (!M11_GameView_DoorBashStaminaActionIsBashPc34(input->action_ordinal)) {
        /*
         * ReDMCSB MENU.C:1311-1316 bash dispatch is the only caller
         * for the closed-door branch. Non-bash actions fall through
         * to the generic melee/spell blocks (C024_ACTION_DISRUPT,
         * C016_ACTION_JAB, etc.) and never reach F0232 on a door.
         */
        out->strength_after_stamina =
            M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
                input->current_stamina_before,
                input->maximum_stamina,
                input->base_strength);
        out->bash_strength_arg_to_f0232 = 0;
        out->outcome = DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34;
        return true;
    }

    if (!M11_GameView_DoorBashStaminaActionCostPc34(
            input->action_ordinal, &table_cost, &random_bit)) {
        out->strength_after_stamina =
            M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
                input->current_stamina_before,
                input->maximum_stamina,
                input->base_strength);
        out->outcome = DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34;
        return true;
    }

    /*
     * ReDMCSB MENU.C:1272-1273 + DEFS.H:4: random_bit is a 1-bit
     * value, either 0 or 1. The contract pins the value the test
     * passed in (not the live F0028_MAIN_Get1BitRandomNumber) so the
     * M11 driver can drive deterministic cases; we still clamp to
     * 0/1 in case the test or caller passes 2 or higher.
     */
    if (input->random_bit > 1) {
        random_bit = (uint8_t)(input->random_bit & 0x01u);
    } else {
        random_bit = input->random_bit;
    }

    out->action_stamina_table_cost = table_cost;
    out->action_stamina_random_bit = random_bit;
    out->action_stamina_total = (int16_t)table_cost + (int16_t)random_bit;

    /*
     * ReDMCSB CHAMPION.C:1237-1303 F0312 calls F0306 on the
     * pre-RNG, pre-bounded-clip base strength. The bash flow then
     * F0026_MAIN_GetBoundedValue(0, str >> 1, 100)-clips the
     * F0306-adjusted value to [0, 100] (DUNGEON.C:561/797 melee cap
     * is 100) and passes that into F0232.
     */
    out->strength_after_stamina =
        M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
            input->current_stamina_before,
            input->maximum_stamina,
            input->base_strength);

    bash_strength_arg = (int16_t)(out->strength_after_stamina >> 1);
    strength_capped = false;
    if (bash_strength_arg < 0) bash_strength_arg = 0;
    if (bash_strength_arg > DM1_V1_DOOR_BASH_STAMINA_MELEE_CAP_PC34) {
        bash_strength_arg = DM1_V1_DOOR_BASH_STAMINA_MELEE_CAP_PC34;
        strength_capped = true;
    }
    out->bash_strength_arg_to_f0232 = bash_strength_arg;
    out->bash_strength_was_capped_to_100 = strength_capped;

    /*
     * ReDMCSB MENU.C:1311-1319 closed-door branch unconditionally
     * sets L1249_ui_ActionDisabledTicks = 6 and routes the
     * destruction event through F0232 with Ticks = 2. */
    out->action_disabled_ticks =
        DM1_V1_DOOR_BASH_STAMINA_DISABLED_TICKS_PC34;
    out->destruction_delay_ticks =
        DM1_V1_DOOR_BASH_STAMINA_DESTRUCTION_DELAY_TICKS_PC34;

    /*
     * ReDMCSB CHAMPION.C:2039-2042 F0325:
     *     L0988_i_Stamina = (L0989_ps_Champion->CurrentStamina -=
     *                        P0673_i_Decrement);
     *     if (L0988_i_Stamina <= 0) {
     *         L0989_ps_Champion->CurrentStamina = 0;
     *         F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(
     *             championIndex, (-L0988_i_Stamina) >> 1,
     *             MASK0x0000_WOUND_NONE, C0_ATTACK_NORMAL);
     *     } else if (L0988_i_Stamina > L0989_ps_Champion->MaximumStamina) {
     *         L0989_ps_Champion->CurrentStamina =
     *             L0989_ps_Champion->MaximumStamina;
     *     }
     */
    stamina_after = (int16_t)(input->current_stamina_before -
                              out->action_stamina_total);

    if (stamina_after <= 0) {
        overflow = (int16_t)((-stamina_after) >>
            DM1_V1_DOOR_BASH_STAMINA_OVERFLOW_DAMAGE_SHIFT_PC34);
        out->current_stamina_after = 0;
        out->overflow_damage = overflow;
        out->applied_pending_damage = overflow;
        out->applied_attribute_mask =
            DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34;
        out->outcome =
            (input->current_stamina_before == out->action_stamina_total)
                ? DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_EXACT_PC34
                : DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OVERFLOW_PC34;
        return true;
    }

    if (stamina_after > input->maximum_stamina) {
        /*
         * ReDMCSB CHAMPION.C:2044-2046 only fires if
         * CurrentStamina is somehow > MaximumStamina. In the bash
         * path the decrement always moves CurrentStamina down, so
         * this branch is unreachable; we still pin it so the
         * contract matches F0325 byte-for-byte.
         */
        out->current_stamina_after = input->maximum_stamina;
        out->applied_attribute_mask =
            DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34;
        out->outcome = DM1_V1_DOOR_BASH_STAMINA_OUTCOME_CLAMP_AT_MAX_PC34;
        return true;
    }

    if (out->action_stamina_total == 0) {
        /* M005_RANDOM(2) == 0 AND table cost == 0 means the action
         * consumes no stamina. CHOP/HACK/KICK/SWING/BASH/BERZERK
         * never have a 0 table cost, so this only happens for
         * actions outside the bash family; we still pin the
         * contract for the rejected-action case. */
        out->current_stamina_after = input->current_stamina_before;
        out->outcome =
            DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NO_DECREMENT_PC34;
        return true;
    }

    out->current_stamina_after = stamina_after;
    out->applied_attribute_mask =
        DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34;
    out->outcome = DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OK_PC34;
    return true;
}

const char *M11_GameView_DoorBashStaminaSourceLockPc34(void)
{
    return
        "DM1 V1 door-bash stamina feedback contract (source-locked to "
        "ReDMCSB MENU.C:1272-1273 + MENU.C:1311-1319 + MENU.C:1620-1624 "
        "+ CHAMPION.C:1078-1103 F0306 + CHAMPION.C:1237-1303 F0312 + "
        "CHAMPION.C:2025-2049 F0325 + CHAMPION.C:2042 overflow damage + "
        "G0494_auc_Graphic560_ActionStamina[44] at MENU.C:292-337 + "
        "DEFS.H:4 M005_RANDOM + DEFS.H:560-564 G0254_as_Graphic559_DoorInfo "
        "(portcullis 110, wooden 42, iron 230, ra 255; melee attacks are "
        "limited to 100 per DUNGEON.C:561/797) "
        "+ DEFS.H:1555-1580 DOOR_INFO + DEFS.H:7998-7999 F0312 anchor + "
        "DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION + DEFS.H:136-138 sound play "
        "modes). MENU.C:1272-1273 sets L1253_i_ActionStamina = "
        "G0494_auc_Graphic560_ActionStamina[P0788_i_ActionIndex] + "
        "M005_RANDOM(2). MENU.C:1311-1319 closed-door branch sets "
        "L1249_ui_ActionDisabledTicks = 6, calls F0232 with the bash "
        "strength arg from F0312, and routes the destruction event with "
        "Ticks = 2. CHAMPION.C:1078-1103 F0306 returns the post-stamina "
        "value via (val/2) + (val/2 * current / halfMax) when current is below "
        "halfMax (BUGX_XX compiler-order hazard at CHAMPION.C:1095; the "
        "contract pins the 'first operand evaluated first' form that PC 3.4 "
        "Turbo C++ 1.01 does not exhibit), else val unchanged. "
        "CHAMPION.C:1237-1303 F0312 wraps the F0306 "
        "result with F0026_MAIN_GetBoundedValue(0, str >> 1, 100). "
        "MENU.C:1620-1622 fires F0330_CHAMPION_DisableAction when "
        "ActionDisabledTicks is non-zero. MENU.C:1623-1624 fires "
        "F0325_CHAMPION_DecrementStamina when L1253_i_ActionStamina is "
        "non-zero. CHAMPION.C:2025-2049 F0325 subtracts the decrement, "
        "clamps CurrentStamina to 0, and routes the overflow via "
        "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage with damage = "
        "(-newStamina) >> 1 and MASK0x0000_WOUND_NONE / C0_ATTACK_NORMAL. "
        "CHAMPION.C:2048 sets M516.Attributes MASK0x0200_LOAD | "
        "MASK0x0100_STATISTICS. The gate is contract-only and emits no "
        "real F0325 / F0330 / F0064 / F0238 / F0321 calls; the M11 driver "
        "still owns the real mutations and timeline schedule. "
        "Companion to dm1_v1_door_bash_feedback_pc34_compat (pass777): "
        "that gate pins the F0232 dispatch + sound + ActionDisabledTicks "
        "contract with a precomputed action_strength input; the present "
        "gate pins the F0306 + F0325 + G0494 + MENU.C:1272-1273 + "
        "MENU.C:1623-1624 stamina contract that feeds the bash family "
        "specifically.";
}
