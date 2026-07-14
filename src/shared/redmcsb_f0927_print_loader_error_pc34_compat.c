#include "redmcsb_f0927_print_loader_error_pc34_compat.h"

#include "redmcsb_f0933_get_hex_string_from_value.h"

int16_t redmcsb_f0927_print_loader_error_pc34_compat(
    int16_t error_code,
    const redmcsb_f0927_print_loader_error_callbacks_pc34_compat *callbacks)
{
    char error_message[9];

    callbacks->console_write(callbacks->context, "Loader error:\a 0x");
    (void)F0933_GetHexStringFromValue((uint32_t)(int32_t)error_code,
                                      error_message);
    callbacks->console_write(callbacks->context, error_message);
    callbacks->console_write(callbacks->context, "\n\r");
    while (!callbacks->key_available(callbacks->context)) {
    }
    callbacks->read_key(callbacks->context);
    return error_code;
}

const char *redmcsb_f0927_print_loader_error_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C:398-416 defines F0927_PrintLoaderError: "
           "Cconws/PRINT emits \"Loader error:\\7 0x\", F0933 formats "
           "the int16_t value as an unsigned long hexadecimal string, "
           "Cconws/PRINT emits the string and \\n\\r, then Cconis/KEYSNS "
           "is polled until Crawcin/GETC consumes one key before the "
           "original value is returned; PRIM.H:285 maps the PC route to "
           "loaderError.";
}
