#include "nexus_v1_bpk_archive.h"

#include <stdint.h>
#include <string.h>

static uint32_t rd32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

static int read_header(const uint8_t *data,
                       size_t data_size,
                       uint32_t *out_count) {
    uint32_t outer_size;
    uint32_t count;

    if (!data || !out_count) return -1;
    if (data_size < 24U) return -1;

    if (rd32_be(data + 0) != NEXUS_V1_BPK_MAGIC_BPPK) return -1;
    outer_size = rd32_be(data + 4);
    if (outer_size != data_size) return -1;
    if (rd32_be(data + 12) != NEXUS_V1_BPK_MAGIC_BMPD) return -1;

    count = rd32_be(data + 20);
    if (count == 0U || count > 4096U) return -1;
    if ((size_t)count > (SIZE_MAX - 24U) / 4U) return -1;
    if (24U + (size_t)count * 4U > data_size) return -1;

    *out_count = count;
    return 0;
}

static int read_candidate_offset(const uint8_t *data,
                                 size_t data_size,
                                 uint32_t count,
                                 uint32_t index,
                                 uint32_t *out_offset) {
    uint32_t offset;

    if (!data || !out_offset || index >= count) return -1;
    offset = rd32_be(data + 24U + (size_t)index * 4U);
    if ((size_t)offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES > data_size) {
        return -1;
    }

    *out_offset = offset;
    return 0;
}

int nexus_v1_bpk_archive_get_entry(const uint8_t *data,
                                   size_t data_size,
                                   uint32_t index,
                                   Nexus_V1_BpkEntry *out_entry) {
    uint32_t count;
    uint32_t offset;
    uint32_t next_offset;
    uint32_t payload_offset;
    int has_prs3;

    if (!out_entry) return -1;
    memset(out_entry, 0, sizeof(*out_entry));

    if (read_header(data, data_size, &count) != 0) return -1;
    if (read_candidate_offset(data, data_size, count, index, &offset) != 0) {
        return -1;
    }

    if (index + 1U < count) {
        if (read_candidate_offset(data, data_size, count, index + 1U,
                                  &next_offset) != 0) {
            return -1;
        }
    } else {
        if (data_size > UINT32_MAX) return -1;
        next_offset = (uint32_t)data_size;
    }

    if (next_offset <= offset) return -1;
    has_prs3 = (rd32_be(data + offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES) ==
                NEXUS_V1_BPK_MAGIC_PRS3);
    payload_offset = offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                     (has_prs3 ? 4U : 0U);
    if (payload_offset > next_offset) return -1;

    out_entry->offset = offset;
    out_entry->next_offset = next_offset;
    out_entry->stored_size = next_offset - offset;
    out_entry->payload_offset = payload_offset;
    out_entry->payload_size = next_offset - payload_offset;
    out_entry->has_prs3 = has_prs3;
    return 0;
}

int nexus_v1_bpk_archive_parse(const uint8_t *data,
                               size_t data_size,
                               Nexus_V1_BpkArchiveInfo *out_info) {
    uint32_t count;
    uint32_t prev = 0U;
    uint32_t prs3 = 0U;
    uint32_t raw = 0U;
    uint32_t first = 0U;
    uint32_t last = 0U;

    if (!out_info) return -1;
    memset(out_info, 0, sizeof(*out_info));

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntry entry;
        uint32_t offset;

        if (read_candidate_offset(data, data_size, count, i, &offset) != 0) {
            return -1;
        }
        if (i > 0U && offset <= prev) return -1;
        if (i == 0U) first = offset;
        last = offset;
        prev = offset;

        if (nexus_v1_bpk_archive_get_entry(data, data_size, i, &entry) != 0) {
            return -1;
        }
        if (entry.has_prs3) {
            ++prs3;
        } else {
            ++raw;
        }
    }

    out_info->outer_size = rd32_be(data + 4);
    out_info->bmpd_size = rd32_be(data + 16);
    out_info->entry_count_hint = count;
    out_info->candidate_offset_count = count;
    out_info->first_candidate_offset = first;
    out_info->last_candidate_offset = last;
    out_info->prs3_payload_count = prs3;
    out_info->raw_payload_count = raw;
    return 0;
}

int nexus_v1_bpk_archive_get_entry_prefix(const uint8_t *data,
                                          size_t data_size,
                                          uint32_t index,
                                          Nexus_V1_BpkEntryPrefix *out_prefix) {
    Nexus_V1_BpkEntry entry;
    uint16_t width;
    uint8_t height;
    uint8_t mode;

    if (!out_prefix) return -1;
    memset(out_prefix, 0, sizeof(*out_prefix));

    if (nexus_v1_bpk_archive_get_entry(data, data_size, index, &entry) != 0) {
        return -1;
    }

    out_prefix->prefix_complete = (entry.stored_size >=
                                   NEXUS_V1_BPK_ENTRY_PREFIX_BYTES);

    /* Even partial entries get the bytes we have; the rest stays zeroed. */
    {
        uint32_t copy = entry.stored_size;
        if (copy > NEXUS_V1_BPK_ENTRY_PREFIX_BYTES) {
            copy = NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;
        }
        if (copy > 0U) {
            memcpy(out_prefix->raw,
                   data + entry.offset,
                   copy);
        }
    }

    width = (uint16_t)(((uint16_t)out_prefix->raw[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET]
                         << 8) |
                        out_prefix->raw[NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET + 1U]);
    height = out_prefix->raw[NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET];
    mode = out_prefix->raw[NEXUS_V1_BPK_PREFIX_MODE_OFFSET];

    out_prefix->width = width;
    out_prefix->height = height;
    out_prefix->mode = mode;
    return 0;
}

int nexus_v1_bpk_archive_inspect_prs3(const uint8_t *data,
                                      size_t data_size,
                                      uint32_t index,
                                      Nexus_V1_BpkPrs3Info *out_info) {
    Nexus_V1_BpkEntry entry;
    Nexus_V1_BpkEntryPrefix prefix;
    uint32_t prs3_off;
    uint32_t prs3_pixel_count;
    uint32_t version;
    uint32_t prefix_pixels;
    uint32_t payload_start;

    if (!out_info) return -1;
    memset(out_info, 0, sizeof(*out_info));

    if (nexus_v1_bpk_archive_get_entry(data, data_size, index, &entry) != 0) {
        return -1;
    }
    if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, index,
                                              &prefix) != 0) {
        return -1;
    }

    out_info->has_prs3 = entry.has_prs3;
    if (!entry.has_prs3) {
        return 0;
    }

    /* Need 20 (prefix) + 12 (PRS3 hdr) = 32 bytes minimum to inspect. */
    if (entry.stored_size <
        NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + NEXUS_V1_BPK_PRS3_HEADER_BYTES) {
        return -1;
    }

    prs3_off = entry.offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES;

    /* PRS3+0 is the magic (already verified by entry.has_prs3 == 1). */
    version = rd32_be(data + prs3_off + 4U);
    out_info->prs3_version_matches = (version == NEXUS_V1_BPK_PRS3_VERSION);

    prs3_pixel_count = rd32_be(data + prs3_off + 8U);
    out_info->prs3_pixel_count = prs3_pixel_count;

    prefix_pixels = (uint32_t)prefix.width * (uint32_t)prefix.height;
    out_info->prefix_pixels = prefix_pixels;
    out_info->pixel_count_matches = (prs3_pixel_count == prefix_pixels);

    payload_start = prs3_off + NEXUS_V1_BPK_PRS3_HEADER_BYTES;
    if (payload_start <= entry.next_offset) {
        out_info->payload_available = 1;
        out_info->compressed_size = entry.next_offset - payload_start;
    }
    return 0;
}

int nexus_v1_bpk_archive_mode_distribution(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkModeDistribution *out_dist) {
    uint32_t count;

    if (!out_dist) return -1;
    memset(out_dist, 0, sizeof(*out_dist));

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntryPrefix prefix;
        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) continue;

        out_dist->mode_count[prefix.mode] += 1U;
        out_dist->total_with_prefix += 1U;
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            out_dist->trailer_index = i;
            out_dist->trailer_found = 1;
        }
    }
    return 0;
}

/* pass1083 — per-entry surface-class contract. Maps the four observed
 * PRS3 pixel-mode tags (6/14/22/30) and the unique directory trailer
 * (10) to a stable enum + bytes-per-pixel value. PRS3 decompression is
 * still intentionally unsupported; this only describes the unpacked
 * surface shape a future decoder would have to fill. */
Nexus_V1_BpkSurfaceClass nexus_v1_bpk_mode_to_surface_class(uint8_t mode) {
    switch (mode) {
    case NEXUS_V1_BPK_MODE_8BPP:     return NEXUS_V1_BPK_SURFACE_INDEXED_8BPP;
    case NEXUS_V1_BPK_MODE_16BPP:    return NEXUS_V1_BPK_SURFACE_RGB565;
    case NEXUS_V1_BPK_MODE_24BPP:    return NEXUS_V1_BPK_SURFACE_RGB888;
    case NEXUS_V1_BPK_MODE_32BPP:    return NEXUS_V1_BPK_SURFACE_RGBA8888;
    case NEXUS_V1_BPK_MODE_TRAILER:  return NEXUS_V1_BPK_SURFACE_DIRECTORY_TRAILER;
    default:                         return NEXUS_V1_BPK_SURFACE_UNKNOWN;
    }
}

uint32_t nexus_v1_bpk_mode_to_bpp(uint8_t mode) {
    switch (mode) {
    case NEXUS_V1_BPK_MODE_8BPP:  return 1U;
    case NEXUS_V1_BPK_MODE_16BPP: return 2U;
    case NEXUS_V1_BPK_MODE_24BPP: return 3U;
    case NEXUS_V1_BPK_MODE_32BPP: return 4U;
    case NEXUS_V1_BPK_MODE_TRAILER:
    default:                      return 0U;
    }
}

int nexus_v1_bpk_archive_surface_estimate(
    const uint8_t *data,
    size_t data_size,
    Nexus_V1_BpkSurfaceEntry *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkSurfaceEstimate *out_summary) {
    uint32_t count;
    uint32_t trailer_skip = 0U;
    uint32_t unknown_skip = 0U;
    uint32_t with_surface = 0U;
    uint32_t used = 0U;
    uint32_t truncated = 0U;
    uint64_t total_bytes = 0U;

    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(*out_summary));
    out_summary->capacity = entry_capacity;

    if (read_header(data, data_size, &count) != 0) return -1;

    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntryPrefix prefix;
        uint32_t bpp;
        uint32_t rowstride;
        uint32_t surface_bytes;

        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) {
            /* Skip partial entries; they have no usable surface layout. */
            continue;
        }
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            ++trailer_skip;
            continue;
        }
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) {
            ++unknown_skip;
            continue;
        }
        rowstride = (uint32_t)prefix.width * bpp;
        surface_bytes = rowstride * (uint32_t)prefix.height;

        if (out_entries && used < entry_capacity) {
            Nexus_V1_BpkSurfaceEntry *row = &out_entries[used];
            row->entry_index = i;
            row->mode = prefix.mode;
            row->width = prefix.width;
            row->height = prefix.height;
            row->pixel_count =
                (uint32_t)prefix.width * (uint32_t)prefix.height;
            row->layout.bpp = bpp;
            row->layout.rowstride = rowstride;
            row->layout.surface_bytes = surface_bytes;
            row->layout.surface_class =
                nexus_v1_bpk_mode_to_surface_class(prefix.mode);
        } else if (out_entries && used >= entry_capacity) {
            truncated = 1U;
        }
        ++used;
        ++with_surface;
        total_bytes += surface_bytes;
    }

    out_summary->used = used;
    out_summary->total_with_surface = with_surface;
    out_summary->total_surface_bytes = total_bytes;
    out_summary->trailer_skipped = trailer_skip;
    out_summary->unknown_skipped = unknown_skip;
    out_summary->truncated = (int)truncated;
    return 0;
}

/* pass1084 — bounded PRS3 compression evidence walker. Surfaces per-entry
 * structural receipts for the (still unknown) PRS3 stream format so we can
 * narrow down the algorithm family in subsequent passes. The walker is
 * intentionally incapable of "running" the compression: it reads the first
 * 4 bytes as a BE u32, copies the first 8 bytes verbatim, and tallies a
 * bounded frequency table. It never advances any conceptual read cursor
 * past the 8-byte receipt and never emits a decoded surface.
 *
 * pass1084b (later in the same walk) adds a bounded 4-quadrant byte-class
 * tally (bk >> 6) over the same sample. This is the cheapest structural
 * fingerprint that distinguishes "random / uniformly distributed" bytes
 * from "stream with length-class hints and repeated literal zeros", and
 * the real MENU.BPK distribution shows q0 (0x00..0x3F) dominating, which
 * is consistent with small header_minus_payload values (4..7) and the
 * short back-reference runs we hypothesise.
 *
 * Source-lock: this pass builds on the pass1082/pass1083 evidence
 *   ledger (the BPPK outer wrapper, BMPD directory, 20-byte entry prefix
 *   anchor, and the entry[0] directory-trailer contract). The algorithm
 *   itself is still cited as
 *     docs/source-lock/nexus_v1_phase0_provenance_gate_H2315.md:291-306
 *     (MENU.BPK packed, game-specific, no formal analysis documented)
 *   and
 *     docs/VERIFIED_HASHES.md:103
 *     (MENU.BPK size 89060 / sha256 ...da886636).
 */
int nexus_v1_bpk_archive_prs3_payload_evidence(
    const uint8_t *data,
    size_t data_size,
    uint32_t sample_size,
    Nexus_V1_BpkPrs3PayloadEvidence *out_entries,
    uint32_t entry_capacity,
    Nexus_V1_BpkPrs3PayloadEvidenceSummary *out_summary) {
    uint32_t count;
    uint32_t trailer_skip = 0U;
    uint32_t unknown_skip = 0U;
    uint32_t trailer_partial = 0U;
    uint32_t used = 0U;
    uint32_t truncated = 0U;
    uint64_t total_uncompressed = 0U;
    uint64_t total_payload = 0U;
    double sum_ratio = 0.0;
    double min_ratio = 0.0;
    double max_ratio = 0.0;
    int have_ratio = 0;
    uint32_t smallest_payload = UINT32_MAX;
    uint32_t largest_payload = 0U;

    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(*out_summary));
    out_summary->capacity = entry_capacity;
    out_summary->smallest_payload = UINT32_MAX;
    out_summary->largest_payload = 0U;
    out_summary->min_compression_ratio = 0.0;
    out_summary->max_compression_ratio = 0.0;

    if (sample_size > NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_SAMPLE_BYTES) {
        sample_size = NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_SAMPLE_BYTES;
    }

    if (read_header(data, data_size, &count) != 0) return -1;

    /* Pass 1: per-entry structural receipts.
     *
     * For every entry whose 20-byte prefix is complete AND whose prefix
     * mode is one of the four PRS3 pixel-mode tags (6/14/22/30), we
     * record (entry_index, dimensions, header_first_u32, first 8 bytes
     * verbatim, compression_ratio). The directory trailer (mode 10) and
     * unknown mode tags are skipped and counted so callers can verify
     * the 162 + 1 = 163 entry sum.
     *
     * Note on payload boundary: the lib's Nexus_V1_BpkEntry.payload_offset
     * starts immediately after the 4-byte PRS3 magic (entry.offset + 24),
     * which still includes the 8-byte PRS3 sub-header (version +
     * pixel_count). For the PRS3 algorithm-evidence receipt we want to
     * examine the bytes AFTER the full 12-byte PRS3 sub-header (i.e.
     * entry.offset + 32, matching inspect_prs3's `compressed_size`
     * measurement). All receipts below use that offset consistently. */
    for (uint32_t i = 0; i < count; ++i) {
        Nexus_V1_BpkEntryPrefix prefix;
        Nexus_V1_BpkEntry entry;
        uint32_t bpp;
        uint32_t uncompressed_size;
        uint32_t payload_size;
        uint32_t stream_offset;
        uint32_t copy;
        int has_first;
        uint32_t header_first = 0U;
        uint32_t header_minus = 0U;
        double r = 0.0;
        Nexus_V1_BpkPrs3PayloadEvidence *row = NULL;

        if (nexus_v1_bpk_archive_get_entry_prefix(data, data_size, i,
                                                  &prefix) != 0) {
            return -1;
        }
        if (!prefix.prefix_complete) {
            /* Skip entries without the full 20-byte prefix; they have no
             * recognisable mode tag and could not produce evidence. */
            continue;
        }
        if (prefix.mode == NEXUS_V1_BPK_MODE_TRAILER) {
            ++trailer_skip;
            continue;
        }
        bpp = nexus_v1_bpk_mode_to_bpp(prefix.mode);
        if (bpp == 0U) {
            ++unknown_skip;
            continue;
        }
        if (nexus_v1_bpk_archive_get_entry(data, data_size, i, &entry) != 0) {
            return -1;
        }

        /* Pass1084 evidence uses the inspect_prs3 `compressed_size`
         * boundary: payload is bytes from (entry.offset + 32) until the
         * next entry's start. If the entry doesn't span past the full
         * 12-byte PRS3 sub-header the stream is "partial" and we still
         * surface payload_size == 0 without producing a negative count. */
        stream_offset = entry.offset + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                        NEXUS_V1_BPK_PRS3_HEADER_BYTES;
        if (stream_offset <= entry.next_offset) {
            payload_size = entry.next_offset - stream_offset;
        } else {
            payload_size = 0U;
        }
        uncompressed_size = (uint32_t)prefix.width *
                            (uint32_t)prefix.height *
                            bpp;

        has_first = (payload_size >= 4U) ? 1 : 0;
        if (has_first) {
            header_first = rd32_be(data + stream_offset);
            header_minus = (header_first >= payload_size)
                               ? (header_first - payload_size)
                               : UINT32_MAX;
        }

        if (out_entries && used < entry_capacity) {
            row = &out_entries[used];
            memset(row, 0, sizeof(*row));
            row->entry_index = i;
            row->mode = prefix.mode;
            row->width = prefix.width;
            row->height = prefix.height;
            row->pixel_count = (uint32_t)prefix.width *
                               (uint32_t)prefix.height;
            row->bpp = bpp;
            row->uncompressed_size = uncompressed_size;
            row->payload_size = payload_size;
            row->payload_available = (payload_size > 0U) ? 1 : 0;
            row->header_first_u32 = header_first;
            row->header_first_readable = has_first;
            row->header_minus_payload = header_minus;
            if (uncompressed_size > 0U) {
                row->compression_ratio =
                    (double)payload_size / (double)uncompressed_size;
                r = row->compression_ratio;
            }
            if (payload_size > 0U) {
                copy = payload_size;
                if (copy > NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_FIRST_BYTES) {
                    copy = NEXUS_V1_BPK_PRS3_EVIDENCE_MAX_FIRST_BYTES;
                }
                memcpy(row->first_payload,
                       data + stream_offset,
                       copy);
            }
        } else if (out_entries && used >= entry_capacity) {
            truncated = 1U;
        }

        if (uncompressed_size > 0U) {
            sum_ratio += r;
            if (!have_ratio) {
                min_ratio = r;
                max_ratio = r;
                have_ratio = 1;
            } else {
                if (r < min_ratio) min_ratio = r;
                if (r > max_ratio) max_ratio = r;
            }
        }

        ++used;
        ++out_summary->mode_count[prefix.mode];
        total_uncompressed += uncompressed_size;
        total_payload += payload_size;
        if (payload_size < smallest_payload) smallest_payload = payload_size;
        if (payload_size > largest_payload) largest_payload = payload_size;
        if (payload_size == 0U) ++trailer_partial;
    }

    out_summary->entries_seen = count;
    out_summary->trailer_skipped = trailer_skip;
    out_summary->unknown_skipped = unknown_skip;
    out_summary->trailer_partial = trailer_partial;
    out_summary->used = used;
    out_summary->smallest_payload = smallest_payload;
    out_summary->largest_payload = largest_payload;
    out_summary->total_uncompressed = total_uncompressed;
    out_summary->total_payload = total_payload;
    out_summary->mean_compression_ratio =
        (used > 0U) ? (sum_ratio / (double)used) : 0.0;
    out_summary->min_compression_ratio = have_ratio ? min_ratio : 0.0;
    out_summary->max_compression_ratio = have_ratio ? max_ratio : 0.0;
    out_summary->truncated = (int)truncated;

    /* Pass 2: bounded byte-frequency receipts for every row the caller
     * actually got back. Each sample is bounded to min(sample_size,
     * payload_size, MAX_SAMPLE_BYTES) so the walker is provably
     * O(sample_size * entry_capacity) and never reads past
     * MAX_SAMPLE_BYTES of any payload. We re-derive the archive
     * absolute offset of the payload via nexus_v1_bpk_archive_get_entry
     * so the inner loop is bounds-checked against data_size on every
     * byte. The byte window starts at entry.offset + 32 (pass1084
     * stream offset = after the 12-byte PRS3 sub-header), NOT at
     * entry.payload_offset (which is +24 in the lib's accounting). */
    if (out_entries && sample_size > 0U) {
        for (uint32_t i = 0; i < used && i < entry_capacity; ++i) {
            Nexus_V1_BpkPrs3PayloadEvidence *row = &out_entries[i];
            Nexus_V1_BpkEntry entry;
            uint32_t sample_n;
            uint32_t sample_offset;
            uint32_t histogram[256];
            uint32_t best_count = 0U;
            uint8_t best_byte = 0U;
            uint32_t distinct = 0U;
            uint32_t b;

            if (row->payload_size == 0U) continue;
            if (nexus_v1_bpk_archive_get_entry(data, data_size,
                                               row->entry_index,
                                               &entry) != 0) {
                continue;
            }
            sample_offset = entry.offset +
                           NEXUS_V1_BPK_ENTRY_PREFIX_BYTES +
                           NEXUS_V1_BPK_PRS3_HEADER_BYTES;
            sample_n = row->payload_size;
            if (sample_n > sample_size) sample_n = sample_size;

            memset(histogram, 0, sizeof(histogram));
            {
                /* pass1084b — bounded 4-quadrant byte-class tally over
                 * the same sample. Index = bk >> 6 maps each byte into
                 * [0..3] covering [0x00..0x3F], [0x40..0x7F],
                 * [0x80..0xBF], [0xC0..0xFF]. */
                uint32_t bc[4] = {0U, 0U, 0U, 0U};
                for (uint32_t k = 0; k < sample_n; ++k) {
                    uint8_t bk = data[sample_offset + k];
                    histogram[bk] += 1U;
                    bc[bk >> 6] += 1U;
                    if (histogram[bk] > best_count) {
                        best_count = histogram[bk];
                        best_byte = bk;
                    }
                }
                row->byte_class_count[0] = bc[0];
                row->byte_class_count[1] = bc[1];
                row->byte_class_count[2] = bc[2];
                row->byte_class_count[3] = bc[3];
            }
            for (b = 0; b < 256U; ++b) {
                if (histogram[b] != 0U) ++distinct;
            }
            row->sample_size_used = sample_n;
            row->most_common_byte = best_byte;
            row->most_common_byte_count = best_count;
            row->distinct_byte_values = distinct;
        }
    }

    return 0;
}
