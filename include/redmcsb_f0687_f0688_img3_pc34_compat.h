#ifndef FIRESTAFF_REDMCSB_F0687_F0688_IMG3_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0687_F0688_IMG3_PC34_COMPAT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct redmcsb_f0687_img3_stream_pc34_compat { const uint8_t *bytes; size_t byte_count,pixel_index; } redmcsb_f0687_img3_stream_pc34_compat;
bool redmcsb_f0687_img3_get_nibble_pc34_compat(redmcsb_f0687_img3_stream_pc34_compat *stream,uint8_t *out_nibble);
bool redmcsb_f0688_img3_get_pixel_count_pc34_compat(redmcsb_f0687_img3_stream_pc34_compat *stream,uint16_t *out_count);
const char *redmcsb_f0687_f0688_img3_source_evidence_pc34(void);
#endif
