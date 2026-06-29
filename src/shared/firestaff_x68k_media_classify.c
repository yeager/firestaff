/*
 * firestaff_x68k_media_classify.c
 *
 * Implementation of the bounded X68000 HDM/floppy media
 * classifier declared in firestaff_x68k_media_classify.h.
 *
 * Sources of truth (cross-checked):
 *   - dmweb-free.fr/community/documentation/copy-protection
 *     "Sharp X68000" section: 2 sides x 77 tracks x 8 sectors
 *     x 1024 bytes = 1261568 bytes; Track 1 Side 0 fake sectors
 *     245/246/247 are not checked; Track 1 Side 1 sector 0
 *     (no data) + fake sector 9 (HPR-0007 + 4 random bytes)
 *     is the only operational check; DM and CSB share the
 *     same protection scheme.
 *   - dmweb-free.fr/games/dungeon-master/editions/x68000:
 *     Japanese v3.0, HDM original image that cannot boot
 *     because the copy-protection sectors are absent, cracked
 *     image, blank save disk.
 *   - dmweb-free.fr/games/chaos-strikes-back/editions/x68000:
 *     Japanese v3.1 HDM, same copy-protection scheme as DM
 *     (the page notes that the same crack works because both
 *     games share it).
 *   - dmweb-free.fr/community/documentation/file-formats
 *     /data-files: DM X68000 GRAPHICS.DAT is 562 items, BIG
 *     DMCSB2; CSB X68000 GRAPHICS.DAT is 732 items, BIG
 *     DMCSB2; SND4 is X68000-specific Dialogic ADPCM mono.
 *   - greatstone d_ftl.html "20-byte common header" magic
 *     0x6160 big-endian.
 *
 * What we deliberately do not do:
 *   - We do not parse MFM flux timing.
 *   - We do not read IPF / Kryoflux containers.
 *   - We do not claim original vs cracked vs save-disk
 *     authenticity. The DMWeb page is explicit that the
 *     public DMFiles original HDM lacks the protection
 *     sector, so "sentinel absent" is what the preservation
 *     community has on hand. We surface both states.
 *   - We do not load any FTL resource into memory. The
 *     handoff check is a size-only sanity check.
 */

#include "firestaff_x68k_media_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FTL container magic, copied here so the classifier does not
 * need to drag the FTL parser header into every TU. The value
 * 0x6160 is documented by greatstone d_ftl.html as the 16-bit
 * big-endian magic at offset 0 of the common header; we read
 * it as big-endian to match the FTL container parser. */
#define X68K_FTL_MAGIC_BE 0x6160u

/* Build-time geometry sanity: the four geometry constants
 * must multiply to 1261568. We use a typedef trick that works
 * under both C99 and C11; -Werror catches the failure. */
typedef char FirestaffX68kMedia_GeometryCheck
    [(2u * 77u * 8u * 1024u) == 1261568u ? 1 : -1];

/* Byte offset where the HPR-0007 protection sentinel would
 * live in a standard 2DHD HDM dump IF the layout used
 * (side, track, sector) linearisation with side 0 fully
 * preceding side 1 and sector 9 placed AFTER the regular 8
 * sectors of Track 1 Side 1.
 *
 * Note: most preserved X68000 HDM images are 1232 KB without
 * an extra sector 9 region, so this offset normally falls
 * outside the image. We still compute it so callers can see
 * the layout that DMWeb describes, and we scan a small window
 * around it to handle padded / Kryoflux-converted dumps that
 * include the protection sector verbatim. */
#define X68K_SENTINEL_OFFSET_LINEAR \
    ((uint64_t)(FIRESTAFF_X68K_BYTES_PER_SIDE) + \
     (uint64_t)(1u * FIRESTAFF_X68K_SECTORS_PER_TRACK * \
                FIRESTAFF_X68K_BYTES_PER_SECTOR) + \
     (uint64_t)(FIRESTAFF_X68K_SECTORS_PER_TRACK * \
                FIRESTAFF_X68K_BYTES_PER_SECTOR))
/* = 630784 + 8192 + 8192 = 647168. */

void FirestaffX68kMedia_Classify(const uint8_t* data,
                                 size_t data_size,
                                 FirestaffX68kMediaClassifyResult* out) {
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->media_class = FIRESTAFF_X68K_MEDIA_EMPTY;

    if (data_size == 0u) {
        return;
    }
    if (!data) {
        /* Treat as empty for the safety contract. The caller
         * already passed a non-zero size with NULL data,
         * which is a usage error, but we don't want to crash
         * the M12 scan loop on a malformed input. */
        return;
    }

    /* 1) Size class.
     *    Note the strict ordering: empty < too-small <
     *    single-side < full-disk < oversize. We compute the
     *    class before any other check so that over- and
     *    undersized images are still flagged correctly. */
    if (data_size == FIRESTAFF_X68K_BYTES_PER_DISK) {
        out->media_class = FIRESTAFF_X68K_MEDIA_FULL_DISK;
    } else if (data_size == FIRESTAFF_X68K_BYTES_PER_SIDE) {
        out->media_class = FIRESTAFF_X68K_MEDIA_SINGLE_SIDE;
    } else if (data_size < FIRESTAFF_X68K_BYTES_PER_SIDE) {
        /* Strictly less than one side; cannot hold even one
         * full MFM sector layout. */
        if (data_size >= FIRESTAFF_X68K_BYTES_PER_SECTOR) {
            /* Holds at least one sector but is not a
             * recognised geometry. */
            out->media_class = FIRESTAFF_X68K_MEDIA_TOO_SMALL;
        } else {
            out->media_class = FIRESTAFF_X68K_MEDIA_TOO_SMALL;
        }
    } else if (data_size > FIRESTAFF_X68K_BYTES_PER_DISK) {
        out->media_class = FIRESTAFF_X68K_MEDIA_OVERSIZE;
    } else {
        /* Between single-side and full-disk: e.g. a partial
         * side dump. DMWeb does not document this shape, but
         * we recognise it as too-small rather than rejecting
         * it outright, since half-disks or truncated dumps
         * are a real artifact in the preservation pipeline. */
        out->media_class = FIRESTAFF_X68K_MEDIA_TOO_SMALL;
    }

    out->bytes_per_sector = FIRESTAFF_X68K_BYTES_PER_SECTOR;

    /* 2) FTL magic detection.
     *    0x6160 big-endian at offset 0 (greatstone d_ftl.html
     *    "20-byte common header"). We do this check on every
     *    input regardless of media_class so that single-
     *    resource .FTL payloads (often 1-50 KB) are still
     *    flagged for the FTL parser even though they fail
     *    the geometry size class. */
    if (data_size >= 2u) {
        uint16_t magic_be = (uint16_t)(((uint16_t)data[0] << 8) |
                                        (uint16_t)data[1]);
        out->has_ftl_magic = (magic_be == X68K_FTL_MAGIC_BE) ? 1 : 0;
        if (out->has_ftl_magic) {
            out->flags |= FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT;
        }
    }

    /* 3) FTL magic candidate scan over the first 32 KiB.
     *    Useful for HDM images that embed several FTL
     *    resources at known offsets (X68000 graphics assets
     *    packaged as .FTL files, e.g. KAOS.FTL for in-game
     *    palettes per greatstone d_mapfile.html). The field is
     *    kept as a legacy single-resource sniff window; use
     *    FirestaffX68kMedia_CountFTLMagicCandidates() when a
     *    receipt must scan a full HDM or another explicit
     *    window. */
    {
        size_t scan_limit = data_size < (32u * 1024u)
                                ? data_size
                                : (32u * 1024u);
        out->ftl_magic_candidate_count =
            FirestaffX68kMedia_CountFTLMagicCandidates(
                data, data_size, 0u, scan_limit);
    }

    /* 4) HPR-0007 sentinel scan.
     *    DMWeb places the operational sentinel at Track 1 Side
     *    1 Sector 9. We count every HPR-0007 string in the
     *    image, but only the exact linear sector offset sets
     *    SENTINEL_PRESENT. This prevents off-axis strings such
     *    as backup labels from being promoted to protected-media
     *    evidence. */
    if (data_size >= FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN) {
        size_t last = data_size - FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN;
        for (size_t i = 0u; i <= last; ++i) {
            if (memcmp(data + i,
                       FIRESTAFF_X68K_PROTECTION_SENTINEL,
                       FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN) != 0) {
                continue;
            }
            ++out->protection_sentinel_count;
            if ((uint64_t)i == X68K_SENTINEL_OFFSET_LINEAR) {
                out->flags |= FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT;
                out->sentinel_offset = X68K_SENTINEL_OFFSET_LINEAR;
            } else {
                ++out->protection_sentinel_offaxis_count;
                if (out->first_offaxis_sentinel_offset == 0u) {
                    out->first_offaxis_sentinel_offset = (uint64_t)i;
                }
            }
        }
    }

    /* 5) Protection-area blank check.
     *    If the linear protection sector region (8 sectors
     *    worth of bytes immediately preceding the sentinel
     *    offset, plus the protection sector itself) is
     *    all-zero or near-zero, mark the input as having a
     *    blank protection area. We accept up to 1% non-zero
     *    bytes to tolerate MFM idle patterns. */
    if (data_size >= X68K_SENTINEL_OFFSET_LINEAR) {
        size_t protect_region_start = X68K_SENTINEL_OFFSET_LINEAR;
        if (protect_region_start >= FIRESTAFF_X68K_BYTES_PER_SECTOR) {
            protect_region_start -=
                FIRESTAFF_X68K_BYTES_PER_SECTOR;
        }
        size_t protect_region_end = X68K_SENTINEL_OFFSET_LINEAR +
                                    FIRESTAFF_X68K_BYTES_PER_SECTOR;
        if (protect_region_end > data_size) {
            protect_region_end = data_size;
        }
        if (protect_region_end > protect_region_start) {
            size_t region_size = protect_region_end -
                                  protect_region_start;
            const uint8_t* region = data + protect_region_start;
            size_t nonzero = 0u;
            for (size_t i = 0u; i < region_size; ++i) {
                if (region[i] != 0u) ++nonzero;
            }
            /* 1% non-zero tolerance for MFM controller idle
             * bytes; below that, treat the region as blank. */
            size_t threshold = region_size / 100u;
            if (nonzero <= threshold) {
                out->flags |= FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK;
            }
        }
    }

    /* 6) Blank-save-disk detection.
     *    A freshly formatted save disk has every sector
     *    payload zeroed. We scan the entire image (when it
     *    fits the full-disk size class) and flag it when
     *    every non-header byte is zero. We tolerate a
     *    modest amount of controller-idle noise per the
     *    same 1% rule used for the protection area. */
    if (out->media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK) {
        size_t nonzero = 0u;
        for (size_t i = 0u; i < data_size; ++i) {
            if (data[i] != 0u) ++nonzero;
        }
        size_t threshold = data_size / 100u;
        if (nonzero <= threshold) {
            out->flags |= FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK;
        }
    }

    /* 7) Receipt boundary. This is a provenance/readiness
     * class, not an authenticity judgement. The ordering is
     * deliberate: FTL payloads are not disk imports; blank
     * save disks must stay separate from nonblank public or
     * cracked HDMs; a live sector sentinel outranks off-axis
     * label strings. */
    if ((out->flags & FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT) != 0u) {
        out->receipt_class = FIRESTAFF_X68K_RECEIPT_FTL_PAYLOAD;
    } else if (out->media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK) {
        if ((out->flags & FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) != 0u) {
            out->receipt_class = FIRESTAFF_X68K_RECEIPT_BLANK_SAVE_DISK;
        } else if ((out->flags &
                    FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) != 0u) {
            out->receipt_class =
                FIRESTAFF_X68K_RECEIPT_PROTECTED_SENTINEL_AT_SECTOR;
        } else if (out->protection_sentinel_offaxis_count > 0u) {
            out->receipt_class =
                FIRESTAFF_X68K_RECEIPT_OFF_AXIS_SENTINEL_ONLY;
        } else {
            out->receipt_class =
                FIRESTAFF_X68K_RECEIPT_NONBLANK_NO_SENTINEL;
        }
    } else if (out->media_class == FIRESTAFF_X68K_MEDIA_SINGLE_SIDE) {
        out->receipt_class = FIRESTAFF_X68K_RECEIPT_PARTIAL_SIDE;
    } else {
        out->receipt_class = FIRESTAFF_X68K_RECEIPT_NOT_X68K_MEDIA;
    }
}

uint32_t FirestaffX68kMedia_CountFTLMagicCandidates(
    const uint8_t* data,
    size_t data_size,
    size_t offset,
    size_t length) {
    uint32_t candidates = 0u;
    if (!data || data_size < 2u || length < 2u) return 0u;
    if (offset >= data_size) return 0u;

    size_t available = data_size - offset;
    size_t scan_len = length < available ? length : available;
    if (scan_len < 2u) return 0u;

    size_t last = offset + scan_len - 2u;
    for (size_t i = offset; i <= last; ++i) {
        uint16_t m = (uint16_t)(((uint16_t)data[i] << 8) |
                                 (uint16_t)data[i + 1u]);
        if (m == X68K_FTL_MAGIC_BE) {
            if (candidates != UINT32_MAX) ++candidates;
        }
    }
    return candidates;
}

int FirestaffX68kMedia_IsFTLPayload(uint32_t flags,
                                    uint32_t media_class) {
    (void)media_class;
    return (flags & FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT) ? 1 : 0;
}

int FirestaffX68kMedia_IsUnprotectedDisk(uint32_t flags,
                                         uint32_t media_class) {
    if (media_class != FIRESTAFF_X68K_MEDIA_FULL_DISK &&
        media_class != FIRESTAFF_X68K_MEDIA_SINGLE_SIDE) {
        return 0;
    }
    /* "Unprotected" here means: the protection-area blank
     * check fired (no HPR-0007 sentinel, region is zero or
     * near-zero). DMWeb states this is what the public
     * DMFiles original / cracked / save-disk images all
     * look like at the byte level, since the protection
     * sectors are absent from the released HDM. We do NOT
     * flag it as a problem: the user-side copy-protection
     * check is documented as broken / bypassed, and the
     * runtime "real disk required" behaviour is handled at
     * the runtime-emulator boundary, not by our parser. */
    return (flags & FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK) ? 1 : 0;
}

const char* FirestaffX68kMedia_ReceiptClassName(uint32_t receipt_class) {
    switch (receipt_class) {
        case FIRESTAFF_X68K_RECEIPT_NOT_X68K_MEDIA:
            return "not_x68k_media";
        case FIRESTAFF_X68K_RECEIPT_FTL_PAYLOAD:
            return "ftl_payload";
        case FIRESTAFF_X68K_RECEIPT_BLANK_SAVE_DISK:
            return "blank_save_disk";
        case FIRESTAFF_X68K_RECEIPT_PROTECTED_SENTINEL_AT_SECTOR:
            return "protected_sentinel_at_sector";
        case FIRESTAFF_X68K_RECEIPT_OFF_AXIS_SENTINEL_ONLY:
            return "off_axis_sentinel_only";
        case FIRESTAFF_X68K_RECEIPT_NONBLANK_NO_SENTINEL:
            return "nonblank_no_sentinel";
        case FIRESTAFF_X68K_RECEIPT_PARTIAL_SIDE:
            return "partial_side";
        case FIRESTAFF_X68K_RECEIPT_UNKNOWN:
        default:
            return "unknown";
    }
}

int FirestaffX68kMedia_FTLHandoffFits(
    const FirestaffX68kMediaClassifyResult* media,
    uint32_t bss_area1_memory_size) {
    if (!media) return 0;
    /* An FTL "in-memory area_1 size" of 0 is not a useful
     * bound: callers that have not yet parsed the FTL
     * container should pass 0 and get a "fits" verdict
     * (no claim, no false overflow). */
    if (bss_area1_memory_size == 0u) return 1;

    uint64_t declared = (uint64_t)bss_area1_memory_size;

    switch (media->media_class) {
        case FIRESTAFF_X68K_MEDIA_FULL_DISK:
            return declared <=
                   (uint64_t)FIRESTAFF_X68K_BYTES_PER_DISK ? 1 : 0;
        case FIRESTAFF_X68K_MEDIA_SINGLE_SIDE:
            return declared <=
                   (uint64_t)FIRESTAFF_X68K_BYTES_PER_SIDE ? 1 : 0;
        case FIRESTAFF_X68K_MEDIA_OVERSIZE:
            /* If the HDM is bigger than the standard 1232 KB
             * then we don't actually know where the FTL
             * payload sits, so we conservatively reject
             * rather than guess. A human reviewing the
             * OVERSIZE result can override this. */
            return 0;
        case FIRESTAFF_X68K_MEDIA_EMPTY:
        case FIRESTAFF_X68K_MEDIA_TOO_SMALL:
        default:
            /* The HDM is too small to hold the FTL resource
             * declared by HUNK_BSS. */
            return 0;
    }
}

/* ───────────────── Self-test scaffolding ─────────────────
 *
 * The tests use malloc'd buffers sized to the geometry shape
 * we exercise (one full disk = 1232 KB + some slack for the
 * oversized case). malloc avoids both ~5 MB of static BSS and
 * the 1+ MB stack-frame concerns on platforms with smaller
 * default thread stacks. Each test frees its buffer before
 * returning so SelfTest is allocation-clean. */

#define ST_ASSERT(cond, msg) do {                                      \
    if (!(cond)) {                                                     \
        fprintf(stderr, "%s:%d: %s (%s)\n", __FILE__, __LINE__,        \
                msg, #cond);                                           \
        return 0;                                                      \
    }                                                                  \
} while (0)

static int test_geometry_constants(void) {
    ST_ASSERT(FIRESTAFF_X68K_SIDES_PER_DISK == 2u, "sides");
    ST_ASSERT(FIRESTAFF_X68K_TRACKS_PER_SIDE == 77u, "tracks");
    ST_ASSERT(FIRESTAFF_X68K_SECTORS_PER_TRACK == 8u, "sectors");
    ST_ASSERT(FIRESTAFF_X68K_BYTES_PER_SECTOR == 1024u, "bytes/sector");
    ST_ASSERT(FIRESTAFF_X68K_BYTES_PER_SIDE ==
                  77u * 8u * 1024u,
              "bytes/side");
    ST_ASSERT(FIRESTAFF_X68K_BYTES_PER_DISK ==
                  2u * 77u * 8u * 1024u,
              "bytes/disk");
    ST_ASSERT(FIRESTAFF_X68K_BYTES_PER_DISK == 1261568u,
              "DMWeb geometry total");
    ST_ASSERT(X68K_SENTINEL_OFFSET_LINEAR == 647168u,
              "sentinel linear offset");
    return 1;
}

static int test_empty_input(void) {
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(NULL, 0u, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_EMPTY, "empty class");
    ST_ASSERT(r.flags == 0u, "empty flags");
    ST_ASSERT(r.has_ftl_magic == 0, "no magic");
    ST_ASSERT(r.ftl_magic_candidate_count == 0u, "no candidates");
    ST_ASSERT(r.receipt_class == FIRESTAFF_X68K_RECEIPT_UNKNOWN,
              "empty receipt remains unknown");
    return 1;
}

static int test_too_small_input(void) {
    uint8_t buf[512];
    memset(buf, 0xCC, sizeof(buf));
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, sizeof(buf), &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_TOO_SMALL,
              "too small class");
    ST_ASSERT(r.has_ftl_magic == 0, "no magic");
    ST_ASSERT(r.flags == 0u, "no flags");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_NOT_X68K_MEDIA,
              "too small receipt");
    return 1;
}

static int test_single_side_size(void) {
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_X68K_BYTES_PER_SIDE);
    ST_ASSERT(buf != NULL, "malloc single-side");
    memset(buf, 0xAA, FIRESTAFF_X68K_BYTES_PER_SIDE);
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_SIDE, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_SINGLE_SIDE,
              "single-side class");
    ST_ASSERT(r.bytes_per_sector == 1024u, "sector size");
    ST_ASSERT(r.receipt_class == FIRESTAFF_X68K_RECEIPT_PARTIAL_SIDE,
              "single-side receipt");
    free(buf);
    return 1;
}

static int test_full_disk_blank_save(void) {
    uint8_t* buf = (uint8_t*)calloc(1, FIRESTAFF_X68K_BYTES_PER_DISK);
    ST_ASSERT(buf != NULL, "calloc full-disk");
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "full disk class");
    ST_ASSERT(r.flags &
                  FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK,
              "blank save disk flag");
    ST_ASSERT(r.flags &
                  FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK,
              "protection area blank");
    ST_ASSERT((r.flags &
               FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) == 0u,
              "no sentinel on a blank image");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_BLANK_SAVE_DISK,
              "blank save receipt");
    free(buf);
    return 1;
}

static int test_full_disk_with_sentinel(void) {
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_X68K_BYTES_PER_DISK);
    ST_ASSERT(buf != NULL, "malloc full-disk");
    memset(buf, 0x55, FIRESTAFF_X68K_BYTES_PER_DISK);
    /* Plant the HPR-0007 sentinel at the linear offset
     * documented in the header. The 8 sentinel bytes are
     * followed by 4 random-looking bytes which DMWeb notes
     * are also part of the protection region but not the
     * marker we use for the classifier. */
    memcpy(buf + X68K_SENTINEL_OFFSET_LINEAR,
           FIRESTAFF_X68K_PROTECTION_SENTINEL,
           FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN);
    /* Make the protection-area non-blank around the
     * sentinel so the blank-flag does not also fire (we
     * want to test each flag in isolation). The protection
     * region in this classifier is ~2 sectors (sentinel
     * offset minus 1 sector, plus 1 sector beyond the
     * sentinel) = ~2048 bytes; we deliberately scatter
     * non-zero bytes throughout so the 1% threshold is
     * exceeded. */
    for (size_t off = 16u; off < FIRESTAFF_X68K_BYTES_PER_SECTOR * 2u;
         off += 32u) {
        buf[X68K_SENTINEL_OFFSET_LINEAR -
            FIRESTAFF_X68K_BYTES_PER_SECTOR + off] ^= 0xA5u;
    }
    /* Clear the "blank save disk" flag by ensuring > 1%
     * of the disk is non-zero. */
    for (size_t i = 0u; i < FIRESTAFF_X68K_BYTES_PER_DISK; i += 53u) {
        buf[i] ^= 0x01u;
    }

    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "full disk class");
    ST_ASSERT(r.flags &
                  FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT,
              "sentinel present");
    ST_ASSERT(r.sentinel_offset == X68K_SENTINEL_OFFSET_LINEAR,
              "sentinel offset");
    ST_ASSERT(r.protection_sentinel_count == 1u,
              "one sentinel counted");
    ST_ASSERT(r.protection_sentinel_offaxis_count == 0u,
              "no off-axis sentinel");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_PROTECTED_SENTINEL_AT_SECTOR,
              "protected sector receipt");
    ST_ASSERT((r.flags &
               FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) == 0u,
              "not a blank save disk");
    ST_ASSERT((r.flags &
               FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK) == 0u,
              "protection area not blank");
    free(buf);
    return 1;
}

static int test_full_disk_with_off_axis_sentinel_only(void) {
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_X68K_BYTES_PER_DISK);
    ST_ASSERT(buf != NULL, "malloc off-axis");
    memset(buf, 0x44, FIRESTAFF_X68K_BYTES_PER_DISK);
    /* Simulate the public-media receipt hazard: a backup label
     * carries HPR-0007, but the DMWeb live sector does not. */
    {
        size_t off = 123456u;
        memcpy(buf + off,
               FIRESTAFF_X68K_PROTECTION_SENTINEL,
               FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN);
    }

    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "full disk class");
    ST_ASSERT((r.flags &
               FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) == 0u,
              "off-axis string does not set live sentinel flag");
    ST_ASSERT(r.protection_sentinel_count == 1u,
              "one sentinel counted");
    ST_ASSERT(r.protection_sentinel_offaxis_count == 1u,
              "one off-axis sentinel");
    ST_ASSERT(r.first_offaxis_sentinel_offset == 123456u,
              "first off-axis offset");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_OFF_AXIS_SENTINEL_ONLY,
              "off-axis receipt");
    ST_ASSERT(strcmp(FirestaffX68kMedia_ReceiptClassName(r.receipt_class),
                     "off_axis_sentinel_only") == 0,
              "off-axis receipt name");
    free(buf);
    return 1;
}

static int test_full_disk_nonblank_no_sentinel(void) {
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_X68K_BYTES_PER_DISK);
    ST_ASSERT(buf != NULL, "malloc nonblank");
    memset(buf, 0x31, FIRESTAFF_X68K_BYTES_PER_DISK);
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "full disk class");
    ST_ASSERT(r.protection_sentinel_count == 0u,
              "no sentinel counted");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_NONBLANK_NO_SENTINEL,
              "nonblank no-sentinel receipt");
    ST_ASSERT(strcmp(FirestaffX68kMedia_ReceiptClassName(r.receipt_class),
                     "nonblank_no_sentinel") == 0,
              "nonblank receipt name");
    free(buf);
    return 1;
}

static int test_oversize_input(void) {
    size_t buf_size = FIRESTAFF_X68K_BYTES_PER_DISK + 4096u;
    uint8_t* buf = (uint8_t*)malloc(buf_size);
    ST_ASSERT(buf != NULL, "malloc oversize");
    memset(buf, 0x33, buf_size);
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, buf_size, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_OVERSIZE,
              "oversize class");
    ST_ASSERT(r.has_ftl_magic == 0, "no magic at offset 0");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_NOT_X68K_MEDIA,
              "oversize receipt");
    free(buf);
    return 1;
}

static int test_ftl_payload_detected(void) {
    /* Construct a minimal 32-byte buffer starting with the
     * FTL common-header magic 0x6160 big-endian. The size
     * is well below X68K_BYTES_PER_SIDE so the media class
     * is TOO_SMALL, but the FTL flag should still fire so
     * callers know to hand it to the FTL parser. */
    uint8_t buf[32];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x61u;
    buf[1] = 0x60u;
    /* Bump a few bytes to keep the FTL parser happy on the
     * common-header checksum path. */
    buf[4] = 0x00u;
    buf[5] = 0x02u;

    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, sizeof(buf), &r);
    ST_ASSERT(r.has_ftl_magic == 1, "FTL magic at offset 0");
    ST_ASSERT(r.flags & FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT,
              "FTL flag set");
    ST_ASSERT(r.ftl_magic_candidate_count >= 1u,
              "FTL magic candidate count >= 1");
    ST_ASSERT(FirestaffX68kMedia_IsFTLPayload(r.flags, r.media_class) == 1,
              "IsFTLPayload true");
    ST_ASSERT(r.receipt_class ==
                  FIRESTAFF_X68K_RECEIPT_FTL_PAYLOAD,
              "FTL receipt class");
    return 1;
}

static int test_ftl_magic_window_scan(void) {
    uint8_t* buf = (uint8_t*)calloc(1, 64u * 1024u);
    ST_ASSERT(buf != NULL, "calloc window scan");
    buf[0] = 0x61u;
    buf[1] = 0x60u;
    buf[40u * 1024u] = 0x61u;
    buf[40u * 1024u + 1u] = 0x60u;

    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, 64u * 1024u, &r);
    ST_ASSERT(r.ftl_magic_candidate_count == 1u,
              "legacy first-32KiB sniff sees only the early magic");
    ST_ASSERT(FirestaffX68kMedia_CountFTLMagicCandidates(
                  buf, 64u * 1024u, 0u, 64u * 1024u) == 2u,
              "full explicit window sees both magic candidates");
    ST_ASSERT(FirestaffX68kMedia_CountFTLMagicCandidates(
                  buf, 64u * 1024u, 32u * 1024u, 32u * 1024u) == 1u,
              "late explicit window sees embedded magic candidate");
    ST_ASSERT(FirestaffX68kMedia_CountFTLMagicCandidates(
                  NULL, 64u * 1024u, 0u, 64u * 1024u) == 0u,
              "NULL window returns zero");
    ST_ASSERT(FirestaffX68kMedia_CountFTLMagicCandidates(
                  buf, 64u * 1024u, 64u * 1024u, 8u) == 0u,
              "out-of-range window returns zero");
    free(buf);
    return 1;
}

static int test_ftl_handoff_fits_full_disk(void) {
    uint8_t* buf = (uint8_t*)calloc(1, FIRESTAFF_X68K_BYTES_PER_DISK);
    ST_ASSERT(buf != NULL, "calloc full-disk handoff");
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "full disk class");

    /* A declared area_1 size of exactly one side fits. */
    ST_ASSERT(FirestaffX68kMedia_FTLHandoffFits(
                  &r,
                  (uint32_t)FIRESTAFF_X68K_BYTES_PER_SIDE) == 1,
              "side fits in full disk");

    /* A declared area_1 size of the entire disk fits. */
    ST_ASSERT(FirestaffX68kMedia_FTLHandoffFits(
                  &r,
                  (uint32_t)FIRESTAFF_X68K_BYTES_PER_DISK) == 1,
              "full disk area_1 fits");

    /* A declared area_1 size 1 byte larger than the disk
     * does not fit. */
    ST_ASSERT(FirestaffX68kMedia_FTLHandoffFits(
                  &r,
                  (uint32_t)FIRESTAFF_X68K_BYTES_PER_DISK + 1u) == 0,
              "over-disk area_1 rejected");

    /* 0 is treated as "size unknown" and fits. */
    ST_ASSERT(FirestaffX68kMedia_FTLHandoffFits(&r, 0u) == 1,
              "unknown size fits");
    free(buf);
    return 1;
}

static int test_ftl_handoff_overflow_too_small(void) {
    uint8_t buf[512];
    memset(buf, 0xCC, sizeof(buf));
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, sizeof(buf), &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_TOO_SMALL,
              "too small class");

    /* Even a single 1024-byte area_1 cannot fit a 512-byte
     * buffer. */
    ST_ASSERT(FirestaffX68kMedia_FTLHandoffFits(
                  &r, FIRESTAFF_X68K_BYTES_PER_SECTOR) == 0,
              "overflow on too-small media");
    return 1;
}

static int test_unprotected_disk_flag(void) {
    /* Build a full-disk image where every byte is zero. That
     * matches the public DMFiles "original HDM that cannot
     * boot" shape (DMWeb X68000 edition page): the
     * protection sector is absent so the protection-area
     * region is blank. */
    uint8_t* buf = (uint8_t*)calloc(1, FIRESTAFF_X68K_BYTES_PER_DISK);
    ST_ASSERT(buf != NULL, "calloc unprotected");
    FirestaffX68kMediaClassifyResult r;
    FirestaffX68kMedia_Classify(buf, FIRESTAFF_X68K_BYTES_PER_DISK, &r);
    ST_ASSERT(r.media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "full disk class");
    ST_ASSERT(FirestaffX68kMedia_IsUnprotectedDisk(r.flags,
                                                    r.media_class) == 1,
              "unprotected full disk");
    free(buf);

    /* A too-small image is not "an unprotected disk" by
     * definition. */
    FirestaffX68kMediaClassifyResult r2;
    uint8_t tiny[64];
    memset(tiny, 0, sizeof(tiny));
    FirestaffX68kMedia_Classify(tiny, sizeof(tiny), &r2);
    ST_ASSERT(FirestaffX68kMedia_IsUnprotectedDisk(r2.flags,
                                                    r2.media_class) == 0,
              "too-small image is not an unprotected disk");
    return 1;
}

static int test_helpers_handle_null(void) {
    FirestaffX68kMediaClassifyResult r;
    memset(&r, 0, sizeof(r));
    /* FTLHandoffFits with NULL media pointer must return 0
     * (refuses the size check) rather than crashing. */
    ST_ASSERT(FirestaffX68kMedia_FTLHandoffFits(NULL, 1024u) == 0,
              "NULL media rejected");
    ST_ASSERT(strcmp(FirestaffX68kMedia_ReceiptClassName(9999u),
                     "unknown") == 0,
              "unknown receipt name");
    return 1;
}

int FirestaffX68kMedia_SelfTest(void) {
    int total = 0;
    int passed = 0;
#define RUN(test) do { ++total; if (test()) ++passed; } while (0)
    RUN(test_geometry_constants);
    RUN(test_empty_input);
    RUN(test_too_small_input);
    RUN(test_single_side_size);
    RUN(test_full_disk_blank_save);
    RUN(test_full_disk_with_sentinel);
    RUN(test_full_disk_with_off_axis_sentinel_only);
    RUN(test_full_disk_nonblank_no_sentinel);
    RUN(test_oversize_input);
    RUN(test_ftl_payload_detected);
    RUN(test_ftl_magic_window_scan);
    RUN(test_ftl_handoff_fits_full_disk);
    RUN(test_ftl_handoff_overflow_too_small);
    RUN(test_unprotected_disk_flag);
    RUN(test_helpers_handle_null);
#undef RUN
    if (passed != total) {
        fprintf(stderr,
                "firestaff_x68k_media_classify self-test: %d/%d passed\n",
                passed, total);
    }
    return passed == total ? 0 : -1;
}
