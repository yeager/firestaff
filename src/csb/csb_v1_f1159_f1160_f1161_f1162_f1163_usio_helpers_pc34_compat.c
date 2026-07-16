#include "csb_v1_f1159_f1160_f1161_f1162_f1163_usio_helpers_pc34_compat.h"

void csb_v1_f1159_empty_pc34_compat(void)
{
}

void F1159_Empty(void)
{
    csb_v1_f1159_empty_pc34_compat();
}

const char *csb_v1_f1159_empty_source_evidence_pc34(void)
{
    return "ReDMCSB USIOMAIN.C:12 F1159_Empty; source-named USIO empty "
           "library boundary with no portable PC34 side effect";
}

void csb_v1_f1160_usio_04_empty_pc34_compat(void)
{
}

void F1160_USIO_04_Empty(void)
{
    csb_v1_f1160_usio_04_empty_pc34_compat();
}

const char *csb_v1_f1160_usio_04_empty_source_evidence_pc34(void)
{
    return "ReDMCSB USIO1.C:29 F1160_USIO_04_Empty; source-named USIO "
           "vector boundary with no portable PC34 side effect";
}

void csb_v1_f1161_usio_05_empty_pc34_compat(void)
{
}

void F1161_USIO_05_Empty(void)
{
    csb_v1_f1161_usio_05_empty_pc34_compat();
}

const char *csb_v1_f1161_usio_05_empty_source_evidence_pc34(void)
{
    return "ReDMCSB USIO1.C:36 F1161_USIO_05_Empty; source-named USIO "
           "vector boundary with no portable PC34 side effect";
}

void csb_v1_f1162_usio_06_hide_pointer_pc34_compat(void)
{
}

void F1162_USIO_06_HidePointer(void)
{
    csb_v1_f1162_usio_06_hide_pointer_pc34_compat();
}

const char *csb_v1_f1162_usio_06_hide_pointer_source_evidence_pc34(void)
{
    return "ReDMCSB USIO1.C:43 F1162_USIO_06_HidePointer; Amiga/USIO "
           "pointer visibility boundary, no PC34 cursor route is synthesized";
}

void csb_v1_f1163_usio_07_show_pointer_pc34_compat(void)
{
}

void F1163_USIO_07_ShowPointer(void)
{
    csb_v1_f1163_usio_07_show_pointer_pc34_compat();
}

const char *csb_v1_f1163_usio_07_show_pointer_source_evidence_pc34(void)
{
    return "ReDMCSB USIO1.C:61 F1163_USIO_07_ShowPointer; Amiga/USIO "
           "pointer visibility boundary, no PC34 cursor route is synthesized";
}
