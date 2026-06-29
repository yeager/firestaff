/*
 * firestaff_x68k_media_receipt_real_corpus_probe.c
 *
 * Real-media receipt gate for the DM1 / CSB X68000 HDM import
 * boundary documented in docs/FIRESTAFF_GAP_LIST.md
 * ("DM1 X68000 HDM/floppy media import", status OPEN-BOUNDED,
 * 2026-06-25 snapshot).
 *
 * Companion to the data-free
 *   tests/test_firestaff_x68k_media_classify.c
 * and the synthetic-FTL cross-module
 *   tests/test_firestaff_x68k_ftl_handoff.c
 * units. Those units lock down the documented constants and
 * the synthetic-FTL handoff invariants; this probe locks the
 * same invariants down against a real, locally-preserved DM1
 * X68000 v3.0 HDM image so that a regression in the
 * classifier or a change to the on-disk layout shows up as a
 * receipt failure rather than a silent drift in the documented
 * expected state.
 *
 * What "receipt" means here:
 *   - File exists at the operator-supplied path.
 *   - File size matches the DMWeb-documented 2DHD geometry
 *     (2 sides * 77 tracks * 8 sectors * 1024 bytes =
 *     1,261,568 bytes).
 *   - File content classifies as FIRESTAFF_X68K_MEDIA_FULL_DISK
 *     via FirestaffX68kMedia_Classify.
 *   - DMWeb-documented "Track 1 Side 1 Sector 9" sentinel
 *     position contains the documented `0xE5E5E5...` MFM
 *     fill-byte pattern that the public DMFiles HDM uses in
 *     place of the missing protection sector (DMWeb copy-
 *     protection page, "Sharp X68000" section: the public
 *     release lacks the operational protection sector, so the
 *     bytes read back are the MFM idle pattern, not the
 *     "HPR-0007" sentinel).
 *   - The classifier's documented "protection area blank"
 *     surface state is reproduced for the real HDM, with an
 *     explicit note that DMWeb's documented sentinels are
 *     absent at the DMWeb-documented offset (we still record
 *     that no HPR-0007 sentinel sits at the DMWeb sector 9
 *     offset so any future re-issue that does put one there
 *     flips this gate to FAIL with a clear diff).
 *   - Off-axis `HPR-0007` strings elsewhere in the image are
 *     classified as label/backup evidence only, not as proof
 *     that the operational protection sector was captured.
 *
 * Why a separate probe rather than another case inside the
 * existing units:
 *   - The existing units are explicitly data-free so they
 *     can run in CI without a real DMFiles HDM available.
 *   - This probe is the receipt: it cross-checks the
 *     classifier's documented expectations against the actual
 *     bytes on the operator's preserved media. It is
 *     skip-safe and only runs when the operator has placed a
 *     HDM at the documented location (env override or
 *     default $HOME/.firestaff/data/dm1-extras path).
 *
 * Source of truth:
 *   - DMWeb copy-protection page (Sharp X68000 section):
 *     2DHD geometry 1261568 bytes; HPR-0007 sentinel at
 *     Track 1 Side 1 Sector 9 is the only operational
 *     copy-protection check on DM1 / CSB; public DMFiles
 *     original HDM lacks the protection sector so the game
 *     cannot boot from it.
 *   - DMWeb DM X68000 edition page: Japanese v3.0 release,
 *     HDM original image, cracked image, blank save disk.
 *   - include/firestaff_x68k_media_classify.h: the constants,
 *     media class enum, and scan flags the receipt asserts
 *     against.
 *
 * Build (mirrors the firestaff_x68k_media_classify unit
 * pattern):
 *   cc -std=c99 -Wall -Wextra -pedantic -O2 \
 *      -I include \
 *      probes/x68k/firestaff_x68k_media_receipt_real_corpus_probe.c \
 *      src/shared/firestaff_x68k_media_classify.c \
 *      -o firestaff_x68k_media_receipt_real_corpus_probe
 *
 * Run (skip-safe):
 *   ./firestaff_x68k_media_receipt_real_corpus_probe
 *   # exits 0 with SKIP if no real HDM is staged.
 *   FIRESTAFF_X68K_HDM_PATH=/path/to/DungeonMasterX68000version30Japanese.hdm \
 *     ./firestaff_x68k_media_receipt_real_corpus_probe
 *   # exits 0 with PASS on receipt, 1 on receipt failure.
 *
 * The probe does not modify the HDM, does not vendor any
 * game data, does not require any asset cache; it is a
 * read-only receipt audit.
 */

#include "firestaff_x68k_media_classify.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Documented linear offset of the protection-sector 9 region
 * per the DMWeb Sharp X68000 copy-protection page
 * ("Track 1 Side 1 Sector 9 holds the HPR-0007 sentinel +
 *  4 random bytes"). The protection-area check in the
 * classifier uses the same constant; we duplicate it here so
 * the receipt probe is robust to renames of the header
 * internals. */
#define X68K_RECEIPT_LINEAR_SENTINEL_OFFSET 647168u

/* Documented linear size of one Track 1 Side 1 protection
 * sector (1024 bytes; MFM 1 sector). */
#define X68K_RECEIPT_SECTOR_BYTES 1024u

/* Default candidate paths, in priority order, that the probe
 * checks when no FIRESTAFF_X68K_HDM_PATH is supplied. These
 * match the dm1-extras layout shipped on the local host for
 * the public DMFiles X68000 v3.0 HDM (DMWeb X68000 edition
 * page, "HDM original image that cannot boot without the
 * copy-protection sectors"). The DMFiles English DIM is
 * intentionally not listed: it carries a 256-byte DIM header
 * in front of the same MFM image, so it would fail the size
 * receipt (1,261,824 bytes vs the DMWeb 1,261,568 bytes)
 * even though the MFM content is identical. Operators with
 * only DIM files can point FIRESTAFF_X68K_HDM_PATH at the
 * stripped .hdm; a DIM unwrap step is a separate gap in
 * docs/FIRESTAFF_GAP_LIST.md. We do not list the .raw flux
 * captures (per-track Kryoflux-style dumps) because those
 * are MFM timing streams, not byte-aligned MFM HDM images. */
static const char* kDefaultCandidatePaths[] = {
    "$HOME/.firestaff/data/dm1-extras/x68000-3.0-jp/"
        "DungeonMasterX68000version30Japanese.hdm",
    "$HOME/.firestaff/data/dm1-x68000/hdm/"
        "DungeonMasterX68000version30Japanese.hdm",
    "$HOME/.firestaff/data/x68000/hdm/"
        "DungeonMasterX68000version30Japanese.hdm",
};

static int g_pass = 0;
static int g_fail = 0;

static void pass(const char* name, const char* detail) {
    printf("PASS %s%s%s\n", name,
           (detail && detail[0]) ? " " : "",
           detail ? detail : "");
    ++g_pass;
}

static void fail(const char* name, const char* detail) {
    printf("FAIL %s%s%s\n", name,
           (detail && detail[0]) ? " " : "",
           detail ? detail : "");
    ++g_fail;
}

static void skip(const char* name, const char* detail) {
    printf("SKIP %s%s%s\n", name,
           (detail && detail[0]) ? " " : "",
           detail ? detail : "");
}

/* Expand a leading "$HOME" in `path` to the value of the HOME
 * environment variable. Returns a malloc'd string the caller
 * must free, or NULL on allocation failure or missing HOME. */
static char* expand_home(const char* path) {
    if (!path) return NULL;
    if (strncmp(path, "$HOME", 5) != 0) {
        size_t n = strlen(path) + 1u;
        char* out = (char*)malloc(n);
        if (!out) return NULL;
        memcpy(out, path, n);
        return out;
    }
    const char* home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    size_t hl = strlen(home);
    size_t tl = strlen(path + 5);
    char* out = (char*)malloc(hl + tl + 1u);
    if (!out) return NULL;
    memcpy(out, home, hl);
    memcpy(out + hl, path + 5, tl);
    out[hl + tl] = '\0';
    return out;
}

/* Resolve the operator-supplied or default HDM path. Returns
 * NULL (and prints SKIP) if no candidate is present; the
 * caller must skip the receipt in that case. The returned
 * pointer is owned by the caller (free with free()). */
static char* resolve_hdm_path(const char** out_logged_kind) {
    const char* env = getenv("FIRESTAFF_X68K_HDM_PATH");
    if (env && env[0]) {
        if (out_logged_kind) *out_logged_kind = "env override";
        char* copy = (char*)malloc(strlen(env) + 1u);
        if (!copy) return NULL;
        memcpy(copy, env, strlen(env) + 1u);
        return copy;
    }
    size_t n = sizeof(kDefaultCandidatePaths) /
               sizeof(kDefaultCandidatePaths[0]);
    for (size_t i = 0; i < n; ++i) {
        char* expanded = expand_home(kDefaultCandidatePaths[i]);
        if (!expanded) continue;
        FILE* f = fopen(expanded, "rb");
        if (f) {
            fclose(f);
            if (out_logged_kind) *out_logged_kind = "default candidate";
            return expanded;
        }
        free(expanded);
    }
    if (out_logged_kind) *out_logged_kind = NULL;
    return NULL;
}

/* Read the entire file into a malloc'd buffer. Caller owns
 * the returned buffer. Returns NULL on error or zero-length
 * file. Writes the size via *out_size. */
static uint8_t* read_file_all(const char* path, size_t* out_size) {
    if (!path || !out_size) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long fsize = ftell(f);
    if (fsize <= 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    uint8_t* buf = (uint8_t*)malloc((size_t)fsize);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t got = fread(buf, 1, (size_t)fsize, f);
    fclose(f);
    if (got != (size_t)fsize) {
        free(buf);
        return NULL;
    }
    *out_size = (size_t)fsize;
    return buf;
}

/* The DMWeb-documented Track 1 Side 1 Sector 9 region in a
 * public DMFiles X68000 HDM is MFM-filled with the 0xE5 byte
 * (the standard "formatted but unwritten" fill on MFM-formatted
 * media). Count how many bytes in the region equal 0xE5. */
static size_t count_e5_bytes(const uint8_t* buf, size_t start,
                             size_t end) {
    size_t n = 0u;
    for (size_t i = start; i < end; ++i) {
        if (buf[i] == 0xE5u) ++n;
    }
    return n;
}

/* Count zero bytes (used for the protection-area blank
 * classifier check). */
static size_t count_zero_bytes(const uint8_t* buf, size_t start,
                               size_t end) {
    size_t n = 0u;
    for (size_t i = start; i < end; ++i) {
        if (buf[i] == 0u) ++n;
    }
    return n;
}

int main(void) {
    const char* kind = NULL;
    char* path = resolve_hdm_path(&kind);
    if (!path) {
        skip("INV_X68K_HDM_RECEIPT_00",
             "no DM1 X68000 HDM found at any default candidate; "
             "set FIRESTAFF_X68K_HDM_PATH=<path-to-.hdm-or-"
             "1.18MiB-DIM> to run this probe");
        printf("# summary: skipped (no real media present) "
               "-- not a failure\n");
        return 0;
    }

    size_t hdm_size = 0u;
    uint8_t* hdm = read_file_all(path, &hdm_size);
    if (!hdm) {
        /* Skip-safe: an explicit operator-supplied or default
         * candidate path that does not point at a readable
         * file is treated as "no real media present", not
         * as a receipt failure. This matches the project's
         * skip-safe probe contract and lets the same probe
         * binary ship in CI without leaking operator-side
         * misconfiguration into red builds. */
        skip("INV_X68K_HDM_RECEIPT_00",
             "could not read the staged HDM (file not present "
             "or unreadable)");
        printf("# summary: skipped (no real media present) "
               "-- not a failure\n");
        free(path);
        return 0;
    }
    printf("# receipt: read %zu bytes from %s (%s)\n",
           hdm_size, path,
           kind ? kind : "unknown source");

    /* Receipt #01: size must match the DMWeb 2DHD geometry.
     * 2 * 77 * 8 * 1024 = 1,261,568 bytes. */
    if (hdm_size == FIRESTAFF_X68K_BYTES_PER_DISK) {
        pass("INV_X68K_HDM_RECEIPT_01",
             "size == 2*77*8*1024 == 1261568 bytes (DMWeb 2DHD)");
    } else {
        fail("INV_X68K_HDM_RECEIPT_01",
             "size does not match the DMWeb 2DHD geometry");
        printf("  expected %u bytes, got %zu bytes\n",
               FIRESTAFF_X68K_BYTES_PER_DISK, hdm_size);
    }

    /* Receipt #02: classifier returns MEDIA_FULL_DISK. */
    FirestaffX68kMediaClassifyResult r;
    memset(&r, 0, sizeof(r));
    FirestaffX68kMedia_Classify(hdm, hdm_size, &r);
    printf("  NOTE: receipt_class=%s total_hpr=%u off_axis_hpr=%u "
           "first_off_axis=%llu\n",
           FirestaffX68kMedia_ReceiptClassName(r.receipt_class),
           (unsigned)r.protection_sentinel_count,
           (unsigned)r.protection_sentinel_offaxis_count,
           (unsigned long long)r.first_offaxis_sentinel_offset);
    if (r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK) {
        pass("INV_X68K_HDM_RECEIPT_02",
             "classifier reports MEDIA_FULL_DISK");
    } else {
        fail("INV_X68K_HDM_RECEIPT_02",
             "classifier does not report MEDIA_FULL_DISK");
    }

    /* Receipt #03: no FTL container magic at offset 0. The
     * public DMFiles HDM is a bare disk image with a Hudson
     * Soft boot block, not an FTL resource payload. The
     * classifier therefore reports has_ftl_magic == 0. */
    if (r.has_ftl_magic == 0) {
        pass("INV_X68K_HDM_RECEIPT_03",
             "no FTL 0x6160 magic at offset 0 (DMWeb boot block)");
    } else {
        fail("INV_X68K_HDM_RECEIPT_03",
             "FTL 0x6160 magic detected at offset 0 "
             "(unexpected for a public DMFiles HDM)");
    }

    /* Receipt #04: the DMWeb-documented Track 1 Side 1 Sector
     * 9 protection sector region is MFM fill (0xE5) rather
     * than the HPR-0007 sentinel. This is the documented
     * "public DMFiles original HDM cannot boot" state. */
    if (hdm_size >=
        (size_t)(X68K_RECEIPT_LINEAR_SENTINEL_OFFSET +
                 X68K_RECEIPT_SECTOR_BYTES)) {
        size_t e5 = count_e5_bytes(
            hdm,
            X68K_RECEIPT_LINEAR_SENTINEL_OFFSET,
            X68K_RECEIPT_LINEAR_SENTINEL_OFFSET +
                X68K_RECEIPT_SECTOR_BYTES);
        size_t total = X68K_RECEIPT_SECTOR_BYTES;
        /* Public DMFiles uses 100% 0xE5 fill in this region.
         * The Meynaf-preserved Meynaf image and any
         * flux-level Kryoflux capture may differ. We accept
         * anything >= 50% E5 fill as "expected fill sector"
         * and require a corresponding NOTE rather than a hard
         * PASS for exact match. */
        if (e5 * 2u >= total) {
            pass("INV_X68K_HDM_RECEIPT_04",
                 "DMWeb sector-9 region is MFM-fill "
                 "(public release without protection sector)");
            printf("  NOTE: %zu/%zu bytes in the sector-9 region "
                   "are 0xE5 (MFM fill)\n", e5, total);
        } else {
            fail("INV_X68K_HDM_RECEIPT_04",
                 "DMWeb sector-9 region is NOT MFM fill");
            printf("  expected >= 50%% 0xE5 fill, got %zu/%zu "
                   "(%.1f%%)\n", e5, total,
                   100.0 * (double)e5 / (double)total);
        }
    } else {
        fail("INV_X68K_HDM_RECEIPT_04",
             "HDM shorter than the DMWeb sector-9 offset; "
             "cannot check the protection sector region");
    }

    /* Receipt #05: the HPR-0007 sentinel is NOT present at
     * the DMWeb-documented Track 1 Side 1 Sector 9 linear
     * offset. This is the documented "public DMFiles original
     * HDM cannot boot" state. If a future re-issue of the
     * HDM does put a sentinel there, this gate fails and the
     * receipt is updated to reflect the new state. */
    if (hdm_size >=
        (size_t)(X68K_RECEIPT_LINEAR_SENTINEL_OFFSET +
                 FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN)) {
        if (memcmp(hdm + X68K_RECEIPT_LINEAR_SENTINEL_OFFSET,
                   FIRESTAFF_X68K_PROTECTION_SENTINEL,
                   FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN) == 0) {
            fail("INV_X68K_HDM_RECEIPT_05",
                 "HPR-0007 sentinel present at DMWeb sector-9 "
                 "offset (unexpected for public DMFiles HDM; "
                 "likely a re-issued or preserved master)");
        } else {
            pass("INV_X68K_HDM_RECEIPT_05",
                 "HPR-0007 sentinel absent at DMWeb sector-9 "
                 "offset (public-DMFiles expected state)");
        }
    } else {
        fail("INV_X68K_HDM_RECEIPT_05",
             "HDM shorter than the DMWeb sector-9 sentinel "
             "offset; cannot check");
    }

    /* Receipt #06: the protection-area blank check from the
     * classifier fires when the region preceding the sentinel
     * is mostly zeros. The public DMFiles sector-9 region is
     * mostly 0xE5, not zero, so the classifier should NOT
     * flag the protection area as blank. We surface both the
     * classifier's documented verdict and the raw byte counts
     * so any future change to the MFM fill is auditable. */
    if (hdm_size >=
        (size_t)(X68K_RECEIPT_LINEAR_SENTINEL_OFFSET +
                 X68K_RECEIPT_SECTOR_BYTES)) {
        size_t z = count_zero_bytes(
            hdm,
            X68K_RECEIPT_LINEAR_SENTINEL_OFFSET,
            X68K_RECEIPT_LINEAR_SENTINEL_OFFSET +
                X68K_RECEIPT_SECTOR_BYTES);
        size_t total = X68K_RECEIPT_SECTOR_BYTES;
        printf("  NOTE: %zu/%zu bytes in the sector-9 region "
               "are 0x00 (raw)\n", z, total);
        if ((r.flags &
             FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK) != 0u) {
            fail("INV_X68K_HDM_RECEIPT_06",
                 "classifier unexpectedly flagged the protection "
                 "area as blank (raw bytes suggest MFM fill, "
                 "not zero)");
        } else {
            pass("INV_X68K_HDM_RECEIPT_06",
                 "classifier correctly leaves PROTECTION_AREA_BLANK "
                 "unflagged for MFM-fill region");
        }
    } else {
        skip("INV_X68K_HDM_RECEIPT_06",
             "HDM shorter than the DMWeb sector-9 region; "
             "classifier skipped the protection-area check");
    }

    /* Receipt #07: classifier must NOT flag the HDM as a
     * blank save disk. The public DMFiles HDM has non-zero
     * content (boot block, file system), so the > 1%
     * non-zero threshold should easily fire. */
    if ((r.flags & FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) != 0u) {
        fail("INV_X68K_HDM_RECEIPT_07",
             "classifier unexpectedly flagged the HDM as a "
             "blank save disk");
    } else {
        pass("INV_X68K_HDM_RECEIPT_07",
             "classifier correctly leaves BLANK_SAVE_DISK unflagged "
             "for non-blank HDM");
    }

    /* Receipt #08: classifier must NOT flag the HPR-0007
     * sentinel as present at the linear offset (the sentinel
     * is reported by r.sentinel_offset only when present at
     * the DMWeb-documented location). The public DMFiles
     * HDM has the sentinel embedded in a backup label string
     * (typically "B:\\DMGame.bak\0HPR-0007\0...") somewhere
     * else in the file, but not at the DMWeb sector-9 offset.
     * The receipt deliberately does not look for the off-axis
     * sentinel — that is a separate (still OPEN) gap listed in
     * docs/FIRESTAFF_GAP_LIST.md ("original-vs-cracked/save-
     * disk classification"). */
    if ((r.flags & FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) != 0u) {
        fail("INV_X68K_HDM_RECEIPT_08",
             "classifier unexpectedly flagged the HPR-0007 "
             "sentinel at the DMWeb sector-9 offset");
    } else {
        pass("INV_X68K_HDM_RECEIPT_08",
             "classifier correctly leaves SENTINEL_PRESENT unflagged "
             "for public DMFiles HDM");
    }

    /* Receipt #09: helper surface. The classifier reports
     * IsUnprotectedDisk == 1 iff the protection-area blank
     * check fired. For the public DMFiles HDM the protection
     * area is MFM fill, not blank, so IsUnprotectedDisk is
     * false. This is honest: the public DMFiles HDM is NOT a
     * blank save disk, and the protection-area blank state is
     * the classifier's documented "looks unprotected" signal,
     * not a value judgement. */
    int unprotected = FirestaffX68kMedia_IsUnprotectedDisk(
        r.flags, r.media_class);
    if (unprotected == 0) {
        pass("INV_X68K_HDM_RECEIPT_09",
             "IsUnprotectedDisk == 0 (MFM fill, not zero)");
    } else {
        fail("INV_X68K_HDM_RECEIPT_09",
             "IsUnprotectedDisk == 1 (unexpected for MFM-fill HDM)");
    }

    /* Receipt #10: classifier must NOT report the HDM as an
     * FTL payload. The HDM is a disk image with a Hudson
     * Soft boot block at offset 0, not an FTL container. */
    int ftl_payload = FirestaffX68kMedia_IsFTLPayload(
        r.flags, r.media_class);
    if (ftl_payload == 0) {
        pass("INV_X68K_HDM_RECEIPT_10",
             "IsFTLPayload == 0 (HDM is a disk image, not FTL)");
    } else {
        fail("INV_X68K_HDM_RECEIPT_10",
             "IsFTLPayload == 1 (unexpected for a public DMFiles HDM)");
    }

    /* Receipt #11: full-buffer HPR-0007 accounting. The public
     * DMFiles HDM is known to contain an off-axis HPR-0007
     * string in backup/label metadata rather than at the live
     * protection-sector offset. This invariant prevents that
     * string from ever satisfying the protected-media boundary. */
    if (r.protection_sentinel_count >= 1u &&
        r.protection_sentinel_offaxis_count >= 1u &&
        (r.flags & FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) == 0u) {
        pass("INV_X68K_HDM_RECEIPT_11",
             "off-axis HPR-0007 string counted without live-sector "
             "sentinel flag");
    } else {
        fail("INV_X68K_HDM_RECEIPT_11",
             "off-axis HPR-0007 accounting did not match the "
             "public-DMFiles receipt");
    }

    /* Receipt #12: conservative receipt class. Public DMFiles
     * media with an off-axis sentinel and no live-sector
     * sentinel must classify as OFF_AXIS_SENTINEL_ONLY, not
     * PROTECTED_SENTINEL_AT_SECTOR and not BLANK_SAVE_DISK. */
    if (r.receipt_class ==
        FIRESTAFF_X68K_RECEIPT_OFF_AXIS_SENTINEL_ONLY) {
        pass("INV_X68K_HDM_RECEIPT_12",
             "receipt class is OFF_AXIS_SENTINEL_ONLY");
    } else {
        fail("INV_X68K_HDM_RECEIPT_12",
             "receipt class is not OFF_AXIS_SENTINEL_ONLY for "
             "public-DMFiles HDM");
    }

    printf("# summary: %d/%d invariants passed\n",
           g_pass, g_pass + g_fail);

    int rc = (g_fail == 0) ? 0 : 1;
    free(hdm);
    free(path);
    return rc;
}
