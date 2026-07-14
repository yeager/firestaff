#include "redmcsb_f0918_prim_memory_free_pc34_compat.h"

bool redmcsb_f0918_prim_memory_free_pc34_compat(
    void *buffer,
    ReDMCSBF0918PrimReleaseCallbackPc34Compat release_callback,
    void *release_context)
{
    if (buffer == NULL) {
        return true;
    }
    if (release_callback == NULL) {
        return false;
    }

    return release_callback(buffer, release_context);
}

const char *redmcsb_f0918_prim_memory_free_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C:27-38 F0918_PRIM_16_Memory_Free returns "
           "without a platform release for NULL and otherwise calls the "
           "platform release once; PRIM.H:320 and PRIM2STB.C:104-108 map "
           "PC 3.4's slot 16 PrimFree boundary to F0918.";
}
