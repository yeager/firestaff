#include "redmcsb_f0709_start_sound_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    redmcsb_f0709_sound_descriptor_pc34_compat descriptor;
    int lookup_calls;
    int play_calls;
    int16_t seen_index;
    void *seen_buffer;
    int16_t seen_volume;
    int16_t seen_period;
} redmcsb_f0709_capture_pc34_compat;

static const redmcsb_f0709_sound_descriptor_pc34_compat *lookup(
    void *context, int16_t sound_index)
{
    redmcsb_f0709_capture_pc34_compat *capture = context;

    capture->lookup_calls++;
    capture->seen_index = sound_index;
    return &capture->descriptor;
}

static void play(void *context, void *buffer, int16_t volume, int16_t period)
{
    redmcsb_f0709_capture_pc34_compat *capture = context;

    capture->play_calls++;
    capture->seen_buffer = buffer;
    capture->seen_volume = volume;
    capture->seen_period = period;
}

int main(void)
{
    redmcsb_f0709_capture_pc34_compat capture = {
        { 42, (void *)0x1234 }, 0, 0, 0, NULL, 0, 0
    };
    redmcsb_f0709_sound_route_pc34_compat route = { lookup, play, &capture };
    (void)route;

    assert(redmcsb_f0709_start_sound_pc34_compat(&route, 3, 17));
    assert(capture.lookup_calls == 1);
    assert(capture.seen_index == 3);
    assert(capture.play_calls == 1);
    assert(capture.seen_buffer == (void *)0x1234);
    assert(capture.seen_volume == 17);
    assert(capture.seen_period == 6000);

    capture.descriptor.graphic_index = -1;
    assert(!redmcsb_f0709_start_sound_pc34_compat(&route, 3, -11));
    assert(capture.play_calls == 1);

    assert(redmcsb_f0709_start_sound_pc34_compat(&route, -1, -11));
    assert(capture.play_calls == 2);
    assert(capture.seen_volume == -11);
    assert(!redmcsb_f0709_start_sound_pc34_compat(NULL, 0, 0));
    assert(strstr(redmcsb_f0709_start_sound_source_evidence_pc34(),
                  "IO.C:3832-3843") != NULL);
    return 0;
}
