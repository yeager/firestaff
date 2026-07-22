#ifndef FIRESTAFF_THERON_MEDIA_CLASSIFY_H
#define FIRESTAFF_THERON_MEDIA_CLASSIFY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_THERON_MEDIA_PATH_CAPACITY 512
#define FIRESTAFF_THERON_MEDIA_MAX_TRACKS 32

typedef enum {
    FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN = 0,
    FIRESTAFF_THERON_MEDIA_LAYOUT_RAW_BIN,
    FIRESTAFF_THERON_MEDIA_LAYOUT_ISO,
    FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE,
    FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE,
    FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY
} FirestaffTheronMediaLayout;

typedef struct {
    FirestaffTheronMediaLayout layout;
    int has_cue;
    int has_track02_data;
    int has_iso9660_pvd;
    int data_track_number;
    /* Exact MODE1 sector width declared for CUE Track 02: 2048 or 2352.
     * Zero means no uniquely declared Track 02 source record. */
    int track02_mode1_sector_bytes;
    /* Set only for exactly one CUE `TRACK 02 MODE1/2048|2352` with one
     * INDEX 01 and a BINARY FILE member. It is independent of CDDA presence:
     * missing audio must not make a hash-verifiable data track invalid. */
    int has_valid_track02_mode1;
    int audio_track_count;
    int ogg_file_count;
    int bin_file_count;
    int iso_file_count;
    int launch_candidate;
    char candidate_path[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    /* track02_path is populated only for a readable valid Track 02 member.
     * CDDA pairing is stricter and remains separately recorded below. */
    int has_track01_audio;
    int paired_track01_track02;
    char cue_path[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    char track01_path[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    char track02_path[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
} FirestaffTheronMediaStatus;

void FirestaffTheronMedia_Init(FirestaffTheronMediaStatus* status);

int FirestaffTheronMedia_ParseCue(const char* cue_text,
                                  size_t cue_len,
                                  FirestaffTheronMediaStatus* status);

int FirestaffTheronMedia_ClassifyPath(const char* path,
                                      FirestaffTheronMediaStatus* status);

int FirestaffTheronMedia_ClassifyDirectory(const char* root,
                                           FirestaffTheronMediaStatus* status);

/* Finds a strict CUE Track 01 AUDIO + Track 02 MODE1 pair whose
 * canonical Track 02 path is exactly the already hash-verified payload.
 * This is provenance matching, not filename-based discovery or extraction. */
int FirestaffTheronMedia_FindCuePairForTrack02(
    const char* root,
    const char* verified_track02_path,
    FirestaffTheronMediaStatus* status);

const char* FirestaffTheronMedia_LayoutId(FirestaffTheronMediaLayout layout);
const char* FirestaffTheronMedia_LayoutLabel(FirestaffTheronMediaLayout layout);

int FirestaffTheronMedia_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_MEDIA_CLASSIFY_H */
