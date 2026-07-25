#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat.h"

typedef struct {
    int calls;
    uint8_t snapshot[32][3];
} upload_capture;

static __attribute__((unused)) void capture_upload(void *context, const uint8_t full_palette[32][3])
{
    upload_capture *capture = context;

    capture->calls++;
    memcpy(capture->snapshot, full_palette, sizeof(capture->snapshot));
}

int main(void)
{
    static const redmcsb_f0694_palette_entry_pc34_compat entries[] = {
        { 0, 1, 2, 3 },
        { 31, 61, 62, 63 },
        { 32, 7, 8, 9 },
        { -1, 0, 0, 0 }
    };
    static const redmcsb_f0694_palette_entry_pc34_compat unterminated[] = {
        { 1, 9, 8, 7 }
    };
    const redmcsb_f0694_palette_definition_pc34_compat palettes[] = {
        { entries, sizeof(entries) / sizeof(entries[0]) },
        { unterminated, sizeof(unterminated) / sizeof(unterminated[0]) }
    };
    (void)palettes;
    uint8_t palette[32][3];
    upload_capture capture;

    memset(palette, 0x55, sizeof(palette));
    memset(&capture, 0, sizeof(capture));

    assert(redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
        palettes, 2U, 0, palette, 0, capture_upload, &capture));
    assert(palette[0][0] == 1U && palette[0][1] == 2U && palette[0][2] == 3U);
    assert(palette[31][0] == 61U && palette[31][1] == 62U &&
           palette[31][2] == 63U);
    assert(palette[30][0] == 0x55U);
    assert(capture.calls == 0);

    assert(redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
        palettes, 2U, 0, palette, 1, capture_upload, &capture));
    assert(capture.calls == 1);
    assert(memcmp(capture.snapshot, palette, sizeof(palette)) == 0);

    assert(!redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
        palettes, 2U, 1, palette, 1, capture_upload, &capture));
    assert(!redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
        palettes, 2U, -1, palette, 1, capture_upload, &capture));
    assert(!redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
        palettes, 2U, 2, palette, 1, capture_upload, &capture));

    puts("ok: ReDMCSB F0694 C25 VGA palette update");
    return 0;
}
