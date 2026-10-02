#include "dm2_v1_light_terminal_receipt.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

typedef struct {
    unsigned edges;
    unsigned nodes;
    int unknown_edge;
    int missing_receipt;
} Fixture;

static int blocked_step(void *context, int map, int x, int y, int direction,
                        unsigned score, int *next_map, int *next_x,
                        int *next_y, int *projection_map, int *projection_x,
                        int *projection_y)
{
    Fixture *fixture = (Fixture *)context;
    (void)map; (void)x; (void)y; (void)direction; (void)score;
    (void)next_map; (void)next_x; (void)next_y;
    (void)projection_map; (void)projection_x; (void)projection_y;
    return fixture->unknown_edge ? -1 : 0;
}

static int zero_node(void *context, int map, int x, int y, int direction,
                     int source_facing, unsigned score,
                     unsigned source_flags, uint8_t effective_flags)
{
    (void)context; (void)map; (void)x; (void)y; (void)direction;
    (void)source_facing; (void)score; (void)source_flags;
    (void)effective_flags;
    return 0;
}

static uint32_t edge_receipt(void *context, int map, int x, int y,
                             int direction, unsigned score, int cost,
                             int next_map, int next_x, int next_y,
                             int projection_map, int projection_x,
                             int projection_y)
{
    Fixture *fixture = (Fixture *)context;
    (void)map; (void)x; (void)y; (void)direction; (void)score;
    (void)next_map; (void)next_x; (void)next_y;
    (void)projection_map; (void)projection_x; (void)projection_y;
    assert(cost == 0);
    ++fixture->edges;
    return fixture->missing_receipt ? 0u : 0xabc123u;
}

static uint32_t node_receipt(void *context, int map, int x, int y,
                             int direction, unsigned score,
                             unsigned source_flags, uint8_t effective_flags,
                             int16_t ambient_before, int16_t darkness_before,
                             int16_t ambient_after, int16_t darkness_after)
{
    Fixture *fixture = (Fixture *)context;
    (void)map; (void)x; (void)y; (void)direction; (void)score;
    assert(source_flags == 3u && effective_flags == 0x4fu);
    assert(ambient_before == ambient_after);
    assert(darkness_before == darkness_after);
    ++fixture->nodes;
    return fixture->missing_receipt ? 0u : 0xdef456u;
}

int main(void)
{
    DM2_V1_1c9aLightVisibility state;
    DM2_V1_LightWalkTerminalProof proof;
    DM2_V1_LightWalkSourceReceipts receipts;
    Fixture fixture;
    uint16_t rng = 1u;
    memset(&fixture, 0, sizeof(fixture));
    memset(&receipts, 0, sizeof(receipts));
    receipts.edge = edge_receipt;
    receipts.node = node_receipt;
    receipts.context = &fixture;
    dm2_v1_1c9a_light_visibility_reset(&state, 3, 25, 38, 20);
    assert(dm2_v1_1c9a_light_mode8_frontier_with_proof(
        &state, 3, 13, 9, blocked_step, &fixture,
        &rng, &receipts, &proof));
    assert(proof.valid && proof.terminal_exhausted &&
           proof.callbacks_authenticated && proof.action == 0x1bu &&
           proof.mode == 8u && proof.rng_before == 1u &&
           proof.rng_after == rng && proof.ordered_node_hash &&
           proof.source_edge_hash && proof.result_hash &&
           fixture.edges == 4u && fixture.nodes == 0u);
    assert(!state.mode8_complete && !state.mode7_complete &&
           !state.source_state_hash);

    fixture.edges = 0u;
    assert(dm2_v1_1c9a_light_mode7_frontier_with_proof(
        &state, 3, 13, 9, 0, 5u, blocked_step, zero_node,
        &fixture, &rng, &receipts, &proof));
    assert(proof.valid && proof.terminal_exhausted &&
           proof.action == 0x17u && proof.mode == 7u &&
           proof.result_hash && fixture.edges == 4u && fixture.nodes == 1u);
    assert(!state.mode8_complete && !state.mode7_complete &&
           !state.source_state_hash);

    assert(!dm2_v1_1c9a_light_mode7_zero_radius_proof(
        &state, 3, 13, 9, rng, 0u, &proof));
    assert(!proof.valid);
    assert(dm2_v1_1c9a_light_mode7_zero_radius_proof(
        &state, 3, 13, 9, rng, 0x98765432u, &proof));
    assert(proof.valid && proof.skipped_zero_radius &&
           proof.radius_source_receipt_hash == 0x98765432u &&
           proof.rng_before == rng && proof.rng_after == rng &&
           !proof.ordered_node_hash && !proof.source_edge_hash &&
           !state.mode7_complete && !state.source_state_hash);

    fixture.missing_receipt = 1;
    assert(!dm2_v1_1c9a_light_mode8_frontier_with_proof(
        &state, 3, 13, 9, blocked_step, &fixture,
        &rng, &receipts, &proof));
    assert(!proof.valid && !proof.terminal_exhausted);
    fixture.missing_receipt = 0;
    fixture.unknown_edge = 1;
    assert(!dm2_v1_1c9a_light_mode7_frontier_with_proof(
        &state, 3, 13, 9, 0, 5u, blocked_step, zero_node,
        &fixture, &rng, &receipts, &proof));
    assert(!proof.valid && !proof.terminal_exhausted);
    assert(!state.mode8_complete && !state.mode7_complete &&
           !state.source_state_hash);
    return 0;
}
