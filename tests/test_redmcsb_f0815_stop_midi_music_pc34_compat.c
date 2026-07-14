#include <string.h>

#include "redmcsb_f0815_stop_midi_music_pc34_compat.h"

struct midi_log {
    int calls;
    int command;
    void *context;
};

static void record_midi_command(int command, void *context)
{
    struct midi_log *log = (struct midi_log *)context;

    ++log->calls;
    log->command = command;
    log->context = context;
}

int main(void)
{
    struct midi_log log = {0, 0, 0};

    redmcsb_f0815_stop_midi_music_pc34_compat(record_midi_command, &log);

    if (log.calls != 1 || log.command != 1 || log.context != &log) {
        return 1;
    }
    if (strcmp(redmcsb_f0815_stop_midi_music_source_evidence_pc34(),
               "ReDMCSB IO.C:4188-4192; IODRV_27 command 1") != 0) {
        return 1;
    }

    return 0;
}
