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

static int path_has_zip_suffix(const char* path) {
    const char* suffix = path ? strrchr(path, '.') : NULL;
    if (!suffix) return 0;
    return suffix[0] == '.' &&
           (suffix[1] == 'z' || suffix[1] == 'Z') &&
           (suffix[2] == 'i' || suffix[2] == 'I') &&
           (suffix[3] == 'p' || suffix[3] == 'P') && suffix[4] == '\0';
}

int main(void) {
    M11_AudioState state;
    const unsigned char* program;
    unsigned char altered[64];
    char expectedSongPath[1024];
    char expectedGraphicsPath[1024];
    const char* dataRoot;
    unsigned int bytes = 0u;
    int ok = 1;

    dataRoot = getenv("FIRESTAFF_DM1_DATA_DIR");
    if (!dataRoot || !dataRoot[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    if (path_has_zip_suffix(dataRoot)) {
        if (snprintf(expectedSongPath, sizeof(expectedSongPath),
                     "%s::DATA/SONG.DAT", dataRoot) <= 0 ||
            snprintf(expectedGraphicsPath, sizeof(expectedGraphicsPath),
                     "%s::DATA/GRAPHICS.DAT", dataRoot) <= 0) {
            fputs("FAIL: unable to form direct-archive DM1 audio paths\n", stderr);
            return 1;
        }
    } else if (snprintf(expectedSongPath, sizeof(expectedSongPath), "%s/SONG.DAT",
                        dataRoot) <= 0 ||
               snprintf(expectedGraphicsPath, sizeof(expectedGraphicsPath),
                        "%s/GRAPHICS.DAT", dataRoot) <= 0) {
        fputs("FAIL: unable to select configured DM1 SONG.DAT\n", stderr);
        return 1;
    }

    memset(&state, 0, sizeof(state));
    program = SWSH_Compat_GetPc34DosoundProgram(&bytes);
    ok &= expect(program != NULL && bytes == 56u,
                 "source program has its full fixed byte count");
    ok &= expect(SWSH_Compat_ValidatePc34DosoundProgram(program, bytes),
                 "source program validates against immutable ReDMCSB bytes");
    ok &= expect(SWSH_Compat_GetPc34DosoundProgramFingerprint() != 0u,
                 "source program has a nonzero receipt fingerprint");
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    ok &= expect(M11_Audio_OriginalSnd3Available(&state) &&
                     M11_Audio_BindOriginalSnd3Path(&state, expectedGraphicsPath),
                 "configured DM1 SND3 effects are consumed directly from selected media");
    /* DM1 source effects are SND3-owned. A missing record must not revive
     * the legacy procedural door/combat/spell marker path. */
    state.originalSounds[0].sampleCount = 0;
    state.lastMarker = M11_AUDIO_MARKER_DOOR;
    {
        int markerCount = state.playedMarkerCount;
        ok &= expect(!M11_Audio_EmitSourceSoundIndex(&state, 0),
                     "missing source SND3 event is silent");
        ok &= expect(state.lastSoundIndex == 0 &&
                         state.lastMarker == M11_AUDIO_MARKER_DOOR &&
                         state.playedMarkerCount == markerCount,
                     "missing source event cannot emit or replace marker history");
        ok &= expect(!M11_Audio_EmitSourceSoundIndex(&state, -1) &&
                         state.lastSoundIndex == -1 &&
                         state.lastMarker == M11_AUDIO_MARKER_NONE,
                     "invalid source event is silent without stale identity");
    }
    ok &= expect(M11_Audio_OriginalSongAvailable(&state) &&
                 strcmp(state.originalSongDatPath, expectedSongPath) == 0,
                 "configured DM1 SONG.DAT is consumed directly from selected media");
    ok &= expect(M11_Audio_BindOriginalSongPath(&state, expectedSongPath) &&
                 M11_Audio_OriginalSongAvailable(&state) &&
                 strcmp(state.originalSongDatPath, expectedSongPath) == 0,
                 "startup receipt can bind the selected source SONG.DAT");
    ok &= expect(!M11_Audio_BindOriginalSongPath(&state,
                                                 "/nonexistent/SONG.DAT") &&
                 !M11_Audio_OriginalSongAvailable(&state) &&
                 state.originalSongDatPath[0] == '\0',
                 "missing selected SONG.DAT clears an earlier default source");
    ok &= expect(M11_Audio_BindOriginalSongPath(&state, expectedSongPath),
                 "selected source SONG.DAT can be rebound after a failed receipt");
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
