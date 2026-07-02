#include "memory_runtime_dynamics_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char* label, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s: got %d expected %d\n",
                label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_u32(const char* label, uint32_t actual, uint32_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s: got %u expected %u\n",
                label, actual, expected);
        return 0;
    }
    return 1;
}

static int test_c70_light_amount_adds_without_clamp(void)
{
    int out = 0;
    int ok = 1;

    /* ReDMCSB TIMELINE.C F0257 line 1759 adds the delta directly.
     * Darkness chains must preserve negative intermediate light amounts. */
    ok &= expect_int("negative darkness add",
                     F0867_RUNTIME_ComputeTotalLightAmount_Compat(-12, 7, &out),
                     1);
    ok &= expect_int("negative darkness intermediate", out, -5);

    ok &= expect_int("final darkness add",
                     F0867_RUNTIME_ComputeTotalLightAmount_Compat(-5, 5, &out),
                     1);
    ok &= expect_int("final darkness zero", out, 0);

    ok &= expect_int("positive light add",
                     F0867_RUNTIME_ComputeTotalLightAmount_Compat(24, -12, &out),
                     1);
    ok &= expect_int("positive light weakens", out, 12);

    return ok ? 0 : 1;
}

static int test_c70_handle_light_decay_followups(void)
{
    struct LightDecayResult_Compat out;
    int ok = 1;

    memset(&out, 0, sizeof(out));
    ok &= expect_int("darkness power2 handled",
                     F0864_RUNTIME_HandleLightDecay_Compat(2, 178, 4, &out),
                     1);
    ok &= expect_int("darkness power2 delta", out.magicalLightAmountDelta, 7);
    ok &= expect_int("darkness power2 followup", out.followupScheduled, 1);
    ok &= expect_int("darkness power2 not expired", out.expired, 0);
    ok &= expect_int("darkness followup kind",
                     out.followupEvent.kind, TIMELINE_EVENT_MAGIC_LIGHT_DECAY);
    ok &= expect_u32("darkness followup tick", out.followupEvent.fireAtTick, 182);
    ok &= expect_int("darkness followup map", out.followupEvent.mapIndex, 4);
    ok &= expect_int("darkness followup aux0", out.followupEvent.aux0, 1);

    memset(&out, 0, sizeof(out));
    ok &= expect_int("light power3 handled",
                     F0864_RUNTIME_HandleLightDecay_Compat(-3, 2730, 6, &out),
                     1);
    ok &= expect_int("light power3 delta", out.magicalLightAmountDelta, -12);
    ok &= expect_int("light power3 followup", out.followupScheduled, 1);
    ok &= expect_int("light followup aux0", out.followupEvent.aux0, -2);
    ok &= expect_u32("light followup tick", out.followupEvent.fireAtTick, 2734);
    ok &= expect_int("light followup map", out.followupEvent.mapIndex, 6);

    memset(&out, 0, sizeof(out));
    ok &= expect_int("terminal power1 handled",
                     F0864_RUNTIME_HandleLightDecay_Compat(1, 200, 6, &out),
                     1);
    ok &= expect_int("terminal power1 delta", out.magicalLightAmountDelta, 5);
    ok &= expect_int("terminal power1 expired", out.expired, 1);
    ok &= expect_int("terminal power1 no followup", out.followupScheduled, 0);

    return ok ? 0 : 1;
}

int main(void)
{
    if (test_c70_light_amount_adds_without_clamp() != 0) return 1;
    if (test_c70_handle_light_decay_followups() != 0) return 1;
    printf("ok: DM1 V1 runtime C70 light decay follows ReDMCSB F0257\n");
    return 0;
}
