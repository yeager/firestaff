#include "dm1_v1_fmtowns_oicon_descriptor.h"

/* Byte 0 of each 6-byte OICON descriptor at [0x224db..0x2500b] in
 * the shipping HMA-240 English EDM.EXP. See header for provenance. */
const uint8_t
dm1_v1_fmtowns_oicon_kind[DM1_V1_FMTOWNS_OICON_KIND_COUNT] = {
      0,  0,  0,  0,  0, 42,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0, 42,  0, 43,  7,  5,  6,  8,  9, 10, 11, 12,
     13, 13, 14, 15, 15, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 24,
     27, 27, 26, 26, 27, 42, 40, 42,  5,  5, 28, 29, 30, 31, 32, 33,
      5, 35, 36, 27,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0, 41, 41, 41, 41,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 41,
      0,  0,  0,  0, 41,  0,  0,  0,  0, 41,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0, 37, 37, 37,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0, 38, 38,  0, 39,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,  0,  0,  0,128,  0,
      0,  0,128,130,131,  0,112,129,113,  0,255,  2,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  2,  2,  0,  0,  0,  0,  0, 20, 30, 10, 10,
};

uint8_t dm1_v1_fmtowns_oicon_kind_at_pc34(uint16_t oicon_index) {
    if (oicon_index >= DM1_V1_FMTOWNS_OICON_KIND_COUNT) return 0xff;
    return dm1_v1_fmtowns_oicon_kind[oicon_index];
}

int dm1_v1_fmtowns_oicon_is_thing_pc34(uint16_t oicon_index) {
    if (oicon_index >= DM1_V1_FMTOWNS_OICON_KIND_COUNT) return 0;
    return dm1_v1_fmtowns_oicon_kind[oicon_index] != 0;
}
