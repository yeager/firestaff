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
 * Status (this commit, 2026-06-27/28):
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
 *   - Interpreting the real inflated custom Theron save body.  The
 *     probe can now read a recognized slot, inflate its gzip body, and
 *     decode one Firestaff-only progression/champion readiness block,
 *     but Sphenx/Greatstone byte mapping remains unknown.
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

#if !defined(FIRESTAFF_THERON_PRODUCTION)
#define TSRM_PROGRESS_PAYLOAD_BYTES 44u
#define TSRM_PARTY_BODY_BYTES 40u
#define TSRM_PARTY_PAYLOAD_BYTES \
    (TSRM_PROGRESS_PAYLOAD_BYTES + 4u + \
     ((size_t)THERON_MAX_CHAMPIONS * TSRM_PARTY_BODY_BYTES))

static const uint8_t g_progress_payload_magic[8] = {
    'F', 'S', 'T', 'Q', 'P', 'R', 'G', '1'
};

static const uint8_t g_party_payload_magic[8] = {
    'F', 'S', 'T', 'Q', 'P', 'T', 'Y', '1'
};
#endif

static uint16_t rd16le(const uint8_t *p);
static uint32_t rd32le(const uint8_t *p);

static uint32_t pce_bram_fnv1a(const uint8_t *data, size_t size) {
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0u; index < size; ++index) {
        hash ^= data[index];
        hash *= 16777619u;
    }
    return hash;
}

int theron_v1_pce_bram_campaign_byte_restorable(uint8_t campaign_byte) {
    /* Both authenticated regional restore routines return immediately for
     * zero, then mask to seven campaign bits and reject values >= 7. */
    return campaign_byte != 0u && (campaign_byte & 0x7fu) < 7u;
}

Theron_V1PceBramStatus theron_v1_pce_bram_classify(
    const uint8_t *data, size_t size, Theron_V1PceBramReceipt *out) {
    static const uint8_t marker[] = {'D', 'M', 'S', '-', 'S', 'G', '.'};
    size_t index;
    if (out) { memset(out, 0, sizeof(*out)); out->status = THERON_V1_PCE_BRAM_BAD_INPUT; }
    if (!data || !out) return THERON_V1_PCE_BRAM_BAD_INPUT;
    out->size_bytes = size;
    if (size != THERON_V1_PCE_BRAM_BYTES) {
        out->status = THERON_V1_PCE_BRAM_WRONG_SIZE;
        return out->status;
    }
    out->bytes_fnv1a = pce_bram_fnv1a(data, size);
    if (memcmp(data, "HUBM", 4u) != 0) {
        out->status = THERON_V1_PCE_BRAM_BAD_HEADER;
        return out->status;
    }
    out->hubm_header_seen = 1;
    for (index = 0x10u; index + sizeof(marker) + 3u <= size; ++index) {
        if (memcmp(data + index, marker, sizeof(marker)) == 0 &&
            data[index + 7u] >= '0' && data[index + 7u] <= '9' &&
            data[index + 8u] >= '0' && data[index + 8u] <= '9' &&
            data[index + 9u] >= '0' && data[index + 9u] <= '9') {
            out->theron_save_disk_marker_seen = 1;
            out->theron_save_disk_marker_offset = index;
            if (index == 0x16u &&
                memcmp(data + index, "DMS-SG.001", 10u) == 0 &&
                ((size_t)data[0x10u] | ((size_t)data[0x11u] << 8u)) ==
                    THERON_V1_PCE_BRAM_RECORD_BYTES &&
                data[0x15u] == 0x0bu &&
                0x20u + THERON_V1_PCE_BRAM_DATA_BYTES <= size) {
                out->save_record_offset = 0x10u;
                out->save_record_bytes = THERON_V1_PCE_BRAM_RECORD_BYTES;
                out->save_data_offset = 0x20u;
                out->save_data_bytes = THERON_V1_PCE_BRAM_DATA_BYTES;
                out->save_slot_bytes = THERON_V1_PCE_BRAM_SLOT_BYTES;
                out->save_slot_count = THERON_V1_PCE_BRAM_SLOT_COUNT;
                out->save_trailing_bytes = 1u;
                out->save_slot_tail_unconsumed_padding = 1;
                out->save_record_layout_proven = 1;
                out->selected_slot_index =
                    data[0x20u + THERON_V1_PCE_BRAM_SELECTED_SLOT_OFFSET];
                if (out->selected_slot_index <
                    THERON_V1_PCE_BRAM_SLOT_COUNT) {
                    out->selected_slot_offset =
                        0x20u + (size_t)out->selected_slot_index *
                                    THERON_V1_PCE_BRAM_SLOT_BYTES;
                    out->selected_slot_layout_proven = 1;
                    out->save_body_offset = out->selected_slot_offset;
                    out->save_body_bytes = 0x86u;
                    out->serialized_campaign_byte_offset =
                        out->selected_slot_offset;
                    out->serialized_campaign_byte =
                        data[out->selected_slot_offset];
                    out->save_body_layout_proven = 1;
                    out->save_body_campaign_valid =
                        theron_v1_pce_bram_campaign_byte_restorable(
                            out->serialized_campaign_byte);
                }
            }
            out->status = THERON_V1_PCE_BRAM_READY;
            return out->status;
        }
    }
    out->status = THERON_V1_PCE_BRAM_NO_THERON_SAVE_DISK;
    return out->status;
}

Theron_V1PceBramStatus theron_v1_pce_bram_classify_path(
    const char *path, Theron_V1PceBramReceipt *out) {
    uint8_t data[THERON_V1_PCE_BRAM_BYTES + 1u];
    FILE *file;
    size_t size;
    if (!path || !out) return THERON_V1_PCE_BRAM_BAD_INPUT;
    file = fopen(path, "rb");
    if (!file) { memset(out, 0, sizeof(*out)); out->status = THERON_V1_PCE_BRAM_BAD_INPUT; return out->status; }
    size = fread(data, 1u, sizeof(data), file);
    if (ferror(file)) { fclose(file); memset(out, 0, sizeof(*out)); out->status = THERON_V1_PCE_BRAM_BAD_INPUT; return out->status; }
    fclose(file);
    return theron_v1_pce_bram_classify(data, size, out);
}

int theron_v1_pce_bram_decode_original_body(
    const uint8_t *data, size_t size, Theron_V1PceBramBodyReceipt *out) {
    Theron_V1PceBramReceipt container;
    const uint8_t *body;
    size_t column;

    if (out) memset(out, 0, sizeof(*out));
    if (!data || !out ||
        theron_v1_pce_bram_classify(data, size, &container) !=
            THERON_V1_PCE_BRAM_READY ||
        !container.save_body_layout_proven ||
        container.save_body_bytes != 0x86u ||
        container.save_body_offset + container.save_body_bytes > size) {
        return 0;
    }

    body = data + container.save_body_offset;
    out->body_fnv1a = pce_bram_fnv1a(body, container.save_body_bytes);
    out->ram_267c_campaign_byte = body[0];
    memcpy(out->ram_267d_2682, body + 1u,
           sizeof(out->ram_267d_2682));
    memcpy(out->ram_2683_2689, body + 7u,
           sizeof(out->ram_2683_2689));
    for (column = 0u; column < 6u; ++column) {
        memcpy(out->ram_268a_2701[column], body + 14u + column * 20u,
               sizeof(out->ram_268a_2701[column]));
    }
    out->theron_max_health = rd16le(out->ram_267d_2682);
    out->theron_max_stamina = rd16le(out->ram_267d_2682 + 2u);
    out->theron_max_mana = rd16le(out->ram_267d_2682 + 4u);
    memcpy(out->theron_max_attributes, out->ram_2683_2689,
           sizeof(out->theron_max_attributes));
    for (column = 0u; column < 20u; ++column) {
        out->theron_skill_temporary_experience[column] =
            (uint16_t)out->ram_268a_2701[0][column] |
            ((uint16_t)out->ram_268a_2701[1][column] << 8);
        out->theron_skill_experience[column] =
            (uint32_t)out->ram_268a_2701[2][column] |
            ((uint32_t)out->ram_268a_2701[3][column] << 8) |
            ((uint32_t)out->ram_268a_2701[4][column] << 16) |
            ((uint32_t)out->ram_268a_2701[5][column] << 24);
    }
    out->semantics_verified = 1;
    out->layout_verified = 1;
    return 1;
}

int theron_v1_pce_bram_decode_original_body_path(
    const char *path, Theron_V1PceBramBodyReceipt *out) {
    uint8_t data[THERON_V1_PCE_BRAM_BYTES + 1u];
    FILE *file;
    size_t size;

    if (out) memset(out, 0, sizeof(*out));
    if (!path || !out || !(file = fopen(path, "rb"))) return 0;
    size = fread(data, 1u, sizeof(data), file);
    if (ferror(file)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return theron_v1_pce_bram_decode_original_body(data, size, out);
}

int theron_v1_pce_bram_decode_original_record(
    const uint8_t *data, size_t size, Theron_V1PceBramRecordReceipt *out) {
    Theron_V1PceBramReceipt container;
    const uint8_t *record_data;
    size_t slot;

    if (out) memset(out, 0, sizeof(*out));
    if (!data || !out ||
        theron_v1_pce_bram_classify(data, size, &container) !=
            THERON_V1_PCE_BRAM_READY ||
        !container.save_record_layout_proven ||
        container.save_data_offset != 0x20u ||
        container.save_data_bytes != THERON_V1_PCE_BRAM_DATA_BYTES ||
        container.save_slot_bytes != THERON_V1_PCE_BRAM_SLOT_BYTES ||
        container.save_slot_count != THERON_V1_PCE_BRAM_SLOT_COUNT ||
        container.save_trailing_bytes != 1u ||
        container.save_data_offset + container.save_data_bytes > size) {
        return 0;
    }

    record_data = data + container.save_data_offset;
    out->data_fnv1a = pce_bram_fnv1a(record_data,
                                     THERON_V1_PCE_BRAM_DATA_BYTES);
    for (slot = 0u; slot < THERON_V1_PCE_BRAM_SLOT_COUNT; ++slot) {
        memcpy(out->slots[slot],
               record_data + slot * THERON_V1_PCE_BRAM_SLOT_BYTES,
               THERON_V1_PCE_BRAM_SLOT_BYTES);
    }
    out->selected_slot_index =
        record_data[THERON_V1_PCE_BRAM_SLOT_COUNT *
                    THERON_V1_PCE_BRAM_SLOT_BYTES];
    out->slot_tail_unconsumed_padding = 1;
    out->layout_verified = 1;
    return 1;
}

int theron_v1_pce_bram_decode_original_record_path(
    const char *path, Theron_V1PceBramRecordReceipt *out) {
    uint8_t data[THERON_V1_PCE_BRAM_BYTES + 1u];
    FILE *file;
    size_t size;

    if (out) memset(out, 0, sizeof(*out));
    if (!path || !out || !(file = fopen(path, "rb"))) return 0;
    size = fread(data, 1u, sizeof(data), file);
    if (ferror(file)) {
        fclose(file);
        memset(out, 0, sizeof(*out));
        return 0;
    }
    fclose(file);
    return theron_v1_pce_bram_decode_original_record(data, size, out);
}

/* ── Path helpers ────────────────────────────────────────────────── */

static int file_exists_regular(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int dir_exists(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/* Bounded gzip header validation for one .srm buffer (restored after the
 * df88dbda4 clobber).  Checks magic/method/reserved flag bits and walks the
 * optional FEXTRA/FNAME/FCOMMENT/FHCRC spans before recording the header
 * metadata; truncated containers fail closed. */
int theron_v1_srm_header_receipt(const uint8_t *data,
                                 size_t size,
                                 Theron_V1SrmHeaderReceipt *out) {
    size_t p = 10u;
    uint8_t f;
    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!data || !out || size < 10u ||
        data[0] != 0x1fu || data[1] != 0x8bu || data[2] != 8u ||
        (data[3] & 0xe0u)) {
        return 0;
    }
    f = data[3];
    if (f & 4u) {
        size_t n;
        if (p + 2u > size) return 0;
        n = (size_t)data[p] | ((size_t)data[p + 1u] << 8u);
        p += 2u;
        if (n > size - p) return 0;
        p += n;
    }
    if (f & 8u) {
        while (p < size && data[p]) ++p;
        if (p == size) return 0;
        ++p;
    }
    if (f & 16u) {
        while (p < size && data[p]) ++p;
        if (p == size) return 0;
        ++p;
    }
    if ((f & 2u) && p + 2u > size) return 0;
    out->valid = 1;
    out->container_bytes = size;
    out->mtime = (uint32_t)data[4] | ((uint32_t)data[5] << 8u) |
        ((uint32_t)data[6] << 16u) | ((uint32_t)data[7] << 24u);
    out->xfl = data[8];
    out->os = data[9];
    return 1;
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

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
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

#if FIRESTAFF_HAS_ZLIB
static int theron_v1_srm_gzip_deflate_slice(const uint8_t *srm_bytes,
                                            size_t srm_size,
                                            size_t *out_offset,
                                            size_t *out_size) {
    if (!srm_bytes || !out_offset || !out_size || srm_size < 18u) {
        return 0;
    }
    if (srm_bytes[0] != THERON_V1_SRM_GZIP_MAGIC_0 ||
        srm_bytes[1] != THERON_V1_SRM_GZIP_MAGIC_1 ||
        srm_bytes[2] != THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE) {
        return 0;
    }

    const uint8_t flags = srm_bytes[3];
    size_t pos = 10u; /* gzip fixed header */

    /* RFC 1952 reserves the high three flag bits.  The raw-DEFLATE path
     * below bypasses zlib's gzip wrapper, so validate this container fact
     * before any body can become SRM evidence. */
    if (flags & 0xe0u) return 0;

    if (flags & 0x04u) { /* FEXTRA */
        if (pos + 2u > srm_size) return 0;
        size_t xlen = (size_t)srm_bytes[pos] | ((size_t)srm_bytes[pos + 1u] << 8);
        pos += 2u;
        if (pos + xlen > srm_size) return 0;
        pos += xlen;
    }
    if (flags & 0x08u) { /* FNAME */
        while (pos < srm_size && srm_bytes[pos] != 0u) pos++;
        if (pos >= srm_size) return 0;
        pos++;
    }
    if (flags & 0x10u) { /* FCOMMENT */
        while (pos < srm_size && srm_bytes[pos] != 0u) pos++;
        if (pos >= srm_size) return 0;
        pos++;
    }
    if (flags & 0x02u) { /* FHCRC */
        uint32_t header_crc;
        if (pos + 2u > srm_size) return 0;
        header_crc = (uint32_t)crc32(crc32(0L, Z_NULL, 0),
                                     srm_bytes, (uInt)pos);
        if ((uint16_t)header_crc != rd16le(srm_bytes + pos)) return 0;
        pos += 2u;
    }

    if (pos + 8u > srm_size) {
        return 0;
    }

    /* DMWeb/greatstone identify Theron save bodies as gzip-wrapped custom
     * save data.  Strip only the gzip container here so both system zlib and
     * bundled miniz can inflate the raw deflate stream; save-body semantics
     * stay in the later Theron progression/champion import milestone. */
    *out_offset = pos;
    *out_size = srm_size - pos - 8u; /* trailer: CRC32 + ISIZE */
    return *out_size > 0u;
}
#endif

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
    size_t deflate_offset = 0;
    size_t deflate_size = 0;

    if (!theron_v1_srm_gzip_deflate_slice(srm_bytes, srm_size,
                                          &deflate_offset, &deflate_size)) {
        return THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED;
    }

    memset(&zs, 0, sizeof(zs));
    zs.next_in = (Bytef *)(srm_bytes + deflate_offset);
    zs.avail_in = (uInt)deflate_size;
    zs.next_out = out_payload;
    zs.avail_out = (uInt)out_payload_capacity;

    ret = inflateInit2(&zs, -MAX_WBITS); /* raw deflate after local gzip parse */
    if (ret != Z_OK) {
        return THERON_V1_SRM_PAYLOAD_PROBE_INFLATE_FAILED;
    }

    ret = inflate(&zs, Z_FINISH);
    if (ret == Z_STREAM_END) {
        size_t trailer_offset = deflate_offset + (size_t)zs.total_in;
        uint32_t expected_crc;
        uint32_t expected_isize;
        uint32_t actual_crc = (uint32_t)crc32(crc32(0L, Z_NULL, 0),
                                              out_payload, (uInt)zs.total_out);
        *out_payload_size = (size_t)zs.total_out;
        inflateEnd(&zs);
        /* A Save Disk slot must be exactly one gzip member.  In particular,
         * do not authenticate the first member against a later trailer. */
        if (trailer_offset + 8u != srm_size) {
            return THERON_V1_SRM_PAYLOAD_PROBE_TRAILING_DATA;
        }
        expected_crc = rd32le(srm_bytes + trailer_offset);
        expected_isize = rd32le(srm_bytes + trailer_offset + 4u);
        if (expected_crc != actual_crc || expected_isize !=
            (uint32_t)(*out_payload_size & 0xffffffffu)) {
            return THERON_V1_SRM_PAYLOAD_PROBE_TRAILER_MISMATCH;
        }
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

#if !defined(FIRESTAFF_THERON_PRODUCTION)
static void theron_v1_srm_capture_body_evidence(
    const uint8_t *srm_bytes,
    size_t srm_size,
    const uint8_t *payload,
    size_t payload_size,
    Theron_V1SrmBodyEvidence *out_evidence) {

    size_t sample_size;

    if (!out_evidence) return;
    memset(out_evidence, 0, sizeof(*out_evidence));
    if (!srm_bytes || srm_size < 18u || !payload || payload_size == 0u) {
        return;
    }

    sample_size = payload_size < 16u ? payload_size : 16u;
    out_evidence->payload_size = payload_size;
    out_evidence->payload_checksum32 = rolling_checksum32(payload, payload_size);
    out_evidence->payload_prefix_checksum32 = rolling_checksum32(payload, sample_size);
    out_evidence->payload_suffix_checksum32 =
        rolling_checksum32(payload + payload_size - sample_size, sample_size);
    out_evidence->gzip_trailer_crc32 = rd32le(srm_bytes + srm_size - 8u);
    out_evidence->gzip_trailer_isize = rd32le(srm_bytes + srm_size - 4u);
    out_evidence->gzip_flags = srm_bytes[3];
    out_evidence->gzip_extra_flags = srm_bytes[8];
    out_evidence->gzip_os = srm_bytes[9];
#if FIRESTAFF_HAS_ZLIB
    out_evidence->computed_crc32 = (uint32_t)crc32(
        crc32(0L, Z_NULL, 0), payload, (uInt)payload_size);
    out_evidence->gzip_trailer_verified =
        out_evidence->computed_crc32 == out_evidence->gzip_trailer_crc32 &&
        out_evidence->gzip_trailer_isize == (uint32_t)(payload_size & 0xffffffffu);
#endif
    out_evidence->captured = 1;
}
#endif

Theron_V1SrmProgressImportStatus theron_v1_srm_decode_progression_payload(
    const uint8_t *payload,
    size_t payload_size,
    Theron_DungeonProgression *out_progression,
    Theron_V1SrmProgressionReceipt *out_receipt) {

#if defined(FIRESTAFF_THERON_PRODUCTION)
    (void)payload;
    (void)payload_size;
    (void)out_progression;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    return THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
#else

    uint8_t version;
    uint8_t current_dungeon_raw;
    uint8_t quest_mask;
    uint8_t current_level;
    uint32_t playtime;
    uint32_t seeds[THERON_DUNGEON_COUNT];
    uint8_t expected_prefix;
    Theron_DungeonProgression restored;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!payload || !out_progression) {
        return THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    }
    if (payload_size < sizeof(g_progress_payload_magic)) {
        return THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    }
    if (memcmp(payload, g_progress_payload_magic, sizeof(g_progress_payload_magic)) != 0) {
        return THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
    }
    if (payload_size < TSRM_PROGRESS_PAYLOAD_BYTES) {
        return THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    }

    version = payload[8];
    if (version != 1u) {
        return THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_VERSION;
    }

    current_dungeon_raw = payload[9];
    quest_mask = payload[10];
    current_level = payload[11];
    playtime = rd32le(payload + 12);

    if (current_dungeon_raw < 1u ||
        current_dungeon_raw > (uint8_t)THERON_DUNGEON_COUNT ||
        (quest_mask & (uint8_t)~THERON_QUEST_ALL_ITEMS) != 0u ||
        current_level < 1u ||
        current_level > 3u) {
        return THERON_V1_SRM_PROGRESS_IMPORT_OUT_OF_RANGE;
    }

    if (quest_mask == THERON_QUEST_ALL_ITEMS) {
        if (current_dungeon_raw != (uint8_t)THERON_DUNGEON_7_DEMON) {
            return THERON_V1_SRM_PROGRESS_IMPORT_NON_MONOTONIC_QUEST_STATE;
        }
    } else {
        expected_prefix = current_dungeon_raw == 1u
            ? 0u
            : (uint8_t)((1u << (current_dungeon_raw - 1u)) - 1u);
        if (quest_mask != expected_prefix) {
            return THERON_V1_SRM_PROGRESS_IMPORT_NON_MONOTONIC_QUEST_STATE;
        }
    }

    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        seeds[i] = rd32le(payload + 16u + ((size_t)i * 4u));
    }

    /* Source: THQUEST.ASM T080/T800 between-dungeon save semantics,
     * encoded here only for the Firestaff readiness envelope.  Unknown
     * Sphenx/Greatstone custom bodies remain unsupported until decoded. */
    theron_v1_dungeon_progression_restore(
        &restored,
        quest_mask,
        (Theron_DungeonID)current_dungeon_raw,
        seeds);
    restored.current_level = current_level;
    restored.dungeon_playtime_seconds = playtime;
    if (quest_mask != THERON_QUEST_ALL_ITEMS) {
        const Theron_DungeonMeta *meta =
            theron_v1_dungeon_meta((Theron_DungeonID)current_dungeon_raw);
        restored.item_reset_mode = meta && meta->champion_reset
            ? THERON_ITEM_RESET_MODE_CHAMPION
            : THERON_ITEM_RESET_MODE_NONE;
    }

    *out_progression = restored;
    if (out_receipt) {
        out_receipt->version = version;
        out_receipt->quest_items_bitmask = quest_mask;
        out_receipt->current_dungeon = (Theron_DungeonID)current_dungeon_raw;
        out_receipt->current_level = current_level;
        out_receipt->dungeon_playtime_seconds = playtime;
        memcpy(out_receipt->dungeon_seeds, seeds, sizeof(seeds));
        out_receipt->restored = 1;
    }
    return THERON_V1_SRM_PROGRESS_IMPORT_OK;
#endif
}

#if !defined(FIRESTAFF_THERON_PRODUCTION)
static void copy_fixed_name(char out[24], const uint8_t *src, size_t src_size) {
    size_t n = 0;
    if (!out || !src) return;
    memset(out, 0, 24u);
    while (n < src_size && n < 23u && src[n] != 0u) {
        out[n] = (char)src[n];
        n++;
    }
}

static int import_body_record(Theron_V1_Champion *champion,
                              const uint8_t *record,
                              int slot) {
    Theron_ChampionClass cls;
    uint16_t attrs;

    if (!champion || !record) return 0;
    if (record[16] >= (uint8_t)THERON_CLASS_COUNT) return 0;
    if (record[17] > 1u) return 0;

    cls = (Theron_ChampionClass)record[16];
    attrs = rd16le(record + 35u);

    copy_fixed_name(champion->name, record, 16u);
    if (champion->name[0] == '\0') {
        /* An empty source name is not evidence for a roster identity. Never
         * synthesize Theron/Companion here; reject the body record until the
         * original name bytes are present. */
        (void)slot;
        return 0;
    }

    /* The SRM body record identifies a champion slot, not a portrait
     * graphic. Do not turn that slot number into a synthetic portrait ID;
     * portrait pixels and their consumer still require a source-bound
     * bitmap capture. */
    (void)slot;
    champion->portrait_index = THERON_PORTRAIT_UNAVAILABLE;
    champion->primary_class = cls;
    champion->alive = record[17];

    champion->health = (int16_t)record[18];
    champion->max_health = (int16_t)record[19];
    champion->stamina = (int16_t)record[20];
    champion->max_stamina = (int16_t)record[21];
    champion->mana = (int16_t)record[22];
    champion->max_mana = (int16_t)record[23];

    champion->strength = (int16_t)record[24];
    champion->dexterity = (int16_t)record[25];
    champion->wisdom = (int16_t)record[26];
    champion->vitality = (int16_t)record[27];
    champion->anti_magic = (int16_t)record[28];
    champion->anti_fire = (int16_t)record[29];

    champion->fighter_level = (int16_t)record[30];
    champion->ninja_level = (int16_t)record[31];
    champion->priest_level = (int16_t)record[32];
    champion->wizard_level = (int16_t)record[33];
    champion->wounds = record[34];
    champion->attributes = attrs;
    champion->food = (int16_t)record[37];
    champion->water = (int16_t)record[38];
    if (!champion->alive || champion->health <= 0) {
        champion->alive = 0;
        champion->health = 0;
        champion->attributes |= THERON_ATTR_DEAD;
    } else {
        champion->attributes &= (uint16_t)~THERON_ATTR_DEAD;
    }

    /* Source: THQUEST.ASM T080/T800 plus
     * docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §5/§9.
     * This readiness envelope imports body/state fields only.  It
     * keeps inventory and equipment empty until the real Sphenx /
     * Greatstone Save Disk body has been decoded. */
    theron_v1_champion_reset_inventory(champion);
    return 1;
}
#endif

Theron_V1SrmProgressImportStatus theron_v1_srm_decode_progression_party_payload(
    const uint8_t *payload,
    size_t payload_size,
    Theron_DungeonProgression *out_progression,
    Theron_V1_Party *out_party,
    Theron_V1SrmPartyImportReceipt *out_receipt) {

#if defined(FIRESTAFF_THERON_PRODUCTION)
    (void)payload;
    (void)payload_size;
    (void)out_progression;
    (void)out_party;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    return THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
#else

    uint8_t progress_payload[TSRM_PROGRESS_PAYLOAD_BYTES];
    Theron_DungeonProgression progression;
    Theron_V1SrmProgressionReceipt progression_receipt;
    Theron_V1_Party party;
    Theron_V1SrmProgressImportStatus status;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!payload || !out_progression || !out_party) {
        return THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    }
    if (payload_size < sizeof(g_party_payload_magic)) {
        return THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    }
    if (memcmp(payload, g_party_payload_magic, sizeof(g_party_payload_magic)) != 0) {
        return THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
    }
    if (payload_size < TSRM_PARTY_PAYLOAD_BYTES) {
        return THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    }

    memcpy(progress_payload, g_progress_payload_magic, sizeof(g_progress_payload_magic));
    memcpy(progress_payload + sizeof(g_progress_payload_magic),
           payload + sizeof(g_party_payload_magic),
           TSRM_PROGRESS_PAYLOAD_BYTES - sizeof(g_progress_payload_magic));

    status = theron_v1_srm_decode_progression_payload(
        progress_payload,
        sizeof(progress_payload),
        &progression,
        &progression_receipt);
    if (status != THERON_V1_SRM_PROGRESS_IMPORT_OK) {
        return status;
    }

    /* A decoded SRM party must be built solely from its source records.
     * Do not seed the import with the legacy fixture roster: malformed or
     * partially decoded records must never inherit synthetic names, classes,
     * stats or inventory from party_init(). */
    memset(&party, 0, sizeof(party));
    party.gold = rd32le(payload + TSRM_PROGRESS_PAYLOAD_BYTES);
    party.champion_count = THERON_MAX_CHAMPIONS;
    party.active_slot = THERON_CHAMPION_SLOT_THERON;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        const uint8_t *record = payload + TSRM_PROGRESS_PAYLOAD_BYTES + 4u +
                                ((size_t)i * TSRM_PARTY_BODY_BYTES);
        if (!import_body_record(&party.champions[i], record, i)) {
            return THERON_V1_SRM_PROGRESS_IMPORT_OUT_OF_RANGE;
        }
    }
    theron_v1_party_recalculate_loads(&party);

    *out_progression = progression;
    *out_party = party;
    if (out_receipt) {
        out_receipt->progression = progression_receipt;
        out_receipt->party_gold = party.gold;
        out_receipt->champion_count = THERON_MAX_CHAMPIONS;
        out_receipt->active_slot = THERON_CHAMPION_SLOT_THERON;
        out_receipt->imported_body_count = THERON_MAX_CHAMPIONS;
        out_receipt->restored = 1;
    }
    return THERON_V1_SRM_PROGRESS_IMPORT_OK;
#endif
}

/* ── Body-decode envelope surface ──────────────────────────────────
 *
 * The envelope decoder takes a raw .srm buffer (or a slot file at
 * `path`) and runs the bounded (gzip magic check → inflate →
 * envelope magic check → decode) chain in one call, returning a typed
 * `Theron_V1SrmEnvelopeKind`.  It exists for the gap's
 * "Theron_DungeonProgression or champion-block decode fixture"
 * requirement: a synthetic gzipped FSTQPRG1 body inflates and
 * produces a restored `Theron_DungeonProgression`, while a real
 * Sphenx/Greatstone custom body returns ENVELOPE_KIND_UNSUPPORTED
 * and stays non-launchable.
 *
 * The implementation is intentionally minimal: it reuses
 * theron_v1_srm_probe_gzip_payload + the existing
 * theron_v1_srm_decode_progression_payload +
 * theron_v1_srm_decode_progression_party_payload pair, so envelope
 * semantics stay exactly the same as the documented lower-level
 * decoders.  The work budget is the same
 * THERON_V1_SRM_BODY_DECODE_MAX_BYTES bound that the header
 * declares; bodies that exceed it return OUTPUT_TRUNCATED at inflate
 * time, which the envelope surfaces as a typed kind + decode_status.
 */

Theron_V1SrmEnvelopeKind theron_v1_srm_decode_envelope(
    const uint8_t *srm_bytes,
    size_t srm_size,
    uint8_t *scratch,
    size_t scratch_capacity,
    Theron_V1SrmEnvelopeReceipt *out_envelope) {

#if defined(FIRESTAFF_THERON_PRODUCTION)
    (void)srm_bytes;
    (void)srm_size;
    (void)scratch;
    (void)scratch_capacity;
    if (out_envelope) {
        memset(out_envelope, 0, sizeof(*out_envelope));
        out_envelope->slot_index = -1;
        out_envelope->kind = THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED;
        out_envelope->decode_status =
            THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
    }
    return THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED;
#else

    Theron_V1SrmPayloadProbeStatus inflate_status;
    size_t payload_size = 0;

    if (!out_envelope) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }
    memset(out_envelope, 0, sizeof(*out_envelope));
    out_envelope->inflate_status = THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT;
    out_envelope->decode_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;
    out_envelope->slot_index = -1;

    if (!srm_bytes || srm_size == 0 || !scratch ||
        scratch_capacity == 0) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }

    /* Cap the work budget at the documented body-decode ceiling so
     * neither the inflate nor the decode path can silently grow the
     * heap.  The previous lower-level functions already capped their
     * own inflation at `scratch_capacity`; this ceiling is the one
     * call-site-visible bound. */
    if (scratch_capacity > THERON_V1_SRM_BODY_DECODE_MAX_BYTES) {
        scratch_capacity = THERON_V1_SRM_BODY_DECODE_MAX_BYTES;
    }

    inflate_status = theron_v1_srm_probe_gzip_payload(
        srm_bytes,
        srm_size,
        scratch,
        scratch_capacity,
        &payload_size);
    out_envelope->inflate_status = inflate_status;
    out_envelope->inflate_payload_size = payload_size;

    if (inflate_status != THERON_V1_SRM_PAYLOAD_PROBE_OK) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }
    theron_v1_srm_capture_body_evidence(srm_bytes, srm_size, scratch,
                                        payload_size,
                                        &out_envelope->body_evidence);

    /* First try the progression+party envelope (the larger of the
     * two).  If the magic does not match, drop to the
     * progression-only envelope.  Both reject unknown magic with
     * UNSUPPORTED_BODY so the envelope surface stays honest. */
    {
        Theron_DungeonProgression progression;
        Theron_V1_Party party;
        Theron_V1SrmPartyImportReceipt party_receipt;
        Theron_V1SrmProgressImportStatus party_status =
            theron_v1_srm_decode_progression_party_payload(
                scratch,
                payload_size,
                &progression,
                &party,
                &party_receipt);
        if (party_status == THERON_V1_SRM_PROGRESS_IMPORT_OK) {
            out_envelope->kind = THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY;
            out_envelope->decode_status = THERON_V1_SRM_PROGRESS_IMPORT_OK;
            out_envelope->progression = party_receipt.progression;
            out_envelope->party = party_receipt;
            return THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY;
        }

        Theron_V1SrmProgressionReceipt progress_receipt;
        Theron_V1SrmProgressImportStatus prog_status =
            theron_v1_srm_decode_progression_payload(
                scratch,
                payload_size,
                &progression,
                &progress_receipt);
        out_envelope->decode_status = prog_status;
        if (prog_status == THERON_V1_SRM_PROGRESS_IMPORT_OK) {
            out_envelope->kind = THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION;
            out_envelope->progression = progress_receipt;
            return THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION;
        }

        /* Distinguish "the bytes inflated fine but the body is a real
         * Sphenx/Greatstone custom format we don't decode yet" from "the
         * body was corrupt" by checking how the party decoder treated
         * the same bytes.  UNSUPPORTED_BODY on either decoder is a real
         * (unknown) body; the other statuses are body-shape problems. */
        if (party_status == THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY ||
            prog_status == THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY) {
            out_envelope->decode_status =
                THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
            out_envelope->kind = THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED;
            return THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED;
        }
    }

    return THERON_V1_SRM_ENVELOPE_KIND_NONE;
#endif
}

Theron_V1SrmEnvelopeKind theron_v1_srm_decode_path(
    const char *path,
    int slot_index,
    uint8_t *scratch,
    size_t scratch_capacity,
    Theron_V1SrmEnvelopeReceipt *out_envelope) {

#if defined(FIRESTAFF_THERON_PRODUCTION)
    (void)path;
    (void)scratch;
    (void)scratch_capacity;
    if (out_envelope) {
        memset(out_envelope, 0, sizeof(*out_envelope));
        out_envelope->slot_index = slot_index;
        out_envelope->kind = THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED;
        out_envelope->decode_status =
            THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY;
    }
    return THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED;
#else

    struct stat st;
    uint8_t *srm_buf = NULL;
    size_t srm_size = 0;
    Theron_V1SrmEnvelopeKind kind = THERON_V1_SRM_ENVELOPE_KIND_NONE;

    if (!out_envelope) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }
    memset(out_envelope, 0, sizeof(*out_envelope));
    out_envelope->slot_index = slot_index;
    out_envelope->inflate_status = THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT;
    out_envelope->decode_status = THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT;

    if (!path || !path[0]) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }

    if (scratch_capacity > THERON_V1_SRM_BODY_DECODE_MAX_BYTES) {
        scratch_capacity = THERON_V1_SRM_BODY_DECODE_MAX_BYTES;
    }

    srm_size = (size_t)st.st_size;
    if (srm_size == 0 || srm_size > scratch_capacity) {
        /* The .srm file is too large to inflate safely inside our
         * work budget; honor the bound and report BAD_INPUT rather
         * than try to truncate or re-allocate.  This is honest and
         * matches the documented "bounded" envelope contract. */
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }

    srm_buf = (uint8_t *)malloc(srm_size);
    if (!srm_buf) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }

    {
        FILE *fp = fopen(path, "rb");
        if (!fp) {
            free(srm_buf);
            return THERON_V1_SRM_ENVELOPE_KIND_NONE;
        }
        size_t got = fread(srm_buf, 1, srm_size, fp);
        fclose(fp);
        if (got != srm_size) {
            free(srm_buf);
            return THERON_V1_SRM_ENVELOPE_KIND_NONE;
        }
    }

    /* Once we have read the file, the envelope owns the file path in
     * the receipt; route through the buffer decoder for the actual
     * inflate + magic + decode chain. */
    kind = theron_v1_srm_decode_envelope(srm_buf, srm_size,
                                          scratch, scratch_capacity,
                                          out_envelope);
    {
        size_t n = strlen(path);
        if (n >= THERON_V1_SRM_PATH_MAX) n = THERON_V1_SRM_PATH_MAX - 1;
        memcpy(out_envelope->source_path, path, n);
        out_envelope->source_path[n] = '\0';
    }
    out_envelope->slot_index = slot_index;
    out_envelope->file_size = (uint64_t)srm_size;
    free(srm_buf);
    return kind;
#endif
}

int theron_v1_srm_catalog_body_evidence(
    const Theron_V1SrmEnvelopeReceipt *envelopes,
    size_t envelope_count,
    Theron_V1SrmBodyEvidenceCatalog *out_catalog) {

    uint32_t checksum = 2166136261u;
    size_t i;

    if (out_catalog) {
        memset(out_catalog, 0, sizeof(*out_catalog));
        out_catalog->first_authenticated_slot = -1;
        for (i = 0u; i < THERON_V1_SRM_DISK_SLOT_COUNT; ++i) {
            out_catalog->fingerprint_group_for_slot[i] = -1;
            out_catalog->first_slot_for_group[i] = -1;
        }
    }
    if (!envelopes || !out_catalog ||
        envelope_count > THERON_V1_SRM_DISK_SLOT_COUNT) {
        return 0;
    }

    for (i = 0u; i < envelope_count; ++i) {
        const Theron_V1SrmEnvelopeReceipt *envelope = &envelopes[i];
        const Theron_V1SrmBodyEvidence *evidence = &envelope->body_evidence;
        int group = -1;
        size_t prior;

        if (!evidence->captured || !evidence->gzip_trailer_verified) {
            continue;
        }
        if (envelope->slot_index < 0 ||
            envelope->slot_index >= THERON_V1_SRM_DISK_SLOT_COUNT ||
            out_catalog->fingerprint_group_for_slot[envelope->slot_index] >= 0) {
            return 0;
        }

        for (prior = 0u; prior < i; ++prior) {
            const Theron_V1SrmEnvelopeReceipt *prior_envelope = &envelopes[prior];
            const Theron_V1SrmBodyEvidence *prior_evidence =
                &prior_envelope->body_evidence;
            if (!prior_evidence->captured ||
                !prior_evidence->gzip_trailer_verified ||
                prior_envelope->slot_index < 0 ||
                prior_envelope->slot_index >= THERON_V1_SRM_DISK_SLOT_COUNT) {
                continue;
            }
            if (prior_evidence->payload_size == evidence->payload_size &&
                prior_evidence->payload_checksum32 == evidence->payload_checksum32 &&
                prior_evidence->payload_prefix_checksum32 ==
                    evidence->payload_prefix_checksum32 &&
                prior_evidence->payload_suffix_checksum32 ==
                    evidence->payload_suffix_checksum32 &&
                prior_evidence->gzip_trailer_crc32 == evidence->gzip_trailer_crc32 &&
                prior_evidence->gzip_trailer_isize == evidence->gzip_trailer_isize) {
                group = out_catalog->fingerprint_group_for_slot[
                    prior_envelope->slot_index];
                break;
            }
        }
        if (group < 0) {
            group = (int)out_catalog->fingerprint_group_count++;
            out_catalog->first_slot_for_group[group] = envelope->slot_index;
        }
        out_catalog->fingerprint_group_for_slot[envelope->slot_index] = group;
        if (out_catalog->first_authenticated_slot < 0 ||
            envelope->slot_index < out_catalog->first_authenticated_slot) {
            out_catalog->first_authenticated_slot = envelope->slot_index;
        }
        ++out_catalog->authenticated_slot_count;
        checksum ^= (uint32_t)envelope->slot_index;
        checksum *= 16777619u;
        checksum ^= (uint32_t)group;
        checksum *= 16777619u;
        checksum ^= (uint32_t)evidence->payload_size;
        checksum *= 16777619u;
        checksum ^= evidence->payload_checksum32;
        checksum *= 16777619u;
        checksum ^= evidence->payload_prefix_checksum32;
        checksum *= 16777619u;
        checksum ^= evidence->payload_suffix_checksum32;
        checksum *= 16777619u;
    }
    out_catalog->catalog_checksum = checksum;
    return 1;
}

static int srm_body_evidence_matches(const Theron_V1SrmBodyEvidence *left,
                                     const Theron_V1SrmBodyEvidence *right) {
    return left->payload_size == right->payload_size &&
           left->payload_checksum32 == right->payload_checksum32 &&
           left->payload_prefix_checksum32 == right->payload_prefix_checksum32 &&
           left->payload_suffix_checksum32 == right->payload_suffix_checksum32 &&
           left->gzip_trailer_crc32 == right->gzip_trailer_crc32 &&
           left->gzip_trailer_isize == right->gzip_trailer_isize;
}

static int srm_body_evidence_index_slots(
    const Theron_V1SrmEnvelopeReceipt *envelopes,
    size_t envelope_count,
    const Theron_V1SrmEnvelopeReceipt *by_slot[THERON_V1_SRM_DISK_SLOT_COUNT],
    unsigned int *out_mask) {
    size_t i;

    if (!envelopes || !by_slot || !out_mask ||
        envelope_count > THERON_V1_SRM_DISK_SLOT_COUNT) {
        return 0;
    }
    memset(by_slot, 0, sizeof(*by_slot) * THERON_V1_SRM_DISK_SLOT_COUNT);
    *out_mask = 0u;
    for (i = 0u; i < envelope_count; ++i) {
        const Theron_V1SrmEnvelopeReceipt *envelope = &envelopes[i];
        const Theron_V1SrmBodyEvidence *evidence = &envelope->body_evidence;
        unsigned int bit;

        if (!evidence->captured || !evidence->gzip_trailer_verified) continue;
        if (envelope->slot_index < 0 ||
            envelope->slot_index >= THERON_V1_SRM_DISK_SLOT_COUNT ||
            by_slot[envelope->slot_index] != NULL) {
            return 0;
        }
        by_slot[envelope->slot_index] = envelope;
        bit = 1u << (unsigned int)envelope->slot_index;
        *out_mask |= bit;
    }
    return 1;
}

int theron_v1_srm_compare_body_evidence(
    const Theron_V1SrmEnvelopeReceipt *left_envelopes,
    size_t left_envelope_count,
    const Theron_V1SrmEnvelopeReceipt *right_envelopes,
    size_t right_envelope_count,
    Theron_V1SrmBodyEvidenceComparison *out_comparison) {
    const Theron_V1SrmEnvelopeReceipt *left_by_slot[THERON_V1_SRM_DISK_SLOT_COUNT];
    const Theron_V1SrmEnvelopeReceipt *right_by_slot[THERON_V1_SRM_DISK_SLOT_COUNT];
    uint32_t checksum = 2166136261u;
    unsigned int left_mask;
    unsigned int right_mask;
    int slot;

    if (!out_comparison ||
        !srm_body_evidence_index_slots(left_envelopes, left_envelope_count,
                                       left_by_slot, &left_mask) ||
        !srm_body_evidence_index_slots(right_envelopes, right_envelope_count,
                                       right_by_slot, &right_mask)) {
        return 0;
    }
    memset(out_comparison, 0, sizeof(*out_comparison));
    out_comparison->left_authenticated_slot_mask = left_mask;
    out_comparison->right_authenticated_slot_mask = right_mask;
    for (slot = 0; slot < THERON_V1_SRM_DISK_SLOT_COUNT; ++slot) {
        const Theron_V1SrmBodyEvidence *left;
        const Theron_V1SrmBodyEvidence *right;
        unsigned int bit = 1u << (unsigned int)slot;

        if (!(left_mask & bit) || !(right_mask & bit)) continue;
        left = &left_by_slot[slot]->body_evidence;
        right = &right_by_slot[slot]->body_evidence;
        out_comparison->comparable_slot_mask |= bit;
        if (srm_body_evidence_matches(left, right)) {
            out_comparison->matching_slot_mask |= bit;
        } else {
            out_comparison->mismatch_slot_mask |= bit;
        }
        checksum ^= bit;
        checksum *= 16777619u;
        checksum ^= left->payload_checksum32;
        checksum *= 16777619u;
        checksum ^= right->payload_checksum32;
        checksum *= 16777619u;
    }
    out_comparison->comparison_checksum = checksum;
    return 1;
}

static int srm_payload_matches_evidence(
    const Theron_V1SrmEnvelopeReceipt *envelope,
    const uint8_t *payload,
    size_t payload_size) {
    const Theron_V1SrmBodyEvidence *evidence;
    size_t sample_size;

    if (!envelope || !payload) return 0;
    evidence = &envelope->body_evidence;
    if (!evidence->captured || !evidence->gzip_trailer_verified ||
        payload_size != evidence->payload_size) {
        return 0;
    }
    /* Keep this in lockstep with theron_v1_srm_capture_body_evidence().
     * The manifest's on-disk prefix sample is wider; body evidence uses
     * 16-byte payload ends so two authenticated receipts can be compared
     * without retaining the body in runtime state. */
    sample_size = payload_size < 16u ? payload_size : 16u;
    return rolling_checksum32(payload, payload_size) ==
               evidence->payload_checksum32 &&
           rolling_checksum32(payload, sample_size) ==
               evidence->payload_prefix_checksum32 &&
           rolling_checksum32(payload + payload_size - sample_size,
                              sample_size) ==
               evidence->payload_suffix_checksum32;
}

int theron_v1_srm_compare_authenticated_payloads(
    const Theron_V1SrmEnvelopeReceipt *left_envelope,
    const uint8_t *left_payload,
    size_t left_payload_size,
    const Theron_V1SrmEnvelopeReceipt *right_envelope,
    const uint8_t *right_payload,
    size_t right_payload_size,
    Theron_V1SrmBodyEvidenceDelta *out_delta) {
    size_t shared_size;
    size_t i;
    uint32_t checksum = 2166136261u;

    if (!out_delta ||
        !srm_payload_matches_evidence(left_envelope, left_payload,
                                      left_payload_size) ||
        !srm_payload_matches_evidence(right_envelope, right_payload,
                                      right_payload_size)) {
        return 0;
    }
    memset(out_delta, 0, sizeof(*out_delta));
    out_delta->left_payload_size = left_payload_size;
    out_delta->right_payload_size = right_payload_size;
    shared_size = left_payload_size < right_payload_size
        ? left_payload_size : right_payload_size;

    while (out_delta->shared_prefix_bytes < shared_size &&
           left_payload[out_delta->shared_prefix_bytes] ==
               right_payload[out_delta->shared_prefix_bytes]) {
        ++out_delta->shared_prefix_bytes;
    }
    while (out_delta->shared_suffix_bytes <
               shared_size - out_delta->shared_prefix_bytes &&
           left_payload[left_payload_size - 1u -
                        out_delta->shared_suffix_bytes] ==
               right_payload[right_payload_size - 1u -
                             out_delta->shared_suffix_bytes]) {
        ++out_delta->shared_suffix_bytes;
    }
    for (i = out_delta->shared_prefix_bytes;
         i < shared_size - out_delta->shared_suffix_bytes;) {
        size_t run_start;
        size_t run_size;
        if (left_payload[i] == right_payload[i]) {
            ++i;
            continue;
        }
        run_start = i;
        do {
            ++i;
        } while (i < shared_size - out_delta->shared_suffix_bytes &&
                 left_payload[i] != right_payload[i]);
        run_size = i - run_start;
        out_delta->differing_byte_count += run_size;
        ++out_delta->differing_run_count;
        if (run_size > out_delta->longest_differing_run_bytes) {
            out_delta->longest_differing_run_bytes = run_size;
        }
        if (out_delta->retained_run_count < THERON_V1_SRM_BODY_DELTA_MAX_RUNS) {
            out_delta->differing_runs[out_delta->retained_run_count].offset = run_start;
            out_delta->differing_runs[out_delta->retained_run_count].byte_count = run_size;
            ++out_delta->retained_run_count;
        } else {
            out_delta->differing_runs_truncated = 1;
        }
    }
    if (left_payload_size != right_payload_size) {
        size_t tail_size = left_payload_size > right_payload_size
            ? left_payload_size - shared_size : right_payload_size - shared_size;
        out_delta->differing_byte_count += tail_size;
        ++out_delta->differing_run_count;
        if (tail_size > out_delta->longest_differing_run_bytes) {
            out_delta->longest_differing_run_bytes = tail_size;
        }
        if (out_delta->retained_run_count < THERON_V1_SRM_BODY_DELTA_MAX_RUNS) {
            out_delta->differing_runs[out_delta->retained_run_count].offset = shared_size;
            out_delta->differing_runs[out_delta->retained_run_count].byte_count = tail_size;
            ++out_delta->retained_run_count;
        } else {
            out_delta->differing_runs_truncated = 1;
        }
    }

    checksum ^= (uint32_t)left_payload_size;
    checksum *= 16777619u;
    checksum ^= (uint32_t)right_payload_size;
    checksum *= 16777619u;
    checksum ^= (uint32_t)out_delta->shared_prefix_bytes;
    checksum *= 16777619u;
    checksum ^= (uint32_t)out_delta->shared_suffix_bytes;
    checksum *= 16777619u;
    checksum ^= (uint32_t)out_delta->differing_byte_count;
    checksum *= 16777619u;
    checksum ^= (uint32_t)out_delta->differing_run_count;
    checksum *= 16777619u;
    checksum ^= (uint32_t)out_delta->longest_differing_run_bytes;
    checksum *= 16777619u;
    checksum ^= (uint32_t)out_delta->differing_runs_truncated;
    checksum *= 16777619u;
    for (i = 0u; i < out_delta->retained_run_count; ++i) {
        checksum ^= (uint32_t)out_delta->differing_runs[i].offset;
        checksum *= 16777619u;
        checksum ^= (uint32_t)out_delta->differing_runs[i].byte_count;
        checksum *= 16777619u;
    }
    out_delta->delta_checksum = checksum;
    return 1;
}

Theron_V1SrmEnvelopeKind theron_v1_srm_probe_slot0_envelope(
    Theron_V1SrmEnvelopeReceipt *out_envelope) {

    char root[THERON_V1_SRM_PATH_MAX] = {0};
    char path[THERON_V1_SRM_PATH_MAX] = {0};
    uint8_t scratch[THERON_V1_SRM_BODY_DECODE_MAX_BYTES];
    struct stat st;

    /* Stat the file first so we can report SKIP for "save-disk root
     * present, slot0 absent" as cleanly as "save-disk root absent".
     * Both are honest skip-clean outcomes: the gate only emits a
     * typed envelope kind when an actual .srm file was staged. */
    if (!theron_v1_srm_default_root(root)) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }
    if (!theron_v1_srm_slot_path(root, 0, path)) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return THERON_V1_SRM_ENVELOPE_KIND_NONE;
    }

    return theron_v1_srm_decode_path(path, 0, scratch,
                                     sizeof(scratch), out_envelope);
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
    case THERON_V1_SRM_PAYLOAD_PROBE_TRAILER_MISMATCH:
        return "TRAILER_MISMATCH";
    case THERON_V1_SRM_PAYLOAD_PROBE_TRAILING_DATA:
        return "TRAILING_DATA";
    }
    return "UNKNOWN";
}

const char *theron_v1_srm_envelope_kind_name(Theron_V1SrmEnvelopeKind kind) {
    switch (kind) {
    case THERON_V1_SRM_ENVELOPE_KIND_NONE:
        return "NONE";
    case THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION:
        return "PROGRESSION";
    case THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY:
        return "PROGRESSION_PARTY";
    case THERON_V1_SRM_ENVELOPE_KIND_UNSUPPORTED:
        return "UNSUPPORTED";
    }
    return "UNKNOWN";
}

const char *theron_v1_srm_progress_import_status_name(Theron_V1SrmProgressImportStatus status) {
    switch (status) {
    case THERON_V1_SRM_PROGRESS_IMPORT_OK:
        return "OK";
    case THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_BODY:
        return "UNSUPPORTED_BODY";
    case THERON_V1_SRM_PROGRESS_IMPORT_BAD_INPUT:
        return "BAD_INPUT";
    case THERON_V1_SRM_PROGRESS_IMPORT_UNSUPPORTED_VERSION:
        return "UNSUPPORTED_VERSION";
    case THERON_V1_SRM_PROGRESS_IMPORT_OUT_OF_RANGE:
        return "OUT_OF_RANGE";
    case THERON_V1_SRM_PROGRESS_IMPORT_NON_MONOTONIC_QUEST_STATE:
        return "NON_MONOTONIC_QUEST_STATE";
    }
    return "UNKNOWN";
}

const char *theron_v1_srm_source_evidence(void) {
#if defined(FIRESTAFF_THERON_PRODUCTION)
    return
        "Theron original-save boundary: authentic 2 KiB PC Engine HUBM "
        "Backup RAM only; Firestaff envelope decoders are fixture-only";
#else
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
        "Status (2026-09-23):\n"
        "  - Authentic 2 KiB PC Engine HUBM Backup RAM is classified\n"
        "    separately from gzip SRM and Firestaff TQSV. DMS-SG.001 at\n"
        "    HUBM offset $16 has a $0199-byte data area: three $88-byte\n"
        "    slots plus the selected-slot index. The selected slot begins\n"
        "    with the authenticated $86-byte writer body from RAM $267C,\n"
        "    whose first byte is the serialized campaign byte. Only its\n"
        "    independently\n"
        "    source-bound low seven artifact bits may be applied; all other\n"
        "    bytes remain preserved but opaque.\n"
        "  - Data-free classifier with 5-slot disk manifest, gzip magic\n"
        "    detection (0x1F 0x8B 0x08), 1 KiB rolling prefix checksum,\n"
        "    present/recognized rollup, and stable string status names.\n"
        "  - Bounded gzip-payload probe inflates a recognized .srm stream\n"
        "    when Firestaff is built with zlib; without zlib it reports\n"
        "    ZLIB_UNAVAILABLE after the cheap gzip/method checks.\n"
        "  - Recognized-slot probe coverage now performs a bounded full-file\n"
        "    read, inflates a gzipped synthetic `.srm` party body, and decodes\n"
        "    one champion/body block through the readiness importer.\n"
        "  - Bounded progression-envelope import maps an inflated\n"
        "    Firestaff readiness payload into Theron_DungeonProgression\n"
        "    with T080/T800 between-dungeon sequence validation.  Unknown\n"
        "    real Sphenx/Greatstone custom bodies return UNSUPPORTED_BODY.\n"
        "  - Bounded party-envelope import maps a Firestaff readiness body\n"
        "    into Theron_DungeonProgression plus Theron_V1_Party body fields\n"
        "    only; inventory/equipment and real Sphenx body decoding remain\n"
        "    explicitly unsupported.\n"
        "  - 2026-06-30 body-decode envelope surface landed: a single\n"
        "    theron_v1_srm_decode_envelope() + theron_v1_srm_decode_path()\n"
        "    pair run the bounded (gzip magic -> inflate -> envelope\n"
        "    magic -> decode) chain end-to-end, returning a typed\n"
        "    Theron_V1SrmEnvelopeKind receipt that carries the restored\n"
        "    Theron_DungeonProgression plus the optional party body.\n"
        "    theron_v1_srm_probe_slot0_envelope() is the skip-clean\n"
        "    real-artifact entry point; when no .srm is staged it\n"
        "    returns ENVELOPE_KIND_NONE without disturbing the\n"
        "    caller's envelope buffer.\n"
        "  - Default save-disk root: $HOME/.firestaff/data/theron/save.\n"
        "  - Override: env FIRESTAFF_THERON_SRM_DIR.\n"
        "  - A missing operator-owned artifact remains a clean probe SKIP;\n"
        "    no test substitutes generated campaign data for a real file.\n"
        "  - Interpreting the real inflated custom Theron save body,\n"
        "    inventory/equipment bytes, and Sphenx champion byte mapping\n"
        "    remains out of scope and is tracked under\n"
        "    docs/FIRESTAFF_GAP_LIST.md A3 'Savegame format (Theron .SRM)'.";
#endif
}
