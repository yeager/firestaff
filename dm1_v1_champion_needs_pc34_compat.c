/*
 * dm1_v1_champion_needs_pc34_compat.c — DM1 V1 Champion Needs System
 *
 * Source-locked to ReDMCSB CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF.
 *
 * === ReDMCSB Source Audit ===
 *
 * F0331 (CHAMPION.C, MEDIA240 branch = DM1 Atari ST 1.2+):
 *
 * FOOD/WATER DEPLETION (per tick, per champion):
 *   AL9995_ui_StaminaGainCycleCount = 4
 *   AL9994_i_StaminaMagnitude = MaximumStamina
 *   while (CurrentStamina < (AL9994 >>= 1)): AL9995 += 2
 *
 *   AL1013_i_StaminaAmount = BoundedValue(1, (MaxStamina >> 8) - 1, 6)
 *   if (resting): AL1013 <<= 1
 *   delay = GameTime - LastPartyMovementTime
 *   if (delay > 80): AL1013++; if (delay > 250): AL1013++
 *
 *   do {
 *     staminaAboveHalf = (cycleCount <= 4)
 *     if (Food < -512):
 *       if (staminaAboveHalf): staminaLoss += staminaAmount; Food -= 2
 *     else:
 *       if (Food >= 0): staminaLoss -= staminaAmount  (gain!)
 *       Food -= staminaAboveHalf ? 2 : cycleCount >> 1
 *
 *     if (Water < -512):
 *       if (staminaAboveHalf): staminaLoss += staminaAmount; Water--
 *     else:
 *       if (Water >= 0): staminaLoss -= staminaAmount
 *       Water -= staminaAboveHalf ? 1 : cycleCount >> 2
 *   } while (--cycleCount && (CurrentStamina - staminaLoss) < MaxStamina)
 *
 *   F0325_DecrementStamina(champIdx, staminaLoss)
 *   Clamp Food to [-1024, ...]  Clamp Water to [-1024, ...]
 *
 * HP HEALING:
 *   if (CurrentHealth < MaxHealth &&
 *       CurrentStamina >= (MaxStamina >> 2) &&
 *       timeCriteria < (Vitality + 12)):
 *     gain = (MaxHealth >> 7) + 1
 *     if (resting): gain <<= 1
 *     if (EkkhardCross): gain += (gain >> 1) + 1
 *     CurrentHealth += min(gain, MaxHealth - CurrentHealth)
 *
 * MANA REGENERATION:
 *   timeCriteria computed from GameTime bit manipulation
 *   if (CurrentMana < MaxMana && timeCriteria < (Wisdom + WizPriestLevel)):
 *     manaGain = MaxMana / 40; if (resting): manaGain <<= 1; manaGain++
 *     staminaCost = manaGain * max(7, 16 - wizPriestLevel)
 *     DecrementStamina(champIdx, staminaCost)
 *     CurrentMana += min(manaGain, MaxMana - CurrentMana)
 *
 * F0325_CHAMPION_DecrementStamina (CHAMPION.C):
 *   CurrentStamina -= decrement
 *   if (stamina <= 0): stamina = 0; damage = (-stamina) >> 1
 *   if (stamina > MaxStamina): stamina = MaxStamina
 */

#include "dm1_v1_champion_needs_pc34_compat.h"

/* ── Helpers matching ReDMCSB macros ─────────────────────────────── */

static int bounded_value(int lo, int val, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

static int min_value(int a, int b) { return a < b ? a : b; }
static int max_value(int a, int b) { return a > b ? a : b; }

/* ── dm1_needs_compute_stamina_amount ─────────────────────────────── */
int dm1_needs_compute_stamina_amount(int max_stamina) {
    /* F0331: BoundedValue(1, (MaxStamina >> 8) - 1, 6) */
    return bounded_value(1, (max_stamina >> 8) - 1, 6);
}

/* ── dm1_needs_compute_health_gain ────────────────────────────────── */
int dm1_needs_compute_health_gain(int max_health, int is_resting,
                                   int has_ekkhard_cross) {
    /* F0331: gain = (MaxHealth >> 7) + 1 */
    int gain = (max_health >> 7) + 1;
    /* if resting: gain <<= 1 */
    if (is_resting) gain <<= 1;
    /* Ekkhard Cross: gain += (gain >> 1) + 1 */
    if (has_ekkhard_cross) gain += (gain >> 1) + 1;
    return gain;
}

/* ── dm1_needs_decrement_stamina ──────────────────────────────────── */
int dm1_needs_decrement_stamina(int16_t *current_stamina,
                                 int16_t max_stamina,
                                 int16_t decrement) {
    int damage = 0;
    int stamina = *current_stamina - decrement;
    if (stamina <= 0) {
        *current_stamina = 0;
        damage = (-stamina) >> 1;
    } else if (stamina > max_stamina) {
        *current_stamina = max_stamina;
    } else {
        *current_stamina = (int16_t)stamina;
    }
    return damage;
}

/* ── dm1_needs_apply_time_effects ─────────────────────────────────── */
void dm1_needs_apply_time_effects(
    const DM1_ChampionNeeds *champ,
    const DM1_NeedsTickContext *ctx,
    DM1_NeedsTickResult *out
) {
    if (!champ || !ctx || !out) return;

    out->stamina_delta = 0;
    out->health_delta = 0;
    out->mana_delta = 0;
    out->food_after = champ->food;
    out->water_after = champ->water;
    out->starvation_damage = 0;

    if (!champ->alive) return;

    /* ── Local copies for mutation during tick ──────────────────────── */
    int16_t food = champ->food;
    int16_t water = champ->water;
    int16_t cur_stamina = champ->current_stamina;
    int16_t max_stamina = champ->max_stamina;

    /*
     * ── Mana regeneration ──────────────────────────────────────────────
     * F0331: timeCriteria computed from GameTime bits.
     * (((GT & 0x80) + ((GT & 0x100) >> 2) + ((GT & 0x40) << 2)) >> 2
     */
    uint16_t gt = (uint16_t)ctx->game_time;
    uint16_t time_criteria = (((gt & 0x0080) + ((gt & 0x0100) >> 2)
                             + ((gt & 0x0040) << 2)) >> 2);

    int wiz_priest_level = champ->wizard_skill_level + champ->priest_skill_level;

    if (champ->current_mana < champ->max_mana &&
        time_criteria < (unsigned)(champ->wisdom_current + wiz_priest_level)) {
        /* manaGain = MaxMana / 40 */
        int mana_gain = champ->max_mana / 40;
        if (ctx->party_is_resting) mana_gain <<= 1;
        mana_gain++;
        /* staminaCost = manaGain * max(7, 16 - wizPriestLevel) */
        int stamina_cost = mana_gain * max_value(7, 16 - wiz_priest_level);
        /* Apply stamina cost (may cause damage) */
        int mana_damage = dm1_needs_decrement_stamina(&cur_stamina, max_stamina,
                                                       (int16_t)stamina_cost);
        out->stamina_delta -= (int16_t)stamina_cost;
        if (mana_damage > 0) out->starvation_damage = 1;
        /* Mana gain clamped */
        int actual_mana = min_value(mana_gain, champ->max_mana - champ->current_mana);
        out->mana_delta = (int16_t)actual_mana;
    } else if (champ->current_mana > champ->max_mana) {
        out->mana_delta = -1;
    }

    /*
     * ── Food/Water/Stamina cycle ──────────────────────────────────────
     * F0331: AL9995_ui_StaminaGainCycleCount starts at 4.
     */
    int cycle_count = 4;
    {
        int magnitude = max_stamina;
        while (cur_stamina < (magnitude >>= 1)) {
            cycle_count += 2;
        }
    }

    int stamina_loss = 0;
    int stamina_amount = dm1_needs_compute_stamina_amount(max_stamina);

    if (ctx->party_is_resting) {
        stamina_amount <<= 1;
    }

    uint32_t delay = ctx->game_time - ctx->last_movement_time;
    if (delay > 80) {
        stamina_amount++;
        if (delay > 250) {
            stamina_amount++;
        }
    }

    do {
        int stamina_above_half = (cycle_count <= 4);

        /* ── Food processing ────────────────────────────────────────── */
        if (food < DM1_NEEDS_STARVATION_THRESH) {
            /* Starving: lose stamina, food keeps dropping */
            if (stamina_above_half) {
                stamina_loss += stamina_amount;
                food -= 2;
            }
        } else {
            /* Normal: if food >= 0, gain stamina (loss is negative) */
            if (food >= 0) {
                stamina_loss -= stamina_amount;
            }
            food -= stamina_above_half ? 2 : (cycle_count >> 1);
        }

        /* ── Water processing ───────────────────────────────────────── */
        if (water < DM1_NEEDS_STARVATION_THRESH) {
            if (stamina_above_half) {
                stamina_loss += stamina_amount;
                water--;
            }
        } else {
            if (water >= 0) {
                stamina_loss -= stamina_amount;
            }
            water -= stamina_above_half ? 1 : (cycle_count >> 2);
        }
    } while (--cycle_count && ((cur_stamina - stamina_loss) < max_stamina));

    /* Apply net stamina change */
    int stam_damage = dm1_needs_decrement_stamina(&cur_stamina, max_stamina,
                                                   (int16_t)stamina_loss);
    out->stamina_delta += (int16_t)(-stamina_loss);  /* net: negative loss = gain */
    /* Correction: stamina_delta tracks actual change. DecrementStamina subtracts
       stamina_loss, so the actual delta is -stamina_loss (but clamped). */
    /* Recalculate precisely: */
    out->stamina_delta = cur_stamina - champ->current_stamina;
    if (stam_damage > 0) out->starvation_damage = 1;

    /* Clamp food/water to minimum */
    if (food < DM1_NEEDS_FOOD_MIN) food = (int16_t)DM1_NEEDS_FOOD_MIN;
    if (water < DM1_NEEDS_WATER_MIN) water = (int16_t)DM1_NEEDS_WATER_MIN;
    out->food_after = food;
    out->water_after = water;

    /*
     * ── HP Healing ────────────────────────────────────────────────────
     * F0331: if (CurrentHealth < MaxHealth &&
     *            CurrentStamina >= (MaxStamina >> 2) &&
     *            timeCriteria < (Vitality + 12))
     * Note: use cur_stamina which was updated by food/water cycle above.
     */
    if (champ->current_health < champ->max_health &&
        cur_stamina >= (max_stamina >> 2) &&
        time_criteria < (unsigned)(champ->vitality_current + 12)) {
        int gain = dm1_needs_compute_health_gain(champ->max_health,
                                                  ctx->party_is_resting,
                                                  champ->has_ekkhard_cross);
        gain = min_value(gain, champ->max_health - champ->current_health);
        out->health_delta = (int16_t)gain;
    }
}
