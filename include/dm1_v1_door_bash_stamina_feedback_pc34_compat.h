#ifndef FIRESTAFF_DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_PC34_COMPAT_H

/*
 * DM1 V1 door-bash stamina feedback contract.
 *
 * ReDMCSB source anchors (Atari ST DM1 V1 / I34E PC):
 *   - MENU.C lines 1272-1273: every melee action sets
 *     L1253_i_ActionStamina = G0494_auc_Graphic560_ActionStamina
 *     [P0788_i_ActionIndex] + M005_RANDOM(2). M005_RANDOM(2) returns
 *     F0028_MAIN_Get1BitRandomNumber() (a 0 or 1 random bit) per DEFS.H:4.
 *     The bash family (BASH/C030=9, HACK/C018=6, BERZERK/C019=40,
 *     KICK/C007=3, SWING/C013=2, CHOP/C002=10) draws from those bytes
 *     of the G0494 table by the same per-action index path.
 *   - MENU.C lines 1311-1319: the closed-door branch (door state
 *     C4_DOOR_STATE_CLOSED, target element C04_ELEMENT_DOOR) sets
 *     L1249_ui_ActionDisabledTicks = 6 and calls
 *     F0232_GROUP_IsDoorDestroyedByAttack(targetX, targetY,
 *     F0312_CHAMPION_GetStrength(championIndex, C01_SLOT_ACTION_HAND),
 *     C0_FALSE, 2). The strength argument is therefore the
 *     F0306-stamina-adjusted value, not the raw base strength.
 *   - MENU.C lines 1623-1624: at the end of the action dispatch, if
 *     L1253_i_ActionStamina is non-zero,
 *     F0325_CHAMPION_DecrementStamina(championIndex, L1253_i_ActionStamina)
 *     runs. The 6-tick disabled cooldown is wired separately at
 *     MENU.C:1620-1622 by F0330_CHAMPION_DisableAction when
 *     L1249_ui_ActionDisabledTicks is non-zero.
 *   - CHAMPION.C lines 1078-1103: F0306_CHAMPION_GetStaminaAdjustedValue
 *     is the stamina multiplier used to scale strength, maximum load,
 *     and other base values. It computes
 *         (value/2) + (value/2 * current / halfMax)
 *     when current < halfMax, and returns value unchanged otherwise.
 *     The "BUGX_XX" comment at line 1095 documents the historical
 *     compiler-order hazard; the contract below pins the "first
 *     operand evaluated first" interpretation that PC 3.4 Turbo C++
 *     1.01 does NOT exhibit, but the closed-door bash branch is
 *     deterministic enough to pin the spec contract.
 *   - CHAMPION.C lines 1237-1303: F0312_CHAMPION_GetStrength adds
 *     M003_RANDOM(16) + Statistics[C1_STATISTIC_STRENGTH][C1_CURRENT]
 *     to the weapon-weight / weapon-class / skill / ready-hand-wound
 *     adjustments, then calls F0306, then halves the value and runs
 *     F0026_MAIN_GetBoundedValue(0, str >> 1, 100). The bash path
 *     uses the post-F0306 value before the bounded-clip, then the
 *     caller passes that bounded value into F0232. We pin only the
 *     F0306 portion here because the bounded-clip + RNG is owned
 *     by the per-call dispatch.
 *   - CHAMPION.C lines 2025-2049: F0325_CHAMPION_DecrementStamina
 *     subtracts the decrement from CurrentStamina, clamps to 0, and
 *     routes the overflow through F0321_CHAMPION_AddPendingDamageAnd
 *     Wounds_GetDamage with damage = (-newStamina) >> 1 and
 *     MASK0x0000_WOUND_NONE / C0_ATTACK_NORMAL when the post-decrement
 *     value is <= 0. It also sets the M516_CHAMPIONS attribute
 *     MASK0x0200_LOAD | MASK0x0100_STATISTICS to schedule the redraw.
 *   - DEFS.H:4 M005_RANDOM, DEFS.H:7998-7999 F0312_CHAMPION_GetStrength,
 *     DEFS.H:7712-7719 F0238_TIMELINE_AddEvent_GetEventIndex_CPSE
 *     (door-destruction event anchor used by the bash flow), DEFS.H:560-565
 *     G0254_as_Graphic559_DoorInfo[4] (Portcullis 110, Wooden 42, Iron
 *     230, Ra 255), DEFS.H:1555-1580 DOOR_INFO struct and the
 *     C02_EVENT_DOOR_DESTRUCTION event type.
 *   - DEFS.H:5627 / 5632 / 934 / 136-138 sound play modes and
 *     C02_EVENT_DOOR_DESTRUCTION constants used by MENU.C:1313-1318.
 *
 * This is a contract-only source-lock gate: it does not load
 * GRAPHICS.DAT / DUNGEON.DAT, does not schedule real timeline events,
 * and does not play sounds. The M11 driver still owns the
 * F0064_SOUND_RequestPlay_CPSD emission, the real
 * F0325_CHAMPION_DecrementStamina mutation, and the
 * F0238_TIMELINE_AddEvent_GetEventIndex_CPSE call. The companion
 * `dm1_v1_door_bash_feedback_pc34_compat` gate (pass777) owns the
 * F0232 dispatch + sound + ActionDisabledTicks contract with a
 * precomputed `action_strength` input; the present gate owns the
 * F0306 + F0325 + G0494 + MENU.C:1272-1273 + MENU.C:1623-1624
 * stamina contract that feeds the bash family specifically.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DEFS.H:4 M005_RANDOM returns a single 0/1 bit. */
#define DM1_V1_DOOR_BASH_STAMINA_RANDOM_BITS_PC34 2

/* ReDMCSB DEFS.H:560-564 + MENU.C:561/797 melee cap. */
#define DM1_V1_DOOR_BASH_STAMINA_MELEE_CAP_PC34 100

/* ReDMCSB MENU.C:1314 closed-door ActionDisabledTicks. */
#define DM1_V1_DOOR_BASH_STAMINA_DISABLED_TICKS_PC34 6

/* ReDMCSB MENU.C:1317 F0232 destruction delay ticks. */
#define DM1_V1_DOOR_BASH_STAMINA_DESTRUCTION_DELAY_TICKS_PC34 2

/* ReDMCSB CHAMPION.C:2048 M516 attribute bits set by F0325. */
#define DM1_V1_DOOR_BASH_STAMINA_ATTR_LOAD_PC34         0x0200
#define DM1_V1_DOOR_BASH_STAMINA_ATTR_STATISTICS_PC34   0x0100
#define DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34 \
    (DM1_V1_DOOR_BASH_STAMINA_ATTR_LOAD_PC34 | \
     DM1_V1_DOOR_BASH_STAMINA_ATTR_STATISTICS_PC34)

/* ReDMCSB DEFS.H:560-564 / DUNGEON.C:560-564 / DEFS.H:1555-1569 door
 * Defense values used by the bash comparison. Mirrored from
 * dm1_v1_door_bash_feedback_pc34_compat.h so the present gate is
 * self-contained. */
#define DM1_V1_DOOR_BASH_STAMINA_DEFENSE_PORTCULLIS_PC34 110
#define DM1_V1_DOOR_BASH_STAMINA_DEFENSE_WOODEN_PC34     42
#define DM1_V1_DOOR_BASH_STAMINA_DEFENSE_IRON_PC34       230
#define DM1_V1_DOOR_BASH_STAMINA_DEFENSE_RA_PC34         255

/* ReDMCSB MENU.C:292-337 G0494_auc_Graphic560_ActionStamina[44] action
 * stamina costs for the bash family. The full 44-entry table is
 * byte-stable across all ReDMCSB DM1 V1 variants; we expose only the
 * six bash-family entries here because the bash dispatch is the only
 * caller for this gate. */
#define DM1_V1_DOOR_BASH_STAMINA_COST_BASH_PC34     9
#define DM1_V1_DOOR_BASH_STAMINA_COST_HACK_PC34     6
#define DM1_V1_DOOR_BASH_STAMINA_COST_BERZERK_PC34  40
#define DM1_V1_DOOR_BASH_STAMINA_COST_KICK_PC34     3
#define DM1_V1_DOOR_BASH_STAMINA_COST_SWING_PC34    2
#define DM1_V1_DOOR_BASH_STAMINA_COST_CHOP_PC34     10

/* ReDMCSB MENU.C:1311-1316 bash action ordinals (matches the
 * DM1_V1_ACTION_*_PC34 set used by dm1_v1_door_bash_feedback_pc34_compat). */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_BASH_PC34     0x30 /* C030 */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_HACK_PC34     0x18 /* C018 */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_BERZERK_PC34  0x13 /* C019 */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_KICK_PC34     0x07 /* C007 */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_SWING_PC34    0x0D /* C013 */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_CHOP_PC34     0x02 /* C002 */

/* ReDMCSB CHAMPION.C:2042 overflow-damage shift. */
#define DM1_V1_DOOR_BASH_STAMINA_OVERFLOW_DAMAGE_SHIFT_PC34 1

/* ReDMCSB DEFS.H:4 + CHAMPION.C:2042 + MENU.C:1272 action index cap. */
#define DM1_V1_DOOR_BASH_STAMINA_ACTION_INDEX_MAX_PC34 43

typedef enum {
    DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34 = 0,    /* non-bash action */
    DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NO_DECREMENT_PC34,    /* M005=0 + table=0 (no stamina cost) */
    DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OK_PC34,    /* decrement < current, no overflow */
    DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_EXACT_PC34, /* decrement == current, no overflow damage */
    DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OVERFLOW_PC34, /* decrement > current, overflow damage */
    DM1_V1_DOOR_BASH_STAMINA_OUTCOME_CLAMP_AT_MAX_PC34     /* post-decrement would exceed max, clamped */
} DM1_V1_DoorBashStaminaOutcomePc34;

typedef struct {
    int16_t current_stamina_before;     /* M516.CurrentStamina at the entry */
    int16_t maximum_stamina;            /* M516.MaximumStamina (byte-stable) */
    uint8_t action_ordinal;             /* MENU.C:1311-1316 bash set */
    uint8_t random_bit;                 /* M005_RANDOM(2) bit (0 or 1) */
    int16_t base_strength;              /* F0312 strength pre-F0306 / pre-bounded-clip */
    int16_t strength_after_stamina;     /* F0306 result on the base_strength */
} DM1_V1_DoorBashStaminaInputPc34;

typedef struct {
    /* ReDMCSB CHAMPION.C:1090-1103 F0306 result: the per-call
     * stamina-adjusted base strength that the bash path passes to
     * F0232 via F0312. Always in [0, 32767]; for the bash flow the
     * F0026_MAIN_GetBoundedValue(0, str >> 1, 100) clip and the
     * M003_RANDOM(16) addition happen in the F0312 caller, so this
     * gate only owns the F0306 part. */
    int16_t strength_after_stamina;

    /* ReDMCSB MENU.C:1272-1273 + G0494_auc_Graphic560_ActionStamina. */
    uint8_t action_stamina_table_cost;
    uint8_t action_stamina_random_bit;
    int16_t action_stamina_total;

    /* ReDMCSB CHAMPION.C:2039-2042 F0325 result. */
    int16_t current_stamina_before;
    int16_t current_stamina_after;
    int16_t maximum_stamina;
    int16_t overflow_damage;            /* (-newStamina) >> 1; 0 if no overflow */
    int16_t applied_pending_damage;     /* mirrors overflow_damage for F0321 input */

    /* ReDMCSB CHAMPION.C:2048 attribute mask (LOAD|STATISTICS). */
    uint16_t applied_attribute_mask;

    /* ReDMCSB MENU.C:1314 + MENU.C:1620-1622. */
    uint8_t action_disabled_ticks;

    /* ReDMCSB MENU.C:1317. */
    uint8_t destruction_delay_ticks;

    /* ReDMCSB F0026_MAIN_GetBoundedValue clip on the bash strength
     * argument to F0232 (the pre-cap melee cap = 100 from
     * DUNGEON.C:561 / 797). */
    int16_t bash_strength_arg_to_f0232;
    bool bash_strength_was_capped_to_100;

    DM1_V1_DoorBashStaminaOutcomePc34 outcome;
} DM1_V1_DoorBashStaminaResultPc34;

/* ReDMCSB CHAMPION.C:1078-1103 F0306_CHAMPION_GetStaminaAdjustedValue.
 * Returns the post-stamina value passed to F0026_MAIN_GetBoundedValue
 * inside F0312's bash path.
 *
 * If current_stamina < maximum_stamina / 2, returns
 *   (value / 2) + ((value / 2) * current_stamina) / (maximum_stamina / 2)
 * Otherwise returns value unchanged. The contract pins the
 * "first operand evaluated first" interpretation called out in
 * the BUGX_XX comment at CHAMPION.C:1095. */
int16_t M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
    int16_t current_stamina,
    int16_t maximum_stamina,
    int16_t base_value);

/* ReDMCSB MENU.C:1272-1273 + G0494_auc_Graphic560_ActionStamina. Looks
 * up the per-action stamina cost (table cost) for the bash family
 * action ordinal, returning true and writing the table cost + the
 * random bit roll into out_cost / out_random_bit; returns false for
 * non-bash actions or out-of-range action ordinals. */
bool M11_GameView_DoorBashStaminaActionCostPc34(
    uint8_t action_ordinal,
    uint8_t *out_cost,
    uint8_t *out_random_bit);

/* ReDMCSB MENU.C:1272-1273 + CHAMPION.C:2025-2049 F0325 +
 * CHAMPION.C:1237-1303 F0312 + MENU.C:1311-1319 closed-door branch
 * + MENU.C:1620-1624 post-action hook. Runs the stamina feedback
 * contract for one bash action and writes the result. The
 * `random_bit` field on the input lets the test pin the M005_RANDOM
 * outcome without touching the global RNG; the production caller
 * passes whatever F0028_MAIN_Get1BitRandomNumber() returned. */
bool M11_GameView_DoorBashStaminaResolvePc34(
    const DM1_V1_DoorBashStaminaInputPc34 *input,
    DM1_V1_DoorBashStaminaResultPc34 *out);

/* ReDMCSB MENU.C:1311-1316 bash group is
 * { C030 BASH, C018 HACK, C019 BERZERK, C007 KICK, C013 SWING,
 *   C002 CHOP }. */
bool M11_GameView_DoorBashStaminaActionIsBashPc34(uint8_t action_ordinal);

const char *M11_GameView_DoorBashStaminaSourceLockPc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_PC34_COMPAT_H */
