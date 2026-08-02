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
#define TQR_US_BIN_FIRST_QUEST_BLOCK_OFFSET 0xa9f90u
#define TQR_JP_BIN_FIRST_QUEST_BLOCK_OFFSET 0xa9660u
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

Theron_Track02BlockStatus theron_v1_track02_extract_quest_block(
    const uint8_t *track02_data, size_t track02_size,
    Theron_Track02Variant variant, size_t dungeon_index,
    uint8_t *out_block, size_t out_size) {
    size_t raw_offset, copied = 0u;
    if (out_block && out_size > 0u) memset(out_block, 0, out_size);
    if (!track02_data || !out_block || out_size < THERON_TRACK02_QUEST_BLOCK_BYTES ||
        dungeon_index >= THERON_TRACK02_QUEST_BLOCK_COUNT)
        return THERON_TRACK02_BLOCK_BAD_INPUT;
    if (variant != THERON_TRACK02_VARIANT_US_BIN &&
        variant != THERON_TRACK02_VARIANT_JP_BIN)
        return THERON_TRACK02_BLOCK_UNSUPPORTED_VARIANT;
    raw_offset = (variant == THERON_TRACK02_VARIANT_US_BIN ?
        TQR_US_BIN_FIRST_QUEST_BLOCK_OFFSET : TQR_JP_BIN_FIRST_QUEST_BLOCK_OFFSET) +
        (dungeon_index * THERON_TRACK02_QUEST_BLOCK_BYTES /
         THERON_TRACK02_RAW_USER_DATA_BYTES) * THERON_TRACK02_RAW_SECTOR_BYTES;
    while (copied < THERON_TRACK02_QUEST_BLOCK_BYTES) {
        size_t sector = raw_offset +
            (copied / THERON_TRACK02_RAW_USER_DATA_BYTES) * THERON_TRACK02_RAW_SECTOR_BYTES;
        size_t chunk = THERON_TRACK02_RAW_USER_DATA_BYTES;
        if (chunk > THERON_TRACK02_QUEST_BLOCK_BYTES - copied)
            chunk = THERON_TRACK02_QUEST_BLOCK_BYTES - copied;
        if (sector > track02_size || chunk > track02_size - sector) {
            memset(out_block, 0, out_size);
            return THERON_TRACK02_BLOCK_OUT_OF_RANGE;
        }
        memcpy(out_block + copied, track02_data + sector, chunk);
        copied += chunk;
    }
    return THERON_TRACK02_BLOCK_OK;
}

const char *theron_v1_track02_block_status_name(
    Theron_Track02BlockStatus status) {
    switch (status) {
    case THERON_TRACK02_BLOCK_OK: return "OK";
    case THERON_TRACK02_BLOCK_BAD_INPUT: return "BAD_INPUT";
    case THERON_TRACK02_BLOCK_UNSUPPORTED_VARIANT: return "UNSUPPORTED_VARIANT";
    case THERON_TRACK02_BLOCK_OUT_OF_RANGE: return "OUT_OF_RANGE";
    default: return "UNKNOWN";
    }
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

static int tqr_ascii_starts_ci(const char *text, const char *prefix) {
    unsigned char ct;
    unsigned char cp;
    if (!text || !prefix) return 0;
    while (*prefix) {
        ct = (unsigned char)*text++;
        cp = (unsigned char)*prefix++;
        if (ct >= 'A' && ct <= 'Z') ct = (unsigned char)(ct + ('a' - 'A'));
        if (cp >= 'A' && cp <= 'Z') cp = (unsigned char)(cp + ('a' - 'A'));
        if (ct != cp) return 0;
    }
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
    /* CUE sheets are plain text, but common macOS/Windows editors may emit a
     * UTF-8 BOM before the first FILE directive. It is metadata, not part of
     * the command; retain the strict FILE/TRACK/INDEX layout checks below. */
    if (text && (unsigned char)text[0] == 0xefu &&
        (unsigned char)text[1] == 0xbbu && (unsigned char)text[2] == 0xbfu) {
        text += 3;
    }
    return text;
}

static int tqr_cue_file_line(const char *line, char *out_name, size_t out_cap) {
    const char *p = tqr_skip_space(line);
    const char *end;
    size_t len;
    if (!p || !tqr_ascii_starts_ci(p, "FILE") ||
        (p[4] != ' ' && p[4] != '\t')) return 0;
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
    return tqr_ascii_starts_ci(p, "BINARY") &&
           (p[6] == '\0' || p[6] == ' ' || p[6] == '\t' || p[6] == '\r' || p[6] == '\n');
}

static int tqr_cue_is_track02_mode1(const char *line) {
    const char *p = tqr_skip_space(line);
    if (!p || !tqr_ascii_starts_ci(p, "TRACK") ||
        (p[5] != ' ' && p[5] != '\t')) return 0;
    p = tqr_skip_space(p + 5u);
    if (strncmp(p, "02", 2u) != 0 || (p[2] != ' ' && p[2] != '\t')) return 0;
    p = tqr_skip_space(p + 2u);
    return tqr_ascii_equal_ci(p, "MODE1/2352") ||
           tqr_ascii_equal_ci(p, "MODE1/2048") ||
           (tqr_ascii_starts_ci(p, "MODE1/2352") &&
            (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n')) ||
           (tqr_ascii_starts_ci(p, "MODE1/2048") &&
            (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n'));
}

static int tqr_cue_path_for_file(const char *cue_path, const char *file_name,
                                 char out_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]);

static int tqr_cue_known_split_track02_path(
    const char *cue_path,
    const char *selected_file,
    char out_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    if (!cue_path || !selected_file || !out_path ||
        strchr(selected_file, '/') || strchr(selected_file, '\\')) return 0;
    return tqr_cue_path_for_file(cue_path, selected_file, out_path);
}

static int tqr_cue_track_number_and_mode(const char *line,
                                         unsigned int *out_track,
                                         int *out_audio,
                                         int *out_track02_mode1) {
    const char *p = tqr_skip_space(line);
    unsigned int track = 0u;
    if (!p || !tqr_ascii_starts_ci(p, "TRACK") ||
        (p[5] != ' ' && p[5] != '\t')) return 0;
    p = tqr_skip_space(p + 5u);
    if (p[0] < '0' || p[0] > '9' || p[1] < '0' || p[1] > '9') return 0;
    track = (unsigned int)((p[0] - '0') * 10 + (p[1] - '0'));
    if (p[2] != ' ' && p[2] != '\t') return 0;
    p = tqr_skip_space(p + 2u);
    if (out_track) *out_track = track;
    if (out_audio) {
        *out_audio = tqr_ascii_starts_ci(p, "AUDIO") &&
            (p[5] == '\0' || p[5] == ' ' || p[5] == '\t' ||
             p[5] == '\r' || p[5] == '\n');
    }
    if (out_track02_mode1) {
        *out_track02_mode1 = track == 2u &&
            (tqr_ascii_equal_ci(p, "MODE1/2352") ||
             tqr_ascii_equal_ci(p, "MODE1/2048") ||
             (tqr_ascii_starts_ci(p, "MODE1/2352") &&
              (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n')) ||
             (tqr_ascii_starts_ci(p, "MODE1/2048") &&
              (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n')));
    }
    return 1;
}

static int tqr_cue_index01(const char *line, unsigned int *out_m,
                           unsigned int *out_s, unsigned int *out_f) {
    const char *p = tqr_skip_space(line);
    unsigned int m, s, f;
    if (!p || !tqr_ascii_starts_ci(p, "INDEX") ||
        (p[5] != ' ' && p[5] != '\t')) return 0;
    p = tqr_skip_space(p + 5u);
    if (strncmp(p, "01", 2u) != 0 || (p[2] != ' ' && p[2] != '\t')) return 0;
    p = tqr_skip_space(p + 2u);
    if (sscanf(p, "%2u:%2u:%2u", &m, &s, &f) != 3 || s >= 60u || f >= 75u) return 0;
    if (out_m) *out_m = m;
    if (out_s) *out_s = s;
    if (out_f) *out_f = f;
    return 1;
}

static int tqr_cue_path_for_file(const char *cue_path, const char *file_name,
                                 char out_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    const char *slash;
    size_t parent_len;
    if (!cue_path || !file_name || !out_path) return 0;
    slash = strrchr(cue_path, '/');
    if (!slash) slash = strrchr(cue_path, '\\');
    parent_len = slash ? (size_t)(slash - cue_path + 1u) : 0u;
    if (parent_len + strlen(file_name) >= THERON_TRACK02_MOUNT_PATH_CAPACITY) return 0;
    if (parent_len) memcpy(out_path, cue_path, parent_len);
    memcpy(out_path + parent_len, file_name, strlen(file_name) + 1u);
    return 1;
}

static int tqr_path_is_readable(const char *path) {
    FILE *file = path ? fopen(path, "rb") : NULL;
    if (!file) return 0;
    fclose(file);
    return 1;
}

static int tqr_file_size(const char *path, size_t *out_size) {
    FILE *file;
    long end;
    if (!path || !out_size) return 0;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 || (end = ftell(file)) < 0) {
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_size = (size_t)end;
    return 1;
}

Theron_Track01CddaStatus theron_v1_track01_cdda_handoff_from_verified_media(
    const char *media_path,
    const char *verified_track02_md5,
    Theron_Track01CddaHandoff *out_handoff) {
    FILE *cue;
    char line[2048];
    char current_file[THERON_TRACK02_MOUNT_PATH_CAPACITY] = {0};
    char audio_file[THERON_TRACK02_MOUNT_PATH_CAPACITY] = {0};
    char track02_file[THERON_TRACK02_MOUNT_PATH_CAPACITY] = {0};
    unsigned int current_track = 0u, track01_count = 0u, track01_index_count = 0u,
                 track02_count = 0u;
    int current_is_track01 = 0, current_audio = 0, current_track02_mode1 = 0;
    if (!out_handoff) return THERON_TRACK01_CDDA_BAD_INPUT;
    memset(out_handoff, 0, sizeof(*out_handoff));
    out_handoff->status = THERON_TRACK01_CDDA_UNAVAILABLE;
    if (!media_path || !media_path[0] || !verified_track02_md5 ||
        theron_v1_track02_variant_for_md5(verified_track02_md5) == THERON_TRACK02_VARIANT_UNKNOWN) {
        out_handoff->status = THERON_TRACK01_CDDA_UNVERIFIED;
        snprintf(out_handoff->unavailable_reason, sizeof(out_handoff->unavailable_reason),
                 "Track 02 provenance is not hash-verified");
        return out_handoff->status;
    }
    out_handoff->track02_variant = theron_v1_track02_variant_for_md5(verified_track02_md5);
    if (!tqr_path_is_cue(media_path)) {
        snprintf(out_handoff->unavailable_reason, sizeof(out_handoff->unavailable_reason),
                 "ISO/BIN media has no Track 01 CDDA metadata");
        return out_handoff->status;
    }
    cue = fopen(media_path, "rb");
    if (!cue) {
        snprintf(out_handoff->unavailable_reason, sizeof(out_handoff->unavailable_reason),
                 "CUE metadata is unavailable");
        return out_handoff->status;
    }
    snprintf(out_handoff->cue_path, sizeof(out_handoff->cue_path), "%s", media_path);
    while (fgets(line, sizeof(line), cue) != NULL) {
        char parsed[THERON_TRACK02_MOUNT_PATH_CAPACITY];
        unsigned int track;
        int audio, track02_mode1;
        if (tqr_cue_file_line(line, parsed, sizeof(parsed))) {
            snprintf(current_file, sizeof(current_file), "%s", parsed);
            continue;
        }
        if (tqr_cue_track_number_and_mode(line, &track, &audio, &track02_mode1)) {
            current_track = track;
            current_audio = audio;
            current_is_track01 = track == 1u;
            current_track02_mode1 = track02_mode1;
            if (current_is_track01 && current_audio && current_file[0]) {
                ++track01_count;
                snprintf(audio_file, sizeof(audio_file), "%s", current_file);
            }
            if (current_track02_mode1 && current_file[0]) {
                ++track02_count;
                snprintf(track02_file, sizeof(track02_file), "%s", current_file);
            }
            continue;
        }
        if (current_track == 1u && current_is_track01 && current_audio &&
            tqr_cue_index01(line, &out_handoff->index_minute,
                            &out_handoff->index_second, &out_handoff->index_frame)) {
            ++track01_index_count;
            out_handoff->index_lba = out_handoff->index_minute * 60u * 75u +
                                     out_handoff->index_second * 75u + out_handoff->index_frame;
        }
    }
    fclose(cue);
    if (track01_count != 1u || track01_index_count != 1u || track02_count != 1u ||
        !audio_file[0] || !track02_file[0] ||
        !tqr_cue_path_for_file(media_path, audio_file, out_handoff->audio_path) ||
        !tqr_cue_path_for_file(media_path, track02_file, out_handoff->track02_path) ||
        !tqr_path_is_readable(out_handoff->audio_path) || !tqr_path_is_readable(out_handoff->track02_path) ||
        !tqr_file_size(out_handoff->audio_path, &out_handoff->audio_file_bytes)) {
        snprintf(out_handoff->unavailable_reason, sizeof(out_handoff->unavailable_reason),
                 "CUE lacks one readable Track 01 AUDIO and Track 02 MODE1/2352 pair");
        return out_handoff->status;
    }
    out_handoff->audio_start_byte = (size_t)out_handoff->index_lba *
        THERON_TRACK01_CDDA_SECTOR_BYTES;
    if (out_handoff->audio_start_byte >= out_handoff->audio_file_bytes ||
        (out_handoff->audio_file_bytes - out_handoff->audio_start_byte) %
            THERON_TRACK01_CDDA_SECTOR_BYTES != 0u) {
        snprintf(out_handoff->unavailable_reason, sizeof(out_handoff->unavailable_reason),
                 "Track 01 AUDIO is not a bounded 2352-byte CDDA sector stream");
        return out_handoff->status;
    }
    out_handoff->audio_sector_count =
        (out_handoff->audio_file_bytes - out_handoff->audio_start_byte) /
        THERON_TRACK01_CDDA_SECTOR_BYTES;
    out_handoff->status = THERON_TRACK01_CDDA_AVAILABLE;
    out_handoff->track_number = 1u;
    out_handoff->original_cdda = 1;
    out_handoff->playback_handoff_ready = 1;
    return out_handoff->status;
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
    size_t selected_index01_count = 0u;
    int current_track02_selected = 0;

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
            current_track02_selected = 0;
        } else if (tqr_cue_is_track02_mode1(line) && current_file[0]) {
            ++selected_count;
            current_track02_selected = 1;
            if (selected_count == 1u) {
                memcpy(selected_file, current_file, strlen(current_file) + 1u);
            }
        } else if (tqr_cue_index01(line, NULL, NULL, NULL)) {
            if (current_track02_selected) {
                ++selected_index01_count;
            }
        } else if (tqr_ascii_starts_ci(tqr_skip_space(line), "TRACK")) {
            current_track02_selected = 0;
        }
    }
    fclose(cue);
    if (selected_count != 1u || selected_index01_count != 1u ||
        !selected_file[0]) return THERON_TRACK02_SIGNAL_NOT_FOUND;
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
    if (!payload && tqr_cue_known_split_track02_path(media_path, selected_file,
                                                     out_payload_path)) {
        payload = fopen(out_payload_path, "rb");
    }
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
            memset(out_evidence, 0, sizeof(*out_evidence));
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
        memset(out_evidence, 0, sizeof(*out_evidence));
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
    if (out_atlas->total_tile_count == 0u ||
        out_atlas->total_nonzero_pixel_count == 0u) {
        memset(out_atlas, 0, sizeof(*out_atlas));
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
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
                                          THERON_DUNGEON_1_AKUTUBA,
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
                                      THERON_DUNGEON_1_AKUTUBA, 0);
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

Theron_Track02SignalStatus
theron_v1_track02_capture_initial_level_object_boundary(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelObjectBoundaryReceipt *out_receipt) {

    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02InitialCandidateBinding binding;
    Theron_Track02SignalStatus status;
    size_t level_sector_offset;
    size_t level_record;
    size_t boundary_sector_offset;
    uint32_t hash = 2166136261u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || track02_size == 0u || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    /* The IPL receipt is the only source of the variant-specific INDEX 01
     * coordinate.  ReDMCSB has no Theron's Quest equivalent; see
     * docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md. */
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK || !loader.valid) return status;
    if (theron_v1_track02_bind_initial_level_candidate(
            track02_data, track02_size, md5_hex,
            loader.variant == THERON_TRACK02_VARIANT_JP_BIN
                ? g_jp_bin_descriptor_offsets[0]
                : g_us_bin_descriptor_offsets[0],
            &binding) != THERON_TRACK02_LEVEL_HANDOFF_OK ||
        !binding.matches_initial_anchor || !binding.candidate.loaded ||
        !binding.candidate.user_data_offset_valid) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    level_sector_offset = binding.candidate.absolute_offset % TQR_RAW_SECTOR_BYTES;
    if (level_sector_offset < TQR_RAW_SECTOR_USER_DATA_OFFSET ||
        binding.candidate.byte_count >
            TQR_RAW_SECTOR_USER_DATA_BYTES -
                (level_sector_offset - TQR_RAW_SECTOR_USER_DATA_OFFSET)) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    level_record = binding.candidate.absolute_offset / TQR_RAW_SECTOR_BYTES;
    if (level_record < loader.data_track_index01_raw_sector) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    level_record -= loader.data_track_index01_raw_sector;
    boundary_sector_offset = level_sector_offset + binding.candidate.byte_count;

    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->track02_record = (uint32_t)level_record;
    out_receipt->data_track_index01_raw_sector =
        loader.data_track_index01_raw_sector;
    out_receipt->level_first_raw_sector =
        binding.candidate.absolute_offset / TQR_RAW_SECTOR_BYTES;
    out_receipt->level_raw_offset = binding.candidate.absolute_offset;
    out_receipt->level_user_data_offset = binding.candidate.user_data_offset;
    out_receipt->level_user_data_offset_in_record =
        level_sector_offset - TQR_RAW_SECTOR_USER_DATA_OFFSET;
    out_receipt->level_byte_count = binding.candidate.byte_count;
    out_receipt->object_boundary_raw_offset =
        binding.candidate.absolute_offset + binding.candidate.byte_count;
    out_receipt->object_boundary_user_data_offset =
        binding.candidate.user_data_offset + binding.candidate.byte_count;
    out_receipt->object_boundary_user_data_offset_in_record =
        boundary_sector_offset - TQR_RAW_SECTOR_USER_DATA_OFFSET;
    out_receipt->following_user_data_bytes_in_record =
        TQR_RAW_SECTOR_USER_DATA_BYTES -
        out_receipt->object_boundary_user_data_offset_in_record;
    out_receipt->following_user_data_hash = tqr_hash_bytes(
        track02_data + out_receipt->object_boundary_raw_offset,
        out_receipt->following_user_data_bytes_in_record);
    out_receipt->level_width = binding.candidate.header_width;
    out_receipt->level_height = binding.candidate.header_height;
    out_receipt->level_seed = binding.candidate.header_seed;
    out_receipt->level_index = binding.candidate.header_level_index;
    /* Both authenticated raw variants retain 0x0103 at offsets 10..11.
     * This is an opaque level-envelope field, not a decoded start pose,
     * object count, flag, or loader command. */
    out_receipt->level_header_extension_be = rd16be(
        track02_data + binding.candidate.absolute_offset + 10u);
    out_receipt->level_payload_hash = tqr_hash_bytes(
        track02_data + binding.candidate.absolute_offset,
        binding.candidate.byte_count);
    out_receipt->object_table_parsed = 0;
    out_receipt->object_table_semantics_proven = 0;
    out_receipt->promotion_blocked = 1;

    hash ^= (uint32_t)out_receipt->track02_record;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->level_user_data_offset_in_record;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->level_byte_count;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->level_header_extension_be;
    hash *= 16777619u;
    hash ^= out_receipt->level_payload_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->object_boundary_user_data_offset_in_record;
    hash *= 16777619u;
    hash ^= out_receipt->following_user_data_hash;
    hash *= 16777619u;
    out_receipt->receipt_hash = hash;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_canonical_iso_projection(
    const uint8_t *raw_track02_data,
    size_t raw_track02_size,
    const char *raw_track02_md5,
    const uint8_t *iso_data,
    size_t iso_size,
    Theron_Track02CanonicalIsoProjectionReceipt *out_receipt) {
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    size_t raw_sector_count;
    size_t iso_sector_count;
    size_t sector;
    size_t first_level_offset;
    size_t post_envelope_offset;
    uint32_t hash = 2166136261u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!raw_track02_data || raw_track02_size == 0u || !raw_track02_md5 ||
        !iso_data || iso_size == 0u || !out_receipt ||
        raw_track02_size % TQR_RAW_SECTOR_BYTES != 0u ||
        iso_size % TQR_RAW_SECTOR_USER_DATA_BYTES != 0u ||
        theron_v1_track02_find_ipl_loader(raw_track02_data, raw_track02_size,
                                          raw_track02_md5, &loader) !=
            THERON_TRACK02_SIGNAL_OK ||
        !loader.valid ||
        theron_v1_track02_capture_initial_level_object_boundary(
            raw_track02_data, raw_track02_size, raw_track02_md5, &boundary) !=
            THERON_TRACK02_SIGNAL_OK ||
        !boundary.valid || boundary.track02_record != 0x0b52u) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    raw_sector_count = raw_track02_size / TQR_RAW_SECTOR_BYTES;
    if (loader.data_track_index01_raw_sector >= raw_sector_count ||
        raw_sector_count - loader.data_track_index01_raw_sector >
            SIZE_MAX / TQR_RAW_SECTOR_USER_DATA_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    iso_sector_count = raw_sector_count - loader.data_track_index01_raw_sector;
    if (iso_size != iso_sector_count * TQR_RAW_SECTOR_USER_DATA_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    for (sector = 0u; sector < iso_sector_count; ++sector) {
        size_t raw_offset =
            (loader.data_track_index01_raw_sector + sector) *
                TQR_RAW_SECTOR_BYTES +
            TQR_RAW_SECTOR_USER_DATA_OFFSET;
        size_t iso_offset = sector * TQR_RAW_SECTOR_USER_DATA_BYTES;
        if (memcmp(raw_track02_data + raw_offset, iso_data + iso_offset,
                   TQR_RAW_SECTOR_USER_DATA_BYTES) != 0) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
    }
    if (boundary.level_user_data_offset_in_record >
            TQR_RAW_SECTOR_USER_DATA_BYTES ||
        boundary.level_byte_count > TQR_RAW_SECTOR_USER_DATA_BYTES -
            boundary.level_user_data_offset_in_record ||
        (size_t)boundary.track02_record * TQR_RAW_SECTOR_USER_DATA_BYTES >
            iso_size ||
        boundary.level_user_data_offset_in_record > iso_size -
            boundary.track02_record * TQR_RAW_SECTOR_USER_DATA_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    first_level_offset = (size_t)boundary.track02_record *
            TQR_RAW_SECTOR_USER_DATA_BYTES +
        boundary.level_user_data_offset_in_record;
    post_envelope_offset = (size_t)boundary.track02_record *
            TQR_RAW_SECTOR_USER_DATA_BYTES +
        boundary.object_boundary_user_data_offset_in_record;
    if (first_level_offset > iso_size || boundary.level_byte_count >
            iso_size - first_level_offset ||
        post_envelope_offset > iso_size ||
        boundary.following_user_data_bytes_in_record >
            iso_size - post_envelope_offset ||
        tqr_hash_bytes(iso_data + first_level_offset,
                       boundary.level_byte_count) != boundary.level_payload_hash ||
        tqr_hash_bytes(iso_data + post_envelope_offset,
                       boundary.following_user_data_bytes_in_record) !=
            boundary.following_user_data_hash) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->data_track_index01_raw_sector =
        loader.data_track_index01_raw_sector;
    out_receipt->iso_sector_count = iso_sector_count;
    out_receipt->iso_byte_count = iso_size;
    out_receipt->first_level_track02_record = boundary.track02_record;
    out_receipt->first_level_iso_user_data_offset = first_level_offset;
    out_receipt->first_level_byte_count = boundary.level_byte_count;
    out_receipt->first_level_hash = boundary.level_payload_hash;
    out_receipt->post_envelope_iso_user_data_offset = post_envelope_offset;
    out_receipt->post_envelope_byte_count =
        boundary.following_user_data_bytes_in_record;
    out_receipt->post_envelope_hash = boundary.following_user_data_hash;
    out_receipt->object_semantics_proven = 0;
    out_receipt->fallback_allowed = 0;
    hash ^= (uint32_t)out_receipt->data_track_index01_raw_sector;
    hash *= 16777619u;
    hash ^= out_receipt->first_level_track02_record;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->first_level_iso_user_data_offset;
    hash *= 16777619u;
    hash ^= out_receipt->first_level_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->post_envelope_iso_user_data_offset;
    hash *= 16777619u;
    hash ^= out_receipt->post_envelope_hash;
    hash *= 16777619u;
    out_receipt->receipt_hash = hash;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_decode_initial_level_envelope(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelEnvelopeReceipt *out_receipt) {

    enum {
        initial_level_envelope_bytes = 12u +
            TQR_RAW_INITIAL_LEVEL_WIDTH * TQR_RAW_INITIAL_LEVEL_HEIGHT
    };
    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_Track02LevelHandoffStatus copy_status;
    uint8_t envelope[initial_level_envelope_bytes];
    size_t copied_byte_count = 0u;
    size_t copied_user_data_offset = 0u;
    uint32_t hash;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || track02_size == 0u || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    /* The boundary capture owns the raw-media hash, IPL coordinate, and
     * descriptor-relative candidate gates.  Re-copy through MODE1 user data
     * so this decoder never treats sector framing as level bytes. */
    if (theron_v1_track02_capture_initial_level_object_boundary(
            track02_data, track02_size, md5_hex, &boundary) !=
            THERON_TRACK02_SIGNAL_OK ||
        !boundary.valid ||
        boundary.level_byte_count != initial_level_envelope_bytes) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    copy_status = theron_v1_track02_copy_initial_level_user_data_window(
        track02_data, track02_size, md5_hex,
        boundary.variant == THERON_TRACK02_VARIANT_JP_BIN
            ? g_jp_bin_descriptor_offsets[0]
            : g_us_bin_descriptor_offsets[0],
        envelope, sizeof(envelope), &copied_byte_count,
        &copied_user_data_offset);
    if (copy_status != THERON_TRACK02_LEVEL_HANDOFF_OK ||
        copied_byte_count != sizeof(envelope) ||
        copied_user_data_offset != boundary.level_user_data_offset ||
        rd16be(envelope + 0u) != boundary.level_width ||
        rd16be(envelope + 2u) != boundary.level_height ||
        rd32be(envelope + 4u) != boundary.level_seed ||
        rd16be(envelope + 8u) != boundary.level_index ||
        rd16be(envelope + 10u) != boundary.level_header_extension_be) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    out_receipt->valid = 1;
    out_receipt->variant = boundary.variant;
    out_receipt->track02_record = boundary.track02_record;
    out_receipt->level_raw_offset = boundary.level_raw_offset;
    out_receipt->level_user_data_offset = boundary.level_user_data_offset;
    out_receipt->level_byte_count = boundary.level_byte_count;
    out_receipt->width = boundary.level_width;
    out_receipt->height = boundary.level_height;
    out_receipt->seed = boundary.level_seed;
    out_receipt->level_index = boundary.level_index;
    out_receipt->header_extension_be = boundary.level_header_extension_be;
    out_receipt->grid_offset_in_envelope = 12u;
    out_receipt->grid_byte_count = sizeof(envelope) - 12u;
    out_receipt->grid_hash = tqr_hash_bytes(envelope + 12u,
                                            out_receipt->grid_byte_count);
    out_receipt->header_semantics_proven = 1;
    out_receipt->grid_semantics_proven = 0;
    out_receipt->header_extension_semantics_proven = 0;
    out_receipt->object_tail_semantics_proven = 0;
    out_receipt->fallback_visuals_allowed = 0;

    hash = boundary.receipt_hash;
    hash ^= out_receipt->grid_hash;
    hash *= 16777619u;
    hash ^= (uint32_t)out_receipt->grid_byte_count;
    hash *= 16777619u;
    out_receipt->receipt_hash = hash;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus
theron_v1_track02_decode_initial_level_loader_semantics(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelLoaderSemanticReceipt *out_receipt) {
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || track02_size == 0u || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    /* `0x0b52` reaches the loader through an authenticated dynamic CD_READ,
     * but the available original control-flow evidence ends at the banked
     * `$3800` handoff. The old code parsed a convenient byte shape with the
     * host level loader and treated that as original dungeon semantics. Keep
     * this compatibility entry point closed until a captured game consumer
     * proves the record's grammar, object ownership, and bitmap relation. */
    return THERON_TRACK02_SIGNAL_NOT_FOUND;
}

Theron_Track02SignalStatus theron_v1_track02_decode_initial_level_object_table(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02InitialLevelObjectTableReceipt *out_receipt) {

    Theron_Track02InitialLevelObjectBoundaryReceipt boundary;
    Theron_Track02SignalStatus status;
    Theron_Track02SemanticBindingStatus bind_status;
    size_t i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || track02_size == 0u || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    /* Capture the proven byte boundary.  The JP/US raw Track 02 corpora agree
     * on record 0x0b52, a 0x36c-byte level envelope, and a 0x380-byte tail. */
    status = theron_v1_track02_capture_initial_level_object_boundary(
        track02_data, track02_size, md5_hex, &boundary);
    if (status != THERON_TRACK02_SIGNAL_OK || !boundary.valid) {
        return status;
    }

    out_receipt->valid = 1;
    out_receipt->variant = boundary.variant;
    out_receipt->object_boundary_raw_offset = boundary.object_boundary_raw_offset;
    out_receipt->object_boundary_user_data_offset =
        boundary.object_boundary_user_data_offset;
    out_receipt->following_user_data_bytes_in_record =
        boundary.following_user_data_bytes_in_record;
    out_receipt->following_user_data_hash = boundary.following_user_data_hash;

    /* An empty tail is a source-proven empty object table. */
    if (boundary.following_user_data_bytes_in_record == 0u) {
        out_receipt->object_table_semantics_proven = 1;
        out_receipt->promotion_blocked = 0;
        return THERON_TRACK02_SIGNAL_OK;
    }

    if (boundary.object_boundary_raw_offset > track02_size ||
        boundary.following_user_data_bytes_in_record >
            track02_size - boundary.object_boundary_raw_offset) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    /* Parse the tail as a count-prefixed compact object table. */
    bind_status = theron_v1_track02_read_object_table(
        track02_data + boundary.object_boundary_raw_offset,
        boundary.following_user_data_bytes_in_record,
        &out_receipt->object_table);
    if (bind_status != THERON_TRACK02_SEMANTIC_BINDING_OK &&
        bind_status != THERON_TRACK02_SEMANTIC_BINDING_ZERO_FILL) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    /* Multi-level object-tail semantics: the initial envelope may contain
     * compact records for any level of the starting dungeon (0..2).  The
     * compact-row shape gate above already bounds level_index, so this is a
     * final source-lock confirmation that no record escapes the per-dungeon
     * level range. */
    for (i = 0u; i < out_receipt->object_table.record_count; ++i) {
        if (out_receipt->object_table.records[i].level_index >=
                THERON_MAX_LEVELS_PER_DUNGEON) {
            memset(&out_receipt->object_table, 0,
                   sizeof(out_receipt->object_table));
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
    }

    out_receipt->object_table_semantics_proven = 1;
    out_receipt->promotion_blocked = 0;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_decode_dungeon_level_object_table(
    const Theron_Track02DungeonRoute *route,
    int level_index,
    Theron_Track02ObjectTable *out_table) {

    size_t i;

    if (out_table) {
        memset(out_table, 0, sizeof(*out_table));
    }
    if (!route || !out_table || level_index < 0 ||
        level_index >= THERON_MAX_LEVELS_PER_DUNGEON) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    /* Source-lock: the route must be a complete, authenticated Track 02
     * transaction.  Synthetic routes that were never promoted from real media
     * are rejected by the route status gate, so this decoder stays fail-closed
     * until a genuine non-startup level/object handoff is captured. */
    if (!route->valid || route->status != THERON_TRACK02_DUNGEON_ROUTE_OK) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    out_table->declared_record_count = route->objects.declared_record_count;
    for (i = 0u; i < route->objects.record_count; ++i) {
        const Theron_Track02ObjectTableRecord *rec = &route->objects.records[i];
        if (rec->level_index != (uint8_t)level_index) {
            continue;
        }
        if (out_table->record_count >= THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) {
            ++out_table->overflow_count;
            continue;
        }
        out_table->records[out_table->record_count++] = *rec;
    }

    out_table->byte_count =
        2u + out_table->record_count * THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES;
    out_table->required_byte_count = out_table->byte_count;
    out_table->shape_ok = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_load_initial_level_loader_route(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    int dungeon_id,
    int sub_level_index,
    Theron_Track02InitialLevelLoaderRoute *out_route) {
    uint8_t envelope[12u + TQR_RAW_INITIAL_LEVEL_WIDTH *
                     TQR_RAW_INITIAL_LEVEL_HEIGHT];
    Theron_Track02LevelHandoffStatus copy_status;
    size_t copied_byte_count = 0u;
    size_t copied_user_data_offset = 0u;
    uint32_t hash;

    if (out_route) memset(out_route, 0, sizeof(*out_route));
    if (!track02_data || track02_size == 0u || !md5_hex || !out_route)
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    if (dungeon_id != THERON_DUNGEON_1_AKUTUBA || sub_level_index != 0)
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    if (theron_v1_track02_decode_initial_level_loader_semantics(
            track02_data, track02_size, md5_hex, &out_route->semantics) !=
            THERON_TRACK02_SIGNAL_OK || !out_route->semantics.valid ||
        !out_route->semantics.loader_grid_semantics_proven) {
        memset(out_route, 0, sizeof(*out_route));
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    copy_status = theron_v1_track02_copy_initial_level_user_data_window(
        track02_data, track02_size, md5_hex,
        out_route->semantics.envelope.variant == THERON_TRACK02_VARIANT_JP_BIN
            ? g_jp_bin_descriptor_offsets[0] : g_us_bin_descriptor_offsets[0],
        envelope, sizeof(envelope), &copied_byte_count, &copied_user_data_offset);
    if (copy_status != THERON_TRACK02_LEVEL_HANDOFF_OK ||
        copied_byte_count != sizeof(envelope) ||
        copied_user_data_offset != out_route->semantics.envelope.level_user_data_offset ||
        theron_v1_level_load(&out_route->level, envelope, (int)sizeof(envelope),
                             dungeon_id, sub_level_index) != THERON_MAP_OK ||
        out_route->level.start_x != out_route->semantics.start_x ||
        out_route->level.start_y != out_route->semantics.start_y ||
        out_route->level.start_dir != out_route->semantics.start_dir) {
        memset(out_route, 0, sizeof(*out_route));
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_route->dungeon_id = dungeon_id;
    out_route->sub_level_index = sub_level_index;
    out_route->fallback_visuals_allowed = 0;
    hash = out_route->semantics.receipt_hash;
    hash ^= (uint32_t)dungeon_id;
    hash *= 16777619u;
    hash ^= (uint32_t)sub_level_index;
    hash *= 16777619u;
    out_route->route_hash = hash;
    out_route->valid = 1;
    return THERON_TRACK02_SIGNAL_OK;
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
        /* Multi-level object-tail semantics: records may reference any level
         * of the current dungeon (0..THERON_MAX_LEVELS_PER_DUNGEON-1) and any
         * coordinate within the TQR 32x32 map envelope.  The initial 32x27
         * startup grid is a subset that still satisfies these bounds. */
        if (record->x >= THERON_MAX_MAP_SIZE) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_X_OUT_OF_RANGE;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }
        if (record->y >= THERON_MAX_MAP_SIZE) {
            out_table->first_bad_record_index = i;
            out_table->reject_reason =
                THERON_TRACK02_OBJECT_TABLE_REJECT_Y_OUT_OF_RANGE;
            out_table->checksum = checksum;
            return THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE;
        }
        if (record->level_index >= THERON_MAX_LEVELS_PER_DUNGEON) {
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
    /* Unknown/bad input is not permission to render invented visuals. */
    receipt->fallback_visuals_allowed = 0;
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
                }
            }
            if (semantic_role == THERON_TRACK02_SEMANTIC_ROLE_UNKNOWN &&
                entries[entry_index].role ==
                    THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA &&
                entries[entry_index].byte_count > 0u) {
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
        if (!out_receipt->object_table_decode_ready) {
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
    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
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
    /* Unknown/bad input is not permission to render invented visuals. */
    receipt->fallback_visuals_allowed = 0;
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
                                        [anchor] == 0u) {
                            out_receipt
                                ->nonstartup_level_candidate_entry_index
                                    [anchor] = entry_index;
                            out_receipt
                                ->nonstartup_level_candidate_raw_offsets
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
                                ->nonstartup_level_candidate_byte_counts
                                    [anchor] =
                                entries[entry_index].byte_count;
                            out_receipt
                                ->nonstartup_level_candidate_nonzero_byte_counts
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
    }

    out_receipt->descriptor_route_ready =
        signal.anchor_count > 0u &&
        out_receipt->descriptor_anchor_mask ==
            ((1u << (unsigned)signal.anchor_count) - 1u);
    out_receipt->blocked_for_missing_nonstartup_level_evidence =
        out_receipt->descriptor_route_ready &&
        !out_receipt->nonstartup_level_decode_ready;
    if (out_receipt->descriptor_route_ready &&
        !out_receipt->nonstartup_level_decode_ready) {
        out_receipt->nonstartup_level_blocked_anchor_count =
            out_receipt->descriptor_anchor_count;
        out_receipt->nonstartup_level_blocked_anchor_mask =
            out_receipt->descriptor_anchor_mask;
    }
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

    /* The only positive level record currently correlated with the original
     * loader is the AKUTUBA entry record.  The shared 32x27 envelope
     * is not evidence for a different dungeon merely because callers can
     * supply another dungeon id to the generic level loader. */
    if (dungeon_id != THERON_DUNGEON_1_AKUTUBA ||
        sub_level_index != 0) {
        return THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
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

    out_level_handoff->loaded = 1;
    return THERON_TRACK02_LEVEL_HANDOFF_OK;
}

const char *theron_v1_track02_graphics_format_name(
    Theron_Track02GraphicsFormat format) {
    switch (format) {
    case THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP:
        return "huc6260-palette-4bpp";
    case THERON_TRACK02_GRAPHICS_FORMAT_LE16_STRIDE_TABLE:
        return "le16-stride-table";
    case THERON_TRACK02_GRAPHICS_FORMAT_UNKNOWN:
    default:
        return "unknown";
    }
}

static int tqr_sector_user_data_is_nonzero(const uint8_t *bytes) {
    size_t i;
    for (i = 0u; i < TQR_RAW_SECTOR_USER_DATA_BYTES; ++i) {
        if (bytes[i] != 0u) return 1;
    }
    return 0;
}

static int tqr_palette_candidate_shape(const uint8_t *bytes,
                                       size_t *out_distinct_nonblack) {
    uint16_t seen[THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT];
    size_t seen_count = 0u;
    size_t i;

    if (out_distinct_nonblack) *out_distinct_nonblack = 0u;
    if (rd16le(bytes) != 0u) return 0;
    for (i = 0u; i < THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT; ++i) {
        const uint16_t word = rd16le(bytes + i * 2u);
        size_t j;
        if ((word & 0xfe00u) != 0u) return 0;
        if (word == 0u) continue;
        for (j = 0u; j < seen_count; ++j) {
            if (seen[j] == word) break;
        }
        if (j == seen_count) seen[seen_count++] = word;
    }
    if (out_distinct_nonblack) *out_distinct_nonblack = seen_count;
    return seen_count >= 8u;
}

static int tqr_le16_stride_table_shape(const uint8_t *bytes,
                                       uint16_t *out_stride) {
    uint16_t previous = rd16le(bytes);
    uint16_t stride;
    size_t i;

    stride = (uint16_t)(rd16le(bytes + 2u) - previous);
    if (stride == 0u || (stride & 0x001fu) != 0u) return 0;
    for (i = 1u; i < 8u; ++i) {
        const uint16_t current = rd16le(bytes + i * 2u);
        if (current <= previous || (uint16_t)(current - previous) != stride) {
            return 0;
        }
        previous = current;
    }
    if (out_stride) *out_stride = stride;
    return 1;
}

static void tqr_catalog_graphics_candidate(
    Theron_Track02GraphicsFormatCatalog *catalog,
    Theron_Track02GraphicsFormat format,
    size_t jp_raw_offset,
    size_t us_raw_offset,
    size_t jp_user_data_offset,
    size_t us_user_data_offset,
    const uint8_t *bytes,
    uint16_t stride,
    size_t nonblack_or_distinct_count) {
    Theron_Track02GraphicsFormatCandidate *candidate;

    if (format == THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP) {
        ++catalog->huc6260_palette_candidate_count;
    } else if (format == THERON_TRACK02_GRAPHICS_FORMAT_LE16_STRIDE_TABLE) {
        ++catalog->le16_stride_table_candidate_count;
    }

    if (catalog->candidate_count >= THERON_TRACK02_MAX_GRAPHICS_FORMAT_CANDIDATES) {
        ++catalog->overflow_count;
        return;
    }
    candidate = &catalog->candidates[catalog->candidate_count++];
    candidate->format = format;
    candidate->jp_raw_offset = jp_raw_offset;
    candidate->us_raw_offset = us_raw_offset;
    candidate->jp_user_data_offset = jp_user_data_offset;
    candidate->us_user_data_offset = us_user_data_offset;
    candidate->byte_count = format == THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP
        ? THERON_TRACK02_4BPP_PALETTE_BYTES : 16u;
    candidate->payload_checksum = tqr_hash_bytes(bytes, candidate->byte_count);
    candidate->first_word = rd16le(bytes);
    candidate->stride = stride;
    candidate->value_count = format == THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP
        ? THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT : 8u;
    candidate->nonblack_or_distinct_count = nonblack_or_distinct_count;
}

Theron_Track02SignalStatus theron_v1_track02_catalog_graphics_format_candidates(
    const uint8_t *jp_track02_data,
    size_t jp_track02_size,
    const char *jp_md5_hex,
    const uint8_t *us_track02_data,
    size_t us_track02_size,
    const char *us_md5_hex,
    Theron_Track02GraphicsFormatCatalog *out_catalog) {
    size_t jp_sector_count;
    size_t us_sector_count;
    size_t jp_sector;

    if (out_catalog) memset(out_catalog, 0, sizeof(*out_catalog));
    if (!jp_track02_data || !us_track02_data || !jp_md5_hex || !us_md5_hex ||
        !out_catalog || jp_track02_size == 0u || us_track02_size == 0u ||
        jp_track02_size % TQR_RAW_SECTOR_BYTES != 0u ||
        us_track02_size % TQR_RAW_SECTOR_BYTES != 0u) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    if (theron_v1_track02_variant_for_md5(jp_md5_hex) != THERON_TRACK02_VARIANT_JP_BIN ||
        theron_v1_track02_variant_for_md5(us_md5_hex) != THERON_TRACK02_VARIANT_US_BIN) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    jp_sector_count = jp_track02_size / TQR_RAW_SECTOR_BYTES;
    us_sector_count = us_track02_size / TQR_RAW_SECTOR_BYTES;
    if (jp_sector_count == 0u || us_sector_count <= 1u) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_catalog->jp_variant = THERON_TRACK02_VARIANT_JP_BIN;
    out_catalog->us_variant = THERON_TRACK02_VARIANT_US_BIN;
    out_catalog->compared_sector_count =
        jp_sector_count < us_sector_count - 1u ? jp_sector_count : us_sector_count - 1u;

    for (jp_sector = 0u; jp_sector < out_catalog->compared_sector_count; ++jp_sector) {
        const uint8_t *jp_bytes = jp_track02_data + jp_sector * TQR_RAW_SECTOR_BYTES +
            TQR_RAW_SECTOR_USER_DATA_OFFSET;
        const uint8_t *us_bytes = us_track02_data + (jp_sector + 1u) * TQR_RAW_SECTOR_BYTES +
            TQR_RAW_SECTOR_USER_DATA_OFFSET;
        size_t offset;
        if (memcmp(jp_bytes, us_bytes, TQR_RAW_SECTOR_USER_DATA_BYTES) != 0 ||
            !tqr_sector_user_data_is_nonzero(jp_bytes)) {
            continue;
        }
        ++out_catalog->matching_nonzero_sector_count;
        for (offset = 0u; offset + THERON_TRACK02_4BPP_PALETTE_BYTES <=
                              TQR_RAW_SECTOR_USER_DATA_BYTES; offset += 2u) {
            size_t distinct_nonblack = 0u;
            uint16_t stride = 0u;
            const uint8_t *candidate = jp_bytes + offset;
            const size_t jp_raw = jp_sector * TQR_RAW_SECTOR_BYTES +
                TQR_RAW_SECTOR_USER_DATA_OFFSET + offset;
            const size_t us_raw = (jp_sector + 1u) * TQR_RAW_SECTOR_BYTES +
                TQR_RAW_SECTOR_USER_DATA_OFFSET + offset;
            const size_t jp_user = jp_sector * TQR_RAW_SECTOR_USER_DATA_BYTES + offset;
            const size_t us_user = (jp_sector + 1u) * TQR_RAW_SECTOR_USER_DATA_BYTES + offset;
            if (tqr_palette_candidate_shape(candidate, &distinct_nonblack)) {
                tqr_catalog_graphics_candidate(out_catalog,
                                                THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP,
                                                jp_raw, us_raw, jp_user, us_user,
                                                candidate, 0u, distinct_nonblack);
            }
            if (tqr_le16_stride_table_shape(candidate, &stride)) {
                tqr_catalog_graphics_candidate(out_catalog,
                                                THERON_TRACK02_GRAPHICS_FORMAT_LE16_STRIDE_TABLE,
                                                jp_raw, us_raw, jp_user, us_user,
                                                candidate, stride, 8u);
            }
        }
    }

    /* No byte signature proves a compression stream: magic-less LZ/RLE
     * schemes require original-loader control-flow evidence first. */
    out_catalog->compression_signature_detected = 0;
    out_catalog->source_loader_binding_verified = 0;
    out_catalog->decoder_blocked = 1;
    out_catalog->valid = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

int theron_v1_track02_graphics_format_catalog_can_decode(
    const Theron_Track02GraphicsFormatCatalog *catalog) {
    return catalog && catalog->valid && catalog->candidate_count == 1u &&
           catalog->overflow_count == 0u &&
           catalog->source_loader_binding_verified &&
           !catalog->decoder_blocked;
}

/* PC Engine CD-ROM IPL information records are relative to Track 02 INDEX 01.
 * The two raw dumps have different pregap lengths, recorded by their CUEs:
 * JP INDEX 01 00:02:74 = sector 224; US INDEX 01 00:03:00 = sector 225.
 * The bytes and API setup below were inspected from the known-MD5 original
 * media.  They are not inferred from any candidate graphics window. */
#define TQR_IPL_INFORMATION_SECTOR_DELTA 1u
#define TQR_IPL_JP_EXECUTABLE_SECTORS 3u
#define TQR_IPL_US_EXECUTABLE_SECTORS 4u
#define TQR_IPL_INFORMATION_SIGNATURE_OFFSET 32u
#define TQR_IPL_INFORMATION_SIGNATURE "PC Engine CD-ROM SYSTEM"
#define TQR_IPL_CD_EXEC_USER_OFFSET 0x80u
#define TQR_IPL_CD_READ_USER_OFFSET 0xc1u
#define TQR_IPL_STAGE2_CD_READ_USER_OFFSET 0x80u

static int tqr_ipl_user_byte(const uint8_t *data,
                             size_t data_size,
                             size_t first_sector,
                             size_t sector_count,
                             size_t user_offset,
                             uint8_t *out_byte) {
    size_t sector_delta;
    size_t offset_in_sector;
    size_t raw_offset;

    if (!data || !out_byte || user_offset >=
        sector_count * TQR_RAW_SECTOR_USER_DATA_BYTES) {
        return 0;
    }
    sector_delta = user_offset / TQR_RAW_SECTOR_USER_DATA_BYTES;
    offset_in_sector = user_offset % TQR_RAW_SECTOR_USER_DATA_BYTES;
    if (first_sector > (SIZE_MAX / TQR_RAW_SECTOR_BYTES) - sector_delta) {
        return 0;
    }
    raw_offset = (first_sector + sector_delta) * TQR_RAW_SECTOR_BYTES;
    if (raw_offset > data_size ||
        TQR_RAW_SECTOR_USER_DATA_OFFSET + offset_in_sector >= data_size - raw_offset) {
        return 0;
    }
    *out_byte = data[raw_offset + TQR_RAW_SECTOR_USER_DATA_OFFSET + offset_in_sector];
    return 1;
}

static int tqr_ipl_user_match(const uint8_t *data,
                              size_t data_size,
                              size_t first_sector,
                              size_t sector_count,
                              size_t user_offset,
                              const uint8_t *expected,
                              size_t expected_size) {
    size_t i;
    uint8_t byte;

    if (!expected) return 0;
    for (i = 0u; i < expected_size; ++i) {
        if (!tqr_ipl_user_byte(data, data_size, first_sector, sector_count,
                               user_offset + i, &byte) || byte != expected[i]) {
            return 0;
        }
    }
    return 1;
}

static uint32_t tqr_ipl_user_hash(const uint8_t *data,
                                  size_t data_size,
                                  size_t first_sector,
                                  size_t sector_count) {
    uint32_t hash = 2166136261u;
    size_t user_size = sector_count * TQR_RAW_SECTOR_USER_DATA_BYTES;
    size_t i;
    uint8_t byte;

    for (i = 0u; i < user_size; ++i) {
        if (!tqr_ipl_user_byte(data, data_size, first_sector, sector_count, i, &byte)) {
            return 0u;
        }
        hash ^= byte;
        hash *= 16777619u;
    }
    return hash;
}

Theron_Track02SignalStatus theron_v1_track02_find_ipl_loader(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02IplLoaderReceipt *out_receipt) {
    static const uint8_t information_prefix[] = {
        0x00u, 0x03u, 0xa3u
    };
    static const uint8_t cd_read_setup[] = {
        0xa9u, 0x00u, 0x85u, 0xfau,
        0xa9u, 0x30u, 0x85u, 0xfbu,
        0xa9u, 0x01u, 0x85u, 0xffu,
        0x20u, 0x09u, 0xe0u
    };
    static const uint8_t cd_exec_setup[] = {
        0x82u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfcu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfeu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xfdu,
        0xe8u, 0xbdu, 0xd5u, 0x40u, 0x85u, 0xf8u,
        0xa9u, 0x00u, 0x85u, 0xfau,
        0xa9u, 0x40u, 0x85u, 0xfbu,
        0xa9u, 0x01u, 0x85u, 0xffu,
        0x20u, 0x0fu, 0xe0u
    };
    static const uint8_t cd_exec_record[] = {0x00u, 0xe7u, 0x03u, 0x11u};
    static const uint8_t stage2_cd_read_setup[] = {
        0xa9u, 0x01u, 0x85u, 0xf8u,
        0xa9u, 0x01u, 0x85u, 0xffu,
        0xa9u, 0x00u, 0x85u, 0xfau,
        0xa9u, 0x38u, 0x85u, 0xfbu,
        0x20u, 0x09u, 0xe0u
    };
    /* BRA $4080: the CD_EXEC failure path re-enters its table-reader loop
     * head. */
    static const uint8_t cd_exec_retry_branch[] = {0x80u, 0xd7u};
    /* CLX; LDA $40dc,X/STA $fc,$fe,$fd,$f8: the CD_READ preload table is
     * loaded through the same zero-page argument map the CD_EXEC setup
     * uses. */
    static const uint8_t cd_read_table_load[] = {
        0x82u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xfcu,
        0xe8u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xfeu,
        0xe8u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xfdu,
        0xe8u, 0xbdu, 0xdcu, 0x40u, 0x85u, 0xf8u
    };
    /* JSR $40ae: the init-path invocation of the stage-two register-seed
     * subroutine. */
    static const uint8_t stage2_seed_call[] = {0x20u, 0xaeu, 0x40u};
    /* BSR +0x2e from $4080: the retry-path invocation lands exactly on the
     * $40ae seed body. */
    static const uint8_t stage2_seed_bsr[] = {0x44u, 0x2eu};
    Theron_Track02Variant variant;
    size_t index01_sector;
    size_t executable_sector_count;
    size_t information_sector;
    size_t executable_sector;
    size_t stage2_sector;
    uint32_t stage2_cd_read_record;
    uint8_t count;
    uint8_t load_lo;
    uint8_t load_hi;
    uint8_t entry_lo;
    uint8_t entry_hi;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt || track02_size == 0u ||
        track02_size % TQR_RAW_SECTOR_BYTES != 0u) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    variant = theron_v1_track02_variant_for_md5(md5_hex);
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        index01_sector = THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR;
        executable_sector_count = TQR_IPL_JP_EXECUTABLE_SECTORS;
    } else if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        index01_sector = THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR;
        executable_sector_count = TQR_IPL_US_EXECUTABLE_SECTORS;
    } else {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }
    stage2_cd_read_record =
        variant == THERON_TRACK02_VARIANT_JP_BIN
            ? THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP
            : THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US;
    if (index01_sector > SIZE_MAX - TQR_IPL_INFORMATION_SECTOR_DELTA ||
        index01_sector > SIZE_MAX - THERON_TRACK02_IPL_RECORD ||
        index01_sector > SIZE_MAX - THERON_TRACK02_IPL_STAGE2_RECORD) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    information_sector = index01_sector + TQR_IPL_INFORMATION_SECTOR_DELTA;
    executable_sector = index01_sector + THERON_TRACK02_IPL_RECORD;
    stage2_sector = index01_sector + THERON_TRACK02_IPL_STAGE2_RECORD;
    if (!tqr_ipl_user_match(track02_data, track02_size, information_sector, 1u,
                            0u, information_prefix, sizeof(information_prefix)) ||
        !tqr_ipl_user_match(track02_data, track02_size, information_sector, 1u,
                            TQR_IPL_INFORMATION_SIGNATURE_OFFSET,
                            (const uint8_t *)TQR_IPL_INFORMATION_SIGNATURE,
                            sizeof(TQR_IPL_INFORMATION_SIGNATURE) - 1u) ||
        !tqr_ipl_user_byte(track02_data, track02_size, information_sector, 1u,
                           3u, &count) ||
        !tqr_ipl_user_byte(track02_data, track02_size, information_sector, 1u,
                           4u, &load_lo) ||
        !tqr_ipl_user_byte(track02_data, track02_size, information_sector, 1u,
                           5u, &load_hi) ||
        !tqr_ipl_user_byte(track02_data, track02_size, information_sector, 1u,
                           6u, &entry_lo) ||
        !tqr_ipl_user_byte(track02_data, track02_size, information_sector, 1u,
                           7u, &entry_hi) ||
        count != executable_sector_count || load_lo != 0x00u || load_hi != 0x40u ||
        entry_lo != 0x00u || entry_hi != 0x40u ||
        !tqr_ipl_user_match(track02_data, track02_size, executable_sector,
                            executable_sector_count, TQR_IPL_CD_READ_USER_OFFSET,
                            cd_read_setup, sizeof(cd_read_setup)) ||
        !tqr_ipl_user_match(track02_data, track02_size, executable_sector,
                            executable_sector_count, TQR_IPL_CD_EXEC_USER_OFFSET,
                            cd_exec_setup, sizeof(cd_exec_setup)) ||
        !tqr_ipl_user_match(track02_data, track02_size, executable_sector,
                            executable_sector_count, 0xd5u, cd_exec_record,
                            sizeof(cd_exec_record)) ||
        !tqr_ipl_user_match(track02_data, track02_size, stage2_sector,
                            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
                            TQR_IPL_STAGE2_CD_READ_USER_OFFSET,
                            stage2_cd_read_setup, sizeof(stage2_cd_read_setup)) ||
        !tqr_ipl_user_match(track02_data, track02_size, executable_sector,
                            executable_sector_count,
                            THERON_TRACK02_IPL_CD_EXEC_RETRY_USER_OFFSET,
                            cd_exec_retry_branch,
                            sizeof(cd_exec_retry_branch)) ||
        !tqr_ipl_user_match(track02_data, track02_size, executable_sector,
                            executable_sector_count,
                            THERON_TRACK02_IPL_CD_READ_TABLE_LOAD_USER_OFFSET,
                            cd_read_table_load, sizeof(cd_read_table_load)) ||
        !tqr_ipl_user_match(track02_data, track02_size, stage2_sector,
                            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
                            THERON_TRACK02_IPL_STAGE2_SEED_CALL_USER_OFFSET,
                            stage2_seed_call, sizeof(stage2_seed_call)) ||
        !tqr_ipl_user_match(track02_data, track02_size, stage2_sector,
                            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
                            THERON_TRACK02_IPL_STAGE2_SEED_BSR_USER_OFFSET,
                            stage2_seed_bsr, sizeof(stage2_seed_bsr))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    out_receipt->valid = 1;
    out_receipt->variant = variant;
    out_receipt->data_track_index01_raw_sector = index01_sector;
    out_receipt->information_raw_sector = information_sector;
    out_receipt->executable_raw_sector = executable_sector;
    out_receipt->executable_sector_count = executable_sector_count;
    out_receipt->executable_user_data_bytes =
        executable_sector_count * TQR_RAW_SECTOR_USER_DATA_BYTES;
    out_receipt->executable_user_data_hash = tqr_ipl_user_hash(
        track02_data, track02_size, executable_sector, executable_sector_count);
    out_receipt->record = THERON_TRACK02_IPL_RECORD;
    out_receipt->load_address = THERON_TRACK02_IPL_LOAD_ADDRESS;
    out_receipt->entry_address = THERON_TRACK02_IPL_LOAD_ADDRESS;
    out_receipt->cd_read_user_data_offset = TQR_IPL_CD_READ_USER_OFFSET;
    out_receipt->cd_read_cpu_address = THERON_TRACK02_IPL_CD_READ_CPU_ADDRESS;
    out_receipt->cd_read_system_card_address =
        THERON_TRACK02_IPL_CD_READ_SYSTEM_CARD_ADDRESS;
    out_receipt->cd_read_destination = THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM;
    out_receipt->cd_read_local_destination =
        THERON_TRACK02_IPL_CD_READ_LOCAL_DESTINATION;
    out_receipt->cd_exec_cpu_address = THERON_TRACK02_IPL_CD_EXEC_CPU_ADDRESS;
    out_receipt->cd_exec_system_card_address =
        THERON_TRACK02_IPL_CD_EXEC_SYSTEM_CARD_ADDRESS;
    out_receipt->stage2_record = THERON_TRACK02_IPL_STAGE2_RECORD;
    out_receipt->stage2_sector_count = THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT;
    out_receipt->stage2_destination = THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM;
    out_receipt->stage2_load_address = THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS;
    /* The authenticated CD_EXEC sequence at $40a4 enters the exact local
     * destination it loads.  This canonical handoff does not identify any
     * subsequent stage-two record, payload, or rendering route. */
    out_receipt->stage2_entry_address = THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->stage2_user_data_bytes =
        THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT * TQR_RAW_SECTOR_USER_DATA_BYTES;
    out_receipt->stage2_user_data_hash = tqr_ipl_user_hash(
        track02_data, track02_size, stage2_sector,
        THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT);
    out_receipt->stage2_cd_read_cpu_address =
        THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS;
    out_receipt->stage2_cd_read_sector_count = 1u;
    out_receipt->stage2_cd_read_destination = THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM;
    out_receipt->stage2_cd_read_local_destination =
        THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION;
    /* Original CUE runtime trace, captured with Mednafen's HuC6280 debugger:
     * JP LBA $1205 -> Track 02 record $4df and US LBA $10a1 -> record $4e0.
     * These are the live CL/DL/CH values consumed by the authenticated
     * $4090 call, not a static-byte inference. */
    out_receipt->stage2_cd_read_record = stage2_cd_read_record;
    out_receipt->stage2_cd_read_raw_sector = stage2_cd_read_record;
    out_receipt->stage2_cd_read_record_proven = 1;
    /* Authenticated bytes at $4080-$4092 prove the local-RAM call shape;
     * the original runtime trace supplies the formerly live record value. */
    out_receipt->stage2_cd_read_dynamic_boundary_valid = 1;
    out_receipt->stage2_cd_read_live_record_register_mask =
        THERON_TRACK02_IPL_STAGE2_LIVE_RECORD_MASK;
    /* The four static windows above completed byte-for-byte, so both
     * original loader read windows are now fully bound: stage one
     * [0xa9..0xd4] joins the CD_READ preload-table load to the existing
     * cd_read_setup and preload_return patterns, and stage two
     * [0x7e..0xb4] plus the 0x29 init call joins both seed invocations to
     * the existing setup, post_read, and seed patterns.  This binds only
     * the instruction bytes; no System Card base arithmetic, record
     * semantics, or graphics role follows. */
    out_receipt->cd_exec_retry_branch_proven = 1;
    out_receipt->cd_read_table_load_proven = 1;
    out_receipt->stage2_seed_call_sites_proven = 1;
    /* This initial loader call has DH=1 (local), never the System Card's
     * VRAM values DH=FE/FF.  It cannot authorize a graphics transfer. */
    out_receipt->vram_transfer_proven = 0;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_inspect_stage2_dynamic_payload(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2DynamicPayloadReceipt *out_receipt) {
    Theron_Track02IplLoaderReceipt loader;
    size_t raw_offset;
    size_t user_offset;
    size_t i;
    const uint8_t *payload;
    Theron_Track02SignalStatus status;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    if (!loader.valid || !loader.stage2_cd_read_record_proven ||
        loader.stage2_cd_read_raw_sector >
            (SIZE_MAX - TQR_RAW_SECTOR_USER_DATA_OFFSET) / TQR_RAW_SECTOR_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    raw_offset = loader.stage2_cd_read_raw_sector * TQR_RAW_SECTOR_BYTES;
    user_offset = raw_offset + TQR_RAW_SECTOR_USER_DATA_OFFSET;
    if (user_offset > track02_size ||
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES >
            track02_size - user_offset) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    payload = track02_data + user_offset;
    if (rd16be(payload) != 0x00ffu || rd16be(payload + 2u) != 0x0308u) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES < 4u ||
        (THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES - 4u) /
                THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_BYTES !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        (THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES - 4u) %
                THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_BYTES != 0u) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    for (i = THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES;
         i < THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES;
         ++i) {
        if (payload[i] != 0u) return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->track02_record = loader.stage2_cd_read_record;
    out_receipt->raw_sector = loader.stage2_cd_read_raw_sector;
    out_receipt->raw_offset = raw_offset;
    out_receipt->user_data_offset = user_offset;
    out_receipt->user_data_bytes = THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES;
    out_receipt->header_word0 = rd16be(payload);
    out_receipt->header_word1 = rd16be(payload + 2u);
    out_receipt->manifest_bytes = THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES;
    out_receipt->manifest_entry_count =
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT;
    out_receipt->nonzero_byte_count = tqr_count_nonzero_bytes(
        payload, THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES);
    out_receipt->user_data_hash = tqr_hash_bytes(
        payload, THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES);
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_entry_path(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2EntryPathReceipt *out_receipt) {
    /* Entry prologue [0x00..0x29): SEI, stack reset, BIT-flag reads, the
     * MPR page map ($FFF5-derived TAM #$08/#$10/#$20/#$40) around the
     * L8000 call, the L40B7 call, and the System Card entry calls up to
     * the already-bound register-seed JSR at 0x29. */
    static const uint8_t stage2_entry_prologue[] = {
        0x78u, 0xa2u, 0xffu, 0x9au, 0xadu, 0xf5u, 0xffu, 0x1au,
        0x53u, 0x08u, 0x1au, 0x1au, 0x1au, 0x53u, 0x10u, 0x48u,
        0x58u, 0x20u, 0x00u, 0x80u, 0x68u, 0x1au, 0x53u, 0x20u,
        0x1au, 0x53u, 0x40u, 0x20u, 0xb7u, 0x40u, 0xeau, 0x20u,
        0x42u, 0xe0u, 0x62u, 0x20u, 0x2du, 0xe0u, 0x20u, 0x18u,
        0xe0u
    };
    /* Main path [0x2c..0x7e): post-seed init, interrupt-mask paging,
     * the L4B2D/L4B73 calls, display/list System Card entries, the
     * bounded $220c TII clear, and the $2700->$2000 TII copy up to the
     * already-bound retry-head seed BSR at 0x7e. */
    static const uint8_t stage2_main_path[] = {
        0x20u, 0x0cu, 0xe0u, 0x78u, 0x64u, 0xf5u, 0xa2u, 0xffu,
        0x9au, 0x58u, 0xa9u, 0x10u, 0x85u, 0xffu, 0x20u, 0xd8u,
        0xe0u, 0xa9u, 0x01u, 0x85u, 0xffu, 0x20u, 0xd8u, 0xe0u,
        0x20u, 0x2du, 0x4bu, 0x20u, 0x73u, 0x4bu, 0xa9u, 0x00u,
        0x20u, 0x6cu, 0xe0u, 0xa9u, 0x00u, 0xa2u, 0x20u, 0xa0u,
        0x1eu, 0x20u, 0x6fu, 0xe0u, 0x20u, 0x78u, 0xe0u, 0x20u,
        0x81u, 0xe0u, 0x20u, 0x99u, 0xe0u, 0xa9u, 0x10u, 0x20u,
        0x9cu, 0xe0u, 0x62u, 0x20u, 0x69u, 0xe0u, 0x9cu, 0x0cu,
        0x22u, 0x73u, 0x0cu, 0x22u, 0x0du, 0x22u, 0x07u, 0x00u,
        0x20u, 0x7bu, 0xe0u, 0x73u, 0x00u, 0x27u, 0x00u, 0x20u,
        0x80u, 0x00u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The entry-stream byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same stream. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_ENTRY_PROLOGUE_USER_OFFSET,
            stage2_entry_prologue, sizeof(stage2_entry_prologue)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_MAIN_PATH_USER_OFFSET,
            stage2_main_path, sizeof(stage2_main_path))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (THERON_TRACK02_IPL_STAGE2_ENTRY_PROLOGUE_USER_OFFSET +
            THERON_TRACK02_IPL_STAGE2_ENTRY_PROLOGUE_BYTES !=
            THERON_TRACK02_IPL_STAGE2_SEED_CALL_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_SEED_CALL_USER_OFFSET + 3u !=
            THERON_TRACK02_IPL_STAGE2_MAIN_PATH_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_MAIN_PATH_USER_OFFSET +
            THERON_TRACK02_IPL_STAGE2_MAIN_PATH_BYTES !=
            THERON_TRACK02_IPL_STAGE2_SEED_BSR_USER_OFFSET) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->entry_path_prologue_bytes =
        THERON_TRACK02_IPL_STAGE2_ENTRY_PROLOGUE_BYTES;
    out_receipt->entry_path_main_path_bytes =
        THERON_TRACK02_IPL_STAGE2_MAIN_PATH_BYTES;
    out_receipt->entry_path_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES;
    out_receipt->entry_prologue_proven = 1;
    out_receipt->main_path_proven = 1;
    out_receipt->entry_path_contiguous_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_call_graph(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2CallGraphReceipt *out_receipt) {
    /* L40B7 command-dispatch loop [0xb7..0xf1): zero-page clear, the
     * L4814 pointer-setup call (at 0xb9, inside this window), the L4F5E
     * selector, the $6000 stream-pointer seed, the L4AF7/LE063 loop
     * head, the $2228 status mask, and the L410D jump-table dispatch
     * with its advance-and-reloop tail. */
    static const uint8_t stage2_dispatcher[] = {
        0x64u, 0x02u, 0x20u, 0x14u, 0x48u, 0x9cu, 0xc1u, 0x4eu,
        0xa9u, 0x02u, 0x20u, 0x5eu, 0x4fu, 0xa9u, 0x00u, 0x85u,
        0x1cu, 0xa9u, 0x60u, 0x85u, 0x1du, 0x20u, 0xf7u, 0x4au,
        0x20u, 0x63u, 0xe0u, 0xadu, 0x28u, 0x22u, 0x29u, 0x0cu,
        0xf0u, 0x03u, 0xeau, 0xeau, 0xeau, 0xc2u, 0xb2u, 0x1cu,
        0x0au, 0xaau, 0x7cu, 0x0du, 0x41u, 0x18u, 0x65u, 0x1cu,
        0x85u, 0x1cu, 0x62u, 0x65u, 0x1du, 0x85u, 0x1du, 0x80u,
        0xdcu, 0x60u
    };
    /* L4B2D count-down delay [0xb2d..0xb3c): register save, the nested
     * DEX/BNE inner loop and DEC/BNE outer loop, and restore/return. */
    static const uint8_t stage2_delay[] = {
        0x48u, 0xdau, 0xa9u, 0xffu, 0xa2u, 0xffu, 0xcau, 0xd0u,
        0xfdu, 0x3au, 0xd0u, 0xf8u, 0xfau, 0x68u, 0x60u
    };
    /* L4B73 port clear [0xb73..0xb96): SEI, the st0/st1/st2 selects,
     * the 256x120-iteration st1/st2 zero-fill loop, and the $F3
     * mask/CLI tail.  The st0/st1/st2 opcodes are bound as instruction
     * bytes only; no video-register role is claimed. */
    static const uint8_t stage2_port_clear[] = {
        0x78u, 0x03u, 0x00u, 0x13u, 0x00u, 0x23u, 0x08u, 0x03u,
        0x02u, 0x82u, 0xa0u, 0x78u, 0x13u, 0x00u, 0x23u, 0x00u,
        0xcau, 0xd0u, 0xf9u, 0x88u, 0xd0u, 0xf6u, 0x03u, 0x05u,
        0xa5u, 0xf3u, 0x29u, 0x3fu, 0x85u, 0xf3u, 0x8du, 0x02u,
        0x00u, 0x58u, 0x60u
    };
    /* L4814 zero-page pointer setup [0x814..0x842): seeds the $D337
     * source pointer, derives the $22/$23/$24 triplet, sets the $2800
     * argument pointer and the $1E/$25 flags, then calls L383E.  The
     * two bytes at 0x81f-0x820 are the da65 `.byte $A5`/`sxy` decode
     * pair, bound to the authenticated media bytes. */
    static const uint8_t stage2_pointer_setup[] = {
        0xa9u, 0xd3u, 0x85u, 0x00u, 0xa9u, 0x37u, 0x85u, 0x01u,
        0x18u, 0xa0u, 0x01u, 0xa5u, 0x02u, 0x71u, 0x00u, 0x85u,
        0x24u, 0xc8u, 0x62u, 0x71u, 0x00u, 0x85u, 0x23u, 0x62u,
        0x72u, 0x00u, 0x85u, 0x22u, 0xa9u, 0x00u, 0x85u, 0x20u,
        0xa9u, 0x28u, 0x85u, 0x21u, 0xa9u, 0x01u, 0x85u, 0x1eu,
        0x85u, 0x25u, 0x20u, 0x3eu, 0x38u, 0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The callee-stream byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET,
            stage2_dispatcher, sizeof(stage2_dispatcher)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_DELAY_USER_OFFSET,
            stage2_delay, sizeof(stage2_delay)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_USER_OFFSET,
            stage2_port_clear, sizeof(stage2_port_clear)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_USER_OFFSET,
            stage2_pointer_setup, sizeof(stage2_pointer_setup))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Every proven call site sits inside an already-bound window: the
     * L40B7 call at 0x1e, the L4B2D call at 0x52, and the L4B73 call at
     * 0x55 are inside the contiguously bound executed entry path
     * [0x00..0xb5), and the L4814 call at 0xb9 is inside the dispatcher
     * body proven above. */
    if (0x1eu + 3u > THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES ||
        0x52u + 3u > THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES ||
        0x55u + 3u > THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES ||
        0xb9u < THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET ||
        0xb9u + 3u > THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET +
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES ||
        THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES +
                THERON_TRACK02_IPL_STAGE2_DELAY_BYTES +
                THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_BYTES +
                THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_BYTES !=
            THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->dispatcher_bytes =
        THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES;
    out_receipt->delay_bytes = THERON_TRACK02_IPL_STAGE2_DELAY_BYTES;
    out_receipt->port_clear_bytes =
        THERON_TRACK02_IPL_STAGE2_PORT_CLEAR_BYTES;
    out_receipt->pointer_setup_bytes =
        THERON_TRACK02_IPL_STAGE2_POINTER_SETUP_BYTES;
    out_receipt->call_graph_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES;
    out_receipt->dispatcher_proven = 1;
    out_receipt->delay_proven = 1;
    out_receipt->port_clear_proven = 1;
    out_receipt->pointer_setup_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_dispatch_machine(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2DispatchMachineReceipt *out_receipt) {
    /* Register-seed tail [0xb5..0xb7): the STZ $FC operand byte and the
     * RTS of the $40ae register-seed subroutine, closing the gap
     * between the executed entry path [0x00..0xb5) and the L40B7
     * dispatcher body. */
    static const uint8_t stage2_seed_tail[] = {0xfcu, 0x60u};
    /* Dispatch stubs [0xf1..0x10d): seven shared return tails that
     * command handlers jump to for selecting the stream-advance count
     * (1, 2, 3, 4, 5, 7, 9); each is an LDA #imm / BRA L40E4 pair. */
    static const uint8_t stage2_dispatch_stubs[] = {
        0xa9u, 0x01u, 0x80u, 0xefu, 0xa9u, 0x02u, 0x80u, 0xebu,
        0xa9u, 0x03u, 0x80u, 0xe7u, 0xa9u, 0x04u, 0x80u, 0xe3u,
        0xa9u, 0x05u, 0x80u, 0xdfu, 0xa9u, 0x07u, 0x80u, 0xdbu,
        0xa9u, 0x09u, 0x80u, 0xd7u
    };
    /* Jump table [0x10d..0x121): ten little-endian handler addresses
     * consumed by the dispatcher's JMP (L410D,X); every entry points
     * inside the loaded image ($41C5..$4253, strictly increasing).  No
     * handler semantics are claimed for the targets. */
    static const uint8_t stage2_jump_table[] = {
        0xc5u, 0x41u, 0xcbu, 0x41u, 0xd8u, 0x41u, 0xdeu, 0x41u,
        0xe6u, 0x41u, 0xecu, 0x41u, 0xf0u, 0x41u, 0xf4u, 0x41u,
        0x14u, 0x42u, 0x53u, 0x42u
    };
    /* L4AF7 MPR-page body [0xaf7..0xb00): CLC, the $FFF5 BIT-flag
     * read, ADC #$01, TAM #$08, RTS — the same $FFF5-derived MPR page
     * map idiom the bound entry prologue uses. */
    static const uint8_t stage2_mpr_page[] = {
        0x18u, 0xadu, 0xf5u, 0xffu, 0x69u, 0x01u, 0x53u, 0x08u,
        0x60u
    };
    /* L4F5E selector body [0xf5e..0xf66): loads the $4EC1 argument
     * address in X/Y and calls L3114. */
    static const uint8_t stage2_selector[] = {
        0xa2u, 0xc1u, 0xa0u, 0x4eu, 0x20u, 0x14u, 0x31u, 0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;
    size_t i;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The dispatch-machine byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_SEED_TAIL_USER_OFFSET,
            stage2_seed_tail, sizeof(stage2_seed_tail)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_USER_OFFSET,
            stage2_dispatch_stubs, sizeof(stage2_dispatch_stubs)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_USER_OFFSET,
            stage2_jump_table, sizeof(stage2_jump_table)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_MPR_PAGE_USER_OFFSET,
            stage2_mpr_page, sizeof(stage2_mpr_page)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_SELECTOR_USER_OFFSET,
            stage2_selector, sizeof(stage2_selector))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Each little-endian jump-table entry must point inside the loaded
     * stage-two image. */
    for (i = 0u; i < THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES; ++i) {
        uint8_t lo;
        uint8_t hi;
        uint32_t target;
        if (!tqr_ipl_user_byte(
                track02_data, track02_size, stage2_sector,
                THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
                THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_USER_OFFSET + 2u * i,
                &lo) ||
            !tqr_ipl_user_byte(
                track02_data, track02_size, stage2_sector,
                THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
                THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_USER_OFFSET + 2u * i + 1u,
                &hi)) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
        target = (uint32_t)lo | ((uint32_t)hi << 8);
        if (target < THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS ||
            target >= THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS +
                THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                    TQR_RAW_SECTOR_USER_DATA_BYTES) {
            return THERON_TRACK02_SIGNAL_NOT_FOUND;
        }
    }
    /* Contiguity and call-site assertions: the seed tail starts where
     * the executed entry path ends; the dispatcher, stubs, and jump
     * table chain without gaps; the L4F5E (0xc1), L4AF7 (0xcc), and
     * JMP (L410D,X) (0xeb) sites sit inside the bound dispatcher
     * window. */
    if (THERON_TRACK02_IPL_STAGE2_SEED_TAIL_USER_OFFSET !=
            THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES ||
        THERON_TRACK02_IPL_STAGE2_SEED_TAIL_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_SEED_TAIL_BYTES !=
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES !=
            THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_BYTES !=
            THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_BYTES !=
            THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES ||
        0xc1u + 3u > THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET +
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES ||
        0xccu + 3u > THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET +
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES ||
        0xebu + 3u > THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET +
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES ||
        THERON_TRACK02_IPL_STAGE2_SEED_TAIL_BYTES +
                THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_BYTES +
                THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_BYTES +
                THERON_TRACK02_IPL_STAGE2_MPR_PAGE_BYTES +
                THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES !=
            THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->seed_tail_bytes = THERON_TRACK02_IPL_STAGE2_SEED_TAIL_BYTES;
    out_receipt->dispatch_stubs_bytes =
        THERON_TRACK02_IPL_STAGE2_DISPATCH_STUBS_BYTES;
    out_receipt->jump_table_bytes = THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_BYTES;
    out_receipt->jump_table_entries =
        THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES;
    out_receipt->mpr_page_bytes = THERON_TRACK02_IPL_STAGE2_MPR_PAGE_BYTES;
    out_receipt->selector_bytes = THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES;
    out_receipt->loop_closure_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES;
    out_receipt->dispatch_machine_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES;
    out_receipt->seed_tail_proven = 1;
    out_receipt->dispatch_stubs_proven = 1;
    out_receipt->jump_table_proven = 1;
    out_receipt->mpr_page_proven = 1;
    out_receipt->selector_proven = 1;
    out_receipt->dispatch_machine_contiguous_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l8000_pair(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L8000PairReceipt *out_receipt) {
    /* L8000 body [0x4000..0x40bc), head of image sector 8: the entry
     * path's first call (JSR $8000 at user offset 0x11).  VDC register
     * clears (STZ $220C/$220D/$2210/$2211, st0/st1/st2 pairs), the
     * L45A6 call at +0x1c, the zero-page result handoff through
     * $4C/$4D, the $47BF-$47D2/$3B6A-$3B6F stores, and the L4696/L48FC
     * tail calls.  Three da65 decode-artifact spans (the $2211 STZ at
     * +0x0b split into .byte/ora, the ADC $00 at +0x2a split into
     * .byte/brk, the STA $47CE/LDA #$00 at +0x4a split into
     * .byte/dec/brk) are bound to the authenticated media bytes; the
     * source-locked disassembly renders several zero-page accesses as
     * absolute labels, so the media bytes are authoritative. */
    static const uint8_t stage2_l8000[] = {
        0xc6u, 0x5au, 0x9cu, 0x0cu, 0x22u, 0x9cu, 0x0du, 0x22u,
        0x9cu, 0x10u, 0x22u, 0x9cu, 0x11u, 0x22u, 0x03u, 0x08u,
        0x13u, 0x00u, 0x23u, 0x00u, 0x03u, 0x07u, 0x13u, 0x00u,
        0x23u, 0x00u, 0x64u, 0x5au, 0x20u, 0xa6u, 0x45u, 0xa5u,
        0x00u, 0x85u, 0x4cu, 0xa5u, 0x01u, 0x85u, 0x4du, 0xa9u,
        0x08u, 0x18u, 0x65u, 0x00u, 0x85u, 0x00u, 0x90u, 0x02u,
        0xe6u, 0x01u, 0xa5u, 0x00u, 0x8du, 0xcbu, 0x47u, 0xa5u,
        0x01u, 0x8du, 0xccu, 0x47u, 0xa5u, 0x01u, 0x18u, 0x69u,
        0x10u, 0x85u, 0x01u, 0xa5u, 0x00u, 0x8du, 0xcdu, 0x47u,
        0xa5u, 0x01u, 0x8du, 0xceu, 0x47u, 0xa9u, 0x00u, 0x8du,
        0xc7u, 0x47u, 0xa9u, 0x01u, 0x8du, 0xc8u, 0x47u, 0xa0u,
        0x04u, 0xb1u, 0x4cu, 0x8du, 0xbfu, 0x47u, 0x85u, 0x0eu,
        0x64u, 0x0fu, 0xc8u, 0xb1u, 0x4cu, 0x8du, 0xbeu, 0x47u,
        0x85u, 0x10u, 0x20u, 0x96u, 0x46u, 0xa5u, 0x0eu, 0x8du,
        0xc9u, 0x47u, 0xa5u, 0x0fu, 0x8du, 0xcau, 0x47u, 0xa0u,
        0x02u, 0xb1u, 0x4cu, 0x85u, 0x00u, 0xc8u, 0xb1u, 0x4cu,
        0x85u, 0x01u, 0xa5u, 0x4cu, 0x18u, 0x65u, 0x00u, 0x85u,
        0x00u, 0xa5u, 0x4du, 0x65u, 0x01u, 0x85u, 0x01u, 0xa5u,
        0x00u, 0x8du, 0x6au, 0x3bu, 0xa5u, 0x01u, 0x8du, 0x6bu,
        0x3bu, 0xa5u, 0x00u, 0x8du, 0xd1u, 0x47u, 0xa5u, 0x01u,
        0x8du, 0xd2u, 0x47u, 0x64u, 0x02u, 0x64u, 0x03u, 0xa0u,
        0x06u, 0xb1u, 0x4cu, 0x8du, 0x6fu, 0x3bu, 0x0au, 0x0au,
        0x0au, 0x0au, 0xaau, 0xadu, 0x68u, 0x3bu, 0xd0u, 0x03u,
        0x20u, 0xfcu, 0x48u, 0x60u
    };
    /* L45A6 body [0x5a6..0x5ca): the ($1C),y table read, the
     * $44E7-$44EA seed copy into $01-$05, and the PLA/LSR branch into
     * the dynamic-lane JSR $3AB7 or the JMP $4105 return.  Cleanly
     * decodable; its only call site is the JSR at L8000+0x1c. */
    static const uint8_t stage2_l45a6[] = {
        0xb1u, 0x1cu, 0x85u, 0x01u, 0xadu, 0xe7u, 0x44u, 0x85u,
        0x02u, 0xadu, 0xe8u, 0x44u, 0x85u, 0x03u, 0xadu, 0xe9u,
        0x44u, 0x85u, 0x04u, 0xadu, 0xeau, 0x44u, 0x85u, 0x05u,
        0x68u, 0x4au, 0xb0u, 0x05u, 0xa9u, 0x17u, 0x20u, 0xb7u,
        0x3au, 0x4cu, 0x05u, 0x41u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The callee-pair byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L8000_USER_OFFSET,
            stage2_l8000, sizeof(stage2_l8000)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L45A6_USER_OFFSET,
            stage2_l45a6, sizeof(stage2_l45a6))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Call-site and span assertions: the L8000 call site (0x11) sits
     * inside the bound entry path [0x00..0xb5); the L45A6 (+0x1c),
     * L4696 (+0x6a), and L48FC (+0xb8) call sites sit inside the bound
     * L8000 window; both windows stay inside the loaded image; the
     * pair byte total chains. */
    if (THERON_TRACK02_IPL_STAGE2_L8000_CALL_SITE_USER_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L45A6_CALL_SITE_L8000_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_L8000_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_L8000_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_L8000_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L48FC_CALL_SITE_L8000_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_L8000_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L8000_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L8000_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L45A6_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L45A6_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L8000_BYTES +
                THERON_TRACK02_IPL_STAGE2_L45A6_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l8000_bytes = THERON_TRACK02_IPL_STAGE2_L8000_BYTES;
    out_receipt->l45a6_bytes = THERON_TRACK02_IPL_STAGE2_L45A6_BYTES;
    out_receipt->pair_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES;
    out_receipt->l8000_proven = 1;
    out_receipt->l45a6_proven = 1;
    out_receipt->l8000_call_site_proven = 1;
    out_receipt->l45a6_single_caller_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_jump_table_handlers(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2JumpTableHandlersReceipt *out_receipt) {
    /* Handler bodies [0x1c5..0x254), one contiguous span holding the ten
     * L410D jump-table targets $41C5..$4253: handler 1 (BSR L41B9 / CLA
     * / JMP L40E4), handler 2 (BSR L41F8 / BNE L41D5 / BSR L41B9 / CLA
     * / JMP L40E4 with the L41D5 JMP L4101 tail), handler 3 (BSR L41F8
     * / BNE L41CF / BRA L41D5), handler 4 (BSR L41F8 / BCC L41D5 / BEQ
     * L41D5 / BRA L41CF), handler 5 (BSR L41F8 / BCS L41D5 / BRA L41CF),
     * handler 6 (BSR L4203 / BRA L41CD), handler 7 (BSR L4203 /
     * BRA L41DA), handler 8 (BSR L4203 / BRA L41E8 plus the shared
     * L41F8/L4203 operand-read sub bodies), handler 9 (the L4215 operand
     * read, the L421C $4EC1/$4D7B store sub with the ADC $3008/STA
     * $3009 pair, the L4F5E selector call, the L4233 carry path, and
     * the L4240/L424B sub ending in the dynamic-lane JSR $383E), and
     * handler 10 (a single RTS).  Three da65 decode-artifact spans of
     * the same class as the L8000 body (the BSR L41F8 at +0x13 split
     * into .byte/.byte, the LDA $2780,x at +0x37 split into .byte/bra,
     * and the ADC $3008/STA $3009 at +0x60 split into .byte/php/bmi/
     * ora) are bound to the authenticated media bytes; the disassembly
     * also renders the +0x80 zero-page STA $20 as the absolute label
     * L0020, so the media bytes are authoritative. */
    static const uint8_t stage2_handlers[] = {
        0x44u, 0xf2u, 0x62u, 0x4cu, 0xe4u, 0x40u, 0x44u, 0x2bu,
        0xd0u, 0x06u, 0x44u, 0xe8u, 0x62u, 0x4cu, 0xe4u, 0x40u,
        0x4cu, 0x01u, 0x41u, 0x44u, 0x1eu, 0xd0u, 0xf3u, 0x80u,
        0xf7u, 0x44u, 0x18u, 0x90u, 0xf3u, 0xf0u, 0xf1u, 0x80u,
        0xe9u, 0x44u, 0x10u, 0xb0u, 0xebu, 0x80u, 0xe3u, 0x44u,
        0x15u, 0x80u, 0xddu, 0x44u, 0x11u, 0x80u, 0xe6u, 0x44u,
        0x0du, 0x80u, 0xf0u, 0xc8u, 0xb1u, 0x1cu, 0xaau, 0xbdu,
        0x80u, 0x27u, 0xc8u, 0xd1u, 0x1cu, 0x60u, 0xc8u, 0xb1u,
        0x1cu, 0xaau, 0xbdu, 0x80u, 0x27u, 0x48u, 0xc8u, 0xb1u,
        0x1cu, 0xaau, 0x68u, 0xddu, 0x80u, 0x27u, 0x60u, 0xc8u,
        0xb1u, 0x1cu, 0x44u, 0x03u, 0x4cu, 0xf5u, 0x40u, 0x8du,
        0xc1u, 0x4eu, 0x8du, 0x7bu, 0x4du, 0x0au, 0x0au, 0x18u,
        0x6du, 0x08u, 0x30u, 0x8du, 0x09u, 0x30u, 0xa9u, 0x02u,
        0x20u, 0x5eu, 0x4fu, 0xb0u, 0x01u, 0x60u, 0x00u, 0xc6u,
        0x5bu, 0x20u, 0xd6u, 0x43u, 0x44u, 0x05u, 0x64u, 0x5bu,
        0x4cu, 0xf5u, 0x40u, 0x20u, 0xd8u, 0x37u, 0xa9u, 0x00u,
        0x85u, 0x20u, 0xa9u, 0x68u, 0x85u, 0x21u, 0xa9u, 0x03u,
        0x85u, 0x1eu, 0x20u, 0x3eu, 0x38u, 0x60u, 0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The handler-body byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_HANDLERS_USER_OFFSET,
            stage2_handlers, sizeof(stage2_handlers))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Span and entry-chain assertions: the span stays inside the loaded
     * image; the handler count matches the bound jump table; the first
     * table target sits at the span head and the last target's single
     * byte closes the span (strictly increasing targets were already
     * range-checked against the loaded image by the round-13 table
     * binding); the JMP (L410D,x) table-read site sits inside the bound
     * dispatcher window; the static span byte count chains. */
    if (THERON_TRACK02_IPL_STAGE2_HANDLERS_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT !=
            THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES ||
        THERON_TRACK02_IPL_STAGE2_HANDLERS_FIRST_CPU_ADDRESS -
                THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_HANDLERS_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_HANDLERS_LAST_CPU_ADDRESS -
                THERON_TRACK02_IPL_STAGE2_LOAD_ADDRESS + 1u !=
            THERON_TRACK02_IPL_STAGE2_HANDLERS_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES ||
        THERON_TRACK02_IPL_STAGE2_HANDLER_TABLE_READ_USER_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_DISPATCHER_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_DISPATCHER_BYTES ||
        sizeof(stage2_handlers) !=
            THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->handlers_bytes = THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES;
    out_receipt->handler_count = THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT;
    out_receipt->first_handler_cpu_address =
        THERON_TRACK02_IPL_STAGE2_HANDLERS_FIRST_CPU_ADDRESS;
    out_receipt->last_handler_cpu_address =
        THERON_TRACK02_IPL_STAGE2_HANDLERS_LAST_CPU_ADDRESS;
    out_receipt->handlers_proven = 1;
    out_receipt->handler_entry_chain_proven = 1;
    out_receipt->handlers_contiguous_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l4696_l3114(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L4696L3114Receipt *out_receipt) {
    /* L4696 body [0x4696..0x46db): the 16-bit shift-add multiply called
     * from the bound L8000 window (JSR $4696 at L8000+0x6a).  da65's
     * linear $4000-based map labels image offset 0x696 as L4696, whose
     * head byte $33 is no HuC6280 opcode (the flagged head-byte decode
     * artifact, emitted as `.byte $33`); the authenticated body at
     * image offset 0x4696 matches da65's own L8696 decode
     * (theron-us-stage2-huc6280.asm:10212-10248) instruction by
     * instruction.  The disassembly renders the $11 zero-page accesses
     * as the absolute label L0011, so the media bytes are
     * authoritative. */
    static const uint8_t stage2_l4696[] = {
        0x64u, 0x0fu, 0x64u, 0x11u, 0xa5u, 0x0eu, 0x85u, 0x12u,
        0x64u, 0x0eu, 0xa2u, 0x01u, 0xffu, 0x12u, 0x16u, 0xefu,
        0x12u, 0x14u, 0xdfu, 0x12u, 0x12u, 0xcfu, 0x12u, 0x10u,
        0xbfu, 0x12u, 0x0eu, 0xafu, 0x12u, 0x0cu, 0x9fu, 0x12u,
        0x0au, 0x8fu, 0x12u, 0x08u, 0x60u, 0xe8u, 0xe8u, 0xe8u,
        0xe8u, 0xe8u, 0xe8u, 0xe8u, 0x46u, 0x12u, 0x90u, 0x0du,
        0x18u, 0xa5u, 0x10u, 0x65u, 0x0eu, 0x85u, 0x0eu, 0xa5u,
        0x11u, 0x65u, 0x0fu, 0x85u, 0x0fu, 0x06u, 0x10u, 0x26u,
        0x11u, 0xcau, 0xd0u, 0xe8u, 0x60u
    };
    /* L3114 body [0x1114..0x1172): called from the bound L4F5E
     * selector window (JSR $3114 at L4F5E+4, selector bytes LDX #$C1 /
     * LDY #$4E / JSR / RTS).  da65 declared L3114 := $3114 absolute
     * without a body decode (CPU $3114 lies below its linear $4000
     * map; the CPU $3xxx window shows image bank 0 at offset
     * CPU-$2000).  The trailing RTS at 0x1171 sits immediately before
     * the da65-declared L3172 entry, confirming the span; the BSR/JSR
     * callees (L3172, $117D, $4F66, $526D, $55E0, $5213) remain
     * unbound future windows. */
    static const uint8_t stage2_l3114[] = {
        0x48u, 0x44u, 0x5bu, 0xadu, 0xdeu, 0x4fu, 0xd0u, 0x04u,
        0x44u, 0x5fu, 0x80u, 0x08u, 0xa9u, 0x1au, 0x20u, 0x66u,
        0x4fu, 0x3au, 0xd0u, 0xfau, 0x68u, 0x8du, 0x8eu, 0x4fu,
        0x68u, 0x8du, 0x8du, 0x4fu, 0xadu, 0x9du, 0x4fu, 0x85u,
        0x06u, 0xadu, 0x9eu, 0x4fu, 0x85u, 0x07u, 0xadu, 0x93u,
        0x4fu, 0x8du, 0x8bu, 0x4fu, 0xadu, 0x94u, 0x4fu, 0x8du,
        0x8cu, 0x4fu, 0x20u, 0x6du, 0x52u, 0x18u, 0xa5u, 0x06u,
        0x69u, 0x04u, 0x85u, 0x06u, 0x90u, 0x02u, 0xe6u, 0x07u,
        0xaeu, 0x8du, 0x4fu, 0xacu, 0x8eu, 0x4fu, 0xdau, 0xa5u,
        0x0eu, 0x48u, 0xa5u, 0x0fu, 0x48u, 0x20u, 0xe0u, 0x55u,
        0x68u, 0x85u, 0x0fu, 0x68u, 0x85u, 0x0eu, 0x20u, 0x13u,
        0x52u, 0xfau, 0x88u, 0xd0u, 0xe9u, 0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The far-callee byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L4696_USER_OFFSET,
            stage2_l4696, sizeof(stage2_l4696)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET,
            stage2_l3114, sizeof(stage2_l3114))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Call-site and span assertions: the L4696 call site (L8000+0x6a)
     * sits inside the bound L8000 window; the L3114 call site
     * (L4F5E+4) sits inside the bound selector window; both windows
     * stay inside the loaded image; the static byte counts chain. */
    if (THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_L8000_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_L8000_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_SELECTOR_OFFSET + 3u >
            THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L4696_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L4696_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        sizeof(stage2_l4696) != THERON_TRACK02_IPL_STAGE2_L4696_BYTES ||
        sizeof(stage2_l3114) != THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L4696_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l4696_bytes = THERON_TRACK02_IPL_STAGE2_L4696_BYTES;
    out_receipt->l3114_bytes = THERON_TRACK02_IPL_STAGE2_L3114_BYTES;
    out_receipt->l4696_l3114_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES;
    out_receipt->l4696_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS;
    out_receipt->l3114_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_CPU_ADDRESS;
    out_receipt->l4696_proven = 1;
    out_receipt->l3114_proven = 1;
    out_receipt->l4696_call_site_proven = 1;
    out_receipt->l3114_call_site_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114CalleesReceipt *out_receipt) {
    /* L3172 body [0x1172..0x117d): the BSR target at L3114+0x01, sitting
     * directly after the bound L3114 body in the low-image region
     * (below $3800, never clobbered by the $3800 dynamic-payload
     * CD_READ).  da65 declared L3172 := $3172 absolute without a body
     * decode; the media decodes LDA #$01 / STA $5C22 / LDA #$01 /
     * STA $5C23 / RTS. */
    static const uint8_t stage2_l3172[] = {
        0xa9u, 0x01u, 0x8du, 0x22u, 0x5cu, 0xa9u, 0x01u, 0x8du,
        0x23u, 0x5cu, 0x60u
    };
    /* $117D far-helper trampoline [0x117d..0x118a): the BSR target at
     * L3114+0x08.  The media decodes JSR $5BF5 / JSR $5C8C / JSR $5CB0 /
     * JSR $5C25 / RTS; its own callees remain unbound future windows. */
    static const uint8_t stage2_far117d[] = {
        0x20u, 0xf5u, 0x5bu, 0x20u, 0x8cu, 0x5cu, 0x20u, 0xb0u,
        0x5cu, 0x20u, 0x25u, 0x5cu, 0x60u
    };
    /* L4F66 delay loop [0x0f66..0x0f7a): the JSR $4F66 target at
     * L3114+0x0e.  da65 lists the body inline under its linear map
     * (CPU = image + $4000, image bank 0), directly after the bound
     * L4F5E selector window: PHA / PHX / PHY / LDA #$03 / CLX / CLY /
     * the DEY/DEX/DEC A BNE nest / PLY / PLX / PLA / RTS (matching the
     * L3114 LDA #$1A loop); the next da65 label L4F7A confirms the
     * span. */
    static const uint8_t stage2_l4f66[] = {
        0x48u, 0xdau, 0x5au, 0xa9u, 0x03u, 0x82u, 0xc2u, 0x88u,
        0xd0u, 0xfdu, 0xcau, 0xd0u, 0xf9u, 0x3au, 0xd0u, 0xf5u,
        0x7au, 0xfau, 0x68u, 0x60u
    };
    /* L5213 body [0x1213..0x121f): the JSR $5213 target at L3114+0x56,
     * decoded inline by da65 under its linear map: CLC / LDA $0E /
     * ADC #$40 / STA $0E / BCC / INC $0F / RTS. */
    static const uint8_t stage2_l5213[] = {
        0x18u, 0xa5u, 0x0eu, 0x69u, 0x40u, 0x85u, 0x0eu, 0x90u,
        0x02u, 0xe6u, 0x0fu, 0x60u
    };
    /* L526D body [0x126d..0x1280): the JSR $526D target at L3114+0x32,
     * decoded inline by da65: JSR L51F9 / LDX #$04 / the LSR $07 /
     * ROR $06 / DEX / BNE shift loop / LDA $07 / ORA #$F0 / STA $07 /
     * RTS (its L51F9 callee remains an unbound future window); the next
     * da65 label L5280 confirms the span. */
    static const uint8_t stage2_l526d[] = {
        0x20u, 0xf9u, 0x51u, 0xa2u, 0x04u, 0x46u, 0x07u, 0x66u,
        0x06u, 0xcau, 0xd0u, 0xf9u, 0xa5u, 0x07u, 0x09u, 0xf0u,
        0x85u, 0x07u, 0x60u
    };
    /* L55E0 body [0x15e0..0x15e8): the JSR $55E0 target at L3114+0x4d,
     * decoded inline by da65: BSR L55F6 / BSR L55E8 / DEX / BNE L55E0 /
     * RTS (its L55F6/L55E8 callees remain unbound future windows). */
    static const uint8_t stage2_l55e0[] = {
        0x44u, 0x14u, 0x44u, 0x04u, 0xcau, 0xd0u, 0xf9u, 0x60u
    };
    /* L3114 call-site signatures at their compile-time-asserted offsets
     * inside the bound L3114 body: BSR L3172, BSR $117D, JSR $4F66,
     * JSR $526D, JSR $55E0, JSR $5213.  The far targets are encoded
     * from the pinned callee CPU addresses. */
    static const uint8_t stage2_l3114_call_l3172[] = { 0x44u, 0x5bu };
    static const uint8_t stage2_l3114_call_far117d[] = { 0x44u, 0x5fu };
    static const uint8_t stage2_l3114_call_l4f66[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l3114_call_l526d[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l3114_call_l55e0[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l3114_call_l5213[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_CPU_ADDRESS >>
                  8)
    };
    /* $45xx-tier L4696 call site: the 3-byte JSR window bound at image
     * offsets 0x45ba and 0x45cb, its target encoded from the pinned
     * L4696 CPU address. */
    static const uint8_t stage2_l4696_call_site[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS & 0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS >> 8)
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The callee byte identity is attested only for the authenticated
     * US stage-two body; the JP body rejects here until staged JP media
     * can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_USER_OFFSET,
            stage2_l3172, sizeof(stage2_l3172)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET,
            stage2_far117d, sizeof(stage2_far117d)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_USER_OFFSET,
            stage2_l4f66, sizeof(stage2_l4f66)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_USER_OFFSET,
            stage2_l5213, sizeof(stage2_l5213)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_USER_OFFSET,
            stage2_l526d, sizeof(stage2_l526d)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET,
            stage2_l55e0, sizeof(stage2_l55e0)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L3172_OFF,
            stage2_l3114_call_l3172, sizeof(stage2_l3114_call_l3172)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_FAR117D_OFF,
            stage2_l3114_call_far117d,
            sizeof(stage2_l3114_call_far117d)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L4F66_OFF,
            stage2_l3114_call_l4f66, sizeof(stage2_l3114_call_l4f66)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L526D_OFF,
            stage2_l3114_call_l526d, sizeof(stage2_l3114_call_l526d)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L55E0_OFF,
            stage2_l3114_call_l55e0, sizeof(stage2_l3114_call_l55e0)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L5213_OFF,
            stage2_l3114_call_l5213, sizeof(stage2_l3114_call_l5213)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_A_USER_OFFSET,
            stage2_l4696_call_site, sizeof(stage2_l4696_call_site)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_B_USER_OFFSET,
            stage2_l4696_call_site, sizeof(stage2_l4696_call_site))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Call-site and span assertions: every L3114 call site sits inside
     * the bound L3114 body; every callee window stays inside the loaded
     * image; L4F66 starts exactly where the bound selector window ends;
     * the four far callees keep da65's linear CPU = image + $4000 form;
     * the $45xx call-site target is the bound L4696 body; the static
     * byte counts chain. */
    if (THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L3172_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_FAR117D_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L4F66_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L526D_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L55E0_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALL_SITE_L5213_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_B_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_USER_OFFSET !=
            THERON_TRACK02_IPL_STAGE2_SELECTOR_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_SELECTOR_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L4696_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L4696_USER_OFFSET ||
        sizeof(stage2_l3172) !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_BYTES ||
        sizeof(stage2_far117d) !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES ||
        sizeof(stage2_l4f66) !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES ||
        sizeof(stage2_l5213) !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_BYTES ||
        sizeof(stage2_l526d) !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES ||
        sizeof(stage2_l55e0) !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES ||
        sizeof(stage2_l4696_call_site) !=
            THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES +
                2u * THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l3172_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L3172_BYTES;
    out_receipt->far117d_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES;
    out_receipt->l4f66_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES;
    out_receipt->l5213_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_BYTES;
    out_receipt->l526d_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES;
    out_receipt->l55e0_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES;
    out_receipt->l3114_callees_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES;
    out_receipt->l4f66_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_CPU_ADDRESS;
    out_receipt->l5213_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_CPU_ADDRESS;
    out_receipt->l526d_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_CPU_ADDRESS;
    out_receipt->l55e0_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_CPU_ADDRESS;
    out_receipt->l3172_proven = 1;
    out_receipt->far117d_proven = 1;
    out_receipt->l4f66_proven = 1;
    out_receipt->l5213_proven = 1;
    out_receipt->l526d_proven = 1;
    out_receipt->l55e0_proven = 1;
    out_receipt->l3114_call_sites_proven = 1;
    out_receipt->l4696_call_sites_45xx_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier2_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier2CalleesReceipt *out_receipt) {
    /* L51F9 body [0x11f9..0x1213): the JSR L51F9 target at the head of
     * the bound L526D body.  da65 lists it inline under its linear map
     * but carries a mid-instruction label artifact: the declared L5200
     * label splits the `ror $0E` at 0x11ff-0x1200 (`.byte $66` plus
     * the garbage `asl $664A` / `asl $0F85` renderings), so the media
     * bytes are authoritative: STZ $0E / LDA $4F8C / LSR A / ROR $0E /
     * LSR A / ROR $0E / STA $0F / CLC / LDA $0E / ADC $4F8B / STA $0E /
     * BCC / INC $0F / RTS.  The trailing RTS sits immediately before
     * the bound L5213 entry, confirming the span. */
    static const uint8_t stage2_l51f9[] = {
        0x64u, 0x0eu, 0xadu, 0x8cu, 0x4fu, 0x4au, 0x66u, 0x0eu,
        0x4au, 0x66u, 0x0eu, 0x85u, 0x0fu, 0x18u, 0xa5u, 0x0eu,
        0x6du, 0x8bu, 0x4fu, 0x85u, 0x0eu, 0x90u, 0x02u, 0xe6u,
        0x0fu, 0x60u
    };
    /* L55E8 body [0x15e8..0x15ef): the BSR L55E8 target at L55E0+0x02,
     * directly after the bound L55E0 body, decoded inline by da65:
     * INC $0E / BNE / INC $0F / RTS (the next da65 label L55EF
     * confirms the span). */
    static const uint8_t stage2_l55e8[] = {
        0xe6u, 0x0eu, 0xd0u, 0x02u, 0xe6u, 0x0fu, 0x60u
    };
    /* L55F6 body [0x15f6..0x1600): the BSR L55F6 target at L55E0+0x00,
     * decoded inline by da65: DEC $5A / JSR L54A0 / BSR L5600 /
     * STZ $5A / RTS (its L54A0/L5600 callees remain unbound future
     * windows; the next da65 label L5600 confirms the span). */
    static const uint8_t stage2_l55f6[] = {
        0xc6u, 0x5au, 0x20u, 0xa0u, 0x54u, 0x44u, 0x03u, 0x64u,
        0x5au, 0x60u
    };
    /* L5BF5 body [0x1bf5..0x1c06): the JSR $5BF5 target at $117D+0x00,
     * decoded inline by da65: LDY #$04 / CLX / the LDA $5C20,x /
     * STA $4F8B,x / INX / DEY / BNE copy loop / STZ $4FD1 / RTS (the
     * next da65 label L5C06 confirms the span; the L5C20 table stays
     * unbound data). */
    static const uint8_t stage2_l5bf5[] = {
        0xa0u, 0x04u, 0x82u, 0xbdu, 0x20u, 0x5cu, 0x9du, 0x8bu,
        0x4fu, 0xe8u, 0x88u, 0xd0u, 0xf6u, 0x9cu, 0xd1u, 0x4fu,
        0x60u
    };
    /* L5C25 body [0x1c25..0x1c69): the JSR $5C25 target at $117D+0x09,
     * decoded inline by da65: LDA #$F0 / STA $5C24 / BRA L5C31 (the
     * L5C2C alternate entry with LDA #$EF stays an unbound window) /
     * JSR L536E / the $4FB8/$4FB9,x indexed $04:$05 setup / LDY $4F8E /
     * LDX $4F8D / the PHY/PHX/BSR L5C69/PLX/PLY/DEY/BNE loop / the
     * $4FD4 save/JSR L5439/restore / RTS (its L536E/L5C69/L5439
     * callees remain unbound future windows; the next da65 label
     * L5C69 confirms the span). */
    static const uint8_t stage2_l5c25[] = {
        0xa9u, 0xf0u, 0x8du, 0x24u, 0x5cu, 0x80u, 0x05u, 0xa9u,
        0xefu, 0x8du, 0x24u, 0x5cu, 0x20u, 0x6eu, 0x53u, 0xadu,
        0xb8u, 0x4fu, 0x3au, 0x0au, 0xaau, 0xbdu, 0xb9u, 0x4fu,
        0x18u, 0x69u, 0x06u, 0x85u, 0x04u, 0xe8u, 0x62u, 0x7du,
        0xb9u, 0x4fu, 0x85u, 0x05u, 0xacu, 0x8eu, 0x4fu, 0xaeu,
        0x8du, 0x4fu, 0x5au, 0xdau, 0x44u, 0x16u, 0xfau, 0x7au,
        0x88u, 0xd0u, 0xf7u, 0xadu, 0xd4u, 0x4fu, 0x48u, 0xa9u,
        0x01u, 0x8du, 0xd4u, 0x4fu, 0x20u, 0x39u, 0x54u, 0x68u,
        0x8du, 0xd4u, 0x4fu, 0x60u
    };
    /* L5C8C body [0x1c8c..0x1c9f): the JSR $5C8C target at $117D+0x03,
     * decoded inline by da65: JSR L5C06 / BEQ / JSR L5C9F / JSR
     * LE063 / LDA $222D / BEQ L5C8C / STA $08 / RTS (its L5C06/L5C9F
     * callees and the LE063 far-call target remain unbound future
     * windows; the next da65 label L5C9F confirms the span). */
    static const uint8_t stage2_l5c8c[] = {
        0x20u, 0x06u, 0x5cu, 0xf0u, 0x03u, 0x20u, 0x9fu, 0x5cu,
        0x20u, 0x63u, 0xe0u, 0xadu, 0x2du, 0x22u, 0xf0u, 0xf0u,
        0x85u, 0x08u, 0x60u
    };
    /* L5CB0 body [0x1cb0..0x1cbf): the JSR $5CB0 target at $117D+0x06,
     * decoded inline by da65: PHA / PHX / PHY / the JSR LE063 /
     * LDA $2228 / BNE poll loop / PLY / PLX / PLA / RTS (the LE063
     * far-call target remains an unbound future window; the next
     * da65 label L5CBF confirms the span). */
    static const uint8_t stage2_l5cb0[] = {
        0x48u, 0xdau, 0x5au, 0x20u, 0x63u, 0xe0u, 0xadu, 0x28u,
        0x22u, 0xd0u, 0xf8u, 0x7au, 0xfau, 0x68u, 0x60u
    };
    /* Tier-2 call-site signatures at their compile-time-asserted
     * offsets inside the bound caller bodies; the far targets are
     * encoded from the pinned callee CPU addresses. */
    static const uint8_t stage2_far117d_call_l5bf5[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_far117d_call_l5c8c[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_far117d_call_l5cb0[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_far117d_call_l5c25[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l526d_call_l51f9[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l55e0_call_l55f6[] = { 0x44u, 0x14u };
    static const uint8_t stage2_l55e0_call_l55e8[] = { 0x44u, 0x04u };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The tier-2 callee byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_USER_OFFSET,
            stage2_l51f9, sizeof(stage2_l51f9)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_USER_OFFSET,
            stage2_l55e8, sizeof(stage2_l55e8)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_USER_OFFSET,
            stage2_l55f6, sizeof(stage2_l55f6)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_USER_OFFSET,
            stage2_l5bf5, sizeof(stage2_l5bf5)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET,
            stage2_l5c25, sizeof(stage2_l5c25)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_USER_OFFSET,
            stage2_l5c8c, sizeof(stage2_l5c8c)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_USER_OFFSET,
            stage2_l5cb0, sizeof(stage2_l5cb0)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5BF5_OFF,
            stage2_far117d_call_l5bf5,
            sizeof(stage2_far117d_call_l5bf5)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5C8C_OFF,
            stage2_far117d_call_l5c8c,
            sizeof(stage2_far117d_call_l5c8c)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5CB0_OFF,
            stage2_far117d_call_l5cb0,
            sizeof(stage2_far117d_call_l5cb0)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5C25_OFF,
            stage2_far117d_call_l5c25,
            sizeof(stage2_far117d_call_l5c25)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L526D_CALL_SITE_L51F9_OFF,
            stage2_l526d_call_l51f9, sizeof(stage2_l526d_call_l51f9)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L55E0_CALL_SITE_L55F6_OFF,
            stage2_l55e0_call_l55f6, sizeof(stage2_l55e0_call_l55f6)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L55E0_CALL_SITE_L55E8_OFF,
            stage2_l55e0_call_l55e8, sizeof(stage2_l55e0_call_l55e8))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Call-site and span assertions: every tier-2 call site sits
     * inside its bound caller body; every callee window stays inside
     * the loaded image; all seven callees keep da65's linear CPU =
     * image + $4000 form; the L51F9 body ends exactly at the bound
     * L5213 entry and L55E8 starts exactly where the bound L55E0 body
     * ends; the static byte counts chain. */
    if (THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5BF5_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES ||
        THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5C8C_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES ||
        THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5CB0_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES ||
        THERON_TRACK02_IPL_STAGE2_FAR117D_CALL_SITE_L5C25_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_FAR117D_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L526D_CALL_SITE_L51F9_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L526D_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L55E0_CALL_SITE_L55F6_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L55E0_CALL_SITE_L55E8_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L5213_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_USER_OFFSET !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L55E0_BYTES ||
        sizeof(stage2_l51f9) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES ||
        sizeof(stage2_l55e8) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_BYTES ||
        sizeof(stage2_l55f6) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES ||
        sizeof(stage2_l5bf5) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_BYTES ||
        sizeof(stage2_l5c25) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES ||
        sizeof(stage2_l5c8c) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES ||
        sizeof(stage2_l5cb0) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l51f9_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_BYTES;
    out_receipt->l55e8_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_BYTES;
    out_receipt->l55f6_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES;
    out_receipt->l5bf5_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_BYTES;
    out_receipt->l5c25_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES;
    out_receipt->l5c8c_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES;
    out_receipt->l5cb0_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_BYTES;
    out_receipt->tier2_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES;
    out_receipt->l51f9_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L51F9_CPU_ADDRESS;
    out_receipt->l55e8_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55E8_CPU_ADDRESS;
    out_receipt->l55f6_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_CPU_ADDRESS;
    out_receipt->l5bf5_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_CPU_ADDRESS;
    out_receipt->l5c25_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_CPU_ADDRESS;
    out_receipt->l5c8c_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_CPU_ADDRESS;
    out_receipt->l5cb0_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_CPU_ADDRESS;
    out_receipt->l51f9_proven = 1;
    out_receipt->l55e8_proven = 1;
    out_receipt->l55f6_proven = 1;
    out_receipt->l5bf5_proven = 1;
    out_receipt->l5c25_proven = 1;
    out_receipt->l5c8c_proven = 1;
    out_receipt->l5cb0_proven = 1;
    out_receipt->far117d_call_sites_proven = 1;
    out_receipt->l526d_call_site_proven = 1;
    out_receipt->l55e0_call_sites_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier3_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier3CalleesReceipt *out_receipt) {
    /* L5C06 body [0x1c06..0x1c20): the JSR L5C06 target at the head of
     * the bound L5C8C body, decoded inline by da65: DEC L4FD1 / BEQ /
     * RTS / the LDA L4FD2 / EOR #$01 / STA L4FD2 toggle / BEQ L5C1B /
     * JSR L5C25 / CLA / RTS / L5C1B: JSR L5C2C / CLA / RTS (the next
     * da65 label L5C20 confirms the span). */
    static const uint8_t stage2_l5c06[] = {
        0xceu, 0xd1u, 0x4fu, 0xf0u, 0x01u, 0x60u, 0xadu, 0xd2u,
        0x4fu, 0x49u, 0x01u, 0x8du, 0xd2u, 0x4fu, 0xf0u, 0x05u,
        0x20u, 0x25u, 0x5cu, 0x62u, 0x60u, 0x20u, 0x2cu, 0x5cu,
        0x62u, 0x60u
    };
    /* L5C20 table [0x1c20..0x1c25): five zero bytes as loaded (da65
     * renders them BRK x5) — the data window read by the bound L5BF5
     * copy loop (LDA $5C20,x) and the L5C24 mask byte written by the
     * bound L5C25/L5C2C entries. */
    static const uint8_t stage2_l5c20[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u
    };
    /* L5C69 body [0x1c69..0x1c8c): the BSR L5C69 target at L5C25+0x2c,
     * decoded inline by da65: LDA $5C24 / CMP #$EF / BEQ L5C74 / LDA
     * #$0D / BRA L5C76 / LDA #$2D / STA L5C7E (self-modifying: the
     * store rewrites the ORA/AND opcode byte at 0x1c7e; the media
     * bytes are the as-loaded image) / PHX / CLY / the INY /
     * LDA ($04),y / ORA $5C24 / STA ($04),y / INY / DEX / BNE L5C7B
     * loop / PLA / JSR L5492 / RTS (its L5492 callee remains an
     * unbound tier-4 window; the body ends exactly at the bound L5C8C
     * entry). */
    static const uint8_t stage2_l5c69[] = {
        0xadu, 0x24u, 0x5cu, 0xc9u, 0xefu, 0xf0u, 0x04u, 0xa9u,
        0x0du, 0x80u, 0x02u, 0xa9u, 0x2du, 0x8du, 0x7eu, 0x5cu,
        0xdau, 0xc2u, 0xc8u, 0xb1u, 0x04u, 0x0du, 0x24u, 0x5cu,
        0x91u, 0x04u, 0xc8u, 0xcau, 0xd0u, 0xf4u, 0x68u, 0x20u,
        0x92u, 0x54u, 0x60u
    };
    /* L5C9F body [0x1c9f..0x1cb0): the JSR L5C9F target at L5C8C+0x05,
     * decoded inline by da65: the L4FD4 save / LDA #$07 / STA L4FD4 /
     * JSR L4F7A / restore / RTS (its L4F7A callee remains an unbound
     * tier-4 window; the body ends exactly at the bound L5CB0
     * entry). */
    static const uint8_t stage2_l5c9f[] = {
        0xadu, 0xd4u, 0x4fu, 0x48u, 0xa9u, 0x07u, 0x8du, 0xd4u,
        0x4fu, 0x20u, 0x7au, 0x4fu, 0x68u, 0x8du, 0xd4u, 0x4fu,
        0x60u
    };
    /* L536E body [0x136e..0x13c4): the JSR L536E target at
     * L5C25+0x0c, decoded inline by da65: the L4FB8-indexed L4FB9,x
     * pair store from L4FD5/L4FD6 / INC L4FB8 / PHX / the
     * $0E:$0F x L4F8D multiply-accumulate loop (CLC / LDA $0E / ADC
     * L4F8D / STA $0E / BCC / INC $0F / DEX / BNE) / ASL $0E /
     * ROL $0F / PLX / the L4FD5/L4FD6 add / the $DFF0 bounds compare /
     * the STZ L4FB9,x pair / RTS (its L53C4 continuation remains an
     * unbound tier-4 window). */
    static const uint8_t stage2_l536e[] = {
        0xadu, 0xb8u, 0x4fu, 0x0au, 0xaau, 0xadu, 0xd5u, 0x4fu,
        0x9du, 0xb9u, 0x4fu, 0xe8u, 0xadu, 0xd6u, 0x4fu, 0x9du,
        0xb9u, 0x4fu, 0xeeu, 0xb8u, 0x4fu, 0xdau, 0x64u, 0x0eu,
        0x64u, 0x0fu, 0xaeu, 0x8eu, 0x4fu, 0x18u, 0xa5u, 0x0eu,
        0x6du, 0x8du, 0x4fu, 0x85u, 0x0eu, 0x90u, 0x02u, 0xe6u,
        0x0fu, 0xcau, 0xd0u, 0xf1u, 0x06u, 0x0eu, 0x26u, 0x0fu,
        0xfau, 0x18u, 0xa5u, 0x0eu, 0x6du, 0xd5u, 0x4fu, 0x85u,
        0x0eu, 0xa5u, 0x0fu, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x0fu,
        0xa5u, 0x0fu, 0xc9u, 0xdfu, 0x90u, 0x06u, 0xd0u, 0x04u,
        0xa5u, 0x0eu, 0xc9u, 0xf0u, 0x90u, 0x08u, 0x9eu, 0xb9u,
        0x4fu, 0xcau, 0x9eu, 0xb9u, 0x4fu, 0x60u
    };
    /* L5439 body [0x1439..0x1455): the JSR L5439 target at
     * L5C25+0x3c, decoded inline by da65: DEC L4FB8 / the L4FB8-indexed
     * L4FB9,x pair load into $04:$05 / the null-pair early-out (LDA
     * $04 / BNE L5455 / LDA $05 / BNE L5455 / RTS) — the next da65
     * label L5455 confirms the span; the L5455 continuation remains an
     * unbound tier-4 window. */
    static const uint8_t stage2_l5439[] = {
        0xceu, 0xb8u, 0x4fu, 0xadu, 0xb8u, 0x4fu, 0x0au, 0xaau,
        0xbdu, 0xb9u, 0x4fu, 0x85u, 0x04u, 0xe8u, 0xbdu, 0xb9u,
        0x4fu, 0x85u, 0x05u, 0xa5u, 0x04u, 0xd0u, 0x05u, 0xa5u,
        0x05u, 0xd0u, 0x01u, 0x60u
    };
    /* L54A0 body [0x14a0..0x14af): the JSR L54A0 target at
     * L55F6+0x02, decoded inline by da65: ST0 #$00 / LDA $0E /
     * STA a:$02 / LDA $0F / STA a:$03 / ST0 #$02 / RTS — the media
     * confirms da65's `a:` absolute-store rendering ($8D $02 $00),
     * and the body ends exactly at the next da65 label L54AF (an
     * unbound tier-4 window). */
    static const uint8_t stage2_l54a0[] = {
        0x03u, 0x00u, 0xa5u, 0x0eu, 0x8du, 0x02u, 0x00u, 0xa5u,
        0x0fu, 0x8du, 0x03u, 0x00u, 0x03u, 0x02u, 0x60u
    };
    /* L5600 body [0x1600..0x160b): the BSR L5600 target at
     * L55F6+0x05, decoded inline by da65: LDA $06 / STA a:$02 /
     * LDA $07 / STA a:$03 / RTS — the media confirms the absolute
     * stores, and the body ends exactly at the next da65 label L560B
     * (an unbound tier-4 window). */
    static const uint8_t stage2_l5600[] = {
        0xa5u, 0x06u, 0x8du, 0x02u, 0x00u, 0xa5u, 0x07u, 0x8du,
        0x03u, 0x00u, 0x60u
    };
    /* Tier-3 call-site signatures at their compile-time-asserted
     * offsets inside the bound caller bodies; the far targets are
     * encoded from the pinned callee CPU addresses, the BSR relatives
     * and the L5C2C entry head are media literals. */
    static const uint8_t stage2_l5c8c_call_l5c06[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l5c8c_call_l5c9f[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l5c25_call_l536e[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l5c25_call_l5c69[] = { 0x44u, 0x16u };
    static const uint8_t stage2_l5c25_call_l5439[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l55f6_call_l54a0[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l55f6_call_l5600[] = { 0x44u, 0x03u };
    static const uint8_t stage2_l5bf5_data_l5c20[] = {
        0xbdu,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l5c2c_entry[] = { 0xa9u, 0xefu };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The tier-3 callee byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_USER_OFFSET,
            stage2_l5c06, sizeof(stage2_l5c06)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_USER_OFFSET,
            stage2_l5c20, sizeof(stage2_l5c20)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_USER_OFFSET,
            stage2_l5c69, sizeof(stage2_l5c69)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_USER_OFFSET,
            stage2_l5c9f, sizeof(stage2_l5c9f)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_USER_OFFSET,
            stage2_l536e, sizeof(stage2_l536e)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_USER_OFFSET,
            stage2_l5439, sizeof(stage2_l5439)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_USER_OFFSET,
            stage2_l54a0, sizeof(stage2_l54a0)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_USER_OFFSET,
            stage2_l5600, sizeof(stage2_l5600)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C8C_CALL_SITE_L5C06_OFF,
            stage2_l5c8c_call_l5c06, sizeof(stage2_l5c8c_call_l5c06)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C8C_CALL_SITE_L5C9F_OFF,
            stage2_l5c8c_call_l5c9f, sizeof(stage2_l5c8c_call_l5c9f)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L536E_OFF,
            stage2_l5c25_call_l536e, sizeof(stage2_l5c25_call_l536e)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L5C69_OFF,
            stage2_l5c25_call_l5c69, sizeof(stage2_l5c25_call_l5c69)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L5439_OFF,
            stage2_l5c25_call_l5439, sizeof(stage2_l5c25_call_l5439)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L55F6_CALL_SITE_L54A0_OFF,
            stage2_l55f6_call_l54a0, sizeof(stage2_l55f6_call_l54a0)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L55F6_CALL_SITE_L5600_OFF,
            stage2_l55f6_call_l5600, sizeof(stage2_l55f6_call_l5600)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5BF5_DATA_SITE_L5C20_OFF,
            stage2_l5bf5_data_l5c20, sizeof(stage2_l5bf5_data_l5c20)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C25_L5C2C_ENTRY_OFF,
            stage2_l5c2c_entry, sizeof(stage2_l5c2c_entry))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Call-site and span assertions: every tier-3 call site sits
     * inside its bound caller body; every callee window stays inside
     * the loaded image; all eight windows keep da65's linear CPU =
     * image + $4000 form; the L5C2C entry offset stays inside the
     * bound L5C25 window; the L5C06/L5C20/L5C25, L5C69/L5C8C, and
     * L5C9F/L5CB0 adjacency chains hold; the static byte counts
     * chain. */
    if (THERON_TRACK02_IPL_STAGE2_L5C8C_CALL_SITE_L5C06_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5C8C_CALL_SITE_L5C9F_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L536E_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L5C69_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5C25_CALL_SITE_L5439_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L55F6_CALL_SITE_L54A0_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L55F6_CALL_SITE_L5600_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L55F6_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5BF5_DATA_SITE_L5C20_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5BF5_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5C25_L5C2C_ENTRY_OFF + 2u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C25_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5C8C_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER2_L5CB0_USER_OFFSET ||
        sizeof(stage2_l5c06) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_BYTES ||
        sizeof(stage2_l5c20) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_BYTES ||
        sizeof(stage2_l5c69) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES ||
        sizeof(stage2_l5c9f) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES ||
        sizeof(stage2_l536e) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_BYTES ||
        sizeof(stage2_l5439) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_BYTES ||
        sizeof(stage2_l54a0) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_BYTES ||
        sizeof(stage2_l5600) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l5c06_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_BYTES;
    out_receipt->l5c20_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_BYTES;
    out_receipt->l5c69_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES;
    out_receipt->l5c9f_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES;
    out_receipt->l536e_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_BYTES;
    out_receipt->l5439_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_BYTES;
    out_receipt->l54a0_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_BYTES;
    out_receipt->l5600_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_BYTES;
    out_receipt->tier3_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES;
    out_receipt->l5c06_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C06_CPU_ADDRESS;
    out_receipt->l5c20_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C20_CPU_ADDRESS;
    out_receipt->l5c69_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_CPU_ADDRESS;
    out_receipt->l5c9f_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_CPU_ADDRESS;
    out_receipt->l536e_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_CPU_ADDRESS;
    out_receipt->l5439_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_CPU_ADDRESS;
    out_receipt->l54a0_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_CPU_ADDRESS;
    out_receipt->l5600_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_CPU_ADDRESS;
    out_receipt->l5c06_proven = 1;
    out_receipt->l5c20_proven = 1;
    out_receipt->l5c69_proven = 1;
    out_receipt->l5c9f_proven = 1;
    out_receipt->l536e_proven = 1;
    out_receipt->l5439_proven = 1;
    out_receipt->l54a0_proven = 1;
    out_receipt->l5600_proven = 1;
    out_receipt->l5c2c_entry_proven = 1;
    out_receipt->l5c8c_call_sites_proven = 1;
    out_receipt->l5c25_call_sites_proven = 1;
    out_receipt->l55f6_call_sites_proven = 1;
    out_receipt->l5bf5_data_site_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier4_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier4CalleesReceipt *out_receipt) {
    /* L4F7A body [0x0f7a..0x0f89): the JSR L4F7A target at L5C9F+0x09
     * (and inside the bound L5455 body), decoded inline by da65: PHX /
     * PHY / LDX L4FD4 / the CLY / DEY / BNE inner and DEX / BNE outer
     * delay nest / PLY / PLX / RTS — starting exactly where the bound
     * L4F66 delay loop ends; the next da65 label L4F89 confirms the
     * span.  Both BNE relatives resolve to their da65 labels (d0 fd ->
     * L4F80, d0 f9 -> L4F7F). */
    static const uint8_t stage2_l4f7a[] = {
        0xdau, 0x5au, 0xaeu, 0xd4u, 0x4fu, 0xc2u, 0x88u, 0xd0u,
        0xfdu, 0xcau, 0xd0u, 0xf9u, 0x7au, 0xfau, 0x60u
    };
    /* L535E body [0x135e..0x136e): the JSR L535E target inside the
     * bound L5482 body, decoded inline by da65: the LDA ($04),y / INY /
     * STA a:$02 / LDA ($04),y / INY / STA a:$03 / DEX / BNE L535E pair
     * loop / RTS — the media confirms da65's `a:` absolute stores
     * ($8D $02 $00), the BNE relative (d0 f1) resolves to the L535E
     * head, and the body ends exactly at the bound L536E entry. */
    static const uint8_t stage2_l535e[] = {
        0xb1u, 0x04u, 0xc8u, 0x8du, 0x02u, 0x00u, 0xb1u, 0x04u,
        0xc8u, 0x8du, 0x03u, 0x00u, 0xcau, 0xd0u, 0xf1u, 0x60u
    };
    /* L53C4 body [0x13c4..0x1403): the fall-through continuation of
     * the bound L536E bounds check, decoded inline by da65: JSR L51F9
     * (target bound in round 18) / the L4FD5/L4FD6 -> $04:$05 setup /
     * the L4F8B,y record-copy loop / the $0E:$0F tail store / BSR
     * L542D / the PHY/PHX/BSR L5403/PLX/PLY/DEY/BNE row loop / the
     * $04:$05 -> L4FD5/L4FD6 writeback / CLC / RTS — every relative
     * (BNE d0 f7 x2, BSR 44 45 -> L542D, BSR 44 11 -> L5403) resolves
     * to its da65 label; the next da65 label L5403 confirms the span
     * (L5403/L541E remain unbound tier-5 windows). */
    static const uint8_t stage2_l53c4[] = {
        0x20u, 0xf9u, 0x51u, 0xadu, 0xd5u, 0x4fu, 0x85u, 0x04u,
        0xadu, 0xd6u, 0x4fu, 0x85u, 0x05u, 0xa2u, 0x04u, 0xc2u,
        0xb9u, 0x8bu, 0x4fu, 0x91u, 0x04u, 0xc8u, 0xcau, 0xd0u,
        0xf7u, 0xa5u, 0x0eu, 0x91u, 0x04u, 0xc8u, 0xa5u, 0x0fu,
        0x91u, 0x04u, 0x44u, 0x45u, 0xacu, 0x8eu, 0x4fu, 0xaeu,
        0x8du, 0x4fu, 0x5au, 0xdau, 0x44u, 0x11u, 0xfau, 0x7au,
        0x88u, 0xd0u, 0xf7u, 0xa5u, 0x04u, 0x8du, 0xd5u, 0x4fu,
        0xa5u, 0x05u, 0x8du, 0xd6u, 0x4fu, 0x18u, 0x60u
    };
    /* L542D body [0x142d..0x1439): the BSR L542D target inside the
     * bound L5455/L53C4 bodies, decoded inline by da65: CLC / LDA $04 /
     * ADC #$06 / STA $04 / BCC L5438 / INC $05 / L5438: RTS — the
     * trailing RTS at 0x1438 sits immediately before the bound L5439
     * entry, confirming the span. */
    static const uint8_t stage2_l542d[] = {
        0x18u, 0xa5u, 0x04u, 0x69u, 0x06u, 0x85u, 0x04u, 0x90u,
        0x02u, 0xe6u, 0x05u, 0x60u
    };
    /* L5455 body [0x1455..0x1482): the continuation branched to from
     * the bound L5439 null-pair check, decoded inline by da65: the
     * $04:$05 -> L4FD5/L4FD6 writeback / the ($04),y field loads into
     * $0E:$0F and X/Y / BSR L542D / the PHY/PHX/JSR L4F7A/BSR
     * L5482/PLX/PLY/DEY/BNE L5475 loop / RTS — every relative (BSR
     * 44 b8 -> L542D, BSR 44 06 -> L5482, BNE d0 f4 -> L5475)
     * resolves to its da65 label; the body ends exactly at the bound
     * L5482 entry. */
    static const uint8_t stage2_l5455[] = {
        0xa5u, 0x04u, 0x8du, 0xd5u, 0x4fu, 0xa5u, 0x05u, 0x8du,
        0xd6u, 0x4fu, 0xa0u, 0x04u, 0xb1u, 0x04u, 0x85u, 0x0eu,
        0xc8u, 0xb1u, 0x04u, 0x85u, 0x0fu, 0xa0u, 0x02u, 0xb1u,
        0x04u, 0xaau, 0xc8u, 0xb1u, 0x04u, 0xa8u, 0x44u, 0xb8u,
        0x5au, 0xdau, 0x20u, 0x7au, 0x4fu, 0x44u, 0x06u, 0xfau,
        0x7au, 0x88u, 0xd0u, 0xf4u, 0x60u
    };
    /* L5482 body [0x1482..0x1492): the BSR L5482 target inside the
     * bound L5455 body, decoded inline by da65: PHX / CLY / DEC $5A /
     * JSR L54A0 (target bound in round 19) / JSR L535E / STZ $5A /
     * PLA / BSR L5492 (44 01) / RTS — the body ends exactly at the
     * bound L5492 entry. */
    static const uint8_t stage2_l5482[] = {
        0xdau, 0xc2u, 0xc6u, 0x5au, 0x20u, 0xa0u, 0x54u, 0x20u,
        0x5eu, 0x53u, 0x64u, 0x5au, 0x68u, 0x44u, 0x01u, 0x60u
    };
    /* L5492 body [0x1492..0x14a0): the JSR L5492 target at
     * L5C69+0x1f (and BSR L5492 inside the bound L5482/L5403 bodies),
     * decoded inline by da65: ASL A / CLC / ADC $04 / STA $04 / BCC
     * L549C / INC $05 / L549C: JSR L5213 (target bound in round 17) /
     * RTS — the body ends exactly at the bound L54A0 entry. */
    static const uint8_t stage2_l5492[] = {
        0x0au, 0x18u, 0x65u, 0x04u, 0x85u, 0x04u, 0x90u, 0x02u,
        0xe6u, 0x05u, 0x20u, 0x13u, 0x52u, 0x60u
    };
    /* L54AF body [0x14af..0x14c5): decoded inline by da65: the
     * L4F9F-indexed LDA L4FD7 / STA L4FA0,x / INX / LDA L4FD8 /
     * STA L4FA0,x / INC L4F9F pair store / RTS — starting exactly
     * where the bound L54A0 body ends; the next da65 label L54C5
     * confirms the span (L54C5 remains an unbound tier-5 window). */
    static const uint8_t stage2_l54af[] = {
        0xadu, 0x9fu, 0x4fu, 0x0au, 0xaau, 0xadu, 0xd7u, 0x4fu,
        0x9du, 0xa0u, 0x4fu, 0xe8u, 0xadu, 0xd8u, 0x4fu, 0x9du,
        0xa0u, 0x4fu, 0xeeu, 0x9fu, 0x4fu, 0x60u
    };
    /* L560B body [0x160b..0x1657): decoded inline by da65 with two
     * flagged artifact classes: the declared L563D label splits the
     * BCC operand byte at 0x163d (`.byte $90` plus the garbage `st0
     * #$EE` / `cld` / `.byte $4F` renderings — the round-18
     * mid-instruction class), so the media bytes are authoritative:
     * the real flow is BCC L5641 / INC $4FD8; and the L0000
     * zero-page-as-absolute renderings are superseded by the media
     * bytes.  The body: the L4FD5/L4FD6 +$11 -> $02:$03 and $5667 ->
     * $00:$01 setup / LDX #$09 / the PHX/$00:$01-save/BSR L5657/JSR
     * L52A2/CLX/JSR L52C8/L4FD7:+$10 row loop / the restore and
     * $00:$01 +8 advance / DEX / BNE L5623 / RTS — every relative
     * (BSR 44 2b -> L5657, BCC 90 03 -> L5641, BCC 90 02 -> L5653,
     * BNE d0 cd -> L5623) resolves to its da65 label; L52A2/L52C8/
     * L5657 remain unbound tier-5 windows. */
    static const uint8_t stage2_l560b[] = {
        0x18u, 0xadu, 0xd5u, 0x4fu, 0x69u, 0x11u, 0x85u, 0x02u,
        0x62u, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x03u, 0xa9u, 0x67u,
        0x85u, 0x00u, 0xa9u, 0x56u, 0x85u, 0x01u, 0xa2u, 0x09u,
        0xdau, 0xa5u, 0x00u, 0x48u, 0xa5u, 0x01u, 0x48u, 0x44u,
        0x2bu, 0x20u, 0xa2u, 0x52u, 0x82u, 0x20u, 0xc8u, 0x52u,
        0x18u, 0xadu, 0xd7u, 0x4fu, 0x69u, 0x10u, 0x8du, 0xd7u,
        0x4fu, 0x90u, 0x03u, 0xeeu, 0xd8u, 0x4fu, 0x68u, 0x85u,
        0x01u, 0x68u, 0x85u, 0x00u, 0xfau, 0x18u, 0xa5u, 0x00u,
        0x69u, 0x08u, 0x85u, 0x00u, 0x90u, 0x02u, 0xe6u, 0x01u,
        0xcau, 0xd0u, 0xcdu, 0x60u
    };
    /* Tier-4 call-site signatures at their compile-time-asserted
     * offsets inside the previously bound caller bodies; the targets
     * are encoded from the pinned callee CPU addresses. */
    static const uint8_t stage2_l5c9f_call_l4f7a[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_CPU_ADDRESS >>
                  8)
    };
    static const uint8_t stage2_l5c69_call_l5492[] = {
        0x20u,
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_CPU_ADDRESS &
                  0xffu),
        (uint8_t)(THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_CPU_ADDRESS >>
                  8)
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The tier-4 callee byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_USER_OFFSET,
            stage2_l4f7a, sizeof(stage2_l4f7a)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_USER_OFFSET,
            stage2_l535e, sizeof(stage2_l535e)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_USER_OFFSET,
            stage2_l53c4, sizeof(stage2_l53c4)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_USER_OFFSET,
            stage2_l542d, sizeof(stage2_l542d)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_USER_OFFSET,
            stage2_l5455, sizeof(stage2_l5455)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_USER_OFFSET,
            stage2_l5482, sizeof(stage2_l5482)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_USER_OFFSET,
            stage2_l5492, sizeof(stage2_l5492)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_USER_OFFSET,
            stage2_l54af, sizeof(stage2_l54af)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_USER_OFFSET,
            stage2_l560b, sizeof(stage2_l560b)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C9F_CALL_SITE_L4F7A_OFF,
            stage2_l5c9f_call_l4f7a, sizeof(stage2_l5c9f_call_l4f7a)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L5C69_CALL_SITE_L5492_OFF,
            stage2_l5c69_call_l5492, sizeof(stage2_l5c69_call_l5492))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Call-site and span assertions: the tier-4 call sites sit inside
     * their bound caller bodies; every callee window stays inside the
     * loaded image; all nine callees keep da65's linear CPU = image +
     * $4000 form; the adjacency chain (L4F66->L4F7A, L535E->L536E,
     * L542D->L5439->L5455->L5482->L5492->L54A0->L54AF, L5600->L560B)
     * holds; the static byte counts chain. */
    if (THERON_TRACK02_IPL_STAGE2_L5C9F_CALL_SITE_L4F7A_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C9F_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L5C69_CALL_SITE_L5492_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5C69_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_USER_OFFSET !=
            THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_CALLEE_L4F66_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L536E_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5439_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L54A0_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_USER_OFFSET !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER3_L5600_BYTES ||
        sizeof(stage2_l4f7a) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_BYTES ||
        sizeof(stage2_l535e) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_BYTES ||
        sizeof(stage2_l53c4) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_BYTES ||
        sizeof(stage2_l542d) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_BYTES ||
        sizeof(stage2_l5455) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_BYTES ||
        sizeof(stage2_l5482) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_BYTES ||
        sizeof(stage2_l5492) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_BYTES ||
        sizeof(stage2_l54af) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_BYTES ||
        sizeof(stage2_l560b) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l4f7a_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_BYTES;
    out_receipt->l535e_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_BYTES;
    out_receipt->l53c4_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_BYTES;
    out_receipt->l542d_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_BYTES;
    out_receipt->l5455_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_BYTES;
    out_receipt->l5482_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_BYTES;
    out_receipt->l5492_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_BYTES;
    out_receipt->l54af_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_BYTES;
    out_receipt->l560b_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES;
    out_receipt->tier4_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES;
    out_receipt->l4f7a_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L4F7A_CPU_ADDRESS;
    out_receipt->l535e_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L535E_CPU_ADDRESS;
    out_receipt->l53c4_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_CPU_ADDRESS;
    out_receipt->l542d_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_CPU_ADDRESS;
    out_receipt->l5455_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5455_CPU_ADDRESS;
    out_receipt->l5482_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5482_CPU_ADDRESS;
    out_receipt->l5492_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L5492_CPU_ADDRESS;
    out_receipt->l54af_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L54AF_CPU_ADDRESS;
    out_receipt->l560b_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_CPU_ADDRESS;
    out_receipt->l4f7a_proven = 1;
    out_receipt->l535e_proven = 1;
    out_receipt->l53c4_proven = 1;
    out_receipt->l542d_proven = 1;
    out_receipt->l5455_proven = 1;
    out_receipt->l5482_proven = 1;
    out_receipt->l5492_proven = 1;
    out_receipt->l54af_proven = 1;
    out_receipt->l560b_proven = 1;
    out_receipt->l5c9f_call_site_proven = 1;
    out_receipt->l5c69_call_site_proven = 1;
    out_receipt->tier4_chain_contiguous_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_enclosing_45xx(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2Enclosing45xxReceipt *out_receipt) {
    /* Enclosing $45xx routine [0x45b1..0x466a): the body that holds the
     * two bound $45xx-tier JSR $4696 windows at +0x09/+0x1a, decoded
     * inline by da65 (asm:10102-10191, no entry label): the LDA $12 /
     * PHA / LDY $11 prologue (the media confirms the A4 11 zero-page
     * form behind da65's L0011 rendering), the two LDA #$08 / STA $0E /
     * JSR L4696 multiply calls with their $54:$55 and $52:$53 result
     * saves, JSR L4552, the $13/accumulator ASL pair into
     * L47B8/L47B9, the DEC $5A / JSR L4932 / STZ $5A window, the
     * $14:$15 -> $0E:$0F / JSR L458E call, the $02:$03 -> $06:$07 and
     * $56-$58 -> L466A/$16/$17 saves (L466A is da65's data label for
     * the absolute $466A operand, not code), and the L47B9/2
     * X-trip-counted main loop (da65's L8610 head at +0x5f): the
     * L466A/$16/$17 -> $56-$58 restore, JSR L424B, the $47E0 ->
     * $00:$01 / $06:$07 -> $02:$03 / L47B8 -> $0E:$0F setup (the media
     * confirms the 85 00 zero-page form behind da65's L0000
     * rendering), JSR L466B, the $06:$07 +$40 advance, the $17
     * EOR #$02 toggle with its $16 row counter (STZ $16 plus the
     * L47BE/L47C4 accumulate and JSR L43D6 every $10 rows), DEX /
     * BNE L8610, RTS — every relative (BCC 90 02 -> +0x95, BNE d0 17
     * and d0 0f -> +0xb4, BNE d0 a7 -> +0x5f) resolves inside the
     * body, and the trailing RTS at 0x4669 sits immediately before the
     * next stream's BRK byte (da65 asm:10193), confirming the span.
     * L4552/L4932/L458E/L424B/L466B/L43D6 remain unbound future
     * windows. */
    static const uint8_t stage2_45xx_routine[] = {
        0xa5u, 0x12u, 0x48u, 0xa4u, 0x11u, 0xa9u, 0x08u, 0x85u,
        0x0eu, 0x20u, 0x96u, 0x46u, 0xa5u, 0x0eu, 0x85u, 0x54u,
        0xa5u, 0x0fu, 0x85u, 0x55u, 0x84u, 0x10u, 0xa9u, 0x08u,
        0x85u, 0x0eu, 0x20u, 0x96u, 0x46u, 0xa5u, 0x0eu, 0x85u,
        0x52u, 0xa5u, 0x0fu, 0x85u, 0x53u, 0x20u, 0x52u, 0x45u,
        0xa5u, 0x13u, 0x0au, 0x8du, 0xb8u, 0x47u, 0x68u, 0x0au,
        0x8du, 0xb9u, 0x47u, 0xc6u, 0x5au, 0x20u, 0x32u, 0x49u,
        0x64u, 0x5au, 0xa5u, 0x14u, 0x85u, 0x0eu, 0xa5u, 0x15u,
        0x85u, 0x0fu, 0x20u, 0x8eu, 0x45u, 0xa5u, 0x02u, 0x85u,
        0x06u, 0xa5u, 0x03u, 0x85u, 0x07u, 0xa5u, 0x56u, 0x8du,
        0x6au, 0x46u, 0xa5u, 0x57u, 0x85u, 0x16u, 0xa5u, 0x58u,
        0x85u, 0x17u, 0xadu, 0xb9u, 0x47u, 0x4au, 0xaau, 0xdau,
        0xadu, 0x6au, 0x46u, 0x85u, 0x56u, 0xa5u, 0x16u, 0x85u,
        0x57u, 0xa5u, 0x17u, 0x85u, 0x58u, 0x20u, 0x4bu, 0x42u,
        0xa9u, 0xe0u, 0x85u, 0x00u, 0xa9u, 0x47u, 0x85u, 0x01u,
        0xa5u, 0x06u, 0x85u, 0x02u, 0xa5u, 0x07u, 0x85u, 0x03u,
        0xadu, 0xb8u, 0x47u, 0x85u, 0x0eu, 0x64u, 0x0fu, 0x20u,
        0x6bu, 0x46u, 0xa9u, 0x40u, 0x18u, 0x65u, 0x06u, 0x85u,
        0x06u, 0x90u, 0x02u, 0xe6u, 0x07u, 0xa5u, 0x17u, 0x49u,
        0x02u, 0x85u, 0x17u, 0xd0u, 0x17u, 0xe6u, 0x16u, 0xa5u,
        0x16u, 0xc9u, 0x10u, 0xd0u, 0x0fu, 0x64u, 0x16u, 0xadu,
        0xbeu, 0x47u, 0x18u, 0x6du, 0xc4u, 0x47u, 0x8du, 0xc4u,
        0x47u, 0x20u, 0xd6u, 0x43u, 0xfau, 0xcau, 0xd0u, 0xa7u,
        0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The $45xx-routine byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same stream. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_USER_OFFSET,
            stage2_45xx_routine, sizeof(stage2_45xx_routine))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Span and call-site assertions: the whole body stays inside the
     * loaded image; the two round-17 JSR $4696 windows sit at their
     * compile-time-asserted offsets inside the body (the whole-body
     * exact match above covers their 20 96 46 bytes); da65's L8610
     * main-loop head lands inside the body; the static byte count
     * chains.  The entry CPU address is not pinned (no bound caller),
     * so the receipt carries 0 for it. */
    if (THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        sizeof(stage2_45xx_routine) !=
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4696_A_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4696_B_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4696_A_OFF !=
            THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_A_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4696_B_OFF !=
            THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_45XX_B_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_LOOP_HEAD_OFF >=
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->routine_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES;
    out_receipt->routine_cpu_address = 0u;
    out_receipt->routine_proven = 1;
    out_receipt->l4696_call_sites_within_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_enclosing_45xx_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2Enclosing45xxCalleesReceipt *out_receipt) {
    /* L424B body [0x424b..0x42bf): the JSR $424B target at +0x6d inside
     * the bound $45xx routine, decoded inline by da65 ($824B
     * rendering): STZ $13 / JSR L43D6 (bound below) / the L47B8 ->
     * $11 / LDA #$02 / STA $10 setup (da65 mis-splits the STA $10 /
     * CLX head into `.byte $85` / `bpl $821C` — the round-18
     * mid-instruction class; the media bytes are authoritative) / CLX /
     * the ($4E),y -> $14:$15 pair loads / JSR L43A1 / the $58-bit
     * X-trip count and the BSR-local (+0x5b) / DEC $11 / DEX / BNE
     * inner loop / JSR L42BF / the $58 &= #$02 update / the L8281
     * second pass with its BNE/BRA loops / RTS, then the BSR-local
     * subroutine: the ($00),y -> L47E0,x pair copy / PLX / RTS (da65
     * renders DEC $11 and LDA ($00),y as L0011/L0000 zero-page-as-
     * absolute — the media bytes are authoritative).  Every relative
     * (BSR 44 33 and 44 0d -> +0x5b, BNE d0 f9/d0 f7, BEQ f0 08, BRA
     * 80 dc -> +0x36) resolves inside the body; the trailing RTS of
     * the subroutine at 0x42be sits immediately before the unbound
     * L42BF (the JSR $42BF target), confirming the span.  L43A1/L42BF
     * remain unbound future windows. */
    static const uint8_t stage2_l424b[] = {
        0x64u, 0x13u, 0x20u, 0xd6u, 0x43u, 0xadu, 0xb8u, 0x47u,
        0x85u, 0x11u, 0xa9u, 0x02u, 0x85u, 0x10u, 0xc2u, 0xb1u,
        0x4eu, 0x85u, 0x14u, 0xc8u, 0xb1u, 0x4eu, 0x85u, 0x15u,
        0xc8u, 0x20u, 0xa1u, 0x43u, 0xa5u, 0x58u, 0x29u, 0x01u,
        0x49u, 0x03u, 0x3au, 0xaau, 0x64u, 0x12u, 0x44u, 0x33u,
        0xc6u, 0x11u, 0xcau, 0xd0u, 0xf9u, 0x20u, 0xbfu, 0x42u,
        0xa5u, 0x58u, 0x29u, 0x02u, 0x85u, 0x58u, 0xa4u, 0x10u,
        0xe6u, 0x10u, 0xe6u, 0x10u, 0xb1u, 0x4eu, 0x85u, 0x14u,
        0xc8u, 0xb1u, 0x4eu, 0x85u, 0x15u, 0x20u, 0xa1u, 0x43u,
        0xa2u, 0x02u, 0x64u, 0x12u, 0x44u, 0x0du, 0xc6u, 0x11u,
        0xf0u, 0x08u, 0xcau, 0xd0u, 0xf7u, 0x20u, 0xbfu, 0x42u,
        0x80u, 0xdcu, 0x60u, 0xdau, 0xa4u, 0x12u, 0xa6u, 0x13u,
        0xb1u, 0x00u, 0x9du, 0xe0u, 0x47u, 0xc8u, 0xe8u, 0xb1u,
        0x00u, 0x9du, 0xe0u, 0x47u, 0xc8u, 0xe8u, 0x84u, 0x12u,
        0x86u, 0x13u, 0xfau, 0x60u
    };
    /* L43D6 body [0x43d6..0x4417): the JSR $43D6 target at +0xb1
     * inside the bound $45xx routine and at +0x02 inside the bound
     * L424B body, decoded inline by da65 ($83D6 rendering): the
     * $57 -> $4E / four ASL $4E / ROL $4F shift pairs / the $56 add /
     * one more shift pair / the L47C4-conditional ASL A add into $4F /
     * the L47CD/L47CE -> $4E:$4F accumulate / RTS — the BEQ (f0 06)
     * resolves to da65's L8407 inside the body; the trailing RTS at
     * 0x4416 sits immediately before the unbound $4417 stream. */
    static const uint8_t stage2_l43d6[] = {
        0xa5u, 0x57u, 0x85u, 0x4eu, 0x64u, 0x4fu, 0x06u, 0x4eu,
        0x26u, 0x4fu, 0x06u, 0x4eu, 0x26u, 0x4fu, 0x06u, 0x4eu,
        0x26u, 0x4fu, 0x06u, 0x4eu, 0x26u, 0x4fu, 0xa5u, 0x56u,
        0x18u, 0x65u, 0x4eu, 0x85u, 0x4eu, 0x62u, 0x65u, 0x4fu,
        0x85u, 0x4fu, 0x06u, 0x4eu, 0x26u, 0x4fu, 0xadu, 0xc4u,
        0x47u, 0xf0u, 0x06u, 0x0au, 0x18u, 0x65u, 0x4fu, 0x85u,
        0x4fu, 0xadu, 0xcdu, 0x47u, 0x18u, 0x65u, 0x4eu, 0x85u,
        0x4eu, 0xadu, 0xceu, 0x47u, 0x65u, 0x4fu, 0x85u, 0x4fu,
        0x60u
    };
    /* L4552 body [0x4552..0x458e): the JSR $4552 target at +0x25
     * inside the bound $45xx routine, decoded inline by da65 ($8552
     * rendering): the $52/$54 nibble shifts into $56/$57 / the
     * $55-counted L47BE multiply-add loop (BEQ f0 0c -> L8573, BNE
     * d0 fa -> L856D) / the $53 accumulate into L47C4 / the
     * $54/$52 bit-3 pair into $58 / RTS — ending exactly at the bound
     * L458E entry (adjacency compile-time-asserted). */
    static const uint8_t stage2_l4552[] = {
        0xa5u, 0x52u, 0x4au, 0x4au, 0x4au, 0x4au, 0x85u, 0x56u,
        0xa5u, 0x54u, 0x4au, 0x4au, 0x4au, 0x4au, 0x85u, 0x57u,
        0x62u, 0xa6u, 0x55u, 0xf0u, 0x0cu, 0xadu, 0xbeu, 0x47u,
        0x85u, 0x0eu, 0x62u, 0x18u, 0x65u, 0x0eu, 0xcau, 0xd0u,
        0xfau, 0x18u, 0x65u, 0x53u, 0x8du, 0xc4u, 0x47u, 0xa5u,
        0x54u, 0x29u, 0x08u, 0x4au, 0x4au, 0x85u, 0x0eu, 0xa5u,
        0x52u, 0x29u, 0x08u, 0x4au, 0x4au, 0x4au, 0x18u, 0x65u,
        0x0eu, 0x85u, 0x58u, 0x60u
    };
    /* L458E body [0x458e..0x45a6): the JSR $458E target at +0x42
     * inside the bound $45xx routine, decoded inline by da65 ($858E
     * rendering): STZ $02 / the $0E LSR/ROR $02 pair x2 / STA $03 /
     * the $0F + $02 -> $02 add with its BCC/INC $03 carry / RTS (BCC
     * 90 02 -> L85A5) — ending exactly at the unbound STZ L47B8 /
     * TII gap routine [0x45a6..0x45b1). */
    static const uint8_t stage2_l458e[] = {
        0x64u, 0x02u, 0xa5u, 0x0eu, 0x4au, 0x66u, 0x02u, 0x4au,
        0x66u, 0x02u, 0x85u, 0x03u, 0xa5u, 0x0fu, 0x18u, 0x65u,
        0x02u, 0x85u, 0x02u, 0x90u, 0x02u, 0xe6u, 0x03u, 0x60u
    };
    /* L466B body [0x466b..0x4696): the JSR $466B target at +0x87
     * inside the bound $45xx routine, decoded inline by da65 ($866B
     * rendering): DEC $5A / ST0 #$00 / the $02:$03 -> $0002:$0003 VDC
     * data writes (da65's `a:$02`/`a:$03` absolute form confirmed by
     * the media 8D 02 00 / 8D 03 00) / ST0 #$02 / the $0E-conditional
     * self-modifying block (STA $4691/$468D/$468E rewrite the TIA
     * length and source operands — the L5C69 class; the media bytes
     * are the as-loaded image) / TIA $00,$02,$0000 / STZ $5A / RTS
     * (BEQ f0 14 -> L8693) — ending exactly at the bound L4696 body
     * (adjacency compile-time-asserted). */
    static const uint8_t stage2_l466b[] = {
        0xc6u, 0x5au, 0x03u, 0x00u, 0xa5u, 0x02u, 0x8du, 0x02u,
        0x00u, 0xa5u, 0x03u, 0x8du, 0x03u, 0x00u, 0x03u, 0x02u,
        0xa5u, 0x0eu, 0xf0u, 0x14u, 0x8du, 0x91u, 0x46u, 0xa5u,
        0x00u, 0x8du, 0x8du, 0x46u, 0xa5u, 0x01u, 0x8du, 0x8eu,
        0x46u, 0xe3u, 0x00u, 0x00u, 0x02u, 0x00u, 0x00u, 0x00u,
        0x64u, 0x5au, 0x60u
    };
    /* L4932 body [0x4932..0x4943): the JSR $4932 target at +0x35
     * inside the bound $45xx routine, decoded inline by da65 ($8932
     * rendering): ST0 #$05 / LDA $F3 / STA $0002 / LDA $F4 / AND #$07
     * / STA $F4 / STA $0003 / RTS (da65's `a:$02`/`a:$03` absolute
     * form confirmed by the media 8D 02 00 / 8D 03 00) — the trailing
     * RTS at 0x4942 sits immediately before the unbound TMA #$08 /
     * PHA stream. */
    static const uint8_t stage2_l4932[] = {
        0x03u, 0x05u, 0xa5u, 0xf3u, 0x8du, 0x02u, 0x00u, 0xa5u,
        0xf4u, 0x29u, 0x07u, 0x85u, 0xf4u, 0x8du, 0x03u, 0x00u,
        0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The $45xx-callee byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_USER_OFFSET,
            stage2_l424b, sizeof(stage2_l424b)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_USER_OFFSET,
            stage2_l43d6, sizeof(stage2_l43d6)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_USER_OFFSET,
            stage2_l4552, sizeof(stage2_l4552)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_USER_OFFSET,
            stage2_l458e, sizeof(stage2_l458e)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_USER_OFFSET,
            stage2_l466b, sizeof(stage2_l466b)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_USER_OFFSET,
            stage2_l4932, sizeof(stage2_l4932))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Span, call-site, and adjacency assertions: every callee window
     * stays inside the loaded image; each JSR target's CPU address is
     * pinned by the operand inside the bound $45xx body (the round-16
     * L4696 class — the encoded $4xxx address equals the image offset);
     * every $45xx call-site offset lands inside the bound $45xx body;
     * the internal JSR L43D6 sits at +0x02 inside the bound L424B
     * body; the L4552->L458E and L466B->L4696 adjacencies hold; the
     * static byte counts chain. */
    if (THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4552_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L4932_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L458E_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L424B_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L466B_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALL_SITE_L43D6_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L43D6_OFF +
                THERON_TRACK02_IPL_STAGE2_L4696_CALL_SITE_BYTES >
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_BYTES !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L4696_USER_OFFSET ||
        sizeof(stage2_l424b) !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES ||
        sizeof(stage2_l43d6) !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_BYTES ||
        sizeof(stage2_l4552) !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_BYTES ||
        sizeof(stage2_l458e) !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_BYTES ||
        sizeof(stage2_l466b) !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_BYTES ||
        sizeof(stage2_l4932) !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_BYTES !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l424b_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES;
    out_receipt->l43d6_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_BYTES;
    out_receipt->l4552_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_BYTES;
    out_receipt->l458e_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_BYTES;
    out_receipt->l466b_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_BYTES;
    out_receipt->l4932_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_BYTES;
    out_receipt->callees_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES;
    out_receipt->l424b_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_CPU_ADDRESS;
    out_receipt->l43d6_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_CPU_ADDRESS;
    out_receipt->l4552_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4552_CPU_ADDRESS;
    out_receipt->l458e_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L458E_CPU_ADDRESS;
    out_receipt->l466b_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L466B_CPU_ADDRESS;
    out_receipt->l4932_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L4932_CPU_ADDRESS;
    out_receipt->l424b_proven = 1;
    out_receipt->l43d6_proven = 1;
    out_receipt->l4552_proven = 1;
    out_receipt->l458e_proven = 1;
    out_receipt->l466b_proven = 1;
    out_receipt->l4932_proven = 1;
    out_receipt->l424b_call_site_proven = 1;
    out_receipt->adjacency_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_l3114_tier5_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage2L3114Tier5CalleesReceipt *out_receipt) {
    /* L5403 body [0x1403..0x141e): the BSR L5403 target at +0x2c
     * inside the bound L53C4 body, decoded inline by da65: PHX / CLY /
     * DEC $5A / BSR L541E / the `a:$02`/`a:$03` absolute-load (the
     * media confirms da65's 3-byte form AD 02 00 / AD 03 00) ->
     * ($04),y pair copy loop (BNE d0 f1 -> L5409) / STZ $5A / PLA /
     * BSR L5492 (target bound in round 20) / RTS — ending exactly at
     * the bound L541E entry. */
    static const uint8_t stage2_l5403[] = {
        0xdau, 0xc2u, 0xc6u, 0x5au, 0x44u, 0x15u, 0xadu, 0x02u,
        0x00u, 0x91u, 0x04u, 0xc8u, 0xadu, 0x03u, 0x00u, 0x91u,
        0x04u, 0xc8u, 0xcau, 0xd0u, 0xf1u, 0x64u, 0x5au, 0x68u,
        0x44u, 0x75u, 0x60u
    };
    /* L541E body [0x141e..0x142d): the BSR L541E target at +0x04
     * inside the bound L5403 body, decoded inline by da65: ST0 #$01 /
     * the $0E:$0F -> $0002:$0003 VDC address writes (da65's `a:$02`/
     * `a:$03` absolute form media-confirmed) / ST0 #$02 / RTS —
     * ending exactly at the bound L542D entry (adjacency
     * compile-time-asserted). */
    static const uint8_t stage2_l541e[] = {
        0x03u, 0x01u, 0xa5u, 0x0eu, 0x8du, 0x02u, 0x00u, 0xa5u,
        0x0fu, 0x8du, 0x03u, 0x00u, 0x03u, 0x02u, 0x60u
    };
    /* L52A2 body [0x12a2..0x12c8): the JSR L52A2 target at +0x21
     * inside the bound L560B body, decoded inline by da65: the
     * L4FD7/L4FD8 -> $06:$07 copy, the L4FD5/L4FD6 +$20 -> $04:$05
     * add, and the $10/$11 -> $15/$14 and 0/1 -> $17/$16 field
     * setup / RTS — ending exactly at the bound L52C8 entry. */
    static const uint8_t stage2_l52a2[] = {
        0xadu, 0xd7u, 0x4fu, 0x85u, 0x06u, 0xadu, 0xd8u, 0x4fu,
        0x85u, 0x07u, 0x18u, 0xadu, 0xd5u, 0x4fu, 0x69u, 0x20u,
        0x85u, 0x04u, 0x62u, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x05u,
        0xa9u, 0x10u, 0x85u, 0x15u, 0x1au, 0x85u, 0x14u, 0x62u,
        0x85u, 0x17u, 0x1au, 0x85u, 0x16u, 0x60u
    };
    /* L52C8 body [0x12c8..0x12da): the JSR L52C8 target at +0x25
     * inside the bound L560B body, decoded inline by da65: the $14,x
     * + L4FD5/L4FD6 -> $00:$01 add (da65 renders STA $00 as L0000
     * zero-page-as-absolute — the media 85 00 is authoritative) /
     * JSR L52FD / RTS — ending exactly at the unbound L52DA entry
     * (L52FD/L52DA remain unbound future windows). */
    static const uint8_t stage2_l52c8[] = {
        0xb5u, 0x14u, 0x18u, 0x6du, 0xd5u, 0x4fu, 0x85u, 0x00u,
        0x62u, 0x6du, 0xd6u, 0x4fu, 0x85u, 0x01u, 0x20u, 0xfdu,
        0x52u, 0x60u
    };
    /* L5657 body [0x1657..0x1667): the BSR L5657 target at +0x1f
     * inside the bound L560B body, decoded inline by da65: CLX / CLY /
     * the LDA ($00),y / SXY / STA ($02),y / INY x2 / INX / SXY /
     * CPY #$08 / BNE L5659 8-byte copy loop / RTS — ending exactly at
     * the bound L5667 data table. */
    static const uint8_t stage2_l5657[] = {
        0x82u, 0xc2u, 0xb1u, 0x00u, 0x02u, 0x91u, 0x02u, 0xc8u,
        0xc8u, 0xe8u, 0x02u, 0xc0u, 0x08u, 0xd0u, 0xf3u, 0x60u
    };
    /* L54C5 body [0x14c5..0x14db): decoded inline by da65: the
     * DEC L4F9F / L4F9F-indexed LDA L4FA0,x / STA L4FD7 / INX /
     * LDA L4FA0,x / STA L4FD8 pair load / RTS — called only from the
     * unbound L54DB stream; ending exactly at that stream (L54DB
     * remains an unbound future window). */
    static const uint8_t stage2_l54c5[] = {
        0xceu, 0x9fu, 0x4fu, 0xadu, 0x9fu, 0x4fu, 0x0au, 0xaau,
        0xbdu, 0xa0u, 0x4fu, 0x8du, 0xd7u, 0x4fu, 0xe8u, 0xbdu,
        0xa0u, 0x4fu, 0x8du, 0xd8u, 0x4fu, 0x60u
    };
    /* L5667 data table [0x1667..0x16af): 9 rows x 8 bytes read
     * through the bound L560B body's $00:$01 setup (LDA #$67 / STA $00
     * / LDA #$56 / STA $01 at L560B+0x0e) by the L5657 copy loop — da65
     * garbage-decodes the table as bbs7/cpy/brk/st0 code, so the media
     * bytes are authoritative; code resumes at da65's L56AF. */
    static const uint8_t stage2_l5667_data[] = {
        0xffu, 0xffu, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u,
        0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u,
        0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xc0u, 0xffu,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xffu,
        0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0x03u, 0xffu
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The tier-5 callee byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_USER_OFFSET,
            stage2_l5403, sizeof(stage2_l5403)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_USER_OFFSET,
            stage2_l541e, sizeof(stage2_l541e)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_USER_OFFSET,
            stage2_l52a2, sizeof(stage2_l52a2)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_USER_OFFSET,
            stage2_l52c8, sizeof(stage2_l52c8)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_USER_OFFSET,
            stage2_l5657, sizeof(stage2_l5657)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_USER_OFFSET,
            stage2_l54c5, sizeof(stage2_l54c5)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_USER_OFFSET,
            stage2_l5667_data, sizeof(stage2_l5667_data))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Span, call-site, and adjacency assertions: every tier-5 window
     * stays inside the loaded image; every callee keeps da65's linear
     * CPU = image + $4000 form; the call sites sit inside their bound
     * caller bodies (the whole-body exact matches above and in rounds
     * 19-20 cover their opcode bytes); the L560B data site lands inside
     * the bound L560B body; the L5403->L541E->L542D, L52A2->L52C8, and
     * L5657->L5667 adjacency chain holds; the static byte counts
     * chain. */
    if (THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_USER_OFFSET +
                0x4000u ||
        THERON_TRACK02_IPL_STAGE2_L53C4_CALL_SITE_L5403_OFF + 2u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L53C4_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L560B_CALL_SITE_L5657_OFF + 2u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L560B_CALL_SITE_L52A2_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L560B_CALL_SITE_L52C8_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L560B_DATA_SITE_L5667_OFF + 8u >
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L560B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER4_L542D_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_USER_OFFSET ||
        sizeof(stage2_l5403) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_BYTES ||
        sizeof(stage2_l541e) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_BYTES ||
        sizeof(stage2_l52a2) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_BYTES ||
        sizeof(stage2_l52c8) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_BYTES ||
        sizeof(stage2_l5657) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_BYTES ||
        sizeof(stage2_l54c5) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_BYTES ||
        sizeof(stage2_l5667_data) !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_BYTES +
                THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_BYTES !=
            THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l5403_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_BYTES;
    out_receipt->l541e_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_BYTES;
    out_receipt->l52a2_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_BYTES;
    out_receipt->l52c8_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_BYTES;
    out_receipt->l5657_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_BYTES;
    out_receipt->l54c5_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_BYTES;
    out_receipt->l5667_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_BYTES;
    out_receipt->tier5_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES;
    out_receipt->l5403_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5403_CPU_ADDRESS;
    out_receipt->l541e_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L541E_CPU_ADDRESS;
    out_receipt->l52a2_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52A2_CPU_ADDRESS;
    out_receipt->l52c8_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L52C8_CPU_ADDRESS;
    out_receipt->l5657_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5657_CPU_ADDRESS;
    out_receipt->l54c5_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L54C5_CPU_ADDRESS;
    out_receipt->l5667_cpu_address =
        THERON_TRACK02_IPL_STAGE2_L3114_TIER5_L5667_CPU_ADDRESS;
    out_receipt->l5403_proven = 1;
    out_receipt->l541e_proven = 1;
    out_receipt->l52a2_proven = 1;
    out_receipt->l52c8_proven = 1;
    out_receipt->l5657_proven = 1;
    out_receipt->l54c5_proven = 1;
    out_receipt->l5667_proven = 1;
    out_receipt->l53c4_call_site_proven = 1;
    out_receipt->l560b_call_sites_proven = 1;
    out_receipt->tier5_chain_contiguous_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}

Theron_Track02SignalStatus theron_v1_track02_verify_stage2_45xx_tier2_callees(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02Stage245xxTier2CalleesReceipt *out_receipt) {
    /* L43A1 body [0x43a1..0x43d6): the JSR $43A1 target at +0x19 and
     * +0x45 inside the bound L424B body, decoded inline by da65 ($83A1
     * rendering): the $14:$15 -> $0E:$0F copy, three ASL $0E / ROL $0F
     * shift pairs, the L47CB/L47CC add, the $58 ASL A add, and the
     * $0E:$0F -> $00:$01 finish (da65 renders STA $00 as L0000
     * zero-page-as-absolute — the media 85 00 is authoritative) / RTS
     * — ending exactly at the bound L43D6 body (adjacency
     * compile-time-asserted). */
    static const uint8_t stage2_l43a1[] = {
        0xa5u, 0x14u, 0x85u, 0x0eu, 0xa5u, 0x15u, 0x85u, 0x0fu,
        0x06u, 0x0eu, 0x26u, 0x0fu, 0x06u, 0x0eu, 0x26u, 0x0fu,
        0x06u, 0x0eu, 0x26u, 0x0fu, 0xadu, 0xcbu, 0x47u, 0x18u,
        0x65u, 0x0eu, 0x85u, 0x0eu, 0xadu, 0xccu, 0x47u, 0x65u,
        0x0fu, 0x85u, 0x0fu, 0xa5u, 0x58u, 0x0au, 0x18u, 0x65u,
        0x0eu, 0x85u, 0x0eu, 0x85u, 0x00u, 0x62u, 0x65u, 0x0fu,
        0x85u, 0x0fu, 0x85u, 0x01u, 0x60u
    };
    /* L42BF body [0x42bf..0x42db): the JSR $42BF target at +0x2d and
     * +0x55 inside the bound L424B body, decoded inline by da65 ($82BF
     * rendering): the $56 $10-counter (INC $56 / LDX $56 / CPX #$10 /
     * BNE L82DA to the trailing RTS) with its L47C4 save / INC A /
     * store / JSR L43D6 (target bound in round 21; the internal call
     * site sits at +0x14) / restore — ending at the unbound $3B75
     * stream (that stream remains an unbound future window). */
    static const uint8_t stage2_l42bf[] = {
        0xe6u, 0x56u, 0xa6u, 0x56u, 0xe0u, 0x10u, 0xd0u, 0x13u,
        0x64u, 0x56u, 0x64u, 0x10u, 0xadu, 0xc4u, 0x47u, 0x48u,
        0x1au, 0x8du, 0xc4u, 0x47u, 0x20u, 0xd6u, 0x43u, 0x68u,
        0x8du, 0xc4u, 0x47u, 0x60u
    };
    /* $45A6 TII gap stream [0x45a6..0x45b1): decoded inline by da65
     * ($85A6 rendering, immediately after its L85A5 RTS): STZ L47B8 /
     * TII $47B8,$47B9,$00A7 / RTS — called only from the unbound $401C
     * stream (JSR $45A6 at image 0x401c), so the entry CPU address is
     * not pinned (the $45xx-routine precedent); the trailing RTS at
     * 0x45b0 sits immediately before the bound $45xx routine entry,
     * confirming the span (adjacency compile-time-asserted). */
    static const uint8_t stage2_gap45a6[] = {
        0x9cu, 0xb8u, 0x47u, 0x73u, 0xb8u, 0x47u, 0xb9u, 0x47u,
        0xa7u, 0x00u, 0x60u
    };
    Theron_Track02IplLoaderReceipt loader;
    Theron_Track02SignalStatus status;
    size_t stage2_sector;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !md5_hex || !out_receipt) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    status = theron_v1_track02_find_ipl_loader(track02_data, track02_size,
                                                md5_hex, &loader);
    if (status != THERON_TRACK02_SIGNAL_OK) return status;
    /* The $45xx-tier-2 byte identity is attested only for the
     * authenticated US stage-two body; the JP body rejects here until
     * staged JP media can verify the same streams. */
    if (loader.variant != THERON_TRACK02_VARIANT_US_BIN ||
        !loader.stage2_seed_call_sites_proven) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    stage2_sector = loader.stage2_raw_sector;
    if (!tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_USER_OFFSET,
            stage2_l43a1, sizeof(stage2_l43a1)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_USER_OFFSET,
            stage2_l42bf, sizeof(stage2_l42bf)) ||
        !tqr_ipl_user_match(
            track02_data, track02_size, stage2_sector,
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT,
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_USER_OFFSET,
            stage2_gap45a6, sizeof(stage2_gap45a6))) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    /* Span, call-site, and adjacency assertions: every window stays
     * inside the loaded image; L43A1/L42BF keep their CPU addresses
     * pinned by the JSR operands inside the bound L424B body (the
     * round-16 L4696 class — the encoded $4xxx address equals the
     * image offset); every L424B call-site offset lands inside the
     * bound L424B body; the internal L42BF JSR L43D6 lands inside the
     * bound L42BF body; the L43A1->L43D6 and $45A6->$45xx-routine
     * adjacencies hold; the static byte counts chain.  The $45A6 entry
     * CPU address is not pinned (no bound caller), so the receipt
     * carries 0 for it. */
    if (THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES >
            THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                TQR_RAW_SECTOR_USER_DATA_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_CPU_ADDRESS !=
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L43A1_A_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L43A1_B_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L42BF_A_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L424B_CALL_SITE_L42BF_B_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L424B_BYTES ||
        THERON_TRACK02_IPL_STAGE2_L42BF_CALL_SITE_L43D6_OFF + 3u >
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_BYTES !=
            THERON_TRACK02_IPL_STAGE2_45XX_CALLEE_L43D6_USER_OFFSET ||
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_USER_OFFSET +
                THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES !=
            THERON_TRACK02_IPL_STAGE2_45XX_ROUTINE_USER_OFFSET ||
        sizeof(stage2_l43a1) !=
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_BYTES ||
        sizeof(stage2_l42bf) !=
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_BYTES ||
        sizeof(stage2_gap45a6) !=
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES ||
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_BYTES +
                THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES !=
            THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_receipt->valid = 1;
    out_receipt->variant = loader.variant;
    out_receipt->stage2_record = loader.stage2_record;
    out_receipt->stage2_raw_sector = stage2_sector;
    out_receipt->l43a1_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_BYTES;
    out_receipt->l42bf_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_BYTES;
    out_receipt->gap45a6_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_GAP45A6_BYTES;
    out_receipt->tier2_bound_bytes =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES;
    out_receipt->l43a1_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L43A1_CPU_ADDRESS;
    out_receipt->l42bf_cpu_address =
        THERON_TRACK02_IPL_STAGE2_45XX_TIER2_L42BF_CPU_ADDRESS;
    out_receipt->gap45a6_cpu_address = 0u;
    out_receipt->l43a1_proven = 1;
    out_receipt->l42bf_proven = 1;
    out_receipt->gap45a6_proven = 1;
    out_receipt->l424b_call_sites_proven = 1;
    out_receipt->adjacency_proven = 1;
    return THERON_TRACK02_SIGNAL_OK;
}
