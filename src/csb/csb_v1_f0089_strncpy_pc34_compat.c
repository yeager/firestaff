#include "csb_v1_f0089_strncpy_pc34_compat.h"

#include "redmcsb_f0089_strncpy.h"

char *csb_v1_f0089_strncpy_pc34_compat(
    char *destination,
    const char *source,
    int16_t count)
{
    char *write;

    if (!destination || !source || count <= 0) {
        return destination;
    }

    write = destination;
    while (count-- > 0) {
        const char c = *source++;
        *write++ = c;
        if (c == '\0') {
            break;
        }
    }
    return destination;
}

char *F0089_strncpy(char *destination, const char *source, int16_t count)
{
    return csb_v1_f0089_strncpy_pc34_compat(destination, source, count);
}

char *redmcsb_f0089_strncpy(char *destination, const char *source, int16_t count)
{
    return csb_v1_f0089_strncpy_pc34_compat(destination, source, count);
}

const char *csb_v1_f0089_strncpy_source_evidence_pc34(void)
{
    return "ReDMCSB STRING.C F0089_strncpy / DEFS.H:3085: "
           "copy at most signed 16-bit count bytes, stop after copying the "
           "source NUL, and do not pad the remaining destination bytes.";
}
