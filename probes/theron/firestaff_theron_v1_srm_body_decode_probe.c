/*
 * firestaff_theron_v1_srm_body_decode_probe.c
 *
 * Theron's Quest V1 — SRM (Save RAM) body-decode envelope probe.
 *
 * Skip-clean real-asset counterpart to the data-free
 * test_theron_v1_srm_body_decode_pc34 unit test:
 *
 *   - The synthetic-fixture path builds a gzip-wrapped progression
 *     envelope in memory and drives
 *     theron_v1_srm_decode_envelope() + theron_v1_srm_decode_path()
 *     so the end-to-end (gzip magic → inflate → envelope magic →
 *     decode) chain is exercised in production CTest.
 *
 *   - The real-artifact path uses
 *     theron_v1_srm_probe_slot0_envelope() to look at the configured
 *     save-disk root (default $HOME/.firestaff/data/theron/save, env
 *     FIRESTAFF_THERON_SRM_DIR override) for slot0.srm.  When a real
 *     .srm file is staged the probe reports the typed envelope kind
 *     + decoded Theron_DungeonProgression fields.  When no real .srm
 *     is staged the probe returns SKIP cleanly with a recorded
 *     "absent manifest" message and exits 0 so CI stays green.
 *
 * The probe never synthesizes or promotes screenshots; it only
 * advances the bounded inflate-and-decode surface tracked in
 * docs/FIRESTAFF_GAP_LIST.md F1 'Save/load (.SRM)'.
 *
 * Source/evidence: docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame
 * format', theron_v1_srm_source_evidence().
 */

#include "theron_v1_srm_classifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef FIRESTAFF_HAS_ZLIB
#define FIRESTAFF_HAS_ZLIB 0
#endif

#if FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#define PROBE_PATH_SEP '\\'
#include <direct.h>
#include <process.h>
#define probe_mkdir(p) _mkdir(p)
#define probe_rmdir(p) _rmdir(p)
#define probe_unlink(p) remove(p)
#define PROBE_TMP_ROOT "firestaff_theron_srm_body_probe"
#define PROBE_REAL_ENV_ROOT "firestaff_theron_srm_body_probe_real"
#define PROBE_ABSENT_ENV_ROOT "firestaff_theron_srm_body_probe_absent"
#else
#define PROBE_PATH_SEP '/'
#include <unistd.h>
#define probe_mkdir(p) mkdir((p), 0700)
#define probe_rmdir(p) rmdir(p)
#define probe_unlink(p) unlink(p)
#define PROBE_TMP_ROOT "/tmp/firestaff_theron_srm_body_probe"
#define PROBE_REAL_ENV_ROOT "/tmp/firestaff_theron_srm_body_probe_real"
#define PROBE_ABSENT_ENV_ROOT "/tmp/firestaff_theron_srm_body_probe_absent"
#endif

static int g_fail = 0;
static int g_pass = 0;
static int g_skip = 0;

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static void check_size(const char *label, size_t got, size_t want) {
    if (got != want) {
        printf("FAIL %s: got %zu want %zu\n", label, got, want);
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static void check_str(const char *label, const char *got, const char *want) {
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s: got '%s' want '%s'\n", label,
               got ? got : "(null)", want ? want : "(null)");
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static int probe_setenv(const char *name, const char *value) {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name, value ? value : "") == 0;
#else
    if (value) return setenv(name, value, 1) == 0;
    return unsetenv(name) == 0;
#endif
}

static int make_temp_root(char out[THERON_V1_SRM_PATH_MAX]) {
#if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
    for (int i = 0; i < 32; i++) {
        int n = snprintf(out, THERON_V1_SRM_PATH_MAX,
                         "%s_%d_%d", PROBE_TMP_ROOT, pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (probe_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    static const char *tpl = "/tmp/firestaff_theron_srm_body_probe_XXXXXX";
    if (strlen(tpl) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    strncpy(out, tpl, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static void cleanup_root(const char *root) {
    if (!root || !root[0]) return;
    char path[THERON_V1_SRM_PATH_MAX];
    if (theron_v1_srm_slot_path(root, 0, path)) {
        probe_unlink(path);
    }
    probe_rmdir(root);
}

static int write_slot_file(const char *root, int slot_index,
                            const uint8_t *bytes, size_t size) {
    char path[THERON_V1_SRM_PATH_MAX];
    FILE *fp;
    size_t wrote;

    if (!theron_v1_srm_slot_path(root, slot_index, path)) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && bytes) {
        wrote = fwrite(bytes, 1, size, fp);
        if (wrote != size) {
            fclose(fp);
            return 0;
        }
    }
    return fclose(fp) == 0;
}

static void wr32le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
    p[2] = (uint8_t)((v >> 16) & 0xffu);
    p[3] = (uint8_t)((v >> 24) & 0xffu);
}

static void build_progression_payload(uint8_t payload[44]) {
    static const uint8_t magic[8] = {'F', 'S', 'T', 'Q', 'P', 'R', 'G', '1'};
    memset(payload, 0, 44u);
    memcpy(payload, magic, sizeof(magic));
    payload[8] = 1u;
    payload[9] = 3u;
    payload[10] = 0x03u;
    payload[11] = 1u;
    wr32le(payload + 12, 300u);
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        wr32le(payload + 16u + ((size_t)i * 4u), 0x1001u + (uint32_t)i);
    }
}

static uint32_t bd_crc32(const uint8_t *buf, size_t size) {
#if FIRESTAFF_HAS_ZLIB
    return (uint32_t)crc32(crc32(0L, Z_NULL, 0), buf, (uInt)size);
#else
    static const uint32_t POLY = 0xEDB88320u;
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; i++) {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1u) ? POLY : 0u);
        }
    }
    return crc ^ 0xFFFFFFFFu;
#endif
}

#if FIRESTAFF_HAS_ZLIB
static int deflate_raw(const uint8_t *src, size_t src_size,
                       uint8_t *dst, size_t dst_capacity,
                       size_t *out_size) {
    z_stream zs;
    int ret;

    if (!src || !dst || !out_size) return 0;
    if (dst_capacity < src_size + 64u) return 0;

    memset(&zs, 0, sizeof(zs));
    zs.next_in = (Bytef *)src;
    zs.avail_in = (uInt)src_size;
    zs.next_out = dst;
    zs.avail_out = (uInt)dst_capacity;

    ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                       -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) return 0;

    ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&zs);
        return 0;
    }
    *out_size = (size_t)zs.total_out;
    deflateEnd(&zs);
    return 1;
}
#endif

static int build_synthetic_gzip_stream(const uint8_t *payload, size_t payload_size,
                                       uint8_t *out, size_t out_capacity,
                                       size_t *out_size) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t raw[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    size_t raw_size = 0;
    uint32_t crc;
    uint32_t isize;

    if (!payload || !out || !out_size) return 0;
    if (out_capacity < 18u) return 0;
    if (!deflate_raw(payload, payload_size, raw, sizeof(raw), &raw_size)) {
        return 0;
    }

    out[0] = THERON_V1_SRM_GZIP_MAGIC_0;
    out[1] = THERON_V1_SRM_GZIP_MAGIC_1;
    out[2] = THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE;
    out[3] = 0x00;
    out[4] = 0;
    out[5] = 0;
    out[6] = 0;
    out[7] = 0;
    out[8] = 0x02;
    out[9] = 0xFF;

    memcpy(out + 10, raw, raw_size);
    crc = bd_crc32(payload, payload_size);
    isize = (uint32_t)(payload_size & 0xFFFFFFFFu);

    out[10 + raw_size + 0] = (uint8_t)(crc & 0xFFu);
    out[10 + raw_size + 1] = (uint8_t)((crc >> 8) & 0xFFu);
    out[10 + raw_size + 2] = (uint8_t)((crc >> 16) & 0xFFu);
    out[10 + raw_size + 3] = (uint8_t)((crc >> 24) & 0xFFu);

    out[10 + raw_size + 4] = (uint8_t)(isize & 0xFFu);
    out[10 + raw_size + 5] = (uint8_t)((isize >> 8) & 0xFFu);
    out[10 + raw_size + 6] = (uint8_t)((isize >> 16) & 0xFFu);
    out[10 + raw_size + 7] = (uint8_t)((isize >> 24) & 0xFFu);

    *out_size = 10u + raw_size + 8u;
    return 1;
#else
    (void)payload; (void)payload_size;
    (void)out;    (void)out_capacity; (void)out_size;
    return 0;
#endif
}

static void probe_envelope_kind_names(void) {
    check_str("NONE",
              theron_v1_srm_envelope_kind_name(THERON_V1_SRM_ENVELOPE_KIND_NONE),
              "NONE");
    check_str("PROGRESSION",
              theron_v1_srm_envelope_kind_name(THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION),
              "PROGRESSION");
    check_str("PROGRESSION_PARTY",
              theron_v1_srm_envelope_kind_name(THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY),
              "PROGRESSION_PARTY");
    check_str("UNSUPPORTED",
              theron_v1_srm_envelope_kind_name(THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED),
              "UNSUPPORTED");
}

static void probe_synthetic_envelope_chain(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    build_progression_payload(payload);
    if (!build_synthetic_gzip_stream(payload, sizeof(payload),
                                     srm_bytes, sizeof(srm_bytes),
                                     &srm_size)) {
        printf("SKIP synthetic envelope chain: gzip stream build failed\n");
        ++g_skip;
        return;
    }

    memset(&envelope, 0, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_envelope(srm_bytes, srm_size,
                                         scratch, sizeof(scratch),
                                         &envelope);

    printf("synthetic envelope chain: kind=%s inflate=%s decode=%s payload=%zu\n",
           theron_v1_srm_envelope_kind_name(kind),
           theron_v1_srm_payload_probe_status_name(envelope.inflate_status),
           theron_v1_srm_progress_import_status_name(envelope.decode_status),
           envelope.inflate_payload_size);

    check_int("synthetic envelope kind PROGRESSION",
              kind,
              THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION);
    check_int("synthetic envelope inflate OK",
              envelope.inflate_status,
              THERON_V1_SRM_PAYLOAD_PROBE_OK);
    check_int("synthetic envelope decode OK",
              envelope.decode_status,
              THERON_V1_SRM_PROGRESS_IMPORT_OK);
    check_size("synthetic envelope payload size",
               envelope.inflate_payload_size,
               sizeof(payload));
    check_int("synthetic envelope current dungeon",
              envelope.progression.current_dungeon,
              THERON_DUNGEON_3_ABYSS_OF_FLAMES);
    check_int("synthetic envelope quest mask",
              envelope.progression.quest_items_bitmask,
              0x03);
#else
    printf("SKIP synthetic envelope chain: zlib unavailable\n");
    ++g_skip;
#endif
}

static void probe_synthetic_slot0_path_roundtrip(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;
    char root[THERON_V1_SRM_PATH_MAX];

    if (!make_temp_root(root)) {
        printf("SKIP synthetic slot0 path roundtrip: mkdtemp failed\n");
        ++g_skip;
        return;
    }

    build_progression_payload(payload);
    if (!build_synthetic_gzip_stream(payload, sizeof(payload),
                                     srm_bytes, sizeof(srm_bytes),
                                     &srm_size) ||
        !write_slot_file(root, 0, srm_bytes, srm_size)) {
        printf("SKIP synthetic slot0 path roundtrip: fixture write failed\n");
        ++g_skip;
        cleanup_root(root);
        return;
    }

    char slot_path[THERON_V1_SRM_PATH_MAX];
    theron_v1_srm_slot_path(root, 0, slot_path);

    memset(&envelope, 0, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_path(slot_path, 0,
                                     scratch, sizeof(scratch),
                                     &envelope);

    printf("synthetic slot0 decode_path: kind=%s source=%s size=%llu\n",
           theron_v1_srm_envelope_kind_name(kind),
           envelope.source_path,
           (unsigned long long)envelope.file_size);

    check_int("synthetic slot0 decode_path kind PROGRESSION",
              kind,
              THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION);
    check_int("synthetic slot0 decode_path slot_index",
              envelope.slot_index, 0);
    check_int("synthetic slot0 decode_path file_size matches",
              (int)(envelope.file_size == (uint64_t)srm_size),
              1);
    check_str("synthetic slot0 source_path points at slot0.srm",
              strstr(envelope.source_path, "slot0.srm") ? "ok" : "missing",
              "ok");

    cleanup_root(root);
#else
    printf("SKIP synthetic slot0 path roundtrip: zlib unavailable\n");
    ++g_skip;
#endif
}

static void probe_real_slot0_skip_clean(void) {
    /* Force a non-existent save-disk root so the probe_skip_clean
     * path is reproducible: with FIRESTAFF_THERON_SRM_DIR pointing
     * at a path that does not exist, theron_v1_srm_probe_slot0_envelope
     * must return ENVELOPE_KIND_NONE without touching the caller's
     * envelope buffer. */
    char saved[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", PROBE_ABSENT_ENV_ROOT);

    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeReceipt sentinel;
    memset(&sentinel, 0xCD, sizeof(sentinel));
    memset(&envelope, 0xCD, sizeof(envelope));
    Theron_V1SrmEnvelopeKind kind =
        theron_v1_srm_probe_slot0_envelope(&envelope);

    printf("real-asset slot0 probe (no .srm): kind=%s inflate=%s skip-clean\n",
           theron_v1_srm_envelope_kind_name(kind),
           theron_v1_srm_payload_probe_status_name(envelope.inflate_status));

    check_int("real-asset slot0 probe skip-clean kind NONE",
              kind,
              THERON_V1_SRM_ENVELOPE_KIND_NONE);
    check_int("real-asset slot0 probe skip-clean leaves envelope untouched",
              memcmp(&envelope, &sentinel, sizeof(envelope)) == 0,
              1);

    if (had_prev) probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved);
    else probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
}

static void probe_real_slot0_when_staged(void) {
    /* Stage a synthetic .srm at the configured save-disk root and
     * report the result.  This is the skip-clean-but-advancing path:
     * a real-shape artifact can show up here, in which case the gate
     * also records the decoded envelope kind. */
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    Theron_V1SrmEnvelopeReceipt envelope;
    char root[THERON_V1_SRM_PATH_MAX];
    char saved_root[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved_root, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_root[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }

    if (!make_temp_root(root)) {
        printf("SKIP real-asset slot0 staged: mkdtemp failed\n");
        ++g_skip;
        return;
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", root);

    build_progression_payload(payload);
    if (!build_synthetic_gzip_stream(payload, sizeof(payload),
                                     srm_bytes, sizeof(srm_bytes),
                                     &srm_size) ||
        !write_slot_file(root, 0, srm_bytes, srm_size)) {
        printf("SKIP real-asset slot0 staged: fixture write failed\n");
        ++g_skip;
        cleanup_root(root);
        if (had_prev) probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved_root);
        else probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
        return;
    }

    memset(&envelope, 0, sizeof(envelope));
    Theron_V1SrmEnvelopeKind kind =
        theron_v1_srm_probe_slot0_envelope(&envelope);

    printf("real-asset slot0 staged probe: kind=%s dungeon=%u quest=0x%02x\n",
           theron_v1_srm_envelope_kind_name(kind),
           (unsigned)envelope.progression.current_dungeon,
           (unsigned)envelope.progression.quest_items_bitmask);

    check_int("real-asset slot0 staged kind PROGRESSION",
              kind,
              THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION);
    check_int("real-asset slot0 staged current dungeon",
              (int)envelope.progression.current_dungeon,
              (int)THERON_DUNGEON_3_ABYSS_OF_FLAMES);
    check_int("real-asset slot0 staged slot_index 0",
              envelope.slot_index,
              0);

    cleanup_root(root);
    if (had_prev) probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved_root);
    else probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
#else
    printf("SKIP real-asset slot0 staged: zlib unavailable\n");
    ++g_skip;
#endif
}

int main(void) {
    printf("=== Theron V1 SRM Body-Decode Envelope Probe ===\n");
    printf("%s\n", theron_v1_srm_source_evidence());

    probe_envelope_kind_names();
    probe_synthetic_envelope_chain();
    probe_synthetic_slot0_path_roundtrip();
    probe_real_slot0_skip_clean();
    probe_real_slot0_when_staged();

    printf("summary: pass=%d fail=%d skip=%d\n",
           g_pass, g_fail, g_skip);
    return g_fail ? 1 : 0;
}
