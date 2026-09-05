/* Real-media gate for the complete I34E sound-event bank.  The selected ZIP
 * stays packed: asset_read_path_alloc resolves the virtual archive member. */
#include "dm1_v1_sound_pc34_compat.h"
#include "graphics_dat_snd3_loader_v1.h"
#include "sound_event_snd3_map_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char* archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    char path[4096];
    char error[256];
    V1_GraphicsSnd3Manifest manifest;
    DM1_SoundSystem sound;
    int index;

    if (!archive || !archive[0]) {
        puts("SKIP: no DM1 DOS PC 3.4 archive");
        return 0;
    }
    if (snprintf(path, sizeof(path), "%s::DATA/GRAPHICS.DAT", archive) >=
        (int)sizeof(path)) {
        fputs("FAIL: archive path is too long\n", stderr);
        return 1;
    }
    if (!V1_GraphicsSnd3_ParseManifest(path, &manifest, error,
                                       sizeof(error))) {
        fprintf(stderr, "FAIL: real PC34 SND3 manifest: %s\n", error);
        return 1;
    }
    DM1_Sound_Init(&sound);
    for (index = 0; index < DM1_SND_COUNT; ++index) {
        const V1_SoundEventSnd3MapEntry* map =
            V1_SoundEventSnd3_Find(index);
        const DM1_SoundData* row =
            DM1_Sound_GetSoundData(&sound, (int16_t)index);
        V1_GraphicsSnd3Buffer decoded;
        if (!map || !row || row->graphicIndex != (int16_t)map->snd3ItemIndex) {
            fprintf(stderr, "FAIL: I34E event %d does not bind its DATA.C item\n",
                    index);
            return 1;
        }
        memset(&decoded, 0, sizeof(decoded));
        if (!V1_GraphicsSnd3_DecodeItem(path, &manifest,
                                        map->snd3ItemIndex, &decoded,
                                        error, sizeof(error)) ||
            !decoded.samples || decoded.decodedSampleCount == 0u ||
            decoded.sampleRateHz != V1_GRAPHICS_DAT_SND3_SAMPLE_RATE_HZ) {
            fprintf(stderr, "FAIL: real PC34 event %d / item %u: %s\n",
                    index, map->snd3ItemIndex, error);
            V1_GraphicsSnd3_FreeBuffer(&decoded);
            return 1;
        }
        V1_GraphicsSnd3_FreeBuffer(&decoded);
    }
    puts("PASS: all 35 I34E events bind and decode authentic packed PC34 SND3");
    return 0;
}
