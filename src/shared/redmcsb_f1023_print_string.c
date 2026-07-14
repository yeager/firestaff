#include "redmcsb_f1023_print_string.h"

void redmcsb_f1023_print_string(redmcsb_f1023_console_print_fn console_print,
                                void *context,
                                const char *string)
{
    console_print(context, string);
}

const char *redmcsb_f1023_print_string_source_evidence(void)
{
    return "ReDMCSB BASE.C:1144-1150 selects the MEDIA607_X30J_X31J "
           "fatal-error route and calls F1023_PrintString for its text; "
           "DEFS.H:9604-9606 declares the string parameter; IO2.C:250-258 "
           "passes that parameter directly to one DOS PRINT call; "
           "CEDT025.C:145-157 confirms the same direct PRINT behavior.";
}
