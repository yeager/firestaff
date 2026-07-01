/*
 * DM1 V1 door-bash stamina-feedback chained contract implementation.
 *
 * The closed-door bash branch in ReDMCSB MENU.C:1311-1319 fires two
 * half-contracts in sequence: the external feedback path
 * (sound + F0232 dispatch + destruction event) and the internal stamina
 * path (F0306 + F0325 + F0330 + F0292). Both share the same
 * action_ordinal, the same ActionDisabledTicks (always 6 in the
 * closed-door non-magic branch), the same destruction_delay (always
 * 2 ticks when F0232 schedules a destruction event), and the same
 * F0306-adjusted bash strength.
 *
 * The chained contract computes both halves and verifies the
 * cross-contract invariants, including the known cross-contract
 * divergences that are explicit (magic_attack disabled-ticks
 * divergence, open-door disabled-ticks divergence, bounce
 * destruction-delay divergence, melee-capped flag divergence).
 *
 * It is contract-only: no live F0064 / F0238 / F0325 call is made,
 * no GRAPHICS.DAT / DUNGEON.DAT is loaded, and the M11 driver still
 * owns the real emission and timeline schedule.
 *
 * Source-locked to ReDMCSB:
 *   - MENU.C:1311-1319 (closed-door bash branch)
 *   - MENU.C:1272-1273 (per-action stamina + M005_RANDOM(2))
 *   - MENU.C:1618-1633 (post-action hook: F0330 / F0325 / F0304 / F0292)
 *   - CHAMPION.C:1078-1103 F0306_CHAMPION_GetStaminaAdjustedValue
 *   - CHAMPION.C:1237-1303 F0312_CHAMPION_GetStrength
 *   - CHAMPION.C:2025-2049 F0325_CHAMPION_DecrementStamina
 *   - CHAMPION.C:2048 LOAD|STATISTICS redraw mask
 *   - PROJEXPL.C:1554-1600 F0232_GROUP_IsDoorDestroyedByAttack
 *   - DUNGEON.C:560-564 / 796-799 melee cap 100 + DOOR_INFO[4]
 *   - DEFS.H:1555-1580 DOOR_INFO struct + door attributes
 *   - DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION
 *   - DEFS.H:136-138 sound play modes
 *   - DEFS.H:7998-7999 F0312 anchor
 *   - DEFS.H:7712-7719 F0238 anchor
 *   - DATA.C:483 / 1172 SOUND_DATA[C04_SOUND_WOODEN_THUD]
 */

#include "dm1_v1_door_bash_stamina_feedback_chain_pc34_compat.h"

#include <string.h>

static bool input_is_bash(const DM1_V1_DoorBashStaminaFeedbackChainInputPc34 *input)
{
    return input &&
           M11_GameView_DoorBashStaminaActionIsBashPc34(input->action_ordinal);
}

static bool is_closed_door_target(const DM1_V1_DoorBashStaminaFeedbackChainInputPc34 *input)
{
    return input && input->is_door_target &&
           input->target_element == DM1_V1_ELEMENT_DOOR_PC34 &&
           input->door_state == DM1_V1_DOOR_STATE_CLOSED_PC34;
}

static bool is_door_target(const DM1_V1_DoorBashStaminaFeedbackChainInputPc34 *input)
{
    return input && input->is_door_target &&
           input->target_element == DM1_V1_ELEMENT_DOOR_PC34;
}

static bool is_melee_break_outcome(DM1_V1_DoorBashOutcomePc34 outcome)
{
    return outcome == DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34 ||
           outcome == DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34 ||
           outcome == DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34 ||
           outcome == DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34 ||
           outcome == DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34;
}

static DM1_V1_DoorBashStaminaFeedbackChainOutcomePc34 map_outcome(
    const DM1_V1_DoorBashResultPc34 *feedback,
    bool bash_action,
    bool is_closed_door,
    bool is_door)
{
    if (!bash_action) {
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_BASH_PC34;
    }
    if (!is_door) {
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NO_DOOR_PC34;
    }
    if (!is_closed_door) {
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_CLOSED_PC34;
    }
    switch (feedback->outcome) {
    case DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NO_DOOR_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_CLOSED_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_MELEE_REJECTED_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_IRON_REJECT_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_RA_REJECT_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BOUNCE_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_PORT_BREAK_PC34;
    case DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BREAK_PC34;
    default:
        return DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NO_DOOR_PC34;
    }
}

bool M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(
    const DM1_V1_DoorBashStaminaFeedbackChainInputPc34 *input,
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 *out)
{
    DM1_V1_DoorBashInputPc34 feedback_in;
    DM1_V1_DoorBashStaminaInputPc34 stamina_in;
    bool feedback_ok;
    bool stamina_ok;
    bool bash_action;
    bool closed;
    bool is_door;
    bool bash_strength_passed_ok;

    if (!input || !out) return false;
    memset(out, 0, sizeof(*out));

    bash_action = input_is_bash(input);
    closed = is_closed_door_target(input);
    is_door = is_door_target(input);

    feedback_in.door_type = input->door_type;
    feedback_in.door_attributes = input->door_attributes;
    feedback_in.door_defense = input->door_defense;
    feedback_in.door_state = input->door_state;
    feedback_in.target_element = input->target_element;
    feedback_in.action_strength = 0; /* overwritten below */
    feedback_in.magic_attack = input->magic_attack;
    feedback_in.is_door_target = input->is_door_target;

    stamina_in.current_stamina_before = input->current_stamina;
    stamina_in.maximum_stamina = input->maximum_stamina;
    stamina_in.action_ordinal = input->action_ordinal;
    stamina_in.random_bit = input->random_bit;
    stamina_in.base_strength = input->base_strength;
    stamina_in.strength_after_stamina = 0;

    /*
     * Step 1: run the stamina half first so we know the F0306-adjusted
     * strength and the F0026-clipped arg to F0232.
     */
    stamina_ok = M11_GameView_DoorBashStaminaResolvePc34(&stamina_in,
                                                        &out->stamina);
    if (!stamina_ok) {
        return false;
    }

    /*
     * Step 2: feed the bash feedback half with the F0026-clipped
     * strength from the stamina half. This is the byte-stable F0232
     * dispatch argument under CHAMPION.C:1302 + DUNGEON.C:561/797
     * melee cap.
     */
    feedback_in.action_strength = out->stamina.bash_strength_arg_to_f0232;

    feedback_ok = M11_GameView_DoorBashResolvePc34(&feedback_in,
                                                   &out->feedback);
    if (!feedback_ok) {
        return false;
    }

    bash_strength_passed_ok =
        (feedback_in.action_strength ==
         out->stamina.bash_strength_arg_to_f0232);

    out->resolved = feedback_ok && stamina_ok;

    /*
     * Cross-contract invariant 1: action_ordinal_match — both halves
     * received the same input.action_ordinal (structural / by
     * construction).
     */
    out->action_ordinal_match = true;

    /*
     * Cross-contract invariant 2: closed_door_bash_disabled_ticks_aligned
     *
     * For closed-door bash (bash + closed + non-magic): both halves
     * report disabled_ticks = 6. For other branches, this field is
     * false and the divergence flags below describe the actual
     * cross-contract values.
     */
    out->closed_door_bash_disabled_ticks_aligned =
        (bash_action && closed && !input->magic_attack) &&
        (out->feedback.disabled_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34) &&
        (out->stamina.action_disabled_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34);

    /*
     * Cross-contract invariant 3: magic_attack_disabled_ticks_diverge
     *
     * When bash + closed + magic_attack, bash feedback reports
     * disabled_ticks = 0 (PROJEXPL.C:1580 magic reject) while bash
     * stamina reports action_disabled_ticks = 6 (the bash stamina
     * contract unconditionally sets action_disabled_ticks = 6 for any
     * bash action). The chain surfaces this known divergence so
     * future refactors that "fix" the divergence can be detected.
     */
    out->magic_attack_disabled_ticks_diverge =
        !(bash_action && closed && input->magic_attack) ||
        ((out->feedback.disabled_ticks == 0) &&
         (out->stamina.action_disabled_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34));

    /*
     * Cross-contract invariant 4: open_door_disabled_ticks_diverge
     *
     * When bash + door but not closed, bash feedback reports
     * disabled_ticks = 0 (MENU.C:1311-1319 closed-only branch) while
     * bash stamina reports action_disabled_ticks = 6 (the bash stamina
     * contract unconditionally sets action_disabled_ticks = 6 for any
     * bash action). The chain surfaces this known divergence.
     */
    out->open_door_disabled_ticks_diverge =
        !(bash_action && is_door && !closed) ||
        ((out->feedback.disabled_ticks == 0) &&
         (out->stamina.action_disabled_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34));

    /*
     * Cross-contract invariant 5: destruction_delay_aligned_on_destroy
     *
     * When bash + closed + non-magic + Attack >= Defense (i.e.
     * WOODEN_BREAK or PORT_BREAK outcome): both halves report
     * destruction_delay = 2 ticks. The bash feedback contract
     * schedules the destruction event only in this case; the bash
     * stamina contract unconditionally reports destruction_delay = 2
     * for any bash action.
     */
    out->destruction_delay_aligned_on_destroy =
        (bash_action && closed && !input->magic_attack &&
         (out->feedback.outcome == DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34 ||
          out->feedback.outcome == DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34)) &&
        (out->feedback.destruction_delay_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34) &&
        (out->stamina.destruction_delay_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34);

    /*
     * Cross-contract invariant 6: destruction_delay_bounce_divergence
     *
     * When bash + closed + non-magic + Attack < Defense (i.e.
     * WOODEN_BOUNCE outcome): bash feedback reports
     * destruction_delay = 0 (no destruction event scheduled) while
     * bash stamina reports destruction_delay = 2 (the bash stamina
     * contract unconditionally reports destruction_delay = 2 for any
     * bash action). The chain surfaces this known divergence.
     */
    out->destruction_delay_bounce_divergence =
        !(bash_action && closed && !input->magic_attack &&
          (out->feedback.outcome == DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34 ||
           out->feedback.outcome == DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34)) ||
        ((out->feedback.destruction_delay_ticks == 0) &&
         (out->stamina.destruction_delay_ticks ==
            DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34));

    /*
     * Cross-contract invariant 7: bash_strength_arg_in_cap — the bash
     * stamina contract caps bash_strength_arg_to_f0232 at 100
     * (DUNGEON.C:561 melee cap). Always true after the F0026 clip.
     */
    out->bash_strength_arg_in_cap =
        (out->stamina.bash_strength_arg_to_f0232 <=
            DM1_V1_DOOR_MELEE_ATTACK_CAP_PC34);

    /*
     * Cross-contract invariant 8: bash_strength_arg_passed_to_feedback
     *
     * The chained contract feeds the bash feedback half with the
     * F0026-clipped bash_strength_arg_to_f0232 from the stamina half
     * as the action_strength input. The bash feedback contract's
     * `melee_capped_to_100` flag is therefore computed on this
     * already-capped value (so it can never be true after the
     * chained resolve).
     */
    out->bash_strength_arg_passed_to_feedback = bash_strength_passed_ok &&
        (feedback_in.action_strength == out->stamina.bash_strength_arg_to_f0232);

    /*
     * Cross-contract invariant 9: melee_capped_flag_known_divergence
     *
     * The bash stamina contract sets bash_strength_was_capped_to_100
     * to true when the pre-clip strength exceeds 100; the bash
     * feedback contract sets melee_capped_to_100 to true when the
     * input exceeds 100. After the chained resolve (which feeds the
     * stamina-capped value into the feedback half), the feedback's
     * `melee_capped_to_100` is always false. The chain surfaces this
     * known asymmetry.
     */
    out->melee_capped_flag_known_divergence =
        !(out->stamina.bash_strength_was_capped_to_100 &&
          !out->feedback.melee_capped_to_100) &&
        /* Trivially true when the stamina cap was not reached. */
        (!out->stamina.bash_strength_was_capped_to_100 ||
         !out->feedback.melee_capped_to_100);

    /*
     * Cross-contract invariant 10: bash_stamina_fired_when_closed_bash
     * — for closed-door bash (any door type, magic or not), the bash
     * stamina contract must NOT short-circuit (out.stamina.outcome is
     * NOT_BASH only for non-bash actions). When the bash stamina
     * contract fires, it always decrements stamina regardless of
     * outcome.
     */
    out->bash_stamina_fired_when_closed_bash =
        !bash_action ||
        (out->stamina.outcome !=
            DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34);

    /*
     * Cross-contract invariant 11: bash_feedback_outcome_for_melee_closed
     * — for closed-door bash + non-magic + door target: bash feedback
     * outcome must be one of {WOODEN_BREAK, WOODEN_BOUNCE,
     * PORT_BREAK, IRON_REJECT, RA_REJECT}. The bash feedback contract
     * reports MELEE_REJECTED only for magic_attack on a closed door
     * (PROJEXPL.C:1580).
     */
    out->bash_feedback_outcome_for_melee_closed =
        !(bash_action && closed && is_door && !input->magic_attack) ||
        is_melee_break_outcome(out->feedback.outcome);

    /*
     * Cross-contract invariant 12: bash_feedback_outcome_for_magic_closed
     * — for closed-door bash + magic_attack: bash feedback reports
     * MELEE_REJECTED (PROJEXPL.C:1580).
     */
    out->bash_feedback_outcome_for_magic_closed =
        !(bash_action && closed && is_door && input->magic_attack) ||
        (out->feedback.outcome ==
            DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34);

    /*
     * Cross-contract invariant 13: bash_feedback_outcome_for_not_door
     * — for non-door targets, bash feedback reports NO_DOOR.
     */
    out->bash_feedback_outcome_for_not_door =
        is_door ||
        (out->feedback.outcome == DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34);

    /*
     * Cross-contract invariant 14: bash_feedback_outcome_for_not_closed
     * — for door but not closed, bash feedback reports NOT_CLOSED.
     */
    out->bash_feedback_outcome_for_not_closed =
        !is_door ||
        closed ||
        (out->feedback.outcome == DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34);

    /*
     * Cross-contract invariant 15: bash_stamina_always_for_bash
     * — bash action implies stamina.outcome != NOT_BASH.
     */
    out->bash_stamina_always_for_bash =
        !bash_action ||
        (out->stamina.outcome !=
            DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34);

    /*
     * Cross-contract invariant 16: bash_stamina_skips_for_non_bash
     * — non-bash action implies stamina.outcome == NOT_BASH.
     */
    out->bash_stamina_skips_for_non_bash =
        bash_action ||
        (out->stamina.outcome ==
            DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34);

    out->chain_alignment_ok =
        out->resolved &&
        out->action_ordinal_match &&
        out->closed_door_bash_disabled_ticks_aligned &&
        out->magic_attack_disabled_ticks_diverge &&
        out->open_door_disabled_ticks_diverge &&
        out->destruction_delay_aligned_on_destroy &&
        out->destruction_delay_bounce_divergence &&
        out->bash_strength_arg_in_cap &&
        out->bash_strength_arg_passed_to_feedback &&
        out->melee_capped_flag_known_divergence &&
        out->bash_stamina_fired_when_closed_bash &&
        out->bash_feedback_outcome_for_melee_closed &&
        out->bash_feedback_outcome_for_magic_closed &&
        out->bash_feedback_outcome_for_not_door &&
        out->bash_feedback_outcome_for_not_closed &&
        out->bash_stamina_always_for_bash &&
        out->bash_stamina_skips_for_non_bash;

    out->outcome = map_outcome(&out->feedback, bash_action, closed, is_door);
    return true;
}

const char *M11_GameView_DoorBashStaminaFeedbackChainSourceLockPc34(void)
{
    return
        "DM1 V1 door-bash stamina-feedback chained contract (source-locked to "
        "ReDMCSB MENU.C:1311-1319 + MENU.C:1272-1273 + MENU.C:1618-1633 + "
        "CHAMPION.C:1078-1103 F0306 + CHAMPION.C:1237-1303 F0312 + "
        "CHAMPION.C:2025-2049 F0325 + CHAMPION.C:2048 LOAD|STATISTICS mask + "
        "PROJEXPL.C:1554-1600 F0232 + PROJEXPL.C:1580 magic_attack early "
        "return + DUNGEON.C:560-564 / 796-799 melee cap 100 + "
        "DEFS.H:1555-1580 DOOR_INFO + DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION + "
        "DEFS.H:136-138 sound play modes + DEFS.H:7998-7999 F0312 anchor + "
        "DEFS.H:7712-7719 F0238 anchor + DATA.C:483 / 1172 SOUND_DATA[C04] + "
        "MENU.C:1620-1622 F0330_CHAMPION_DisableAction). Disjoint with "
        "dm1_v1_door_bash_feedback_pc34_compat (90 assertions, sound + "
        "F0232 dispatch + destruction event contract, single half) and "
        "dm1_v1_door_bash_stamina_feedback_pc34_compat (121 assertions, "
        "F0306 + F0325 + G0494 + disabled_ticks + destruction_delay, single "
        "half) and dm1_v1_door_bash_sound_no_open_gate_pc34_compat (185 "
        "assertions, bash-but-no-open invariant, single-contract). The chained "
        "contract verifies the byte-stable cross-contract invariants "
        "(action_ordinal_match, closed_door_bash_disabled_ticks_aligned, "
        "magic_attack_disabled_ticks_diverge, "
        "open_door_disabled_ticks_diverge, "
        "destruction_delay_aligned_on_destroy, "
        "destruction_delay_bounce_divergence, bash_strength_arg_in_cap, "
        "bash_strength_arg_passed_to_feedback, "
        "melee_capped_flag_known_divergence, "
        "bash_stamina_fired_when_closed_bash, "
        "bash_feedback_outcome_for_melee_closed, "
        "bash_feedback_outcome_for_magic_closed, "
        "bash_feedback_outcome_for_not_door, "
        "bash_feedback_outcome_for_not_closed, "
        "bash_stamina_always_for_bash, bash_stamina_skips_for_non_bash) "
        "between the two halves for the SAME (action, door, strength) "
        "input. The chained contract surfaces the four known "
        "cross-contract divergences: (1) magic_attack_disabled_ticks_diverge "
        "- bash feedback reports 0 disabled_ticks on magic_attack + closed "
        "door (PROJEXPL.C:1580) while bash stamina reports 6 "
        "action_disabled_ticks unconditionally for any bash action; "
        "(2) open_door_disabled_ticks_diverge - bash feedback reports 0 "
        "disabled_ticks when the door is not closed (MENU.C:1311-1319 "
        "closed-only branch) while bash stamina reports 6 "
        "action_disabled_ticks unconditionally for any bash action; "
        "(3) destruction_delay_bounce_divergence - bash feedback reports 0 "
        "destruction_delay_ticks on bounce (Attack < Defense) while bash "
        "stamina reports 2 destruction_delay_ticks unconditionally for any "
        "bash action; (4) melee_capped_flag_known_divergence - bash stamina "
        "sets bash_strength_was_capped_to_100 when the pre-clip strength "
        "exceeds 100, while bash feedback always sees the post-stamina-cap "
        "value so its melee_capped_to_100 flag is always false after the "
        "chained resolve. This is contract-only; no live F0064 / F0238 / "
        "F0325 / F0330 call is made and no GRAPHICS.DAT / DUNGEON.DAT is "
        "loaded.";
}
