#include "dm1_v1_fmtowns_direct_io.h"

const dm1_v1_fmtowns_direct_io_profile_t
dm1_v1_fmtowns_direct_io_profile_edm_exp_pc34 = {
    .sound_int_reason       = 1,
    .rs232c_modem_control   = 0,
    .cmos_boot_dev_flag     = 0,
    .cmos_def_boot_dev_type = 0,
    .cmos_def_boot_dev_unit = 0,
    .total                  = 1
};

const dm1_v1_fmtowns_direct_io_profile_t
dm1_v1_fmtowns_direct_io_profile_jdm_exp_pc34 = {
    .sound_int_reason       = 1,
    .rs232c_modem_control   = 0,
    .cmos_boot_dev_flag     = 0,
    .cmos_def_boot_dev_type = 0,
    .cmos_def_boot_dev_unit = 0,
    .total                  = 1
};

const dm1_v1_fmtowns_direct_io_profile_t
dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34 = {
    .sound_int_reason       = 1,
    .rs232c_modem_control   = 1,
    .cmos_boot_dev_flag     = 2,
    .cmos_def_boot_dev_type = 1,
    .cmos_def_boot_dev_unit = 2,
    .total                  = 7
};

int dm1_v1_fmtowns_direct_io_count_pc34(
        const uint8_t *image, unsigned long image_size,
        dm1_v1_fmtowns_direct_io_profile_t *out) {
    unsigned long i;
    if (!image || !out) return 0;
    if (image_size < 6UL) return 0;
    out->sound_int_reason = 0;
    out->rs232c_modem_control = 0;
    out->cmos_boot_dev_flag = 0;
    out->cmos_def_boot_dev_type = 0;
    out->cmos_def_boot_dev_unit = 0;
    out->total = 0;
    for (i = 0; i + 5UL <= image_size; ++i) {
        /* Pattern: 66 ba disp16 <in/out opcode>. The operand-size
         * prefix 0x66 tells the 32-bit-mode decoder to treat the
         * following ba as `mov dx, imm16` (5 bytes) rather than
         * `mov edx, imm32` (5 bytes without the prefix). */
        if (image[i] != 0x66 || image[i+1] != 0xba) continue;
        unsigned int port = (unsigned int)image[i+2] |
                            ((unsigned int)image[i+3] << 8);
        unsigned int op = image[i+4];
        if (op != 0xec && op != 0xed && op != 0xee && op != 0xef) continue;
        switch (port) {
            case DM1_V1_FMTOWNS_IO_SOUND_INT_REASON:
                ++out->sound_int_reason; ++out->total; break;
            case DM1_V1_FMTOWNS_IO_RS232C_MODEM_CONTROL:
                ++out->rs232c_modem_control; ++out->total; break;
            case DM1_V1_FMTOWNS_IO_CMOS_BOOT_DEV_FLAG:
                ++out->cmos_boot_dev_flag; ++out->total; break;
            case DM1_V1_FMTOWNS_IO_CMOS_DEF_BOOT_DEV_TYPE:
                ++out->cmos_def_boot_dev_type; ++out->total; break;
            case DM1_V1_FMTOWNS_IO_CMOS_DEF_BOOT_DEV_UNIT:
                ++out->cmos_def_boot_dev_unit; ++out->total; break;
            default: /* not a DM1-relevant port */ break;
        }
    }
    return 1;
}
