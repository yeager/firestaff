#include "redmcsb_f0698_invert_box_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
    unsigned int calls;
} RedmcsbF0698CapturePc34Compat;

static void capture_invert(void *context,
                           int16_t left,
                           int16_t right,
                           int16_t top,
                           int16_t bottom)
{
    RedmcsbF0698CapturePc34Compat *capture = context;

    capture->left = left;
    capture->right = right;
    capture->top = top;
    capture->bottom = bottom;
    ++capture->calls;
}

int main(void)
{
    const RedmcsbF0698ZonePc34Compat zone = { -3, 319, 4, 199 };
    (void)zone;
    RedmcsbF0698CapturePc34Compat capture = { 0 };
    const RedmcsbF0698VideoDriverPc34Compat driver = {
        capture_invert,
        &capture
    };
    (void)driver;

    assert(redmcsb_f0698_invert_box_pc34_compat(&driver, &zone));
    assert(capture.calls == 1U);
    assert(capture.left == -3 && capture.right == 319);
    assert(capture.top == 4 && capture.bottom == 199);
    assert(!redmcsb_f0698_invert_box_pc34_compat(NULL, &zone));
    assert(!redmcsb_f0698_invert_box_pc34_compat(&driver, NULL));
    assert(strstr(redmcsb_f0698_invert_box_source_evidence_pc34(),
                  "IMAGE.C:231-294") != NULL);
    return 0;
}
