#include "redmcsb_f0619_slot_box_border.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int assertions;

static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL:%d: %s\n", line, expression);
    }
}

#define CHECK(condition) check((condition), #condition, __LINE__)

static int resolve_zone(void *context, int16_t zone, int16_t initial_width,
                       int16_t initial_height, RedmcsbF0619Rect *out)
{
    int *calls = (int *)context;
    ++*calls;
    if (zone != 545 || initial_width != 16 || initial_height != 16 || !out)
        return 0;
    out->x = 30;
    out->y = 40;
    out->width = 12;
    out->height = 10;
    return 1;
}

static int reject_zone(void *context, int16_t zone, int16_t initial_width,
                       int16_t initial_height, RedmcsbF0619Rect *out)
{
    (void)context;
    (void)zone;
    (void)initial_width;
    (void)initial_height;
    (void)out;
    return 0;
}

int main(void)
{
    RedmcsbF0619Receipt receipt;
    int calls = 0;

    memset(&receipt, 0xa5, sizeof(receipt));
    CHECK(redmcsb_f0619_get_slot_box_border(
        545, 16, 16, 0, REDMCSB_F0619_NO_MARGIN,
        resolve_zone, &calls, &receipt));
    CHECK(calls == 1 && receipt.accepted);
    CHECK(receipt.rect.x == 30 && receipt.rect.y == 40);
    CHECK(receipt.rect.width == 12 && receipt.rect.height == 10);

    CHECK(redmcsb_f0619_get_slot_box_border(
        545, 16, 16, 2, REDMCSB_F0619_MEDIA508_MARGIN,
        resolve_zone, &calls, &receipt));
    CHECK(receipt.rect.x == 28 && receipt.rect.y == 38);
    CHECK(receipt.rect.width == 16 && receipt.rect.height == 14);

    CHECK(redmcsb_f0619_get_slot_box_border(
        545, 16, 16, 2, REDMCSB_F0619_MEDIA720_MARGIN,
        resolve_zone, &calls, &receipt));
    CHECK(receipt.rect.x == 28 && receipt.rect.y == 38);
    CHECK(receipt.rect.width == 16 && receipt.rect.height == 14);

    CHECK(!redmcsb_f0619_get_slot_box_border(
        545, 16, 16, 0, REDMCSB_F0619_NO_MARGIN,
        reject_zone, NULL, &receipt));
    CHECK(!receipt.accepted);
    CHECK(!redmcsb_f0619_get_slot_box_border(
        545, 16, 16, 1, REDMCSB_F0619_NO_MARGIN,
        resolve_zone, &calls, &receipt));
    CHECK(!redmcsb_f0619_get_slot_box_border(
        545, 16, 16, 32767, REDMCSB_F0619_MEDIA720_MARGIN,
        resolve_zone, &calls, &receipt));

    printf("F0619 assertions=%d failures=%d\n", assertions, failures);
    return failures ? 1 : 0;
}
