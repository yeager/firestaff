#include "csb_v1_f0579_entrance_bitplanes_runtime_coupling_pc34_compat.h"

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

static CSB_V1_StartupRuntimeCouplingReceipt_PC34 make_runtime_coupling(void)
{
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.real_startup_assets_bound = 1;
    receipt.title_phase_route_complete = 1;
    receipt.hud_runtime_coupled = 1;
    receipt.door_runtime_coupled = 1;
    receipt.draw_consumes_receipt_only = 1;
    receipt.input_consumes_receipt_only = 1;
    receipt.no_legacy_wrappers = 1;
    receipt.no_synthetic_visuals = 1;
    return receipt;
}

static CSB_V1_F0579_EntranceBitplanesFacts_PC34 make_complete_facts(void)
{
    CSB_V1_F0579_EntranceBitplanesFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.source_composite_real_asset_bound = 1;
    facts.target_screen_real_asset_bound = 1;
    facts.source_width = CSB_V1_F0579_SOURCE_COMPOSITE_WIDTH_PC34;
    facts.source_height = CSB_V1_F0579_SOURCE_COMPOSITE_HEIGHT_PC34;
    facts.target_width = CSB_V1_F0579_TARGET_SCREEN_WIDTH_PC34;
    facts.target_height = CSB_V1_F0579_TARGET_SCREEN_HEIGHT_PC34;
    facts.upper_half_target_y = CSB_V1_F0579_TARGET_UPPER_Y_PC34;
    facts.lower_half_target_y = CSB_V1_F0579_TARGET_LOWER_Y_PC34;
    facts.source_plane_count = CSB_V1_F0579_DOOR_HALF_PLANE_COUNT_PC34;
    facts.target_plane_count = CSB_V1_F0579_DOOR_HALF_PLANE_COUNT_PC34;
    facts.no_legacy_bitplane_wrapper = 1;
    facts.no_synthetic_visuals = 1;
    facts.runtime_coupling = make_runtime_coupling();
    return facts;
}

static void test_accepts_real_runtime_coupled_bitplanes(void)
{
    CSB_V1_F0579_EntranceBitplanesFacts_PC34 facts = make_complete_facts();
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 receipt;

    CHECK(F0579_ENTRANCE_InitializeBitPlanes(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.source_composite_bound == 1);
    CHECK(receipt.target_screen_bound == 1);
    CHECK(receipt.geometry_source_locked == 1);
    CHECK(receipt.runtime_coupling_consumed == 1);
    CHECK(receipt.title_hud_door_runtime_ready == 1);
    CHECK(receipt.no_legacy_bitplane_wrapper == 1);
    CHECK(receipt.no_synthetic_visuals == 1);
}

static void test_rejects_missing_runtime_or_wrong_geometry(void)
{
    CSB_V1_F0579_EntranceBitplanesFacts_PC34 facts = make_complete_facts();
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 receipt;

    facts.runtime_coupling.title_phase_route_complete = 0;
    CHECK(F0579_ENTRANCE_InitializeBitPlanes(&facts, &receipt) == 0);
    CHECK(receipt.valid == 0);
    CHECK(receipt.no_synthetic_visuals == 1);

    facts = make_complete_facts();
    facts.source_width = 255;
    CHECK(F0579_ENTRANCE_InitializeBitPlanes(&facts, &receipt) == 0);
    CHECK(receipt.geometry_source_locked == 0);

    facts = make_complete_facts();
    facts.upper_half_target_y = 28;
    CHECK(F0579_ENTRANCE_InitializeBitPlanes(&facts, &receipt) == 0);

    facts = make_complete_facts();
    facts.no_legacy_bitplane_wrapper = 0;
    CHECK(F0579_ENTRANCE_InitializeBitPlanes(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    const char *evidence =
        csb_v1_f0579_entrance_initialize_bitplanes_source_evidence_pc34();
    check_contains(evidence, "ENTRANCE.C:1095-1123");
    check_contains(evidence, "256x161");
    check_contains(evidence, "320x200");
    check_contains(evidence, "y=30");
    check_contains(evidence, "y=110");
}

int main(void)
{
    test_accepts_real_runtime_coupled_bitplanes();
    test_rejects_missing_runtime_or_wrong_geometry();
    test_evidence_string();
    return 0;
}
