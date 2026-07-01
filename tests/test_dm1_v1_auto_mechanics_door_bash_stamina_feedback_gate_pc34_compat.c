/*
 * DM1 V1 auto-mechanics door-bash stamina feedback gate.
 *
 * Pinned ReDMCSB path: MENU.C:1311-1319 closed-door bash branch
 * combined with the post-action hook at MENU.C:1618-1633 (F0330 +
 * F0325). The chained contract verifies that the bash feedback half
 * (sound + F0232 dispatch + destruction event) and the bash stamina
 * feedback half (F0306 + F0325 + F0330) produce byte-stable
 * cross-contract fields when called together for the SAME (action,
 * door, strength) input.
 *
 * Disjoint with:
 *   - tests/test_dm1_v1_door_bash_feedback_source_lock_pc34_compat.c
 *     (90 assertions, single half, sound + F0232 dispatch + 6-tick
 *     cooldown contract)
 *   - tests/test_dm1_v1_door_bash_stamina_feedback_source_lock_pc34_compat.c
 *     (121 assertions, single half, F0306 + F0325 + G0494 + disabled_ticks
 *     + destruction_delay contract)
 *   - tests/test_dm1_v1_door_bash_sound_no_open_gate_pc34_compat.c
 *     (185 assertions, bash-but-no-open invariant, single-contract)
 *
 * This is contract-only; it does not load GRAPHICS.DAT / DUNGEON.DAT,
 * does not drive the live M11 action input, does not schedule a real
 * F0064 / F0238 / F0325 call, and does not claim original DOS pixel
 * parity.
 */

#include "dm1_v1_door_bash_stamina_feedback_chain_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

/* FNV-1a 32-bit, seeded with the chained contract identity. */
#define FNV1A_OFFSET_BASIS_PC34 0x811C9DC5u
#define FNV1A_PRIME_PC34       0x01000193u

static uint32_t fnv1a_u8(uint32_t hash, uint8_t value)
{
    hash ^= (uint32_t)value;
    hash *= FNV1A_PRIME_PC34;
    return hash;
}

static uint32_t fnv1a_i16(uint32_t hash, int16_t value)
{
    return fnv1a_u8(hash, (uint8_t)(value & 0xFFu)) +
        0u * fnv1a_u8(0u, (uint8_t)((value >> 8) & 0xFFu));
}

static void expect_bool(const char *id, int got, int want,
                        const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_u8(const char *id, uint8_t got, uint8_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", id, (unsigned)got,
               (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %u anchor=%s\n", id, (unsigned)want, anchor);
    }
}

static void expect_i16(const char *id, int16_t got, int16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, (int)got,
               (int)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, (int)want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing='%s' anchor=%s\n", id,
               needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains '%s' anchor=%s\n", id, needle, anchor);
    }
}

/*
 * Build a chain input. The closed-door bash family is the only target;
 * the test uses {BASH=0x30, HACK=0x18, BERZRK=0x13, KICK=0x07, SWING=0x0D,
 * CHOP=0x02}.
 */
static void build_closed_door_input(
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 *in,
    uint8_t action_ordinal,
    uint8_t door_type,
    uint8_t door_attributes,
    uint8_t door_defense,
    int16_t current_stamina,
    int16_t maximum_stamina,
    int16_t base_strength,
    uint8_t random_bit)
{
    memset(in, 0, sizeof(*in));
    in->door_type = door_type;
    in->door_attributes = door_attributes;
    in->door_defense = door_defense;
    in->door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    in->target_element = DM1_V1_ELEMENT_DOOR_PC34;
    in->action_ordinal = action_ordinal;
    in->magic_attack = false;
    in->is_door_target = true;
    in->current_stamina = current_stamina;
    in->maximum_stamina = maximum_stamina;
    in->base_strength = base_strength;
    in->random_bit = random_bit;
}

/*
 * One full sweep over the closed-door bash matrix. For each of the six
 * bash actions, verify the chained contract invariants.
 */
static void test_closed_door_bash_chain_six_actions(void)
{
    /* ReDMCSB MENU.C:1311-1316 bash family. */
    static const struct {
        uint8_t action;
        const char *name;
    } kBashActions[6] = {
        { 0x30, "C030 BASH"    },
        { 0x18, "C018 HACK"    },
        { 0x13, "C019 BERZRK"  },
        { 0x07, "C007 KICK"    },
        { 0x0D, "C013 SWING"   },
        { 0x02, "C002 CHOP"    }
    };
    int i;

    for (i = 0; i < 6; ++i) {
        DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
        DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
        bool ok;

        build_closed_door_input(&in,
                                kBashActions[i].action,
                                DM1_V1_DOOR_INFO_WOODEN_PC34,
                                0,
                                DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                                /* cur/max stamina */
                                50, 100,
                                /* base strength high enough that
                                 * F0026 clip lands at >= 42 (wooden
                                 * defense), producing WOODEN_BREAK
                                 * outcome in the bash feedback half. */
                                100,
                                /* random_bit=0 */
                                0);
        ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
        expect_bool("closed.resolve", ok, true,
                    "MENU.C:1311-1319 chain resolve");
        expect_bool("closed.chain_alignment_ok",
                    out.chain_alignment_ok, true,
                    "MENU.C:1311-1319 + MENU.C:1618-1633 chain invariants");
        expect_bool("closed.action_ordinal_match",
                    out.action_ordinal_match, true,
                    "Both halves received same action ordinal");
        expect_bool("closed.disabled_ticks_aligned",
                    out.closed_door_bash_disabled_ticks_aligned, true,
                    "MENU.C:1317 + MENU.C:1620-1622 both halves report 6");
        expect_bool("closed.magic_diverge",
                    out.magic_attack_disabled_ticks_diverge, true,
                    "Non-magic path → no divergence");
        expect_bool("closed.destruction_delay_aligned_on_destroy",
                    out.destruction_delay_aligned_on_destroy, true,
                    "MENU.C:1317 + PROJEXPL.C:1554-1600 both halves report 2");
        expect_bool("closed.bash_strength_arg_in_cap",
                    out.bash_strength_arg_in_cap, true,
                    "CHAMPION.C:1302 F0026 clip ≤ 100");
        expect_bool("closed.melee_capped_flag_known_divergence",
                    out.melee_capped_flag_known_divergence, true,
                    "No cap divergence when the stamina cap is not reached");
        expect_bool("closed.bash_stamina_fired",
                    out.bash_stamina_fired_when_closed_bash, true,
                    "MENU.C:1272 + CHAMPION.C:2025 F0325 fires");
        expect_bool("closed.bash_feedback_outcome_for_melee_closed",
                    out.bash_feedback_outcome_for_melee_closed, true,
                    "PROJEXPL.C:1584-1600 melee outcome bucket");
        expect_bool("closed.bash_feedback_outcome_for_not_door",
                    out.bash_feedback_outcome_for_not_door, true,
                    "Door target → not NO_DOOR");
        expect_bool("closed.bash_feedback_outcome_for_not_closed",
                    out.bash_feedback_outcome_for_not_closed, true,
                    "Closed door → not NOT_CLOSED");
        expect_u8("closed.feedback.disabled_ticks",
                  out.feedback.disabled_ticks,
                  DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
                  "MENU.C:1317 L1249 = 6");
        expect_u8("closed.stamina.action_disabled_ticks",
                  out.stamina.action_disabled_ticks,
                  DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
                  "MENU.C:1620-1622 F0330 = 6");
        expect_u8("closed.feedback.destruction_delay",
                  out.feedback.destruction_delay_ticks,
                  DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34,
                  "MENU.C:1317 PROJEXPL.C:1554-1600 delay = 2");
        expect_u8("closed.stamina.destruction_delay",
                  out.stamina.destruction_delay_ticks,
                  DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34,
                  "MENU.C:1317 + CHAMPION.C:2048 delay = 2");
        /* Wood + strength >= 42 → WOODEN_BREAK bucket. */
        expect_int("closed.outcome", (int)out.outcome,
                   (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BREAK_PC34,
                   "PROJEXPL.C:1584-1600 wooden break");
        /* Strength 80 < 100 cap so both melee_capped flags are false. */
        expect_bool("closed.melee_capped_false",
                    out.feedback.melee_capped_to_100, false,
                    "DUNGEON.C:561 melee cap not reached");
        expect_bool("closed.stamina_capped_false",
                    out.stamina.bash_strength_was_capped_to_100, false,
                    "CHAMPION.C:1302 F0026 clip not reached");
    }
}

/*
 * Wood + strength below defense: bash bounces. Verify destruction_delay
 * = 0 (F0232 does not schedule the event because Attack < Defense) but
 * disabled_ticks = 6 (the bash attempt already happened).
 */
static void test_closed_door_wooden_bounce_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30, /* BASH cost 9 */
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            50, 100,
                            /* strength 30 < wooden defense 42 → bounce */
                            30,
                            0);
    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("bounce.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("bounce.chain_alignment_ok", out.chain_alignment_ok, true,
                "MENU.C:1311-1319 + MENU.C:1618-1633 chain invariants");
    expect_int("bounce.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BOUNCE_PC34,
               "PROJEXPL.C:1583-1599 bounce");
    /* Cooldown still applies (MENU.C:1317 + MENU.C:1620-1622). */
    expect_u8("bounce.disabled_ticks",
              out.feedback.disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1317 6-tick cooldown on bounce");
    expect_u8("bounce.stamina_disabled_ticks",
              out.stamina.action_disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1620-1622 F0330 6-tick cooldown on bounce");
    /* No destruction event scheduled. */
    expect_u8("bounce.destruction_delay_zero",
              out.feedback.destruction_delay_ticks, 0,
              "PROJEXPL.C:1583-1599 no destruction event on bounce");
    expect_u8("bounce.stamina_destruction_delay",
              out.stamina.destruction_delay_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34,
              "Stamina half reports the bash destruction delay for any bash action");
    expect_bool("bounce.destruction_delay_bounce_divergence",
                out.destruction_delay_bounce_divergence, true,
                "Feedback has no destruction event while stamina reports delay 2");
    /* Stamina decrement still applies (cost 9). */
    expect_i16("bounce.stamina_after",
               out.stamina.current_stamina_after,
               (int16_t)(50 - 9),
               "MENU.C:1272 BASH=9 + CHAMPION.C:2025 decrement");
    /* BASH feedback's melee-cap flag is false (strength 30 < 100 cap). */
    expect_bool("bounce.feedback_melee_capped",
                out.feedback.melee_capped_to_100, false,
                "DUNGEON.C:561 melee cap not reached");
    expect_bool("bounce.stamina_melee_capped",
                out.stamina.bash_strength_was_capped_to_100, false,
                "Stamina F0026 clip not reached");
}

/*
 * Iron door + strength 250 (above melee cap 100): iron rejects but
 * cooldown still applies.
 */
static void test_closed_door_iron_reject_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30, /* BASH cost 9 */
                            DM1_V1_DOOR_INFO_IRON_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_IRON_PC34,
                            50, 100,
                            250, /* melee cap 100 fires */
                            0);
    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("iron.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("iron.chain_alignment_ok", out.chain_alignment_ok, true,
                "MENU.C:1311-1319 + MENU.C:1618-1633 chain invariants");
    expect_int("iron.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_IRON_REJECT_PC34,
               "PROJEXPL.C:1554-1580 iron reject");
    expect_u8("iron.disabled_ticks",
              out.feedback.disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1317 6-tick cooldown on iron reject");
    /* Stamina still applies (F0325 has no defense gate). */
    expect_i16("iron.stamina_after",
               out.stamina.current_stamina_after,
               (int16_t)(50 - 9),
               "CHAMPION.C:2025 F0325 fires on iron reject");
    expect_bool("iron.feedback_melee_capped_after_chain",
                out.feedback.melee_capped_to_100, false,
                "Feedback receives the already-capped F0026 strength");
    expect_bool("iron.stamina_melee_capped",
                out.stamina.bash_strength_was_capped_to_100, true,
                "CHAMPION.C:1302 F0026 clip at 100");
    expect_bool("iron.melee_capped_flag_known_divergence",
                out.melee_capped_flag_known_divergence, true,
                "Stamina records the cap while feedback sees the capped value");
    expect_i16("iron.bash_arg_in_cap",
               out.stamina.bash_strength_arg_to_f0232,
               DM1_V1_DOOR_MELEE_ATTACK_CAP_PC34,
               "DUNGEON.C:561 melee cap = 100");
}

/*
 * Portcullis + strength 100 (cap not reached) + below port defense 110:
 * bounce.
 */
static void test_closed_door_portcullis_bounce_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_PORTCULLIS_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34,
                            50, 100,
                            100, /* strength 100 < port defense 110 */
                            0);
    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("port.bounce.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("port.bounce.chain_alignment_ok",
                out.chain_alignment_ok, true,
                "MENU.C:1311-1319 chain invariants");
    expect_int("port.bounce.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_PORT_BREAK_PC34,
               "PROJEXPL.C:1584 port break path");
    /* Strength 100 == cap → both melee_capped flags are false
     * (the cap is a max-value clip; equality is not capped). */
    expect_bool("port.bounce.feedback_capped",
                out.feedback.melee_capped_to_100, false,
                "DUNGEON.C:561 cap 100 not exceeded");
    expect_bool("port.bounce.stamina_capped",
                out.stamina.bash_strength_was_capped_to_100, false,
                "CHAMPION.C:1302 F0026 clip not reached");
    expect_bool("port.bounce.melee_capped_aligned",
                out.melee_capped_flag_known_divergence, true,
                "No cap divergence when the stamina cap is not reached");
}

/*
 * Non-closed door: open door (state 0). The bash feedback reports
 * NOT_CLOSED; the bash stamina contract still applies F0325 because
 * the action itself was performed, just not against a closed door.
 */
static void test_open_door_no_closed_branch_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            50, 100,
                            80, 0);
    /* Open door: state = 0 (C0_DOOR_STATE_OPEN). */
    in.door_state = 0;

    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("open.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("open.chain_alignment_ok", out.chain_alignment_ok, true,
                "Not-closed chain invariants");
    expect_int("open.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_CLOSED_PC34,
               "MENU.C:1311-1319 closed precondition");
    expect_int("open.feedback_outcome",
               (int)out.feedback.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34,
               "MENU.C:1311-1319 NOT_CLOSED bucket");
    /* Bash feedback contract: closed precondition fails → cooldown 0. */
    expect_u8("open.disabled_ticks_zero",
              out.feedback.disabled_ticks, 0,
              "MENU.C:1317 closed-only 6-tick cooldown");
    /* Bash stamina contract: F0325 still fires (cost 9). */
    expect_i16("open.stamina_after",
               out.stamina.current_stamina_after,
               (int16_t)(50 - 9),
               "CHAMPION.C:2025 F0325 fires regardless of closed state");
    expect_u8("open.stamina_disabled_ticks",
              out.stamina.action_disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1620-1622 F0330 disabled_ticks = 6 for bash actions");
    expect_bool("open.disabled_ticks_diverge",
                out.open_door_disabled_ticks_diverge, true,
                "Open door keeps feedback disabled_ticks=0 while stamina reports 6");
    expect_u8("open.stamina_destruction_delay",
              out.stamina.destruction_delay_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34,
              "Stamina reports bash destruction delay even when feedback has no event");
    expect_bool("open.not_closed_outcome",
                out.bash_feedback_outcome_for_not_closed, true,
                "Not-closed → NOT_CLOSED bucket");
}

/*
 * Magic attack on a closed door: bash feedback rejects (MELEE_REJECTED,
 * disabled_ticks=0), stamina still decrements (action_disabled_ticks=6).
 * This is the magic_attack_disabled_ticks_diverge invariant.
 */
static void test_magic_attack_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            50, 100,
                            80, 0);
    in.magic_attack = true;

    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("magic.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("magic.chain_alignment_ok", out.chain_alignment_ok, true,
                "Magic attack chain invariants");
    expect_int("magic.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_MELEE_REJECTED_PC34,
               "PROJEXPL.C:1580-1582 magic reject");
    /* Bash feedback contract: MELEE_REJECTED → disabled_ticks = 0. */
    expect_u8("magic.disabled_ticks_zero",
              out.feedback.disabled_ticks, 0,
              "MENU.C:1317 magic-only 0 cooldown");
    /* Bash stamina contract: still decrements (F0325 has no magic gate). */
    expect_i16("magic.stamina_after",
               out.stamina.current_stamina_after,
               (int16_t)(50 - 9),
               "CHAMPION.C:2025 F0325 fires on magic attack");
    expect_u8("magic.stamina_disabled_ticks",
              out.stamina.action_disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1620-1622 F0330 = 6 on magic attack");
    /* The chained contract should surface the known divergence. */
    expect_bool("magic.diverge",
                out.magic_attack_disabled_ticks_diverge, true,
                "PROJEXPL.C:1580 magic reject keeps feedback=0 while "
                "stamina=6 (known divergence)");
}

/*
 * Non-bash action: bash stamina short-circuits with NOT_BASH; bash
 * feedback reports NO_DOOR (the bash feedback contract does not gate
 * on action ordinal).
 */
static void test_non_bash_action_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x14, /* C020 FIREBALL, not a bash */
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            50, 100,
                            80, 0);

    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("nonbash.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("nonbash.chain_alignment_ok", out.chain_alignment_ok, true,
                "Non-bash chain invariants");
    expect_int("nonbash.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_BASH_PC34,
               "MENU.C:1272 non-bash short-circuit");
    expect_int("nonbash.stamina_outcome",
               (int)out.stamina.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34,
               "MENU.C:1311-1316 non-bash → NOT_BASH bucket");
    expect_int("nonbash.feedback_outcome",
               (int)out.feedback.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34,
               "Bash feedback still evaluates the door target; chain maps action to NOT_BASH");
    expect_bool("nonbash.short_circuits",
                out.bash_stamina_skips_for_non_bash, true,
                "MENU.C:1311-1316 short-circuit invariant");
    /* Stamina's `action_stamina_table_cost` is 0 because the bash family
     * table does not include C020 FIREBALL. */
    expect_u8("nonbash.stamina_table_cost_zero",
              out.stamina.action_stamina_table_cost, 0,
              "MENU.C:1272 non-bash lookup returns 0");
}

/*
 * Non-door target: bash feedback reports NO_DOOR; bash stamina contract
 * still fires F0325 because the action was performed (just not against
 * a door).
 */
static void test_non_door_target_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            50, 100,
                            80, 0);
    /* Target is not a door. */
    in.is_door_target = false;
    in.target_element = 0; /* C00_ELEMENT_FLOOR or similar */

    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("nondoor.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("nondoor.chain_alignment_ok", out.chain_alignment_ok, true,
                "Non-door chain invariants");
    expect_int("nondoor.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NO_DOOR_PC34,
               "MENU.C:1311 closed-door precondition fails");
    expect_int("nondoor.feedback_outcome",
               (int)out.feedback.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34,
               "Bash feedback NO_DOOR");
    expect_u8("nondoor.disabled_ticks_zero",
              out.feedback.disabled_ticks, 0,
              "MENU.C:1317 closed-only 6-tick cooldown");
    /* Stamina decrement still applies. */
    expect_i16("nondoor.stamina_after",
               out.stamina.current_stamina_after,
               (int16_t)(50 - 9),
               "CHAMPION.C:2025 F0325 fires regardless of door target");
    expect_bool("nondoor.outcome_for_not_door",
                out.bash_feedback_outcome_for_not_door, true,
                "Non-door target → NO_DOOR bucket");
}

/*
 * F0306 strength collapse: stamina below halfMax collapses the bash
 * strength arg. Verify both halves see the same value.
 */
static void test_f0306_strength_collapse_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    /* cur=25, max=100 → halfMax=50. base_strength=50 lands at
     * (50/2) + (25*25)/50 = 25 + 12 = 37. str>>1 = 18. */
    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            25, 100,
                            50, 0);

    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("collapse.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("collapse.chain_alignment_ok", out.chain_alignment_ok, true,
                "F0306 collapse chain invariants");
    expect_i16("collapse.f0306",
               out.stamina.strength_after_stamina,
               37,
               "CHAMPION.C:1095 val=50 current=25 halfMax=50");
    expect_i16("collapse.bash_arg",
               out.stamina.bash_strength_arg_to_f0232,
               18,
               "CHAMPION.C:1302 F0026 clip 0..100");
    expect_bool("collapse.feedback_capped_aligned",
                out.melee_capped_flag_known_divergence, true,
                "No cap divergence when the stamina cap is not reached");
    /* 18 < wooden defense 42 → WOODEN_BOUNCE bucket. */
    expect_int("collapse.outcome",
               (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BOUNCE_PC34,
               "PROJEXPL.C:1583-1599 bounce when below defense");
}

/*
 * Stamina overflow: cur=5, BASH cost=9+0=9 → stamina=−4, overflow
 * damage = 4 >> 1 = 2. Bash stamina reports DECREMENT_OVERFLOW; bash
 * feedback's destruction_delay stays at 2 (F0232 schedules the event
 * regardless of stamina cost).
 */
static void test_stamina_overflow_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            5, /* cur 5 < BASH 9 */
                            100,
                            80, 0);

    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("overflow.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("overflow.chain_alignment_ok", out.chain_alignment_ok, true,
                "Overflow chain invariants");
    expect_i16("overflow.stamina_after",
               out.stamina.current_stamina_after, 0,
               "CHAMPION.C:2042 F0325 clamps to 0");
    expect_i16("overflow.damage",
               out.stamina.overflow_damage, 2,
               "CHAMPION.C:2042 overflow_damage = (4) >> 1 = 2");
    expect_int("overflow.stamina_outcome",
               (int)out.stamina.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OVERFLOW_PC34,
               "CHAMPION.C:2042 F0325 overflow bucket");
    /* Bash feedback cooldown still applies. */
    expect_u8("overflow.disabled_ticks",
              out.feedback.disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1317 6-tick cooldown on overflow");
    expect_u8("overflow.stamina_disabled_ticks",
              out.stamina.action_disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1620-1622 F0330 6-tick cooldown on overflow");
}

/*
 * Random bit flips: BASH cost 9 + random_bit=1 = 10. Both halves share
 * the random bit in the bash stamina contract; bash feedback is
 * unaffected.
 */
static void test_random_bit_chain(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    bool ok;

    build_closed_door_input(&in,
                            0x30,
                            DM1_V1_DOOR_INFO_WOODEN_PC34,
                            0,
                            DM1_V1_DOOR_DEFENSE_WOODEN_PC34,
                            50, 100,
                            80, 1 /* M005_RANDOM(2) == 1 */
    );
    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
    expect_bool("random.resolve", ok, true,
                "MENU.C:1311-1319 chain resolve");
    expect_bool("random.chain_alignment_ok", out.chain_alignment_ok, true,
                "Random bit chain invariants");
    expect_u8("random.table_cost",
              out.stamina.action_stamina_table_cost, 9,
              "MENU.C:1272 G0494[30]=9");
    expect_u8("random.random_bit",
              out.stamina.action_stamina_random_bit, 1,
              "DEFS.H:4 M005_RANDOM(2) = 1");
    expect_i16("random.total_cost",
               out.stamina.action_stamina_total, 10,
               "MENU.C:1272 table+random = 10");
    expect_i16("random.stamina_after",
               out.stamina.current_stamina_after,
               (int16_t)(50 - 10),
               "CHAMPION.C:2025 F0325 decrement");
    expect_u8("random.disabled_ticks",
              out.feedback.disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34,
              "MENU.C:1317 6-tick cooldown");
}

/*
 * Deterministic FNV-1a hash over the chained contract sweep. Verifies
 * the chain invariants are stable across runs.
 */
static void test_deterministic_hash_stable(void)
{
    static const struct {
        uint8_t door_type;
        uint8_t door_defense;
        int16_t base_strength;
    } kDoors[4] = {
        { DM1_V1_DOOR_INFO_WOODEN_PC34, DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 80 },
        { DM1_V1_DOOR_INFO_WOODEN_PC34, DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 30 },
        { DM1_V1_DOOR_INFO_IRON_PC34, DM1_V1_DOOR_DEFENSE_IRON_PC34, 250 },
        { DM1_V1_DOOR_INFO_PORTCULLIS_PC34, DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34, 100 }
    };
    static const uint8_t kActions[6] = { 0x30, 0x18, 0x13, 0x07, 0x0D, 0x02 };
    static const uint8_t kRbits[2] = { 0, 1 };
    uint32_t hash1 = FNV1A_OFFSET_BASIS_PC34;
    uint32_t hash2 = FNV1A_OFFSET_BASIS_PC34;
    int i, j, k, l;

    for (i = 0; i < 6; ++i) {
        for (j = 0; j < 2; ++j) {
            for (k = 0; k < 4; ++k) {
                for (l = 0; l < 2; ++l) {
                    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
                    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
                    bool ok;
                    build_closed_door_input(&in,
                                            kActions[i],
                                            kDoors[k].door_type,
                                            0,
                                            kDoors[k].door_defense,
                                            50, 100,
                                            kDoors[k].base_strength,
                                            kRbits[j]);
                    in.is_door_target = (l == 0); /* half is_door, half not */
                    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
                    hash1 = fnv1a_u8(hash1, ok ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.chain_alignment_ok ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.action_ordinal_match ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.closed_door_bash_disabled_ticks_aligned ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.magic_attack_disabled_ticks_diverge ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.destruction_delay_aligned_on_destroy ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_strength_arg_in_cap ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.melee_capped_flag_known_divergence ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_stamina_fired_when_closed_bash ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_feedback_outcome_for_melee_closed ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_feedback_outcome_for_magic_closed ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_feedback_outcome_for_not_door ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_feedback_outcome_for_not_closed ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, out.bash_stamina_skips_for_non_bash ? 1u : 0u);
                    hash1 = fnv1a_u8(hash1, (uint8_t)out.outcome);
                    hash1 = fnv1a_u8(hash1, out.feedback.disabled_ticks);
                    hash1 = fnv1a_u8(hash1, out.feedback.destruction_delay_ticks);
                    hash1 = fnv1a_u8(hash1, out.stamina.action_disabled_ticks);
                    hash1 = fnv1a_u8(hash1, out.stamina.destruction_delay_ticks);
                    hash1 = fnv1a_i16(hash1, out.stamina.current_stamina_after);
                    hash1 = fnv1a_i16(hash1, out.stamina.bash_strength_arg_to_f0232);
                }
            }
        }
    }
    /* Recompute to verify stability. */
    for (i = 0; i < 6; ++i) {
        for (j = 0; j < 2; ++j) {
            for (k = 0; k < 4; ++k) {
                for (l = 0; l < 2; ++l) {
                    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
                    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
                    bool ok;
                    build_closed_door_input(&in,
                                            kActions[i],
                                            kDoors[k].door_type,
                                            0,
                                            kDoors[k].door_defense,
                                            50, 100,
                                            kDoors[k].base_strength,
                                            kRbits[j]);
                    in.is_door_target = (l == 0);
                    ok = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, &out);
                    hash2 = fnv1a_u8(hash2, ok ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.chain_alignment_ok ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.action_ordinal_match ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.closed_door_bash_disabled_ticks_aligned ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.magic_attack_disabled_ticks_diverge ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.destruction_delay_aligned_on_destroy ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_strength_arg_in_cap ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.melee_capped_flag_known_divergence ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_stamina_fired_when_closed_bash ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_feedback_outcome_for_melee_closed ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_feedback_outcome_for_magic_closed ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_feedback_outcome_for_not_door ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_feedback_outcome_for_not_closed ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, out.bash_stamina_skips_for_non_bash ? 1u : 0u);
                    hash2 = fnv1a_u8(hash2, (uint8_t)out.outcome);
                    hash2 = fnv1a_u8(hash2, out.feedback.disabled_ticks);
                    hash2 = fnv1a_u8(hash2, out.feedback.destruction_delay_ticks);
                    hash2 = fnv1a_u8(hash2, out.stamina.action_disabled_ticks);
                    hash2 = fnv1a_u8(hash2, out.stamina.destruction_delay_ticks);
                    hash2 = fnv1a_i16(hash2, out.stamina.current_stamina_after);
                    hash2 = fnv1a_i16(hash2, out.stamina.bash_strength_arg_to_f0232);
                }
            }
        }
    }
    expect_int("hash.stable", hash1 == hash2 ? 1 : 0, 1,
               "FNV-1a deterministic over chained contract sweep");
    expect_int("hash.nonzero", hash1 != 0 ? 1 : 0, 1,
               "FNV-1a mixes real chained contract values, not all-zero");
    printf("HASH_DOOR_BASH_STAMINA_FEEDBACK_CHAIN 0x%08X\n",
           (unsigned int)hash1);
}

static void test_null_inputs_rejected(void)
{
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 out;
    DM1_V1_DoorBashStaminaFeedbackChainInputPc34 in;
    int rc1 = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(NULL, &out);
    memset(&in, 0, sizeof(in));
    int rc2 = M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(&in, NULL);
    expect_int("null.input_rejected", rc1 == 0 ? 1 : 0, 1,
               "NULL input rejected");
    expect_int("null.output_rejected", rc2 == 0 ? 1 : 0, 1,
               "NULL output rejected");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *evidence =
        M11_GameView_DoorBashStaminaFeedbackChainSourceLockPc34();
    expect_contains("evidence.m1311_1319", evidence, "MENU.C:1311-1319",
                    "MENU.C:1311-1319 closed-door bash anchor");
    expect_contains("evidence.m1618_1633", evidence, "MENU.C:1618-1633",
                    "MENU.C:1618-1633 post-action hook anchor");
    expect_contains("evidence.f0232", evidence, "F0232",
                    "PROJEXPL.C:1554-1600 F0232 anchor");
    expect_contains("evidence.f0306", evidence, "F0306",
                    "CHAMPION.C:1078-1103 F0306 anchor");
    expect_contains("evidence.f0312", evidence, "F0312",
                    "CHAMPION.C:1237-1303 F0312 anchor");
    expect_contains("evidence.f0325", evidence, "F0325",
                    "CHAMPION.C:2025-2049 F0325 anchor");
    expect_contains("evidence.f0330", evidence, "F0330",
                    "MENU.C:1620-1622 F0330 anchor");
    expect_contains("evidence.door_info", evidence, "DOOR_INFO",
                    "DEFS.H:1555-1580 DOOR_INFO anchor");
    expect_contains("evidence.melee_cap", evidence, "melee cap 100",
                    "DUNGEON.C:560-564 / 796-799 melee cap anchor");
    expect_contains("evidence.c04", evidence, "C04",
                    "MENU.C:1311 C04 sound anchor");
    expect_contains("evidence.disjoint_bash_feedback",
                    evidence, "dm1_v1_door_bash_feedback_pc34_compat",
                    "Disjoint with bash feedback contract");
    expect_contains("evidence.disjoint_bash_stamina_feedback",
                    evidence, "dm1_v1_door_bash_stamina_feedback_pc34_compat",
                    "Disjoint with bash stamina feedback contract");
    expect_contains("evidence.disjoint_no_open_gate",
                    evidence, "dm1_v1_door_bash_sound_no_open_gate_pc34_compat",
                    "Disjoint with bash sound no-open gate");
    expect_contains("evidence.contract_only", evidence, "contract-only",
                    "Contract-only honesty line");
    expect_contains("evidence.divergence_marker",
                    evidence, "magic_attack_disabled_ticks_diverge",
                    "Magic-attack divergence marker present");
}

int main(void)
{
    printf("probe=dm1_v1_auto_mechanics_door_bash_stamina_feedback_gate_pc34_compat\n");

    test_closed_door_bash_chain_six_actions();
    test_closed_door_wooden_bounce_chain();
    test_closed_door_iron_reject_chain();
    test_closed_door_portcullis_bounce_chain();
    test_open_door_no_closed_branch_chain();
    test_magic_attack_chain();
    test_non_bash_action_chain();
    test_non_door_target_chain();
    test_f0306_strength_collapse_chain();
    test_stamina_overflow_chain();
    test_random_bit_chain();
    test_null_inputs_rejected();
    test_deterministic_hash_stable();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_auto_mechanics_door_bash_stamina_feedback_gate_pc34_compat"
               " failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_auto_mechanics_door_bash_stamina_feedback_gate_pc34_compat"
           " %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
