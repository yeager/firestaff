#include "dm1_v2_movement_command_adapter_pc34.h"

#include <stdint.h>
#include <stdio.h>

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

static uint64_t fnv1a_u32(uint64_t hash, uint32_t value) {
    int byteIndex;
    for (byteIndex = 0; byteIndex < 4; byteIndex++) {
        hash ^= (uint64_t)((value >> (byteIndex * 8)) & 0xFFU);
        hash *= 1099511628211ULL;
    }
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
    SourceRouteState v1 = {4, 4, 0, 0};
    SourceRouteState v2 = {4, 4, 0, 0};
    uint64_t v1Hash = 14695981039346656037ULL;
    uint64_t v2Hash = 14695981039346656037ULL;
    size_t i;

    for (i = 0; i < sizeof(script) / sizeof(script[0]); i++) {
        DM1_V2_MovementCommand command;
        DM1_V2_MovementCommandRoute route;

        CHECK(command_to_v2_movement(script[i], &command));
        route = dm1_v2_movement_command_route_for_presentation(0, command);
        CHECK(route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.v2PresentationEnabled == 0);
        CHECK(route.sourceCommand == script[i]);
        CHECK(route.runtimeCommand == script[i]);

        CHECK(apply_source_command(&v1, script[i]) == 1);
        CHECK(apply_source_command(&v2, route.runtimeCommand) == 1);
        CHECK(v1.x == v2.x);
        CHECK(v1.y == v2.y);
        CHECK(v1.dir == v2.dir);
        CHECK(v1.accepted == v2.accepted);

        v1Hash = hash_state(v1Hash, &v1);
        v2Hash = hash_state(v2Hash, &v2);
        CHECK(v1Hash == v2Hash);
    }

    CHECK(v1.x == 4);
    CHECK(v1.y == 5);
    CHECK(v1.dir == 0);
    CHECK(v1Hash == 0x6b6e36b34cca3cd3ULL);
    CHECK(v2Hash == 0x6b6e36b34cca3cd3ULL);
    printf("dm1_v2_source_route_state_hash_pc34: script_len=%zu hash=%016llx\n",
           sizeof(script) / sizeof(script[0]),
           (unsigned long long)v1Hash);
    return 0;
}

int main(void) {
    return test_v1_v2_source_route_state_hashes_match();
}
