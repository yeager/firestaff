/*
 * theron_v1_srm_classifier.c — Theron V1 SRM (Save RAM) classifier
 *
 * Bounded real-artifact boundary for Theron's Quest Save Disk data.
 * See include/theron_v1_srm_classifier.h for the full contract.
 *
 * Source/evidence (also exposed via theron_v1_srm_source_evidence()):
 *   - docs/DMWEB_REFERENCE.md §6 "Theron's Quest savegame format"
 *   - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   - THQUEST.ASM T080 — no in-dungeon saves (TQR design)
 *   - DMWeb community docs credit Sphenx with several custom TQ saves
 *     documented at greatstone; Sphenx is also a SKWIN co-author.
 *
 * Status (this commit, 2026-06-27):
 *   - The classifier is data-free and runs anywhere.  It is the bounded
 *     real-artifact counterpart to the synthetic slot-N.tqsv in-game
 *     save format in theron_v1_save_load.c.
 *   - No real .srm file is present in the local data root
 *     (~/.firestaff/data/theron/save/) on this host, so the
 *     classification outcome for the default root is `present_count=0,
 *     recognized_count=0`.  This is the expected honest outcome and is
 *     recorded as a SKIP, not a failure, by the probe/test.
 *   - The classifier will accept a real .srm file when one is placed
 *     under the configured root and a hash-gated slot-path override is
 *     set, so future runs on hosts that have Sphenx's TQ-RTC save
 *     dumps can promote present/recognized counts without changing
 *     the public API.
 *
 * Phases tracked separately (out of scope for this commit):
 *   - Interpreting the inflated custom Theron save body.
 *   - Cross-slot import: real Sphenx .srm -> Theron_DungeonProgression
 *     and champion blocks.  This would close the greatstone section
 *     referenced in docs/DMWEB_REFERENCE.md §6.
 *   - Save Disk raw sector classification (raw .iso Track save area,
 *     analogous to theron_v1_track02_bank for the data track).
 */

#include "theron_v1_srm_classifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#define TSRM_PATH_SEP '\\'
#else
#define TSRM_PATH_SEP '/'
#endif

/* ── Path helpers ────────────────────────────────────────────────── */

static int file_exists_regular(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int dir_exists(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/* ── Default root resolution ─────────────────────────────────────── */

int theron_v1_srm_default_root(char out_root[THERON_V1_SRM_PATH_MAX]) {
    if (!out_root) return 0;

    /* 1) Env override wins. */
    const char *env_root = getenv("FIRESTAFF_THERON_SRM_DIR");
    if (env_root && env_root[0]) {
        size_t n = strlen(env_root);
        if (n >= THERON_V1_SRM_PATH_MAX) n = THERON_V1_SRM_PATH_MAX - 1;
        memcpy(out_root, env_root, n);
        out_root[n] = '\0';
        return 1;
    }

    /* 2) $HOME/.firestaff/data/theron/save */
    const char *home = getenv("HOME");
    if (home && home[0]) {
        snprintf(out_root, THERON_V1_SRM_PATH_MAX,
                 "%s%c.firestaff%cdata%ctheron%csave",
                 home, TSRM_PATH_SEP, TSRM_PATH_SEP, TSRM_PATH_SEP, TSRM_PATH_SEP);
        return 1;
    }

    /* 3) ./theron-save (deterministic last-resort path so the
     *    classifier never silently misroutes). */
    snprintf(out_root, THERON_V1_SRM_PATH_MAX, ".%ctheron-save", TSRM_PATH_SEP);
    return 1;
}

int theron_v1_srm_slot_path(const char *root,
                             int slot_index,
                             char out_path[THERON_V1_SRM_PATH_MAX]) {
    if (!out_path) return 0;
    if (slot_index < 0 || slot_index >= THERON_V1_SRM_DISK_SLOT_COUNT) {
        out_path[0] = '\0';
        return 0;
    }
    if (!root || !root[0]) {
        out_path[0] = '\0';
        return 0;
    }

    /* Strip any trailing separator on the root so the joined path
     * doesn't carry double separators on POSIX.  Windows accepts
     * either, but a clean path is easier to log. */
    size_t root_len = strlen(root);
    while (root_len > 0 && (root[root_len - 1] == '/' || root[root_len - 1] == '\\')) {
        root_len--;
    }
    if (root_len == 0) {
        out_path[0] = '\0';
        return 0;
    }

    int n = snprintf(out_path, THERON_V1_SRM_PATH_MAX,
                     "%.*s%cslot%d.srm",
                     (int)root_len, root, TSRM_PATH_SEP, slot_index);
    if (n <= 0 || (size_t)n >= THERON_V1_SRM_PATH_MAX) {
        out_path[0] = '\0';
        return 0;
    }
    return 1;
}

/* ── Checksum + magic-byte inspection ────────────────────────────── */

/* Simple 32-bit rolling sum over the input bytes.  This is a receipt
 * hash, not a cryptographic digest: it lets the probe detect a
 * silently-truncated or substituted .srm file across runs without
 * committing a specific real artifact to the manifest contract. */
static uint32_t rolling_checksum32(const uint8_t *buf, size_t size) {
    uint32_t sum = 0xC1A551F1u; /* arbitrary non-zero seed */
    for (size_t i = 0; i < size; i++) {
        sum = (sum * 33u) + buf[i];
    }
    return sum;
}

/* Read up to `max_bytes` from `path` into a heap buffer.  Returns the
 * number of bytes actually read, or 0 on failure.  The caller owns the
 * returned buffer. */
static size_t read_prefix(const char *path, size_t max_bytes, uint8_t **out_buf) {
    if (!path || !out_buf) return 0;
    *out_buf = NULL;

    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    size_t want = (size_t)size;
    if (want > max_bytes) want = max_bytes;
    if (want == 0) {
        fclose(fp);
        return 0;
    }

    uint8_t *buf = (uint8_t *)malloc(want);
    if (!buf) {
        fclose(fp);
        return 0;
    }
    size_t got = fread(buf, 1, want, fp);
    fclose(fp);
    if (got == 0) {
        free(buf);
        return 0;
    }
    *out_buf = buf;
    return got;
}

static Theron_V1SrmSlotStatus classify_file(const char *path, uint64_t *out_size,
                                            uint32_t *out_checksum,
                                            int *out_gzip_magic,
                                            int *out_gzip_deflate) {
    if (out_size) *out_size = 0;
    if (out_checksum) *out_checksum = 0;
    if (out_gzip_magic) *out_gzip_magic = 0;
    if (out_gzip_deflate) *out_gzip_deflate = 0;

    if (!file_exists_regular(path)) {
        return THERON_V1_SRM_SLOT_ABSENT;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        return THERON_V1_SRM_SLOT_ABSENT;
    }
    uint64_t size = (uint64_t)(st.st_size);
    if (out_size) *out_size = size;

    /* Empty files are malformed in the gzipped format sense: the
     * gzip header alone is 10 bytes minimum, and a real Theron save
     * body is non-empty. */
    if (size < 10) {
        if (out_checksum) *out_checksum = 0;
        return THERON_V1_SRM_SLOT_MALFORMED;
    }

    uint8_t *buf = NULL;
    size_t got = read_prefix(path, THERON_V1_SRM_PREFIX_BYTES, &buf);
    if (!buf || got < 10) {
        free(buf);
        return THERON_V1_SRM_SLOT_MALFORMED;
    }

    int gzip_magic = (buf[0] == THERON_V1_SRM_GZIP_MAGIC_0 &&
                      buf[1] == THERON_V1_SRM_GZIP_MAGIC_1);
    int gzip_deflate = (gzip_magic && buf[2] == THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE);

    if (out_gzip_magic) *out_gzip_magic = gzip_magic;
    if (out_gzip_deflate) *out_gzip_deflate = gzip_deflate;
    if (out_checksum) *out_checksum = rolling_checksum32(buf, got);

    free(buf);

    if (!gzip_magic) {
        return THERON_V1_SRM_SLOT_UNRECOGNIZED;
    }
    if (!gzip_deflate) {
        /* Magic matched but compression method is not DEFLATE.  This
         * is plausible (gzip reserves other methods) but the
         * dmweb-anchored TQ save body is gzipped DEFLATE, so we
         * surface this as MALFORMED rather than PRESENT. */
        return THERON_V1_SRM_SLOT_MALFORMED;
    }
    return THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED;
}

/* ── Manifest population ─────────────────────────────────────────── */

int theron_v1_srm_classify_root(const char *root,
                                Theron_V1SrmManifest *out_manifest) {
    if (!out_manifest) return 0;
    memset(out_manifest, 0, sizeof(*out_manifest));
    out_manifest->slot_count = THERON_V1_SRM_DISK_SLOT_COUNT;

    if (root && root[0]) {
        size_t n = strlen(root);
        if (n >= THERON_V1_SRM_PATH_MAX) n = THERON_V1_SRM_PATH_MAX - 1;
        memcpy(out_manifest->root, root, n);
        out_manifest->root[n] = '\0';
        out_manifest->root_resolved = 1;
    } else {
        /* Fall back to the canonical default root so the manifest
         * always reflects the actual save-disk directory the caller
         * is using. */
        if (!theron_v1_srm_default_root(out_manifest->root)) {
            return 0;
        }
        out_manifest->root_resolved = 1;
    }

    /* Honest boundary: a missing save root is not an error.  The
     * manifest simply records zero present/recognized slots. */
    int root_present = dir_exists(out_manifest->root);

    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        Theron_V1SrmSlotInfo *slot = &out_manifest->slots[i];
        slot->slot_index = i;
        slot->status = THERON_V1_SRM_SLOT_ABSENT;
        slot->path[0] = '\0';
        slot->size_bytes = 0;
        slot->prefix_checksum32 = 0;
        slot->gzip_magic_seen = 0;
        slot->gzip_deflate_method_seen = 0;

        if (!theron_v1_srm_slot_path(out_manifest->root, i, slot->path)) {
            continue;
        }

        if (!root_present) {
            /* Root dir does not exist → all slots ABSENT. */
            continue;
        }

        slot->status = classify_file(slot->path,
                                     &slot->size_bytes,
                                     &slot->prefix_checksum32,
                                     &slot->gzip_magic_seen,
                                     &slot->gzip_deflate_method_seen);
        if (slot->status != THERON_V1_SRM_SLOT_ABSENT) {
            out_manifest->present_count++;
            if (slot->status == THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED) {
                out_manifest->recognized_count++;
            }
        }
    }

    return 1;
}

/* ── Bounded gzip-payload probe ──────────────────────────────────── */

Theron_V1SrmPayloadProbeStatus theron_v1_srm_probe_gzip_payload(
    const uint8_t *srm_bytes,
    size_t srm_size,
    uint8_t *out_payload,
    size_t out_payload_capacity,
    size_t *out_payload_size) {

    if (out_payload_size) *out_payload_size = 0;
    if (!srm_bytes || srm_size < 10 || !out_payload ||
        out_payload_capacity == 0 || !out_payload_size) {
        return THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT;
    }
    if (srm_bytes[0] != THERON_V1_SRM_GZIP_MAGIC_0 ||
        srm_bytes[1] != THERON_V1_SRM_GZIP_MAGIC_1) {
        return THERON_V1_SRM_PAYLOAD_PROBE_NOT_GZIP;
    }
    if (srm_bytes[2] != THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE) {
        return THERON_V1_SRM_PAYLOAD_PROBE_UNSUPPORTED_METHOD;
    }

#if FIRESTAFF_HAS_ZLIB
    z_stream zs;
    int ret;

    memset(&zs, 0, sizeof(zs));
    zs.next_in = (Bytef *)srm_bytes;
    zs.avail_in = (uInt)srm_size;
    zs.next_out = out_payload;
    zs.avail_out = (uInt)out_payload_capacity;

    ret = inflateInit2(&zs, 16 + MAX_WBITS); /* gzip wrapper + deflate */
    if (ret != Z_OK) {
        return THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED;
    }

    ret = inflate(&zs, Z_FINISH);
    if (ret == Z_STREAM_END) {
        *out_payload_size = (size_t)zs.total_out;
        inflateEnd(&zs);
        return THERON_V1_SRM_PAYLOAD_PROBE_OK;
    }

    *out_payload_size = (size_t)zs.total_out;
    inflateEnd(&zs);
    return ret == Z_BUF_ERROR && *out_payload_size == out_payload_capacity
        ? THERON_V1_SRM_PAYLOAD_PROBE_OUTPUT_TRUNCATED
        : THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED;
#else
    (void)out_payload;
    (void)out_payload_capacity;
    return THERON_V1_SRM_PAYLOAD_PROBE_ZLIB_UNAVAILABLE;
#endif
}

/* ── Status names + source evidence ──────────────────────────────── */

const char *theron_v1_srm_slot_status_name(Theron_V1SrmSlotStatus status) {
    switch (status) {
    case THERON_V1_SRM_SLOT_ABSENT:
        return "ABSENT";
    case THERON_V1_SRM_SLOT_UNRECOGNIZED:
        return "UNRECOGNIZED";
    case THERON_V1_SRM_SLOT_MALFORMED:
        return "MALFORMED";
    case THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED:
        return "PRESENT_AND_RECOGNIZED";
    }
    return "UNKNOWN";
}

const char *theron_v1_srm_payload_probe_status_name(Theron_V1SrmPayloadProbeStatus status) {
    switch (status) {
    case THERON_V1_SRM_PAYLOAD_PROBE_OK:
        return "OK";
    case THERON_V1_SRM_PAYLOAD_PROBE_ZLIB_UNAVAILABLE:
        return "ZLIB_UNAVAILABLE";
    case THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT:
        return "BAD_INPUT";
    case THERON_V1_SRM_PAYLOAD_PROBE_NOT_GZIP:
        return "NOT_GZIP";
    case THERON_V1_SRM_PAYLOAD_PROBE_UNSUPPORTED_METHOD:
        return "UNSUPPORTED_METHOD";
    case THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED:
        return "INFLATE_FAILED";
    case THERON_V1_SRM_PAYLOAD_PROBE_OUTPUT_TRUNCATED:
        return "OUTPUT_TRUNCATED";
    }
    return "UNKNOWN";
}

const char *theron_v1_srm_source_evidence(void) {
    return
        "Theron V1 SRM (Save RAM) classifier — bounded real-artifact boundary\n"
        "\n"
        "Source/evidence:\n"
        "  - docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame format':\n"
        "      'TQ's save format is *completely different* from DM's (it uses\n"
        "       gzipped custom format with a header).'\n"
        "  - dmweb community docs credit Sphenx with several custom TQ saves\n"
        "    documented at greatstone; Sphenx is also a SKWIN DM2 skproject\n"
        "    co-author.\n"
        "  - docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md anchors\n"
        "    the JP/US Track 02 hash and the THQUEST.ASM T080 (no in-dungeon\n"
        "    saves) design rule.\n"
        "  - THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon)\n"
        "  - THQUEST.ASM T800  — champion persistence between dungeons\n"
        "\n"
        "Status (2026-06-27):\n"
        "  - Data-free classifier with 5-slot disk manifest, gzip magic\n"
        "    detection (0x1F 0x8B 0x08), 1 KiB rolling prefix checksum,\n"
        "    present/recognized rollup, and stable string status names.\n"
        "  - Bounded gzip-payload probe inflates a recognized .srm stream\n"
        "    when Firestaff is built with zlib; without zlib it reports\n"
        "    ZLIB_UNAVAILABLE after the cheap gzip/method checks.\n"
        "  - Default save-disk root: $HOME/.firestaff/data/theron/save.\n"
        "  - Override: env FIRESTAFF_THERON_SRM_DIR.\n"
        "  - No real .srm file is present in the local data root on this\n"
        "    host; the manifest reports present_count=0, recognized_count=0\n"
        "    on the default root.  This is the expected honest outcome and\n"
        "    is recorded as a SKIP, not a failure, by the probe and unit\n"
        "    test.\n"
        "  - Interpreting the inflated custom Theron save body and\n"
        "    cross-slot import to Theron_DungeonProgression/champion blocks\n"
        "    remain out of scope and are tracked under docs/FIRESTAFF_GAP_LIST.md\n"
        "    A3 'Savegame format (Theron .SRM)'.";
}
