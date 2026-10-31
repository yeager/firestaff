#include "redmcsb_f1131_invert_box_amiga_compat.h"

#include <assert.h>
#include <string.h>

enum {
    EVENT_DRAW_MODE = 1,
    EVENT_PEN,
    EVENT_MASK,
    EVENT_WAIT_SCAN_LINE,
    EVENT_RECT_FILL,
    EVENT_WAIT_BLIT
};

typedef struct {
    int events[8];
    int32_t args[8][4];
    int count;
} RasterCapture;

static void record_event(RasterCapture *capture, int event,
                         int32_t a, int32_t b, int32_t c, int32_t d)
{
    assert(capture->count < 8);
    capture->events[capture->count] = event;
    capture->args[capture->count][0] = a;
    capture->args[capture->count][1] = b;
    capture->args[capture->count][2] = c;
    capture->args[capture->count][3] = d;
    capture->count++;
}

static void set_draw_mode(void *context, int32_t mode)
{
    record_event((RasterCapture *)context, EVENT_DRAW_MODE, mode, 0, 0, 0);
}

static void set_pen(void *context, int32_t pen)
{
    record_event((RasterCapture *)context, EVENT_PEN, pen, 0, 0, 0);
}

static void set_mask(void *context, uint8_t mask)
{
    record_event((RasterCapture *)context, EVENT_MASK, mask, 0, 0, 0);
}

static void wait_for_scan_line(void *context, int32_t line)
{
    record_event((RasterCapture *)context, EVENT_WAIT_SCAN_LINE, line, 0, 0, 0);
}

static void rect_fill(void *context, int32_t left, int32_t top,
                      int32_t right, int32_t bottom)
{
    record_event((RasterCapture *)context, EVENT_RECT_FILL,
                 left, top, right, bottom);
}

static void wait_blit(void *context)
{
    record_event((RasterCapture *)context, EVENT_WAIT_BLIT, 0, 0, 0, 0);
}

int main(void)
{
    RasterCapture capture = { { 0 }, { { 0 } }, 0 };
    const RedmcsbF1131AmigaRasterCompat raster = {
        &capture, set_draw_mode, set_pen, set_mask, wait_for_scan_line,
        rect_fill, wait_blit
    };
    const RedmcsbF1131AmigaBoxCompat box = { 10, 20, 30, 40 };
    RedmcsbF1131AmigaRasterCompat incomplete = raster;

    assert(redmcsb_f1131_invert_box_amiga_compat(&raster, &box));
    assert(capture.count == 6);
    assert(capture.events[0] == EVENT_DRAW_MODE &&
           capture.args[0][0] == 2);
    assert(capture.events[1] == EVENT_PEN && capture.args[1][0] == -1);
    assert(capture.events[2] == EVENT_MASK && capture.args[2][0] == 0x0f);
    assert(capture.events[3] == EVENT_WAIT_SCAN_LINE &&
           capture.args[3][0] == 59);
    assert(capture.events[4] == EVENT_RECT_FILL &&
           capture.args[4][0] == 10 && capture.args[4][1] == 20 &&
           capture.args[4][2] == 39 && capture.args[4][3] == 59);
    assert(capture.events[5] == EVENT_WAIT_BLIT);
    assert(strstr(redmcsb_f1131_invert_box_amiga_source_evidence(),
                  "FILLBOX.C:736-751") != NULL);

    capture.count = 0;
    assert(redmcsb_f1131_invert_box_amiga_compat(NULL, NULL));
    assert(capture.count == 0);
    assert(!redmcsb_f1131_invert_box_amiga_compat(NULL, &box));
    assert(capture.count == 0);
    incomplete.wait_blit = NULL;
    assert(!redmcsb_f1131_invert_box_amiga_compat(&incomplete, &box));
    assert(capture.count == 0);
    return 0;
}
