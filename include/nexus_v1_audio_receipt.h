#ifndef NEXUS_V1_AUDIO_RECEIPT_H
#define NEXUS_V1_AUDIO_RECEIPT_H

#include <stdint.h>

#define NEXUS_V1_AUDIO_LEVEL_COUNT 16
#define NEXUS_V1_AUDIO_CDDA_TRACK_FIRST 2
#define NEXUS_V1_AUDIO_CDDA_TRACK_LAST 9
#define NEXUS_V1_AUDIO_CDDA_TRACK_COUNT 8

typedef enum {
    NEXUS_V1_AUDIO_KIND_UNKNOWN = 0,
    NEXUS_V1_AUDIO_KIND_SAL_BANK = 1,
    NEXUS_V1_AUDIO_KIND_MAP_TABLE = 2,
    NEXUS_V1_AUDIO_KIND_CDDA_LAYOUT = 3
} Nexus_V1_AudioKind;

typedef enum {
    NEXUS_V1_AUDIO_RECEIPT_UNKNOWN = 0,
    NEXUS_V1_AUDIO_RECEIPT_NAME_ONLY = 1,
    NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH = 2,
    NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH = 3,
    NEXUS_V1_AUDIO_RECEIPT_SIZE_MISMATCH = 4,
    NEXUS_V1_AUDIO_RECEIPT_HASH_MISMATCH = 5,
    NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_MATCH = 6,
    NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_PARTIAL = 7
} Nexus_V1_AudioReceiptClass;

typedef enum {
    NEXUS_V1_AUDIO_OK = 0,
    NEXUS_V1_AUDIO_ERR_NULL = -1,
    NEXUS_V1_AUDIO_ERR_BAD_NAME = -2,
    NEXUS_V1_AUDIO_ERR_BAD_LEVEL = -3,
    NEXUS_V1_AUDIO_ERR_BAD_KIND = -4,
    NEXUS_V1_AUDIO_ERR_BOUNDS = -5
} Nexus_V1_AudioStatus;

typedef struct {
    Nexus_V1_AudioKind kind;
    Nexus_V1_AudioReceiptClass receipt_class;
    int level_index;
    int cd_track;
    uint32_t expected_size;
    uint32_t observed_size;
    const char *expected_sha256;
    char expected_name[16];
} Nexus_V1_AudioReceipt;

typedef struct {
    Nexus_V1_AudioReceiptClass receipt_class;
    int data_track_count;
    int audio_track_count;
    int first_audio_track;
    int last_audio_track;
} Nexus_V1_CddaLayoutReceipt;

int nexus_v1_audio_expected_asset(Nexus_V1_AudioKind kind,
                                  int level_index,
                                  Nexus_V1_AudioReceipt *out);

int nexus_v1_audio_classify_file(const char *path,
                                 uint32_t size,
                                 const char *sha256,
                                 Nexus_V1_AudioReceipt *out);

int nexus_v1_audio_classify_cdda_layout(int data_track_count,
                                        int audio_track_count,
                                        int first_audio_track,
                                        int last_audio_track,
                                        Nexus_V1_CddaLayoutReceipt *out);

int nexus_v1_audio_cd_track_for_level_receipt(int level_index);

int nexus_v1_audio_decode_supported(Nexus_V1_AudioKind kind);

const char *nexus_v1_audio_kind_name(Nexus_V1_AudioKind kind);
const char *nexus_v1_audio_receipt_class_name(Nexus_V1_AudioReceiptClass cls);
const char *nexus_v1_audio_status_string(int status);
const char *nexus_v1_audio_source_evidence(void);

#endif
