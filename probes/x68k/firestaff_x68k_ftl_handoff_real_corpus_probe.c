/*
 * firestaff_x68k_ftl_handoff_real_corpus_probe.c
 *
 * Real-corpus receipt gate for the X68000 HDM <-> FTL container
 * handoff boundary declared in
 * include/firestaff_x68k_media_classify.h.
 *
 * Companion to the data-free / synthetic-FTL
 *   tests/test_firestaff_x68k_ftl_handoff.c
 * unit. The unit verifies the handoff math against an
 * in-memory synthetic FTL container; this probe verifies the
 * same handoff math against a real, locally-preserved DM1
 * X68000 v3.0 HDM image. The public DMFiles HDM embeds the
 * X68000 graphics / SWSH / palette FTL resources as
 * concatenated `.FTL` blobs at known offsets in the disk
 * image. The probe locates the FTL magic (0x6160 big-endian,
 * greatstone d_ftl.html "20-byte common header") inside the
 * real HDM using the classifier's explicit windowed scanner,
 * parses embedded FTLs, verifies their BSS area_1 sizes fit
 * the on-disk media class, and records the documented
 * checksums for both parseable real-media modules.
 *
 * Source of truth:
 *   - greatstone d_ftl.html "20-byte common header" magic
 *     0x6160 big-endian; hunk_count at offset 18; checksum
 *     Note 1.
 *   - dmweb-free.fr/community/documentation/copy-protection,
 *     "Sharp X68000" section: 2DHD geometry 1261568 bytes.
 *   - docs/FIRESTAFF_GAP_LIST.md "DM1 X68000 HDM/floppy
 *     media import" gap row.
 *   - include/firestaff_ftl_container.h: FTL parser contract.
 *   - include/firestaff_x68k_media_classify.h: media
 *     classifier contract and FTLHandoffFits() helper.
 *
 * What the probe deliberately does NOT do:
 *   - It does NOT extract FTL payloads to disk.
 *   - It does NOT establish the 0x5223 HUNK_CODE compression
 *     format: neither parseable CODE hunk in the staged HDM
 *     begins with that signature.
 *   - It does NOT interpret FTL resources or execute 68k code.
 *     The optional local FTLCODE reference classifies the
 *     11-byte CODE modification in the cracked module only;
 *     it does not classify the whole disk as original,
 *     cracked, or save-disk media. The DMWeb page is explicit
 *     that the public DMFiles HDM lacks the protection sector;
 *     that receipt is handled by
 *     firestaff_x68k_media_receipt_real_corpus_probe.
 *
 * Skip-safe: the probe exits 0 with a SKIP message when no
 * real HDM is staged; CTest stays green on hosts without
 * the public DMFiles X68000 v3.0 HDM.
 *
 * Build:
 *   cc -std=c99 -Wall -Wextra -pedantic -O2 \
 *      -I include \
 *      probes/x68k/firestaff_x68k_ftl_handoff_real_corpus_probe.c \
 *      src/shared/firestaff_x68k_media_classify.c \
 *      src/shared/firestaff_ftl_container.c \
 *      -o firestaff_x68k_ftl_handoff_real_corpus_probe
 */

#include "firestaff_ftl_container.h"
#include "firestaff_x68k_media_classify.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Documented linear offset of the protection-sector 9 region
 * per the DMWeb Sharp X68000 copy-protection page. Same as
 * the receipt probe. Duplicated here so the probe is robust
 * to header-internal renames. */
#define X68K_HDM_LINEAR_SENTINEL_OFFSET 647168u
#define X68K_HDM_SECTOR_BYTES 1024u

/* Maximum number of FTL magic candidates the probe will
 * inspect per HDM. The real DMFiles X68000 HDM has 8 raw
 * 0x6160 hits but only 2 parseable FTL containers; the rest
 * are ordinary instruction/data words that happen to encode
 * 0x6160. We scan every candidate up to this cap so future
 * real media that genuinely embeds more FTL resources can be
 * classified without bumping the cap. */
#define X68K_HDM_MAX_FTL_CANDIDATES 64u

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

/* Default candidate paths, in priority order. Mirrors the
 * receipt probe's candidate list. The DMFiles English DIM
 * is intentionally omitted (256-byte DIM header changes
 * the on-disk byte alignment). */
static const char* kDefaultCandidatePaths[] = {
    "$HOME/.firestaff/data/dm1-extras/x68000-3.0-jp/"
        "DungeonMasterX68000version30Japanese.hdm",
    "$HOME/.firestaff/data/dm1-x68000/hdm/"
        "DungeonMasterX68000version30Japanese.hdm",
    "$HOME/.firestaff/data/x68000/hdm/"
        "DungeonMasterX68000version30Japanese.hdm",
};

/* Optional original-data positive control; never copied into the repository.
 * CI hosts without this private preservation/reference file remain skip-safe. */
static const char* kDefaultOriginalFtlPaths[] = {
    "$HOME/Reference/Original/X30J/FTLCODE",
    "$HOME/redmcsb/Reference/Original/X30J/FTLCODE",
    "$HOME/ReDMCSB/Reference/Original/X30J/FTLCODE",
};

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

static char* resolve_original_ftl_path(void) {
    const char* env = getenv("FIRESTAFF_X68K_ORIGINAL_FTL_PATH");
    if (env && env[0]) {
        char* copy = (char*)malloc(strlen(env) + 1u);
        if (!copy) return NULL;
        memcpy(copy, env, strlen(env) + 1u);
        return copy;
    }
    size_t n = sizeof(kDefaultOriginalFtlPaths) /
               sizeof(kDefaultOriginalFtlPaths[0]);
    for (size_t i = 0; i < n; ++i) {
        char* expanded = expand_home(kDefaultOriginalFtlPaths[i]);
        if (!expanded) continue;
        FILE* f = fopen(expanded, "rb");
        if (f) {
            fclose(f);
            return expanded;
        }
        free(expanded);
    }
    return NULL;
}

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

/* Try to parse a real FTL container at every offset that
 * carries the documented 0x6160 big-endian magic. Stops
 * after `max_candidates` raw magic hits so the probe stays
 * bounded on real HDMs. Records the first parseable
 * candidate's bss.data_area1_memory_size via *out_area1
 * (or 0 if none parse). Returns the number of parseable
 * FTL containers found. */
typedef struct {
    size_t modules_with_checksums;
    size_t common_matches;
    size_t bss_matches;
    size_t data_matches;
    size_t code_matches;
    size_t code_mismatches;
    size_t code_unsupported;
    const uint8_t* cracked_code_mismatch;
    size_t cracked_code_mismatch_size;
} X68kFtlChecksumReceipt;

static size_t scan_ftl_candidates(const uint8_t* hdm,
                                  size_t hdm_size,
                                  size_t max_candidates,
                                  uint32_t* out_area1,
                                  size_t* out_first_offset,
                                  X68kFtlChecksumReceipt* checksum_receipt) {
    if (!hdm || !out_area1 || !out_first_offset || !checksum_receipt) return 0u;
    *out_area1 = 0u;
    *out_first_offset = 0u;
    memset(checksum_receipt, 0, sizeof(*checksum_receipt));
    size_t parseable = 0u;
    size_t inspected = 0u;
    for (size_t off = 0u;
         off + 1u < hdm_size && inspected < max_candidates;
         ++off) {
        if (hdm[off] != 0x61u || hdm[off + 1u] != 0x60u) continue;
        ++inspected;
        FirestaffFtlContainer ftl;
        int rc = FirestaffFtlContainer_Parse(
            hdm + off, hdm_size - off, &ftl);
        if (rc != 0) continue;
        ++parseable;
        if (ftl.code_index < 0 ||
            !ftl.hunks[ftl.code_index].payload ||
            ftl.hunks[ftl.code_index].size < 2u) {
            fprintf(stderr,
                    "FAIL: parsed X68000 FTL at offset %zu has no bounded CODE payload\n",
                    off);
            ++g_fail;
            continue;
        }
        FirestaffFtlChecksumReport sums;
        memset(&sums, 0, sizeof(sums));
        if (FirestaffFtlContainer_VerifyChecksums(hdm + off, hdm_size - off,
                                                  &ftl, &sums) == 0 &&
            sums.has_required_bss_data_code) {
            ++checksum_receipt->modules_with_checksums;
            if (sums.common_header.status == FIRESTAFF_FTL_CHECK_MATCH)
                ++checksum_receipt->common_matches;
            if (sums.bss_hunk.status == FIRESTAFF_FTL_CHECK_MATCH)
                ++checksum_receipt->bss_matches;
            if (sums.data_hunk.status == FIRESTAFF_FTL_CHECK_MATCH)
                ++checksum_receipt->data_matches;
            if (sums.code_hunk.status == FIRESTAFF_FTL_CHECK_MATCH)
                ++checksum_receipt->code_matches;
            else if (sums.code_hunk.status == FIRESTAFF_FTL_CHECK_MISMATCH) {
                ++checksum_receipt->code_mismatches;
                checksum_receipt->cracked_code_mismatch =
                    ftl.hunks[ftl.code_index].payload;
                checksum_receipt->cracked_code_mismatch_size =
                    ftl.hunks[ftl.code_index].size;
            }
            const FirestaffFtlHunkHeader* code = &ftl.hunks[ftl.code_index];
            if (sums.code_hunk.status ==
                FIRESTAFF_FTL_CHECK_UNSUPPORTED_COMPRESSED_CODE)
                ++checksum_receipt->code_unsupported;
            printf("  NOTE: FTL module offset=%zu (0x%zx), CODE prefix=%02x%02x, "
                   "checksum status header/BSS/DATA/CODE=%d/%d/%d/%d "
                   "(CODE expected=%04x actual=%04x)\n",
                   off, off, code->payload[0], code->payload[1],
                   sums.common_header.status, sums.bss_hunk.status,
                   sums.data_hunk.status, sums.code_hunk.status,
                   sums.code_hunk.expected, sums.code_hunk.actual);
        }
        if (*out_first_offset == 0u) {
            *out_first_offset = off;
        }
        if (ftl.has_bss_metadata) {
            *out_area1 = ftl.bss.data_area1_memory_size;
            /* Found the first FTL with BSS metadata; we
             * record the area_1 and continue scanning so we
             * can report the total parseable count, but the
             * handoff verdict is anchored on the first
             * documented BSS metadata. */
        }
    }
    return parseable;
}

/* If the private original FTL reference exists locally, use it as a positive
 * control for the known cracked module. The reference is read in place and
 * is never copied into build output or the source tree. */
static int verify_original_code_control(
    const X68kFtlChecksumReceipt* checksum_receipt) {
    char* path = resolve_original_ftl_path();
    size_t original_size = 0u;
    uint8_t* original_bytes;
    FirestaffFtlContainer original;
    FirestaffFtlChecksumReport sums;
    size_t differing_bytes = 0u;
    int valid = 1;

    if (!path) {
        skip("INV_X68K_FTL_HANDOFF_REAL_11",
             "no local original FTLCODE reference found; cracked CODE discrepancy remains unclassified");
        return 0;
    }
    original_bytes = read_file_all(path, &original_size);
    if (!original_bytes ||
        FirestaffFtlContainer_Parse(original_bytes, original_size,
                                    &original) != 0 ||
        original.code_index < 0 ||
        FirestaffFtlContainer_VerifyChecksums(original_bytes, original_size,
                                               &original, &sums) != 0) {
        fprintf(stderr, "FAIL: could not parse original FTL control %s\n", path);
        fail("INV_X68K_FTL_HANDOFF_REAL_11",
             "local original FTLCODE control could not be parsed");
        free(original_bytes);
        free(path);
        return -1;
    }
    if (checksum_receipt->code_matches != 1u ||
        checksum_receipt->code_mismatches != 1u ||
        sums.code_hunk.status != FIRESTAFF_FTL_CHECK_MATCH ||
        !original.hunks[original.code_index].payload ||
        original.hunks[original.code_index].size < 2u ||
        !checksum_receipt->cracked_code_mismatch ||
        original.hunks[original.code_index].size !=
            checksum_receipt->cracked_code_mismatch_size) {
        valid = 0;
    } else {
        const uint8_t* control_code = original.hunks[original.code_index].payload;
        const uint8_t* cracked_code = checksum_receipt->cracked_code_mismatch;
        size_t code_size = original.hunks[original.code_index].size;
        for (size_t i = 0; i < code_size; ++i) {
            if (control_code[i] != cracked_code[i]) ++differing_bytes;
        }
        if (differing_bytes != 11u) valid = 0;
    }
    if (valid) {
        pass("INV_X68K_FTL_HANDOFF_REAL_11",
             "original CODE checksum matches; cracked module differs at the expected 11 code bytes");
        printf("  NOTE: original reference=%s, CODE checksum=%04x, "
               "byte differences in cracked module=%zu\n",
               path, sums.code_hunk.actual, differing_bytes);
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_11",
             "original CODE control did not confirm the known 11-byte cracked-module change");
        printf("  NOTE: original reference=%s, CODE status=%d, "
               "CODE checksum expected/actual=%04x/%04x, differing bytes=%zu\n",
               path, sums.code_hunk.status, sums.code_hunk.expected,
               sums.code_hunk.actual, differing_bytes);
    }
    free(original_bytes);
    free(path);
    return valid ? 1 : -1;
}

int main(void) {
    const char* kind = NULL;
    char* path = resolve_hdm_path(&kind);
    if (!path) {
        skip("INV_X68K_FTL_HANDOFF_REAL_00",
             "no DM1 X68000 HDM found at any default candidate; "
             "set FIRESTAFF_X68K_HDM_PATH=<path-to-.hdm> to run "
             "this probe");
        printf("# summary: skipped (no real media present) "
               "-- not a failure\n");
        return 0;
    }

    size_t hdm_size = 0u;
    uint8_t* hdm = read_file_all(path, &hdm_size);
    if (!hdm) {
        skip("INV_X68K_FTL_HANDOFF_REAL_00",
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

    /* Handoff #01: classifier reports MEDIA_FULL_DISK on the
     * real DMWeb-documented DM1 X68000 v3.0 HDM (1261568
     * bytes, no FTL magic at offset 0, Hudson Soft boot
     * block). The HDM is a disk image, not an FTL payload. */
    FirestaffX68kMediaClassifyResult media;
    memset(&media, 0, sizeof(media));
    FirestaffX68kMedia_Classify(hdm, hdm_size, &media);
    FirestaffX68kMediaClassifyResult full_media;
    memset(&full_media, 0, sizeof(full_media));
    FirestaffX68kMedia_ClassifyEx(
        hdm, hdm_size, FIRESTAFF_X68K_SCAN_WINDOW_FULL, &full_media);
    if (media.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK) {
        pass("INV_X68K_FTL_HANDOFF_REAL_01",
             "real HDM classifies as MEDIA_FULL_DISK");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_01",
             "real HDM does not classify as MEDIA_FULL_DISK");
    }

    /* Handoff #02: the classifier reports has_ftl_magic == 0
     * because the real DMFiles HDM starts with the Hudson
     * Soft boot block (0x601c 4875...), not the FTL 0x6160
     * big-endian magic. */
    if (media.has_ftl_magic == 0) {
        pass("INV_X68K_FTL_HANDOFF_REAL_02",
             "real HDM has no FTL magic at offset 0 "
             "(Hudson Soft boot block)");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_02",
             "real HDM unexpectedly reports FTL magic at offset 0");
    }

    /* Handoff #03: compare the default 32 KiB FTL-magic
     * scan window with an explicit full-HDM scan. The real
     * DMFiles X68000 v3.0 HDM embeds FTL resources at
     * offsets > 32 KiB (per greatstone d_mapfile.html,
     * per-resource IMG1 / FTL pointers live deeper in the
     * MFM image), so the legacy default scan should stay at
     * 0 raw candidates while a full-HDM ClassifyEx scan
     * should see later raw 0x6160 hits. The HDM still must
     * NOT become an FTL payload: FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT
     * is tied to offset 0 and remains clear for Hudson Soft
     * boot-block disks. */
    uint32_t full_window_magic = FirestaffX68kMedia_CountFTLMagicCandidates(
        hdm, hdm_size, 0u, hdm_size);
    if (media.scan_window_used_bytes ==
            FIRESTAFF_X68K_SCAN_WINDOW_DEFAULT_BYTES &&
        media.ftl_magic_candidate_count == 0u &&
        full_media.scan_window_full_disk == 1 &&
        full_media.scan_window_used_bytes == (uint64_t)hdm_size &&
        full_media.ftl_magic_candidate_count > 0u &&
        full_window_magic == full_media.ftl_magic_candidate_count &&
        FirestaffX68kMedia_IsFTLPayload(
            full_media.flags, full_media.media_class) == 0) {
        pass("INV_X68K_FTL_HANDOFF_REAL_03",
             "32 KiB scan stays empty; full-HDM scan sees raw "
             "FTL magic without classifying the HDM as FTL");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_03",
             "default/full FTL scan-window receipt changed");
    }
    printf("  NOTE: default scan window=%llu bytes, candidates=%u; "
           "full scan window=%llu bytes, candidates=%u, "
           "IsFTLPayload=%d\n",
           (unsigned long long)media.scan_window_used_bytes,
           media.ftl_magic_candidate_count,
           (unsigned long long)full_media.scan_window_used_bytes,
           full_media.ftl_magic_candidate_count,
           FirestaffX68kMedia_IsFTLPayload(
               full_media.flags, full_media.media_class));

    /* Handoff #04: scan the real HDM for parseable embedded
     * FTL resources. The raw 0x6160 full-HDM count is only a
     * receipt that the window reached the embedded-resource
     * area; the FTL parser is still the authoritative filter
     * because 0x6160 BE can also appear as ordinary 68000
     * instruction/data words. */
    uint32_t first_area1 = 0u;
    size_t first_ftl_off = 0u;
    X68kFtlChecksumReceipt checksum_receipt;
    size_t parseable = scan_ftl_candidates(
        hdm, hdm_size,
        X68K_HDM_MAX_FTL_CANDIDATES,
        &first_area1, &first_ftl_off, &checksum_receipt);

    if (full_window_magic >= parseable && full_window_magic > 0u) {
        pass("INV_X68K_FTL_HANDOFF_REAL_04",
             "explicit full-HDM FTL magic scan covers embedded "
             "resource candidates");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_04",
             "explicit full-HDM FTL magic scan did not cover "
             "parseable embedded resources");
    }

    /* Handoff #10: receipt and verify each parseable module's documented
     * common/BSS/DATA/CODE checksums as encoded in the real HDM. Common,
     * BSS, and DATA must match. CODE results are reported as evidence; a raw
     * mismatch is not promoted to a decode result or silently suppressed. */
    if (checksum_receipt.modules_with_checksums == parseable && parseable >= 2u &&
        checksum_receipt.common_matches == parseable &&
        checksum_receipt.bss_matches == parseable &&
        checksum_receipt.data_matches == parseable &&
        checksum_receipt.code_matches + checksum_receipt.code_mismatches ==
            parseable && checksum_receipt.code_unsupported == 0u) {
        pass("INV_X68K_FTL_HANDOFF_REAL_10",
             "two real-media modules have matching common/BSS/DATA checksums; CODE checksums inspected and reported");
        printf("  NOTE: checksum receipts modules=%zu common=%zu BSS=%zu DATA=%zu "
               "CODE match=%zu mismatch=%zu; unsupported compressed CODE=%zu\n",
               checksum_receipt.modules_with_checksums,
               checksum_receipt.common_matches, checksum_receipt.bss_matches,
               checksum_receipt.data_matches, checksum_receipt.code_matches,
               checksum_receipt.code_mismatches,
               checksum_receipt.code_unsupported);
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_10",
             "real-media checksum receipt did not meet the supported BSS/DATA/CODE reporting invariants");
        printf("  NOTE: modules=%zu common=%zu BSS=%zu DATA=%zu CODE match=%zu "
               "mismatch=%zu unsupported=%zu\n",
               checksum_receipt.modules_with_checksums,
               checksum_receipt.common_matches, checksum_receipt.bss_matches,
               checksum_receipt.data_matches, checksum_receipt.code_matches,
               checksum_receipt.code_mismatches,
               checksum_receipt.code_unsupported);
    }
    int original_control = verify_original_code_control(&checksum_receipt);
    if (original_control < 0 && g_fail == 0) {
        fail("INV_X68K_FTL_HANDOFF_REAL_11",
             "original-code positive control failed without recording a failure");
    }
    printf("  NOTE: full-HDM raw magic candidates = %u; "
           "parseable FTL count = %zu\n",
           full_media.ftl_magic_candidate_count, parseable);

    /* Handoff #05: the real DMFiles X68000 HDM must yield at
     * least one parseable FTL container. The exact count
     * depends on the MFM bit stream; the public DMFiles
     * X68000 v3.0 HDM yields 2 parseable FTL containers per
     * greatstone d_mapfile.html's `dm_atari_demo.map`
     * shape (graphics + SWSH). We accept >= 1 as the
     * "FTL resources embedded" receipt. */
    if (parseable >= 1u) {
        pass("INV_X68K_FTL_HANDOFF_REAL_05",
             "real HDM embeds >= 1 parseable FTL container");
        printf("  NOTE: %zu parseable FTL container(s); "
               "first at offset %zu (0x%zx); "
               "first BSS area_1 = %u bytes\n",
               parseable, first_ftl_off, first_ftl_off,
               (unsigned)first_area1);
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_05",
             "real HDM has no parseable FTL container");
        printf("  NOTE: raw magic-byte hits = %u (ordinary "
               "instruction/data collisions on 0x6160 BE are "
               "expected but do not parse as FTL)\n",
               media.ftl_magic_candidate_count);
    }

    /* Handoff #06: cross-module handoff verdict. The FTL
     * parser's parsed BSS metadata reports an in-memory
     * area_1 size; the X68000 classifier must accept that
     * size as fitting the on-disk HDM media class. For the
     * real DMFiles X68000 v3.0 HDM the FTL resources embed
     * small palettes / SWSH blobs (BSS area_1 typically
     * 100s to low thousands of bytes), so the verdict is
     * FIT. */
    if (parseable >= 1u && first_area1 > 0u) {
        int fits = FirestaffX68kMedia_FTLHandoffFits(
            &media, first_area1);
        if (fits == 1) {
            pass("INV_X68K_FTL_HANDOFF_REAL_06",
                 "first parsed FTL BSS area_1 fits the HDM");
            printf("  NOTE: area_1=%u bytes; HDM=%zu bytes; "
                   "fits=%d\n",
                   (unsigned)first_area1, hdm_size, fits);
        } else {
            fail("INV_X68K_FTL_HANDOFF_REAL_06",
                 "first parsed FTL BSS area_1 does NOT fit "
                 "the HDM (handoff reject)");
        }
    } else if (parseable >= 1u) {
        /* We found parseable FTL containers but none had
         * BSS metadata; the handoff verdict is undefined
         * for that case (FirestaffX68kMedia_FTLHandoffFits
         * treats size 0 as "size unknown" -> fits). */
        pass("INV_X68K_FTL_HANDOFF_REAL_06",
             "no BSS metadata in first FTL; handoff undefined, "
             "treated as size-unknown FITS");
    } else {
        skip("INV_X68K_FTL_HANDOFF_REAL_06",
             "no parseable FTL in real HDM; handoff undefined");
    }

    /* Handoff #07: the real DMFiles X68000 HDM must NOT
     * accept an over-disk area_1 size. We cross-check the
     * FTLHandoffFits helper by feeding it a deliberately
     * over-disk value and confirming the verdict is 0. */
    int fits_overflow = FirestaffX68kMedia_FTLHandoffFits(
        &media, (uint32_t)hdm_size + 1u);
    if (fits_overflow == 0) {
        pass("INV_X68K_FTL_HANDOFF_REAL_07",
             "FTLHandoffFits rejects over-disk area_1 (size+1)");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_07",
             "FTLHandoffFits unexpectedly accepted over-disk "
             "area_1");
    }

    /* Handoff #08: zero-area_1 size must be treated as
     * "size unknown" and accept (matches the synthetic
     * unit's documented contract). */
    int fits_zero = FirestaffX68kMedia_FTLHandoffFits(
        &media, 0u);
    if (fits_zero == 1) {
        pass("INV_X68K_FTL_HANDOFF_REAL_08",
             "FTLHandoffFits accepts area_1 == 0 as size-unknown");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_08",
             "FTLHandoffFits unexpectedly rejected area_1 == 0");
    }

    /* Handoff #09: handoff size-limit sanity. The full-disk
     * FTLHandoffFits contract accepts any area_1 <=
     * disk_bytes; we cross-check that boundary by feeding
     * exactly disk_bytes and confirming the verdict is 1. */
    int fits_exact = FirestaffX68kMedia_FTLHandoffFits(
        &media, (uint32_t)hdm_size);
    if (fits_exact == 1) {
        pass("INV_X68K_FTL_HANDOFF_REAL_09",
             "FTLHandoffFits accepts area_1 == disk_bytes");
    } else {
        fail("INV_X68K_FTL_HANDOFF_REAL_09",
             "FTLHandoffFits unexpectedly rejected "
             "area_1 == disk_bytes");
    }

    printf("# summary: %d/%d invariants passed\n",
           g_pass, g_pass + g_fail);

    int rc = (g_fail == 0) ? 0 : 1;
    free(hdm);
    free(path);
    return rc;
}
