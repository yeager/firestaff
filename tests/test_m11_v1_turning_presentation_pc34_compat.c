#include <stdio.h>
#include <string.h>

#include "m11_v1_turning_presentation_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

int main(void)
{
    struct M11V1TurningChampionPosePc34Compat poses[2];
    struct M11V1TurningPresentationResultPc34Compat r;
    struct PartyState_Compat party;
    int ok = 1;

    printf("probe=m11_v1_turning_presentation_pc34_compat\n");
    printf("sourceEvidence=%s\n", m11_v1_turning_presentation_source_evidence_pc34_compat());

    ok &= expect_int("left command recognized", m11_v1_turning_is_turn_command_pc34_compat(DM1_V1_COMMAND_TURN_LEFT), 1);
    ok &= expect_int("right command recognized", m11_v1_turning_is_turn_command_pc34_compat(DM1_V1_COMMAND_TURN_RIGHT), 1);
    ok &= expect_int("forward is not turn", m11_v1_turning_is_turn_command_pc34_compat(DM1_V1_COMMAND_MOVE_FORWARD), 0);
    ok &= expect_int("right from north becomes east", m11_v1_turning_target_direction_pc34_compat(DIR_NORTH, DM1_V1_COMMAND_TURN_RIGHT), DIR_EAST);
    ok &= expect_int("left from north becomes west", m11_v1_turning_target_direction_pc34_compat(DIR_NORTH, DM1_V1_COMMAND_TURN_LEFT), DIR_WEST);

    poses[0].cell = DIR_NORTH;
    poses[0].direction = DIR_NORTH;
    poses[1].cell = DIR_WEST;
    poses[1].direction = DIR_SOUTH;
    r = m11_v1_turning_apply_original_presentation_pc34_compat(
        M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL,
        DM1_V1_COMMAND_TURN_RIGHT,
        DIR_NORTH,
        poses,
        2);
    ok &= expect_int("right applied", r.applied, 1);
    ok &= expect_int("right delta is one quarter", r.delta, 1);
    ok &= expect_int("right one source step per ninety", r.quarterTurnSteps, 1);
    ok &= expect_int("right single rendered endpoint frame", r.animationFrames, 1);
    ok &= expect_int("right no intermediate yaw frames", r.intermediateFrames, 0);
    ok &= expect_int("right render direction is endpoint", r.renderDirection, DIR_EAST);
    ok &= expect_int("right waits for viewport vblank", r.waitsForViewportVBlank, 1);
    ok &= expect_int("right redraw requested by next game loop", r.redrawOnNextGameLoop, 1);
    ok &= expect_int("right stops waiting for input", r.stopWaitingForPlayerInput, 1);
    ok &= expect_int("right has no wall block check", r.wallBlockCheck, 0);
    ok &= expect_int("right highlight", r.highlightRight, 1);
    ok &= expect_int("left highlight off", r.highlightLeft, 0);
    ok &= expect_int("pose0 cell rotates", poses[0].cell, DIR_EAST);
    ok &= expect_int("pose0 direction rotates", poses[0].direction, DIR_EAST);
    ok &= expect_int("pose1 cell wraps", poses[1].cell, DIR_NORTH);
    ok &= expect_int("pose1 direction rotates", poses[1].direction, DIR_WEST);

    poses[0].cell = DIR_EAST;
    poses[0].direction = DIR_EAST;
    r = m11_v1_turning_apply_original_presentation_pc34_compat(
        M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL,
        DM1_V1_COMMAND_TURN_LEFT,
        DIR_NORTH,
        poses,
        1);
    ok &= expect_int("left applied", r.applied, 1);
    ok &= expect_int("left delta is three modulo quarter steps", r.delta, 3);
    ok &= expect_int("left target west", r.newDirection, DIR_WEST);
    ok &= expect_int("left highlight", r.highlightLeft, 1);
    ok &= expect_int("pose cell rotates left", poses[0].cell, DIR_NORTH);

    r = m11_v1_turning_apply_original_presentation_pc34_compat(
        M11_V1_TURNING_PRESENTATION_MODE_OTHER,
        DM1_V1_COMMAND_TURN_RIGHT,
        DIR_NORTH,
        poses,
        1);
    ok &= expect_int("non-v1 presentation guard no-op", r.applied, 0);
    ok &= expect_int("non-v1 direction unchanged", r.newDirection, DIR_NORTH);

    memset(&party, 0, sizeof(party));
    party.direction = DIR_WEST;
    party.championCount = 2;
    party.champions[0].direction = DIR_NORTH;
    party.champions[1].direction = DIR_EAST;
    r = m11_v1_turning_apply_party_original_presentation_pc34_compat(
        M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL,
        DM1_V1_COMMAND_TURN_RIGHT,
        &party);
    ok &= expect_int("party turn applied", r.applied, 1);
    ok &= expect_int("party direction wraps west->north", party.direction, DIR_NORTH);
    ok &= expect_int("champion0 direction rotated", party.champions[0].direction, DIR_EAST);
    ok &= expect_int("champion1 direction rotated", party.champions[1].direction, DIR_SOUTH);

    if (ok) {
        printf("PASS m11_v1_turning_presentation_pc34_compat\n");
        return 0;
    }
    return 1;
}
