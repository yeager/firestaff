#include "dm2_v1_light_terminal_receipt.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void fixture(DM2_V1_1c9aLightVisibility *state,
                    DM2_V1_LightWalkTerminalProof *mode8,
                    DM2_V1_LightWalkTerminalProof *mode7,
                    DM2_V1_LightTerminalSourceInputs *inputs)
{
    memset(state, 0, sizeof(*state));
    memset(mode8, 0, sizeof(*mode8));
    memset(mode7, 0, sizeof(*mode7));
    memset(inputs, 0, sizeof(*inputs));
    state->current_map = 38;
    state->current_width = 20u;
    state->alternate_map = 3;
    state->alternate_width = 25u;
    state->current[6u * 32u + 6u] = 1u;
    state->v1e0974 = 10;
    state->v1e0978 = 2;
    inputs->map = 38;
    inputs->x = 6u;
    inputs->y = 6u;
    inputs->radius = 5u;
    inputs->rng_final = 0x2345u;
    inputs->map_descriptor_hash = 0x1001u;
    inputs->dungeon_hash = 0x1002u;
    inputs->record_pool_hash = 0x1003u;
    inputs->gdat_hash = 0x1004u;
    inputs->party_inventory_hash = 0x1005u;
    inputs->weather_hash = 0x1006u;
    mode8->valid = 1u;
    mode8->action = 0x1bu;
    mode8->mode = 8u;
    mode8->terminal_exhausted = 1u;
    mode8->callbacks_authenticated = 1u;
    mode8->map = 38;
    mode8->x = 6u;
    mode8->y = 6u;
    mode8->radius = 25u;
    mode8->rng_before = 1u;
    mode8->rng_after = 0x1234u;
    mode8->ordered_node_hash = 0x2001u;
    mode8->source_edge_hash = 0x2002u;
    mode8->result_hash = dm2_v1_light_terminal_visibility_hash(state);
    mode7->valid = 1u;
    mode7->action = 0x17u;
    mode7->mode = 7u;
    mode7->terminal_exhausted = 1u;
    mode7->callbacks_authenticated = 1u;
    mode7->map = 38;
    mode7->x = 6u;
    mode7->y = 6u;
    mode7->radius = 5u;
    mode7->rng_before = mode8->rng_after;
    mode7->rng_after = inputs->rng_final;
    mode7->ordered_node_hash = 0x3001u;
    mode7->source_edge_hash = 0x3002u;
    mode7->result_hash = dm2_v1_light_terminal_accumulator_hash(
        state->v1e0974, state->v1e0978);
}

static void assert_rejected(DM2_V1_1c9aLightVisibility *state,
                            DM2_V1_LightWalkTerminalProof *mode8,
                            DM2_V1_LightWalkTerminalProof *mode7,
                            DM2_V1_LightTerminalSourceInputs *inputs)
{
    state->mode8_complete = 1u;
    state->mode7_complete = 1u;
    state->source_state_hash = 0xdeadbeefu;
    assert(!dm2_v1_light_terminal_receipt_commit(
        state, mode8, mode7, inputs));
    assert(!state->mode8_complete && !state->mode7_complete &&
           !state->source_state_hash);
}

int main(void)
{
    DM2_V1_1c9aLightVisibility state;
    DM2_V1_LightWalkTerminalProof mode8, mode7;
    DM2_V1_LightTerminalSourceInputs inputs;
    uint32_t committed_hash;

    fixture(&state, &mode8, &mode7, &inputs);
    assert(dm2_v1_light_terminal_receipt_commit(
        &state, &mode8, &mode7, &inputs));
    committed_hash = state.source_state_hash;
    assert(state.mode8_complete && state.mode7_complete && committed_hash);

    fixture(&state, &mode8, &mode7, &inputs);
    mode8.terminal_exhausted = 0u;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    mode7.unknown_branch_seen = 1u;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    mode7.fallthrough_seen = 1u;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    mode7.callbacks_authenticated = 0u;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    mode7.rng_before++;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    mode7.result_hash++;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    inputs.weather_hash = 0u;
    assert_rejected(&state, &mode8, &mode7, &inputs);
    fixture(&state, &mode8, &mode7, &inputs);
    inputs.weather_hash++;
    assert(dm2_v1_light_terminal_receipt_commit(
        &state, &mode8, &mode7, &inputs));
    assert(state.source_state_hash != committed_hash);

    fixture(&state, &mode8, &mode7, &inputs);
    inputs.radius = 0u;
    state.v1e0974 = 0;
    state.v1e0978 = 0;
    mode7.radius = 0u;
    mode7.skipped_zero_radius = 1u;
    mode7.rng_after = mode7.rng_before;
    inputs.rng_final = mode7.rng_after;
    mode7.ordered_node_hash = 0u;
    mode7.source_edge_hash = 0u;
    mode7.result_hash = dm2_v1_light_terminal_accumulator_hash(0, 0);
    assert(dm2_v1_light_terminal_receipt_commit(
        &state, &mode8, &mode7, &inputs));
    printf("PASS: DM2 light terminal receipt gates provenance\n");
    return 0;
}
