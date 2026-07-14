#include "redmcsb_f1080_close_input_device_pc34_compat.h"

void redmcsb_f1080_close_input_device_pc34_compat(void)
{
}

const char *redmcsb_f1080_close_input_device_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:200-233 "
           "encloses F1079_OpenInputDevice and F1080_CloseInputDevice "
           "for EXETYPE == C03_GAME. AMIGINIT.C:217-232 defines "
           "F1080_CloseInputDevice: when G3157_B_InputDeviceAllocated "
           "is true, it calls CloseDevice(G1034_ps_IOStdReq) and clears "
           "G3157_B_InputDeviceAllocated; when G1034_ps_IOStdReq is "
           "non-null, it calls DeleteStdIO and clears it; when "
           "G3158_ps_MsgPort1 is non-null, it calls DeletePort and clears "
           "it. AMIGINIT.C:363-373 calls F1080_CloseInputDevice from "
           "F1089_CloseAmigaStuff. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
