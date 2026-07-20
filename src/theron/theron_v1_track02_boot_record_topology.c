#include "theron_v1_track02_boot_record_topology.h"

#include <string.h>

#define THERON_V1_BOOT_RAW_SECTOR_BYTES 2352u
#define THERON_V1_BOOT_MODE1_SYNC_TAIL_OFFSET 11u
#define THERON_V1_BOOT_MODE1_MODE_OFFSET 15u

static int theron_v1_boot_mode1_sector_is_valid(const uint8_t *sector) {
    size_t index;

    if (!sector || sector[0] != 0x00u ||
        sector[THERON_V1_BOOT_MODE1_SYNC_TAIL_OFFSET] != 0x00u ||
        sector[THERON_V1_BOOT_MODE1_MODE_OFFSET] != 0x01u) {
        return 0;
    }
    for (index = 1u; index < THERON_V1_BOOT_MODE1_SYNC_TAIL_OFFSET; ++index) {
        if (sector[index] != 0xffu) return 0;
    }
    return 1;
}

static int theron_v1_boot_mark_span(uint8_t *bits,
                                    uint32_t boot_first,
                                    uint32_t first,
                                    size_t count,
                                    uint32_t boot_last,
                                    size_t *out_named_delta) {
    size_t index;

    for (index = 0u; index < count; ++index) {
        uint32_t sector = first + (uint32_t)index;
        uint32_t slot;

        if (sector < boot_first || sector > boot_last) return 0;
        slot = sector - boot_first;
        if ((bits[slot / 8u] & (uint8_t)(1u << (slot % 8u))) == 0u) {
            bits[slot / 8u] |= (uint8_t)(1u << (slot % 8u));
            if (out_named_delta) ++*out_named_delta;
        }
    }
    return 1;
}

static int theron_v1_boot_verify_span_envelopes(const uint8_t *track02_data,
                                                size_t track02_size,
                                                uint32_t first,
                                                size_t count) {
    size_t index;

    for (index = 0u; index < count; ++index) {
        uint32_t sector = first + (uint32_t)index;
        size_t raw_offset;

        if ((size_t)sector > SIZE_MAX / THERON_V1_BOOT_RAW_SECTOR_BYTES) {
            return 0;
        }
        raw_offset = (size_t)sector * THERON_V1_BOOT_RAW_SECTOR_BYTES;
        if (raw_offset > track02_size ||
            THERON_V1_BOOT_RAW_SECTOR_BYTES > track02_size - raw_offset) {
            return 0;
        }
        if (!theron_v1_boot_mode1_sector_is_valid(track02_data + raw_offset)) {
            return 0;
        }
    }
    return 1;
}

int theron_v1_track02_boot_record_topology_from_chain(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02IplLoaderReceipt *loader,
    const Theron_V1Stage3ManifestEvidence *manifest,
    const Theron_V1Stage3DescriptorCorpusMediaCorrelation *corpus,
    const Theron_V1Stage3DescriptorRecordSpan *span,
    Theron_V1Track02BootRecordTopology *out_topology) {
    size_t index01;
    uint32_t record32;
    size_t index;

    if (!out_topology) return 0;
    memset(out_topology, 0, sizeof(*out_topology));
    if (!track02_data || !loader || !manifest || !corpus || !span ||
        track02_size % THERON_V1_BOOT_RAW_SECTOR_BYTES != 0u ||
        !loader->valid || !manifest->valid || !corpus->valid ||
        !corpus->corpus_media_proven || corpus->descriptor_semantics_proven ||
        !span->valid || !span->span_topology_proven ||
        span->descriptor_semantics_proven ||
        loader->variant != manifest->variant ||
        manifest->variant != corpus->variant ||
        corpus->variant != span->variant) {
        return 0;
    }

    index01 = loader->data_track_index01_raw_sector;
    if (loader->executable_raw_sector !=
            index01 + THERON_TRACK02_IPL_RECORD ||
        loader->stage2_raw_sector !=
            index01 + THERON_TRACK02_IPL_STAGE2_RECORD ||
        (loader->executable_sector_count != 3u &&
         loader->executable_sector_count != 4u) ||
        loader->stage2_sector_count != THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT ||
        loader->stage2_cd_read_record == 0u ||
        loader->stage2_cd_read_raw_sector != loader->stage2_cd_read_record ||
        manifest->track02_record != loader->stage2_cd_read_record ||
        manifest->raw_sector != manifest->track02_record ||
        corpus->stage3_track02_record != manifest->track02_record ||
        span->stage3_track02_record != manifest->track02_record ||
        corpus->derived_record_base != span->derived_record_base ||
        corpus->distinct_record_count != span->referenced_record_count ||
        corpus->min_resolved_record != span->min_referenced_record ||
        corpus->max_resolved_record != span->max_referenced_record ||
        corpus->resolved_record_count != corpus->nonzero_selector_count) {
        return 0;
    }
    if (index01 + THERON_TRACK02_IPL_RECORD > UINT32_MAX ||
        index01 + THERON_TRACK02_IPL_STAGE2_RECORD > UINT32_MAX ||
        index01 + THERON_V1_BOOT_TOPOLOGY_IPL_PRELOAD_RECORD > UINT32_MAX) {
        return 0;
    }

    out_topology->index01_raw_sector = index01;
    out_topology->ipl_executable_first_sector =
        (uint32_t)(index01 + THERON_TRACK02_IPL_RECORD);
    out_topology->ipl_executable_sector_count = loader->executable_sector_count;
    out_topology->ipl_preload_first_sector =
        (uint32_t)(index01 + THERON_V1_BOOT_TOPOLOGY_IPL_PRELOAD_RECORD);
    out_topology->ipl_preload_sector_count =
        THERON_V1_BOOT_TOPOLOGY_IPL_PRELOAD_SECTOR_COUNT;
    out_topology->stage2_first_sector =
        (uint32_t)(index01 + THERON_TRACK02_IPL_STAGE2_RECORD);
    out_topology->stage2_sector_count = loader->stage2_sector_count;
    out_topology->stage3_sector = manifest->track02_record;
    out_topology->corpus_min_record = span->min_referenced_record;
    out_topology->corpus_max_record = span->max_referenced_record;
    out_topology->corpus_referenced_count = span->referenced_record_count;

    /* Every loader-named record must be a well-formed MODE1 sector on the
     * hash-gated media; the corpus set was media-proven upstream. */
    if (!theron_v1_boot_verify_span_envelopes(
            track02_data, track02_size,
            out_topology->ipl_executable_first_sector,
            out_topology->ipl_executable_sector_count) ||
        !theron_v1_boot_verify_span_envelopes(
            track02_data, track02_size,
            out_topology->ipl_preload_first_sector,
            out_topology->ipl_preload_sector_count) ||
        !theron_v1_boot_verify_span_envelopes(
            track02_data, track02_size,
            out_topology->stage2_first_sector,
            out_topology->stage2_sector_count) ||
        !theron_v1_boot_verify_span_envelopes(
            track02_data, track02_size,
            out_topology->stage3_sector, 1u)) {
        memset(out_topology, 0, sizeof(*out_topology));
        return 0;
    }

    out_topology->boot_first_sector = out_topology->ipl_executable_first_sector;
    if (out_topology->corpus_min_record < out_topology->boot_first_sector) {
        out_topology->boot_first_sector = out_topology->corpus_min_record;
    }
    if (out_topology->ipl_preload_first_sector < out_topology->boot_first_sector) {
        out_topology->boot_first_sector = out_topology->ipl_preload_first_sector;
    }
    out_topology->boot_last_sector =
        out_topology->ipl_executable_first_sector +
        (uint32_t)out_topology->ipl_executable_sector_count - 1u;
    record32 = out_topology->ipl_preload_first_sector +
        (uint32_t)out_topology->ipl_preload_sector_count - 1u;
    if (record32 > out_topology->boot_last_sector) {
        out_topology->boot_last_sector = record32;
    }
    record32 = out_topology->stage2_first_sector +
        (uint32_t)out_topology->stage2_sector_count - 1u;
    if (record32 > out_topology->boot_last_sector) {
        out_topology->boot_last_sector = record32;
    }
    if (out_topology->stage3_sector > out_topology->boot_last_sector) {
        out_topology->boot_last_sector = out_topology->stage3_sector;
    }
    if (out_topology->corpus_max_record > out_topology->boot_last_sector) {
        out_topology->boot_last_sector = out_topology->corpus_max_record;
    }
    if (out_topology->boot_last_sector < out_topology->boot_first_sector ||
        out_topology->boot_last_sector - out_topology->boot_first_sector + 1u >
            THERON_V1_BOOT_TOPOLOGY_SLOT_CAPACITY) {
        memset(out_topology, 0, sizeof(*out_topology));
        return 0;
    }
    out_topology->boot_slot_count =
        out_topology->boot_last_sector - out_topology->boot_first_sector + 1u;

    /* Loader spans first (deduplicated), then the corpus referenced set. */
    {
        uint8_t loader_bits[THERON_V1_BOOT_TOPOLOGY_BITMAP_BYTES];

        memset(loader_bits, 0, sizeof(loader_bits));
        if (!theron_v1_boot_mark_span(
                loader_bits, out_topology->boot_first_sector,
                out_topology->ipl_executable_first_sector,
                out_topology->ipl_executable_sector_count,
                out_topology->boot_last_sector,
                &out_topology->loader_named_sector_count) ||
            !theron_v1_boot_mark_span(
                loader_bits, out_topology->boot_first_sector,
                out_topology->ipl_preload_first_sector,
                out_topology->ipl_preload_sector_count,
                out_topology->boot_last_sector,
                &out_topology->loader_named_sector_count) ||
            !theron_v1_boot_mark_span(
                loader_bits, out_topology->boot_first_sector,
                out_topology->stage2_first_sector,
                out_topology->stage2_sector_count,
                out_topology->boot_last_sector,
                &out_topology->loader_named_sector_count) ||
            !theron_v1_boot_mark_span(
                loader_bits, out_topology->boot_first_sector,
                out_topology->stage3_sector, 1u,
                out_topology->boot_last_sector,
                &out_topology->loader_named_sector_count)) {
            memset(out_topology, 0, sizeof(*out_topology));
            return 0;
        }
        memcpy(out_topology->named_slot_bits, loader_bits,
               sizeof(loader_bits));
    }
    out_topology->boot_named_sector_count =
        out_topology->loader_named_sector_count;
    for (index = 0u; index < span->span_record_slots; ++index) {
        uint32_t record = span->min_referenced_record + (uint32_t)index;
        int loader_named;
        uint32_t slot;

        if (!theron_v1_stage3_descriptor_record_span_contains(span, record)) {
            continue;
        }
        if (record < out_topology->boot_first_sector ||
            record > out_topology->boot_last_sector) {
            memset(out_topology, 0, sizeof(*out_topology));
            return 0;
        }
        slot = record - out_topology->boot_first_sector;
        loader_named = (out_topology->named_slot_bits[slot / 8u] &
                        (uint8_t)(1u << (slot % 8u))) != 0u;
        if (loader_named) {
            ++out_topology->loader_corpus_overlap_count;
            if (record == out_topology->stage3_sector) {
                out_topology->stage3_self_record_referenced = 1;
            }
            if (record >= out_topology->stage2_first_sector &&
                record < out_topology->stage2_first_sector +
                    (uint32_t)out_topology->stage2_sector_count) {
                ++out_topology->stage2_corpus_overlap_count;
            }
        } else {
            out_topology->named_slot_bits[slot / 8u] |=
                (uint8_t)(1u << (slot % 8u));
            ++out_topology->boot_named_sector_count;
        }
    }
    if (out_topology->boot_named_sector_count !=
        out_topology->loader_named_sector_count +
            out_topology->corpus_referenced_count -
            out_topology->loader_corpus_overlap_count) {
        memset(out_topology, 0, sizeof(*out_topology));
        return 0;
    }

    out_topology->named_slot_flag_hash = 2166136261u;
    for (index = 0u; index < out_topology->boot_slot_count; ++index) {
        uint8_t flag = (uint8_t)(
            (out_topology->named_slot_bits[index / 8u] >> (index % 8u)) & 1u);

        out_topology->named_slot_flag_hash ^= flag;
        out_topology->named_slot_flag_hash *= 16777619u;
    }
    out_topology->valid = 1;
    out_topology->variant = loader->variant;
    out_topology->boot_topology_proven =
        out_topology->loader_named_sector_count != 0u &&
        out_topology->corpus_referenced_count != 0u &&
        out_topology->boot_named_sector_count != 0u &&
        out_topology->stage3_self_record_referenced &&
        out_topology->named_slot_flag_hash != 0u;
    out_topology->record_semantics_proven = 0;
    if (!out_topology->boot_topology_proven) {
        memset(out_topology, 0, sizeof(*out_topology));
        return 0;
    }
    return 1;
}

int theron_v1_track02_boot_record_topology_contains(
    const Theron_V1Track02BootRecordTopology *topology,
    uint32_t file_sector) {
    uint32_t slot;

    if (!topology || !topology->valid || !topology->boot_topology_proven ||
        topology->boot_slot_count == 0u ||
        topology->boot_slot_count > THERON_V1_BOOT_TOPOLOGY_SLOT_CAPACITY ||
        file_sector < topology->boot_first_sector ||
        file_sector > topology->boot_last_sector) {
        return 0;
    }
    slot = file_sector - topology->boot_first_sector;
    return (topology->named_slot_bits[slot / 8u] >> (slot % 8u)) & 1u;
}
