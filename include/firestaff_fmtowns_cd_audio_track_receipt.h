/*
 * firestaff_fmtowns_cd_audio_track_receipt.h
 *
 * Bounded CD-audio track receipt gate for FM Towns redump-style
 * BIN/CUE discs (DM1 / CSB / DM2).
 *
 * Purpose
 * -------
 * The companion `firestaff_fmtowns_cd_classify` module only parses
 * the CUE layout: track count, kind (DATA / AUDIO), and sector mode
 * (2352 / 2048). It does NOT look at the actual CD-DA bytes for
 * audio tracks.
 *
 * This module adds the CD-audio *track receipt* gate. For every
 * AUDIO track in a parsed layout it:
 *
 *   - Locates the track's byte range inside the BIN/CUE media
 *     (either a real BIN file referenced by the CUE's FILE entry,
 *     or a synthetic in-memory CD-DA byte stream supplied by the
 *     caller).
 *   - Validates the Red Book CD-DA sector shape: every sector is
 *     exactly 2352 bytes (2352 = 16-byte sync/header + 2x588 stereo
 *     16-bit PCM samples at 44100 Hz).
 *   - Computes per-sector and per-track silence metrics
 *     (max-abs-sample, mean-abs-sample, sample range) so the receipt
 *     can distinguish documented-silent tracks from real-audio ones.
 *   - Cross-references the per-track silence verdict against the
 *     DMWeb-documented CD-audio track tables for FM Towns:
 *
 *       DM1 v2.0 (tracks 02..20):
 *         track 04 -- unused per DMWeb
 *         track 07 -- unused per DMWeb
 *         track 20 -- 20 seconds of silence, unused
 *
 *       CSB v3.1 (tracks 02..31):
 *         three unused "water-drop-like" tracks somewhere in the
 *         02..31 range. We mark every track whose mean-abs-sample
 *         < FMTOWNS_CD_AUDIO_RECEIPT_SILENCE_MEAN_THRESHOLD as a
 *         candidate unused track; we then assert the candidate
 *         count == 3.
 *
 *       DM2 v1.0 (tracks 02..06 + silent track 8):
 *         track 08 -- silent
 *         tracks 02..06 -- real audio, slightly quieter
 *
 * Source of truth: docs/DMWEB_REFERENCE.md and the DMWeb FM-Towns
 * edition pages for DM / CSB / DM2.
 *
 * Scope
 * -----
 * The receipt gate is a *layout-vs-bytes* consistency check. It
 * deliberately does NOT:
 *
 *   - Decode or play the audio payload.
 *   - Extract ISO 9660 file entries from the data track.
 *   - Decode GRAPHICS.DAT / DUNGEON.DAT / IMG2.
 *   - Launch any FM Towns emulator.
 *
 * Those gates are tracked separately under docs/FIRESTAFF_GAP_LIST.md.
 *
 * Determinism
 * -----------
 * All sampling is bounded by sector count and a configurable
 * max_sectors_per_track cap. The module allocates no heap; receipts
 * are written into a caller-supplied fixed-capacity array.
 *
 * Public API surface
 * ------------------
 *   FirestaffFmtownsCd_AudioReceiptKind
 *       -- per-track verdict (REAL_AUDIO / SILENT_PER_DOC /
 *          SILENT_DETECTED / UNUSED_PER_DOC / BAD_HEADER /
 *          MISSING_BYTES / SHORT_TRACK / INCONSISTENT_WITH_DOC)
 *   FirestaffFmtownsCd_AudioReceipt
 *       -- one track's receipt: number, kind, sector_count,
 *          sampled_sectors, bytes_seen, max_abs_sample,
 *          mean_abs_sample, sample_range, expected_doc_role
 *   FirestaffFmtownsCd_AudioReceipts
 *       -- receipts array + counts
 *
 *   FirestaffFmtownsCd_AudioReceiptBuild(layout, bin_dir_or_null,
 *                                        in_memory_cdda_or_null,
 *                                        in_memory_cdda_len,
 *                                        receipts, max_receipts)
 *       -- build the receipts for every AUDIO track in the layout.
 *          Either bin_dir is non-NULL (real BIN file lookup) or
 *          in_memory_cdda is non-NULL (synthetic). Returns the
 *          number of audio receipts written, or -1 on error.
 *
 *   FirestaffFmtownsCd_AudioReceiptSelfTest()
 *       -- internal coverage used by the test driver + CTest.
 */

#ifndef FIRESTAFF_FMTOWNS_CD_AUDIO_TRACK_RECEIPT_H
#define FIRESTAFF_FMTOWNS_CD_AUDIO_TRACK_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#include "firestaff_fmtowns_cd_classify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Red Book CD-DA sector size in bytes. 588 stereo samples per channel
 * at 16-bit (2 bytes) == 2352 bytes per sector after the 16-byte sync
 * + header. For CD-DA tracks there is no subheader, so the whole
 * sector is 2352 bytes of little-endian 16-bit stereo PCM. */
#define FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES  2352u

/* Red Book CD-DA sample rate in Hz. */
#define FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SAMPLE_RATE   44100u

/* How many samples per sector (588 stereo frames = 1176 samples). */
#define FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SAMPLES_PER_SECTOR 1176u

/* Default hard cap on how many sectors to scan per track. Real FM
 * Towns CD-audio tracks are short (10s..3min); 300 sectors ~= 6.6
 * minutes which covers the longest track and stays data-free for
 * CI. Tests can pass a smaller cap to keep the synthetic case fast. */
#define FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_DEFAULT_MAX_SECTORS 300u

/* Mean-abs-sample threshold below which a track is considered
 * "silent-detected" (silence floor for 16-bit PCM). The CD-DA
 * format leaves the LSB undefined per the Red Book spec but every
 * real FM Towns master we've seen uses zero-fill for unused tracks,
 * so mean-abs < 8 is a strong silence signal. */
#define FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENCE_MEAN_THRESHOLD  8

/* Maximum audio receipts we'll return per layout. Matches the
 * existing FIRESTAFF_FMTOWNS_CD_MAX_TRACKS upper bound. */
#define FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MAX  FIRESTAFF_FMTOWNS_CD_MAX_TRACKS

/* Documented roles a track can have per DMWeb's FM-Towns CD-audio
 * track tables. */
typedef enum {
    FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_NONE       = 0,
    /* No documented per-track role; track is real audio. */
    FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_REAL       = 1,
    /* DMWeb explicitly says the track is unused. We accept
     * zero-filled or near-zero PCM as matching. */
    FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_UNUSED     = 2,
    /* DMWeb explicitly says the track is silent (DM1 track 20,
     * DM2 track 8). Same byte contract as UNUSED but the documented
     * duration is part of the receipt. */
    FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_SILENT     = 3,
    /* CSB: water-drop-like unused track. Three of these expected. */
    FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_WATER_DROP = 4
} FirestaffFmtownsCd_AudioDocRole;

typedef enum {
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_REAL_AUDIO        = 0,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_PER_DOC    = 1,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_DETECTED   = 2,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_UNUSED_PER_DOC     = 3,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_BAD_HEADER        = 4,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MISSING_BYTES     = 5,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SHORT_TRACK       = 6,
    FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT      = 7
} FirestaffFmtownsCd_AudioReceiptKind;

typedef struct {
    int track_number;
    FirestaffFmtownsCd_AudioReceiptKind kind;
    FirestaffFmtownsCd_AudioDocRole doc_role;

    /* Byte-range and shape observations. */
    unsigned long byte_offset;        /* byte offset inside the BIN file or in-memory stream */
    unsigned long bytes_seen;         /* bytes actually scanned */
    unsigned long sector_count_seen;  /* sectors actually scanned */
    unsigned long sampled_sectors;    /* sectors we sampled (bounded by max_sectors_per_track) */
    unsigned long expected_sector_size; /* always 2352 for CD-DA */

    /* Per-track PCM statistics from the sampled sectors. */
    int max_abs_sample;               /* 0..32767 */
    int mean_abs_sample;              /* 0..32767 */
    int sample_range;                 /* max_abs - mean_abs */

    /* Documented expected duration in seconds for documented silent
     * tracks. -1 when not documented. */
    int documented_seconds;

    /* "Looks like audio" signal: true if the PCM samples vary
     * enough across the sampled sectors to look like real audio. */
    int looks_like_audio;
} FirestaffFmtownsCd_AudioReceipt;

typedef struct {
    FirestaffFmtownsCd_AudioReceipt items[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MAX];
    int count;

    /* Aggregate counters that summarise the entire audio receipt
     * against the documented FM Towns CD-audio table. */
    int real_audio_count;
    int silent_count;
    int unused_count;
    int bad_count;
    int missing_count;

    /* CSB-only: how many of the unused tracks we marked as
     * WATER_DROP. DMWeb expects exactly 3. */
    int csb_water_drop_count;
} FirestaffFmtownsCd_AudioReceipts;

/* Build the per-track CD-audio receipts for every AUDIO track in
 * the parsed layout. Pass either:
 *
 *   bin_dir   != NULL  -- look up BIN files in this directory by
 *                         the layout's `files[]` names, and scan
 *                         the audio track byte ranges from disk.
 *   in_memory != NULL  -- use the single CD-DA byte stream as the
 *                         payload for every audio track. The byte
 *                         stream is sampled sequentially; each audio
 *                         track starts at offset
 *                           track_index * (in_memory_cdda_len /
 *                                          audio_track_count)
 *                         so a synthetic disc with N audio tracks
 *                         splits the stream evenly. (Real layout
 *                         receipts use bin_dir instead.)
 *
 * max_sectors_per_track caps how many sectors we scan per track.
 * Pass 0 to use the default.
 *
 * Returns the number of audio receipts written, or -1 on error.
 * receipts->count is also set to that number. */
int FirestaffFmtownsCd_AudioReceiptBuild(
    const FirestaffFmtownsCd_Layout *layout,
    const char *bin_dir,
    const unsigned char *in_memory_cdda,
    size_t in_memory_cdda_len,
    unsigned long max_sectors_per_track,
    FirestaffFmtownsCd_AudioReceipts *receipts);

/* Convenience: build receipts for a single AUDIO track's bytes.
 * Exposed so callers can feed a single track's byte slice from
 * another extractor. offset is the byte offset into the BIN/stream
 * where the track starts; byte_count is the total bytes available
 * (the receipt scans the smaller of byte_count and the per-track
 * sector cap). Returns 0 on success, -1 on error. */
int FirestaffFmtownsCd_AudioReceiptOne(
    int track_number,
    FirestaffFmtownsCd_Game game,
    const unsigned char *bytes,
    unsigned long byte_count,
    unsigned long byte_offset,
    unsigned long max_sectors_per_track,
    FirestaffFmtownsCd_AudioReceipt *out);

/* Run the internal self-test suite. Returns 0 on success, -1 on
 * any failure. Used by tests/test_firestaff_fmtowns_cd_audio_track_receipt.c. */
int FirestaffFmtownsCd_AudioReceiptSelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FMTOWNS_CD_AUDIO_TRACK_RECEIPT_H */
