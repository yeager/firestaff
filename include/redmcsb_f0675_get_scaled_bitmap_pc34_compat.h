#ifndef FIRESTAFF_REDMCSB_F0675_GET_SCALED_BITMAP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0675_GET_SCALED_BITMAP_PC34_COMPAT_H
#include <stdbool.h>
#include <stdint.h>
typedef struct { uint8_t *pixels; int16_t width, height; } RedmcsbF0675BitmapPc34Compat;
typedef struct { RedmcsbF0675BitmapPc34Compat *(*get_native)(void *,uint16_t); bool (*is_cached)(void *,int16_t); RedmcsbF0675BitmapPc34Compat *(*get_derived)(void *,int16_t), *(*get_temporary)(void *); bool (*shrink)(void *,const RedmcsbF0675BitmapPc34Compat *,RedmcsbF0675BitmapPc34Compat *,int16_t,int16_t,const uint8_t *); void (*add_cache)(void *,int16_t); } RedmcsbF0675BitmapOpsPc34Compat;
const RedmcsbF0675BitmapPc34Compat *redmcsb_f0675_get_scaled_bitmap_pc34_compat(void *,const RedmcsbF0675BitmapOpsPc34Compat *,uint16_t,int16_t,int16_t,int16_t,const uint8_t *);
const char *redmcsb_f0675_get_scaled_bitmap_source_evidence_pc34(void);
#endif
