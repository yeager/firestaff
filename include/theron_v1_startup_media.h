#ifndef THERON_V1_STARTUP_MEDIA_H
#define THERON_V1_STARTUP_MEDIA_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERON_STARTUP_MEDIA_ROSTER_CAPACITY 8u
#define THERON_STARTUP_MEDIA_PROMPT_CAPACITY 40u

typedef struct {
    int track02_variant;
    char track02_md5[33];
    size_t track02_size;
    int startup_media_ready;
    int startup_bitmap_decode_status;
    int startup_bitmap_sample_count;
    unsigned int startup_bitmap_route_mask;
    size_t startup_bitmap_nonzero_pixel_count;
    uint32_t startup_bitmap_checksum;
    int startup_roster_name_status;
    int startup_roster_name_count;
    char startup_roster_names[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                              [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    char startup_roster_titles[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                               [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
    int startup_text_prompt_status;
    int startup_text_prompt_count;
    char startup_text_prompt[THERON_STARTUP_MEDIA_PROMPT_CAPACITY];
} Theron_StartupMedia;

typedef struct {
    int track02_variant;
    char track02_md5[33];
    size_t track02_size;
    int startup_media_ready;
    int startup_bitmap_decode_status;
    int startup_bitmap_sample_count;
    unsigned int startup_bitmap_route_mask;
    size_t startup_bitmap_nonzero_pixel_count;
    uint32_t startup_bitmap_checksum;
    int startup_roster_name_status;
    int startup_roster_name_count;
    char startup_roster_names[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                              [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    char startup_roster_titles[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                               [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
    int startup_text_prompt_status;
    int startup_text_prompt_count;
    char startup_text_prompt[THERON_STARTUP_MEDIA_PROMPT_CAPACITY];
} Theron_StartupMediaStateReceipt;

void theron_v1_startup_media_init(Theron_StartupMedia *media);
void theron_v1_startup_media_state_receipt_init(
    Theron_StartupMediaStateReceipt *receipt);

void theron_v1_startup_media_capture_track02(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMedia *out_media);

void theron_v1_startup_media_capture_track02_state_receipt(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_StartupMediaStateReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_MEDIA_H */
