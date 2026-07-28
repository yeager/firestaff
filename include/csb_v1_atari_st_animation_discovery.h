#ifndef FIRESTAFF_CSB_V1_ATARI_ST_ANIMATION_DISCOVERY_H
#define FIRESTAFF_CSB_V1_ATARI_ST_ANIMATION_DISCOVERY_H

#include "asset_find_by_hash.h"

typedef struct {
    int valid;
    int source_is_virtual;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    char source_identity[ASSET_PATH_MAX];
} CSB_V1_AtariStAnimationDiscoveryReceipt;

/* Locate the paired Atari ST animation files by their known original hashes.
 * The pair must originate from the same directory or archive, so a stray
 * ANIMATE.SCR cannot be combined with a DAT from another release. Virtual
 * archive paths are retained for the caller to materialize through the shared
 * asset extractor. */
int csb_v1_atari_st_animation_discover(
    const char *search_root, CSB_V1_AtariStAnimationDiscoveryReceipt *out);

/* Turn a verified virtual pair into ordinary files below cache_root. Loose
 * pairs are returned unchanged. The extracted files are rechecked by hash so
 * a stale cache or a changed archive cannot reach the Atari renderer. */
int csb_v1_atari_st_animation_materialize(
    const CSB_V1_AtariStAnimationDiscoveryReceipt *receipt,
    const char *cache_root, char script_path[ASSET_PATH_MAX],
    char data_path[ASSET_PATH_MAX]);

#endif
