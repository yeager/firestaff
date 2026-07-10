#include "dm1_v1_original_save_classifier.h"
#include "dm1_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#define DM1OS_PATH_SEP '\\'
#else
#include <dirent.h>
#define DM1OS_PATH_SEP '/'
#endif

#define DM1OS_HALF_WORDS 128u
#define DM1OS_DM_KEY_INDEX 10u
#define DM1OS_FORMAT_DM_ATARI_ST 1u
#define DM1OS_FORMAT_COMPAT_AMIGA_2X 2u
#define DM1OS_FORMAT_APPLE_IIGS 3u
#define DM1OS_FORMAT_AMIGA_36_PC 5u
#define DM1OS_PLATFORM_PC 9u
#define DM1OS_DUNGEON_DM 10u
#define DM1OS_SAVE_PART_COUNT 5u
#define DM1OS_CORPUS_MAX_DEPTH 4

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

static void copy_path(char *dst, size_t cap, const char *src) {
    size_t n;
    if (!dst || cap == 0u) return;
    dst[0] = '\0';
    if (!src) return;
    n = strlen(src);
    if (n >= cap) n = cap - 1u;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void dm1_original_save_corpus_count_result(
    DM1OriginalSaveCorpusManifest *manifest,
    const DM1OriginalSaveClassifyResult *result) {
    if (!manifest || !result) return;
    if (result->shape != DM1_ORIGINAL_SAVE_SHAPE_ABSENT) {
        manifest->present_count++;
    }
    if (result->readiness == DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY) {
        manifest->classified_count++;
    }
    if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1) {
        manifest->original_dm1_count++;
    }
    if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34) {
        manifest->original_dm1_count++;
        manifest->original_dm1_pc34_count++;
    }
    if (result->pc34_importer_candidate) {
        manifest->pc34_importer_candidate_count++;
    }
    if (result->pc34_loader_part_envelope_candidate) {
        manifest->pc34_loader_part_envelope_count++;
    }
    if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE) {
        manifest->firestaff_native_count++;
    }
    if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_REJECTED) {
        manifest->rejected_count++;
    }
}

static int classify_pc34_loader_part_envelope(
    const uint8_t *bytes,
    size_t size,
    const uint8_t header[DM1_ORIGINAL_SAVE_HEADER_BYTES],
    DM1OSEndian endian,
    DM1OriginalSaveClassifyResult *out)
{
    size_t cursor = DM1_ORIGINAL_SAVE_HEADER_BYTES;
    uint32_t payload_bytes = 0u;
    uint16_t ok_count = 0u;

    if (!bytes || !header || !out) return 0;

    for (size_t part = 0u; part < DM1OS_SAVE_PART_COUNT; ++part) {
        uint16_t byte_count;
        uint16_t key;
        uint16_t expected;
        uint16_t actual;
        uint8_t *part_bytes;

        if (cursor + 2u > size) break;
        byte_count = rd16(bytes + cursor, endian);
        cursor += 2u;
        if ((byte_count & 1u) != 0u ||
            byte_count > 0xfffeu ||
            cursor + (size_t)byte_count > size) {
            break;
        }
        part_bytes = NULL;
        if (byte_count > 0u) {
            part_bytes = (uint8_t *)malloc((size_t)byte_count);
            if (!part_bytes) break;
            memcpy(part_bytes, bytes + cursor, (size_t)byte_count);
        }
        key = rd16(header + 310u + (part * 2u), endian);
        expected = rd16(header + 342u + (part * 2u), endian);
        actual = obfuscate_and_checksum_words(
            part_bytes ? part_bytes : (uint8_t *)"",
            key,
            (size_t)byte_count / 2u,
            endian);
        free(part_bytes);
        if (actual != expected) {
            break;
        }
        ok_count++;
        payload_bytes += byte_count;
        cursor += (size_t)byte_count;
    }

    out->save_part_loader_envelope_ok_count = ok_count;
    out->save_part_loader_envelope_payload_bytes = payload_bytes;
    out->pc34_loader_part_envelope_candidate =
        ok_count == (uint16_t)DM1OS_SAVE_PART_COUNT;
    return out->pc34_loader_part_envelope_candidate;
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
    out->pc34_importer_candidate = 0;

    if (!format_is_known(format_id)) {
        out->shape = DM1_ORIGINAL_SAVE_SHAPE_REJECTED;
        out->readiness = DM1_ORIGINAL_SAVE_READY_REJECTED;
        set_reason(out, "header checksum ok but unknown original format id");
        return 1;
    }
    if (format_id != DM1OS_FORMAT_DM_ATARI_ST) {
        /* ReDMCSB LOADSAVE.C F0433 stamps I34E/I34M saves as
         * C5_FORMAT_DM_AMIGA_36_PC_CSB_AMIGA_PC98_X68000_FM_TOWNS;
         * F0435 accepts that format on C9_PLATFORM_PC. Firestaff's
         * F0796 PC34 importer is the bounded handoff candidate for
         * exactly that DM1 PC 3.4 envelope, while real original-byte
         * round-trip proof remains a separate gate. */
        if (format_id == DM1OS_FORMAT_AMIGA_36_PC &&
            platform == DM1OS_PLATFORM_PC &&
            dungeon_id == DM1OS_DUNGEON_DM) {
            out->shape = DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34;
            out->pc34_importer_candidate = 1;
            /* ReDMCSB CEDTINCD.C F7051 lines ~224-266 accepts the DM/PC
             * header, then F7057 reads each save part by length, key and
             * checksum.  This bounded classifier proves that envelope before
             * startup receipts count a real user corpus as save-part backed. */
            (void)classify_pc34_loader_part_envelope(
                bytes, size, header, endian, out);
            set_reason(out,
                       out->pc34_loader_part_envelope_candidate
                           ? "recognized DM1 PC 3.4 save header and F7057 save-part envelope"
                           : "recognized DM1 PC 3.4 save header; save-part envelope not proven");
            return 1;
        }
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
        if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34) {
            out_manifest->original_dm1_count++;
            out_manifest->original_dm1_pc34_count++;
        }
        if (result->pc34_importer_candidate) {
            out_manifest->pc34_importer_candidate_count++;
        }
        if (result->pc34_loader_part_envelope_candidate) {
            out_manifest->pc34_loader_part_envelope_count++;
        }
        if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE) {
            out_manifest->firestaff_native_count++;
        }
    }

    return 1;
}

static int corpus_add_classified_file(
    DM1OriginalSaveCorpusManifest *manifest,
    const char *path) {
    DM1OriginalSaveClassifyResult *result;
    int slot;

    if (!manifest || !path || !path[0]) return 0;
    manifest->scanned_file_count++;
    if (manifest->present_count >=
        (int)DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP) {
        manifest->truncated_count++;
        return 1;
    }
    slot = manifest->present_count;
    result = &manifest->results[slot];
    if (!dm1_v1_original_save_classify_file(path, result)) {
        return 0;
    }
    if (result->shape == DM1_ORIGINAL_SAVE_SHAPE_ABSENT) {
        return 1;
    }
    copy_path(manifest->paths[slot],
              sizeof(manifest->paths[slot]),
              path);
    dm1_original_save_corpus_count_result(manifest, result);
    return 1;
}

#if !defined(_WIN32) && !defined(_WIN64)
static int corpus_scan_directory_recursive(
    DM1OriginalSaveCorpusManifest *manifest,
    const char *dir_path,
    int depth) {
    DIR *dir;
    struct dirent *entry;

    if (!manifest || !dir_path || depth > DM1OS_CORPUS_MAX_DEPTH) {
        return 1;
    }

    dir = opendir(dir_path);
    if (!dir) {
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        char path[DM1_ORIGINAL_SAVE_PATH_MAX];
        struct stat st;
        int n;
        if (entry->d_name[0] == '.') {
            continue;
        }
        n = snprintf(path, sizeof(path), "%s%c%s",
                     dir_path, DM1OS_PATH_SEP, entry->d_name);
        if (n <= 0 || (size_t)n >= sizeof(path)) {
            manifest->truncated_count++;
            continue;
        }
        if (stat(path, &st) != 0) {
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            if (!corpus_scan_directory_recursive(manifest, path, depth + 1)) {
                closedir(dir);
                return 0;
            }
            continue;
        }
        if (!S_ISREG(st.st_mode)) {
            continue;
        }
        /*
         * ReDMCSB SAVEHEAD.C F0429/F0430 lines ~30-104 only identifies the
         * obfuscated header; CEDTINCD.C F7051/F7057 lines ~226-294 then
         * proves the loader route by reading the five checksum-protected save
         * parts.  The corpus scan is therefore recursive by user layout, but
         * still byte-shaped: filename is never enough to promote a DM1 save.
         */
        if (!corpus_add_classified_file(manifest, path)) {
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
    return 1;
}
#endif

int dm1_v1_original_save_classify_corpus_root(
    const char *root,
    DM1OriginalSaveCorpusManifest *out_manifest) {
    if (!out_manifest) return 0;
    memset(out_manifest, 0, sizeof(*out_manifest));
    out_manifest->candidate_capacity =
        (int)DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP;

    if (root && root[0]) {
        copy_path(out_manifest->root, sizeof(out_manifest->root), root);
    } else if (!dm1_v1_original_save_default_root(out_manifest->root)) {
        return 0;
    }

#if defined(_WIN32) || defined(_WIN64)
    {
        DM1OriginalSaveManifest fixed;
        if (!dm1_v1_original_save_classify_root(out_manifest->root, &fixed)) {
            return 0;
        }
        for (int i = 0; i < fixed.candidate_count; ++i) {
            if (fixed.results[i].shape == DM1_ORIGINAL_SAVE_SHAPE_ABSENT) {
                continue;
            }
            if (out_manifest->present_count >=
                (int)DM1_ORIGINAL_SAVE_CORPUS_CANDIDATE_CAP) {
                out_manifest->truncated_count++;
                break;
            }
            out_manifest->results[out_manifest->present_count] =
                fixed.results[i];
            copy_path(out_manifest->paths[out_manifest->present_count],
                      sizeof(out_manifest->paths[out_manifest->present_count]),
                      fixed.paths[i]);
            out_manifest->scanned_file_count++;
            dm1_original_save_corpus_count_result(
                out_manifest,
                &out_manifest->results[out_manifest->present_count]);
        }
        return 1;
    }
#else
    return corpus_scan_directory_recursive(out_manifest, out_manifest->root, 0);
#endif
}

const char *dm1_v1_original_save_shape_name(DM1OriginalSaveShape shape) {
    switch (shape) {
        case DM1_ORIGINAL_SAVE_SHAPE_ABSENT: return "ABSENT";
        case DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE: return "FIRESTAFF_NATIVE";
        case DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1: return "ORIGINAL_DM1";
        case DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY: return "ORIGINAL_COMPAT_FAMILY";
        case DM1_ORIGINAL_SAVE_SHAPE_REJECTED: return "REJECTED";
        case DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34: return "ORIGINAL_DM1_PC34";
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
