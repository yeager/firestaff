#include "redmcsb_f0085_blockcmp_pc34_compat.h"

int16_t redmcsb_f0085_blockcmp_pc34_compat(
    const void *left,
    const void *right,
    int16_t byte_count)
{
    const uint8_t *a = (const uint8_t *)left;
    const uint8_t *b = (const uint8_t *)right;
    int16_t i;

    if (byte_count <= 0) {
        return 0;
    }
    if (!a || !b) {
        return (int16_t)(a == b ? 0 : (a ? 1 : -1));
    }

    for (i = 0; i < byte_count; ++i) {
        if (a[i] != b[i]) {
            return (int16_t)((a[i] < b[i]) ? -1 : 1);
        }
    }
    return 0;
}

int16_t F0085_blockcmp(const void *left, const void *right, int16_t byte_count)
{
    return redmcsb_f0085_blockcmp_pc34_compat(left, right, byte_count);
}

const char *redmcsb_f0085_blockcmp_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:6902 F0085: Megamax C _blockcmp runtime helper "
           "for comparing structure bytes; PC34 shim returns zero/equal or "
           "signed ordering without reading beyond the caller byte count.";
}
