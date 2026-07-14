#include "firestaff/dm1/v1/f0461_start_allocate_flipped_wall_bitmaps_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "failed: %s (%s:%d)\\n", #expression, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static void test_source_ordered_derived_allocation_plan(void)
{
    DM1_V1_F0461FlippedWallBitmapPlanPc34 plans[
        DM1_V1_F0461_FLIPPED_WALL_BITMAP_COUNT_PC34];
    DM1_V1_F0461AllocateFlippedWallBitmapsResultPc34 result;

    memset(plans, 0, sizeof(plans));
    CHECK(DM1_V1_F0461_START_AllocateFlippedWallBitmaps_Pc34Compat(
        plans, DM1_V1_F0461_FLIPPED_WALL_BITMAP_COUNT_PC34, &result));
    CHECK(result.allocation_count == 5U);
    CHECK(result.total_allocation_byte_count == 26936U);
    CHECK(result.derives_by_horizontal_flip);
    CHECK(!result.contains_graphic_bytes);
    CHECK(strstr(result.source_evidence, "ATARIST.H:223") != NULL);

    CHECK(plans[0].bitmap == DM1_V1_F0461_FLIPPED_WALL_D3LCR_PC34);
    CHECK(plans[0].row_byte_width == 64U && plans[0].height == 51U);
    CHECK(plans[0].allocation_byte_count == 3264U);
    CHECK(strcmp(plans[0].native_symbol,
                 "G0095_puc_Bitmap_WallD3LCR_Native") == 0);
    CHECK(strcmp(plans[0].flipped_symbol,
                 "G0090_puc_Bitmap_WallD3LCR_Flipped") == 0);

    CHECK(plans[1].row_byte_width == 72U && plans[1].height == 71U);
    CHECK(plans[1].allocation_byte_count == 5112U);
    CHECK(plans[2].row_byte_width == 128U && plans[2].height == 111U);
    CHECK(plans[2].allocation_byte_count == 14208U);
    CHECK(plans[3].bitmap == DM1_V1_F0461_FLIPPED_WALL_D0L_PC34);
    CHECK(strcmp(plans[3].native_symbol,
                 "G0099_puc_Bitmap_WallD0R_Native") == 0);
    CHECK(strcmp(plans[3].flipped_symbol,
                 "G0093_puc_Bitmap_WallD0L_Flipped") == 0);
    CHECK(plans[4].bitmap == DM1_V1_F0461_FLIPPED_WALL_D0R_PC34);
    CHECK(strcmp(plans[4].native_symbol,
                 "G0098_puc_Bitmap_WallD0L_Native") == 0);
    CHECK(strcmp(plans[4].flipped_symbol,
                 "G0094_puc_Bitmap_WallD0R_Flipped") == 0);
}

static void test_rejects_incomplete_plan_storage_without_writing(void)
{
    DM1_V1_F0461FlippedWallBitmapPlanPc34 plans[4];
    DM1_V1_F0461AllocateFlippedWallBitmapsResultPc34 result;

    memset(plans, 0xA5, sizeof(plans));
    CHECK(!DM1_V1_F0461_START_AllocateFlippedWallBitmaps_Pc34Compat(
        plans, 4U, &result));
    CHECK(((const unsigned char *)plans)[0] == 0xA5U);
    CHECK(result.allocation_count == 0U);
    CHECK(!result.contains_graphic_bytes);
}

int main(void)
{
    test_source_ordered_derived_allocation_plan();
    test_rejects_incomplete_plan_storage_without_writing();
    return failures != 0;
}
