#ifndef NEXUS_V1_SLEV_SAL_ASSET_DISCOVERY_H
#define NEXUS_V1_SLEV_SAL_ASSET_DISCOVERY_H

#include "asset_find_by_hash.h"

#include <stdint.h>

#define NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT 16U

/* Direct-file identity only. No caller receives or retains the source bytes. */
typedef struct {
    const char *slev_name;
    const char *slev_md5;
    const char *sal_name;
    const char *sal_md5;
    const char *map_name;
    const char *map_md5;
} Nexus_V1_SlevSalExpectedLevel;

typedef struct {
    int valid;
    char direct_path[ASSET_PATH_MAX];
    char canonical_name[16];
    char md5[33];
    uint64_t byte_count;
    uint64_t fnv1a64;
} Nexus_V1_SlevSalDirectIdentity;

typedef struct {
    int valid;
    uint32_t level_index;
    Nexus_V1_SlevSalDirectIdentity slev;
    Nexus_V1_SlevSalDirectIdentity sal;
    Nexus_V1_SlevSalDirectIdentity map;
} Nexus_V1_SlevSalDirectLevelIdentity;

typedef struct {
    int valid;
    int skipped_missing_assets;
    int rejected_mixed_assets;
    int direct_files_only;
    int payload_materialized;
    uint64_t corpus_fnv1a64;
    uint64_t corpus_byte_count;
    Nexus_V1_SlevSalDirectIdentity sound_driver;
    Nexus_V1_SlevSalDirectLevelIdentity
        levels[NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT];
} Nexus_V1_SlevSalAssetDiscoveryReceipt;

/* Testable bounded primitive. Every expected entry must name a canonical
 * direct file and provide its exact MD5; virtual/container identities never
 * participate in this route. */
int nexus_v1_slev_sal_assets_discover_direct_expected(
    const char *data_dir,
    const Nexus_V1_SlevSalExpectedLevel *expected_levels,
    uint32_t expected_count, const char *driver_name, const char *driver_md5,
    Nexus_V1_SlevSalAssetDiscoveryReceipt *out_receipt);

/* Production names and known Track 1 MD5 identities for SLEV00--15,
 * SNDLEV00--15 SAL/MAP, and the global SDDRVS.TSK. */
int nexus_v1_slev_sal_assets_discover_direct(
    const char *data_dir, Nexus_V1_SlevSalAssetDiscoveryReceipt *out_receipt);

/* Rehashes one previously admitted direct identity without retaining its
 * bytes. This detects a missing, replaced, or otherwise stale source before
 * a launcher route consumes the receipt. */
int nexus_v1_slev_sal_direct_identity_still_matches(
    const Nexus_V1_SlevSalDirectIdentity *identity);

/* Revalidates the exact direct SLEV/SAL/MAP triplet and the shared driver for
 * one level. It remains identity-only and never parses any payload. */
int nexus_v1_slev_sal_level_identities_still_match(
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *receipt, uint32_t level_index);

#endif
