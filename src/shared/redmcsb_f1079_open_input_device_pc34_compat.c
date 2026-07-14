#include "redmcsb_f1079_open_input_device_pc34_compat.h"

void redmcsb_f1079_open_input_device_pc34_compat(void)
{
}

const char *redmcsb_f1079_open_input_device_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:31-35 declares "
           "G1034_ps_IOStdReq, G3158_ps_MsgPort1, and "
           "G3157_B_InputDeviceAllocated only for C03_GAME. "
           "AMIGINIT.C:200-215 encloses and defines F1079_OpenInputDevice "
           "for C03_GAME: it clears G3157_B_InputDeviceAllocated, creates "
           "G3158_ps_MsgPort1 with CreatePort(NULL, 0L), creates "
           "G1034_ps_IOStdReq with CreateStdIO, opens input.device at unit "
           "0L, then sets G3157_B_InputDeviceAllocated true. CreatePort, "
           "CreateStdIO, and OpenDevice failures respectively report "
           "F1050_AlertCSBSystemError(0x80FF000D), "
           "F1050_AlertCSBSystemError(0x80FF000E), and "
           "F1050_AlertCSBSystemError(0x80FF000F). AMIGINIT.C:333-361 "
           "calls F1079_OpenInputDevice from F1088_OpenAmigaStuff for "
           "C03_GAME; AMIGINIT.C:363-389 calls F1080_CloseInputDevice from "
           "F1089_CloseAmigaStuff for C03_GAME. No PC 3.4 branch or portable "
           "host behavior is supplied by the source.";
}
