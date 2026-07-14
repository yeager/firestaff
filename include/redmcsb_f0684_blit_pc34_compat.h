#ifndef FIRESTAFF_REDMCSB_F0684_BLIT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0684_BLIT_PC34_COMPAT_H
#include <stdint.h>
enum { REDMCSB_F0684_FLIP_HORIZONTAL=1, REDMCSB_F0684_FLIP_VERTICAL=2 };
typedef struct redmcsb_f0684_box_pc34_compat { int16_t left,right,top,bottom; } redmcsb_f0684_box_pc34_compat;
typedef void (*redmcsb_f0684_line_pc34_compat)(void *, int source_pixel, int destination_pixel, int pixel_count, int transparent_color);
typedef struct redmcsb_f0684_runtime_pc34_compat { redmcsb_f0684_line_pc34_compat forward, flipped; void *context; } redmcsb_f0684_runtime_pc34_compat;
int redmcsb_f0684_blit_pc34_compat(const redmcsb_f0684_box_pc34_compat *box,int16_t source_x,int16_t source_y,int16_t source_width,int16_t destination_width,int16_t transparent_color,int16_t flip,const redmcsb_f0684_runtime_pc34_compat *runtime);
const char *redmcsb_f0684_blit_source_evidence_pc34(void);
#endif
