#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "redmcsb_f0709_start_sound_pc34_compat.h"

typedef struct {
    int calls;
    int16_t sound_index;
    int16_t sound_volume;
} start_capture;

static void capture_start(
    void *context,
    int16_t sound_index,
    int16_t sound_volume)
{
    start_capture *capture = context;

    capture->calls++;
    capture->sound_index = sound_index;
    capture->sound_volume = sound_volume;
}

int main(void)
{
    start_capture capture = { 0, 0, 0 };

    redmcsb_f0709_start_sound_pc34_compat(
        34, 3, capture_start, &capture);
    assert(capture.calls == 1);
    assert(capture.sound_index == 34);
    assert(capture.sound_volume == 3);

    redmcsb_f0709_start_sound_pc34_compat(
        INT16_C(-1), INT16_C(-32768), capture_start, &capture);
    assert(capture.calls == 2);
    assert(capture.sound_index == -1);
    assert(capture.sound_volume == INT16_MIN);

    redmcsb_f0709_start_sound_pc34_compat(1, 2, NULL, &capture);
    assert(capture.calls == 2);

    puts("ok: ReDMCSB F0709 PC 3.4 start-sound dispatch");
    return 0;
}
