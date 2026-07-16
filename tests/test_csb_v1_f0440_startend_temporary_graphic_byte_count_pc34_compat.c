#include "csb_v1_f0440_startend_temporary_graphic_byte_count_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static CSB_V1_StartupGraphicsBoundaryReceipt_PC34 make_graphics_boundary(void)
{
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.attempted_stage_mask =
        CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34;
    receipt.blocked_stage_mask =
        CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34;
    receipt.no_synthetic_fallback_mask =
        CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34;
    return receipt;
}

static CSB_V1_F0440_TemporaryGraphicFacts_PC34 make_complete_facts(
    int graphic_index)
{
    CSB_V1_F0440_TemporaryGraphicFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.graphic_index = graphic_index;
    facts.decompressed_byte_count = 4096;
    facts.target_pointer_bound = 1;
    facts.allocated_on_temporary_heap_top = 1;
    facts.not_expanded_graphic_route = 1;
    facts.real_graphics_dat_member_bound = 1;
    facts.real_decompressed_payload_bound = 1;
    facts.load_decompress_expand_route_reviewed = 1;
    facts.no_synthetic_graphic_bytes = 1;
    facts.no_synthetic_file_handle = 1;
    facts.no_legacy_graphics_wrapper = 1;
    facts.graphics_boundary = make_graphics_boundary();
    return facts;
}

static void test_accepts_real_entrance_temporary_graphics(void)
{
    const int graphics[] = {
        CSB_V1_F0440_GRAPHIC_ENTRANCE_PC34,
        CSB_V1_F0440_GRAPHIC_CREDITS_PC34,
        CSB_V1_F0440_GRAPHIC_SOUND_DOOR_RATTLE_PC34,
        CSB_V1_F0440_GRAPHIC_SOUND_SWITCH_PC34
    };
    size_t i;

    for (i = 0; i < sizeof(graphics) / sizeof(graphics[0]); i++) {
        CSB_V1_F0440_TemporaryGraphicFacts_PC34 facts =
            make_complete_facts(graphics[i]);
        CSB_V1_F0440_TemporaryGraphicReceipt_PC34 receipt;
        const long byte_count =
            F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
                &facts, &receipt);

        CHECK(byte_count == facts.decompressed_byte_count);
        CHECK(receipt.valid == 1);
        CHECK(receipt.graphic_index == graphics[i]);
        CHECK(receipt.decompressed_byte_count == facts.decompressed_byte_count);
        CHECK(receipt.temporary_heap_allocation_bound == 1);
        CHECK(receipt.not_expanded_graphic_route == 1);
        CHECK(receipt.graphics_boundary_consumed == 1);
        CHECK(receipt.no_synthetic_graphic_bytes == 1);
        CHECK(receipt.no_synthetic_file_handle == 1);
        CHECK(receipt.no_legacy_graphics_wrapper == 1);
    }
}

static void test_rejects_synthetic_or_unreviewed_routes(void)
{
    CSB_V1_F0440_TemporaryGraphicFacts_PC34 facts =
        make_complete_facts(CSB_V1_F0440_GRAPHIC_ENTRANCE_PC34);
    CSB_V1_F0440_TemporaryGraphicReceipt_PC34 receipt;

    facts.graphic_index = 999;
    CHECK(F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
              &facts, &receipt) == 0);
    CHECK(receipt.valid == 0);
    CHECK(receipt.no_synthetic_graphic_bytes == 1);
    CHECK(receipt.no_synthetic_file_handle == 1);

    facts = make_complete_facts(CSB_V1_F0440_GRAPHIC_CREDITS_PC34);
    facts.decompressed_byte_count = 0;
    CHECK(F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
              &facts, &receipt) == 0);

    facts = make_complete_facts(CSB_V1_F0440_GRAPHIC_CREDITS_PC34);
    facts.real_decompressed_payload_bound = 0;
    CHECK(F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
              &facts, &receipt) == 0);

    facts = make_complete_facts(CSB_V1_F0440_GRAPHIC_CREDITS_PC34);
    facts.no_synthetic_graphic_bytes = 0;
    CHECK(F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
              &facts, &receipt) == 0);

    facts = make_complete_facts(CSB_V1_F0440_GRAPHIC_CREDITS_PC34);
    facts.graphics_boundary.no_synthetic_fallback_mask = 0;
    CHECK(F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
              &facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    const char *evidence =
        csb_v1_f0440_temporary_loaded_graphic_byte_count_source_evidence_pc34();
    check_contains(evidence, "ENTRANCE.C:600-617");
    check_contains(evidence, "F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount");
    check_contains(evidence, "F0490_MEMORY_LoadDecompressAndExpandGraphic");
    check_contains(evidence, "C004 entrance");
    check_contains(evidence, "C005 credits");
}

int main(void)
{
    test_accepts_real_entrance_temporary_graphics();
    test_rejects_synthetic_or_unreviewed_routes();
    test_evidence_string();
    return 0;
}
