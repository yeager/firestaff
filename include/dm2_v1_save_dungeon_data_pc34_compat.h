#ifndef DM2_V1_SAVE_DUNGEON_DATA_PC34_COMPAT_H
#define DM2_V1_SAVE_DUNGEON_DATA_PC34_COMPAT_H

#include "dm2_v1_save_load.h"
#include "dm2_v1_dungeon_loader.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 save: per-tile SUPPRESS byte count for DM2_STORE_EXTRA_DUNGEON_DATA.
 * Source: sksvgame.cpp:1992-2023 — tile type (bits 7:5) maps to a byte count
 * that controls how many SUPPRESS bytes are written for the tile header.
 *
 * Type 0 (wall):       0 bytes
 * Type 1 (open):       0 bytes
 * Type 2 (pit):        8 bytes
 * Type 3 (stairs):     0 bytes
 * Type 4 (door):       7 bytes
 * Type 5 (teleporter): 8 bytes (or 0 if forward reference)
 * Type 6 (fake wall):  4 bytes
 * Type 7 (open+trick): 0 bytes */
int dm2_v1_save_tile_suppress_size(uint8_t tile_type_3bit);

/* Determine whether a teleporter tile's target map index creates a forward
 * reference (target map > current map). Forward references skip the tile
 * SUPPRESS write and the record-checkcode walk.
 * Source: sksvgame.cpp:2010-2017.
 * Returns 1 if forward reference (skip), 0 if normal (write). */
int dm2_v1_save_teleporter_is_forward_ref(int current_map, int target_map);

typedef struct {
    int valid;
    int maps_processed;
    int tiles_processed;
    int suppress_bytes_written;
    int record_chains_written;
    int forward_teleporters_skipped;
    int fail_closed;
} DM2_V1_SaveDungeonDataReceipt;

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_DUNGEON_DATA_PC34_COMPAT_H */
