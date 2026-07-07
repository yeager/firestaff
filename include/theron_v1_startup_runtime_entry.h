#ifndef THERON_V1_STARTUP_RUNTIME_ENTRY_H
#define THERON_V1_STARTUP_RUNTIME_ENTRY_H

#include "theron_v1_dungeon_progression.h"
#include "theron_v1_startup_flow.h"
#include "theron_v1_world.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int theron_v1_startup_runtime_load_initial_level(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    char *receipt,
    size_t receipt_cap);

typedef struct {
    const uint8_t *hucard_rom;
    size_t hucard_rom_size;
    const char *md5_hex;
    const char *const *roster_names;
    int roster_name_count;
} Theron_V1StartupRuntimeEntryRequest;

typedef struct {
    Theron_StartupResult result;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
} Theron_V1StartupRuntimeEntryResult;

void theron_v1_startup_runtime_entry_request_init(
    Theron_V1StartupRuntimeEntryRequest *request);
void theron_v1_startup_runtime_entry_result_init(
    Theron_V1StartupRuntimeEntryResult *result);
int theron_v1_startup_runtime_enter_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const Theron_V1StartupRuntimeEntryRequest *request,
    Theron_V1StartupRuntimeEntryResult *out_result,
    char *receipt,
    size_t receipt_cap);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_RUNTIME_ENTRY_H */
