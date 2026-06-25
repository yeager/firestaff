/*
 * firestaff_pc98_hdm_classify.h
 *
 * Read-only classifier/parser for PC-9801 HDM/floppy media images
 * covering the DM1 PC-9801 (and CSB PC-9801) import gap.
 *
 * Background:
 *   The Dungeon Master / Chaos Strikes Back PC-9801 line ships
 *   as raw HDM (BKDSK) floppy images. The DMWeb PC-9801 edition
 *   page identifies three DM1 v2.0 Japanese images:
 *
 *     - "2.0a Original (Not working).hdm"
 *       Original floppy contents but the copy-protection sector
 *       is missing, so the game refuses to boot.
 *     - "2.0a Cracked.hdm"
 *       Patched copy that bypasses NECIO.EXE / FIRES.EXE copy-
 *       protection checks. The crack patches specific bytes in
 *       NECIO.EXE (offset 0x1CF1: 74 03 EB 62 -> EB 64 EB 62)
 *       and FIRES.EXE (offset 0x2636D, 0x263A3, 0x263BD).
 *     - "2.0b Original.hdm"
 *       Newer, bug-fixed release that is NOT copy protected.
 *
 *   We need a bounded, data-free classifier that:
 *     - distinguishes a PC-98 2HD (1,261,568 B) or 2DD
 *       (737,280 B) raw HDM image from other byte streams;
 *     - detects the documented DM1 / CSB PC-98 boot-sector
 *       and FAT directory fingerprints (NECIO.SYS, FIRES.EXE,
 *       GRAPHICS.DAT, DUNGEON.DAT, CSBGAME.EXE etc.);
 *     - classifies the 2.0a / 2.0b split using the known
 *       protection-status markers (NECIO.EXE byte at 0x1CF1 and
 *       the FIRES.EXE byte at 0x2636D);
 *     - distinguishes "2.0a Original (not working)" from
 *       "2.0a Cracked" by spotting the documented crack-patch
 *       bytes vs the original protection bytes;
 *     - never asserts a runtime/playable claim from a raw HDM
 *       alone -- copy-protection provenance, real runtime
 *       launch, and emulator handoff remain separate work.
 *
 *   The classifier operates purely on bytes in a caller-owned
 *   buffer. It does not touch any file system, does not mount
 *   the image, and does not require DMWeb/dmweb hash matching
 *   to run. Tests are entirely data-free (synthetic fixtures).
 *
 * Source references:
 *   - DMWeb DM PC-9801 edition page (dmweb.free.fr) -- 2.0a
 *     original/cracked/2.0b distinction and crack offsets.
 *   - DMWeb CSB PC-9801 edition page -- CSBGAME.EXE protection.
 *   - dmweb community documentation "copy-protection" -- the
 *     "no flux area" technique used on X68000 and PC-9801.
 *   - pc98-disk-tools (barbeque GitHub) -- HDM vs FDI header
 *     facts: HDM = raw dd-style image; FDI = HDM with a 4096
 *     byte header prepended.
 *   - DMWeb DM PC-9801 page -- 2HD geometry = 1024 B * 8 sectors
 *     * 2 surfaces * 77 tracks = 1,261,568 bytes; 2DD geometry
 *     = 512 B * 8 sectors * 2 surfaces * 77 tracks = 737,280 B.
 *
 * Scope:
 *   - Read-only byte classification.
 *   - Data-free synthetic fixtures for tests (no real HDM media).
 *   - No runtime claim, no emulator wiring, no copy-protection
 *     sector extraction.
 */

#ifndef FIRESTAFF_PC98_HDM_CLASSIFY_H
#define FIRESTAFF_PC98_HDM_CLASSIFY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard PC-9801 floppy geometry, derived from the
 * dmweb PC-9801 page + pc98-disk-tools reference:
 *   - 2HD (1.2 MB): 1024 B/sector * 8 sectors * 2 sides * 77 tracks
 *   - 2DD ( 720 KB): 512 B/sector * 8 sectors * 2 sides * 77 tracks
 * Tracks/sides/sectors-per-track match the dmweb PC-9801
 * description and the 1024-byte MFM sector used for both DM
 * and CSB 2.0/3.1 PC-9801 floppies. */
#define FIRESTAFF_PC98_2HD_BYTES   1261568u  /* 1024 * 8 * 2 * 77 */
#define FIRESTAFF_PC98_2DD_BYTES    737280u  /*  512 * 8 * 2 * 77 */
#define FIRESTAFF_PC98_FDI_HEADER      4096u  /* FDI header prepend */
#define FIRESTAFF_PC98_BYTES_PER_TRACK_2HD 16384u /* 1024 * 8 * 2 */

/* DM1 PC-9801 protection-check byte offsets. These are
 * absolute file offsets into the EXE binaries on the HDM,
 * which live at their normal FAT directory positions. The
 * classifier uses them as fingerprint offsets; they are
 * exact because NECIO.EXE and FIRES.EXE are the only files
 * at those file names in the DM1 PC-98 2.0a image. */
#define FIRESTAFF_PC98_DM1_NECIO_CRACK_OFFSET  0x1CF1u
#define FIRESTAFF_PC98_DM1_NECIO_ORIG_BYTES    { 0x74, 0x03, 0xEB, 0x62 }
#define FIRESTAFF_PC98_DM1_NECIO_CRACK_BYTES   { 0xEB, 0x64, 0xEB, 0x62 }

#define FIRESTAFF_PC98_DM1_FIRES_CRACK_OFFSET_1 0x2636Du
#define FIRESTAFF_PC98_DM1_FIRES_CRACK_OFFSET_2 0x263A3u
#define FIRESTAFF_PC98_DM1_FIRES_CRACK_OFFSET_3 0x263BDu

/* CSB PC-9801 protection markers, used for the CSBGAME.EXE
 * utility-disk boundary (kept out of the DM1 v2.0a/2.0b
 * classifier, but exposed for symmetry). */
#define FIRESTAFF_PC98_CSB_CSBGAME_PROTECT_OFFSET 0x21989u

typedef enum {
    FIRESTAFF_PC98_MEDIA_UNKNOWN = 0,
    FIRESTAFF_PC98_MEDIA_2HD_RAW,        /* HDM: 1,261,568 B raw */
    FIRESTAFF_PC98_MEDIA_2DD_RAW,        /* HDM: 737,280 B raw */
    FIRESTAFF_PC98_MEDIA_2HD_FDI,        /* FDI: header + 2HD body */
    FIRESTAFF_PC98_MEDIA_2DD_FDI,        /* FDI: header + 2DD body */
    FIRESTAFF_PC98_MEDIA_NOT_PC98        /* wrong size / random bytes */
} FirestaffPc98MediaKind;

typedef enum {
    FIRESTAFF_PC98_GAME_UNKNOWN = 0,
    FIRESTAFF_PC98_GAME_DM1,             /* Dungeon Master PC-9801 */
    FIRESTAFF_PC98_GAME_CSB,             /* Chaos Strikes Back PC-9801 */
    FIRESTAFF_PC98_GAME_DM2              /* Dungeon Master II PC-9801 */
} FirestaffPc98Game;

typedef enum {
    FIRESTAFF_PC98_VERSION_UNKNOWN = 0,
    FIRESTAFF_PC98_VERSION_DM1_20A_ORIGINAL, /* 2.0a original, has copy-protection */
    FIRESTAFF_PC98_VERSION_DM1_20A_CRACKED,  /* 2.0a cracked, bypass patched */
    FIRESTAFF_PC98_VERSION_DM1_20B_ORIGINAL, /* 2.0b original, NOT copy-protected */
    FIRESTAFF_PC98_VERSION_CSB_31_ORIGINAL,  /* CSB 3.1 original */
    FIRESTAFF_PC98_VERSION_CSB_31_CRACKED    /* CSB 3.1 cracked */
} FirestaffPc98Version;

typedef enum {
    FIRESTAFF_PC98_PROTECT_UNKNOWN = 0,
    FIRESTAFF_PC98_PROTECT_NONE,             /* 2.0b / 3.1 cracked: no protection */
    FIRESTAFF_PC98_PROTECT_PRESENT,           /* 2.0a original: copy-protection sector present */
    FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED /* cracked image: missing sector but bypass installed */
} FirestaffPc98Protection;

typedef struct {
    FirestaffPc98MediaKind media;
    FirestaffPc98Game      game;
    FirestaffPc98Version   version;
    FirestaffPc98Protection protection;

    /* Offsets where the classifier found evidence. All offsets
     * are absolute from the start of the input buffer and are
     * 0 when the corresponding marker was not seen. */
    size_t graphics_dat_offset;       /* directory entry for GRAPHICS.DAT */
    size_t dungeon_dat_offset;        /* directory entry for DUNGEON.DAT  */
    size_t necio_exe_offset;          /* directory entry for NECIO.EXE    */
    size_t fires_exe_offset;          /* directory entry for FIRES.EXE    */
    size_t csbgame_exe_offset;        /* directory entry for CSBGAME.EXE  */
    size_t copy_protection_sector_offset; /* "no flux area" probe location */

    /* Synthetic byte counts for the test fixtures only -- not
     * a claim about real media. */
    uint32_t synthetic_bytes_total;
    uint32_t synthetic_bytes_match;
} FirestaffPc98HdmClassification;

/*
 * Classify a caller-owned byte buffer as a PC-9801 HDM/FDI
 * image. The function does not allocate, does not read files,
 * and never modifies *data.
 *
 * Returns 0 on success and fills *out with the detected
 * media/game/version/protection tuple (or media = NOT_PC98
 * if the buffer does not look like a PC-98 floppy image).
 * Returns -1 on null pointer / zero-length input.
 */
int FirestaffPc98HdmClassify(const uint8_t* data, size_t data_size,
                             FirestaffPc98HdmClassification* out);

/*
 * Return a short string for a media kind (for logs and JSON
 * receipts). Returned pointer is to a static buffer.
 */
const char* FirestaffPc98HdmMediaName(FirestaffPc98MediaKind kind);
const char* FirestaffPc98HdmGameName(FirestaffPc98Game game);
const char* FirestaffPc98HdmVersionName(FirestaffPc98Version version);
const char* FirestaffPc98HdmProtectionName(FirestaffPc98Protection protect);

/*
 * In-process self-test. Verifies the 2.0a-original,
 * 2.0a-cracked, 2.0b-original, CSB 3.1 original, CSB 3.1
 * cracked, and "not a PC-98 image" classification branches
 * against synthetic fixtures (no real media needed).
 *
 * Returns 0 on success, -1 on first failed invariant.
 */
int FirestaffPc98HdmClassify_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_PC98_HDM_CLASSIFY_H */
