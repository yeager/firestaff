#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "asset_status_m12.h"
#include "theron_v1_track02_raw_media_intake.h"

#define THERON_V1_TRACK02_CUE_MAX_BYTES (1024u * 1024u)

static int theron_v1_track02_media_ieq(const char *left, const char *right) {
    while (left && right && *left && *right) {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) {
            return 0;
        }
        ++left;
        ++right;
    }
    return left && right && *left == '\0' && *right == '\0';
}

static int theron_v1_track02_media_starts_with_i(const char *text,
                                                  const char *prefix) {
    while (text && prefix && *prefix) {
        if (!*text || tolower((unsigned char)*text) !=
                          tolower((unsigned char)*prefix)) {
            return 0;
        }
        ++text;
        ++prefix;
    }
    return prefix && *prefix == '\0';
}

static const char *theron_v1_track02_media_extension(const char *path) {
    const char *dot = path ? strrchr(path, '.') : NULL;
    return dot ? dot : "";
}

const char *theron_v1_track02_media_failure_reason_id(
    Theron_V1Track02MediaFailureReason reason) {
    switch (reason) {
    case THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE: return "path_unavailable";
    case THERON_V1_TRACK02_MEDIA_REASON_UNSUPPORTED_CONTAINER: return "unsupported_container";
    case THERON_V1_TRACK02_MEDIA_REASON_CUE_LAYOUT_INVALID: return "cue_layout_invalid";
    case THERON_V1_TRACK02_MEDIA_REASON_PAYLOAD_UNAVAILABLE: return "payload_unavailable";
    case THERON_V1_TRACK02_MEDIA_REASON_SECTOR_ALIGNMENT_INVALID: return "sector_alignment_invalid";
    case THERON_V1_TRACK02_MEDIA_REASON_TRACK02_HASH_UNKNOWN: return "track02_hash_unknown";
    case THERON_V1_TRACK02_MEDIA_REASON_LAYOUT_HASH_MISMATCH: return "layout_hash_mismatch";
    case THERON_V1_TRACK02_MEDIA_REASON_CUE_INDEX_INVALID: return "cue_index_invalid";
    case THERON_V1_TRACK02_MEDIA_REASON_USER_DATA_WINDOW_INVALID: return "user_data_window_invalid";
    case THERON_V1_TRACK02_MEDIA_REASON_EXPECTED_HASH_MISMATCH: return "expected_hash_mismatch";
    default: return "none";
    }
}

static void theron_v1_track02_media_reject(
    Theron_V1Track02RawMediaIntakeReceipt *receipt,
    Theron_V1Track02MediaFailureReason reason) {
    receipt->status = THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED;
    receipt->failure_reason = reason;
}

static int theron_v1_track02_media_file_size(const char *path, size_t *out) {
    FILE *file;
    long size;

    if (!path || !out || !(file = fopen(path, "rb"))) {
        return 0;
    }
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0) {
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = (size_t)size;
    return 1;
}

static int theron_v1_track02_media_join_cue_member(
    const char *cue_path,
    const char *member,
    char out[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY]) {
    const char *slash;
    size_t prefix;

    if (!cue_path || !member || !member[0] || !out) {
        return 0;
    }
    if (member[0] == '/' || member[0] == '\\' ||
        (isalpha((unsigned char)member[0]) && member[1] == ':')) {
        return snprintf(out, THERON_V1_TRACK02_MEDIA_PATH_CAPACITY, "%s",
                        member) < THERON_V1_TRACK02_MEDIA_PATH_CAPACITY;
    }
    slash = strrchr(cue_path, '/');
    if (!slash) {
        slash = strrchr(cue_path, '\\');
    }
    prefix = slash ? (size_t)(slash - cue_path + 1) : 0u;
    if (prefix + strlen(member) >= THERON_V1_TRACK02_MEDIA_PATH_CAPACITY) {
        return 0;
    }
    memcpy(out, cue_path, prefix);
    snprintf(out + prefix, THERON_V1_TRACK02_MEDIA_PATH_CAPACITY - prefix,
             "%s", member);
    return 1;
}

static int theron_v1_track02_media_parse_msf(const char *text,
                                              uint32_t *out_sector) {
    unsigned int minute;
    unsigned int second;
    unsigned int frame;

    if (!text || !out_sector ||
        sscanf(text, "%u:%u:%u", &minute, &second, &frame) != 3 ||
        second >= 60u || frame >= 75u ||
        minute > (UINT32_MAX / (60u * 75u))) {
        return 0;
    }
    *out_sector = (minute * 60u + second) * 75u + frame;
    return 1;
}

static int theron_v1_track02_media_parse_index01(const char *text,
                                                   uint32_t *out_sector) {
    if (!text || !theron_v1_track02_media_starts_with_i(text, "INDEX 01 ")) {
        return 0;
    }
    return theron_v1_track02_media_parse_msf(text + strlen("INDEX 01 "),
                                              out_sector);
}

static int theron_v1_track02_media_parse_cue(const char *cue_path,
                                               char payload_path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY],
                                               int *out_sector_bytes,
                                               uint32_t *out_index01_sector,
                                               uint32_t *out_payload_index01_sector) {
    FILE *file;
    char line[1024];
    char current_member[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY] = {0};
    int current_binary = 0;
    int track02_count = 0;
    int index01_count = 0;
    int pregap_count = 0;
    int sector_bytes = 0;
    unsigned int current_track = 0u;
    uint32_t index01_sector = 0u;
    uint32_t pregap_sector = 0u;
    long cue_bytes;

    if (!cue_path || !payload_path || !out_sector_bytes ||
        !out_index01_sector || !out_payload_index01_sector ||
        !(file = fopen(cue_path, "rb"))) {
        return 0;
    }
    if (fseek(file, 0L, SEEK_END) != 0 || (cue_bytes = ftell(file)) < 0 ||
        (unsigned long)cue_bytes > THERON_V1_TRACK02_CUE_MAX_BYTES ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    while (fgets(line, sizeof(line), file)) {
        char member[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY];
        unsigned int track;
        char mode[32];
        char *p = line;

        while (*p == ' ' || *p == '\t') ++p;
        /* A UTF-8 BOM may prefix the first CUE directive. It carries no CUE
         * syntax, so discard only the exact marker and keep every layout
         * validation below unchanged. */
        if ((unsigned char)p[0] == 0xefu &&
            (unsigned char)p[1] == 0xbbu && (unsigned char)p[2] == 0xbfu) {
            p += 3;
        }
        if (theron_v1_track02_media_starts_with_i(p, "FILE ")) {
            char type[32];
            if (sscanf(p, "FILE \"%511[^\"]\" %31s", member, type) != 2 &&
                sscanf(p, "FILE %511s %31s", member, type) != 2) {
                fclose(file);
                return 0;
            }
            snprintf(current_member, sizeof(current_member), "%s", member);
            current_binary = theron_v1_track02_media_ieq(type, "BINARY");
        } else if (sscanf(p, "TRACK %u %31s", &track, mode) == 2) {
            current_track = track;
            if (track == 2u) {
                ++track02_count;
                if (!current_binary || current_member[0] == '\0') {
                    fclose(file);
                    return 0;
                }
                if (theron_v1_track02_media_ieq(mode, "MODE1/2352")) {
                    sector_bytes = 2352;
                } else if (theron_v1_track02_media_ieq(mode, "MODE1/2048")) {
                    sector_bytes = 2048;
                } else {
                    fclose(file);
                    return 0;
                }
                if (!theron_v1_track02_media_join_cue_member(
                        cue_path, current_member, payload_path)) {
                    fclose(file);
                    return 0;
                }
            }
        } else if (current_track == 2u &&
                   theron_v1_track02_media_starts_with_i(p, "INDEX 01 ")) {
            ++index01_count;
            if (!theron_v1_track02_media_parse_index01(p, &index01_sector)) {
                fclose(file);
                return 0;
            }
        } else if (current_track == 2u &&
                   theron_v1_track02_media_starts_with_i(p, "PREGAP ")) {
            ++pregap_count;
            if (pregap_count != 1 ||
                !theron_v1_track02_media_parse_msf(p + strlen("PREGAP "),
                                                    &pregap_sector)) {
                fclose(file);
                return 0;
            }
        }
    }
    fclose(file);
    if (track02_count != 1 || index01_count != 1 || sector_bytes == 0) {
        return 0;
    }
    if (pregap_sector > UINT32_MAX - index01_sector) return 0;
    *out_sector_bytes = sector_bytes;
    *out_index01_sector = pregap_sector + index01_sector;
    *out_payload_index01_sector = index01_sector;
    return 1;
}

static int theron_v1_track02_media_resolve_track02_alias(
    char payload_path[THERON_V1_TRACK02_MEDIA_PATH_CAPACITY])
{
    static const struct { const char *declared; const char *materialized; } aliases[] = {
        { "TQJP02.iso", "TQJP02End.iso" },
        { "TQUS02.iso", "TQUS02End.iso" }
    };
    char *leaf;
    size_t i;
    FILE *file;

    if (!payload_path || !payload_path[0]) return 0;
    if ((file = fopen(payload_path, "rb")) != NULL) {
        fclose(file);
        return 1;
    }
    leaf = strrchr(payload_path, '/');
    if (!leaf) leaf = strrchr(payload_path, '\\');
    leaf = leaf ? leaf + 1 : payload_path;
    for (i = 0u; i < sizeof(aliases) / sizeof(aliases[0]); ++i) {
        size_t remaining = THERON_V1_TRACK02_MEDIA_PATH_CAPACITY -
            (size_t)(leaf - payload_path);
        if (!theron_v1_track02_media_ieq(leaf, aliases[i].declared) ||
            snprintf(leaf, remaining, "%s", aliases[i].materialized) >=
                (int)remaining) continue;
        if ((file = fopen(payload_path, "rb")) != NULL) {
            fclose(file);
            return 1;
        }
    }
    return 0;
}

static uint32_t theron_v1_track02_expected_raw_index01(
    Theron_Track02Variant variant) {
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) return 224u;
    if (variant == THERON_TRACK02_VARIANT_US_BIN) return 225u;
    return 0u;
}

Theron_V1Track02MediaFailureReason
theron_v1_track02_raw_media_intake_validate_verified_layout(
    const char *track02_md5,
    int sector_bytes,
    int cue_consumed,
    uint32_t cue_index01_sector,
    uint32_t payload_index01_sector,
    size_t payload_bytes,
    Theron_Track02Variant *out_variant) {
    Theron_Track02Variant variant;
    size_t sector_count;
    size_t first_user_data_offset;

    if (out_variant) *out_variant = THERON_TRACK02_VARIANT_UNKNOWN;
    if (!track02_md5 ||
        (variant = theron_v1_track02_variant_for_md5(track02_md5)) ==
            THERON_TRACK02_VARIANT_UNKNOWN) {
        return THERON_V1_TRACK02_MEDIA_REASON_TRACK02_HASH_UNKNOWN;
    }
    if (sector_bytes != 2048 && sector_bytes != 2352) {
        return THERON_V1_TRACK02_MEDIA_REASON_LAYOUT_HASH_MISMATCH;
    }
    if (payload_bytes == 0u || payload_bytes % (size_t)sector_bytes != 0u) {
        return THERON_V1_TRACK02_MEDIA_REASON_SECTOR_ALIGNMENT_INVALID;
    }
    if (((variant == THERON_TRACK02_VARIANT_JP_BIN ||
          variant == THERON_TRACK02_VARIANT_US_BIN) && sector_bytes != 2352) ||
        ((variant == THERON_TRACK02_VARIANT_US_ISO ||
          variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) && sector_bytes != 2048)) {
        return THERON_V1_TRACK02_MEDIA_REASON_LAYOUT_HASH_MISMATCH;
    }
    if (cue_consumed && sector_bytes == 2352 &&
        cue_index01_sector != theron_v1_track02_expected_raw_index01(variant)) {
        return THERON_V1_TRACK02_MEDIA_REASON_CUE_INDEX_INVALID;
    }
    sector_count = payload_bytes / (size_t)sector_bytes;
    if (payload_index01_sector >= sector_count) {
        return THERON_V1_TRACK02_MEDIA_REASON_USER_DATA_WINDOW_INVALID;
    }
    first_user_data_offset = sector_bytes == 2352
        ? (size_t)payload_index01_sector * 2352u + THERON_TRACK02_RAW_USER_DATA_OFFSET
        : (size_t)payload_index01_sector * 2048u;
    if (first_user_data_offset > payload_bytes ||
        payload_bytes - first_user_data_offset < 2048u) {
        return THERON_V1_TRACK02_MEDIA_REASON_USER_DATA_WINDOW_INVALID;
    }
    if (out_variant) *out_variant = variant;
    return THERON_V1_TRACK02_MEDIA_REASON_NONE;
}

int theron_v1_track02_raw_media_intake_discover(
    const char *media_path,
    Theron_V1Track02RawMediaIntakeReceipt *out) {
    Theron_V1Track02RawMediaIntakeReceipt receipt = {0};
    char md5[33];
    int sector_bytes;
    uint32_t index01_sector = 0u;
    uint32_t payload_index01_sector = 0u;
    size_t payload_bytes;
    Theron_Track02Variant variant;

    if (!out) return 0;
    *out = receipt;
    if (!media_path || !media_path[0]) {
        receipt.status = THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE;
        receipt.failure_reason = THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    snprintf(receipt.media_path, sizeof(receipt.media_path), "%s", media_path);
    if (theron_v1_track02_media_ieq(
            theron_v1_track02_media_extension(media_path), ".cue")) {
        receipt.cue_consumed = 1;
        if (!theron_v1_track02_media_file_size(media_path, &payload_bytes)) {
            receipt.status = THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE;
            receipt.failure_reason = THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE;
            *out = receipt;
            return 1;
        }
        if (!theron_v1_track02_media_parse_cue(media_path, receipt.payload_path,
                                                &sector_bytes, &index01_sector,
                                                &payload_index01_sector)) {
            theron_v1_track02_media_reject(&receipt,
                                            THERON_V1_TRACK02_MEDIA_REASON_CUE_LAYOUT_INVALID);
            *out = receipt;
            return 1;
        }
    } else if (theron_v1_track02_media_ieq(
                   theron_v1_track02_media_extension(media_path), ".bin")) {
        sector_bytes = 2352;
        snprintf(receipt.payload_path, sizeof(receipt.payload_path), "%s",
                 media_path);
    } else if (theron_v1_track02_media_ieq(
                   theron_v1_track02_media_extension(media_path), ".iso")) {
        sector_bytes = 2048;
        snprintf(receipt.payload_path, sizeof(receipt.payload_path), "%s",
                 media_path);
    } else {
        theron_v1_track02_media_reject(&receipt,
                                        THERON_V1_TRACK02_MEDIA_REASON_UNSUPPORTED_CONTAINER);
        *out = receipt;
        return 1;
    }
    (void)theron_v1_track02_media_resolve_track02_alias(receipt.payload_path);
    if (!theron_v1_track02_media_file_size(receipt.payload_path,
                                            &payload_bytes)) {
        receipt.status = THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE;
        receipt.failure_reason = THERON_V1_TRACK02_MEDIA_REASON_PAYLOAD_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (!m12_file_md5_hex(receipt.payload_path, md5)) {
        theron_v1_track02_media_reject(&receipt,
                                        THERON_V1_TRACK02_MEDIA_REASON_TRACK02_HASH_UNKNOWN);
        *out = receipt;
        return 1;
    }
    {
        Theron_V1Track02MediaFailureReason reason =
            theron_v1_track02_raw_media_intake_validate_verified_layout(
                md5, sector_bytes, receipt.cue_consumed, index01_sector,
                payload_index01_sector, payload_bytes, &variant);
        if (reason != THERON_V1_TRACK02_MEDIA_REASON_NONE) {
            theron_v1_track02_media_reject(&receipt, reason);
            *out = receipt;
            return 1;
        }
    }

    receipt.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    receipt.mode1_2352 = sector_bytes == 2352;
    receipt.mode1_2048 = sector_bytes == 2048;
    receipt.raw_trace_preparation_allowed = receipt.cue_consumed &&
        receipt.mode1_2352 && theron_v1_track02_expected_raw_index01(variant) != 0u;
    receipt.variant = variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", md5);
    receipt.cue_index01_sector = index01_sector;
    receipt.payload_bytes = payload_bytes;
    receipt.sector_count = payload_bytes / (size_t)sector_bytes;
    receipt.first_user_data_offset = receipt.mode1_2352 ?
        (size_t)payload_index01_sector * 2352u + THERON_TRACK02_RAW_USER_DATA_OFFSET :
        (size_t)payload_index01_sector * 2048u;
    receipt.logical_user_data_window_bytes =
        (receipt.sector_count - (size_t)payload_index01_sector) * 2048u;
    *out = receipt;
    return 1;
}

int theron_v1_track02_raw_media_intake_prepare_trace_input(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    Theron_V1Track02RawTraceMediaInput *out) {
    Theron_V1Track02RawTraceMediaInput input = {0};

    if (!out) return 0;
    *out = input;
    if (!intake || intake->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !intake->cue_consumed || !intake->mode1_2352 ||
        !intake->raw_trace_preparation_allowed ||
        intake->variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        intake->cue_index01_sector !=
            theron_v1_track02_expected_raw_index01(intake->variant) ||
        !intake->track02_md5[0] || !intake->payload_path[0] ||
        intake->logical_user_data_window_bytes < 2048u) {
        return 0;
    }
    input.valid = 1;
    input.variant = intake->variant;
    snprintf(input.track02_md5, sizeof(input.track02_md5), "%s",
             intake->track02_md5);
    snprintf(input.payload_path, sizeof(input.payload_path), "%s",
             intake->payload_path);
    input.cue_index01_sector = intake->cue_index01_sector;
    input.first_user_data_offset = intake->first_user_data_offset;
    input.logical_user_data_window_bytes = intake->logical_user_data_window_bytes;
    *out = input;
    return 1;
}
