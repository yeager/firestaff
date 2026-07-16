#include "dm2_v1_skproject_core.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        ++failed; \
        printf("FAIL: %s\n", msg); \
    } else { \
        printf("PASS: %s\n", msg); \
    } \
} while (0)

static void test_between_value(void)
{
    CHECK(dm2_v1_skproject_between_value(10, 9, 20) == 10,
          "BETWEEN_VALUE clamps below min");
    CHECK(dm2_v1_skproject_between_value(10, 10, 20) == 10,
          "BETWEEN_VALUE admits min");
    CHECK(dm2_v1_skproject_between_value(10, 17, 20) == 17,
          "BETWEEN_VALUE admits middle");
    CHECK(dm2_v1_skproject_between_value(10, 20, 20) == 20,
          "BETWEEN_VALUE admits max");
    CHECK(dm2_v1_skproject_between_value(10, 21, 20) == 20,
          "BETWEEN_VALUE clamps above max");
    CHECK(dm2_v1_skproject_dm2_between_value(-1, 127, 200) == 127,
          "DM2_BETWEEN_VALUE wrapper uses v5 argument order");
}

static void test_temp_rect_ring(void)
{
    DM2_V1_SkprojectTempRectRing ring;
    DM2_V1_SkprojectTempRectReceipt receipt[5];

    dm2_v1_skproject_temp_rect_ring_init(&ring);
    memset(receipt, 0, sizeof(receipt));

    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4,
                                           &receipt[0]) == 1,
          "ALLOC_TEMP_RECT accepts first rect");
    CHECK(receipt[0].slot == 0 && receipt[0].next_slot == 1 &&
              receipt[0].rect.x == 1 && receipt[0].rect.y == 2 &&
              receipt[0].rect.w == 3 && receipt[0].rect.h == 4,
          "ALLOC_TEMP_RECT writes slot 0 and advances ring");
    CHECK(dm2_v1_skproject_alloc_temp_origin_rect(&ring, 5, 6,
                                                  &receipt[1]) == 1,
          "ALLOC_TEMP_ORIGIN_RECT delegates to temp rect");
    CHECK(receipt[1].slot == 1 && receipt[1].next_slot == 2 &&
              receipt[1].rect.x == 0 && receipt[1].rect.y == 0 &&
              receipt[1].rect.w == 5 && receipt[1].rect.h == 6,
          "ALLOC_TEMP_ORIGIN_RECT writes origin rectangle");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 7, 8, 9, 10,
                                           &receipt[2]) == 1 &&
              receipt[2].slot == 2 && receipt[2].next_slot == 3,
          "third temp rect uses slot 2");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 11, 12, 13, 14,
                                           &receipt[3]) == 1 &&
              receipt[3].slot == 3 && receipt[3].next_slot == 0,
          "fourth temp rect wraps next index to zero");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 15, 16, 17, 18,
                                           &receipt[4]) == 1 &&
              receipt[4].slot == 0 && receipt[4].next_slot == 1 &&
              ring.rects[0].x == 15 && ring.rects[0].h == 18,
          "fifth temp rect overwrites slot 0 like skproject ringbuffer");
    CHECK(receipt[0].receipt_hash != 0u && receipt[4].receipt_hash != 0u &&
              receipt[0].receipt_hash != receipt[4].receipt_hash,
          "temp rect receipts carry nonzero slot/value hash");
    CHECK(dm2_v1_skproject_alloc_temp_rect(0, 1, 2, 3, 4,
                                           &receipt[0]) == 0,
          "ALLOC_TEMP_RECT rejects missing ring");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4, 0) == 0,
          "ALLOC_TEMP_RECT rejects missing receipt");
}

int main(void)
{
    test_between_value();
    test_temp_rect_ring();
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_TEMP_RECT") != 0,
          "source evidence names ALLOC_TEMP_RECT");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_BETWEEN_VALUE") != 0,
          "source evidence names DM2_BETWEEN_VALUE");

    if (failed) {
        printf("%d failure(s)\n", failed);
        return 1;
    }
    puts("all DM2 skproject core helper checks passed");
    return 0;
}
