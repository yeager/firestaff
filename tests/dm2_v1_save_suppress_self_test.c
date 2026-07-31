/* Test-only SUPPRESS codec self-check. Runtime save admission operates on
 * authenticated save candidates and must never carry fixed diagnostic bytes. */

#include "dm2_v1_save_load.h"

#include <string.h>

bool dm2_v1_save_suppress_self_test(void)
{
    /* SKProject c_savegame.cpp DM2_SUPPRESS_WRITER: masks select source
     * bit positions, scanning 7 -> 0 and emitting MSB-first. */
    const uint8_t data[3] = { 0x81u, 0x00u, 0xD2u };
    const uint8_t mask[3] = { 0x81u, 0x42u, 0xFFu };
    uint8_t enc[64];
    uint8_t dec[3];
    int enc_sz = dm2_suppress_encode(data, mask, 3u, enc, sizeof(enc));

    if (enc_sz < 0 ||
        dm2_suppress_decode(enc, (size_t)enc_sz, mask, 3u, dec, 0) < 0) {
        return false;
    }
    return memcmp(data, dec, sizeof(data)) == 0;
}
