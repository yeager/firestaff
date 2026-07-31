/* DM1 PC34 FTL swoosh audio: consume the exact SWSH.C Dosound program. */
#include "audio_sdl_m11.h"
#include "swsh_frontend_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int expect(int condition, const char* message) {
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void) {
    M11_AudioState state;
    const unsigned char* program;
    unsigned char altered[64];
    char expectedSongPath[1024];
    const char* home;
    FILE* songFile;
    unsigned int bytes = 0u;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    program = SWSH_Compat_GetPc34DosoundProgram(&bytes);
    ok &= expect(program != NULL && bytes == 56u,
                 "source program has its full fixed byte count");
    ok &= expect(SWSH_Compat_ValidatePc34DosoundProgram(program, bytes),
                 "source program validates against immutable ReDMCSB bytes");
    ok &= expect(SWSH_Compat_GetPc34DosoundProgramFingerprint() != 0u,
                 "source program has a nonzero receipt fingerprint");
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    home = getenv("HOME");
    songFile = NULL;
    if (home && home[0] && !getenv("FIRESTAFF_SONG_DAT") &&
        snprintf(expectedSongPath, sizeof(expectedSongPath),
                 "%s/.firestaff/data/dm1/SONG.DAT", home) > 0) {
        songFile = fopen(expectedSongPath, "rb");
    }
    if (songFile) {
        fclose(songFile);
        ok &= expect(M11_Audio_OriginalSongAvailable(&state) &&
                     strcmp(state.originalSongDatPath, expectedSongPath) == 0,
                     "DM1-local SONG.DAT is consumed before legacy locations");
    }
    ok &= expect(M11_Audio_PlayDm1SwshDosoundProgram(&state, program,
                                                      (int)bytes, 20u),
                 "exact source program produces the PSG stream");
    ok &= expect(state.dm1SwshProgramAccepted &&
                     state.dm1SwshRegisterWriteCount == 17 &&
                     state.dm1SwshWaitVblankCount == 20,
                 "receipt retains all source register writes and VBlank waits");
    ok &= expect(state.dm1SwshProgram.sampleCount == 8820,
                 "PSG stream duration follows 20 original PAL VBlanks");
    ok &= expect(state.dm1SwshProgram.sampleCount > 0 &&
                     state.dm1SwshProgram.samples != NULL,
                 "source program yields a concrete audio stream without marker fallback");

    memcpy(altered, program, bytes);
    altered[1] ^= 1u;
    ok &= expect(!M11_Audio_PlayDm1SwshDosoundProgram(&state, altered,
                                                       (int)bytes, 20u),
                 "mutated source program fails closed");
    ok &= expect(!state.dm1SwshProgramAccepted &&
                     state.dm1SwshProgram.sampleCount == 0,
                 "failed program cannot retain a stale startup sound");
    ok &= expect(!M11_Audio_PlayDm1SwshDosoundProgram(&state, program,
                                                       (int)bytes, 19u),
                 "non-PC34 VBlank cadence is rejected");

    M11_Audio_Shutdown(&state);
    return ok ? 0 : 1;
}
