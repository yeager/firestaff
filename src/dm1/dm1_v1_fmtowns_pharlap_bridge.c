#include "dm1_v1_fmtowns_pharlap_bridge.h"

const dm1_v1_fmtowns_pharlap_call_profile_t
dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34 = {
    .slot_tbios         = 70,
    .slot_secondary     = 20,
    .slot_timing        = 1,
    .slot_hardware_init = 2,
    .total              = 93
};

const dm1_v1_fmtowns_pharlap_call_profile_t
dm1_v1_fmtowns_pharlap_profile_jdm_exp_pc34 = {
    .slot_tbios         = 70,
    .slot_secondary     = 20,
    .slot_timing        = 1,
    .slot_hardware_init = 2,
    .total              = 93
};

const dm1_v1_fmtowns_pharlap_call_profile_t
dm1_v1_fmtowns_pharlap_profile_tmenu_exp_pc34 = {
    .slot_tbios         = 70,
    .slot_secondary     = 20,
    .slot_timing        = 1,
    .slot_hardware_init = 1,
    .total              = 92
};

int dm1_v1_fmtowns_pharlap_slot_layout_is_valid_pc34(void) {
    return DM1_V1_FMTOWNS_PHARLAP_SLOT_TBIOS         == 0x20U &&
           DM1_V1_FMTOWNS_PHARLAP_SLOT_SECONDARY     == 0x40U &&
           DM1_V1_FMTOWNS_PHARLAP_SLOT_TIMING        == 0x48U &&
           DM1_V1_FMTOWNS_PHARLAP_SLOT_HARDWARE_INIT == 0x80U &&
           DM1_V1_FMTOWNS_PHARLAP_REALMODE_SELECTOR  == 0x110U;
}

int dm1_v1_fmtowns_pharlap_count_call_sites_pc34(
        const uint8_t *image, unsigned long image_size,
        dm1_v1_fmtowns_pharlap_call_profile_t *out) {
    unsigned long i;
    if (!image || !out) return 0;
    if (image_size < 7UL) return 0;
    out->slot_tbios = 0;
    out->slot_secondary = 0;
    out->slot_timing = 0;
    out->slot_hardware_init = 0;
    out->total = 0;
    for (i = 0; i + 7UL <= image_size; ++i) {
        /* Pattern: 64 ff 1d <disp32>  =  call far ptr fs:[disp32]. */
        if (image[i] != 0x64 || image[i+1] != 0xff || image[i+2] != 0x1d) {
            continue;
        }
        uint32_t disp = (uint32_t)image[i+3] |
                        ((uint32_t)image[i+4] << 8) |
                        ((uint32_t)image[i+5] << 16) |
                        ((uint32_t)image[i+6] << 24);
        switch (disp) {
            case 0x20U: ++out->slot_tbios; ++out->total; break;
            case 0x40U: ++out->slot_secondary; ++out->total; break;
            case 0x48U: ++out->slot_timing; ++out->total; break;
            case 0x80U: ++out->slot_hardware_init; ++out->total; break;
            default: /* not a canonical DM1 slot; ignore */ break;
        }
    }
    return 1;
}
