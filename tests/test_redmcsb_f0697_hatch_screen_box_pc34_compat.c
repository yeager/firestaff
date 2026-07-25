#include "redmcsb_f0697_hatch_screen_box_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
    unsigned int calls;
} RedmcsbF0697CapturePc34Compat;

static void capture_hatch(void *context,
                          int16_t left,
                          int16_t right,
                          int16_t top,
                          int16_t bottom)
{
    RedmcsbF0697CapturePc34Compat *capture = context;

    capture->left = left;
    capture->right = right;
    capture->top = top;
    capture->bottom = bottom;
    ++capture->calls;
}

int main(void)
{
    const RedmcsbF0697ZonePc34Compat zone = { 12, 267, -4, 199 };
    (void)zone;
    RedmcsbF0697CapturePc34Compat capture = { 0 };
    const RedmcsbF0697VideoDriverPc34Compat driver = {
        capture_hatch,
        &capture
    };
    (void)driver;

    assert(redmcsb_f0697_hatch_screen_box_pc34_compat(&driver, &zone,
                                                        0x001fU));
    assert(capture.calls == 1U);
    assert(capture.left == 12 && capture.right == 267);
    assert(capture.top == -4 && capture.bottom == 199);
    assert(!redmcsb_f0697_hatch_screen_box_pc34_compat(NULL, &zone, 0U));
    assert(!redmcsb_f0697_hatch_screen_box_pc34_compat(&driver, NULL, 0U));
    assert(strstr(redmcsb_f0697_hatch_screen_box_source_evidence_pc34(),
                  "IMAGE.C:160-178") != NULL);
    return 0;
}
