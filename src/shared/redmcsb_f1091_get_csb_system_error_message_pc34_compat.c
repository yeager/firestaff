#include "redmcsb_f1091_get_csb_system_error_message_pc34_compat.h"

uint8_t *redmcsb_f1091_get_csb_system_error_message_pc34_compat(void)
{
    static uint8_t message[] =
        "\x00\x94\x10" "Chaos Strikes Back Internal Error: 00000000\x00"
        "\xff\x00\xb8\x1c" "Press Left Mouse Button to Restart\x00";

    return message;
}

const char *redmcsb_f1091_get_csb_system_error_message_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:411-428 defines F1091_GetCSBSystemErrorMessage "
           "only for C03_GAME. Its assembly returns a mutable two-substring "
           "DisplayAlert template: coordinates 0x0094,0x10 and text 'Chaos "
           "Strikes Back Internal Error: 00000000', then continuation 0xff, "
           "coordinates 0x00b8,0x1c, and 'Press Left Mouse Button to Restart'. "
           "F1094 mutates the eight hexadecimal digits. This preserves message "
           "data, not the Amiga alert/restart behavior.";
}
