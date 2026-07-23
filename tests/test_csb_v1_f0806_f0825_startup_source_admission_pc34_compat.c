#include "csb_v1_f0806_f0825_startup_source_admission_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static CSB_V1_StartupRealPackageConsumptionReceipt_PC34 complete_package(void)
{
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package;
    memset(&package, 0, sizeof(package));
    package.valid = 1;
    package.real_package_matched = 1;
    package.c001_title_consumed = 1;
    package.c001_presents_consumed = 1;
    package.c001_chaos_consumed = 1;
    package.c001_strikes_back_consumed = 1;
    package.c002_left_door_consumed = 1;
    package.c003_right_door_consumed = 1;
    package.c004_entrance_consumed = 1;
    package.c017_hud_consumed = 1;
    package.c040_hud_consumed = 1;
    package.title_to_entrance_same_session = 1;
    package.title_to_hud_same_session = 1;
    package.no_legacy_wrappers = 1;
    package.no_fallback_routes = 1;
    package.source_tick = 77u;
    package.session_generation = 4u;
    package.real_asset_receipt_hash = UINT64_C(0x3a77);
    return package;
}

int main(void)
{
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package = complete_package();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 entrance;
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 door;
    CSB_V1_F0806F0825StartupSourceReceiptPc34 receipt;

    memset(&entrance, 0, sizeof(entrance));
    memset(&door, 0, sizeof(door));
    entrance.valid = 1;
    entrance.opening_material_consumed = 1;
    entrance.no_synthetic_input = 1;
    entrance.no_synthetic_graphics_bytes = 1;
    entrance.no_fallback_visuals = 1;
    entrance.real_asset_receipt_hash = package.real_asset_receipt_hash;
    entrance.session_generation = package.session_generation;
    door.valid = 1;
    door.animation_step_bound = 1;
    door.target_screen_bound = 1;
    door.bitplanes_consumed = 1;
    door.runtime_coupling_consumed = 1;
    door.draw_consumes_receipt_only = 1;
    door.no_synthetic_visuals = 1;
    door.accepted_animation_step_index = CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34;

    check(csb_v1_f0806_f0825_startup_source_admit_pc34(
              &package, &entrance, &door, 806u, &receipt) == 1,
          "F0806 consumes the real package session");
    check(receipt.authentic_package_consumed && receipt.existing_runtime_owner_required &&
              receipt.runtime_execution_blocked && receipt.package_receipt_hash ==
              package.real_asset_receipt_hash,
          "F0806 receipt is read-only and source-bound");
    check(csb_v1_f0806_f0825_startup_source_admit_pc34(
              &package, &entrance, &door, 807u, &receipt) == 1,
          "F0807 consumes the real door animation session");
    package.c003_right_door_consumed = 0;
    check(csb_v1_f0806_f0825_startup_source_admit_pc34(
              &package, &entrance, &door, 807u, &receipt) == 0,
          "missing door package material fails closed");
    package = complete_package();
    check(csb_v1_f0806_f0825_startup_source_admit_pc34(
              &package, &entrance, &door, 813u, &receipt) == 0,
          "MIDI route remains blocked without a verified package owner");
    check(csb_v1_f0806_f0825_startup_source_admit_pc34(
              &package, &entrance, &door, 820u, &receipt) == 0,
          "unnumbered F0820 route remains fail-closed");

    printf("csb_v1_f0806_f0825_startup_source_admission: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
