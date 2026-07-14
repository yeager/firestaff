#ifndef FIRESTAFF_DM1_V1_F0461_START_ALLOCATE_FLIPPED_WALL_BITMAPS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0461_START_ALLOCATE_FLIPPED_WALL_BITMAPS_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F0461 reserves the five persistent, pre-flipped wall buffers.  F0096
 * later derives each buffer by calling F0099; this interface describes that
 * ownership without allocating or populating graphic bytes.
 */
enum {
    DM1_V1_F0461_FLIPPED_WALL_BITMAP_COUNT_PC34 = 5
};

typedef enum {
    DM1_V1_F0461_FLIPPED_WALL_D3LCR_PC34 = 0,
    DM1_V1_F0461_FLIPPED_WALL_D2LCR_PC34 = 1,
    DM1_V1_F0461_FLIPPED_WALL_D1LCR_PC34 = 2,
    DM1_V1_F0461_FLIPPED_WALL_D0L_PC34 = 3,
    DM1_V1_F0461_FLIPPED_WALL_D0R_PC34 = 4
} DM1_V1_F0461FlippedWallBitmapPc34;

typedef struct {
    DM1_V1_F0461FlippedWallBitmapPc34 bitmap;
    const char *native_symbol;
    const char *flipped_symbol;
    size_t row_byte_width;
    size_t height;
    size_t allocation_byte_count;
} DM1_V1_F0461FlippedWallBitmapPlanPc34;

typedef struct {
    size_t allocation_count;
    size_t total_allocation_byte_count;
    bool derives_by_horizontal_flip;
    bool contains_graphic_bytes;
    const char *source_evidence;
} DM1_V1_F0461AllocateFlippedWallBitmapsResultPc34;

/*
 * Writes the source-ordered F0461 allocation plan. Returns false without
 * writing when output cannot hold all five records.
 */
bool DM1_V1_F0461_START_AllocateFlippedWallBitmaps_Pc34Compat(
    DM1_V1_F0461FlippedWallBitmapPlanPc34 *plans,
    size_t plan_capacity,
    DM1_V1_F0461AllocateFlippedWallBitmapsResultPc34 *out);

const char *DM1_V1_F0461_START_AllocateFlippedWallBitmaps_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
