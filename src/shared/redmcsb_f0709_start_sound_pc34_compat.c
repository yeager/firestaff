#include "redmcsb_f0709_start_sound_pc34_compat.h"

void redmcsb_f0709_start_sound_pc34_compat(
    int16_t sound_index,
    int16_t sound_volume,
    redmcsb_f0709_start_sound_pc34_backend backend,
    void *backend_context)
{
    if (backend != NULL) {
        backend(backend_context, sound_index, sound_volume);
    }
}
