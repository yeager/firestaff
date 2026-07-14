#include <stdint.h>
#include <string.h>

#include "redmcsb_f0814_transition_midi_music_pc34_compat.h"

struct call_log {
    int calls;
    int16_t command;
    void *context;
};

static void record_midi_music_control(int16_t command, void *context)
{
    struct call_log *log = context;

    log->calls += 1;
    log->command = command;
    log->context = context;
}

int main(void)
{
    struct call_log log;

    memset(&log, 0, sizeof(log));
    redmcsb_f0814_transition_midi_music_pc34_compat(
        record_midi_music_control,
        &log);

    if (log.calls != 1 || log.command != 2 || log.context != &log) {
        return 1;
    }
    if (strcmp(
            redmcsb_f0814_transition_midi_music_source_evidence_pc34(),
            "ReDMCSB_WIP20210206/Toolchains/Common/Source/IO.C:4181-4186") != 0) {
        return 1;
    }
    return 0;
}
