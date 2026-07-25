#include "dm1_v1_save_load_system_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_SL_SAVE_MAGIC == 0x444D3156);
    assert(DM1_SL_HEADER_SIZE == 256);
    assert(DM1_SL_ADDITIONAL_DATA == 134);
    assert(DM1_SL_MAX_SLOTS == 20);
    assert(DM1_SL_SOURCE_RUNTIME_SLOT == 0);
    assert(DM1_SL_SOURCE_RUNTIME_SLOT_COUNT == 1);
}

static void test_init(void)
{
    DM1_V1_SaveLoadStatePc34 s;
    DM1_V1_SaveLoad_InitPc34Compat(&s, "/tmp/test_saves");
    assert(s.initialized == true);
    assert(s.slot_count == 0);
}

static void test_slot_not_occupied(void)
{
    DM1_V1_SaveLoadStatePc34 s;
    DM1_V1_SaveLoad_InitPc34Compat(&s, "/tmp/test_saves");
    int occ = DM1_V1_SaveLoad_SlotOccupiedPc34Compat(&s, 0);
    (void)occ;
    assert(!occ);
}

static void test_header_struct(void)
{
    DM1_V1_SaveLoadHeaderPc34 h;
    memset(&h, 0, sizeof(h));
    h.magic = DM1_SL_SAVE_MAGIC;
    h.game_id = 12345;
    h.party_facing = 2;
    assert(h.magic == 0x444D3156);
    assert(h.game_id == 12345);
    assert(h.party_facing == 2);
}

static void test_runtime_slot_supported(void)
{
    int s0 = DM1_V1_SaveLoad_SourceRuntimeSlotSupportedPc34Compat(0);
    int s1 = DM1_V1_SaveLoad_SourceRuntimeSlotSupportedPc34Compat(1);
    (void)s0; (void)s1;
    assert(s0 == 1);
    assert(s1 == 0);
}

static void test_runtime_slot_count(void)
{
    uint8_t c = DM1_V1_SaveLoad_SourceRuntimeSlotCountPc34Compat();
    (void)c;
    assert(c == 1);
}

int main(void)
{
    test_constants();
    test_init();
    test_slot_not_occupied();
    test_header_struct();
    test_runtime_slot_supported();
    test_runtime_slot_count();

    puts("ok: DM1 save load system (Q-DM1-08) 6 tests passed");
    return 0;
}
