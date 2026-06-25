/*
 * firestaff_x68k_media_classify.h
 *
 * Bounded HDM/floppy media classifier for the DM1 / CSB X68000
 * import boundary documented in docs/FIRESTAFF_GAP_LIST.md
 * ("DM1 X68000 HDM/floppy media import", status OPEN-BOUNDED,
 * 2026-06-25 snapshot).
 *
 * Scope:
 *   This module is a read-only, data-free classifier. It does not
 *   decompress FTL containers, it does not parse FTL/PAK/IMG1/IMG2,
 *   and it does not read real DM1 / CSB HDM data into memory. It
 *   locks down:
 *
 *     - The standard 2DHD MFM floppy geometry for Sharp X68000
 *       (DMWeb copy-protection page: 2 sides x 77 tracks x 8
 *       sectors x 1024 bytes = 1232 KB)
 *     - Recognized HDM size classes: full double-sided, single
 *       side, blank save disk, cracked/original media with
 *       protection-sector signature bytes
 *     - The "HPR-0007" 8-byte protection-sentinel that DMWeb
 *       identifies as the only operational copy-protection check
 *       on the X68000 port (Track 1 Side 1 Sector 9)
 *     - Cross-checks against the FTL container parser
 *       (firestaff_ftl_container.h): the in-memory area_1 size
 *       declared by HUNK_BSS must not exceed the on-disk HDM
 *       size class
 *     - X68000 / PC-9801 / FM-Towns endianness pin (BIG DMCSB2
 *       for X68000, per dmweb data-files.html)
 *
 * Source of truth:
 *   - DMWeb copy-protection page, "Sharp X68000" section
 *     (community/documentation/copy-protection/copy-protection):
 *     2 sides x 77 tracks x 8 sectors x 1024 bytes geometry;
 *     Track 1 Side 0 fake sectors 245/246/247 (broken scheme);
 *     Track 1 Side 1 sector 0 + fake sector 9 with the
 *     "HPR-0007" + 4 random bytes string (operational scheme);
 *     "much weaker than Atari ST / Amiga / Apple IIGS";
 *     DM and CSB share the same protection scheme.
 *   - DMWeb DM X68000 edition page and CSB X68000 edition
 *     page (dmweb-free-fr/games/dungeon-master/editions/x68000
 *     and .../chaos-strikes-back/editions/x68000): Japanese
 *     v3.0 / v3.1 lines, HDM original (no copy-protection
 *     sectors present in the public DMFiles download, so the
 *     game cannot boot), cracked image, blank save disk.
 *   - DMWeb data-files.html ("GRAPHICS.DAT DM X68000" +
 *     "GRAPHICS.DAT CSB X68000" + "SND4 data"): X68000 assets
 *     are big-endian DMCSB2; SND4 is X68000-specific
 *     Dialogic ADPCM mono.
 *   - greatstone d_ftl.html (FTL container format) for the
 *     handoff invariants documented in
 *     firestaff_ftl_container.h (HUNK_BSS area_1 memory size
 *     field, common header magic 0x6160, big-endian words).
 *
 * What this module deliberately does NOT do:
 *   - It does NOT claim that any real X68000 HDM is or is not
 *     authentic. The DMWeb page states that the public DMFiles
 *     original HDM lacks the copy-protection sector, so the
 *     "presence of HPR-0007" boundary here is a documented
 *     classifier, not an authenticity judgement. We surface the
 *     signature bytes we can read without inventing an
 *     "original" verdict.
 *   - It does NOT call into any asset loader. The FTL handoff
 *     is a size-only sanity check, not a loader.
 *   - It does NOT parse MFM flux timing or Kryoflux/IPF
 *     containers. Those are separate larger lanes against
 *     docs/FIRESTAFF_GAP_LIST.md.
 *
 * Thread safety: the classifier is pure (no globals, no malloc)
 * and can be called concurrently from multiple threads.
 */

#ifndef FIRESTAFF_X68K_MEDIA_CLASSIFY_H
#define FIRESTAFF_X68K_MEDIA_CLASSIFY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard 2DHD MFM geometry for Sharp X68000 HDM images.
 *
 * Source: dmweb-free.fr/community/documentation/copy-protection,
 * "Sharp X68000" / "Nec PC-9801" / "Fujitsu FM-Towns" share the
 * "2 sides x 77 tracks x 8 sectors x 1024 bytes = 1261568
 * Bytes = 1232 KB" geometry. We use the exact arithmetic:
 *
 *   2 * 77 * 8 * 1024 = 1,261,568 bytes.
 *
 * The PC-9801 and FM-Towns pages document the same geometry for
 * the Japanese-disk trio; we do not bake that into this header
 * (a future PC-98 / FM-Towns header can reuse the constants). */
#define FIRESTAFF_X68K_SIDES_PER_DISK        2u
#define FIRESTAFF_X68K_TRACKS_PER_SIDE       77u
#define FIRESTAFF_X68K_SECTORS_PER_TRACK     8u
#define FIRESTAFF_X68K_BYTES_PER_SECTOR      1024u
#define FIRESTAFF_X68K_BYTES_PER_SIDE        \
    (FIRESTAFF_X68K_TRACKS_PER_SIDE *       \
     FIRESTAFF_X68K_SECTORS_PER_TRACK *     \
     FIRESTAFF_X68K_BYTES_PER_SECTOR)
#define FIRESTAFF_X68K_BYTES_PER_DISK        \
    (FIRESTAFF_X68K_SIDES_PER_DISK *         \
     FIRESTAFF_X68K_BYTES_PER_SIDE)
/* Sanity: 2 * 77 * 8 * 1024 must equal 1261568. Verified at
 * build time via the static_assert in
 * firestaff_x68k_media_classify.c. */

/* Copy-protection sentinel bytes documented by DMWeb as the only
 * operational check on X68000: the start of Track 1 Side 1
 * Sector 9 (the "fake sector 9 in addition to the regular 8
 * sectors") holds the ASCII string "HPR-0007" followed by 4
 * random bytes. The string is 8 ASCII characters.
 *
 * In the public DMFiles download of the original DM1 X68000 v3.0
 * and CSB X68000 v3.1 images, the copy-protection sectors are
 * absent (DMWeb X68000 edition pages); in a cracked image the
 * sector is bypassed at runtime via DM.X patching rather than
 * having its contents removed. A real local HDM that contains
 * the sentinel is therefore likely either an internal/preserved
 * master or a dump captured with a Kryoflux-class flux-level
 * tool; the classifier only reports presence/absence, it does
 * not pass judgement. */
#define FIRESTAFF_X68K_PROTECTION_SENTINEL   "HPR-0007"
#define FIRESTAFF_X68K_PROTECTION_SENTINEL_LEN 8u

/* X68000 / PC-98 / FM-Towns Japanese-disk size constants.
 *
 * The public DMFiles DM1 X68000 v3.0 download is the standard
 * 1232 KB double-sided MFM image. The "blank save disk" listed
 * on the same DMWeb page is a freshly formatted 1232 KB image
 * with all sector payloads zeroed.
 *
 * A "cracked" image in DMWeb's classification still covers the
 * full 1232 KB but bypasses the protection check at runtime
 * through patched DM.X code; we do not need a separate size
 * class for it. */
typedef enum {
    /* Empty media: too small to be any X68000 disk image. */
    FIRESTAFF_X68K_MEDIA_EMPTY = 0,

    /* Too small to be a usable X68000 image but not zero. */
    FIRESTAFF_X68K_MEDIA_TOO_SMALL = 1,

    /* Single-side dump or half-disk partial capture (one side
     * of the 2DHD geometry). DMWeb does not list this as an
     * official distribution shape but Kryoflux-style per-side
     * captures are common in the preservation community, and
     * this classifier needs to recognize rather than reject
     * them so we can flag them as needing a partner side. */
    FIRESTAFF_X68K_MEDIA_SINGLE_SIDE = 2,

    /* Standard 2DHD double-sided MFM image (1232 KB). This is
     * the on-disk size of every official DM1 / CSB X68000
     * download on DMWeb: original HDM, cracked HDM, blank save
     * disk. The classifier cannot tell those three apart by
     * size alone; see FIRESTAFF_X68K_SCAN_FLAG_* flags. */
    FIRESTAFF_X68K_MEDIA_FULL_DISK = 3,

    /* Larger than the standard geometry. The X68000 cannot
     * natively read or write a 3.5" MFM disk beyond 1232 KB,
     * so anything bigger is either an HDM with extra header /
     * footer / sector-padding bytes, a non-X68000 format
     * (Sega Saturn save, custom shell, etc.), or a corruption.
     * Flag for human review. */
    FIRESTAFF_X68K_MEDIA_OVERSIZE = 4
} FirestaffX68kMediaClass;

/* Scan flags reported alongside the size class. These can be
 * combined in a bitmask (bitwise OR). */
typedef enum {
    FIRESTAFF_X68K_SCAN_FLAG_NONE              = 0u,

    /* HPR-0007 sentinel was located in Track 1 Side 1 Sector 9
     * of the input. The exact byte offset is reported in
     * scan->sentinel_offset. This implies the dumped media
     * captured the only operational copy-protection sector.
     * DMWeb notes that public DMFiles HDMs do not contain it,
     * so seeing this is informative but does not constitute a
     * "real" or "fake" verdict. */
    FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT  = (1u << 0),

    /* The Track 1 Side 1 region of the input is all-zero (or
     * near-zero). On a real original HDM this is what DMWeb
     * describes for the public DMFiles download: the sector is
     * physically absent from the image so a "blind" sector
     * read returns zeroed bytes. Useful as a no-protector
     * flag for the cracked-media and save-disk cases. */
    FIRESTAFF_X68K_SCAN_FLAG_PROTECTION_AREA_BLANK = (1u << 1),

    /* The image appears to be a freshly formatted "blank save
     * disk": all sector payloads are zero, geometry matches
     * full disk. Detected via a deterministic zero-fill scan
     * (we do not require every byte to be zero; we accept up
     * to a documented tolerance for MFM controller idle
     * bytes, which the DMWeb copy-protection page notes are
     * still present in the public HDMs). */
    FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK   = (1u << 2),

    /* The image is a known FTL-container magic-bearing payload
     * at offset 0. This is consistent with a single-resource
     * FTL blob (e.g. one of the X68000 .FTL asset files in
     * csb-extras/legacy-jp-x68000/), not a full HDM. The
     * presence of the magic means this is NOT a disk-image
     * import candidate; it should be handed to the FTL
     * container parser instead. */
    FIRESTAFF_X68K_SCAN_FLAG_FTL_PRESENT       = (1u << 3)
} FirestaffX68kScanFlag;

/* Result of classifying an HDM-sized buffer. */
typedef struct {
    /* Coarse size class (one of the FIRESTAFF_X68K_MEDIA_*
     * values). */
    uint32_t media_class;

    /* OR of FIRESTAFF_X68K_SCAN_FLAG_* values. */
    uint32_t flags;

    /* Bytes-per-sector the input matched against. Always 1024
     * for a real X68000 HDM; zero if the input is too small to
     * hold even one sector. */
    uint32_t bytes_per_sector;

    /* Side 1 Track 1 byte offset where the HPR-0007 sentinel
     * was found. Only meaningful when
     * FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT is set. */
    uint64_t sentinel_offset;

    /* Non-zero iff a 0x6160 big-endian magic was detected at
     * offset 0 (FTL container header, see greatstone
     * d_ftl.html "20-byte common header"). Reported for the
     * handoff test even when the size class is
     * FIRESTAFF_X68K_MEDIA_EMPTY / TOO_SMALL. */
    int has_ftl_magic;

    /* Number of FTL-size sentinel-class hits found in the
     * first 32 KiB of the input. Zero on every non-FTL HDM;
     * small on a single-resource .FTL payload. */
    uint32_t ftl_magic_candidate_count;
} FirestaffX68kMediaClassifyResult;

/* Classify a buffer as an X68000 HDM media candidate.
 *
 *   data         pointer to the input bytes (HDM dump, .FTL
 *                payload, save disk image, etc.). May be NULL
 *                iff data_size == 0.
 *   data_size    byte count of the input.
 *   out          caller-owned result struct; always written.
 *
 * The classifier does not allocate and does not mutate the
 * input. It always returns a media_class; an unknown / too
 * small / too large image is reported as such, never silently
 * classified as something else. */
void FirestaffX68kMedia_Classify(const uint8_t* data,
                                  size_t data_size,
                                  FirestaffX68kMediaClassifyResult* out);

/* Convenience helper: true iff the FIRESTAFF_X68K_SCAN_FLAG_*
 * flag set reported by FirestaffX68kMedia_Classify is
 * consistent with a single-resource .FTL payload rather than a
 * full HDM. */
int FirestaffX68kMedia_IsFTLPayload(uint32_t flags,
                                     uint32_t media_class);

/* True iff the scan flags are consistent with a public-DMFiles
 * original / cracked HDM that lacks the operational
 * protection-sector sentinel. */
int FirestaffX68kMedia_IsUnprotectedDisk(uint32_t flags,
                                          uint32_t media_class);

/* FTL handoff check: does the in-memory area_1 size declared by
 * HUNK_BSS in a parsed FTL container fit within the on-disk
 * HDM media class size? Returns 1 on "fits" or "size unknown",
 * 0 on "declared in-memory area_1 cannot fit this HDM".
 *
 * This is a deliberate, narrow sanity check. It does NOT load
 * the FTL resource, does NOT consult HUNK_DATA compression,
 * and does NOT open the HDM as a filesystem. It only confirms
 * that the size in the FTL header is not strictly larger than
 * the HDM image (which would be impossible since the FTL
 * resources are stored inside the HDM).
 *
 * Pass the parsed FirestaffFtlContainer (or a sentinel with
 * bss.data_area1_memory_size set to the value of interest and
 * every other field zeroed) and the media classify result. */
int FirestaffX68kMedia_FTLHandoffFits(
    const FirestaffX68kMediaClassifyResult* media,
    uint32_t bss_area1_memory_size);

/* Bounded self-test covering:
 *   - empty input
 *   - too-small input
 *   - exact single-side size
 *   - exact full-disk size, all-zero (blank save disk)
 *   - exact full-disk size with sentinel at Track 1 Side 1
 *     Sector 9 byte offset (1261568 - 1024 + 0 == end of file
 *     for sector 9 of the last track on side 1)
 *   - oversized input
 *   - FTL magic at offset 0 (single-resource .FTL payload)
 *   - FTL handoff: declared area_1 fits / overflows the HDM
 *
 * Returns 0 on success and writes a short PASS / FAIL summary
 * to stderr if any case fails. */
int FirestaffX68kMedia_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_X68K_MEDIA_CLASSIFY_H */
