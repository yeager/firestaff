#ifndef FIRESTAFF_SOUND_EVENT_SND3_MAP_V1_H
#define FIRESTAFF_SOUND_EVENT_SND3_MAP_V1_H

/*
 * sound_event_snd3_map_v1 — Pass 52 bounded V1 audio mapping.
 *
 * Maps DM PC v3.4 English sound event indices (the F0064_SOUND_RequestPlay
 * sound-index namespace from ReDMCSB/DEFS.H + DATA.C for I34E/I34M) to the
 * GRAPHICS.DAT SND3 item indices decoded by graphics_dat_snd3_loader_v1.
 *
 * This module deliberately does not perform runtime playback.  It is a pure
 * table/API layer so SDL wiring can be evidenced separately.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define V1_DM_SOUND_EVENT_COUNT 35u
#define V1_DM_SOUND_EVENT_NONE  (-1)

typedef struct {
    int          soundIndex;       /* DM PC v3.4 sound event index, 0..34 */
    unsigned int snd3ItemIndex;    /* GRAPHICS.DAT item index, e.g. 671 */
    unsigned int snd3Ordinal;      /* Greatstone Sound NN, 0..32 */
    const char*  macroName;        /* ReDMCSB DEFS.H event macro */
    const char*  role;             /* Short source/use description */
} V1_SoundEventSnd3MapEntry;

const V1_SoundEventSnd3MapEntry* V1_SoundEventSnd3_Map(unsigned int* outCount);
const V1_SoundEventSnd3MapEntry* V1_SoundEventSnd3_Find(int soundIndex);
int V1_SoundEventSnd3_ItemForSoundIndex(int soundIndex);
const char* V1_SoundEventSnd3_LabelForSoundIndex(int soundIndex);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SOUND_EVENT_SND3_MAP_V1_H */
