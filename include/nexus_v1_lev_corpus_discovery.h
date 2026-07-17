#ifndef NEXUS_V1_LEV_CORPUS_DISCOVERY_H
#define NEXUS_V1_LEV_CORPUS_DISCOVERY_H

#include "nexus_v1_engine.h"
#include "asset_find_by_hash.h"

#include <stdint.h>

#define NEXUS_V1_LEV_CORPUS_LEVEL_COUNT 16U

/* Discovery keeps only direct-file identity. It never retains a LEV payload
 * and virtual archive/ISO paths are outside this startup/capture route. */
typedef struct {
    uint32_t level_index;
    const char *expected_md5;
} Nexus_V1_LevCorpusExpectedLevel;

typedef struct {
    int valid;
    uint32_t level_index;
    char direct_path[ASSET_PATH_MAX];
    uint64_t byte_count;
    uint64_t fnv1a64;
    char md5[33];
} Nexus_V1_LevCorpusDirectLevelIdentity;

typedef struct {
    int valid;
    int skipped_missing_corpus;
    int direct_files_only;
    int payload_materialized;
    uint64_t corpus_fnv1a64;
    uint64_t corpus_byte_count;
    Nexus_V1_LevCorpusDirectLevelIdentity
        levels[NEXUS_V1_LEV_CORPUS_LEVEL_COUNT];
} Nexus_V1_LevCorpusDiscoveryReceipt;

/* Testable bounded primitive. `expected_count` must be exactly 16 with one
 * expected MD5 per level. It only accepts ordinary files returned by the
 * direct-file scanner and rejects missing, duplicate, or cross-level bytes. */
int nexus_v1_lev_corpus_discover_direct_expected(
    const char *data_dir, const Nexus_V1_LevCorpusExpectedLevel *expected,
    uint32_t expected_count, Nexus_V1_LevCorpusDiscoveryReceipt *out_receipt);

/* Production convenience route for the known Nexus LEV00.DGN--LEV15.DGN
 * identities. It is suitable for M12/runtime capture preparation only. */
int nexus_v1_lev_corpus_discover_direct(
    const char *data_dir, Nexus_V1_LevCorpusDiscoveryReceipt *out_receipt);

/* Rehash one retained direct-file identity without retaining its payload.
 * Virtual/container paths, non-regular files, MD5 drift, size drift, and FNV
 * drift reject before an active DGN route can consume the discovery receipt. */
int nexus_v1_lev_corpus_direct_identity_still_matches(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity);

/* Read parser-observed container-header and counted Structure1F descriptor
 * spans from already loaded direct LEV bytes. Identity is compared to the
 * supplied source only; callers bind the ordinary file rehash separately. */
int nexus_v1_lev_corpus_build_header_descriptor_provenance(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    Nexus_V1_DgnDirectLevHeaderDescriptorProvenance *out_provenance);

/* Bind a previously discovered direct identity to an already-loaded active
 * DGN buffer. The input bytes remain caller-owned; this only installs the
 * verified source envelope used by existing no-draw Structure1F/2/3 routes. */
int nexus_v1_lev_corpus_bind_active_level_no_draw(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *receipt,
    uint32_t level_index, const uint8_t *active_dgn, int active_dgn_size);

/* M12/M11 dungeon admission wrapper. It binds the selected direct corpus row
 * and a parser-proven 1F/2/3 no-draw receipt to one monotonic route epoch. */
int nexus_v1_lev_corpus_admit_m11_dungeon_no_draw(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *receipt,
    uint64_t route_epoch, uint32_t level_index,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry);

#endif
