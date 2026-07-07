#ifndef THERON_V1_STARTUP_RUNTIME_ENTRY_H
#define THERON_V1_STARTUP_RUNTIME_ENTRY_H

#include "theron_v1_dungeon_progression.h"
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

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_RUNTIME_ENTRY_H */
