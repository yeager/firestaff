#include "redmcsb_f1022_print_character.h"

void redmcsb_f1022_print_character(redmcsb_f1022_console_print_fn console_print,
                                   void *context,
                                   char character)
{
    static char string[2];

    string[0] = character;
    console_print(context, string);
}

const char *redmcsb_f1022_print_character_source_evidence(void)
{
    return "ReDMCSB BASE.C:1144-1147 selects the MEDIA607_X30J_X31J "
           "fatal-error route and calls F1022_PrintCharacter for each error "
           "digit; DEFS.H:9601-9603 declares the character parameter; "
           "IO2.C:239-248 writes G3099_ac_String[0] then issues one DOS "
           "PRINT call; CEDT025.C:127-143 confirms the same persistent "
           "two-byte string behavior.";
}
