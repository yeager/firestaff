#include "theron_v1_iso_end_receipt.h"

#include "theron_v1_track02.h"

int main(void) {
    unsigned char jp[32] = {0};
    unsigned char us[32] = {0};
    unsigned int same = 0u;
    unsigned int diff = 0u;
    Theron_V1IsoEndSpan ok[] = {{0u, 8u}, {8u, 8u}};
    Theron_V1IsoEndReceipt receipt;

    us[8] = 1u;
    if (!theron_v1_iso_end_receipt(THERON_TRACK02_MD5_US_ISO,
                                   612352u,
                                   ok,
                                   2u,
                                   &receipt) ||
        !receipt.opaque_only ||
        receipt.loader_usable ||
        receipt.bitmap_usable ||
        receipt.level_route_usable) {
        return 1;
    }
    if (theron_v1_iso_end_receipt(THERON_TRACK02_MD5_US_BIN,
                                  612352u,
                                  ok,
                                  2u,
                                  &receipt)) {
        return 1;
    }
    return theron_v1_iso_end_compare(jp, 32u, us, 32u, ok, 2u, &same,
                                     &diff) &&
        same == 1u && diff == 1u ? 0 : 1;
}
