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
    int audio_track_count;
    int ogg_file_count;
    int bin_file_count;
    int iso_file_count;
    int launch_candidate;
    char candidate_path[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
} FirestaffTheronMediaStatus;

void FirestaffTheronMedia_Init(FirestaffTheronMediaStatus* status);

int FirestaffTheronMedia_ParseCue(const char* cue_text,
                                  size_t cue_len,
                                  FirestaffTheronMediaStatus* status);

int FirestaffTheronMedia_ClassifyPath(const char* path,
                                      FirestaffTheronMediaStatus* status);

int FirestaffTheronMedia_ClassifyDirectory(const char* root,
                                           FirestaffTheronMediaStatus* status);

const char* FirestaffTheronMedia_LayoutId(FirestaffTheronMediaLayout layout);
const char* FirestaffTheronMedia_LayoutLabel(FirestaffTheronMediaLayout layout);

int FirestaffTheronMedia_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_MEDIA_CLASSIFY_H */
