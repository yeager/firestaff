#include "redmcsb_f0813_play_midi_music_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct RecordedCall {
    int calls;
    int command;
    const char *midi_data;
    void *context;
} RecordedCall;

static void record_driver_call(int command, const char *midi_data, void *context)
{
    RecordedCall *record = context;

    record->calls += 1;
    record->command = command;
    record->midi_data = midi_data;
    record->context = context;
}

static int expect(int condition, const char *message)
{
    if (!condition) {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    static const char midi_data[] = "ENTER.SNG";
    RecordedCall record = {0, -1, NULL, NULL};
    ReDMCSB_F0813_MidiDriver driver = {record_driver_call, &record};

    ReDMCSB_F0813_PlayMIDIMusic_PC34_Compat(&driver, midi_data);
    if (!expect(record.calls == 1, "F0813 dispatches exactly once") ||
        !expect(record.command == 0, "F0813 uses IODRV_27 command 0") ||
        !expect(record.midi_data == midi_data, "F0813 preserves MIDI data pointer") ||
        !expect(strcmp(record.midi_data, "ENTER.SNG") == 0,
                "F0813 does not mutate MIDI data")) {
        return 1;
    }

    ReDMCSB_F0813_PlayMIDIMusic_PC34_Compat(NULL, midi_data);
    if (!expect(record.calls == 1, "missing host driver cannot synthesize playback")) {
        return 1;
    }

    driver.iodrv_27 = NULL;
    ReDMCSB_F0813_PlayMIDIMusic_PC34_Compat(&driver, midi_data);
    if (!expect(record.calls == 1, "missing IODRV_27 is a no-op")) {
        return 1;
    }

    return 0;
}
