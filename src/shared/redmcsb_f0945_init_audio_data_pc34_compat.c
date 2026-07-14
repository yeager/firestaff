#include "redmcsb_f0945_init_audio_data_pc34_compat.h"

bool redmcsb_f0945_init_audio_data_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0945_init_audio_data_source_evidence_pc34(void)
{
    return "ReDMCSB SOUND.C:1 and SOUND.C:115-136 select the audio "
           "request state only for MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M; "
           "SOUND.C:425-494 defines F0945_InitAudioData with AllocMem, "
           "OpenDevice(\"audio.device\"), CreatePort, DoIO, and four Amiga "
           "channel assignments. EXEC.C:433-501 contains the older Amiga "
           "variant. Neither source route includes PC 3.4 I34E/I34M, and no "
           "portable host adapter is supplied.";
}
