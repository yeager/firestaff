/*
 * theron_v1_track02.c -- narrow Track 02 evidence helpers.
 *
 * This is not a Theron's Quest dungeon loader.  It locks one small,
 * hash-gated Track 02 bank-descriptor signal so later dungeon-bank work
 * can build from bytes that are regression-proved against real data.
 *
 * Source/evidence:
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md sections 1.3,
 *   1.5, and 4.2 record Track 02 provenance and state that the dungeon
 *   format remains unknown.  The offsets below are from local byte
 *   inspection of the hash-verified JP/US raw Track 02 BINs and the derived
 *   US Track 02 ISO, not from ReDMCSB.
 */

#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

#define TQR_US_ISO_BANK_STRIDE_OFFSET 0x1584u
#define TQR_US_ISO_BANK_STRIDE_COUNT  9u
#define TQR_US_ISO_BANK_STRIDE_BYTES  (TQR_US_ISO_BANK_STRIDE_COUNT * 2u)
#define TQR_US_ISO_BANK_STRIDE_STEP   0x0400u
#define TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR 5u
#define TQR_US_ISO_BANK_BOUNDARY_OFFSET 0x3000u
#define TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES 16u
#define TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES 44u
#define TQR_RAW_SECTOR_BYTES THERON_TRACK02_RAW_SECTOR_BYTES
#define TQR_RAW_SECTOR_USER_DATA_OFFSET THERON_TRACK02_RAW_USER_DATA_OFFSET
#define TQR_RAW_SECTOR_USER_DATA_BYTES THERON_TRACK02_RAW_USER_DATA_BYTES
#define TQR_RAW_BIN_BANK_ANCHOR_COUNT 3u
#define TQR_RAW_INITIAL_LEVEL_WIDTH 32u
#define TQR_RAW_INITIAL_LEVEL_HEIGHT 27u
#define TQR_RAW_INITIAL_LEVEL_SEED 0x0108e938u
#define TQR_RAW_INITIAL_LEVEL_INDEX 0x0026u
#define TQR_RAW_INITIAL_LEVEL_DESCRIPTOR_DELTA \
    (TQR_US_ISO_BANK_STRIDE_OFFSET + 0x92ceu)

/* Audio-bank marker fingerprint (raw Track 02 BIN only).
 *
 * The 16-byte prefix immediately preceding each post-boundary span is
 * exactly:
 *   bytes 0..11   = 0x00, 0xff*10, 0x00  (12-byte sentinel)
 *   bytes 12..15  = a 4-byte little-endian audio-bank id word
 *
 * The sentinel prefix itself appears in many places (sector-index table),
 * but the (sentinel + 4-byte LE word + 44-byte post-boundary span) tuple
 * has been observed to occur exactly once per anchor in raw US and JP
 * Track 02 BINs at the offsets documented in
 * theron_v1_track02_source_evidence().  The 4-byte LE word therefore
 * acts as a per-anchor audio-bank-id marker.
 *
 * Source/evidence: docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 * §10.2 marks the ADPCM data block location as STUB; this marker is one
 * ADPCM-bank anchor candidate.  Bytes inspected locally from
 *   THERON_TRACK02_MD5_US_BIN  (f23601102138f87c33025877767ebf76)
 *   THERON_TRACK02_MD5_JP_BIN  (b7afb338ad31be1025b53f9aff12d73a)
 * via a 2352-byte CD-sector pointer scan.
 */
#define TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES 12u
#define TQR_RAW_BIN_AUDIO_BANK_ID_BYTES      4u

static const uint8_t g_us_iso_bank_stride_descriptor[TQR_US_ISO_BANK_STRIDE_BYTES] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static const size_t g_us_bin_descriptor_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x70be06u, 0x70e2c6u, 0x710904u
};

static const size_t g_jp_bin_descriptor_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x70b4d6u, 0x70d996u, 0x70ffd4u
};

static const size_t g_us_bin_post_boundary_span_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x2d53e0u, 0x47d040u, 0x712840u
};

static const size_t g_jp_bin_post_boundary_span_offsets[TQR_RAW_BIN_BANK_ANCHOR_COUNT] = {
    0x2d4ab0u, 0x47c710u, 0x711f10u
};

static const uint8_t g_us_iso_bank_boundary_prefix[TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80
};

static const uint8_t g_us_iso_post_boundary_span[TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES] = {
    0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
    0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
    0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
    0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
    0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
    0x93, 0x80, 0x00, 0x3f
};

/* 12-byte sentinel that immediately precedes the 4-byte audio-bank id word
 * (and therefore the post-boundary span) at every audio-bank anchor in raw
 * US/JP Track 02 BINs.  See TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES. */
static const uint8_t g_audio_bank_prefix[TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES] = {
    0x00,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0x00
};

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint32_t tqr_hash_bytes(const uint8_t *bytes, size_t byte_count);

/* Read and validate the audio-bank marker at one raw-BIN anchor.
 *
 * anchor_index selects which of (descriptor_offsets, span_offsets) to use;
 * the function verifies that the 16-byte audio-bank prefix
 * (12-byte sentinel + 4-byte LE word) immediately precedes the
 * post-boundary span at that anchor.
 *
 * Returns 1 on success (out_* populated), 0 otherwise.  Out-args are
 * always zeroed on failure so callers can rely on default-zero state. */
static int read_audio_bank_marker(const uint8_t *track02_data,
                                  size_t track02_size,
                                  const size_t *span_offsets,
                                  size_t anchor_index,
                                  uint32_t *out_audio_bank_id,
                                  size_t *out_audio_bank_id_offset,
                                  size_t *out_audio_bank_prefix_offset) {
    const size_t prefix_total_bytes =
        TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES + TQR_RAW_BIN_AUDIO_BANK_ID_BYTES;
    size_t span_offset;
    size_t prefix_offset;
    size_t id_offset;

    if (out_audio_bank_id) *out_audio_bank_id = 0u;
    if (out_audio_bank_id_offset) *out_audio_bank_id_offset = 0u;
    if (out_audio_bank_prefix_offset) *out_audio_bank_prefix_offset = 0u;

    if (!track02_data || !span_offsets ||
        anchor_index >= TQR_RAW_BIN_BANK_ANCHOR_COUNT ||
        prefix_total_bytes > track02_size) {
        return 0;
    }

    span_offset = span_offsets[anchor_index];
    if (span_offset < prefix_total_bytes ||
        span_offset > track02_size ||
        TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES >
            track02_size - span_offset) {
        return 0;
    }

    id_offset = span_offset - TQR_RAW_BIN_AUDIO_BANK_ID_BYTES;
    prefix_offset = id_offset - TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES;

    if (memcmp(track02_data + prefix_offset,
               g_audio_bank_prefix,
               TQR_RAW_BIN_AUDIO_BANK_PREFIX_BYTES) != 0) {
        return 0;
    }

    if (out_audio_bank_id) *out_audio_bank_id = rd32le(track02_data + id_offset);
    if (out_audio_bank_id_offset) *out_audio_bank_id_offset = id_offset;
    if (out_audio_bank_prefix_offset) *out_audio_bank_prefix_offset = prefix_offset;
    return 1;
}

Theron_Track02Variant theron_v1_track02_variant_for_md5(const char *md5_hex) {
    if (!md5_hex) return THERON_TRACK02_VARIANT_UNKNOWN;
    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) == 0) {
        return THERON_TRACK02_VARIANT_US_BIN;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_JP_BIN) == 0) {
        return THERON_TRACK02_VARIANT_JP_BIN;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_ISO) == 0) {
        return THERON_TRACK02_VARIANT_US_ISO;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_JP_REV1_ISO) == 0) {
        return THERON_TRACK02_VARIANT_JP_REV1_ISO;
    }
    return THERON_TRACK02_VARIANT_UNKNOWN;
}

static int track_is_all_zero(const uint8_t *data, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        if (data[i] != 0) return 0;
    }
    return 1;
}

static int variant_is_raw_bin(Theron_Track02Variant variant) {
    return variant == THERON_TRACK02_VARIANT_US_BIN ||
           variant == THERON_TRACK02_VARIANT_JP_BIN;
}

static int variant_has_plain_user_data(Theron_Track02Variant variant) {
    return variant == THERON_TRACK02_VARIANT_US_ISO;
}

static int tqr_ascii_equal_ci(const char *a, const char *b) {
    unsigned char ca;
    unsigned char cb;
    if (!a || !b) return 0;
    do {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb + ('a' - 'A'));
        if (ca != cb) return 0;
    } while (ca != '\0');
    return 1;
}

static int tqr_path_is_cue(const char *path) {
    const char *dot;
    if (!path) return 0;
    dot = strrchr(path, '.');
    return dot && tqr_ascii_equal_ci(dot, ".cue");
}

static const char *tqr_skip_space(const char *text) {
    while (text && (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n')) {
        ++text;
    }
    return text;
}

static int tqr_cue_file_line(const char *line, char *out_name, size_t out_cap) {
    const char *p = tqr_skip_space(line);
    const char *end;
    size_t len;
    if (!p || strncmp(p, "FILE", 4u) != 0 || (p[4] != ' ' && p[4] != '\t')) return 0;
    p = tqr_skip_space(p + 4u);
    if (*p != '"') return 0;
    ++p;
    end = strchr(p, '"');
    if (!end || end == p) return 0;
    len = (size_t)(end - p);
    if (len >= out_cap) return 0;
    memcpy(out_name, p, len);
    out_name[len] = '\0';
    p = tqr_skip_space(end + 1u);
    return strncmp(p, "BINARY", 6u) == 0 &&
           (p[6] == '\0' || p[6] == ' ' || p[6] == '\t' || p[6] == '\r' || p[6] == '\n');
}

static int tqr_cue_is_track02_mode1(const char *line) {
    const char *p = tqr_skip_space(line);
    if (!p || strncmp(p, "TRACK", 5u) != 0 || (p[5] != ' ' && p[5] != '\t')) return 0;
    p = tqr_skip_space(p + 5u);
    if (strncmp(p, "02", 2u) != 0 || (p[2] != ' ' && p[2] != '\t')) return 0;
    p = tqr_skip_space(p + 2u);
    return tqr_ascii_equal_ci(p, "MODE1/2352") ||
           (strncmp(p, "MODE1/2352", 10u) == 0 &&
            (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n'));
}

Theron_Track02SignalStatus theron_v1_track02_resolve_media_path(
    const char *media_path,
    char out_payload_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    FILE *cue;
    FILE *payload;
    char line[2048];
    char current_file[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char selected_file[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char parent[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    const char *slash;
    size_t parent_len;
    size_t selected_count = 0u;

    if (!out_payload_path) return THERON_TRACK02_SIGNAL_BAD_INPUT;
    out_payload_path[0] = '\0';
    if (!media_path || !media_path[0]) return THERON_TRACK02_SIGNAL_BAD_INPUT;
    if (!tqr_path_is_cue(media_path)) {
        if (strlen(media_path) >= THERON_TRACK02_MOUNT_PATH_CAPACITY) {
            return THERON_TRACK02_SIGNAL_BAD_INPUT;
        }
        memcpy(out_payload_path, media_path, strlen(media_path) + 1u);
        return THERON_TRACK02_SIGNAL_OK;
    }
    cue = fopen(media_path, "rb");
    if (!cue) return THERON_TRACK02_SIGNAL_NOT_FOUND;
    current_file[0] = '\0';
    selected_file[0] = '\0';
    while (fgets(line, sizeof(line), cue) != NULL) {
        char parsed[THERON_TRACK02_MOUNT_PATH_CAPACITY];
        if (tqr_cue_file_line(line, parsed, sizeof(parsed))) {
            memcpy(current_file, parsed, strlen(parsed) + 1u);
        } else if (tqr_cue_is_track02_mode1(line) && current_file[0]) {
            ++selected_count;
            if (selected_count == 1u) {
                memcpy(selected_file, current_file, strlen(current_file) + 1u);
            }
        }
    }
    fclose(cue);
    if (selected_count != 1u || !selected_file[0]) return THERON_TRACK02_SIGNAL_NOT_FOUND;
    slash = strrchr(media_path, '/');
    if (!slash) slash = strrchr(media_path, '\\');
    parent_len = slash ? (size_t)(slash - media_path + 1u) : 0u;
    if (parent_len >= sizeof(parent) ||
        parent_len + strlen(selected_file) >= THERON_TRACK02_MOUNT_PATH_CAPACITY) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (parent_len) memcpy(parent, media_path, parent_len);
    parent[parent_len] = '\0';
    snprintf(out_payload_path, THERON_TRACK02_MOUNT_PATH_CAPACITY, "%s%s", parent, selected_file);
    payload = fopen(out_payload_path, "rb");
    if (!payload) {
        out_payload_path[0] = '\0';
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    fclose(payload);
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_raw_user_data_size(
    size_t track02_size,
    const char *md5_hex,
    size_t *out_sector_count,
    size_t *out_user_data_size) {

    Theron_Track02Variant variant = theron_v1_track02_variant_for_md5(md5_hex);
    size_t sector_count;

    if (out_sector_count) *out_sector_count = 0u;
    if (out_user_data_size) *out_user_data_size = 0u;
    if (track02_size == 0u || !out_sector_count || !out_user_data_size) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (!variant_is_raw_bin(variant)) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }
    if ((track02_size % TQR_RAW_SECTOR_BYTES) != 0u) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    sector_count = track02_size / TQR_RAW_SECTOR_BYTES;
    *out_sector_count = sector_count;
    *out_user_data_size = sector_count * TQR_RAW_SECTOR_USER_DATA_BYTES;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_raw_offset_to_user_offset(
    size_t raw_offset,
    size_t track02_size,
    const char *md5_hex,
    size_t *out_user_offset) {

    size_t sector_count = 0u;
    size_t user_data_size = 0u;
    size_t sector;
    size_t within_sector;
    Theron_Track02SignalStatus status;

    if (out_user_offset) *out_user_offset = 0u;
    if (!out_user_offset) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    status = theron_v1_track02_raw_user_data_size(track02_size,
                                                  md5_hex,
                                                  &sector_count,
                                                  &user_data_size);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }
    if (raw_offset >= track02_size) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    sector = raw_offset / TQR_RAW_SECTOR_BYTES;
    within_sector = raw_offset % TQR_RAW_SECTOR_BYTES;
    if (within_sector < TQR_RAW_SECTOR_USER_DATA_OFFSET ||
        within_sector >=
            TQR_RAW_SECTOR_USER_DATA_OFFSET + TQR_RAW_SECTOR_USER_DATA_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    *out_user_offset =
        sector * TQR_RAW_SECTOR_USER_DATA_BYTES +
        (within_sector - TQR_RAW_SECTOR_USER_DATA_OFFSET);
    (void)sector_count;
    (void)user_data_size;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_copy_raw_user_data(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    uint8_t *out_user_data,
    size_t out_user_data_capacity,
    size_t *out_user_data_size) {

    size_t sector_count = 0u;
    size_t user_data_size = 0u;
    size_t sector;
    Theron_Track02SignalStatus status;

    if (out_user_data_size) *out_user_data_size = 0u;
    if (!track02_data || !out_user_data || !out_user_data_size) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    status = theron_v1_track02_raw_user_data_size(track02_size,
                                                  md5_hex,
                                                  &sector_count,
                                                  &user_data_size);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }
    if (out_user_data_capacity < user_data_size) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    for (sector = 0u; sector < sector_count; ++sector) {
        const size_t raw_offset = sector * TQR_RAW_SECTOR_BYTES;
        const size_t user_offset = sector * TQR_RAW_SECTOR_USER_DATA_BYTES;
        memcpy(out_user_data + user_offset,
               track02_data + raw_offset + TQR_RAW_SECTOR_USER_DATA_OFFSET,
               TQR_RAW_SECTOR_USER_DATA_BYTES);
    }

    *out_user_data_size = user_data_size;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_copy_raw_user_data_range(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t raw_offset,
    size_t byte_count,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_user_data_offset) {

    size_t copied = 0u;
    size_t first_user_offset = 0u;
    Theron_Track02SignalStatus status;

    if (out_user_data_offset) {
        *out_user_data_offset = 0u;
    }
    if (!track02_data || !out_bytes || !out_user_data_offset ||
        byte_count == 0u || out_bytes_capacity < byte_count) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (raw_offset >= track02_size || byte_count > track02_size - raw_offset) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    status = theron_v1_track02_raw_offset_to_user_offset(raw_offset,
                                                         track02_size,
                                                         md5_hex,
                                                         &first_user_offset);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    while (copied < byte_count) {
        size_t sector = raw_offset / TQR_RAW_SECTOR_BYTES;
        size_t within = raw_offset % TQR_RAW_SECTOR_BYTES;
        size_t sector_user_end =
            TQR_RAW_SECTOR_USER_DATA_OFFSET + TQR_RAW_SECTOR_USER_DATA_BYTES;
        size_t chunk;
        size_t user_offset = 0u;

        status = theron_v1_track02_raw_offset_to_user_offset(raw_offset,
                                                             track02_size,
                                                             md5_hex,
                                                             &user_offset);
        if (status != THERON_TRACK02_SIGNAL_OK) {
            return status;
        }
        if (user_offset != first_user_offset + copied) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
        if (within < TQR_RAW_SECTOR_USER_DATA_OFFSET ||
            within >= sector_user_end) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }

        chunk = sector_user_end - within;
        if (chunk > byte_count - copied) {
            chunk = byte_count - copied;
        }
        if (raw_offset > track02_size || chunk > track02_size - raw_offset) {
            return THERON_TRACK02_SIGNAL_BAD_INPUT;
        }

        memcpy(out_bytes + copied, track02_data + raw_offset, chunk);
        copied += chunk;
        if (copied < byte_count) {
            raw_offset = (sector + 1u) * TQR_RAW_SECTOR_BYTES +
                         TQR_RAW_SECTOR_USER_DATA_OFFSET;
            if (raw_offset >= track02_size) {
                return THERON_TRACK02_SIGNAL_BAD_INPUT;
            }
        }
    }

    *out_user_data_offset = first_user_offset;
    return THERON_TRACK02_SIGNAL_OK;
}

static Theron_Track02SignalStatus track02_copy_startup_bitmap_bytes(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t raw_offset,
    size_t byte_count,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_user_data_offset)
{
    Theron_Track02Variant variant;

    if (out_user_data_offset) {
        *out_user_data_offset = 0u;
    }
    if (!track02_data || !md5_hex || !out_bytes || !out_user_data_offset ||
        byte_count == 0u || out_bytes_capacity < byte_count) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (raw_offset >= track02_size || byte_count > track02_size - raw_offset) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant_is_raw_bin(variant)) {
        return theron_v1_track02_copy_raw_user_data_range(
            track02_data,
            track02_size,
            md5_hex,
            raw_offset,
            byte_count,
            out_bytes,
            out_bytes_capacity,
            out_user_data_offset);
    }
    if (!variant_has_plain_user_data(variant)) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    memcpy(out_bytes, track02_data + raw_offset, byte_count);
    *out_user_data_offset = raw_offset;
    return THERON_TRACK02_SIGNAL_OK;
}

const char *theron_v1_track02_user_data_window_role_name(
    Theron_Track02UserDataWindowRole role) {
    switch (role) {
    case THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE:
        return "bank-descriptor-table";
    case THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN:
        return "post-boundary-span";
    case THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE:
        return "initial-level-candidate";
    case THERON_TRACK02_USER_DATA_WINDOW_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_startup_text_marker_kind_name(
    Theron_Track02StartupTextMarkerKind kind) {
    switch (kind) {
    case THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT:
        return "us-resurrect-theron-prompt";
    case THERON_TRACK02_STARTUP_TEXT_JP_CHAMPION_ROSTER_CLUSTER:
        return "jp-champion-roster-cluster";
    case THERON_TRACK02_STARTUP_TEXT_UNKNOWN:
    default:
        return "unknown";
    }
}

static void catalog_add_user_data_window(
    Theron_Track02UserDataWindowCatalog *catalog,
    Theron_Track02UserDataWindowRole role,
    size_t raw_offset,
    size_t user_data_offset,
    size_t byte_count,
    size_t anchor_index,
    size_t candidate_index) {

    Theron_Track02UserDataWindow *entry;
    if (!catalog) {
        return;
    }
    if (catalog->entry_count >= THERON_TRACK02_MAX_USER_DATA_WINDOWS) {
        ++catalog->overflow_count;
        return;
    }
    entry = &catalog->entries[catalog->entry_count++];
    entry->role = role;
    entry->raw_offset = raw_offset;
    entry->user_data_offset = user_data_offset;
    entry->byte_count = byte_count;
    entry->anchor_index = anchor_index;
    entry->candidate_index = candidate_index;
}

static void catalog_add_startup_text_marker(
    Theron_Track02StartupTextMarkerCatalog *catalog,
    Theron_Track02StartupTextMarkerKind kind,
    size_t raw_offset,
    size_t user_data_offset,
    size_t byte_count,
    size_t occurrence_index) {

    Theron_Track02StartupTextMarker *marker;
    if (!catalog) {
        return;
    }
    if (catalog->marker_count >= THERON_TRACK02_MAX_STARTUP_TEXT_MARKERS) {
        ++catalog->overflow_count;
        return;
    }
    marker = &catalog->markers[catalog->marker_count++];
    marker->kind = kind;
    marker->raw_offset = raw_offset;
    marker->user_data_offset = user_data_offset;
    marker->byte_count = byte_count;
    marker->occurrence_index = occurrence_index;
}

static void catalog_add_startup_roster_name(
    Theron_Track02StartupRosterNameCatalog *catalog,
    const char *name,
    size_t raw_offset,
    size_t user_data_offset,
    const char *title,
    size_t title_raw_offset,
    size_t title_user_data_offset,
    int title_offset_valid) {

    Theron_Track02StartupRosterName *entry;
    if (!catalog || !name || !name[0]) {
        return;
    }
    if (catalog->name_count >= THERON_TRACK02_MAX_STARTUP_ROSTER_NAMES) {
        ++catalog->overflow_count;
        return;
    }
    entry = &catalog->names[catalog->name_count++];
    snprintf(entry->name, sizeof(entry->name), "%s", name);
    snprintf(entry->title, sizeof(entry->title), "%s", title ? title : "");
    entry->raw_offset = raw_offset;
    entry->user_data_offset = user_data_offset;
    entry->title_raw_offset = title_raw_offset;
    entry->title_user_data_offset = title_user_data_offset;
    entry->title_offset_valid = title_offset_valid ? 1 : 0;
}

static void catalog_add_startup_bitmap_sample(
    Theron_Track02StartupBitmapCatalog *catalog,
    unsigned int route_bit,
    size_t raw_offset,
    size_t user_data_offset,
    const uint8_t pixels[THERON_TRACK02_STARTUP_BITMAP_PIXELS],
    size_t nonzero_pixel_count,
    uint32_t checksum) {

    Theron_Track02StartupBitmapSample *sample;
    if (!catalog || !pixels || route_bit == 0u) {
        return;
    }
    if (catalog->sample_count >= THERON_TRACK02_MAX_STARTUP_BITMAP_SAMPLES) {
        ++catalog->overflow_count;
        return;
    }
    sample = &catalog->samples[catalog->sample_count++];
    sample->route_bit = route_bit;
    sample->raw_offset = raw_offset;
    sample->user_data_offset = user_data_offset;
    sample->byte_count = THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES;
    sample->width = 8u;
    sample->height = 8u;
    sample->bpp = 4u;
    sample->nonzero_pixel_count = nonzero_pixel_count;
    sample->checksum = checksum;
    memcpy(sample->pixels, pixels, THERON_TRACK02_STARTUP_BITMAP_PIXELS);
    catalog->route_mask |= route_bit;
}

static Theron_Track02SignalStatus catalog_add_startup_bitmap_sample_from_offset(
    Theron_Track02StartupBitmapCatalog *catalog,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    unsigned int route_bit,
    size_t raw_offset)
{
    uint8_t tile[THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES];
    uint8_t pixels[THERON_TRACK02_STARTUP_BITMAP_PIXELS];
    size_t user_offset = 0u;
    size_t nonzero_pixels = 0u;
    uint32_t checksum = 0u;
    Theron_Track02SignalStatus status;

    if (!catalog || !track02_data || !md5_hex || route_bit == 0u) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = track02_copy_startup_bitmap_bytes(
        track02_data,
        track02_size,
        md5_hex,
        raw_offset,
        sizeof(tile),
        tile,
        sizeof(tile),
        &user_offset);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }
    status = theron_v1_track02_decode_4bpp_tile(tile,
                                                 sizeof(tile),
                                                 pixels,
                                                 &nonzero_pixels,
                                                 &checksum);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }
    catalog_add_startup_bitmap_sample(catalog,
                                      route_bit,
                                      raw_offset,
                                      user_offset,
                                      pixels,
                                      nonzero_pixels,
                                      checksum);
    return THERON_TRACK02_SIGNAL_OK;
}

static int bytes_find(const uint8_t *data,
                      size_t data_size,
                      const uint8_t *needle,
                      size_t needle_size,
                      size_t *out_offset) {
    if (out_offset) {
        *out_offset = 0u;
    }
    if (!data || !needle || needle_size == 0u || needle_size > data_size) {
        return 0;
    }
    for (size_t i = 0u; i <= data_size - needle_size; ++i) {
        if (memcmp(data + i, needle, needle_size) == 0) {
            if (out_offset) {
                *out_offset = i;
            }
            return 1;
        }
    }
    return 0;
}

Theron_Track02SignalStatus theron_v1_track02_catalog_user_data_windows(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02UserDataWindowCatalog *out_catalog) {

    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02Variant variant;
    uint8_t scratch[64];

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
    }
    if (!track02_data || track02_size == 0u || !out_catalog) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_catalog->variant = variant;
    if (!variant_is_raw_bin(variant)) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    signal_status =
        theron_v1_track02_find_bank_signal(track02_data,
                                           track02_size,
                                           md5_hex,
                                           &signal);
    if (signal_status != THERON_TRACK02_SIGNAL_OK) {
        return signal_status;
    }

    for (size_t i = 0u; i < signal.anchor_count; ++i) {
        size_t user_offset = 0u;
        signal_status = theron_v1_track02_copy_raw_user_data_range(
            track02_data,
            track02_size,
            md5_hex,
            signal.descriptor_offsets[i],
            TQR_US_ISO_BANK_STRIDE_BYTES,
            scratch,
            sizeof(scratch),
            &user_offset);
        if (signal_status != THERON_TRACK02_SIGNAL_OK) {
            return signal_status;
        }
        catalog_add_user_data_window(
            out_catalog,
            THERON_TRACK02_USER_DATA_WINDOW_BANK_DESCRIPTOR_TABLE,
            signal.descriptor_offsets[i],
            user_offset,
            TQR_US_ISO_BANK_STRIDE_BYTES,
            i,
            (size_t)-1);

        signal_status = theron_v1_track02_copy_raw_user_data_range(
            track02_data,
            track02_size,
            md5_hex,
            signal.post_boundary_span_offsets[i],
            TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES,
            scratch,
            sizeof(scratch),
            &user_offset);
        if (signal_status != THERON_TRACK02_SIGNAL_OK) {
            return signal_status;
        }
        catalog_add_user_data_window(
            out_catalog,
            THERON_TRACK02_USER_DATA_WINDOW_POST_BOUNDARY_SPAN,
            signal.post_boundary_span_offsets[i],
            user_offset,
            TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES,
            i,
            (size_t)-1);
    }

    if (signal.anchor_count > 0u) {
        Theron_Track02InitialCandidateBinding binding;
        Theron_Track02LevelHandoffStatus binding_status =
            theron_v1_track02_bind_initial_level_candidate(
                track02_data,
                track02_size,
                md5_hex,
                signal.descriptor_offsets[0],
                &binding);
        if (binding_status == THERON_TRACK02_LEVEL_HANDOFF_OK &&
            binding.candidate.user_data_offset_valid) {
            catalog_add_user_data_window(
                out_catalog,
                THERON_TRACK02_USER_DATA_WINDOW_INITIAL_LEVEL_CANDIDATE,
                binding.candidate.absolute_offset,
                binding.candidate.user_data_offset,
                binding.candidate.byte_count,
                0u,
                binding.candidate_index);
        }
    }

    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_text_markers(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupTextMarkerCatalog *out_catalog) {

    static const uint8_t us_prompt[] =
        "GO AWAY AND RESURRECT THERON";
    static const uint8_t jp_anchor[] = "THERON";
    static const uint8_t jp_required[][6] = {
        "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN", "DOTAN"
    };
    Theron_Track02Variant variant;
    const uint8_t *cursor;
    const uint8_t *end;
    size_t occurrence = 0u;

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
    }
    if (!track02_data || track02_size == 0u || !out_catalog) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_catalog->variant = variant;
    if (!variant_is_raw_bin(variant)) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    cursor = track02_data;
    end = track02_data + track02_size;
    while (cursor < end) {
        size_t remaining = (size_t)(end - cursor);
        const uint8_t *found = NULL;
        size_t marker_raw_offset;
        size_t marker_user_offset = 0u;
        size_t marker_byte_count = 0u;
        Theron_Track02StartupTextMarkerKind kind =
            THERON_TRACK02_STARTUP_TEXT_UNKNOWN;

        if (variant == THERON_TRACK02_VARIANT_US_BIN) {
            size_t local_offset = 0u;
            if (!bytes_find(cursor,
                            remaining,
                            us_prompt,
                            sizeof(us_prompt) - 1u,
                            &local_offset)) {
                break;
            }
            found = cursor + local_offset;
            marker_byte_count = sizeof(us_prompt) - 1u;
            kind = THERON_TRACK02_STARTUP_TEXT_US_RESURRECT_THERON_PROMPT;
        } else if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
            size_t local_offset = 0u;
            size_t cluster_span = 0u;
            int complete_cluster = 1;
            if (!bytes_find(cursor,
                            remaining,
                            jp_anchor,
                            sizeof(jp_anchor) - 1u,
                            &local_offset)) {
                break;
            }
            found = cursor + local_offset;
            if ((size_t)(end - found) < 768u) {
                cursor = found + 1u;
                continue;
            }
            for (size_t i = 0u;
                 i < sizeof(jp_required) / sizeof(jp_required[0]);
                 ++i) {
                size_t required_offset = 0u;
                size_t required_len = strlen((const char *)jp_required[i]);
                if (!bytes_find(found,
                                768u,
                                jp_required[i],
                                required_len,
                                &required_offset)) {
                    complete_cluster = 0;
                    break;
                }
                if (required_offset + required_len > cluster_span) {
                    cluster_span = required_offset + required_len;
                }
            }
            if (!complete_cluster) {
                cursor = found + 1u;
                continue;
            }
            marker_byte_count = cluster_span;
            kind = THERON_TRACK02_STARTUP_TEXT_JP_CHAMPION_ROSTER_CLUSTER;
        } else {
            break;
        }

        marker_raw_offset = (size_t)(found - track02_data);
        if (theron_v1_track02_raw_offset_to_user_offset(marker_raw_offset,
                                                         track02_size,
                                                         md5_hex,
                                                         &marker_user_offset) ==
            THERON_TRACK02_SIGNAL_OK) {
            catalog_add_startup_text_marker(out_catalog,
                                            kind,
                                            marker_raw_offset,
                                            marker_user_offset,
                                            marker_byte_count,
                                            occurrence++);
        }
        cursor = found + 1u;
    }

    return out_catalog->marker_count > 0u ? THERON_TRACK02_SIGNAL_OK
                                          : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_copy_startup_text_marker(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupTextMarkerKind kind,
    size_t occurrence_index,
    char *out_text,
    size_t out_text_capacity,
    size_t *out_byte_count,
    Theron_Track02StartupTextMarker *out_marker) {

    Theron_Track02StartupTextMarkerCatalog catalog;
    Theron_Track02SignalStatus status;
    size_t seen = 0u;

    if (out_text && out_text_capacity > 0u) {
        out_text[0] = '\0';
    }
    if (out_byte_count) {
        *out_byte_count = 0u;
    }
    if (out_marker) {
        memset(out_marker, 0, sizeof(*out_marker));
    }
    if (!track02_data || track02_size == 0u || !out_text ||
        out_text_capacity == 0u || !out_byte_count ||
        kind == THERON_TRACK02_STARTUP_TEXT_UNKNOWN) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    status = theron_v1_track02_catalog_startup_text_markers(track02_data,
                                                             track02_size,
                                                             md5_hex,
                                                             &catalog);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    for (size_t i = 0u; i < catalog.marker_count; ++i) {
        const Theron_Track02StartupTextMarker *marker = &catalog.markers[i];
        size_t user_data_offset = 0u;

        if (marker->kind != kind) {
            continue;
        }
        if (seen++ != occurrence_index) {
            continue;
        }
        if (out_text_capacity <= marker->byte_count) {
            return THERON_TRACK02_SIGNAL_BAD_INPUT;
        }
        status = theron_v1_track02_copy_raw_user_data_range(
            track02_data,
            track02_size,
            md5_hex,
            marker->raw_offset,
            marker->byte_count,
            (uint8_t *)out_text,
            out_text_capacity - 1u,
            &user_data_offset);
        if (status != THERON_TRACK02_SIGNAL_OK) {
            out_text[0] = '\0';
            return status;
        }
        if (user_data_offset != marker->user_data_offset) {
            out_text[0] = '\0';
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
        out_text[marker->byte_count] = '\0';
        *out_byte_count = marker->byte_count;
        if (out_marker) {
            *out_marker = *marker;
        }
        return THERON_TRACK02_SIGNAL_OK;
    }

    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_roster_names(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupRosterNameCatalog *out_catalog) {

    static const char *required_names[] = {
        "THERON", "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN", "DOTAN",
        "PENTAI"
    };
    static const char *required_titles[] = {
        "", "GUARDIAN OF WISDO", "THE RESOLUTE", "LORD OF FEALTY",
        "THE BRAVE", "KNIGHT OF STRENGT", "MASTER OF THE WIN",
        "THE SURVIVOR"
    };
    Theron_Track02StartupTextMarkerCatalog text_catalog;
    Theron_Track02Variant variant;
    const Theron_Track02StartupTextMarker *marker = NULL;
    size_t cluster_available;

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
    }
    if (!track02_data || track02_size == 0u || !out_catalog) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_catalog->variant = variant;
    if (variant != THERON_TRACK02_VARIANT_JP_BIN) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    if (theron_v1_track02_catalog_startup_text_markers(
            track02_data,
            track02_size,
            md5_hex,
            &text_catalog) != THERON_TRACK02_SIGNAL_OK ||
        text_catalog.marker_count == 0u) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    marker = &text_catalog.markers[0];
    if (marker->raw_offset >= track02_size) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    cluster_available = track02_size - marker->raw_offset;
    if (cluster_available > 768u) {
        cluster_available = 768u;
    }

    for (size_t i = 0u;
         i < sizeof(required_names) / sizeof(required_names[0]);
         ++i) {
        const uint8_t *cluster = track02_data + marker->raw_offset;
        const char *name = required_names[i];
        const char *title = required_titles[i];
        size_t name_offset = 0u;
        size_t user_offset = 0u;
        size_t title_offset = 0u;
        size_t title_user_offset = 0u;
        int title_offset_valid = 0;
        Theron_Track02SignalStatus status;

        if (!bytes_find(cluster,
                        cluster_available,
                        (const uint8_t *)name,
                        strlen(name),
                        &name_offset)) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
        status = theron_v1_track02_raw_offset_to_user_offset(
            marker->raw_offset + name_offset,
            track02_size,
            md5_hex,
            &user_offset);
        if (status != THERON_TRACK02_SIGNAL_OK) {
            return status;
        }
        if (title && title[0] != '\0') {
            if (!bytes_find(cluster,
                            cluster_available,
                            (const uint8_t *)title,
                            strlen(title),
                            &title_offset)) {
                return THERON_TRACK02_SIGNAL_NOT_FOUND;
            }
            status = theron_v1_track02_raw_offset_to_user_offset(
                marker->raw_offset + title_offset,
                track02_size,
                md5_hex,
                &title_user_offset);
            if (status != THERON_TRACK02_SIGNAL_OK) {
                return status;
            }
            title_offset_valid = 1;
        }
        catalog_add_startup_roster_name(out_catalog,
                                        name,
                                        marker->raw_offset + name_offset,
                                        user_offset,
                                        title,
                                        title_offset_valid
                                            ? marker->raw_offset + title_offset
                                            : 0u,
                                        title_user_offset,
                                        title_offset_valid);
    }

    return out_catalog->name_count > 0u ? THERON_TRACK02_SIGNAL_OK
                                        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_decode_4bpp_tile(
    const uint8_t *tile_bytes,
    size_t tile_size,
    uint8_t out_pixels[THERON_TRACK02_STARTUP_BITMAP_PIXELS],
    size_t *out_nonzero_pixel_count,
    uint32_t *out_checksum) {

    size_t nonzero = 0u;
    uint32_t checksum = 2166136261u;

    if (out_pixels) {
        memset(out_pixels, 0, THERON_TRACK02_STARTUP_BITMAP_PIXELS);
    }
    if (out_nonzero_pixel_count) {
        *out_nonzero_pixel_count = 0u;
    }
    if (out_checksum) {
        *out_checksum = 0u;
    }
    if (!tile_bytes || tile_size < THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES ||
        !out_pixels || !out_nonzero_pixel_count || !out_checksum) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    for (size_t row = 0u; row < 8u; ++row) {
        const uint8_t p0 = tile_bytes[row * 2u + 0u];
        const uint8_t p1 = tile_bytes[row * 2u + 1u];
        const uint8_t p2 = tile_bytes[16u + row * 2u + 0u];
        const uint8_t p3 = tile_bytes[16u + row * 2u + 1u];
        for (size_t x = 0u; x < 8u; ++x) {
            const unsigned int bit = 7u - (unsigned int)x;
            const uint8_t pixel =
                (uint8_t)((((p0 >> bit) & 1u) << 0) |
                          (((p1 >> bit) & 1u) << 1) |
                          (((p2 >> bit) & 1u) << 2) |
                          (((p3 >> bit) & 1u) << 3));
            out_pixels[row * 8u + x] = pixel;
            if (pixel != 0u) {
                ++nonzero;
            }
            checksum ^= pixel;
            checksum *= 16777619u;
        }
    }

    *out_nonzero_pixel_count = nonzero;
    *out_checksum = checksum;
    return nonzero > 0u ? THERON_TRACK02_SIGNAL_OK
                        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

static uint8_t expand_pce_palette_component(uint16_t value) {
    return (uint8_t)((value * 255u + 3u) / 7u);
}

Theron_Track02SignalStatus theron_v1_track02_decode_4bpp_palette(
    const uint8_t *palette_bytes,
    size_t palette_size,
    Theron_Track02Palette4Bpp *out_palette) {

    uint32_t checksum = 2166136261u;
    size_t i;

    if (out_palette) {
        memset(out_palette, 0, sizeof(*out_palette));
    }
    if (!palette_bytes || !out_palette ||
        palette_size < THERON_TRACK02_4BPP_PALETTE_BYTES) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    for (i = 0u; i < THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT; ++i) {
        const uint16_t raw_word = rd16le(palette_bytes + i * 2u);
        Theron_Track02PaletteEntry *entry = &out_palette->entries[i];

        if ((raw_word & 0xfe00u) != 0u) {
            memset(out_palette, 0, sizeof(*out_palette));
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
        entry->raw_word = raw_word;
        /* HuC6260 CTW/CTR: B=bits 0..2, R=bits 3..5, G=bits 6..8.
         * This decodes a caller-supplied, separately verified payload only;
         * it does not identify a Track 02 palette window or promote it. */
        entry->blue = expand_pce_palette_component(raw_word & 0x0007u);
        entry->red = expand_pce_palette_component((raw_word >> 3u) & 0x0007u);
        entry->green = expand_pce_palette_component((raw_word >> 6u) & 0x0007u);
        if (raw_word != 0u) {
            ++out_palette->nonblack_entry_count;
        }
        checksum ^= raw_word;
        checksum *= 16777619u;
    }

    if (out_palette->nonblack_entry_count == 0u) {
        memset(out_palette, 0, sizeof(*out_palette));
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_palette->checksum = checksum;
    out_palette->valid = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_inspect_4bpp_palette_window(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t raw_offset,
    Theron_Track02PaletteWindowEvidence *out_evidence) {

    Theron_Track02Variant variant;
    Theron_Track02SignalStatus status;
    uint8_t palette_bytes[THERON_TRACK02_4BPP_PALETTE_BYTES];
    size_t user_data_offset = 0u;

    if (out_evidence) {
        memset(out_evidence, 0, sizeof(*out_evidence));
    }
    if (!track02_data || !out_evidence || !md5_hex ||
        raw_offset > track02_size ||
        THERON_TRACK02_4BPP_PALETTE_BYTES > track02_size - raw_offset) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    out_evidence->variant = variant;
    out_evidence->raw_offset = raw_offset;
    out_evidence->byte_count = THERON_TRACK02_4BPP_PALETTE_BYTES;
    if (variant_is_raw_bin(variant)) {
        status = theron_v1_track02_copy_raw_user_data_range(
            track02_data, track02_size, md5_hex, raw_offset,
            sizeof(palette_bytes), palette_bytes, sizeof(palette_bytes),
            &user_data_offset);
        if (status != THERON_TRACK02_SIGNAL_OK) {
            return status;
        }
        out_evidence->raw_offset_is_user_data = 1;
    } else if (variant_has_plain_user_data(variant)) {
        memcpy(palette_bytes, track02_data + raw_offset, sizeof(palette_bytes));
        user_data_offset = raw_offset;
        out_evidence->raw_offset_is_user_data = 1;
    } else {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    out_evidence->user_data_offset = user_data_offset;
    out_evidence->payload_checksum =
        tqr_hash_bytes(palette_bytes, sizeof(palette_bytes));
    status = theron_v1_track02_decode_4bpp_palette(
        palette_bytes, sizeof(palette_bytes), &out_evidence->palette);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    out_evidence->format_valid = 1;
    /* No Track 02 loader reference currently binds any palette window. */
    out_evidence->semantic_binding_verified = 0;
    out_evidence->promotion_allowed = 0;
    return THERON_TRACK02_SIGNAL_OK;
}

int theron_v1_track02_palette_window_evidence_can_promote(
    const Theron_Track02PaletteWindowEvidence *evidence) {
    return evidence && evidence->format_valid &&
           evidence->semantic_binding_verified &&
           evidence->promotion_allowed;
}

Theron_Track02SignalStatus theron_v1_track02_colorize_startup_bitmap_route(
    const Theron_Track02StartupBitmapAtlasRoute *indexed_route,
    const Theron_Track02Palette4Bpp *palette,
    Theron_Track02StartupBitmapRgbaRoute *out_route) {

    uint32_t checksum = 2166136261u;
    size_t x;
    size_t y;

    if (out_route) {
        memset(out_route, 0, sizeof(*out_route));
    }
    if (!indexed_route || !palette || !out_route || !palette->valid ||
        indexed_route->route_bit == 0u || indexed_route->width == 0u ||
        indexed_route->width > THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH ||
        indexed_route->height == 0u ||
        indexed_route->height > THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_HEIGHT ||
        indexed_route->tile_count == 0u) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    out_route->route_bit = indexed_route->route_bit;
    out_route->width = indexed_route->width;
    out_route->height = indexed_route->height;
    for (y = 0u; y < indexed_route->height; ++y) {
        for (x = 0u; x < indexed_route->width; ++x) {
            const size_t indexed_offset =
                y * THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH + x;
            const size_t rgba_offset = indexed_offset * 4u;
            const Theron_Track02PaletteEntry *entry =
                &palette->entries[indexed_route->pixels[indexed_offset] & 0x0fu];

            out_route->rgba[rgba_offset + 0u] = entry->red;
            out_route->rgba[rgba_offset + 1u] = entry->green;
            out_route->rgba[rgba_offset + 2u] = entry->blue;
            out_route->rgba[rgba_offset + 3u] = 0xffu;
            checksum ^= entry->raw_word;
            checksum *= 16777619u;
        }
    }
    out_route->checksum = checksum;
    out_route->valid = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_catalog_startup_bitmap_samples(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02StartupBitmapCatalog *out_catalog) {

    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus status;
    Theron_Track02Variant variant;
    const struct {
        unsigned int route_bit;
        size_t anchor_index;
        size_t span_delta;
    } sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 0u, 0u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 0u, 4u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 0u, 8u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 0u, 12u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 0u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 4u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 8u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 12u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 28u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 0u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 4u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 8u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 12u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 28u }
    };
    const struct {
        unsigned int route_bit;
        size_t span_delta;
    } raw_tail_sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 28u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 32u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 36u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 40u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 44u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 48u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 52u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 56u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 60u }
    };
    const struct {
        unsigned int route_bit;
        size_t anchor_index;
        size_t span_delta;
    } raw_bank_mirror_sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 32u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 36u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 40u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 44u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 32u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 36u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 40u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 44u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 1u, 0u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 1u, 4u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 2u, 8u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 2u, 12u }
    };
    const struct {
        unsigned int route_bit;
        size_t anchor_index;
        size_t span_delta;
    } raw_bank_deep_sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 48u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 52u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 56u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 1u, 60u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 48u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 52u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 56u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 2u, 60u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 1u, 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 1u, 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 1u, 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 1u, 28u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 2u, 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 2u, 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 2u, 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 2u, 28u }
    };
    const struct {
        unsigned int route_bit;
        size_t span_delta;
    } iso_extended_sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 0u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 4u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 8u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 12u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 0u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 4u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 8u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 12u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 28u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 16u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 20u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 24u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 28u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 32u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 36u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 40u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 44u }
    };
    const struct {
        unsigned int route_bit;
        size_t span_delta;
    } iso_tail_sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 48u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 52u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 56u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 60u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 64u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 68u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 72u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 76u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 80u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 84u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 88u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 92u }
    };
    const struct {
        unsigned int route_bit;
        size_t span_delta;
    } iso_deep_sample_specs[] = {
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 96u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 100u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 104u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 108u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 112u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 116u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 120u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 124u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 128u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 132u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 136u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 140u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 144u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 148u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 152u },
        { THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
          TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES + 156u }
    };

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
    }
    if (!track02_data || track02_size == 0u || !out_catalog) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_catalog->variant = variant;
    if (!variant_is_raw_bin(variant) &&
        !variant_has_plain_user_data(variant)) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    status = theron_v1_track02_find_bank_signal(track02_data,
                                                track02_size,
                                                md5_hex,
                                                &signal);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    for (size_t i = 0u; i < sizeof(sample_specs) / sizeof(sample_specs[0]); ++i) {
        size_t raw_offset;

        if (sample_specs[i].anchor_index >= signal.anchor_count) {
            continue;
        }
        raw_offset =
            signal.post_boundary_span_offsets[sample_specs[i].anchor_index] +
            sample_specs[i].span_delta;
        (void)catalog_add_startup_bitmap_sample_from_offset(
            out_catalog,
            track02_data,
            track02_size,
            md5_hex,
            sample_specs[i].route_bit,
            raw_offset);
    }
    if (variant_is_raw_bin(variant) && signal.anchor_count > 0u) {
        for (size_t i = 0u;
             i < sizeof(raw_tail_sample_specs) /
                     sizeof(raw_tail_sample_specs[0]);
             ++i) {
            size_t raw_offset =
                signal.post_boundary_span_offsets[0] +
                raw_tail_sample_specs[i].span_delta;
            (void)catalog_add_startup_bitmap_sample_from_offset(
                out_catalog,
                track02_data,
                track02_size,
                md5_hex,
                raw_tail_sample_specs[i].route_bit,
                raw_offset);
        }
        for (size_t i = 0u;
             i < sizeof(raw_bank_mirror_sample_specs) /
                     sizeof(raw_bank_mirror_sample_specs[0]);
             ++i) {
            size_t raw_offset;

            if (raw_bank_mirror_sample_specs[i].anchor_index >=
                signal.anchor_count) {
                continue;
            }
            raw_offset =
                signal.post_boundary_span_offsets
                    [raw_bank_mirror_sample_specs[i].anchor_index] +
                raw_bank_mirror_sample_specs[i].span_delta;
            (void)catalog_add_startup_bitmap_sample_from_offset(
                out_catalog,
                track02_data,
                track02_size,
                md5_hex,
                raw_bank_mirror_sample_specs[i].route_bit,
                raw_offset);
        }
        for (size_t i = 0u;
             i < sizeof(raw_bank_deep_sample_specs) /
                     sizeof(raw_bank_deep_sample_specs[0]);
             ++i) {
            size_t raw_offset;

            if (raw_bank_deep_sample_specs[i].anchor_index >=
                signal.anchor_count) {
                continue;
            }
            raw_offset =
                signal.post_boundary_span_offsets
                    [raw_bank_deep_sample_specs[i].anchor_index] +
                raw_bank_deep_sample_specs[i].span_delta;
            (void)catalog_add_startup_bitmap_sample_from_offset(
                out_catalog,
                track02_data,
                track02_size,
                md5_hex,
                raw_bank_deep_sample_specs[i].route_bit,
                raw_offset);
        }
    }

    if (variant == THERON_TRACK02_VARIANT_US_ISO &&
        signal.anchor_count == 1u) {
        for (size_t i = 0u;
             i < sizeof(iso_extended_sample_specs) /
                     sizeof(iso_extended_sample_specs[0]);
             ++i) {
            size_t raw_offset =
                signal.post_boundary_span_offsets[0] +
                iso_extended_sample_specs[i].span_delta;

            if (out_catalog->route_mask &
                iso_extended_sample_specs[i].route_bit) {
                if (iso_extended_sample_specs[i].span_delta <
                    TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES) {
                    continue;
                }
            }
            (void)catalog_add_startup_bitmap_sample_from_offset(
                out_catalog,
                track02_data,
                track02_size,
                md5_hex,
                iso_extended_sample_specs[i].route_bit,
                raw_offset);
        }
        for (size_t i = 0u;
             i < sizeof(iso_tail_sample_specs) /
                     sizeof(iso_tail_sample_specs[0]);
             ++i) {
            size_t raw_offset;

            raw_offset =
                signal.post_boundary_span_offsets[0] +
                iso_tail_sample_specs[i].span_delta;
            (void)catalog_add_startup_bitmap_sample_from_offset(
                out_catalog,
                track02_data,
                track02_size,
                md5_hex,
                iso_tail_sample_specs[i].route_bit,
                raw_offset);
        }
        for (size_t i = 0u;
             i < sizeof(iso_deep_sample_specs) /
                     sizeof(iso_deep_sample_specs[0]);
             ++i) {
            size_t raw_offset =
                signal.post_boundary_span_offsets[0] +
                iso_deep_sample_specs[i].span_delta;
            (void)catalog_add_startup_bitmap_sample_from_offset(
                out_catalog,
                track02_data,
                track02_size,
                md5_hex,
                iso_deep_sample_specs[i].route_bit,
                raw_offset);
        }
    }

    return out_catalog->sample_count > 0u ? THERON_TRACK02_SIGNAL_OK
                                          : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

static Theron_Track02StartupBitmapAtlasRoute *startup_atlas_route_for_bit(
    Theron_Track02StartupBitmapAtlas *atlas,
    unsigned int route_bit)
{
    size_t i;

    if (!atlas || route_bit == 0u) {
        return NULL;
    }
    for (i = 0u; i < atlas->route_count; ++i) {
        if (atlas->routes[i].route_bit == route_bit) {
            return &atlas->routes[i];
        }
    }
    if (atlas->route_count >= THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX) {
        ++atlas->overflow_count;
        return NULL;
    }
    atlas->routes[atlas->route_count].route_bit = route_bit;
    atlas->routes[atlas->route_count].height = 8u;
    atlas->route_mask |= route_bit;
    return &atlas->routes[atlas->route_count++];
}

static int startup_atlas_append_tile(
    Theron_Track02StartupBitmapAtlasRoute *route,
    const Theron_Track02StartupBitmapSample *sample)
{
    size_t dst_x;

    if (!route || !sample) {
        return 0;
    }
    if (route->tile_count >=
        THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH / 8u) {
        return 0;
    }
    dst_x = route->tile_count * 8u;
    for (size_t y = 0u; y < 8u; ++y) {
        memcpy(route->pixels + y * THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH +
                   dst_x,
               sample->pixels + y * 8u,
               8u);
    }
    if (route->tile_count == 0u) {
        route->first_raw_offset = sample->raw_offset;
        route->first_user_data_offset = sample->user_data_offset;
    }
    route->raw_offsets[route->tile_count] = sample->raw_offset;
    route->user_data_offsets[route->tile_count] = sample->user_data_offset;
    route->last_raw_offset = sample->raw_offset;
    ++route->tile_count;
    route->width = (uint16_t)(route->tile_count * 8u);
    route->nonzero_pixel_count += sample->nonzero_pixel_count;
    route->checksum ^=
        sample->checksum + (uint32_t)(route->tile_count * 16777619u);
    return 1;
}

Theron_Track02SignalStatus theron_v1_track02_build_startup_bitmap_atlas(
    const Theron_Track02StartupBitmapCatalog *catalog,
    Theron_Track02StartupBitmapAtlas *out_atlas)
{
    size_t i;

    if (out_atlas) {
        memset(out_atlas, 0, sizeof(*out_atlas));
    }
    if (!catalog || !out_atlas) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    out_atlas->variant = catalog->variant;
    if (catalog->sample_count == 0u) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    for (i = 0u; i < catalog->sample_count; ++i) {
        const Theron_Track02StartupBitmapSample *sample =
            &catalog->samples[i];
        Theron_Track02StartupBitmapAtlasRoute *route =
            startup_atlas_route_for_bit(out_atlas, sample->route_bit);
        if (!route) {
            continue;
        }
        if (route->tile_count >=
            THERON_TRACK02_STARTUP_BITMAP_ATLAS_LEGACY_MAX_WIDTH / 8u) {
            ++out_atlas->overflow_count;
            continue;
        }
        if (!startup_atlas_append_tile(route, sample)) {
            ++out_atlas->overflow_count;
        }
    }
    for (i = 0u; i < out_atlas->route_count; ++i) {
        out_atlas->total_tile_count += out_atlas->routes[i].tile_count;
        out_atlas->total_nonzero_pixel_count +=
            out_atlas->routes[i].nonzero_pixel_count;
        out_atlas->checksum ^=
            out_atlas->routes[i].checksum +
            (uint32_t)(out_atlas->routes[i].route_bit * 2166136261u);
    }
    return out_atlas->route_count > 0u ? THERON_TRACK02_SIGNAL_OK
                                       : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

static int startup_atlas_route_has_raw_offset(
    const Theron_Track02StartupBitmapAtlasRoute *route,
    size_t raw_offset)
{
    size_t i;

    if (!route) {
        return 0;
    }
    for (i = 0u; i < route->tile_count; ++i) {
        if (route->raw_offsets[i] == raw_offset) {
            return 1;
        }
    }
    return 0;
}

static int startup_atlas_promote_route_wide(
    const Theron_Track02StartupBitmapCatalog *catalog,
    Theron_Track02StartupBitmapAtlasRoute *route,
    size_t target_tile_count)
{
    size_t added = 0u;

    if (!catalog || !route || route->tile_count == 0u) {
        return 0;
    }
    for (size_t i = 0u; i < catalog->sample_count; ++i) {
        const Theron_Track02StartupBitmapSample *sample =
            &catalog->samples[i];
        if (route->tile_count >= target_tile_count) {
            break;
        }
        if (sample->route_bit != route->route_bit ||
            startup_atlas_route_has_raw_offset(route, sample->raw_offset)) {
            continue;
        }
        if (startup_atlas_append_tile(route, sample)) {
            ++added;
        }
    }
    return (int)added;
}

static size_t startup_atlas_count_unmaterialized_samples(
    const Theron_Track02StartupBitmapCatalog *catalog,
    const Theron_Track02StartupBitmapAtlas *atlas)
{
    size_t overflow = 0u;

    if (!catalog || !atlas) {
        return 0u;
    }
    for (size_t i = 0u; i < catalog->sample_count; ++i) {
        const Theron_Track02StartupBitmapSample *sample =
            &catalog->samples[i];
        const Theron_Track02StartupBitmapAtlasRoute *route = NULL;

        for (size_t r = 0u; r < atlas->route_count; ++r) {
            if (atlas->routes[r].route_bit == sample->route_bit) {
                route = &atlas->routes[r];
                break;
            }
        }
        if (!route ||
            !startup_atlas_route_has_raw_offset(route, sample->raw_offset)) {
            ++overflow;
        }
    }
    return overflow;
}

Theron_Track02SignalStatus theron_v1_track02_build_startup_bitmap_atlas_wide(
    const Theron_Track02StartupBitmapCatalog *catalog,
    Theron_Track02StartupBitmapAtlas *out_atlas)
{
    Theron_Track02SignalStatus status;
    size_t promoted = 0u;

    status = theron_v1_track02_build_startup_bitmap_atlas(catalog, out_atlas);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    for (size_t i = 0u; i < out_atlas->route_count; ++i) {
        Theron_Track02StartupBitmapAtlasRoute *route = &out_atlas->routes[i];
        size_t before = route->tile_count;

        (void)startup_atlas_promote_route_wide(
            catalog,
            route,
            THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH / 8u);
        if (route->tile_count > before) {
            promoted += route->tile_count - before;
            out_atlas->promoted_wide_route_mask |= route->route_bit;
        }
    }

    out_atlas->promoted_wide_tile_count = promoted;
    out_atlas->overflow_count =
        catalog->overflow_count +
        startup_atlas_count_unmaterialized_samples(catalog, out_atlas);
    out_atlas->total_tile_count = 0u;
    out_atlas->total_nonzero_pixel_count = 0u;
    out_atlas->checksum = 0u;
    for (size_t i = 0u; i < out_atlas->route_count; ++i) {
        out_atlas->total_tile_count += out_atlas->routes[i].tile_count;
        out_atlas->total_nonzero_pixel_count +=
            out_atlas->routes[i].nonzero_pixel_count;
        out_atlas->checksum ^=
            out_atlas->routes[i].checksum +
            (uint32_t)(out_atlas->routes[i].route_bit * 2166136261u);
    }
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_copy_user_data_window_by_role(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02UserDataWindowRole role,
    size_t occurrence_index,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_byte_count,
    Theron_Track02UserDataWindow *out_window) {

    Theron_Track02UserDataWindowCatalog catalog;
    Theron_Track02SignalStatus status;
    size_t seen = 0u;

    if (out_byte_count) {
        *out_byte_count = 0u;
    }
    if (out_window) {
        memset(out_window, 0, sizeof(*out_window));
    }
    if (!track02_data || track02_size == 0u || !out_bytes ||
        !out_byte_count || role == THERON_TRACK02_USER_DATA_WINDOW_UNKNOWN) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    status = theron_v1_track02_catalog_user_data_windows(track02_data,
                                                          track02_size,
                                                          md5_hex,
                                                          &catalog);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    for (size_t i = 0u; i < catalog.entry_count; ++i) {
        const Theron_Track02UserDataWindow *entry = &catalog.entries[i];
        size_t user_data_offset = 0u;

        if (entry->role != role) {
            continue;
        }
        if (seen++ != occurrence_index) {
            continue;
        }
        if (out_bytes_capacity < entry->byte_count) {
            return THERON_TRACK02_SIGNAL_BAD_INPUT;
        }

        status = theron_v1_track02_copy_raw_user_data_range(
            track02_data,
            track02_size,
            md5_hex,
            entry->raw_offset,
            entry->byte_count,
            out_bytes,
            out_bytes_capacity,
            &user_data_offset);
        if (status != THERON_TRACK02_SIGNAL_OK) {
            return status;
        }
        if (user_data_offset != entry->user_data_offset) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }

        *out_byte_count = entry->byte_count;
        if (out_window) {
            *out_window = *entry;
        }
        return THERON_TRACK02_SIGNAL_OK;
    }

    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

static int range_is_all_zero(const uint8_t *data, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        if (data[i] != 0) return 0;
    }
    return 1;
}

static void copy_offsets(size_t *dst, const size_t *src, size_t count) {
    size_t i;
    for (i = 0; i < count && i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        dst[i] = src[i];
    }
}

static int pattern_matches_at_offsets(const uint8_t *data,
                                      size_t size,
                                      const uint8_t *pattern,
                                      size_t pattern_size,
                                      const size_t *offsets,
                                      size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) {
        if (offsets[i] > size || pattern_size > size - offsets[i]) {
            return 0;
        }
        if (memcmp(data + offsets[i], pattern, pattern_size) != 0) {
            return 0;
        }
    }
    return 1;
}

static void fill_raw_sector_coordinates(Theron_Track02BankSignal *out_signal,
                                        const size_t *descriptor_offsets,
                                        const size_t *span_offsets,
                                        size_t count) {
    size_t i;

    out_signal->raw_sector_bytes = TQR_RAW_SECTOR_BYTES;
    out_signal->raw_sector_user_data_offset = TQR_RAW_SECTOR_USER_DATA_OFFSET;
    for (i = 0; i < count && i < THERON_TRACK02_MAX_BANK_ANCHORS; ++i) {
        const size_t descriptor_remainder = descriptor_offsets[i] % TQR_RAW_SECTOR_BYTES;
        const size_t span_remainder = span_offsets[i] % TQR_RAW_SECTOR_BYTES;

        out_signal->descriptor_raw_sector_numbers[i] =
            descriptor_offsets[i] / TQR_RAW_SECTOR_BYTES;
        out_signal->post_boundary_span_raw_sector_numbers[i] =
            span_offsets[i] / TQR_RAW_SECTOR_BYTES;
        out_signal->descriptor_raw_sector_user_offsets[i] =
            descriptor_remainder >= TQR_RAW_SECTOR_USER_DATA_OFFSET
                ? descriptor_remainder - TQR_RAW_SECTOR_USER_DATA_OFFSET
                : descriptor_remainder;
        out_signal->post_boundary_span_raw_sector_user_offsets[i] =
            span_remainder >= TQR_RAW_SECTOR_USER_DATA_OFFSET
                ? span_remainder - TQR_RAW_SECTOR_USER_DATA_OFFSET
                : span_remainder;
    }
}

static size_t count_pattern_occurrences(const uint8_t *data,
                                        size_t size,
                                        const uint8_t *pattern,
                                        size_t pattern_size) {
    size_t count = 0;
    size_t i;

    if (!data || !pattern || pattern_size == 0 || size < pattern_size) return 0;
    for (i = 0; i <= size - pattern_size; ++i) {
        if (memcmp(data + i, pattern, pattern_size) == 0) {
            ++count;
        }
    }
    return count;
}

static Theron_Track02SignalStatus find_raw_bin_bank_signal(
    const uint8_t *track02_data,
    size_t track02_size,
    Theron_Track02BankSignal *out_signal,
    const size_t *descriptor_offsets,
    const size_t *span_offsets) {

    const size_t occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_bank_stride_descriptor,
                                  TQR_US_ISO_BANK_STRIDE_BYTES);
    const size_t boundary_prefix_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_bank_boundary_prefix,
                                  TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES);
    const size_t post_boundary_span_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_post_boundary_span,
                                  TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES);

    if (!pattern_matches_at_offsets(track02_data,
                                    track02_size,
                                    g_us_iso_bank_stride_descriptor,
                                    TQR_US_ISO_BANK_STRIDE_BYTES,
                                    descriptor_offsets,
                                    TQR_RAW_BIN_BANK_ANCHOR_COUNT)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (!pattern_matches_at_offsets(track02_data,
                                    track02_size,
                                    g_us_iso_post_boundary_span,
                                    TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES,
                                    span_offsets,
                                    TQR_RAW_BIN_BANK_ANCHOR_COUNT)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    out_signal->anchor_count = TQR_RAW_BIN_BANK_ANCHOR_COUNT;
    out_signal->descriptor_offset = descriptor_offsets[0];
    out_signal->descriptor_size = TQR_US_ISO_BANK_STRIDE_BYTES;
    copy_offsets(out_signal->descriptor_offsets,
                 descriptor_offsets,
                 TQR_RAW_BIN_BANK_ANCHOR_COUNT);
    out_signal->occurrence_count = occurrence_count;
    out_signal->first_value = rd16le(g_us_iso_bank_stride_descriptor);
    out_signal->last_value =
        rd16le(g_us_iso_bank_stride_descriptor + TQR_US_ISO_BANK_STRIDE_BYTES - 2u);
    out_signal->stride = TQR_US_ISO_BANK_STRIDE_STEP;
    out_signal->value_count = TQR_US_ISO_BANK_STRIDE_COUNT;
    out_signal->next_nonzero_offset = span_offsets[0];
    out_signal->boundary_prefix_size = TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES;
    out_signal->boundary_prefix_occurrence_count = boundary_prefix_occurrence_count;
    out_signal->post_boundary_span_size = TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES;
    copy_offsets(out_signal->post_boundary_span_offsets,
                 span_offsets,
                 TQR_RAW_BIN_BANK_ANCHOR_COUNT);
    out_signal->post_boundary_span_occurrence_count = post_boundary_span_occurrence_count;
    out_signal->post_boundary_span_first_word = rd16le(g_us_iso_post_boundary_span);
    out_signal->post_boundary_span_last_word =
        rd16le(g_us_iso_post_boundary_span + TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES - 2u);
    fill_raw_sector_coordinates(out_signal,
                                descriptor_offsets,
                                span_offsets,
                                TQR_RAW_BIN_BANK_ANCHOR_COUNT);

    /* Per-anchor audio-bank marker: 4-byte LE word that immediately
     * precedes the post-boundary span.  Read independently for each
     * anchor; failure on any one anchor is recorded in the per-anchor
     * recognized[] flag rather than failing the overall signal. */
    for (size_t i = 0; i < TQR_RAW_BIN_BANK_ANCHOR_COUNT; ++i) {
        out_signal->audio_bank_id_recognized[i] = read_audio_bank_marker(
            track02_data,
            track02_size,
            span_offsets,
            i,
            &out_signal->audio_bank_id[i],
            &out_signal->audio_bank_id_offsets[i],
            &out_signal->audio_bank_prefix_offsets[i]);
    }

    return occurrence_count == TQR_RAW_BIN_BANK_ANCHOR_COUNT &&
           boundary_prefix_occurrence_count == TQR_RAW_BIN_BANK_ANCHOR_COUNT &&
           post_boundary_span_occurrence_count == TQR_RAW_BIN_BANK_ANCHOR_COUNT
        ? THERON_TRACK02_SIGNAL_OK
        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_find_bank_signal(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02BankSignal *out_signal) {

    Theron_Track02Variant variant;
    size_t occurrence_count;
    size_t boundary_prefix_occurrence_count;
    size_t post_boundary_span_occurrence_count;
    const size_t zero_offset = TQR_US_ISO_BANK_STRIDE_OFFSET + TQR_US_ISO_BANK_STRIDE_BYTES;
    const size_t zero_bytes = TQR_US_ISO_BANK_BOUNDARY_OFFSET - zero_offset;

    if (out_signal) {
        memset(out_signal, 0, sizeof(*out_signal));
    }
    if (!track02_data || track02_size == 0 || !md5_hex || !out_signal) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_signal->variant = variant;

    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        return track_is_all_zero(track02_data, track02_size)
            ? THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        return find_raw_bin_bank_signal(track02_data,
                                        track02_size,
                                        out_signal,
                                        g_us_bin_descriptor_offsets,
                                        g_us_bin_post_boundary_span_offsets);
    }
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        return find_raw_bin_bank_signal(track02_data,
                                        track02_size,
                                        out_signal,
                                        g_jp_bin_descriptor_offsets,
                                        g_jp_bin_post_boundary_span_offsets);
    }
    if (variant != THERON_TRACK02_VARIANT_US_ISO) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    if (track02_size < TQR_US_ISO_BANK_BOUNDARY_OFFSET + TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_STRIDE_OFFSET,
               g_us_iso_bank_stride_descriptor,
               TQR_US_ISO_BANK_STRIDE_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (!range_is_all_zero(track02_data + zero_offset, zero_bytes)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_BOUNDARY_OFFSET,
               g_us_iso_bank_boundary_prefix,
               TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_BOUNDARY_OFFSET,
               g_us_iso_post_boundary_span,
               TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    occurrence_count = count_pattern_occurrences(track02_data,
                                                 track02_size,
                                                 g_us_iso_bank_stride_descriptor,
                                                 TQR_US_ISO_BANK_STRIDE_BYTES);
    boundary_prefix_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_bank_boundary_prefix,
                                  TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES);
    post_boundary_span_occurrence_count =
        count_pattern_occurrences(track02_data,
                                  track02_size,
                                  g_us_iso_post_boundary_span,
                                  TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES);
    out_signal->descriptor_offset = TQR_US_ISO_BANK_STRIDE_OFFSET;
    out_signal->descriptor_size = TQR_US_ISO_BANK_STRIDE_BYTES;
    out_signal->anchor_count = 1u;
    out_signal->descriptor_offsets[0] = TQR_US_ISO_BANK_STRIDE_OFFSET;
    out_signal->occurrence_count = occurrence_count;
    out_signal->first_value = rd16le(g_us_iso_bank_stride_descriptor);
    out_signal->last_value =
        rd16le(g_us_iso_bank_stride_descriptor + TQR_US_ISO_BANK_STRIDE_BYTES - 2u);
    out_signal->stride = TQR_US_ISO_BANK_STRIDE_STEP;
    out_signal->value_count = TQR_US_ISO_BANK_STRIDE_COUNT;
    out_signal->post_descriptor_zero_offset = zero_offset;
    out_signal->post_descriptor_zero_bytes = zero_bytes;
    out_signal->next_nonzero_offset = TQR_US_ISO_BANK_BOUNDARY_OFFSET;
    out_signal->boundary_prefix_size = TQR_US_ISO_BANK_BOUNDARY_PREFIX_BYTES;
    out_signal->boundary_prefix_occurrence_count = boundary_prefix_occurrence_count;
    out_signal->post_boundary_span_size = TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES;
    out_signal->post_boundary_span_offsets[0] = TQR_US_ISO_BANK_BOUNDARY_OFFSET;
    out_signal->post_boundary_span_occurrence_count = post_boundary_span_occurrence_count;
    out_signal->post_boundary_span_first_word = rd16le(g_us_iso_post_boundary_span);
    out_signal->post_boundary_span_last_word =
        rd16le(g_us_iso_post_boundary_span + TQR_US_ISO_POST_BOUNDARY_SPAN_BYTES - 2u);

    return occurrence_count == 1u &&
           boundary_prefix_occurrence_count == 1u &&
           post_boundary_span_occurrence_count == 1u
        ? THERON_TRACK02_SIGNAL_OK
        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_find_audio_bank_marker(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t anchor_index,
    uint32_t *out_audio_bank_id,
    size_t *out_audio_bank_id_offset,
    size_t *out_audio_bank_prefix_offset) {
    Theron_Track02Variant variant;

    if (out_audio_bank_id) *out_audio_bank_id = 0u;
    if (out_audio_bank_id_offset) *out_audio_bank_id_offset = 0u;
    if (out_audio_bank_prefix_offset) *out_audio_bank_prefix_offset = 0u;

    if (!track02_data || track02_size == 0 || !md5_hex) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (anchor_index >= TQR_RAW_BIN_BANK_ANCHOR_COUNT) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        return read_audio_bank_marker(track02_data,
                                      track02_size,
                                      g_us_bin_post_boundary_span_offsets,
                                      anchor_index,
                                      out_audio_bank_id,
                                      out_audio_bank_id_offset,
                                      out_audio_bank_prefix_offset)
            ? THERON_TRACK02_SIGNAL_OK
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        return read_audio_bank_marker(track02_data,
                                      track02_size,
                                      g_jp_bin_post_boundary_span_offsets,
                                      anchor_index,
                                      out_audio_bank_id,
                                      out_audio_bank_id_offset,
                                      out_audio_bank_prefix_offset)
            ? THERON_TRACK02_SIGNAL_OK
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* US ISO is a partial extract and JP Rev 1 ISO is zero-filled; both
     * lack the post-boundary span audio-bank anchors, so the marker is
     * unsupported on those variants rather than missing. */
    return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
}

const char *theron_v1_track02_signal_status_name(Theron_Track02SignalStatus status) {
    switch (status) {
    case THERON_TRACK02_SIGNAL_OK:
        return "ok";
    case THERON_TRACK02_SIGNAL_NOT_FOUND:
        return "not-found";
    case THERON_TRACK02_SIGNAL_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT:
        return "unsupported-variant";
    case THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE:
        return "insufficient-zero-image";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_variant_name(Theron_Track02Variant variant) {
    switch (variant) {
    case THERON_TRACK02_VARIANT_JP_BIN:
        return "jp-bin";
    case THERON_TRACK02_VARIANT_US_BIN:
        return "us-bin";
    case THERON_TRACK02_VARIANT_JP_REV1_ISO:
        return "jp-rev1-iso";
    case THERON_TRACK02_VARIANT_US_ISO:
        return "us-iso";
    case THERON_TRACK02_VARIANT_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_source_evidence(void) {
    return "theron_v1_track02.c: US Track 02 ISO MD5 "
           THERON_TRACK02_MD5_US_ISO
           " has a unique little-endian bank-stride descriptor at offset "
           "0x1584 (9 words, 0x0020..0x2020, stride 0x0400), followed by "
           "zero-fill through a unique opaque 44-byte boundary span at offset "
           "0x3000; raw US Track 02 BIN "
           THERON_TRACK02_MD5_US_BIN
           " carries the same descriptor at offsets 0x70be06, 0x70e2c6, "
           "0x710904 and the same opaque span at offsets 0x2d53e0, "
           "0x47d040, 0x712840; raw JP Track 02 BIN "
           THERON_TRACK02_MD5_JP_BIN
           " carries the same anchors exactly one 2352-byte raw CD sector "
           "earlier at descriptor offsets 0x70b4d6, 0x70d996, 0x70ffd4 "
           "and span offsets 0x2d4ab0, 0x47c710, 0x711f10; JP Rev 1 ISO "
           THERON_TRACK02_MD5_JP_REV1_ISO
           " is hash-verified but zero-filled in the available image, so no "
           "JP Rev 1 ISO dungeon-bank offset is claimed.  Audio-bank marker: "
           "raw US/JP BINs each carry a 12-byte `00 ff*10 00` sentinel "
           "immediately preceding the 4-byte little-endian audio-bank id word "
           "at every post-boundary span anchor; US ids are 0x01725800, "
           "0x01600801, 0x01122401 at offsets 0x2d53dc, 0x47d03c, "
           "0x71283c; JP ids are 0x01530301, 0x01411301, 0x01682801 at "
           "offsets 0x2d4aac, 0x47c70c, 0x711f0c.  Initial level candidate: "
           "a hash-gated Track 02 scan finds exactly one loader-compatible "
           "32x27 startup payload with seed 0x0108e938 and level index 0x0026 "
           "in each raw image: US offset 0x7015b4 and JP offset 0x700c84.  "
           "Raw-sector user-data bridge: JP/US raw BINs are MODE1/2352 sector "
           "images; descriptor/span payload offsets map into the 2048-byte "
           "user-data stream, while the audio-bank id words sit outside "
           "user-data immediately before the span.  Startup text markers: raw "
           "US Track 02 carries seven user-data occurrences of `GO AWAY AND "
           "RESURRECT THERON` starting at 0xa0722; raw JP Track 02 carries "
           "seven user-data champion-roster marker clusters starting at "
           "0xb3d98 with THERON/MARA/LINOS/HEXA/HAKAR/TIRAN/DOTAN ASCII "
           "names.  The first decoded startup roster additionally carries "
           "PENTAI.  This is a bounded initial-level, raw-sector, and startup "
           "text-marker handoff, not a full dungeon-record decoder, "
           "object-table decoder, graphics/menu-art decoder, font/text "
           "renderer, ADPCM decode, CD-DA decode, or runtime playback proof.";
}

/* ── Semantic dungeon-descriptor table decoder ──────────────────── */

/* Decode the documented 9-word little-endian stride table at the supplied
 * bytes.  Returns THERON_TRACK02_TABLE_DECODE_OK on success.
 *
 * Shape locked (see theron_v1_track02.h for the source citation):
 *   - 9 little-endian uint16 entries
 *   - strictly ascending (entries[i+1] > entries[i])
 *   - constant stride 0x0400 between adjacent entries
 *   - all entries + stride land in the closed range [0x0020, 0x2020 + 0x0400)
 *
 * The decoder is independent of any single offset: callers pass the raw
 * bytes, not an offset into Track 02.  This keeps the function
 * regression-testable from synthetic fixtures and from real Track 02 data
 * via theron_v1_track02_find_bank_signal(). */
Theron_Track02TableDecodeStatus theron_v1_track02_decode_descriptor_table(
    const uint8_t *descriptor_bytes,
    size_t descriptor_size,
    uint16_t expected_stride,
    Theron_Track02DescriptorTable *out_table) {

    const size_t required_bytes = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES * 2u;
    size_t i;

    if (out_table) {
        memset(out_table, 0, sizeof(*out_table));
    }
    if (!descriptor_bytes || !out_table || descriptor_size < required_bytes) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (expected_stride == 0u) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }

    for (i = 0; i < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        out_table->entries[i] = rd16le(descriptor_bytes + (i * 2u));
    }
    out_table->entry_count = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES;
    out_table->first_value = out_table->entries[0];
    out_table->last_value =
        out_table->entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES - 1u];
    out_table->stride = expected_stride;
    out_table->exclusive_upper_bound =
        (uint16_t)(out_table->last_value + expected_stride);

    /* Strictly ascending: every adjacent pair must increase. */
    for (i = 0; i + 1u < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        if (out_table->entries[i + 1u] <= out_table->entries[i]) {
            return THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING;
        }
    }

    /* Constant stride: every adjacent difference must equal expected_stride. */
    for (i = 0; i + 1u < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES; ++i) {
        const uint16_t diff =
            (uint16_t)(out_table->entries[i + 1u] - out_table->entries[i]);
        if (diff != expected_stride) {
            return THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE;
        }
    }

    /* Range sanity: documented shape is [0x0020, 0x2020 + 0x0400) inclusive
     * on both ends (the last entry is 0x2020; the stride window ends at
     * 0x2020 + 0x0400 == 0x2420, exclusive).  We also accept any strictly
     * ascending 9-word stride sequence whose first value, last value, and
     * exclusive upper bound all fit in 16 bits -- but require the range to
     * stay sane (exclusive_upper_bound > last_value; first_value > 0) so
     * empty-range and zero-origin decoders are rejected. */
    if (out_table->first_value == 0u) {
        return THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING;
    }
    if (out_table->exclusive_upper_bound <= out_table->last_value) {
        return THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE;
    }

    /* The documented 0x0020..0x2020 inclusive + 0x0400 stride window. */
    {
        const uint16_t lo = 0x0020u;
        const uint16_t hi_inclusive = (uint16_t)(0x2020u + expected_stride);
        out_table->range_inclusive =
            (out_table->first_value >= lo) &&
            (out_table->exclusive_upper_bound <= hi_inclusive) ? 1 : 0;
    }

    return THERON_TRACK02_TABLE_DECODE_OK;
}

Theron_Track02TableDecodeStatus theron_v1_track02_bind_descriptor_windows(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    const Theron_Track02DescriptorTable *table,
    Theron_Track02DescriptorWindowBinding *out_binding) {

    size_t base_offset;
    size_t i;

    if (out_binding) {
        memset(out_binding, 0, sizeof(*out_binding));
    }
    if (!track02_data || track02_size == 0 || !table || !out_binding) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (table->entry_count != THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES ||
        table->stride == 0u) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (descriptor_offset < TQR_US_ISO_BANK_STRIDE_OFFSET ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size ||
        descriptor_offset > track02_size - TQR_US_ISO_BANK_STRIDE_BYTES) {
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    /* The source-locked descriptor offset is 0x1584 bytes into the
     * descriptor region.  ReDMCSB has no Theron's Quest Track 02 loader;
     * docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md records
     * this byte anchor for the US ISO and JP/US raw BIN replicas. */
    base_offset = descriptor_offset - TQR_US_ISO_BANK_STRIDE_OFFSET;

    out_binding->entry_count = table->entry_count;
    out_binding->base_offset = base_offset;
    out_binding->descriptor_offset = descriptor_offset;
    out_binding->window_size = table->stride;

    for (i = 0; i < table->entry_count; ++i) {
        Theron_Track02DescriptorWindow *window = &out_binding->windows[i];
        const size_t absolute_offset = base_offset + (size_t)table->entries[i];
        const size_t window_size = (size_t)table->stride;
        size_t j;
        int saw_nonzero = 0;

        if (absolute_offset < base_offset ||
            absolute_offset > track02_size ||
            window_size > track02_size - absolute_offset) {
            memset(out_binding, 0, sizeof(*out_binding));
            return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
        }

        window->entry_index = i;
        window->relative_offset = table->entries[i];
        window->absolute_offset = absolute_offset;
        window->byte_count = window_size;

        for (j = 0; j < window_size; ++j) {
            if (track02_data[absolute_offset + j] != 0u) {
                if (!saw_nonzero) {
                    window->first_nonzero_offset = absolute_offset + j;
                    saw_nonzero = 1;
                }
                window->last_nonzero_offset = absolute_offset + j;
                ++window->nonzero_byte_count;
            }
        }

        window->contains_descriptor_table =
            descriptor_offset >= absolute_offset &&
            descriptor_offset <= (absolute_offset + window_size) &&
            TQR_US_ISO_BANK_STRIDE_BYTES <=
                (absolute_offset + window_size) - descriptor_offset;
        if (window->contains_descriptor_table) {
            window->kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE;
        } else if (window->nonzero_byte_count == 0u) {
            window->kind = THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL;
        } else {
            window->kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
        }
    }

    if (!out_binding
             ->windows[TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR]
             .contains_descriptor_table) {
        memset(out_binding, 0, sizeof(*out_binding));
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    return THERON_TRACK02_TABLE_DECODE_OK;
}

/* ── Semantic role binding for descriptor table entries ──────────────── */

/* Look up the entry_index of the descriptor-window in a semantic-binding
 * array.  Returns -1 when no entry has is_descriptor_window set. */
int theron_v1_track02_find_descriptor_window_entry_index(
    const Theron_Track02DescriptorEntrySemanticBinding *entries,
    size_t entry_count) {
    size_t i;

    if (!entries || entry_count == 0u) return -1;
    for (i = 0; i < entry_count; ++i) {
        if (entries[i].is_descriptor_window) {
            return (int)i;
        }
    }
    return -1;
}

const char *theron_v1_track02_descriptor_entry_role_name(
    Theron_Track02DescriptorEntryRole role) {
    switch (role) {
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL:
        return "reserved-zero-fill";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE:
        return "contains-descriptor-table";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA:
        return "pre-descriptor-data";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA:
        return "post-descriptor-data";
    case THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_UNKNOWN:
    default:
        return "unknown";
    }
}

/* Bind semantic roles to every entry of a decoded descriptor table.
 *
 * This is a deterministic byte-level binding derived from the existing
 * window classification plus three descriptor-window markers:
 *   - descriptor_at_window_tail: descriptor_offset + 18 ==
 *     absolute_offset + byte_count (descriptor occupies the last 18
 *     bytes of its window).  Observed in the US Track 02 ISO and
 *     hash-verified JP raw BIN anchor 0; the descriptor sits at the
 *     tail of an RTS-terminated code region in both cases.
 *   - byte_before_descriptor_is_rts: track02_data[descriptor_offset - 1]
 *     == 0x60 (HuC6280 / 65C02-derivative RTS opcode).
 *   - all_zero_after_descriptor: every byte after the descriptor within
 *     its 0x0400-byte window is zero.
 *
 * The function is shape-driven, not magic-number-driven: it does not
 * look up specific byte sequences, only derives positions and counts
 * from the supplied descriptor_offset and the windows already classified
 * by theron_v1_track02_bind_descriptor_windows().
 *
 * Bounded non-claim: this binding does not interpret the bytes as code,
 * graphics, palette, text, or any data-domain payload.  It only pins
 * byte-shape relationships around the descriptor.
 */
Theron_Track02TableDecodeStatus theron_v1_track02_bind_descriptor_entry_roles(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    const Theron_Track02DescriptorTable *table,
    Theron_Track02DescriptorEntrySemanticBinding *out_entries) {

    Theron_Track02DescriptorWindowBinding windows;
    Theron_Track02TableDecodeStatus status;
    int descriptor_window_index = -1;
    size_t i;

    if (out_entries) {
        memset(out_entries, 0,
               sizeof(*out_entries) * THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES);
    }
    if (!track02_data || track02_size == 0 || !table || !out_entries) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (table->entry_count != THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES ||
        table->stride == 0u) {
        return THERON_TRACK02_TABLE_DECODE_BAD_INPUT;
    }
    if (descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    /* Reuse the existing window classification.  This keeps the role
     * binding consistent with the byte-level kind flags already shipped
     * by theron_v1_track02_bind_descriptor_windows(). */
    status = theron_v1_track02_bind_descriptor_windows(
        track02_data,
        track02_size,
        descriptor_offset,
        table,
        &windows);
    if (status != THERON_TRACK02_TABLE_DECODE_OK) {
        return status;
    }

    /* Locate the descriptor-window entry index.  This index drives
     * PRE_DESCRIPTOR_DATA / POST_DESCRIPTOR_DATA classification for
     * the other 8 entries. */
    for (i = 0; i < windows.entry_count; ++i) {
        if (windows.windows[i].contains_descriptor_table) {
            descriptor_window_index = (int)i;
            break;
        }
    }
    if (descriptor_window_index < 0) {
        return THERON_TRACK02_TABLE_DECODE_NOT_FOUND;
    }

    for (i = 0; i < table->entry_count; ++i) {
        const Theron_Track02DescriptorWindow *window = &windows.windows[i];
        Theron_Track02DescriptorEntrySemanticBinding *entry = &out_entries[i];

        entry->entry_index = i;
        entry->relative_offset = window->relative_offset;
        entry->absolute_offset = window->absolute_offset;
        entry->byte_count = window->byte_count;
        entry->nonzero_byte_count = window->nonzero_byte_count;
        entry->first_nonzero_offset = window->first_nonzero_offset;
        entry->last_nonzero_offset = window->last_nonzero_offset;
        entry->is_descriptor_window = window->contains_descriptor_table ? 1 : 0;

        /* Order the role assignment deterministically.  Descriptor-window
         * check must come first because the descriptor-window is the
         * reference point for PRE/POST classification. */
        if (entry->is_descriptor_window) {
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_CONTAINS_DESCRIPTOR_TABLE;
        } else if (window->nonzero_byte_count == 0u) {
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_RESERVED_ZERO_FILL;
        } else if ((int)i < descriptor_window_index) {
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_PRE_DESCRIPTOR_DATA;
        } else {
            /* i > descriptor_window_index is the only remaining case.
             * Equal-to is excluded by the is_descriptor_window branch. */
            entry->role = THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA;
        }

        if (entry->is_descriptor_window) {
            /* Bounded byte-tail markers for the descriptor-window:
             *   - byte_before_descriptor_is_rts
             *   - all_zero_after_descriptor
             *   - first_nonzero_after_descriptor (0 when all-zero)
             *
             * These markers do not interpret the bytes; they only
             * describe their relative position around the descriptor. */
            if (descriptor_offset > 0u &&
                descriptor_offset <= track02_size) {
                entry->byte_before_descriptor =
                    track02_data[descriptor_offset - 1u];
                entry->byte_before_descriptor_is_rts =
                    (entry->byte_before_descriptor == 0x60u) ? 1 : 0;
            }
            {
                const size_t after_offset =
                    descriptor_offset + TQR_US_ISO_BANK_STRIDE_BYTES;
                size_t j;
                int saw_nonzero_after = 0;
                if (after_offset <= track02_size &&
                    (entry->absolute_offset + entry->byte_count) <=
                        track02_size) {
                    const size_t after_end =
                        entry->absolute_offset + entry->byte_count;
                    for (j = after_offset; j < after_end; ++j) {
                        if (track02_data[j] != 0u) {
                            entry->first_nonzero_after_descriptor = j;
                            saw_nonzero_after = 1;
                            break;
                        }
                    }
                }
                entry->all_zero_after_descriptor = saw_nonzero_after ? 0 : 1;
            }
        }
    }

    return THERON_TRACK02_TABLE_DECODE_OK;
}

static uint32_t tqr_receipt_hash_add_u64(uint32_t hash, uint64_t value) {
    size_t byte_index;

    for (byte_index = 0u; byte_index < 8u; ++byte_index) {
        hash ^= (uint8_t)(value & 0xffu);
        hash *= 16777619u;
        value >>= 8u;
    }
    return hash;
}

Theron_Track02SignalStatus theron_v1_track02_capture_nonstartup_sector_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02NonstartupSectorReceipt *out_receipt) {

    Theron_Track02BankSignal signal;
    Theron_Track02Variant variant;
    Theron_Track02SignalStatus status;
    uint32_t receipt_hash = 2166136261u;
    size_t anchor_index;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!track02_data || track02_size == 0u || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (!variant_is_raw_bin(variant)) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }
    status = theron_v1_track02_find_bank_signal(track02_data,
                                                track02_size,
                                                md5_hex,
                                                &signal);
    if (status != THERON_TRACK02_SIGNAL_OK || signal.anchor_count == 0u) {
        return status;
    }

    out_receipt->variant = variant;
    out_receipt->anchor_count = signal.anchor_count;
    out_receipt->verified_track02 = 1;
    out_receipt->opaque_only = 1;
    out_receipt->promotion_blocked = 1;

    for (anchor_index = 0u; anchor_index < signal.anchor_count; ++anchor_index) {
        Theron_Track02DescriptorTable table;
        Theron_Track02DescriptorEntrySemanticBinding entries
            [THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
        const size_t descriptor_offset = signal.descriptor_offsets[anchor_index];
        size_t entry_index;

        out_receipt->descriptor_raw_offsets[anchor_index] = descriptor_offset;

        if (theron_v1_track02_decode_descriptor_table(
                track02_data + descriptor_offset,
                TQR_US_ISO_BANK_STRIDE_BYTES,
                TQR_US_ISO_BANK_STRIDE_STEP,
                &table) != THERON_TRACK02_TABLE_DECODE_OK ||
            theron_v1_track02_bind_descriptor_entry_roles(
                track02_data,
                track02_size,
                descriptor_offset,
                &table,
                entries) != THERON_TRACK02_TABLE_DECODE_OK) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }

        for (entry_index = 0u; entry_index < table.entry_count; ++entry_index) {
            const Theron_Track02DescriptorEntrySemanticBinding *entry =
                &entries[entry_index];
            Theron_Track02NonstartupSectorWindowReceipt *window;
            size_t user_data_offset = 0u;
            size_t window_index;
            size_t physical_end_offset;
            size_t raw_cursor;

            if (entry->role != THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA) {
                continue;
            }
            window_index = out_receipt->window_count[anchor_index];
            if (window_index >= THERON_TRACK02_MAX_NONSTARTUP_SECTOR_WINDOWS ||
                entry->absolute_offset > track02_size ||
                entry->byte_count > track02_size - entry->absolute_offset) {
                memset(out_receipt, 0, sizeof(*out_receipt));
                return THERON_TRACK02_SIGNAL_NOT_FOUND;
            }

            window = &out_receipt->windows[anchor_index][window_index];
            window->descriptor_entry_index = entry->entry_index;
            window->raw_offset = entry->absolute_offset;
            window->byte_count = entry->byte_count;
            window->first_raw_sector = entry->absolute_offset / TQR_RAW_SECTOR_BYTES;
            if (theron_v1_track02_raw_offset_to_user_offset(
                    entry->absolute_offset, track02_size, md5_hex,
                    &user_data_offset) == THERON_TRACK02_SIGNAL_OK) {
                window->first_raw_byte_is_user_data = 1;
                window->first_sector_user_data_offset =
                    entry->absolute_offset % TQR_RAW_SECTOR_BYTES -
                    TQR_RAW_SECTOR_USER_DATA_OFFSET;
                window->user_data_offset = user_data_offset;
            }
            physical_end_offset = entry->absolute_offset + entry->byte_count - 1u;
            window->last_raw_sector = physical_end_offset / TQR_RAW_SECTOR_BYTES;
            window->last_sector_user_data_offset = physical_end_offset %
                TQR_RAW_SECTOR_BYTES;
            window->crosses_raw_sector_boundary =
                window->first_raw_sector != window->last_raw_sector;
            window->raw_span_contains_non_user_data = 0;
            for (raw_cursor = entry->absolute_offset;
                 raw_cursor <= physical_end_offset;
                 ++raw_cursor) {
                size_t within = raw_cursor % TQR_RAW_SECTOR_BYTES;
                if (within < TQR_RAW_SECTOR_USER_DATA_OFFSET ||
                    within >= TQR_RAW_SECTOR_USER_DATA_OFFSET +
                        TQR_RAW_SECTOR_USER_DATA_BYTES) {
                    window->raw_span_contains_non_user_data = 1;
                    break;
                }
            }
            window->user_data_span_contiguous =
                window->raw_span_contains_non_user_data ? 0 : 1;
            if (window->user_data_span_contiguous) {
                window->user_data_end_offset = user_data_offset + entry->byte_count - 1u;
                window->last_sector_user_data_offset =
                    window->user_data_end_offset % TQR_RAW_SECTOR_USER_DATA_BYTES;
            }
            window->raw_span_hash = tqr_hash_bytes(
                track02_data + entry->absolute_offset, entry->byte_count);
            window->opaque = 1;
            window->promotion_blocked = 1;
            ++out_receipt->window_count[anchor_index];

            receipt_hash = tqr_receipt_hash_add_u64(
                receipt_hash, (uint64_t)anchor_index);
            receipt_hash = tqr_receipt_hash_add_u64(
                receipt_hash, (uint64_t)window->descriptor_entry_index);
            receipt_hash = tqr_receipt_hash_add_u64(
                receipt_hash, (uint64_t)window->raw_offset);
            receipt_hash = tqr_receipt_hash_add_u64(
                receipt_hash, (uint64_t)window->user_data_offset);
            receipt_hash = tqr_receipt_hash_add_u64(
                receipt_hash, (uint64_t)window->raw_span_hash);
        }
    }

    out_receipt->receipt_hash = receipt_hash;
    out_receipt->valid = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

const char *theron_v1_track02_nonstartup_sector_layout_comparison_status_name(
    Theron_Track02NonstartupSectorLayoutComparisonStatus status) {

    switch (status) {
    case THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_OK:
        return "ok";
    case THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT:
        return "unverified-receipt";
    case THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR:
        return "unsupported-variant-pair";
    default:
        return "unknown";
    }
}

Theron_Track02NonstartupSectorLayoutComparisonStatus
theron_v1_track02_compare_nonstartup_sector_layout_variants(
    const Theron_Track02NonstartupSectorReceipt *first,
    const Theron_Track02NonstartupSectorReceipt *second,
    Theron_Track02NonstartupSectorLayoutComparisonReceipt *out_receipt) {
    const Theron_Track02NonstartupSectorReceipt *jp;
    const Theron_Track02NonstartupSectorReceipt *us;
    uint32_t comparison_hash = 2166136261u;
    size_t anchor;

    if (!out_receipt) {
        return THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_BAD_INPUT;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_BAD_INPUT;
    if (!first || !second) {
        return out_receipt->status;
    }
    if (!first->valid || !second->valid || !first->verified_track02 ||
        !second->verified_track02 || !first->opaque_only || !second->opaque_only ||
        !first->promotion_blocked || !second->promotion_blocked) {
        out_receipt->status =
            THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT;
        return out_receipt->status;
    }
    if (first->variant == THERON_TRACK02_VARIANT_JP_BIN &&
        second->variant == THERON_TRACK02_VARIANT_US_BIN) {
        jp = first;
        us = second;
    } else if (first->variant == THERON_TRACK02_VARIANT_US_BIN &&
               second->variant == THERON_TRACK02_VARIANT_JP_BIN) {
        jp = second;
        us = first;
    } else {
        out_receipt->status =
            THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR;
        return out_receipt->status;
    }

    out_receipt->jp_variant = jp->variant;
    out_receipt->us_variant = us->variant;
    out_receipt->opaque_only = 1;
    out_receipt->promotion_blocked = 1;
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        const unsigned int bit = 1u << (unsigned int)anchor;
        size_t window_index;
        uint32_t layout_hash = 2166136261u;
        int layout_matches = 1;
        int content_matches = 1;

        if (anchor >= jp->anchor_count || anchor >= us->anchor_count ||
            jp->window_count[anchor] == 0u ||
            jp->window_count[anchor] != us->window_count[anchor] ||
            jp->window_count[anchor] > THERON_TRACK02_MAX_NONSTARTUP_SECTOR_WINDOWS ||
            jp->descriptor_raw_offsets[anchor] == 0u ||
            us->descriptor_raw_offsets[anchor] == 0u) {
            continue;
        }
        out_receipt->comparable_anchor_mask |= bit;
        out_receipt->window_counts[anchor] = jp->window_count[anchor];
        for (window_index = 0u; window_index < jp->window_count[anchor];
             ++window_index) {
            const Theron_Track02NonstartupSectorWindowReceipt *jp_window =
                &jp->windows[anchor][window_index];
            const Theron_Track02NonstartupSectorWindowReceipt *us_window =
                &us->windows[anchor][window_index];
            const size_t jp_delta = jp_window->raw_offset -
                                    jp->descriptor_raw_offsets[anchor];
            const size_t us_delta = us_window->raw_offset -
                                    us->descriptor_raw_offsets[anchor];

            if (jp_window->descriptor_entry_index != us_window->descriptor_entry_index ||
                jp_window->byte_count != us_window->byte_count ||
                jp_delta != us_delta ||
                jp_window->first_raw_byte_is_user_data != us_window->first_raw_byte_is_user_data ||
                jp_window->crosses_raw_sector_boundary != us_window->crosses_raw_sector_boundary ||
                jp_window->raw_span_contains_non_user_data != us_window->raw_span_contains_non_user_data ||
                jp_window->user_data_span_contiguous != us_window->user_data_span_contiguous) {
                layout_matches = 0;
            }
            if (jp_window->raw_span_hash != us_window->raw_span_hash) {
                content_matches = 0;
            }
            layout_hash = tqr_receipt_hash_add_u64(
                layout_hash, (uint64_t)jp_window->descriptor_entry_index);
            layout_hash = tqr_receipt_hash_add_u64(
                layout_hash, (uint64_t)jp_window->byte_count);
            layout_hash = tqr_receipt_hash_add_u64(layout_hash, (uint64_t)jp_delta);
            layout_hash = tqr_receipt_hash_add_u64(
                layout_hash, (uint64_t)jp_window->first_raw_byte_is_user_data);
            layout_hash = tqr_receipt_hash_add_u64(
                layout_hash, (uint64_t)jp_window->crosses_raw_sector_boundary);
            layout_hash = tqr_receipt_hash_add_u64(
                layout_hash, (uint64_t)jp_window->raw_span_contains_non_user_data);
            layout_hash = tqr_receipt_hash_add_u64(
                layout_hash, (uint64_t)jp_window->user_data_span_contiguous);
        }
        if (layout_matches) {
            out_receipt->layout_matching_anchor_mask |= bit;
            out_receipt->layout_fingerprint[anchor] = layout_hash;
            if (content_matches) {
                out_receipt->content_matching_anchor_mask |= bit;
            } else {
                out_receipt->content_mismatch_anchor_mask |= bit;
            }
        }
    }
    comparison_hash = tqr_receipt_hash_add_u64(comparison_hash, out_receipt->comparable_anchor_mask);
    comparison_hash = tqr_receipt_hash_add_u64(comparison_hash, out_receipt->layout_matching_anchor_mask);
    comparison_hash = tqr_receipt_hash_add_u64(comparison_hash, out_receipt->content_matching_anchor_mask);
    comparison_hash = tqr_receipt_hash_add_u64(comparison_hash, out_receipt->content_mismatch_anchor_mask);
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        comparison_hash = tqr_receipt_hash_add_u64(
            comparison_hash, (uint64_t)out_receipt->window_counts[anchor]);
        comparison_hash = tqr_receipt_hash_add_u64(
            comparison_hash, (uint64_t)out_receipt->layout_fingerprint[anchor]);
    }
    out_receipt->comparison_hash = comparison_hash;
    out_receipt->valid = 1;
    out_receipt->status = THERON_TRACK02_NONSTARTUP_SECTOR_LAYOUT_COMPARISON_OK;
    return out_receipt->status;
}

static uint16_t rd16be(const uint8_t *p) {
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

static uint32_t rd32be(const uint8_t *p) {
    return ((uint32_t)rd16be(p) << 16) | rd16be(p + 2);
}

static int tqr_square_is_passable(uint8_t square) {
    return square != THERON_SQUARE_WALL && square != THERON_SQUARE_SECRET;
}

static void choose_initial_level_start_pose(Theron_V1_Level *level) {
    static const int dirs[4] = {1, 2, 3, 0}; /* prefer E/S/W/N for visible corridor entry */
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int y;
    int x;

    if (!level || level->width <= 2 || level->height <= 2) {
        return;
    }

    for (y = 1; y + 1 < level->height; ++y) {
        for (x = 1; x + 1 < level->width; ++x) {
            int di;
            if (level->squares[y][x] != THERON_SQUARE_FLOOR) {
                continue;
            }
            for (di = 0; di < 4; ++di) {
                int dir = dirs[di];
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                if (nx >= 0 && nx < level->width &&
                    ny >= 0 && ny < level->height &&
                    tqr_square_is_passable(level->squares[ny][nx])) {
                    level->start_x = (int16_t)x;
                    level->start_y = (int16_t)y;
                    level->start_dir = dir;
                    return;
                }
            }
        }
    }
}

static int tqr_level_candidate_header_matches(const uint8_t *bytes,
                                               size_t available,
                                               uint16_t *out_width,
                                               uint16_t *out_height,
                                               size_t *out_payload_size) {
    uint16_t width;
    uint16_t height;
    size_t grid_size;

    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (out_payload_size) *out_payload_size = 0u;
    if (!bytes || available < 12u) {
        return 0;
    }

    width = rd16be(bytes + 0);
    height = rd16be(bytes + 2);
    if (width != TQR_RAW_INITIAL_LEVEL_WIDTH ||
        height != TQR_RAW_INITIAL_LEVEL_HEIGHT ||
        rd32be(bytes + 4) != TQR_RAW_INITIAL_LEVEL_SEED ||
        rd16be(bytes + 8) != TQR_RAW_INITIAL_LEVEL_INDEX) {
        return 0;
    }

    grid_size = (size_t)width * (size_t)height;
    if (grid_size > available - 12u) {
        return 0;
    }

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    if (out_payload_size) *out_payload_size = 12u + grid_size;
    return 1;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_scan_level_candidates(
    const uint8_t *track02_data,
    size_t track02_size,
    Theron_Track02LevelCandidateCatalog *out_catalog) {

    size_t offset;

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
    }
    if (!track02_data || track02_size < 12u || !out_catalog) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    out_catalog->scanned_bytes = track02_size;
    for (offset = 0; offset + 12u <= track02_size; ++offset) {
        uint16_t width = 0u;
        uint16_t height = 0u;
        size_t payload_size = 0u;
        Theron_V1_Level level;
        Theron_MapLoadResult map_status;
        Theron_Track02LevelCandidate *candidate;

        if (!tqr_level_candidate_header_matches(track02_data + offset,
                                                track02_size - offset,
                                                &width,
                                                &height,
                                                &payload_size)) {
            continue;
        }

        map_status = theron_v1_level_load(&level,
                                          track02_data + offset,
                                          (int)payload_size,
                                          THERON_DUNGEON_1_HALL_OF_RECORDS,
                                          (int)out_catalog->candidate_count);
        if (map_status != THERON_MAP_OK) {
            continue;
        }

        if (out_catalog->candidate_count >=
            THERON_TRACK02_MAX_LEVEL_CANDIDATES) {
            ++out_catalog->overflow_count;
            continue;
        }

        candidate =
            &out_catalog->candidates[out_catalog->candidate_count++];
        candidate->absolute_offset = offset;
        candidate->byte_count = payload_size;
        candidate->header_width = width;
        candidate->header_height = height;
        candidate->header_seed = rd32be(track02_data + offset + 4u);
        candidate->header_level_index = rd16be(track02_data + offset + 8u);
        candidate->map_status = map_status;
        candidate->start_x = level.start_x;
        candidate->start_y = level.start_y;
        candidate->start_dir = level.start_dir;
        candidate->loaded = 1;
    }

    return out_catalog->candidate_count > 0u
        ? THERON_TRACK02_LEVEL_HANDOFF_OK
        : THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
}

int theron_v1_track02_bind_level_candidate_anchor(
    size_t descriptor_offset,
    Theron_Track02LevelCandidateCatalog *catalog) {

    size_t expected_offset = 0u;
    int expected_ok;

    if (!catalog) {
        return 0;
    }

    expected_ok = theron_v1_track02_initial_candidate_expected_offset(
        descriptor_offset,
        &expected_offset);
    for (size_t i = 0; i < catalog->candidate_count; ++i) {
        Theron_Track02LevelCandidate *candidate = &catalog->candidates[i];
        candidate->descriptor_delta = 0u;
        candidate->matches_initial_anchor = 0;
        if (descriptor_offset >= candidate->absolute_offset) {
            candidate->descriptor_delta =
                descriptor_offset - candidate->absolute_offset;
        }
        if (expected_ok && candidate->absolute_offset == expected_offset) {
            candidate->matches_initial_anchor = 1;
        }
    }
    return 1;
}

Theron_Track02SignalStatus theron_v1_track02_bind_level_candidate_user_offsets(
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02LevelCandidateCatalog *catalog) {

    size_t sector_count = 0u;
    size_t user_data_size = 0u;
    Theron_Track02SignalStatus status;

    if (!catalog) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    for (size_t i = 0; i < catalog->candidate_count; ++i) {
        catalog->candidates[i].user_data_offset = 0u;
        catalog->candidates[i].user_data_offset_valid = 0;
    }

    status = theron_v1_track02_raw_user_data_size(track02_size,
                                                  md5_hex,
                                                  &sector_count,
                                                  &user_data_size);
    if (status != THERON_TRACK02_SIGNAL_OK) {
        return status;
    }

    for (size_t i = 0; i < catalog->candidate_count; ++i) {
        Theron_Track02LevelCandidate *candidate = &catalog->candidates[i];
        size_t user_offset = 0u;
        Theron_Track02SignalStatus offset_status =
            theron_v1_track02_raw_offset_to_user_offset(
                candidate->absolute_offset,
                track02_size,
                md5_hex,
                &user_offset);
        if (offset_status == THERON_TRACK02_SIGNAL_OK) {
            candidate->user_data_offset = user_offset;
            candidate->user_data_offset_valid = 1;
        }
    }

    (void)sector_count;
    (void)user_data_size;
    return THERON_TRACK02_SIGNAL_OK;
}

int theron_v1_track02_initial_candidate_expected_offset(
    size_t descriptor_offset,
    size_t *out_candidate_offset) {

    if (out_candidate_offset) {
        *out_candidate_offset = 0u;
    }
    if (descriptor_offset < TQR_RAW_INITIAL_LEVEL_DESCRIPTOR_DELTA) {
        return 0;
    }
    if (out_candidate_offset) {
        *out_candidate_offset =
            descriptor_offset - TQR_RAW_INITIAL_LEVEL_DESCRIPTOR_DELTA;
    }
    return 1;
}

/* Recover the one source-locked startup payload when its bytes span two raw
 * MODE1 sectors.  The ordinary raw-image scan deliberately remains useful
 * evidence for contiguous candidates; this path is only reached when that
 * scan finds none.  JP/US layouts differ in absolute anchors but share the
 * descriptor-relative startup relation. */
static int tqr_bind_split_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    Theron_Track02InitialCandidateBinding *out_binding) {

    enum {
        payload_size = 12u + TQR_RAW_INITIAL_LEVEL_WIDTH *
            TQR_RAW_INITIAL_LEVEL_HEIGHT
    };
    uint8_t payload[payload_size];
    Theron_V1_Level level;
    Theron_MapLoadResult map_status;
    Theron_Track02SignalStatus signal_status;
    size_t candidate_offset = 0u;
    size_t user_data_offset = 0u;
    uint16_t width = 0u;
    uint16_t height = 0u;
    size_t decoded_size = 0u;

    if (!variant_is_raw_bin(theron_v1_track02_variant_for_md5(md5_hex)) ||
        !theron_v1_track02_initial_candidate_expected_offset(
            descriptor_offset, &candidate_offset) ||
        candidate_offset >= track02_size ||
        payload_size > track02_size - candidate_offset) {
        return 0;
    }

    signal_status = theron_v1_track02_copy_raw_user_data_range(
        track02_data, track02_size, md5_hex, candidate_offset,
        payload_size, payload, sizeof(payload), &user_data_offset);
    if (signal_status != THERON_TRACK02_SIGNAL_OK ||
        !tqr_level_candidate_header_matches(payload, sizeof(payload),
                                            &width, &height, &decoded_size) ||
        decoded_size != payload_size) {
        return 0;
    }

    map_status = theron_v1_level_load(&level, payload, (int)decoded_size,
                                      THERON_DUNGEON_1_HALL_OF_RECORDS, 0);
    if (map_status != THERON_MAP_OK) {
        return 0;
    }

    out_binding->candidate_count = 1u;
    out_binding->candidate_index = 0u;
    out_binding->expected_offset_valid = 1;
    out_binding->expected_offset = candidate_offset;
    out_binding->matches_initial_anchor = 1;
    out_binding->candidate.absolute_offset = candidate_offset;
    out_binding->candidate.byte_count = decoded_size;
    out_binding->candidate.header_width = width;
    out_binding->candidate.header_height = height;
    out_binding->candidate.header_seed = rd32be(payload + 4u);
    out_binding->candidate.header_level_index = rd16be(payload + 8u);
    out_binding->candidate.map_status = map_status;
    out_binding->candidate.start_x = level.start_x;
    out_binding->candidate.start_y = level.start_y;
    out_binding->candidate.start_dir = level.start_dir;
    out_binding->candidate.descriptor_delta =
        descriptor_offset - candidate_offset;
    out_binding->candidate.matches_initial_anchor = 1;
    out_binding->candidate.user_data_offset = user_data_offset;
    out_binding->candidate.user_data_offset_valid = 1;
    out_binding->candidate.loaded = 1;
    return 1;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_bind_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    Theron_Track02InitialCandidateBinding *out_binding) {

    Theron_Track02DescriptorTable table;
    Theron_Track02TableDecodeStatus table_status;
    Theron_Track02LevelCandidateCatalog catalog;
    size_t expected_candidate_offset = 0u;
    int expected_ok;
    Theron_Track02LevelHandoffStatus status =
        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;

    if (out_binding) {
        memset(out_binding, 0, sizeof(*out_binding));
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
        out_binding->candidate_index = (size_t)-1;
    }

    if (!track02_data || track02_size == 0u || !out_binding) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    out_binding->descriptor_offset = descriptor_offset;
    if (descriptor_offset < TQR_US_ISO_BANK_STRIDE_OFFSET ||
        descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
        return out_binding->status;
    }

    table_status = theron_v1_track02_decode_descriptor_table(
        track02_data + descriptor_offset,
        TQR_US_ISO_BANK_STRIDE_BYTES,
        TQR_US_ISO_BANK_STRIDE_STEP,
        &table);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
        return out_binding->status;
    }

    status = theron_v1_track02_scan_level_candidates(track02_data,
                                                     track02_size,
                                                     &catalog);
    out_binding->candidate_count = catalog.candidate_count;
    if (status != THERON_TRACK02_LEVEL_HANDOFF_OK) {
        if (status == THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
            tqr_bind_split_initial_level_candidate(track02_data,
                                                    track02_size,
                                                    md5_hex,
                                                    descriptor_offset,
                                                    out_binding)) {
            out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_OK;
            return out_binding->status;
        }
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }
    if (catalog.candidate_count > 1u) {
        out_binding->status =
            THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES;
        return out_binding->status;
    }
    if (catalog.candidate_count != 1u) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }

    theron_v1_track02_bind_level_candidate_anchor(descriptor_offset, &catalog);
    (void)theron_v1_track02_bind_level_candidate_user_offsets(track02_size,
                                                              md5_hex,
                                                              &catalog);
    expected_ok = theron_v1_track02_initial_candidate_expected_offset(
        descriptor_offset,
        &expected_candidate_offset);
    out_binding->expected_offset_valid = expected_ok ? 1 : 0;
    out_binding->expected_offset = expected_candidate_offset;
    out_binding->candidate_index = 0u;
    out_binding->candidate = catalog.candidates[0];
    out_binding->matches_initial_anchor =
        catalog.candidates[0].matches_initial_anchor;

    if (!expected_ok ||
        catalog.candidates[0].absolute_offset != expected_candidate_offset ||
        !catalog.candidates[0].matches_initial_anchor) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }

    /* Normalize the promoted raw-BIN payload through MODE1 user data even
     * when the raw scanner could see its header.  That makes the semantic
     * handoff insensitive to JP/US sector framing at the payload tail. */
    if (variant_is_raw_bin(theron_v1_track02_variant_for_md5(md5_hex)) &&
        !tqr_bind_split_initial_level_candidate(track02_data,
                                                 track02_size,
                                                 md5_hex,
                                                 descriptor_offset,
                                                 out_binding)) {
        out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_binding->status;
    }

    out_binding->status = THERON_TRACK02_LEVEL_HANDOFF_OK;
    return out_binding->status;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_copy_initial_level_user_data_window(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    uint8_t *out_bytes,
    size_t out_bytes_capacity,
    size_t *out_byte_count,
    size_t *out_user_data_offset) {

    Theron_Track02InitialCandidateBinding binding;
    Theron_Track02LevelHandoffStatus status;
    size_t user_data_offset = 0u;

    if (out_byte_count) {
        *out_byte_count = 0u;
    }
    if (out_user_data_offset) {
        *out_user_data_offset = 0u;
    }
    if (!track02_data || track02_size == 0u || !out_bytes ||
        !out_byte_count || !out_user_data_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    status = theron_v1_track02_bind_initial_level_candidate(track02_data,
                                                            track02_size,
                                                            md5_hex,
                                                            descriptor_offset,
                                                            &binding);
    if (status != THERON_TRACK02_LEVEL_HANDOFF_OK) {
        return status;
    }
    if (!binding.candidate.user_data_offset_valid ||
        binding.candidate.absolute_offset > track02_size ||
        binding.candidate.byte_count >
            track02_size - binding.candidate.absolute_offset ||
        out_bytes_capacity < binding.candidate.byte_count) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    if (theron_v1_track02_copy_raw_user_data_range(
            track02_data,
            track02_size,
            md5_hex,
            binding.candidate.absolute_offset,
            binding.candidate.byte_count,
            out_bytes,
            out_bytes_capacity,
            &user_data_offset) != THERON_TRACK02_SIGNAL_OK ||
        user_data_offset != binding.candidate.user_data_offset) {
            return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    *out_byte_count = binding.candidate.byte_count;
    *out_user_data_offset = user_data_offset;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_load_descriptor_window_level(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    size_t entry_index,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02LevelHandoff *out_handoff) {

    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorWindowBinding binding;
    const Theron_Track02DescriptorWindow *window;
    Theron_Track02TableDecodeStatus table_status;
    const uint8_t *level_bytes;
    Theron_MapLoadResult map_status;

    if (out_handoff) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        out_handoff->map_status = THERON_MAP_ERR_NULL;
    }
    if (out_level) {
        memset(out_level, 0, sizeof(*out_level));
    }

    if (!track02_data || track02_size == 0 || !out_level || !out_handoff) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }
    if (entry_index >= THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }
    if (descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
    }

    table_status = theron_v1_track02_decode_descriptor_table(
        track02_data + descriptor_offset,
        TQR_US_ISO_BANK_STRIDE_BYTES,
        TQR_US_ISO_BANK_STRIDE_STEP,
        &table);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
    }

    table_status = theron_v1_track02_bind_descriptor_windows(
        track02_data,
        track02_size,
        descriptor_offset,
        &table,
        &binding);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND;
    }

    window = &binding.windows[entry_index];
    out_handoff->entry_index = entry_index;
    out_handoff->absolute_offset = window->absolute_offset;
    out_handoff->byte_count = window->byte_count;
    out_handoff->window_kind = window->kind;

    if (window->kind != THERON_TRACK02_DESCRIPTOR_WINDOW_DATA) {
        return THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA;
    }
    if (window->byte_count < 12u ||
        window->absolute_offset > track02_size ||
        window->byte_count > track02_size - window->absolute_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    level_bytes = track02_data + window->absolute_offset;
    out_handoff->header_width = rd16be(level_bytes + 0);
    out_handoff->header_height = rd16be(level_bytes + 2);
    out_handoff->header_seed = rd32be(level_bytes + 4);
    out_handoff->header_level_index = rd16be(level_bytes + 8);

    /* Handoff target: theron_v1_world.c T560-shaped 12-byte header + grid
     * loader.  This keeps Track 02 selection separate from world parsing
     * until real dungeon-window semantics are known. */
    map_status = theron_v1_level_load(out_level,
                                      level_bytes,
                                      (int)window->byte_count,
                                      dungeon_id,
                                      sub_level_index);
    out_handoff->map_status = map_status;
    if (map_status != THERON_MAP_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED;
    }

    out_handoff->loaded = 1;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_load_initial_level_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02LevelHandoff *out_handoff) {

    Theron_Track02LevelHandoffStatus binding_status;
    Theron_Track02InitialCandidateBinding binding;
    const Theron_Track02LevelCandidate *candidate;
    const uint8_t *level_bytes;
    Theron_MapLoadResult map_status;

    if (out_handoff) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        out_handoff->map_status = THERON_MAP_ERR_NULL;
    }
    if (out_level) {
        memset(out_level, 0, sizeof(*out_level));
    }

    if (!track02_data || track02_size == 0u || !out_level || !out_handoff) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }
    binding_status = theron_v1_track02_bind_initial_level_candidate(
        track02_data,
        track02_size,
        md5_hex,
        descriptor_offset,
        &binding);
    out_handoff->binding_status = (int32_t)binding_status;
    out_handoff->candidate_count = binding.candidate_count;
    out_handoff->expected_offset = binding.expected_offset;
    out_handoff->matches_initial_anchor = binding.matches_initial_anchor;
    if (binding.candidate_count == 1u) {
        out_handoff->descriptor_delta = binding.candidate.descriptor_delta;
        out_handoff->user_data_offset = binding.candidate.user_data_offset;
        out_handoff->user_data_offset_valid =
            binding.candidate.user_data_offset_valid;
    }
    if (binding_status != THERON_TRACK02_LEVEL_HANDOFF_OK) {
        return binding_status;
    }

    candidate = &binding.candidate;
    if (candidate->absolute_offset > track02_size ||
        candidate->byte_count > track02_size - candidate->absolute_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    level_bytes = track02_data + candidate->absolute_offset;
    out_handoff->entry_index = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES;
    out_handoff->absolute_offset = candidate->absolute_offset;
    out_handoff->byte_count = candidate->byte_count;
    out_handoff->window_kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
    out_handoff->header_width = rd16be(level_bytes + 0);
    out_handoff->header_height = rd16be(level_bytes + 2);
    out_handoff->header_seed = rd32be(level_bytes + 4);
    out_handoff->header_level_index = rd16be(level_bytes + 8);

    /* Real raw JP/US Track 02 candidate gate.  These four header fields
     * are identical in the hash-verified JP and US raw BINs at
     * descriptor_base - 0x92ce.  They deliberately keep this handoff
     * narrower than a broad scan, avoiding tiny false-positive headers
     * that also exist near the same bank. */
    if (out_handoff->header_width != TQR_RAW_INITIAL_LEVEL_WIDTH ||
        out_handoff->header_height != TQR_RAW_INITIAL_LEVEL_HEIGHT ||
        out_handoff->header_seed != TQR_RAW_INITIAL_LEVEL_SEED ||
        out_handoff->header_level_index != TQR_RAW_INITIAL_LEVEL_INDEX) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    map_status = theron_v1_level_load(out_level,
                                      level_bytes,
                                      (int)candidate->byte_count,
                                      dungeon_id,
                                      sub_level_index);
    out_handoff->map_status = map_status;
    if (map_status != THERON_MAP_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED;
    }

    choose_initial_level_start_pose(out_level);
    out_handoff->loaded = 1;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}

const char *theron_v1_track02_level_handoff_status_name(
    Theron_Track02LevelHandoffStatus status) {
    switch (status) {
    case THERON_TRACK02_LEVEL_HANDOFF_OK:
        return "ok";
    case THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL:
        return "no-level";
    case THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_LEVEL_HANDOFF_TABLE_NOT_FOUND:
        return "table-not-found";
    case THERON_TRACK02_LEVEL_HANDOFF_WINDOW_NOT_DATA:
        return "window-not-data";
    case THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED:
        return "level-load-failed";
    case THERON_TRACK02_LEVEL_HANDOFF_AMBIGUOUS_CANDIDATES:
        return "ambiguous-candidates";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_table_decode_status_name(
    Theron_Track02TableDecodeStatus status) {
    switch (status) {
    case THERON_TRACK02_TABLE_DECODE_OK:
        return "ok";
    case THERON_TRACK02_TABLE_DECODE_NOT_FOUND:
        return "not-found";
    case THERON_TRACK02_TABLE_DECODE_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_TABLE_DECODE_WRONG_ENTRY_COUNT:
        return "wrong-entry-count";
    case THERON_TRACK02_TABLE_DECODE_NOT_STRICTLY_ASCENDING:
        return "not-strictly-ascending";
    case THERON_TRACK02_TABLE_DECODE_WRONG_STRIDE:
        return "wrong-stride";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_descriptor_window_kind_name(
    Theron_Track02DescriptorWindowKind kind) {
    switch (kind) {
    case THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL:
        return "zero-fill";
    case THERON_TRACK02_DESCRIPTOR_WINDOW_DATA:
        return "data";
    case THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE:
        return "descriptor-table";
    case THERON_TRACK02_DESCRIPTOR_WINDOW_UNKNOWN:
    default:
        return "unknown";
    }
}

/* ── Semantic dungeon-descriptor binding ─────────────────────────── */

/* Documented semantic bindings (see header source-locks section):
 *   entry 0 -> DUNGEON_SEED_TABLE
 *   entry 5 -> DESCRIPTOR_TABLE  (already classified structurally;
 *                                  semantic role is a re-statement of
 *                                  the existing contains_descriptor_table
 *                                  flag for the middle entry)
 *   every other entry -> UNKNOWN
 *
 * Other entries keep their structural classification but no semantic role
 * until real Track 02 decoding promotes them. */
Theron_Track02SemanticRole theron_v1_track02_semantic_role_for_entry(
    size_t entry_index) {
    if (entry_index == 0u) return THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE;
    if (entry_index == (size_t)TQR_US_ISO_BANK_STRIDE_WINDOW_WITH_DESCRIPTOR) {
        return THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE;
    }
    return THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN;
}

const char *theron_v1_track02_semantic_role_name(
    Theron_Track02SemanticRole role) {
    switch (role) {
    case THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE:
        return "dungeon-seed-table";
    case THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE:
        return "descriptor-table";
    case THERON_TRACK02_SEMANTIC_OBJECT_TABLE:
        return "object-table";
    case THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE:
        return "level-grid-table";
    case THERON_TRACK02_SEMANTIC_TEXT_TABLE:
        return "text-table";
    case THERON_TRACK02_SEMANTIC_PALETTE_TABLE:
        return "palette-table";
    case THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_semantic_binding_status_name(
    Theron_Track02SemanticBindingStatus status) {
    switch (status) {
    case THERON_TRACK02_SEMANTIC_BINDING_OK:
        return "ok";
    case THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND:
        return "not-bound";
    case THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL:
        return "window-too-small";
    case THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE:
        return "bad-shape";
    case THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL:
        return "zero-fill";
    default:
        return "unknown";
    }
}

Theron_Track02SemanticBindingStatus theron_v1_track02_read_dungeon_seed_table(
    const uint8_t *seed_bytes,
    size_t seed_size,
    Theron_Track02DungeonSeedTable *out_table) {
    size_t i;
    int shape_ok;

    if (out_table) {
        memset(out_table, 0, sizeof(*out_table));
    }
    if (!seed_bytes || !out_table) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (seed_size < THERON_TRACK02_DUNGEON_SEED_TABLE_BYTES) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    for (i = 0; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        out_table->seeds[i] = rd32le(seed_bytes + (i * THERON_TRACK02_DUNGEON_SEED_BYTES_PER_ENTRY));
    }

    /* Strictly nonzero: every seed must be non-zero.  This rejects an
     * uninitialized/zero-filled window even if it otherwise fits. */
    shape_ok = 1;
    for (i = 0; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        if (out_table->seeds[i] == 0u) {
            shape_ok = 0;
            break;
        }
    }

    /* Non-decreasing: seeds[i+1] >= seeds[i].  This mirrors the documented
     * working-hypothesis placeholder list (313/414/527/632/749/856/967,
     * strictly ascending in the current build) while still accepting equal
     * adjacent seeds, which would be the natural shape if two dungeons
     * shared a starting seed. */
    if (shape_ok) {
        for (i = 0; i + 1u < THERON_TRACK02_DUNGEON_COUNT; ++i) {
            if (out_table->seeds[i + 1u] < out_table->seeds[i]) {
                shape_ok = 0;
                break;
            }
        }
    }

    out_table->shape_ok = shape_ok;
    return shape_ok ? THERON_TRACK02_SEMANTIC_BINDING_OK
                    : THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
}

static size_t tqr_count_nonzero_bytes(const uint8_t *bytes, size_t byte_count);
static uint32_t tqr_hash_bytes(const uint8_t *bytes, size_t byte_count);

Theron_Track02SemanticBindingStatus theron_v1_track02_read_object_table(
    const uint8_t *object_bytes,
    size_t object_size,
    Theron_Track02ObjectTable *out_table) {

    size_t record_count;
    size_t needed;
    uint32_t checksum = 2166136261u;

    if (out_table) {
        memset(out_table, 0, sizeof(*out_table));
    }
    if (!object_bytes || !out_table || object_size < 2u) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    record_count = rd16le(object_bytes);
    out_table->declared_record_count = record_count;
    if (record_count == 0u) {
        out_table->byte_count = 2u;
        out_table->required_byte_count = 2u;
        out_table->checksum = tqr_hash_bytes(object_bytes, 2u);
        out_table->reject_reason =
            THERON_TRACK02_OBJECT_TABLE_REJECT_ZERO_COUNT;
        return THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL;
    }
    if (record_count > THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) {
        out_table->overflow_count =
            record_count - THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS;
        out_table->required_byte_count =
            2u + record_count * THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES;
        out_table->byte_count = 2u;
        out_table->nonzero_byte_count = tqr_count_nonzero_bytes(object_bytes, 2u);
        out_table->checksum = tqr_hash_bytes(object_bytes, 2u);
        out_table->first_bad_record_index =
            THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS;
        out_table->reject_reason =
            THERON_TRACK02_OBJECT_TABLE_REJECT_DECLARED_OVERFLOW;
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
    }
    needed = 2u + record_count * THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES;
    out_table->required_byte_count = needed;
    if (needed > object_size) {
        out_table->reject_reason =
            THERON_TRACK02_OBJECT_TABLE_REJECT_WINDOW_TOO_SMALL;
        return THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL;
    }

    out_table->record_count = record_count;
    out_table->byte_count = needed;
    for (size_t i = 0u; i < needed; ++i) {
        if (object_bytes[i] != 0u) {
            ++out_table->nonzero_byte_count;
        }
        checksum ^= object_bytes[i];
        checksum *= 16777619u;
    }

    for (size_t i = 0u; i < record_count; ++i) {
        const uint8_t *row =
            object_bytes + 2u + i * THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES;
        Theron_Track02ObjectTableRecord *record = &out_table->records[i];
        record->object_id = row[0];
        record->kind = row[1];
        record->x = row[2];
        record->y = row[3];
        record->level_index = row[4];
        record->flags = row[5];
        record->argument = rd16le(row + 6u);

        if (record->object_id == 0u) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_ZERO_OBJECT_ID;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }
        if (record->kind == 0u) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_ZERO_KIND;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }
        if (record->x >= TQR_RAW_INITIAL_LEVEL_WIDTH) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_X_OUT_OF_RANGE;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }
        if (record->y >= TQR_RAW_INITIAL_LEVEL_HEIGHT) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_Y_OUT_OF_RANGE;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }
        if (record->level_index >= THERON_TRACK02_DUNGEON_COUNT) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_LEVEL_OUT_OF_RANGE;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }

        out_table->level_mask |= 1u << (unsigned int)record->level_index;
        if (out_table->level_record_counts[record->level_index] == 0u) {
            out_table->level_record_hashes[record->level_index] = 2166136261u;
            out_table->level_position_hashes[record->level_index] = 2166136261u;
            out_table->level_first_record_indexes[record->level_index] = i;
        }
        ++out_table->level_record_counts[record->level_index];
        out_table->level_last_record_indexes[record->level_index] = i;
        for (size_t byte_index = 0u; byte_index < 4u; ++byte_index) {
            out_table->level_position_hashes[record->level_index] ^=
                (uint8_t)(i >> (byte_index * 8u));
            out_table->level_position_hashes[record->level_index] *= 16777619u;
        }
        for (size_t byte_index = 0u;
             byte_index < THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES;
             ++byte_index) {
            out_table->level_record_hashes[record->level_index] ^= row[byte_index];
            out_table->level_record_hashes[record->level_index] *= 16777619u;
            out_table->level_position_hashes[record->level_index] ^= row[byte_index];
            out_table->level_position_hashes[record->level_index] *= 16777619u;
        }
    }

    out_table->checksum = checksum;
    out_table->shape_ok = 1;
    return THERON_TRACK02_SEMANTIC_BINDING_OK;
}

const char *theron_v1_track02_dungeon_route_status_name(
    Theron_Track02DungeonRouteStatus status) {

    switch (status) {
    case THERON_TRACK02_DUNGEON_ROUTE_OK: return "ok";
    case THERON_TRACK02_DUNGEON_ROUTE_NOT_FOUND: return "not-found";
    case THERON_TRACK02_DUNGEON_ROUTE_BAD_INPUT: return "bad-input";
    case THERON_TRACK02_DUNGEON_ROUTE_LEVEL_REJECTED: return "level-rejected";
    case THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED: return "object-rejected";
    case THERON_TRACK02_DUNGEON_ROUTE_BITMAP_REJECTED: return "bitmap-rejected";
    default: return "unknown";
    }
}

const char *theron_v1_track02_route_catalog_status_name(
    Theron_Track02RouteCatalogStatus status) {

    switch (status) {
    case THERON_TRACK02_ROUTE_CATALOG_OK: return "ok";
    case THERON_TRACK02_ROUTE_CATALOG_NOT_FOUND: return "not-found";
    case THERON_TRACK02_ROUTE_CATALOG_BAD_INPUT: return "bad-input";
    case THERON_TRACK02_ROUTE_CATALOG_ROUTE_REJECTED: return "route-rejected";
    case THERON_TRACK02_ROUTE_CATALOG_DUNGEON_MISMATCH: return "dungeon-mismatch";
    case THERON_TRACK02_ROUTE_CATALOG_DUPLICATE_LEVEL: return "duplicate-level";
    case THERON_TRACK02_ROUTE_CATALOG_NONCONTIGUOUS: return "noncontiguous";
    default: return "unknown";
    }
}

Theron_Track02RouteCatalogStatus theron_v1_track02_select_dungeon_route(
    const Theron_Track02DungeonRoute *routes,
    size_t route_count,
    int dungeon_id,
    int level_index,
    const Theron_Track02DungeonRoute **out_route,
    Theron_Track02RouteCatalogReceipt *out_receipt) {

    Theron_Track02RouteCatalogStatus status = THERON_TRACK02_ROUTE_CATALOG_BAD_INPUT;
    unsigned int level_mask = 0u;
    uint32_t checksum = 2166136261u;
    size_t matching_route_count = 0u;
    size_t i;

    if (out_route) *out_route = NULL;
    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = status;
        out_receipt->dungeon_id = dungeon_id;
        out_receipt->requested_level_index = level_index;
        out_receipt->route_count = route_count;
    }
    if (!routes || !out_route || route_count == 0u ||
        route_count > THERON_MAX_LEVELS_PER_DUNGEON ||
        dungeon_id < 1 || dungeon_id > (int)THERON_DUNGEON_COUNT ||
        level_index < 0 || level_index >= THERON_MAX_LEVELS_PER_DUNGEON) {
        return status;
    }

    for (i = 0u; i < route_count; ++i) {
        const Theron_Track02DungeonRoute *route = &routes[i];
        unsigned int level_bit;
        if (!route->valid || route->status != THERON_TRACK02_DUNGEON_ROUTE_OK) {
            status = THERON_TRACK02_ROUTE_CATALOG_ROUTE_REJECTED;
            goto finish;
        }
        if (route->dungeon_id != dungeon_id) {
            status = THERON_TRACK02_ROUTE_CATALOG_DUNGEON_MISMATCH;
            goto finish;
        }
        if (route->level_index < 0 ||
            route->level_index >= THERON_MAX_LEVELS_PER_DUNGEON) {
            status = THERON_TRACK02_ROUTE_CATALOG_ROUTE_REJECTED;
            goto finish;
        }
        level_bit = 1u << (unsigned int)route->level_index;
        if ((level_mask & level_bit) != 0u) {
            status = THERON_TRACK02_ROUTE_CATALOG_DUPLICATE_LEVEL;
            goto finish;
        }
        level_mask |= level_bit;
        checksum ^= route->checksum;
        checksum *= 16777619u;
        if (route->level_index == level_index) {
            ++matching_route_count;
            *out_route = route;
        }
    }

    if (level_mask != ((1u << (unsigned int)route_count) - 1u)) {
        status = THERON_TRACK02_ROUTE_CATALOG_NONCONTIGUOUS;
        *out_route = NULL;
        goto finish;
    }
    status = matching_route_count == 1u ? THERON_TRACK02_ROUTE_CATALOG_OK
                                         : THERON_TRACK02_ROUTE_CATALOG_NOT_FOUND;
    if (status != THERON_TRACK02_ROUTE_CATALOG_OK) *out_route = NULL;

finish:
    if (status != THERON_TRACK02_ROUTE_CATALOG_OK) {
        *out_route = NULL;
    }
    if (out_receipt) {
        out_receipt->status = status;
        out_receipt->level_mask = level_mask;
        out_receipt->matching_route_count = matching_route_count;
        out_receipt->catalog_checksum = checksum;
        out_receipt->selected = status == THERON_TRACK02_ROUTE_CATALOG_OK;
    }
    return status;
}

const char *theron_v1_track02_level_transition_status_name(
    Theron_Track02LevelTransitionStatus status) {

    switch (status) {
    case THERON_TRACK02_LEVEL_TRANSITION_OK: return "ok";
    case THERON_TRACK02_LEVEL_TRANSITION_NOT_PENDING: return "not-pending";
    case THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT: return "bad-input";
    case THERON_TRACK02_LEVEL_TRANSITION_SOURCE_REJECTED: return "source-rejected";
    case THERON_TRACK02_LEVEL_TRANSITION_TARGET_REJECTED: return "target-rejected";
    case THERON_TRACK02_LEVEL_TRANSITION_TARGET_MISMATCH: return "target-mismatch";
    default: return "unknown";
    }
}

Theron_Track02LevelTransitionStatus theron_v1_track02_apply_level_transition(
    Theron_V1_World *world,
    const Theron_Track02DungeonRoute *source_route,
    const Theron_Track02DungeonRoute *target_route,
    Theron_Track02LevelTransitionReceipt *out_receipt) {

    Theron_Track02LevelTransitionStatus status;
    int dungeon_slot;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT;
    }
    if (!world || !source_route || !target_route) {
        return THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT;
    }
    if (!world->transition_pending ||
        world->transition_type != THERON_TRANSITION_STAIRS) {
        status = THERON_TRACK02_LEVEL_TRANSITION_NOT_PENDING;
    } else if (world->current_dungeon < 1 ||
               world->current_dungeon > (int)THERON_DUNGEON_COUNT ||
               world->current_level < 0 ||
               world->current_level >= THERON_MAX_LEVELS_PER_DUNGEON ||
               !world->level_loaded[world->current_dungeon - 1]
                                   [world->current_level] ||
               !source_route->valid ||
               source_route->status != THERON_TRACK02_DUNGEON_ROUTE_OK ||
               source_route->dungeon_id != world->current_dungeon ||
               source_route->level_index != world->current_level) {
        status = THERON_TRACK02_LEVEL_TRANSITION_SOURCE_REJECTED;
    } else if (!target_route->valid ||
               target_route->status != THERON_TRACK02_DUNGEON_ROUTE_OK) {
        status = THERON_TRACK02_LEVEL_TRANSITION_TARGET_REJECTED;
    } else if (target_route->dungeon_id != world->current_dungeon ||
               target_route->level_index != world->transition_target_level ||
               target_route->level_index < 0 ||
               target_route->level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
               target_route->level.width <= 0 || target_route->level.height <= 0 ||
               target_route->level.start_x < 0 || target_route->level.start_y < 0 ||
               target_route->level.start_x >= target_route->level.width ||
               target_route->level.start_y >= target_route->level.height) {
        status = THERON_TRACK02_LEVEL_TRANSITION_TARGET_MISMATCH;
    } else {
        dungeon_slot = world->current_dungeon - 1;
        if (dungeon_slot < 0 || dungeon_slot >= (int)THERON_DUNGEON_COUNT) {
            status = THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT;
        } else {
            world->levels[dungeon_slot][target_route->level_index] =
                target_route->level;
            world->level_loaded[dungeon_slot][target_route->level_index] = 1;
            world->current_level = target_route->level_index;
            world->transition_pending = 0;
            theron_v1_world_runtime_media_invalidate_cache(world);
            status = THERON_TRACK02_LEVEL_TRANSITION_OK;
        }
    }

    if (out_receipt) {
        out_receipt->status = status;
        out_receipt->dungeon_id = world->current_dungeon;
        out_receipt->source_level_index = source_route->level_index;
        out_receipt->target_level_index = target_route->level_index;
        out_receipt->target_object_record_count = target_route->objects.record_count;
        out_receipt->source_route_checksum = source_route->checksum;
        out_receipt->target_route_checksum = target_route->checksum;
        out_receipt->applied = status == THERON_TRACK02_LEVEL_TRANSITION_OK;
    }
    return status;
}

Theron_Track02LevelTransitionStatus
theron_v1_track02_apply_level_transition_from_catalog(
    Theron_V1_World *world,
    const Theron_Track02DungeonRoute *routes,
    size_t route_count,
    Theron_Track02LevelTransitionReceipt *out_receipt,
    Theron_Track02RouteCatalogReceipt *out_source_receipt,
    Theron_Track02RouteCatalogReceipt *out_target_receipt) {

    const Theron_Track02DungeonRoute *source_route = NULL;
    const Theron_Track02DungeonRoute *target_route = NULL;
    Theron_Track02RouteCatalogStatus source_status;
    Theron_Track02RouteCatalogStatus target_status;
    Theron_Track02LevelTransitionStatus status;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT;
    }
    if (!world) return THERON_TRACK02_LEVEL_TRANSITION_BAD_INPUT;
    if (!world->transition_pending ||
        world->transition_type != THERON_TRANSITION_STAIRS) {
        status = THERON_TRACK02_LEVEL_TRANSITION_NOT_PENDING;
        goto finish;
    }

    source_status = theron_v1_track02_select_dungeon_route(
        routes, route_count, world->current_dungeon, world->current_level,
        &source_route, out_source_receipt);
    if (source_status != THERON_TRACK02_ROUTE_CATALOG_OK) {
        status = THERON_TRACK02_LEVEL_TRANSITION_SOURCE_REJECTED;
        goto finish;
    }
    target_status = theron_v1_track02_select_dungeon_route(
        routes, route_count, world->current_dungeon,
        world->transition_target_level, &target_route, out_target_receipt);
    if (target_status != THERON_TRACK02_ROUTE_CATALOG_OK) {
        status = THERON_TRACK02_LEVEL_TRANSITION_TARGET_REJECTED;
        goto finish;
    }
    return theron_v1_track02_apply_level_transition(
        world, source_route, target_route, out_receipt);

finish:
    if (out_receipt) {
        out_receipt->status = status;
        out_receipt->dungeon_id = world->current_dungeon;
        out_receipt->source_level_index = world->current_level;
        out_receipt->target_level_index = world->transition_target_level;
        out_receipt->applied = 0;
    }
    return status;
}

static int tqr_bitmap_atlas_is_complete(
    const Theron_Track02StartupBitmapAtlas *atlas) {
    const unsigned int required = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    return atlas && (atlas->route_mask & required) == required &&
        atlas->route_count == THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX;
}

Theron_Track02DungeonRouteStatus theron_v1_track02_build_dungeon_route(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    const Theron_Track02StartupBitmapAtlas *bitmap_atlas,
    Theron_Track02DungeonRoute *out_route) {

    if (out_route) {
        memset(out_route, 0, sizeof(*out_route));
        out_route->status = THERON_TRACK02_DUNGEON_ROUTE_BAD_INPUT;
    }
    if (!track02_data || !out_route || descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_DUNGEON_ROUTE_BAD_INPUT;
    }
    if (!tqr_bitmap_atlas_is_complete(bitmap_atlas)) {
        out_route->status = THERON_TRACK02_DUNGEON_ROUTE_BITMAP_REJECTED;
        return out_route->status;
    }
    /* Entries 6 and 8 were disproved as object/level routes in both
     * hash-verified raw variants. No other descriptor-local layout has
     * original loader evidence, so this must not scan for a convenient
     * synthetic-looking level/object pair. */
    out_route->descriptor_offset = descriptor_offset;
    out_route->dungeon_id = dungeon_id;
    out_route->level_index = sub_level_index;
    out_route->status = THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED;
    return out_route->status;
}

Theron_Track02DungeonRouteStatus theron_v1_track02_load_verified_dungeon_route(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    int dungeon_id,
    const Theron_Track02StartupBitmapAtlas *bitmap_atlas,
    Theron_Track02DungeonRoute *out_route) {

    if (out_route) {
        memset(out_route, 0, sizeof(*out_route));
        out_route->status = THERON_TRACK02_DUNGEON_ROUTE_BAD_INPUT;
    }
    if (!track02_data || track02_size == 0u || !md5_hex || !out_route ||
        !tqr_bitmap_atlas_is_complete(bitmap_atlas)) {
        if (out_route) out_route->status = THERON_TRACK02_DUNGEON_ROUTE_BITMAP_REJECTED;
        return out_route ? out_route->status : THERON_TRACK02_DUNGEON_ROUTE_BAD_INPUT;
    }
    out_route->descriptor_offset = descriptor_offset;
    out_route->dungeon_id = dungeon_id;
    out_route->status = THERON_TRACK02_DUNGEON_ROUTE_OBJECT_REJECTED;
    return out_route->status;
}

Theron_Track02SemanticBindingStatus theron_v1_track02_bind_semantic_descriptor(
    const uint8_t *track02_data,
    size_t track02_size,
    size_t descriptor_offset,
    size_t entry_index,
    Theron_Track02SemanticBinding *out_binding) {

    Theron_Track02DescriptorTable table;
    Theron_Track02DescriptorWindowBinding binding;
    const Theron_Track02DescriptorWindow *window;
    Theron_Track02TableDecodeStatus table_status;
    Theron_Track02SemanticBindingStatus status = THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;

    if (out_binding) {
        memset(out_binding, 0, sizeof(*out_binding));
    }
    if (!track02_data || track02_size == 0 || !out_binding) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (entry_index >= THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (descriptor_offset > track02_size ||
        TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    out_binding->entry_index = entry_index;
    out_binding->role = theron_v1_track02_semantic_role_for_entry(entry_index);
    if (out_binding->role == THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN) {
        out_binding->status = THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
        return THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
    }

    /* Decode the table once so the entry window absolute offsets can be
     * derived from a verified-shape descriptor. */
    table_status = theron_v1_track02_decode_descriptor_table(
        track02_data + descriptor_offset,
        TQR_US_ISO_BANK_STRIDE_BYTES,
        TQR_US_ISO_BANK_STRIDE_STEP,
        &table);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        out_binding->status = THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    table_status = theron_v1_track02_bind_descriptor_windows(
        track02_data,
        track02_size,
        descriptor_offset,
        &table,
        &binding);
    if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
        out_binding->status = THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
        return THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }

    window = &binding.windows[entry_index];
    out_binding->absolute_offset = window->absolute_offset;
    out_binding->byte_count = window->byte_count;
    out_binding->window_kind = window->kind;

    /* Role-specific decode. DUNGEON_SEED_TABLE reads a value payload;
     * DESCRIPTOR_TABLE binds the descriptor-bearing window. No descriptor
     * entry is currently bound as an object table. */
    if (out_binding->role == THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE) {
        if (window->kind == THERON_TRACK02_DESCRIPTOR_WINDOW_ZERO_FILL) {
            status = THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL;
        } else if (window->byte_count <
                   THERON_TRACK02_DUNGEON_SEED_TABLE_BYTES) {
            status = THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL;
        } else {
            status = theron_v1_track02_read_dungeon_seed_table(
                track02_data + window->absolute_offset,
                window->byte_count,
                &out_binding->dungeon_seed_table);
        }
    } else if (out_binding->role == THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE) {
        status = window->kind == THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE
            ? THERON_TRACK02_SEMANTIC_BINDING_OK
            : THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
    } else {
        status = THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
    }

    out_binding->status = status;
    return status;
}

static int theron_v1_track02_object_table_shape_score(
    Theron_Track02SemanticBindingStatus status,
    const Theron_Track02ObjectTable *table) {

    if (!table) {
        return 0;
    }
    if (status == THERON_TRACK02_SEMANTIC_BINDING_OK) {
        return 5;
    }
    if (status == THERON_TRACK02_SEMANTIC_BINDING_WINDOW_TOO_SMALL &&
        table->record_count > 0u &&
        table->record_count <= THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) {
        return 4;
    }
    if (status == THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
        table->record_count > 0u &&
        table->record_count <= THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) {
        return 3;
    }
    if (status == THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
        table->overflow_count > 0u) {
        return 2;
    }
    if (status == THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL) {
        return 1;
    }
    return 0;
}

static void theron_v1_track02_object_table_note_shape_candidate(
    Theron_Track02ObjectTableRouteReceipt *receipt,
    size_t anchor,
    size_t entry_index,
    size_t raw_offset,
    size_t window_offset,
    size_t user_data_offset,
    int user_data_valid,
    Theron_Track02SemanticBindingStatus status,
    const Theron_Track02ObjectTable *table) {

    int score;

    if (!receipt || !table || anchor >= THERON_TRACK02_MAX_BANK_ANCHORS) {
        return;
    }
    score = theron_v1_track02_object_table_shape_score(status, table);
    if (score <= receipt->object_table_shape_best_score[anchor]) {
        return;
    }
    receipt->object_table_shape_best_score[anchor] = score;
    receipt->object_table_shape_best_status[anchor] = (int)status;
    receipt->object_table_shape_best_entry_index[anchor] = entry_index;
    receipt->object_table_shape_best_raw_offsets[anchor] = raw_offset;
    receipt->object_table_shape_best_window_offsets[anchor] = window_offset;
    receipt->object_table_shape_best_user_data_offsets[anchor] =
        user_data_offset;
    receipt->object_table_shape_best_user_data_valid[anchor] =
        user_data_valid ? 1 : 0;
    receipt->object_table_shape_best_record_counts[anchor] =
        table->record_count;
    receipt->object_table_shape_best_overflow_counts[anchor] =
        table->overflow_count;
    receipt->object_table_shape_best_byte_counts[anchor] = table->byte_count;
    receipt->object_table_shape_best_checksums[anchor] = table->checksum;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_bind_startup_semantic_handoff(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    Theron_Track02StartupSemanticHandoff *out_handoff) {

    Theron_Track02LevelHandoffStatus candidate_status;
    size_t i;

    if (out_handoff) {
        memset(out_handoff, 0, sizeof(*out_handoff));
        out_handoff->status = THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
        out_handoff->descriptor_offset = descriptor_offset;
        out_handoff->startup_seed_table_index = (size_t)-1;
    }
    if (!track02_data || track02_size == 0u || !out_handoff) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    out_handoff->seed_table_status =
        theron_v1_track02_bind_semantic_descriptor(
            track02_data,
            track02_size,
            descriptor_offset,
            0u,
            &out_handoff->seed_table_binding);

    candidate_status = theron_v1_track02_bind_initial_level_candidate(
        track02_data,
        track02_size,
        md5_hex,
        descriptor_offset,
        &out_handoff->initial_candidate);

    out_handoff->status = candidate_status;
    if (candidate_status != THERON_TRACK02_LEVEL_HANDOFF_OK) {
        return candidate_status;
    }

    out_handoff->startup_seed =
        out_handoff->initial_candidate.candidate.header_seed;
    out_handoff->startup_level_index =
        out_handoff->initial_candidate.candidate.header_level_index;
    out_handoff->user_data_offset =
        out_handoff->initial_candidate.candidate.user_data_offset;
    out_handoff->user_data_offset_valid =
        out_handoff->initial_candidate.candidate.user_data_offset_valid;

    if (!out_handoff->user_data_offset_valid) {
        out_handoff->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_handoff->status;
    }
    if (out_handoff->seed_table_status !=
        THERON_TRACK02_SEMANTIC_BINDING_OK) {
        out_handoff->status = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
        return out_handoff->status;
    }

    for (i = 0u; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        if (out_handoff->seed_table_binding.dungeon_seed_table.seeds[i] ==
            out_handoff->startup_seed) {
            out_handoff->startup_seed_in_seed_table = 1;
            out_handoff->startup_seed_table_index = i;
            break;
        }
    }

    out_handoff->ready_for_runtime = 1;
    out_handoff->status = THERON_TRACK02_LEVEL_HANDOFF_OK;
    return out_handoff->status;
}

int theron_v1_track02_startup_runtime_receipt_from_handoff(
    const Theron_Track02StartupSemanticHandoff *handoff,
    Theron_Track02StartupRuntimeReceipt *out_receipt) {

    const Theron_Track02LevelCandidate *candidate;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
        out_receipt->seed_table_status =
            THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
    }
    if (!handoff || !out_receipt) {
        return 0;
    }

    candidate = &handoff->initial_candidate.candidate;
    out_receipt->status = handoff->status;
    out_receipt->seed_table_status = handoff->seed_table_status;
    out_receipt->descriptor_offset = handoff->descriptor_offset;
    out_receipt->raw_offset = candidate->absolute_offset;
    out_receipt->byte_count = candidate->byte_count;
    out_receipt->user_data_offset = handoff->user_data_offset;
    out_receipt->user_data_offset_valid = handoff->user_data_offset_valid;
    out_receipt->header_width = candidate->header_width;
    out_receipt->header_height = candidate->header_height;
    out_receipt->header_seed = candidate->header_seed;
    out_receipt->header_level_index = candidate->header_level_index;
    out_receipt->progression_seed0 =
        handoff->seed_table_binding.dungeon_seed_table.seeds[0];
    out_receipt->ready_for_runtime = handoff->ready_for_runtime ? 1 : 0;
    out_receipt->fallback_visuals_allowed =
        handoff->ready_for_runtime ? 0 : 1;
    out_receipt->valid =
        handoff->status == THERON_TRACK02_LEVEL_HANDOFF_OK &&
        handoff->seed_table_status == THERON_TRACK02_SEMANTIC_BINDING_OK &&
        handoff->ready_for_runtime &&
        handoff->user_data_offset_valid &&
        candidate->loaded;
    return out_receipt->valid ? 1 : 0;
}

void theron_v1_track02_object_table_route_receipt_init(
    Theron_Track02ObjectTableRouteReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->signal_status = THERON_TRACK02_SIGNAL_BAD_INPUT;
    receipt->variant = THERON_TRACK02_VARIANT_UNKNOWN;
    receipt->fallback_visuals_allowed = 1;
}

static size_t tqr_count_nonzero_bytes(const uint8_t *bytes, size_t byte_count) {
    size_t count = 0u;

    if (!bytes) {
        return 0u;
    }
    for (size_t i = 0u; i < byte_count; ++i) {
        if (bytes[i] != 0u) {
            ++count;
        }
    }
    return count;
}

static uint32_t tqr_hash_bytes(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;

    if (!bytes) {
        return 0u;
    }
    for (size_t i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static void tqr_profile_post_descriptor_candidate(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02DescriptorEntrySemanticBinding *entry,
    size_t *out_nonzero_byte_count,
    uint16_t *out_header_width,
    uint16_t *out_header_height,
    uint32_t *out_header_seed,
    uint16_t *out_header_level_index,
    int *out_header_matches_startup_shape,
    uint32_t *out_candidate_hash) {

    uint16_t width = 0u;
    uint16_t height = 0u;
    size_t payload_size = 0u;

    if (out_nonzero_byte_count) *out_nonzero_byte_count = 0u;
    if (out_header_width) *out_header_width = 0u;
    if (out_header_height) *out_header_height = 0u;
    if (out_header_seed) *out_header_seed = 0u;
    if (out_header_level_index) *out_header_level_index = 0u;
    if (out_header_matches_startup_shape) {
        *out_header_matches_startup_shape = 0;
    }
    if (out_candidate_hash) *out_candidate_hash = 0u;

    if (!track02_data || !entry ||
        entry->absolute_offset > track02_size ||
        entry->byte_count > track02_size - entry->absolute_offset) {
        return;
    }

    if (out_nonzero_byte_count) {
        *out_nonzero_byte_count =
            tqr_count_nonzero_bytes(track02_data + entry->absolute_offset,
                                    entry->byte_count);
    }
    if (out_candidate_hash) {
        *out_candidate_hash =
            tqr_hash_bytes(track02_data + entry->absolute_offset,
                           entry->byte_count);
    }
    if (entry->byte_count >= 12u) {
        const uint8_t *bytes = track02_data + entry->absolute_offset;
        if (out_header_width) *out_header_width = rd16be(bytes + 0u);
        if (out_header_height) *out_header_height = rd16be(bytes + 2u);
        if (out_header_seed) *out_header_seed = rd32be(bytes + 4u);
        if (out_header_level_index) {
            *out_header_level_index = rd16be(bytes + 8u);
        }
        if (out_header_matches_startup_shape &&
            tqr_level_candidate_header_matches(bytes,
                                               entry->byte_count,
                                               &width,
                                               &height,
                                               &payload_size) &&
            width == TQR_RAW_INITIAL_LEVEL_WIDTH &&
            height == TQR_RAW_INITIAL_LEVEL_HEIGHT &&
            rd32be(bytes + 4u) == TQR_RAW_INITIAL_LEVEL_SEED &&
            rd16be(bytes + 8u) == TQR_RAW_INITIAL_LEVEL_INDEX) {
            *out_header_matches_startup_shape = 1;
        }
    }
}

/* Compare only row evidence already accepted by the compact-table decoder.
 * The result is deliberately receipt-only: matching rows across descriptor
 * anchors do not establish their gameplay meaning or make them runnable. */
static void tqr_capture_object_table_level_consensus(
    Theron_Track02ObjectTableRouteReceipt *receipt) {

    size_t level_index;

    if (!receipt || receipt->descriptor_anchor_mask == 0u) {
        return;
    }

    for (level_index = 0u;
         level_index < THERON_TRACK02_DUNGEON_COUNT;
         ++level_index) {
        size_t anchor;
        size_t reference_count = 0u;
        uint32_t reference_hash = 0u;
        size_t reference_first_index = 0u;
        size_t reference_last_index = 0u;
        uint32_t reference_position_hash = 0u;
        unsigned int matching_mask = 0u;
        int have_reference = 0;

        for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
            unsigned int anchor_bit = 1u << (unsigned int)anchor;
            size_t count;
            uint32_t row_hash;
            size_t first_index;
            size_t last_index;
            uint32_t position_hash;

            if ((receipt->descriptor_anchor_mask & anchor_bit) == 0u ||
                receipt->object_table_anchor_binding_status[anchor] !=
                    THERON_TRACK02_SEMANTIC_BINDING_OK ||
                (receipt->object_table_level_mask[anchor] &
                 (1u << (unsigned int)level_index)) == 0u) {
                continue;
            }

            count = receipt->object_table_level_record_counts[anchor][level_index];
            row_hash = receipt->object_table_level_record_hashes[anchor][level_index];
            first_index = receipt->object_table_level_first_record_indexes[anchor]
                                                                            [level_index];
            last_index = receipt->object_table_level_last_record_indexes[anchor]
                                                                          [level_index];
            position_hash = receipt->object_table_level_position_hashes[anchor]
                                                                       [level_index];
            if (!have_reference) {
                reference_count = count;
                reference_hash = row_hash;
                reference_first_index = first_index;
                reference_last_index = last_index;
                reference_position_hash = position_hash;
                have_reference = 1;
            }
            if (count == reference_count && row_hash == reference_hash &&
                first_index == reference_first_index &&
                last_index == reference_last_index &&
                position_hash == reference_position_hash) {
                matching_mask |= anchor_bit;
            }
        }

        receipt->object_table_level_consensus_anchor_masks[level_index] =
            matching_mask;
        receipt->object_table_level_consensus_record_counts[level_index] =
            reference_count;
        receipt->object_table_level_consensus_record_hashes[level_index] =
            reference_hash;
        receipt->object_table_level_consensus_first_record_indexes[level_index] =
            reference_first_index;
        receipt->object_table_level_consensus_last_record_indexes[level_index] =
            reference_last_index;
        receipt->object_table_level_consensus_position_hashes[level_index] =
            reference_position_hash;
        if (have_reference &&
            matching_mask == receipt->descriptor_anchor_mask) {
            receipt->object_table_level_consensus_mask |=
                1u << (unsigned int)level_index;
        }
    }
}

int theron_v1_track02_capture_object_table_route_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02ObjectTableRouteReceipt *out_receipt) {

    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    uint32_t hash = 2166136261u;
    size_t anchor;

    if (!out_receipt) {
        return 0;
    }
    theron_v1_track02_object_table_route_receipt_init(out_receipt);
    if (!track02_data || track02_size == 0u || !md5_hex || !md5_hex[0]) {
        return 0;
    }

    out_receipt->variant = theron_v1_track02_variant_for_md5(md5_hex);
    signal_status = theron_v1_track02_find_bank_signal(track02_data,
                                                       track02_size,
                                                       md5_hex,
                                                       &signal);
    out_receipt->signal_status = signal_status;
    if (signal_status != THERON_TRACK02_SIGNAL_OK) {
        return 0;
    }

    out_receipt->verified_track02 = 1;
    out_receipt->fallback_visuals_allowed = 0;
    out_receipt->variant = signal.variant;

    for (anchor = 0u; anchor < signal.anchor_count; ++anchor) {
        Theron_Track02DescriptorTable table;
        Theron_Track02DescriptorEntrySemanticBinding
            entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
        Theron_Track02TableDecodeStatus table_status;
        size_t descriptor_offset = signal.descriptor_offsets[anchor];
        size_t entry_index;
        int anchor_object_table_ready = 0;

        if (descriptor_offset > track02_size ||
            TQR_US_ISO_BANK_STRIDE_BYTES > track02_size - descriptor_offset) {
            continue;
        }
        table_status = theron_v1_track02_decode_descriptor_table(
            track02_data + descriptor_offset,
            TQR_US_ISO_BANK_STRIDE_BYTES,
            TQR_US_ISO_BANK_STRIDE_STEP,
            &table);
        if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
            continue;
        }
        table_status = theron_v1_track02_bind_descriptor_entry_roles(
            track02_data,
            track02_size,
            descriptor_offset,
            &table,
            entries);
        if (table_status != THERON_TRACK02_TABLE_DECODE_OK) {
            continue;
        }

        out_receipt->descriptor_offsets[out_receipt->descriptor_anchor_count] =
            descriptor_offset;
        ++out_receipt->descriptor_anchor_count;
        out_receipt->descriptor_anchor_mask |= 1u << (unsigned)anchor;
        out_receipt->descriptor_entries_bound += table.entry_count;
        hash ^= (uint32_t)descriptor_offset;
        hash *= 16777619u;
        hash ^= (uint32_t)table.entry_count;
        hash *= 16777619u;

        for (entry_index = 0u; entry_index < table.entry_count; ++entry_index) {
            Theron_Track02SemanticRole semantic_role =
                theron_v1_track02_semantic_role_for_entry(entry_index);
            if ((unsigned int)semantic_role < (sizeof(unsigned int) * 8u)) {
                out_receipt->semantic_role_mask |=
                    1u << (unsigned int)semantic_role;
            }
            if (semantic_role == THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE) {
                Theron_Track02SemanticBinding binding;
                if (theron_v1_track02_bind_semantic_descriptor(
                        track02_data,
                        track02_size,
                        descriptor_offset,
                        entry_index,
                        &binding) == THERON_TRACK02_SEMANTIC_BINDING_OK) {
                    ++out_receipt->descriptor_table_semantic_count;
                    ++out_receipt->descriptor_table_semantic_anchor_count;
                    out_receipt->descriptor_table_semantic_anchor_mask |=
                        1u << (unsigned)anchor;
                }
            }
            if (semantic_role == THERON_TRACK02_SEMANTIC_OBJECT_TABLE) {
                Theron_Track02SemanticBinding binding;
                Theron_Track02SemanticBindingStatus binding_status;
                out_receipt->object_table_role_mapped = 1;
                ++out_receipt->object_table_candidate_count;
                out_receipt->object_table_candidate_anchor_mask |=
                    1u << (unsigned)anchor;
                binding_status = theron_v1_track02_bind_semantic_descriptor(
                        track02_data,
                        track02_size,
                        descriptor_offset,
                        entry_index,
                        &binding);
                if (anchor < THERON_TRACK02_MAX_BANK_ANCHORS) {
                    out_receipt->object_table_candidate_entry_index[anchor] =
                        entry_index;
                    out_receipt->object_table_candidate_raw_offsets[anchor] =
                        binding.absolute_offset;
                    if (binding.absolute_offset >= descriptor_offset) {
                        out_receipt
                            ->object_table_candidate_after_descriptor[anchor] = 1;
                        out_receipt
                            ->object_table_candidate_descriptor_delta[anchor] =
                            binding.absolute_offset - descriptor_offset;
                    }
                    out_receipt->object_table_candidate_byte_counts[anchor] =
                        binding.byte_count;
                    tqr_profile_post_descriptor_candidate(
                        track02_data,
                        track02_size,
                        &entries[entry_index],
                        &out_receipt
                             ->object_table_candidate_nonzero_byte_counts
                                 [anchor],
                        &out_receipt->object_table_candidate_header_width
                            [anchor],
                        &out_receipt->object_table_candidate_header_height
                            [anchor],
                        &out_receipt->object_table_candidate_header_seed
                            [anchor],
                        &out_receipt
                             ->object_table_candidate_header_level_index
                                 [anchor],
                        &out_receipt
                             ->object_table_candidate_header_matches_startup_shape
                                 [anchor],
                        &out_receipt->object_table_candidate_hash[anchor]);
                    out_receipt->object_table_candidate_entry_role[anchor] =
                        entries[entry_index].role;
                    out_receipt->object_table_candidate_window_kind[anchor] =
                        binding.window_kind;
                    if (theron_v1_track02_raw_offset_to_user_offset(
                            binding.absolute_offset,
                            track02_size,
                            md5_hex,
                            &out_receipt
                                 ->object_table_candidate_user_data_offsets
                                     [anchor]) == THERON_TRACK02_SIGNAL_OK) {
                        out_receipt
                            ->object_table_candidate_user_data_valid[anchor] = 1;
                    }
                    out_receipt->object_table_anchor_binding_status[anchor] =
                        (int)binding_status;
                    out_receipt->object_table_anchor_hash[anchor] =
                        ((uint32_t)descriptor_offset ^ (uint32_t)entry_index ^
                         ((uint32_t)binding_status << 24));
                    out_receipt->object_table_declared_record_count[anchor] =
                        binding.object_table.declared_record_count;
                    out_receipt->object_table_record_count[anchor] =
                        binding.object_table.record_count;
                    out_receipt->object_table_required_byte_count[anchor] =
                        binding.object_table.required_byte_count;
                    out_receipt->object_table_overflow_count[anchor] =
                        binding.object_table.overflow_count;
                    out_receipt->object_table_first_bad_record_index[anchor] =
                        binding.object_table.first_bad_record_index;
                    out_receipt->object_table_reject_reason[anchor] =
                        binding.object_table.reject_reason;
                    out_receipt->object_table_record_hash[anchor] =
                        binding.object_table.checksum;
                    out_receipt->object_table_level_mask[anchor] =
                        binding.object_table.level_mask;
                    memcpy(out_receipt->object_table_level_record_counts[anchor],
                           binding.object_table.level_record_counts,
                           sizeof(binding.object_table.level_record_counts));
                    memcpy(out_receipt->object_table_level_record_hashes[anchor],
                           binding.object_table.level_record_hashes,
                           sizeof(binding.object_table.level_record_hashes));
                    memcpy(out_receipt
                               ->object_table_level_first_record_indexes[anchor],
                           binding.object_table.level_first_record_indexes,
                           sizeof(binding.object_table.level_first_record_indexes));
                    memcpy(out_receipt
                               ->object_table_level_last_record_indexes[anchor],
                           binding.object_table.level_last_record_indexes,
                           sizeof(binding.object_table.level_last_record_indexes));
                    memcpy(out_receipt
                               ->object_table_level_position_hashes[anchor],
                           binding.object_table.level_position_hashes,
                           sizeof(binding.object_table.level_position_hashes));
                }
                if (binding_status == THERON_TRACK02_SEMANTIC_BINDING_OK) {
                    out_receipt->object_table_decode_ready = 1;
                    anchor_object_table_ready = 1;
                }
            }
            if (semantic_role == THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN &&
                entries[entry_index].role ==
                    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA &&
                entries[entry_index].byte_count > 0u) {
                Theron_Track02ObjectTable row_table;
                Theron_Track02SemanticBindingStatus row_status =
                    THERON_TRACK02_SEMANTIC_BINDING_BAD_INPUT;
                ++out_receipt->object_table_candidate_count;
                out_receipt->object_table_candidate_anchor_mask |=
                    1u << (unsigned)anchor;
                if (anchor < THERON_TRACK02_MAX_BANK_ANCHORS &&
                    out_receipt->object_table_candidate_raw_offsets[anchor] ==
                        0u) {
                    size_t user_data_offset = 0u;
                    out_receipt->object_table_candidate_entry_index[anchor] =
                        entry_index;
                    out_receipt->object_table_candidate_raw_offsets[anchor] =
                        entries[entry_index].absolute_offset;
                    if (entries[entry_index].absolute_offset >= descriptor_offset) {
                        out_receipt->object_table_candidate_after_descriptor[anchor] = 1;
                        out_receipt->object_table_candidate_descriptor_delta[anchor] =
                            entries[entry_index].absolute_offset - descriptor_offset;
                    }
                    if (theron_v1_track02_raw_offset_to_user_offset(
                            entries[entry_index].absolute_offset,
                            track02_size,
                            md5_hex,
                            &user_data_offset) == THERON_TRACK02_SIGNAL_OK) {
                        out_receipt->object_table_candidate_user_data_offsets[anchor] =
                            user_data_offset;
                        out_receipt->object_table_candidate_user_data_valid[anchor] = 1;
                    }
                    out_receipt->object_table_candidate_byte_counts[anchor] =
                        entries[entry_index].byte_count;
                    tqr_profile_post_descriptor_candidate(
                        track02_data,
                        track02_size,
                        &entries[entry_index],
                        &out_receipt
                             ->object_table_candidate_nonzero_byte_counts
                                 [anchor],
                        &out_receipt->object_table_candidate_header_width
                            [anchor],
                        &out_receipt->object_table_candidate_header_height
                            [anchor],
                        &out_receipt->object_table_candidate_header_seed
                            [anchor],
                        &out_receipt
                             ->object_table_candidate_header_level_index
                                 [anchor],
                        &out_receipt
                             ->object_table_candidate_header_matches_startup_shape
                                 [anchor],
                        &out_receipt->object_table_candidate_hash[anchor]);
                    out_receipt->object_table_candidate_entry_role[anchor] =
                        entries[entry_index].role;
                    out_receipt->object_table_candidate_window_kind[anchor] =
                        THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
                }
            }
        }
        if (!anchor_object_table_ready) {
            ++out_receipt->object_table_blocked_anchor_count;
            out_receipt->object_table_blocked_anchor_mask |=
                1u << (unsigned)anchor;
        }
    }

    out_receipt->descriptor_route_ready =
        out_receipt->descriptor_anchor_count == signal.anchor_count &&
        signal.anchor_count > 0u;
    out_receipt->blocked_for_missing_real_object_evidence =
        out_receipt->descriptor_route_ready &&
        !out_receipt->object_table_decode_ready;
    tqr_capture_object_table_level_consensus(out_receipt);
    hash ^= out_receipt->semantic_role_mask;
    hash *= 16777619u;
    hash ^= out_receipt->descriptor_table_semantic_anchor_mask;
    hash *= 16777619u;
    hash ^= out_receipt->object_table_candidate_anchor_mask;
    hash *= 16777619u;
    hash ^= out_receipt->object_table_blocked_anchor_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_row_probe_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_row_reject_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_row_bad_shape_count;
    hash *= 16777619u;
    hash ^= (uint32_t)
        out_receipt->object_table_row_window_too_small_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_row_zero_fill_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_inner_scan_probe_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_inner_scan_shaped_count;
    hash *= 16777619u;
    hash ^= out_receipt->object_table_inner_scan_shaped_anchor_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_table_row_shaped_count;
    hash *= 16777619u;
    hash ^= out_receipt->object_table_row_shaped_anchor_mask;
    hash *= 16777619u;
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        hash ^= (uint32_t)
            out_receipt->object_table_candidate_anchor_counts[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_entry_index[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_raw_offsets[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_user_data_offsets[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_user_data_valid[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_byte_counts[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->object_table_candidate_nonzero_byte_counts[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_header_width[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_header_height[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->object_table_candidate_header_seed[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->object_table_candidate_header_level_index[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt
                ->object_table_candidate_header_matches_startup_shape[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->object_table_candidate_hash[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_descriptor_delta[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_after_descriptor[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_entry_role[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_candidate_window_kind[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_anchor_binding_status[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->object_table_anchor_hash[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_declared_record_count[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_record_count[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_required_byte_count[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_overflow_count[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_first_bad_record_index[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->object_table_reject_reason[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->object_table_record_hash[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->object_table_level_mask[anchor];
        hash *= 16777619u;
        for (size_t level_index = 0u;
             level_index < THERON_TRACK02_DUNGEON_COUNT;
             ++level_index) {
            hash ^= (uint32_t)
                out_receipt->object_table_level_record_counts[anchor][level_index];
            hash *= 16777619u;
            hash ^= out_receipt->object_table_level_record_hashes[anchor][level_index];
            hash *= 16777619u;
            hash ^= (uint32_t)out_receipt
                ->object_table_level_first_record_indexes[anchor][level_index];
            hash *= 16777619u;
            hash ^= (uint32_t)out_receipt
                ->object_table_level_last_record_indexes[anchor][level_index];
            hash *= 16777619u;
            hash ^= out_receipt
                ->object_table_level_position_hashes[anchor][level_index];
            hash *= 16777619u;
        }
    }
    hash ^= out_receipt->object_table_level_consensus_mask;
    hash *= 16777619u;
    for (size_t level_index = 0u;
         level_index < THERON_TRACK02_DUNGEON_COUNT;
         ++level_index) {
        hash ^= out_receipt->object_table_level_consensus_anchor_masks[level_index];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->object_table_level_consensus_record_counts[level_index];
        hash *= 16777619u;
        hash ^= out_receipt->object_table_level_consensus_record_hashes[level_index];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt
            ->object_table_level_consensus_first_record_indexes[level_index];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt
            ->object_table_level_consensus_last_record_indexes[level_index];
        hash *= 16777619u;
        hash ^= out_receipt
            ->object_table_level_consensus_position_hashes[level_index];
        hash *= 16777619u;
    }
    hash ^= (uint32_t)out_receipt->object_table_candidate_header_probe_count;
    hash *= 16777619u;
    hash ^= (uint32_t)
        out_receipt->object_table_candidate_startup_header_shape_count;
    hash *= 16777619u;
    out_receipt->route_hash = hash;
    out_receipt->valid = out_receipt->verified_track02 &&
        out_receipt->descriptor_route_ready;
    return out_receipt->valid ? 1 : 0;
}

const char *theron_v1_track02_object_layout_comparison_status_name(
    Theron_Track02ObjectLayoutComparisonStatus status) {
    switch (status) {
    case THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_OK:
        return "ok";
    case THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT:
        return "unverified-receipt";
    case THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR:
        return "unsupported-variant-pair";
    default:
        return "unknown";
    }
}

Theron_Track02ObjectLayoutComparisonStatus
theron_v1_track02_compare_object_table_layout_variants(
    const Theron_Track02ObjectTableRouteReceipt *first,
    const Theron_Track02ObjectTableRouteReceipt *second,
    Theron_Track02ObjectLayoutComparisonReceipt *out_receipt) {
    const Theron_Track02ObjectTableRouteReceipt *jp;
    const Theron_Track02ObjectTableRouteReceipt *us;
    size_t level_index;
    uint32_t hash = 2166136261u;

    if (!out_receipt) {
        return THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_BAD_INPUT;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_BAD_INPUT;
    if (!first || !second) {
        return out_receipt->status;
    }
    if (!first->valid || !second->valid || !first->verified_track02 ||
        !second->verified_track02 ||
        !first->descriptor_route_ready ||
        !second->descriptor_route_ready || first->fallback_visuals_allowed ||
        second->fallback_visuals_allowed) {
        out_receipt->status =
            THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT;
        return out_receipt->status;
    }
    if (first->variant == THERON_TRACK02_VARIANT_JP_BIN &&
        second->variant == THERON_TRACK02_VARIANT_US_BIN) {
        jp = first;
        us = second;
    } else if (first->variant == THERON_TRACK02_VARIANT_US_BIN &&
               second->variant == THERON_TRACK02_VARIANT_JP_BIN) {
        jp = second;
        us = first;
    } else {
        out_receipt->status =
            THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR;
        return out_receipt->status;
    }

    out_receipt->jp_variant = jp->variant;
    out_receipt->us_variant = us->variant;
    out_receipt->jp_level_mask = jp->object_table_level_consensus_mask;
    out_receipt->us_level_mask = us->object_table_level_consensus_mask;
    out_receipt->comparable_level_mask = out_receipt->jp_level_mask &
                                          out_receipt->us_level_mask;
    for (level_index = 0u; level_index < THERON_TRACK02_DUNGEON_COUNT;
         ++level_index) {
        unsigned int bit = 1u << (unsigned int)level_index;
        size_t count;
        uint32_t row_hash;
        size_t first_index;
        size_t last_index;
        uint32_t position_hash;

        if ((out_receipt->comparable_level_mask & bit) == 0u) {
            continue;
        }
        count = jp->object_table_level_consensus_record_counts[level_index];
        row_hash = jp->object_table_level_consensus_record_hashes[level_index];
        first_index = jp->object_table_level_consensus_first_record_indexes[level_index];
        last_index = jp->object_table_level_consensus_last_record_indexes[level_index];
        position_hash = jp->object_table_level_consensus_position_hashes[level_index];
        if (count == us->object_table_level_consensus_record_counts[level_index] &&
            row_hash == us->object_table_level_consensus_record_hashes[level_index] &&
            first_index == us->object_table_level_consensus_first_record_indexes[level_index] &&
            last_index == us->object_table_level_consensus_last_record_indexes[level_index] &&
            position_hash == us->object_table_level_consensus_position_hashes[level_index]) {
            out_receipt->matching_level_mask |= bit;
            out_receipt->matching_record_counts[level_index] = count;
            out_receipt->matching_record_hashes[level_index] = row_hash;
            out_receipt->matching_first_record_indexes[level_index] = first_index;
            out_receipt->matching_last_record_indexes[level_index] = last_index;
            out_receipt->matching_position_hashes[level_index] = position_hash;
        } else {
            out_receipt->mismatch_level_mask |= bit;
        }
    }
    hash ^= (uint32_t)out_receipt->jp_variant;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->us_variant;
    hash *= 16777619u;
    hash ^= out_receipt->comparable_level_mask;
    hash *= 16777619u;
    hash ^= out_receipt->matching_level_mask;
    hash *= 16777619u;
    hash ^= out_receipt->mismatch_level_mask;
    hash *= 16777619u;
    for (level_index = 0u; level_index < THERON_TRACK02_DUNGEON_COUNT;
         ++level_index) {
        hash ^= (uint32_t)out_receipt->matching_record_counts[level_index];
        hash *= 16777619u;
        hash ^= out_receipt->matching_record_hashes[level_index];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->matching_first_record_indexes[level_index];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->matching_last_record_indexes[level_index];
        hash *= 16777619u;
        hash ^= out_receipt->matching_position_hashes[level_index];
        hash *= 16777619u;
    }
    out_receipt->comparison_hash = hash;
    out_receipt->valid = 1;
    out_receipt->status = THERON_TRACK02_OBJECT_LAYOUT_COMPARISON_OK;
    return out_receipt->status;
}

void theron_v1_track02_level_route_receipt_init(
    Theron_Track02LevelRouteReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->signal_status = THERON_TRACK02_SIGNAL_BAD_INPUT;
    receipt->variant = THERON_TRACK02_VARIANT_UNKNOWN;
    receipt->fallback_visuals_allowed = 1;
}

int theron_v1_track02_capture_level_route_receipt(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02LevelRouteReceipt *out_receipt) {

    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    uint32_t hash = 2166136261u;
    size_t anchor;

    if (!out_receipt) {
        return 0;
    }
    theron_v1_track02_level_route_receipt_init(out_receipt);
    if (!track02_data || track02_size == 0u || !md5_hex || !md5_hex[0]) {
        return 0;
    }

    out_receipt->variant = theron_v1_track02_variant_for_md5(md5_hex);
    signal_status = theron_v1_track02_find_bank_signal(track02_data,
                                                       track02_size,
                                                       md5_hex,
                                                       &signal);
    out_receipt->signal_status = signal_status;
    if (signal_status != THERON_TRACK02_SIGNAL_OK) {
        return 0;
    }

    out_receipt->verified_track02 = 1;
    out_receipt->fallback_visuals_allowed = 0;
    out_receipt->variant = signal.variant;
    out_receipt->descriptor_anchor_count = signal.anchor_count;
    for (anchor = 0u; anchor < signal.anchor_count; ++anchor) {
        Theron_Track02DescriptorTable table;
        Theron_Track02DescriptorEntrySemanticBinding
            entries[THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES];
        Theron_Track02TableDecodeStatus table_status;
        Theron_Track02StartupSemanticHandoff handoff;
        Theron_Track02LevelHandoffStatus handoff_status;
        Theron_Track02SemanticRole semantic_role;
        size_t descriptor_offset = signal.descriptor_offsets[anchor];
        size_t entry_index;
        int anchor_nonstartup_level_ready = 0;

        out_receipt->descriptor_anchor_mask |= 1u << (unsigned)anchor;
        hash ^= (uint32_t)descriptor_offset;
        hash *= 16777619u;

        for (entry_index = 0u;
             entry_index < THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES;
             ++entry_index) {
            semantic_role =
                theron_v1_track02_semantic_role_for_entry(entry_index);
            if ((unsigned int)semantic_role < (sizeof(unsigned int) * 8u)) {
                out_receipt->semantic_role_mask |=
                    1u << (unsigned int)semantic_role;
            }
            if (semantic_role == THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE) {
                out_receipt->level_grid_role_mapped = 1;
            }
        }
        if (descriptor_offset <= track02_size &&
            TQR_US_ISO_BANK_STRIDE_BYTES <= track02_size - descriptor_offset) {
            table_status = theron_v1_track02_decode_descriptor_table(
                track02_data + descriptor_offset,
                TQR_US_ISO_BANK_STRIDE_BYTES,
                TQR_US_ISO_BANK_STRIDE_STEP,
                &table);
            if (table_status == THERON_TRACK02_TABLE_DECODE_OK &&
                theron_v1_track02_bind_descriptor_entry_roles(
                    track02_data,
                    track02_size,
                    descriptor_offset,
                    &table,
                    entries) == THERON_TRACK02_TABLE_DECODE_OK) {
                for (entry_index = 0u; entry_index < table.entry_count;
                     ++entry_index) {
                    if (entries[entry_index].role ==
                            THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA &&
                        entries[entry_index].byte_count > 0u) {
                        Theron_Track02ObjectTable candidate_objects;
                        Theron_Track02SemanticBindingStatus
                            candidate_object_table_status;
                        uint32_t candidate_hash = 0u;
                        size_t candidate_nonzero_byte_count = 0u;
                        uint16_t candidate_header_width = 0u;
                        uint16_t candidate_header_height = 0u;
                        uint32_t candidate_header_seed = 0u;
                        uint16_t candidate_header_level_index = 0u;
                        int candidate_header_matches_startup_shape = 0;
                        size_t user_data_offset = 0u;
                        int user_data_valid = 0;
                        size_t descriptor_delta = 0u;
                        tqr_profile_post_descriptor_candidate(
                            track02_data,
                            track02_size,
                            &entries[entry_index],
                            &candidate_nonzero_byte_count,
                            &candidate_header_width,
                            &candidate_header_height,
                            &candidate_header_seed,
                            &candidate_header_level_index,
                            &candidate_header_matches_startup_shape,
                            &candidate_hash);
                        /* Original-media observation only.  A compact-row
                         * match here is not a semantic binding and cannot
                         * make this non-startup window runnable. */
                        candidate_object_table_status =
                            theron_v1_track02_read_object_table(
                                track02_data + entries[entry_index].absolute_offset,
                                entries[entry_index].byte_count,
                                &candidate_objects);
                        if (entries[entry_index].absolute_offset >=
                            descriptor_offset) {
                            descriptor_delta =
                                entries[entry_index].absolute_offset -
                                descriptor_offset;
                        }
                        if (theron_v1_track02_raw_offset_to_user_offset(
                                entries[entry_index].absolute_offset,
                                track02_size,
                                md5_hex,
                                &user_data_offset) ==
                            THERON_TRACK02_SIGNAL_OK) {
                            user_data_valid = 1;
                        }
                        ++out_receipt->nonstartup_level_candidate_count;
                        out_receipt->nonstartup_level_candidate_anchor_mask |=
                            1u << (unsigned)anchor;
                        if (anchor < THERON_TRACK02_MAX_BANK_ANCHORS) {
                            size_t sample_slot =
                                out_receipt
                                    ->nonstartup_level_candidate_sample_count
                                        [anchor];
                            if (sample_slot <
                                THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES) {
                                out_receipt
                                    ->nonstartup_level_candidate_sample_entry_index
                                        [anchor][sample_slot] = entry_index;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_raw_offsets
                                        [anchor][sample_slot] =
                                    entries[entry_index].absolute_offset;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_user_data_offsets
                                        [anchor][sample_slot] =
                                    user_data_offset;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_user_data_valid
                                        [anchor][sample_slot] =
                                    user_data_valid;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_byte_counts
                                        [anchor][sample_slot] =
                                    entries[entry_index].byte_count;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_descriptor_delta
                                        [anchor][sample_slot] =
                                    descriptor_delta;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_hash
                                        [anchor][sample_slot] =
                                    candidate_hash;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_nonzero_byte_counts
                                        [anchor][sample_slot] =
                                    candidate_nonzero_byte_count;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_header_width
                                        [anchor][sample_slot] =
                                    candidate_header_width;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_header_height
                                        [anchor][sample_slot] =
                                    candidate_header_height;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_header_seed
                                        [anchor][sample_slot] =
                                    candidate_header_seed;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_header_level_index
                                        [anchor][sample_slot] =
                                    candidate_header_level_index;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_object_table_status
                                        [anchor][sample_slot] =
                                    (int)candidate_object_table_status;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_object_table_declared_record_counts
                                        [anchor][sample_slot] =
                                    candidate_objects.declared_record_count;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_object_table_record_counts
                                        [anchor][sample_slot] =
                                    candidate_objects.record_count;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_object_table_level_masks
                                        [anchor][sample_slot] =
                                    candidate_objects.level_mask;
                                out_receipt
                                    ->nonstartup_level_candidate_sample_object_table_reject_reasons
                                        [anchor][sample_slot] =
                                    candidate_objects.reject_reason;
                            }
                            ++out_receipt
                                  ->nonstartup_level_candidate_sample_count
                                      [anchor];
                        }
                        if (anchor < THERON_TRACK02_MAX_BANK_ANCHORS &&
                            out_receipt
                                    ->nonstartup_level_candidate_raw_offsets
                                        [anchor] =
                                    entries[entry_index].absolute_offset;
                                out_receipt
                                    ->nonstartup_level_candidate_byte_counts
                                        [anchor] =
                                    entries[entry_index].byte_count;
                                out_receipt
                                    ->nonstartup_level_candidate_nonzero_byte_counts
                                        [anchor] =
                                    entries[entry_index].nonzero_byte_count;
                                out_receipt
                                    ->nonstartup_level_candidate_hashes
                                        [anchor] =
                                    tqr_fnv1a_bytes(
                                        track02_data +
                                            entries[entry_index].absolute_offset,
                                        entries[entry_index].byte_count);
                                if (entries[entry_index].absolute_offset >=
                                    descriptor_offset) {
                                    out_receipt
                                        ->nonstartup_level_candidate_after_descriptor
                                            [anchor] = 1;
                                    out_receipt
                                        ->nonstartup_level_candidate_descriptor_delta
                                            [anchor] =
                                        entries[entry_index].absolute_offset -
                                        descriptor_offset;
                                }
                                if (theron_v1_track02_raw_offset_to_user_offset(
                                        entries[entry_index].absolute_offset,
                                        track02_size,
                                        md5_hex,
                                        &user_data_offset) ==
                                    THERON_TRACK02_SIGNAL_OK) {
                                    out_receipt
                                        ->nonstartup_level_candidate_user_data_offsets
                                            [anchor] = user_data_offset;
                                    out_receipt
                                        ->nonstartup_level_candidate_user_data_valid
                                            [anchor] = 1;
                                }
                                out_receipt
                                    ->nonstartup_level_candidate_entry_role
                                        [anchor] =
                                    entries[entry_index].role;
                                out_receipt
                                    ->nonstartup_level_candidate_window_kind
                                        [anchor] =
                                    THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
                            }
                            if (entries[entry_index].byte_count >= 12u &&
                                entries[entry_index].absolute_offset <=
                                    track02_size &&
                                entries[entry_index].byte_count <=
                                    track02_size -
                                        entries[entry_index].absolute_offset) {
                                Theron_V1_Level probe_level;
                                const uint8_t *candidate =
                                    track02_data +
                                    entries[entry_index].absolute_offset;
                                Theron_MapLoadResult map_status;
                                uint16_t header_width = rd16be(candidate + 0u);
                                uint16_t header_height = rd16be(candidate + 2u);
                                uint32_t header_seed = rd32be(candidate + 4u);
                                uint16_t header_level_index =
                                    rd16be(candidate + 8u);
                                map_status = theron_v1_level_load(
                                    &probe_level,
                                    candidate,
                                    (int)entries[entry_index].byte_count,
                                    1,
                                    0);
                                ++out_receipt
                                      ->nonstartup_level_candidate_header_probe_count;
                                if (out_receipt
                                        ->nonstartup_level_candidate_anchor_counts
                                            [anchor] == 1u) {
                                    out_receipt
                                        ->nonstartup_level_candidate_header_width
                                            [anchor] = header_width;
                                    out_receipt
                                        ->nonstartup_level_candidate_header_height
                                            [anchor] = header_height;
                                    out_receipt
                                        ->nonstartup_level_candidate_header_seed
                                            [anchor] = header_seed;
                                    out_receipt
                                        ->nonstartup_level_candidate_header_level_index
                                            [anchor] = header_level_index;
                                    out_receipt
                                        ->nonstartup_level_candidate_map_status
                                            [anchor] = (int)map_status;
                                }
                                if (map_status == THERON_MAP_OK) {
                                    size_t loaded_user_data_offset = 0u;
                                    out_receipt->nonstartup_level_decode_ready =
                                        1;
                                    anchor_nonstartup_level_ready = 1;
                                    ++out_receipt->nonstartup_level_loaded_count;
                                    out_receipt->nonstartup_level_loaded_anchor_mask |=
                                        1u << (unsigned)anchor;
                                    ++out_receipt
                                          ->nonstartup_level_loaded_anchor_counts
                                              [anchor];
                                    if (out_receipt
                                            ->nonstartup_level_loaded_anchor_counts
                                                [anchor] == 1u) {
                                        out_receipt
                                            ->nonstartup_level_loaded_entry_index
                                                [anchor] = entry_index;
                                        out_receipt
                                            ->nonstartup_level_loaded_raw_offsets
                                                [anchor] =
                                            entries[entry_index].absolute_offset;
                                        out_receipt
                                            ->nonstartup_level_loaded_width
                                                [anchor] = header_width;
                                        out_receipt
                                            ->nonstartup_level_loaded_height
                                                [anchor] = header_height;
                                        out_receipt
                                            ->nonstartup_level_loaded_seed
                                                [anchor] = header_seed;
                                        out_receipt
                                            ->nonstartup_level_loaded_level_index
                                                [anchor] = header_level_index;
                                        if (theron_v1_track02_raw_offset_to_user_offset(
                                                entries[entry_index]
                                                    .absolute_offset,
                                                track02_size,
                                                md5_hex,
                                                &loaded_user_data_offset) ==
                                            THERON_TRACK02_SIGNAL_OK) {
                                            out_receipt
                                                ->nonstartup_level_loaded_user_data_offsets
                                                    [anchor] =
                                                loaded_user_data_offset;
                                            out_receipt
                                                ->nonstartup_level_loaded_user_data_valid
                                                    [anchor] = 1;
                                        }
                                    }
                                } else {
                                    ++out_receipt
                                          ->nonstartup_level_candidate_loader_reject_count;
                                }
                                if (map_status != THERON_MAP_OK) {
                                    const size_t max_scan =
                                        entries[entry_index].byte_count < 0x80u
                                            ? entries[entry_index].byte_count
                                            : 0x80u;
                                    size_t inner_offset;
                                    for (inner_offset = 2u;
                                         inner_offset + 12u <= max_scan;
                                         inner_offset += 2u) {
                                        Theron_V1_Level inner_level;
                                        const uint8_t *inner_candidate =
                                            track02_data +
                                            entries[entry_index].absolute_offset +
                                            inner_offset;
                                        Theron_MapLoadResult inner_status;
                                        uint16_t inner_width =
                                            rd16be(inner_candidate + 0u);
                                        uint16_t inner_height =
                                            rd16be(inner_candidate + 2u);
                                        uint32_t inner_seed =
                                            rd32be(inner_candidate + 4u);
                                        uint16_t inner_level_index =
                                            rd16be(inner_candidate + 8u);
                                        inner_status = theron_v1_level_load(
                                            &inner_level,
                                            inner_candidate,
                                            (int)(entries[entry_index]
                                                      .byte_count -
                                                  inner_offset),
                                            1,
                                            0);
                                        ++out_receipt
                                              ->nonstartup_level_inner_scan_probe_count;
                                        if (anchor <
                                            THERON_TRACK02_MAX_BANK_ANCHORS) {
                                            ++out_receipt
                                                  ->nonstartup_level_inner_scan_anchor_counts
                                                      [anchor];
                                        }
                                        if (inner_status == THERON_MAP_OK) {
                                            size_t inner_user_data_offset = 0u;
                                            const size_t inner_raw_offset =
                                                entries[entry_index]
                                                    .absolute_offset +
                                                inner_offset;
                                            out_receipt
                                                ->nonstartup_level_decode_ready =
                                                1;
                                            anchor_nonstartup_level_ready = 1;
                                            ++out_receipt
                                                  ->nonstartup_level_inner_scan_loaded_count;
                                            out_receipt
                                                ->nonstartup_level_inner_scan_loaded_anchor_mask |=
                                                1u << (unsigned)anchor;
                                            if (anchor <
                                                    THERON_TRACK02_MAX_BANK_ANCHORS &&
                                                out_receipt
                                                        ->nonstartup_level_inner_scan_loaded_raw_offsets
                                                            [anchor] == 0u) {
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_entry_index
                                                        [anchor] =
                                                    entry_index;
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_raw_offsets
                                                        [anchor] =
                                                    inner_raw_offset;
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_window_offsets
                                                        [anchor] =
                                                    inner_offset;
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_width
                                                        [anchor] =
                                                    inner_width;
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_height
                                                        [anchor] =
                                                    inner_height;
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_seed
                                                        [anchor] =
                                                    inner_seed;
                                                out_receipt
                                                    ->nonstartup_level_inner_scan_loaded_level_index
                                                        [anchor] =
                                                    inner_level_index;
                                                if (theron_v1_track02_raw_offset_to_user_offset(
                                                        inner_raw_offset,
                                                        track02_size,
                                                        md5_hex,
                                                        &inner_user_data_offset) ==
                                                    THERON_TRACK02_SIGNAL_OK) {
                                                    out_receipt
                                                        ->nonstartup_level_inner_scan_loaded_user_data_offsets
                                                            [anchor] =
                                                        inner_user_data_offset;
                                                    out_receipt
                                                        ->nonstartup_level_inner_scan_loaded_user_data_valid
                                                            [anchor] = 1;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            out_receipt
                                ->nonstartup_level_candidate_last_entry_index
                                    [anchor] = entry_index;
                            out_receipt
                                ->nonstartup_level_candidate_last_raw_offsets
                                    [anchor] =
                                entries[entry_index].absolute_offset;
                            if (entries[entry_index].absolute_offset >=
                                descriptor_offset) {
                                out_receipt
                                    ->nonstartup_level_candidate_after_descriptor
                                        [anchor] = 1;
                                out_receipt
                                    ->nonstartup_level_candidate_descriptor_delta
                                        [anchor] =
                                    descriptor_delta;
                            }
                            if (user_data_valid) {
                                out_receipt
                                    ->nonstartup_level_candidate_user_data_offsets
                                        [anchor] = user_data_offset;
                                out_receipt
                                    ->nonstartup_level_candidate_user_data_valid
                                        [anchor] = 1;
                            }
                            out_receipt
                                ->nonstartup_level_candidate_last_byte_counts
                                    [anchor] =
                                entries[entry_index].byte_count;
                            out_receipt
                                ->nonstartup_level_candidate_last_hashes
                                    [anchor] =
                                candidate_nonzero_byte_count;
                            out_receipt
                                ->nonstartup_level_candidate_header_width
                                    [anchor] =
                                candidate_header_width;
                            out_receipt
                                ->nonstartup_level_candidate_header_height
                                    [anchor] =
                                candidate_header_height;
                            out_receipt
                                ->nonstartup_level_candidate_header_seed
                                    [anchor] =
                                candidate_header_seed;
                            out_receipt
                                ->nonstartup_level_candidate_header_level_index
                                    [anchor] =
                                candidate_header_level_index;
                            out_receipt
                                ->nonstartup_level_candidate_header_matches_startup_shape
                                    [anchor] =
                                candidate_header_matches_startup_shape;
                            out_receipt
                                ->nonstartup_level_candidate_hash[anchor] =
                                candidate_hash;
                            out_receipt
                                ->nonstartup_level_candidate_entry_role
                                    [anchor] =
                                entries[entry_index].role;
                            out_receipt
                                ->nonstartup_level_candidate_window_kind
                                    [anchor] =
                                THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
                        }
                    }
                }
            }
        }

        handoff_status = theron_v1_track02_bind_startup_semantic_handoff(
            track02_data,
            track02_size,
            md5_hex,
            descriptor_offset,
            &handoff);
        if (anchor < THERON_TRACK02_MAX_BANK_ANCHORS) {
            out_receipt->startup_level_anchor_status[anchor] =
                (int)handoff_status;
        }
        if (handoff_status == THERON_TRACK02_LEVEL_HANDOFF_OK &&
            handoff.ready_for_runtime &&
            handoff.user_data_offset_valid) {
            const Theron_Track02LevelCandidate *candidate =
                &handoff.initial_candidate.candidate;
            ++out_receipt->startup_level_route_count;
            out_receipt->startup_level_route_mask |= 1u << (unsigned)anchor;
            if (!out_receipt->startup_level_route_ready) {
                out_receipt->startup_level_route_ready = 1;
                out_receipt->startup_descriptor_offset = descriptor_offset;
                out_receipt->startup_raw_offset = candidate->absolute_offset;
                out_receipt->startup_user_data_offset =
                    handoff.user_data_offset;
                out_receipt->startup_user_data_offset_valid =
                    handoff.user_data_offset_valid;
                out_receipt->startup_header_width =
                    candidate->header_width;
                out_receipt->startup_header_height =
                    candidate->header_height;
                out_receipt->startup_header_seed =
                    candidate->header_seed;
                out_receipt->startup_header_level_index =
                    candidate->header_level_index;
            }
            hash ^= (uint32_t)candidate->absolute_offset;
            hash *= 16777619u;
            hash ^= (uint32_t)handoff.user_data_offset;
            hash *= 16777619u;
            if (anchor < THERON_TRACK02_MAX_BANK_ANCHORS) {
                out_receipt->startup_level_anchor_raw_offsets[anchor] =
                    candidate->absolute_offset;
                out_receipt->startup_level_anchor_user_data_offsets[anchor] =
                    handoff.user_data_offset;
                out_receipt->startup_level_anchor_user_data_valid[anchor] =
                    handoff.user_data_offset_valid;
                out_receipt->startup_level_anchor_width[anchor] =
                    candidate->header_width;
                out_receipt->startup_level_anchor_height[anchor] =
                    candidate->header_height;
                out_receipt->startup_level_anchor_seed[anchor] =
                    candidate->header_seed;
                out_receipt->startup_level_anchor_level_index[anchor] =
                    candidate->header_level_index;
            }
            out_receipt->semantic_role_mask |=
                1u << THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE;
            out_receipt->level_grid_role_mapped = 1;
            ++out_receipt->startup_level_grid_record_count;
            if (!out_receipt->startup_level_grid_record_ready) {
                out_receipt->startup_level_grid_record_ready = 1;
                out_receipt->startup_level_grid_descriptor_offset =
                    descriptor_offset;
                out_receipt->startup_level_grid_raw_offset =
                    candidate->absolute_offset;
                out_receipt->startup_level_grid_user_data_offset =
                    handoff.user_data_offset;
                out_receipt->startup_level_grid_user_data_offset_valid =
                    handoff.user_data_offset_valid;
            }
        } else {
            ++out_receipt->startup_level_blocked_anchor_count;
            out_receipt->startup_level_blocked_anchor_mask |=
                1u << (unsigned)anchor;
        }
        if (!anchor_nonstartup_level_ready) {
            ++out_receipt->nonstartup_level_blocked_anchor_count;
            out_receipt->nonstartup_level_blocked_anchor_mask |=
                1u << (unsigned)anchor;
        }
    }

    out_receipt->descriptor_route_ready =
        signal.anchor_count > 0u &&
        out_receipt->descriptor_anchor_mask ==
            ((1u << (unsigned)signal.anchor_count) - 1u);
    out_receipt->blocked_for_missing_nonstartup_level_evidence =
        out_receipt->descriptor_route_ready &&
        out_receipt->nonstartup_level_blocked_anchor_count > 0u;
    hash ^= out_receipt->semantic_role_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->startup_level_grid_record_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->startup_level_grid_raw_offset;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->startup_level_grid_user_data_offset;
    hash *= 16777619u;
    hash ^= out_receipt->startup_level_blocked_anchor_mask;
    hash *= 16777619u;
    hash ^= out_receipt->nonstartup_level_blocked_anchor_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->nonstartup_level_loaded_count;
    hash *= 16777619u;
    hash ^= out_receipt->nonstartup_level_loaded_anchor_mask;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->nonstartup_level_inner_scan_probe_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->nonstartup_level_inner_scan_loaded_count;
    hash *= 16777619u;
    hash ^= out_receipt->nonstartup_level_inner_scan_loaded_anchor_mask;
    hash *= 16777619u;
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        size_t sample_slot;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_sample_count[anchor];
        hash *= 16777619u;
        for (sample_slot = 0u;
             sample_slot <
             THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES;
             ++sample_slot) {
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_entry_index
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_raw_offsets
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_user_data_offsets
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_user_data_valid
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_byte_counts
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_descriptor_delta
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^=
                out_receipt
                    ->nonstartup_level_candidate_sample_hash
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_nonzero_byte_counts
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_header_width
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_header_height
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= out_receipt
                ->nonstartup_level_candidate_sample_header_seed
                    [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_header_level_index
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_object_table_status
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_object_table_declared_record_counts
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_object_table_record_counts
                        [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= out_receipt
                ->nonstartup_level_candidate_sample_object_table_level_masks
                    [anchor][sample_slot];
            hash *= 16777619u;
            hash ^= (uint32_t)
                out_receipt
                    ->nonstartup_level_candidate_sample_object_table_reject_reasons
                        [anchor][sample_slot];
            hash *= 16777619u;
        }
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_anchor_counts[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_entry_index[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_raw_offsets[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_user_data_offsets[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_user_data_valid[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_byte_counts[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_nonzero_byte_counts[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_header_width[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_header_height[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->nonstartup_level_candidate_header_seed[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_header_level_index[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt
                ->nonstartup_level_candidate_header_matches_startup_shape[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->nonstartup_level_candidate_hash[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_descriptor_delta[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_after_descriptor[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_entry_role[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->nonstartup_level_candidate_window_kind[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->startup_level_anchor_status[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)out_receipt->startup_level_anchor_raw_offsets[anchor];
        hash *= 16777619u;
        hash ^= (uint32_t)
            out_receipt->startup_level_anchor_user_data_offsets[anchor];
        hash *= 16777619u;
    }
    out_receipt->route_hash = hash;
    out_receipt->valid = out_receipt->verified_track02 &&
        out_receipt->descriptor_route_ready &&
        out_receipt->startup_level_route_ready;
    return out_receipt->valid ? 1 : 0;
}

const char *theron_v1_track02_nonstartup_level_layout_comparison_status_name(
    Theron_Track02NonstartupLevelLayoutComparisonStatus status) {

    switch (status) {
    case THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_OK:
        return "ok";
    case THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT:
        return "unverified-receipt";
    case THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR:
        return "unsupported-variant-pair";
    default:
        return "unknown";
    }
}

Theron_Track02NonstartupLevelLayoutComparisonStatus
theron_v1_track02_compare_nonstartup_level_layout_variants(
    const Theron_Track02LevelRouteReceipt *first,
    const Theron_Track02LevelRouteReceipt *second,
    Theron_Track02NonstartupLevelLayoutComparisonReceipt *out_receipt) {
    const Theron_Track02LevelRouteReceipt *jp;
    const Theron_Track02LevelRouteReceipt *us;
    uint32_t hash = 2166136261u;
    size_t anchor;

    if (!out_receipt) {
        return THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_BAD_INPUT;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status =
        THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_BAD_INPUT;
    if (!first || !second) {
        return out_receipt->status;
    }
    if (!first->verified_track02 || !second->verified_track02 ||
        !first->descriptor_route_ready ||
        !second->descriptor_route_ready || first->fallback_visuals_allowed ||
        second->fallback_visuals_allowed) {
        out_receipt->status =
            THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_UNVERIFIED_RECEIPT;
        return out_receipt->status;
    }
    if (first->variant == THERON_TRACK02_VARIANT_JP_BIN &&
        second->variant == THERON_TRACK02_VARIANT_US_BIN) {
        jp = first;
        us = second;
    } else if (first->variant == THERON_TRACK02_VARIANT_US_BIN &&
               second->variant == THERON_TRACK02_VARIANT_JP_BIN) {
        jp = second;
        us = first;
    } else {
        out_receipt->status =
            THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_UNSUPPORTED_VARIANT_PAIR;
        return out_receipt->status;
    }

    out_receipt->jp_variant = jp->variant;
    out_receipt->us_variant = us->variant;
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        unsigned int bit = 1u << (unsigned int)anchor;
        size_t sample_count =
            jp->nonstartup_level_candidate_sample_count[anchor];
        uint32_t sample_hash = 2166136261u;
        int matches = 1;
        size_t sample;

        if ((jp->nonstartup_level_candidate_anchor_mask & bit) == 0u ||
            (us->nonstartup_level_candidate_anchor_mask & bit) == 0u ||
            sample_count == 0u ||
            sample_count > THERON_TRACK02_MAX_NONSTARTUP_LEVEL_RECEIPT_CANDIDATES ||
            sample_count !=
                us->nonstartup_level_candidate_sample_count[anchor]) {
            continue;
        }
        out_receipt->comparable_anchor_mask |= bit;
        out_receipt->candidate_counts[anchor] = sample_count;
        for (sample = 0u; sample < sample_count; ++sample) {
            if (jp->nonstartup_level_candidate_sample_entry_index[anchor][sample] !=
                    us->nonstartup_level_candidate_sample_entry_index[anchor][sample] ||
                jp->nonstartup_level_candidate_sample_byte_counts[anchor][sample] !=
                    us->nonstartup_level_candidate_sample_byte_counts[anchor][sample] ||
                jp->nonstartup_level_candidate_sample_descriptor_delta[anchor][sample] !=
                    us->nonstartup_level_candidate_sample_descriptor_delta[anchor][sample] ||
                jp->nonstartup_level_candidate_sample_hash[anchor][sample] !=
                    us->nonstartup_level_candidate_sample_hash[anchor][sample]) {
                matches = 0;
            }
            sample_hash ^= (uint32_t)
                jp->nonstartup_level_candidate_sample_entry_index[anchor][sample];
            sample_hash *= 16777619u;
            sample_hash ^= (uint32_t)
                jp->nonstartup_level_candidate_sample_byte_counts[anchor][sample];
            sample_hash *= 16777619u;
            sample_hash ^= (uint32_t)
                jp->nonstartup_level_candidate_sample_descriptor_delta[anchor][sample];
            sample_hash *= 16777619u;
            sample_hash ^=
                jp->nonstartup_level_candidate_sample_hash[anchor][sample];
            sample_hash *= 16777619u;
        }
        if (matches) {
            out_receipt->matching_anchor_mask |= bit;
            out_receipt->matching_sample_hashes[anchor] = sample_hash;
        } else {
            out_receipt->mismatch_anchor_mask |= bit;
        }
    }
    hash ^= (uint32_t)out_receipt->jp_variant;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->us_variant;
    hash *= 16777619u;
    hash ^= out_receipt->comparable_anchor_mask;
    hash *= 16777619u;
    hash ^= out_receipt->matching_anchor_mask;
    hash *= 16777619u;
    hash ^= out_receipt->mismatch_anchor_mask;
    hash *= 16777619u;
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        hash ^= (uint32_t)out_receipt->candidate_counts[anchor];
        hash *= 16777619u;
        hash ^= out_receipt->matching_sample_hashes[anchor];
        hash *= 16777619u;
    }
    out_receipt->comparison_hash = hash;
    out_receipt->valid = 1;
    out_receipt->status = THERON_TRACK02_NONSTARTUP_LEVEL_LAYOUT_COMPARISON_OK;
    return out_receipt->status;
}

Theron_Track02LevelHandoffStatus theron_v1_track02_load_startup_semantic_level(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    size_t descriptor_offset,
    int dungeon_id,
    int sub_level_index,
    Theron_V1_Level *out_level,
    Theron_Track02StartupSemanticHandoff *out_semantic_handoff,
    Theron_Track02LevelHandoff *out_level_handoff) {

    Theron_Track02StartupSemanticHandoff local_semantic;
    Theron_Track02StartupSemanticHandoff *semantic =
        out_semantic_handoff ? out_semantic_handoff : &local_semantic;
    const Theron_Track02LevelCandidate *candidate;
    const uint8_t *level_bytes;
    uint8_t user_data_level_bytes[12u + TQR_RAW_INITIAL_LEVEL_WIDTH *
                                  TQR_RAW_INITIAL_LEVEL_HEIGHT];
    Theron_Track02LevelHandoffStatus status;
    Theron_MapLoadResult map_status;
    size_t copied_byte_count = 0u;
    size_t copied_user_data_offset = 0u;

    if (out_semantic_handoff) {
        memset(out_semantic_handoff, 0, sizeof(*out_semantic_handoff));
    }
    if (out_level_handoff) {
        memset(out_level_handoff, 0, sizeof(*out_level_handoff));
        out_level_handoff->map_status = THERON_MAP_ERR_NULL;
    }
    if (out_level) {
        memset(out_level, 0, sizeof(*out_level));
    }
    if (!track02_data || track02_size == 0u || !out_level ||
        !out_level_handoff) {
        return THERON_TRACK02_LEVEL_HANDOFF_BAD_INPUT;
    }

    status = theron_v1_track02_bind_startup_semantic_handoff(track02_data,
                                                              track02_size,
                                                              md5_hex,
                                                              descriptor_offset,
                                                              semantic);
    out_level_handoff->binding_status = (int32_t)status;
    out_level_handoff->candidate_count = semantic->initial_candidate.candidate_count;
    out_level_handoff->expected_offset = semantic->initial_candidate.expected_offset;
    out_level_handoff->matches_initial_anchor =
        semantic->initial_candidate.matches_initial_anchor;
    out_level_handoff->user_data_offset = semantic->user_data_offset;
    out_level_handoff->user_data_offset_valid = semantic->user_data_offset_valid;
    if (semantic->initial_candidate.candidate_count == 1u) {
        out_level_handoff->descriptor_delta =
            semantic->initial_candidate.candidate.descriptor_delta;
    }
    if (status != THERON_TRACK02_LEVEL_HANDOFF_OK ||
        !semantic->ready_for_runtime) {
        return status;
    }

    candidate = &semantic->initial_candidate.candidate;
    if (candidate->absolute_offset > track02_size ||
        candidate->byte_count > track02_size - candidate->absolute_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    if (candidate->byte_count > sizeof(user_data_level_bytes) ||
        theron_v1_track02_copy_initial_level_user_data_window(
            track02_data,
            track02_size,
            md5_hex,
            descriptor_offset,
            user_data_level_bytes,
            sizeof(user_data_level_bytes),
            &copied_byte_count,
            &copied_user_data_offset) != THERON_TRACK02_LEVEL_HANDOFF_OK ||
        copied_byte_count != candidate->byte_count ||
        copied_user_data_offset != candidate->user_data_offset) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    level_bytes = user_data_level_bytes;
    out_level_handoff->entry_index = THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES;
    out_level_handoff->absolute_offset = candidate->absolute_offset;
    out_level_handoff->byte_count = candidate->byte_count;
    out_level_handoff->window_kind = THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
    out_level_handoff->header_width = rd16be(level_bytes + 0);
    out_level_handoff->header_height = rd16be(level_bytes + 2);
    out_level_handoff->header_seed = rd32be(level_bytes + 4);
    out_level_handoff->header_level_index = rd16be(level_bytes + 8);

    if (out_level_handoff->header_width != TQR_RAW_INITIAL_LEVEL_WIDTH ||
        out_level_handoff->header_height != TQR_RAW_INITIAL_LEVEL_HEIGHT ||
        out_level_handoff->header_seed != TQR_RAW_INITIAL_LEVEL_SEED ||
        out_level_handoff->header_level_index != TQR_RAW_INITIAL_LEVEL_INDEX) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    }

    map_status = theron_v1_level_load(out_level,
                                      level_bytes,
                                      (int)candidate->byte_count,
                                      dungeon_id,
                                      sub_level_index);
    out_level_handoff->map_status = map_status;
    if (map_status != THERON_MAP_OK) {
        return THERON_TRACK02_LEVEL_HANDOFF_LEVEL_LOAD_FAILED;
    }

    choose_initial_level_start_pose(out_level);
    out_level_handoff->loaded = 1;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}
