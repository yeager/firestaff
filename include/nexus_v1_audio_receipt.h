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
    NEXUS_V1_AUDIO_KIND_CDDA_LAYOUT = 3,
    NEXUS_V1_AUDIO_KIND_SOUND_DRIVER = 4
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

/* Byte-bound structure receipt for the retail SDDRVS.TSK sound-CPU image.
 * This is deliberately not a playback ABI: the source image's command
 * dispatch and PCM voice corridors are identified, while event ownership,
 * SAL codec semantics and host playback remain closed. */
typedef struct {
    int valid;
    uint32_t source_size;
    uint32_t code_entry_offset;
    uint32_t sound_cpu_ram_base;
    uint32_t work_ram_base;
    uint32_t stack_base;
    uint32_t command_dispatch_offset;
    uint32_t command_jump_table_offset;
    uint32_t command_jump_table_count;
    uint32_t pcm_voice_handler_offset;
    int m68k_instruction_stream_proven;
    int command_dispatch_proven;
    int pcm_voice_register_route_proven;
    int event_dispatch_proven;
    int playback_permitted;
} Nexus_V1_SddrvsDisassemblyReceipt;

/* Receipt for the only common on-disk SAL prefix observed in all sixteen
 * retail banks. It deliberately does not identify a file format, payload
 * boundary, codec, sample table, or playback ABI. */
typedef struct {
    int valid;
    uint32_t opaque_prefix_bytes;
    int signature_matches;
    int reserved_zero_bytes_match;
    int marker_matches;
    int codec_semantics_proven;
    int sample_semantics_proven;
    int playback_semantics_proven;
    int blocks_decode;
} Nexus_V1_SalOpaquePrefixReceipt;

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

int nexus_v1_audio_sal_opaque_prefix_receipt(
    const uint8_t *data,
    uint32_t size,
    Nexus_V1_SalOpaquePrefixReceipt *out);

/* Verify the authenticated retail SDDRVS.TSK's 68k loader/dispatch
 * corridors. The function accepts bytes only; callers must separately bind
 * the returned source to the canonical SDDRVS.TSK identity. */
int nexus_v1_audio_sddrvs_disassembly_receipt(
    const uint8_t *data,
    uint32_t size,
    Nexus_V1_SddrvsDisassemblyReceipt *out);

int nexus_v1_audio_cd_track_for_level_receipt(int level_index);

int nexus_v1_audio_decode_supported(Nexus_V1_AudioKind kind);

const char *nexus_v1_audio_kind_name(Nexus_V1_AudioKind kind);
const char *nexus_v1_audio_receipt_class_name(Nexus_V1_AudioReceiptClass cls);
const char *nexus_v1_audio_status_string(int status);
const char *nexus_v1_audio_source_evidence(void);

#endif
