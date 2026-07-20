#ifndef THERON_V1_TRACK02_BOOT_RECORD_TOPOLOGY_H
#define THERON_V1_TRACK02_BOOT_RECORD_TOPOLOGY_H

#include "theron_v1_later_record_correlation.h"
#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Boot-chain record topology for the authenticated Track 02 loader chain.
 *
 * The boot chain names records in two coordinate frames: the IPL-family
 * spans (IPL executable, preload, stage-two executable) are indexed by the
 * data track's INDEX 01 raw sector, while the stage-three manifest record
 * and the descriptor corpus resolve to file-relative raw sectors of the
 * Track 02 BIN.  This binder anchors every named span into the single
 * file-relative frame, re-verifies each loader-named record's MODE1
 * envelope against the hash-gated media, and joins the spans with the
 * proven descriptor referenced-record set into one membership bitmap.
 *
 * The receipt proves record-span topology only: span coordinates, counts,
 * overlaps, gaps, and membership.  It never assigns a record a level,
 * object, tile, palette, bitmap, code, or command meaning, and it does not
 * prove that an unnamed record is unreachable through any other path.
 */

/* The IPL preload table names record 0x3e3 with a two-sector read
 * (source-locked in theron_v1_stage2_runtime_handoff.c; the table bytes
 * themselves are bound by the static IPL read-window proofs). */
#define THERON_V1_BOOT_TOPOLOGY_IPL_PRELOAD_RECORD 0x0003e3u
#define THERON_V1_BOOT_TOPOLOGY_IPL_PRELOAD_SECTOR_COUNT 2u

/* Fail-closed capacity for the joined membership bitmap.  A corrupt or
 * hostile input could otherwise force an unbounded slot span. */
#define THERON_V1_BOOT_TOPOLOGY_SLOT_CAPACITY 4096u
#define THERON_V1_BOOT_TOPOLOGY_BITMAP_BYTES \
    (THERON_V1_BOOT_TOPOLOGY_SLOT_CAPACITY / 8u)

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    /* Frame anchor: the data track INDEX 01 raw sector every IPL-family
     * record is relative to (225 on the authenticated US media, 224 JP). */
    size_t index01_raw_sector;
    /* Loader-named spans, anchored to the file-relative frame. */
    uint32_t ipl_executable_first_sector;
    size_t ipl_executable_sector_count;
    uint32_t ipl_preload_first_sector;
    size_t ipl_preload_sector_count;
    uint32_t stage2_first_sector;
    size_t stage2_sector_count;
    uint32_t stage3_sector;
    /* Joined descriptor corpus facts (already media-proven upstream). */
    uint32_t corpus_min_record;
    uint32_t corpus_max_record;
    size_t corpus_referenced_count;
    /* Joined topology over [boot_first_sector, boot_last_sector]. */
    uint32_t boot_first_sector;
    uint32_t boot_last_sector;
    size_t boot_slot_count;
    /* Distinct sectors named by the four loader spans (deduplicated). */
    size_t loader_named_sector_count;
    /* Distinct sectors named by any source (loader spans + corpus set). */
    size_t boot_named_sector_count;
    /* Loader-named sectors also referenced by the descriptor corpus.  On
     * the authenticated US media this proves that the stage-three table
     * references the stage-two executable's final two sectors and the
     * stage-three manifest's own record; it is a coordinate overlap fact
     * only, never a self-modification or code/data role claim. */
    size_t loader_corpus_overlap_count;
    size_t stage2_corpus_overlap_count;
    int stage3_self_record_referenced;
    /* FNV-1a over one 0/1 flag byte per boot span slot, first..last. */
    uint32_t named_slot_flag_hash;
    uint8_t named_slot_bits[THERON_V1_BOOT_TOPOLOGY_BITMAP_BYTES];
    int boot_topology_proven;
    int record_semantics_proven;
} Theron_V1Track02BootRecordTopology;

/* Builds the boot-chain record topology from a fully authenticated chain:
 * hash-gated Track 02 bytes, the IPL loader receipt, the stage-three
 * manifest evidence, the proven descriptor corpus, and the proven
 * referenced-record span.  Every input must be valid and agree on variant,
 * stage-three record, and frame anchor; every loader-named record must be a
 * well-formed MODE1 sector on the media.  Any disagreement, malformed
 * envelope, over-capacity span, or unproven upstream receipt fails closed
 * with a zeroed topology. */
int theron_v1_track02_boot_record_topology_from_chain(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02IplLoaderReceipt *loader,
    const Theron_V1Stage3ManifestEvidence *manifest,
    const Theron_V1Stage3DescriptorCorpusMediaCorrelation *corpus,
    const Theron_V1Stage3DescriptorRecordSpan *span,
    Theron_V1Track02BootRecordTopology *out_topology);

/* Membership query over a proven topology: returns 1 only when the
 * file-relative sector is named by the boot chain (any loader span or the
 * descriptor referenced set), 0 for unnamed slots, out-of-span sectors,
 * and invalid topologies. */
int theron_v1_track02_boot_record_topology_contains(
    const Theron_V1Track02BootRecordTopology *topology,
    uint32_t file_sector);

#endif /* THERON_V1_TRACK02_BOOT_RECORD_TOPOLOGY_H */
