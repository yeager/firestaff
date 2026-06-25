/*
 * firestaff_fmtowns_cd_classify.h
 *
 * Bounded FM Towns CD-image import classifier for DM1, CSB, and DM2.
 *
 * Purpose
 * -------
 * Firestaff needs to ingest redump-style BIN/CUE or ISO/CUE Compact
 * Disc images for the FM Towns releases of:
 *
 *   - Dungeon Master (DM1) v2.0, English/Japanese, November 1989.
 *     Tracks 02..20 are CD audio; 04/07/20 are unused (track 20 is
 *     20 seconds of silence); 05 covers entrance and map 6; 13 is
 *     game over; 18 is game won.
 *
 *   - Chaos Strikes Back (CSB) v3.1, English/Japanese, 1990-12-14.
 *     Tracks 02..31 are CD audio; includes three unused water-drop-
 *     like tracks.
 *
 *   - Dungeon Master II (DM2) v1.0, Japanese, 1994-01-28. CD audio
 *     follows the same six-track table as PC-9821/Japanese Mac;
 *     tracks 2..6 are slightly quieter, plus an extra silent track 8.
 *
 * Source of truth:
 *   docs/DMWEB_REFERENCE.md and the DMWeb FM-Towns edition pages
 *   for DM, CSB, and DM2. We do not ship or vendor any game data.
 *
 * Scope
 * -----
 * This module is a *layout* classifier only. It parses the redump
 * CUE sheet, extracts the disc track layout (track number, kind,
 * mode), counts audio tracks, and reports a candidate game/version
 * match. It does NOT extract ISO 9660 files, does NOT read game data
 * from the data track, and does NOT launch any FM Towns emulator.
 * Those gates are tracked separately under
 * docs/FIRESTAFF_GAP_LIST.md.
 *
 * The classifier is intentionally bounded so it can run in CI without
 * any user-supplied retail data. Tests build synthetic CUE sheets
 * in memory and validate parser behavior + game-candidate scoring.
 *
 * Public API surface
 * ------------------
 *   FirestaffFmtownsCd_Kind  -- track kind (data / audio / unknown)
 *   FirestaffFmtownsCd_Mode  -- data track sector mode (2352 / 2048)
 *   FirestaffFmtownsCd_Game  -- candidate FM Towns game family
 *   FirestaffFmtownsCd_Track
 *       -- one parsed track: number, kind, mode, audio_track_count
 *   FirestaffFmtownsCd_Layout
 *       -- parsed disc: array of tracks, totals, ISO 9660 PVD flag
 *   FirestaffFmtownsCd_ParseCue (cue_text, cue_len, &layout)
 *       -- bounded in-memory CUE sheet parser; no filesystem or malloc
 *   FirestaffFmtownsCd_Classify(&layout)
 *       -- game/version candidate + confidence from a parsed layout
 *   FirestaffFmtownsCd_SelfTest()
 *       -- internal coverage used by test driver + CTest
 */

#ifndef FIRESTAFF_FMTOWNS_CD_CLASSIFY_H
#define FIRESTAFF_FMTOWNS_CD_CLASSIFY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bounded parser limits. Redump CD images for FM Towns DM/CSB/DM2
 * are well under 100 tracks, so a fixed-capacity layout keeps the
 * classifier free of dynamic allocation. */
#define FIRESTAFF_FMTOWNS_CD_MAX_TRACKS        64
#define FIRESTAFF_FMTOWNS_CD_MAX_FILE_LINE     512
#define FIRESTAFF_FMTOWNS_CD_MAX_FILES         32

typedef enum {
    FIRESTAFF_FMTOWNS_CD_KIND_UNKNOWN = 0,
    FIRESTAFF_FMTOWNS_CD_KIND_DATA    = 1,
    FIRESTAFF_FMTOWNS_CD_KIND_AUDIO   = 2,
    FIRESTAFF_FMTOWNS_CD_KIND_OTHER   = 3
} FirestaffFmtownsCd_Kind;

typedef enum {
    FIRESTAFF_FMTOWNS_CD_MODE_UNKNOWN = 0,
    /* MODE1/2352: raw sectors with 16-byte header + 2048-byte user
     * data + 288-byte EDC/ECC. Standard redump split-track layout. */
    FIRESTAFF_FMTOWNS_CD_MODE_2352   = 1,
    /* MODE1/2048: cooked ISO 9660 sectors. Used when the BIN is a
     * plain ISO file instead of a raw 2352-byte-per-sector image. */
    FIRESTAFF_FMTOWNS_CD_MODE_2048   = 2,
    /* AUDIO: 2352 bytes/sector, 16-bit stereo PCM at 44100 Hz.
     * Red Book CD audio. */
    FIRESTAFF_FMTOWNS_CD_MODE_AUDIO  = 3
} FirestaffFmtownsCd_Mode;

typedef enum {
    FIRESTAFF_FMTOWNS_CD_GAME_NONE  = 0,
    FIRESTAFF_FMTOWNS_CD_GAME_DM1   = 1,
    FIRESTAFF_FMTOWNS_CD_GAME_CSB   = 2,
    FIRESTAFF_FMTOWNS_CD_GAME_DM2   = 3
} FirestaffFmtownsCd_Game;

typedef struct {
    int number;                       /* 1-based CD track number */
    FirestaffFmtownsCd_Kind kind;     /* data / audio / other */
    FirestaffFmtownsCd_Mode mode;     /* 2352 / 2048 / audio / unknown */
    int file_index;                   /* which FILE entry this track lives in */
    int has_index01;                  /* INDEX 01 marker present */
    int has_pregap;                   /* INDEX 00 marker present */
} FirestaffFmtownsCd_Track;

typedef struct {
    FirestaffFmtownsCd_Track tracks[FIRESTAFF_FMTOWNS_CD_MAX_TRACKS];
    int track_count;

    /* FILE entries (one per .bin / .iso). Each track records the
     * index into this array. */
    char     files[FIRESTAFF_FMTOWNS_CD_MAX_FILES][FIRESTAFF_FMTOWNS_CD_MAX_FILE_LINE];
    int      file_count;

    int data_track_count;             /* tracks of kind DATA */
    int audio_track_count;            /* tracks of kind AUDIO */
    int max_track_number;             /* highest track number seen */

    /* Set when the data track's first sector shows the standard
     * ISO 9660 Primary Volume Descriptor signature ("CD001" at
     * byte 16*2352+16+1 in a raw MODE1/2352 data track, or byte
     * 0x8001 in a cooked MODE1/2048 ISO). Detected only when a
     * BIN/ISO payload is supplied; the CUE-only parser leaves this 0. */
    int has_iso9660_pvd;
} FirestaffFmtownsCd_Layout;

/* Expected FM Towns CD-audio track counts per DMWeb. The track
 * numbers referenced here are the audio-track-only count (excluding
 * the data Track 01). */
#define FIRESTAFF_FMTOWNS_DM1_AUDIO_TRACKS_MIN  17  /* 02..18 game music */
#define FIRESTAFF_FMTOWNS_DM1_AUDIO_TRACKS_MAX  19  /* 02..20 including unused 04/07/20 */
#define FIRESTAFF_FMTOWNS_CSB_AUDIO_TRACKS_MIN  28  /* 02..29 game audio */
#define FIRESTAFF_FMTOWNS_CSB_AUDIO_TRACKS_MAX  30  /* 02..31 including unused */
#define FIRESTAFF_FMTOWNS_DM2_AUDIO_TRACKS_MIN   5  /* 02..06 (quieter) */
#define FIRESTAFF_FMTOWNS_DM2_AUDIO_TRACKS_MAX   8  /* 02..06 + silent 8 */

typedef struct {
    FirestaffFmtownsCd_Game game;
    int confidence;        /* 0..100 -- 100 = audio track count is exact, <=0 = no match */
    int audio_track_count;
    int max_track_number;
    int unused_track_match; /* 1 if the unused/silent-track pattern matches DMWeb notes */
} FirestaffFmtownsCd_Classification;

/* Parse a redump CUE sheet from a memory buffer. The parser is
 * line-oriented, tolerant of CRLF/LF line endings, ignores comments
 * (lines whose first non-whitespace character is '#'), and accepts
 * the documented redump keyword set:
 *
 *   FILE "name" BINARY|WAVE|AIFF|MOTOROLA
 *   TRACK NN MODE1/2352|MODE1/2048|AUDIO|...
 *   INDEX mm mm:ss:ff
 *   PREGAP mm:ss:ff
 *
 * Returns 0 on success and fills *layout. Returns -1 on parse
 * failure (truncated FILE entry, missing TRACK, malformed INDEX,
 * over-capacity track list, NULL pointer). On -1, layout is
 * zero-initialized so callers can safely bail. */
int FirestaffFmtownsCd_ParseCue(const char *cue_text,
                                size_t cue_len,
                                FirestaffFmtownsCd_Layout *layout);

/* Convenience: parse a CUE file from disk. Returns 0 on success. */
int FirestaffFmtownsCd_ParseCueFile(const char *cue_path,
                                    FirestaffFmtownsCd_Layout *layout);

/* Inspect the data track of a parsed layout and look for the ISO 9660
 * Primary Volume Descriptor signature. bin_path may be NULL when the
 * CUE only was parsed; in that case has_iso9660_pvd stays 0 and the
 * function returns 0. Returns 0 on success, -1 on file I/O error. */
int FirestaffFmtownsCd_DetectIso9660Pvd(FirestaffFmtownsCd_Layout *layout,
                                        const char *bin_path);

/* Score a parsed layout against the documented DMWeb FM Towns
 * CD-audio track tables. Returns the best candidate game family
 * and a confidence score; if no family matches, game is set to
 * NONE and confidence is 0. */
FirestaffFmtownsCd_Classification FirestaffFmtownsCd_Classify(
    const FirestaffFmtownsCd_Layout *layout);

/* Run the internal self-test suite. Returns 0 on success, -1 on
 * any failure. Used by tests/test_firestaff_fmtowns_cd_classify.c. */
int FirestaffFmtownsCd_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FMTOWNS_CD_CLASSIFY_H */
