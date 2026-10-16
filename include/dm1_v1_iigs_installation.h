#ifndef DM1_V1_IIGS_INSTALLATION_H
#define DM1_V1_IIGS_INSTALLATION_H

#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_metadata.h"
#include "dm1_v1_iigs_dungeon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_IigsInstallation {
    char graphics_path[ASSET_PATH_MAX];
    char program_path[ASSET_PATH_MAX];
    char start_path[ASSET_PATH_MAX];
    char dungeon_path[ASSET_PATH_MAX];
    DM1_V1_IigsMetadata metadata;
    DM1_V1_IigsDungeonReceipt world;
} DM1_V1_IigsInstallation;

/* Inspect every matching graphics owner under the selected directory or
 * physical container, without promoting it to its parent. Require all four
 * authentic files in the same native .po image; no neighboring disk supplies
 * companions. expected_version is 0 (either), 20 or 21. Returns 1 for the
 * first complete owner in sorted path order, 0 if none, -1 on invalid input
 * or collection/allocation failure, -2 if more than 256 graphics paths exist.
 * Collection overflow rejects the complete request without partial output.
 * Output is copied and zero on failure; no original bytes are retained.
 * This receipt does not admit a game or claim executable/runtime parity. */
int dm1_v1_iigs_installation_find(const char *selected_root,
    int expected_version, DM1_V1_IigsInstallation *out);

#ifdef __cplusplus
}
#endif
#endif
