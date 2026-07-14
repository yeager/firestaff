#include "redmcsb_f1035_unreferenced.h"

void redmcsb_f1035_unreferenced(redmcsb_f1035_f1034_fn f1034,
                                 void *context,
                                 int character)
{
    char string[2];

    string[0] = (char)character;
    string[1] = '\0';
    f1034(context, string);
}

const char *redmcsb_f1035_unreferenced_source_evidence(void)
{
    return "ReDMCSB IO2.C:219-225 defines F1034_, which writes only when "
           "FindTask(0L) equals G3165_ps_CurrentTask; IO2.C:227-236 "
           "defines MEDIA746 F1035_Unreferenced, stores "
           "((char)P2769_i_ << 8) & 0xFF00 in a local int16_t to create "
           "a one-character string, then calls F1034_ once. The explicit "
           "two-byte C11 buffer preserves the original big-endian string "
           "layout without reproducing its target-specific integer storage.";
}
