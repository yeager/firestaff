#include "redmcsb_f1078_close_console_device_pc34_compat.h"

void redmcsb_f1078_close_console_device_pc34_compat(void)
{
}

const char *redmcsb_f1078_close_console_device_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:182-198 "
           "defines F1078_CloseConsoleDevice: when "
           "G3154_ps_Device_Console is non-null, it calls "
           "CloseDevice(G3156_ps_IOStdReq_Console) and clears "
           "G3154_ps_Device_Console; when G3156_ps_IOStdReq_Console is "
           "non-null, it calls DeleteStdIO(G3156_ps_IOStdReq_Console) and "
           "clears it; when G3155_ps_MsgPort is non-null, it calls "
           "DeletePort(G3155_ps_MsgPort) and clears it. AMIGINIT.C:363-379 "
           "calls F1078_CloseConsoleDevice from F1089_CloseAmigaStuff after "
           "F1082_CloseNIL when EXETYPE is C03_GAME, C07_HINT, C01_SWOOSH, "
           "or defined(MEDIA762_AU1E) with C06_CEDT. No PC 3.4 branch or "
           "portable host behavior is supplied by the source.";
}
