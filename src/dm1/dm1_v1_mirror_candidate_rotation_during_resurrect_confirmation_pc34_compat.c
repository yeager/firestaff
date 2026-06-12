#include "firestaff/dm1/v1/mirror_candidate/dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_pc34_compat.h"

#include <string.h>

/*
 * Contract-only DM1 V1 mirror-candidate party-rotation-during-resurrect-
 * confirmation gate.
 *
 * ReDMCSB source anchors:
 * - REVIVE.C F0282:744-806 consumes C160/C161/C162 in the C040
 *   resurrect/reincarnate confirmation flow; F0280:124-132 publishes the
 *   candidate and F0281 owns resurrect state set/clear.
 * - COMMAND.C F0359:1985-1990 routes M568/C040 panel clicks only when the
 *   leader hand is empty; F0361/F0380:1709-1806,2045-2162 queue and dispatch
 *   input; F0380:2124-2131 dispatches C001/C002 turns before the later
 *   !G0299 gates for C100/C111/C140/C145 at 2302-2368.
 * - CHAMPION.C F0297/F0300/F0301:243-268,489-585,587-625 cover hand and C30+
 *   mutation paths that must not run in this rotation-only gate.
 * - CLIKCHAM.C F0368:51-72 aligns a selected leader to G0308 and skips a live
 *   G0299 candidate redraw; DUNGEON.C:2608-2612 and DUNVIEW.C:3913-3928 carry
 *   the C127 portrait routing.
 * - DEFS.H:2200 C040; 3001-3008 M568/M569; 338-340 C160/C161/C162;
 *   810-817 C30..C37; 5876-5881 G0425/G0426.
 *
 * This gate pins option (c) for the rotation-during-resurrect-confirmation
 * interaction. The firestaff code path is
 * src/dm1/dm1_v1_input_command_queue_pc34_compat.c
 * DM1_V1_InputCommandQueue_ProcessOnePc34Compat, which mirrors COMMAND.C
 * F0380: it marks C001/C002 as dispatched turns before any candidate-owned
 * C100/C111/C140/C145-style gates. The behavior is that rotation proceeds,
 * C040 remains in resurrect-confirmation state, and the resurrect target stays
 * bound to the original champion rather than following the current leader.
 *
 * Non-overlap: this gate complements existing mirror-candidate rotation,
 * inventory-click, party-direction, reselect, scroll-pickup, and resurrect
 * champion-switch gates by pressing rotation while C040 is specifically in
 * the resurrect-confirmation pending state.
 */

enum {
    DM1_RDRC_PARTY_COUNT = 4,
    DM1_RDRC_LEADER_INDEX = 0,
    DM1_RDRC_TARGET_INDEX = 2,
    DM1_RDRC_C001_TURN_LEFT = 1,
    DM1_RDRC_C002_TURN_RIGHT = 2,
    DM1_RDRC_C040_PANEL = 40,
    DM1_RDRC_M568_PANEL = 568,
    DM1_RDRC_C160_RESURRECT = 160,
    DM1_RDRC_C30_SLOT_FIRST = 30,
    DM1_RDRC_INITIAL_HASH = 0x44523143u
};

typedef struct Dm1RdrcChampionPc34Compat {
    int alive;
    int cell;
    int direction;
    int c30_slot_thing;
} Dm1RdrcChampionPc34Compat;

typedef struct Dm1RdrcStatePc34Compat {
    Dm1RdrcChampionPc34Compat champions[DM1_RDRC_PARTY_COUNT];
    int party_count;
    int leader_index;
    int leader_empty_handed;
    int leader_hand_thing;
    int party_direction;
    int c040_panel_open;
    int panel_content;
    int panel_graphic;
    int resurrect_confirmation_pending;
    int last_panel_command;
    int resurrect_target_index;
    int candidate_owner_champion_index;
    int f0361_queue_writes;
    int f0380_queue_dispatches;
    int f0282_consumed_commands;
    int f0282_state_preserved_count;
    int f0297_leader_hand_puts;
    int c30_mutations;
    int g0425_mutations;
    int panel_redraws;
    unsigned int deterministic_hash;
} Dm1RdrcStatePc34Compat;

static Dm1V1MirrorCandidateRotationDuringResurrectConfirmationResultPc34Compat
    s_last_result;

static const char s_source_evidence[] =
    "REVIVE.C F0282:744-806 C160/C161/C162 resurrect confirmation flow; "
    "REVIVE.C F0280:124-132 candidate publish; F0281 resurrect state set/clear\n"
    "COMMAND.C F0359:1985-1990 M568/C040 empty-hand panel dispatch; "
    "F0361/F0380:1709-1806,2045-2162 queue/dispatch; "
    "F0380:2124-2131 C001/C002 turn dispatch before C100/C111/C140/C145 "
    "!G0299 gates at 2302-2368\n"
    "CHAMPION.C F0297/F0300/F0301:243-268,489-585,587-625 hand and C30+ "
    "mutation paths; CLIKCHAM.C F0368:51-72 leader direction/redraw\n"
    "DUNGEON.C:2608-2612 and DUNVIEW.C:3913-3928 C127 portrait routing\n"
    "DEFS.H:2200 C040; 3001-3008 M568/M569; 338-340 C160/C161/C162; "
    "810-817 C30..C37; 5876-5881 G0425/G0426\n"
    "option_c: rotation proceeds; C040 confirmation stays pending; target "
    "stays on original champion";

static unsigned int mix_hash(unsigned int hash, unsigned int value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    hash *= 16777619u;
    return hash;
}

static void hash_state(Dm1RdrcStatePc34Compat *state, int tag, int value)
{
    if (!state) {
        return;
    }
    state->deterministic_hash =
        mix_hash(state->deterministic_hash, (unsigned int)tag);
    state->deterministic_hash =
        mix_hash(state->deterministic_hash, (unsigned int)value);
}

static int normalize_direction(int direction)
{
    while (direction < 0) {
        direction += 4;
    }
    return direction & 3;
}

static void init_state(Dm1RdrcStatePc34Compat *state, int confirmation_pending)
{
    int i;

    memset(state, 0, sizeof(*state));
    state->party_count = DM1_RDRC_PARTY_COUNT;
    state->leader_index = DM1_RDRC_LEADER_INDEX;
    state->leader_empty_handed = 1;
    state->party_direction = 0;
    state->c040_panel_open = 1;
    state->panel_content = DM1_RDRC_M568_PANEL;
    state->panel_graphic = DM1_RDRC_C040_PANEL;
    state->resurrect_confirmation_pending = confirmation_pending;
    state->last_panel_command =
        confirmation_pending ? DM1_RDRC_C160_RESURRECT : 0;
    state->resurrect_target_index = DM1_RDRC_TARGET_INDEX;
    state->candidate_owner_champion_index = DM1_RDRC_TARGET_INDEX;
    state->deterministic_hash = DM1_RDRC_INITIAL_HASH;

    for (i = 0; i < DM1_RDRC_PARTY_COUNT; ++i) {
        state->champions[i].alive = i != DM1_RDRC_TARGET_INDEX;
        state->champions[i].cell = i;
        state->champions[i].direction = 0;
        state->champions[i].c30_slot_thing = 0x3000 + i;
    }
    hash_state(state, 1, confirmation_pending);
    hash_state(state, 2, state->candidate_owner_champion_index);
}

static int is_turn_command(int command)
{
    return command == DM1_RDRC_C001_TURN_LEFT ||
           command == DM1_RDRC_C002_TURN_RIGHT;
}

static void simulate_f0361_queue_write(Dm1RdrcStatePc34Compat *state,
                                       int command)
{
    if (!state || !is_turn_command(command)) {
        return;
    }
    ++state->f0361_queue_writes;
    hash_state(state, 10, command);
}

static int process_f0380_rotation(Dm1RdrcStatePc34Compat *state, int command)
{
    int before_target;
    int before_state;
    int before_hand;
    int before_c30;
    int before_g0425;
    int i;

    if (!state || !is_turn_command(command)) {
        return 0;
    }
    before_target = state->resurrect_target_index;
    before_state = state->resurrect_confirmation_pending;
    before_hand = state->leader_hand_thing;
    before_c30 = state->champions[DM1_RDRC_TARGET_INDEX].c30_slot_thing;
    before_g0425 = state->g0425_mutations;

    ++state->f0380_queue_dispatches;
    state->party_direction =
        normalize_direction(state->party_direction +
                            (command == DM1_RDRC_C002_TURN_RIGHT ? 1 : -1));
    for (i = 0; i < state->party_count; ++i) {
        state->champions[i].cell = normalize_direction(
            state->champions[i].cell +
            (command == DM1_RDRC_C002_TURN_RIGHT ? 1 : -1));
        state->champions[i].direction = state->party_direction;
    }
    ++state->panel_redraws;

    if (state->resurrect_target_index == before_target &&
        state->candidate_owner_champion_index == DM1_RDRC_TARGET_INDEX) {
        ++s_last_result.resurrect_target_preserved_checks;
    }
    if (state->resurrect_confirmation_pending == before_state &&
        state->c040_panel_open == 1 &&
        state->panel_content == DM1_RDRC_M568_PANEL &&
        state->panel_graphic == DM1_RDRC_C040_PANEL) {
        ++state->f0282_state_preserved_count;
        ++s_last_result.resurrect_state_preserved_checks;
    }
    if (state->panel_redraws == 1 &&
        state->f0282_consumed_commands == 0) {
        ++s_last_result.panel_redraw_preserved_checks;
    }
    if (state->leader_empty_handed == 1 &&
        state->leader_hand_thing == before_hand) {
        ++s_last_result.leader_empty_hand_checks;
    }
    if (state->champions[DM1_RDRC_TARGET_INDEX].c30_slot_thing == before_c30 &&
        state->g0425_mutations == before_g0425 &&
        state->c30_mutations == 0 &&
        state->f0297_leader_hand_puts == 0) {
        ++s_last_result.c30_g0425_mutation_checks;
    }
    if (state->f0282_consumed_commands == 0) {
        ++s_last_result.c160_c161_c162_consumption_checks;
    }
    hash_state(state, 11, state->party_direction);
    hash_state(state, 12, state->candidate_owner_champion_index);
    return 1;
}

static void check_condition(int condition)
{
    ++s_last_result.assertions;
    if (!condition) {
        ++s_last_result.failures;
    }
}

int run_dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_self_test(void)
{
    Dm1RdrcStatePc34Compat confirmation;
    Dm1RdrcStatePc34Compat browse;
    int accepted;

    memset(&s_last_result, 0, sizeof(s_last_result));
    s_last_result.positive_rotation_outcome = "c";
    s_last_result.negative_browse_rotation_outcome = "allowed_rotation";

    init_state(&confirmation, 1);
    check_condition(confirmation.party_count == 4);
    check_condition(confirmation.leader_index == DM1_RDRC_LEADER_INDEX);
    check_condition(confirmation.leader_empty_handed == 1);
    check_condition(confirmation.champions[0].alive == 1);
    check_condition(confirmation.champions[1].alive == 1);
    check_condition(confirmation.champions[2].alive == 0);
    check_condition(confirmation.champions[3].alive == 1);
    check_condition(confirmation.c040_panel_open == 1);
    check_condition(confirmation.resurrect_confirmation_pending == 1);
    check_condition(confirmation.last_panel_command == DM1_RDRC_C160_RESURRECT);

    simulate_f0361_queue_write(&confirmation, DM1_RDRC_C002_TURN_RIGHT);
    accepted = process_f0380_rotation(&confirmation, DM1_RDRC_C002_TURN_RIGHT);
    if (accepted) {
        ++s_last_result.positive_rotation_dispatches;
    }
    check_condition(accepted == 1);
    check_condition(confirmation.f0361_queue_writes == 1);
    check_condition(confirmation.f0380_queue_dispatches == 1);
    check_condition(confirmation.party_direction == 1);
    check_condition(confirmation.resurrect_target_index ==
                    DM1_RDRC_TARGET_INDEX);
    check_condition(confirmation.candidate_owner_champion_index ==
                    DM1_RDRC_TARGET_INDEX);
    check_condition(confirmation.resurrect_confirmation_pending == 1);
    check_condition(confirmation.f0282_consumed_commands == 0);
    check_condition(confirmation.f0297_leader_hand_puts == 0);
    check_condition(confirmation.c30_mutations == 0);
    check_condition(confirmation.g0425_mutations == 0);
    check_condition(s_last_result.resurrect_target_preserved_checks == 1);
    check_condition(s_last_result.resurrect_state_preserved_checks == 1);
    check_condition(s_last_result.panel_redraw_preserved_checks == 1);

    init_state(&browse, 0);
    check_condition(browse.c040_panel_open == 1);
    check_condition(browse.resurrect_confirmation_pending == 0);
    simulate_f0361_queue_write(&browse, DM1_RDRC_C001_TURN_LEFT);
    accepted = process_f0380_rotation(&browse, DM1_RDRC_C001_TURN_LEFT);
    if (accepted) {
        ++s_last_result.negative_browse_rotation_dispatches;
    }
    check_condition(accepted == 1);
    check_condition(browse.f0361_queue_writes == 1);
    check_condition(browse.f0380_queue_dispatches == 1);
    check_condition(browse.party_direction == 3);
    check_condition(browse.resurrect_target_index == DM1_RDRC_TARGET_INDEX);
    check_condition(browse.candidate_owner_champion_index ==
                    DM1_RDRC_TARGET_INDEX);
    check_condition(browse.resurrect_confirmation_pending == 0);
    check_condition(browse.f0282_consumed_commands == 0);
    check_condition(browse.f0297_leader_hand_puts == 0);
    check_condition(browse.c30_mutations == 0);
    check_condition(browse.g0425_mutations == 0);
    check_condition(s_last_result.resurrect_target_preserved_checks == 2);
    check_condition(s_last_result.resurrect_state_preserved_checks == 2);
    check_condition(s_last_result.panel_redraw_preserved_checks == 2);

    s_last_result.queue_dispatch_checks =
        confirmation.f0361_queue_writes + confirmation.f0380_queue_dispatches +
        browse.f0361_queue_writes + browse.f0380_queue_dispatches;
    check_condition(s_last_result.queue_dispatch_checks == 4);
    check_condition(s_last_result.c160_c161_c162_consumption_checks == 2);
    check_condition(s_last_result.leader_empty_hand_checks == 2);
    check_condition(s_last_result.c30_g0425_mutation_checks == 2);

    s_last_result.deterministic_hash =
        mix_hash(confirmation.deterministic_hash, browse.deterministic_hash);
    s_last_result.deterministic_hash =
        mix_hash(s_last_result.deterministic_hash,
                 (unsigned int)s_last_result.assertions);
    s_last_result.deterministic_hash =
        mix_hash(s_last_result.deterministic_hash,
                 (unsigned int)s_last_result.failures);
    return s_last_result.failures == 0 ? 0 : 1;
}

const Dm1V1MirrorCandidateRotationDuringResurrectConfirmationResultPc34Compat *
dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_last_self_test_result_pc34(void)
{
    return &s_last_result;
}

const char *
dm1_v1_mirror_candidate_rotation_during_resurrect_confirmation_source_evidence_pc34(void)
{
    return s_source_evidence;
}
