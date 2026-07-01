#ifndef FIRESTAFF_DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_PC34_COMPAT_H

/*
 * DM1 V1 door-bash stamina-feedback chained contract.
 *
 * ReDMCSB source anchors (Atari ST DM1 V1 / I34E PC):
 *   - MENU.C:1311-1319 closed-door bash branch fires both the external
 *     feedback path (M563 + C04 + F0232 dispatch + 2-tick destruction
 *     event) AND the internal stamina path (F0306 strength collapse +
 *     F0325 stamina decrement + F0330 disable-action + F0292 redraw).
 *     Both paths share the same action_ordinal, the same ActionDisabledTicks
 *     (always 6 in the closed-door non-magic branch), and the same
 *     destruction_delay (always 2 ticks when F0232 schedules an event).
 *   - MENU.C:1618-1633 post-action hook: F0330 fires only when
 *     L1249_ui_ActionDisabledTicks != 0; F0325 fires only when
 *     L1253_i_ActionStamina != 0. Both must agree about whether the
 *     action was a bash, and both must produce identical disabled_ticks
 *     and destruction_delay values for the closed-door bash.
 *   - PROJEXPL.C:1580-1582 magic_attack on a closed door causes the
 *     bash feedback contract to short-circuit with MELEE_REJECTED
 *     (disabled_ticks=0). The bash stamina contract does NOT gate on
 *     magic_attack and still fires F0325 with action_disabled_ticks=6.
 *     This is a known cross-contract divergence that the chained
 *     contract surfaces explicitly via magic_attack_disabled_ticks_diverge.
 *   - CHAMPION.C:2025-2049 F0325 + CHAMPION.C:1078-1103 F0306 +
 *     CHAMPION.C:1237-1303 F0312 + CHAMPION.C:2048 LOAD|STATISTICS mask.
 *   - PROJEXPL.C:1554-1600 F0232 + DUNGEON.C:560-564 + DEFS.H:1555-1580.
 *
 * The chain gate pins the cross-contract invariant that the bash feedback
 * and bash stamina feedback contract helpers must produce consistent
 * (and consistent with each other) values for the SAME (action, door,
 * strength) input. The two existing gates
 *   dm1_v1_door_bash_feedback_pc34_compat (sound + F0232 dispatch +
 *   destruction event)
 *   dm1_v1_door_bash_stamina_feedback_pc34_compat (F0306 + F0325 +
 *   G0494 + disabled_ticks + destruction_delay)
 * cover each half independently; this chained contract verifies that
 * the two halves fire together with byte-stable cross-fields. This is
 * the auto-mechanics angle: the chained resolve that the live engine
 * would invoke in one tick when a bash action is performed against a
 * closed door.
 *
 * Known cross-contract divergences that the chain explicitly surfaces
 * (so future refactors that "fix" them can be detected):
 *   1. magic_attack + closed: feedback.disabled_ticks = 0, stamina
 *      .action_disabled_ticks = 6 (the stamina contract does not gate
 *      on magic_attack).
 *   2. open door + bash: feedback.disabled_ticks = 0, stamina
 *      .action_disabled_ticks = 6 (the stamina contract unconditionally
 *      sets action_disabled_ticks = 6 for any bash action, regardless
 *      of door state).
 *   3. bounce (Attack < Defense) + bash + closed + non-magic: feedback
 *      .destruction_delay_ticks = 0, stamina.destruction_delay_ticks
 *      = 2 (the stamina contract unconditionally sets
 *      destruction_delay_ticks = 2 for any bash action; feedback only
 *      schedules it on Attack >= Defense).
 *   4. melee_capped_to_100: feedback.melee_capped_to_100 is always
 *      false because the stamina half feeds the F0026-clipped
 *      bash_strength_arg_to_f0232 (<= 100) into the feedback half;
 *      stamina.bash_strength_was_capped_to_100 may be true when the
 *      pre-clip strength exceeds 100.
 *
 * Non-duplicative with:
 *   - dm1_v1_door_bash_feedback_pc34_compat (90 assertions, broad,
 *     sound + F0232 dispatch + destruction event contract)
 *   - dm1_v1_door_bash_stamina_feedback_pc34_compat (121 assertions,
 *     stamina + F0306 + F0325 + G0494 contract)
 *   - dm1_v1_door_bash_sound_no_open_gate_pc34_compat (185 assertions,
 *     bash-but-no-open invariant, single-contract)
 *   - dm1_v1_door_bash_feedback_source_lock_pc34_compat (test binary
 *     for the broad feedback contract)
 *   - dm1_v1_door_bash_stamina_feedback_source_lock_pc34_compat (test
 *     binary for the broad stamina contract)
 *   - dm1_v1_door_bash_sound_no_open_gate_pc34_compat (test binary
 *     for the no-open invariant)
 *   - dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat
 *     (door front occlusion that calls into the bash feedback chain
 *     for partly-open doors, but only consumes `door_bash_chain_ok`,
 *     not the cross-contract fields)
 *
 * This is a contract-only gate: it does not load GRAPHICS.DAT /
 * DUNGEON.DAT, does not drive the live M11 action input, does not
 * schedule a real F0238/F0064 call, and does not claim original DOS
 * pixel parity.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_door_bash_feedback_pc34_compat.h"
#include "dm1_v1_door_bash_stamina_feedback_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB MENU.C:1317 + MENU.C:1620-1622 closed-door bash disabled ticks. */
#define DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DISABLED_TICKS_PC34 6

/* ReDMCSB MENU.C:1317 + PROJEXPL.C:1554-1600 destruction delay. */
#define DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_DESTRUCTION_DELAY_TICKS_PC34 2

/*
 * ReDMCSB PROJEXPL.C:1554-1600 F0232 returns C0_FALSE for any
 * non-wooden-or-portcullis door melee path; the chained contract pins
 * that the bash strength arg, the bash strength cap, and the bash
 * outcome bucket must agree between the bash feedback and bash
 * stamina feedback contracts for the same input.
 */
typedef enum {
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NO_DOOR_PC34 = 0,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_CLOSED_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_MELEE_REJECTED_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_IRON_REJECT_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_RA_REJECT_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BOUNCE_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_PORT_BREAK_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_WOODEN_BREAK_PC34,
    DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_OUTCOME_NOT_BASH_PC34
} DM1_V1_DoorBashStaminaFeedbackChainOutcomePc34;

typedef struct {
    uint8_t door_type;
    uint8_t door_attributes;
    uint8_t door_defense;
    uint8_t door_state;
    uint8_t target_element;
    uint8_t action_ordinal;
    bool magic_attack;
    bool is_door_target;
    /* F0306 + F0312 + F0325 inputs from the bash stamina contract. */
    int16_t current_stamina;
    int16_t maximum_stamina;
    int16_t base_strength;
    uint8_t random_bit;
} DM1_V1_DoorBashStaminaFeedbackChainInputPc34;

typedef struct {
    DM1_V1_DoorBashResultPc34 feedback;
    DM1_V1_DoorBashStaminaResultPc34 stamina;

    /*
     * Cross-contract invariants: byte-stable when the bash action
     * fires against a closed door (or, for some fields, when the
     * bash stamina contract fires in the closed-door branch
     * regardless of bash action type).
     *
     * The chained contract pins the cross-contract invariants and
     * surfaces the known divergences explicitly so future refactors
     * that "fix" a divergence can be detected.
     *
     *   - resolved: both contracts returned true.
     *
     *   - closed_door_bash_disabled_ticks_aligned:
     *       bash + closed + non-magic: feedback.disabled_ticks = 6
     *       AND stamina.action_disabled_ticks = 6.
     *       For other branches (open, magic, non-bash) this field is
     *       true trivially and the divergence flags below describe why.
     *
     *   - magic_attack_disabled_ticks_diverge:
     *       bash + closed + magic: feedback.disabled_ticks = 0 AND
     *       stamina.action_disabled_ticks = 6 (known cross-contract
     *       divergence: feedback gates on magic_attack, stamina does
     *       not).
     *       For non-magic branches this field is true trivially (the
     *       divergence does not apply).
     *
     *   - open_door_disabled_ticks_diverge:
     *       bash + open (not closed): feedback.disabled_ticks = 0
     *       AND stamina.action_disabled_ticks = 6 (known divergence:
     *       feedback requires closed precondition, stamina does not).
     *
     *   - destruction_delay_aligned_on_destroy:
     *       bash + closed + non-magic + Attack >= Defense: feedback
     *       .destruction_delay_ticks = 2 AND stamina
     *       .destruction_delay_ticks = 2.
     *       For bounce (Attack < Defense) the divergence is surfaced
     *       via destruction_delay_bounce_divergence.
     *
     *   - destruction_delay_bounce_divergence:
     *       bash + closed + non-magic + Attack < Defense: feedback
     *       .destruction_delay_ticks = 0 AND stamina
     *       .destruction_delay_ticks = 2 (known divergence: feedback
     *       only schedules on Attack >= Defense, stamina always
     *       reports 2 for bash actions).
     *
     *   - bash_strength_arg_in_cap:
     *       stamina.bash_strength_arg_to_f0232 <= 100 (DUNGEON.C:561
     *       melee cap). This is always true after the F0026 clip on
     *       the stamina half.
     *
     *   - bash_strength_arg_passed_to_feedback:
     *       feedback_in.action_strength ==
     *       stamina.bash_strength_arg_to_f0232 (the chained contract
     *       passes the F0026-clipped stamina value into the feedback
     *       half as the bash strength argument).
     *
     *   - melee_capped_flag_known_divergence:
     *       stamina.bash_strength_was_capped_to_100 may be true (when
     *       the pre-clip strength exceeded 100) while
     *       feedback.melee_capped_to_100 is always false (because
     *       feedback receives the already-capped value). This is a
     *       known divergence that the chain surfaces.
     *
     *   - bash_stamina_fired_when_closed_bash:
     *       bash + closed implies stamina.outcome != NOT_BASH
     *       (F0325 fires regardless of door type when the action is
     *       a bash).
     *
     *   - bash_feedback_outcome_for_melee_closed:
     *       bash + closed + non-magic + door target: feedback.outcome
     *       in {WOODEN_BREAK, WOODEN_BOUNCE, PORT_BREAK, IRON_REJECT,
     *       RA_REJECT, MELEE_REJECTED}. The chain explicitly includes
     *       MELEE_REJECTED here because the bash feedback contract
     *       uses that bucket only when magic_attack is true, which
     *       is the non-magic case, so MELEE_REJECTED is impossible
     *       here; we still include it as a sentinel for the
     *       explicit non-magic outcome bucket.
     *
     *   - bash_feedback_outcome_for_magic_closed:
     *       bash + closed + magic: feedback.outcome ==
     *       MELEE_REJECTED (PROJEXPL.C:1580).
     *
     *   - bash_feedback_outcome_for_not_door:
     *       non-door target: feedback.outcome == NO_DOOR.
     *
     *   - bash_feedback_outcome_for_not_closed:
     *       door but not closed: feedback.outcome == NOT_CLOSED.
     *
     *   - bash_stamina_always_for_bash:
     *       bash action implies stamina.outcome != NOT_BASH (the
     *       stamina contract fires F0325 for any bash action,
     *       including not-closed and non-door targets).
     *
     *   - bash_stamina_skips_for_non_bash:
     *       non-bash action implies stamina.outcome == NOT_BASH.
     *
     *   - action_ordinal_match:
     *       The action ordinal was the same for both halves (by
     *       construction; both halves received the same input).
     */
    bool resolved;
    bool action_ordinal_match;
    bool closed_door_bash_disabled_ticks_aligned;
    bool magic_attack_disabled_ticks_diverge;
    bool open_door_disabled_ticks_diverge;
    bool destruction_delay_aligned_on_destroy;
    bool destruction_delay_bounce_divergence;
    bool bash_strength_arg_in_cap;
    bool bash_strength_arg_passed_to_feedback;
    bool melee_capped_flag_known_divergence;
    bool bash_stamina_fired_when_closed_bash;
    bool bash_feedback_outcome_for_melee_closed;
    bool bash_feedback_outcome_for_magic_closed;
    bool bash_feedback_outcome_for_not_door;
    bool bash_feedback_outcome_for_not_closed;
    bool bash_stamina_always_for_bash;
    bool bash_stamina_skips_for_non_bash;

    bool chain_alignment_ok; /* every alignment field above is true */
    DM1_V1_DoorBashStaminaFeedbackChainOutcomePc34 outcome;
} DM1_V1_DoorBashStaminaFeedbackChainResultPc34;

bool M11_GameView_DoorBashStaminaFeedbackChainResolvePc34(
    const DM1_V1_DoorBashStaminaFeedbackChainInputPc34 *input,
    DM1_V1_DoorBashStaminaFeedbackChainResultPc34 *out);

const char *M11_GameView_DoorBashStaminaFeedbackChainSourceLockPc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DOOR_BASH_STAMINA_FEEDBACK_CHAIN_PC34_COMPAT_H */
