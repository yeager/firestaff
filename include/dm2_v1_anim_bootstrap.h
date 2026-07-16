#ifndef FIRESTAFF_DM2_V1_ANIM_BOOTSTRAP_H
#define FIRESTAFF_DM2_V1_ANIM_BOOTSTRAP_H

#include <stddef.h>
#include <stdint.h>

#define DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS 8

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    int argc;
    const char *argv[DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS];
    char command_line[96];
} DM2_V1_AnimBootstrapReceipt;

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    uint16_t width;
    uint16_t height;
    uint16_t even_width;
    uint32_t decoded_pixels;
    uint32_t consumed_bytes;
    uint32_t fill_run_count;
    uint32_t literal_run_count;
    uint32_t skip_run_count;
    uint32_t previous_row_run_count;
} DM2_V1_AnimDecodeImg1Receipt;

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    int handle;
    uint32_t requested_bytes;
    uint32_t transferred_bytes;
    uint32_t chunk_count;
    uint32_t file_size;
} DM2_V1_AnimFileReceipt;

typedef struct {
    uint8_t *arena;
    uint32_t arena_size;
    uint8_t active;
    uint8_t reset_count;
    uint32_t last_offset;
} DM2_V1_AnimStreamState;

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    uint8_t reset_stream;
    uint8_t blocked_missing_state;
    uint32_t receipt_hash;
} DM2_V1_AnimStreamResetReceipt;

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    uint32_t offset;
    uint32_t arena_size;
    uint8_t *ptr;
    uint8_t resolved_pointer;
    uint8_t blocked_missing_state;
    uint8_t blocked_inactive_stream;
    uint8_t blocked_out_of_bounds;
    uint32_t receipt_hash;
} DM2_V1_AnimStreamPtrReceipt;

typedef struct {
    int argc;
    const char *argv[DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS];
    const char *input_name;
    uint32_t input_size;
    uint32_t farcoreleft_bytes;
    uint32_t stream_capacity;
    uint8_t sound_available;
    uint8_t sound_reload_ok;
} DM2_V1_AnimMainRequest;

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    int argc;
    uint8_t parsed_filename;
    uint8_t flag_abort_on_event;
    uint8_t flag_loop;
    uint8_t flag_no_display;
    uint8_t flag_hide_after;
    uint8_t flag_clear_after;
    uint8_t flag_clear_before;
    uint8_t flag_force_stream;
    uint8_t flag_preload_file;
    uint8_t volume;
    uint16_t output_channel;
    uint32_t input_size;
    uint32_t farcoreleft_bytes;
    uint32_t stream_capacity;
    uint8_t requested_capture_int_ff;
    uint8_t requested_install_timer;
    uint8_t drained_pending_events;
    uint8_t allocated_screen_buffer;
    uint8_t allocated_sound_buffer;
    uint8_t requested_sound_reload;
    uint8_t stream_mode;
    uint8_t heap_mode;
    uint8_t requested_0759_0855;
    uint8_t requested_0759_0869;
    uint8_t requested_palette_zero;
    uint8_t requested_screen_clear_before;
    uint8_t requested_screen_clear_after;
    uint8_t requested_mouse_sound_release;
    uint8_t blocked_missing_request;
    uint8_t blocked_missing_input;
    uint8_t blocked_stream_capacity;
    uint32_t first_stream_offset;
    uint32_t receipt_hash;
} DM2_V1_AnimMainReceipt;

int dm2_v1_anim_bootstrap_swoosh(DM2_V1_AnimBootstrapReceipt *out_receipt);
int dm2_v1_anim_bootstrap_title(DM2_V1_AnimBootstrapReceipt *out_receipt);

int dm2_v1_anim_file_open(const char *filename,
                          DM2_V1_AnimFileReceipt *out_receipt);
uint32_t dm2_v1_anim_get_file_size(int handle,
                                   DM2_V1_AnimFileReceipt *out_receipt);
int dm2_v1_anim_read_huge_file(int handle,
                               uint32_t read_size,
                               uint8_t *buffer,
                               DM2_V1_AnimFileReceipt *out_receipt);
void dm2_v1_anim_file_close(int handle,
                            DM2_V1_AnimFileReceipt *out_receipt);
char *dm2_v1_anim_strcpy(char *dst,
                         const char *src,
                         DM2_V1_AnimFileReceipt *out_receipt);
int dm2_v1_anim_toupper(int value,
                        DM2_V1_AnimFileReceipt *out_receipt);
void *dm2_v1_anim_farmalloc(uint32_t size,
                            DM2_V1_AnimFileReceipt *out_receipt);
void dm2_v1_anim_farfree(void *buffer,
                         DM2_V1_AnimFileReceipt *out_receipt);
uint32_t dm2_v1_anim_farcoreleft(DM2_V1_AnimFileReceipt *out_receipt);
void dm2_v1_anim_stream_state_init(DM2_V1_AnimStreamState *state,
                                   uint8_t *arena,
                                   uint32_t arena_size);
int dm2_v1_anim_0759_0855_reset_stream(
    DM2_V1_AnimStreamState *state,
    DM2_V1_AnimStreamResetReceipt *out_receipt);
uint8_t *dm2_v1_anim_0759_0869_resolve_stream_ptr(
    DM2_V1_AnimStreamState *state,
    uint32_t offset,
    DM2_V1_AnimStreamPtrReceipt *out_receipt);
int dm2_v1_anim_0759_08e7_plan_main(
    const DM2_V1_AnimMainRequest *request,
    DM2_V1_AnimStreamState *stream_state,
    DM2_V1_AnimMainReceipt *out_receipt);

int dm2_v1_anim_setpixel_seq_4bpp(uint8_t *dst,
                                  size_t dst_size,
                                  uint16_t pixel_offset,
                                  uint8_t color);
int dm2_v1_anim_fill_seq_4bpp(uint8_t *dst,
                              size_t dst_size,
                              uint16_t pixel_offset,
                              uint8_t color,
                              uint16_t count);
int dm2_v1_anim_decode_img1(const uint8_t *src,
                            size_t src_size,
                            uint8_t *dst,
                            size_t dst_size,
                            DM2_V1_AnimDecodeImg1Receipt *out_receipt);
int dm2_v1_anim_blit_to_memory_row_4to4bpp(const uint8_t *src,
                                           size_t src_size,
                                           uint16_t off_src,
                                           uint8_t *dst,
                                           size_t dst_size,
                                           uint16_t off_dst,
                                           uint16_t width,
                                           DM2_V1_AnimFileReceipt *out_receipt);

#endif
