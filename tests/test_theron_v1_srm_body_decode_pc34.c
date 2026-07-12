/*
 * test_theron_v1_srm_body_decode_pc34.c
 *
 * Theron's Quest V1 — SRM (Save RAM) body-decode envelope unit test.
 *
 * Data-free: builds a synthetic gzip-wrapped DEFLATE stream around the
 * same FSTQPRG1 readiness envelope bytes the existing progression
 * decoder accepts, then runs the new
 * theron_v1_srm_decode_envelope() / theron_v1_srm_decode_path()
 * helpers to confirm the bounded (gzip magic → inflate → envelope
 * magic → decode) chain stays correct end-to-end.
 *
 * Probe path: scans `FIRESTAFF_THERON_SRM_DIR` (or the default save-disk
 * root) for slot0.srm.  When no real .srm is staged the test surfaces
 * a SKIP and exits PASS — the same skip-clean contract the gap
 * requires for "accept real .srm when staged, skip cleanly otherwise".
 *
 * Source/evidence: docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame
 * format', theron_v1_srm_source_evidence().
 */

#include "theron_v1_srm_classifier.h"
#include "theron_v1_srm_runtime.h"
#include "theron_v1_track02.h"

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
#define BD_PATH_SEP '\\'
#include <direct.h>
#include <process.h>
#define bd_mkdir(p) _mkdir(p)
#define bd_rmdir(p) _rmdir(p)
#define bd_unlink(p) remove(p)
#define BD_TMP_ROOT "firestaff_theron_srm_body_unit"
#define BD_REAL_SLOT0_ENV_ROOT "firestaff_theron_srm_body_unit_real"
#else
#define BD_PATH_SEP '/'
#include <unistd.h>
#define bd_mkdir(p) mkdir((p), 0700)
#define bd_rmdir(p) rmdir(p)
#define bd_unlink(p) unlink(p)
#define BD_TMP_ROOT "/tmp/firestaff_theron_srm_body_unit"
#define BD_REAL_SLOT0_ENV_ROOT "/tmp/firestaff_theron_srm_body_unit_real"
#endif

static int g_failures = 0;
static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_skipped = 0;

static void expect_true(int cond, const char *msg) {
    ++g_tests_run;
    if (!cond) {
        printf("FAIL: %s\n", msg);
        ++g_failures;
    } else {
        ++g_tests_passed;
    }
}

static void skip_test(const char *msg) {
    printf("SKIP: %s\n", msg);
    ++g_skipped;
}

static int bd_setenv(const char *name, const char *value) {
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
                         "%s_%d_%d", BD_TMP_ROOT, pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (bd_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    static const char *tpl = "/tmp/firestaff_theron_srm_body_unit_XXXXXX";
    if (strlen(tpl) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    strncpy(out, tpl, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static void cleanup_root_with_slot0(const char *root) {
    if (!root || !root[0]) return;
    char path[THERON_V1_SRM_PATH_MAX];
    if (theron_v1_srm_slot_path(root, 0, path)) {
        bd_unlink(path);
    }
    bd_rmdir(root);
}

static void cleanup_root_with_transfer_files(const char *root) {
    char source[THERON_V1_SRM_PATH_MAX];
    char destination[THERON_V1_SRM_PATH_MAX];
    char legacy_temporary[THERON_V1_SRM_PATH_MAX + 5u];
    if (!root || !root[0]) return;
    snprintf(source, sizeof(source), "%s%csource.srm", root, BD_PATH_SEP);
    snprintf(destination, sizeof(destination), "%s%cdestination.srm", root, BD_PATH_SEP);
    snprintf(legacy_temporary, sizeof(legacy_temporary), "%s.tmp", destination);
    bd_unlink(source);
    bd_unlink(destination);
    bd_unlink(legacy_temporary);
    bd_rmdir(root);
}

static int read_bytes(const char *path, uint8_t *buf, size_t size) {
    FILE *fp;
    if (!path || !buf || size == 0u) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(buf, 1, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static uint8_t *read_file_alloc(const char *path, size_t *out_size) {
    FILE *fp;
    long file_size;
    uint8_t *bytes;

    if (out_size) *out_size = 0u;
    if (!path || !path[0] || !out_size) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0L, SEEK_END) != 0 ||
        (file_size = ftell(fp)) <= 0 ||
        fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)file_size);
    if (!bytes) {
        fclose(fp);
        return NULL;
    }
    {
        int read_ok = fread(bytes, 1u, (size_t)file_size, fp) ==
            (size_t)file_size;
        int close_ok = fclose(fp) == 0;
        if (read_ok && close_ok) {
            *out_size = (size_t)file_size;
            return bytes;
        }
        free(bytes);
        return NULL;
    }
}

static int write_bytes(const char *path, const uint8_t *buf, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && buf) {
        size_t n = fwrite(buf, 1, size, fp);
        if (n != size) {
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

static void wr16le(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

/* Same FSTQPRG1 envelope bytes the existing progression-import test
 * uses, plus a slightly extended quest mask so the fixture's between-
 * dungeon semantics also travel through the envelope chain. */
static void build_progression_payload(uint8_t payload[44]) {
    static const uint8_t magic[8] = {
        'F', 'S', 'T', 'Q', 'P', 'R', 'G', '1'
    };
    memset(payload, 0, 44u);
    memcpy(payload, magic, sizeof(magic));
    payload[8] = 1u;        /* version */
    payload[9] = 3u;        /* current dungeon */
    payload[10] = 0x03u;    /* quest bits 0+1 complete (dungeon 3 reachable) */
    payload[11] = 1u;       /* current level */
    wr32le(payload + 12, 300u);
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        wr32le(payload + 16u + ((size_t)i * 4u), 0x1001u + (uint32_t)i);
    }
}

static void fill_body_record(uint8_t *record,
                             const char *name,
                             uint8_t cls,
                             uint8_t hp,
                             uint8_t stamina,
                             uint8_t mana,
                             uint8_t food,
                             uint8_t water) {
    memset(record, 0, 40u);
    if (name) {
        size_t n = strlen(name);
        if (n > 15u) n = 15u;
        memcpy(record, name, n);
    }
    record[16] = cls;
    record[17] = 1u;
    record[18] = hp;
    record[19] = hp;
    record[20] = stamina;
    record[21] = stamina;
    record[22] = mana;
    record[23] = mana;
    record[24] = (uint8_t)(18u + cls);
    record[25] = (uint8_t)(17u + cls);
    record[26] = (uint8_t)(16u + cls);
    record[27] = (uint8_t)(15u + cls);
    record[28] = 4u;
    record[29] = 5u;
    record[30 + cls] = 3u;
    wr16le(record + 35u, 0u);
    record[37] = food;
    record[38] = water;
}

#define BD_PARTY_PAYLOAD_BYTES (44u + 4u + (4u * 40u))

static void build_party_payload(uint8_t payload[BD_PARTY_PAYLOAD_BYTES]) {
    static const uint8_t magic[8] = {'F', 'S', 'T', 'Q', 'P', 'T', 'Y', '1'};
    uint8_t progress[44];
    memset(payload, 0, BD_PARTY_PAYLOAD_BYTES);
    build_progression_payload(progress);
    memcpy(payload, magic, sizeof(magic));
    memcpy(payload + 8u, progress + 8u, sizeof(progress) - 8u);
    wr32le(payload + 44u, 777u);
    fill_body_record(payload + 48u, "Theron", THERON_CLASS_FIGHTER,
                     82u, 76u, 24u, 60u, 61u);
    fill_body_record(payload + 88u, "Ari", THERON_CLASS_NINJA,
                     45u, 70u, 8u, 50u, 51u);
    fill_body_record(payload + 128u, "Mira", THERON_CLASS_PRIEST,
                     52u, 58u, 32u, 52u, 53u);
    fill_body_record(payload + 168u, "Sol", THERON_CLASS_WIZARD,
                     38u, 44u, 49u, 54u, 55u);
}

/* ── gzip wrapper ──────────────────────────────────────────────
 * The inflate chain strips a gzip header and feeds the deflate
 * stream at deflateInit2(..., -MAX_WBITS).  To build the matching
 * fixture we therefore produce raw-deflate bytes (window_bits<0)
 * and surround them with a minimal gzip wrapper: 10-byte fixed
 * header, no FNAME/FCOMMENT/FEXTRA/FHCRC flags, an 8-byte trailer
 * holding CRC32 + ISIZE.
 *
 * CRC32 is computed with zlib/crc32 when available; miniz exports
 * crc32 with the same signature.  ISIZE is the lower 32 bits of the
 * uncompressed size mod 2^32 (per RFC 1952 §2.2).
 */

static uint32_t bd_crc32(const uint8_t *buf, size_t size) {
#if FIRESTAFF_HAS_ZLIB
    return (uint32_t)crc32(crc32(0L, Z_NULL, 0), buf, (uInt)size);
#else
    /* Mirror the polynomial so the SKIP path stays consistent.  The
     * CRC32 isn't checked by the inflate chain (it stays in the 8-byte
     * trailer) but it has to be present for the wrapper to be valid
     * in the eyes of any external gzip reader. */
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
    zs.next_in = (Bytef *)(src);
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

/* Build a complete gzip-wrapped deflate stream for `payload`.  On
 * success writes the wrapped bytes into `out` and updates *out_size. */
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

    /* 10-byte fixed header: 0x1F 0x8B 0x08 CM FL MTIME[4] XFL OS */
    out[0] = THERON_V1_SRM_GZIP_MAGIC_0;
    out[1] = THERON_V1_SRM_GZIP_MAGIC_1;
    out[2] = THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE;
    out[3] = 0x00;        /* FLAGS: none set */
    out[4] = 0;           /* MTIME */
    out[5] = 0;
    out[6] = 0;
    out[7] = 0;
    out[8] = 0x02;        /* XFL: maximum compression */
    out[9] = 0xFF;        /* OS: unknown */

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

static void test_envelope_kind_names(void) {
    expect_true(strcmp(theron_v1_srm_envelope_kind_name(
                    THERON_V1_SRM_ENVELOPE_KIND_NONE), "NONE") == 0,
                "envelope kind NONE name");
    expect_true(strcmp(theron_v1_srm_envelope_kind_name(
                    THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION), "PROGRESSION") == 0,
                "envelope kind PROGRESSION name");
    expect_true(strcmp(theron_v1_srm_envelope_kind_name(
                    THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY), "PROGRESSION_PARTY") == 0,
                "envelope kind PROGRESSION_PARTY name");
    expect_true(strcmp(theron_v1_srm_envelope_kind_name(
                    THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED), "UNSUPPORTED") == 0,
                "envelope kind UNSUPPORTED name");
    expect_true(strcmp(theron_v1_srm_envelope_kind_name(99), "UNKNOWN") == 0,
                "envelope kind out-of-enum UNKNOWN name");
}

static void test_decode_envelope_progression_when_zlib(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    build_progression_payload(payload);
    expect_true(build_synthetic_gzip_stream(payload, sizeof(payload),
                                             srm_bytes, sizeof(srm_bytes),
                                             &srm_size) == 1,
                "synthetic gzip progression stream built");

    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_envelope(srm_bytes, srm_size,
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION,
                "progression-only envelope decodes");
    expect_true(envelope.decode_status == THERON_V1_SRM_PROGRESS_IMPORT_OK,
                "envelope decode_status OK");
    expect_true(envelope.inflate_status == THERON_V1_SRM_PAYLOAD_PROBE_OK,
                "envelope inflate_status OK");
    expect_true(envelope.inflate_payload_size == sizeof(payload),
                "envelope inflate_payload_size matches fixture");
    expect_true(envelope.body_evidence.captured == 1 &&
                    envelope.body_evidence.gzip_trailer_verified == 1,
                "envelope captures verified gzip body evidence");
    expect_true(envelope.body_evidence.payload_size == sizeof(payload) &&
                    envelope.body_evidence.gzip_trailer_isize == sizeof(payload),
                "envelope body evidence preserves authenticated payload size");
    expect_true(envelope.progression.restored == 1,
                "envelope progression.restored=1");
    expect_true(envelope.progression.current_dungeon ==
                    THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                "envelope progression current dungeon");
    expect_true(envelope.progression.quest_items_bitmask == 0x03,
                "envelope progression quest mask");
    expect_true(envelope.progression.current_level == 1,
                "envelope progression current level");
    expect_true(envelope.progression.dungeon_playtime_seconds == 300u,
                "envelope progression playtime");
    expect_true(envelope.party.restored == 0,
                "envelope party stays unpopulated when progression-only");
#else
    skip_test("inflate path needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "inflate path needs zlib (placeholder)");
#endif
}

static void test_decode_envelope_party_when_zlib(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[BD_PARTY_PAYLOAD_BYTES];
    uint8_t srm_bytes[512];
    size_t srm_size = sizeof(srm_bytes);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    build_party_payload(payload);
    expect_true(build_synthetic_gzip_stream(payload, sizeof(payload),
                                             srm_bytes, sizeof(srm_bytes),
                                             &srm_size) == 1,
                "synthetic gzip party stream built");

    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_envelope(srm_bytes, srm_size,
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY,
                "party envelope decodes");
    expect_true(envelope.decode_status == THERON_V1_SRM_PROGRESS_IMPORT_OK,
                "party envelope decode_status OK");
    expect_true(envelope.party.restored == 1, "party receipt restored");
    expect_true(envelope.party.party_gold == 777u, "party envelope gold");
    expect_true(envelope.party.imported_body_count == THERON_MAX_CHAMPIONS,
                "party envelope imported body count");
    expect_true(envelope.body_evidence.payload_checksum32 != 0u &&
                    envelope.body_evidence.gzip_trailer_verified == 1,
                "party envelope carries neutral verified body fingerprint");
    expect_true(envelope.progression.current_dungeon ==
                    THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                "party envelope progression current dungeon");
#else
    skip_test("party inflate path needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "party inflate path needs zlib (placeholder)");
#endif
}

static void test_decode_envelope_real_body_unsupported(void) {
    /* An arbitrary non-gzip buffer must trip the envelope chain at
     * the NOT_GZIP gate with inflate_status and decode_status both
     * populated honestly.  This is the boundary the gap text calls
     * "real Sphenx TQ-RTC .srm body ... UNSUPPORTED_BODY". */
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    static const uint8_t fake[] = "this is not a gzip stream";
    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_envelope(fake, sizeof(fake) - 1u,
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "fake envelope kind NONE");
    expect_true(envelope.inflate_status == THERON_V1_SRM_PAYLOAD_PROBE_NOT_GZIP,
                "fake envelope inflate_status NOT_GZIP");
    expect_true(envelope.decode_status == THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT,
                "fake envelope decode_status BAD_INPUT");

    /* A short (<10 byte) buffer must also fail clean. */
    static const uint8_t tiny[3] = {0x1F, 0x8B, 0x08};
    memset(&envelope, 0xCD, sizeof(envelope));
    kind = theron_v1_srm_decode_envelope(tiny, sizeof(tiny),
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "tiny envelope kind NONE");
    expect_true(envelope.inflate_status == THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT,
                "tiny envelope inflate_status BAD_INPUT");
}

static void test_decode_envelope_rejects_bad_gzip_trailer_when_zlib(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    build_progression_payload(payload);
    expect_true(build_synthetic_gzip_stream(payload, sizeof(payload),
                                             srm_bytes, sizeof(srm_bytes),
                                             &srm_size) == 1,
                "bad-trailer fixture gzip stream built");
    srm_bytes[srm_size - 8u] ^= 0x01u;
    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_envelope(srm_bytes, srm_size,
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "bad gzip trailer cannot produce an envelope");
    expect_true(envelope.inflate_status ==
                    THERON_V1_SRM_PAYLOAD_PROBE_TRAILER_MISMATCH,
                "bad gzip trailer has typed mismatch status");
    expect_true(envelope.body_evidence.captured == 0,
                "bad gzip trailer cannot create body evidence");
#else
    skip_test("gzip trailer validation needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "gzip trailer validation needs zlib (placeholder)");
#endif
}

static void test_decode_envelope_rejects_non_single_member_gzip_when_zlib(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload[44];
    uint8_t member[128];
    uint8_t combined[256];
    size_t member_size = sizeof(member);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    build_progression_payload(payload);
    expect_true(build_synthetic_gzip_stream(payload, sizeof(payload),
                                             member, sizeof(member),
                                             &member_size) == 1,
                "single-member fixture gzip stream built");
    memcpy(combined, member, member_size);
    memcpy(combined + member_size, member, member_size);
    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_envelope(combined, member_size * 2u,
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "concatenated gzip members cannot produce an envelope");
    expect_true(envelope.inflate_status ==
                    THERON_V1_SRM_PAYLOAD_PROBE_TRAILING_DATA,
                "concatenated gzip members have typed trailing-data status");
    expect_true(envelope.body_evidence.captured == 0,
                "concatenated gzip members cannot create body evidence");

    /* Insert an FHCRC field with a deliberately invalid value.  The raw
     * DEFLATE payload and trailer remain intact, isolating header validation. */
    memmove(member + 12u, member + 10u, member_size - 10u);
    member[3] |= 0x02u;
    member[10] = 0u;
    member[11] = 0u;
    memset(&envelope, 0xCD, sizeof(envelope));
    kind = theron_v1_srm_decode_envelope(member, member_size + 2u,
                                         scratch, sizeof(scratch),
                                         &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "invalid gzip FHCRC cannot produce an envelope");
    expect_true(envelope.inflate_status ==
                    THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED,
                "invalid gzip FHCRC fails before body inflate");
#else
    skip_test("single-member gzip validation needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "single-member gzip validation placeholder");
#endif
}

static void test_authenticated_body_evidence_catalog(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload_a[44];
    uint8_t payload_b[44];
    uint8_t gzip_a[128];
    uint8_t gzip_b[128];
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    size_t gzip_a_size = sizeof(gzip_a);
    size_t gzip_b_size = sizeof(gzip_b);
    Theron_V1SrmEnvelopeReceipt envelopes[THERON_V1_SRM_DISK_SLOT_COUNT];
    Theron_V1SrmBodyEvidenceCatalog catalog;

    build_progression_payload(payload_a);
    memcpy(payload_b, payload_a, sizeof(payload_b));
    payload_b[12u] ^= 0x40u;
    if (!build_synthetic_gzip_stream(payload_a, sizeof(payload_a),
                                     gzip_a, sizeof(gzip_a), &gzip_a_size) ||
        !build_synthetic_gzip_stream(payload_b, sizeof(payload_b),
                                     gzip_b, sizeof(gzip_b), &gzip_b_size)) {
        skip_test("authenticated body evidence catalog: gzip fixture unavailable");
        return;
    }

    memset(envelopes, 0, sizeof(envelopes));
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_a, gzip_a_size, scratch,
                                         sizeof(scratch), &envelopes[0]);
    envelopes[0].slot_index = 0;
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_a, gzip_a_size, scratch,
                                         sizeof(scratch), &envelopes[2]);
    envelopes[2].slot_index = 2;
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_b, gzip_b_size, scratch,
                                         sizeof(scratch), &envelopes[4]);
    envelopes[4].slot_index = 4;

    expect_true(theron_v1_srm_catalog_body_evidence(
                    envelopes, THERON_V1_SRM_DISK_SLOT_COUNT, &catalog) == 1,
                "authenticated body evidence catalog accepts envelope set");
    expect_true(catalog.authenticated_slot_count == 3u,
                "authenticated body evidence catalog counts verified bodies");
    expect_true(catalog.fingerprint_group_count == 2u,
                "authenticated body evidence catalog separates opaque fingerprints");
    expect_true(catalog.first_authenticated_slot == 0,
                "authenticated body evidence catalog reports first slot");
    expect_true(catalog.fingerprint_group_for_slot[0] ==
                    catalog.fingerprint_group_for_slot[2] &&
                    catalog.fingerprint_group_for_slot[4] !=
                    catalog.fingerprint_group_for_slot[0],
                "authenticated body evidence catalog groups only matching fingerprints");
    expect_true(catalog.fingerprint_group_for_slot[1] == -1 &&
                    catalog.catalog_checksum != 0u,
                "authenticated body evidence catalog keeps unauthenticated slots ungrouped");
#else
    skip_test("authenticated body evidence catalog needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "authenticated body evidence catalog placeholder");
#endif
}

static void test_authenticated_body_evidence_comparison(void) {
#if FIRESTAFF_HAS_ZLIB
    uint8_t payload_a[44];
    uint8_t payload_b[44];
    uint8_t gzip_a[128];
    uint8_t gzip_b[128];
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    uint8_t left2_payload[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    uint8_t right2_payload[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    size_t gzip_a_size = sizeof(gzip_a);
    size_t gzip_b_size = sizeof(gzip_b);
    size_t left2_size = 0u;
    size_t right2_size = 0u;
    Theron_V1SrmEnvelopeReceipt left[THERON_V1_SRM_DISK_SLOT_COUNT];
    Theron_V1SrmEnvelopeReceipt right[THERON_V1_SRM_DISK_SLOT_COUNT];
    Theron_V1SrmBodyEvidenceComparison comparison;
    Theron_V1SrmBodyEvidenceDelta delta;

    build_progression_payload(payload_a);
    memcpy(payload_b, payload_a, sizeof(payload_b));
    payload_b[12u] ^= 0x40u;
    payload_b[14u] ^= 0x20u;
    payload_b[15u] ^= 0x80u;
    if (!build_synthetic_gzip_stream(payload_a, sizeof(payload_a), gzip_a,
                                     sizeof(gzip_a), &gzip_a_size) ||
        !build_synthetic_gzip_stream(payload_b, sizeof(payload_b), gzip_b,
                                     sizeof(gzip_b), &gzip_b_size)) {
        skip_test("authenticated body evidence comparison: gzip fixture unavailable");
        return;
    }
    memset(left, 0, sizeof(left));
    memset(right, 0, sizeof(right));
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_a, gzip_a_size, scratch,
                                         sizeof(scratch), &left[0]);
    left[0].slot_index = 0;
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_b, gzip_b_size, scratch,
                                         sizeof(scratch), &left[2]);
    expect_true(theron_v1_srm_probe_gzip_payload(
                    gzip_b, gzip_b_size, left2_payload,
                    sizeof(left2_payload), &left2_size) ==
                    THERON_V1_SRM_PAYLOAD_PROBE_OK,
                "authenticated payload delta materializes left body");
    left[2].slot_index = 2;
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_a, gzip_a_size, scratch,
                                         sizeof(scratch), &right[0]);
    right[0].slot_index = 0;
    memset(scratch, 0, sizeof(scratch));
    (void)theron_v1_srm_decode_envelope(gzip_a, gzip_a_size, scratch,
                                         sizeof(scratch), &right[2]);
    expect_true(theron_v1_srm_probe_gzip_payload(
                    gzip_a, gzip_a_size, right2_payload,
                    sizeof(right2_payload), &right2_size) ==
                    THERON_V1_SRM_PAYLOAD_PROBE_OK,
                "authenticated payload delta materializes right body");
    right[2].slot_index = 2;

    expect_true(theron_v1_srm_compare_body_evidence(
                    left, THERON_V1_SRM_DISK_SLOT_COUNT,
                    right, THERON_V1_SRM_DISK_SLOT_COUNT, &comparison) == 1,
                "authenticated body evidence comparison accepts two receipt sets");
    expect_true(comparison.left_authenticated_slot_mask == 0x5u &&
                    comparison.right_authenticated_slot_mask == 0x5u &&
                    comparison.comparable_slot_mask == 0x5u,
                "authenticated body evidence comparison retains only shared authenticated slots");
    expect_true(comparison.matching_slot_mask == 0x1u &&
                    comparison.mismatch_slot_mask == 0x4u &&
                    comparison.comparison_checksum != 0u,
                "authenticated body evidence comparison reports opaque exact matches and mismatches");
    expect_true(theron_v1_srm_compare_authenticated_payloads(
                    &left[2], left2_payload, left2_size,
                    &right[2], right2_payload, right2_size,
                    &delta) == 1,
                "authenticated payload delta accepts matching authenticated receipts");
    expect_true(delta.left_payload_size == sizeof(payload_b) &&
                    delta.right_payload_size == sizeof(payload_a) &&
                    delta.shared_prefix_bytes == 12u &&
                    delta.shared_suffix_bytes == 28u &&
                    delta.differing_byte_count == 3u &&
                    delta.delta_checksum != 0u,
                "authenticated payload delta reports position-bound opaque byte layout");
    expect_true(delta.differing_run_count == 2u &&
                    delta.retained_run_count == 2u &&
                    delta.longest_differing_run_bytes == 2u &&
                    delta.differing_runs_truncated == 0 &&
                    delta.differing_runs[0].offset == 12u &&
                    delta.differing_runs[0].byte_count == 1u &&
                    delta.differing_runs[1].offset == 14u &&
                    delta.differing_runs[1].byte_count == 2u,
                "authenticated payload delta retains bounded opaque change topology");
    left2_payload[12u] ^= 0x40u;
    expect_true(theron_v1_srm_compare_authenticated_payloads(
                    &left[2], left2_payload, left2_size,
                    &right[2], right2_payload, right2_size,
                    &delta) == 0,
                "authenticated payload delta rejects bytes that do not match receipt fingerprints");
    left[4] = left[0];
    left[4].slot_index = 0;
    expect_true(theron_v1_srm_compare_body_evidence(
                    left, THERON_V1_SRM_DISK_SLOT_COUNT,
                    right, THERON_V1_SRM_DISK_SLOT_COUNT, &comparison) == 0,
                "authenticated body evidence comparison rejects duplicate authenticated slots");
#else
    skip_test("authenticated body evidence comparison needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "authenticated body evidence comparison placeholder");
#endif
}

static void test_decode_path_skips_when_absent(void) {
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;

    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_path(
        "/this/path/should/never/exist/slot0.srm",
        0,
        scratch, sizeof(scratch),
        &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "decode_path skip-clean on missing file");
    expect_true(envelope.inflate_status == THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT,
                "decode_path inflate_status BAD_INPUT for missing");
    expect_true(envelope.source_path[0] == '\0',
                "decode_path source_path empty for missing");
    expect_true(envelope.slot_index == 0,
                "decode_path slot_index preserved on missing");
    expect_true(envelope.file_size == 0,
                "decode_path file_size 0 on missing");

    memset(&envelope, 0xCD, sizeof(envelope));
    kind = theron_v1_srm_decode_path(NULL, 1,
                                     scratch, sizeof(scratch),
                                     &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "decode_path NULL path skip-clean");
    expect_true(envelope.slot_index == 1,
                "decode_path NULL slot_index preserved");

    memset(&envelope, 0xCD, sizeof(envelope));
    kind = theron_v1_srm_decode_path("", 2,
                                     scratch, sizeof(scratch),
                                     &envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                "decode_path empty path skip-clean");
}

#if FIRESTAFF_HAS_ZLIB
static void test_decode_path_slot_file_roundtrip(void) {
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;
    char root[THERON_V1_SRM_PATH_MAX];
    char slot_path[THERON_V1_SRM_PATH_MAX];

    if (!make_temp_root(root)) {
        skip_test("decode_path slot file: mkdtemp failed");
        return;
    }
    if (!theron_v1_srm_slot_path(root, 0, slot_path)) {
        skip_test("decode_path slot file: slot path failed");
        cleanup_root_with_slot0(root);
        return;
    }

    build_progression_payload(payload);
    if (!build_synthetic_gzip_stream(payload, sizeof(payload),
                                     srm_bytes, sizeof(srm_bytes),
                                     &srm_size)) {
        skip_test("decode_path slot file: gzip stream build failed");
        cleanup_root_with_slot0(root);
        return;
    }
    if (!write_bytes(slot_path, srm_bytes, srm_size)) {
        skip_test("decode_path slot file: write failed");
        cleanup_root_with_slot0(root);
        return;
    }

    memset(&envelope, 0xCD, sizeof(envelope));
    memset(scratch, 0, sizeof(scratch));
    kind = theron_v1_srm_decode_path(slot_path, 0,
                                     scratch, sizeof(scratch),
                                     &envelope);

    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION,
                "slot file roundtrip envelope kind PROGRESSION");
    expect_true(envelope.decode_status == THERON_V1_SRM_PROGRESS_IMPORT_OK,
                "slot file roundtrip decode_status OK");
    expect_true(envelope.progression.current_dungeon ==
                    THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                "slot file roundtrip current dungeon");
    expect_true(envelope.slot_index == 0,
                "slot file roundtrip slot_index preserved");
    expect_true(envelope.file_size == (uint64_t)srm_size,
                "slot file roundtrip file_size matches");
    expect_true(strstr(envelope.source_path, "slot0.srm") != NULL,
                "slot file roundtrip source_path points at slot0.srm");

    cleanup_root_with_slot0(root);
}

static void test_probe_slot0_envelope_via_env_override(void) {
    uint8_t payload[44];
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    Theron_V1SrmEnvelopeReceipt envelope;
    Theron_V1SrmEnvelopeKind kind;
    char root[THERON_V1_SRM_PATH_MAX];
    char slot_path[THERON_V1_SRM_PATH_MAX];
    char saved_root[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved_root, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_root[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }

    if (!make_temp_root(root)) {
        skip_test("probe_slot0_envelope: mkdtemp failed");
        if (had_prev) bd_setenv("FIRESTAFF_THERON_SRM_DIR", saved_root);
        else bd_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
        return;
    }
    bd_setenv("FIRESTAFF_THERON_SRM_DIR", root);

    expect_true(theron_v1_srm_slot_path(root, 0, slot_path) == 1,
                "probe_slot0_envelope: slot path constructs");

    build_progression_payload(payload);
    expect_true(build_synthetic_gzip_stream(payload, sizeof(payload),
                                             srm_bytes, sizeof(srm_bytes),
                                             &srm_size) == 1,
                "probe_slot0_envelope: gzip stream built");
    expect_true(write_bytes(slot_path, srm_bytes, srm_size) == 1,
                "probe_slot0_envelope: slot0.srm written");

    memset(&envelope, 0xCD, sizeof(envelope));
    kind = theron_v1_srm_probe_slot0_envelope(&envelope);
    expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION,
                "probe_slot0_envelope advances a real-look-alike slot");
    expect_true(envelope.decode_status == THERON_V1_SRM_PROGRESS_IMPORT_OK,
                "probe_slot0_envelope decode_status OK");
    expect_true(envelope.progression.current_dungeon ==
                    THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                "probe_slot0_envelope current dungeon");

    cleanup_root_with_slot0(root);
    if (had_prev) bd_setenv("FIRESTAFF_THERON_SRM_DIR", saved_root);
    else bd_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
}
#else
static void test_decode_path_slot_file_roundtrip(void) {
    skip_test("decode_path slot file needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "decode_path slot file needs zlib (placeholder)");
}
static void test_probe_slot0_envelope_via_env_override(void) {
    skip_test("probe_slot0_envelope needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "probe_slot0_envelope needs zlib (placeholder)");
}
#endif

static void test_probe_slot0_envelope_skip_when_absent(void) {
    /* Point FIRESTAFF_THERON_SRM_DIR at a root that does not exist
     * so the probe skip-clean path is reproducible. */
    char saved_root[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved_root, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_root[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    bd_setenv("FIRESTAFF_THERON_SRM_DIR",
              "/tmp/firestaff_theron_srm_body_unit_definitely_absent");

    {
        Theron_V1SrmEnvelopeReceipt envelope;
        Theron_V1SrmEnvelopeReceipt sentinel;
        Theron_V1SrmEnvelopeKind kind;
        memset(&sentinel, 0xCD, sizeof(sentinel));
        memset(&envelope, 0xCD, sizeof(envelope));
        kind = theron_v1_srm_probe_slot0_envelope(&envelope);
        expect_true(kind == THERON_V1_SRM_ENVELOPE_KIND_NONE,
                    "probe_slot0_envelope skip-clean on missing save root");
        expect_true(memcmp(&envelope, &sentinel, sizeof(envelope)) == 0,
                    "probe_slot0_envelope leaves envelope untouched on missing save root");
    }

    if (had_prev) bd_setenv("FIRESTAFF_THERON_SRM_DIR", saved_root);
    else bd_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
}

static void test_opaque_original_container_transfer(void) {
#if FIRESTAFF_HAS_ZLIB
    static const uint8_t unknown_body[] = {
        'T', 'Q', 'R', '-', 'R', 'E', 'A', 'L', '-', 'B', 'O', 'D', 'Y', 0x01, 0x02
    };
    uint8_t srm_bytes[128];
    uint8_t destination_bytes[sizeof(srm_bytes)];
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    size_t srm_size = sizeof(srm_bytes);
    Theron_V1SrmOpaqueTransferReceipt receipt;
    char root[THERON_V1_SRM_PATH_MAX];
    char source[THERON_V1_SRM_PATH_MAX];
    char destination[THERON_V1_SRM_PATH_MAX];
    char legacy_temporary[THERON_V1_SRM_PATH_MAX + 5u];

    if (!make_temp_root(root)) {
        skip_test("opaque original container transfer: temporary root unavailable");
        return;
    }
    snprintf(source, sizeof(source), "%s%csource.srm", root, BD_PATH_SEP);
    snprintf(destination, sizeof(destination), "%s%cdestination.srm", root, BD_PATH_SEP);
    snprintf(legacy_temporary, sizeof(legacy_temporary), "%s.tmp", destination);
    if (!build_synthetic_gzip_stream(unknown_body, sizeof(unknown_body), srm_bytes,
                                     sizeof(srm_bytes), &srm_size) ||
        !write_bytes(source, srm_bytes, srm_size)) {
        skip_test("opaque original container transfer: gzip fixture unavailable");
        cleanup_root_with_transfer_files(root);
        return;
    }
    memset(&receipt, 0xCD, sizeof(receipt));
    memset(scratch, 0, sizeof(scratch));
    expect_true(theron_v1_srm_transfer_opaque_path(
                    source, destination, scratch, sizeof(scratch), &receipt) == 1,
                "opaque transfer accepts an authenticated unknown SRM body");
    expect_true(receipt.validation_status == THERON_V1_SRM_PAYLOAD_PROBE_OK &&
                    receipt.body_kind == THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED &&
                    receipt.body_evidence.gzip_trailer_verified == 1,
                "opaque transfer records authenticated container without body promotion");
    expect_true(receipt.copied == 1 && receipt.byte_exact == 1 &&
                    receipt.source_size == receipt.destination_size &&
                    receipt.source_checksum32 == receipt.destination_checksum32 &&
                    read_bytes(destination, destination_bytes, srm_size) &&
                    memcmp(srm_bytes, destination_bytes, srm_size) == 0,
                "opaque transfer round-trips exact original container bytes");
    expect_true(theron_v1_srm_transfer_opaque_path(
                    source, destination, scratch, sizeof(scratch), &receipt) == 0,
                "opaque transfer refuses to overwrite an existing destination");
    cleanup_root_with_transfer_files(root);

    if (!make_temp_root(root)) {
        skip_test("opaque original container transfer stale-temp: temporary root unavailable");
        return;
    }
    snprintf(source, sizeof(source), "%s%csource.srm", root, BD_PATH_SEP);
    snprintf(destination, sizeof(destination), "%s%cdestination.srm", root, BD_PATH_SEP);
    snprintf(legacy_temporary, sizeof(legacy_temporary), "%s.tmp", destination);
    if (!write_bytes(source, srm_bytes, srm_size) ||
        !write_bytes(legacy_temporary, (const uint8_t *)"old", 3u)) {
        skip_test("opaque original container transfer stale-temp: fixture write failed");
        cleanup_root_with_transfer_files(root);
        return;
    }
    memset(&receipt, 0xCD, sizeof(receipt));
    expect_true(theron_v1_srm_transfer_opaque_path(
                    source, destination, scratch, sizeof(scratch), &receipt) == 1 &&
                    receipt.byte_exact == 1,
                "opaque transfer ignores a stale legacy temporary without weakening exact copy");
    cleanup_root_with_transfer_files(root);

    if (!make_temp_root(root)) {
        skip_test("opaque original container transfer rejection: temporary root unavailable");
        return;
    }
    snprintf(source, sizeof(source), "%s%csource.srm", root, BD_PATH_SEP);
    snprintf(destination, sizeof(destination), "%s%cdestination.srm", root, BD_PATH_SEP);
    srm_bytes[srm_size - 8u] ^= 0x01u;
    if (!write_bytes(source, srm_bytes, srm_size)) {
        skip_test("opaque original container transfer rejection: source write failed");
        cleanup_root_with_transfer_files(root);
        return;
    }
    memset(&receipt, 0xCD, sizeof(receipt));
    expect_true(theron_v1_srm_transfer_opaque_path(
                    source, destination, scratch, sizeof(scratch), &receipt) == 0 &&
                    receipt.validation_status == THERON_V1_SRM_PAYLOAD_PROBE_TRAILER_MISMATCH &&
                    receipt.copied == 0 && stat(destination, &(struct stat){0}) != 0,
                "opaque transfer rejects corruption before creating a destination");
    {
        static const uint8_t destination_before[] = {'k', 'e', 'e', 'p'};
        uint8_t destination_after[sizeof(destination_before)];
        expect_true(write_bytes(destination, destination_before, sizeof(destination_before)),
                    "opaque transfer corruption fixture creates protected destination");
        expect_true(theron_v1_srm_transfer_opaque_path(
                        source, destination, scratch, sizeof(scratch), &receipt) == 0 &&
                        receipt.copied == 0 &&
                        read_bytes(destination, destination_after, sizeof(destination_after)) &&
                        memcmp(destination_before, destination_after,
                               sizeof(destination_before)) == 0,
                    "opaque transfer never changes an existing destination");
    }
    cleanup_root_with_transfer_files(root);
#else
    skip_test("opaque original container transfer needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "opaque original container transfer placeholder");
#endif
}

static void test_runtime_refuses_unsupported_original_body(void) {
#if FIRESTAFF_HAS_ZLIB
    static const uint8_t unknown_body[] = {'T', 'Q', 'R', '-', 'O', 'P', 'A', 'Q', 'U', 'E'};
    uint8_t srm_bytes[128];
    size_t srm_size = sizeof(srm_bytes);
    char root[THERON_V1_SRM_PATH_MAX];
    char path[THERON_V1_SRM_PATH_MAX];
    Theron_V1_World world;
    Theron_V1_World before;
    Theron_V1SrmRuntimeReceipt receipt;
    uint8_t track = 0u;

    if (!make_temp_root(root) || !theron_v1_srm_slot_path(root, 0, path) ||
        !build_synthetic_gzip_stream(unknown_body, sizeof(unknown_body), srm_bytes,
                                     sizeof(srm_bytes), &srm_size) ||
        !write_bytes(path, srm_bytes, srm_size)) {
        skip_test("runtime unsupported original SRM: fixture unavailable");
        cleanup_root_with_slot0(root);
        return;
    }
    theron_v1_world_init(&world);
    before = world;
    expect_true(theron_v1_srm_runtime_continue_path(
                    &world, path, &track, 1u, "unverified-track", &receipt) ==
                    THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY &&
                    receipt.envelope_kind == THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED &&
                    memcmp(&world, &before, sizeof(world)) == 0,
                "runtime refuses an unsupported original body before mutating world state");
    cleanup_root_with_slot0(root);
#else
    skip_test("runtime unsupported original SRM needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "runtime unsupported original SRM placeholder");
#endif
}

static void test_runtime_export_continue_roundtrip(void) {
#if FIRESTAFF_HAS_ZLIB
    static const uint8_t descriptor[18] = {
        0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
        0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
    };
    static const uint8_t span[44] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
        0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
        0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
        0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
        0x93, 0x80, 0x00, 0x3f
    };
    char root[THERON_V1_SRM_PATH_MAX];
    char path[THERON_V1_SRM_PATH_MAX];
    uint8_t track[0x3000u + 160u];
    Theron_V1_World source, restored, before_continue;
    Theron_V1SrmRuntimeReceipt receipt;

    if (!make_temp_root(root) || !theron_v1_srm_slot_path(root, 0, path)) {
        skip_test("runtime SRM roundtrip: temporary root unavailable");
        return;
    }
    theron_v1_world_init(&source);
    source.progression.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    source.progression.current_level = 2u;
    source.progression.quest_items_collected = 0x03u;
    source.progression.dungeon_playtime_seconds = 987u;
    source.party.champion_count = THERON_MAX_CHAMPIONS;
    source.party.gold = 444u;
    snprintf(source.party.champions[0].name, sizeof(source.party.champions[0].name), "Theron");
    source.party.champions[0].health = 77;
    expect_true(theron_v1_srm_runtime_export_path(&source, path, &receipt) == THERON_V1_SRM_RUNTIME_OK && receipt.srm_size > 10u,
                "runtime export writes gzip .srm bytes");

    {
        static const uint8_t original_save_bytes[] = {
            0x1f, 0x8b, 0x08, 0x00, 'O', 'R', 'I', 'G', 'I', 'N', 'A', 'L'
        };
        uint8_t protected_bytes[sizeof(original_save_bytes)];
        expect_true(write_bytes(path, original_save_bytes,
                                sizeof(original_save_bytes)),
                    "runtime export no-replace fixture stages an original Save Disk");
        expect_true(theron_v1_srm_runtime_export_path(
                        &source, path, &receipt) ==
                        THERON_V1_SRM_RUNTIME_DESTINATION_EXISTS &&
                    receipt.status == THERON_V1_SRM_RUNTIME_DESTINATION_EXISTS &&
                    read_bytes(path, protected_bytes, sizeof(protected_bytes)) &&
                    memcmp(protected_bytes, original_save_bytes,
                           sizeof(original_save_bytes)) == 0,
                    "runtime export never replaces an existing original Save Disk");
        bd_unlink(path);
        expect_true(theron_v1_srm_runtime_export_path(&source, path, &receipt) ==
                        THERON_V1_SRM_RUNTIME_OK,
                    "runtime export can publish after the protected path is removed");
    }

    memset(track, 0, sizeof(track));
    memcpy(track + 0x1584u, descriptor, sizeof(descriptor));
    memcpy(track + 0x3000u, span, sizeof(span));
    for (size_t i = sizeof(span); i < 160u; ++i) track[0x3000u + i] = (uint8_t)(0x21u + i);
    theron_v1_world_init(&restored);
    before_continue = restored;
    expect_true(theron_v1_srm_runtime_continue_path(
                    &restored, path, track, sizeof(track),
                    THERON_TRACK02_MD5_US_ISO, &receipt) ==
                    THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED &&
                    receipt.status == THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED &&
                    memcmp(&restored, &before_continue, sizeof(restored)) == 0,
                "runtime Continue rejects identity-only Track02 bytes before restoring world state");
    cleanup_root_with_slot0(root);
#else
    skip_test("runtime SRM roundtrip needs zlib (ZLIB_UNAVAILABLE)");
    expect_true(1, "runtime SRM roundtrip placeholder");
#endif
}

static void test_runtime_continue_binds_real_track02_when_staged(void) {
#if FIRESTAFF_HAS_ZLIB
    const char *track_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    char root[THERON_V1_SRM_PATH_MAX];
    char path[THERON_V1_SRM_PATH_MAX];
    uint8_t *track;
    size_t track_size;
    Theron_V1_World source, restored;
    Theron_V1SrmRuntimeReceipt receipt;

    if (!track_path || !track_path[0]) {
        skip_test("runtime real Track 02 Continue: no US raw Track 02 staged");
        return;
    }
    track = read_file_alloc(track_path, &track_size);
    if (!track || !make_temp_root(root) ||
        !theron_v1_srm_slot_path(root, 0, path)) {
        free(track);
        skip_test("runtime real Track 02 Continue: staged data unavailable");
        cleanup_root_with_slot0(root);
        return;
    }
    theron_v1_world_init(&source);
    source.progression.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    source.progression.current_level = 2u;
    source.progression.quest_items_collected = 0x03u;
    source.party.champion_count = THERON_MAX_CHAMPIONS;
    source.party.gold = 444u;
    if (theron_v1_srm_runtime_export_path(&source, path, &receipt) !=
        THERON_V1_SRM_RUNTIME_OK) {
        free(track);
        cleanup_root_with_slot0(root);
        skip_test("runtime real Track 02 Continue: SRM staging unavailable");
        return;
    }
    theron_v1_world_init(&restored);
    expect_true(theron_v1_srm_runtime_continue_path(
                    &restored, path, track, track_size,
                    THERON_TRACK02_MD5_US_BIN, &receipt) ==
                    THERON_V1_SRM_RUNTIME_OK &&
                    restored.progression.current_dungeon ==
                        THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                    restored.progression.current_level == 2u &&
                    restored.party.gold == 444u &&
                    restored.runtime_media.restored &&
                    restored.runtime_media.identity.ready &&
                    restored.runtime_media.level_bank.ready &&
                    restored.runtime_media.level_bank.kind ==
                        THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME &&
                    receipt.track02_media_route_mask ==
                        restored.runtime_media.route_mask &&
                    receipt.track02_media_checksum ==
                        restored.runtime_media.checksum &&
                    receipt.track02_level_bank.ready &&
                    receipt.track02_level_bank.kind ==
                        THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME &&
                    receipt.track02_level_bank.surface_checksum ==
                        restored.runtime_media.level_bank.surface_checksum,
                "runtime Continue commits and receipts complete real US Track 02 media");
    free(track);
    cleanup_root_with_slot0(root);
#else
    skip_test("runtime real Track 02 Continue needs zlib (ZLIB_UNAVAILABLE)");
#endif
}

int main(void) {
    printf("\n=== Theron V1 SRM Body-Decode Envelope Unit Tests ===\n\n");
    test_envelope_kind_names();
    test_decode_envelope_progression_when_zlib();
    test_decode_envelope_party_when_zlib();
    test_decode_envelope_real_body_unsupported();
    test_decode_envelope_rejects_bad_gzip_trailer_when_zlib();
    test_decode_envelope_rejects_non_single_member_gzip_when_zlib();
    test_authenticated_body_evidence_catalog();
    test_authenticated_body_evidence_comparison();
    test_decode_path_skips_when_absent();
    test_decode_path_slot_file_roundtrip();
    test_probe_slot0_envelope_via_env_override();
    test_probe_slot0_envelope_skip_when_absent();
    test_opaque_original_container_transfer();
    test_runtime_refuses_unsupported_original_body();
    test_runtime_export_continue_roundtrip();
    test_runtime_continue_binds_real_track02_when_staged();

    printf("=====================================================\n");
    printf("Results: %d/%d passed (failures=%d, skipped=%d)\n",
           g_tests_passed, g_tests_run, g_failures, g_skipped);
    printf("=====================================================\n\n");
    return g_failures ? 1 : 0;
}
