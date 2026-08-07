#include "dm2_v1_fmtowns_cross_game_shared_ranges.h"
#include <stddef.h>

const dm2_v1_fmtowns_cross_range_t
dm2_v1_fmtowns_dm1_to_dm2_ranges[DM2_V1_FMTOWNS_DM1_TO_DM2_RANGE_COUNT] = {
    { 0x029788u, 0x006928u, 32015u, 0x029588u, 0x006728u },
    { 0x03650bu, 0x006917u, 32005u, 0x03630bu, 0x006717u },
    { 0x031abeu, 0x006927u, 18614u, 0x0318beu, 0x006727u },
    { 0x024a47u, 0x006933u,  4097u, 0x024847u, 0x006733u },
    { 0x025a49u, 0x005901u,  2131u, 0x025849u, 0x005701u },
    { 0x0262d7u, 0x004fbbu,  1417u, 0x0260d7u, 0x004dbbu },
    { 0x03fdf1u, 0x004fe1u,  1075u, 0x03fbf1u, 0x004de1u },
    { 0x023ff7u, 0x0016b7u,   861u, 0x023df7u, 0x0014b7u },
    { 0x040347u, 0x0016e3u,   597u, 0x040147u, 0x0014e3u },
};

const dm2_v1_fmtowns_cross_range_t
dm2_v1_fmtowns_csb_to_dm2_ranges[DM2_V1_FMTOWNS_CSB_TO_DM2_RANGE_COUNT] = {
    { 0x02d678u, 0x00693cu, 32015u, 0x02d478u, 0x00673cu },
    { 0x03a4c7u, 0x00694bu, 32013u, 0x03a2c7u, 0x00674bu },
    { 0x035a86u, 0x00693fu, 18614u, 0x035886u, 0x00673fu },
    { 0x02a5f3u, 0x006923u,  4097u, 0x02a3f3u, 0x006723u },
    { 0x02b5f5u, 0x005901u,  2131u, 0x02b3f5u, 0x005701u },
    { 0x02be73u, 0x004fabu,  1417u, 0x02bc73u, 0x004dabu },
    { 0x044b2cu, 0x004fd8u,  1300u, 0x04492cu, 0x004dd8u },
    { 0x044bfeu, 0x01407eu,  1206u, 0x0449feu, 0x013e7eu },
    { 0x029bb7u, 0x0016b7u,   857u, 0x0299b7u, 0x0014b7u },
    { 0x04515fu, 0x0016ebu,   517u, 0x044f5fu, 0x0014ebu },
};

static int lookup(const dm2_v1_fmtowns_cross_range_t *tbl, int n,
                  uint32_t src, uint32_t *out) {
    if (!out) return 0;
    for (int i = 0; i < n; ++i) {
        if (src >= tbl[i].src_file_offset &&
            src <  tbl[i].src_file_offset + tbl[i].length_bytes) {
            *out = tbl[i].dm2_file_offset + (src - tbl[i].src_file_offset);
            return 1;
        }
    }
    return 0;
}

int dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(
        uint32_t dm1_file_offset, uint32_t *dm2_file_offset_out) {
    return lookup(dm2_v1_fmtowns_dm1_to_dm2_ranges,
                  DM2_V1_FMTOWNS_DM1_TO_DM2_RANGE_COUNT,
                  dm1_file_offset, dm2_file_offset_out);
}

int dm2_v1_fmtowns_csb_to_dm2_file_offset_pc34(
        uint32_t csb_file_offset, uint32_t *dm2_file_offset_out) {
    return lookup(dm2_v1_fmtowns_csb_to_dm2_ranges,
                  DM2_V1_FMTOWNS_CSB_TO_DM2_RANGE_COUNT,
                  csb_file_offset, dm2_file_offset_out);
}
