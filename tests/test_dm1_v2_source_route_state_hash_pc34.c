#include "dm1_v2_movement_command_adapter_pc34.h"
#include "dm1_v2_phase_gate_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

typedef struct {
    int32_t x;
    int32_t y;
    int32_t dir;
    int32_t accepted;
} SourceRouteState;

typedef struct {
    SourceRouteState state;
    uint64_t stateHash;
    uint64_t pixelHash;
} SideBySideLane;

#define SEED_FRAME_W 16
#define SEED_FRAME_H 12

static uint64_t fnv1a_u32(uint64_t hash, uint32_t value) {
    int byteIndex;
    for (byteIndex = 0; byteIndex < 4; byteIndex++) {
        hash ^= (uint64_t)((value >> (byteIndex * 8)) & 0xFFU);
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t fnv1a_byte(uint64_t hash, uint8_t value) {
    hash ^= (uint64_t)value;
    hash *= 1099511628211ULL;
    return hash;
}

static uint64_t hash_state(uint64_t hash, const SourceRouteState* state) {
    hash = fnv1a_u32(hash, (uint32_t)state->x);
    hash = fnv1a_u32(hash, (uint32_t)state->y);
    hash = fnv1a_u32(hash, (uint32_t)state->dir);
    hash = fnv1a_u32(hash, (uint32_t)state->accepted);
    return hash;
}

static int apply_source_command(SourceRouteState* state, int command) {
    static const int directionToStepEast[4] = {0, 1, 0, -1};
    static const int directionToStepNorth[4] = {-1, 0, 1, 0};
    static const int movementForward[4] = {1, 0, -1, 0};
    static const int movementRight[4] = {0, 1, 0, -1};
    int movementIndex;
    int rightDirection;

    if (!state) return 0;
    state->accepted = 0;

    switch (command) {
        case 1:
            state->dir = (state->dir + 3) & 3;
            state->accepted = 1;
            return 1;
        case 2:
            state->dir = (state->dir + 1) & 3;
            state->accepted = 1;
            return 1;
        case 3:
        case 4:
        case 5:
        case 6:
            movementIndex = command - 3;
            rightDirection = (state->dir + 1) & 3;
            state->x += directionToStepEast[state->dir] * movementForward[movementIndex];
            state->y += directionToStepNorth[state->dir] * movementForward[movementIndex];
            state->x += directionToStepEast[rightDirection] * movementRight[movementIndex];
            state->y += directionToStepNorth[rightDirection] * movementRight[movementIndex];
            state->accepted = 1;
            return 1;
        default:
            return 0;
    }
}

static void draw_seed_frame(const SourceRouteState* state, uint8_t pixels[SEED_FRAME_H][SEED_FRAME_W]) {
    static const int dirMarkerX[4] = {8, 9, 8, 7};
    static const int dirMarkerY[4] = {5, 6, 7, 6};
    int x;
    int y;
    int px;
    int py;
    int dir;

    memset(pixels, 0, SEED_FRAME_W * SEED_FRAME_H);
    for (x = 0; x < SEED_FRAME_W; x++) {
        pixels[0][x] = 0x11U;
        pixels[SEED_FRAME_H - 1][x] = 0x11U;
    }
    for (y = 0; y < SEED_FRAME_H; y++) {
        pixels[y][0] = 0x11U;
        pixels[y][SEED_FRAME_W - 1] = 0x11U;
    }

    if (!state) return;
    px = (state->x % (SEED_FRAME_W - 2)) + 1;
    py = (state->y % (SEED_FRAME_H - 2)) + 1;
    if (px < 1) px += SEED_FRAME_W - 2;
    if (py < 1) py += SEED_FRAME_H - 2;
    dir = state->dir & 3;

    pixels[py][px] = 0xE0U;
    pixels[dirMarkerY[dir]][dirMarkerX[dir]] = 0xA0U;
    if (state->accepted) {
        pixels[1][1] = 0x77U;
    }
}

static uint64_t hash_frame(const uint8_t pixels[SEED_FRAME_H][SEED_FRAME_W]) {
    uint64_t hash = 14695981039346656037ULL;
    int x;
    int y;

    for (y = 0; y < SEED_FRAME_H; y++) {
        for (x = 0; x < SEED_FRAME_W; x++) {
            hash = fnv1a_byte(hash, pixels[y][x]);
        }
    }
    return hash;
}

static uint64_t hash_side_by_side_frame(const uint8_t left[SEED_FRAME_H][SEED_FRAME_W],
                                        const uint8_t right[SEED_FRAME_H][SEED_FRAME_W]) {
    uint64_t hash = 14695981039346656037ULL;
    int x;
    int y;

    for (y = 0; y < SEED_FRAME_H; y++) {
        for (x = 0; x < SEED_FRAME_W; x++) {
            hash = fnv1a_byte(hash, left[y][x]);
        }
        hash = fnv1a_byte(hash, 0x44U);
        for (x = 0; x < SEED_FRAME_W; x++) {
            hash = fnv1a_byte(hash, right[y][x]);
        }
    }
    return hash;
}

static int command_to_v2_movement(int command, DM1_V2_MovementCommand* out) {
    if (!out) return 0;
    switch (command) {
        case 1: *out = DM1_V2_MOVEMENT_COMMAND_TURN_LEFT; return 1;
        case 2: *out = DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT; return 1;
        case 3: *out = DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD; return 1;
        case 4: *out = DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT; return 1;
        case 5: *out = DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD; return 1;
        case 6: *out = DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT; return 1;
        default: return 0;
    }
}

static int test_v1_v2_source_route_state_hashes_match(void) {
    static const int script[] = {3, 3, 2, 4, 1, 6, 5, 2, 3, 4, 4, 1, 3};
    SideBySideLane v1 = {{4, 4, 0, 0}, 14695981039346656037ULL, 0};
    SideBySideLane v2 = {{4, 4, 0, 0}, 14695981039346656037ULL, 0};
    DM1_V2_PhaseGateConfig config;
    uint64_t sideBySideHash = 14695981039346656037ULL;
    size_t i;

    dm1_v2_phase_gate_defaults(&config);
    CHECK(config.v2PresentationEnabled == 0);

    for (i = 0; i < sizeof(script) / sizeof(script[0]); i++) {
        DM1_V2_MovementCommand command;
        DM1_V2_MovementCommandRoute route;
        DM1_V2_PhaseGateDecision renderGate;
        uint8_t v1Pixels[SEED_FRAME_H][SEED_FRAME_W];
        uint8_t v2Pixels[SEED_FRAME_H][SEED_FRAME_W];
        uint64_t compositeHash;

        CHECK(command_to_v2_movement(script[i], &command));
        renderGate = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
        CHECK(renderGate.v1SourceLocked == 0);
        CHECK(renderGate.v2PresentationAllowed == 0);

        route = dm1_v2_movement_command_route_for_presentation(
            renderGate.v2PresentationAllowed,
            command);
        CHECK(route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.v2PresentationEnabled == 0);
        CHECK(route.sourceCommand == script[i]);
        CHECK(route.runtimeCommand == script[i]);

        /* ReDMCSB CLIKMENU.C F0365/F0366 and DUNGEON.C direction tables are
         * represented here as the deterministic V1 command-truth seed; the V2
         * lane may only mirror it while presentation is disabled. */
        CHECK(apply_source_command(&v1.state, script[i]) == 1);
        CHECK(apply_source_command(&v2.state, route.runtimeCommand) == 1);
        CHECK(v1.state.x == v2.state.x);
        CHECK(v1.state.y == v2.state.y);
        CHECK(v1.state.dir == v2.state.dir);
        CHECK(v1.state.accepted == v2.state.accepted);

        v1.stateHash = hash_state(v1.stateHash, &v1.state);
        v2.stateHash = hash_state(v2.stateHash, &v2.state);
        CHECK(v1.stateHash == v2.stateHash);

        draw_seed_frame(&v1.state, v1Pixels);
        draw_seed_frame(&v2.state, v2Pixels);
        v1.pixelHash = hash_frame(v1Pixels);
        v2.pixelHash = hash_frame(v2Pixels);
        CHECK(v1.pixelHash == v2.pixelHash);

        compositeHash = hash_side_by_side_frame(v1Pixels, v2Pixels);
        sideBySideHash = fnv1a_u32(sideBySideHash, (uint32_t)(compositeHash & 0xFFFFFFFFULL));
        sideBySideHash = fnv1a_u32(sideBySideHash, (uint32_t)(compositeHash >> 32));
    }

    CHECK(v1.state.x == 4);
    CHECK(v1.state.y == 5);
    CHECK(v1.state.dir == 0);
    CHECK(v1.stateHash == 0x6b6e36b34cca3cd3ULL);
    CHECK(v2.stateHash == 0x6b6e36b34cca3cd3ULL);
    CHECK(sideBySideHash == 0xb457e27fff087299ULL);
    printf("dm1_v2_source_route_state_hash_pc34: script_len=%zu state_hash=%016llx side_by_side_hash=%016llx\n",
           sizeof(script) / sizeof(script[0]),
           (unsigned long long)v1.stateHash,
           (unsigned long long)sideBySideHash);
    return 0;
}

int main(void) {
    return test_v1_v2_source_route_state_hashes_match();
}
