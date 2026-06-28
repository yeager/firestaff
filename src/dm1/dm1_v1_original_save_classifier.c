#include "dm1_v1_original_save_classifier.h"
#include "dm1_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#define DM1OS_PATH_SEP '\\'
#else
#define DM1OS_PATH_SEP '/'
#endif

#define DM1OS_HALF_WORDS 128u
#define DM1OS_DM_KEY_INDEX 10u
#define DM1OS_FORMAT_DM_ATARI_ST 1u
#define DM1OS_FORMAT_COMPAT_AMIGA_2X 2u
#define DM1OS_FORMAT_APPLE_IIGS 3u
#define DM1OS_FORMAT_AMIGA_36_PC 5u

typedef enum {
    DM1OS_ENDIAN_LE = 0,
    DM1OS_ENDIAN_BE = 1
} DM1OSEndian;

static const char *candidate_name(int index) {
    static const char *names[DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT] = {
        "DMSAVE.DAT",
        "DMSAVE.BAK",
        "DMGAME.DAT",
        "DMGAME.BAK"
    };
    if (index < 0 || index >= (int)DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT) {
        return NULL;
    }
    return names[index];
}

static int file_exists_regular(const char *path, uint64_t *out_size) {
    struct stat st;
    if (out_size) *out_size = 0;
    if (!path || !path[0]) return 0;
    if (stat(path, &st) != 0) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    if (out_size) *out_size = (uint64_t)st.st_size;
    return 1;
}

static uint16_t rd16(const uint8_t *p, DM1OSEndian endian) {
    if (endian == DM1OS_ENDIAN_BE) {
        return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
    }
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd32(const uint8_t *p, DM1OSEndian endian) {
    if (endian == DM1OS_ENDIAN_BE) {
        return ((uint32_t)p[0] << 24) |
               ((uint32_t)p[1] << 16) |
               ((uint32_t)p[2] << 8) |
               (uint32_t)p[3];
    }
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void wr16(uint8_t *p, uint16_t value, DM1OSEndian endian) {
    if (endian == DM1OS_ENDIAN_BE) {
        p[0] = (uint8_t)((value >> 8) & 0xffu);
        p[1] = (uint8_t)(value & 0xffu);
    } else {
        p[0] = (uint8_t)(value & 0xffu);
        p[1] = (uint8_t)((value >> 8) & 0xffu);
    }
}

static uint16_t header_first_half_checksum(const uint8_t *bytes,
                                           DM1OSEndian endian) {
    uint16_t acc = 0;
    for (size_t i = 0; i < 32u; i++) {
        acc = (uint16_t)(acc + rd16(bytes + ((i * 8u) + 0u), endian));
        acc = (uint16_t)(acc ^ rd16(bytes + ((i * 8u) + 2u), endian));
        acc = (uint16_t)(acc - rd16(bytes + ((i * 8u) + 4u), endian));
        acc = (uint16_t)(acc ^ rd16(bytes + ((i * 8u) + 6u), endian));
    }
    return acc;
}

static uint16_t obfuscate_and_checksum_words(uint8_t *bytes,
                                             uint16_t key,
                                             size_t word_count,
                                             DM1OSEndian endian) {
    uint16_t checksum = key;
    uint16_t rolling_key = key;

    for (size_t i = 0; i < word_count; i++) {
        uint8_t *word = bytes + (i * 2u);
        uint16_t value = rd16(word, endian);
        checksum = (uint16_t)(checksum + value);
        value = (uint16_t)(value ^ rolling_key);
        wr16(word, value, endian);
        checksum = (uint16_t)(checksum + value);
        rolling_key = (uint16_t)(rolling_key + (uint16_t)word_count);
    }
    return checksum;
}

static uint16_t second_half_sum(const uint8_t *bytes, DM1OSEndian endian) {
    uint16_t sum = 0;
    for (size_t i = 0; i < DM1OS_HALF_WORDS; i++) {
        sum = (uint16_t)(sum + rd16(bytes + (i * 2u), endian));
    }
    return sum;
}

static uint32_t rolling_checksum32(const uint8_t *bytes, size_t size) {
    uint32_t sum = 0xD1515A7Eu;
    for (size_t i = 0; i < size; i++) {
        sum = (sum * 33u) ^ bytes[i];
    }
    return sum;
}

static int format_is_known(uint16_t format_id) {
    return format_id == DM1OS_FORMAT_DM_ATARI_ST ||
           format_id == DM1OS_FORMAT_COMPAT_AMIGA_2X ||
           format_id == DM1OS_FORMAT_APPLE_IIGS ||
           format_id == DM1OS_FORMAT_AMIGA_36_PC;
}

static void set_reason(DM1OriginalSaveClassifyResult *out, const char *reason) {
    size_t n;
    if (!out) return;
    out->reason[0] = '\0';
    if (!reason) return;
    n = strlen(reason);
    if (n >= sizeof(out->reason)) n = sizeof(out->reason) - 1u;
    memcpy(out->reason, reason, n);
    out->reason[n] = '\0';
}

static int classify_original_header_with_endian(
    const uint8_t *bytes,
    size_t size,
    DM1OSEndian endian,
    DM1OriginalSaveClassifyResult *out) {
    uint8_t header[DM1_ORIGINAL_SAVE_HEADER_BYTES];
    uint16_t expected;
    uint16_t actual;
    uint16_t key;
    uint16_t format_id;
    uint16_t useless;
    uint32_t game_id;
    uint16_t platform;
    uint16_t dungeon_id;
    uint16_t key_count = 0;
    uint16_t checksum_count = 0;

    if (!bytes || size < DM1_ORIGINAL_SAVE_HEADER_BYTES || !out) return 0;

    memcpy(header, bytes, sizeof(header));
    expected = header_first_half_checksum(header, endian);
    key = rd16(header + (DM1OS_DM_KEY_INDEX * 2u), endian);

    /* ReDMCSB SAVEHEAD.C F0429 lines 42-54 deobfuscates the last 256
     * bytes with Noise[10], then compares the sum of those 128 words
     * to the checksum carried by the first half. */
    obfuscate_and_checksum_words(header + 256u, key, DM1OS_HALF_WORDS, endian);
    actual = second_half_sum(header + 256u, endian);
    if (expected != actual) {
        out->header_expected_checksum = expected;
        out->header_actual_checksum = actual;
        out->header_key = key;
        return 0;
    }

    useless = (uint16_t)header[298u];
    format_id = (uint16_t)header[299u];
    game_id = rd32(header + 306u, endian);
    platform = rd16(header + 374u, endian);
    dungeon_id = rd16(header + 376u, endian);

    for (size_t i = 0; i < 16u; i++) {
        if (rd16(header + 310u + (i * 2u), endian) != 0u) key_count++;
        if (rd16(header + 342u + (i * 2u), endian) != 0u) checksum_count++;
    }

    out->shape = (format_id == DM1OS_FORMAT_DM_ATARI_ST)
        ? DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1
        : DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY;
    out->readiness = DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY;
    out->header_key = key;
    out->format_id = format_id;
    out->useless = useless;
    out->game_id = game_id;
    out->save_and_play = rd16(header + 304u, endian) ? 1u : 0u;
    out->platform = platform;
    out->dungeon_id = dungeon_id;
    out->save_part_key_count_nonzero = key_count;
    out->save_part_checksum_count_nonzero = checksum_count;
    out->header_expected_checksum = expected;
    out->header_actual_checksum = actual;
    out->header_checksum_ok = 1;
    out->import_blocked_until_roundtrip = 1;

    if (!format_is_known(format_id)) {
        out->shape = DM1_ORIGINAL_SAVE_SHAPE_REJECTED;
        out->readiness = DM1_ORIGINAL_SAVE_READY_REJECTED;
        set_reason(out, "header checksum ok but unknown original format id");
        return 1;
    }
    if (format_id != DM1OS_FORMAT_DM_ATARI_ST) {
        set_reason(out, "recognized ReDMCSB-compatible save family, not DM1 Atari/ST-format header");
        return 1;
    }

    set_reason(out, "recognized DM1 original save header; importer blocked until byte round-trip");
    return 1;
}

int dm1_v1_original_save_default_root(char out_root[DM1_ORIGINAL_SAVE_PATH_MAX]) {
    const char *env_root;
    const char *home;

    if (!out_root) return 0;

    env_root = getenv("FIRESTAFF_DM1_ORIGINAL_SAVE_DIR");
    if (env_root && env_root[0]) {
        size_t n = strlen(env_root);
        if (n >= DM1_ORIGINAL_SAVE_PATH_MAX) n = DM1_ORIGINAL_SAVE_PATH_MAX - 1u;
        memcpy(out_root, env_root, n);
        out_root[n] = '\0';
        return 1;
    }

    home = getenv("HOME");
    if (home && home[0]) {
        int n = snprintf(out_root, DM1_ORIGINAL_SAVE_PATH_MAX,
                         "%s%c.firestaff%cdata%cdm1%csave",
                         home,
                         DM1OS_PATH_SEP,
                         DM1OS_PATH_SEP,
                         DM1OS_PATH_SEP,
                         DM1OS_PATH_SEP);
        if (n > 0 && (size_t)n < DM1_ORIGINAL_SAVE_PATH_MAX) return 1;
    }

    snprintf(out_root, DM1_ORIGINAL_SAVE_PATH_MAX, ".%cdm1-save", DM1OS_PATH_SEP);
    return 1;
}

int dm1_v1_original_save_candidate_path(
    const char *root,
    int candidate_index,
    char out_path[DM1_ORIGINAL_SAVE_PATH_MAX]) {
    const char *name = candidate_name(candidate_index);
    size_t root_len;
    int n;

    if (!out_path) return 0;
    out_path[0] = '\0';
    if (!root || !root[0] || !name) return 0;

    root_len = strlen(root);
    while (root_len > 0u &&
           (root[root_len - 1u] == '/' || root[root_len - 1u] == '\\')) {
        root_len--;
    }
    if (root_len == 0u) return 0;

    n = snprintf(out_path, DM1_ORIGINAL_SAVE_PATH_MAX,
                 "%.*s%c%s", (int)root_len, root, DM1OS_PATH_SEP, name);
    if (n <= 0 || (size_t)n >= DM1_ORIGINAL_SAVE_PATH_MAX) {
        out_path[0] = '\0';
        return 0;
    }
    return 1;
}

int dm1_v1_original_save_classify_bytes(
    const uint8_t *bytes,
    size_t size,
    DM1OriginalSaveClassifyResult *out_result) {
    if (!out_result) return 0;
    memset(out_result, 0, sizeof(*out_result));
    out_result->size_bytes = (uint64_t)size;
    out_result->shape = DM1_ORIGINAL_SAVE_SHAPE_REJECTED;
    out_result->readiness = DM1_ORIGINAL_SAVE_READY_REJECTED;
    out_result->import_blocked_until_roundtrip = 1;

    if (!bytes) {
        set_reason(out_result, "null byte buffer");
        return 0;
    }
    if (size == 0u) {
        out_result->shape = DM1_ORIGINAL_SAVE_SHAPE_ABSENT;
        out_result->readiness = DM1_ORIGINAL_SAVE_READY_ABSENT;
        set_reason(out_result, "absent save bytes");
        return 1;
    }

    out_result->prefix_checksum32 =
        rolling_checksum32(bytes, size < 1024u ? size : 1024u);

    if (size >= 8u && memcmp(bytes, DM1_SAVE_MAGIC, 8u) == 0) {
        out_result->shape = DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE;
        out_result->readiness = DM1_ORIGINAL_SAVE_READY_NOT_ORIGINAL;
        out_result->import_blocked_until_roundtrip = 0;
        set_reason(out_result, "Firestaff-native DM1 save, not an original DMSAVE shape");
        return 1;
    }

    if (size < DM1_ORIGINAL_SAVE_MIN_BYTES) {
        set_reason(out_result, "too small for 512-byte original save header");
        return 1;
    }

    if (classify_original_header_with_endian(bytes, size, DM1OS_ENDIAN_LE, out_result)) {
        return 1;
    }
    if (classify_original_header_with_endian(bytes, size, DM1OS_ENDIAN_BE, out_result)) {
        return 1;
    }

    out_result->header_checksum_ok = 0;
    out_result->import_blocked_until_roundtrip = 1;
    set_reason(out_result, "not a valid ReDMCSB DM_SAVE_HEADER checksum shape");
    return 1;
}

int dm1_v1_original_save_classify_file(
    const char *path,
    DM1OriginalSaveClassifyResult *out_result) {
    FILE *fp;
    uint64_t file_size = 0;
    uint8_t *buf;
    size_t got;
    int rc;

    if (!out_result) return 0;
    memset(out_result, 0, sizeof(*out_result));
    out_result->shape = DM1_ORIGINAL_SAVE_SHAPE_ABSENT;
    out_result->readiness = DM1_ORIGINAL_SAVE_READY_ABSENT;
    set_reason(out_result, "absent");

    if (!path || !path[0]) return 0;
    if (!file_exists_regular(path, &file_size)) {
        return 1;
    }
    if (file_size == 0u || file_size > (16u * 1024u * 1024u)) {
        out_result->size_bytes = file_size;
        out_result->shape = DM1_ORIGINAL_SAVE_SHAPE_REJECTED;
        out_result->readiness = DM1_ORIGINAL_SAVE_READY_REJECTED;
        out_result->import_blocked_until_roundtrip = 1;
        set_reason(out_result, "file size outside bounded classifier range");
        return 1;
    }

    fp = fopen(path, "rb");
    if (!fp) return 0;
    buf = (uint8_t *)malloc((size_t)file_size);
    if (!buf) {
        fclose(fp);
        return 0;
    }
    got = fread(buf, 1, (size_t)file_size, fp);
    fclose(fp);
    if (got != (size_t)file_size) {
        free(buf);
        return 0;
    }

    rc = dm1_v1_original_save_classify_bytes(buf, got, out_result);
    free(buf);
    return rc;
}

int dm1_v1_original_save_classify_root(
    const char *root,
    DM1OriginalSaveManifest *out_manifest) {
    if (!out_manifest) return 0;
    memset(out_manifest, 0, sizeof(*out_manifest));
    out_manifest->candidate_count = (int)DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT;

    if (root && root[0]) {
        size_t n = strlen(root);
        if (n >= DM1_ORIGINAL_SAVE_PATH_MAX) n = DM1_ORIGINAL_SAVE_PATH_MAX - 1u;
        memcpy(out_manifest->root, root, n);
        out_manifest->root[n] = '\0';
    } else if (!dm1_v1_original_save_default_root(out_manifest->root)) {
        return 0;
    }

    for (int i = 0; i < out_manifest->candidate_count; i++) {
        DM1OriginalSaveClassifyResult *result = &out_manifest->results[i];
        if (!dm1_v1_original_save_candidate_path(out_manifest->root, i,
                                                 out_manifest->paths[i])) {
            continue;
        }
        if (!dm1_v1_original_save_classify_file(out_manifest->paths[i], result)) {
            return 0;
        }
        if (result->shape != DM1_ORIGINAL_SAVE_SHAPE_ABSENT) {
            out_manifest->present_count++;
        }
        if (result->readiness == DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY) {
            out_manifest->classified_count++;
        }
        if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1) {
            out_manifest->original_dm1_count++;
        }
        if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE) {
            out_manifest->firestaff_native_count++;
        }
    }

    return 1;
}

const char *dm1_v1_original_save_shape_name(DM1OriginalSaveShape shape) {
    switch (shape) {
        case DM1_ORIGINAL_SAVE_SHAPE_ABSENT: return "ABSENT";
        case DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE: return "FIRESTAFF_NATIVE";
        case DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1: return "ORIGINAL_DM1";
        case DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY: return "ORIGINAL_COMPAT_FAMILY";
        case DM1_ORIGINAL_SAVE_SHAPE_REJECTED: return "REJECTED";
        default: return "UNKNOWN";
    }
}

const char *dm1_v1_original_save_readiness_name(DM1OriginalSaveReadiness readiness) {
    switch (readiness) {
        case DM1_ORIGINAL_SAVE_READY_ABSENT: return "ABSENT";
        case DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY: return "CLASSIFIED_HEADER_ONLY";
        case DM1_ORIGINAL_SAVE_READY_NOT_ORIGINAL: return "NOT_ORIGINAL";
        case DM1_ORIGINAL_SAVE_READY_REJECTED: return "REJECTED";
        default: return "UNKNOWN";
    }
}

const char *dm1_v1_original_save_source_evidence(void) {
    return "ReDMCSB DEFS.H:468-480/500-508; SAVEHEAD.C:30-54/76-104; "
           "READWRIT.C:191-209; LOADSAVE.C:1590-1628/2665-2722; "
           "FILENAME.C:10-13/52-55";
}
