#include "dm1_v1_champion_live_m11_capture_lifecycle_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_state_struct(void)
{
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 s;
    memset(&s, 0, sizeof(s));
    assert(s.lastTick == 0);
    assert(s.lastGeneration == 0);
}

static void test_receipt_struct(void)
{
    Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.clearOnly == 0);
    assert(r.commandCount == 0);
}

static void test_init(void)
{
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 s;
    s.lastTick = 99;
    s.lastGeneration = 99;
    dm1_v1_champion_live_m11_capture_lifecycle_init_pc34(&s);
    assert(s.lastTick == 0);
    assert(s.lastGeneration == 0);
}

static void test_step_null_state(void)
{
    Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 r;
    int ok = dm1_v1_champion_live_m11_capture_lifecycle_step_pc34(
        NULL, NULL, NULL, &r);
    (void)ok;
    assert(ok == 0);
}

static void test_step_null_receipt(void)
{
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 s;
    dm1_v1_champion_live_m11_capture_lifecycle_init_pc34(&s);
    int ok = dm1_v1_champion_live_m11_capture_lifecycle_step_pc34(
        &s, NULL, NULL, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_champion_live_m11_capture_lifecycle_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_state_struct();
    test_receipt_struct();
    test_init();
    test_step_null_state();
    test_step_null_receipt();
    test_source_evidence();

    puts("ok: DM1 champion live M11 capture lifecycle (Q-DM1-07) 6 tests passed");
    return 0;
}
