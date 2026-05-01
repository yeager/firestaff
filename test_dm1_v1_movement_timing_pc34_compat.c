#include <stdio.h>
#include <string.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static void setup_party(struct PartyState_Compat* party)
{
    memset(party, 0, sizeof(*party));
    party->championCount = 3;
    party->mapIndex = 0;
    party->mapX = 3;
    party->mapY = 2;
    party->direction = DIR_NORTH;

    party->champions[0].hp.current = 25;
    party->champions[0].load = 40;
    party->champions[0].maxLoad = 100;

    party->champions[1].hp.current = 0;
    party->champions[1].load = 250;
    party->champions[1].maxLoad = 100;

    party->champions[2].hp.current = 30;
    party->champions[2].load = 100;
    party->champions[2].maxLoad = 100;
    party->champions[2].wounds = LIFECYCLE_WOUND_FEET;
}

int main(void)
{
    struct PartyState_Compat party;
    struct Dm1V1MovementTimingResultPc34Compat timing;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult;
    int footwear[CHAMPION_MAX_PARTY] = { 0, 0, LIFECYCLE_ICON_BOOT_OF_SPEED, 0 };
    int dx;
    int dy;
    int ok = 1;

    printf("probe=dm1_v1_movement_timing_pc34_compat\n");
    printf("sourceEvidence=%s\n", DM1_V1_MovementTiming_SourceEvidencePc34Compat());

    ok &= expect_int("unloaded champion cadence 2 ticks",
        (int)DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(40, 100, 0, 0), 2);
    ok &= expect_int("heavy but under max cadence 3 ticks",
        (int)DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(70, 100, 0, 0), 3);
    ok &= expect_int("BUG0_72 load equals max uses overloaded cadence",
        (int)DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(100, 100, 0, 0), 4);
    ok &= expect_int("wounded overloaded boots cadence",
        (int)DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(100, 100, LIFECYCLE_WOUND_FEET, LIFECYCLE_ICON_BOOT_OF_SPEED), 5);

    setup_party(&party);
    ok &= expect_int("party cadence uses max living champion and ignores dead overweight champion",
        DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat(&party, footwear), 5);

    F0701_MOVEMENT_GetStepDelta_Compat(party.direction, MOVE_RIGHT, &dx, &dy);
    ok &= expect_int("right side-step dx from command cadence path", dx, 1);
    ok &= expect_int("right side-step dy from command cadence path", dy, 0);
    party.mapX += dx;
    party.mapY += dy;
    timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(&party, 0, 3, 2, 100476ul, 100400ul, footwear);
    ok &= expect_int("successful side-step sets disabled movement ticks", timing.disabledMovementTicks, 5);
    ok &= expect_int("successful side-step clears projectile movement ticks", timing.projectileDisabledMovementTicks, 0);
    ok &= expect_int("successful side-step records scent delay", timing.scentDelayTicks, 76);
    ok &= expect_int("successful side-step updates last party movement time", (int)timing.lastPartyMovementTime, 100476);
    ok &= expect_int("successful side-step records scent only after square change", timing.scentRecorded, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("turn key enqueues while cadence active", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 5, 0, 0);
    ok &= expect_int("turn bypasses step cadence disabled gate", queueResult.movementDisabledGate, 0);
    ok &= expect_int("turn dispatches without movement cadence update", queueResult.dispatchedTurn, 1);
    ok &= expect_int("turn is not a step dispatch", queueResult.dispatchedMove, 0);

    party.championCount = 0;
    timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(&party, 0, 3, 2, 100500ul, 100476ul, footwear);
    ok &= expect_int("empty-party successful move keeps baseline one tick cadence", timing.disabledMovementTicks, 1);
    ok &= expect_int("empty-party successful move does not record scent", timing.scentRecorded, 0);
    ok &= expect_int("empty-party successful move keeps prior last movement time", (int)timing.lastPartyMovementTime, 100476);

    printf("dm1V1MovementTimingInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
