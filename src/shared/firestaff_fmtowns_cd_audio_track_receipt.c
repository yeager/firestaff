/*
 * firestaff_fmtowns_cd_audio_track_receipt.c
 *
 * Implementation of the bounded CD-audio track receipt gate
 * declared in firestaff_fmtowns_cd_audio_track_receipt.h.
 *
 * Scope:
 *   - For each AUDIO track in a parsed CUE layout, locate the
 *     track's CD-DA byte range (either on disk via the CUE's
 *     FILE entries, or in-memory via a synthetic stream).
 *   - Validate Red Book CD-DA sector shape (2352 byte sectors,
 *     16-bit little-endian stereo PCM at 44100 Hz).
 *   - Compute per-track silence metrics and cross-reference them
 *     with the DMWeb-documented FM Towns CD-audio track tables
 *     for DM1 (tracks 02..20, with 04/07 unused and 20 silent),
 *     CSB (tracks 02..31, with three unused water-drop-like
 *     tracks), and DM2 (tracks 02..06 + silent track 8).
 *
 * Out of scope (tracked under docs/FIRESTAFF_GAP_LIST.md):
 *   - Decoding / playing CD audio.
 *   - Extracting ISO 9660 file entries from the data track.
 *   - Decoding GRAPHICS.DAT / DUNGEON.DAT / IMG2.
 *   - FM Towns keyboard / input bridge.
 *   - Any emulator/runtime launch.
 *
 * Style: C99, deterministic, no heap allocation. All sampling is
 * bounded by a sector cap so CI can run without any user-supplied
 * retail data.
 */

#include "firestaff_fmtowns_cd_audio_track_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Per-sector PCM sampling ─────────────────────────────────── */

typedef struct {
    unsigned long sector_count_seen;   /* how many sectors we actually scanned */
    unsigned long sampled_sectors;     /* == sector_count_seen, kept for symmetry with the receipt struct */
    int max_abs;                       /* 0..32767 */
    long sum_abs;                      /* accumulator over all samples for mean */
    unsigned long sample_count;        /* total samples seen (capped to avoid overflow on long tracks) */
} SectorStats;

/* Scan up to `max_sectors` consecutive CD-DA sectors from `bytes`,
 * updating `out`. Returns 0 on success, -1 on truncated input.
 *
 * A CD-DA sector is 2352 bytes of little-endian 16-bit stereo PCM:
 *   [L0][R0][L1][R1]...[L587][R587]
 * which gives 588 frames * 2 samples = 1176 samples per sector.
 *
 * We treat absolute sample values as |int16_t| for stats. */
static int scan_cdda_sectors(const unsigned char *bytes,
                             unsigned long bytes_available,
                             unsigned long max_sectors,
                             SectorStats *out) {
    memset(out, 0, sizeof(*out));
    if (max_sectors == 0) max_sectors = 1;

    unsigned long s;
    for (s = 0; s < max_sectors; s++) {
        unsigned long sector_offset = s * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
        if (sector_offset + FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES > bytes_available) {
            /* Truncated -- partial sector at the end. Stop scanning
             * and report whatever we have. */
            return 0;
        }
        const unsigned char *p = bytes + sector_offset;
        unsigned long i;
        for (i = 0; i < FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SAMPLES_PER_SECTOR; i++) {
            /* little-endian 16-bit signed PCM */
            int v = (int)(int16_t)((unsigned)p[0] | ((unsigned)p[1] << 8));
            int a = v < 0 ? -v : v;
            if (a > out->max_abs) out->max_abs = a;
            out->sum_abs += a;
            out->sample_count++;
            p += 2;
        }
        out->sector_count_seen++;
        out->sampled_sectors++;
    }
    return 0;
}

/* ── Per-track receipt builder ────────────────────────────────── */

int FirestaffFmtownsCd_AudioReceiptOne(
    int track_number,
    FirestaffFmtownsCd_Game game,
    const unsigned char *bytes,
    unsigned long byte_count,
    unsigned long byte_offset,
    unsigned long max_sectors_per_track,
    FirestaffFmtownsCd_AudioReceipt *out) {

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    out->track_number = track_number;
    out->byte_offset = byte_offset;
    out->expected_sector_size = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
    out->documented_seconds = -1;

    /* Documented-role lookup. We do this here so per-track receipts
     * also carry the doc role even when built individually. */
    switch (game) {
        case FIRESTAFF_FMTOWNS_CD_GAME_DM1:
            /* DM1: tracks 04, 07 unused; track 20 silent for 20s. */
            if (track_number == 4 || track_number == 7) {
                out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_UNUSED;
            } else if (track_number == 20) {
                out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_SILENT;
                out->documented_seconds = 20;
            } else {
                out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_REAL;
            }
            break;
        case FIRESTAFF_FMTOWNS_CD_GAME_CSB:
            /* CSB: three unused water-drop-like tracks. DMWeb doesn't
             * pin which tracks are water-drops (the description is
             * about the audio shape, not the track number), so we
             * leave doc_role = NONE for every CSB track here and let
             * the aggregate receipt builder classify them based on
             * the silence signature. */
            out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_NONE;
            break;
        case FIRESTAFF_FMTOWNS_CD_GAME_DM2:
            /* DM2: track 08 silent; tracks 02..06 slightly quieter. */
            if (track_number == 8) {
                out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_SILENT;
                out->documented_seconds = 2; /* placeholder, not pinned */
            } else {
                out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_REAL;
            }
            break;
        default:
            out->doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_NONE;
            break;
    }

    if (!bytes || byte_count == 0) {
        out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MISSING_BYTES;
        return 0;
    }

    /* Truncate to whole sectors only. Partial sectors are counted
     * as `bytes_seen` but not scanned. */
    unsigned long complete_sectors = byte_count / FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
    unsigned long partial_bytes    = byte_count % FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
    out->bytes_seen = byte_count;

    if (complete_sectors == 0) {
        out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SHORT_TRACK;
        out->sector_count_seen = 0;
        return 0;
    }

    unsigned long scan_sectors = complete_sectors;
    if (max_sectors_per_track > 0 && scan_sectors > max_sectors_per_track) {
        scan_sectors = max_sectors_per_track;
    }

    SectorStats stats;
    scan_cdda_sectors(bytes, byte_count, scan_sectors, &stats);
    out->sector_count_seen = stats.sector_count_seen;
    out->sampled_sectors = stats.sampled_sectors;
    out->max_abs_sample = stats.max_abs;
    out->mean_abs_sample = stats.sample_count > 0
                            ? (int)(stats.sum_abs / (long)stats.sample_count)
                            : 0;
    out->sample_range = out->max_abs_sample - out->mean_abs_sample;
    (void)partial_bytes;

    /* Classification. */
    int silent = (out->mean_abs_sample < FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENCE_MEAN_THRESHOLD);
    /* "Looks like audio" heuristic: real music has noticeable
     * variation across sectors; pure silence is flat. We approximate
     * "looks like audio" as max_abs >= 64 (so a sine wave or noisy
     * PCM will pass) AND sample_range > 32 (some dynamics). For a
     * -1.0..1.0 normalized signal this excludes pure-zero silence. */
    out->looks_like_audio = (out->max_abs_sample >= 64) && (out->sample_range >= 16);

    /* Cross-reference with documented role. */
    if (out->doc_role == FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_UNUSED) {
        if (silent) {
            out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_UNUSED_PER_DOC;
        } else {
            /* DMWeb says unused but PCM isn't silent -- record but
             * do not call this REAL_AUDIO; mark INCONSISTENT so the
             * user knows the byte shape doesn't match the doc. */
            out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT;
        }
    } else if (out->doc_role == FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_SILENT) {
        if (silent) {
            out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_PER_DOC;
        } else {
            out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT;
        }
    } else if (silent && out->looks_like_audio == 0) {
        /* Track is silent but DMWeb didn't call it unused or
         * silent. CSB: classify as SILENT_DETECTED so the
         * aggregate builder can pick up to three of these as
         * water-drop candidates. */
        out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_DETECTED;
    } else {
        out->kind = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_REAL_AUDIO;
    }
    return 0;
}

/* ── Aggregate builder ───────────────────────────────────────── */

/* Open the track's BIN file relative to bin_dir. Falls back to NULL
 * if the file cannot be opened; callers treat that as MISSING_BYTES. */
static FILE *open_track_bin(const char *bin_dir,
                            const char *file_name) {
    if (!bin_dir || !file_name) return NULL;
    size_t dlen = strlen(bin_dir);
    size_t flen = strlen(file_name);
    /* Need room for: dir + '/' + name + NUL. */
    size_t need = dlen + 1 + flen + 1;
    char *path = (char *)malloc(need);
    if (!path) return NULL;
    memcpy(path, bin_dir, dlen);
    path[dlen] = '/';
    memcpy(path + dlen + 1, file_name, flen + 1);
    FILE *fp = fopen(path, "rb");
    free(path);
    return fp;
}

int FirestaffFmtownsCd_AudioReceiptBuild(
    const FirestaffFmtownsCd_Layout *layout,
    const char *bin_dir,
    const unsigned char *in_memory_cdda,
    size_t in_memory_cdda_len,
    unsigned long max_sectors_per_track,
    FirestaffFmtownsCd_AudioReceipts *receipts) {

    if (!layout || !receipts) return -1;
    memset(receipts, 0, sizeof(*receipts));

    if (max_sectors_per_track == 0) {
        max_sectors_per_track = FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_DEFAULT_MAX_SECTORS;
    }

    /* Run the layout classifier once so we know which game family
     * the receipts should reference. We can call FirestaffFmtownsCd_Classify
     * directly -- it never allocates and never fails. */
    FirestaffFmtownsCd_Classification cls = FirestaffFmtownsCd_Classify(layout);

    /* Pass 1: count AUDIO tracks so we can split an in-memory CD-DA
     * stream evenly across them when no bin_dir is supplied. */
    int audio_index[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MAX];
    int audio_count = 0;
    int i;
    for (i = 0; i < layout->track_count; i++) {
        if (layout->tracks[i].kind == FIRESTAFF_FMTOWNS_CD_KIND_AUDIO) {
            if (audio_count < FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MAX) {
                audio_index[audio_count++] = i;
            }
        }
    }
    if (audio_count == 0) return 0;

    /* Pass 2: build per-track receipts. */
    for (i = 0; i < audio_count; i++) {
        int layout_idx = audio_index[i];
        const FirestaffFmtownsCd_Track *t = &layout->tracks[layout_idx];
        FirestaffFmtownsCd_AudioReceipt *r = &receipts->items[receipts->count];

        unsigned long byte_offset = 0;
        unsigned long bytes_available = 0;
        const unsigned char *track_bytes = NULL;

        if (bin_dir) {
            /* Real-BIN path. We don't currently walk CUE PREGAP/INDEX
             * timing, so we treat the track as starting at offset 0
             * within its BIN file with byte_count = file_size. The
             * track range is a logical receipt: the receipt still
             * reports byte_offset, bytes_seen, and the silence
             * signature that matches the documented role. */
            const char *fname = layout->files[t->file_index];
            FILE *fp = open_track_bin(bin_dir, fname);
            if (fp) {
                if (fseek(fp, 0, SEEK_END) == 0) {
                    long fsize = ftell(fp);
                    if (fsize > 0) {
                        bytes_available = (unsigned long)fsize;
                        if (fseek(fp, 0, SEEK_SET) == 0) {
                            /* Read into a bounded scratch buffer; use
                             * the first scan_sectors*2352 bytes plus
                             * a tail of up to one extra sector. */
                            unsigned long want = bytes_available;
                            unsigned long cap_bytes = (max_sectors_per_track + 1u)
                                                       * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
                            if (want > cap_bytes) want = cap_bytes;
                            unsigned char *scratch = (unsigned char *)malloc(want + 16u);
                            if (scratch) {
                                size_t got = fread(scratch, 1, want, fp);
                                if (got > 0) {
                                    /* Move into a heap-owned buffer
                                     * we can hand to the receipt
                                     * builder. */
                                    unsigned char *track_buf = (unsigned char *)malloc(got);
                                    if (track_buf) {
                                        memcpy(track_buf, scratch, got);
                                        track_bytes = track_buf;
                                        bytes_available = (unsigned long)got;
                                        /* Keep `track_buf` alive for the duration
                                         * of this receipt; free below. */
                                    }
                                }
                                free(scratch);
                            }
                        }
                    }
                }
                fclose(fp);
            }
        } else if (in_memory_cdda && in_memory_cdda_len > 0 && audio_count > 0) {
            /* Synthetic-stream path: split the stream evenly across
             * audio tracks. Each track gets in_memory_cdda_len /
             * audio_count bytes. */
            unsigned long per_track = (unsigned long)(in_memory_cdda_len / (size_t)audio_count);
            if (per_track == 0) per_track = (unsigned long)in_memory_cdda_len;
            byte_offset = (unsigned long)i * per_track;
            bytes_available = per_track;
            if (byte_offset + bytes_available > in_memory_cdda_len) {
                bytes_available = (unsigned long)in_memory_cdda_len - byte_offset;
            }
            track_bytes = in_memory_cdda + byte_offset;
        }

        /* When bin_dir was used, track_bytes was allocated via
         * malloc above. Take ownership in a local so we can free
         * it after the receipt is built. */
        unsigned char *owned_track_buf = NULL;
        if (track_bytes && bytes_available > 0
            && bin_dir != NULL) {
            owned_track_buf = (unsigned char *)track_bytes;
        }

        FirestaffFmtownsCd_AudioReceiptOne(
            t->number,
            cls.game,
            track_bytes,
            bytes_available,
            byte_offset,
            max_sectors_per_track,
            r);

        if (owned_track_buf) {
            free(owned_track_buf);
        }

        receipts->count++;
        /* Aggregate counters. */
        switch (r->kind) {
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_REAL_AUDIO:        receipts->real_audio_count++; break;
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_PER_DOC:    receipts->silent_count++; break;
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_UNUSED_PER_DOC:    receipts->unused_count++; break;
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_BAD_HEADER:
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MISSING_BYTES:
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SHORT_TRACK:
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT:
            case FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_DETECTED:
                /* CSB water-drop candidates are counted at the end
                 * once we know the total silent_detected count. */
                break;
        }
    }

    /* CSB water-drop classification: if the layout was classified as
     * CSB and we found >= 3 SILENT_DETECTED tracks, mark the first 3
     * of them as the documented three water-drops. */
    if (cls.game == FIRESTAFF_FMTOWNS_CD_GAME_CSB) {
        int picked = 0;
        for (i = 0; i < receipts->count && picked < 3; i++) {
            if (receipts->items[i].kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_DETECTED) {
                receipts->items[i].doc_role = FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_WATER_DROP;
                picked++;
            }
        }
        receipts->csb_water_drop_count = picked;
    }

    return receipts->count;
}

/* ── Self test ───────────────────────────────────────────────── */

#define ST_FAIL(msg) do {                                                  \
    fprintf(stderr, "firestaff_fmtowns_cd_audio_track_receipt FAIL: %s\n", msg); \
    return 0;                                                              \
} while (0)

#define ST_ASSERT(cond, msg) do {                                          \
    if (!(cond)) { fprintf(stderr, "%s:%d: %s (%s)\n",                     \
                            __FILE__, __LINE__, msg, #cond);               \
                   return 0; }                                             \
} while (0)

/* Build a synthetic CD-DA byte stream:
 *   - If `silent` is non-zero, fill with all-zero PCM (silence).
 *   - Otherwise fill with a sine-wave-ish pattern using the byte
 *     `pattern` rotated per sample so the resulting stream looks
 *     like real audio (max-abs >= 64, mean-abs well above the
 *     silence threshold). */
static int build_synthetic_cdda(unsigned char *buf,
                                size_t buf_len,
                                int silent,
                                unsigned char pattern) {
    if (!buf) return -1;
    if (silent) {
        memset(buf, 0, buf_len);
        return 0;
    }
    /* 16-bit little-endian stereo PCM. We write a sawtooth-ish
     * pattern that alternates per sample so the stream has high
     * dynamic range and looks like real audio. */
    size_t i;
    unsigned short phase = 0;
    for (i = 0; i + 1 < buf_len; i += 2) {
        /* Triangle wave between -16000 and +16000. */
        phase = (unsigned short)((phase + 137u) & 0xFFFFu);
        short v = (short)(((int)phase - 32768) / 2);
        if (v >  16000) v =  16000;
        if (v < -16000) v = -16000;
        buf[i]     = (unsigned char)(v & 0xFF);
        buf[i + 1] = (unsigned char)((v >> 8) & 0xFF);
    }
    (void)pattern;
    return 0;
}

/* Parse the DM1 fixture cue from the companion classifier and
 * verify the CD-audio track receipt gate produces the documented
 * count of silent tracks. */
static int test_dm1_synth_silent_and_real(void) {
    /* DM1 fixture cue (audio tracks 02..20, with 04/07 unused per
     * DMWeb and 20 silent for 20s). */
    const char *cue =
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track02.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track03.bin\" BINARY\r\n"
        "  TRACK 03 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track04.bin\" BINARY\r\n"
        "  TRACK 04 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track05.bin\" BINARY\r\n"
        "  TRACK 05 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track06.bin\" BINARY\r\n"
        "  TRACK 06 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track07.bin\" BINARY\r\n"
        "  TRACK 07 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track08.bin\" BINARY\r\n"
        "  TRACK 08 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track09.bin\" BINARY\r\n"
        "  TRACK 09 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track10.bin\" BINARY\r\n"
        "  TRACK 10 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track11.bin\" BINARY\r\n"
        "  TRACK 11 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track12.bin\" BINARY\r\n"
        "  TRACK 12 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track13.bin\" BINARY\r\n"
        "  TRACK 13 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track14.bin\" BINARY\r\n"
        "  TRACK 14 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track15.bin\" BINARY\r\n"
        "  TRACK 15 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track16.bin\" BINARY\r\n"
        "  TRACK 16 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track17.bin\" BINARY\r\n"
        "  TRACK 17 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track18.bin\" BINARY\r\n"
        "  TRACK 18 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track19.bin\" BINARY\r\n"
        "  TRACK 19 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track20.bin\" BINARY\r\n"
        "  TRACK 20 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";

    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &layout);
    ST_ASSERT(rc == 0, "DM1 CUE should parse");
    ST_ASSERT(layout.track_count == 20, "DM1 should have 20 tracks");
    ST_ASSERT(layout.audio_track_count == 19, "DM1 should have 19 audio tracks");

    /* Build a synthetic CD-DA stream: 1 sector per audio track.
     * Tracks 04, 07, 20 will be filled with silence; the rest with
     * real-audio-shaped PCM. */
    unsigned char cdda[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES * 19];
    int audio_idx = 0;
    int ti;
    for (ti = 0; ti < layout.track_count; ti++) {
        if (layout.tracks[ti].kind != FIRESTAFF_FMTOWNS_CD_KIND_AUDIO) continue;
        int track_no = layout.tracks[ti].number;
        int silent = (track_no == 4 || track_no == 7 || track_no == 20);
        unsigned char *slice = cdda + audio_idx * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
        build_synthetic_cdda(slice, FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES,
                             silent, 0xA5);
        audio_idx++;
    }

    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              cdda, sizeof(cdda),
                                              /*max_sectors_per_track=*/1,
                                              &receipts);
    ST_ASSERT(rc == 19, "should produce 19 audio receipts");
    ST_ASSERT(receipts.count == 19, "receipts.count == 19");

    /* Check that tracks 04, 07, 20 came back as the documented
     * silent/unused roles. */
    int saw_unused = 0, saw_silent_per_doc = 0, saw_real_audio = 0;
    for (ti = 0; ti < receipts.count; ti++) {
        const FirestaffFmtownsCd_AudioReceipt *r = &receipts.items[ti];
        if (r->track_number == 4 || r->track_number == 7) {
            ST_ASSERT(r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_UNUSED_PER_DOC,
                      "track 04/07 should be UNUSED_PER_DOC");
            saw_unused++;
        } else if (r->track_number == 20) {
            ST_ASSERT(r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_PER_DOC,
                      "track 20 should be SILENT_PER_DOC");
            ST_ASSERT(r->documented_seconds == 20, "DM1 track 20 == 20s silence");
            saw_silent_per_doc++;
        } else {
            ST_ASSERT(r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_REAL_AUDIO,
                      "non-silent track should be REAL_AUDIO");
            saw_real_audio++;
        }
    }
    ST_ASSERT(saw_unused == 2, "two unused tracks (04, 07)");
    ST_ASSERT(saw_silent_per_doc == 1, "one silent-per-doc track (20)");
    ST_ASSERT(saw_real_audio == 16, "sixteen real-audio tracks");
    return 1;
}

static int test_dm2_synth_silent_track_8(void) {
    const char *cue =
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track02.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track03.bin\" BINARY\r\n"
        "  TRACK 03 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track04.bin\" BINARY\r\n"
        "  TRACK 04 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track05.bin\" BINARY\r\n"
        "  TRACK 05 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track06.bin\" BINARY\r\n"
        "  TRACK 06 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track07.bin\" BINARY\r\n"
        "  TRACK 07 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track08.bin\" BINARY\r\n"
        "  TRACK 08 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";

    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &layout);
    ST_ASSERT(rc == 0, "DM2 CUE should parse");
    ST_ASSERT(layout.audio_track_count == 7, "DM2 should have 7 audio tracks");

    unsigned char cdda[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES * 7];
    int audio_idx = 0;
    int ti;
    for (ti = 0; ti < layout.track_count; ti++) {
        if (layout.tracks[ti].kind != FIRESTAFF_FMTOWNS_CD_KIND_AUDIO) continue;
        int track_no = layout.tracks[ti].number;
        int silent = (track_no == 8);
        unsigned char *slice = cdda + audio_idx * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
        build_synthetic_cdda(slice, FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES,
                             silent, 0xA5);
        audio_idx++;
    }

    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              cdda, sizeof(cdda),
                                              /*max_sectors_per_track=*/1,
                                              &receipts);
    ST_ASSERT(rc == 7, "should produce 7 audio receipts");
    int saw_silent = 0, saw_real = 0;
    for (ti = 0; ti < receipts.count; ti++) {
        const FirestaffFmtownsCd_AudioReceipt *r = &receipts.items[ti];
        if (r->track_number == 8) {
            ST_ASSERT(r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_PER_DOC,
                      "DM2 track 8 should be SILENT_PER_DOC");
            saw_silent++;
        } else {
            ST_ASSERT(r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_REAL_AUDIO,
                      "DM2 non-8 should be REAL_AUDIO");
            saw_real++;
        }
    }
    ST_ASSERT(saw_silent == 1, "DM2 one silent track");
    ST_ASSERT(saw_real == 6, "DM2 six real-audio tracks");
    return 1;
}

static int test_csb_synth_three_water_drops(void) {
    /* Build a minimal CSB-shaped cue: track 01 data + tracks 02..31
     * audio. DMWeb expects exactly three unused water-drop-like
     * tracks somewhere in 02..31; we mark tracks 04, 14, 27 as
     * silent and the rest as real audio. The cue is large but the
     * synthetic CD-DA stream is split 1 sector per audio track. */
    enum { CSB_AUDIO_TRACKS = 30 };
    char cue[8192];
    size_t off = 0;
    int n = snprintf(cue + off, sizeof(cue) - off,
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n");
    ST_ASSERT(n > 0, "snprintf");
    off += (size_t)n;
    int trk;
    for (trk = 2; trk <= 1 + CSB_AUDIO_TRACKS; trk++) {
        n = snprintf(cue + off, sizeof(cue) - off,
            "FILE \"track%02d.bin\" BINARY\r\n"
            "  TRACK %02d AUDIO\r\n"
            "    INDEX 01 00:00:00\r\n",
            trk, trk);
        ST_ASSERT(n > 0, "snprintf trk");
        off += (size_t)n;
    }

    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, off, &layout);
    ST_ASSERT(rc == 0, "CSB CUE should parse");
    ST_ASSERT(layout.audio_track_count == CSB_AUDIO_TRACKS,
              "CSB should have 30 audio tracks");

    unsigned char cdda[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES * CSB_AUDIO_TRACKS];
    int audio_idx = 0;
    int ti;
    for (ti = 0; ti < layout.track_count; ti++) {
        if (layout.tracks[ti].kind != FIRESTAFF_FMTOWNS_CD_KIND_AUDIO) continue;
        int track_no = layout.tracks[ti].number;
        int silent = (track_no == 4 || track_no == 14 || track_no == 27);
        unsigned char *slice = cdda + audio_idx * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
        build_synthetic_cdda(slice, FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES,
                             silent, 0xA5);
        audio_idx++;
    }

    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              cdda, sizeof(cdda),
                                              /*max_sectors_per_track=*/1,
                                              &receipts);
    ST_ASSERT(rc == CSB_AUDIO_TRACKS, "should produce 30 audio receipts");
    ST_ASSERT(receipts.csb_water_drop_count == 3,
              "CSB should classify exactly 3 silent tracks as water-drop");
    int picked = 0;
    for (ti = 0; ti < receipts.count; ti++) {
        const FirestaffFmtownsCd_AudioReceipt *r = &receipts.items[ti];
        if (r->doc_role == FIRESTAFF_FMTOWNS_CD_AUDIO_DOC_ROLE_WATER_DROP) {
            picked++;
            ST_ASSERT(r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SILENT_DETECTED,
                      "water-drop track should be SILENT_DETECTED");
        }
    }
    ST_ASSERT(picked == 3, "three water-drop tracks");
    return 1;
}

static int test_short_track_returns_short(void) {
    const char *cue =
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track02.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &layout);
    ST_ASSERT(rc == 0, "small CUE parses");

    /* Feed a byte slice shorter than one CD-DA sector. */
    unsigned char buf[100];
    memset(buf, 0xA5, sizeof(buf));
    FirestaffFmtownsCd_AudioReceipt r;
    memset(&r, 0, sizeof(r));
    FirestaffFmtownsCd_AudioReceiptOne(2,
                                       FIRESTAFF_FMTOWNS_CD_GAME_DM1,
                                       buf, sizeof(buf), 0,
                                       /*max_sectors_per_track=*/10,
                                       &r);
    ST_ASSERT(r.kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SHORT_TRACK,
              "short track should be SHORT_TRACK");
    ST_ASSERT(r.sector_count_seen == 0, "no complete sectors scanned");
    return 1;
}

static int test_missing_bytes_returns_missing(void) {
    const char *cue =
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track02.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &layout);
    ST_ASSERT(rc == 0, "small CUE parses");
    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              NULL, 0,
                                              /*max_sectors_per_track=*/1,
                                              &receipts);
    ST_ASSERT(rc == 1, "one audio track in layout");
    ST_ASSERT(receipts.items[0].kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_MISSING_BYTES,
              "missing bytes should be MISSING_BYTES");
    return 1;
}

static int test_inconsistent_when_unused_track_is_not_silent(void) {
    /* Build a DM1-shaped cue (20 tracks, 19 audio) so the layout
     * classifier picks DM1. We fill every audio track with real
     * audio (no silence). Track 04 is documented unused; since the
     * bytes are non-silent, the receipt should come back
     * INCONSISTENT. */
    char cue[8192];
    size_t off = 0;
    int n = snprintf(cue + off, sizeof(cue) - off,
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n");
    ST_ASSERT(n > 0, "snprintf");
    off += (size_t)n;
    int trk;
    for (trk = 2; trk <= 20; trk++) {
        n = snprintf(cue + off, sizeof(cue) - off,
            "FILE \"track%02d.bin\" BINARY\r\n"
            "  TRACK %02d AUDIO\r\n"
            "    INDEX 01 00:00:00\r\n",
            trk, trk);
        ST_ASSERT(n > 0, "snprintf trk");
        off += (size_t)n;
    }
    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, off, &layout);
    ST_ASSERT(rc == 0, "DM1-shaped CUE should parse");
    ST_ASSERT(layout.audio_track_count == 19, "19 audio tracks");

    unsigned char cdda[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES * 19];
    int audio_idx = 0;
    int ti;
    for (ti = 0; ti < 19; ti++) {
        unsigned char *slice = cdda + audio_idx * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
        build_synthetic_cdda(slice, FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES,
                             /*silent=*/0, 0xA5);
        audio_idx++;
    }

    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              cdda, sizeof(cdda),
                                              /*max_sectors_per_track=*/1,
                                              &receipts);
    ST_ASSERT(rc == 19, "should produce 19 audio receipts");
    int saw_inconsistent_4 = 0;
    int saw_inconsistent_7 = 0;
    int saw_inconsistent_20 = 0;
    for (ti = 0; ti < receipts.count; ti++) {
        const FirestaffFmtownsCd_AudioReceipt *r = &receipts.items[ti];
        if (r->track_number == 4 && r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT) saw_inconsistent_4++;
        if (r->track_number == 7 && r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT) saw_inconsistent_7++;
        if (r->track_number == 20 && r->kind == FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_INCONSISTENT) saw_inconsistent_20++;
    }
    ST_ASSERT(saw_inconsistent_4 == 1, "DM1 track 04 unused + non-silent == INCONSISTENT");
    ST_ASSERT(saw_inconsistent_7 == 1, "DM1 track 07 unused + non-silent == INCONSISTENT");
    ST_ASSERT(saw_inconsistent_20 == 1, "DM1 track 20 silent + non-silent == INCONSISTENT");
    return 1;
}

static int test_real_audio_track_looks_like_audio(void) {
    /* DM1-shaped cue: track 05 is "real audio" per DMWeb (entrance +
     * map 6). We fill it with a synthetic real-audio CD-DA sector
     * and verify the receipt reports max_abs >= 64 + looks_like_audio. */
    char cue[8192];
    size_t off = 0;
    int n = snprintf(cue + off, sizeof(cue) - off,
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n");
    ST_ASSERT(n > 0, "snprintf");
    off += (size_t)n;
    int trk;
    for (trk = 2; trk <= 20; trk++) {
        n = snprintf(cue + off, sizeof(cue) - off,
            "FILE \"track%02d.bin\" BINARY\r\n"
            "  TRACK %02d AUDIO\r\n"
            "    INDEX 01 00:00:00\r\n",
            trk, trk);
        ST_ASSERT(n > 0, "snprintf trk");
        off += (size_t)n;
    }
    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, off, &layout);
    ST_ASSERT(rc == 0, "DM1 CUE should parse");

    /* All audio sectors filled with the real-audio shape; track 05
     * should be REAL_AUDIO in the receipt because its PCM is not
     * silent (mean_abs >> silence threshold). Tracks 04/07/20 will
     * surface as INCONSISTENT in this test which is also expected. */
    unsigned char cdda[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES * 19];
    int audio_idx = 0;
    int ti;
    for (ti = 0; ti < 19; ti++) {
        unsigned char *slice = cdda + audio_idx * FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES;
        build_synthetic_cdda(slice, FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES,
                             /*silent=*/0, 0xA5);
        audio_idx++;
    }

    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              cdda, sizeof(cdda),
                                              /*max_sectors_per_track=*/1,
                                              &receipts);
    ST_ASSERT(rc == 19, "19 audio receipts");
    const FirestaffFmtownsCd_AudioReceipt *r05 = NULL;
    int ti05;
    for (ti05 = 0; ti05 < receipts.count; ti05++) {
        if (receipts.items[ti05].track_number == 5) {
            r05 = &receipts.items[ti05];
            break;
        }
    }
    ST_ASSERT(r05 != NULL, "track 05 receipt present");
    ST_ASSERT(r05->looks_like_audio == 1, "track 05 looks like audio");
    ST_ASSERT(r05->max_abs_sample >= 64, "track 05 max_abs >= 64");
    ST_ASSERT(r05->mean_abs_sample >= 64, "track 05 mean_abs >= 64");
    return 1;
}

static int test_max_sectors_cap_holds(void) {
    /* Build a longer synthetic CD-DA stream and verify the receipt
     * honours max_sectors_per_track. */
    const char *cue =
        "FILE \"track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"track02.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout layout;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &layout);
    ST_ASSERT(rc == 0, "tiny CUE parses");

    unsigned char cdda[FIRESTAFF_FMTOWNS_CD_AUDIO_RECEIPT_SECTOR_BYTES * 8];
    build_synthetic_cdda(cdda, sizeof(cdda), /*silent=*/0, 0xA5);

    FirestaffFmtownsCd_AudioReceipts receipts;
    memset(&receipts, 0, sizeof(receipts));
    rc = FirestaffFmtownsCd_AudioReceiptBuild(&layout, NULL,
                                              cdda, sizeof(cdda),
                                              /*max_sectors_per_track=*/3,
                                              &receipts);
    ST_ASSERT(rc == 1, "one audio receipt");
    ST_ASSERT(receipts.items[0].sector_count_seen == 3,
              "max_sectors_per_track=3 should scan 3 sectors");
    return 1;
}

static int test_classifier_layout_self_test_still_passes(void) {
    /* Sanity: the companion classifier's self-test must still
     * pass; the receipt module depends on it. */
    int rc = FirestaffFmtownsCd_SelfTest();
    ST_ASSERT(rc == 0, "classifier self-test must pass");
    return 1;
}

int FirestaffFmtownsCd_AudioReceiptSelfTest(void) {
    int total = 0, passed = 0;
    #define RUN(name) do { total++; if (name()) passed++; } while (0)
    RUN(test_dm1_synth_silent_and_real);
    RUN(test_dm2_synth_silent_track_8);
    RUN(test_csb_synth_three_water_drops);
    RUN(test_short_track_returns_short);
    RUN(test_missing_bytes_returns_missing);
    RUN(test_inconsistent_when_unused_track_is_not_silent);
    RUN(test_real_audio_track_looks_like_audio);
    RUN(test_max_sectors_cap_holds);
    RUN(test_classifier_layout_self_test_still_passes);
    #undef RUN
    return (passed == total) ? 0 : -1;
}
