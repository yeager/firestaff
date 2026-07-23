#ifndef THERON_V1_CD_AUDIO_AVAILABILITY_H
#define THERON_V1_CD_AUDIO_AVAILABILITY_H

#include "theron_v1_track02.h"

/* Canonical Theron CD layout has 19 tracks.  Track 01 is the title intro,
 * Track 02 is the data track, Tracks 03-18 are the in-game CD-DA audio
 * tracks, and Track 19 is the final data extent.  Source-locked receipts
 * verify the CUE declares this exact layout before any audio output is
 * authorized. */
#define THERON_V1_CD_AUDIO_TRACK_COUNT 19u

typedef enum {
    THERON_V1_CD_AUDIO_MISSING = 0,
    THERON_V1_CD_AUDIO_FORMAT_MISMATCH = 1,
    THERON_V1_CD_AUDIO_READY = 2,
    THERON_V1_CD_AUDIO_CUE_NOT_FOUND = 3,
    THERON_V1_CD_AUDIO_CUE_PARSE_ERROR = 4,
    THERON_V1_CD_AUDIO_LAYOUT_MISMATCH = 5,
    THERON_V1_CD_AUDIO_TRACK_FILE_MISSING = 6
} Theron_V1CdAudioAvailability;

typedef struct {
    Theron_V1CdAudioAvailability availability;
    int playback_allowed;
    unsigned int track_count;
    unsigned int audio_track_count;
    unsigned int data_track_count;
    char audio_directory[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    /* track_paths and presence are 1-indexed by CD track number (1..19). */
    char track_paths[THERON_V1_CD_AUDIO_TRACK_COUNT + 1u]
                    [THERON_TRACK02_MOUNT_PATH_CAPACITY];
    int track_present[THERON_V1_CD_AUDIO_TRACK_COUNT + 1u];
    int track_is_audio[THERON_V1_CD_AUDIO_TRACK_COUNT + 1u];
    char unavailable_reason[160];
} Theron_V1CdAudioReceipt;

/* Build a source-locked CD-DA routing receipt from an original CUE sheet.
 *
 * cue_path  - path to the .cue file that declares the original CD track
 *             layout (e.g. TQUS.cue / TQJP.cue).
 * data_root - optional directory where the referenced files live.  If NULL
 *             the directory containing cue_path is used.
 *
 * The receipt parses the CUE, resolves each declared track file relative to
 * the data root, and allows a .ogg fallback when the CUE names a .wav file
 * (the locally staged original CD audio is supplied as OGG).  It verifies
 * the canonical 19-track Theron layout:
 *   Track 01 AUDIO, Track 02 MODE1/{2048,2352}, Tracks 03-18 AUDIO,
 *   Track 19 MODE1/{2048,2352}.
 *
 * playback_allowed is set only when every declared audio track (01, 03-18)
 * has a readable local file and the layout matches the original. */
Theron_V1CdAudioReceipt theron_v1_cd_audio_availability(
    const char *cue_path,
    const char *data_root);

#endif
