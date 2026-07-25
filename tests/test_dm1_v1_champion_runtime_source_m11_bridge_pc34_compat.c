#include "dm1_v1_champion_runtime_source_m11_bridge_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_command_kind_enum(void)
{
    assert(DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34 == 1);
    assert(DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34 == 2);
    assert(DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34 == 3);
    assert(DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34 == 4);
    assert(DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34 == 5);
}

static void test_init(void)
{
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 s;
    dm1_v1_champion_runtime_source_m11_bridge_init_pc34(&s);
    assert(s.lastTick == 0);
    assert(s.lastGeneration == 0);
}

static void test_state_struct_layout(void)
{
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 s;
    memset(&s, 0, sizeof(s));
    assert(s.lastTick == 0);
    assert(s.lastGeneration == 0);
}

static void test_command_struct_layout(void)
{
    Dm1V1ChampionRuntimeSourceM11CommandPc34 cmd;
    memset(&cmd, 0, sizeof(cmd));
    assert(cmd.kind == 0);
    assert(cmd.championIndex == 0);
    assert(cmd.originalPixels == NULL);
    assert(cmd.portraitPixels == NULL);
}

static void test_receipt_struct_layout(void)
{
    Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.clearOnly == 0);
    assert(r.commandCount == 0);
}

static void test_source_evidence(void)
{
    const char* e = dm1_v1_champion_runtime_source_m11_bridge_source_evidence_pc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_command_kind_enum();
    test_init();
    test_state_struct_layout();
    test_command_struct_layout();
    test_receipt_struct_layout();
    test_source_evidence();

    puts("ok: DM1 champion runtime source M11 bridge (Q-DM1-07) 6 tests passed");
    return 0;
}
