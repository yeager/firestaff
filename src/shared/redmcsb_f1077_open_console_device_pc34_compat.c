#include "redmcsb_f1077_open_console_device_pc34_compat.h"

void redmcsb_f1077_open_console_device_pc34_compat(void)
{
}

const char *redmcsb_f1077_open_console_device_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGINIT.C:4 opens the "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
           "AU2G_AU3E Amiga-only source guard. AMIGINIT.C:132-198 "
           "encloses F1077_OpenConsoleDevice for C03_GAME, C07_HINT, "
           "C01_SWOOSH, and MEDIA762_AU1E C06_CEDT. "
           "AMIGINIT.C:154-180 defines it: it clears G3154_ps_Device_Console, "
           "creates G3155_ps_MsgPort with CreatePort(NULL, 0L), creates "
           "G3156_ps_IOStdReq_Console with CreateStdIO, opens console.device "
           "at unit -1L, and stores io_Device in G3154_ps_Device_Console. "
           "CreatePort failure reports F1050_AlertCSBSystemError(0x80FF000A) "
           "for C03_GAME or F9073_DisplayError(0x80F1000A) otherwise; "
           "CreateStdIO failure reports F1050_AlertCSBSystemError(0x80FF000B) "
           "for C03_GAME or F9073_DisplayError(0x80F1000A) otherwise; "
           "OpenDevice failure reports F1050_AlertCSBSystemError(0x80FF000C) "
           "for C03_GAME or F9073_DisplayError(0x80F1000A) otherwise. "
           "AMIGINIT.C:333-361 calls F1077_OpenConsoleDevice from "
           "F1088_OpenAmigaStuff. No PC 3.4 branch or portable host behavior "
           "is supplied by the source.";
}
