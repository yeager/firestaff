#ifndef DM1_V1_FMTOWNS_DIRECT_IO_H
#define DM1_V1_FMTOWNS_DIRECT_IO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns direct I/O port audit for DM1.
 *
 * Recovered 2026-08-07 by scanning EDM.EXP, JDM.EXP and TMENU.EXP
 * for `mov dx, imm16 ; in/out {dx | al,dx}` sequences. Port
 * semantics cross-referenced against Tsugaru emulator's
 * `src/towns/townsdef/townsdef.h` (Soji Yamakawa, BSD-3).
 *
 * KEY FINDING: DM1's game executable (EDM.EXP + JDM.EXP) touches
 * ONLY ONE hardware port directly (0x04E9 Sound INT reason). Every
 * other real-mode transition goes through the Phar Lap fs:[0x20]
 * TBIOS bridge. This means a hosted BIOS integration needs to
 * implement 0x04E9 + the four fs: slots to run DM1's game code.
 *
 * TMENU.EXP touches 5 ports because it owns the launcher / disc
 * boot: CMOS registers (0x3180/3182/3184), RS232C (0x0A0A), Sound
 * INT reason (0x04E9).
 *
 * Byte-verified counts:
 *
 *   Binary       0x04E9  0x0A0A  0x3180  0x3182  0x3184  total
 *   EDM.EXP        1       0       0       0       0       1
 *   JDM.EXP        1       0       0       0       0       1
 *   TMENU.EXP      1       1       2       1       2       7
 */

/* Ports the DM1/TMENU FM Towns runtime touches directly. Names
 * mirror Tsugaru's townsdef.h. */
#define DM1_V1_FMTOWNS_IO_SOUND_INT_REASON        0x04E9U /* [2] p.19 */
#define DM1_V1_FMTOWNS_IO_RS232C_MODEM_CONTROL    0x0A0AU /* Techn. p.21 */
#define DM1_V1_FMTOWNS_IO_CMOS_BOOT_DEV_FLAG      0x3180U /* CMOS */
#define DM1_V1_FMTOWNS_IO_CMOS_DEF_BOOT_DEV_TYPE  0x3182U /* 1=HD 2=FD 8=CD */
#define DM1_V1_FMTOWNS_IO_CMOS_DEF_BOOT_DEV_UNIT  0x3184U /* CMOS */

/* Byte-verified per-binary access profile. */
typedef struct {
    uint32_t sound_int_reason;
    uint32_t rs232c_modem_control;
    uint32_t cmos_boot_dev_flag;
    uint32_t cmos_def_boot_dev_type;
    uint32_t cmos_def_boot_dev_unit;
    uint32_t total;
} dm1_v1_fmtowns_direct_io_profile_t;

extern const dm1_v1_fmtowns_direct_io_profile_t
    dm1_v1_fmtowns_direct_io_profile_edm_exp_pc34;
extern const dm1_v1_fmtowns_direct_io_profile_t
    dm1_v1_fmtowns_direct_io_profile_jdm_exp_pc34;
extern const dm1_v1_fmtowns_direct_io_profile_t
    dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34;

/* Scan a Phar Lap P3 image for `mov dx, imm16 ; in/out` sequences
 * and count how many hit each of the five DM1-relevant ports.
 * Returns 1 on success. Non-DM1 ports are ignored (a caller that
 * wants to detect drift adds its own catch-all). */
int dm1_v1_fmtowns_direct_io_count_pc34(
    const uint8_t                        *image,
    unsigned long                         image_size,
    dm1_v1_fmtowns_direct_io_profile_t   *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_DIRECT_IO_H */
