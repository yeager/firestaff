#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0756_evaluate_memory_requirements_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\\n", message); \
            return 1; \
        } \
    } while (0)

struct fixture {
    int32_t available_by_map[3];
    int16_t seen_maps[3];
    int seen_count;
    int out_of_memory_count;
    int32_t reported_kib;
    int endgame_count;
    int32_t *available_memory_for_sounds;
};

static void process_map(void *user, int16_t map_index)
{
    struct fixture *fixture = (struct fixture *)user;

    fixture->seen_maps[fixture->seen_count++] = map_index;
    *fixture->available_memory_for_sounds = fixture->available_by_map[map_index];
}

static void out_of_memory(void *user, int32_t kib_needed)
{
    struct fixture *fixture = (struct fixture *)user;

    fixture->out_of_memory_count++;
    fixture->reported_kib = kib_needed;
}

static void endgame(void *user)
{
    struct fixture *fixture = (struct fixture *)user;

    fixture->endgame_count++;
}

int main(void)
{
    int16_t party_map_index = 7;
    int32_t available_memory_for_sounds = 0;
    int32_t needed_memory = -8000;
    uint8_t evaluating = 0U;
    struct fixture fixture;
    struct redmcsb_f0756_memory_requirements_pc34_compat state;

    memset(&fixture, 0, sizeof(fixture));
    fixture.available_by_map[0] = 9000;
    fixture.available_by_map[1] = 5000;
    fixture.available_by_map[2] = 7000;
    fixture.available_memory_for_sounds = &available_memory_for_sounds;
    state.map_count = 3;
    state.party_map_index = &party_map_index;
    state.available_memory_for_sounds = &available_memory_for_sounds;
    state.needed_memory = &needed_memory;
    state.evaluating_memory_requirements = &evaluating;
    state.process_map = process_map;
    state.out_of_memory = out_of_memory;
    state.endgame = endgame;
    state.user = &fixture;

    CHECK(redmcsb_f0756_evaluate_memory_requirements_pc34_compat(
              &state, 0, 1000) == 1,
          "minimum available sound memory remains nonnegative");
    CHECK(fixture.seen_count == 3 && fixture.seen_maps[0] == 0 &&
              fixture.seen_maps[1] == 1 && fixture.seen_maps[2] == 2,
          "every map is processed in ascending order");
    CHECK(needed_memory == 4000,
          "needed memory becomes the minimum per-map availability");
    CHECK(evaluating == 0U && party_map_index == 7,
          "source temporary state is cleared and party map is restored");
    CHECK(fixture.out_of_memory_count == 0 && fixture.endgame_count == 0,
          "nonnegative route does not invoke fatal callbacks");

    memset(&fixture, 0, sizeof(fixture));
    fixture.available_by_map[0] = 100;
    fixture.available_by_map[1] = -3000;
    fixture.available_memory_for_sounds = &available_memory_for_sounds;
    needed_memory = -3000;
    party_map_index = 2;
    CHECK(redmcsb_f0756_evaluate_memory_requirements_pc34_compat(
              &state, 1, 0) == 0,
          "negative final profile reports failure");
    CHECK(fixture.out_of_memory_count == 1 && fixture.reported_kib == 3 &&
              fixture.endgame_count == 1,
          "final negative route reports rounded KiB then calls endgame");
    CHECK(evaluating == 0U && party_map_index == 2,
          "cleanup remains observable when portable endgame callback returns");
    CHECK(strstr(redmcsb_f0756_evaluate_memory_requirements_source_evidence_pc34(),
                 "STARTUP2.C:1081-1120") != NULL,
          "source evidence names the exact ReDMCSB range");

    puts("ok: ReDMCSB F0756 PC 3.4 memory requirement evaluation");
    return 0;
}
