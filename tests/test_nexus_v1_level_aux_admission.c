#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_LevelAuxAdmissionReceipt receipt;

    memset(&engine, 0, sizeof(engine));
    engine.level_loaded = 1;
    engine.level_aux_runtime_receipt.level_index = 4;
    engine.level_aux_runtime_receipt.canonical_pair_bound = 1;
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 1;
    engine.level_aux_runtime_receipt.sal.canonical_hash_verified = 1;
    engine.level_aux_runtime_receipt.map.canonical_hash_verified = 1;
    engine.level_aux_runtime_receipt.sound_driver.canonical_hash_verified = 1;
    engine.script_runtime_receipt.level_index = 4;
    engine.script_runtime_receipt.status =
        NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT;
    engine.script_runtime_receipt.real_task_profile_supported = 1;
    engine.script_runtime_receipt.real_task_header_supported = 1;
    engine.script_runtime_receipt.blocks_real_script_dispatch = 1;
    engine.sfx_runtime_receipt.level_index = 4;
    engine.sfx_runtime_receipt.status =
        NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE;
    engine.sfx_runtime_receipt.sal_loaded = 1;
    engine.sfx_runtime_receipt.map_loaded = 1;
    engine.sfx_runtime_receipt.sal_canonical_source_verified = 1;
    engine.sfx_runtime_receipt.map_canonical_source_verified = 1;
    engine.sfx_runtime_receipt.sound_driver_canonical_source_verified = 1;
    engine.sfx_runtime_receipt.blocks_real_sfx_playback = 1;
    CHECK(nexus_v1_current_level_aux_admission_receipt(&engine, &receipt) == 1 &&
              receipt.status == NEXUS_V1_LEVEL_AUX_ADMISSION_READY_NO_RUNTIME &&
              receipt.canonical_sources_bound && receipt.slev_task_profile_bound &&
              receipt.sal_map_profile_bound && receipt.sound_driver_bound &&
              receipt.no_runtime_only && !receipt.fallback_visuals_permitted,
          "verified SLEV/SAL/MAP/driver profiles join without runtime promotion");
    engine.sfx_runtime_receipt.playback_enabled = 1;
    CHECK(nexus_v1_current_level_aux_admission_receipt(&engine, &receipt) == 0 &&
              receipt.status == NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SOUND &&
              receipt.blocks_real_sfx_playback && !receipt.fallback_visuals_permitted,
          "playback promotion blocks the auxiliary route");
    engine.sfx_runtime_receipt.playback_enabled = 0;
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 0;
    CHECK(nexus_v1_current_level_aux_admission_receipt(&engine, &receipt) == 0 &&
              receipt.status == NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SOURCE &&
              !receipt.fallback_visuals_permitted,
          "a changed SLEV source blocks the whole auxiliary route");

    if (failures) return 1;
    puts("Nexus level auxiliary admission passed");
    return 0;
}
