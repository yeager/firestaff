#include "redmcsb_f1090_get_csb_internal_error_message_pc34_compat.h"

uint8_t *redmcsb_f1090_get_csb_internal_error_message_pc34_compat(void)
{
    static uint8_t message[] =
        "\x00\xb8\x10" "Chaos Strikes Back System Error 00\x00"
        "\xff\x00\xb8\x1c" "Press Left Mouse Button to Restart\x00";

    return message;
}

const char *redmcsb_f1090_get_csb_internal_error_message_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:392-409 defines F1090_GetCSBInternalErrorMessage "
           "only for C03_GAME. Its assembly returns a mutable two-substring "
           "DisplayAlert template: coordinates 0x00b8,0x10 and text 'Chaos "
           "Strikes Back System Error 00', then continuation 0xff, coordinates "
           "0x00b8,0x1c, and 'Press Left Mouse Button to Restart'. F1093 "
           "mutates the final two decimal digits. This preserves message data, "
           "not the Amiga alert/restart behavior.";
}
